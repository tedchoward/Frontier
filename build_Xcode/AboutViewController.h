//
//  AboutViewController.h
//  Frontier Universal Binary
//
//  Created by Ted Howard on 12/20/13.
//  Copyright 2013 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

void setMessageText(void *viewController, CFStringRef text);

@interface AboutViewController : NSViewController {
	IBOutlet NSTextField *_textField;
}

- (NSTextField *)textField;

@end
