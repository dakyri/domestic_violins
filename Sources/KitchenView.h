#ifndef _KITCH
#define _KITCH

class KitchenWindow : public BWindow
{
public:
	KitchenWindow( BRect inRect, const char* inTitle, ulong inFlags );
	virtual bool QuitRequested( );
};


class BackYardView : public BView
{
public:
	BButton*		bGo;
	BButton*		bReset;
	BButton*		bPause;
	BButton*		bLoad;
	BButton*		bClose;
	BTextControl	*bSelect;
					BackYardView(BRect frame, char *name);
	virtual void	MessageReceived(BMessage *in);
	virtual bool	QuitRequested();
	virtual void	Draw(BRect region);
					~BackYardView();
};
#endif
