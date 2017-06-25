#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <arpa/inet.h>
#include <thread>
#include <mutex>

using namespace std;

typedef struct{
	Mat* frame;
	char* data;
} ClientData;

class Client{
public:
	Client(int socket);

	void setInfo(struct sockaddr_in* info);
	
	Mat getVideoMatrix();
	Mat getDepthMatrix();
	
	void isActive(){ return _running; };
	
	~Client();
	

private:
	void _threadHandle();
	int _handleFrameMessage(int len);
	void _clearData();
	
	Connection _con;

	ClientData _video;
	ClientData _depth;
	
	volatile int _running;
	mutex _data_mutex;
	thread _client_thread;
};

#endif
