#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <thread>
#include "../masa_protocol/include/communicator.hpp"
#include "../masa_protocol/include/messages.hpp"
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace masa;

int receive_messages(Communicator<MasaMessage> &comm, std::vector<MasaMessage> *input_messages)
{
    int message_size = 50000;
    char client_message[message_size];
    memset(client_message, 0, message_size);
    int len;
    struct sockaddr_in cliaddr{};
    int socket_desc = comm.get_socket();
    struct timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 5000; // 10000;
    bool receive = true;
    while (receive) {
        if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv)) >= 0) {
            int res = recvfrom(socket_desc, client_message, message_size, 0,
                     (struct sockaddr *) &cliaddr, (socklen_t *) &len);
            // std::cout << "Recvfrom size is " << res << " bytes" << std::endl;
            if (res > 0) {
                // we have read information coming from the car
                // std::cout << "Message actually received: ";
                std::string s((char *) client_message, message_size);
                MasaMessage m;
                comm.deserialize_coords(s, &m);
                // std::cout << m.num_objects << " objects coming from camera " << m.cam_idx << std::endl;
                if (m.objects.size() > 0) {
                    std::cout << "Message has more than 0 objects inside" << std::endl;
                    input_messages->push_back(m);
                }
                else {
                    std::cout << "Message does not have any object inside, dummy message. Exiting loop..." << std::endl;
                    break;
                }
            } else {
                std::cout << "No data received from recvfrom" << std::endl;
                receive = false;
            }
        } else {
            std::cout << "Error in setsockopt" << std::endl;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now(), t2;
	t1 = high_resolution_clock::now();
	
	Communicator<MasaMessage> *sendr;
	Communicator<MasaMessage> *recv;
	int sendPort = 18889;
	int recvPort = 18888;
	sendr = new Communicator<MasaMessage>(SOCK_DGRAM);
	//sendr->open_client_socket("172.17.0.2", sendPort_);
	sendr->open_client_socket("127.0.0.1", sendPort);

	recv = new Communicator<MasaMessage>(SOCK_DGRAM);
	recv->open_server_socket(recvPort);
	int socketDesc = recv->get_socket();

	std::vector<MasaMessage> sync_message;
	MasaMessage *dummy = new MasaMessage;
	dummy->objects.clear();
	dummy->lights.clear();
	dummy->num_objects = 0;
	dummy->cam_idx = 0;

	sendr->send_message(dummy, sendPort);
	std::cout << "Request SENT!" << std::endl;
	receive_messages(*recv, &sync_message);
	/*
	while (!recv->receive_message(socketDesc, sync_message))
	{
		std::cout << "RECEIVED!" << std::endl;
		std::cout << "Message sent " << sync_message->cam_idx << " " << sync_message->t_stamp_ms << std::endl;
		if (sync_message->cam_idx == 0)
			break;
	}
	*/
	t2 = high_resolution_clock::now();
	duration<double, std::milli> ms_double = t2 - t1;
	std::cout << ms_double.count() << " DURATION ms \n"<<flush;
	return 0;
}
