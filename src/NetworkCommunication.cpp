#include "NetworkCommunication.h"

#include <iostream>
#include <unistd.h>
#include <errno.h>

#include "Common.h"
#include "../gen/NetworkMessage.pb.h"
#include "Logger.h"

NetworkCommunication::NetworkCommunication()
	: _udp_con(CONNECTION_PORT)
	, _running(false)
	, _nw_thread(&NetworkCommunication::_threadHandle, this)
	, _is_leader(false)
	, _trigger(false)
	, _sync_answer_count(0) {}

NetworkCommunication::~NetworkCommunication(){
	_running = false;
	_udp_con.closeConnection();
	_nw_thread.join();
}

void NetworkCommunication::sendTriggerMessage(){
	NetworkMessage nm;

	nm.set_type(NetworkMessage_Type_TRIGGER);

	_sendMessage(nm, BROADCAST_IP);

	LOG_DEBUG << "broadcasted trigger message" << endl;
	usleep(700);
	_trigger = true;
}

bool NetworkCommunication::isTriggered(){
	if(_trigger){
		_trigger = false;
		return true;
	} else {
		return false;
	}
}

int NetworkCommunication::connect(){
	NetworkMessage sm;
	int timeout = 10000;
	int found_leader = 0;

	_udp_con.createConnection(CLIENT, -1, "");
	_udp_con.enableBroadcast();
	_udp_con.setRecvTimout(timeout);

	sm.set_type(NetworkMessage_Type_ELECTION);
	sm.set_leader(false);

	_sendMessage(sm, BROADCAST_IP);

	int ret = 0;
	while (1){
		if ((ret = _recvMessage(sm)) == 1){
			//timout
			LOG_DEBUG << "Election timeout" << endl;
			if (found_leader == 0){
				_is_leader = true;
				LOG_DEBUG << "I am the new leader" << endl;
			}
			break;
		} else if (ret == 0) {
			// TODO save ips in list
			LOG_DEBUG << "Received election message from "
				<< _udp_con.getIPFromLastSender() << ", Leader = "
				<< sm.leader() << endl;
			if (sm.type() == NetworkMessage_Type_ELECTION && sm.leader()){
				found_leader = 1;
			}
		}
	}

	_running = true;

	return !found_leader;
}

void NetworkCommunication::_setTime(int64_t offset_usec){
	struct timeval t;
	if(gettimeofday(&t, NULL) == -1){
		LOG_WARNING << "settime error: " << strerror(errno) << endl;
	}

	t.tv_sec += (offset_usec / 1000000ULL);
	t.tv_usec += (offset_usec % 1000000ULL);

	LOG_DEBUG << "syncing " << t.tv_sec << " s and " << t.tv_usec << " us ("<<offset_usec<<")" << endl;

	if(settimeofday(&t, NULL) == -1){
		LOG_WARNING << "settime error: " << strerror(errno) << endl;
	}

	LOG_DEBUG << "settimeofday succsefull" << endl;
}

void NetworkCommunication::_sendMessage(MessageLite& m, string ip){
	if (m.ByteSize() > 254){
		LOG_WARNING << "serialized NetworkMessage length exceeded 254 Bytes ("
			<< m.ByteSize() << ")" << endl;
		return;
	}

	char size = m.ByteSize();
	char buffer[255] = {0};

	m.SerializeToArray(&buffer[1], 254);
	buffer[0] = size;

	_udp_con.sendData(buffer, 255, ip);
}

int NetworkCommunication::_recvMessage(MessageLite& m){
	char buffer[255] = {0};

	int ret = 0;
	if ((ret = _udp_con.recvData(buffer, 255)) != 0){
		return ret;
	}

	int msg_len = buffer[0];

	m.ParseFromArray(&buffer[1], msg_len);

	return 0;
}

void NetworkCommunication::synchronize(){
	__berkleyAlgorithm();
}

void NetworkCommunication::__berkleyAlgorithm(){
	if (!_is_leader){
		return;
	}

	NetworkMessage nm;

	//send broadcast
	LOG_DEBUG << "SYNC: send broadcast" << endl;
	nm.set_type(NetworkMessage_Type_SYNC);
	nm.set_sync_mode(NetworkMessage_SyncMode_SEND);
	_sendMessage(nm, BROADCAST_IP);
	double rtt1 = Common::getTime(NULL);

	// wait for responses
	LOG_DEBUG << "SYNC: wait for responses" << endl;
	while(_sync_answer_count != MAX_CLIENTS-1){};
	uint64_t ping = (uint64_t) ((Common::getTime(NULL) - rtt1) / 1000.0);
	LOG_ERROR << "SYNC_PING: " << ping << endl;
	_sync_answer_count = 0;

	uint64_t old_ts = (uint64_t) (Common::getTime(NULL) * 1000.0);
	uint64_t avg = old_ts;

	//average all clock times
	for (auto const &client : _rtt_map){
		avg = (avg + client.second) / 2;
	}
	avg += ping / 2;
	old_ts += ping;
	LOG_DEBUG << "SYNC: avg is " << avg << endl;

	//send offset
	nm.set_type(NetworkMessage_Type_SYNC);
	nm.set_sync_mode(NetworkMessage_SyncMode_ADJUST);
	for (auto const &client : _rtt_map){
		nm.set_offset_usec(avg - client.second);
		_sendMessage(nm, client.first);

		LOG_DEBUG << nm.offset_usec() << " us offset to " << client.first << endl;
	}

	_setTime(avg - old_ts);
}

void NetworkCommunication::_threadHandle(){
	NetworkMessage sm;

	//wait until instance is connected
	while (!_running);

	while (_running){
		if (_udp_con.isClosed()){
			_running = false;
			break;
		}

		if (_recvMessage(sm) != 0){
			continue;
		}

		if (sm.type() == NetworkMessage_Type_SYNC){
			if (_is_leader){
				if (sm.sync_mode() == NetworkMessage_SyncMode_REPLY){
					_rtt_map[_udp_con.getIPFromLastSender()] = sm.timestamp();
					LOG_DEBUG << "SYNC: add " << _udp_con.getIPFromLastSender() << " with " << sm.timestamp() << endl;
					_sync_answer_count++;
				}
			} else {
				if (sm.sync_mode() == NetworkMessage_SyncMode_SEND){
					sm.set_sync_mode(NetworkMessage_SyncMode_REPLY);
					sm.set_timestamp((uint64_t) (Common::getTime(NULL) * 1000.0));
					_sendMessage(sm, _udp_con.getIPFromLastSender());
					LOG_DEBUG << "SYNC: send reply to " << _udp_con.getIPFromLastSender() << endl;
				} else if (sm.sync_mode() == NetworkMessage_SyncMode_ADJUST){
					_setTime(sm.offset_usec());
				}
			}
		} else if (sm.type() == NetworkMessage_Type_TRIGGER) {
			if (!_is_leader){
				_trigger = true;
				LOG_DEBUG << "received trigger message from master node" << endl;
			}
		} else if (sm.type() == NetworkMessage_Type_ELECTION) {
			sm.set_type(NetworkMessage_Type_ELECTION);
			sm.set_leader(_is_leader);
			_sendMessage(sm, _udp_con.getIPFromLastSender());
		} else {
			LOG_WARNING << "Unknown NetworkMessage type " << sm.type() << endl;
		}
	}

	LOG_DEBUG << "leaving _threadHandle" << endl;
}
