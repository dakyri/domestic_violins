#ifndef _AUDIO
#define _AUDIO
class AudioSignal {
public:
	short		Buf[1024];
	long		Len;
};

class SignalGenerator{
public:
	virtual short operator() (AudioSignal *buf) = 0;
};

class SignalReceiver{
public:
	virtual short operator[] (AudioSignal *buf) = 0;
};


class AudioIn: BAudioSubscriber
{
public:
					AudioIn(SignalReceiver &sigrcv);
					~AudioIn();
	void			Stop();
					
	AudioSignal		In;
	SignalReceiver	*receiver;
	long			vectorsInInBuf;
	static long		Comp( void* userData, long error);
	static bool		Stream(void *userData, char *buffer, long count);
};

class AudioOut: BAudioSubscriber
{
public:
					AudioOut(SignalGenerator &sigGen);
					~AudioOut();
	void			Stop();
					
	double			pan;
	bool			mute;
	AudioSignal		Out;
	SignalGenerator	*generator;
	long			vectorsInOutBuf;
	static long		Comp( void* userData, long error);
	static bool		Stream(void *userData, char *buffer, long count);
};


long AudioOutComp( void* userData, long error);
bool AudioOutStream(void *userData, char *buffer, long count);


//long AudioInComp( void* userData, long error);
//bool AudioInStream(void *userData, char *buffer, long count);

#define SAMPLE_IN_RATE	22050
#define SAMPLE_OUT_RATE	22050

#endif