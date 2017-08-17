#include "UDPConnection.h"

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

#define MAX_UDP_FRAME 1024
#define MIN(x,y) (x < y ? x : y)

UDPConnection::UDPConnection(int port)
	: _port(port) {
	_socket = 0;
	_type = UNDEFINED;
	_info = {};

	LOG_DEBUG << "created udp connection object with port " << port << endl;
}

int UDPConnection::createConnection(ConnectionType type, int port, string ip_address){
	_type = type;
	if (port != -1){
		_port = port;
	}

	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_socket < 0){
	   LOG_ERROR << "failed to create socket " << strerror(errno) << endl;
	   return -1;
	}

	struct sockaddr_in me;

	memset((char *) &me, 0, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_port = htons(_port);
	me.sin_addr.s_addr = INADDR_ANY;

	if (bind(_socket, (struct sockaddr*) &me, sizeof(me)) != 0){
	   LOG_ERROR << "failed to bind socket " << strerror(errno) << endl;
	   closeConnection();
	   return -1;
	}

	LOG_DEBUG << "succesfully created and bound socket " << _socket
	 	<< " port " << _port << endl;

	return 0;
}

void UDPConnection::sendData(const void *buffer, size_t buffer_size){
	while(remain > 0){
		if (_socket){
			if ( sendto(_socket, (void*) to_send, len, 0
				, (struct sockaddr *) &_info, sizeof(_info)) < 0){
				LOG_ERROR << "failed to send data " << strerror(errno) << endl;
				return;
			}
		} else {
			LOG_ERROR << "failed to send data because the socket is closed"
				<< endl;
			return;
		}
	}

	LOG_DEBUG << "sent udp packet, socket: " << _socket << " address: "
		<< inet_ntoa(server.sin_addr) << " port: " << _port << endl;
}

void UDPConnection::recvData(void* buffer, size_t buffer_size){
	struct sockaddr_in server;
	socklen_t addrin_len = sizeof(server);

	while(remain > 0){
		if (_socket){
			if (recvfrom(_socket, (void*) to_rcv, len, 0
				, (struct sockaddr *) &server, &addrin_len) < 0){
				LOG_ERROR << "failed to receive data " << strerror(errno)
					<< endl;
				return;
			}
		} else {
			LOG_ERROR << "failed to send data because the socket is closed"
				<< endl;
			return;
		}
	}

	LOG_DEBUG << "received udp packet, socket: " << _socket << " address: "
	 	<< inet_ntoa(server.sin_addr) << " port: " << ntohs(server.sin_port)
		<< endl;
}
