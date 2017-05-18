#include <stdint.h>

class Sensor{

public:
	/*
	*	Read data from the sensor
	*	output_buffer OUT: Pointer to a buffer to write the return value. Length has to be SENSOR_MAX_BYTE_OUTPUT
	*	timestamp OUT: Contains the time at which the data got taken
	*	returns: The amount of bytes in output_buffer on succes, -1 on failure.
	*/
    int getData(char* output_buffer, uint32_t* timestamp);
	~Sensor();

private:
	Sensor();
	const int SENSOR_MAX_BYTE_OUTPUT = 424688;

};
