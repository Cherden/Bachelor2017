#include "KinectWrapper.h"

extern "C"{
#include "libfreenect/libfreenect.h"
#include "libfreenect/libfreenect_sync.h"
}


KinectWrapper::KinectWrapper(){}

KinectWrapper::~KinectWrapper(){
    freenect_sync_stop();
}

KinectWrapper KinectWrapper::getInstance(){
	static KinectWrapper kw;
	return kw;
}

int KinectWrapper::getData(FrameInfo info, char** data){
    int ret = 0;
	uint32_t timestamp = 0;

    switch(info){
        case VIDEO:
            ret = freenect_sync_get_video_with_res((void **) data, &timestamp
				, 0, FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB);
            break;

        case DEPTH:
        	ret = freenect_sync_get_depth_with_res((void **) data, &timestamp
				, 0, FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED);
            break;
    }

	return ret;
}
