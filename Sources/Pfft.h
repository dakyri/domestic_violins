#ifndef _PFFT
#define _PFFT

class Pfft
{
public:
			Pfft(int n, double SR);
			~Pfft(void);
	int		size(void)	{ return N; }
	void	forward(double *data, double *real, double *imag);
	void	reverse(double *data, double *real, double *imag);
	void	tospec(double *pow, double *f,
				 double *real, double *imag);
	void	fromspec(double *pow, double *f,
				 double *real, double *imag);
	int		permute(int n);
	
	int			N;
	int			P;
	double		SampleRate;
	short		*pertable;
	double		*ctable, *stablet, *stablei, *lastphase;

};
#endif
