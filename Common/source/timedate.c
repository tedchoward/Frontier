
/*	$Id$    */

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

#include "frontier.h"
#include "standard.h"

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "langinternal.h"
#include "shell.h"

#include "timedate.h"

    #include "MacDateHelpers.h"
	#define tydate DateTimeRec



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

#pragma pack(2)
typedef struct tyvalidationtoken
   {
   short ty;
   char * pos;
   short count;
   long val;
   unsigned long flag;
   } tyvalidationtoken, * tyvalidationtokenptr;
#pragma options align=reset



#pragma pack(2)
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
#pragma options align=reset



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





void timestamp (long *ptime) {
    
    *ptime = (CFAbsoluteTimeGetCurrent() + kCFAbsoluteTimeIntervalSince1904);
    
	} /*timestamp*/
	
	
unsigned long timenow (void) {
	
	/*
	2.1b4 dmb; more convenient than timestamp for most callers
	*/
    
    unsigned long now = (CFAbsoluteTimeGetCurrent() + kCFAbsoluteTimeIntervalSince1904);
	
	return (now);
	} /*timenow*/
	
	
boolean setsystemclock (unsigned long secs) {

	/*
	3/10/97 dmb: set the system clock, using Macintosh time 
	conventions (seconds since 12:00 PM Jan 1, 1904)
	*/

    
		return (!oserror(unsupportedOSErr));

	} /*setsystemclock*/


static void
adjustforcurrenttimezone (unsigned long *ptime)
{
	/*
	5.1b23 dmb: avoid wraparound for near-zero dates
	*/

	unsigned long adjustedtime = *ptime + getcurrenttimezonebias ();

	if (sgn (*ptime) == sgn (adjustedtime))
		*ptime = adjustedtime;
	} /*adjustforcurrenttimezone*/


boolean
timegreaterthan (
		unsigned long time1,
		unsigned long time2)
{
	return (time1 > time2);
	} /*timegreaterthan*/


boolean
timelessthan (
		unsigned long time1,
		unsigned long time2)
{
	return (time1 < time2);
} /*timelessthan*/


boolean timetotimestring (unsigned long ptime, bigstring bstime, boolean flwantseconds) {

    
        CFLocaleRef locale = CFLocaleCopyCurrent();
        CFDateFormatterRef timeFormatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, flwantseconds ? kCFDateFormatterMediumStyle : kCFDateFormatterShortStyle);
        CFStringRef timeString = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, timeFormatter, ptime - kCFAbsoluteTimeIntervalSince1904);
        CFStringGetPascalString(timeString, bstime, sizeof(bigstring), kCFStringEncodingMacRoman);
        CFRelease(timeString);
        CFRelease(timeFormatter);
        CFRelease(locale);

		return (true);

	} /*timetotimestring*/


boolean timetodatestring (unsigned long ptime, bigstring bsdate, boolean flabbreviate) {


        CFLocaleRef locale = CFLocaleCopyCurrent();
        CFDateFormatterRef formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, flabbreviate ? kCFDateFormatterMediumStyle : kCFDateFormatterShortStyle, kCFDateFormatterNoStyle);
        CFStringRef dateString = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, formatter, ptime - kCFAbsoluteTimeIntervalSince1904);
        
        CFStringGetPascalString(dateString, bsdate, sizeof(bigstring), kCFStringEncodingMacRoman);
        
        CFRelease(dateString);
        CFRelease(formatter);
        CFRelease(locale);

        return (true);

	} /*timetodatestring*/


boolean stringtotime (bigstring bsdate, unsigned long *ptime) {
	
	/*
	9/13/91 dmb: use the script manager to translate a string to a 
	time in seconds since 12:00 AM 1904.
	
	if a date is provided, but no time, the time is 12:00 AM
	
	if a time is provided, but no date, the date is 1/1/1904.
	
	we return true if any time/date information was extracted.
	*/
	
    
        boolean flUseGMT = false;
        long idx;
        
        idx = stringlength (bsdate);
        
        while (getstringcharacter(bsdate, idx-1) == ' ')
            --idx;
        
        if (idx > 3) {
            if ((getstringcharacter(bsdate, idx - 3) == 'G') && (getstringcharacter(bsdate, idx - 2) == 'M') && (getstringcharacter(bsdate, idx -1) == 'T')) {
                flUseGMT = true;
            }
        }
        
        *ptime = 0; /*default return value*/

        
        CFStringRef dateString = CFStringCreateWithPascalString(kCFAllocatorDefault, bsdate, kCFStringEncodingMacRoman);
        CFAbsoluteTime absoluteTime = stringToDate(dateString);
        CFRelease(dateString);
        if (absoluteTime > 0) {
            *ptime = (absoluteTime + kCFAbsoluteTimeIntervalSince1904);
            if (flUseGMT) {
                adjustforcurrenttimezone (ptime);
            }
            return true;
        } else {
            return false;
        }


	return (true);
	} /*stringtotime*/


long datetimetoseconds (short day, short month, short year, short hour, short minute, short second) {
	
	/*
	5.0a12 dmb: Win version, must handle hour, minute, second wraparound
	*/

    
        unsigned long secs = convertDateTimeToSeconds(day, month, year, hour, minute, second) + kCFAbsoluteTimeIntervalSince1904;


	
	return (secs);
	} /*datetimetoseconds*/


void secondstodatetime (long secs, short *day, short *month, short *year, short *hour, short *minute, short *second) {
	
    
        CFAbsoluteTime timeInterval = ((uint32_t) secs) - kCFAbsoluteTimeIntervalSince1904;
        
        convertSecondsToDateTime(timeInterval, day, month, year, hour, minute, second);

	} /*secondstodatetime*/


void secondstodayofweek (long secs, short *dayofweek) {
	
    
        CFAbsoluteTime timeInterval = ((uint32_t)secs) - kCFAbsoluteTimeIntervalSince1904;
        *dayofweek = convertSecondsToDayOfWeek(timeInterval);

	} /*secondstodayofweek*/


static void fixdate (tydate * date) {
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

	} /*fixdate*/


unsigned long nextmonth(unsigned long date) {

	/*
	6.0a10 dmb: limit day to max days for new month
	*/
	
    
        CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;
        CFAbsoluteTime incrementedTime = incrementDateByMonth(timeInterval, 1);

		return (incrementedTime + kCFAbsoluteTimeIntervalSince1904);

	} /*nextmonth*/

unsigned long nextyear(unsigned long date) {
    
        CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;
        CFAbsoluteTime incrementedTime = incrementDateByYear(timeInterval, 1);

		return (incrementedTime + kCFAbsoluteTimeIntervalSince1904);

	} /*nextyear*/

unsigned long prevmonth(unsigned long date) {

	/*
	6.0a10 dmb: limit day to max days for new month
	*/
	
    
        CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;
        CFAbsoluteTime incrementedTime = incrementDateByMonth(timeInterval, -1);

		return (incrementedTime + kCFAbsoluteTimeIntervalSince1904);

	} /*prevmonth*/

unsigned long prevyear(unsigned long date) {
    
        CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;
        CFAbsoluteTime incrementedTime = incrementDateByYear(timeInterval, 1);

		return (incrementedTime + kCFAbsoluteTimeIntervalSince1904);

	} /*prevyear*/

unsigned long firstofmonth(unsigned long date) {
    
        CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;
        CFAbsoluteTime newTime = getFirstDayOfMonth(timeInterval);
        
        return newTime + kCFAbsoluteTimeIntervalSince1904;


	} /*firstofmonth*/

unsigned long lastofmonth(unsigned long date) {
    
        CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;
        CFAbsoluteTime newTime = getLastDayOfMonth(timeInterval);
        
        return newTime + kCFAbsoluteTimeIntervalSince1904;


	} /*lastofmonth*/


#define DATE_STRING(date, bs, format) \
    CFLocaleRef locale = CFLocaleCopyCurrent();\
    CFDateFormatterRef formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, format, kCFDateFormatterNoStyle);\
    CFAbsoluteTime timeInterval = date - kCFAbsoluteTimeIntervalSince1904;\
    CFStringRef dateString = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, formatter, timeInterval);\
    CFStringGetPascalString(dateString, bs, sizeof(bigstring), kCFStringEncodingMacRoman);\
    CFRelease(dateString);\
    CFRelease(formatter);\
    CFRelease(locale);\


void shortdatestring (unsigned long date, bigstring bs) {
        DATE_STRING(date, bs, kCFDateFormatterShortStyle)

	} /*shortdatestring*/

void longdatestring (unsigned long date, bigstring bs) {
        DATE_STRING(date, bs, kCFDateFormatterLongStyle)

	} /*longdatestring*/

void abbrevdatestring (unsigned long date, bigstring bs) {
        DATE_STRING(date, bs, kCFDateFormatterMediumStyle)

	} /*abbrevdatestring*/

void getdaystring (short dayofweek, bigstring bs, boolean flFullname) {
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

	} /*getdaystring*/

long getcurrenttimezonebias(void) {
		MachineLocation ml;
		long res;

		ReadLocation (&ml);
		
		res = ml.u.gmtDelta & 0x00FFFFFF;

		if ((res & 0x00800000) == 0x00800000)  //if this is a negative number extend the sign bits
			res = res | 0xFF000000;

		return (res);

	} /*getcurrenttimezonebias*/





