#ifndef _BLENDER
#define _BLENDER

#include "SpecBufQueue.h"
#include "Averager.h"	

class Blender
{
public:
					Blender(SpecBufQueue *sbq);
					~Blender();
	void			Main();
	static long		MainWrapper(void *data);
	void			Triggers();
	static long		TriggersWrapper(void *data);
	void			Friggers();
	static long		FriggersWrapper(void *data);
	void			ProcessSpectrum(double *lAmp, double *lFreq,
							 		double *rAmp, double *rFreq);
					
	double			*lAmp,
					*lFreq,
					*rAmp,
					*rFreq;
	short			*FilterParam;
	SpecBufQueue	*in_tray,
					*outre;
	thread_id		main_thread,
					trig_thread,
					frig_thread;
	Averager		*trig_avg,
					*process_avg;
};
#define FILTER_PARAM_0_GAIN	10
#endif