#ifndef _SYNC_H_
#define _SYNC_H_

#include <thread>

#include "Common.h"

#include "UDPConnection.h"

using namespace std;

class Sync{
public:
	Sync(int id);
	
	int isActive(){ return _running; };

	~Sync();


private:
	int _id;
	void _threadHandle();

	UDPConnection _udp_con;

	volatile int _running;
	thread _sync_thread;
};

#endif
