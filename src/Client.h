#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <arpa/inet.h>
#include <thread>
#include <mutex>

#include "Common.h"

#include "UDPConnection.h"
#include "TCPConnection.h"

#include "../gen/KinectFrameMessage.pb.h"

using namespace std;

class Client{
public:
	Client(int tcp_socket, int udp_port);

	void setInfo(struct sockaddr_in* info);
	int getData(char** video, char** depth, float** cloud);

	int isActive(){ return _running; };

	~Client();


private:
	void _threadHandle();
	void _handleFrameMessage(int len);
	void _sendConnectionMessage();

	TCPConnection _tcp_con;
	UDPConnection _udp_con;

	KinectFrameMessage _sensor_data;
	volatile int _data_available;

	volatile int _running;
	mutex _data_mutex;
	thread _client_thread;

	bool _use_point_cloud;
	int _video_height;
	int _video_width;
	int _depth_height;
	int _depth_width;
};

#endif
