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
	if (signal == SIGALRM){
		us+=10;
		if (us == 1000){
			us = 0;
			ms++;
		}
		if (ms == 1000){
			ms = 0;
			sec++;
		}
		if (sec == 5){
			ualarm(0, 0);
			running = false;
		}
	}

	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT){
		running = false;
	}
}

int main(){
	signal(SIGALRM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	high_resolution_clock::time_point start_time;
	duration<double, std::milli> diff_time;
	high_resolution_clock::time_point end_time;

	start_time = high_resolution_clock::now();
	ualarm(10, 10);

	while(running){

	}
	end_time = high_resolution_clock::now();
	diff_time = end_time - start_time;

	cout << "after "<< diff_time.count()/1000 <<" seconds: sec=" << sec << " milli=" << ms << " micro=" << us << endl;


	/*double tget_data = 0;
	double tget_data_min = 2147483647;
	double tget_data_max = 0;
	double tget_data_avg = 0;

	high_resolution_clock::time_point start_time;
	duration<double, std::micro> diff_time;
	high_resolution_clock::time_point end_time;

	int t = 1000;

	cout << "Start" << endl;
	start_time = high_resolution_clock::now();

	while (running){
		if (ticks == 1){
			end_time = high_resolution_clock::now();
			diff_time = end_time - start_time;

			tget_data = diff_time.count();
			tget_data_min = tget_data < tget_data_min ? tget_data : tget_data_min;
			tget_data_max = tget_data > tget_data_max ? tget_data : tget_data_max;
			tget_data_avg = (tget_data_avg + tget_data) / 2;
			ticks = 0;

			start_time = high_resolution_clock::now();
			ualarm(1000, 0);
		}
	}
	cout << "Time to trigger signal (" << t << " microsekonds): \n"
			<< "\tMIN = " << tget_data_min
			<< "\tMAX = " << tget_data_max
			<< "\tAVG = " << tget_data_avg
			<< "\n\nEnd" << endl;*/


	return 0;
}
