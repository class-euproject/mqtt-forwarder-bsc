#ifndef RECEIVER_H
#define RECEIVER_H

#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include "../masa_protocol/include/communicator.hpp"
#include "../masa_protocol/include/messages.hpp"

using namespace masa;

namespace fog {

typedef void * (*THREADFUNCPTR)(void *);

class Receiver {

private:
    pthread_t snifferThread;
    int port;
    int socketDesc;
    Communicator<MasaMessage> *comm;
public:
    Receiver(int port_);
    ~Receiver();
    void start();
    void end();
    void *receive(void *n);
};
}

#endif /* RECEIVER_H */
