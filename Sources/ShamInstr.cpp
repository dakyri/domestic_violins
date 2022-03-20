#include "ShamInstr.h"

ShamInstr::ShamInstr(char *nm)
{
	strcpy(name, nm);
	strcpy(header.name, nm);
	header.n_samples = 0;
	header.amp = 1;
	header.pan = 0;
	play_pos = 0;
	active = FALSE;
	stop_t = 0;
	fprintf(stderr, "created new inst %s/%s\n", name, header.name);
}

ShamInstr::~ShamInstr()
{
	for (short i=0; i<header.n_samples; i++)
		delete shamples[i];
}

void ShamInstr::Play(short at_key, AudioSignal *out)
{
	if (! active)
		return;
	for (short i=0; i<header.n_samples; i++)
		shamples[i]->Play(at_key, out);
}

void ShamInstr::SetName(char *nm)
{
	strcpy(name, nm);
	strcpy(header.name, nm);
}
	

void ShamInstr::AddSample(short *buf, long len,
						 short base_key, char *name)
{
	shamples[header.n_samples] = new Shample(buf, len,
						 base_key, name);
	header.n_samples++;
}

void ShamInstr::AddShample(Shample *the_shample)
{
	shamples[header.n_samples] = the_shample;
	header.n_samples++;
}



void ShamInstr::DeleteShample(short which)
{
	header.n_samples--;
	for (short i=0; i<header.n_samples; i++)
		shamples[i] = shamples[i+1];
}

void ShamInstr::Reset()
{
	for (short i=0; i<header.n_samples; i++) {
		fprintf(stderr, "reset %x %d\n", this, i);
		shamples[i]->Reset();
	}
	play_pos = 0;
}

short ShamInstr::NextSample(short at_key)
{
	short sample = 0;
	for (short i=0; i<header.n_samples; i++)
		sample += shamples[i]->NextSample(at_key);
	return sample;
}


long ShamInstr::Write(BFile *fp)
{
	fp->Write(&header, sizeof(header));
	for (short i=0; i<header.n_samples; i++)
		shamples[i]->Write(fp);
	return B_NO_ERROR;
}

long ShamInstr::Read(BFile *fp)
{
	fp->Read(&header, sizeof(header));
	strcpy(name, header.name);
	for (short i=0; i<header.n_samples; i++) {
		shamples[i] = new Shample(NULL, 0, 0, "");
		shamples[i]->Read(fp);
	}
	return B_NO_ERROR;
}