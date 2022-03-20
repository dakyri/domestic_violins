#ifndef _SPECBUFQUE
#define _SPECBUFQUE
struct QNode {
	double			*la, *lf, *ra, *rf;
	struct QNode	*next;
};

class SpecBufQueue {
public:
				SpecBufQueue(int fftn);
				~SpecBufQueue();
	void		AddToQueue(double *la,double *lf,
								double *ra,double *rf);
	void		TakeFromQueue(double *la,double *lf,
								double *ra,double *rf);
	sem_id		my_sem;
	QNode		*head, *tail, *free;
	int			n, cnt;
	bool		closed;
};
#endif
