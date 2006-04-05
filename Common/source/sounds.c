
/*	$Id$    */

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

#include "frontier.h"
#include "standard.h"

#include "error.h"
#include "strings.h"
#include "sounds.h"

#define squareWaveSynth 1
#define ampCmd 43
#define freqDurationCmd 40

#if oldsounds

static struct {

    short mode;
    
    Tone triplets [2];
    
    } myTone = {swMode, 0};
    
static IOParam pb = {0, 0};



boolean dosound (short duration, short amplitude, short frequency) {
	
	/*
	duration is in 60ths of a second.
	
	amplitude can range between 0 and 255, it determines the volume.
	
	10/12/91 dmb: limit amplitude to 255 explicity here
	*/

	OSErr errcode;

    myTone.triplets [0].count = (short) (7833600 / frequency);
    
    myTone.triplets [0].amplitude = min (amplitude, 255);
    
    myTone.triplets [0].duration = duration;

    pb.ioRefNum = -4;
    
    pb.ioBuffer = (Ptr) &myTone;
    
    pb.ioReqCount = (long) sizeof (myTone);

    errcode = PBWrite ((ParmBlkPtr) &pb, false);
    
    return (errcode == noErr);
	} /*dosound*/

#else

boolean dosound (short duration, short amplitude, short frequency) {
	
#ifdef MACVERSION
	//#if TARGET_API_MAC_CARBON
	//sysbeep();
	//return true;
	//#else

	SndChannelPtr channel;
	SndCommand cmd;
	long ltime;
	float note;
	OSErr err;
	
	channel = nil;
	
	err = SndNewChannel (&channel, squareWaveSynth, 0, nil);
	
	if (oserror (err))
		return (false);
	
	cmd.cmd = ampCmd;
	cmd.param1 = amplitude;
	cmd.param2 = 0;
	err = SndDoCommand (channel, &cmd, false);
	
	ltime = duration * (2000/60);
	
	ltime = min (infinity, ltime);
	
	note = (frequency / 129.0) / 69;
	
	note = 69 + 12.0 * (note - 1);
	
	cmd.cmd = freqDurationCmd;
	cmd.param1 = ltime;
	cmd.param2 = note;
	err = SndDoCommand (channel, &cmd, false);
	
	/*
	cmd.cmd = noteCmd;
	cmd.param1 = 30;
	cmd.param2 = $FF000000 + 83;
	err = SndDoCommand (channel, &cmd, false);
	
	
	cmd.cmd = quietCmd;
	cmd.param1 = 0;
	cmd.param2 = 0;
	err = SndDoCommand (channel, &cmd, false);
	*/
	
	err = SndDisposeChannel (channel, false);
	
	return (!oserror (err));
//#endif
#endif

#ifdef WIN95VERSION
	//NOTE:  Beep does not take amplitude.  
	//       Also the parameters are ignored on Windows 95/98
	long ltime;

	ltime = (duration * 1000L) / 60L;
	return (Beep (frequency, ltime));
//	return (false); // *** WIN95 not implemented
#endif
	} /*dosound*/

#endif


void motorsound (void) {
	
	dosound (1, 100, 100);
	} /*motorsound*/


void ouch (void) {
	
#ifdef MACVERSION
	sysbeep (); 
#endif

#ifdef WIN95VERSIOON
	Beep(1000, 500);
#endif
	
	//dosound (1 /*duration*/, 250 /*amplitude*/, 14300 /*frequency*/);
	
	} /*ouch*/


boolean playnamedsound (bigstring bsname) {

#ifdef MACVERSION
	Handle hsound;
	
	hsound = GetNamedResource ('snd ', bsname);
	
	if (hsound == nil)
		return (false);
	
	return (SndPlay (nil, (SndListHandle) hsound, false) == noErr);
#endif

#ifdef WIN95VERSION
	char sndname[256];
	copyptocstring (bsname, sndname);
	return (PlaySound (sndname, NULL, SND_FILENAME | SND_NODEFAULT | SND_ASYNC));
//	return (false);	// *** WIN95 not implemented
#endif

	} /*playnamedsound*/



