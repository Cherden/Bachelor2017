#ifndef _TRANSFER_H_
#define _TRANSFER_H_

class Transfer{
public:
	Transfer(int port, char* ip_address);
	void createConnection();
	void sendData(void *buffer, int buffer_size);
	void closeConnection();
	~Transfer();
private:
	int _port;
	char* _ip_address;
	int _socket;

}

#endif
