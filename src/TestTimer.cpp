#include <iostream>
#include <time.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

#include "Common.h"
#include "Connection.h"
#include "UDPConnection.h"
#include "TimingClient.h"
#include "../gen/TimingMessage.pb.h"

volatile bool running = true;
volatile int us = 0;
volatile int ms = 0;
volatile int sec = 0;

void signalHandler(int signal){
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT){
		running = false;
	}
}

void getTime(uint64_t* t){
	struct timeval tv;

	gettimeofday(&tv, NULL);
	cout << "tv.tv_sec " << tv.tv_sec << endl;
	cout << "tv.tv_usec "  << tv.tv_usec << endl;

	t[0] = (uint64_t) tv.tv_sec;
	t[1] = (uint64_t) tv.tv_usec;
}

int main(){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	UDPConnection udp_con(CONNECTION_PORT);

	struct sockaddr_in me;

	memset((char *) &me, 0, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_port = htons(CONNECTION_PORT);

#define SENDER
#ifndef SENDER
	inet_aton("192.168.1.2", &me.sin_addr);
#else
	inet_aton("192.168.1.234", &me.sin_addr);
#endif

	udp_con.setInfo(&me);
	udp_con.createConnection(CLIENT, -1, "");
#ifndef SENDER
	TimingClient tc(&udp_con);
#else
	TimingMessage tm;
	tm.set_sec(1503930178);
	tm.set_usec(143286);
	string serialized_message;
	bool k = false;
#endif

	while(running){
#ifdef SENDER
		uint64_t t[2] = {0};
		getTime(t);
		if (!k){
			tm.SerializeToString(&serialized_message);
			k = true;
		}

		cout << tm.ByteSize() << endl;
		udp_con.sendData((void*) serialized_message.c_str(), tm.ByteSize());
		udp_con.recvData((void*) serialized_message.c_str(), 10);
		tm.ParseFromString(serialized_message);
		cout << "my t = {" << t[0] << ", " << t[1] << "} his time = {" << tm.sec() << ", " << tm.usec() << "}" << endl;
		tm.set_sec(t[0] - tm.sec());
		tm.set_usec(t[1] - tm.usec());
		tm.SerializeToString(&serialized_message);
		cout << tm.ByteSize() << endl;
		udp_con.sendData((void*) serialized_message.c_str(), tm.ByteSize());
		usleep(1000000);
#endif
	}

	cout << "ende" << endl;

	return 0;
}
