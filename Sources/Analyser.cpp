#include "Audio.h"
#include "Pfft.h"
#include "Analyser.h"

Analyser::Analyser(Pfft *mouth, BView *view, BRect v1, BRect v2,
									BRect v3, BRect v4):
	avg(mouth->N)
{
	footIn = mouth;
	
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
	
	fftq = new SpecBufQueue(footIn->N);
	
	if ((lbuf_sem = create_sem(1, "lbuf_sem")) < 0) {
		fprintf(stderr, "failed to create sem\n");
		exit(1);
	}
	if ((rbuf_sem = create_sem(1, "rbuf_sem")) < 0) {
		fprintf(stderr, "failed to create sem\n");
		exit(1);
	}
	lfft = new SpecDisplay(v1, lbuf_sem, footIn->N/2, lReal,
							footIn->N/2, lImage, 45, 1000);
	rfft = new SpecDisplay(v2, rbuf_sem, footIn->N/2, rReal,
							footIn->N/2, rImage, 45, 1000);
	view->AddChild(lfft);
	view->AddChild(rfft);
	
	lspec = new SpecDisplay(v3, lbuf_sem, footIn->N/2, lAmp,
							footIn->N/2, lFreq, 0, 1000);
	rspec = new SpecDisplay(v4, rbuf_sem, footIn->N/2, rAmp,
							footIn->N/2, rFreq, 0, 1000);
	view->AddChild(lspec);
	view->AddChild(rspec);
}

void Analyser::DrawOut()
{
	lfft->Draw();
	rfft->Draw();
	lspec->Draw();
	rspec->Draw();
}

Analyser::~Analyser()
{
	delete left;
	delete right;
	delete lReal;
	delete lImage;
	delete rReal;
	delete rImage;
	delete lAmp,
	delete lFreq,
	delete rAmp,
	delete rFreq;
	delete_sem(lbuf_sem);
	delete_sem(rbuf_sem);
	delete fftq;
}
	
// receive input buf
short Analyser::operator[](AudioSignal *In)
{
	int		i, j;
	short	*bufp;

	bufp = In->Buf;	
	for (i=0; i<In->Len/(2*footIn->N); i++) {
		for (j=0; j<footIn->N; j++) {
			left[j] = bufp[2*j];
			right[j] = bufp[2*j+1];
			
		}
		acquire_sem(lbuf_sem);
		footIn->forward(left, lReal, lImage);
		footIn->tospec(lAmp, lFreq, lReal, lImage);
		release_sem(lbuf_sem);
		acquire_sem(rbuf_sem);
		footIn->forward(right, rReal, rImage);
		footIn->tospec(rAmp, rFreq, rReal, rImage);
		release_sem(rbuf_sem);
		
		fftq->AddToQueue(lAmp, lFreq, rAmp, rFreq);
		avg.AddToQueue(lAmp, lFreq, rAmp, rFreq);
		avg.Process();		
//		fftq->AddToQueue(lReal, lImage, rReal, rImage);
		
		bufp += 2 * footIn->N;
	} 
	return 0;	
	
}

