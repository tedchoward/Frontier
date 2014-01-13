/*
 *  AppleEventUtils.h
 *  Frontier Universal Binary
 *
 *  Created by Ted Howard on 1/10/14.
 *  Copyright 2014 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __APPLE_EVENT_UTILS_H__
#define __APPLE_EVENT_UTILS_H__

OSErr sendEventReturnBigstring(const AEIdleUPP idleProcUPP, const AppleEvent *event, Str255 returnValue);
OSErr sendEventReturnVoid(const AEIdleUPP idleProcUPP, const AppleEvent *event);
OSStatus createObjectSpecifierFromURL(const CFURLRef url, AEDesc *objectSpecifier);
void disposeAEDesc(AEDesc *desc);
AEIdleUPP createIdleUPP(void);

#endif