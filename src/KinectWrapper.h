#ifndef _KINECT_WRAPPER_H_
#define _KINECT_WRAPPER_H_

#include <stdint.h>


typedef enum{
    DEPTH,
    VIDEO
} FrameInfo;

/**
	The size of data for one video frame
*/
#define VIDEO_FRAME_MAX_SIZE 307200*3 	//(640*480)*3

/**
	The size of data for one depth frame
*/
#define DEPTH_FRAME_MAX_SIZE 422400 	//640*480*1.375

class KinectWrapper{
public:
	/**
		Meyers' singleton pattern.
		@return A static reference to the only class instance.
	*/
	static KinectWrapper getInstance();

	/**
		Get a depth or video frame from a Microsoft Kinect using the C-Sync
		Wrapper of the OpenKinect driver.

		@param info The type of frame to get. Either VIDEO or DEPTH.
		@param data Pointer to an uninitialized pointer where the frame data
		will be written.
		@return The return value of freenect_sync_get_{video|depth}_with_res().
	*/
    int getData(FrameInfo info, char** data);

	~KinectWrapper();

private:
	KinectWrapper();

};

#endif
