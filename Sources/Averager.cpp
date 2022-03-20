#include "Averager.h"

Averager::Averager(int ns)
{
	n = ns;
	q = new SpecBufQueue(n);
	lAmp = new double[n];
	lFreq = new double[n];
	rAmp = new double[n];
	rFreq = new double[n];
	lAAvg = new double[n];
	rAAvg = new double[n];
	lATot = new double[n];
	rATot = new double[n];
	
	for (short i=0; i<n; i++) {
		lAAvg[i] = rAAvg[i] = lATot[i] = rATot[i] = 0;
	}
}

Averager::~Averager()
{
	delete q;
	delete lAmp;
	delete lFreq;
	delete rAmp;
	delete rFreq;
	delete lAAvg;
	delete rAAvg;
	delete lATot;
	delete rATot;
}

#define AVG_MEM	10

void Averager::Process()
{
	while (q->cnt > AVG_MEM) {
		q->TakeFromQueue(lAmp, lFreq, rAmp, rFreq);
		for (short i=0; i<n; i++) {
			lATot[i] -= lAmp[i];
			rATot[i] -= rAmp[i];
		}
	}
	for (short i=0; i<n; i++) {
		lAAvg[i] = lATot[i]/q->cnt;
		rAAvg[i] = rATot[i]/q->cnt;
	}
}

void Averager::AddToQueue(double *lAmp, double *lFreq,
					double * rAmp, double *rFreq)
{
	q->AddToQueue(lAmp, lFreq, rAmp, rFreq);
	for (short i=0; i<n; i++) {
		lATot[i] += lAmp[i];
		rATot[i] += rAmp[i];
	}
}

