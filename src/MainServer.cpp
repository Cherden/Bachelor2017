#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../gen/KinectFrameMessage.h"
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

Mat* video_frame;
char* video_data;
Mat* depth_frame;
char* depth_data;

int handleFrameMessage(Connection con, int len){
	KinectFrameMessage frame;
	string input_string(len, 0);

	con.recvData((void *) &input_string[0], len);
	frame.ParseFromString(input_string);

	if (!(frame.has_video_data() || frame.has_depth_data() || frame.has_timestamp())){
		LOG_ERROR << "message does not conatin at least one required field" << endl;
		return -1;
	}
	
	/* Create opencv matrix for video frame */
	video_data = (char*) malloc(len);
	memcpy(video_data, &frame.get_video_data(), len);

	video_frame = new Mat(Size(640, 480), CV_8UC3, video_data);
	cvtColor(*video_frame, *video_frame, CV_RGB2BGR);

	
	/* Create opencv matrix for depth frame */
	depth_data = (char*) malloc(len);
	memcpy(depth_data, &frame.get_depth_data(), len);

	depth_frame = new Mat(Size(640, 480), CV_16UC1, depth_data);
	
	return frame.get_timestamp();
}

int main(void){
	int client_socket = 0;
	int timestamp = 0;
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

	SerializationHeader sh = {};
	
	while(running){
		client.recvData((void*) &sh, sizeof(sh));

		if (sh.header != SERIALIZATION_HEADER){
			LOG_WARNING << "unknown header in SerializationHeader, sh.header=" << sh.header << endl;
		}

		timestamp = handleFrameMessage(client, sh.size, frame);

		if (timestamp < 0) {
			LOG_DEBUG << "not showing frame" << endl;
		} else {
			LOG_DEBUG << "try to show frame" << endl;
			imshow("Frame at " << timestamp, *frame);

			free(video_data);
			free(depth_data);
			delete video_frame;
			delete depth_frame;
		}
	}

	con.closeConnection();
	client.closeConnection();

	return 0;
}
