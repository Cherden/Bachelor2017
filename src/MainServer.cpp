#include <iostream>
#include <thread>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../gen/KinectFrameMessage.pb.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;
using namespace cv;

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

Mat* video_frame;
char* video_data;
Mat* depth_frame;
char* depth_data;

Connection* clients[MAX_CLIENTS] = {0};

int handleFrameMessage(Connection& con, int len){
	KinectFrameMessage frame;
	char* buf = (char*) malloc(len);

	con.recvData((void *) buf, len);
	frame.ParseFromArray(buf, len);

	if (!(frame.fvideo_data() == "" || frame.fdepth_data() == ""
	 	|| frame.timestamp() == 0)){
		LOG_ERROR << "message does not contain at least one required field"
			<< endl;
		return -1;
	}

	/* Create opencv matrix for video frame */
	video_data = (char*) malloc(VIDEO_FRAME_MAX_SIZE);
	memcpy(video_data, frame.fvideo_data().c_str(), VIDEO_FRAME_MAX_SIZE);

	video_frame = new Mat(Size(frame.fvideo_width(), frame.fvideo_height())
					, frame.fvideo_depth(), video_data);
	cvtColor(*video_frame, *video_frame, CV_RGB2BGR);


	/* Create opencv matrix for depth frame */
	depth_data = (char*) malloc(DEPTH_FRAME_MAX_SIZE);
	memcpy(depth_data, &frame.fdepth_data(), DEPTH_FRAME_MAX_SIZE);

	depth_frame = new Mat(Size(frame.fdepth_width(), frame.fdepth_height())
					, frame.fdepth_depth(), depth_data);

	free(buf);

	return frame.timestamp();
}

void handleClient(int socket, int place){
	clients[place] = new Connection(socket);

	uint64_t size = 0;
	int timestamp = 0;

	while (1){
		if (clients[place]->isClosed()){
			break;
		}

		clients[place]->recvData((void*) &size, 4);
		size = ntohl(size);

		LOG_DEBUG << "next protobuf message size is " << size << endl;

		timestamp = handleFrameMessage(*clients[place], size);

		if (timestamp < 0) {
			LOG_DEBUG << "not showing frame" << endl;
		} else {
			LOG_DEBUG << "try to show frame" << endl;
			imshow("Frame", *video_frame);
			cvWaitKey(10);

			LOG_DEBUG << "Timestamp = " << timestamp << endl;

			free(video_data);
			delete video_frame;
			free(depth_data);
			delete depth_frame;
		}
	}
}

void acceptClient(Connection& accepting, int* size){
	int new_socket = 0;
	int max = *size;
	*size = 0;

	while(*size < max) {
		new_socket = accepting.acceptConnection(NULL);
		thread handle_client(handleClient, new_socket, *size++);
		handle_client.detach();
	}
}

int main(void){
	//int client_socket = 0;

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	Connection con(CONNECTION_PORT, "127.0.0.1");
	con.createConnection(SERVER);

	/*
	client_socket = con.acceptConnection(NULL);
	Connection client(client_socket);
	*/

	/*
	for (int i = 0; i < MAX_CLIENTS; i++){
		con.acceptConnection(&client_socket);
		client[i] = Connection(client_socket);
	}
	*/

	int amount_clients = MAX_CLIENTS;
	thread accept_clients(acceptClient, ref(con), &amount_clients);
	accept_clients.detach();

	while (running){

	}

	con.closeConnection();

	for (int i = 0; i < amount_clients; i++){
		clients[i]->closeConnection();
		delete clients[i];
	}

	return 0;
}
