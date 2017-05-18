#include "KinectWrapper.h"

extern "C"{
	#include "libfreenect/libfreenect.h"
    #include "libfreenect/libfreenect_sync.h"
}

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

int KinectWrapper::getFrame(FrameInfo info, char* frame_return){
    int ret = 0;
	uint32_t* timestamp

    switch(info){
        case RGB:
            ret = freenect_sync_get_video_with_res((void **) &frame_return, timestamp, 0,
        	           FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB);
            break;

        case DEPTH:
        	// Pull a depth frame registered to the above image
        	ret = freenect_sync_get_depth_with_res((void **) &frame_return, timestamp, 0,
        				FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED);
            break;
    }

	return ret;
}

int KinectWrapper::getData(char* output_buffer){
	int ret = 0;

	ret = getFrame(RGB, output_buffer);
	ret |= getFrame(DEPTH, output_buffer + VIDEO_FRAME_MAX_SIZE);

	return ret;
}

int KinectWrapper::getVideoFrameSize(){
	return VIDEO_FRAME_MAX_SIZE;
}

int KinectWrapper::getVideoFrameSize(){
	return DEPTH_FRAME_MAX_SIZE;
}
