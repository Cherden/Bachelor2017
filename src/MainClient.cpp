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
#include "Server.h"
#include "NetworkCommunication.h"
#include "Logger.h"
#include "PCLUtil.h"
#include "Timer.h"

#define LOG_LEVEL ERROR

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

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	SET_LOG_LEVEL(LOG_LEVEL);

	kinect.setLed(LED_YELLOW);

	char* video_image;
	char* depth_image;

	string video_string;
	video_string.resize(VIDEO_FRAME_MAX_SIZE);

	string depth_string;
	depth_string.resize(DEPTH_FRAME_MAX_SIZE);

  	KinectFrameMessage frame_message;
	int id = 0;
	int is_leader = 0;


	/*
		the first one always takes ~10 times the normal time (usb handshake,
		resync etc.), so just do it once before the "real" program starts
	*/
	LOG_DEBUG << "handle usb handshake..." << endl;
	cout << "Initialize Kincet.." << endl;
	kinect.handleUSBHandshake();

	NetworkCommunication nc;
	is_leader = nc.connect();

	Server server(&nc);

	if ((id = server.connect(is_leader)) == -1){
		kinect.setLed(LED_RED);
		return -1;
	}

	double timestamp;

	cout << "Sending data to server.." << endl;
	kinect.setLed(LED_GREEN);


	usleep(3000000);

	if (is_leader){
		nc.synchronize();
	}

		usleep(3000000);

	double start_time = Common::getTime(NULL);
	while(running){
		if (server.isClosed()){
			break;
		}

		while(!nc.isTriggered()){
			if (server.isClosed() || !running){
				break;
			}
		};

		double measure_time = Common::getTime(NULL);
		if (kinect.getData(VIDEO, &video_image) != 0){
			LOG_WARNING << "could not receive video frame from kinect" << endl;
			continue;
		}
		if (kinect.getData(DEPTH, &depth_image) != 0){
			LOG_WARNING << "could not receive depth frame from kinect" << endl;
			continue;
		}

		timestamp = Common::getTime(NULL);
		LOG_DEBUG << "TIMESTAMP: " << timestamp << endl;
		frame_message.set_timestamp(timestamp);

		memcpy(&video_string[0], video_image, VIDEO_FRAME_MAX_SIZE);
		frame_message.set_allocated_fvideo_data(&video_string);


#if defined(USE_POINT_CLOUD) && defined(PROCESS_CLOUD_DISTRIBUTED)
		PCLUtil::convertToXYZPointCloud(frame_message, (uint16_t*) depth_image
			, DEPTH_FRAME_HEIGHT, DEPTH_FRAME_WIDTH);
#else
		memcpy(&depth_string[0], depth_image, DEPTH_FRAME_MAX_SIZE);
		frame_message.set_allocated_fdepth_data(&depth_string);
#endif
		{
			Timer t(&f);

			server.sendFrameMessage(frame_message);
		}


		double end_time = Common::getTime(NULL);
		if (end_time - start_time >= 180000){
			break;
		}
	}

	f.close();

	kinect.setLed(LED_BLINK_GREEN);

	return 0;
}
