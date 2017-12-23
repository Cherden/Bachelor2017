#ifndef __COMMON_H__
#define __COMMON_H__

#include <unistd.h>
#include <sys/time.h>

/**
	Port used for communication from this program
*/
#define CONNECTION_PORT 79421

#define SERVER_IP "192.168.1.2"
//#define BROADCAST_IP "255.255.255.255"
#define BROADCAST_IP "192.168.1.255"

//#define PROCESS_CLOUD_DISTRIBUTED

/**
	Amount of clients the server can accept
*/
#define MAX_CLIENTS 3

class Common{
public:
  static double getTime(uint64_t* t){
    struct timeval tv;

    gettimeofday(&tv, NULL);

    if (t != NULL){
      t[0] = (uint64_t) tv.tv_sec;
      t[1] = (uint64_t) tv.tv_usec;
    }

    return ((double) tv.tv_sec * 1000000 + (double) tv.tv_usec) / 1000.0;
  }
};

#endif
