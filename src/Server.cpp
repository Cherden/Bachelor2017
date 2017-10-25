#include "Server.h"

#include <iostream>
#include <unistd.h>
#include <chrono>

#include "MessageCom.h"
#include "Logger.h"
#include "PCLUtil.h"
#include "KinectWrapper.h"
#include "../gen/ConnectionMessage.pb.h"
#include "../gen/SyncMessage.pb.h"

using namespace chrono;

Server::Server(Sync* sync, condition_variable* send_cond, mutex* send_mutex)
	: _tcp_con()
	, _running(false)
	, _server_thread(&Server::_threadHandle, this)
	, _sync(sync)
	, _can_send(false)
	, _send_cond(send_cond)
	, _send_mutex(send_mutex) {}

Server::~Server(){
	_tcp_con.closeConnection();
	_send_cond->notify_all();
}

int Server::connect(int is_leader){
	ConnectionMessage cm;
	KinectFrameMessage kfm;
	string serialized_message;
	uint32_t size_nw = 0;
	uint32_t size = 0;

	int id = 0;

	char video_buf[VIDEO_FRAME_MAX_SIZE] = {1};
	char depth_buf[DEPTH_FRAME_MAX_SIZE] = {1};
	time_t timestamp = system_clock::to_time_t(high_resolution_clock::now());

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
	}

	cm.SerializeToString(&serialized_message);
	size = cm.ByteSize();
	size_nw = htonl(size);

	LOG_DEBUG << "send connect message" << endl;
	_tcp_con.sendData((void*) &size_nw, 4);
	_tcp_con.sendData((void*) serialized_message.c_str(), size);
	LOG_DEBUG << "connect message sent" << endl;

	_running = true;

	return id;
}

void Server::sendFrameMessage(KinectFrameMessage& kfm){
	string serialized_message;
	uint32_t size = kfm.ByteSize();

	kfm.SerializeToString(&serialized_message);
	kfm.release_fvideo_data();
	kfm.release_fdepth_data();

	_tcp_con.sendData((void*) serialized_message.c_str(), size);
}


void Server::_threadHandle(){
	SyncMessage sm;
	int rtt = 0;

	//wait until instance is connected
	while (!_running);

	if (!_sync->isLeader()){
		return;
	}

	//sync
	//_sync->__berkleyAlgorithm();

	while (_running){
		unique_lock<mutex> lock(*_send_mutex);

		MessageCom::recvSmallMessage(sm, _tcp_con);

		if (sm.type() != SyncMessage_Type_READY){
			continue;
		}

		rtt = _sync->__berkleyAlgorithm();

		_sync->notifyNodes();

		usleep(rtt);

		//notify client

		_can_send = true;
		_send_cond->notify_one();
	}

	LOG_DEBUG << "leaving _threadHandle" << endl;
}
