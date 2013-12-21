//
//  AboutViewController.m
//  Frontier Universal Binary
//
//  Created by Ted Howard on 12/20/13.
//  Copyright 2013 __MyCompanyName__. All rights reserved.
//

#import "AboutViewController.h"

void setMessageText(void *viewController, CFStringRef text) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSTextField *tf = [(AboutViewController *)viewController textField];
	
	[tf setStringValue:(NSString *)text];
	
	[pool release];
}


@implementation AboutViewController

- (id)init {
	self = [super initWithNibName:@"AboutViewController" bundle:nil];
	
	if (self) {
	}
	
	return self;
}

- (NSTextField *)textField {
	return _textField;
}

@end
