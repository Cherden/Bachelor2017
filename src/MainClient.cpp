#include <iostream>
#include <ratio>
#include <chrono>
#include <ctime>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "Common.h"

#include "../gen/KinectFrameMessage.pb.h"
#include "KinectWrapper.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG

using namespace std;
using namespace chrono;

volatile bool running = true;
void signalHandler(int signal){
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT){
		running = false;
	}
}

int main(){
	if (getuid()){
		cout << "You have to have root permission! Try sudo." << endl;
		kinect.setLed(LED_RED);
		return -1;
	}
	
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);
	
	SET_LOG_LEVEL(LOG_LEVEL);

	KinectWrapper kinect = KinectWrapper::getInstance();
	kinect.setLed(LED_YELLOW);

	char* video_image;
	char* depth_image;

	string video_string;
	video_string.resize(VIDEO_FRAME_MAX_SIZE);

	string depth_string;
	depth_string.resize(DEPTH_FRAME_MAX_SIZE);

  	KinectFrameMessage frame_message;


	/*
		the first one always takes ~10 times the normal time (usb handshake,
		resync etc.), so just do it once before the "real" program starts
	*/
	LOG_DEBUG << "handle usb handshake..." << endl;
	cout << "Initialize Kincet.." << endl;
	kinect.handleUSBHandshake();

	Server server();
	
	if ((id = server.connect()) == -1){
		kinect.setLed(LED_RED);
		return -1;
	}
	
	Sync sync(id);

	time_t timestamp = 0;

	cout << "Sending data to server.." << endl;
	kinect.setLed(LED_GREEN);
	while(running){
		total_start_time = high_resolution_clock::now();

		LOG_DEBUG << "trying to get frame from kinect" << endl;

		if ((ret = kinect.getData(VIDEO, &video_image)) != 0){
			LOG_WARNING << "could not receive video frame from kinect" << endl;
			continue;
		}
		if ((ret = kinect.getData(DEPTH, &depth_image)) != 0){
			LOG_WARNING << "could not receive depth frame from kinect" << endl;
			continue;
		}

#ifdef USE_POINT_CLOUD
		KinectWrapper::convertToXYZPointCloud(frame_message, (uint16_t*) depth_image);
#endif


		timestamp = system_clock::to_time_t(end_time);

		memcpy(&video_string[0], video_image, VIDEO_FRAME_MAX_SIZE);
		memcpy(&depth_string[0], depth_image, DEPTH_FRAME_MAX_SIZE);

		frame_message.set_allocated_fvideo_data(&video_string);
		frame_message.set_allocated_fdepth_data(&depth_string);

		frame_message.set_timestamp(timestamp);

		server.sendFrameMessage(frame_message);
	}

	kinect.setLed(LED_BLINK_GREEN);

	return 0;
}
