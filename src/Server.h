#ifndef _SERVER_H_
#define _SERVER_H_

#include <thread>
#include <condition_variable>
#include <mutex>

#include "Common.h"

#include "TCPConnection.h"
#include "Sync.h"
#include "../gen/KinectFrameMessage.pb.h"

using namespace std;

class Server{
public:
	Server(Sync* sync, condition_variable* send_cond);

	int connect(int is_leader);
	void sendFrameMessage(KinectFrameMessage& kfm);
	int isClosed(){ return _tcp_con.isClosed(); };

	bool canSend(){ return _can_send; };
	void haveSent(){ _can_send = false; };

	~Server();


private:
	TCPConnection _tcp_con;
	volatile bool _running;
	thread _server_thread;
	Sync* _sync;

	bool _can_send;
	condition_variable* _send_cond;

	void _threadHandle();
};

#endif
