#include "ShamInstr.h"

#ifndef _SMALLCOMFORT
#define _SMALLCOMFORT

struct buf_list_i {
	short	*buf;
	long	len;
	bool	record;
	struct buf_list_i	*next;
};

#define MAX_INSTR	30


class SmallComfort: public SignalGenerator, SignalReceiver
{
public:
					SmallComfort();
					~SmallComfort();
	void			StartRecord();
	Shample			*StopRecord();
	void			StartPlay();
	void			StopPlay();
	void			SetInstr(ShamInstr *p);
	
   	bool			AudioThrough;
	bool			Recording;
	bool			Playing;
	bool			SamplerOff;
	buf_list_i		*buf_list,
					*last_buf_played,
					*free_list,
					*buf_tail;
	ShamInstr		*CurrentInstr[MAX_INSTR];
	int				NInstr;
	sem_id			buf_sem;
	long			buf_cnt;
	
	virtual short operator[](AudioSignal *In);
	virtual short operator()(AudioSignal *Out);
};

#endif