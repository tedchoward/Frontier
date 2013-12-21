//
//  AboutWindowController.m
//  Frontier Universal Binary
//
//  Created by Ted Howard on 12/20/13.
//  Copyright 2013 __MyCompanyName__. All rights reserved.
//

#import "AboutWindowController.h"
#include "frontier.h"
#include "standard.h"
#include "about.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "versions.h"

static AboutWindowController *sharedController;

extern void aboutsegment () {
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//	NSLog(@"aboutsegment()");
//	[pool release];
}

// display splash screen
boolean openabout(boolean flzoom, long ctreservebytes) {
#pragma unused(flzoom, ctreservebytes)
	return true;
}

void closeabout(boolean flzoom, short minticks) {
#pragma unused(flzoom, minticks)
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSLog(@"closeabout(%s, %hd)", flzoom ? "YES" : "NO", minticks);
	AboutWindowController *awc = [AboutWindowController sharedController];
	
	if (awc) {
		[awc hideWindow];
	}
	
	[pool release];	
}

// display about window
boolean aboutcommand() {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSLog(@"aboutcommand()");
	
	AboutWindowController *awc = [AboutWindowController sharedController];
	if (!awc) awc = [[AboutWindowController alloc] init];
	
	[awc showWindow];
	
	
	[pool release];
	return true;
}

boolean aboutstart() {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSLog(@"aboutstart()");
	AboutWindowController *awc = [AboutWindowController sharedController];
	if (!awc) awc = [[AboutWindowController alloc] init];

	[pool release];
	return true;
}

boolean aboutsetmiscstring(bigstring bsmisc) {
	return ccmsg(bsmisc, false);
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//	NSLog(@"aboutsetmiscstring(%.*s)", bsmisc[0], &bsmisc[1]);
//	[pool release];
//	return false;
}

boolean aboutsetthreadstring(hdlprocessthread hp, boolean flin) {
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//	NSLog(@"aboutsetthreadstring(%p, %s)", hp, flin ? "YES" : "NO");
//	[pool release];
	return false;
}

boolean aboutstatsshowing() {
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//	NSLog(@"aboutstatsshowing()");
//	[pool release];
	return false;
}

void aboutsetstatsflag(boolean fl) {
//	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
//	NSLog(@"aboutsetstatsflag(%s)\n", fl ? "YES" : "NO");
//	[pool release];
}

/* defined in cancooninternal.h */
boolean ccmsg(bigstring bs, boolean flbackgroundmsg) {
	AboutWindowController *sharedController = [AboutWindowController sharedController];
	if (sharedController == nil) return false;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	CFStringRef messageString = CFStringCreateWithPascalString(kCFAllocatorDefault, bs, kCFStringEncodingMacRoman);
	
	NSLog(@"ccmsg(%@, %s)", messageString, flbackgroundmsg ? "YES" : "NO");
	
	if (!flbackgroundmsg) {
		[sharedController setMessageText:(NSString *)messageString];
	}
	
	CFRelease(messageString);
	[pool release];
	
	return true;
}


@implementation AboutWindowController

+ (AboutWindowController *)sharedController {
	return sharedController;
}

- (id)init {
	self = [super init];
	
	if (self) {
		[NSBundle loadNibNamed:@"AboutWindow" owner:self];
		[[_window standardWindowButton:NSWindowZoomButton] setEnabled:NO];
		[_messageTextField setFont:[NSFont fontWithName:@"Geneva" size:13.0]];
		[_iconLabel setFont:[NSFont boldSystemFontOfSize:9.0]];
		[_iconLabel setStringValue:(NSString *)cf_APP_NAME];
		WebPreferences *prefs = [WebPreferences standardPreferences];
		[prefs setStandardFontFamily:@"Geneva"];
		[prefs setDefaultFontSize:9];
		[prefs setJavaEnabled:NO];
		[prefs setJavaScriptEnabled:NO];
		[prefs setPlugInsEnabled:NO];
		
		[_webView setPreferences:prefs];
		[_webView setDrawsBackground:NO];
		[[_window contentView] addSubview:_webView];
		
		NSString *aboutText = [[NSString alloc] initWithFormat:@"<i>%@</i><br>%@<br>%@<br><a href=\"%@\">%@</a>", (NSString *)cf_APP_SLOGAN, (NSString *)cf_APP_COPYRIGHT2, (NSString *)cf_APP_COPYRIGHT, (NSString *)cf_APP_URL, (NSString *)cf_APP_URL];
		
		[[_webView mainFrame] loadHTMLString:aboutText baseURL:nil];
		
		[aboutText release];
	}
	
	if (sharedController != nil) [sharedController release];
	sharedController = self;
	
	return self;
}

- (NSTextField *)messageTextField {
	return _messageTextField;
}

- (void)setMessageText:(NSString *)messageText {
	[_messageTextField setStringValue:messageText];
}

- (void)showWindow {
	[_window makeKeyAndOrderFront:nil];
}

- (void)hideWindow {
//	[_window performClose:nil];
	[_window orderOut:nil];
}

- (void)dealloc {
	if (self == sharedController) {
		sharedController = nil;
	}
	
	[super dealloc];
}

@end
