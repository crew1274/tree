/*
 * Model.h
 *
 *  Created on: 2018年7月11日
 *      Author: 171104
 */

#ifndef SRC_APP_MODEL_H_
#define SRC_APP_MODEL_H_

#include <vector>
#include <cmath>
#include <numeric>
#include <cstdio>
#include <cstdlib>

using namespace std;

class Model {
public:
	Model();
	~Model();

	bool STD_Filter(float a[], int a_size, float threshold, int n_segs);
	static bool STD_Filter_V2(std::vector<float> a, float threshold, int n_segs);
	static bool STD_Filter_V2(std::vector<float> arr, float threshold);
	int* Histogram(float a[], int a_size, int bins, float range[2]);
	static std::vector<float> Histogram_V2(vector<float> a, int bins, float range_down, float range_up);
	//float* Histogram_V3(float_2046 a, int bins, float range_down, float range_up);
};


#endif /* SRC_APP_MODEL_H_ */
