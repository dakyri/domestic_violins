#include "SpecFilter.h"
#define FILTER_PARAM_0_GAIN	10

SpecFilter::SpecFilter(int nffts)
{
	n = nffts;
	for (int i=0; i<n; i++)
		Params[i] = FILTER_PARAM_0_GAIN;
}

SpecFilter::~SpecFilter()
{
}

void SpecFilter::Apply(double *a, double *f)
{
	for (int i=0; i<n; i++)
		a[i] *= (double)Params[i]/(double)FILTER_PARAM_0_GAIN;
}

#define MAX_ADAPT 256

void SpecFilter::Adapt(SpecFilter *f)
{
	int			nadapt = 0;
	
//	for (short i=0; i<n; i++) {
//	fprintf(stderr, "%d ", f->Params[i]);
//	}
	fprintf(stderr, "\n");
	for (short i=2; i<128; i++) {
		if (Params[i] > f->Params[i]) {
			Params[i]--;
			nadapt++;
		} else if (Params[i] < f->Params[i]) {
			Params[i]++;
			nadapt++;
		}
		if (nadapt > MAX_ADAPT)
			break;
	}
}
