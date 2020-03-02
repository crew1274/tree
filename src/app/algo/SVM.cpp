/*
 * SVM.cpp
 *
 *  Created on: 2018年7月13日
 *      Author: 171104
 */

#include <app/algo/SVM.h>

SVM::SVM() {
	// TODO Auto-generated constructor stub

}

SVM::~SVM() {
	// TODO Auto-generated destructor stub
}

float SVM::decision_function_V2( vector<float> x_test,
								 vector<vector<float> > sv,
								 vector<float> ALPHA,
								 float gamma,
								 float INTERCEPT,
								 float threshold)
{
	float decision_value = 0;
	for(uint i=0; i<sv.size(); i++)
	{
		decision_value += rbf_kernel_v2(x_test, sv[i], gamma) * ALPHA[i];
	}
	decision_value = decision_value + INTERCEPT + threshold;
	return decision_value;
}

float SVM::L2_norm_v2(vector<float> x1, vector<float> x2)
{
     float sum = 0.0;
     for(int i=0; i< x2.size(); i++)
     {
    	 sum += pow(x1[i] - x2[i], 2);
     }
     return sqrt(sum);
}

float SVM::rbf_kernel_v2(vector<float> x1, vector<float> x2, float gamma)
{
	return exp((-gamma) * pow(L2_norm_v2(x1, x2), 2));
}

