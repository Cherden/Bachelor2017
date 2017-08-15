#include "Client.h"

#include <iostream>

#include "Logger.h"
#include "../gen/ConnectionMessage.pb.h"

Client::Client(int tcp_socket, int udp_port)
	: _tcp_con(tcp_socket)
	, _udp_con(udp_port)
	, _sensor_data()
	, _data_available(0)
	, _running(1)
	, _data_mutex()
	, _client_thread(&Client::_threadHandle, this) {}

Client::~Client(){
	_running = 0;
	_tcp_con.closeConnection();
	_udp_con.closeConnection();
	_client_thread.join();
}

void Client::setInfo(struct sockaddr_in* info){
	_tcp_con.setInfo(info);
	_udp_con.setInfo(info);
}

int Client::getData(char** video, char** depth, float** cloud){
	if (!_data_available){
		return -1;
	}

	_data_mutex.lock();

	int size = _sensor_data.fvideo_data().capacity();
	if (*video == NULL){
		*video = (char*) malloc(size);
	}
	memcpy(*video, _sensor_data.fvideo_data().c_str(), size);


	if (!_use_point_cloud && _sensor_data.fdepth_data() != ""){
		size = _sensor_data.fdepth_data().capacity();
		if (*depth == NULL){
			*depth = (char*) malloc(size);
		}
		memcpy(*depth, _sensor_data.fdepth_data().c_str(), size);
	} else if (_use_point_cloud){
		if (*cloud == NULL){
			*cloud = (float*) malloc(size * 3 * sizeof(float));
		}
		for (int i = 0; i < size * 3; i++){
			*cloud[i] = _sensor_data.cloud(i);
		}
	} else {
		_data_mutex.unlock();
		return -1;
	}

	_data_available = 0;

	_data_mutex.unlock();

	return 0;
}

void Client::_handleFrameMessage(int len){
	char* buf = (char*) malloc(len);

	_tcp_con.recvData((void *) buf, len);

	_data_mutex.lock();
	_sensor_data.ParseFromArray(buf, len);

	if (_sensor_data.fvideo_data() == "" || _sensor_data.timestamp() == 0){
		LOG_ERROR << "message does not contain at least one required field"
			<< endl;
	} else {
		_data_available = 1;
	}

	_data_mutex.unlock();

	free(buf);
}

void Client::_sendConnectionMessage(){
	ConnectionMessage m;
	string serialized_message;

	m.set_udp_port(_udp_con.getPort());
	m.SerializeToString(&serialized_message);

	uint32_t size = m.ByteSize();
	uint32_t size_nw = htonl(size);
	_tcp_con.sendData((void*) &size_nw, 4);
	_tcp_con.sendData((void*) serialized_message.c_str(), size);

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
}

void Client::_threadHandle(){
	uint64_t size = 0;
	int timestamp = 0;

	_udp_con.createConnection(SERVER, -1, NULL);
	_sendConnectionMessage();

	while (_running){
		if (_tcp_con.isClosed()){
			_running = 0;
			break;
		}

		_tcp_con.recvData((void*) &size, 4);
		size = ntohl(size);

		LOG_DEBUG << "next protobuf message size is " << size << endl;

		_handleFrameMessage(size);

		_data_mutex.lock();
		timestamp = _sensor_data.timestamp();
		_data_mutex.unlock();

		if (timestamp > 0) {
			LOG_DEBUG << "Timestamp = " << timestamp << endl;
		}
	}

	LOG_DEBUG << "leaving _threadHandle" << endl;
}
