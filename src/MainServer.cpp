#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "KinectWrapper.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;

volatile bool running = true;
void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		running = false;
	}
}

IplImage* handleVideoFrame(Connection con, int len){
	VideoFrame vf = {};
	char* data = 0;

	IplImage* ret = cvCreateImageHeader(cvSize(640, 480), 8, 3);

	con.recvData((void *) &vf, sizeof(VideoFrame));
	LOG_DEBUG << "VideoFrame h=" << vf.h << " sizeof=" << sizeof(VideoFrame) << endl;


	if (vf.h != VIDEO_FRAME){
		LOG_WARNING << "expected to receive video frame, got " << vf.h << endl;
		return NULL;
	}

	data = (char*) malloc(len);
	memcpy(data, vf.data, len);


	cvSetData(ret, data, 640*3);
	cvCvtColor(ret, ret, CV_RGB2BGR);

	return ret;
}

IplImage* handleDepthFrame(Connection con, int len){
	DepthFrame df = {};
	char* data = 0;

	IplImage* ret = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_16U, 1);

	con.recvData((void *) &df, sizeof(DepthFrame));

	if (df.h != DEPTH_FRAME){
		LOG_WARNING << "expected to receive depth frame, got " << df.h << endl;
		return NULL;
	}

	data = (char*) malloc(len);
	memcpy(data, df.data, len);


	cvSetData(ret, data, 640);

	return ret;
}

int main(void){
	int client_socket = 0;
	Header h = UNKNOWN;
	IplImage* frame = 0;
	//Connection clients[MAX_CLIENTS] = {0};

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	Connection con(CONNECTION_PORT, "127.0.0.1");
	con.createConnection(SERVER);

	con.acceptConnection(&client_socket);
	Connection client(client_socket);

	/*
	for (int i = 0; i < MAX_CLIENTS; i++){
		con.acceptConnection(&client_socket);
		client[i] = Connection(client_socket);
	}
	*/

	//while(running){
	for (int i = 0; i<10; i++){
		//if ((h = client.peekHeader()) == FRAME_MESSAGE){
			FrameMessage fm = {};

			client.recvData((void*) &fm, sizeof(FrameMessage));

			LOG_DEBUG << "FrameMessage h=" << fm.h << " info="<<fm.info <<" length="<<fm.length << " sizeof=" << sizeof(FrameMessage) << endl;

			if(fm.info == VIDEO){
				frame = handleVideoFrame(client, fm.length);
			} else if(fm.info == DEPTH) {
				frame = handleDepthFrame(client, fm.length);
			} else {
				LOG_WARNING << "received unknown frame_info " << fm.info << endl;
				continue;
			}
		/*} else {
			LOG_WARNING << "received wrong header " << h << endl;
			continue;
		}*/

		LOG_DEBUG << "try to show frame" << endl;
		cvShowImage("Frame", frame);
		cvFree(&frame);
	}

	con.closeConnection();
	client.closeConnection();

	return 0;
}
