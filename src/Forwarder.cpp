#include "Forwarder.h"

#include <chrono>
using namespace std::chrono;

namespace fog
{

    Forwarder::Forwarder(ClassAggregatorMessage &sharedMessage,
                         int recvPort_,
                         int sendPort_,
                         bool logWriterFlag)
    {

        cm = &sharedMessage;
        sendPort = sendPort_;
        sendr = new Communicator<MasaMessage>(SOCK_DGRAM);
        //sendr->open_client_socket("172.17.0.3", sendPort_);
        sendr->open_client_socket("127.0.0.1", sendPort_);

        recv = new Communicator<MasaMessage>(SOCK_DGRAM);
        recv->open_server_socket(recvPort_);
        socketDesc = recv->get_socket();

        lw_flag = logWriterFlag;
        if (lw_flag){
            lw = new LogWriter("../log/sent_messages/");
        }
    }

    Forwarder::~Forwarder()
    {
        delete lw;
        delete sendr;
        delete recv;
    }

    void Forwarder::start()
    {
        if (pthread_create(&writerThread, NULL, (THREADFUNCPTR)&Forwarder::send, this))
            perror("could not create thread");
    }

    void Forwarder::end()
    {
        pthread_join(writerThread, NULL);
    }

    void *Forwarder::send(void *n)
    {
        std::vector<MasaMessage> input_messages;
        MasaMessage *sync_message = new MasaMessage;
        MasaMessage *dummy = new MasaMessage;
        dummy->objects.clear();
        dummy->lights.clear();
        dummy->num_objects = 0;
        dummy->cam_idx = 0;
        while (true)
        {
            recv->receive_message(socketDesc, sync_message);
            std::cout << "Request RECEIVED!"<<std::endl;
            high_resolution_clock::time_point t1 = high_resolution_clock::now(), t2;
	        t1 = high_resolution_clock::now();
	            
            input_messages = this->cm->getMessages();
            auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(high_resolution_clock::now().time_since_epoch()).count();

            for (int i = 0; i < input_messages.size(); i++)
            {
                input_messages.at(i).t_stamp_ms = timestamp_ms;
                sendr->send_message(&input_messages.at(i), sendPort);
                std::cout << "Message sent " << input_messages.at(i).cam_idx << " " << input_messages.at(i).t_stamp_ms << std::endl;
                if(this->lw_flag) {
                    this->lw->write(input_messages.at(i));
                }
            }
            sendr->send_message(dummy, sendPort);
            input_messages.clear();
            t2 = high_resolution_clock::now();
	        duration<double, std::milli> ms_double = t2 - t1;
            std::cout << ms_double.count() << " DURATION ms \n"<<std::flush;
        }
        return (void *)NULL;
    }
}
