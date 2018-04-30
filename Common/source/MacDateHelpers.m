/*    $Id$    */

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

#include "MacDateHelpers.h"

#import <Foundation/Foundation.h>

CFAbsoluteTime stringToTime(CFStringRef dateString) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFAbsoluteTime timeInterval = 0;
    CFTimeZoneRef timeZone = CFTimeZoneCopyDefault();
    
    CFDateFormatterRef formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterShortStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterMediumStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterLongStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterFullStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }

    CFRelease(formatter);
    CFRelease(timeZone);
    CFRelease(locale);
    return 0;
}

CFAbsoluteTime stringToDateTime(CFStringRef dateString, CFDateFormatterStyle dateStyle) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFAbsoluteTime timeInterval = 0;
    CFTimeZoneRef timeZone = CFTimeZoneCopyDefault();
    
    CFDateFormatterRef formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, dateStyle, kCFDateFormatterShortStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, dateStyle, kCFDateFormatterMediumStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, dateStyle, kCFDateFormatterLongStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, dateStyle, kCFDateFormatterFullStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    if (CFDateFormatterGetAbsoluteTimeFromString(formatter, dateString, NULL, &timeInterval)) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        return timeInterval;
    }

    CFRelease(formatter);
    CFRelease(timeZone);
    CFRelease(locale);
    return 0;
}

CFAbsoluteTime stringToDate(CFStringRef dateString) {
    CFLocaleRef locale = CFLocaleCopyCurrent();
    CFTimeZoneRef timeZone = CFTimeZoneCopyDefault();
    CFIndex stringLength = CFStringGetLength(dateString);
    CFRange parseRange = CFRangeMake(0, stringLength);
    
    CFDateFormatterRef formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterNoStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    CFDateRef date = CFDateFormatterCreateDateFromString(kCFAllocatorDefault, formatter, dateString, &parseRange);
    
    if (date != NULL) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        
        if (parseRange.length != stringLength) {
            CFRelease(date);
            return stringToDateTime(dateString, kCFDateFormatterShortStyle);
        } else {
            CFAbsoluteTime absoluteTime = CFDateGetAbsoluteTime(date);
            CFRelease(date);
            return absoluteTime;
        }
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterMediumStyle, kCFDateFormatterNoStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    date = CFDateFormatterCreateDateFromString(kCFAllocatorDefault, formatter, dateString, &parseRange);
    
    if (date != NULL) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        
        if (parseRange.length != stringLength) {
            CFRelease(date);
            return stringToDateTime(dateString, kCFDateFormatterMediumStyle);
        } else {
            CFAbsoluteTime absoluteTime = CFDateGetAbsoluteTime(date);
            CFRelease(date);
            return absoluteTime;
        }
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterLongStyle, kCFDateFormatterNoStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    date = CFDateFormatterCreateDateFromString(kCFAllocatorDefault, formatter, dateString, &parseRange);
    
    if (date != NULL) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        
        if (parseRange.length != stringLength) {
            CFRelease(date);
            return stringToDateTime(dateString, kCFDateFormatterLongStyle);
        } else {
            CFAbsoluteTime absoluteTime = CFDateGetAbsoluteTime(date);
            CFRelease(date);
            return absoluteTime;
        }
    }
    
    CFRelease(formatter);
    formatter = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterFullStyle, kCFDateFormatterNoStyle);
//    CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timeZone);
    date = CFDateFormatterCreateDateFromString(kCFAllocatorDefault, formatter, dateString, &parseRange);
    
    if (date != NULL) {
        CFRelease(formatter);
        CFRelease(timeZone);
        CFRelease(locale);
        
        if (parseRange.length != stringLength) {
            CFRelease(date);
            return stringToDateTime(dateString, kCFDateFormatterFullStyle);
        } else {
            CFAbsoluteTime absoluteTime = CFDateGetAbsoluteTime(date);
            CFRelease(date);
            return absoluteTime;
        }
    }
    
    CFRelease(formatter);
    CFRelease(timeZone);
    CFRelease(locale);
    
    return stringToTime(dateString);
}

CFAbsoluteTime convertDateTimeToSeconds(int16_t day, int16_t month, int16_t year, int16_t hour, int16_t minute, int16_t second) {
    CFAbsoluteTime absoluteTime = 0;
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSDateComponents *components = [[NSDateComponents alloc] init];
    
    [components setDay:day];
    [components setMonth:month];
    [components setYear:year];
    [components setHour:hour];
    [components setMinute:minute];
    [components setSecond:second];
    
    NSCalendar *calendar = [NSCalendar currentCalendar];
    NSDate *date = [calendar dateFromComponents:components];
    [components release];
    
    absoluteTime = [date timeIntervalSinceReferenceDate];
    
    [pool release];
    
    return absoluteTime;
}

void convertSecondsToDateTime(CFAbsoluteTime secs, int16_t *day, int16_t *month, int16_t *year, int16_t *hour, int16_t *minute, int16_t *second) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    NSCalendar *calendar = [NSCalendar currentCalendar];
    CFDateRef date = CFDateCreate(kCFAllocatorDefault, secs);

    NSDateComponents *components = [calendar components:(NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit)
                                               fromDate:(NSDate *)date];
    
    *day = [components day];
    *month = [components month];
    *year = [components year];
    *hour = [components hour];
    *minute = [components minute];
    *second = [components second];
    
    CFRelease(date);
    
    [pool release];
}

int16_t convertSecondsToDayOfWeek(CFAbsoluteTime secs) {
    NSInteger weekDay = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CFDateRef date = CFDateCreate(kCFAllocatorDefault, secs);
    NSCalendar *calendar = [NSCalendar currentCalendar];
    NSDateComponents *components = [calendar components:NSWeekdayCalendarUnit fromDate:(NSDate *)date];
    weekDay = [components weekday];
    
    CFRelease(date);
    [pool release];
    
    return weekDay;
}

CFAbsoluteTime incrementDateByDay(CFAbsoluteTime time, int16_t days) {
    CFAbsoluteTime newTime = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CFDateRef date = CFDateCreate(kCFAllocatorDefault, time);
    NSCalendar *calendar = [NSCalendar currentCalendar];
    
    NSDateComponents *components = [[NSDateComponents alloc] init];
    [components setDay:days];
    
    NSDate *newDate = [calendar dateByAddingComponents:components toDate:(NSDate *)date options:0];
    [components release];
    CFRelease(date);
    
    newTime = CFDateGetAbsoluteTime((CFDateRef)newDate);
    
    [pool release];
    return newTime;
}


CFAbsoluteTime incrementDateByMonth(CFAbsoluteTime time, int16_t months) {
    CFAbsoluteTime newTime = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CFDateRef date = CFDateCreate(kCFAllocatorDefault, time);
    NSCalendar *calendar = [NSCalendar currentCalendar];
    
    NSDateComponents *components = [[NSDateComponents alloc] init];
    [components setMonth:months];
    
    NSDate *newDate = [calendar dateByAddingComponents:components toDate:(NSDate *)date options:0];
    [components release];
    CFRelease(date);
    
    newTime = CFDateGetAbsoluteTime((CFDateRef)newDate);
    
    [pool release];
    return newTime;
}

CFAbsoluteTime incrementDateByYear(CFAbsoluteTime time, int16_t years) {
    CFAbsoluteTime newTime = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CFDateRef date = CFDateCreate(kCFAllocatorDefault, time);
    NSCalendar *calendar = [NSCalendar currentCalendar];
    
    NSDateComponents *components = [[NSDateComponents alloc] init];
    [components setYear:years];
    
    NSDate *newDate = [calendar dateByAddingComponents:components toDate:(NSDate *)date options:0];
    [components release];
    CFRelease(date);
    
    newTime = CFDateGetAbsoluteTime((CFDateRef)newDate);
    
    [pool release];
    return newTime;
}

CFAbsoluteTime getFirstDayOfMonth(CFAbsoluteTime time) {
    CFAbsoluteTime newTime = 0;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CFDateRef date = CFDateCreate(kCFAllocatorDefault, time);
    NSCalendar *calendar = [NSCalendar currentCalendar];
    NSDateComponents *components = [calendar components:(NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit)
                                               fromDate:(NSDate *)date];

    [components setDay:1];
    NSDate *newDate = [calendar dateFromComponents:components];
    newTime = CFDateGetAbsoluteTime((CFDateRef)newDate);
    
    [pool release];
    
    return newTime;
}

CFAbsoluteTime getLastDayOfMonth(CFAbsoluteTime time) {
    CFAbsoluteTime nextMonth = incrementDateByMonth(time, 1);
    CFAbsoluteTime firstOfNext = getFirstDayOfMonth(nextMonth);
    return incrementDateByDay(firstOfNext, -1);
}
