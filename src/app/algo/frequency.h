#ifndef __FREQUENCY_H__
#define __FREQUENCY_H__
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <cassert>
#include <numeric>
#include <cmath>
#include <fftw3.h>
#include "app/algo/window.h"

// Macros for real and imaginary parts
#define REAL 0
#define IMAG 1

using namespace std;

double freqFromFFT(vector<double> data, double sampling_rate);
vector<double> dftFreqBand(int n, double d);
vector<double> fftAmplitude(vector<double> data);
void rfft1d(vector<double> data, fftw_complex* y);
void ifft1d(fftw_complex* x, fftw_complex* y, int n);
fftw_complex* multiply(fftw_complex * A, fftw_complex * B, int n);
vector<double> fft_convolve(vector<double>& a, vector<double>& b);
vector<double> correlate(vector<double> a, vector<double> b);
void rollVector(vector<double> &a, int shift);
double vectorMean(vector<double> v);
double vectorStd(vector<double> v);
void normVector(vector<double> &a, double mean, double std);
double percentile(vector<double> vectorIn, double percent);
vector<double> difference(vector<double> v1, vector<double> v2);
vector<double> vectorAbs(vector<double> v);
vector<double> nthDiff(vector<double> v, int n);
double roughness(vector<double> v);
# endif
