#ifndef _ANALYSER
#define _ANALYSER

#include "SpecBufQueue.h"
#include "SpecDisplay.h"
#include "Averager.h"

class Analyser: public SignalReceiver
{
public:
					Analyser(Pfft *hiredHelp, BView *view, BRect v1, BRect v2,
									BRect v3, BRect v4);
					~Analyser();
	void			DrawOut();
	
	Averager		avg;
	
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
	sem_id			lbuf_sem,
					rbuf_sem;
	Pfft			*footIn;
	SpecDisplay		*lfft,
					*rfft,
					*lspec,
					*rspec;	
	SpecBufQueue	*fftq;

	virtual short operator[](AudioSignal *In);
};

#endif