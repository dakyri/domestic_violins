// format for the sham sample instrument
#ifndef _SHAMINST
#define _SHAMINST

#include "Shample.h"
#include "Audio.h"

#define MAX_SHAMPLES_PER_INST	20
#define MAX_SHAMPLEINSTR_NAME	60
struct ShamInstrHeader {
	short		magic_no;
	short		n_samples;
	char		name[MAX_SHAMPLEINSTR_NAME];
	double		amp,			// amplitude, 0..1
				pan;			// pan -1, ..., 1
};

// n_samples * long for the array of offsets of sham-ples.

// shample data

class ShamInstr {
public:
	ShamInstrHeader	header;
	char			name[MAX_SHAMPLEINSTR_NAME];
	Shample			*shamples[MAX_SHAMPLES_PER_INST];
	long			play_pos;
	bool			active;
	long			stop_t;
	
					ShamInstr(char *name);
					~ShamInstr();
	void			SetName(char *name);
	void 			AddSample(short *buf, long len,
						 short base_key, char *name);
	void			DeleteShample(short which);
	void			AddShample(Shample *the_shample);
	void			Reset();
	void 			Play(short at_key, AudioSignal *out);
	short			NextSample(short at_key);
	long			Read(BFile *fp);
	long			Write(BFile *fp);
};

#endif