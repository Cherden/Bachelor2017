#ifndef _SERVER_API_H_
#define _SERVER_API_H_

#include <thread>

#include "Common.h"
#include "Client.h"

class ServerAPI{
public:
	static ServerAPI getInstance();

	Client* getClient(int index);

	int getClientCount() { return _clients_amount; };

	int isAbleToDeliverData() { return _able_to_deliver_data; };

	~ServerAPI();

private:
	ServerAPI();
	void _acceptClients();
	void _startThread();

	volatile int _able_to_deliver_data;
	volatile bool _running;
	Client* _clients[MAX_CLIENTS];
	volatile int _clients_amount;
	thread* _accept_clients_thread;
};

#endif
