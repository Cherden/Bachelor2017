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

#define PRINT_TIME_INFO


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
	KinectWrapper kinect = KinectWrapper::getInstance();

	if (getuid()){
		cout << "You have to have root permission! Try sudo." << endl;
		kinect.setLed(LED_RED);
		return -1;
	}

	kinect.setLed(LED_YELLOW);

	char* video_image;
	char* depth_image;

	string video_string;
	video_string.resize(VIDEO_FRAME_MAX_SIZE);

	string depth_string;
	depth_string.resize(DEPTH_FRAME_MAX_SIZE);

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
	cout << "Initialize Kincet.." << endl;
	kinect.getData(VIDEO, &video_image);
	kinect.getData(DEPTH, &depth_image);

	LOG_DEBUG << "try to create connection..." << endl;
	cout << "Connect to server.." << endl;
	Connection con;
	if (con.createConnection(CLIENT, CONNECTION_PORT, "192.168.1.2") < 0){
		cout << "Can not connect to server!" << endl;
		kinect.setLed(LED_RED);
		return -1;
	}


	high_resolution_clock::time_point total_start_time;
	high_resolution_clock::time_point start_time;
	high_resolution_clock::time_point end_time;
	duration<double, std::milli> diff_time;
	time_t timestamp = 0;

#ifdef PRINT_TIME_INFO
	high_resolution_clock::time_point for_fps = high_resolution_clock::now();
	int frames = 0;
	int saved_frames = 0;

	double tget_data = 0;
	double tget_data_min = 2147483647;
	double tget_data_max = 0;
	double tget_data_avg = 0;


	double tset_data = 0;
	double tset_data_min = 2147483647;
	double tset_data_max = 0;
	double tset_data_avg = 0;

	double tserialize_data = 0;
	double tserialize_data_min = 2147483647;
	double tserialize_data_max = 0;
	double tserialize_data_avg = 0;

	double tsend_data = 0;
	double tsend_data_min = 2147483647;
	double tsend_data_max = 0;
	double tsend_data_avg = 0;
#endif


	frame_message.set_fvideo_size(VIDEO_FRAME_MAX_SIZE);
	frame_message.set_fvideo_height(VIDEO_FRAME_HEIGHT);
	frame_message.set_fvideo_width(VIDEO_FRAME_WIDTH);
	frame_message.set_fvideo_depth(VIDEO_FRAME_DEPTH);

	frame_message.set_fdepth_size(DEPTH_FRAME_MAX_SIZE);
	frame_message.set_fdepth_height(DEPTH_FRAME_HEIGHT);
	frame_message.set_fdepth_width(DEPTH_FRAME_WIDTH);
	frame_message.set_fdepth_depth(DEPTH_FRAME_DEPTH);



	cout << "Sending data to server.." << endl;
	kinect.setLed(LED_GREEN);
	while(running){
		if (con.isClosed()){
			break;
		}

		total_start_time = high_resolution_clock::now();

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

#ifdef PRINT_TIME_INFO
		tget_data = diff_time.count();
		tget_data_min = tget_data < tget_data_min ? tget_data : tget_data_min;
		tget_data_max = tget_data > tget_data_max ? tget_data : tget_data_max;
		tget_data_avg = (tget_data_avg + tget_data) / 2;

		start_time = high_resolution_clock::now();
#endif

		memcpy(&video_string[0], video_image, VIDEO_FRAME_MAX_SIZE);
		memcpy(&depth_string[0], depth_image, DEPTH_FRAME_MAX_SIZE);

		frame_message.set_allocated_fvideo_data(&video_string);
		frame_message.set_allocated_fdepth_data(&depth_string);
		frame_message.set_timestamp(timestamp);

#ifdef PRINT_TIME_INFO
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;

		tset_data = diff_time.count();
		tset_data_min = tset_data < tset_data_min ? tset_data : tset_data_min;
		tset_data_max = tset_data > tset_data_max ? tset_data : tset_data_max;
		tset_data_avg = (tset_data_avg + tset_data) / 2;

		start_time = high_resolution_clock::now();
#endif

		uint32_t size = frame_message.ByteSize();
		LOG_DEBUG << "serialized data size is " << size << endl;

		send_data = malloc(size);
		frame_message.SerializeToArray(send_data, size);

#ifdef PRINT_TIME_INFO
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;

		tserialize_data = diff_time.count();
		tserialize_data_min = tserialize_data < tserialize_data_min
			? tserialize_data : tserialize_data_min;
		tserialize_data_max = tserialize_data > tserialize_data_max
			? tserialize_data : tserialize_data_max;
		tserialize_data_avg = (tserialize_data_avg + tserialize_data) / 2;

		start_time = high_resolution_clock::now();
#endif

		uint32_t size_nw = htonl(size);
		con.sendData((void*) &size_nw, 4);
		con.sendData(send_data, size);

		free(send_data);
		frame_message.release_fvideo_data();
		frame_message.release_fdepth_data();

#ifdef PRINT_TIME_INFO
		end_time = high_resolution_clock::now();
		diff_time = end_time - start_time;

		tsend_data = diff_time.count();
		tsend_data_min = tsend_data < tsend_data_min ? tsend_data : tsend_data_min;
		tsend_data_max = tsend_data > tsend_data_max ? tsend_data : tsend_data_max;
		tsend_data_avg = (tsend_data_avg + tsend_data) / 2;

		frames++;
		diff_time = high_resolution_clock::now() - for_fps;
		if (diff_time.count() >= 1000){
			saved_frames = frames;

			for_fps = high_resolution_clock::now();
			frames = 0;
		}

		cout << "\x1B[2J\x1B[H"		//clear screen
			<< "Time to capture sensor data (ms): \n"
				<< "\tACT = " << tget_data
				<< "\tMIN = " << tget_data_min
				<< "\tMAX = " << tget_data_max
				<< "\tAVG = " << tget_data_avg
			<< "\nTime to put data in protobuf message (ms): \n"
				<< "\tACT = " << tset_data
				<< "\tMIN = " << tset_data_min
				<< "\tMAX = " << tset_data_max
				<< "\tAVG = " << tset_data_avg
			<< "\nTime to serialize protobuf message (ms): \n"
				<< "\tACT = " << tserialize_data
				<< "\tMIN = " << tserialize_data_min
				<< "\tMAX = " << tserialize_data_max
				<< "\tAVG = " << tserialize_data_avg
			<< "\nTime to send protobuf message (ms): \n"
				<< "\tACT = " << tsend_data
				<< "\tMIN = " << tsend_data_min
				<< "\tMAX = " << tsend_data_max
				<< "\tAVG = " << tsend_data_avg
			<< "\nRunning at " << saved_frames << " FPS" << endl;
#endif

		end_time = high_resolution_clock::now();
		diff_time = end_time - total_start_time;

#ifdef PRINT_TIME_INFO
		cout << "Total time for processing data " << diff_time.count() << endl;
#endif

		int fps_time = 33333;
		int duration = (int) diff_time.count() * 1000;
		if (fps_time - duration > 0){
			usleep(fps_time - duration);
		}
	}

	kinect.setLed(LED_BLINK_GREEN);
	con.closeConnection();

	return 0;
}
