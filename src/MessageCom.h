
#include <google/protobuf/message_lite.h>

#include "Connection.h"

using namespace std;
using namespace google::protobuf;

class MessageCom{
public:
  static void sendSmallMessage(MessageLite& m, Connection& con, string ip = ""){
  	if (m.ByteSize() > 254){
  		LOG_WARNING << "serialized SyncMessage length exceeded 254 Bytes ("
  			<< m.ByteSize() << ")" << endl;
  		return;
  	}

  	char size = m.ByteSize();
  	char buffer[255] = {0};

  	m.SerializeToArray(&buffer[1], 254);
  	buffer[0] = size;

  	con.sendData(buffer, 255, ip);
  };

  static int recvSmallMessage(MessageLite& m, Connection& con){
  	char buffer[255] = {0};

  	int ret = 0;
  	if ((ret = con.recvData(buffer, 255)) != 0){
  		return ret;
  	}

  	int msg_len = buffer[0];

  	m.ParseFromArray(&buffer[1], msg_len);

  	return 0;
  };

  ~MessageCom();

private:
  MessageCom();
};
