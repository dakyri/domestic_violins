#include "Pfft.h"
#include "Audio.h"
#include "SmallComfort.h"
#include "Analyser.h"
#include "KitchenView.h"
#include "Mangler.h"
#include "Blender.h"

#define N_FILTERS	30

#define GO	667
#define LOAD_INST	666


class DomesticViolin: public BApplication
{
public:
						DomesticViolin();
						~DomesticViolin();
	virtual void		MessageReceived(BMessage *msg);
	virtual bool		QuitRequested();
	virtual void		RefsReceived(BMessage *msg);
	virtual void		Pulse();
	void				RateFilters();
	void				NextGenFilters();
	
	KitchenWindow		*kitchenWindow;
	BackYardView		*backyardView;
	SmallComfort		*smallComfort;
	Mangler				*Trouble;
	Blender				*AFairAttempt;
	Analyser			*homespunWisdom;
	AudioIn				*LineIn;
	AudioIn				*AdviceLine;
	AudioOut			*LineOut;
	AudioOut			*TheInevitableMangler;
	Pfft				MothersLittleHelper;
	
	SpecFilter			*Filters[N_FILTERS];
	short				currentLeftFilter,
						currentRightFilter;
	long				FilterScore[N_FILTERS];
	long				rec_timeout;
	long				FilterChangeoverTicks;
	long				FilterAdaptTicks;
	long				FilterNextGenTicks;
	long				RadioOnTicks;
	long				MaxScore;
	
	long				the_time;
};