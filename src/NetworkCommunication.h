#ifndef _NETWORK_COMMUNICATION_H_
#define _NETWORK_COMMUNICATION_H_

#include <thread>
#include <google/protobuf/message_lite.h>
#include <map>

#include "Common.h"

#include "UDPConnection.h"

using namespace std;
using namespace google::protobuf;

class NetworkCommunication{
public:
	NetworkCommunication();

	int connect();
	int isActive(){ return _running; };
	bool isTriggered();
	void synchronize();

	void sendTriggerMessage();

	~NetworkCommunication();

private:
	UDPConnection _udp_con;

	volatile bool _running;
	thread _nw_thread;
	bool _is_leader;
	bool _trigger;
	volatile int _sync_answer_count;
	map<string, uint64_t> _rtt_map;

	void _setTime(int64_t offset_nsec);
	void _sendMessage(MessageLite& m, string ip);
	int _recvMessage(MessageLite& m);
	void _threadHandle();

	void __berkleyAlgorithm();
};

#endif
