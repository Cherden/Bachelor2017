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

int UDPConnection::next_port = CONNECTION_PORT;

 UDPConnection::UDPConnection(int port)
 : _port(port)
 , _ip_address("") {
 	_socket = 0;
 	_type = UNDEFINED;
 	_info = {};

 	LOG_DEBUG << "created udp connection object with port " << port << endl;
 }

void UDPConnection::createConnection(ConnectionType type, int port, string ip_address){
	struct sockaddr_in me;

	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_socket < 0){
		LOG_ERROR << "failed to create socket " << strerror(errno) << endl;
		return;
	}


	_ip_address = ip_address;
	_type = type;

	memset((char *) &me, 0, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_port = htons(port);
	me.sin_addr.s_addr = INADDR_ANY;

	if (bind(_socket, (struct sockaddr*) &me, sizeof(me)) != 0){
		LOG_ERROR << "failed to bind the socket " << strerror(errno) << endl;
		closeConnection();
		return;
	}

	if (_type == CLIENT){
		int msg = 1;
		struct sockaddr_in server;
		socklen_t len = sizeof(struct sockaddr_in);

		memset((char*) &server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		inet_aton(_ip_address.c_str(), &server.sin_addr);

		if (sendto(_socket, &msg, sizeof(msg), 0, &server, len) < 0){
			LOG_ERROR << "failed to connect (send)" << strerror(errno) << endl;
		}

		if (recvfrom(_socket, &msg, sizeof(msg), 0, &server, &len) < 0){
			LOG_ERROR << "failed to accept (recv)" << strerror(errno) << endl;
			return -1;
		}

		_port = msg;
	} else {
		_port = port;
	}

	LOG_DEBUG << "succesfully created and bound socket " << _socket
	 	<< " port " << _port << endl;
}

int UDPConnection::acceptConnection(struct sockaddr_in* new_client){
	if (_type != SERVER){
		LOG_WARNING << "called acceptConnection() with non SERVER type" << endl;
		return -1;
	}

	int port = -1;
	struct sockaddr_in server;
	socklen_t len = sizeof(struct sockaddr_in);

	int msg = 0;

	if (_socket){
		if (recvfrom(_socket, &msg, sizeof(msg), 0, new_client, &len) < 0){
			if (!(errno == EAGAIN || errno == EWOULDBLOCK)){
				LOG_WARNING << "failed to accept (recv)" << strerror(errno) << endl;
			}
			return -1;
		}

		port = ++(UDPConnection::next_port);

		if (sendto(_socket, &port, sizeof(port), 0, new_client, len) < 0){
			LOG_WARNING << "failed to accept (send)" << strerror(errno) << endl;
		}
	} else {
		LOG_ERROR << "failed to accept because the socket is closed" << endl;
		return -1;
	}

	LOG_DEBUG << "accepted new client, socket: " << _socket << " address: "
		<< inet_ntoa(server.sin_addr) << " port: " << port);

	return port;
}

void UDPConnection::sendData(void *buffer, int buffer_size){
	struct sockaddr_in server;

	memset((char*) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(_port);
	inet_aton(_ip_address.c_str(), &server.sin_addr);

	if (_socket){
		if (sendto(_socket, buffer, buffer_size, 0
			, (struct sockaddr *) &server, sizeof(server)) < 0){
			LOG_ERROR << "failed to send data " << strerror(errno) << endl;
		} else {
			LOG_DEBUG << "sent udp packet, socket: " << _socket << " address: "
				<< inet_ntoa(server.sin_addr) << " port: " << _port << endl;
		}
	} else {
		LOG_ERROR << "failed to send data because the socket is closed" << endl;
	}
}

void UDPConnection::recvData(void *buffer, int buffer_size){
	struct sockaddr_in server;
	socklen_t len = sizeof(server);

	if (_socket){
		if (recvfrom(_socket, buffer, buffer_size, 0
			, (struct sockaddr *) &server, &len) < 0){
			LOG_ERROR << "failed to receive data " << strerror(errno) << endl;
		} else {
			LOG_DEBUG << "received udp packet, socket: " << _socket << " address: "
			  << inet_ntoa(server.sin_addr) << " port: " << ntohs(server.sin_port));
		}
	} else {
		LOG_ERROR << "failed to receive data because the socket is closed"
			<< endl;
	}
}
