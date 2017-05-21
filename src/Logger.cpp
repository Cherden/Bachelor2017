#include "Logger.h"

Logger::Logger()
 : _level(DEBUG)
 , _file(0) {
	_file.open("log.txt");
}

Logger::~Logger(){
	_file.close();
}

void Logger::log(LogLevel l, char* msg){
	if (l < _level) {
		return;
	}
	
	_file << "[" << getLogLevelString(l) << "] : " << msg << endl;
}

string Logger::getLogLevelString(LogLevel l){
	switch (l){
		case DEBUG : return "DEBUG";
		case WARNING : return "WARNING";
		case ERROR : return "ERROR";
	}
}

void Logger::setLogLevel(LogLevel l){
	_level = l;
}
