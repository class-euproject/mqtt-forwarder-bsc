#include "Forwarder.h"

namespace fog
{

    Forwarder::Forwarder(ClassAggregatorMessage &sharedMessage,
                         int recvPort_,
                         int sendPort_)
    {

        cm = &sharedMessage;
        sendPort = sendPort_;
        sendr = new Communicator<MasaMessage>(SOCK_DGRAM);
        sendr->open_client_socket("172.17.0.2", sendPort_);

        recv = new Communicator<MasaMessage>(SOCK_DGRAM);
        recv->open_server_socket(recvPort_);
        socketDesc = recv->get_socket();
    }

    Forwarder::~Forwarder()
    {
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
            input_messages = this->cm->getMessages();
            // std::cout<<"send dim reading list: "<<input_messages.size()<<std::endl;
            if (input_messages.size() == 0)
            {
                sendr->send_message(dummy, sendPort);
                continue; // no received messages
            }

            for (int i = 0; i < input_messages.size(); i++)
            {
                sendr->send_message(&input_messages.at(i), sendPort);
                std::cout << "Message sent " << input_messages.at(i).cam_idx << " " << input_messages.at(i).t_stamp_ms << std::endl;
            }
            input_messages.clear();
        }
        return (void *)NULL;
    }
}
