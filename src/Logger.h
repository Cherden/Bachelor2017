#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <fstream>
#include <string>

using namespace std;

#define LOGGING_ENABLED

#define CPP_STRING(x) string(x)

#define LOG_HEAD "[" << Logger::getLogger().getLogLevelString(x) << "] " << clock() << " : "

#ifdef LOGGING_ENABLED
#define LOG(x, y) (*Logger::getLogger().getStream()) << y
#else
#define LOG(x, y) 
#endif
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
	static Logger getLogger();
	void log(LogLevel l, string msg);
	void setLogLevel(LogLevel l);
	ofstream* getStream();
	~Logger();

private:
	Logger();
	string getLogLevelString(LogLevel l);

private:
	LogLevel _level;
	ofstream *_file;

};

#endif
