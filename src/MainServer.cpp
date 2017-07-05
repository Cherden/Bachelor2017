#include <iostream>
#include <ratio>
#include <chrono>
#include <thread>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include "Client.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;
//using namespace cv;
using namespace chrono;

volatile bool running = true;
void signalHandler(int signal)
{
	if (signal == SIGINT
	 || signal == SIGTERM
	 || signal == SIGQUIT)
	{
		running = false;
	}
}

Client* clients[MAX_CLIENTS] = {0};

void acceptClient(int* size){
	int new_socket = 0;
	struct sockaddr_in client_info = {};
	int max = *size;
	*size = 0;

	Connection con;
	con.createConnection(SERVER, CONNECTION_PORT, "");
	con.setNonBlocking();

	/*
		At the moment it tries to accept three clients and die. Should keep
		running to accept clients again after they died.
	*/
	while (running){
		if (*size < max){
			new_socket = con.acceptConnection(&client_info);
			if (new_socket >= 0){
				cout << '\r' << "Accepted client " << *size << ".." << endl;
				clients[*size] = new Client(new_socket);
				clients[*size]->setInfo(&client_info);
				(*size)++;
			}
		}

		usleep(50000); //sleep for 50000 to give enough ersources to main thread
	}

	con.closeConnection();
}

int main(void){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	cout << "Start accepting thread.." << endl;

	int amount_clients = MAX_CLIENTS;
	thread accept_clients(acceptClient, &amount_clients);

	high_resolution_clock::time_point for_fps = high_resolution_clock::now();
	duration<double, std::milli> diff_time;
	int frames[MAX_CLIENTS] = {0};


	cout << "Waiting for clients.." << endl;
	while (running){
		for (int i = 0; i < amount_clients; i++){
			if (clients[i] == NULL){
				continue;
			} else if (!clients[i]->isActive()){
				cout << "\rClient " << i << " disconnected.." << endl;
				delete clients[i];
				clients[i] = NULL;
				continue;
			}

			char* video;
			char* depth;

			if (clients[i]->getData(&video, &depth)){
				continue;
			}

			//do stuff

			free(video);
			free(depth);

			frames[i]++;
		}

		diff_time = high_resolution_clock::now() - for_fps;
		if (diff_time.count() >= 1000){
			int counted_clients = 0;
			int sum = 0;

			for(int i = 0; i < MAX_CLIENTS; i++){
				if (clients[i] != NULL && clients[i]->isActive()){
					counted_clients++;
					sum += frames[i];
				}
				frames[i] = 0;
			}

			if (counted_clients > 0){
				cout << "\rRunning at " << sum/counted_clients << " FPS"
					<< flush;
			}

			for_fps = high_resolution_clock::now();
		}
	}

	accept_clients.join();

	for (int i = 0; i < amount_clients; i++){
		if (clients[i] != NULL){
			delete clients[i];
		}
	}

	return 0;
}
