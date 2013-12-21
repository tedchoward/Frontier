//
//  AboutWindowController.h
//  Frontier Universal Binary
//
//  Created by Ted Howard on 12/20/13.
//  Copyright 2013 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>


@interface AboutWindowController : NSObject {
	IBOutlet NSWindow *_window;
	IBOutlet NSTextField *_messageTextField;
	IBOutlet WebView *_webView;
	IBOutlet NSTextField *_iconLabel;
}

+ (AboutWindowController *)sharedController;

- (void)setMessageText:(NSString *)messageText;
- (void)showWindow;
- (void)hideWindow;

@end
