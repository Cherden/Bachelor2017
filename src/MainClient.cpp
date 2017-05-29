#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

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

	//size_t frame_buffer_size = KinectWrapper::getBufferSizeForBothFrames();
	//size_t frame_buffer_size = 2*424688;
	//char frame_buffer[frame_buffer_size] = {0};
	IplImage* video_image = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_8U, 3);
	IplImage* depth_image = cvCreateImageHeader(cvSize(640, 480), IPL_DEPTH_16U, 1);

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
	clock_t diff_time = 0;
	clock_t diff_time_total = 0;

	while(running){
		LOG_DEBUG << "trying to receive frame" << endl;

		start_time = clock();
		if ((ret = kinect.getData(VIDEO, video_image)) != 0){
			LOG_WARNING << "error on receiving frame from kinect" << endl;
			continue;
		}
		timestamp = clock();
		diff_time = timestamp - start_time;

		con.sendData(frame_buffer, frame_buffer_size);

		diff_time_total = clock() - start_time;

		printf("took %lf milliseconds\n", (double) (diff_time_total) / 1000);

		LOG_DEBUG << "received frame, took "
			<< (double) (diff_time) / 1000 << "ms "
			<< ", " << (double) (diff_time_total) / 1000
			<< "ms total" << endl;

		clock_t wait_time = clock() + 33333 - diff_time_total;
		while (wait_time - clock()  > 0){}
	}

	con.closeConnection();

	return 0;
}
