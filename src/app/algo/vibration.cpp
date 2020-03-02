#include "vibration.h"

double rmsVibVelocity(vector<double> data, double freq)
{
	double v2 = 0.;
	for (int i = 0; i < data.size(); i++) v2 += pow(1000 * data[i] * g / (2 * M_PI*freq), 2);
	double rms_v = sqrt(v2 / data.size());
	return rms_v;
}

double calOverAllValue(vector<double> freqBand, vector<double> amplitude) {
	double sum = 0;
	for (int i = 0; i < freqBand.size(); i++) {
		sum += pow(amplitude[i], 2);
	}
	double oa = 0.8165*sqrt(sum);
	return oa;
}
double calOverAllValue(vector<double> freqBand, vector<double> amplitude, double minBand, double maxBand) {
	double sum = 0;
	for (int i = 0; i < freqBand.size(); i++) {
		if (freqBand[i] >= minBand && freqBand[i] < maxBand)
			sum += pow(amplitude[i], 2);
	}
	double oa = 0.8165*sqrt(sum);
	return oa;
}