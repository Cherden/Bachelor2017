#include "TimingClient.h"

#include <time.h>
#include "../gen/TimingMessage.pb.h"

TimingClient::TimingClient(UDPConnection* con)
	: _con(con)
	, _running(1)
	, _thread(&TimingClient::_threadHandle, this){

}

TimingClient::~TimingClient(){
	_running = 0;
}

void TimingClient::getTime(uint64_t* t){
	struct timeval tv;

	gettimeofday(&tv, NULL);
	cout << "tv.tv_sec " << tv.tv_sec << endl;
	cout << "tv.tv_usec "  << tv.tv_usec << endl;

	t[0] = (uint64_t) tv.tv_sec;
	t[1] = (uint64_t) tv.tv_usec;
}

void TimingClient::setTime(long time_offset){
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec += time_offset / 1000.0;
	t.tv_nsec += time_offset - (t.tv_sec * 1000.0);

	clock_settime(CLOCK_REALTIME, &t);
}

void TimingClient::_threadHandle(){
	uint64_t var[2] = {0};
	TimingMessage tm;
	string serialized_message;
	serialized_message.resize(10);

	while (_running){
		if (_con->isClosed()){
			_running = 0;
			break;
		}

		// recv trigger message
		_con->recvData((void*) serialized_message.c_str(), 10);

		// send getTime()
		getTime(var);
		tm.set_sec(var[0]);
		tm.set_usec(var[1]);
		cout << "my time = {" << tm.sec() << ", " << tm.usec() << "}" << endl;
		tm.SerializeToString(&serialized_message);
		cout << tm.ByteSize() << endl;
		_con->sendData((void*) serialized_message.c_str(), tm.ByteSize());

		// recv offset
		_con->recvData((void*) serialized_message.c_str(), 10);
		tm.ParseFromString(serialized_message);

		// call setTime(offset)
		cout << "offset = {" << tm.sec() << ", " << tm.usec() << "}" << endl;
	}

	LOG_DEBUG << "leaving TimingClient::_threadHandle" << endl;
}
