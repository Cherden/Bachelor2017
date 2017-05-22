#include "Logger.h"

#include <time.h>

Logger::Logger()
 : _level(DEBUG)
 , _file(0) {
	_file->open("log.txt");
}

Logger::~Logger(){
	_file->close();
}

Logger Logger::getLogger(){
	static Logger logger;
	return logger;
}

void Logger::log(LogLevel l, string msg){
	if (l < _level) {
		return;
	}

	(*_file) << "[" << getLogLevelString(l) << "] " << clock() << " : " << msg << endl;
}

ofstream* Logger::getStream(){
	return _file;
}

string Logger::getLogLevelString(LogLevel l){
	switch (l){
		case DEBUG : return "DEBUG";
		case WARNING : return "WARNING";
		case ERROR : return "ERROR";
		default : return "UNKNOWN";
	}
}

void Logger::setLogLevel(LogLevel l){
	_level = l;
}
