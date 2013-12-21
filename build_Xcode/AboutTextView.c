/*
 *  AboutTextView.c
 *  Frontier Universal Binary
 *
 *  Created by Ted Howard on 12/19/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include "AboutTextView.h"

#define kAboutTextViewClassID CFSTR("com.scripting.Frontier.AboutTextView")

typedef struct {
	HIViewRef view;
} AboutTextViewData;

OSStatus aboutTextViewHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData);
OSStatus aboutTextViewConstruct(EventRef inEvent);
OSStatus aboutTextViewDestruct(EventRef inEvent, AboutTextViewData *inData);
OSStatus aboutTextViewInitialize(EventHandlerCallRef inCallRef, EventRef inEvent, AboutTextViewData *inData);
OSStatus aboutTextViewDraw(EventRef inEvent, AboutTextViewData *inData);

void drawString(CGContextRef context, CFStringRef string, CGFloat x, CGFloat y);
void translateToQuartzCoordinates(CGContextRef context, HIRect bounds);
void translateToHICoordinates(CGContextRef context);	

OSStatus aboutTextViewRegister() {
	fprintf(stderr, "aboutTextViewRegister()\n");
	OSStatus err = noErr;
	static HIObjectClassRef sAboutTextViewClassRef = NULL;
	
	if (sAboutTextViewClassRef == NULL) {
		EventTypeSpec eventList[] = {
			{ kEventClassHIObject, kEventHIObjectConstruct },
			{ kEventClassHIObject, kEventHIObjectInitialize },
			{ kEventClassHIObject, kEventHIObjectDestruct },
			{ kEventClassControl, kEventControlInitialize },
			{ kEventClassControl, kEventControlDraw }//,
//			{ kEventClassControl, kEventControlHitTest },
//			{ kEventClassControl, kEventControlGetPartRegion }
		};
		
		err = HIObjectRegisterSubclass(
			kAboutTextViewClassID,			// class ID
			kHIViewClassID,					// base class ID
			0,							// option bits
			aboutTextViewHandler,			// construct proc
			GetEventTypeCount(eventList),
			eventList, 
			NULL,							// construct data
			&sAboutTextViewClassRef);
	}
	
	return err;
}

OSStatus aboutTextViewHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData) {
	fprintf(stderr, "aboutTextViewHandler(%p, %p, %p)\n", inCallRef, inEvent, inUserData);
	OSStatus err = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(inEvent);
	UInt32 eventKind = GetEventKind(inEvent);
	AboutTextViewData *data = (AboutTextViewData *)inUserData;
	
	switch (eventClass) {
		case kEventClassHIObject:
		{
			switch (eventKind) {
				case kEventHIObjectConstruct:
					err = aboutTextViewConstruct(inEvent);
					break;
					
				case kEventHIObjectInitialize:
					err = aboutTextViewInitialize(inCallRef, inEvent, data);
					break;
					
				case kEventHIObjectDestruct:
					err = aboutTextViewDestruct(inEvent, data);
					break;
			}
		}
			
		break;
			
		case kEventClassControl:
		{
			switch (eventKind) {
				case kEventControlInitialize:
					err = noErr;
					break;
					
				case kEventControlDraw:
					err = aboutTextViewDraw(inEvent, data);
					break;
					
//				case kEventControlHitTest:
//					err = aboutTextViewHitText(inEvent, data);
//					break;
//					
//				case kEventControlGetPartRegion:
//					err = aboutTextViewGetRegion(inEvent, data);
//					break;
			}
		}
			
		break;
	}
	
	return err;
}

OSStatus aboutTextViewConstruct(EventRef inEvent) {
	fprintf(stderr, "aboutTextViewConstruct(%p)\n", inEvent);
	OSStatus err;
	AboutTextViewData *data;
	
	data = malloc(sizeof (AboutTextViewData));
	require_action(data != NULL, CantMalloc, err = memFullErr);
	
	err = GetEventParameter(inEvent, kEventParamHIObjectInstance, 
							typeHIObjectRef, NULL, sizeof (HIObjectRef), 
							NULL, (HIObjectRef *) &data->view);
	require_noerr(err, ParameterMissing);
	
	// Set the userData to be used with all subsequent eventHandler calls
	err = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof (AboutTextViewData*), &data);
	
ParameterMissing:
	if (err != noErr) {
		free(data);
	}
	
CantMalloc:
	return err;
}

OSStatus aboutTextViewDestruct(EventRef inEvent, AboutTextViewData *inData) {
	fprintf(stderr, "aboutTextViewDestruct(%p, %p)", inEvent, inData);
	free(inData);
	return noErr;
}

OSStatus aboutTextViewInitialize(EventHandlerCallRef inCallRef, EventRef inEvent, AboutTextViewData *inData) {
	fprintf(stderr, "aboutTextViewInitialize(%p, %p, %p)", inCallRef, inEvent, inData);
	OSStatus err;
//	HIRect bounds;
	
	// Let any parent classes have a chance at initialization
	err = CallNextEventHandler(inCallRef, inEvent);
	require_noerr(err, TroubleInSuperClass);
	
	// Extract the bounds from the initialilzation event
//	err = GetEventParameter(inEvent, 'Boun', typeHIRect, NULL, sizeof (HIRect), NULL, &bounds);
//	require_noerr(err, ParameterMissing);
	
	// Resize the bounds
//	HIViewSetFrame(inData->view, &bounds);
	
//ParameterMissing:
TroubleInSuperClass:
	return err;
}

OSStatus aboutTextViewDraw(EventRef inEvent, AboutTextViewData *inData) {
	fprintf(stderr, "aboutTextViewDraw(%p, %p)", inEvent, inData);
	OSStatus err;
	CGContextRef context;
	
	err = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof (CGContextRef), NULL, &context);
	require_noerr(err, CGContextMissing);
	
	HIRect bounds;
	HIViewGetBounds(inData->view, &bounds);
	
	// Drawing code goes here
	translateToQuartzCoordinates(context, bounds);
	
	CGFloat x = 0.0;
	CGFloat y = bounds.size.height - 7.0;
	
	drawString(context, CFSTR("Powerful OPML editing on your desktop"), x, y);
	y -= 13.0;
	drawString(context, CFSTR("(c) 1992-2004 Userland Software, Inc."), x, y);
	y -= 13.0;
	drawString(context, CFSTR("(c) 2004-2013 Scripting News, Inc."), x, y);
	y -= 13.0;
	drawString(context, CFSTR("http://support.opml.org"), x, y);
	
	translateToHICoordinates(context);
		
//	CGContextSetLineWidth(context, 2.0);
//	
//	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
//	CGFloat components[] = { 0.0, 0.0, 1.0, 1.0 };
//	CGColorRef color = CGColorCreate(colorspace, components);
//	
//	CGContextSetStrokeColorWithColor(context, color);
//	
//	CGContextMoveToPoint(context, 0.0, 0.0);
//	CGContextAddLineToPoint(context, 422.0, 124.0);
//	
//	CGContextStrokePath(context);
//	CGColorSpaceRelease(colorspace);
//	CGColorRelease(color);
	
CGContextMissing:
	return err;
}

void translateToQuartzCoordinates(CGContextRef context, HIRect bounds) {
	CGContextSaveGState(context);
	CGContextTranslateCTM(context, 0, bounds.origin.y + bounds.size.height);
	CGContextScaleCTM(context, 1.0f, -1.0f);
}

void translateToHICoordinates(CGContextRef context) {
	CGContextRestoreGState(context);
}

void drawString(CGContextRef context, CFStringRef string, CGFloat x, CGFloat y) {
	CTFontRef font = CTFontCreateWithName(CFSTR("Geneva"), 9.0, NULL);
	
	CFStringRef keys[] = { kCTFontAttributeName };
	CFTypeRef values [] = { font };
	
	CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault, (const void **)&keys,
													(const void **)&values, sizeof (keys) / sizeof (keys[0]), 
													&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	CFRelease(font);
	
	CFAttributedStringRef attrString = CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
	CFRelease(attributes);
	
	CTLineRef line = CTLineCreateWithAttributedString(attrString);
	CFRelease(attrString);
	
	CGContextSetTextPosition(context, x, y);
	CTLineDraw(line, context);
	CFRelease(line);
	
}