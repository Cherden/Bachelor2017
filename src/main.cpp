#include <iostream>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "KinectWrapper.h"
#include "Transfer.h"

#define LOOPS 1800

using namespace std;

int main(void){
	KinectWrapper kw = KinectWrapper::getInstance();

	size_t frame_buffer_size = kw.getVideoFrameSize + kw.getDepthFrameSize;
	char frame_buffer[frame_buffer_size] = {0};

	int ret = 0;

	Transfer con(1234, "127.0.0.1");
	con.createConnection();


	clock_t start_time = 0;
	clock_t timestamp = 0;
	clock_t end_time = 0;
	clock_t diff_time = 0;
	clock_t diff_time_total = 0;

	while(1){
		start_time = clock();

		if ((ret = kw.getData(frame_buffer)) != 0){
			cout << "ERROR: getData returned " << ret << endl;
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
