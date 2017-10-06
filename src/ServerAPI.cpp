#include "ServerAPI.h"

#include <stdexcept>

#include "TCPConnection.h"

ServerAPI::ServerAPI()
	: _able_to_deliver_data(0)
	, _running(true)
	, _clients{}
	, _clients_amount(0)
	, _accept_clients_thread(0) {}

ServerAPI::~ServerAPI(){
	_running = false;

	for (int i = 0; i < MAX_CLIENTS; i++){
		if (_clients[i] != NULL){
			delete _clients[i];
		}
	}

	_accept_clients_thread->join();
	delete _accept_clients_thread;
}

ServerAPI ServerAPI::getInstance(){
	static ServerAPI api;
	api._startThread();
	return api;
}

Client* ServerAPI::getClient(int index){
	if (index < 0 || index >= MAX_CLIENTS || _clients[index] == NULL){
		throw std::invalid_argument("illegal index in getClient");
	}

	return (_clients[index]);
}

void ServerAPI::_startThread(){
	_accept_clients_thread = new thread(&ServerAPI::_acceptClients, this);
}

void ServerAPI::_acceptClients(){
	int tcp_socket = 0;

	TCPConnection con;
	if (con.createConnection(SERVER, CONNECTION_PORT, "") < 0){
		LOG_ERROR << "Failed to create tcp socket" << endl;
		_running = false;
		return;
	}
	con.setNonBlocking();

	while (_running){
		for (int i = 0; i < _clients_amount; i++){
			if (!_clients[i]->isActive()){
				LOG_WARNING << "Node " << i << " disconnected" << endl;
				delete _clients[i];
				_able_to_deliver_data = 0;
				cout << "disconnect, reset is able" << endl;
			}
		}

		if (_clients_amount < MAX_CLIENTS){
			tcp_socket = con.acceptConnection();
			if (tcp_socket >= 0){
				int pos = 0;

				for (; pos < MAX_CLIENTS; pos++){
					if (_clients[pos] == NULL){
						break;
					}
				}

				_clients[pos] = new Client(pos, tcp_socket);

				if (++_clients_amount == MAX_CLIENTS){
					cout << "Clientcount reached" << endl;
					_able_to_deliver_data = 1;
					cout << "Is Able debug = " << _able_to_deliver_data << endl;
				} else {
					cout << "_clients_amount = " << _clients_amount << endl;
				}
			}
		}

		usleep(100000); //sleep for 1s to give main thread time
	}

	con.closeConnection();
}
