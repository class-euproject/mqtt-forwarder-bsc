#include <iostream>
#include <pthread.h>
#include "Receiver.h"

int main(int argc, char **argv) {

    fog::Receiver r(8888);
    r.start();

    r.end();
    return EXIT_SUCCESS;
}
