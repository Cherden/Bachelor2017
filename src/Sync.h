#ifndef _SYNC_H_
#define _SYNC_H_

#include <thread>

#include "Common.h"
#include "UDPConnection.h"

using namespace std;

class Sync{
public:
	Sync();

	int connect();
	void getTime(uint64_t* t);
	int isActive(){ return _running; };
	void notifyNodes();

	bool isLeader() { return _is_leader; };
	int __berkleyAlgorithm();

	~Sync();

private:
	UDPConnection _udp_con;

	volatile bool _running;
	thread _sync_thread;
	bool _is_leader;

	void _setTime(int64_t offset_sec, int64_t offset_nsec);
	void _threadHandle();

};

#endif
