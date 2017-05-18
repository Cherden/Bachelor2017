#include "Sensor.h"
#include "KinectWrapper.h"

int Sensor::getData(char* output_buffer, uint32_t* timestamp){
	KinectWrapper.getInstance().getFrame();
}
