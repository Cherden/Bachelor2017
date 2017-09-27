#include <iostream>
#include <ratio>
#include <chrono>
#include <thread>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "Common.h"

#include "Connection.h"
#include "UDPConnection.h"
#include "TCPConnection.h"

#include "Client.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG
//#define SHOW_IMAGE


using namespace std;
using namespace chrono;
using namespace cv;

int Connection::next_port = CONNECTION_PORT;

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

void acceptClient(int* amount_clients){
	int tcp_socket = 0;
	struct sockaddr_in client_info = {};

	TCPConnection con;
	if (con.createConnection(SERVER, CONNECTION_PORT, "") < 0){
		LOG_ERROR << "Failed to connect tcp socket" << endl;
		running = false;
		return;
	}
	con.setNonBlocking();

	/*
		At the moment it tries to accept three clients and die. Should keep
		running to accept clients again after they died.
	*/
	while (running){
		if (*amount_clients < MAX_CLIENTS){
			tcp_socket = con.acceptConnection(&client_info);
			if (tcp_socket >= 0){
				int pos = 0;

				for (; pos < MAX_CLIENTS; pos++){
					if (clients[pos] == NULL){
						break;
					}
				}

				cout << '\r' << "Client " << pos << " connected" << endl;
				clients[pos] = new Client(tcp_socket, ++(UDPConnection::next_port));
				clients[pos]->setInfo(&client_info);

				(*amount_clients)++;
			}
		}

		usleep(50000); //sleep for 0.5s to give main thread time
	}

	con.closeConnection();
}

int main(){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	cout << "Start accepting thread ..." << endl;

	int amount_clients = 0;
	thread accept_clients(acceptClient, &amount_clients);

	high_resolution_clock::time_point for_fps = high_resolution_clock::now();
	duration<double, std::milli> diff_time;
	int frames[MAX_CLIENTS] = {0};

	cout << "Waiting for clients ..." << endl;
	while (running){
		for (int i = 0; i < MAX_CLIENTS; i++){
			if (clients[i] == NULL){
				continue;
			} else if (!clients[i]->isActive()){
				cout << "\rClient " << i << " disconnected" << endl;
				delete clients[i];
				clients[i] = NULL;
				amount_clients--;
				continue;
			}

			char* video = NULL;
			char* depth = NULL;
			float* cloud = NULL;

			if (clients[i]->getData(&video, &depth, &cloud)){
				continue;
			}

#ifdef SHOW_IMAGE
			Mat video_mat(Size(640, 480), CV_8UC3, video);
			Mat depth_mat(Size(640, 480), CV_16UC1, depth);

			cvtColor(video_mat, video_mat, CV_RGB2BGR);
			depth_mat.convertTo(depth_mat, CV_8UC1, 255.0/2048.0);

			imshow("rgb " + to_string(i), video_mat);
			moveWindow("rgb " + to_string(i), i*640, 0);
 			imshow("depth " + to_string(i), depth_mat);
			moveWindow("depth " + to_string(i), i*640, 500);
 			cvWaitKey(1);
#endif

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

	for (int i = 0; i < MAX_CLIENTS; i++){
		if (clients[i] != NULL){
			delete clients[i];
		}
	}

	return 0;
}
