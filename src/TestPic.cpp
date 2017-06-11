// Be sure to link with -lfreenect_sync
#include <stdlib.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "libfreenect/libfreenect_sync.h"

#include "../gen/KinectFrameMessage.pb.h"
#include "KinectWrapper.h"

#define VIDEO_FRAME_MAX_SIZE 307200*3

using namespace cv;

void fillFrameMessage(KinectFrameMessage& kfm){
	char *data;
	unsigned int timestamp;
	KinectWrapper kw = KinectWrapper::getInstance();

	kw.getData(VIDEO, &data);
	//freenect_sync_get_video((void**)(&data), &timestamp, 0, FREENECT_VIDEO_RGB);
	kfm.set_video_data((void*) data, VIDEO_FRAME_MAX_SIZE);
	kfm.set_timestamp(timestamp);

	//free(data);
}

int main(){
	Mat* frame = 0;
	char* video_data;
	KinectFrameMessage kfm;
	KinectFrameMessage kfm_after;
	string s = "";

	fillFrameMessage(kfm);

	kfm.SerializeToString(&s);

	kfm_after.ParseFromString(s);

	video_data = (char*) malloc(VIDEO_FRAME_MAX_SIZE);
	memcpy(video_data, kfm_after.video_data().c_str(), VIDEO_FRAME_MAX_SIZE);

	frame = new Mat(Size(640, 480), CV_8UC3, video_data);
	cvtColor(*frame, *frame, CV_RGB2BGR);
	imwrite("foo.png", *frame);

	freenect_sync_stop();
	free(video_data);
	delete frame;
	return EXIT_SUCCESS;
}
