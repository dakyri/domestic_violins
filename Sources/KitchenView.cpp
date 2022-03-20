#include <stdio.h>

#include <app/Roster.h>
#include <app/Message.h>
#include <interface/Button.h>

#include "KitchenView.h"
#include "DomesticViolin.h"

KitchenWindow::KitchenWindow( BRect inRect, const char* inTitle, ulong inFlags )
	: BWindow( inRect, inTitle, B_TITLED_WINDOW, inFlags )
{
}
	
bool KitchenWindow::QuitRequested( )
{
	be_app->PostMessage( B_QUIT_REQUESTED );
	return TRUE;
}

BackYardView::BackYardView(BRect rect, char *name)
	: BView(rect, name, B_FOLLOW_ALL, B_WILL_DRAW)
	
{
	BRect 		ctrlRect;
	BMessage	*msg;
	int			i;
	

	SetFontName( "Emily Bold" );
	SetFontSize( 12 );

	ctrlRect.Set( 220, 5, 270, 35 );
	bGo = new BButton( ctrlRect, "",
		"Play", new BMessage(GO), B_FOLLOW_ALL, B_WILL_DRAW );
	AddChild(bGo);
}
	
bool BackYardView::QuitRequested()
{
	fprintf(stderr, "QuaView signing off!\n");
//	be_app->PostMessage( B_QUIT_REQUESTED );
	return TRUE;
}



void BackYardView::MessageReceived( BMessage* inMsg )
{
}

void BackYardView::Draw(BRect region)
{
}


BackYardView::~BackYardView()
{
}
