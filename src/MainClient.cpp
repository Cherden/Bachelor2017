#include <iostream>
#include <ratio>
#include <chrono>
#include <ctime>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "../gen/KinectFrameMessage.pb.h"
#include "KinectWrapper.h"
#include "Connection.h"
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
	Connection con;
	con.createConnection(CLIENT, CONNECTION_PORT, "192.168.1.2");

	high_resolution_clock::time_point start_time;
	high_resolution_clock::time_point end_time;
	time_t timestamp;

	duration<double, std::milli> diff_time;


	frame_message.set_fvideo_size(VIDEO_FRAME_MAX_SIZE);
	frame_message.set_fvideo_height(VIDEO_FRAME_HEIGHT);
	frame_message.set_fvideo_width(VIDEO_FRAME_WIDTH);
	frame_message.set_fvideo_depth(VIDEO_FRAME_DEPTH);

	frame_message.set_fdepth_size(DEPTH_FRAME_MAX_SIZE);
	frame_message.set_fdepth_height(DEPTH_FRAME_HEIGHT);
	frame_message.set_fdepth_width(DEPTH_FRAME_WIDTH);
	frame_message.set_fdepth_depth(DEPTH_FRAME_DEPTH);

	high_resolution_clock::time_point for_fps = high_resolution_clock::now();
	int frames = 0;
	while(running){
		if (con.isClosed()){
			break;
		}

		LOG_DEBUG << "trying to get frame from kinect" << endl;

		start_time = high_resolution_clock::now();
		if ((ret = kinect.getData(VIDEO, &video_image)) != 0){
			LOG_WARNING << "could not receive video frame from kinect" << endl;
			continue;
		}
		if ((ret = kinect.getData(DEPTH, &depth_image)) != 0){
			LOG_WARNING << "could not receive depth frame from kinect" << endl;
			continue;
		}
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;

		timestamp = system_clock::to_time_t(end_time);

		LOG_DEBUG << "time to capture sensor data: " << diff_time.count()
			<< " ms" << endl;

		start_time = high_resolution_clock::now();
		frame_message.set_fvideo_data((void*) video_image, VIDEO_FRAME_MAX_SIZE);
		frame_message.set_fdepth_data((void*) depth_image, DEPTH_FRAME_MAX_SIZE);
		frame_message.set_timestamp(timestamp);
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;

		LOG_DEBUG << "time to put data in protobuf message: "
			<< diff_time.count() << " ms" << endl;

		start_time = high_resolution_clock::now();
		uint32_t size = frame_message.ByteSize();
		LOG_DEBUG << "serialized data size is " << size << endl;

		send_data = malloc(size);
		frame_message.SerializeToArray(send_data, size);
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;


		LOG_DEBUG << "time to serialize protobuf message: "
			<< diff_time.count() << " ms" << endl;

		start_time = high_resolution_clock::now();
		uint32_t size_nw = htonl(size);
		con.sendData((void*) &size_nw, 4);
		con.sendData(send_data, size);

		free(send_data);
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;

		LOG_DEBUG << "time to send protobuf message: "
			<< diff_time.count() << " ms" << endl;


		frames++;
		diff_time = high_resolution_clock::now() - for_fps;
		if (diff_time.count() >= 1000){
			cout << frames << " FPS" << endl;
			for_fps = high_resolution_clock::now();
			frames = 0;
		}

		//clock_t wait_time = clock() + 33333 - diff_time_total;
		//while (wait_time - clock()  > 0){}
		//usleep(wait_time);
	}

	con.closeConnection();

	return 0;
}
