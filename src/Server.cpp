#include "Server.h"

#include <iostream>

#include "Logger.h"
#include "KinectWrapper.h"
#include "../gen/ConnectionMessage.pb.h"

Server::Server(int id)
	: _tcp_con() {}

Server::~Server(){
	_tcp_con.closeConnection();
}

int Server::connect(){
	ConnectionMessage cm;
	KinectFrameMessage kfm;
	string serialized_message;
	uint32_t size_nw = 0;
	uint32_t size = 0;
	
	int id = 0;
	
	char video_buf[VIDEO_FRAME_MAX_SIZE] = {1};
	char depth_buf[DEPTH_FRAME_MAX_SIZE] = {1};
	time_t timestamp = 1;
	
	kfm.set_fvideo_data((void*) video_buf, VIDEO_FRAME_MAX_SIZE);
	kfm.set_fdepth_data((void*) depth_buf, DEPTH_FRAME_MAX_SIZE);
	kfm.set_timestamp(timestamp);
	
	LOG_DEBUG << "try to create connection..." << endl;
	cout << "Connect to server.." << endl;
	if (_tcp_con.createConnection(CLIENT, CONNECTION_PORT, SERVER_IP) < 0){
		cout << "Can not connect to server!" << endl;
		return -1;
	}

	// receive size of connection message
	_tcp_con.recvData((void *) &size_nw, 4);
	size = ntohl(size_nw);
	
	char* buf[size] = {0};
	_tcp_con.recvData((void*) buf, size);

	cm.ParseFromArray(buf, size);

	id = cm.id();
	
	cm.set_message_size(kfm.ByteSize());

#ifdef USE_POINT_CLOUD
	cm.set_use_point_cloud(true);
#else
	cm.set_use_point_cloud(false);
#endif

	cm.set_video_height(VIDEO_FRAME_HEIGHT);
	cm.set_video_width(VIDEO_FRAME_WIDTH);
	cm.set_depth_height(DEPTH_FRAME_HEIGHT);
	cm.set_depth_width(DEPTH_FRAME_WIDTH);

	cm.SerializeToString(&serialized_message);
	size = cm.ByteSize();
	size_nw = htonl(size);
	_tcp_con.sendData((void*) &size_nw, 4);
	_tcp_con.sendData((void*) serialized_message.c_str(), size);
	
	return id;
}
