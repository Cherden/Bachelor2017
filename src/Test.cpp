
#include <iostream>

using namespace std;

extern "C"{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
	#include "libfreenect/libfreenect.h"
}

class Wrapper{

public:
	static Wrapper getInstance();
	int init();
	void getPicture();
	void turnLEDOn();
	void destroy();

private:
	Wrapper();

private:
	void _rgbCallback(freenect_device *dev, void *rgb, uint32_t timestamp);
	void _depthCallback(freenect_device *dev, void *rgb, uint32_t timestamp);

private:
	freenect_device* _dev;
	freenect_context* _ctx;
	uint8_t* _rgb_buffer;


};

Wrapper::Wrapper() : _dev(0), _ctx(0), _rgb_buffer(0){}

Wrapper Wrapper::getInstance(){
	static Wrapper w;
	return w;
}

int Wrapper::init(){
    _rgb_buffer = (uint8_t*)malloc(1280*1024*3);

	if (freenect_init(&_ctx, NULL) < 0) {
		printf("freenect_init() failed\n");
		return -1;
	}

    freenect_set_log_level(_ctx, FREENECT_LOG_FATAL);
	freenect_select_subdevices(_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

	if (freenect_open_device(_ctx, &_dev, 0) < 0) {
		printf("Could not open device\n");
		this->destroy();
		return -2;
	}


	/*freenect_set_tilt_degs(_dev, 0);
	freenect_set_led(_dev, LED_RED);
	freenect_set_depth_callback(_dev, _depthCallback);
	freenect_set_video_callback(_dev, _rgbCallback);
	freenect_set_video_mode(_dev, freenect_find_video_mode(FREENECT_RESOLUTION_HIGH, FREENECT_VIDEO_RGB));
	freenect_set_depth_mode(_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_HIGH, FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(_dev, _rgb_buffer);*/

	return 0;
}

void Wrapper::destroy(){
	free(_rgb_buffer);
    freenect_shutdown(_ctx);
}

void Wrapper::turnLEDOn(){
	freenect_set_led(_dev, LED_RED);
	freenect_set_tilt_degs(_dev, 0);
}

void Wrapper::_rgbCallback(freenect_device *dev, void *rgb, uint32_t timestamp)
{}

void Wrapper::_depthCallback(freenect_device *dev, void *rgb, uint32_t timestamp)
{}

int main(int argc, char **argv){
	Wrapper w = Wrapper::getInstance();

    if (w.init() < 0){
        cout << "Failed to initialize Kinect" << endl;
        return 0;
    }
	//w.turnLEDOn();
	//w.destroy();

    return 0;
}
