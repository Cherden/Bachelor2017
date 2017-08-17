#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include "Connection.h"

class UDPConnection: public Connection{
public:

	UDPConnection() : Connection(){};
	UDPConnection(int port);

	int getPort(){ return _port; };
	void setInfo(struct sockaddr_in* info){
		std::memcpy(&_info, info, sizeof(struct sockaddr_in));
		if (_port != 0){
			_info.sin_port = htons(_port);
		}
	};

	/**
		Inherited from Connection.
	*/
	int createConnection(ConnectionType type, int port, std::string ip_address);
	void sendData(const void* buffer, size_t buffer_size);
	void recvData(void* buffer, size_t buffer_size);

private:
	int _port;
};

#endif
