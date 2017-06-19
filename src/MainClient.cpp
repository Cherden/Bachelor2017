#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <google/protobuf/arena.h>
#include "../gen/KinectFrameMessage.pb.h"
#include "KinectWrapper.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;
using namespace google::protobuf;

volatile bool running = true;
void signalHandler(int signal){
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT){
		running = false;
	}
}

int main(void){
	if (getuid()){
		LOG_ERROR << "you have to have root permission" << endl;
		return -1;
	}

	KinectWrapper kinect = KinectWrapper::getInstance();

	char* video_image;
	char* depth_image;

  	KinectFrameMessage frame_message;

	void* send_data = 0;

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
	kinect.getData(VIDEO, &video_image);
	kinect.getData(DEPTH, &depth_image);

	LOG_DEBUG << "try to create connection..." << endl;
	Connection con(CONNECTION_PORT, "192.168.1.2");
	con.createConnection(CLIENT);

	clock_t start_time = 0;
	clock_t timestamp = 0;
	clock_t diff_time = 0;
	clock_t diff_time_total = 0;

	frame_message.fvideo_size(VIDEO_FRAME_MAX_SIZE);
	frame_message.fvideo_height(VIDEO_FRAME_HEIGHT);
	frame_message.fvideo_width(VIDEO_FRAME_WIDTH);
	frame_message.fvideo_depth(VIDEO_FRAME_DEPTH);

	frame_message.fdepth_size(DEPTH_FRAME_MAX_SIZE);
	frame_message.fdepth_height(DEPTH_FRAME_HEIGHT);
	frame_message.fdepth_width(DEPTH_FRAME_WIDTH);
	frame_message.fdepth_depth(DEPTH_FRAME_DEPTH);

	while(running){
		if (con.isClosed()){
			break;
		}

		LOG_DEBUG << "trying to get frame from kinect" << endl;

		start_time = clock();
		if ((ret = kinect.getData(VIDEO, &video_image)) != 0){
			LOG_WARNING << "could not receive video frame from kinect" << endl;
			continue;
		}
		if ((ret = kinect.getData(DEPTH, &depth_image)) != 0){
			LOG_WARNING << "could not receive depth frame from kinect" << endl;
			continue;
		}
		timestamp = clock();
		diff_time = timestamp - start_time;

		frame_message.set_fvideo_data((void*) video_image, VIDEO_FRAME_MAX_SIZE);
		frame_message.set_fdepth_data((void*) depth_image, DEPTH_FRAME_MAX_SIZE);
		frame_message.set_timestamp(timestamp);

		uint32_t size = frame_message.ByteSize();
		LOG_DEBUG << "serialized data size is " << size << endl;

		send_data = malloc(size);
		frame_message.SerializeToArray(send_data, size);

		uint32_t size_nw = htonl(size);
		con.sendData((void*) &size_nw, 4);
		con.sendData(send_data, size);

		free(send_data);


		diff_time_total = clock() - start_time;

		LOG_DEBUG << "received frame, took "
			<< (double) (diff_time) / 1000 << "ms, "
			<< (double) (diff_time_total) / 1000
			<< "ms total" << endl;

		clock_t wait_time = clock() + 33333 - diff_time_total;
		//while (wait_time - clock()  > 0){}
		//usleep(wait_time);
	}

	con.closeConnection();

	return 0;
}
