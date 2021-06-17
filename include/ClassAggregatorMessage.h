#ifndef CAMESSAGE_H
#define CAMESSAGE_H

#include <pthread.h>
#include <assert.h>
#include "../masa_protocol/include/messages.hpp"

using namespace masa;

namespace fog {

struct double_buffer{
    std::vector<MasaMessage> messageList1, messageList2;
    pthread_mutex_t mutex1, mutex2;
    u_short selector = 0;
    uint32_t id = -1;
};

class ClassAggregatorMessage {

protected:
    double_buffer car1, car2, car3;

public:
    ClassAggregatorMessage();
    ~ClassAggregatorMessage() {};
    static bool deleteOld(MasaMessage m, std::vector<MasaMessage> messageList);
    void insertMessage(MasaMessage m);
    std::vector<MasaMessage> getMessages();
    
    
};
}

#endif /* CAMESSAGE_H */
