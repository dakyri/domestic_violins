#include <stdio.h>
#include <time.h>

#include "KitchenView.h"
#include "DomesticViolin.h"
#include "Mangler.h"
#include "Blender.h"

DomesticViolin		*LaViol;

FILE				*dbfp;

#define FFT_BITS 8
#define FFT_SAMPLES (1 << FFT_BITS)

main()
{

	LaViol = new DomesticViolin();
	if (LaViol) {
		LaViol->Run();
		delete LaViol;
	}
	return 0;
}

long randint(int n1, int n2);
long randint(int n1, int n2)
{
	double rand1 = ((double)rand())/((double)RAND_MAX);
	rand1 = rand1 * (n2-n1);
	return n1 + rand1;
}


DomesticViolin::DomesticViolin():
	BApplication('viol'),
	MothersLittleHelper(FFT_BITS, SAMPLE_IN_RATE)
{
	BRect	windowRect, lf, rf, ls, rs;
	
    setbuf(stdout, NULL);
    dbfp = fopen("debug", "w");
    setbuf(dbfp, NULL);

	windowRect.Set( 200, 200, 600, 600 );
	kitchenWindow = new KitchenWindow( windowRect, "Domestic Violins", B_NOT_ZOOMABLE | B_NOT_RESIZABLE );
	windowRect.Set( 0, 0, 400, 400 );
	backyardView = new BackYardView(windowRect, "outside");
	kitchenWindow->AddChild(backyardView);

	lf.Set(0,5, 200, 95);
	rf.Set(0,105, 200, 195);
	ls.Set(0,205, 200, 295);
	rs.Set(0,305, 200, 395);
	smallComfort = new SmallComfort();
	homespunWisdom = new Analyser(&MothersLittleHelper,
						 backyardView, lf, rf, ls, rs);
//	AFairAttempt = new Blender(homespunWisdom->fftq);
	Trouble = new Mangler(&MothersLittleHelper,
						homespunWisdom->fftq);// AFairAttempt->outre);
	
	AdviceLine = new AudioIn(*homespunWisdom);
	LineIn = new AudioIn(*smallComfort);
	LineOut = new AudioOut(*smallComfort);
	TheInevitableMangler = new AudioOut(*Trouble);

	kitchenWindow->Lock();
	kitchenWindow->Show();
	kitchenWindow->Unlock();
	
	currentLeftFilter = randint(0,N_FILTERS-1);
	currentRightFilter = randint(0,N_FILTERS-1);
	for (short i=0; i<N_FILTERS; i++) {
		FilterScore[i] = 10;
		Filters[i] = new SpecFilter(FFT_SAMPLES);
		for (short j=0; j<0.4*Filters[i]->n; j++) {
			Filters[i]->Params[j] = randint(5,15);
		}
		for (short j=0.4*Filters[i]->n; j<Filters[i]->n; j++) {
			Filters[i]->Params[j] = 10;
		}
		
//		for (short j=0; j<Filters[i]->n; j++) {
//			fprintf(stderr, "%d ", Filters[i]->Params[j]);
//		}
//		fprintf(stderr, "\n");
	}

	MaxScore = 0;
	rec_timeout = 0;
	FilterChangeoverTicks = 0;
	FilterAdaptTicks = 0;
	FilterNextGenTicks = 0;
	RadioOnTicks = 36000;
	smallComfort->SetInstr(new ShamInstr("radio"));

	RunFilePanel("load instrument",
				"load",
				"no way",
				FALSE,
				new BMessage(LOAD_INST));
				
}

bool DomesticViolin::QuitRequested()
{
	long	crap;
	
	return TRUE;
}

void DomesticViolin::Pulse()
{
	the_time = clock()/1000;
	
	kitchenWindow->Lock();
	homespunWisdom->DrawOut();
	kitchenWindow->Unlock();

// triggers
	for (short i=0; i<smallComfort->NInstr; i++) {
		if (smallComfort->CurrentInstr[i]->active) {
			if (the_time > smallComfort->CurrentInstr[i]->stop_t) {
				smallComfort->CurrentInstr[i]->active = FALSE;
			}
		} else {
			if (randint(0,1000) < 5) {
				smallComfort->CurrentInstr[i]->active = TRUE;
			} else {
				int		width = FFT_SAMPLES/(2*smallComfort->NInstr),
						lbound = i * width+1,
						rbound = lbound + width-1;
				for (short j=lbound; j<rbound; j++) {
					fprintf(stderr, "%g\n", homespunWisdom->avg.lAAvg[j]);
					if (homespunWisdom->avg.lAAvg[j] > 1000) {
						smallComfort->CurrentInstr[i]->active
								 = TRUE;
						//break;
					}
				}
	
			}
			if (smallComfort->CurrentInstr[i]->active)
				smallComfort->CurrentInstr[i]->stop_t =
					the_time + 60;
		}					
				
	}
	
	FilterChangeoverTicks++;
	FilterNextGenTicks++;
	FilterAdaptTicks++;
	if (FilterChangeoverTicks >= 20) {
		RateFilters();
		if (FilterNextGenTicks > 3000) {
			NextGenFilters();
			FilterNextGenTicks = 0;
		}
		currentLeftFilter = randint(0,N_FILTERS-1);
		currentRightFilter = randint(0,N_FILTERS-1);
		FilterChangeoverTicks = 0;
		FilterAdaptTicks = 0;
	}
	
	if (FilterAdaptTicks > 2) {
		Trouble->lfilter.Adapt(Filters[currentLeftFilter]);
		Trouble->rfilter.Adapt(Filters[currentRightFilter]);
		FilterAdaptTicks = 0;
	}
	
	if (smallComfort->Recording) {
		if (the_time > rec_timeout) {
			Shample	*new_sham =	smallComfort->StopRecord();
			if (new_sham != NULL) {
				if (smallComfort->CurrentInstr[0]->header.n_samples) {
					Shample *old_sham =
						 smallComfort->CurrentInstr[0]->shamples[0];
					smallComfort->CurrentInstr[0]->DeleteShample(0);
					delete old_sham;
				}
				smallComfort->CurrentInstr[0]->AddShample(new_sham);
				smallComfort->CurrentInstr[0]->active = TRUE;
				smallComfort->CurrentInstr[0]->stop_t = the_time + 60;
			}
		}
	}		
//	if (++RadioOnTicks > 36000) {//
//		smallComfort->StartRecord();
//		rec_timeout = the_time + 10;
//	}
		
}

void DomesticViolin::RateFilters()
{
	for (short i=2; i<FFT_SAMPLES; i+=2) {
		FilterScore[currentLeftFilter] +=
			Trouble->avg.lAAvg[i];
		if (FilterScore[currentLeftFilter] > MaxScore)
			MaxScore = FilterScore[currentLeftFilter];
		FilterScore[currentRightFilter] +=
			Trouble->avg.rAAvg[i];
		if (FilterScore[currentLeftFilter] > MaxScore)
			MaxScore = FilterScore[currentLeftFilter];
	}
}

void DomesticViolin::NextGenFilters()
{
	SpecFilter	*NextGen[N_FILTERS];
	
	long	Total = FilterScore[0];
	
	for (short i=0; i<N_FILTERS; i++) {
		NextGen[i] = new SpecFilter(FFT_SAMPLES);
	}
	
	for (short i=1; i<N_FILTERS; i++) {
		Total += FilterScore[i];
		FilterScore[i] += FilterScore[i-1];
	}
	
	for (short i=0; i<N_FILTERS; i++) {
		long toss = randint(0, Total);
		for (short j=0; j<N_FILTERS; j++) {
			if (FilterScore[j] >= toss) {
				break;
			}
		}
		 		
		memcpy(NextGen[i]->Params, Filters[i]->Params,
				FFT_SAMPLES*sizeof(short));
	}
	
	for (short i=0; i<N_FILTERS; i++) {
		if (randint(0,100) < 10) {	// crossover
			short where = randint(0,FFT_SAMPLES);
			short which = randint(0,N_FILTERS);
			for (short j=where; j<FFT_SAMPLES; j++) {
				short temp = NextGen[i]->Params[j];
				NextGen[i]->Params[j] =
						 NextGen[which]->Params[j];
				NextGen[which]->Params[j] = temp;
			}
		}
		if (randint(0,100) < 1) { // mutate
			short which = randint(0,FFT_SAMPLES/2);
			NextGen[i]->Params[which] = 20 -
				NextGen[i]->Params[which];
		}
		if (randint(0,100) < 1) { // mutate
			short which = randint(0,FFT_SAMPLES/2);
			if (NextGen[i]->Params[which]++ == 20)
				NextGen[i]->Params[which] = 0;
		}
	}
	
	for (short i=0; i<N_FILTERS; i++) {
		delete Filters[i];
		Filters[i] = NextGen[i];
		FilterScore[i] = 10;
	}
	MaxScore = 0;
}

void DomesticViolin::RefsReceived(BMessage* inMsg)
{
	;
}


void DomesticViolin::MessageReceived(BMessage* inMsg)
{
	long		crap;

	switch ( inMsg->what ) {
	case GO:
		SetPulseRate(100000.0);
		smallComfort->StartPlay();
		break;
		
	case LOAD_INST:
		if (inMsg->HasRef("refs")) {	// do it!
			record_ref	file_ref;
			BFile		*the_file = new BFile();
			char		name[60] = "instr";
			BMessage	*dispMsg;
			ShamInstr	*CurrentInstr;
		
			CloseFilePanel();
			file_ref = inMsg->FindRef("refs");
			if (the_file->SetRef(file_ref) != B_NO_ERROR)
				fprintf(stderr, "Can't set file\n");
			if (the_file->Open(B_READ_ONLY) != B_NO_ERROR)
				fprintf(stderr, "Can't open file\n");
			fprintf(stderr, "size is %d\n", the_file->Size());
			the_file->Seek(0, B_SEEK_TOP);
			CurrentInstr = new ShamInstr(name);
			CurrentInstr->Read(the_file);
			the_file->Close();
			smallComfort->SetInstr(CurrentInstr);
			
			RunFilePanel("load instrument",
						"load",
						"no way",
						FALSE,
						new BMessage(LOAD_INST));
		} else {
		}
	break;
	default:
		BApplication::MessageReceived( inMsg );
	}
}

DomesticViolin::~DomesticViolin()
{
	fprintf(stderr, "The end of all things.\n");
	
	smallComfort->SamplerOff = TRUE;
	homespunWisdom->fftq->closed = TRUE;
//	AFairAttempt->outre->closed = TRUE;
	
	fprintf(stderr, "stopping in lines\n");
	LineIn->Stop();
	fprintf(stderr, "stopping out lines\n");
	LineOut->Stop();
	fprintf(stderr, "deleting smallComfort\n");
	delete smallComfort;
	fprintf(stderr, "stopping advice line\n");
	AdviceLine->Stop();
	fprintf(stderr, "stopping the inevitable mangler\n");
	TheInevitableMangler->Stop();
	fprintf(stderr, "deleting homespunWisdom\n");
	delete homespunWisdom;
	fprintf(stderr, "deleting a fair attempt\n");
//	delete AFairAttempt;
}


