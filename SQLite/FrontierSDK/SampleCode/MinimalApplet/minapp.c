
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <applet.h>
#include <applettextedit.h>




#define uppercasetoken	'ucas'
#define lowercasetoken	'lcas'

#define fastinserttoken	'fins'


typedef struct tydata { /*what app.appdata points to*/
	
	boolean flactive;
	
	hdleditrecord heditrecord;
	
	PicHandle macpicture; /*used for printing*/
	} tydata, **hdldata;
	
	
typedef struct tyoptionsrecord { /*information we save in every data file*/
	
	short selstart, selend;
	
	char waste [64];
	} tyoptionsrecord, **hdloptionsrecord;
	
	
	
#define wordwrapping true /*if false, it's a script editor; true, text editor*/



	
static void madechanges (void) {
	
	hdlappwindow ha = app.appwindow;
	
	if (ha != nil) {
	
		(**ha).flmadechanges = true;
	
		/*(**ha).flresetscrollbars = true;*/
		}
	} /*madechanges*/
	
	
static boolean newrecord (void) {
	
	hdlappwindow ha = app.appwindow;
	hdldata hdata = nil;
	hdleditrecord heditrecord = nil;
	bigstring bs;
	boolean fl;
	
	if (!newclearhandle (longsizeof (tydata), (Handle *) &hdata))
		return (false);
	
	setstringlength (bs, 0);
	
	pushstyle ((**ha).defaultfont, (**ha).defaultsize, (**ha).defaultstyle);
	
	fl = editnewbuffer ((**ha).contentrect, wordwrapping, bs, &heditrecord);
		
	popstyle ();
	
	if (!fl)
		goto error;
		
	(**heditrecord).flwindowbased = true;
		
	editautoscroll (heditrecord); /*automatically scroll for drags across bottom of window*/
		
	(**hdata).heditrecord = heditrecord;
	
	app.appdata = (Handle) hdata;
		
	return (true);
	
	error:
	
	disposehandle ((Handle) hdata);
	
	editdispose (heditrecord);
	
	return (false);
	} /*newrecord*/


static boolean disposerecord (void) {
	
	hdldata hd = (hdldata) app.appdata;

	editdispose ((**hd).heditrecord);
	
	disposehandle ((Handle) hd);
		
	return (true);
	} /*disposerecord*/
	
	
static boolean activate (boolean flactive) {
	
	hdldata hd = (hdldata) app.appdata;
	
	(**hd).flactive = flactive;
	
	editactivate ((**hd).heditrecord, flactive);
	
	return (true);
	} /*activate*/
	
	
static boolean update (void) {
	
	hdldata hd = (hdldata) app.appdata;	

	editupdate ((**hd).heditrecord);
	
	return (true);
	} /*update*/


static boolean keystroke (void) { 
	
	hdldata hd = (hdldata) app.appdata;	
			
	editkeystroke (keyboardstatus.chkb, (**hd).heditrecord);
	
	madechanges ();
	
	return (true);
	} /*keystroke*/
	

static boolean mousedown (void) {
	
	hdldata hd = (hdldata) app.appdata;	
		
	editclick (mousestatus.localpt, keyboardstatus.flshiftkey, (**hd).heditrecord);
	
	return (true);
	} /*mousedown*/
	

static boolean packrecord (Handle *hpacked) {
	
	hdldata hd = (hdldata) app.appdata;	
		
	return (editgettexthandlecopy ((**hd).heditrecord, hpacked));
	} /*packrecord*/
	
	
static boolean unpackrecord (Handle hpacked) {
	
	hdldata hd = (hdldata) app.appdata;	
	hdlappwindow ha = app.appwindow;
		
	if (!editsettexthandle ((**hd).heditrecord, hpacked, false))
		return (false);
		
	editsetrect ((**hd).heditrecord, (**ha).contentrect);
		
	editscroll (
		(**hd).heditrecord, 
		
		-getscrollbarcurrent ((**ha).horizbar), 
		
		-getscrollbarcurrent ((**ha).vertbar));
	
	return (true);
	} /*unpackrecord*/
	
	
static boolean getoptions (Handle *hoptions) {
	
	hdldata hd = (hdldata) app.appdata;	
	tyoptionsrecord x;
	
	clearbytes (&x, longsizeof (x));
	
	editgetselection ((**hd).heditrecord, &x.selstart, &x.selend);
	
	return (newfilledhandle (&x, longsizeof (x), hoptions));
	} /*getoptions*/
	
	
static boolean putoptions (Handle hoptions) {
	
	hdldata hd = (hdldata) app.appdata;	
	tyoptionsrecord x;
	
	moveleft (*hoptions, &x, longsizeof (x));
	
	editsetselection ((**hd).heditrecord, x.selstart, x.selend);
	
	return (true);
	} /*putoptions*/
	
	
static boolean pastetext (Handle htext) {
	
	hdldata hd = (hdldata) app.appdata;	
	
	madechanges ();
		
	return (editreplacehandle (htext, (**hd).heditrecord));
	} /*pastetext*/
	
	
static boolean copytext (Handle *htext) {
	
	hdldata hd = (hdldata) app.appdata;	
		
	return (editgetselectedtexthandle ((**hd).heditrecord, htext));
	} /*copytext*/
	

static boolean deleteselectedtext (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	bigstring bs;
	
	setstringlength (bs, 0);
	
	editreplace (bs, (**hd).heditrecord);
	
	madechanges ();
	
	return (true);
	} /*deleteselectedtext*/
	
	
static boolean windowresize (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	
	editsetrect ((**hd).heditrecord, (**app.appwindow).contentrect);
	
	return (true);
	} /*windowresize*/
	
	
static boolean scrollto (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	hdlappwindow ha = app.appwindow;
	
	editscrollto (
		(**hd).heditrecord, 
		
		getscrollbarcurrent ((**ha).horizbar), 
		
		getscrollbarcurrent ((**ha).vertbar));
		
	return (true);
	} /*scrollto*/
	
	
static boolean lowercaseverb (void) {

	hdldata hd = (hdldata) app.appdata;	
	Handle htext;
	short ctchars, i;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	editgettexthandle ((**hd).heditrecord, &htext);
	
	ctchars = GetHandleSize (htext);
	
	for (i = 0; i < ctchars; i++) {
		
		char ch = (*htext) [i];
		
		if ((ch >= 'A') && (ch <= 'Z'))
			(*htext) [i] = (char) ch + 32;
		} /*for*/
		
	editupdate ((**hd).heditrecord);
	
	madechanges ();
	
	IACreturnboolean (true);
	
	return (true);
	} /*lowercaseverb*/
	
	
static boolean uppercaseverb (void) {

	hdldata hd = (hdldata) app.appdata;	
	Handle htext;
	short ctchars, i;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	editgettexthandle ((**hd).heditrecord, &htext);
	
	ctchars = GetHandleSize (htext);
	
	for (i = 0; i < ctchars; i++) {
		
		char ch = (*htext) [i];
		
		if ((ch >= 'a') && (ch <= 'z'))
			(*htext) [i] = (char) ch - 32;
		} /*for*/
		
	editupdate ((**hd).heditrecord);
	
	madechanges ();
	
	IACreturnboolean (true);
	
	return (true);
	} /*uppercaseverb*/
	
	
static boolean iacmessage (void) {
	
	switch (IACgetverbtoken ()) {
		
		case lowercasetoken:
			lowercaseverb (); break;
			
		case uppercasetoken:
			uppercaseverb (); break;
		
		} /*switch*/
		
	return (true);
	} /*iacmessage*/
	
	
static boolean getpicture (PicHandle *hpicture) {
	
	hdldata hd = (hdldata) app.appdata;	
	Rect r;
	
	*hpicture = nil; /*return value if error*/
	
	if (hd == nil) /*no data, it can't be our window*/
		return (false);
	
	r.top = 0;
	
	r.left = 0;
	
	editgetbuffersize ((**hd).heditrecord, &r.bottom, &r.right);
	
	*hpicture = OpenPicture (&r);
	
	ClipRect (&r);
	
	editupdateport ((**hd).heditrecord, r, qd.thePort);
	
	ClosePicture ();
	
	return (true);
	} /*getpicture*/
	
	
static boolean pagesetup (void) {
	
	return (true);
	} /*pagesetup*/


static boolean openprint (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	Rect rpict, rpaper;
	PicHandle hpict;
	short ctpages, vpictpixels;
	
	getpicture (&hpict);
	
	(**hd).macpicture = hpict;
	
	rpict = (**hpict).picFrame;
	
	rpaper = app.printinfo.paperrect;
	
	vpictpixels = rpict.bottom - rpict.top;
	
	ctpages = vpictpixels / app.printinfo.vpagepixels;
	
	if (vpictpixels > (ctpages * app.printinfo.vpagepixels))
		ctpages++;
	
	app.printinfo.ctpages = ctpages;
	
	return (true);
	} /*openprint*/


static boolean printpage (short pagenumber) {
	
	hdldata hd = (hdldata) app.appdata;	
	PicHandle macpicture = (**hd).macpicture;
	short hoffset, voffset;
	Rect rpict, rpaper;
	
	rpict = (**macpicture).picFrame;
	
	rpaper = app.printinfo.paperrect;
	
	hoffset = rpaper.left - rpict.left;
	
	voffset = (rpaper.top - rpict.top) - ((pagenumber - 1) * app.printinfo.vpagepixels) - 1;
	
	SetOrigin (-hoffset, -voffset);
	
	ClipRect (&(*qd.thePort).portRect);
	
	DrawPicture (macpicture, &rpict);
	
	return (true);
	} /*printpage*/
	
	
static boolean closeprint (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	PicHandle hpict = (**hd).macpicture;
	
	if (hpict != nil) {
		
		KillPicture (hpict);
		
		(**hd).macpicture = nil;
		}
	
	return (true);
	} /*closeprint*/


static boolean haveselection (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	
	return (edithaveselection ((**hd).heditrecord));
	} /*haveselection*/
	
	
static boolean selectall (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	
	editselectall ((**hd).heditrecord);
		
	return (true);
	} /*selectall*/
	

static boolean setselectioninfo (void) {

	/*
	9/3/92 DW: added code to set x.fontnum, x.fontsize.
	*/
	
	hdlappwindow ha = app.appwindow;
	tyselectioninfo x = (**ha).selectioninfo;
	
	x.flcansetfont = true;
	
	x.flcansetsize = true;
	
	x.flcansetstyle = false;
	
	x.flcansetjust = false;	
	
	x.fldirty = false;
	
	x.fontnum = (**ha).defaultfont;
	
	x.fontsize = (**ha).defaultsize;
	
	(**ha).selectioninfo = x;
	
	return (true);
	} /*setselectioninfo*/
	
		
static boolean setfont (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	hdlappwindow ha = app.appwindow;
	tyselectioninfo x = (**ha).selectioninfo;
	
	if (x.fontnum != -1)
		(**ha).defaultfont = x.fontnum;
	
	if (x.fontsize != -1)
		(**ha).defaultsize = x.fontsize;
	
	editsetfont ((**hd).heditrecord, (**ha).defaultfont, (**ha).defaultsize);
		
	invalappwindow (ha, true);
		
	return (true);
	} /*setfont*/
	
	
static boolean getcontentsize (void) {
	
	hdldata hd = (hdldata) app.appdata;	
	hdlappwindow ha = app.appwindow;
	short height, width;
	
	pushstyle ((**ha).defaultfont, (**ha).defaultsize, (**ha).defaultstyle);
	
	editgetbuffersize ((**hd).heditrecord, &height, &width);
	
	popstyle ();
	
	(**ha).zoomheight = height;
	
	(**ha).zoomwidth = width;
	
	return (true);
	} /*getcontentsize*/
	
	
static boolean idle (void) {
	
	/*
	this code is guaranteed to be running when minapp is the current context,
	not from a fast event handler. so we process display oriented tasks 
	generated by fast event handlers in this callback.
	*/
	
	hdlappwindow ha = app.appwindow;
	hdldata hd = (hdldata) app.appdata;
	Point pt = mousestatus.localpt;
	hdleditrecord he;
	
	if (hd == nil)
		return (true);
		
	he = (**hd).heditrecord;
	
	if ((**he).flscrolled) {
	
		setscrollbarcurrent ((**ha).vertbar, (**he).vertcurrent);
		
		setscrollbarcurrent ((**ha).horizbar, (**he).horizcurrent);
		
		(**ha).flresetscrollbars = true; /*force max and min to be recalculated*/
		
		(**he).flscrolled = false; /*consume it*/
		}
	
	if (PtInRect (pt, &(**ha).contentrect))
		setcursortype (cursorisibeam);
	else
		setcursortype (cursorisarrow);
		
	editidle (he);
	
	return (true);
	} /*idle*/
	

void main (void) {

	clearbytes (&app, longsizeof (app)); /*init all fields to 0*/
	
	app.creator = 'MINA'; 
	
	app.filetype = 'TEXT';
	
	app.haswindowmenu = true; /*3.0*/
	
	app.usetempmemory = true; /*3.0*/
	
	app.resizeable = true;
	
	app.eraseonresize = true;
	
	app.horizscroll = true;
	
	app.vertscroll = true;
	
	app.usecolor = false;
	
	app.defaultfont = monaco;
	
	app.defaultsize = 9;
	
	app.defaultstyle = 0;
	
	app.newrecordcallback = &newrecord;
	
	app.disposerecordcallback = &disposerecord;
	
	app.idlecallback = &idle;
	
	app.activatecallback = (tyappbooleancallback) &activate;
	
	app.updatecallback = &update;
	
	app.iacmessagecallback = &iacmessage;
	
	app.packcallback = &packrecord;
	
	app.gettextcallback = &copytext;
	
	app.puttextcallback = &pastetext;
	
	app.clearcallback = &deleteselectedtext;
	
	app.unpackcallback = &unpackrecord;
	
	app.keystrokecallback = &keystroke;
	
	app.mousecallback = &mousedown;
	
	app.windowresizecallback = &windowresize;
	
	app.scrolltocallback = &scrollto;
	
	app.openprintcallback = &openprint;
	
	app.pagesetupcallback = &pagesetup;
	
	app.printpagecallback = (tyappshortcallback) &printpage;
	
	app.closeprintcallback = &closeprint;
	
	app.setselectioninfocallback = &setselectioninfo;
	
	app.haveselectioncallback = &haveselection;
	
	app.selectallcallback = &selectall;
	
	app.setfontcallback = &setfont;
	
	app.setsizecallback = &setfont;
	
	app.getcontentsizecallback = &getcontentsize;
	
	app.getoptionscallback = &getoptions;
	
	app.putoptionscallback = &putoptions;
	
	runapplet ();
	} /*main*/
	

