#ifndef FORWARDER_H
#define FORWARDER_H

#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <thread>
#include "LogWriter.h"
#include "ClassAggregatorMessage.h"
#include "../masa_protocol/include/communicator.hpp"

namespace fog {

typedef void * (*THREADFUNCPTR)(void *);

class Forwarder {

private:
    ClassAggregatorMessage *cm;
    int socketDesc;
    int sendPort;
    Communicator<MasaMessage> *sendr;
    Communicator<MasaMessage> *recv;
    LogWriter *lw;
    bool lw_flag;
    pthread_t writerThread;

public:
    Forwarder(ClassAggregatorMessage &sharedMessage,
            int recvPort_,
            int sendPort_,
            bool logWriterFlag);
    ~Forwarder();
    void start();
    void end();
    void *send(void *n);
};
}

#endif /* FORWARDER_H */