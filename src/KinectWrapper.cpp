#include "KinectWrapper.h"

extern "C"{
#include "libfreenect/libfreenect.h"
#include "libfreenect/libfreenect_sync.h"
}

#include <opencv/cv.h>
#include <opencv/highgui.h>


#define VIDEO_FRAME_MAX_SIZE 640*480
#define DEPTH_FRAME_MAX_SIZE 640*480


KinectWrapper::KinectWrapper(){}

KinectWrapper::~KinectWrapper(){
    freenect_sync_stop();
}

KinectWrapper KinectWrapper::getInstance(){
	static KinectWrapper kw;
	return kw;
}

int KinectWrapper::getData(FrameInfo info, IplImage* image){
    int ret = 0;
	uint32_t timestamp = 0;
	char* data;

    switch(info){
        case VIDEO:
            ret = freenect_sync_get_video_with_res((void **) &data, &timestamp, 0,
        	           FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB);

		   	cvSetData(image, data, 640*3);
   			cvCvtColor(image, image, CV_RGB2BGR);
            break;

        case DEPTH:
        	ret = freenect_sync_get_depth_with_res((void **) &data, &timestamp, 0,
        				FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED);
						
			cvSetData(image, data, 640);
            break;
    }

	return ret;
}

int KinectWrapper::getVideoFrameSize(){
	return VIDEO_FRAME_MAX_SIZE;
}

int KinectWrapper::getDepthFrameSize(){
	return DEPTH_FRAME_MAX_SIZE;
}

int KinectWrapper::getBufferSizeForBothFrames(){
	return DEPTH_FRAME_MAX_SIZE + VIDEO_FRAME_MAX_SIZE;
}
