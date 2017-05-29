#include "Connection.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include "Logger.h"

using namespace std;

Connection::Connection(int port, string ip_address)
 : _port(port)
 , _ip_address(ip_address)
 , _socket(0)
 , _type(UNDEFINED) {}

Connection::Connection(int socket)
 : _port(0)
 , _ip_address("")
 , _socket(socket)
 , _type(SERVER) {}

Connection::~Connection(){
	if (_socket){
		close(_socket);
	}
}

void Connection::createConnection(ConnectionType type){
	struct sockaddr_in me;

	memset ((void*) &me, 0, sizeof(me));

	if (_type != UNDEFINED){
		return;
	}
	_type = type;

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0){
		LOG_ERROR << "failed to create socket : " << strerror(errno) << endl;
		return;
	}

	me.sin_family = AF_INET;
	me.sin_port = htons(_port);

	if (_type == SERVER){
		me.sin_addr.s_addr = INADDR_ANY;

		if (bind(_socket, (struct sockaddr*) &me, sizeof(me)) != 0){
			LOG_ERROR << "failed to bind the socket : " << strerror(errno) << endl;
			closeConnection();
			return;
		}

		listen(_socket, MAX_CLIENTS);

		LOG_DEBUG << "server socket succesfully created, listening " << _socket << endl;
	} else if (_type == CLIENT){
		inet_aton(_ip_address.c_str(), &me.sin_addr);

		if (connect(_socket, (struct sockaddr*) &me, sizeof(me)) != 0){
			LOG_ERROR << "failed to connect to server : " << strerror(errno) << endl;
			closeConnection();
			return;
		}

		LOG_DEBUG << "client connection succesfully created " << _socket << endl;
	}
}

struct sockaddr_in* Connection::acceptConnection(int *new_socket){
	socklen_t client_len = sizeof(struct sockaddr_in);
	struct sockaddr_in* client = (struct sockaddr_in*) malloc(client_len);

	memset((void*) client, 0, client_len);

	if ((*new_socket = accept(_socket, (struct sockaddr*) client, &client_len)) < 0){
		LOG_WARNING << "accepting new client failed : " << strerror(errno) << endl;
		free(client);
		return 0;
	}

	LOG_DEBUG << "accepted new client on " << _socket << ", new socket " << *new_socket << endl;

	return client;
}

void Connection::sendData(char *buffer, int buffer_size){
	if (_socket){
		if (send(_socket, (void *) buffer, buffer_size, 0) <= 0){
			LOG_ERROR << "failed to send data : " << strerror(errno) << endl;
			closeConnection();
		} else {
			LOG_DEBUG << "sent packet, socket: " << _socket << " address: " \
			 << _ip_address << " port: " << _port << endl;
		}
	} else {
		LOG_ERROR << "failed to send data because the socket is closed" << endl;
	}
}

void Connection::recvData(char *buffer, int buffer_size){
	int ret = 0;

	if (_socket){
		if ((ret = recvChunks(buffer, buffer_size)) < 0){
			LOG_ERROR << "failed to receive data : " << strerror(errno) << endl;
		} else if (ret == 0) {
			LOG_ERROR << "received 0 data, closing connection" << endl;
			closeConnection();
		} else {
			LOG_DEBUG <<"received " << ret << " bytes over socket " << _socket << endl;
		}
	} else {
		LOG_ERROR << "failed to receive data because the socket is closed"  << endl;
	}
}

int Connection::recvChunks(char *buffer, int buffer_size){
	int ret = 0;
	int recevied_bytes = 0;

	while(recevied_bytes != buffer_size){
		if ((ret = recv(_socket, (void *) &buffer[recevied_bytes]
				, buffer_size - recevied_bytes, 0)) < 0){
			return ret;
		}

		recevied_bytes += ret;
	}

	return recevied_bytes;
}

void Connection::closeConnection(){
	if (_socket){
		LOG_DEBUG << "closing socket " << _socket << endl;
		close(_socket);
		_socket = 0;
	}
}
