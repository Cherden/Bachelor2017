#ifndef _TIMER_H_
#define _TIMER_H_


#include "Sync.h"

using namespace std;

class Timer{
public:
	Timer(ofstream* file)
  : _file(file){

  	uint64_t timestamp_arr[2] = {0};
    Sync::getTime(timestamp_arr);
		_ts = timestamp_arr[0] * 100 + timestamp_arr[1] / 100000;
  };

	~Timer(){
    uint64_t timestamp_arr[2] = {0};
    Sync::getTime(timestamp_arr);
    uint64_t new_ts = timestamp_arr[0] * 100 + timestamp_arr[1] / 100000;

    *_file << new_ts - _ts << endl;
  };

private:
	ofstream* _file;
  uint64_t _ts;
};

#endif
