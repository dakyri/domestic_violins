#include "Blender.h"

Blender::Blender(SpecBufQueue *sbq)
{
	short		i;
	
	in_tray = sbq;
	outre = new SpecBufQueue(in_tray->n);
	
	lAmp = new double[sbq->n];
	lFreq = new double[sbq->n];
	rAmp = new double[sbq->n];
	rFreq = new double[sbq->n];
	FilterParam = new short[sbq->n];
	trig_avg = new Averager(sbq->n);
	process_avg = new Averager(sbq->n);
	
	for (i=0; i<sbq->n; i++)
		FilterParam[i] = 10;
	
	main_thread = spawn_thread(MainWrapper, 
	         "blender thread", 
	          B_URGENT_DISPLAY_PRIORITY,
	          this);
	if ( (resume_thread(main_thread)) != B_NO_ERROR) {
		fprintf(stderr, "Can't run blender thread\n");
		exit(1);
	}
	trig_thread = spawn_thread(TriggersWrapper, 
			"trigger",
			B_DISPLAY_PRIORITY,
			this);
   if ( (resume_thread(trig_thread)) != B_NO_ERROR) {
		fprintf(stderr, "Can't run trigger thread\n");
		exit(1);
	}
	frig_thread = spawn_thread(FriggersWrapper,
	          "frigger thread", 
				B_DISPLAY_PRIORITY,
				this);
	if ( (resume_thread(frig_thread)) != B_NO_ERROR) {
		fprintf(stderr, "Can't run frigger thread\n");
		exit(1);
	}
}


Blender::~Blender()
{
	kill_thread(main_thread);
	kill_thread(trig_thread);
	kill_thread(frig_thread);
	delete outre;
	delete lAmp;
	delete rAmp;
	delete lFreq;
	delete rFreq;
	delete FilterParam;
	delete trig_avg;
	delete process_avg;
}

long Blender::MainWrapper(void *data)
{
	((Blender *)data)->Main();
	return 0;
}

void Blender::Main()
{
	for (;;) {
		in_tray->TakeFromQueue(lAmp, lFreq, rAmp, rFreq);
		trig_avg->AddToQueue(lAmp, lFreq, rAmp, rFreq);
		memcpy(rAmp, lAmp, outre->n);
		memcpy(rFreq, lFreq, outre->n);
		ProcessSpectrum(lAmp, lFreq, rAmp, rFreq);
		process_avg->AddToQueue(lAmp, lFreq, rAmp, rFreq);
		outre->AddToQueue(lAmp, lFreq, rAmp, rFreq);	
	}
}

long Blender::TriggersWrapper(void *data)
{
	((Blender *)data)->Triggers();
	return 0;
}

void Blender::Triggers()
{
	for (;;) {
		trig_avg->Process();
		// loop around making control decisions based on the input.
	}
}

long Blender::FriggersWrapper(void *data)
{
	((Blender *)data)->Friggers();
	return 0;
}

void Blender::Friggers()
{
	for (;;) {
		process_avg->Process();
		// loop around frigging with the filter parameters,
		// according to what has come out the other end.
	}
}

void Blender::ProcessSpectrum(double *lAmp, double *lFreq,
							 double *rAmp, double *rFreq)
{
	static factor=2;
	for (short i=0; i<outre->n; i++) {
		lAmp[i] = FilterParam[i]*lAmp[i]/FILTER_PARAM_0_GAIN;
		rAmp[i] = FilterParam[i]*rAmp[i]/FILTER_PARAM_0_GAIN;
	}
}

/*	
// generate out buf
short Blender::operator()(AudioSignal *Out)
{
	int		i, j;
	short	*bufp;

	bufp = Out->Buf;	
	for (i=0; i<Out->Len/(2*footIn->N); i++) {
//		fftq->TakeFromQueue(lAmp, lFreq, rAmp, rFreq);
		fftq->TakeFromQueue(lReal, lImage, rReal, rImage);
		
//		footIn->fromspec(lAmp, lFreq, lReal, lImage);
//		footIn->reverse(left, lReal, lImage);
//		footIn->fromspec(rAmp, rFreq, rReal, rImage);
//		footIn->reverse(right, rReal, rImage);

		footIn->reverse(left, lReal, lImage);
		footIn->reverse(right, rReal, rImage);
		
		// add rAmp, rFreq, lAmp, lFreq
		for (j=0; j<footIn->N; j++) {
			bufp[2*j] = left[j];
			bufp[2*j+1] = right[j];
//			fprintf(stderr, "out %d %d\n", bufp[2*j], bufp[2*j+1]);
			
		}
		
		bufp += 2 * footIn->N;
	} 
	return 0;	
	
}
*/
