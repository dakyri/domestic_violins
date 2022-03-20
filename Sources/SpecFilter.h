#ifndef _SPECFILTER
#define _SPECFILTER

class SpecFilter {
public:
				SpecFilter(int n);
				~SpecFilter();
	void		Apply(double *a, double *f);
	void		Adapt(SpecFilter *f);
	
	int			n;
	short		Params[512];
};
#endif
