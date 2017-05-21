#ifndef _UDP_CONNECTION_H_
#define _UDP_CONNECTION_H_

#define CONNECTION_PORT 79421

class UDPConnection{
public:
	UDPConnection(int port, char* ip_address, int option);
	void createConnection();
	void sendData(void *buffer, int buffer_size);
	void recvData(void *buffer, int buffer_size);
	void closeConnection();
	~UDPConnection();
	
private:
	int _port;
	char* _ip_address;
	int _socket;
	int _option;
	
};

#endif
