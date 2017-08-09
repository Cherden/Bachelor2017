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

int UDPConnection::next_port = CONNECTION_PORT;

 UDPConnection::UDPConnection(int port)
 : _port(port)
 , _ip_address("") {
 	_socket = 0;
 	_type = UNDEFINED;
 	_info = {};

 	LOG_DEBUG << "created udp connection object with port " << port << endl;
 }

int UDPConnection::bind2(){
	if (_socket){
		close(_socket);
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

	LOG_DEBUG << "bind2 succesfull " << _socket	<< " port " << _port << endl;

	return 0;
}

int UDPConnection::createConnection(ConnectionType type, int port, string ip_address){
	_ip_address = ip_address;
	_type = type;
	_port = port;

	if (this->bind2() != 0){
		return -1;
	}

	if (_type == CLIENT){
		int msg = 1;
		struct sockaddr_in server;
		socklen_t len = sizeof(struct sockaddr_in);

		memset((char*) &server, 0, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		inet_aton(_ip_address.c_str(), &server.sin_addr);

		if (sendto(_socket, &msg, sizeof(msg), 0, (struct sockaddr*) &server, len) < 0){
			LOG_ERROR << "failed to connect (send)" << strerror(errno) << endl;
			return -1;
		}

		if (recvfrom(_socket, &msg, sizeof(msg), 0, (struct sockaddr*) &server, &len) < 0){
			LOG_ERROR << "failed to accept (recv)" << strerror(errno) << endl;
			return -1;
		}

		_port = ntohs(msg);

		if (this->bind2() != 0){
			return -1;
		}
	}

	LOG_DEBUG << "succesfully created and bound socket " << _socket
	 	<< " port " << _port << endl;

	return 0;
}

int UDPConnection::acceptConnection(struct sockaddr_in* new_client){
	if (_type != SERVER){
		LOG_WARNING << "called acceptConnection() with non SERVER type" << endl;
		return -1;
	}

	int port = -1;
	socklen_t len = sizeof(struct sockaddr_in);

	int msg = 0;

	if (_socket){
		if (recvfrom(_socket, &msg, sizeof(msg), 0, (struct sockaddr*) new_client, &len) < 0){
			if (!(errno == EAGAIN || errno == EWOULDBLOCK)){
				LOG_WARNING << "failed to accept (recv)" << strerror(errno) << endl;
			}
			return -1;
		}

		port = htons(++(UDPConnection::next_port));

		if (sendto(_socket, &port, sizeof(port), 0, (struct sockaddr*) new_client, len) < 0){
			LOG_WARNING << "failed to accept (send)" << strerror(errno) << endl;
			return -1;
		}
	} else {
		LOG_ERROR << "failed to accept because the socket is closed" << endl;
		return -1;
	}

	LOG_DEBUG << "accepted new client, socket: " << _socket << " address: "
		<< inet_ntoa(new_client->sin_addr) << " port: " << ntohs(port) << endl;

	return port;
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
	inet_aton(_ip_address.c_str(), &server.sin_addr);

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
