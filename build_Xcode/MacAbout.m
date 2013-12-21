/*
 *  MacAbout.c
 *  Frontier Universal Binary
 *
 *  Created by Ted Howard on 12/6/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>

#include "frontier.h"
#include "standard.h"
#include "versions.h"
#include "about.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "AboutTextView.h"
#import "AboutViewController.h"

OSStatus loadCocoaView();

static WindowRef _window;

typedef struct tyaboutrecord {
	Rect messageArea;
	Rect aboutArea;
	boolean flBootSplash;
	boolean flBigWindow;
	boolean flExtraStats;
	SInt32 refcon;
} tyaboutrecord, *ptraboutrecord, **hdlaboutrecord;
	
static hdlaboutrecord _aboutData;

boolean createAboutWindow(void);

#pragma mark - public functions

void aboutsegment(void) {
	fprintf(stderr, "aboutsegment()\n");
}

boolean openabout(boolean flzoom, long ctreservebytes) {
	fprintf(stderr, "openabout(%s, %ld)\n", flzoom ? "true" : "false", ctreservebytes);
	
	boolean success = false;
	
	if (_window == NULL) {
		success = createAboutWindow();
	}
	
	if (success) {
		ShowWindow(_window);
	}
	
	return success;
}

void closeabout(boolean flzoom, short minticks) {
	fprintf(stderr, "closeabout(%s, %hd)\n", flzoom ? "true" : "false", minticks);
//	DisposeWindow(_window);
}

boolean aboutcommand(void) {
	fprintf(stderr, "aboutcommand()\n");
	return false;
}

boolean aboutstart(void) {
	fprintf(stderr, "aboutstart()\n");
	return true;
}

boolean aboutsetmiscstring(bigstring bsmisc) {
	fprintf(stderr, "aboutsetmiscstring(%.*s)\n", bsmisc[0], &bsmisc[1]);
	return false;
}

boolean aboutsetthreadstring(hdlprocessthread hp, boolean flin) {
//	fprintf(stderr, "aboutsetthreadstring(%p, %s)\n", hp, flin ? "true" : "false");
	return true;
}

boolean aboutstatsshowing(void) {
	fprintf(stderr, "aboutstatsshowing()\n");
	return false;
}

void aboutsetstatsflag(boolean fl) {
	fprintf(stderr, "aboutsetstatsflag(%s)\n", fl ? "true" : "false");
}

/* defined in cancoocinternal.h */
boolean ccmsg(bigstring bs, boolean flbackgroundmsg) {
	CFStringRef messageText = CFStringCreateWithPascalString(kCFAllocatorDefault, bs, kCFStringEncodingMacRoman);
	setMessageText((void *)GetWRefCon(_window), messageText);
	fprintf(stderr, "ccmsg(%.*s, %s)\n", bs[0], &bs[1], flbackgroundmsg ? "true" : "false");
	CFRelease(messageText);
	return true;
}

#pragma mark - internal functions

#define RectToCGRect(r) CGRectMake(r.left, r.top, r.right - r.left, r.bottom - r.top)

boolean createAboutWindow() {
	OSStatus err;
	IBNibRef theNib;
	HIRect windowBounds;
	Rect r;
	
	_aboutData = (hdlaboutrecord) NewHandleClear(sizeof(tyaboutrecord));
	
	err = aboutTextViewRegister();
	
	if (!err) {
	
	err = CreateNibReference(CFSTR("About"), &theNib);
		
		if (!err) {
			CreateWindowFromNib(theNib, CFSTR("About"), &_window);
			DisposeNibReference(theNib);
			
			CFStringRef windowTitle = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("About %@"), cf_APP_NAME);
			
			SetWindowTitleWithCFString(_window, windowTitle);
			
			ccgetwindowrect(ixaboutinfo, &r);
			
			windowBounds = RectToCGRect(r);
			
			HIWindowSetBounds(_window, kWindowStructureRgn, kHICoordSpaceWindow, &windowBounds);
			
			//SetWindowBounds(_window, kWindowStructureRgn, &windowBounds);
			//100, 100, 250, 260
			
			loadCocoaView();
			
			CFRelease(windowTitle);
			
			return true;
		}
	}
	
	return false;
}

const HIViewID kAboutViewControlID = {'View', 130 };

OSStatus loadCocoaView() {
	OSStatus err = noErr;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	AboutViewController *viewController = [[AboutViewController alloc] init];
	SetWRefCon(_window, (SRefCon) viewController); // so the view controller is disposed when the window is closed
	
	HIViewRef view;
	err = HIViewFindByID(HIViewGetRoot(_window), kAboutViewControlID, &view);
	NSView *cocoaView = [viewController view];
	
	if (cocoaView != NULL) {
		err = HICocoaViewSetView(view, cocoaView);
	}
	
	[pool release];
	
	return err;
}
