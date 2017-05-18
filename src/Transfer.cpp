#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Transfer::Transfer(int port, char* ip_address)
	: _port(0)
	, _ip_address(0)
	, _socket(0){

	size_t ip_len = strlen(ip_address) + 1;
	_ip_address = (char *) malloc(ip_len);
	strncpy(_ip_address, ip_address, ip_len);
}

Transfer::~Transfer(){
	free(_ip_address);

	if (_socket){
		close(_socket);
	}
}

void Transfer::createConnection(){
	struct sockaddr_in dst;

	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
	if (_socket < 0){
		//TODO
	}

	dst.sin_family = AF_INET;
	dst.sin_port = htons(_port);
	dst.sin_addr.s_addr = inet_addr(_ip_address);

	if (connect(_socket, (struct sockaddr*) &dst, sizeof(dst)) != 0){
		//TODO
		if (_socket){
			close(_socket);
		}
	}
}

void Transfer::sendData(void *buffer, int buffer_size){
	if (_socket){
		if (send(_socket, buffer, buffer_size, 0) != buffer_size){
			//TODO
		}
	} else {
		//TODO
	}
}

void Transfer::closeConnection(){
	if (_socket){
		close(_socket);
	}
}
