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
	size_t frame_buffer_size = KinectWrapper::getBufferSizeForBothFrames();
	char frame_buffer[frame_buffer_size] = {0};

	UDPConnection con(CONNECTION_PORT, "127.0.0.1");
	con.createConnection();

	while(1){
		cout << "wait for data" << endl;
		con.recvData(frame_buffer, frame_buffer_size);
		cout << "received data" << endl;
	}

	return 0;
}
