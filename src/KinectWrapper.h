#ifndef _KINECT_WRAPPER_H_
#define _KINECT_WRAPPER_H_

#include <stdint.h>
#include <opencv2/opencv.hpp>


typedef enum{
    DEPTH,
    VIDEO
} FrameInfo;

/**
	The size of data for one video frame
*/
#define VIDEO_FRAME_WIDTH 		640
#define VIDEO_FRAME_HEIGHT 		480
#define VIDEO_FRAME_DEPTH 		CV_8UC3
#define VIDEO_FRAME_MAX_SIZE 	VIDEO_FRAME_HEIGHT * VIDEO_FRAME_WIDTH * 3

/**
	The size of data for one depth frame
*/
#define DEPTH_FRAME_WIDTH 		640
#define DEPTH_FRAME_HEIGHT 		480
#define DEPTH_FRAME_DEPTH 		CV_16UC1
#define DEPTH_FRAME_MAX_SIZE 	VIDEO_FRAME_HEIGHT * VIDEO_FRAME_WIDTH * 2

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
