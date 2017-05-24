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

int KinectWrapper::getData(FrameInfo info, char* output_buffer){
    int ret = 0;
	uint32_t timestamp = 0;

    switch(info){
        case VIDEO:
            ret = freenect_sync_get_video_with_res((void **) &output_buffer, &timestamp, 0,
        	           FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB);
            break;

        case DEPTH:
        	ret = freenect_sync_get_depth_with_res((void **) &output_buffer, &timestamp, 0,
        				FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED);
            break;
		case BOTH:
			ret = freenect_sync_get_video_with_res((void **) &output_buffer, &timestamp, 0,
        	        FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB);
			ret |= freenect_sync_get_depth_with_res((void **) &output_buffer[VIDEO_FRAME_MAX_SIZE],
					&timestamp, 0, FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED);
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
