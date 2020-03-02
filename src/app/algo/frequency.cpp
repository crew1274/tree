#include "frequency.h"

double freqFromFFT(vector<double> data, double sampling_rate) {
	
	const int n = data.size();
	
	double weight[n];
	for (int i = 0; i < n; i++)
	{
		weight[i] = data[i];
	}

	window win;
	win.blackman_harris(weight, n);

	vector<double> w(n);
	for (int i = 0; i < n; i++)w[i] = data[i] * weight[i];

	// Input array
	fftw_complex y[n];
	rfft1d(w, y);

	double a[n/2];
	for (int i = 0; i < n/2; i++)
	{
		a[i] = sqrt(pow(y[i][0], 2) + pow(y[i][1], 2));
	}

	int index = distance(a, max_element(a, a + n/2));
	double index_ = 1 / 2. * (log(a[index - 1]) - log(a[index + 1])) / (log(a[index - 1]) - 2 * log(a[index]) + log(a[index + 1])) + index;
	double freq = sampling_rate * index_ / n;

	return freq;
}

vector<double> dftFreqBand(int n, double d) {
	double val = 1. / (n*d);
	int N = (n - 1) / 2 + 1;
	int N2 = -n / 2;
	vector<double> results(n);
	for (uint i = 0; i < results.size(); i++) {
		if (i < N)
			results[i] = i*val;
		else
			results[i] = (N2 + i - N)*val;
	}
	return results;
}

vector<double> fftAmplitude(vector<double> data) {
	const int n = data.size();
	// Input array
	fftw_complex y[n];
	rfft1d(data, y);

	vector<double> amplitude(n);
	for (int i = 0; i < n; i++)
	{
		amplitude[i] = sqrt(pow(y[i][0], 2) + pow(y[i][1], 2));
	}
	return amplitude;
}

void rfft1d(vector<double> data, fftw_complex* y) {
	const int n = data.size();
	fftw_complex x[n];
	for (int i = 0; i < n; i++) {
		x[i][REAL] = data[i];
		x[i][IMAG] = 0;
	}

	fftw_plan plan = fftw_plan_dft_1d(n, x, y, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(plan);
	// Do some cleaning
	fftw_destroy_plan(plan);
	fftw_cleanup();
}

void ifft1d(fftw_complex* x, fftw_complex* y, int n) {
	fftw_plan plan = fftw_plan_dft_1d(n, y, x, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(plan);
	// Do some cleaning
	fftw_destroy_plan(plan);
	fftw_cleanup();
}

vector<double> fft_convolve(vector<double>& a, vector<double>& b) {
	// Recall that element-wise
	uint padded_length = a.size() + b.size() - 1;
	if (padded_length > a.size()) {
		a.resize(padded_length, 0);
		b.resize(padded_length, 0);
	}
	else {
		a.resize(padded_length);
		b.resize(padded_length);
	}

	fftw_complex C1[padded_length];
	fftw_complex C2[padded_length];
	rfft1d(a, C1);
	rfft1d(b, C2);
	fftw_complex* R = multiply(C1, C2, padded_length);
	fftw_complex* invR = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * padded_length);
	ifft1d(invR, R, padded_length);

	vector<double> results(padded_length);
	for (uint i = 0; i < results.size(); i++)
	{
		results[i] = invR[i][REAL];
	}
	return results;
}

fftw_complex * multiply(fftw_complex * A, fftw_complex * B, int n) {
	fftw_complex* R;
	R = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) *n);
	for (int i = 0; i < n; i++) {
		R[i][REAL] = (A[i][REAL]*B[i][REAL] - A[i][IMAG] * B[i][IMAG])/n;
		R[i][IMAG] = (A[i][REAL]*B[i][IMAG] + A[i][IMAG] * B[i][REAL])/n;
	}
	return R;
}

vector<double> correlate(vector<double> a, vector<double> b) {
//	uint n = a.size() + b.size() - 1;
	vector<double> reversed(b.size());
	for (uint i = 0; i < b.size(); i++)
	{
			reversed[i] = b[i];
	}
	reverse(reversed.begin(), reversed.end());
	return fft_convolve(a, reversed);
}

void rollVector(vector<double> &a, int shift) {
   if (shift > 0) {
       vector<double> temp(shift);
       for (int i = 0; i < shift; i++) temp[i] = a[a.size() - shift + i];
       a.erase(a.end() - shift, a.end());
       for (int i = 0; i < shift; i++)a.insert(a.begin(), temp[temp.size() - i - 1]);
   }
   else if (shift < 0){
       vector<double> temp(-shift);
       for (int i = 0; i < -shift; i++) temp[i] = a[i];
       a.erase(a.begin(), a.begin() - shift);
       for (int i = 0; i < -shift; i++)a.insert(a.end(), temp[i]);
   }
}

double vectorMean(vector<double> v) {
	double sum = accumulate(v.begin(), v.end(), 0.0);
	double mean = sum / v.size();
	return mean;
}

double vectorStd(vector<double> v) {
	double mean = vectorMean(v);
	vector<double> diff(v.size());
	transform(v.begin(), v.end(), diff.begin(),
		bind2nd(minus<double>(), mean));
	double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
	double stdev = sqrt(sq_sum / v.size());
	return stdev;
}

void normVector(vector<double> &a, double mean, double std) {
	for (uint i = 0; i < a.size(); i++) a[i] = (a[i] - mean) / std;
}

double percentile(vector<double> vectorIn, double percent)
{
	vector<double>::iterator nth = vectorIn.begin() + (percent*vectorIn.size()) / 100;
	nth_element(vectorIn.begin(), nth, vectorIn.end());
	return (double)*nth;
}

vector<double> difference(vector<double> v1, vector<double> v2) {
	vector<double> diff(v1.size());
	//not need to sort since it already sorted
	transform(v1.begin(), v1.end(), v2.begin(), diff.begin(), minus<double>());
	return diff;
}

vector<double> vectorAbs(vector<double> v) {
	vector<double> absValue(v.size());
	for (uint i = 0; i < absValue.size(); i++)
	{
		absValue[i] = abs(v[i]);
	}
	return absValue;
}

vector<double> nthDiff(vector<double> v, int n) {
	vector<double> diff(v.size() - 1);
	for (uint i = 0; i < diff.size(); i++)
	{
		diff[i] = v[i + 1] - v[i];
	}
	if (n == 1) {		
		return diff;
	}
	else {
		return nthDiff(diff, n - 1);
	}
}

double roughness(vector<double> v) {
	vector<double> squared(v.size() - 2);
	for (uint i = 0; i < squared.size(); i++)
	{
		squared[i] = pow((v[i] + v[i + 1]) / 2 - v[i + 2], 2);
	}
	return vectorMean(squared);
}
