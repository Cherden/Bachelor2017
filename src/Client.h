#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <arpa/inet.h>
#include <thread>
#include <mutex>

#include "TCPConnection.h"
#include "../gen/KinectFrameMessage.pb.h"

using namespace std;

class Client{
public:
	Client(int socket);

	void setInfo(struct sockaddr_in* info);
	int getData(char** video, char** depth, float** cloud);

	int isActive(){ return _running; };

	~Client();


private:
	void _threadHandle();
	void _handleFrameMessage(int len);

	TCPConnection _con;

	KinectFrameMessage _sensor_data;
	volatile int _data_available;

	volatile int _running;
	mutex _data_mutex;
	thread _client_thread;
};

#endif
