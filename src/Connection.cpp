#include "Connection.h"

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>

#include "Logger.h"

using namespace std;

Connection::Connection()
	: _socket(0)
	, _type(UNDEFINED)
	, _info({}) {}

Connection::Connection(int socket)
	: _socket(socket)
	, _type(CLIENT)
	, _info({}) {
		LOG_DEBUG << "created connection object with socket " << socket << endl;
	}

Connection::~Connection(){
	if (_socket){
		LOG_WARNING << "connection instance died without closing first" << endl;
		close(_socket);
	}
}

int Connection::createConnection(ConnectionType type, int port, string ip_address){
	struct sockaddr_in me;

	memset((void*) &me, 0, sizeof(me));

	if (_type != UNDEFINED){
		return -1;
	}
	_type = type;

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0){
		LOG_ERROR << "failed to create socket, strerror : " << strerror(errno)
			<< endl;

		return -1;
	}

	_info.sin_family = AF_INET;
	_info.sin_port = htons(port);

	if (_type == SERVER){
		_info.sin_addr.s_addr = INADDR_ANY;

		if (bind(_socket, (struct sockaddr*) &_info, sizeof(_info)) != 0){
			LOG_ERROR << "failed to bind the socket, strerror : "
				<< strerror(errno) << endl;

			closeConnection();
			return -1;
		}

		listen(_socket, MAX_CLIENTS);

		LOG_DEBUG << "server socket succesfully created, listening " << _socket
			<< endl;
	} else if (_type == CLIENT){
		inet_aton(ip_address.c_str(), &_info.sin_addr);

		if (connect(_socket, (struct sockaddr*) &_info, sizeof(_info)) != 0){
			LOG_ERROR << "failed to connect to server, strerror : "
				<< strerror(errno) << endl;

			closeConnection();
			return -1;
		}

		LOG_DEBUG << "client connection succesfully created " << _socket
			<< endl;
	}

	return 0;
}

/*	DEPRECATED VERSION

struct sockaddr_in* Connection::acceptConnection(int* new_socket){
	socklen_t client_len = sizeof(struct sockaddr_in);
	struct sockaddr_in* client = (struct sockaddr_in*) malloc(client_len);

	memset((void*) client, 0, client_len);

	if ((*new_socket
	 	 = accept(_socket, (struct sockaddr*) client, &client_len)) < 0){
		LOG_WARNING << "accepting new client failed, strerror : "
			<< strerror(errno) << endl;
		free(client);
		return 0;
	}

	LOG_DEBUG << "accepted new client on " << _socket << ", new socket "
		<< *new_socket << endl;

	return client;
}*/

int Connection::acceptConnection(struct sockaddr_in* new_client){
	if (_type != SERVER){
		LOG_WARNING << "called acceptConnection() with non SERVER type" << endl;
		return -1;
	}

	int socket = 0;
	socklen_t client_size = sizeof(struct sockaddr_in);

	if ((socket = accept4(_socket, (struct sockaddr*) new_client
			, &client_size, SOCK_NONBLOCK)) < 0){
		if (!(socket == EAGAIN || socket == EWOULDBLOCK)){
			LOG_WARNING << "accepting new client failed, strerror : "
			<< strerror(errno) << endl;
		}

		return -1;
	}

	LOG_DEBUG << "accepted new client on " << _socket << ", new socket "
		<< socket << endl;

	return socket;
}

void Connection::sendData(void* buffer, size_t buffer_size){
	if (_socket){
		if (send(_socket, buffer, buffer_size, MSG_NOSIGNAL) <= 0){
			LOG_ERROR << "failed to send data (" << _socket << "), strerror : "
			 	<< strerror(errno) << endl;

			closeConnection();
		} else {
			LOG_DEBUG << "sent " << buffer_size << " bytes, socket: "
				<< _socket << endl;
		}
	} else {
		LOG_ERROR << "failed to send data because the socket closed" << endl;
	}
}

/*	DEPRECATED FUNCTION

	Header Connection::peekHeader(){
	Header h = UNKNOWN;
	int ret = 0;

	if (_socket){
		if ((ret = recv(_socket, (void*) &h, sizeof(h), MSG_PEEK)) < 0){
			LOG_ERROR << "failed to peek header : " << strerror(errno) << endl;
		} else if (ret == 0) {
			LOG_ERROR << "received 0 data, closing connection" << endl;
			closeConnection();
		} else {
			LOG_DEBUG <<"received data with header " << h
				<< " on socket " << _socket << endl;
		}
	} else {
		LOG_ERROR << "failed to peek header because the socket is closed"
		  << endl;
	}
	return h;
}*/

void Connection::recvData(void* buffer, int buffer_size){
	int ret = EAGAIN;

	if (_socket){
		/*
		int recv_buff_size = 0;
		size_t recv_buff_size_size = sizeof(recv_buff_size);

		getsockopt(_socket, SOL_SOCKET, SO_RCVBUF, (void *)&recv_buff_size
			, &recv_buff_size_size);

		if (recv_buff_size < buffer_size) {
			ret = _recvChunks(buffer, buffer_size);
		} else {
			ret = recv(_socket, buffer, buffer_size, 0);
		}

		if (ret < 0){
		*/

		while (ret == EAGAIN){
			//ret = _recvChunks(buffer, buffer_size);
			ret = recv(_socket, buffer, buffer_size, MSG_WAITALL);
		}

		if (ret < 0){
			LOG_ERROR << "failed to receive data (" << _socket
				<< "), strerror : " << strerror(errno) << endl;
			closeConnection();
		} else if (ret == 0) {
			LOG_ERROR << "received 0 data, closing connection" << endl;
			closeConnection();
		} else {
			LOG_DEBUG <<"received " << ret << " bytes over socket " << _socket
				<< endl;
		}


	} else {
		LOG_ERROR << "failed to receive data because the socket closed" << endl;
	}
}

int Connection::_recvChunks(void* buffer, int buffer_size){
	int ret = 0;
	int recevied_bytes = 0;

	while(recevied_bytes != buffer_size){
		if (_socket){
			if ((ret = recv(_socket, (void*) &((char*)buffer)[recevied_bytes]
					, buffer_size - recevied_bytes, 0)) < 0){
				return ret;
			}

			recevied_bytes += ret;
		} else {
			return 0;
		}
	}

	return recevied_bytes;
}

void Connection::setInfo(struct sockaddr_in* info){
	memcpy(&_info, info, sizeof(struct sockaddr_in));
}

void Connection::closeConnection(){
	if (_socket){
		LOG_DEBUG << "closing socket " << _socket << endl;
		close(_socket);
		_socket = 0;
	}
}

int Connection::isClosed(){
	return _socket==0;
}
