#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <thread>
#include <arpa/inet.h>

using namespace std;

class Client{
public:
	Client(int socket);

	void setInfo(struct sockaddr_in* info);

	void handle();

	void run();

	~Client();

private:
	Connection _con;

	Mat* _video_frame;
	Mat* _depth_frame;

	char* _video_data;
	char* _depth_data;

	volatile int _running;
	struct sockaddr_in _info;


};

#endif
