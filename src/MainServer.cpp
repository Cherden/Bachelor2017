#include <iostream>
#include <ratio>
#include <chrono>
#include <thread>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "Client.h"
#include "Connection.h"
#include "Logger.h"


#define LOG_LEVEL DEBUG


using namespace std;
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

void acceptClient(int* amount_clients){
	int new_socket = 0;
	struct sockaddr_in client_info = {};

	Connection con;
	con.createConnection(SERVER, CONNECTION_PORT, "");
	con.setNonBlocking();

	/*
		At the moment it tries to accept three clients and die. Should keep
		running to accept clients again after they died.
	*/
	while (running){
		if (*amount_clients < MAX_CLIENTS){
			new_socket = con.acceptConnection(&client_info);
			if (new_socket >= 0){
				int pos = 0;

				for (; pos < MAX_CLIENTS; pos++){
					if (clients[pos] == NULL){
						break;
					}
				}

				cout << '\r' << "Accepted client " << pos << ".." << endl;
				clients[pos] = new Client(new_socket);
				clients[pos]->setInfo(&client_info);
				(*amount_clients)++;
			}
		}

<<<<<<< HEAD
	/* Create opencv matrix for video frame */
	video_data = (char*) malloc(VIDEO_FRAME_MAX_SIZE);
	memcpy(video_data, frame.video_data().c_str(), VIDEO_FRAME_MAX_SIZE);

	video_frame = new Mat(Size(640, 480), CV_8UC3, video_data);
	cvtColor(*video_frame, *video_frame, CV_RGB2BGR);


	/* Create opencv matrix for depth frame */
	depth_data = (char*) malloc(DEPTH_FRAME_MAX_SIZE);
	memcpy(depth_data, frame.depth_data().c_str(), DEPTH_FRAME_MAX_SIZE);

	depth_frame = new Mat(Size(640, 480), CV_16UC1, depth_data);
	depth_frame->convertTo(*depth_frame, CV_8UC1, 255.0/2048.0);

	free(buf);
=======
		usleep(50000); //sleep for 0.5s to give main thread time
	}
>>>>>>> dev2

	con.closeConnection();
}

int main(void){
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGQUIT, signalHandler);

	cout << "Start accepting thread.." << endl;

	int amount_clients = 0;
	thread accept_clients(acceptClient, &amount_clients);

	high_resolution_clock::time_point for_fps = high_resolution_clock::now();
	duration<double, std::milli> diff_time;
	int frames[MAX_CLIENTS] = {0};


<<<<<<< HEAD
	namedWindow("rgb", CV_WINDOW_AUTOSIZE );
	namedWindow("depth", CV_WINDOW_AUTOSIZE );
	while(running){
		if (client.isClosed()){
			break;
		}
=======
	cout << "Waiting for clients.." << endl;
	while (running){
		for (int i = 0; i < MAX_CLIENTS; i++){
			if (clients[i] == NULL){
				continue;
			} else if (!clients[i]->isActive()){
				cout << "\rClient " << i << " disconnected.." << endl;
				delete clients[i];
				clients[i] = NULL;
				amount_clients--;
				continue;
			}
>>>>>>> dev2

			char* video;
			char* depth;

			if (clients[i]->getData(&video, &depth)){
				continue;
			}

			//do stuff

<<<<<<< HEAD
		if (timestamp < 0) {
			LOG_DEBUG << "not showing frame" << endl;
		} else {
			LOG_DEBUG << "try to show frame" << endl;

			imshow("rgb", *video_frame);
			imshow("depth", *depth_frame);
			cvWaitKey(10);
=======
			free(video);
			free(depth);
>>>>>>> dev2

			frames[i]++;
		}

<<<<<<< HEAD
			free(video_data);
			delete video_frame;
			free(depth_data);
			delete depth_frame;
=======
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
>>>>>>> dev2
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
