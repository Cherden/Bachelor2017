#ifndef _PCLUTIL_H_
#define _PCLUTIL_H_

#include "../gen/KinectFrameMessage.pb.h"

/**

These utility functions are based on the Point Cloud Library (PCL)
Thei function can be found here https://github.com/PointCloudLibrary/pcl/blob/46cb8fe5589e88e36d79f9b8b8e5f4ff4fceb5de/io/src/openni_grabber.cpp#L532

*/

class PCLUtil{
public:
	static void convertToXYZPointCloud(KinectFrameMessage& message, uint16_t* depth, int height, int width){
		int step = 3;
		float center_x = (float) (width >> 1);
		float center_y = (float) (height >> 1);
		float f_inv = 1.0f / 517.4f; // Inverse from 640 / (2 * tan(58.5 / 2)) = 517.4
		//float f_inv = 1.0f / 525; //found here http://www.pcl-users.org/Getting-strange-results-when-moving-from-depth-map-to-point-cloud-td4025104.html#a4025138

		int depth_idx = 0; //index for depth data array
		int cloud_idx = 0; //index for pointcloud

		message.clear_cloud();
		for (int h = 0; h < height; h++){
			for (int w = 0; w < width; w++, cloud_idx += step){
				float tmp = depth[depth_idx++] * 0.001f;
				message.add_cloud(tmp);

				tmp = tmp * f_inv;
				message.add_cloud((w - center_x) * tmp);
				message.add_cloud((h - center_y) * tmp);
			}
		}
	};

	static void convertToXYZPointCloud(float* cloud, const char* depth, int height, int width){
		int step = 3;
		float center_x = (float) (width >> 1);
		float center_y = (float) (height >> 1);
		float f_inv = 1.0f / 517.4f; // Inverse from 640 / (2 * tan(58.5 / 2)) = 517.4
		//float f_inv = 1.0f / 525; //found here http://www.pcl-users.org/Getting-strange-results-when-moving-from-depth-map-to-point-cloud-td4025104.html#a4025138

		int depth_idx = 0; //index for depth data array
		int cloud_idx = 0; //index for pointcloud

		for (int h = 0; h < height; h++){
			for (int w = 0; w < width; w++, cloud_idx += step){
				uint16_t depth_value = (depth[depth_idx]  << 8) | depth[depth_idx + 1];
				float tmp = depth_value * 0.001f;
				cloud[cloud_idx] = tmp;

				tmp = tmp * f_inv;
				cloud[cloud_idx + 1] = (w - center_x) * tmp;
				cloud[cloud_idx + 2] = (h - center_y) * tmp;

				depth_idx += 2;
			}
		}
	};

	~PCLUtil();

private:
	PCLUtil();
};

#endif
