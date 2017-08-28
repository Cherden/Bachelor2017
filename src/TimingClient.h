#ifndef _TIMING_CLIENT_H_
#define _TIMING_CLIENT_H_

#include "Common.h"
#include "UDPConnection.h"
#include <thread>

class TimingClient{
public:
	TimingClient(UDPConnection* con);
	~TimingClient();

	void getTime(uint64_t* t);
	void setTime(long time_offset);


private:
	void _threadHandle();

	UDPConnection* _con;
	volatile int _running;
	thread _thread;
};

#endif
