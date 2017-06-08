#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <string>
#include "KinectWrapper.h"

#define CONNECTION_PORT 79421
#define MAX_CLIENTS 1

#define SERIALIZATION_HEADER 1

using namespace std;

typedef enum{
	UNDEFINED,
	SERVER,
	CLIENT
} ConnectionType;

typedef struct{
	int header;
	int size;
} SerializationHeader;

class Connection{
public:
	Connection(int port, string ip_address);
	Connection(int socket);
	void createConnection(ConnectionType type);
	struct sockaddr_in* acceptConnection(int* new_socket);
	void sendData(void* buffer, size_t buffer_size);
	Header peekHeader();
	void recvData(void* buffer, int buffer_size);
	int recvChunks(void* buffer, int buffer_size);
	void closeConnection();
	~Connection();

private:
	int _port;
	string _ip_address;
	int _socket;
	int _option;
	ConnectionType _type;

};

#endif
