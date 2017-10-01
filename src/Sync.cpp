#include "Sync.h"

#include <iostream>

#include "Logger.h"

Sync::Sync(int id)
	: _id(id)
	, _udp_con(CONNECTION_PORT)
	, _running(1)
	, _sync_thread(&Sync::_threadHandle, this) {}

Sync::~Sync(){
	_running = 0;
	_udp_con.closeConnection();
	_sync_thread.join();
}

void Sync::_threadHandle(){
	uint64_t size = 0;
	int timestamp = 0;

	_udp_con.createConnection(SERVER, -1, "");

	while (_running){
		// sync
	}

	LOG_DEBUG << "leaving _threadHandle" << endl;
}
