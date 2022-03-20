#include "Audio.h"
#include "Pfft.h"
#include "Analyser.h"
#include "Mangler.h"

Mangler::Mangler(Pfft *mouth, SpecBufQueue *sbq):
	avg(mouth->N),
	lfilter(mouth->N),
	rfilter(mouth->N)
{
	if ((my_sem = create_sem(1, "spec buf sem")) < 0) {
		fprintf(stderr, "failed to create sem\n");
		exit(1);
	}
	footIn = mouth;
	fftq = sbq;
	
	left = new double[footIn->N];
	right = new double[footIn->N];
	
	lReal = new double[footIn->N];
	lImage = new double[footIn->N];
	rReal = new double[footIn->N];
	rImage = new double[footIn->N];
	
	lAmp = new double[footIn->N];
	lFreq = new double[footIn->N];
	rAmp = new double[footIn->N];
	rFreq = new double[footIn->N];
	
	pan = 0;
	
}


Mangler::~Mangler()
{
	delete left;
	delete right;
	delete lReal;
	delete lImage;
	delete rReal;
	delete rImage;
	delete lAmp;
	delete lFreq;
	delete rAmp;
	delete rFreq;
}
	
// generate out buf
short Mangler::operator()(AudioSignal *Out)
{
	int		i, j;
	short	*bufp;

	bufp = Out->Buf;	
	for (i=0; i<Out->Len/(2*footIn->N); i++) {
		acquire_sem(my_sem);
		fftq->TakeFromQueue(lAmp, lFreq, rAmp, rFreq);
//		fftq->TakeFromQueue(lReal, lImage, rReal, rImage);

		memcpy(rAmp, lAmp, sizeof(double)*footIn->N);
		memcpy(rFreq, lFreq, sizeof(double)*footIn->N);

		lfilter.Apply(lAmp, lFreq);
		rfilter.Apply(rAmp, rFreq);
				
		footIn->fromspec(lAmp, lFreq, lReal, lImage);
		footIn->reverse(left, lReal, lImage);
		footIn->fromspec(rAmp, rFreq, rReal, rImage);
		footIn->reverse(right, rReal, rImage);
		
		avg.AddToQueue(lAmp, lFreq, rAmp, rFreq);
		avg.Process();		
		
		// add rAmp, rFreq, lAmp, lFreq
		for (j=0; j<footIn->N; j++) {
			bufp[2*j] = (1+pan)*left[j];
			bufp[2*j+1] = (1-pan)*right[j];
//			fprintf(stderr, "out %d %d\n", bufp[2*j], bufp[2*j+1]);
			
		}
		release_sem(my_sem);
		
		bufp += 2 * footIn->N;
	} 
	return 0;	
	
}

