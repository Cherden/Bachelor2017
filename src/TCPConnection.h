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
		Accept one new client from the listening socket. On accept() failure it
		logs the errno output string.
		@param new_client Pointer, where the info struct of the accepted client
		will be saved.
		@return Socket of the accepted client on success, -1 otherwise.
	*/
	int acceptConnection(struct sockaddr_in* new_client);
	

	/**
		Inherited from Connection.
	*/
	int createConnection(ConnectionType type, int port, std::string ip_address);
	void sendData(const void* buffer, size_t buffer_size);
	void recvData(void* buffer, size_t buffer_size);
};

#endif
