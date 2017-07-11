#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include <string>
#include <arpa/inet.h>

/**
	Port used for communication from this program
*/
#define CONNECTION_PORT 79421

/**
	Amount of clients the server can accept
*/
#define MAX_CLIENTS 2


typedef enum{
	UNDEFINED,
	SERVER,
	CLIENT
} ConnectionType;

class TCPConnection{
public:
	/**
		Constructor to create new socket.

	*/
	TCPConnection();

	/**
		Constructor to create instance of getting a socket from accept().

		@param Socket from the new accepted client.
	*/
	TCPConnection(int socket);

	/**
		Connect to _ip_address or listen() on socket. On connect(), listen() or
		bind()failure it logs the errno output string.

		@param type Whether to handle this instance as a server or client.
		@param port The port this connection should use.
		@param ip_address IPv4 address where the instance should connect itself
		to. Can be NULL for type == SERVER.
		@return 0 on success, -1 otherwise.
	*/
	int createConnection(ConnectionType type, int port, std::string ip_address);

	/**
		Accept one new client from the listening socket. On accept() failure it
		logs the errno output string.

		@param new_client Pointer, where the info struct of the accepted client
		will be saved.
		@return Socket of the accepted client on success, -1 otherwise.
	*/
	int acceptConnection(struct sockaddr_in* new_client);

	/**
		Send data over _socket. On send() failure it logs the errno output
		string.

		@param buffer Pointer to the data buffer, which should be sent.
		@param buffer_size The amount of data to be sent from buffer.
	*/
	void sendData(const void* buffer, size_t buffer_size);

	//Header peekHeader();

	/**
		Receive data from socket. Is blocking. Calls _recvChunks() to receive
		bigger amounts of data. On recv() failure it logs the errno output
		string.

		@param buffer The buffer in which the received data will be stored.
		@param buffer_size The maximum amount of bytes to be written in the
		buffer.
	*/
	void recvData(void* buffer, int buffer_size);

	/**
		Check wether the socket is closed or not.

		@return 1 if the socket is closed, 0 otherwise
	*/
	int isClosed();

	/**
		Marks the socket as non-blocking.
	*/
	void setNonBlocking();

	/**
		Copies the info about the connection.

		@param info The struct, from which the connection will copy.
	*/
	void setInfo(struct sockaddr_in* info);

	/**
		Close the socket. The instance will not be deleted, so be carefull
		calling other functions with a closed socket.
	*/
	void closeConnection();
	~TCPConnection();

private:
	/**
		If buffer_size > SO_RCVBUF call this function to put the chunks together
		in one buffer.

		@param buffer The buffer in which the received data will be stored.
		@param buffer_size The maximum amount of bytes to be written in the
		buffer.
		@returns recv() return value.
	*/
	int _recvChunks(void* buffer, int buffer_size);

	int _socket;				//The socket of the connection.
	ConnectionType _type;		//The type of connection (SERVER or CLIENT).
	struct sockaddr_in _info;	//Contains information about the other part of
								//the connection.

};

#endif
