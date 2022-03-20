#ifndef _SPECDISPLAY
#define _SPECDISPLAY

class SpecDisplay: public BView
{
public:
					SpecDisplay(BRect where, sem_id buf_sem,
							int n1, double *x, int n2, double *y,
							float sc, float off);
	virtual void	Draw(BRect region);
	void			Draw();
	double			*x, *y;
	float			*snap;
	int				nx, ny;
	BRect			reg;
	float			scale,
					offset;
	sem_id			sem;
};

#endif
