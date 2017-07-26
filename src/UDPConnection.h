#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include "Connection.h"

class UDPConnection: public Connection{
public:
	/**
		Inherited from Connection.
	*/
	int createConnection(ConnectionType type, int port, std::string ip_address);
	int acceptConnection(struct sockaddr_in* new_client);
	void sendData(const void* buffer, size_t buffer_size);
	void recvData(void* buffer, int buffer_size);
};

private:
	int _port;

#endif
