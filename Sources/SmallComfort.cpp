#include "Audio.h"
#include "Shample.h"
#include "ShamInstr.h"
#include "SmallComfort.h"

SmallComfort::SmallComfort()
{
	AudioThrough = FALSE;
	Recording = FALSE;
	Playing = FALSE;
	buf_list = NULL;
	buf_cnt = 0;
	free_list = NULL;
	last_buf_played = NULL;
	buf_tail = NULL;
	SamplerOff = FALSE;
	if ((buf_sem = create_sem(1, "buf_sem")) < 0) {
		fprintf(stderr, "failed to create sem\n");
		exit(1);
	}
	NInstr = 0;
}

SmallComfort::~SmallComfort()
{
	buf_list_i		*p, *q;

	AudioThrough = FALSE;
	acquire_sem(buf_sem);	
	for (p=buf_list; p != NULL; ) {
		q = p->next;
		free(p->buf);
		free(p);
		p = q;
	}
	for (p=free_list; p != NULL; ) {
		q = p->next;
		free(p->buf);
		free(p);
		p = q;
	}
	release_sem(buf_sem);
	delete_sem(buf_sem);
}
	
// generate output buf
short SmallComfort::operator()(AudioSignal *Out)
{
	buf_list_i	*p, **q;
// find a full buffer, or wait for one
	if (SamplerOff)
		return TRUE;
	if (AudioThrough || Recording) {
		for (;;) {
			acquire_sem(buf_sem);
			p = (last_buf_played == NULL)? buf_list : last_buf_played->next;
			if (SamplerOff) {
				release_sem(buf_sem);
				return TRUE;
			}
			if (p != NULL)			
				break;
			release_sem(buf_sem);
		}
		
		if (!Recording) {
			buf_list = p->next;
			if (buf_list == NULL)
				buf_tail = NULL;
			p->next = free_list;
			free_list = p;
			last_buf_played = NULL;
			buf_cnt--;
		} else
			last_buf_played = p;
			
		release_sem(buf_sem);

// fill out buf. remember p is only the R channel		
		for (long i=0; i<Out->Len/2; i++) {
			Out->Buf[2*i+1] = p->buf[i];
			Out->Buf[2*i] = 0;
		}
		
	} else {
		for (long i=0; i<Out->Len; i++)
			Out->Buf[i] = 0;
	}
	
// now add in some playback	
	if (Playing) {
		for (short i=0; i<NInstr; i++) {
			CurrentInstr[i]->Play(-1, Out);	// play at recorded pitch
		}
	}
	
	return 0;
}

// receive input buf
short SmallComfort::operator[](AudioSignal *In)
{
	struct buf_list_i	*p, **qp, *q;

	if (AudioThrough || Recording) {	
// find first free buffer, or make one
		acquire_sem(buf_sem);
		if (free_list == NULL) {	// no free buffers
			p = (buf_list_i *)malloc(sizeof(buf_list_i));
			p->buf = (short *)malloc(In->Len * sizeof(short)/2);
			p->len = In->Len;
			p->next = NULL;	
			p->record = Recording;		
		} else {
			p = free_list;
			free_list = p->next;
			p->next = NULL;
		}
		buf_cnt++;
		release_sem(buf_sem);
		
// fill it with this signal
// but for our purposes, only the right (radio) channel
		for (long i=1; i<In->Len; i+=2) {
			p->buf[i] = In->Buf[i];
		}

		acquire_sem(buf_sem);
		if (buf_tail) {
			buf_tail->next = p;
		}
		if (buf_list == NULL) {
			buf_list = p;
		}
		buf_tail = p;
		release_sem(buf_sem);
	}

	return 0;
}

void SmallComfort::SetInstr(ShamInstr *p)
{
	if (NInstr < MAX_INSTR) {
		CurrentInstr[NInstr] = p;
		NInstr++;
	}
}

void SmallComfort::StartRecord()
{
	if (Recording)
		return;
	Recording = TRUE;
}

Shample *SmallComfort::StopRecord()
{
	buf_list_i		*the_recording,
					*last_recd_buf,
					*p;
	Shample			*the_sample;
	long			n_recd_buf,
					sam_len;
	short			*new_sample,
					*sam_ptr;

	if (!Recording)
		return NULL;

fprintf(stderr, "got in\n");			
// now grab the whole list
	acquire_sem(buf_sem);
	Recording = FALSE;
	the_recording = buf_list;
	last_recd_buf = buf_tail;
	last_recd_buf->next = NULL;
	n_recd_buf = buf_cnt;
	fprintf(stderr, "list %x tail %x cnt %d\n", buf_list, buf_tail, buf_cnt);
	last_buf_played = NULL;
	buf_list = NULL;
	buf_tail = NULL;
	buf_cnt = 0;
	release_sem(buf_sem);

fprintf(stderr, "got sem and %d bufs\n", n_recd_buf);	
// process buffers.
	the_sample = NULL;
	if (n_recd_buf > 0) {
		area_id		loc;
		char		name[30];
		
		fprintf(stderr, "trying for an area\n");
		loc = create_area("name",
					&new_sample,
					B_ANY_ADDRESS,
					B_PAGE_SIZE*
						((n_recd_buf*1024*sizeof(short))/B_PAGE_SIZE + 1),
					B_NO_LOCK,
					B_READ_AREA|B_WRITE_AREA);
		if (loc == B_BAD_VALUE || loc == B_ERROR || loc == B_NO_MEMORY) {
			fprintf(stderr, "fucked out bad on create\n");
			exit(1);
		}
		
		area_info	a;
		get_area_info(loc, &a);
		fprintf(stderr, "size = %d\n", a.size);
		
		sam_ptr = new_sample;
		sam_len = 0;
		short cnt = 0;
		for (p=the_recording; p!=NULL; p=p->next) {
			cnt++;
			for (short i=0; i<1024; i++) {
				sam_ptr[i] = p->buf[i];
			}
			sam_ptr += 1024;
			sam_len += 1024;
		}
		sprintf(name, "sample%x", new_sample);
		the_sample = new Shample(NULL, 0, -1, name);
		fprintf(stderr, "name %s\n", the_sample->sham_header.name);
		the_sample->SetWaveData(new_sample, loc, sam_len);
	}
fprintf(stderr, "processed the sample, len %d\n", sam_len);
// free the used list.	
	acquire_sem(buf_sem);
	last_recd_buf->next = free_list;
	free_list =	the_recording;
	release_sem(buf_sem);
fprintf(stderr, "and left the sampler %x free = %x, %d, %x %x\n",
			 the_sample, free_list, buf_cnt, buf_list, buf_tail);
	return the_sample;
}


void SmallComfort::StartPlay()
{
	Playing = TRUE;
	for (short i=0; i<NInstr; i++)
		CurrentInstr[i]->Reset();
}


void SmallComfort::StopPlay()
{
	Playing = FALSE;
}


