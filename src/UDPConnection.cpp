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
#include "Logger.h"


UDPConnection::UDPConnection(int port, string ip_address)
 : _port(port)
 , _ip_address(ip_address)
 , _socket(0) {


	/*size_t ip_len = strlen(ip_address.c_str()) + 1;
	_ip_address = (char *) malloc(ip_len);
	strncpy(_ip_address, ip_address, ip_len);*/
}

UDPConnection::~UDPConnection(){
	if (_socket){
		close(_socket);
	}
}

void UDPConnection::createConnection(){
	struct sockaddr_in me;

	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_socket < 0){
		LOG_ERROR("failed to create socket " << strerror(errno));
	}

	memset((char *) &me, 0, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_port = htons(_port);
	me.sin_addr.s_addr = inet_addr(INADDR_ANY);

	if (bind(_socket, (struct sockaddr*) &me, sizeof(me)) != 0){
		LOG_ERROR("failed to bind the socket " << strerror(errno));
		closeConnection();
	}

	LOG_DEBUG("succesfully created and bound socket " << _socket);
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
			LOG_ERROR("failed to send data " << strerror(errno));
		} else {
			LOG_DEBUG("sent udp packet, socket: " << _socket << " address: "
			 << inet_ntoa(server.sin_addr) << " port: " << _port);
		}
	} else {
		LOG_ERROR("failed to send data because the socket is closed");
	}
}

void UDPConnection::recvData(void *buffer, int buffer_size){
	struct sockaddr_in server;
	socklen_t len = sizeof(server);

	if (_socket){
		if (recvfrom(_socket, buffer, buffer_size, 0, (struct sockaddr *) &server, &len) < 0){
			LOG_ERROR("failed to receive data " << strerror(errno));
		} else {
			LOG_DEBUG("received udp packet, socket: " << _socket << " address: "
			  << inet_ntoa(server.sin_addr) << " port: " << ntohs(server.sin_port));
		}
	} else {
		LOG_ERROR("failed to receive data because the socket is closed");
	}
}

void UDPConnection::closeConnection(){
	if (_socket){
		LOG_DEBUG("closing socket " << _socket);
		close(_socket);
	}
}
