#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../gen/KinectFrameMessage.h"
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

int main(void){
	if (getuid()){
		LOG_ERROR << "you have to have root permission" << endl;
		return -1;
	}

	KinectWrapper kinect = KinectWrapper::getInstance();

	char video_image[VIDEO_FRAME_MAX_SIZE] = {0};
	char depth_image[DEPTH_FRAME_MAX_SIZE] = {0};

	KinectFrameMessage frame_message;
	string send_string;

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	SET_LOG_LEVEL(LOG_LEVEL);

	int ret = 0;

	/*
		the first one always takes ~10 times the normal time (usb handshake,
		resync etc.), so just do it once before the "real" program starts
	*/
	LOG_DEBUG << "handle usb handshake..." << endl;
	kinect.getData(VIDEO, video_image);
	kinect.getData(DEPTH, depth_image);

	LOG_DEBUG << "try to create connection..." << endl;
	Connection con(CONNECTION_PORT, "192.168.1.2");
	con.createConnection(CLIENT);

	clock_t start_time = 0;
	clock_t timestamp = 0;
	clock_t diff_time = 0;
	clock_t diff_time_total = 0;
	
	SerializationHeader sh = {
		.header = SERIALIZATION_HEADER,
		.size = 0
	};

	while(running){
		LOG_DEBUG << "trying to get frame from kinect" << endl;

		start_time = clock();
		if ((ret = kinect.getData(VIDEO, vf.data)) != 0){
			LOG_WARNING << "error on receiving video frame from kinect" << endl;
			continue;
		}
		if ((ret = kinect.getData(DEPTH, depth_image)) != 0){
			LOG_WARNING << "error on receiving depth frame from kinect" << endl;
			continue;
		}
		timestamp = clock();
		diff_time = timestamp - start_time;

		frame_message.set_video_data(string(video_image));
		frame_message.set_depth_data(string(depth_image));
		frame_message.set_timestamp(timestamp);
		frame_message.SerializeToString(&send_string);
		
		sh.size = send_string.size();

		con.sendData((void*) &sh, sizeof(sh));
		con.sendData((void*) &send_string, send_string.size());

		diff_time_total = clock() - start_time;

		printf("took %lf milliseconds\n", (double) (diff_time_total) / 1000);

		LOG_DEBUG << "received frame, took "
			<< (double) (diff_time) / 1000 << "ms, "
			<< (double) (diff_time_total) / 1000
			<< "ms total" << endl;

		clock_t wait_time = clock() + 33333 - diff_time_total;
		//while (wait_time - clock()  > 0){}
		usleep(wait_time);
	}

	con.closeConnection();

	return 0;
}
