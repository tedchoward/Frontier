
/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#ifdef MACVERSION
	#include <fp.h>
	#include <ctype.h>
	#include <Gestalt.h>
	#include <Script.h>
	#include <standard.h>
#endif

#ifdef WIN95VERSION
	#include <ctype.h>
	#include <math.h>
	#include "standard.h"
#endif

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "langinternal.h"
#include "shell.h"
#include "timedate.h"

#ifdef MACVERSION
	#define tydate DateTimeRec
#endif

#ifdef WIN95VERSION
	#define tydate SYSTEMTIME
#endif


typedef struct tyValidationOrder {
	short item;
	short type;
} tyValidationOrder, * tyValidationOrderPtr;

typedef unsigned long tyCharacterAttributes;


static short daysInMonthsArray[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define DateTimeSet_kDayOfWeek		0x0100
#define DateTimeSet_kYear			0x0040
#define DateTimeSet_kMonth			0x0020
#define DateTimeSet_kDay			0x0010
#define DateTimeSet_kHour			0x0008
#define DateTimeSet_kMinute			0x0004
#define DateTimeSet_kSecond			0x0002
#define DateTimeSet_kHundredth		0x0001

#define DurationUnit_kDays			0x0010

#define err_kTooManyNumberFields	20000
#define err_kTooFewValues			20001
#define err_kOutOfOrder				20002
#define err_kItemUsed				20003
#define err_kInvalidPunctuation		20004
#define err_kHundredthValueInvalid  20005
#define err_kSecondValueInvalid		20006
#define err_kMinuteValueInvalid		20007
#define err_kHourValueInvalid		20008
#define err_kAMorPMon24HourClock	20009

#define CharAttr_kShortDate			0x00000001
#define CharAttr_kLongDate			0x00000002
#define CharAttr_kTime				0x00000004

#define SCAN_DOW	1
#define SCAN_MONTH	2
#define SCAN_YEAR	4

#define VALTYPE_NONE		0
#define VALTYPE_KEYMONTHNAME1	1
#define VALTYPE_KEYMONTHNAME2	2
#define VALTYPE_KEYDAYNAME1	3
#define VALTYPE_KEYDAYNAME2	4
#define VALTYPE_KEYYEARNAME	5
#define VALTYPE_KEYTIMEAM	6
#define VALTYPE_KEYTIMEPM	7
#define VALTYPE_KEYTIME24	8
#define VALTYPE_CURRENCY	9
#define VALTYPE_BLANK	       10
#define VALTYPE_NUMERIC        11
#define VALTYPE_PUNC	       12
#define VALTYPE_NONNUMERIC     13

#define VALORDER_NUMERIC	1
#define VALORDER_FIXEDNUMERIC	2
#define VALORDER_KEYWORD	3

#define VALFLAG_ISDOLLARS	(0x1000)
#define VALFLAG_ISCENTS 	(0x2000)

typedef struct tyvalidationtoken
   {
   short ty;
   char * pos;
   short count;
   long val;
   unsigned long flag;
   } tyvalidationtoken, * tyvalidationtokenptr;


static tyinternationalinfo globalII;
static tyinternationalinfoptr globalIIptr = NULL;

typedef struct tydaterec {
	short year;
	short month;
	short dayOfWeek;
	short day;
	short hour;
	short minute;
	short second;
	short hundredths;
	} tydaterec, * tydaterecptr;

typedef struct tyvalidationerror {
	long errorNumber;
	long stringPosition;
	char * auxilaryPointer;
	} tyvalidationerror, * tyvalidationerrorptr;


#ifdef WIN95VERSION
boolean ValidDate (
   unsigned char * strIn,
   short cnt,
   tydaterecptr returnDT,
   tydaterecptr actualDT,
   unsigned long * validUnits,
   tyvalidationerrorptr err);

boolean ValidTime (
   char * strIn,
   short cnt,
   tydaterecptr returnDT,
   tydaterecptr actualDT,
   unsigned long * validUnits,
   tyvalidationerrorptr err);
#endif


static boolean getNewLocaleString (unsigned long lc, char ** foo, char * defValue) {
	#ifdef MACVERSION
		return (false);
	#endif

	#ifdef WIN95VERSION
		char buf[300];
		short len;

		len = GetLocaleInfo (LOCALE_USER_DEFAULT, lc, buf, 300);

		if (len != 0) {
			*foo = malloc (len + 2);
			if (*foo != NULL) {
				moveleft (buf, stringbaseaddress((*foo)), len);
				setstringlength ((*foo), len-1);
				nullterminate ((*foo));
				return (true);
				}
			}
		else {
			len = strlen (defValue);
			*foo = malloc (len + 2);
			if (*foo != NULL) {
				moveleft (defValue, stringbaseaddress((*foo)), len);
				setstringlength ((*foo), len);
				nullterminate ((*foo));
				return (true);
				}
			}

		return (false);
	#endif
	} /*getNewLocaleString*/


static boolean getNewLocalePatternString (unsigned long lc, char ** foo, char * defValue) {
	#ifdef MACVERSION
		return (false);
	#endif

	#ifdef WIN95VERSION
		char buf[256];
		char buf2[256];
		short i, j, cnt, len;
		char c;

		len = GetLocaleInfo (LOCALE_USER_DEFAULT, lc, buf, 256);
		--len;
		if (len != 0) {
			i = 0;
			j = 0;

			while (len > i) {
				c = buf[i++];

				switch (c) {
					case 'M':
					case 'd':
					case 'y':
					case 'h':
					case 'm':
					case 's':
					case 't':
						cnt = 1;
						while (buf[i] == c) {
							++cnt;
							++i;
							}

						buf2[j++] = '%';
						buf2[j++] = '%';

						buf2[j++] = toupper(c);

						if (cnt > 2)
							buf2[j++] = 'N';
						else
							buf2[j++] = '#';
						break;

					default:
						buf2[j++] = c;
						break;
					}
				}

			len = j;

			*foo = malloc (len + 2);
			if (*foo != NULL) {
				moveleft (buf2, stringbaseaddress((*foo)), len);
				setstringlength ((*foo), len);
				nullterminate ((*foo));
				return (true);
				}
			}
		else {
			len = strlen (defValue);
			*foo = malloc (len + 2);
			if (*foo != NULL) {
				moveleft (defValue, stringbaseaddress((*foo)), len);
				setstringlength ((*foo), len);
				nullterminate ((*foo));
				return (true);
				}
			}

		return (false);
	#endif
	}


static boolean initInternationalInfo() {

	#ifdef MACVERSION
		boolean res = false;
	#endif

	#ifdef WIN95VERSION
		short i;
		boolean res = true;

		res = res && getNewLocaleString (LOCALE_SDAYNAME7, &globalII.longDaysOfWeek[0], "Sunday");
		res = res && getNewLocaleString (LOCALE_SDAYNAME1, &globalII.longDaysOfWeek[1], "Monday");
		res = res && getNewLocaleString (LOCALE_SDAYNAME2, &globalII.longDaysOfWeek[2], "Tuesday");
		res = res && getNewLocaleString (LOCALE_SDAYNAME3, &globalII.longDaysOfWeek[3], "Wednesday");
		res = res && getNewLocaleString (LOCALE_SDAYNAME4, &globalII.longDaysOfWeek[4], "Thursday");
		res = res && getNewLocaleString (LOCALE_SDAYNAME5, &globalII.longDaysOfWeek[5], "Friday");
		res = res && getNewLocaleString (LOCALE_SDAYNAME6, &globalII.longDaysOfWeek[6], "Saturday");
		res = res && getNewLocaleString (999999, &globalII.longDaysOfWeek[7], "Yesterday");
		res = res && getNewLocaleString (999999, &globalII.longDaysOfWeek[8], "Today");
		res = res && getNewLocaleString (999999, &globalII.longDaysOfWeek[9], "Tomorrow");

		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME7, &globalII.shortDaysOfWeek[0], "Sun");
		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME1, &globalII.shortDaysOfWeek[1], "Mon");
		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME2, &globalII.shortDaysOfWeek[2], "Tue");
		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME3, &globalII.shortDaysOfWeek[3], "Wed");
		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME4, &globalII.shortDaysOfWeek[4], "Thu");
		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME5, &globalII.shortDaysOfWeek[5], "Fri");
		res = res && getNewLocaleString (LOCALE_SABBREVDAYNAME6, &globalII.shortDaysOfWeek[6], "Sat");
		res = res && getNewLocaleString (999999, &globalII.shortDaysOfWeek[7], "Yest");
		res = res && getNewLocaleString (999999, &globalII.shortDaysOfWeek[8], "Tod");
		res = res && getNewLocaleString (999999, &globalII.shortDaysOfWeek[9], "Tom");

		res = res && getNewLocaleString (LOCALE_SMONTHNAME1, &globalII.longMonths[0], "January");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME2, &globalII.longMonths[1], "February");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME3, &globalII.longMonths[2], "March");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME4, &globalII.longMonths[3], "April");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME5, &globalII.longMonths[4], "May");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME6, &globalII.longMonths[5], "June");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME7, &globalII.longMonths[6], "July");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME8, &globalII.longMonths[7], "August");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME9, &globalII.longMonths[8], "September");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME10, &globalII.longMonths[9], "October");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME11, &globalII.longMonths[10], "November");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME12, &globalII.longMonths[11], "December");
		res = res && getNewLocaleString (LOCALE_SMONTHNAME13, &globalII.longMonths[12], "");
		
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME1, &globalII.shortMonths[0], "Jan");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME2, &globalII.shortMonths[1], "Feb");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME3, &globalII.shortMonths[2], "Mar");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME4, &globalII.shortMonths[3], "Apr");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME5, &globalII.shortMonths[4], "May");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME6, &globalII.shortMonths[5], "Jun");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME7, &globalII.shortMonths[6], "Jul");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME8, &globalII.shortMonths[7], "Aug");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME9, &globalII.shortMonths[8], "Sep");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME10, &globalII.shortMonths[9], "Oct");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME11, &globalII.shortMonths[10], "Nov");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME12, &globalII.shortMonths[11], "Dec");
		res = res && getNewLocaleString (LOCALE_SABBREVMONTHNAME13, &globalII.shortMonths[12], "");
		
		res = res && getNewLocaleString (LOCALE_S1159, &globalII.morning, "AM");
		res = res && getNewLocaleString (LOCALE_S2359, &globalII.evening, "PM");
		res = res && getNewLocaleString (999999, &globalII.military, "hours");
		
		res = res && getNewLocaleString (LOCALE_SCURRENCY, &globalII.currency, "$");
		res = res && getNewLocaleString (LOCALE_SINTLSYMBOL, &globalII.intlCurrency, "USD");
		res = res && getNewLocaleString (LOCALE_SDECIMAL, &globalII.decimal, ".");
		res = res && getNewLocaleString (LOCALE_STHOUSAND, &globalII.thousands, ",");
		res = res && getNewLocaleString (LOCALE_SLIST, &globalII.list, ",");
		res = res && getNewLocaleString (LOCALE_STIME, &globalII.timesep, ":");
		res = res && getNewLocaleString (LOCALE_SDATE, &globalII.datesep, "/");

		res = res && getNewLocalePatternString (LOCALE_SSHORTDATE, &globalII.shortDateFormatPattern, "M#/D#/Y#");
		res = res && getNewLocalePatternString (LOCALE_SLONGDATE, &globalII.longDateFormatPattern, "DN, MN D#, YN");

		globalII.numberOfDays = 7;		//Make this 10 to use yesterday, today, tommorrow - dave wanted them removed.
		globalII.numberOfMonths = 12;
		globalII.numberOfYears = 0;

		globalII.defaultTimeFormat = false;


		for (i = 0; i < globalII.numberOfMonths; i++) {
			globalII.daysInMonth[i] = daysInMonthsArray[i];
			}

		for (i = globalII.numberOfMonths; i < 13; i++) {
			globalII.daysInMonth[i] = 0;
			}

		if (res)
			globalIIptr = &globalII;
	#endif

	return (res);
	} /*initInternationalInfo*/


tyinternationalinfoptr getIntlInfo () {
	if (globalIIptr == NULL) {
		if (initInternationalInfo()) {
			return (globalIIptr);
			}
		}
	else {
		return (globalIIptr);
		}

	return (NULL);
	}

static void charStringToBigstring (bigstring bs, char * source, short len) {
	moveleft (source, stringbaseaddress(bs), len);
	setstringlength (bs, len);
	} /*charStringToBigstring*/

static long charStringToLong (char * str, short len) {
	bigstring bs;
	long ret;

	charStringToBigstring (bs, str, len);
	stringtonumber (bs, &ret);
	return (ret);
	} /*charStringToLong*/


#ifdef WIN95VERSION
static void getDateTime (tydaterecptr dt) {
	SYSTEMTIME sysdt;

	GetLocalTime (&sysdt);

	dt->year = sysdt.wYear;
	dt->month = sysdt.wMonth;
	dt->dayOfWeek = sysdt.wDayOfWeek;
	dt->day = sysdt.wDay;
	dt->hour = sysdt.wHour;
	dt->minute = sysdt.wMinute;
	dt->second = sysdt.wSecond;
	dt->hundredths = sysdt.wMilliseconds;
	} /*getDateTime*/

#endif

boolean isLeapYear (short year) {
	/* this procedure assumes the existance of the Gregorian calendar only */
	/* This basically means it's not valid for years before the adoption of
	   the Gregorian calendar.  This adoption varies by country, starting in
	   Europe as early as the 1400's and as late as the 1900's in Russia. */

	boolean res;

	res = false;

	if ((year % 4) == 0) {
		res = true;

		if ((year % 100) == 0) {
			res = false;

			if ((year % 400) == 0) {
				res = true;
				}
			}
		}

	return (res);
	} /*isLeapYear*/


short daysInMonth (short month, short year) {
	short res;

	/*this procedure assumes month valid from 1 to 12*/

	res = daysInMonthsArray [month-1];

	if (month == 2) {
		if (isLeapYear (year))
			++res;
		}

	return (res);
	} /* daysInMonth */



#ifdef WIN95VERSION
long filetimetoseconds (const FILETIME *filetime) {

	LARGE_INTEGER basetime;
	LARGE_INTEGER nowtime;

	basetime.HighPart = 0x0153b281;
	basetime.LowPart = 0xe0fb4000;

	nowtime.HighPart = (*filetime).dwHighDateTime;
	nowtime.LowPart = (*filetime).dwLowDateTime;

	nowtime.QuadPart = nowtime.QuadPart - basetime.QuadPart;

	return ((long) (nowtime.QuadPart / 10000000));	//convert to seconds
	} /*filetimetoseconds*/


void secondstofiletime (long seconds, FILETIME *filetime) {

	LARGE_INTEGER basetime;
	LARGE_INTEGER nowtime;
	LARGE_INTEGER mul;
	
	basetime.HighPart = 0x0153b281;
	basetime.LowPart = 0xe0fb4000;

	mul.LowPart = 10000000;
	mul.HighPart = 0;

	nowtime.LowPart = seconds;
	nowtime.HighPart = 0;

	nowtime.QuadPart = nowtime.QuadPart * mul.QuadPart;
	
	nowtime.QuadPart = nowtime.QuadPart + basetime.QuadPart;

	(*filetime).dwHighDateTime = nowtime.HighPart;
	(*filetime).dwLowDateTime = nowtime.LowPart;
	} /*secondstofiletime*/


void GetDateTime (long * secs) {
	
	FILETIME filetimeUTC;
	FILETIME filetime;
	//This is the Macintosh base time of Jan 1, 1904 at midnight.
	GetSystemTimeAsFileTime (&filetimeUTC);

	FileTimeToLocalFileTime (&filetimeUTC, &filetime);

	*secs = filetimetoseconds (&filetime);
	} /*GetDateTime*/

#endif


void timestamp (long *time) {
	
	GetDateTime ((unsigned long *) time);
	} /*timestamp*/
	
	
unsigned long timenow (void) {
	
	/*
	2.1b4 dmb; more convenient than timestamp for most callers
	*/
	
	unsigned long now;
	
	GetDateTime (&now);
	
	return (now);
	} /*timenow*/
	
	
boolean setsystemclock (unsigned long secs) {

	/*
	3/10/97 dmb: set the system clock, using Macintosh time 
	conventions (seconds since 12:00 PM Jan 1, 1904)
	*/

	#ifdef MACVERSION
		return (!oserror (SetDateTime (secs)));
	#endif

	#ifdef WIN95VERSION
		SYSTEMTIME date;
		FILETIME filetime;
		FILETIME utcFileTime;

		secondstofiletime (secs, &filetime);

		LocalFileTimeToFileTime (&filetime, &utcFileTime);
		FileTimeToSystemTime (&utcFileTime, &date);

		if (! SetSystemTime (&date)) {
			oserror (GetLastError());
			return (false);
			}

		return (true);
	#endif
	} /*setsystemclock*/


static void adjustforcurrenttimezone (unsigned long *time) {
	
	/*
	5.1b23 dmb: avoid wraparound for near-zero dates
	*/

	unsigned long adjustedtime = *time + getcurrenttimezonebias ();
	
	if (sgn (*time) == sgn (adjustedtime))
		*time = adjustedtime;
	} /*adjustforcurrenttimezone*/


boolean timegreaterthan (unsigned long time1, unsigned long time2) {
	
	return (time1 > time2);
	} /*timegreaterthan*/
	
	
boolean timelessthan (unsigned long time1, unsigned long time2) {
	
	return (time1 < time2);
	} /*timelessthan*/


boolean timetotimestring (unsigned long time, bigstring bstime, boolean flwantseconds) {

	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:45:30 PM
		//updated call for carbon, the nil parameter says use the current script
		//for formatting the time.
		TimeString (time, flwantseconds, bstime, nil);

		return (true);
	#endif

	#ifdef WIN95VERSION
		SYSTEMTIME date;
		FILETIME filetime;
		int len;

		secondstofiletime (time, &filetime);

		FileTimeToSystemTime (&filetime, &date);

		len = GetTimeFormat (LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &date, NULL, stringbaseaddress(bstime), 255);

		setstringlength(bstime, len - 1);
		
		return (true);
	#endif
	} /*timetotimestring*/


boolean timetodatestring (unsigned long time, bigstring bsdate, boolean flabbreviate) {

	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:45:49 PM
		//Updated call for carbon
		DateString (time, flabbreviate? abbrevDate : shortDate, bsdate, nil);

		return (true);
	#endif

	#ifdef WIN95VERSION
		SYSTEMTIME date;
		FILETIME filetime;
		int len;

		secondstofiletime (time, &filetime);

		FileTimeToSystemTime (&filetime, &date);

		len = GetDateFormat (LOCALE_USER_DEFAULT, flabbreviate? DATE_LONGDATE : DATE_SHORTDATE, &date, NULL, stringbaseaddress(bsdate), 255);
		
		setstringlength(bsdate, len - 1);
		
		return (true);
	#endif
	} /*timetodatestring*/


boolean stringtotime (bigstring bsdate, unsigned long *time) {
	
	/*
	9/13/91 dmb: use the script manager to translate a string to a 
	time in seconds since 12:00 AM 1904.
	
	if a date is provided, but no time, the time is 12:00 AM
	
	if a time is provided, but no date, the date is 1/1/1904.
	
	we return true if any time/date information was extracted.
	*/
	
	#ifdef MACVERSION

		Ptr p = (Ptr) bsdate + 1;
		long n = stringlength (bsdate);
		long used = 0;
		DateCacheRecord cache;
		LongDateRec longdate;
		boolean flgotdate;
		boolean flgottime;
		DateTimeRec shortdate;
		boolean flUseGMT = false;
		long index;
		
		index = stringlength (bsdate);

		while (getstringcharacter(bsdate, index-1) == ' ')
			--index;

		if (index > 3) {
			if ((getstringcharacter(bsdate, index - 3) == 'G') && (getstringcharacter(bsdate, index - 2) == 'M') && (getstringcharacter(bsdate, index -1) == 'T')) {
				flUseGMT = true;
				}
			}

		*time = 0; /*default return value*/
		
		clearbytes (&longdate, sizeof (longdate));
		
		if (InitDateCache (&cache) != noErr) /*must do this before calling StringToDate*/
			return (false);
		
		flgotdate = ((StringToDate (p, n, &cache, &used, &longdate) & fatalDateTime) == 0);
		
		flgottime = ((StringToTime (p + used, n - used, &cache, &used, &longdate) & fatalDateTime) == 0);
		
		if (!flgotdate && !flgottime) /*nuthin' doin'*/
			return (false);
		
		if (flgotdate) {
		
			shortdate.day = longdate.ld.day;
			
			shortdate.month = longdate.ld.month;
			
			shortdate.year = longdate.ld.year;
			}
		else {
			
			SecondsToDate (0, &shortdate);
			
			/*
			GetTime (&shortdate);
			*/
			}
		
		shortdate.hour = longdate.ld.hour; /*time fields will be zero if !flgottime*/
		
		shortdate.minute = longdate.ld.minute;
		
		shortdate.second = longdate.ld.second;
		
		DateToSeconds (&shortdate, time);

		if (flUseGMT) {
			adjustforcurrenttimezone (time);
			}
		
	#endif

	#ifdef WIN95VERSION
		tydaterec returnDT;
		tydaterec return2DT;
		tydaterec actualDT;
		unsigned long vu;
		tyvalidationerror errinfo;
		bigstring bsdate1, bstime1;
		short index;
		tyinternationalinfo * ii;
		boolean flUseGMT = false;
		boolean flSaveTimeFormat;

		ii = getIntlInfo();

		if (ii == NULL)
			return (false);

		index = stringlength (bsdate);
		
		if (index > 128) //5.0b17 dmb: too long for a date, avoid crash
			return (false);

		while (getstringcharacter(bsdate, index-1) == ' ')
			--index;

		if (index > 3) {
			if ((getstringcharacter(bsdate, index - 3) == 'G') && (getstringcharacter(bsdate, index - 2) == 'M') && (getstringcharacter(bsdate, index -1) == 'T')) {
				flUseGMT = true;
				flSaveTimeFormat = ii->defaultTimeFormat;
				ii->defaultTimeFormat = true;
				}
			}

		index = 1;
		if (scanstring (getstringcharacter(ii->timesep, 0), bsdate, &index)) {
			--index;
			/* now check to see if this is a time preceded by a date */
			if (index > 5) {  /*blank digit colon - 3 min, possibly 4 - date must be at least 3 i.e. 1/5 */
				if (isnumeric (getstringcharacter(bsdate, index-1))) {
					if (index + 1 < stringlength (bsdate)) {
						if (isnumeric (getstringcharacter(bsdate, index+1))) {
							if (getstringcharacter(bsdate, index-2) == ' ') {
								setstringcharacter(bsdate, index-2, ';');
								}
							else {
								if ((getstringcharacter(bsdate, index-3) == ' ') && (isnumeric (getstringcharacter(bsdate, index-2)))) {
									setstringcharacter(bsdate, index-3, ';');
									}
								}
							}
						}
					}
				}
			}
		else {
			index = 1;
			if (scanstring (':', bsdate, &index)) {
				--index;
				/* now check to see if this is a time preceded by a date */
				if (index > 5) {  /*blank digit colon - 3 min, possibly 4 - date must be at least 3 i.e. 1/5 */
					if (isnumeric (getstringcharacter(bsdate, index-1))) {
						if (index + 1 < stringlength (bsdate)) {
							if (isnumeric (getstringcharacter(bsdate, index+1))) {
								if (getstringcharacter(bsdate, index-2) == ' ') {
									setstringcharacter(bsdate, index-2, ';');
									}
								else {
									if ((getstringcharacter(bsdate, index-3) == ' ') && (isnumeric (getstringcharacter(bsdate, index-2)))) {
										setstringcharacter(bsdate, index-3, ';');
										}
									}
								}
							}
						}
					}
				}
			}

		/* if the string contains a semicolon ";" then split into two strings are parse date;time */

		index = 1;
		if (scanstring (';', bsdate, &index)) {
			midstring (bsdate, 1, index-1, bsdate1);
			midstring (bsdate, index + 1, stringlength(bsdate) - index, bstime1);

			++index;
			if (scanstring (';', bsdate, &index)) {
				midstring (bsdate, index + 1, stringlength(bsdate) - index, bstime1);
				}


			ValidDate (stringbaseaddress (bsdate1), stringlength (bsdate1), &returnDT, &actualDT, &vu, &errinfo); 

			if (errinfo.errorNumber == 0)
				{
				ValidTime (stringbaseaddress (bstime1), stringlength (bstime1), &return2DT, &actualDT, &vu, &errinfo); 

				*time = datetimetoseconds (returnDT.day, returnDT.month, returnDT.year, return2DT.hour, return2DT.minute, return2DT.second);

				if (errinfo.errorNumber == 0) {
					if (flUseGMT) {
						adjustforcurrenttimezone (time);
						ii->defaultTimeFormat = flSaveTimeFormat;
						}
					return (true);
					}

				/* try normal */
				}
			}


		ValidDate (stringbaseaddress (bsdate), stringlength (bsdate), &returnDT, &actualDT, &vu, &errinfo); 

		*time = datetimetoseconds (returnDT.day, returnDT.month, returnDT.year, 0,0,0);

		if (errinfo.errorNumber != 0)
			{
			if (errinfo.errorNumber == err_kInvalidPunctuation) {
				ValidTime (stringbaseaddress (bsdate), stringlength (bsdate), &returnDT, &actualDT, &vu, &errinfo); 

				*time = datetimetoseconds (returnDT.day, returnDT.month, returnDT.year, returnDT.hour, returnDT.minute, returnDT.second);

				if (errinfo.errorNumber == 0) {
					if (flUseGMT) {
						adjustforcurrenttimezone (time);
						ii->defaultTimeFormat = flSaveTimeFormat;
						}
					return (true);
					}
				}

			if (flUseGMT) {
				ii->defaultTimeFormat = flSaveTimeFormat;
				}

			return (false);
			}
	#endif

	return (true);
	} /*stringtotime*/


long datetimetoseconds (short day, short month, short year, short hour, short minute, short second) {
	
	/*
	5.0a12 dmb: Win version, must handle hour, minute, second wraparound
	*/

	#ifdef MACVERSION

		DateTimeRec date;
		unsigned long secs;
		
		if (year < 100) { /*use script manager's StringToDate heuristic -- pg 188 in § docs*/
			
			long thisyear, century;
			
			GetTime (&date);
			
			thisyear = date.year % 100;
			
			century = date.year - thisyear;
			
			if ((thisyear <= 10) && (year >= 90)) /*assume last century*/
				century -= 100;
			else
				if ((thisyear >= 90) && (year <= 10)) /*assume next century*/
					century += 100;
			
			year += century;
			}
		
		clearbytes (&date, sizeof (date));
		
		date.day = day;
		
		date.month = month;
		
		date.year = year;
		
		date.hour = hour;
		
		date.minute = minute;
		
		date.second = second;
		
		DateToSeconds (&date, &secs);
	#endif

	#ifdef WIN95VERSION

		SYSTEMTIME date;
		FILETIME filetime;
		unsigned long secs;
		
		if (year < 100) { /*use script manager's StringToDate heuristic -- pg 188 in § docs*/
			
			long thisyear, century;
			
			GetSystemTime (&date);
			
			thisyear = date.wYear % 100;
			
			century = date.wYear - thisyear;
			
			if ((thisyear <= 10) && (year >= 90)) /*assume last century*/
				century -= 100;
			else
				if ((thisyear >= 90) && (year <= 10)) /*assume next century*/
					century += 100;
			
			year += century;
			}
		
		// Win call doesn't wrap around h:m:s

		minute = minute + (second / 60);
		second = second % 60;
		hour = hour + (minute / 60);
		minute = minute % 60;
		day = day + (hour / 24);
		hour = hour % 24;

		/* we pre-adjust the month to ensure that it is in a valid range */
		year = year + ((month - 1) / 12);
		month = ((month - 1) % 12) + 1;

		while (day > daysInMonth(month, year)) {
			day = day - daysInMonth(month, year);

			if (month < 12) {
				++month;
				}
			else {
				month = 1;
				++year;
				}
			}
		
		clearbytes (&date, sizeof (date));
		
		date.wDay = day;
		
		date.wMonth = month;
		
		date.wYear = year;
		
		date.wHour = hour;
		
		date.wMinute = minute;
		
		date.wSecond = second;
		
		SystemTimeToFileTime (&date, &filetime);

		secs = filetimetoseconds (&filetime);
	#endif
	
	return (secs);
	} /*datetimetoseconds*/


void secondstodatetime (long secs, short *day, short *month, short *year, short *hour, short *minute, short *second) {
	
	#ifdef MACVERSION

		DateTimeRec date;
		
		SecondsToDate (secs, &date);
		
		*day = date.day;
		
		*month = date.month;
		
		*year = date.year;
		
		*hour = date.hour;
		
		*minute = date.minute;
		
		*second = date.second;
	#endif

	#ifdef WIN95VERSION

		SYSTEMTIME date;
		FILETIME filetime;

		secondstofiletime (secs, &filetime);

		FileTimeToSystemTime (&filetime, &date);

		*day = date.wDay;
		
		*month = date.wMonth;
		
		*year = date.wYear;
		
		*hour = date.wHour;
		
		*minute = date.wMinute;
		
		*second = date.wSecond;
	#endif
	} /*secondstodatetime*/


void secondstodayofweek (long secs, short *dayofweek) {
	
	#ifdef MACVERSION

		DateTimeRec date;
		
		SecondsToDate (secs, &date);
		
		*dayofweek = date.dayOfWeek;
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME date;
		FILETIME filetime;

		secondstofiletime (secs, &filetime);
		
		FileTimeToSystemTime (&filetime, &date);
		
		*dayofweek = date.wDayOfWeek + 1;
		
	#endif
	} /*secondstodayofweek*/


static void fixdate (tydate * date) {
	#ifdef MACVERSION	
		date->minute = date->minute + (date->second / 60);
		date->second = date->second % 60;
		date->hour = date->hour + (date->minute / 60);
		date->minute = date->minute % 60;
		date->day = date->day + (date->hour / 24);
		date->hour = date->hour % 24;

		/* we pre-adjust the month to ensure that it is in a valid range */
		date->year = date->year + ((date->month - 1) / 12);
		date->month = ((date->month - 1) % 12) + 1;

		while (date->day > daysInMonth(date->month, date->year)) {
			date->day = date->day - daysInMonth(date->month, date->year);

			if (date->month < 12) {
				++(date->month);
				}
			else {
				date->month = 1;
				++(date->year);
				}
			}
	#endif

	#ifdef WIN95VERSION

		date->wMinute = date->wMinute + (date->wSecond / 60);
		date->wSecond = date->wSecond % 60;
		date->wHour = date->wHour + (date->wMinute / 60);
		date->wMinute = date->wMinute % 60;
		date->wDay = date->wDay + (date->wHour / 24);
		date->wHour = date->wHour % 24;

		/* we pre-adjust the month to ensure that it is in a valid range */
		date->wYear = date->wYear + ((date->wMonth - 1) / 12);
		date->wMonth = ((date->wMonth - 1) % 12) + 1;

		while (date->wDay > daysInMonth(date->wMonth, date->wYear)) {
			date->wDay = date->wDay - daysInMonth(date->wMonth, date->wYear);

			if (date->wMonth < 12) {
				++(date->wMonth);
				}
			else {
				date->wMonth = 1;
				++(date->wYear);
				}
			}
	#endif
	} /*fixdate*/


unsigned long nextmonth(unsigned long date) {

	/*
	6.0a10 dmb: limit day to max days for new month
	*/
	
	#ifdef MACVERSION

		DateTimeRec daterec;
		short maxday;
		
		SecondsToDate (date, &daterec);
		
		if (daterec.month < 12) {
			++daterec.month;
			}
		else {
			daterec.month = 1;
			++daterec.year;
			}
		
		maxday = daysInMonth(daterec.month, daterec.year);
		
		if (daterec.day > maxday)
			daterec.day = maxday;
		
		fixdate (&daterec);

		DateToSeconds (&daterec, &date);

		return (date);
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME daterec;
		FILETIME filetime;
		short maxday;

		secondstofiletime (date, &filetime);
		
		FileTimeToSystemTime (&filetime, &daterec);
		
		if (daterec.wMonth < 12) {
			++daterec.wMonth;
			}
		else {
			daterec.wMonth = 1;
			++daterec.wYear;
			}
		
		maxday = daysInMonth (daterec.wMonth, daterec.wYear);
		
		if (daterec.wDay > maxday)
			daterec.wDay = maxday;

		fixdate (&daterec);

		SystemTimeToFileTime (&daterec, &filetime);

		return (filetimetoseconds (&filetime));
	#endif
	} /*nextmonth*/

unsigned long nextyear(unsigned long date) {
	#ifdef MACVERSION

		DateTimeRec daterec;
		
		SecondsToDate (date, &daterec);
		
		++daterec.year;

		fixdate (&daterec);

		DateToSeconds (&daterec, &date);

		return (date);
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME daterec;
		FILETIME filetime;

		secondstofiletime (date, &filetime);
		
		FileTimeToSystemTime (&filetime, &daterec);
		
		++daterec.wYear;

		fixdate (&daterec);

		SystemTimeToFileTime (&daterec, &filetime);

		return (filetimetoseconds (&filetime));
	#endif
	} /*nextyear*/

unsigned long prevmonth(unsigned long date) {

	/*
	6.0a10 dmb: limit day to max days for new month
	*/
	
	#ifdef MACVERSION

		DateTimeRec daterec;
		short maxday;
		
		SecondsToDate (date, &daterec);
		
		if (daterec.month > 1) {
			--daterec.month;
			}
		else {
			daterec.month = 12;
			--daterec.year;
			}

		maxday = daysInMonth(daterec.month, daterec.year);
		
		if (daterec.day > maxday)
			daterec.day = maxday;
		
		fixdate (&daterec);

		DateToSeconds (&daterec, &date);

		return (date);
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME daterec;
		FILETIME filetime;
		short maxday;

		secondstofiletime (date, &filetime);
		
		FileTimeToSystemTime (&filetime, &daterec);
		
		if (daterec.wMonth > 1) {
			--daterec.wMonth;
			}
		else {
			daterec.wMonth = 12;
			--daterec.wYear;
			}

		maxday = daysInMonth(daterec.wMonth, daterec.wYear);
		
		if (daterec.wDay > maxday)
			daterec.wDay = maxday;
		
		fixdate (&daterec);

		SystemTimeToFileTime (&daterec, &filetime);

		return (filetimetoseconds (&filetime));
	#endif
	} /*prevmonth*/

unsigned long prevyear(unsigned long date) {
	#ifdef MACVERSION

		DateTimeRec daterec;
		
		SecondsToDate (date, &daterec);
		
		--daterec.year;

		fixdate (&daterec);

		DateToSeconds (&daterec, &date);

		return (date);
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME daterec;
		FILETIME filetime;

		secondstofiletime (date, &filetime);
		
		FileTimeToSystemTime (&filetime, &daterec);
		
		--daterec.wYear;

		fixdate (&daterec);

		SystemTimeToFileTime (&daterec, &filetime);

		return (filetimetoseconds (&filetime));
	#endif
	} /*prevyear*/

unsigned long firstofmonth(unsigned long date) {
	#ifdef MACVERSION

		DateTimeRec daterec;
		
		SecondsToDate (date, &daterec);
		
		daterec.day  = 1;

		daterec.hour = 0;
		daterec.minute = 0;
		daterec.second = 0;

		DateToSeconds (&daterec, &date);

		return (date);
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME daterec;
		FILETIME filetime;

		secondstofiletime (date, &filetime);
		
		FileTimeToSystemTime (&filetime, &daterec);
		
		daterec.wDay  = 1;

		daterec.wHour = 0;
		daterec.wMinute = 0;
		daterec.wSecond = 0;
		daterec.wMilliseconds = 0;

		SystemTimeToFileTime (&daterec, &filetime);

		return (filetimetoseconds (&filetime));
	#endif
	} /*firstofmonth*/

unsigned long lastofmonth(unsigned long date) {
	#ifdef MACVERSION

		DateTimeRec daterec;
		
		SecondsToDate (date, &daterec);
		
		daterec.day  = daysInMonth(daterec.month, daterec.year);

		daterec.hour = 0;
		daterec.minute = 0;
		daterec.second = 0;

		DateToSeconds (&daterec, &date);

		return (date);
	#endif

	#ifdef WIN95VERSION
		
		SYSTEMTIME daterec;
		FILETIME filetime;

		secondstofiletime (date, &filetime);
		
		FileTimeToSystemTime (&filetime, &daterec);
		
		daterec.wDay  = daysInMonth(daterec.wMonth, daterec.wYear);

		daterec.wHour = 0;
		daterec.wMinute = 0;
		daterec.wSecond = 0;
		daterec.wMilliseconds = 0;

		SystemTimeToFileTime (&daterec, &filetime);

		return (filetimetoseconds (&filetime));
	#endif
	} /*lastofmonth*/


void shortdatestring (unsigned long date, bigstring bs) {
	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:48:03 PM
		//Updated for carbon
		DateString (date, shortDate, bs, nil);
	#endif

	#ifdef WIN95VERSION
		timetodatestring (date, bs, false);
	#endif
	} /*shortdatestring*/

void longdatestring (unsigned long date, bigstring bs) {
	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:48:20 PM
		//Updated for Carbon
		DateString (date, longDate, bs, nil);
	#endif

	#ifdef WIN95VERSION
		timetodatestring (date, bs, true);
	#endif
	} /*longdatestring*/

void abbrevdatestring (unsigned long date, bigstring bs) {
	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:48:36 PM
		//updated for carbon
		DateString (date, abbrevDate, bs, nil);
	#endif

	#ifdef WIN95VERSION
		timetodatestring (date, bs, true);
	#endif
	} /*abbrevdatestring*/

void getdaystring (short dayofweek, bigstring bs, boolean flFullname) {
	#ifdef MACVERSION	
		switch (dayofweek) {
			case 1:
				copyctopstring ("Sunday", bs);
				break;

			case 2:
				copyctopstring ("Monday", bs);
				break;

			case 3:
				copyctopstring ("Tuesday", bs);
				break;

			case 4:
				copyctopstring ("Wednesday", bs);
				break;

			case 5:
				copyctopstring ("Thursday", bs);
				break;

			case 6:
				copyctopstring ("Friday", bs);
				break;

			case 7:
				copyctopstring ("Saturday", bs);
				break;

			default:
				copyctopstring ("Day of week number is invalid (not between 1 and 7)", bs);
				break;
			}

		if (! flFullname)
			setstringlength (bs, 3);
	#endif

	#ifdef WIN95VERSION
//		//reset day of week to a Monday thru Sunday
//		dayofweek = dayofweek - 1;
//
//		if (dayofweek == 0)
//			dayofweek = 7;
//
//		getWindowsLocaleString (flFullname?LOCALE_SDAYNAME1 + dayofweek - 1:LOCALE_SABBREVDAYNAME1 + dayofweek - 1, bs);
	tyinternationalinfo * ii;
//	char temp[100]; 							/*hold token string*/
//	short i;

	ii = getIntlInfo();

	if (ii == NULL)
		return;

	if (flFullname)
		copystring (ii->longDaysOfWeek[dayofweek-1], bs);
	else
		copystring (ii->shortDaysOfWeek[dayofweek-1], bs);
	#endif
	} /*getdaystring*/

long getcurrenttimezonebias() {
	#ifdef MACVERSION
		MachineLocation ml;
		long res;

		ReadLocation (&ml);
		
		res = ml.u.gmtDelta & 0x00FFFFFF;

		if ((res & 0x00800000) == 0x00800000)  //if this is a negative number extend the sign bits
			res = res | 0xFF000000;

		return (res);
	#endif

	#ifdef WIN95VERSION
		TIME_ZONE_INFORMATION tzi;
		DWORD tziResult;

		tziResult = GetTimeZoneInformation (&tzi);

		if (tziResult == TIME_ZONE_ID_STANDARD)
			tzi.Bias += tzi.StandardBias;
		else if (tziResult == TIME_ZONE_ID_DAYLIGHT)
			tzi.Bias += tzi.DaylightBias;

		return (tzi.Bias * -60L);
	#endif
	} /*getcurrenttimezonebias*/




#ifdef WIN95VERSION

static void addDurationToDateTime(tydaterecptr dt, short delta, long units) {
	long actualDelta;
	long secs;

	actualDelta = 0;
	
	switch (units) {
		case DurationUnit_kDays:
			actualDelta = (long) delta * (24L * 60L * 60L);
			break;

		default:
			break;
		}

	secs = datetimetoseconds (dt->day, dt->month, dt->year, dt->hour, dt->minute, dt->second);

	secs += actualDelta;

	secondstodatetime (secs, &dt->day, &dt->month, &dt->year, &dt->hour, &dt->minute, &dt->second);
	} /*addDurationToDateTime*/


static char ValNextChar (char ** pos) {
	char g;

	g = **pos;
	*pos = *pos + 1;

	return (g);
	} /*ValNextChar*/


static boolean ValGetFullPunc (char c, tyCharacterAttributes * f) {
	switch (c) {
		case ':':
			*f = CharAttr_kTime;
			return (true);
			break;

		case '/':
			*f = CharAttr_kShortDate;
			return (true);
			break;

		case '.':
			*f = CharAttr_kShortDate | CharAttr_kLongDate | CharAttr_kTime; /*6.1b5 AR: added shortdate*/
			return (true);
			break;

		case '-':
			*f = CharAttr_kShortDate | CharAttr_kTime | CharAttr_kLongDate;
			return (true);
			break;

		case ',':
			*f = CharAttr_kLongDate;
			return (true);
			break;

		case ';':
			*f = CharAttr_kLongDate;
			return (true);
			break;


		default:
			break;
		}

	*f = 0;
	return (false);
	}


static void ValCheckForKeyWord (tyvalidationtokenptr o) {
	tyinternationalinfo * ii;
	char temp[100]; 							/*hold token string*/
	short i;

	ii = getIntlInfo();

	if (ii == NULL)
		return;

	charStringToBigstring(temp, (o->pos), o->count);

	for (i = 0; i < ii->numberOfMonths; i++) {
		if (equalidentifiers(ii->longMonths[i], temp)) {
			/*We have found a match*/
			o->ty = VALTYPE_KEYMONTHNAME1;
			o->val = i;
			o->flag = DateTimeSet_kMonth;
			return;
			}

		if (equalidentifiers(ii->shortMonths[i], temp)) {
			/*We have found a match*/
			o->ty = VALTYPE_KEYMONTHNAME2;
			o->val = i;
			o->flag = DateTimeSet_kMonth;
			return;
			}

		if (i < ii->numberOfYears) {
			if (equalidentifiers(ii->longYears[i], temp)) {
				/*We have found a match*/
				o->ty = VALTYPE_KEYYEARNAME;
				o->val = i;
				o->flag = DateTimeSet_kYear;
				return;
				}
			}

		if (i < ii->numberOfDays) {		/*should this be 10 to add yesterday, today, tommorow*/
			if (equalidentifiers(ii->longDaysOfWeek[i], temp)) {
				/*We have found a match*/
				o->ty = VALTYPE_KEYDAYNAME1;
				o->val = i;
				return;
				}

			if (equalidentifiers(ii->shortDaysOfWeek[i], temp)) {
				/*We have found a match*/
				o->ty = VALTYPE_KEYDAYNAME2;
				o->val = i;
				return;
				}
			}
		}	/*end for*/

	if (equalidentifiers(ii->morning, temp)) {
		/*We have found a match*/
		o->ty = VALTYPE_KEYTIMEAM;
		o->val = 1;
		return;
		}

	if (equalidentifiers(ii->evening, temp)) {
		/*We have found a match*/
		o->ty = VALTYPE_KEYTIMEPM;
		o->val = 2;
		return;
		}

	if (equalidentifiers(ii->military, temp)) {
		/*We have found a match*/
		o->ty = VALTYPE_KEYTIME24;
		o->val = 3;
		return;
		}

	if (equalidentifiers(ii->currency, temp)) {
		o->ty = VALTYPE_CURRENCY;
		return;
		}

	if (equalidentifiers(ii->intlCurrency, temp)) {
		o->ty = VALTYPE_CURRENCY;
		return;
		}
	} /*ValCheckForKeyWord*/


static short ValGetCharType (char c)
	{
	if (isspace(c))
		return (VALTYPE_BLANK);

	if (isdigit(c))
		return (VALTYPE_NUMERIC);

	if (ispunct(c))
		return (VALTYPE_PUNC);

	return (VALTYPE_NONNUMERIC);
	} /*ValGetCharType*/

/*
   ValParseOne - parses a string into %#, %B and %S

   ValParseTwo - converts a parse one string so that certain %S are
		 better defined into %P, %Kx
*/
static short ValParseOne (char * strIn, short cnt, Handle * tokenOut) {
	short fTokenCount;					/*number of actual BRRCToke found*/
	short len;							/*length of input string*/
	char * pos;							/*current pointer into string for back reference*/
	char * curpos;						/*current pointer into string for back reference*/
	short curTy;						/*currently active token type*/
	char c;								/*character from string*/
	short ty;							/*Gets token type of 'c'*/
	Handle ho; 							/*handle to o*/
	tyvalidationtokenptr o;				/*pointer to a series of tyvalidationtoken structures*/
	short i;							/*loop variable*/
	tyinternationalinfoptr ii;


	ii = getIntlInfo();

	if (ii == NULL)
		return (-1);

	fTokenCount = 0;
	*tokenOut = NULL;

	ho = NewHandle((sizeof(tyvalidationtoken)) * 100);

	if (!ho)
		return (-1);							/*fError*/

	o = (tyvalidationtokenptr) HLock(ho);	/*lock it down*/

	if (!o) {
		DisposeHandle (ho);
		return (-2);
		}

	len = cnt;

	pos = strIn;

	(o + fTokenCount)->pos = pos;
	(o + fTokenCount)->ty = VALTYPE_NONE;
	(o + fTokenCount)->count = 0;
	curTy = VALTYPE_NONE;

	while (len > 0) {
		curpos = pos;
		c = ValNextChar(&pos);
		--len;

		ty = ValGetCharType(c);

		if ((curTy != ty) || (ty == VALTYPE_PUNC)) {/*we are changing types*/
			/* strip leading blanks */
			if ((fTokenCount != 0) || (ty != VALTYPE_BLANK))
				++fTokenCount;
			(o + fTokenCount)->ty = ty;
			(o + fTokenCount)->pos = curpos;
			(o + fTokenCount)->count = 1;
			(o + fTokenCount)->val = -1;
			(o + fTokenCount)->flag = 0L;
			curTy = ty;
			}
		else {
			++((o + fTokenCount)->count);
			}
		}	/*endwhile*/

	for (i = 1; i <= fTokenCount; i++) {

		switch ((o + i)->ty) {
			case VALTYPE_BLANK:
				break;

			case VALTYPE_NONNUMERIC:
				ValCheckForKeyWord(&(o[i]));
				break;

			case VALTYPE_NUMERIC:
				(o + i)->val = charStringToLong((o + i)->pos, (o + i)->count);

				(o + i)->flag = VALFLAG_ISDOLLARS | VALFLAG_ISCENTS | DateTimeSet_kYear;

				if ((o + i)->val < 100)
					(o + i)->flag |= DateTimeSet_kHundredth;

				if ((o + i)->val < 60) {
					(o + i)->flag |= DateTimeSet_kMinute;
					(o + i)->flag |= DateTimeSet_kSecond;
					}

				if (((o + i)->val < 32) && ((o + i)->val > 0))
					(o + i)->flag |= DateTimeSet_kDay;

				if ((o + i)->val <= 24)
					(o + i)->flag |= DateTimeSet_kHour;

				if (((o + i)->val <= ii->numberOfMonths) && ((o + i)->val > 0))
					(o + i)->flag |= DateTimeSet_kMonth;

				break;

			case VALTYPE_PUNC:
				c = *((o + i)->pos);
				(o + i)->val = ValGetFullPunc(c, &((o + i)->flag));
				break;

			default:
				break;
		}										/*fEnd switch*/
	}											/*endfor*/

	/*re-allocate structure*/

	HUnlock(ho);

	ho = SetHandleSize(ho, (sizeof(tyvalidationtoken)) * (fTokenCount + 1));

	*tokenOut = ho;

	return (fTokenCount);
	} /*ValParseOne*/


static short ValGetDateOrder (tyValidationOrderPtr orderRec, short ty)
	{
	tyinternationalinfoptr ii;
	short orderPos;
	char c, d;
	char * pos;
	bigstring p;
	boolean flYear, flDay, flMonth;

	flYear = FALSE;
	flDay = FALSE;
	flMonth = FALSE;

	orderPos = 0;

	ii = getIntlInfo();

	if (ii == NULL)
		return (0);

	if (ty == 0)								/*then use short date order*/
		copystring (ii->shortDateFormatPattern, p);
	else
		copystring (ii->longDateFormatPattern, p);

	pos = stringbaseaddress(p);

	while (pos < (stringbaseaddress(p) + stringlength(p))) {
		c = ValNextChar(&pos);

		if (c == '%') {
			c = ValNextChar(&pos);

			if (c == '%') {
				c = ValNextChar(&pos);

				switch (c) {
					case 'Y':
						if (!flYear) {
							(orderRec + orderPos)->item = DateTimeSet_kYear;
							flYear = TRUE;
							++orderPos;
							}
						break;

					case 'M':
					case 'D':
						d = ValNextChar(&pos);

						if ((c == 'M') && ((d == 'N') || (d == '#'))) {
							if (!flMonth) {
								(orderRec + orderPos)->item = DateTimeSet_kMonth;
								++orderPos;
								flMonth = TRUE;
								}
							}

						if ((c == 'D') && (d == '#')) {
							if (!flDay) {
								(orderRec + orderPos)->item = DateTimeSet_kDay;
								++orderPos;
								flDay = TRUE;
								}
							}

						break;

					default:
						break;
					}
				}
			}
		} /*end while*/


	if (orderPos < 3) {							/*default format does not do everything*/
		if (!flMonth) {
			(orderRec + orderPos)->item = DateTimeSet_kMonth;
			++orderPos;
			}
		if (!flDay) {
			(orderRec + orderPos)->item = DateTimeSet_kDay;
			++orderPos;
			}
		if (!flYear) {
			(orderRec + orderPos)->item = DateTimeSet_kYear;
			++orderPos;
			}
		}
	return (orderPos);
	}


static void ValDeleteFromOrder (
	tyValidationOrderPtr o,
	short * cnt,
	short field)

{
	short i;
	boolean flStart;

	flStart = FALSE;

	for (i = 0; i < *cnt; i++)
	{
		if (flStart)
		{
			(o + i - 1)->item = (o + i)->item;
			(o + i - 1)->type = (o + i)->type;
		}

		if ((o + i)->item == field)
		{
			flStart = TRUE;
		}
	}

	if (flStart)
		*cnt = *cnt - 1;
}												/*ValDeleteFromOrder*/


boolean ValidDate (
   unsigned char * strIn,
   short cnt,
   tydaterecptr returnDT,
   tydaterecptr actualDT,
   unsigned long * validUnits,
   tyvalidationerrorptr err)

{
	tyvalidationtokenptr o;
	Handle ho;
	short bC, uC, kC, pC, nC;
	short i;
	tyCharacterAttributes j;
	short tokCount;
	short dateF;
	boolean uniformP;
	tydaterec currentDT;
	short errSet;
	long pType;
	tyValidationOrder orderRec[6];
	short orderCnt;
	short op;
	short vu;
	short cYear, cCen;
	boolean flShortFormat;

	/**** FOR DATES ONLY! ****/

	tokCount = ValParseOne(strIn, cnt, &ho);
	
	if (tokCount < 0)
		return (false);

	o = (tyvalidationtokenptr) HLock(ho);

	bC = uC = kC = nC = pC = 0;					/*I hate doing this*/
	uniformP = FALSE;
	flShortFormat = FALSE;
	dateF = 0;

	for (i = 1; i <= tokCount; i++) {
		switch ((o + i)->ty) {
			case VALTYPE_BLANK:
				++bC;
				break;

			case VALTYPE_NONNUMERIC:
				++uC;
				break;

			case VALTYPE_KEYDAYNAME1:
			case VALTYPE_KEYDAYNAME2:
				dateF |= SCAN_DOW;
				++kC;
				break;

			case VALTYPE_KEYMONTHNAME1:
			case VALTYPE_KEYMONTHNAME2:
				dateF |= SCAN_MONTH;
				++kC;
				break;

			case VALTYPE_KEYYEARNAME:
				/* not yet supported */
				break;

			case VALTYPE_KEYTIMEAM:
			case VALTYPE_KEYTIMEPM:
			case VALTYPE_KEYTIME24:
				break;

			case VALTYPE_NUMERIC:
				if (((o + i)->flag & (DateTimeSet_kYear | DateTimeSet_kMonth | DateTimeSet_kDay)) == DateTimeSet_kYear)
					dateF |= SCAN_YEAR;

				++nC;
				break;

			case VALTYPE_PUNC:
				if (pC == 0) {
					pType = (o + i)->val;
					uniformP = TRUE;
					}
				else {
					if (pType != (o + i)->val)
						uniformP = FALSE;
					}

				++pC;
				break;

			default:
				break;
			} /*fEnd switch*/
		} /*endfor*/

	getDateTime(&currentDT);

	cYear = currentDT.year;
	cCen = 100 * (cYear / 100);

	errSet = 0;
	vu = 0;
	op = 0;

	/************************************************************************
	  Day of fWeek entries only have meaning if they are alone, i.e. a user
	  enters Friday or Yesterday, etc.  With anything else it meaning is
	  overridden.
	*************************************************************************/

	if ((dateF == SCAN_DOW) && (nC == 0) && (kC == 1))
		{
		/*process Day of Week FormatAndValidationKeywords only */

		/* find FormatAndValidationKeywords */

		i = 1;
		while (((o + i)->ty != VALTYPE_KEYDAYNAME1) && ((o + i)->ty != VALTYPE_KEYDAYNAME2)) {
			++i;
			}

		/* set actual structure */
		if (actualDT != NULL) {
			actualDT->dayOfWeek = (short)(o + i)->val;
			}

		vu = DateTimeSet_kDayOfWeek;

		if ((o + i)->val > 6)					/*handle yesterday, today, and tommorrow*/
			{
			addDurationToDateTime(&currentDT, ((short)((o + i)->val) - 8), DurationUnit_kDays);
			}
		else
			{
			j = ((short)(o + i)->val + 7) - currentDT.dayOfWeek;

			if (j > 6)
				j = j - 7;

			addDurationToDateTime(&currentDT, (short)j, DurationUnit_kDays);
			}
		/*we are done here...*/
		goto ValidDateExit;
		}


	/********** Not DayOfWeek FormatAndValidationKeywords only *************/

	if ((dateF & SCAN_MONTH) != SCAN_MONTH) 	/* we have no fMonth BR_FormatAndValidationKeywords*/
	{
		/*
		  We know we are using the "short" date format since there are no
		  meaningfull keywords, of course a number such as 5.25 qualifies
		  and only becuase we are trying for a date we will BR_TRY for May 25th.
		*/

		orderCnt = ValGetDateOrder(orderRec, 0);

		flShortFormat = TRUE;

		for (i = 0; i < orderCnt; i++)
		{
			orderRec[i].type = VALORDER_NUMERIC;

			if (orderRec[i].item == DateTimeSet_kYear)
			{
				if ((dateF & SCAN_YEAR) == SCAN_YEAR)/* we have a fYear! */
				{
					orderRec[i].type = VALORDER_FIXEDNUMERIC;
				}
			}
		}

		switch (nC)
		{
			case 1:
				/*Number must be the fYear only*/
				orderCnt = 1;
				orderRec[0].item = DateTimeSet_kYear;
				if ((dateF & SCAN_YEAR) == SCAN_YEAR)
					orderRec[0].type = VALORDER_FIXEDNUMERIC;
				else
					orderRec[0].type = VALORDER_NUMERIC;
				break;

			case 2:
				/*Numbers must be fMonth/fYear or fDay/fMonth pair*/
				if ((dateF & SCAN_YEAR) == SCAN_YEAR)
				{
					/*it must be the fMonth/fYear combination, fYear fFixed*/
					/*we do not need to have fYear in list since it is fFixed*/
					orderCnt = 1;
					orderRec[0].item = DateTimeSet_kMonth;
					orderRec[0].type = VALORDER_NUMERIC;
				}
				else
				{
					/*there is no determinate fYear so use fMonth/fDay combination*/
					ValDeleteFromOrder(orderRec, &orderCnt, DateTimeSet_kYear);
				}
				break;

			case 3:
				/*normal trio*/
				break;

			default:
				/*to many numbers or to few numbers*/
				errSet = err_kTooManyNumberFields;
				if (nC == 0)
					errSet = err_kTooFewValues;
				goto ValidDateExit;
				break;
		}										/*fEnd switch*/
	}
	/*endif on short format*/

	else										/* we have the fMonth BR_FormatAndValidationKeywords */
		{
			orderCnt = ValGetDateOrder(orderRec, 1);

			ValDeleteFromOrder(orderRec, &orderCnt, DateTimeSet_kMonth);

			if ((dateF & SCAN_YEAR) == SCAN_YEAR)/* we have a fYear! */
			{
				/*only need the fDay*/
				ValDeleteFromOrder(orderRec, &orderCnt, DateTimeSet_kYear);
			}
		}
	/*
	  do retreive.	it is this that would actually set the errors.

	  go through each token, when you get a BR_FormatAndValidationKeywords or number BR_TRY to match it
	  with the order record, if it is "fFixed" place it in catagory
	*/


	for (i = 1; i <= tokCount; i++)
	{
		switch ((o + i)->ty)
		{
			case VALTYPE_KEYDAYNAME1:
			case VALTYPE_KEYDAYNAME2:
				if (actualDT != NULL)
					actualDT->dayOfWeek = (short)(o + i)->val;

				vu |= DateTimeSet_kDayOfWeek;
				currentDT.dayOfWeek = (short)(o + i)->val;
				break;

			case VALTYPE_KEYMONTHNAME1:
			case VALTYPE_KEYMONTHNAME2:
				if (actualDT != NULL)
					actualDT->month = (short)(o + i)->val + 1;

				vu |= DateTimeSet_kMonth;

				currentDT.month = (short)(o + i)->val + 1;
				break;

			case VALTYPE_KEYYEARNAME:
				/* not yet supported */
				break;

			case VALTYPE_PUNC:
				if (flShortFormat)
					j = CharAttr_kShortDate;
				else
					j = CharAttr_kLongDate;

				if (!((o + i)->flag & j))
					errSet = err_kInvalidPunctuation;
				break;

			case VALTYPE_NUMERIC:
				/*
				  has this token's valid areas already been consummed
				  if so then we have an fError.
				*/
				if ((o + i)->flag & ((DateTimeSet_kYear | DateTimeSet_kMonth | DateTimeSet_kDay) ^ vu))
				{
					/*so far so good*/
					/*
					  see what is left that we need.  if more then one item
					  then check order to see which it should be.
					*/
					if (((o + i)->flag & (DateTimeSet_kYear | DateTimeSet_kMonth | DateTimeSet_kDay)) == DateTimeSet_kYear)
					{
						/*This item qualifies as a fYear only, apply it*/
						if (actualDT != NULL)
							actualDT->year = (short)(o + i)->val;

						currentDT.year = (short)(o + i)->val;
						vu |= DateTimeSet_kYear;

						++op; /*6.1b5 AR*/
					}
					else
					{
						/*the item is an indeterminate - use orderRec*/

						/*Is the item specified in the order record needed?*/
						if (orderRec[op].item & ((DateTimeSet_kYear | DateTimeSet_kMonth | DateTimeSet_kDay) ^ vu))
						{
							/*does order item fit token?*/
							if (((o + i)->flag) & (orderRec[op].item))
							{
								/*apply it*/
								switch (orderRec[op].item)
								{
									case DateTimeSet_kYear:
										if (actualDT != NULL)
											actualDT->year = (short)(o + i)->val;

										currentDT.year = (short)(o + i)->val;
										vu |= DateTimeSet_kYear;
										break;

									case DateTimeSet_kMonth:
										if (actualDT != NULL)
											actualDT->month = (short)(o + i)->val;

										currentDT.month = (short)(o + i)->val;
										vu |= DateTimeSet_kMonth;
										break;

									case DateTimeSet_kDay:
										if (actualDT != NULL)
											actualDT->day = (short)(o + i)->val;

										currentDT.day = (short)(o + i)->val;
										vu |= DateTimeSet_kDay;
										break;

									default:
										break;
								}				/*fEnd switch*/

								++op;
							}

							else
							{
								/*Item does not fit order position*/
								errSet = err_kOutOfOrder;
							}
						}
						else
						{
							/*Item in order record already used*/
							errSet = err_kItemUsed;
						}
					}							/*endelse - indeterminate*/
				}
				else
				{
					/*item is an fError, already used*/
					errSet = err_kItemUsed;
				}

				break;

			default:
				break;
		}										/*fEnd switch*/

		if (errSet != 0)
			break;
	}											/*endfor*/

ValidDateExit:
	if (validUnits != NULL)
		*validUnits = vu;

	if (vu == 0)								/*no valid units, i.e. garbage*/
		errSet = err_kTooFewValues;

	vu |= DateTimeSet_kDay | DateTimeSet_kMonth | DateTimeSet_kYear;

	if ((currentDT.year < 100) && (currentDT.year > 0))
	{
		currentDT.year += cCen;

		if (currentDT.year < (unsigned short)(cYear - 50))
			currentDT.year += 100;

		if (currentDT.year > (unsigned short)(cYear + 50))
			currentDT.year = currentDT.year - 100;
	}

/////////////	BR_AdjustDateTime(&currentDT, vu);

	if (returnDT != NULL)
		*returnDT = currentDT;

	if (err != NULL)
	{
		err->errorNumber = errSet;

		if (errSet != 0 && i <= tokCount)
		{
			err->stringPosition = (o + i)->pos - strIn;
			err->auxilaryPointer = NULL;
		}
	}

	unlockhandle (ho);
	DisposeHandle (ho);

	if (errSet != 0)
		return (FALSE);

	return (TRUE);
}






boolean ValidTime (
   char * strIn,
   short cnt,
   tydaterecptr returnDT,
   tydaterecptr actualDT,
   unsigned long * validUnits,
   tyvalidationerrorptr err) {

   /*
   5.1.5b14 dmb: fixed bug where we disposed handle before checking errors
   */

	tyvalidationtokenptr o;
	Handle ho;
	short i;
	short tokCount;
	short errSet;
	short vu;
	short tlAM;
	tyinternationalinfo * ii;
	tydaterec currentDT;

	tokCount = ValParseOne(strIn, cnt, &ho);

	if (tokCount < 0)
		return (false);

	o = (tyvalidationtokenptr) HLock(ho);

	getDateTime(&currentDT);
	currentDT.hour = 0;
	currentDT.minute = 0;
	currentDT.second = 0;
	currentDT.hundredths = 0;

	/*
	  do retreive.	it is this that would actually set the errors.

	  go through each token, when you get a BR_FormatAndValidationKeywords or number BR_TRY to match it
	  with the order record, if it is "fFixed" place it in catagory
	*/

	errSet = 0;
	vu = 0;
	tlAM = 0;

	for (i = 1; i <= tokCount; i++)
	{
		switch ((o + i)->ty)
		{
			case VALTYPE_KEYTIMEAM:
				tlAM = -1;
				break;

			case VALTYPE_KEYTIMEPM:
				tlAM = 1;
				break;

			case VALTYPE_KEYTIME24:
				break;							/*has no real meaning*/

			case VALTYPE_PUNC:
				if (!((o + i)->flag & CharAttr_kTime))
					errSet = err_kInvalidPunctuation;
				break;

			case VALTYPE_NUMERIC:
				/*first fHour, then fMinute, then fSecond, then hundreth*/
				if ((DateTimeSet_kHour & vu) == DateTimeSet_kHour)
				{
					/*Hour used*/
					if ((DateTimeSet_kMinute & vu) == DateTimeSet_kMinute)
					{
						/*fMinute used*/
						if ((DateTimeSet_kSecond & vu) == DateTimeSet_kSecond)
						{
							/*fSecond used*/
							if ((DateTimeSet_kHundredth & vu) == DateTimeSet_kHundredth)
							{
								errSet = err_kTooManyNumberFields;
							}
							else
							{
								if (((o + i)->flag & DateTimeSet_kHundredth) == DateTimeSet_kHundredth)
								{
									if (actualDT != NULL)
										actualDT->hundredths = (short)(o + i)->val;

									currentDT.hundredths = (short)(o + i)->val;
									vu |= DateTimeSet_kHundredth;
								}
								else
								{
									errSet = err_kHundredthValueInvalid;
								}
							}
						}
						else
						{
							if (((o + i)->flag & DateTimeSet_kSecond) == DateTimeSet_kSecond)
							{
								if (actualDT != NULL)
									actualDT->second = (short)(o + i)->val;

								currentDT.second = (short)(o + i)->val;
								vu |= DateTimeSet_kSecond;
							}
							else
							{
								errSet = err_kSecondValueInvalid;
							}
						}
					}
					else
					{
						if (((o + i)->flag & DateTimeSet_kMinute) == DateTimeSet_kMinute)
						{
							if (actualDT != NULL)
								actualDT->minute = (short)(o + i)->val;

							currentDT.minute = (short)(o + i)->val;
							vu |= DateTimeSet_kMinute;
						}
						else
						{
							errSet = err_kMinuteValueInvalid;
						}
					}
				}
				else
				{
					if (((o + i)->flag & DateTimeSet_kHour) == DateTimeSet_kHour)
					{
						if (actualDT != NULL)
							actualDT->hour = (short)(o + i)->val;

						currentDT.hour = (short)(o + i)->val;
						vu |= DateTimeSet_kHour;
					}
					else
					{
						errSet = err_kHourValueInvalid;
					}
				}

				break;

			default:
				break;
		}										/*fEnd switch*/

		if (errSet != 0)
			break;
	}											/*endfor*/


	if (validUnits != NULL)
		*validUnits = vu;

	if (vu == 0)								/*no valid units, i.e. garbage*/
		errSet = err_kTooFewValues;

	if (currentDT.hour > 12)
	{
		if (tlAM != 0)
			errSet = err_kAMorPMon24HourClock;
	}
	else
	{
		switch (tlAM)
		{
			case -1:
				if (currentDT.hour == 12)
					currentDT.hour = 0;
				break;

			case 0:
				ii = getIntlInfo();

				if (ii->defaultTimeFormat == FALSE)/*we are on a 12 fHour clock*/
				{
					if (currentDT.hour != 0)
					{
						/* assume 8 to 11 is AM, 12 is PM 1 to 7 is PM */
						if (currentDT.hour < 8)
							currentDT.hour += 12;
					}
				}
				break;

			case 1:
				currentDT.hour += 12;
				if (currentDT.hour == 24)
					currentDT.hour = 12;
				break;

			default:
				break;
		}
	}
	
	if (returnDT != NULL)
		*returnDT = currentDT;

	if (err != NULL)
	{
		err->errorNumber = errSet;

		if (errSet != 0 && i <= tokCount)
		{
			err->stringPosition = (o + i)->pos - strIn;
			err->auxilaryPointer = NULL;
		}
	}

	unlockhandle(ho);
	disposehandle (ho);

	if (errSet != 0)
		return (FALSE);

	return (TRUE);
}

#endif

