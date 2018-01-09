#include "Server.h"

#include <iostream>
#include <unistd.h>
#include <chrono>

#include "Logger.h"
#include "PCLUtil.h"
#include "KinectWrapper.h"
#include "../gen/ConnectionMessage.pb.h"

using namespace chrono;

Server::Server(NetworkCommunication* nc)
	: _tcp_con()
	, _running(false)
	, _server_thread(NULL)
	, _is_leader(false)
	, _nc(nc) {}

Server::~Server(){
	_running = false;
	_tcp_con.closeConnection();
}

int Server::connect(int is_leader){
	ConnectionMessage cm;
	KinectFrameMessage kfm;
	string serialized_message;
	uint32_t size_nw = 0;
	uint32_t size = 0;
	uint64_t timestamp_arr[2] = {0};

	int id = 0;

	char video_buf[VIDEO_FRAME_MAX_SIZE] = {1};
	char depth_buf[DEPTH_FRAME_MAX_SIZE] = {1};

	Common::getTime(timestamp_arr);
	uint64_t timestamp = timestamp_arr[0] * 100 + timestamp_arr[1] / 100000;

	kfm.set_fvideo_data((void*) video_buf, VIDEO_FRAME_MAX_SIZE);

#if defined(USE_POINT_CLOUD) && defined(PROCESS_CLOUD_DISTRIBUTED)
	PCLUtil::convertToXYZPointCloud(kfm, (uint16_t*) depth_buf
		, DEPTH_FRAME_HEIGHT, DEPTH_FRAME_WIDTH);
#else
	kfm.set_fdepth_data((void*) depth_buf, DEPTH_FRAME_MAX_SIZE);
#endif

	kfm.set_timestamp(timestamp);

	LOG_DEBUG << "try to create connection..." << endl;
	LOG_DEBUG << "Connect to server.." << endl;

	if (_tcp_con.createConnection(CLIENT, CONNECTION_PORT, SERVER_IP) < 0){
		LOG_ERROR << "Connection failed!" << endl;
	}

	LOG_DEBUG << "KinectFrameMessage size = " << kfm.ByteSize() << endl;
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

	if (is_leader){
		cm.set_is_leader(true);
		_running = true;
		_server_thread = new thread(&Server::_threadHandle, this);
	}

	cm.SerializeToString(&serialized_message);
	size = cm.ByteSize();
	size_nw = htonl(size);

	LOG_DEBUG << "send connect message" << endl;
	_tcp_con.sendData((void*) &size_nw, 4, "");
	_tcp_con.sendData((void*) serialized_message.c_str(), size, "");
	LOG_DEBUG << "connect message sent" << endl;

	return id;
}

void Server::sendFrameMessage(KinectFrameMessage& kfm){
	string serialized_message;
	uint32_t size = kfm.ByteSize();

	kfm.SerializeToString(&serialized_message);
	kfm.release_fvideo_data();
	kfm.release_fdepth_data();

	_tcp_con.sendData((void*) serialized_message.c_str(), size, "");
}

void Server::_threadHandle(){
	char msg_nw = 0;
	char msg = 0;

	while(_running){
		if (_tcp_con.isClosed()){
			_running = false;
			break;
		}

		_tcp_con.recvData((void*) &msg_nw, 1);
		msg = ntohl(msg_nw);

		LOG_DEBUG << "received trigger from server, msg = " << (int) msg << endl;
		if (msg == 0){
			_nc->sendTriggerMessage();
		}
	}

	LOG_WARNING << "leave trigger thread" << endl;
}
