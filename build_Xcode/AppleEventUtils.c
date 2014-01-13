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
static OSStatus findProcessBySignature(const OSType creator, const OSType type, ProcessSerialNumberPtr psn);
static OSStatus getProcessInformation(const ProcessSerialNumberPtr psn, ProcessInfoRecPtr pir);
static OSErr createAppleEventProcessTarget(const ProcessSerialNumberPtr psn, AEEventClass eventClass, AEEventID eventID, AppleEvent *appleEvent);

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

static OSStatus findProcessBySignature(const OSType creator, const OSType type, ProcessSerialNumberPtr psn) {
	OSStatus err = noErr;
	
	psn->lowLongOfPSN = kNoProcess;
	psn->highLongOfPSN = kNoProcess;
	
	while (!(err = GetNextProcess(psn))) {
		ProcessInfoRec pir;
		
		if (!(err = getProcessInformation(psn, &pir))) {
			if ((creator == pir.processSignature) && (type == pir.processType)) {
				break;
			}
		}
	}
	
	return err;
}

static OSStatus getProcessInformation(const ProcessSerialNumberPtr psn, ProcessInfoRecPtr pir) {
	pir->processInfoLength = sizeof (*pir);
	pir->processName = nil;
	pir->processAppSpec = nil;
	
	if (psn) {
		return GetProcessInformation(psn, pir);
	} else {
		ProcessSerialNumber noProcPSN = { kNoProcess, kCurrentProcess };
		return GetProcessInformation(&noProcPSN, pir);
	}
}

static OSErr createAppleEventProcessTarget(const ProcessSerialNumberPtr psn, AEEventClass eventClass, AEEventID eventID, AppleEvent *appleEvent) {
	OSErr err = noErr;
	AEDesc targetAppDesc = {typeNull, nil};
	
	err = AECreateDesc(typeProcessSerialNumber, psn, sizeof (ProcessSerialNumber), &targetAppDesc);
	
	if (noErr == err) {
		err = AECreateAppleEvent(eventClass, eventID, &targetAppDesc, kAutoGenerateReturnID, kAnyTransactionID, appleEvent);
	}
	
	disposeAEDesc(&targetAppDesc);
	
	return err;
}

OSErr createAppleEventSignatureTarget(OSType type, OSType creator, AEEventClass eventClass, AEEventID eventID, AppleEvent *appleEvent) {
	OSErr err = noErr;
	ProcessSerialNumber psn = { kNoProcess, kNoProcess };
	
	err = findProcessBySignature(creator, type, &psn);
	
	if (noErr == err) {
		err = createAppleEventProcessTarget(&psn, eventClass, eventID, appleEvent);
	}
	
	return err;
}

OSErr addAliasParameterFromFSRef(const FSRefPtr fsRef, const DescType keyword, AERecord *record) {
	OSErr err = noErr;
	AliasHandle alias;
	
	err = FSNewAlias(nil, fsRef, &alias);
	if (noErr == err && alias == nil) {
		err = paramErr;
	}
	
	if (noErr == err) {
		SInt8 handleState = HGetState((Handle)alias);
		Size handleSize = GetHandleSize((Handle)alias);

		HLock((Handle)alias);
		
		err = AEPutParamPtr(record, keyword, typeAlias, *alias, handleSize);
		
		HSetState((Handle)alias, handleState);
		DisposeHandle((Handle)alias);
	}
	
	return err;
}