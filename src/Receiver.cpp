#include "Receiver.h"

namespace fog {

Receiver::Receiver(ClassAggregatorMessage &sharedMessage,
                   int port_) {
    port = port_;
    cm = &sharedMessage;
    comm = new Communicator<MasaMessage>(SOCK_DGRAM);

    comm->open_server_socket(port);
    socketDesc = comm->get_socket();
    if (socketDesc == -1)
        perror("error in socket");
}

Receiver::~Receiver() {
    delete comm;
}

void Receiver::start() {
    if (pthread_create(&snifferThread, NULL, (THREADFUNCPTR) &Receiver::receive, this)) 
        perror("could not create thread");
}

void Receiver::end() {
    pthread_join(snifferThread, NULL);
}

void * Receiver::receive(void *n) {
    MasaMessage *m = new MasaMessage();
    while (this->comm->receive_message(this->socketDesc, m) == 0) {
        if((*m).cam_idx == 20 or (*m).cam_idx == 21 or (*m).cam_idx == 30 or (*m).cam_idx == 31 or (*m).cam_idx == 40){
            this->cm->insertMessage(*m);
        }
    }
    delete m;
    return (void *)NULL;
}
}