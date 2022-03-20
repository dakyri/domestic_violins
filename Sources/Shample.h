// my sham sample format. I should change this to a
// standard format when I find one that suits.
// ??? maybe the Ensoniq native format.

#include "Audio.h"

#ifndef _SHAMPLE
#define _SHAMPLE

#define MAX_SHAMPLE_NAME_LEN	60

struct ShampleHeader {
	short		magic_no;
	long		datastart,		// file offset of data
				datalength,		// length of data
				samplestart,	// sample offset to start
				sampleend,		// sample offset to end
				nloops,
				flags;
	short		base_key,
				hi_key,
				lo_key;
	double		amp,			// amplitude, 0..1
				pan;			// pan -1, ..., 1
	char		name[MAX_SHAMPLE_NAME_LEN];
};

// loop points
struct ShampleLoopInfo {
	long		count,
				start,
				end;
};

// followed by sample data: 16 bit int stereo


class Shample
{
public:
	short		*sham_data;
	area_id		locale;
	long		next_sample;
	ShampleHeader	sham_header;
	
				Shample(short *buf, long len, short base_key, char *name);
				Shample(char *name);
				~Shample();
	void		SetWaveData(short *buf, area_id loc, long len);
	void 		SetSampleBounds(long start, long end);
	void 		SetSampleKeyRange(short lo_key, short hi_key);
	void		Reset();
	void 		Play(short at_key, AudioSignal *out);
	short		NextSample(short at_key);
	long		Read(BFile *fp);
	long		Write(BFile *fp);
};

#endif