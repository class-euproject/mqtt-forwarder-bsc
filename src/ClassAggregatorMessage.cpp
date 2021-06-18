#include "ClassAggregatorMessage.h"

namespace fog {

ClassAggregatorMessage::ClassAggregatorMessage() {
    pthread_mutexattr_t mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutex_init(&car1.mutex1, &mutexattr);
    pthread_mutex_init(&car1.mutex2, &mutexattr);
    pthread_mutex_init(&car2.mutex1, &mutexattr);
    pthread_mutex_init(&car2.mutex2, &mutexattr);
    pthread_mutex_init(&car3.mutex1, &mutexattr);
    pthread_mutex_init(&car3.mutex2, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    car1.id = 20;
    car2.id = 30;
    car3.id = 31;
}

bool ClassAggregatorMessage::deleteOld(MasaMessage m, std::vector<MasaMessage> messageList) {
    bool deleted = false;
    auto it = messageList.begin();
    while (it != messageList.end()) {
        if (it->cam_idx == m.cam_idx && it->t_stamp_ms < m.t_stamp_ms) {
            it = messageList.erase(it);
            deleted = true;
        }
        else 
            ++it;
    }
    return deleted;
}

void ClassAggregatorMessage::insertMessage(MasaMessage m) {
    switch (m.cam_idx)
    {
        case 20:
            if(car1.selector == 0){
                pthread_mutex_lock(&car1.mutex1);
                deleteOld(m, car1.messageList1);
                car1.messageList1.push_back(m);
                car1.selector = 1;
                pthread_mutex_unlock(&car1.mutex1);
            } else {
                pthread_mutex_lock(&car1.mutex2);
                deleteOld(m, car1.messageList2);
                car1.messageList2.push_back(m);
                car1.selector = 0;
                pthread_mutex_unlock(&car1.mutex2);
            }
            break;

        case 30:
            if(car2.selector == 0){
                pthread_mutex_lock(&car2.mutex1);
                deleteOld(m, car2.messageList1);
                car2.messageList1.push_back(m);
                car2.selector = 1;
                pthread_mutex_unlock(&car2.mutex1);
            } else {
                pthread_mutex_lock(&car2.mutex2);
                deleteOld(m, car2.messageList2);
                car2.messageList2.push_back(m);
                car2.selector = 0;
                pthread_mutex_unlock(&car2.mutex2);
            }
            break;

        case 31:
            if(car3.selector == 0){
                pthread_mutex_lock(&car3.mutex1);
                deleteOld(m, car3.messageList1);
                car3.messageList1.push_back(m);
                car3.selector = 1;
                pthread_mutex_unlock(&car3.mutex1);
            } else {
                pthread_mutex_lock(&car3.mutex2);
                deleteOld(m, car3.messageList2);
                car3.messageList2.push_back(m);
                car3.selector = 0;
                pthread_mutex_unlock(&car3.mutex2);
            }
            break;
        
        default:
            break;
    }
    
}

MasaMessage getMessageFromCar(double_buffer& car){
    MasaMessage m;
    m.cam_idx=0;
    if (car.selector == 0)
    {
        if(car.messageList2.size() > 0){
            m = car.messageList2.at(0);
            car.messageList2.clear();
            return m;
        }
    }
    else
    {
        if(car.messageList1.size() > 0){
            m = car.messageList1.at(0);
            car.messageList1.clear();
            return m;
        }
    }
    return m;
    /*
    bool got_message = false;
    while(!got_message){
        if(pthread_mutex_trylock(&car.mutex1) > 0){
            if(car.messageList1.size() > 0){
                m = car.messageList1.at(0);
                car.messageList1.clear();
                got_message = true;
            }
            pthread_mutex_unlock(&car.mutex1);
            return m;
        } else if(pthread_mutex_trylock(&car.mutex2) > 0){
            if(car.messageList2.size() > 0){
                m = car.messageList2.at(0);
                car.messageList2.clear();
                got_message = true;
            }
            pthread_mutex_unlock(&car.mutex2);
            return m;
        }
    }
    */
}

std::vector<MasaMessage> ClassAggregatorMessage::getMessages() {

    std::vector<MasaMessage> copy_list;
    MasaMessage m = getMessageFromCar(car1);
    if (m.cam_idx != 0)
        copy_list.push_back(m);
    m = getMessageFromCar(car2);
    if (m.cam_idx != 0)
        copy_list.push_back(m);
    m = getMessageFromCar(car3);
    if (m.cam_idx != 0)
        copy_list.push_back(m);

//    copy_list.push_back(getMessageFromCar(car2));
//    copy_list.push_back(getMessageFromCar(car3));

    return copy_list;

}
}