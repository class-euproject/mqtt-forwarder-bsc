#include <iostream>
#include <pthread.h>
#include "ClassAggregatorMessage.h"
#include "Receiver.h"
#include "Forwarder.h"

int main(int argc, char **argv) {
    fog::ClassAggregatorMessage received_messages;       // Receiver fills this message, Deduplicator deduplicates its objects
    
    fog::Receiver r(received_messages, 8888, true);
    fog::Forwarder f(received_messages, 18889, 18888, true);
    r.start();
    f.start();
    f.end();
    r.end();
    return EXIT_SUCCESS;
}