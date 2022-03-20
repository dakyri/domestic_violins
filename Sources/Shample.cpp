#include "Shample.h"

Shample::Shample(char *name)
{
	Shample(NULL, 0, -1, name);
			fprintf(stderr, "here... name %s\n", sham_header.name);

}

Shample::Shample(short *buf, long len, short base_key, char *name)
{
	fprintf(stderr, "new shample %x %d %d %s\n", buf, len,
					base_key, name);
	if (len > 0) {
//		sham_data = (short *) malloc(len * sizeof(short));
		locale = create_area("name",
					&sham_data,
					B_ANY_ADDRESS,
					B_PAGE_SIZE*
						((len*sizeof(short))/B_PAGE_SIZE + 1),
					B_NO_LOCK,
					B_READ_AREA|B_WRITE_AREA); 
		if (locale == B_BAD_VALUE || locale == B_ERROR || locale == B_NO_MEMORY) {
			fprintf(stderr, "fucked out bad on create are\n");
			exit(1);
		}
		for (long i=0; i<len; i++) {
			sham_data[i] = buf[i];
		}
	} else {
		sham_data = NULL;
	}
	sham_header.magic_no = 0;
	sham_header.datastart = 0;
	sham_header.datalength = len;
	sham_header.samplestart = 0;
	sham_header.sampleend = len;
	sham_header.nloops = 0;
	sham_header.flags = 0;
	sham_header.base_key = base_key;
	sham_header.hi_key = 0;
	sham_header.lo_key = 127;
	sham_header.amp = 1;
	sham_header.pan = 0;
	strcpy(sham_header.name, name);
	fprintf(stderr, "stored %s\n", sham_header.name);
}

void Shample::SetWaveData(short *buf, area_id loc, long len)
{
	fprintf(stderr, "setting wv %x %d %s\n", buf, len, sham_header.name);

	if (sham_data) {
		delete_area(locale);
	}
	sham_data = buf;
	locale = loc;
	sham_header.datalength = len;
	sham_header.samplestart = 0;
	sham_header.sampleend = len;
	fprintf(stderr, "set wave data %x %d %s\n", buf, len, sham_header.name);
}	

Shample::~Shample()
{
	delete_area(locale);
}

void Shample::SetSampleBounds(long start, long end)
{
	sham_header.samplestart = start;
	sham_header.sampleend = end;
}

void Shample::SetSampleKeyRange(short lo_key, short hi_key)
{
	sham_header.lo_key = lo_key;
	sham_header.hi_key = hi_key;
}

void Shample::Play(short at_key, AudioSignal *out)
{
	long	i;

	for (i=0; i<out->Len/2; i++) {
		if (next_sample >= sham_header.sampleend)
			next_sample = sham_header.samplestart;
		out->Buf[2*i] += 0.75 * (1-sham_header.pan) * sham_header.amp *
							sham_data[next_sample];
		out->Buf[2*i+1] += 0.75 *   (1 + sham_header.pan) * sham_header.amp *
							sham_data[next_sample++];
	}
}

void Shample::Reset()
{
	next_sample = sham_header.samplestart;
}

short Shample::NextSample(short at_key)
{
	short sample = sham_data[next_sample];
	if (++next_sample == sham_header.sampleend)
		next_sample = sham_header.samplestart;
	return sample;
}

long Shample::Write(BFile *fp)
{
	long	hdrloc = fp->Seek(0, B_SEEK_MIDDLE);
	
	fp->Write(&sham_header, sizeof(sham_header));
	sham_header.datastart = fp->Seek(0, B_SEEK_MIDDLE);
	fp->Seek(hdrloc, B_SEEK_TOP); 
	fp->Write(&sham_header, sizeof(sham_header));
	fp->Write(sham_data, sham_header.datalength * sizeof(short));
	return B_NO_ERROR;
}

long Shample::Read(BFile *fp)
{
	if (sham_data != NULL)
		free(sham_data);
	fp->Read(&sham_header, sizeof(sham_header));
	fp->Seek(sham_header.datastart, B_SEEK_TOP);
//	sham_data = (short *) malloc(sham_header.datalength * sizeof(short));
	locale = create_area("name",
				&sham_data,
				B_ANY_ADDRESS,
				B_PAGE_SIZE*
					((sham_header.datalength*sizeof(short))/B_PAGE_SIZE + 1),
				B_NO_LOCK,
				B_READ_AREA|B_WRITE_AREA); 
	if (locale == B_BAD_VALUE || locale == B_ERROR || locale == B_NO_MEMORY) {
		fprintf(stderr, "fucked out bad on create are\n");
		exit(1);
	}
	fp->Read(sham_data, sham_header.datalength * sizeof(short));
	fprintf(stderr, "data %d bytes\n",
			 sham_header.datalength * sizeof(short));
	return B_NO_ERROR;
}