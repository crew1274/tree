#ifndef __VIBRATION_H__
#define __VIBRATION_H__
#define _USE_MATH_DEFINES
#include <vector>
#include "math.h"

static const double g = 9.80665;

using namespace std;

double rmsVibVelocity(vector<double> data, double freq);
vector<double> dftFreqBand(int n, double d);
double calOverAllValue(vector<double> freqBand, vector<double> amplitude);
double calOverAllValue(vector<double> freqBand, vector<double> amplitude, double minBand, double maxBand);

# endif