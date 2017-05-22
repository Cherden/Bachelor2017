#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#include <string>

#define CONNECTION_PORT 79421

using namespace std;

class UDPConnection{
public:
	UDPConnection(int port, string ip_address);
	void createConnection();
	void sendData(void *buffer, int buffer_size);
	void recvData(void *buffer, int buffer_size);
	void closeConnection();
	~UDPConnection();

private:
	int _port;
	string _ip_address;
	int _socket;
	int _option;

};

#endif
