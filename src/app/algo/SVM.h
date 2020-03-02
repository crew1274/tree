/*
 * SVM.h
 *
 *  Created on: 2018年7月13日
 *      Author: 171104
 */

#ifndef SRC_APP_SVM_H_
#define SRC_APP_SVM_H_

#include "Model.h"

using namespace std;

class SVM: public Model {
public:
	SVM();
	~SVM();

	float L2_norm_v2(vector<float> x1, vector<float> x2);
	float rbf_kernel_v2(vector<float> x1, vector<float> x2, float gamma);
	float decision_function_V2(
				vector<float> x_test,
				vector<vector<float> > sv,
				vector<float> ALPHA,
				float gamma,
				float INTERCEPT,
				float threshold);
};

#endif /* SRC_APP_SVM_H_ */
