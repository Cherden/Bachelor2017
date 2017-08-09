#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <arpa/inet.h>
#include <thread>
#include <mutex>

#include "Common.h"

#ifdef USE_UDP
#include "UDPConnection.h"
#else
#include "TCPConnection.h"
#endif

#include "../gen/KinectFrameMessage.pb.h"

using namespace std;

class Client{
public:
	Client(int socket);

	void setInfo(struct sockaddr_in* info);
	int getData(char** video, char** depth, float** cloud);

#ifdef USE_UDP
	UDPConnection& getConnection(){ return _con;};
#else
	TCPConnection& getConnection(){ return _con;};
#endif
	int isActive(){ return _running; };

	~Client();


private:
	void _threadHandle();
	void _handleFrameMessage(int len);

#ifdef USE_UDP
	UDPConnection _con;
#else
	TCPConnection _con;
#endif

	KinectFrameMessage _sensor_data;
	volatile int _data_available;

	volatile int _running;
	mutex _data_mutex;
	thread _client_thread;
};

#endif
