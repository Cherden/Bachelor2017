// Be sure to link with -lfreenect_sync
#include <stdlib.h>
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "libfreenect/libfreenect_sync.h"
int main()
{
	//IplImage *image = cvCreateImageHeader(cvSize(640, 480), 8, 3);
	IplImage *image = cvCreateImageHeader(cvSize(640, 480), 8, 1);

	char *data;
	unsigned int timestamp;
	//freenect_sync_get_video((void**)(&data), &timestamp, 0, FREENECT_VIDEO_RGB);
	freenect_sync_get_depth((void**)(&data), &timestamp, 0, FREENECT_DEPTH_REGISTERED);
	cvSetData(image, data, 640);
	//cvCvtColor(image, image, CV_RGB2BGR);
	cvSaveImage("foo.png",image);

	freenect_sync_stop();
	cvFree(&image);
	return EXIT_SUCCESS;
}
