#include "Client.h"

#include <iostream>

#include "Logger.h"

Client::Client(int socket)
	: _con(socket)
	, _sensor_data()
	, _data_available(0)
	, _running(1)
	, _data_mutex()
	, _client_thread(&Client::_threadHandle, this) {}

Client::~Client(){
	_running = 0;
	_con.closeConnection();
	_client_thread.join();
}

void Client::setInfo(struct sockaddr_in* info){
	_con.setInfo(info);
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


	if (!_sensor_data.is_point_cloud() && _sensor_data.fdepth_data() != ""){
		size = _sensor_data.fdepth_data().capacity();
		if (*depth == NULL){
			*depth = (char*) malloc(size);
		}
		memcpy(*depth, _sensor_data.fdepth_data().c_str(), size);
	} else if (_sensor_data.is_point_cloud()){
		if (*cloud == NULL){
			*cloud = (float*) malloc(_sensor_data.fdepth_size() * sizeof(float));
		}
		for (int i = 0; i < _sensor_data.cloud_size(); i++){
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

	_con.recvData((void *) buf, len);

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

void Client::_threadHandle(){
	uint64_t size = 0;
	int timestamp = 0;

	while (_running){
		if (_con.isClosed()){
			_running = 0;
			break;
		}

		_con.recvData((void*) &size, 4);
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
