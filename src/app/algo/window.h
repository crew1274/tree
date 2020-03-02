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

#ifndef __WINDOW_H__
#define __WINDOW_H__
#define _USE_MATH_DEFINES
#include "math.h"

class window
{
public:
	window(void);
	~window(void);
	void rectangle(double *x, int n);
	void sine(double *x, int n);
	void hanning(double *x, int n);
	void vorbis(double *x, int n);
	void blackman_harris(double *x, int n);
	void tgauss_deep(double *x, int n);
	void dolphcheb(double *x, int n);
	void maxwell1(double *x, int n);
private:
	float beta(int n, double alpha);
	double T(double n, double x);
};

# endif