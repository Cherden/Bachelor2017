#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <chrono>

using namespace std;
using namespace chrono;


volatile bool running = true;
volatile int us = 0;
volatile int ms = 0;
volatile int sec = 0;

void signalHandler(int signal){
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT){
		running = false;
	}
}

int main(){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);



	return 0;
}
