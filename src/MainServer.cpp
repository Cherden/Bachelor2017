#include <iostream>
#include <thread>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Client.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;
using namespace cv;

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

	while(*size < max || running){
		new_socket = con.acceptConnection(NULL);
		if (new_socket >= 0){
			clients[*size] = new Client(new_socket);
			//clients[*size]->setInfo(&client_info);
			(*size)++;
		}
	}

	con.closeConnection();
}

int main(void){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	int amount_clients = MAX_CLIENTS;
	thread accept_clients(acceptClient, &amount_clients);


	while (running){
		for (int i = 0; i < amount_clients; i++){
			if (clients[i] == NULL){
				continue;
			} else if (!clients[i]->isActive()){
				delete clients[i];
				clients[i] = NULL;
				continue;
			}

			Mat x, y;
			if (clients[i]->lockData() < 0){
				//LOG_WARNING << "Client " << i << ": failed to lock data" << endl;
				continue;
			}

			if (clients[i]->getData(&x, &y)){
				LOG_WARNING << "Client " << i << ": failed to get data" << endl;
			}

			clients[i]->releaseData();

			LOG_DEBUG << "Client " << i << ": processed data" << endl;
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
