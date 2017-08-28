#include <iostream>
#include <ratio>
#include <chrono>
#include <ctime>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "Common.h"

#include "UDPConnection.h"
#include "TCPConnection.h"

#include "../gen/KinectFrameMessage.pb.h"
#include "../gen/ConnectionMessage.pb.h"
#include "KinectWrapper.h"
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

int main(){
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
	string serialized_message;

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
	TCPConnection tcp_con;
	if (tcp_con.createConnection(CLIENT, CONNECTION_PORT, "192.168.1.2") < 0){
		cout << "Can not connect to server!" << endl;
		kinect.setLed(LED_RED);
		return -1;
	}

	ConnectionMessage m;
	uint32_t size_nw = 0;

	tcp_con.recvData((void *) &size_nw, 4);
	uint32_t size = ntohl(size_nw);
	char* buf[size] = {0};
	tcp_con.recvData((void*) buf, size);

	m.ParseFromArray(buf, size);
	UDPConnection udp_con(m.udp_port());
	udp_con.createConnection(CLIENT, -1, "");
	udp_con.setInfo(tcp_con.getInfo());

#ifdef USE_POINT_CLOUD
	m.set_use_point_cloud(true);
#else
	m.set_use_point_cloud(false);
#endif
	m.set_video_height(VIDEO_FRAME_HEIGHT);
	m.set_video_width(VIDEO_FRAME_WIDTH);
	m.set_depth_height(DEPTH_FRAME_HEIGHT);
	m.set_depth_width(DEPTH_FRAME_WIDTH);

	m.SerializeToString(&serialized_message);
	size = m.ByteSize();
	size_nw = htonl(size);
	tcp_con.sendData((void*) &size_nw, 4);
	tcp_con.sendData((void*) serialized_message.c_str(), size);

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

	cout << "Sending data to server.." << endl;
	kinect.setLed(LED_GREEN);
	while(running){
		if (tcp_con.isClosed()){
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

#ifdef USE_POINT_CLOUD
		KinectWrapper::convertToXYZPointCloud(frame_message, (uint16_t*) depth_image);
#endif

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

		frame_message.SerializeToString(&serialized_message);
		frame_message.release_fvideo_data();
		frame_message.release_fdepth_data();

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
		tcp_con.sendData((void*) &size_nw, 4);
		tcp_con.sendData((void*) serialized_message.c_str(), size);

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

#ifdef PRINT_TIME_INFO
		cout << "Total time for processing data "
			<< tget_data + tset_data + tserialize_data + tsend_data << endl;
#endif

		int fps_time = 30000;
		diff_time = high_resolution_clock::now() - total_start_time;
		int duration = (int) diff_time.count();
		if (fps_time - duration > 0){
			usleep(fps_time - duration);
		}
	}

	kinect.setLed(LED_BLINK_GREEN);
	tcp_con.closeConnection();
	udp_con.closeConnection();

	return 0;
}
