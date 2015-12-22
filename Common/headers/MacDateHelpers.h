//
//  MacDateHelpers.h
//  Frontier Universal Binary
//
//  Created by Ted Howard on 12/21/15.
//
//

#ifndef MacDateHelpers_h
#define MacDateHelpers_h

#include <CoreFoundation/CoreFoundation.h>

CFAbsoluteTime stringToTime(CFStringRef dateString);
CFAbsoluteTime stringToDateTime(CFStringRef dateString, CFDateFormatterStyle dateStyle);
CFAbsoluteTime stringToDate(CFStringRef dateString);
CFAbsoluteTime convertDateTimeToSeconds(int16_t day, int16_t month, int16_t year, int16_t hour, int16_t minute, int16_t second);
void convertSecondsToDateTime(CFAbsoluteTime secs, int16_t *day, int16_t *month, int16_t *year, int16_t *hour, int16_t *minute, int16_t *second);
int16_t convertSecondsToDayOfWeek(CFAbsoluteTime secs);
CFAbsoluteTime incrementDateByDay(CFAbsoluteTime time, int16_t days);
CFAbsoluteTime incrementDateByMonth(CFAbsoluteTime time, int16_t months);
CFAbsoluteTime incrementDateByYear(CFAbsoluteTime time, int16_t years);
CFAbsoluteTime getFirstDayOfMonth(CFAbsoluteTime time);
CFAbsoluteTime getLastDayOfMonth(CFAbsoluteTime time);

#endif /* MacDateHelpers_h */
