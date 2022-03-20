#ifndef _MANGLER
#define _MANGLER

#include "Averager.h"
#include "SpecFilter.h"

class Mangler: public SignalGenerator
{
public:
					Mangler(Pfft *mouth, SpecBufQueue *sbq);
					~Mangler();
					
	double			*left,
					*right,
					*lReal,
					*lImage,
					*rReal,
					*rImage,
					*lAmp,
					*lFreq,
					*rAmp,
					*rFreq;
	SpecBufQueue	*fftq;
	Averager		avg;
	SpecFilter		lfilter,
					rfilter;
	Pfft			*footIn;
	sem_id			my_sem;
	double			pan;

	virtual short operator()(AudioSignal *In);
};

#endif