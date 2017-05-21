#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <fstream>
#include <string>

using namespace std;

#define LOG(x, y) Logger::getInstance().log(x, y)
#define LOG_DEBUG(x) LOG(DEBUG, x)
#define LOG_WARNING(x) LOG(WARNING, x)
#define LOG_ERROR(x) LOG(ERROR, x)

typedef enum {
	DEBUG,
	WARNING,
	ERROR
} LogLevel;

class Logger{
public:
	Logger getLogger();
	void log(LogLevel l, char* msg);
	void setLogLevel(LogLevel l);
	~Logger();
	
private:
	Logger();
	string getLogLevelString(LogLevel l);
	
private:
	LogLevel _level;
	ofstream _file;
	
};

#endif
