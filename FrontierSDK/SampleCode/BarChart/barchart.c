
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <applet.h>

/*
these are the IAC messages that BarChart can receive from Frontier scripts.

for the other side of this interface, check out system.verbs.apps.BarChart in
Frontier.root.
*/
	#define setbarvaluetoken 	'sval'
	#define getbarvaluetoken 	'gval'
	#define setbarlabeltoken 	'slab'
	#define getbarlabeltoken 	'glab'
	#define setbartoken			'sbar'
	#define addbartoken 		'abar'
	#define setunitstoken		'sunt'
	#define setbarcounttoken	'scnt'
	#define getbarcounttoken	'gcnt'
	#define updatetoken			'updt'
	

#define minwindowwidth 100 /*for zooming windows*/

#define labelcolor blackindex /*draw labels in black, other colors look flaky*/

#define firstbarcolor 8 /*start of a set of bold colors*/

RGBColor backgroundcolor = {52428, 65535, 52428}; /*a soft green color*/


#define maxbars 25

typedef long tyvalues [maxbars]; /*holds an array worth of values*/

#define maxlabelchars 32


typedef struct tybarrecord {
	
	boolean flhasbeenset: 1; /*avoid displaying if it's never been assigned to*/
	
	boolean fldirty: 1; /*the bar needs updating, watch for this in the idle callback*/
	
	long value;
	
	char label [maxlabelchars];
	} tybarrecord;


typedef struct tybarchartrecord {
	
	short versionnumber; /*this structure is saved on disk*/
	
	PicHandle backgroundpicture; /*picture displayed behind bars*/
	
	bigstring barunits; /*concatenated at the end of every displayed value*/
	
	short ctbars; /*number of bars in the chart*/
	
	long minvalue, maxvalue; /*the range of values*/
	
	short pixelsbetweenbars; /*how much space between the bars?*/
	
	short onebarwidth; /*how wide is one of the bars?*/
	
	short maxbarheight; /*max height in pixels of a bar*/
	
	short horizbaseline; /*the bottom edge of all the bars*/
	
	short vertbaseline; /*the left edge of the first bar*/
	
	short labelfont, labelsize, labelstyle; /*how are labels drawn?*/
	
	short labelbaseline; /*the horiz baseline for bar labels*/
	
	short labellineheight; /*vertical pixels for a label*/
	
	tybarrecord bars [maxbars];
	} tybarchartrecord, *ptrbarchartrecord, **hdlbarchartrecord;
	
	
bigstring bsbarchartsearch; /*see selectbarchartwindow*/

#define pictmargin 20 /*leave this much room on all sides of chart*/

boolean flbitmap = false; /*true if an offscreen bitmap is open*/




static boolean isFrontProcess () {
	
	ProcessSerialNumber currentprocess, frontprocess;
	Boolean fl;
	
	GetCurrentProcess (&currentprocess);
	
	GetFrontProcess (&frontprocess);
	
	SameProcess (&currentprocess, &frontprocess, &fl);
	
	return (fl);
	} /*isFrontProcess*/
	
	
static boolean isActiveWindow (hdlappwindow appwindow) {
	
	if (!isFrontProcess ())
		return (false);
	
	return (FrontWindow () == (**appwindow).macwindow);
	} /*isActiveWindow*/
	
	
static RGBColor *getBackgroundColor (hdlappwindow appwindow) {
	
	if (isActiveWindow (appwindow))
		return (&backgroundcolor);
	else
		return (&whitecolor);
	} /*getBackgroundColor*/
	
	
static void pushlabelstyle (void) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	
	pushstyle ((**hb).labelfont, (**hb).labelsize, (**hb).labelstyle);
	} /*pushlabelstyle*/
	
	
static boolean setchartconsts (void) {
	
	/*
	set values for the computed and constant fields of the barchart record.
	
	assume non-computed fields have been set and are accurate.
	*/
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	hdlappwindow ha = app.appwindow;
	short totalwidth;
	short ctbars = (**hb).ctbars;
	Rect r;
	
	(**hb).pixelsbetweenbars = 10;
	
	(**hb).labelfont = geneva;
	
	(**hb).labelsize = 9;
	
	(**hb).labelstyle = bold;
	
	pushlabelstyle ();
	
	(**hb).labellineheight = globalfontinfo.ascent + globalfontinfo.descent;
	
	popstyle ();
	
	r = (**ha).windowrect;
	
	totalwidth = (**ha).windowhorizpixels - (2 * pictmargin);
	
	totalwidth -= (ctbars + 1) * (**hb).pixelsbetweenbars;
	
	if (ctbars <= 0) 
		(**hb).onebarwidth = 0; /*defensive driving*/
	else
		(**hb).onebarwidth = totalwidth / ctbars;
	
	(**hb).labelbaseline = r.bottom - pictmargin;
	
	(**hb).horizbaseline = r.bottom - pictmargin - (2 * (**hb).labellineheight);
	
	(**hb).maxbarheight = (**hb).horizbaseline - r.top - pictmargin;
	
	(**hb).vertbaseline = r.left + pictmargin;
	
	return (true);
	} /*setchartconsts*/
	
	
static short getbarleftedge (short barnum) {

	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;

	return ( 
		(**hb).vertbaseline + 
		
		((**hb).pixelsbetweenbars * (barnum + 1)) + 
		
		((**hb).onebarwidth * barnum));
	} /*getbarleftedge*/
	

static boolean drawbar (short barnumber) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	hdlappwindow ha = app.appwindow;
	short barnum = barnumber;
	short maxbarheight = (**hb).maxbarheight;
	long value = (**hb).bars [barnum].value;
	long maxvalue = (**hb).maxvalue;
	short whitearea;
	Rect rbar, rwindow;
	RGBColor rgb;
	boolean flclosebitmap = false;
	
	if (!(**hb).bars [barnum].flhasbeenset)
		return (true);
		
	rwindow = (**ha).windowrect;
	
	rbar.left = getbarleftedge (barnum);
		
	rbar.right = rbar.left + (**hb).onebarwidth;
	
	rbar.bottom = (**hb).horizbaseline;
	
	rbar.top = rbar.bottom - maxbarheight;
	
	if (!flbitmap) {
		
		flbitmap = appopenbitmap (rbar, ha);
		
		flclosebitmap = true;
		}
		
	oldclutconverter (firstbarcolor + barnumber, &rgb);
	
	pushforecolor (&rgb); 
	
	pushbackcolor (getBackgroundColor (ha));
	
	EraseRect (&rbar);
	
	if (maxvalue == 0) /*avoid divide by zero error*/
		whitearea = 0;
		
	else { /*do arithmetic with longs*/
		
		long l1 = maxvalue - value;
		long l2 = maxbarheight;
		long l3 = maxvalue;
		long l4 = (l1 * l2) / l3;
		
		whitearea = (short) l4;
		}
	
	if (whitearea > 0)
		rbar.top += whitearea;
	
	if (isActiveWindow (ha))
		FillRect (&rbar, &quickdrawglobal (gray));
	else 
		FillRect (&rbar, &quickdrawglobal (ltGray));		
	
	pushpen ();
	
	PenPat (&quickdrawglobal (black));

	FrameRect (&rbar);
	
	poppen ();
	
	popbackcolor ();
	
	popforecolor ();
	
	if (flclosebitmap) {
		
		appclosebitmap (ha);
		
		flbitmap = false;
		}
	
	return (true);
	} /*drawbar*/
	
	
static boolean drawlabel (short barnumber) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	hdlappwindow ha = app.appwindow;
	short lh = (**hb).labellineheight;
	bigstring bs;
	Rect r;
	boolean flclosebitmap = false;
	
	if (!(**hb).bars [barnumber].flhasbeenset)
		return (true);
		
	copystring ((**hb).bars [barnumber].label, bs);
	
	r.left = getbarleftedge (barnumber);
		
	r.right = r.left + (**hb).onebarwidth;
	
	r.bottom = (**hb).labelbaseline - lh;
	
	r.top = r.bottom - lh;
	
	if (!flbitmap) {
		
		Rect rbitmap = r;
		
		rbitmap.bottom += lh;
		
		flbitmap = appopenbitmap (rbitmap, ha);
		
		flclosebitmap = true;
		}
		
	pushforecolor (&blackcolor); 
	
	pushbackcolor (getBackgroundColor (ha));
		
	pushlabelstyle ();
	
	EraseRect (&r);
	
	centerstring (r, bs);
	
	r.bottom += lh;
	
	r.top += lh;
	
	NumToString ((**hb).bars [barnumber].value, bs);
	
	pushstring ((**hb).barunits, bs);
		
	EraseRect (&r);
	
	centerstring (r, bs);
	
	popstyle ();
	
	popbackcolor ();
	
	popforecolor ();
	
	if (flclosebitmap) {
		
		appclosebitmap (ha);
		
		flbitmap = false;
		}
	
	return (true);
	} /*drawlabel*/
	
	
static void drawbarchart (void) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	short i;
	
	for (i = 0; i < (**hb).ctbars; i++) {
		
		drawbar (i);
		
		drawlabel (i);
		} /*for*/
	} /*drawbarchart*/
	

static void drawfreemem (void) {
	
	Rect r;
	Str255 s;
	
	pushbackcolor (getBackgroundColor (app.appwindow));
	
	r = (**app.appwindow).windowrect;
	
	r.top = r.bottom - 15;
	
	r.right -= 20;
	
	EraseRect (&r);
	
	NumToString (FreeMem () / 1024, s);
	
	MoveTo (r.left + 3, r.bottom - 3);
	
	setfontsizestyle (geneva, 9, 0);
	
	DrawString (s);
	
	DrawString ("\pK");
	
	popbackcolor ();
	} /*drawfreemem*/
	
	
static boolean setminmax (void) {
	
	/*
	examine the values array, and reset the min and max, if necessary.
	
	return true if at least one changed, false otherwise.
	*/
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	long newmax, newmin;
	long val;
	short i;
	
	newmax = intminusinfinity; /*assume only short values*/
	
	for (i = 0; i < (**hb).ctbars; i++) {
		
		val = (**hb).bars [i].value;
		
		if (val > newmax)
			newmax = val;
		} /*for*/
			
	newmin = intinfinity; /*assume only short values*/
	
	for (i = 0; i < (**hb).ctbars; i++) {
		
		val = (**hb).bars [i].value;
		
		if (val < newmin)
			newmin = val;
		} /*for*/
			
	if ((newmax != (**hb).maxvalue) || (newmin != (**hb).minvalue)) {
		
		(**hb).maxvalue = newmax;
		
		(**hb).minvalue = newmin;
		
		return (true);
		}
	
	return (false); /*no change*/
	} /*setminmax*/
	
	
static void smashchartdisplay (boolean flerase) {

	hdlappwindow ha = app.appwindow;

	setchartconsts (); /*reset all the drawing parameters*/
	
	setminmax (); /*min and max might have changed*/
	
	invalappwindow (ha, flerase);
	
	updateappwindow (ha);
	} /*smashchartdisplay*/
	

static boolean setbarunits (bigstring bs) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;

	copystring (bs, (**hb).barunits);
	
	smashchartdisplay (false); /*redraw the whole thing*/
	
	(**app.appwindow).flmadechanges = true;
	
	return (true);
	} /*setbarunits*/
	
	
static boolean setbarvalue (short barnumber, long barvalue) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;

	if ((barnumber < 0) || (barnumber >= (**hb).ctbars)) /*defensive driving*/
		return (false);
	
	(**hb).bars [barnumber].value = barvalue;
	
	(**hb).bars [barnumber].flhasbeenset = true;
	
	if (setminmax ())
		smashchartdisplay (false); /*redraw the whole thing*/
		
	else {
		drawbar (barnumber);
	
		drawlabel (barnumber);
		}
	
	(**app.appwindow).flmadechanges = true;

	return (true);
	} /*setbarvalue*/
	

static boolean getbarvalue (short barnumber, long *barvalue) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;

	*barvalue = 0; /*default returned value*/
	
	if ((barnumber < 0) || (barnumber >= (**hb).ctbars)) /*defensive driving*/
		return (false);
	
	*barvalue = (**hb).bars [barnumber].value;
	
	return (true);
	} /*getbarvalue*/
	
	
static boolean setallvalues (short ct, tyvalues vals) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	short i;
	
	for (i = 0; i < ct; i++) {
	
		(**hb).bars [i].value = vals [i];
		
		(**hb).bars [i].flhasbeenset = true;
		} /*for*/
		
	setminmax ();
	
	smashchartdisplay (false); /*redraw the whole thing*/
	
	(**app.appwindow).flmadechanges = true;

	return (true);
	} /*setallvalues*/
	
	
static boolean setbarlabel (short barnumber, bigstring bslabel) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	
	if ((barnumber < 0) || (barnumber >= (**hb).ctbars)) /*defensive driving*/
		return (false);
		
	copystring (bslabel, (**hb).bars [barnumber].label);
	
	(**hb).bars [barnumber].flhasbeenset = true;
	
	drawlabel (barnumber);
	
	(**app.appwindow).flmadechanges = true;
	
	return (true);
	} /*setbarlabel*/
	

static boolean setbar (short barnumber, bigstring bslabel, long barvalue) {
	
	/*
	sets both the label and the value for the indicated bar.
	
	this is provided so that a single IAC call can set both.
	*/
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;

	if ((barnumber < 0) || (barnumber >= (**hb).ctbars)) /*defensive driving*/
		return (false);
	
	if (!setbarlabel (barnumber, bslabel))
		return (false);
		
	return (setbarvalue (barnumber, barvalue));
	} /*setbar*/
	
	
static short addbar (bigstring bslabel, long barvalue) {
	
	/*
	returns the number of the new bar, -1 if it failed.
	*/
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	short barnumber = (**hb).ctbars;
	
	(**hb).ctbars++;
	
	setbarvalue (barnumber, barvalue);
	
	if (!setbarlabel (barnumber, bslabel)) {
		
		(**hb).ctbars--;
		
		return (-1);
		}
	
	smashchartdisplay (true);
	
	(**app.appwindow).flmadechanges = true;

	return (barnumber);
	} /*addbar*/
	

static boolean getbarlabel (short barnumber, bigstring bslabel) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	
	setemptystring (bslabel); /*default returned value*/
	
	if ((barnumber < 0) || (barnumber >= (**hb).ctbars)) /*defensive driving*/
		return (false);
	
	copystring ((**hb).bars [barnumber].label, bslabel);
	
	return (true);
	} /*getbarlabel*/
	

static boolean resetbarchartwindow (short ctbars) {
	
	(**(hdlbarchartrecord) app.appdata).ctbars = ctbars;
	
	smashchartdisplay (false);
	
	(**app.appwindow).flmadechanges = true;

	return (true);
	} /*resetbarchartwindow*/
	
	
static boolean bcnewrecord (void) {
	
	hdlbarchartrecord hb;
	short i;
	
	if (!newclearhandle (longsizeof (tybarchartrecord), (Handle *) &app.appdata))
		return (false);
		
	hb = (hdlbarchartrecord) app.appdata;
	
	(**hb).versionnumber = 1;
	
	(**hb).ctbars = 0;
	
	(**hb).minvalue = 0;
	
	(**hb).maxvalue = 0;
	
	(**hb).pixelsbetweenbars = 10;
	
	for (i = 0; i < maxbars; i++) {
		
		(**hb).bars [i].value = 0;
		
		setstringlength ((**hb).bars [i].label, 0);
		} /*for*/
	
	setchartconsts ();
	
	return (true);
	} /*bcnewrecord*/


static boolean bcdisposerecord (void) {
	
	disposehandle ((Handle) app.appdata);
		
	return (true);
	} /*bcdisposerecord*/
	
	
static boolean bcadjustcursor (void) {

	arrowcursor (); /*BarChart's "adjust cursor" is pretty simple*/
	
	return (true);
	} /*bcadjustcursor*/


static boolean bcwindowresize (void) {

	setchartconsts (); /*reset all computed values*/
	
	return (true);
	} /*bcwindowresize*/
	

static boolean bcactivate (boolean flactive) {
	
	hdlappwindow ha = app.appwindow;
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	
	if (hb == nil) /*no data, it can't be our window*/
		return (false);
		
	invalappwindow (ha, false);
		
	return (true);
	} /*bcactivate*/
	
	
static boolean bcupdate (void) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	hdlappwindow ha = app.appwindow;
	Rect rcontent = (**ha).contentrect;
	
	if (hb == nil) /*no data, it can't be our window*/
		return (false);
		
	flbitmap = appopenbitmap ((**ha).windowrect, ha);
	
	pushbackcolor (getBackgroundColor (ha));
	
	/*erase everything but the size box -- new in 3.0*/ {
		
		Rect r = rcontent;
		short sizegrow = 16;
		
		r.right -= sizegrow;
		
		r.bottom -= sizegrow;
		
		EraseRect (&r);
	
		r = rcontent;
		
		r.top = r.bottom - sizegrow;
		
		r.right -= sizegrow;
		
		EraseRect (&r);
		
		r = rcontent;
		
		r.left = r.right - sizegrow;
		
		r.bottom -= sizegrow;
		
		EraseRect (&r);
		}
	
	DrawPicture ((**hb).backgroundpicture, &rcontent);
	
	drawbarchart ();
	
	drawfreemem ();
	
	popbackcolor ();
	
	if (flbitmap)
		appclosebitmap (ha);
		
	flbitmap = false;
		
	return (true);
	} /*bcupdate*/
	
	
static void allbarsdirty (void) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	short i;
	
	for (i = 0; i < (**hb).ctbars; i++) 
		(**hb).bars [i].fldirty = true;
	} /*bcidle*/
	

static boolean setbarvalueverb (void) {
	
	/*
	verb that sets the value of one of the bars in the target window.
	*/
	
	short barnumber;
	long barvalue;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &barnumber))
		return (false);
		
	barnumber--; /*our IAC interface is 1-based, internally we're 0-based*/
		
	if (!IACgetlongparam ('vbar', &barvalue))
		return (false);
		
	return (IACreturnboolean (setbarvalue (barnumber, barvalue)));
	} /*setbarvalueverb*/
	

static boolean getbarvalueverb (void) {
	
	/*
	verb that returns the value of one of the bars in the target window.
	*/
	
	short barnumber;
	long barvalue;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &barnumber))
		return (false);
		
	barnumber--; /*our IAC interface is 1-based, internally we're 0-based*/
		
	getbarvalue (barnumber, &barvalue);
		
	return (IACreturnlong (barvalue));
	} /*getbarvalueverb*/
	

static boolean setbarverb (void) {
	
	/*
	verb that sets both the value and label of one of the bars in the
	target window.
	*/
	
	short barnumber;
	bigstring bslabel;
	long barvalue;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &barnumber))
		return (false);
		
	barnumber--; /*our IAC interface is 1-based, internally we're 0-based*/
		
	if (!IACgetstringparam ('lbar', bslabel))
		return (false);
		
	if (!IACgetlongparam ('vbar', &barvalue))
		return (false);

	return (IACreturnboolean (setbar (barnumber, bslabel, barvalue)));
	} /*setbarverb*/
	
	
static boolean setbarlabelverb (void) {
	
	/*
	verb that sets the label of one of the bars in the target window.
	*/
	
	short barnumber;
	bigstring bslabel;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &barnumber))
		return (false);
		
	barnumber--; /*our IAC interface is 1-based, internally we're 0-based*/
		
	if (!IACgetstringparam ('lbar', bslabel))
		return (false);
		
	return (IACreturnboolean (setbarlabel (barnumber, bslabel)));
	} /*setbarlabelverb*/
	
	
static boolean addbarverb (void) {
	
	/*
	verb that adds a new bar to the target window with indicated label and
	value.
	*/
	
	bigstring bslabel;
	long barvalue;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetstringparam (keyDirectObject, bslabel))
		return (false);
		
	if (!IACgetlongparam ('vbar', &barvalue))
		return (false);
		
	return (IACreturnshort (addbar (bslabel, barvalue) + 1));
	} /*addbarverb*/
	
	
static boolean setbarcountverb (void) {
	
	/*
	changes the number of bars in the target window.
	*/
	
	short ctbars;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &ctbars))
		return (false);
		
	return (IACreturnboolean (resetbarchartwindow (ctbars)));
	} /*setbarcountverb*/
	

static boolean getbarcountverb (void) {
	
	/*
	returns the number of bars in the target window.
	*/
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	return (IACreturnshort ((**(hdlbarchartrecord) app.appdata).ctbars));
	} /*getbarcountverb*/
	

static boolean setunitsverb (void) {
	
	/*
	sets the units string of the target window.  this string is displayed
	with the value of each of the bars.
	*/
	
	bigstring bs;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
	
	setbarunits (bs);
		
	return (IACreturnboolean (true));
	} /*setunitsverb*/
	
	
static boolean getbarlabelverb (void) {
	
	/*
	returns the label of one of the bars in the target window.  you provide
	us with a bar number.
	*/
	
	short barnumber;
	bigstring barlabel;
	
	if (!settargetglobals ()) /*this verb requires an open barchart window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &barnumber))
		return (false);
		
	barnumber--; /*our IAC interface is 1-based, internally we're 0-based*/
		
	getbarlabel (barnumber, barlabel);
		
	return (IACreturnstring (barlabel));
	} /*getbarlabelverb*/
	

static boolean bciacmessage (void) {
	
	switch (IACgetverbtoken ()) {
		
		case setbarvaluetoken:
			setbarvalueverb (); break;
		
		case getbarvaluetoken:
			getbarvalueverb (); break;
			
		case setbarlabeltoken:
			setbarlabelverb (); break;
			
		case setbartoken:
			setbarverb (); break;
			
		case getbarlabeltoken:
			getbarlabelverb (); break;
		
		case addbartoken:
			addbarverb (); break;
			
		case setunitstoken:
			setunitsverb (); break;
				
		case setbarcounttoken:
			setbarcountverb (); break;		
			
		case getbarcounttoken:
			getbarcountverb (); break;	
			
		case updatetoken:
			drawbarchart (); break;
			
		default:
			IACnothandlederror (); break;
		} /*switch*/
		
	return (false);
	} /*bciacmessage*/
	
	
static boolean bcpack (hpacked) Handle *hpacked; {
	
	hdlbarchartrecord hb;
	
	if (!copyhandle (app.appdata, hpacked)) 
		return (false);
		
	hb = (hdlbarchartrecord) *hpacked;
	
	return (true);
	} /*bcpack*/
	

static boolean bcunpack (hpacked) Handle hpacked; {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) hpacked;
	hdlbarchartrecord hcopy;
	
	if ((**hb).versionnumber != 1)
		return (false);
		
	if (!copyhandle ((Handle) hb, (Handle *) &hcopy))
		return (false);
		
	disposehandle (app.appdata);
	
	app.appdata = (Handle) hcopy;
	
	setchartconsts (); /*reset all computed values*/
		
	return (true);
	} /*bcunpack*/
	
	
static boolean bcgetpicture (hpicture) PicHandle *hpicture; {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	hdlappwindow ha = app.appwindow;
	Rect r = (**ha).contentrect;
	
	*hpicture = nil; /*return value if error*/
	
	if (hb == nil) /*no data, it can't be our window*/
		return (false);
	
	*hpicture = OpenPicture (&r);
	
	ClipRect (&r);
	
	drawbarchart ();
	
	ClosePicture ();
	
	return (true);
	} /*bcgetpicture*/
	

static boolean bcputpicture (hpicture) PicHandle hpicture; {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	
	(**hb).backgroundpicture = hpicture;
	
	bcupdate ();
	
	return (true);
	} /*bcputpicture*/


static boolean bcopenprint (void) {
	
	app.printinfo.ctpages = 1;
	
	return (true);
	} /*bcopenprint*/


static boolean bccloseprint (void) {
	
	setchartconsts (); /*restore drawing parameters*/
	
	return (true);
	} /*bccloseprint*/


static boolean bcprintpage (pagenumber) short pagenumber; {
	
	setchartconsts (); /*reset all the drawing parameters*/
	
	drawbarchart ();
	
	return (true);
	} /*bcprintpage*/
	
	
static boolean bchaveselection (void) {
	
	return (false); /*no selection in BarChart*/
	} /*bchaveselection*/
	
	
static boolean bcselectall (void) {
	
	return (false); /*no selection in BarChart*/
	} /*bcselectall*/
	
	
static boolean bcgetcontentsize (void) { 
	
	/*
	called when a BarChart window is zoomed. we return the optimal size
	for the window. horizontally, it's a function of the number of bars
	and the width of their labels. vertically, we make it 3/5 of the 
	window width.
	*/

	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	hdlappwindow ha = app.appwindow;
	short i;
	bigstring bs;
	short maxwidth = 0;
	short width;
	
	pushlabelstyle ();
	
	for (i = 0; i < (**hb).ctbars; i++) {
		
		short x = StringWidth ((ConstStr255Param) (**hb).bars [i].label) + 4;
		
		maxwidth = max (maxwidth, x);
		
		NumToString ((**hb).bars [i].value, bs);
	
		pushstring ((**hb).barunits, bs);
		
		x = StringWidth (bs) + 4;
		
		maxwidth = max (maxwidth, x);
		} /*for*/
	
	width = 	
		((**hb).vertbaseline * 2) + 
		
		(maxwidth * (**hb).ctbars) + 
		
		((**hb).pixelsbetweenbars * ((**hb).ctbars - 1));
	
	(**ha).zoomwidth = max (width, minwindowwidth);
	
	(**ha).zoomheight = ((**ha).zoomwidth * 3) / 5;
	
	return (true);
	} /*bcgetcontentsize*/
	

static boolean bcidle (void) {
	
	hdlbarchartrecord hb = (hdlbarchartrecord) app.appdata;
	short i;
	
	setcursortype (cursorisarrow);
	
	if (hb == nil) /*no window open*/
		return (true);
	
	for (i = 0; i < (**hb).ctbars; i++) {
	
		if ((**hb).bars [i].fldirty) {
			
			drawbar (i);
	
			drawlabel (i);
			}
		
		(**hb).bars [i].fldirty = false;
		} /*for*/
	
	return (true);
	} /*bcidle*/
	

void main (void) {
	
	clearbytes (&app, longsizeof (app)); /*init all fields to 0*/
	
	app.creator = 'BARC';
	
	app.filetype = 'CHRT';
	
	app.usecolor = true;
	
	app.resizeable = true;
	
	app.eraseonresize = true;
	
	app.haswindowmenu = true; /*3.0*/
	
	app.usetempmemory = true; /*3.0*/
	
	app.newrecordcallback = &bcnewrecord;
	
	app.disposerecordcallback = &bcdisposerecord;
	
	app.idlecallback = &bcadjustcursor;
	
	app.activatecallback = (tyappbooleancallback) &bcactivate;
	
	app.switchcallback = (tyappbooleancallback) &bcactivate; /*3.0*/
	
	app.updatecallback = &bcupdate;
	
	app.windowresizecallback = &bcwindowresize;
	
	app.iacmessagecallback = &bciacmessage;
	
	app.packcallback = &bcpack;
	
	app.unpackcallback = &bcunpack;
	
	app.getpictcallback = (tyapphandleptrcallback) &bcgetpicture;
	
	app.putpictcallback = (tyapphandlecallback) &bcputpicture;
	
	app.openprintcallback = &bcopenprint;
	
	app.printpagecallback = (tyappshortcallback) &bcprintpage;
	
	app.closeprintcallback = &bccloseprint;
	
	app.haveselectioncallback = &bchaveselection;
	
	app.selectallcallback = &bcselectall;
	
	app.idlecallback = &bcidle;
	
	app.getcontentsizecallback = &bcgetcontentsize; 

	runapplet ();
	} /*main*/
	
	
	