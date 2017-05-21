#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "KinectWrapper.h"
#include "UDPConnection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;


int main(void){
	KinectWrapper kinect = KinectWrapper::getInstance();

	size_t frame_buffer_size = KinectWrapper::getBufferSizeForBothFrames();
	char frame_buffer[frame_buffer_size] = {0};
	
	Logger logger = Logger::getInstance();
	logger.setLogLevel(LOG_LEVEL);

	int ret = 0;

	UDPConnection con(CONNECTION_PORT, "127.0.0.1", 0);
	con.createConnection();

	clock_t start_time = 0;
	clock_t timestamp = 0;
	clock_t end_time = 0;
	clock_t diff_time = 0;
	clock_t diff_time_total = 0;

	while(1){
		start_time = clock();

		if ((ret = kinect.getData(frame_buffer)) != 0){
			logger.log(WARNING, "error on receiving frame from kinect");
			break;
		}

		timestamp = clock();
		diff_time = timestamp - start_time;

		con.sendData(frame_buffer, frame_buffer_size);

		diff_time_total = clock() - start_time;
		if (33333 - diff_time_total > 0){
			usleep(33333 - diff_time_total);
		}
	}

	return 0;
}
