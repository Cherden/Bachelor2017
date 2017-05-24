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
	size_t frame_buffer_size = KinectWrapper::getBufferSizeForBothFrames();
	char frame_buffer[frame_buffer_size] = {0};
	int client_socket = 0;
	clock_t start_time = 0;

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	Connection con(CONNECTION_PORT, "127.0.0.1");
	con.createConnection(SERVER);

	con.acceptConnection(&client_socket);

	Connection client(client_socket);

	while(running){
		LOG_DEBUG << "wait for data" << endl;
		start_time = clock();
		client.recvData(frame_buffer, frame_buffer_size);
		LOG_DEBUG << "received data " << clock() - start_time << frame_buffer << endl;
	}

	con.closeConnection();
	client.closeConnection();

	return 0;
}
