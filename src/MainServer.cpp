#include <iostream>
#include <ratio>
#include <chrono>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <algorithm>    // std::min max

#include "Timer.h"
#include "Common.h"

#include "ServerAPI.h"
#include "Client.h"


#define LOG_LEVEL ERROR
//#define SHOW_IMAGE
#define SHOW_FPS


using namespace std;
using namespace cv;


int main(){
	ServerAPI api;

	char* video = NULL;
	char* depth = NULL;
	float* cloud = NULL;

	uint64_t timestamp_arr[3] = {0};

	int video_size = 0;
	int depth_size = 0;
	int cloud_size = 0;

	uint64_t t[2] = {0};
	uint64_t t_ref[2] = {0};
	int frames = 0;
	Common::getTime(t_ref);

	cout << "Waiting for clients ..." << endl;
	while(!api.allClientsConnected()){};
	cout << "All connected!" << endl;


	ofstream f;
	f.open("../all.txt");

	while (true){
		api.obtainNewData();

		while(!api.isAbleToDeliverData());

		for(int i = 0; i < MAX_CLIENTS; i++){

			{
//				Timer t(&f);

				video_size = api.getVideo(i, &video, video_size);
				depth_size = api.getDepth(i, &depth, depth_size);

				timestamp_arr[i] = api.getTimestamp(i);

				//cloud_size = api.getCloud(i, &cloud, cloud_size);
			}

#ifdef SHOW_IMAGE
			Mat video_mat(Size(640, 480), CV_8UC3, video);
			//Mat depth_mat(Size(640, 480), CV_16UC1, depth);

			cvtColor(video_mat, video_mat, CV_RGB2BGR);
			//depth_mat.convertTo(depth_mat, CV_8UC1, 255.0/2048.0);

			//imshow("depth " + to_string(i), depth_mat);
			//moveWindow("depth " + to_string(i), i*640, 500);
			imshow("rgb " + to_string(i), video_mat);
			moveWindow("rgb " + to_string(i), i*640, 0);
 			cvWaitKey(1);
#endif
		}

		cout << "timestamp ms: " << timestamp_arr[0]<< " "<<timestamp_arr[1]<<" "<<timestamp_arr[2]<<endl;
		cout << "timestamp difference " << Common::absMinMax(timestamp_arr[0], timestamp_arr[1], timestamp_arr[2])
			<< endl;



#ifdef SHOW_FPS
		frames++;

		Common::getTime(t);
		if (t[0] - t_ref[0] >= 1){
			Common::getTime(t_ref);
			cout << "\r" << frames << " FPS" << flush;
			frames = 0;
		}
#endif
	}

	f.close();

	return 0;
}
