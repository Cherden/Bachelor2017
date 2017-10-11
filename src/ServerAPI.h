#ifndef _SERVER_API_H_
#define _SERVER_API_H_

#include <thread>

#include "Common.h"
#include "Client.h"

class ServerAPI{
public:
	ServerAPI();

	Client* getClient(int index);

	int getClientCount() { return _clients_amount; };

	bool isAbleToDeliverData();

	~ServerAPI();

private:
	void _acceptClients();

	volatile bool _all_clients_connected;
	volatile bool _running;
	Client* _clients[MAX_CLIENTS];
	volatile int _clients_amount;
	thread _accept_clients_thread;
};

#endif
