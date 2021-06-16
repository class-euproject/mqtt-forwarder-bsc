#include "Receiver.h"
#include "Tracker.h"
#include "../class-tracker/include/obj.h"
#include "../class-tracker/include/Tracking.h"
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>

namespace fog {

Receiver::Receiver(ClassAggregatorMessage &sharedMessage,
                   int port_,
                   bool logWriterFlag) {
    port = port_;
    cm = &sharedMessage;
    comm = new Communicator<MasaMessage>(SOCK_DGRAM);

    comm->open_server_socket(port);
    socketDesc = comm->get_socket();
    if (socketDesc == -1)
        perror("error in socket");

    lw_flag = logWriterFlag;
    if (lw_flag){
        lw = new LogWriter("../log/recived_messages/");
    }
}

Receiver::~Receiver() {
    delete lw;
    delete comm;
}

void Receiver::start() {
    if (pthread_create(&snifferThread, NULL, (THREADFUNCPTR) &Receiver::receive, this)) 
        perror("could not create thread");
}

void Receiver::end() {
    pthread_join(snifferThread, NULL);
}

void GPS2pixel(double *adfGeoTransform, double lat, double lon, int &x, int &y){
    //conversion from GPS to pixels, via georeferenced map parameters
    x = int(round( (lon - adfGeoTransform[0]) / adfGeoTransform[1]) );
    y = int(round( (lat - adfGeoTransform[3]) / adfGeoTransform[5]) );
}

/**
 *  Convert orientation from radian to quantized degree (from 360 to 255)
*/
uint8_t orientation_to_uint8(float yaw)
{
    // 57.29 is (180/pi) -> conversion in degrees
    // 17 / 24 is (255/360) -> quantization
    // let a yaw in radians, it is converted into degrees (*57.29), 
    // then into positive degrees, then it is quantized into 255. 
    uint8_t orientation = uint8_t((int((yaw * 57.29 + 360)) % 360) * 17 / 24);
    return orientation;
}

/**
 *  Convert speed from m/s to quantized km/h every 1/2 km/h
*/
uint8_t speed_to_uint8(float vel)
{
    // 3.6 -> conversion in km/h
    // *2 -> quantization km/h evrey 1/2
    // let a velocity in m/s, it is converted into km/h (*3.6), then (*2) 
    // we achive a double speed. In a urban track we can consider a maximum 
    // speed of 127 km/h. So we can fit 127 on a byte with a multiplication 
    // by 2. Each increment corresponds to a speed greater than 1/2 km/h.
    uint8_t velocity = uint8_t(std::abs(vel * 3.6 * 2));
    return velocity;
}  

void create_message_from_tracker(const std::vector<tracking::Tracker> &trackers, MasaMessage *m, 
                                 geodetic_converter::GeodeticConverter &gc, double *adfGeoTransform,
                                 const std::vector<uint16_t> camera_id) {
    double lat, lon, alt;
    for (auto t : trackers) {
        if (t.predList.size() > 0) {
            Categories cat = (Categories) t.cl;
            gc.enu2Geodetic(t.predList.back().x, t.predList.back().y, 0, &lat, &lon, &alt);
            int pix_x, pix_y;
            GPS2pixel(adfGeoTransform, lat, lon, pix_x, pix_y);
            uint8_t orientation = orientation_to_uint8(t.predList.back().yaw);
            uint8_t velocity = speed_to_uint8(t.predList.back().vel);
            RoadUser r;
            r.camera_id = camera_id;
            r.latitude = static_cast<float>(lat);
            r.longitude = static_cast<float>(lon);
            std::vector<uint16_t> obj_id_vector;
            obj_id_vector.push_back(t.id);
            r.object_id = obj_id_vector;
            r.error = t.traj.back().error;
            r.speed = velocity;
            r.orientation = orientation;
            r.category = cat;
            //std::cout << std::setprecision(10) << r.latitude << " , " << r.longitude << " " << int(r.speed) << " " << int(r.orientation) << " " << r.category << std::endl;
            m->objects.push_back(r);
        }
    }
}


/**
 * Check if the messages contain the tracker information (camera_id and object_id vectors).
 * If object_id vector is empty, then fill it with that information.
*/
std::vector<MasaMessage> fillTrackerInfo(std::vector<MasaMessage> input_messages, 
                                        tracking::Tracking *tr, 
                                        geodetic_converter::GeodeticConverter gc,  
                                        double *adfGeoTransform) {
    // std::cout<<"messages: "<<input_messages.size()<<std::endl;
    std::vector<MasaMessage> copy = input_messages;
    std::vector<int> delete_ids;

    MasaMessage tracked_message;
    std::vector<uint16_t> camera_id;
    int count_no_tracking_messages = 0;
    //copy the deduplicated objects into a single MasaMessage. Check if some objects need to be tracked
    std::vector<tracking::obj_m> objects_to_track;
    for(size_t i = 0; i < input_messages.size(); i++) {

        // the first object could be a special car, so skip it and check on the second one.
        // If it is a connected vehicle or it is a smart vehicle with no other road user, its size is equal to one.
        // If it is a Traffic Light, objects vector is empty.
        if(input_messages.at(i).objects.size() <= 1 || 
            input_messages.at(i).objects.size() > 1 && input_messages.at(i).objects.at(1).object_id.size()!=0) 
            continue;
        
        //TODO: only a message with no tracker information is supported. For each of this kind of message a tracker is needed 
        if(count_no_tracking_messages == 1) {
            std::cout<<"WARNING!!! TOO MANY MESSAGES WITHOUT TRACKING INFORMATION\n";
            continue;
        }

        delete_ids.push_back(i);
        //The smart vehicle does not need to be tracked so it can immediately be pushed in output messages
        tracked_message.cam_idx = input_messages.at(i).cam_idx;
        tracked_message.t_stamp_ms = input_messages.at(i).t_stamp_ms;
        tracked_message.objects.push_back(input_messages.at(i).objects.at(0));
        for(size_t j = 1; j < input_messages.at(i).objects.size(); j++) {
            double north, east, up;
            gc.geodetic2Enu(input_messages.at(i).objects.at(j).latitude, input_messages.at(i).objects.at(j).longitude, 0, &east, &north, &up);
            objects_to_track.push_back(tracking::obj_m(east, north, 0, input_messages.at(i).objects.at(j).category, 1, 1, 0));
        }
        count_no_tracking_messages ++;
    }

    //if some object need to be tracked, track it
    if(objects_to_track.size() > 0){
        camera_id.push_back(tracked_message.cam_idx);
        tr->track(objects_to_track, false);
        create_message_from_tracker(tr->getTrackers(), &tracked_message, gc, adfGeoTransform, camera_id);
    }
    tracked_message.num_objects = tracked_message.objects.size();

    if (delete_ids.size()==0)
        return input_messages;
    if (delete_ids.size() != 1)
        std::sort(delete_ids.begin(), delete_ids.end(), [](int a, int b) {return a > b; });
    delete_ids.erase( std::unique( delete_ids.begin(), delete_ids.end() ), delete_ids.end() );
    for(auto d : delete_ids)
        copy.erase(copy.begin() + d);
    copy.push_back(tracked_message);
    return copy;
}

void readTiff(char *filename, double *adfGeoTransform) {
    GDALDataset *poDataset;
    GDALAllRegister();
    poDataset = (GDALDataset *)GDALOpen(filename, GA_ReadOnly);
    if (poDataset != NULL)
    {
        poDataset->GetGeoTransform(adfGeoTransform);
    }
}

void * Receiver::receive(void *n) {
    MasaMessage *m = new MasaMessage();

    double *adfGeoTransform = (double *)malloc(6 * sizeof(double));
    readTiff("../data/MASA_4670.tif", adfGeoTransform);
    int initialAge = 5;//8; //15; //-5;
    int nStates = 5;
    float dt = 0.03;
    bool trVerbose = false;
    geodetic_converter::GeodeticConverter gc;
    gc.initialiseReference(44.655540, 10.934315, 0);
    tracking::Tracking *tr = new tracking::Tracking(nStates, dt, initialAge, tracking::UKF_t);

    while (this->comm->receive_message(this->socketDesc, m) == 0) {
        if((*m).cam_idx == 20 or (*m).cam_idx == 21 or (*m).cam_idx == 30 or (*m).cam_idx == 31 or (*m).cam_idx == 40){
            auto tracked_messages = fillTrackerInfo({(*m)}, tr, gc, adfGeoTransform);
            (*m) = tracked_messages.at(0);
            this->cm->insertMessage(*m);
            if(this->lw_flag) {
                this->lw->write(*m);
            }
	        std::cout << "Message acquired: " << m->cam_idx << " " << m->t_stamp_ms << " with "<< m->objects.size() << " objects" <<std::endl;
        }
    }
    delete m;
    delete adfGeoTransform;
    return (void *)NULL;
}
}
