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

	MasaMessage *sync_message = new MasaMessage;
	MasaMessage *dummy = new MasaMessage;
	dummy->objects.clear();
	dummy->lights.clear();
	dummy->num_objects = 0;
	dummy->cam_idx = 0;

	sendr->send_message(dummy, sendPort);
	std::cout << "Request SENT!" << std::endl;
	while (!recv->receive_message(socketDesc, sync_message))
	{
		std::cout << "RECEIVED!" << std::endl;
		std::cout << "Message sent " << sync_message->cam_idx << " " << sync_message->t_stamp_ms << std::endl;
		if (sync_message->cam_idx == 0)
			break;
	}
	t2 = high_resolution_clock::now();
	duration<double, std::milli> ms_double = t2 - t1;
	std::cout << ms_double.count() << " DURATION ms \n"<<flush;
	return 0;
}
