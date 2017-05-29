#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <string>

#define CONNECTION_PORT 79421
#define MAX_CLIENTS 3

using namespace std;

typedef enum{
	UNDEFINED,
	SERVER,
	CLIENT
} ConnectionType;

class Connection{
public:
	Connection(int port, string ip_address);
	Connection(int socket);
	void createConnection(ConnectionType type);
	struct sockaddr_in* acceptConnection(int *new_socket);
	void sendData(char *buffer, int buffer_size);
	void recvData(char *buffer, int buffer_size);
	int recvChunks(char *buffer, int buffer_size);
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
