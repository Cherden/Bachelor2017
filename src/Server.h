#ifndef _SERVER_H_
#define _SERVER_H_

#include <thread>

#include "Common.h"

#include "TCPConnection.h"

using namespace std;

class Server{
public:
	Server();
	
	int connect();
	void sendFrameMessage(KinectFrameMessage& kfm);

	~Server();


private:
	TCPConnection _tcp_con;
};

#endif
