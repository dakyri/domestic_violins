#ifndef _AVERAGER
#define _AVERAGER

#include "SpecBufQueue.h"

class Averager { // than what?
public:
					Averager(int n);
					~Averager();
	void			AddToQueue(double *lAmp, double *lFreq,
						double * rAmp, double *rFreq);
	void			Process();
	double			*lAmp,
					*lFreq,
					*rAmp,
					*rFreq,
					*lAAvg,
					*rAAvg,
					*lATot,
					*rATot;
	SpecBufQueue	*q;
	int				n;
};

#endif
