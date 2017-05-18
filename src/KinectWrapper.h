#ifndef _KINECT_WRAPPER_H_
#define _KINECT_WRAPPER_H_


#include <stdint.h>

typedef enum frame_info{
    DEPTH,
    RGB
} FrameInfo;

class KinectWrapper{

public:
	static KinectWrapper getInstance();
    int getFrame(FrameInfo info, char* frame_return);
	int getData(char* output_buffer);
	~KinectWrapper();
	int getVideoFrameSize();
	int getDepthFrameSize();

private:
	KinectWrapper();

};
#endif
