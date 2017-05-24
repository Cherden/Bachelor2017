#ifndef _KINECT_WRAPPER_H_
#define _KINECT_WRAPPER_H_

#include <stdint.h>

typedef enum{
    DEPTH,
    VIDEO,
	BOTH
} FrameInfo;

class KinectWrapper{
public:
	static KinectWrapper getInstance();
    int getData(FrameInfo info, char* output_buffer);
	~KinectWrapper();
	static int getVideoFrameSize();
	static int getDepthFrameSize();
	static int getBufferSizeForBothFrames();

private:
	KinectWrapper();

};

#endif
