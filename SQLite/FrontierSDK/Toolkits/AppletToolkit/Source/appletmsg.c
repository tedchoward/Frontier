
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletinternal.h"
#include "appletmsg.h"


#define messagefont geneva
#define messagesize 9
#define messagestyle 0



void appsetmessagerect (hdlappwindow appwindow) {
	
	Rect r;
		
	if (app.hasmessagearea) {
	
		Rect rwindow = (*(**appwindow).macwindow).portRect;
		
		r.bottom = rwindow.bottom;
		
		r.left = rwindow.left;
		
		r.top = r.bottom - getscrollbarwidth () + 1;
		
		r.right = r.left + ((rwindow.right - rwindow.left) / 2);
		}
	else
		zerorect (&r);
	
	(**appwindow).messagerect = r;
	} /*appsetmessagerect*/
	

void appdrawmessage (hdlappwindow appwindow) {
	
	if (app.hasmessagearea) {
		
		hdlappwindow ha = appwindow;
		WindowPtr w = (**ha).macwindow;
		Rect r = (**ha).messagerect;
		Rect rerase, rtext;
		bigstring bs;
		boolean flbitmap = false;
		
		pushmacport (w);
		
		/*flbitmap = openbitmap (r, w);*/
		
		pushclip (r);
		
		rerase = r; rerase.top++; rerase.right--; eraserect (rerase);
		
		pushforecolor (&blackcolor);
		
		MoveTo (r.left, r.top); LineTo (r.right, r.top);
		
		popforecolor ();
		
		texthandletostring ((**ha).hmessage, bs);
	
		pushstyle (messagefont, messagesize, messagestyle);
		
		rtext = r;
		
		InsetRect (&rtext, 1, 1);
	
		centerstring (rtext, bs);
	
		popstyle ();
		
		popclip ();
		
		if (flbitmap)
			closebitmap (w);
		
		popmacport ();
		}
	} /*appdrawmessage*/
	
	
void appsetmessage (hdlappwindow appwindow, bigstring bs) {
	
	hdlappwindow ha = appwindow;
	
	if (ha == nil)
		return;
		
	if (app.hasmessagearea) {
		
		Handle htext;
		
		/*8/29/94 DW: don't do anything if the message hasn't changed*/ {
			
			bigstring bscurrentmessage;
			
			texthandletostring ((**ha).hmessage, bscurrentmessage);
			
			if (equalstrings (bscurrentmessage, bs)) /*nothing to do*/
				return;
			}
	
		disposehandle ((**ha).hmessage);
		
		if (stringlength (bs) == 0)
			htext = nil;
		else
			newtexthandle (bs, &htext);
	
		(**ha).hmessage = htext;
	
		appdrawmessage (ha);
		}
	} /*appsetmessage*/
	
	
void appmessageclick (hdlappwindow appwindow) {
	
	if ((**appwindow).msgclickcallback != nil)
		(*(**appwindow).msgclickcallback) ();
	} /*appmessageclick*/
	
	
	
	