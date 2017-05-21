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
	
	UDPConnection con(CONNECTION_PORT, "127.0.0.1", 0);

	while(1){
		con.recvData(frame_buffer, frame_buffer_size);		
	}

	return 0;
}
