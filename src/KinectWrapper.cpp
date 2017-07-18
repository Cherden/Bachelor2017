#include "KinectWrapper.h"



KinectWrapper::KinectWrapper(){}

KinectWrapper::~KinectWrapper(){
    freenect_sync_stop();
}

KinectWrapper KinectWrapper::getInstance(){
	static KinectWrapper kw;
	return kw;
}

void KinectWrapper::convertToXYZPointCloud(float* cloud, uint16_t* depth){
	int step = 3;
	float center_x = (float) (DEPTH_FRAME_WIDTH >> 1);
	float center_y = (float) (DEPTH_FRAME_HEIGHT >> 1);
	float f_inv = 1.0f / 517.4f // Inverse from 640 / (2 * tan(58.5 / 2)) = 517.4
	//float f_inv = 1.0f / 525 //found here http://www.pcl-users.org/Getting-strange-results-when-moving-from-depth-map-to-point-cloud-td4025104.html#a4025138

	int depth_idx = 0; //index for depth data array
	int cloud_idx = 0; //index for pointcloud
	for (int h = 0; h < DEPTH_FRAME_HEIGHT; h++){
		for (int w = 0; w < DEPTH_FRAME_WIDTH; w++, cloud_idx += step){
			cloud[cloud_idx] = depth[depth_idx++] * 0.001f;

			float tmp = cloud[cloud_idx] * f_inv;
			cloud[cloud_idx+1] = (w - center_x) * tmp;
			cloud[cloud_idx+2] = (h - center_y) * tmp;
		}
	}
}

void KinectWrapper::setLed(LedOption op){
	freenect_sync_set_led((freenect_led_options) op, 0);
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
