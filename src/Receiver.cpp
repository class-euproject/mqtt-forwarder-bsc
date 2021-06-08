#include "Receiver.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"

using namespace std;

namespace fog {

const string DFLT_SERVER_ADDRESS	{ "tcp://192.168.7.42:1883" };
const string CLIENT_ID				{ "forwarder" };

const string TOPIC { "CC" };

const int  QOS = 0;

const auto TIMEOUT = std::chrono::milliseconds(10);

/////////////////////////////////////////////////////////////////////////////

/**
 * A callback class for use with the main MQTT client.
 */
class callback : public virtual mqtt::callback
{
public:
	void connection_lost(const string& cause) override {
		cout << "\nConnection lost" << endl;
		if (!cause.empty())
			cout << "\tcause: " << cause << endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		cout << "\tDelivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A base action listener.
 */
class action_listener : public virtual mqtt::iaction_listener
{
protected:
	void on_failure(const mqtt::token& tok) override {
		cout << "\tListener failure for token: "
			<< tok.get_message_id() << endl;
	}

	void on_success(const mqtt::token& tok) override {
		cout << "\tListener success for token: "
			<< tok.get_message_id() << endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A derived action listener for publish events.
 */
class delivery_action_listener : public action_listener
{
	atomic<bool> done_;

	void on_failure(const mqtt::token& tok) override {
		action_listener::on_failure(tok);
		done_ = true;
	}

	void on_success(const mqtt::token& tok) override {
		action_listener::on_success(tok);
		done_ = true;
	}

public:
	delivery_action_listener() : done_(false) {}
	bool is_done() const { return done_; }
};

/////////////////////////////////////////////////////////////////////////////


Receiver::Receiver(int port_) {
    port = port_;
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

	mqtt::async_client client(DFLT_SERVER_ADDRESS, CLIENT_ID);

	callback cb;
	client.set_callback(cb);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session()
		.will(mqtt::message(TOPIC, "EXIT", QOS))
		.finalize();

	cout << "  ...OK" << endl;

	try {
		cout << "\nConnecting..." << endl;
		mqtt::token_ptr conntok = client.connect(connOpts);
		cout << "Waiting for the connection..." << endl;
		conntok->wait();
		cout << "  ...OK" << endl;

		// Now try with itemized publish.
/*
		cout << "\nSending next message..." << endl;
		mqtt::delivery_token_ptr pubtok;
		pubtok = client.publish(TOPIC, PAYLOAD2, strlen(PAYLOAD2), QOS, false);
		cout << "  ...with token: " << pubtok->get_message_id() << endl;
		cout << "  ...for message with " << pubtok->get_message()->get_payload().size()
			<< " bytes" << endl;
		pubtok->wait_for(TIMEOUT);
		cout << "  ...OK" << endl;
*/

	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return (void *)1;
	}


    MasaMessage *m = new MasaMessage();
    Communicator<MasaMessage> bsc_forwarder(SOCK_DGRAM);
    //bsc_forwarder.open_client_socket("127.0.0.1", this->bsc_reciving_port);
    while (this->comm->receive_message(this->socketDesc, m) == 0) 
    {
        if((*m).cam_idx == 20 or (*m).cam_idx == 21 or (*m).cam_idx == 30 or (*m).cam_idx == 31 or (*m).cam_idx == 40)
        {
            /*auto mess = MasaMessage();
            mess.cam_idx = (*m).cam_idx;
            mess.lights = (*m).lights;
            mess.num_objects = (*m).num_objects;
            mess.objects = (*m).objects;
            mess.t_stamp_ms = (*m).t_stamp_ms;*/

        	// First use a message pointer.
            try {
		std::stringbuf *message = new std::stringbuf();
		comm->serialize_coords(m,message);
                mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC, message->str().data(), message->str().length());
                pubmsg->set_qos(QOS);
                client.publish(pubmsg)->wait_for(TIMEOUT);
		std::cout << (*m).cam_idx << " " << (*m).t_stamp_ms << " " << (*m).objects.size() << std::endl;
		}
            catch(const mqtt::exception& exc) {
                cerr << exc.what() << endl;
                return (void *)1;
            }


//            bsc_forwarder.send_message(m, this->bsc_reciving_port);
        }
    }
    delete m;
    return (void *)NULL;
}

}
