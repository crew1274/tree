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

double freqFromFFT(std::vector<double> data, double sampling_rate);
std::vector<double> dftFreqBand(int n, double d);
std::vector<double> fftAmplitude(std::vector<double> data);
void rfft1d(std::vector<double> data, fftw_complex* y);
void ifft1d(fftw_complex* x, fftw_complex* y, int n);
fftw_complex* multiply(fftw_complex * A, fftw_complex * B, int n);
std::vector<double> fft_convolve(std::vector<double>& a, std::vector<double>& b);
std::vector<double> correlate(std::vector<double> a, std::vector<double> b);
void rollVector(std::vector<double> &a, int shift);
double vectorMean(std::vector<double> v);
double vectorStd(std::vector<double> v);
void normVector(std::vector<double> &a, double mean, double std);
double percentile(std::vector<double> vectorIn, double percent);
std::vector<double> difference(std::vector<double> v1, std::vector<double> v2);
std::vector<double> vectorAbs(std::vector<double> v);
std::vector<double> nthDiff(std::vector<double> v, int n);
double roughness(std::vector<double> v);
# endif
