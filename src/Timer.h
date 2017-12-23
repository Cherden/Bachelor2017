#ifndef _TIMER_H_
#define _TIMER_H_

#include <iostream>
#include <fstream>
#include <unistd.h>

#include "Common.h"

using namespace std;

class Timer{
public:
	Timer(ofstream* file)
  : _file(file){
		_ts = Common::getTime(NULL);
  };

	~Timer(){
	 	double new_ts = Common::getTime(NULL);

    (*_file) << new_ts - _ts << endl;
	};

private:
	ofstream* _file;
  double _ts;
};

#endif
