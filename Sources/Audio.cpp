#include "Audio.h"

#define kAudioBufferSize	8192	// bytes
#define kNumAudioBuffers	6


bool cbb(void *data, char *buf, long size);
void want_little_b();

bool cbb(void *data, char *buf, long size)
{
	long *count = (long *) data;
	if (size >= 8192)
		--*count;
	return (*count > 0);
}

void want_little_b()
{
	long bc = 8;
	BAudioSubscriber* as = new BAudioSubscriber("");
	as->Subscribe(B_DAC_STREAM, B_SHARED_SUBSCRIBER_ID, TRUE);
	as->SetStreamBuffers(8192, bc);
	as->EnterStream(NULL, TRUE, &bc, cbb, NULL, FALSE);
	delete as;
}

AudioOut::AudioOut(SignalGenerator& sigGen):
	BAudioSubscriber("AudioOut")
{

#ifdef ARGH
	want_little_b();	// will cause a glitch
#endif

	generator = &sigGen;
	pan = 0;
	mute = FALSE;
	Out.Len = 1024;	
	vectorsInOutBuf = kAudioBufferSize / (Out.Len * 2 * sizeof(short));
	
	long result;
	result = Subscribe( B_DAC_STREAM,
							 B_INVISIBLE_SUBSCRIBER_ID, FALSE);
	result = SetStreamBuffers( kAudioBufferSize,
										 kNumAudioBuffers);

// send it out in stereo
	result = SetDACSampleInfo(2, 2,
							 B_BIG_ENDIAN, B_LINEAR_SAMPLES);
	result = SetSamplingRate( SAMPLE_OUT_RATE );
	result = EnterStream(NULL,TRUE,this,
			&Stream,&Comp,TRUE);
}


AudioOut::~AudioOut()
{
}

void AudioOut::Stop()
{
	ExitStream(TRUE);
}

long AudioOut::Comp( void* userData, long error)
{
	fprintf(stderr, "exitting out stream: err %d\n", error);
	return TRUE;
}

bool AudioOut::Stream(void *userData, char *buffer, long count)
{
	
	float	ampl, ampr;

	/*
	 * Synthesize into the buffer that has just completed and set it running
	 */
	
	short *samples = (short*) buffer;

	for(short i=0;i<((AudioOut*)userData)->vectorsInOutBuf;i++){
		if ((ampr = 1.0 + ((AudioOut*)userData)->pan) < 0)
			ampr = 0;
		if ((ampl = 1.0 - ((AudioOut*)userData)->pan) < 0)
			ampl = 0;


		(*((AudioOut*)userData)->generator)
					(&((AudioOut*)userData)->Out);

		for(unsigned long j=0;j<((AudioOut*)userData)->Out.Len;j++){
			if (!((AudioOut*)userData)->mute) {
				samples[2*j]+=ampl*
					((AudioOut*)userData)->Out.Buf[j];
				samples[2*j+1]+=ampr*
					((AudioOut*)userData)->Out.Buf[j];
			}
		}
		
		samples+=2*((AudioOut*)userData)->Out.Len;
	}
	
	return TRUE; 
}


AudioIn::AudioIn(SignalReceiver& sigRcv):
	BAudioSubscriber("AudioIn")
{

#ifdef ARGH
	want_little_b();	// will cause a glitch
#endif

	receiver = &sigRcv;
	In.Len = 1024;	
	vectorsInInBuf = kAudioBufferSize / (2 * In.Len * sizeof(short));
	
	long result;
	result = Subscribe( B_ADC_STREAM,
							 B_INVISIBLE_SUBSCRIBER_ID, FALSE);
	result = SetStreamBuffers( kAudioBufferSize,
										 kNumAudioBuffers);

// send it out in stereo
	result = SetADCSampleInfo(2, 2,
							 B_BIG_ENDIAN, B_LINEAR_SAMPLES);
	result = SetSamplingRate( SAMPLE_IN_RATE );
	result = EnterStream(NULL,TRUE,this,
			&Stream,&Comp,TRUE);
}


AudioIn::~AudioIn()
{
}

long AudioIn::Comp( void* userData, long error)
{
	fprintf(stderr, "exitting in stream: err %d\n", error);
	return TRUE;
}

bool AudioIn::Stream(void *userData, char *buffer, long count)
{
	
	float	ampl, ampr;

	/*
	 * Synthesize into the buffer that has just completed and set it running
	 */
	
	short *samples = (short*) buffer;

	for(short i=0;i<((AudioIn*)userData)->vectorsInInBuf;i++){
		for(unsigned long j=0;j<((AudioIn*)userData)->In.Len;j++){
			((AudioIn*)userData)->In.Buf[j] = (samples[2*j]/2) +
										(samples[2*j+1]/2);
		}
		
		(*((AudioIn*)userData)->receiver)
					[&((AudioIn*)userData)->In];

		samples+=2*((AudioIn*)userData)->In.Len;
	
	}
	
	return TRUE; 
}

void AudioIn::Stop()
{
	ExitStream(TRUE);
}

