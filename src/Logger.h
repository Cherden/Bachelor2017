#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;

#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED
	#define LOG(x) Logger::getLogger().log(x)
	#define SET_LOG_LEVEL(x) Logger::getLogger().setLogLevel(x)
#else
	#define LOG(x, y)
	#define SET_LOG_LEVEL(x)
#endif

#define LOG_DEBUG LOG(DEBUG)
#define LOG_WARNING if (Logger::getLogger().getLogLevel() < WARNING); \
                    else LOG(WARNING)
#define LOG_ERROR if (Logger::getLogger().getLogLevel() < WARNING); \
                    else LOG(ERROR)

typedef enum {
	DEBUG,
	WARNING,
	ERROR
} LogLevel;

class Logger{
public:
	static Logger& getLogger(){
		static Logger logger;
		return logger;
	}        
	ofstream& log(LogLevel l){
		_file << "[" << getLogLevelString(l) << "] " << clock() << " : ";
		return _file;
	}        
	void setLogLevel(LogLevel l){
			_level = l;
	}
	LogLevel getLogLevel(){
		return _level;
	}
	~Logger(){
		_file.close();
	}

private:
	Logger()
	 : _level(DEBUG)
	 , _file(0) {
	   _file.open("log.txt");
	}
	string getLogLevelString(LogLevel l){
		switch (l){
			case DEBUG : return "DEBUG";
			case WARNING : return "WARNING";
			case ERROR : return "ERROR";
			default : return "UNKNOWN";
		}
	}

private:
	LogLevel _level;
	ofstream _file;

};

#endif
