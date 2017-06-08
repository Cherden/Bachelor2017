#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "KinectWrapper.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;
using namespace cv;

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

char* handleVideoFrame(Connection con, int len, Mat* ret){
	VideoFrame vf = {};
	char* data = 0;

	con.recvData((void *) &vf, sizeof(VideoFrame));

	if (vf.h != VIDEO_FRAME){
		LOG_WARNING << "expected to receive video frame, got " << vf.h << endl;
		return NULL;
	}

	data = (char*) malloc(len);
	memcpy(data, vf.data, len);

	ret = new Mat(Size(640, 480), CV_8UC3, data);
	cvtColor(*ret, *ret, CV_RGB2BGR);

	return data;
}

char* handleDepthFrame(Connection con, int len, Mat* ret){
	DepthFrame df = {};
	char* data = 0;

	con.recvData((void *) &df, sizeof(DepthFrame));

	if (df.h != DEPTH_FRAME){
		LOG_WARNING << "expected to receive depth frame, got " << df.h << endl;
		return NULL;
	}

	data = (char*) malloc(len);
	memcpy(data, df.data, len);

	ret = new Mat(Size(640, 480), CV_16UC1, data);

	return data;
}

int main(void){
	int client_socket = 0;
	Header h = UNKNOWN;
	Mat* frame = 0;
	char* data = 0;
	string recv_string;
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

	while(running){
		client.recvData((void*) &recv_string, sizeof(FrameMessage));


		if(fm.info == VIDEO){
			data = handleVideoFrame(client, fm.length, frame);
		} else if(fm.info == DEPTH) {
			data = handleDepthFrame(client, fm.length, frame);
		} else {
			LOG_WARNING << "received unknown frame_info " << fm.info << endl;
			continue;
		}

		LOG_DEBUG << "try to show frame" << endl;
		imshow("Frame", *frame);

		free(data);
		delete(frame);
	}

	con.closeConnection();
	client.closeConnection();

	return 0;
}
