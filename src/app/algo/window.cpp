/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggGhost SOFTWARE CODEC SOURCE CODE.    *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggGhost SOURCE CODE IS (C) COPYRIGHT 2007-2011              *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: window functions for research code
 last mod: $Id$

 ********************************************************************/

#include "window.h"

/* A few windows */

window::window(void){}
window::~window(void) {}

/* Not-a-window */
void window::rectangle(double *x, int n){
  int i;
  for(i=0;i<n;i++)
    x[i]=1.;
}

/* sine window */
void window::sine(double *x, int n){
  int i;
  float scale = M_PI/n;

  for(i=0;i<n;i++){
    float i5 = i+.5;
    x[i] = sin(scale*i5);
  }
}

/* Minimum 4-term Blackman Harris; highest sidelobe = -92dB */
#define A0 .35875
#define A1 .48829
#define A2 .14128
#define A3 .01168

void window::blackman_harris(double *x, int n){
  int i;
  float scale = 2*M_PI/n;

  for(i=0;i<n;i++){
    float i5 = i+.5;
    float w = A0 - A1*cos(scale*i5) + A2*cos(scale*i5*2) - A3*cos(scale*i5*3);
    x[i] = w;
  }
}

/* Good 'ol Hanning window (narrow mainlobe, fast falloff, high sidelobes) */
void window::hanning(double *x, int n){
  int i;
  float scale = 2*M_PI/n;

  for(i=0;i<n;i++){
    float i5 = i+.5;
    x[i] = (.5-.5*cos(scale*i5));
  }
}

/* Triangular * gaussian (sidelobeless window) */
#define TGA 1.e-4
#define TGB 21.6
void window::tgauss_deep(double *x, int n){
  int i;
  for(i=0;i<n;i++){
    float f = (i+.5-n/2.)/(n/2.);
    x[i] = exp(-TGB*pow(f,2)) * pow(1.-fabs(f),TGA);
  }
}

void window::vorbis(double *d, int n){
  int i;
  for(i=0;i<n;i++)
    d[i] = sin(0.5 * M_PI * sin((i+.5)/n * M_PI)*sin((i+.5)/n * M_PI));
}

float window::beta(int n, double alpha){
  return cosh (acosh(pow(10,alpha))/(n-1));
}

double window::T(double n, double x){
  if(fabs(x)<=1){
    return cos(n*acos(x));
  }else{
    return cosh(n*acosh(x));
  }
}

/* Dolph-Chebyshev window (a=6., all sidelobes < -120dB) */
void window::dolphcheb(double *d, int n){
  int i,k;
  float a = 6.;
  int M=n/2;
  int N=M*2;
  double b = window::beta(N,a);

  for(i=0;i<n;i++){
    double sum=0;
    for(k=0;k<M;k++)
      sum += (k&1?-1:1)*T(N,b*cos(M_PI*k/N)) * cos (2*i*k*M_PI/N);
    sum /= window::T(N,b);
    sum-=.5;
    d[i]=sum;
  }
}

/* sidelobeless window machine optimized for fast convergence */
void window::maxwell1(double *x, int n){
  int i;
  float scale = 2*M_PI/n;
  for(i=0;i<n;i++){
    float i5 = i+.5;
    x[i] = pow( 119.72981
                - 119.24098*cos(scale*i5)
                + 0.10283622*cos(2*scale*i5)
                + 0.044013144*cos(3*scale*i5)
                + 0.97203713*cos(4*scale*i5),
                1.9730763)/50000.;
  }
}
