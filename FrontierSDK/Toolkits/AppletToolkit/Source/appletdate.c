
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletstrings.h"
#include "appletmemory.h"
#include "appletdate.h"
#include <Packages.h>


void datetomacdate (tydaterecord rec, unsigned long *secs) {
	
	DateTimeRec macrec;
	
	macrec.year = rec.year;
	
	macrec.month = rec.month;
	
	macrec.day = rec.day;
	
	macrec.hour = rec.hour;
	
	macrec.minute = rec.minute;
	
	macrec.second = rec.second;
	
	macrec.dayOfWeek = 0;

	Date2Secs (&macrec, secs);
	} /*datetomacdate*/
	

void appletdatetostring (tydaterecord rec, bigstring bs) {
	
	unsigned long secs;
	bigstring bstime;
		
	datetomacdate (rec, &secs);
	
	IUDateString (secs, abbrevDate, bs);
	
	IUTimeString (secs, true, bstime);
	
	pushstring ("\p;", bs);
	
	pushstring (bstime, bs);
	} /*appletdatetostring*/
	
	
void getcurrenttimestring (bigstring bs) {
	
	unsigned long now;
	bigstring bstime;
	
	GetDateTime (&now);

	IUDateString (now, shortDate, bs);
	
	IUTimeString (now, true, bstime);
	
	pushstring ("\p;", bs);
	
	pushstring (bstime, bs);
	} /*getcurrenttimestring*/
		
	
static void popnumber (bigstring bs, long *num) {
	
	long x = 0;
	
	while (true) {
		
		short len = bs [0];
		char ch;
		
		if (len == 0) { /*ran out of characters*/
			
			*num = x;
			
			return;
			}
		
		ch = bs [1];
		
		moveleft (&bs [2], &bs [1], --len);
		
		bs [0] = len;
		
		if ((ch < '0') || (ch > '9')) { /*not a numeric character*/
			
			*num = x;
			
			return;
			}
		
		x = (x * 10) + (ch - '0');
		} /*while*/
	} /*popnumber*/
	
	
static void popshort (bigstring bs, short *num) {
	
	long x;
	
	popnumber (bs, &x);
	
	*num = (short) x;
	} /*popshort*/
	
	
boolean stringtoappletdate (bigstring bs, tydaterecord *rec) {
	
	popshort (bs, &(*rec).month);
	
	popshort (bs, &(*rec).day);
	
	popshort (bs, &(*rec).year);
	
	popshort (bs, &(*rec).hour);
	
	popshort (bs, &(*rec).minute);
	
	popshort (bs, &(*rec).second);
	
	return (true);
	} /*stringtoappletdate*/


boolean stringtotime (bigstring bsdate, unsigned long *time) {
	
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
	
	clearbytes (&longdate, longsizeof (longdate));
	
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
	else 		
		Secs2Date (0, &shortdate);
	
	shortdate.hour = longdate.ld.hour; /*time fields will be zero if !flgottime*/
	
	shortdate.minute = longdate.ld.minute;
	
	shortdate.second = longdate.ld.second;
	
	Date2Secs (&shortdate, time);
	
	return (true);
	} /*stringtotime*/


unsigned long netscapedatetomacdate (unsigned long date) {
	
	return (date + 2082844800);
	} /*netscapedatetomacdate*/
	

unsigned long macdatetonetscapedate (unsigned long date) {

	return (date - 2082844800);
	} /*macdatetonetscapedate*/


