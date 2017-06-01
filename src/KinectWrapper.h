#ifndef _KINECT_WRAPPER_H_
#define _KINECT_WRAPPER_H_

#include <stdint.h>

typedef enum{
    DEPTH,
    VIDEO
} FrameInfo;

#define VIDEO_FRAME_MAX_SIZE 307200*3 //640*480
#define DEPTH_FRAME_MAX_SIZE 422400 //640*480*1.375

class KinectWrapper{
public:
	static KinectWrapper getInstance();
    int getData(FrameInfo info, char* data);
	~KinectWrapper();

private:
	KinectWrapper();

};

#endif
