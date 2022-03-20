#include "SpecBufQueue.h"

SpecBufQueue::SpecBufQueue(int fftn)
{
	if ((my_sem = create_sem(1, "spec buf sem")) < 0) {
		fprintf(stderr, "failed to create sem\n");
		exit(1);
	}
	n = fftn;
	head = tail = free = NULL;
	closed = FALSE;
	cnt = 0;
}

void SpecBufQueue::AddToQueue(double *la,double *lf,
								double *ra,double *rf)
{
	QNode	*p;
	if (closed) {// ended
		return;
	}
	acquire_sem(my_sem);
//	fprintf(stderr, "a-q\n");
	if (free == NULL) {
		p = new QNode;
		p->la = new double[n];
		p->lf = new double[n];
		p->ra = new double[n];
		p->rf = new double[n];
		p->next = NULL;
	} else {
		p = free;
		free = p->next;
		p->next = NULL;
	}
	release_sem(my_sem);
	memcpy(p->la, la, n * sizeof(double));
	memcpy(p->lf, lf, n * sizeof(double));
	memcpy(p->ra, ra, n * sizeof(double));
	memcpy(p->rf, rf, n * sizeof(double));
	acquire_sem(my_sem);
	if (tail == NULL) {
		head = p;
	} else {
		tail->next = p;
	}
	tail = p;
	cnt++;
//	fprintf(stderr, "a-q2 %x\n", tail);
	release_sem(my_sem);
}

SpecBufQueue::~SpecBufQueue()
{
	delete_sem(my_sem);
	my_sem = -1;
}
 
	
void SpecBufQueue::TakeFromQueue(double *la,double *lf,
								double *ra,double *rf)
{
	QNode	*p;
	
//	fprintf(stderr, "t-q\n");
	if (closed) {// ended
		return;
	}
	for(;;) {
		acquire_sem(my_sem);
		if (closed) {// ended
			release_sem(my_sem);
			return;
		}
		if (head != NULL)
			break;
		release_sem(my_sem);
	}
//	fprintf(stderr, "t-q 2 %x\n", head);
	
	p = head;
	head = head->next;
	
	release_sem(my_sem);
	memcpy(la, p->la, n * sizeof(double));
	memcpy(lf, p->lf, n * sizeof(double));
	memcpy(ra, p->ra, n * sizeof(double));
	memcpy(rf, p->rf, n * sizeof(double));

	acquire_sem(my_sem);
	p->next = free;
	free = p;
	if (head == NULL)
		tail = NULL;
	cnt--;
	release_sem(my_sem);
} 
	

