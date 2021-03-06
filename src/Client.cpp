#include "Client.h"

#include <iostream>
#include <stdexcept>

#include "Timer.h"
#include "Logger.h"
#include "Common.h"
#include "../gen/ConnectionMessage.pb.h"
#include "PCLUtil.h"

int Client::leader_id = -1;

Client::Client(int id, int tcp_socket)
	: _tcp_con(tcp_socket)
	, _sensor_data()
	, _data_available(0)
	, _running(1)
	, _data_mutex()
	, _client_thread(&Client::_threadHandle, this)
	, _use_point_cloud(false)
	, _video_height(0)
	, _video_width(0)
	, _depth_height(0)
	, _depth_width(0)
	, _message_size(0)
	, _recv_buf(0)
	, _id(id) {}

Client::~Client(){
	_running = 0;
	_tcp_con.closeConnection();
	_client_thread.join();
}

void Client::setInfo(struct sockaddr_in* info){
	_tcp_con.setInfo(info);
}

uint64_t Client::getTimestamp(){
	uint64_t ret = 0;
	_data_mutex.lock();
	ret = (uint64_t) (_sensor_data.timestamp() * 1000.0);
	_data_mutex.unlock();

	return ret;
}

void Client::sendTriggerMessage(){
	if (_id != Client::leader_id){
		return;
	}

	char msg = 42;
	char msg_nw = htonl(msg);
	_tcp_con.sendData((void*) &msg_nw, 1, "");

	LOG_DEBUG << "sent trigger message to master node" << endl;
}

int Client::getVideo(char** video, int size){
	_data_mutex.lock();

	int size_new = _sensor_data.fvideo_data().capacity();

	if (size < size_new && size > 0){
		free(*video);
	}

	if (*video == NULL){
		*video = (char*) malloc(size_new);
	}

	memcpy(*video, _sensor_data.fvideo_data().c_str(), size_new);

	_data_mutex.unlock();

	return size_new;
}

int Client::getDepth(char** depth, int size){
	if (_use_point_cloud){
		cout << "error in " << _id << endl;
		throw invalid_argument("illegal index in getVideo");
	}

	_data_mutex.lock();

	int size_new = _sensor_data.fdepth_data().capacity();

	if (size < size_new && size > 0){
		free(*depth);
	}

	if (*depth == NULL){
		*depth = (char*) malloc(size_new);
	}

	memcpy(*depth, _sensor_data.fdepth_data().c_str(), size_new);

	_data_mutex.unlock();

	return size_new;
}

int Client::getCloud(float** cloud, int size){
	if (!_use_point_cloud){
		return -1;
	}

	if (!_data_available){
		return -2;
	}

	_data_mutex.lock();

#ifdef PROCESS_CLOUD_DISTRIBUTED
	int size_new = _sensor_data.cloud_size();
	LOG_DEBUG << "rececived processed cloud" << endl;
#else
	int size_new = _sensor_data.fdepth_data().capacity() * 3;
	LOG_DEBUG << "processing cloud on server, size = " << size_new << endl;
#endif
	LOG_DEBUG << "testcloud ptr" << endl;
	if (size != size_new && size > 0 && *cloud != NULL){
		LOG_DEBUG << "free cloud ptr" << endl;
		free(*cloud);
	}

	if (*cloud == NULL && size_new > 0){
		LOG_DEBUG << "malloc cloud ptr with size " << size_new << endl;
		*cloud = (float*) malloc(size_new * sizeof(float));
		LOG_DEBUG << "malloc done" << endl;
	}

#ifdef PROCESS_CLOUD_DISTRIBUTED
	LOG_DEBUG << "copying cloud" << endl;
	for (int i = 0; i < size_new; i++){
		(*cloud)[i] = _sensor_data.cloud(i);
	}
#else
	LOG_DEBUG << "converting cloud" << endl;
	PCLUtil::convertToXYZPointCloud(*cloud, _sensor_data.fdepth_data().c_str(), _depth_height, _depth_width);
#endif

	_data_mutex.unlock();

	LOG_DEBUG << "returning cloud with size " << size_new << endl;
	return size_new;
}

void Client::_handleFrameMessage(){
	_tcp_con.recvData((void *) _recv_buf, _message_size);

	_data_mutex.lock();
	_sensor_data.ParseFromArray(_recv_buf, _message_size);

	LOG_DEBUG << "RECV_TIMESTAMP: " << _sensor_data.timestamp() << endl;
	
	_data_available = 1;

	_data_mutex.unlock();
}

void Client::_recvConnectionMessage(){
	ConnectionMessage m;
	uint32_t size = 0;
	uint32_t size_nw = 0;

	_tcp_con.recvData((void*) &size_nw, 4);

	size = ntohl(size_nw);
	char* buf[size] = {0};
	_tcp_con.recvData((void*) buf, size);

	m.ParseFromArray(buf, size);

	_use_point_cloud = m.use_point_cloud();
	_video_height = m.video_height();
	_video_width = m.video_width();
	_depth_height = m.depth_height();
	_depth_width = m.depth_width();

	if (m.is_leader()){
		Client::leader_id = _id;
	}

	_message_size = m.message_size();
	_recv_buf = (char*) malloc(_message_size);
}

void Client::_threadHandle(){
	int timestamp = 0;

	_recvConnectionMessage();

	while (_running){
		if (_tcp_con.isClosed()){
			_running = 0;
			break;
		}

		_handleFrameMessage();

		_data_mutex.lock();
		timestamp = _sensor_data.timestamp();
		_data_mutex.unlock();

		if (timestamp > 0) {
			LOG_DEBUG << "Timestamp = " << timestamp << endl;
		}
	}

	LOG_DEBUG << "leaving _threadHandle" << endl;
}
