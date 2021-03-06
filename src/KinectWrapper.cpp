#include "KinectWrapper.h"

#include <unistd.h>


KinectWrapper::KinectWrapper(){
	freenect_sync_set_tilt_degs(10, 0);
}

KinectWrapper::~KinectWrapper(){
    freenect_sync_stop();
}

KinectWrapper KinectWrapper::getInstance(){
	static KinectWrapper kw;
	return kw;
}

void KinectWrapper::setLed(LedOption op){
	freenect_sync_set_led((freenect_led_options) op, 0);
}

void KinectWrapper::handleUSBHandshake(){
	char* video_image;
	char* depth_image;

	getData(VIDEO, &video_image);
	getData(DEPTH, &depth_image);

	usleep(35000);
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
