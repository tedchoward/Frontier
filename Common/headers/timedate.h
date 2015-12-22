
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

#ifndef timedateinclude
#define timedateinclude

#pragma pack(2)
typedef struct tyinternationalinfo {
	char * longDaysOfWeek[10];
	char * shortDaysOfWeek[10];
	char * longMonths[13];
	char * shortMonths[13];
	char * longYears[12];		//Year descriptions
	char * morning;
	char * evening;
	char * military;
	char * currency;			//for the US this is $
	char * intlCurrency;		//for the US this is USD
	char * shortDateFormatPattern;
	char * longDateFormatPattern;
	char * decimal;
	char * thousands;
	char * list;
	char * timesep;
	char * datesep;
	short numberOfDays;			//Usally 7 plus 3 for Yesterday, Today, and Tomorrow
	short numberOfMonths;
	short daysInMonth[13];		//For Gregorian calendar February is listed as 28 and corrected in code.
	short numberOfYears;
	boolean defaultTimeFormat;	//false = 12hour
	} tyinternationalinfo, * tyinternationalinfoptr;
#pragma options align=reset


#ifdef MACVERSION
/* #define getlongermilliseconds() (unsigned long long)FastMilliseconds() */
#define getmilliseconds() (long)FastMilliseconds()
#endif

/*prototypes*/

extern void timestamp (long *);

extern unsigned long timenow (void);

extern boolean setsystemclock (unsigned long);

extern boolean timegreaterthan (unsigned long, unsigned long);

extern boolean timelessthan (unsigned long, unsigned long);

extern boolean timetotimestring (unsigned long, bigstring, boolean);

extern boolean timetodatestring (unsigned long, bigstring, boolean);

extern boolean stringtotime (bigstring, unsigned long *);

extern long datetimetoseconds (short, short, short, short, short, short);

extern void secondstodatetime (long, short *, short *, short *, short *, short *, short *);

extern void secondstodayofweek (long, short *);

#ifdef WIN95VERSION

extern long filetimetoseconds (const FILETIME *);

extern void secondstofiletime (long seconds, FILETIME *);

extern tyinternationalinfoptr getIntlInfo ();

#endif

extern unsigned long nextmonth(unsigned long date);

extern unsigned long nextyear(unsigned long date);

extern unsigned long prevmonth(unsigned long date);

extern unsigned long prevyear(unsigned long date);

extern unsigned long firstofmonth(unsigned long date);

extern unsigned long lastofmonth(unsigned long date);

extern short daysInMonth (short month, short year);

extern void shortdatestring (unsigned long date, bigstring bs);

extern void longdatestring (unsigned long date, bigstring bs);

extern void abbrevdatestring (unsigned long date, bigstring bs);

extern void getdaystring (short dayofweek, bigstring bs, boolean flFullname);

extern long getcurrenttimezonebias(void);

extern boolean isLeapYear (short year);

#ifdef WIN95VERSION

extern long getmilliseconds(void);  /* mac version is #defined near the top of the file */

#endif /* WIN95VERSION */

#endif /*timedateinclude*/
