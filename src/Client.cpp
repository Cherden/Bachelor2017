#include "Client.h"

Client::Client(int socket)
	: _con(socket)
	, _video_frame(0)
	, _depth_frame(0)
	, _video_data(0)
	, _depth_data(0)
	, _running(0)
	, _info(0){ }

Client::~Client(){
	_running = 0;
}

void Client::setInfo(struct sockaddr_in* info){
	memcpy(&_info, info, sizeof(struct sockaddr_in));
}

void Client::run(){
	_running = 1;
	thread client_thread(&Client::handle, this);
	client_thread.detach();
}

void Client::handle(){
	clients[place] = new Connection(socket);

	uint64_t size = 0;
	int timestamp = 0;

	while (_running){
		if (con.isClosed()){
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
