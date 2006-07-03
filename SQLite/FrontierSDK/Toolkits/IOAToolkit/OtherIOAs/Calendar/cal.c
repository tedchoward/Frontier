
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Packages.h>
#include <Script.h>
#include <appletdefs.h>
#include <ioa.h>


#define calendartype -1

#define tydate DateTimeRec
#define dayofweek dayOfWeek

#define daysinweek 7
#define monthsinyear 12
#define maxdaysinmonth 31

#define drawinset 5 /*inset 5 pixels all around*/


bigstring daystrings [daysinweek] = {
	
	"\pSunday", "\pMonday", "\pTuesday", "\pWednesday", 
	
	"\pThursday", "\pFriday", "\pSaturday"
	};

bigstring monthstrings [monthsinyear] = {

	"\pJanuary", "\pFebruary", "\pMarch", "\pApril", 
	
	"\pMay", "\pJune", "\pJuly", "\pAugust", "\pSeptember",
	
	"\pOctober", "\pNovember", "\pDecember" 
	};


typedef struct tydata {
	
	tydate date; /*the date that the cursor is on*/
	
	short ctdays; /*number of days in the month*/
	
	short ctweeks; /*number of weeks the month "touches"*/
	
	short dayoffirst; /*the day of the week of the first day of the month*/
	
	short dayoflast; /*the day of the week of the last day of the month*/
	
	short monthfont, monthsize, monthstyle;
	
	short labelfont, labelsize, labelstyle;
	
	short dayfont, daysize, daystyle;

	Rect rmonthyear, rdaysofweek, rbody;
	
	short colpixels;
	
	short bodyrowheight;
	
	boolean flactive;
	
	RGBColor framecolor;
	
	RGBColor textcolor;
	
	RGBColor fillcolor;
	
	RGBColor cellfillcolor;
	
	RGBColor cellcursorcolor;
	
	boolean invalwindowpending;
	} tydata, **hdldata;
	
	
static hdldata caldata = nil;

static hdlobject calobject = nil;

static FontInfo globalfontinfo;

static tydate mindate, maxdate;


static RGBColor whitecolor = {65535, 65535, 65535};

static RGBColor blackcolor = {0, 0, 0};

static RGBColor greencolor = {0, 32767, 0};

static RGBColor lightgreencolor = {39321, 65535, 39321};

static RGBColor yellowcolor = {64512, 62333, 1327};

	
#define ctstyle 5 /*we can remember font/size/styles up to 5 levels deep*/

static short topstyle = 0;

static struct {short fnum, fsize, fstyle;} stylestack [ctstyle];





static void setglobals (hdlobject h) {
	
	caldata = (hdldata) (**h).objectdata; 
	
	calobject = h;
	} /*setglobals*/
	
	
static void getmonthstring (short monthnum, bigstring bsmonth) {

	IOAcopystring (monthstrings [monthnum - 1], bsmonth);
	} /*getmonthstring*/
	
	
static void getdaystring (short daynum, bigstring bsday) {
	
	IOAcopystring (daystrings [daynum - 1], bsday);
	} /*getdaystring*/
	

static short daysinmonth (short monthnum, short year) {
	
	/*
	the Mac ROM has information on how many days there are in any given
	month.  but we have to trick it into telling us!
	*/
	
	tydate date;
	long secs;
	register short ctdays;
	register short month = monthnum;
	
	date.year = year;
	
	date.month = month;
	
	date.day = ctdays = 27;
	
	date.hour = date.minute = date.second = 0;
	
	Date2Secs (&date, &secs);
	
	while (true) {
		
		Secs2Date (secs, &date);
		
		if (date.month != month) 
			return (ctdays - 1);
			
		ctdays++;
		
		date.day++;
		
		Date2Secs (&date, &secs);
		} /*while*/
	} /*daysinmonth*/
	

static void allupper (bs) bigstring bs; {
	
	register char len = bs [0];
	register ptrchar p = (ptrchar) &bs [1];
	register char ch;
	
	while (len--) {
		
		ch = *p;
		
		if ((ch >= 'a') && (ch <= 'z'))
			*p -= 32;
			
		p++;
		} /*while*/
	} /*allupper*/
	
	
static void getmonthyearstring (tydate date, bigstring bs) {
	
	/*
	return a string like "September 1988"
	*/
	
	bigstring bsmonth, bsyear;
	
	getmonthstring (date.month, bs);
	
	IOApushstring ("\p ", bs);
	
	NumToString (date.year, bsyear);
	
	IOApushstring (bsyear, bs);
	
	allupper (bs);
	} /*getmonthyearstring*/
	

static void getfulldatestring (tydate date, bigstring bs) {
	
	/*
	return a string like "October 3, 1998"
	*/
	
	long secs;
	
	Date2Secs (&date, &secs);
	
	IUDateString (secs, longDate, bs);
	} /*getfulldatestring*/
	
	
static boolean equaltimes (tydate *dleft, tydate *dright) {
	
	tydate d1, d2;
	
	d1 = *dleft;
	
	d2 = *dright;
	
	return (
		
		(d1.year == d2.year) && (d1.month == d2.month) && (d1.day == d2.day) &&
		
		(d1.hour == d2.hour) && (d1.minute == d2.minute) && (d1.second == d2.second));
	} /*equaltimes*/
	

static boolean equalhours (tydate *dleft, tydate *dright) {
	
	tydate d1, d2;
	
	d1 = *dleft;
	
	d2 = *dright;
	
	return (
		
		(d1.year == d2.year) && (d1.month == d2.month) && (d1.day == d2.day) &&
		
		(d1.hour == d2.hour));
	} /*equalhours*/
	

static boolean equaldates (tydate *dleft, tydate *dright) {
	
	tydate d1, d2;
	
	d1 = *dleft;
	
	d2 = *dright;
	
	return ((d1.year == d2.year) && (d1.month == d2.month) && (d1.day == d2.day));
	} /*equaldates*/
	

static boolean datelessthan (tydate *dleft, tydate *dright) {
	
	tydate d1, d2;
	
	d1 = *dleft;
	
	d2 = *dright;
	
	if (d1.year > d2.year)
		return (false);
		
	if (d1.year < d2.year)
		return (true);
		
	if (d1.month > d2.month)
		return (false);
		
	if (d1.month < d2.month)
		return (true);
		
	if (d1.day > d2.day)
		return (false);
		
	if (d1.day < d2.day)
		return (true);
		
	if (d1.hour > d2.hour)
		return (false);
		
	if (d1.hour < d2.hour)
		return (true);
		
	if (d1.second > d2.second)
		return (false);
		
	if (d1.second < d2.second)
		return (true);
		
	return (false); /*dates are totally equal*/
	} /*datelessthan*/
	
	
static void fixdate (tydate *date) {
	
	/*
	by converting to seconds and then converting back into the record
	format we turn dates like December 32, 1988 to January 1, 1989.
	
	we also fill in the dayofweek field of a date record.
	
	also check to make sure that the date doesn't exceed the bounds of
	the Macintosh ROM routines.
	*/
	
	long secs;
	
	if (datelessthan (date, &mindate))
		*date = mindate;
		
	if (datelessthan (&maxdate, date))
		*date = maxdate;
	
	Date2Secs (date, &secs);
	
	Secs2Date (secs, date);
	} /*fixdate*/
	

static void tomorrow (tydate *date) {
	
	(*date).day++;
	
	fixdate (date);
	} /*tomorrow*/
	
	
static void yesterday (tydate *date) {
	
	(*date).day--;
	
	fixdate (date);
	} /*yesterday*/
	
	
static void nextweek (tydate *date) {
	
	register short i;
	
	for (i = 1; i <= daysinweek; i++)
		tomorrow (date);
	} /*nextweek*/
	

static void prevweek (tydate *date) {
	
	register short i;
	
	for (i = 1; i <= daysinweek; i++)
		yesterday (date);
	} /*prevweek*/
	

static void nextmonth (tydate *date) {
	
	register short month = (*date).month;
	
	if (month == monthsinyear) {
		
		month = 1;
		
		(*date).year++;
		}
	else
		month++;
		
	(*date).month = month;
	
	fixdate (date);
	} /*nextmonth*/
	

static void prevmonth (tydate *date) {
	
	register short month = (*date).month;
	
	if (month == 1) {
		
		month = monthsinyear;
		
		(*date).year--;
		}
	else
		month--;
		
	(*date).month = month;
	
	fixdate (date);
	} /*prevmonth*/
	

static void nextyear (tydate *date) {
	
	(*date).year++;
	
	fixdate (date);
	} /*nextyear*/
	
	
static void prevyear (tydate *date) {
	
	(*date).year--;
	
	fixdate (date);
	} /*prevyear*/
	
	
static void firstofmonth (tydate *date, short month, short year) {
	
	(*date).year = year;
	
	(*date).month = month;
	
	(*date).day = 1;
	
	(*date).hour = (*date).minute = (*date).second = 0;
	
	fixdate (date);
	} /*firstofmonth*/
	

static void lastofmonth (tydate *date, short month, short year) {
	
	(*date).year = year;
	
	(*date).month = month;
	
	(*date).day = daysinmonth (month, year);
	
	(*date).hour = (*date).minute = (*date).second = 0;
	
	fixdate (date);
	} /*lastofmonth*/
	

static short firstdayofmonth (short month, short year) {
	
	tydate date;
	
	firstofmonth (&date, month, year);
	
	return (date.dayofweek);
	} /*firstdayofmonth*/
	
	
static short lastdayofmonth (short month, short year) {
	
	tydate date;
	
	lastofmonth (&date, month, year);
	
	return (date.dayofweek);
	} /*lastdayofmonth*/
	
	
static short weeksinmonth (short month, short year) {
	
	/*
	returns the number of weeks that the month "touches".  this can be used
	to determine how many rows need to be drawn in a matrix that shows the
	month.
	*/
	
	register short ctdays, ctweeks;
	register short i;
	tydate date;
	
	firstofmonth (&date, month, year);
	
	ctdays = daysinmonth (month, year);
	
	ctweeks = 1;
	
	for (i = 1; i < ctdays; i++) {
	
		tomorrow (&date);
		
		if (date.dayofweek == 1) /*Sunday starts a new week*/
			ctweeks++;
		} /*for*/
	
	return (ctweeks);
	} /*weeksinmonth*/


static boolean pushstyle (short fnum, short fsize, short fstyle) {
	
	WindowPtr w = (**(**calobject).owningcard).drawwindow;
	
	if (topstyle >= ctstyle)
		return (false);
		
	stylestack [topstyle].fnum = (*w).txFont;
	
	stylestack [topstyle].fsize = (*w).txSize;
	
	stylestack [topstyle].fstyle = (*w).txFace;
	
	topstyle++;
	
	TextFont (fnum);
	
	TextSize (fsize);
	
	TextFace (fstyle);
	
	GetFontInfo (&globalfontinfo);
	
	return (true);
	} /*pushstyle*/
		

static boolean popstyle (void) {
	
	if (topstyle <= 0)
		return (false);
	
	topstyle--;
	
	TextFont (stylestack [topstyle].fnum);
	
	TextSize (stylestack [topstyle].fsize);
	
	TextFace (stylestack [topstyle].fstyle);
	
	GetFontInfo (&globalfontinfo);
	
	return (true);
	} /*popstyle*/


static void pushmonthstyle (void) {
	
	hdldata hd = caldata;	
	
	pushstyle ((**hd).monthfont, (**hd).monthsize, (**hd).monthstyle);
	} /*pushmonthstyle*/
	
	
static void pushlabelstyle (void) {
	
	hdldata hd = caldata;	
	
	pushstyle ((**hd).labelfont, (**hd).labelsize, (**hd).labelstyle);
	} /*pushlabelstyle*/
	
	
static void pushdaystyle (void) {
	
	hdldata hd = caldata;	
	
	pushstyle ((**hd).dayfont, (**hd).daysize, (**hd).daystyle);
	} /*pushdaystyle*/
	
	
static boolean stringtotime (bigstring bsdate, unsigned long *time) {
	
	/*
	9/13/91 dmb: use the script manager to translate a string to a 
	time in seconds since 12:00 AM 1904.
	
	if a date is provided, but no time, the time is 12:00 AM
	
	if a time is provided, but no date, the date is 1/1/1904.
	
	we return true if any time/date information was extracted.
	*/
	
	Ptr p = (Ptr) bsdate + 1;
	long n = stringlength (bsdate);
	long used = 0;
	DateCacheRecord cache;
	LongDateRec longdate;
	boolean flgotdate;
	boolean flgottime;
	DateTimeRec shortdate;
	
	*time = 0; /*default return value*/
	
	IOAclearbytes (&longdate, longsizeof (longdate));
	
	if (InitDateCache (&cache) != noErr) /*must do this before calling String2Date*/
		return (false);
	
	flgotdate = ((String2Date (p, n, &cache, &used, &longdate) & fatalDateTime) == 0);
	
	flgottime = ((String2Time (p + used, n - used, &cache, &used, &longdate) & fatalDateTime) == 0);
	
	if (!flgotdate && !flgottime) /*nuthin' doin'*/
		return (false);
	
	if (flgotdate) {
	
		shortdate.day = longdate.ld.day;
		
		shortdate.month = longdate.ld.month;
		
		shortdate.year = longdate.ld.year;
		}
	else {
		
		Secs2Date (0, &shortdate);
		
		/*
		GetTime (&shortdate);
		*/
		}
	
	shortdate.hour = longdate.ld.hour; /*time fields will be zero if !flgottime*/
	
	shortdate.minute = longdate.ld.minute;
	
	shortdate.second = longdate.ld.second;
	
	Date2Secs (&shortdate, time);
	
	return (true);
	} /*stringtotime*/


#define horizinset 8
#define vertinset 8


static void calsetdateinfo (void) {
	
	hdldata hd = caldata;	
	hdlobject ho = calobject;
	short lh;
	short rwidth;
	short extra;
	short ctweeks;
	short colpixels;
	short leftextra, rightextra;
	short vertextra;
	tydate date;
	Rect rwhole;
	Rect r;
	
	date = (**hd).date;
	
	(**hd).ctdays = daysinmonth (date.month, date.year);
			
	ctweeks = (**hd).ctweeks = weeksinmonth (date.month, date.year);
			
	(**hd).dayoffirst = firstdayofmonth (date.month, date.year);
			
	(**hd).dayoflast = lastdayofmonth (date.month, date.year);
	
	rwhole = (**ho).objectrect;
	
	InsetRect (&rwhole, horizinset, vertinset);
	
	rwidth = rwhole.right - rwhole.left;
		
	colpixels = rwidth / daysinweek;
	
	/*colpixels--;*/ /*add a little extra whitespace to left and right of calendar body*/
	
	(**hd).colpixels = colpixels;
	
	extra = rwidth - (colpixels * daysinweek);
	
	leftextra = extra / 2;
	
	rightextra = extra - leftextra;
	
	pushmonthstyle ();
	
	lh = globalfontinfo.ascent + globalfontinfo.descent;
	
	popstyle ();
	
	vertextra = lh / 2;
	
	r.top = rwhole.top + vertextra;
	
	r.bottom = r.top + lh;
	
	r.left = rwhole.left + leftextra;
	
	r.right = rwhole.right - rightextra;
	
	(**hd).rmonthyear = r;
	
	pushlabelstyle ();
	
	r.top = r.bottom + vertextra;
	
	r.bottom = r.top + globalfontinfo.ascent + globalfontinfo.descent;
	
	popstyle ();
	
	(**hd).rdaysofweek = r;
	
	r.top = r.bottom + 2;
	
	r.bottom = rwhole.bottom - vertextra;
	
	(**hd).rbody = r;
	
	(**hd).bodyrowheight = (r.bottom - r.top) / ctweeks;
	
	/*
	r.top = (**hd).rdaysofweek.top;
	
	r.bottom = rwhole.bottom - 3;
	
	r.left = rwhole.left + 3;
	
	r.right = rwhole.right - 3;
	
	(**hd).rhours = r;
	*/
	} /*calsetdateinfo*/
	

static void getdaylocation (short daynumber, short *dayofweeknumber, short *weeknumber) {
	
	/*
	give me a day in the current month, and I'll tell you what day of 
	the week it is, and which week it is in.
	*/
	
	hdldata hd = (hdldata) caldata;	
	short dayoffirst = (**hd).dayoffirst;
	short day = daynumber;
	short dayofweek;
	short weeknum;
	
	dayofweek = (dayoffirst + ((day - 1) % daysinweek)) % daysinweek;
	
	if (dayofweek == 0)
		dayofweek = 7;
		
	weeknum = 1 + ((dayoffirst - 1 + day - 1) / daysinweek);
	
	*dayofweeknumber = dayofweek;
	
	*weeknumber = weeknum;
	} /*getdaylocation*/
	
	
static void getdayrect (short daynumber, Rect *dayrect) {

	hdldata hd = (hdldata) caldata;	
	short bodyrowheight = (**hd).bodyrowheight;
	short colpixels = (**hd).colpixels;
	short dayofweek, weeknumber;
	Rect r;
	
	getdaylocation (daynumber, &dayofweek, &weeknumber);
	
	r.top = (**hd).rbody.top + ((weeknumber - 1) * bodyrowheight);
	
	r.bottom = r.top + bodyrowheight + 1;
	
	r.left = (**hd).rbody.left + ((dayofweek - 1) * colpixels);
	
	r.right = r.left + colpixels + 1;
	
	*dayrect = r;
	} /*getdayrect*/
	
	
static void caldisplayday (short daynum, Rect r) {
	
	hdldata hd = (hdldata) caldata;	
	boolean flcolor;
	tydate cursordate = (**hd).date;
	bigstring bs;
	tydate d;
	boolean flcursor;
	
	flcolor = IOAincolorwindow (calobject);
	
	d = cursordate;
	
	d.day = daynum;
	
	flcursor = equaldates (&cursordate, &d);
	
	if (flcursor && flcolor)
		IOApushbackcolor (&(**hd).cellcursorcolor);
	else
		IOApushbackcolor (&(**hd).cellfillcolor);
	
	EraseRect (&r);
	
	pushdaystyle ();
	
	IOApushforecolor (&(**hd).textcolor);
	
	NumToString (daynum, bs);
	
	IOAcenterstring (r, bs);
	
	IOApopforecolor ();
	
	popstyle ();
	
	IOApushforecolor (&(**hd).framecolor);
	
	FrameRect (&r);
	
	IOApopforecolor ();
	
	if (flcursor && (!flcolor)) {
		
		InsetRect (&r, 2, 2);
		
		InvertRect (&r);
		}
		
	IOApopbackcolor ();
	} /*caldisplayday*/
	

static void drawcalendarbody (void) {
	
	/*
	highly optimized to make drawing the entire calendar body much 
	faster than it would be if we drew it one cell at a time.
	*/
	
	hdldata hd = (hdldata) caldata;	
	short rowheight = (**hd).bodyrowheight;
	short colwidth = (**hd).colpixels;
	short ctweeks = (**hd).ctweeks;
	Rect rbody = (**hd).rbody;
	
	rbody.right++; /*hide a bug*/

	IOApushforecolor (&(**hd).framecolor);
	
	IOApushbackcolor (&(**hd).cellfillcolor);
	
	/*frame the weeks and erase the cell interiors*/ {
	
		register short i;
		Rect r;
		
		for (i = 1; i <= ctweeks; i++) {
			
			r = rbody;
			
			r.top += (i - 1) * rowheight;
			
			r.bottom = r.top + rowheight + 1;
			
			if (i == 1)
				r.left += ((**hd).dayoffirst - 1) * colwidth;
				
			if (i == ctweeks)
				r.right -= (7 - (**hd).dayoflast) * colwidth;
				
			EraseRect (&r);
			
			FrameRect (&r);
			} /*for*/
		} /*frame and erase the weeks*/
		
	/*draw the vertical bars separating the days*/ {
		
		register short h, top, bottom, i;
		
		h = rbody.left + colwidth;
		
		for (i = 1; i <= 6; i++) {
			
			top = rbody.top;
			
			if (i < (**hd).dayoffirst)
				top += rowheight;
				
			bottom = rbody.top + (ctweeks * rowheight);
		
			if (i > (**hd).dayoflast)
				bottom -= rowheight;
			
			MoveTo (h, top);
			
			LineTo (h, bottom);
			
			h += colwidth;
			} /*for*/
		} /*drawing vertical bars*/
		
	IOApopbackcolor ();
	
	IOApopforecolor ();
	
	/*center the day strings within each cell*/ {
		
		register short i;
		bigstring bs;
		Rect r;
		
		IOApushforecolor (&(**hd).textcolor);
		
		IOApushbackcolor (&(**hd).cellfillcolor);
		
		pushdaystyle ();
		
		for (i = 1; i <= (**hd).ctdays; i++) {
			
			getdayrect (i, &r);
			
			NumToString (i, bs);
			
			/*DebugStr (bs);*/
			
			IOAcenterstring (r, bs);
			} /*for*/
		
		popstyle ();
		
		IOApopbackcolor ();
		
		IOApopforecolor ();
		}
	} /*drawcalendarbody*/
	

static void drawdaysinweek (void) {

	hdldata hd = (hdldata) caldata;	
	short colwidth = (**hd).colpixels;
	Rect r;
	short i;
	bigstring bs;
	
	r = (**hd).rdaysofweek;
	
	IOApushbackcolor (&(**hd).fillcolor);
	
	EraseRect (&r);
	
	pushlabelstyle ();
	
	IOApushforecolor (&(**hd).textcolor);
	
	for (i = 1; i <= daysinweek; i++) {
		
		r.right = r.left + colwidth;
		
		getdaystring (i, bs);
		
		allupper (bs);
		
		setstringlength (bs, 3); /*length of abbreviated string*/
		
		/*DebugStr (bs);*/
		
		IOAcenterstring (r, bs);
		
		r.left += colwidth;
		} /*for*/
	
	IOApopforecolor ();
	
	popstyle ();
	
	IOApopbackcolor ();
	} /*drawdaysinweek*/
	
	
static void drawmonthyear (void) {
	
	hdldata hd = (hdldata) caldata;	
	Rect r;
	bigstring bs;
	
	getmonthyearstring ((**hd).date, bs);
	
	r = (**hd).rmonthyear;
	
	pushmonthstyle ();
	
	IOApushforecolor (&(**hd).textcolor);
	
	IOApushbackcolor (&(**hd).fillcolor);
	
	EraseRect (&r);
	
	IOAcenterstring (r, bs);
	
	/*DebugStr (bs);*/
	
	IOApopbackcolor ();
	
	IOApopforecolor ();
	
	popstyle ();
	} /*drawmonthyear*/
	
	
static void drawcalendar (void) {
	
	hdldata hd = (hdldata) caldata;	
	
	IOApushbackcolor (&(**hd).fillcolor);
		
	EraseRect (&(**calobject).objectrect);
	
	calsetdateinfo ();
	
	drawmonthyear ();
	
	drawdaysinweek ();
		
	drawcalendarbody ();
	
	/*display the day*/ {
		
		short day;
		Rect r;
		
		day = (**hd).date.day;
	
		getdayrect (day, &r);
	
		caldisplayday (day, r); 
		}
	
	IOApopbackcolor ();
	} /*drawcalendar*/
	

static void calmovedaycursorto (short daynum) {

	hdldata hd = (hdldata) caldata;	
	short day = daynum;
	short olddaynum;
	Rect r;
	
	olddaynum = (**hd).date.day;
	
	(**hd).date.day = day;
	
	getdayrect (olddaynum, &r);
	
	caldisplayday (olddaynum, r); /*erase the old cursor*/
	
	getdayrect (day, &r);
	
	caldisplayday (day, r); /*show the new cursor*/
	} /*calmovedaycursorto*/
	

static boolean cleancalendar (hdlobject h, short height, short width, Rect *r) {

	register hdlcard hc = (**h).owningcard;
	register short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
	
	height = IOAmakemultiple ((*r).bottom - (*r).top, gridunits);
	
	(*r).right = (*r).left + width;

	(*r).bottom = (*r).top + height;
	} /*cleancalendar*/
	

static boolean canreplicatecalendar (hdlobject h) {
	
	return (false); /*it can't be replicated*/
	} /*canreplicatecalendar*/
	

static boolean getcalendareditrect (hdlobject h, Rect *r) {

	return (false); /*it can't be edited*/
	} /*getcalendareditrect*/
	

static boolean getcalendarvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue)); 
	} /*getcalendarvalue*/
	

static boolean setcalendarvalue (hdlobject h, Handle hvalue) {
	
	(**h).objectvalue = hvalue;
	
	return (true);
	} /*setcalendarvalue*/
	
	
static boolean debugcalendar (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugcalendar*/
	

static boolean drawcalendarobject (hdlobject h) {
	
	Rect r;
	
	setglobals (h);
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	drawcalendar ();
	} /*drawcalendarobject*/
	
	
static boolean initcalendar (tyobject *obj) {
	
	hdldata h;
	
	mindate.year = 1904;
	
	mindate.month = 1;
	
	mindate.day = 1;
	
	mindate.hour = mindate.minute = mindate.second = 0;
	
	maxdate.year = 2040;
	
	maxdate.month = 12;
	
	maxdate.day = 31;
	
	maxdate.hour = 23;
	
	maxdate.minute = 59;
	
	maxdate.second = 59;
	
	h = (hdldata) NewHandleClear (longsizeof (tydata));
	
	if (h == nil)
		return (false);
	
	GetTime (&(**h).date);
	
	(**h).monthfont = helvetica;
	
	(**h).monthsize = 18;
	
	(**h).monthstyle = bold;
	
	(**h).labelfont = geneva;
	
	(**h).labelsize = 9;
	
	(**h).labelstyle = 0;
	
	(**h).dayfont = geneva;
	
	(**h).daysize = 9;
	
	(**h).daystyle = 0;
	
	(**h).framecolor = greencolor; 
	
	(**h).textcolor = blackcolor; 
	
	(**h).fillcolor = lightgreencolor; 
	
	(**h).cellfillcolor = whitecolor; 
	
	(**h).cellcursorcolor = yellowcolor; 
	
	(*obj).objectdata = (Handle) h;

	return (false); /*we do not want to edit it*/
	} /*initcalendar*/
	
	
static boolean recalccalendar (hdlobject h, boolean flmajorrecalc) {
	
	bigstring bs;
	unsigned long secs;
	
	if (!flmajorrecalc) 
		return (true);
		
	if (!IOArecalcobjectvalue (h))
		return (false);
		
	IOAtexthandletostring ((**h).objectvalue, bs);
	
	if (!stringtotime (bs, &secs))
		return (false);
	
	Secs2Date (secs, &(**(hdldata) (**h).objectdata).date);
	
	return (true);
	} /*recalccalendar*/
	
	
static boolean calmoveto (tydate date) {
	
	hdldata hd = (hdldata) caldata;	
	tydate origdate;
	
	origdate = (**hd).date;
	
	if ((origdate.month == date.month) && (origdate.year == date.year)) {
		
		calmovedaycursorto (date.day);
		
		return (true);
		}
	
	fixdate (&date); /*make sure date is in bounds, and represents an actual date*/
	
	(**hd).date = date; /*fresh the whole calendar*/
	
	return (true);
	} /*calmoveto*/
	

static void calmovecursor (tydirection dir, boolean flamplified) {
	
	hdldata hd = (hdldata) caldata;	
	tydate origdate, date;
	
	origdate = date = (**hd).date;
	
	switch (dir) {
	
		case up:
			if (flamplified)
				prevyear (&date);
			else
				prevweek (&date);
			
			break;
			
		case down:
			if (flamplified)
				nextyear (&date);
			else
				nextweek (&date);
			
			break;
			
		case left:
			if (flamplified) 
				prevmonth (&date);
			else
				yesterday (&date);
			
			break;
			
		case right: 
			if (flamplified)
				nextmonth (&date);
			else
				tomorrow (&date);
			
			break;
		} /*switch*/
		
	calmoveto (date);
	} /*calmovecursor*/
	

static boolean keystrokecalendar (hdlobject h, char ch) {
	
	tydirection dir;
	
	setglobals (h);
	
	switch (ch) {
		
		case chuparrow:
			dir = up; break;
			
		case chdownarrow:
			dir = down; break;
			
		case chleftarrow:
			dir = left; break;
			
		case chrightarrow:
			dir = right; break;
		
		default:
			dir = nodirection; break;
		} /*switch*/
	
	if (dir != nodirection) {
	
		calmovecursor (dir, false);
		
		return (true);
		}
		
	return (true);
	} /*keystrokecalendar*/
	
	
static boolean clickcalendar (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey) {
	
	short i;
	Rect r;
	
	setglobals (h);
	
	for (i = 1; i <= (**caldata).ctdays; i++) {
		
		getdayrect (i, &r);
		
		if (PtInRect (pt, &r)) {
			
			calmovedaycursorto (i);
			
			return (true);
			}
		} /*for*/
		
	return (false);
	} /*clickcalendar*/
	

void setupconfig (tyconfigrecord *config) {

	IOAcopystring ("\pCalendar", (*config).objectTypeName);
	
	(*config).objectTypeID = calendartype;
	
	(*config).handlesMouseTrack = true; /*avoid drawing and redrawing on each mouse click*/
	
	(*config).initObjectCallback = initcalendar;
	
	(*config).drawObjectCallback = drawcalendarobject;
	
	(*config).clickObjectCallback = clickcalendar;
	
	(*config).keystrokeObjectCallback = keystrokecalendar;
	
	(*config).cleanupObjectCallback = cleancalendar;
	
	(*config).recalcObjectCallback = recalccalendar;
	
	(*config).canReplicateObjectCallback = canreplicatecalendar;
	
	(*config).getObjectEditRectCallback = getcalendareditrect;
	
	(*config).getValueForScriptCallback = getcalendarvalue;
	
	(*config).setValueFromScriptCallback = setcalendarvalue;
	
	(*config).debugObjectCallback = debugcalendar;
	} /*setupconfig*/
	
	
