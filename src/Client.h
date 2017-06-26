#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Connection.h"

using namespace std;
using namespace cv;

typedef struct{
	Mat* frame;
	char* data;
} ClientData;

class Client{
public:
	Client(int socket);

	void setInfo(struct sockaddr_in* info);

	int lockData();
	void releaseData();
	int getData(Mat* video, Mat* depth);

	int isActive(){ return _running; };

	~Client();


private:
	void _threadHandle();
	int _handleFrameMessage(int len);
	void _clearData();

	Connection _con;

	ClientData _video;
	ClientData _depth;
	volatile int _processed;

	volatile int _running;
	mutex _data_mutex;
	thread _client_thread;
};

#endif
