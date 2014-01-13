/*
 *  AppleEventUtils.c
 *  Frontier Universal Binary
 *
 *  Created by Ted Howard on 1/10/14.
 *  Copyright 2014 __MyCompanyName__. All rights reserved.
 *
 */

#include "AppleEventUtils.h"

#include "frontier.h"
#include "standard.h"
#include "shell.h"

static OSErr sendEventReturnData(const AEIdleUPP idleProcUPP, const AppleEvent *appleEvent, DescType desiredType, DescType *actualType, void *data, Size maxSize, Size *actualSize);
static OSErr getHandlerError(const AppleEvent *reply);
static Boolean idleProc(EventRecord *eventRecord, UInt32 *sleepTime, RgnHandle *mouseRgn);

OSErr sendEventReturnBigstring(const AEIdleUPP idleProcUPP, const AppleEvent *event, bigstring returnValue) {
	DescType actualType;
	Size actualSize;
	OSErr err;
	
	err = sendEventReturnData(idleProcUPP, event, typePString, &actualType, returnValue, sizeof (bigstring), &actualSize);
	
	if (errAECoercionFail == err) {
		err = sendEventReturnData(idleProcUPP, event, typeChar, &actualType, (Ptr) &returnValue[1], sizeof (bigstring), &actualSize);
		
		if (actualSize < 256) {
			returnValue[0] = actualSize;
		} else {
			err = errAECoercionFail;
		}
	}
	
	return err;
}

OSErr sendEventReturnVoid(const AEIdleUPP idleProcUPP, const AppleEvent *event) {
	OSErr err = noErr;
	AppleEvent reply = {typeNull, nil};
	AESendMode sendMode;
	
	sendMode = (nil == idleProcUPP) ? kAENoReply : kAEWaitReply;
	
	err = AESend(event, &reply, sendMode, kAENormalPriority, kNoTimeOut, idleProcUPP, nil);
	
	if ((noErr == err) && (kAEWaitReply == sendMode)) {
		err = getHandlerError(&reply);
	}
	
	disposeAEDesc(&reply);
	
	return err;
}

static OSErr sendEventReturnData(const AEIdleUPP idleProcUPP, const AppleEvent *appleEvent, DescType desiredType, DescType *actualType, void *data, Size maxSize, Size *actualSize) {
	OSErr err = noErr;
	
	if (idleProcUPP == nil) {
		err = paramErr;
	} else {
		AppleEvent reply = {typeNull, nil};
		AESendMode sendMode = kAEWaitReply;
		
		err = AESend(appleEvent, &reply, sendMode, kAENormalPriority, kNoTimeOut, idleProcUPP, nil);
		
		if (noErr == err) {
			err = getHandlerError(&reply);
			
			if (!err & reply.descriptorType != typeNull) {
				err = AEGetParamPtr(&reply, keyDirectObject, desiredType, actualType, data, maxSize, actualSize);
			}
			
			disposeAEDesc(&reply);
		}
	}
	
	return err;
}

static OSErr getHandlerError(const AppleEvent *reply) {
	OSErr err = noErr;
	OSErr handlerErr;
	
	DescType actualType;
	Size actualSize;
	
	if (reply->descriptorType != typeNull) {
		OSErr getErrErr = noErr;
		
		getErrErr = AEGetParamPtr(reply, keyErrorNumber, typeShortInteger, &actualType, &handlerErr, sizeof (OSErr), &actualSize);
		
		if (getErrErr != errAEDescNotFound) {
			err = handlerErr;
		}
	}
	
	return err;
}

void disposeAEDesc(AEDesc *desc) {
	OSStatus err;
	
	err = AEDisposeDesc(desc);
	
	desc->descriptorType = typeNull;
	desc->dataHandle = nil;
}

OSStatus createObjectSpecifierFromURL(const CFURLRef url, AEDesc *objectSpecifier) {
	OSErr err = paramErr;
	
	if (nil != url) {
		Boolean isDirectory = CFURLHasDirectoryPath(url);
		CFStringRef filePath = CFURLCopyFileSystemPath(url, kCFURLHFSPathStyle);
		AEDesc containerDesc = {typeNull, NULL};
		AEDesc nameDesc = {typeNull, NULL};
		UniCharPtr buf = nil;
		
		if (nil != filePath) {
			Size bufSize = (CFStringGetLength(filePath) + (isDirectory ? 1 : 0)) * sizeof (UniChar);
			buf = (UniCharPtr)NewPtr(bufSize);
			
			if ((err = MemError()) == noErr) {
				CFStringGetCharacters(filePath, CFRangeMake(0, bufSize / 2), buf);
				if (isDirectory) (buf)[(bufSize-1)/2] = (UniChar) 0x003A;
			}
		} else {
			err = coreFoundationUnknownErr;
		}
		
		CFRelease(filePath);
		
		if (err == noErr) {
			err = AECreateDesc(typeUnicodeText, buf, GetPtrSize((Ptr)buf), &nameDesc);
		}
		
		if (err == noErr) {
			err = CreateObjSpecifier(isDirectory ? cFolder : cFile, &containerDesc, formName, &nameDesc, false, objectSpecifier);
		}
		
		disposeAEDesc(&nameDesc);
		
		if (buf) DisposePtr((Ptr)buf);
	}
	
	return err;
}

static Boolean idleProc(EventRecord *eventRecord, UInt32 *sleepTime, RgnHandle *mouseRgn) {
#pragma unused (sleepTime, mouseRgn)
	
	if (eventRecord->what != nullEvent) {
		shellprocessevent(eventRecord);
	}
	
	return false;
}

AEIdleUPP createIdleUPP() {
	return NewAEIdleUPP(idleProc);
}