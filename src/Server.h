#ifndef _SERVER_H_
#define _SERVER_H_

#include <thread>

#include "Common.h"

#include "TCPConnection.h"
#include "NetworkCommunication.h"
#include "../gen/KinectFrameMessage.pb.h"

using namespace std;

class Server{
public:
	Server(NetworkCommunication* nc);

	int connect(int is_leader);
	void sendFrameMessage(KinectFrameMessage& kfm);
	int isClosed(){ return _tcp_con.isClosed(); };

	~Server();


private:
	TCPConnection _tcp_con;
	volatile bool _running;
	thread* _server_thread;
	bool _is_leader;
	NetworkCommunication* _nc;

	void _threadHandle();
};

#endif
