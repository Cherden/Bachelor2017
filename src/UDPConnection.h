#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include "Connection.h"

class UDPConnection: public Connection{
public:
	static int next_port;

public:

	UDPConnection() : Connection(){};
	UDPConnection(int port);

	int bind2();

	/**
		Inherited from Connection.
	*/
	int createConnection(ConnectionType type, int port, std::string ip_address);
	int acceptConnection(struct sockaddr_in* new_client);
	void sendData(const void* buffer, size_t buffer_size);
	void recvData(void* buffer, size_t buffer_size);

private:	
	int _port;
	std::string _ip_address;
};

#endif
