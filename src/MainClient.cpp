#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

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
		LOG_ERROR << "you have to be root" << endl;
		return -1;
	}

	KinectWrapper kinect = KinectWrapper::getInstance();

	size_t frame_buffer_size = KinectWrapper::getBufferSizeForBothFrames();
	char frame_buffer[frame_buffer_size] = {0};

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	SET_LOG_LEVEL(LOG_LEVEL);

	int ret = 0;

	/*
		the first one always takes ~10 times the normal time (usb handshake,
		resync etc.), so just do it once before the "real" program starts
	*/
	kinect.getData(BOTH, frame_buffer);

	Connection con(CONNECTION_PORT, "192.168.1.2");
	con.createConnection(CLIENT);

	clock_t start_time = 0;
	clock_t timestamp = 0;
	clock_t end_time = 0;
	clock_t diff_time = 0;
	clock_t diff_time_total = 0;

	while(running){
		start_time = clock();

		LOG_DEBUG << "trying to receive frame" << endl;
		if ((ret = kinect.getData(BOTH, frame_buffer)) != 0){
			LOG_WARNING << "error on receiving frame from kinect" << endl;
			continue;
		}

		timestamp = clock();
		diff_time = timestamp - start_time;

		LOG_DEBUG << "received frame, took " << diff_time << " ms" << endl;

		con.sendData(frame_buffer, frame_buffer_size);

		diff_time_total = clock() - start_time;
		if (33333 - diff_time_total > 0){
			usleep(33333 - diff_time_total);
		}
	}

	con.closeConnection();

	return 0;
}
