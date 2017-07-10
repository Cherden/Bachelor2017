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

int Client::getData(char** video, char** depth){
	if (!_data_available){
		return -1;
	}

	_data_mutex.lock();

	*video = (char*) malloc(_sensor_data.fvideo_size());
	memcpy(*video, _sensor_data.fvideo_data().c_str()
		, _sensor_data.fvideo_size());


	*depth = (char*) malloc(_sensor_data.fdepth_size());
	memcpy(*depth, _sensor_data.fdepth_data().c_str()
		, _sensor_data.fdepth_size());

	_data_available = 0;

	_data_mutex.unlock();

	return 0;
}

void Client::_handleFrameMessage(int len){
	char* buf = (char*) malloc(len);

	_con.recvData((void *) buf, len);

	_data_mutex.lock();
	_sensor_data.ParseFromArray(buf, len);

	if (_sensor_data.fvideo_data() == "" || _sensor_data.fdepth_data() == ""
		|| _sensor_data.timestamp() == 0){
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
