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
	   LOG_ERROR << "failed to create socket (bind2) " << strerror(errno) << endl;
	   return -1;
	}

	struct sockaddr_in me;

	memset((char *) &me, 0, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_port = htons(_port);
	me.sin_addr.s_addr = INADDR_ANY;

	if (bind(_socket, (struct sockaddr*) &me, sizeof(me)) != 0){
	   LOG_ERROR << "failed to rebind the socket " << strerror(errno) << endl;
	   closeConnection();
	   return -1;
	}

	LOG_DEBUG << "succesfully created and bound socket " << _socket
	 	<< " port " << _port << endl;

	return 0;
}

void UDPConnection::sendData(const void *buffer, size_t buffer_size){
	size_t len = MIN(buffer_size, MAX_UDP_FRAME-1);
	size_t remain = buffer_size;
	size_t send_bytes = 0;
	char* b = (char*) buffer;
	char to_send[MAX_UDP_FRAME] = {0};
	char id = 0;

	struct sockaddr_in server;

	memset((char*) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);
	//inet_aton(_ip_address.c_str(), &server.sin_addr); TODO

	while(remain > 0){
		if (_socket){
			to_send[0] = id++;
			memcpy(&to_send[1], b, MAX_UDP_FRAME-1);

			if ((send_bytes = sendto(_socket, (void*) to_send, len, 0
				, (struct sockaddr *) &server, sizeof(server))) < 0){
				LOG_ERROR << "failed to send data " << strerror(errno) << endl;
				return;
			}

			b += send_bytes;
			remain -= send_bytes;
			len = MIN(remain, MAX_UDP_FRAME - 1);
		} else {
			LOG_ERROR << "failed to send data because the socket is closed" << endl;
			return;
		}
	}

	LOG_DEBUG << "sent udp packet, socket: " << _socket << " address: "
		<< inet_ntoa(server.sin_addr) << " port: " << _port << endl;
}

void UDPConnection::recvData(void* buffer, size_t buffer_size){
	size_t len = MIN(buffer_size, MAX_UDP_FRAME-1);
	size_t remain = buffer_size;
	size_t rcvd_bytes = 0;
	char* b = (char*) buffer;
	char to_rcv[MAX_UDP_FRAME] = {0};

	struct sockaddr_in server;
	socklen_t addrin_len = sizeof(server);

	while(remain > 0){
		if (_socket){
			if ((rcvd_bytes = recvfrom(_socket, (void*) to_rcv, len, 0
				, (struct sockaddr *) &server, &addrin_len)) < 0){
				LOG_ERROR << "failed to receive data " << strerror(errno) << endl;
				return;
			}

			int idx = to_rcv[0] * MAX_UDP_FRAME-1;
			memcpy(&b[idx], &to_rcv[1], rcvd_bytes - 1);

			remain -= (rcvd_bytes - 1);
			len = MIN(remain, MAX_UDP_FRAME - 1);
		} else {
			LOG_ERROR << "failed to send data because the socket is closed" << endl;
			return;
		}
	}

	LOG_DEBUG << "received udp packet, socket: " << _socket << " address: "
	  << inet_ntoa(server.sin_addr) << " port: " << ntohs(server.sin_port) << endl;
}
