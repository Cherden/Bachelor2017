#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include "Connection.h"

class TCPConnection: public Connection{
public:
	/**
		Call super class constructor.
	*/
	TCPConnection() : Connection(){};

	/**
		Constructor to create instance of getting a socket from accept().

		@param Socket from the new accepted client.
	*/
	TCPConnection(int socket);

	/**
		Inherited from Connection.
	*/
	int createConnection(ConnectionType type, int port, std::string ip_address);
	int acceptConnection(struct sockaddr_in* new_client);
	void sendData(const void* buffer, size_t buffer_size);
	void recvData(void* buffer, int buffer_size);
	int isClosed();
	void setNonBlocking();
	void setInfo(struct sockaddr_in* info);
	void closeConnection();
};

#endif
