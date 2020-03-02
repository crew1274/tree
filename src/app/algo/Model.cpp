/*
 * Model.cpp
 *
 *  Created on: 2018年7月11日
 *      Author: 171104
 */

#include <app/algo/Model.h>

Model::Model() {
	// TODO Auto-generated constructor stub

}

Model::~Model() {
	// TODO Auto-generated destructor stub
}

bool Model::STD_Filter(float a[], int a_size, float threshold, int n_segs)
{
	bool sel = true;
	int n_interval = a_size / n_segs;

	for(int seg = 0; seg < n_segs; seg++)
	{
		// For each segment
		float sum = 0;
		for(int j = 0; j < n_interval; j++)
			sum += a[seg*n_interval + j];
		float mean = sum / n_interval;

		float ss = 0 ;
		for(int j = 0; j < n_interval; j++)
			ss += pow(a[seg*n_interval + j] - mean, 2);
		float sd = sqrt(ss/n_interval);

		if(sd < threshold){
			sel = false;
			break;
		}
	}
	return sel;
}

bool Model::STD_Filter_V2(std::vector<float> a, float threshold, int n_segs)
{
	int n_interval = a.size() / n_segs;

	for(int seg = 0; seg < n_segs; seg++)
	{
		// For each segment
		float sum = 0;
		for(int j = 0; j < n_interval; j++)
		{
			sum += a[seg*n_interval + j];
		}
		float mean = sum / n_interval;

		float ss = 0;

		for(int j = 0; j < n_interval; j++)
		{
			ss += pow(a[seg*n_interval + j] - mean, 2);
		}

		float sd = sqrt(ss/n_interval);

		if(sd < threshold)
		{
			return false;
		}
	}
	return true;
}

bool Model::STD_Filter_V2(std::vector<float> arr, float threshold)
{
	float mean = std::accumulate(arr.begin(), arr.end(), 0.0) / arr.size();

	float ss = 0;

	for(uint j = 0; j<arr.size(); j++)
	{
		ss += pow(arr[j] - mean, 2);
	}
	if(sqrt(ss) < threshold)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int* Model::Histogram(float a[], int a_size, int bins, float range[2])
{
	int* out = new int[bins]; //dynamic memory array!

	// Initialize Empty Bins
	for(int i=0; i<bins; i++)
		out[i] = 0;

	// Count Values
	float interval = ( range[1] - range[0] ) / bins;

	for(int i=0; i<a_size; i++){
		for(int j=0; j<bins; j++){

			if(j != bins-1){
				if(a[i] >= range[0] + j*interval && a[i] < range[0] + (j+1)*interval)
					out[j] += 1;
			}
			else{
				if(a[i] >= range[0] + j*interval && a[i] <= range[0] + (j+1)*interval)
					out[j] += 1;
			}
		}
	}
	return out;
}

vector<float> Model::Histogram_V2(vector<float> a, int bins, float range_down, float range_up)
{
	//int* out = new int[bins]; //dynamic memory array!
	vector<float> out;

	// Initialize Empty Bins
	for(int k=0; k<bins; k++)
	{
		out.push_back(0);
	}
	//std::cout << "Initialize Empty Bins" << bins << range_down << range_up << std::endl;
	// Count Values
	float interval = ( range_up - range_down ) / bins;

	for(int i=0; i<a.size(); i++)
	{
		for(int j=0; j<bins; j++)
		{

			if(j != bins-1)
			{
				if(a[i] >= range_down + j*interval && a[i] < range_down + (j+1)*interval)
				{
					out[j] += 1;
				}
			}
			else
			{
				if(a[i] >= range_down + j*interval && a[i] <= range_down + (j+1)*interval)
				{
					out[j] += 1;
				}
			}
		}
	}
	//std::cout << "Count Values" << std::endl;
	return out;
}
/*
float* Model::Histogram_V3(float_2046 a, int bins, float range_down, float range_up)
{
	float* out = new float[bins]; //dynamic memory array!

	// Initialize Empty Bins
	for(int i=0; i<bins; i++)
	{
		out[i] = 0;
	}

	// Count Values
	float interval = ( range_up - range_down ) / bins;

	for(int i=0; i<2046; i++)
	{
		for(int j=0; j<bins; j++)
		{
			if(j != bins-1)
			{
				if(a[i] >= range_down + j*interval && a[i] < range_down + (j+1)*interval)
				{
					out[j] += 1;
				}
			}
			else
			{
				if(a[i] >= range_down + j*interval && a[i] <= range_down + (j+1)*interval)
				{
					out[j] += 1;
				}
			}
		}
	}

	return out;
}
*/
