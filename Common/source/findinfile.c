
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
#include "memory.h"
#include "strings.h"
#include "file.h"
#include "lang.h"
#include "langinternal.h"



#define filebuffersize 1024


typedef struct tyopenfile {
	
	tyfilespec fs; /*the file specifier*/
	
	hdlfilenum fnum; /*the file refnum*/
	
	Handle hbuffer; /*holds the file data*/
	
	long ixbuffer; /*points at the next character in the file*/
	
	long ctbufferbytes; /*number of valid chars in buffer*/
	
	long ctbytesleft; /*number of valid characters left unread*/
	
	long fpos; /*current file position*/
	
	struct tyopenfile **nextfile;
	
	long refcon;
	} tyopenfile, *ptropenfile, **hdlopenfile;
	
	
hdlopenfile hfirstfile = nil; /*header of file list*/
	


static boolean loadbufferfromfile (hdlfilenum fnum, Handle hbuffer, long *ctbytes) {

	boolean fl;
	long ctread;

	ctread = gethandlesize (hbuffer);
	
	lockhandle (hbuffer);
	
	fl = filereaddata (fnum, ctread, ctbytes, *hbuffer);
	
	unlockhandle (hbuffer);
	
	return (fl);
	} /*loadbufferfromfile*/
	
	
static boolean backupfilepos (hdlfilenum fnum, long ctbytes) {
	
	long origfpos, newfpos;
	boolean fl;
	
	filegetposition (fnum, &origfpos);
	
	if (origfpos >= ctbytes)
		newfpos = origfpos - ctbytes;
	else
		newfpos = 0L;

	fl = filesetposition (fnum, newfpos); 
	
	/* filegetposition (fnum, &newfpos); */
	
	return (fl);
	} /*backupfilepos*/


static void lowercasehandle (Handle h) {
	
	lowertext ((ptrbyte) *h, gethandlesize (h));
	} /*lowercasehandle*/
	
	
static boolean findopenfile (const tyfilespec *fs, hdlopenfile *hfile, hdlopenfile *hprev) {
	
	register hdlopenfile x = hfirstfile;
	
	*hfile = nil;
	
	*hprev = nil;
	
	while (true) {
		
		if (x == nil) /*fell off list*/
			return (false);
			
		if (equalfilespecs (fs, &(**x).fs)) {
			
			*hfile = x;
			
			return (true);
			}
		
		*hprev = x;
		
		x = (**x).nextfile;
		} /*while*/
	} /*findopenfile*/


static void fifopenfileerror (const tyfilespec *fs) {
	
	/*
	5.0.1 dmb: new, additional error reporting
	*/
	
	bigstring bsfile;

	getfsfile (fs, bsfile);

	lang2paramerror (filenotopenederror, bsfunctionname, bsfile);
	} /*fifopenfileerror*/


#if 0
static OSErr openforkperm (const tyfilespec *fs, boolean flresource, short perm, hdlfilenum *fnum) {
	
	OSErr errcode;
	
	if (flresource)
		errcode = FSpOpenRF (fs, perm, fnum);
	else
		errcode = FSpOpenDF (fs, perm, fnum);
	
	return (errcode);
	} /*openforkperm*/
#endif

static boolean fileopener (const tyfilespec *fs, hdlopenfile *hfile, hdlfilenum *fnum) {
	
	/*
	2/13/92 dmb: changed fldontclose logic to something that actually works; 
	we no longer ignore opWrErr errors.  instead, we try to open read/write, 
	but settle for read only.
	
	1/25/93 dmb: instead of using fldontclose boolean, we return the hdlopenfile 
	if found. the new filecloser uses this to restore things properly
	
	2.1b6 dmb: generate an oserror when a file can't be opened
	*/
	
	boolean fl;
	hdlopenfile hprev;
	
	if (findopenfile (fs, hfile, &hprev)) { /*file was already opened by us*/
		
		register hdlopenfile hf = *hfile;
		long position;
		
		filegetposition ((**hf).fnum, &position);
		
		(**hf).fpos = position;
		
		*fnum = (**hf).fnum;
		
		return (true);
		}
	
	*hfile = nil;
	
	disablelangerror ();

	fl = openfile (fs, fnum, false);
	
	enablelangerror();

	if (!fl)
		fl = openfile (fs, fnum, true);

#ifdef NEWFILESPECTYPE
	setoserrorparam ((ptrstring) (*fs).fullSpecifier);
#else
	setoserrorparam ((ptrstring) (*fs).name);
#endif
	
	return (fl);
	} /*fileopener*/


static boolean fileloadbuffer (hdlopenfile hf) {

	/*
	5.1.4 dmb: broke this out of fifread
	*/
	
	long ctbytes;
	
	if (!loadbufferfromfile ((**hf).fnum, (**hf).hbuffer, &ctbytes)) { /*non-EOF error*/
		
		(**hf).ctbytesleft = 0;
		
		return (false);
		}
	
	(**hf).ctbufferbytes = ctbytes;
	
	(**hf).ixbuffer = 0;
	
	return (true);
	} /*fileloadbuffer*/


static void filecloser (hdlopenfile hfile, hdlfilenum fnum) {
	
	if (fnum != 0) {
		
		if (hfile == nil) /*file wasn't already open*/
			closefile (fnum);
		
		else /*need to restor file position*/
			filesetposition (fnum, (**hfile).fpos);
		}
	} /*filecloser*/


boolean fiffindinfile (const tyfilespec *fs, bigstring pattern, long *index) {
	
	/*
	search for the indicated pattern in the file. return the index that the
	match was found at, -1 if not found.
	
	returns false if there was an error.
	
	2/2/93 dmb: fixed off-by-one error; last character in string wasn't checked.
	
	2.1b6 dmb: fixed infinite looping when partial matches were found
	*/
	
	hdlfilenum fnum = 0;
	short lenpattern;
	Handle hbuffer = nil;
	long ctbytes;
	long fpos = 0;
	long oldctbytes;
	boolean flreturned = false;
	hdlopenfile hfile = nil;
	
	*index = -1; /*indicate an error*/
	
	lenpattern = stringlength (pattern);
	
	if (lenpattern == 0) { /*no error, but it wasn't found*/
		
		flreturned = true;
		
		goto exit;
		}
		
	alllower (pattern); /*search is unicase*/
	
	if (!fileopener (fs, &hfile, &fnum))
		goto exit;
		
	if (!largefilebuffer (&hbuffer)) /*couldn't allocate minimum buffer*/
		goto exit;
		
	if (!loadbufferfromfile (fnum, hbuffer, &ctbytes))
		goto exit;
		
	if (ctbytes == 0) { /*it's an empty file*/
		
		flreturned = true;
		
		goto exit;
		}
	
	oldctbytes = ctbytes;
	
	while (true) {
		
		register char *pbuffer = *hbuffer;
		register char *pbufferend = pbuffer + ctbytes;
		register char chfirst = pattern [1];
		
		//lockhandle (hbuffer);
		
		lowercasehandle (hbuffer); /*search is unicase*/
		
		while (pbuffer < pbufferend) {
			
			if (*pbuffer++ == chfirst) { /*matched first character*/
				
				register short ixpattern;
				register char *p = pbuffer;
				
				for (ixpattern = 2; ixpattern <= lenpattern; ixpattern++) {
					
					if (p >= pbufferend) { /*ran out of chars in the buffer*/
						
						--ixpattern; /*make it the number of chars already matched*/
						
						backupfilepos (fnum, (long) ixpattern);
						
						fpos -= ixpattern;
						
						goto reloadbuffer;
						}
					
					if (*p++ != pattern [ixpattern]) 
						goto fastloop;
					
					} /*for*/
				
				*index = fpos + (pbuffer - *hbuffer - 1);
				
				flreturned = true;
				
				goto exit;
				} /*first character matched*/
			
			fastloop:
				;
			} /*for*/
		
		reloadbuffer:
		
		//unlockhandle (hbuffer);
		
		if (!loadbufferfromfile (fnum, hbuffer, &ctbytes))
			goto exit;
		
		if (ctbytes < lenpattern) { /*not enough chars to look at*/
			
			flreturned = false;
			
			goto exit;
			}
		
		fpos += oldctbytes; /*for computing returned value*/
		
		oldctbytes = ctbytes;
		} /*while*/
		
	exit:
	
	disposehandle (hbuffer);
	
	filecloser (hfile, fnum);
	
	return (flreturned);
	} /*fiffindinfile*/
	
	
boolean fifcomparefiles (const tyfilespec *fs1, const tyfilespec *fs2) {
	
	/*
	compare the two files byte-by-byte and return true if they are exactly
	equal. only compares the data forks of the files.
	*/
	
	hdlfilenum fnum1 = 0, fnum2 = 0;
	Handle hbuffer1 = nil, hbuffer2 = nil;
	long ctbytes1, ctbytes2;
	boolean flreturned = false;
	hdlopenfile hfile1, hfile2;
	
	if (!fileopener (fs1, &hfile1, &fnum1))
		goto exit;
	
	if (!fileopener (fs2, &hfile2, &fnum2))
		goto exit;
	
	if (!newclearhandle ((long) filebuffersize, &hbuffer1))
		goto exit;
	
	if (!newclearhandle ((long) filebuffersize, &hbuffer2))
		goto exit;
	
	while (true) {
		
		register long ix;
		register char *p1, *p2;
		
		if (!loadbufferfromfile (fnum1, hbuffer1, &ctbytes1))
			goto exit;
		
		if (!loadbufferfromfile (fnum2, hbuffer2, &ctbytes2))
			goto exit;
		
		if (ctbytes1 == 0) { /**no more chars to look at*/
			
			flreturned = ctbytes2 == 0; /*true if it's also empty*/
			
			goto exit;
			}
		
		if (ctbytes1 != ctbytes2) {
			
			flreturned = false;
			
			goto exit;
			}
		
		//lockhandle (hbuffer1);
		
		p1 = *hbuffer1; /*copy into register*/
		
		//lockhandle (hbuffer2);
		
		p2 = *hbuffer2; /*copy into register*/
		
		for (ix = 0; ix < ctbytes1; ix++) {
			
			if (*p1++ != *p2++) {
				
				flreturned = false;
				
				goto exit;
				}
			} /*for*/
			
		//unlockhandle (hbuffer1);
		
		//unlockhandle (hbuffer2);
		} /*while*/
		
	exit:
	
	disposehandle (hbuffer1);
		
	disposehandle (hbuffer2);
		
	filecloser (hfile1, fnum1);
	
	filecloser (hfile2, fnum2);
	
	return (flreturned);
	} /*fifcomparefiles*/
	

boolean fifcharcounter (const tyfilespec *fs, char chlookfor, long *count) {
	
	/*
	searches the indicated file for the character. count returns with the number 
	of matches. set chlookfor to (char) 13 to get the number of carriage returns
	or number of lines in the file.
	*/
	
	register long ctmatches = 0;
	hdlfilenum fnum = 0;
	Handle hbuffer = nil;
	long ctbytes;
	boolean flreturned = false;
	hdlopenfile hfile = nil;
	
	if (!fileopener (fs, &hfile, &fnum))
		goto exit;
		
	if (!largefilebuffer (&hbuffer)) /*couldn't allocate minimum buffer*/
		goto exit;
		
	if (!loadbufferfromfile (fnum, hbuffer, &ctbytes))
		goto exit;
		
	if (ctbytes == 0) { /*it's an empty file*/
		
		flreturned = true;
		
		goto exit;
		}
		
	while (true) {
		
		register long ct = ctbytes;
		register long ix;
		register char *pbuffer;
		register char ch = chlookfor;
		
		//lockhandle (hbuffer);
		
		pbuffer = *hbuffer; /*copy into register*/
		
		for (ix = 0; ix < ct; ix++) {
			
			if (*pbuffer++ == ch) 
				ctmatches++;
			} /*for*/
			
		//unlockhandle (hbuffer);
		
		if (!loadbufferfromfile (fnum, hbuffer, &ctbytes))
			goto exit;
			
		if (ctbytes == 0) { /*no more chars to look at*/
			
			flreturned = true;
			
			goto exit;
			}
		} /*while*/
		
	exit:
	
	*count = ctmatches;
	
	disposehandle (hbuffer);
		
	filecloser (hfile, fnum);
		
	return (flreturned);
	} /*fifcharcounter*/
	

boolean fifclosefile (const tyfilespec *fs) {
	
	/*
	close the file indicated by fs. deallocate the record and buffer. close
	it if it wasn't open when openfile was called.
	*/
	
	hdlopenfile hfile, hprev;
	register hdlopenfile hf;
	
	if (!findopenfile (fs, &hfile, &hprev)) { /*file isn't open*/
		
		fifopenfileerror (fs);

		return (false);
		}
	
	hf = hfile; /*copy into register*/
	
	closefile ((**hf).fnum);
	
	if (hprev == nil)
		hfirstfile = (**hf).nextfile;
	else
		(**hprev).nextfile = (**hf).nextfile;
		
	disposehandle ((**hf).hbuffer);
	
	disposehandle ((Handle) hf);
	
	return (true);
	} /*fifclosefile*/


boolean fifcloseallfiles (long refcon) {
	
	register hdlopenfile x = hfirstfile;
	register hdlopenfile hnext;
	tyfilespec fs;
	
	while (x != nil) {
		
		hnext = (**x).nextfile;
		
		if ((**x).refcon == refcon) {
			
			fs = (**x).fs;
			
			fifclosefile (&fs);
			}
		
		x = hnext;
		} /*while*/
	
	return (true);
	} /*fifcloseallfiles*/


boolean fifopenfile (const tyfilespec *fs, long refcon) {
	
	/*
	open the indicated file, allocating a record and linking it into the list.
	allocate a buffer. return true if the file could be opened.
	
	10/31/91 dmb: check for eof when file is first opened
	*/
	
	hdlfilenum fnum;
	hdlopenfile hfile = nil;
	hdlopenfile hprev;
	register hdlopenfile hf;
	Handle hbuffer;
	long eof;
	
	if (findopenfile (fs, &hfile, &hprev)) /*file was already opened by us*/
		fifclosefile (fs);
	
	if (!fileopener (fs, &hfile, &fnum))
		return (false);
	
	if (!newclearhandle (sizeof (tyopenfile), (Handle *) &hfile))
		goto error;
		
	if (!newclearhandle ((long) filebuffersize, &hbuffer))
		goto error;
		
	hf = hfile; /*copy into register*/
	
	if (!filegeteof (fnum, &eof))
		goto error;
	
	(**hf).ctbytesleft = eof;
	
	(**hf).fnum = fnum;
	
	(**hf).fs = *fs;
	
	(**hf).nextfile = hfirstfile; /*insert at the head of the file list*/
	
	hfirstfile = hf;
	
	(**hf).hbuffer = hbuffer;
	
	(**hf).ixbuffer = filebuffersize; /*force file read on first getline/char*/
	
	(**hf).ctbufferbytes = filebuffersize; //do getposition will work
	
	(**hf).refcon = refcon;
	
	return (true);
	
	error:
	
	disposehandle ((**hfile).hbuffer);
	
	disposehandle ((Handle) hfile);
	
	return (false);
	} /*fifopenfile*/
	
	
boolean fifendoffile (const tyfilespec *fs) {
	
	/*
	return true if there's no more info in the file to be read.
	*/
	
	hdlopenfile h1, h2;
	
	if (!findopenfile (fs, &h1, &h2))
		return (true);
	
	return ((**h1).ctbytesleft <= 0);
	} /*fifendoffile*/


static boolean fifread (const tyfilespec *fs, byte eolmarker, long ctmax, Handle *hdata) {
	
	/*
	read a chunk of data from the indicated file to a maximum of ctmax bytes.
	
	if eolmarker is non-zero, stop when a matching character is encountered.
	
	5.0.2b3 dmb: ignore actual value of eolmarker; it's either chnul or not. If not, 
	look for any kind of line ending.
	
	5.1.4 dmb: flstriplf handling gets smart about buffer boundaries.
	*/
	
	register Handle *x = hdata;
	hdlopenfile hfile, hprev;
	register hdlopenfile hf;
	register long ctread = ctmax;
	byte ch;
	boolean flstriplf = false;
	
	*x = nil;
	
	if (!findopenfile (fs, &hfile, &hprev)) { /*file isn't open*/
		
		fifopenfileerror (fs);

		return (false);
		}
	
	hf = hfile; /*copy into register*/
	
	while (true) {
		
		register long ixbuffer = (**hf).ixbuffer;
		register long ixend = (**hf).ctbufferbytes;
		
		if (ixbuffer >= ixend) { /*ran out of chars in buffer*/
			
			if (!fileloadbuffer (hf)) {
				
				disposehandle (*x);
				
				*x = nil;
				
				return (false);
				}
			
			ixbuffer = (**hf).ixbuffer;
			
			ixend = (**hf).ctbufferbytes;
			}
		
		/*
		first, see if first character is linefeed following previous cr
		*/
		
		/*
		first, extract what we want out of current buffer
		*/
		
		ctread -= ixend - ixbuffer; /*start by planning to read entire buffer*/
		
		if (ctread < 0) /*too many bytes; bring ixend in to match*/
			ixend += ctread;
		
		if (eolmarker != chnul) { /*see if eol is within range we're about to read*/
			
			register long ix;
			register byte *buf = (byte *) *(**hf).hbuffer;
			
			for (ix = ixbuffer; ix < ixend; ++ix) {
				
				ch = buf [ix];
				
				if (ch == chreturn || ch == chlinefeed) { /*found it*/
					
					if (ch == chreturn)
						flstriplf = true;
					
					ixend = ix; /*shorten range*/
					
					ctread = 0; /*don't read no mo'*/
					
					break;
					}
				}
			}
		
		bundle { /*move the data*/
			
			register Handle buf = (**hf).hbuffer;
			register long ct = ixend - ixbuffer;
			
			if (*x == nil) { /*first time thru loop*/
				
				long ix = ixbuffer;
				
				if (!loadfromhandletohandle (buf, &ix, ct, false, x))
					return (false);
				}
			else {
				
				long ctbytes = gethandlesize (*x);
				
				if (!sethandlesize (*x, ctbytes + ct)) {
					
					disposehandle (*x);
					
					*x = nil;
					
					return (false);
					}
				
				moveleft (*buf + ixbuffer, **x + ctbytes, ct);
				}
			
			(**hf).ctbytesleft -= ct;
			}
		
		(**hf).ixbuffer = ixend; /*where we left off*/
		
		if (ctread <= 0) { /*we're done!*/
			
			if (eolmarker != chnul) { /*make sure we skip past the marker*/
				
				++(**hf).ixbuffer;
				
				--(**hf).ctbytesleft;
				
				if (flstriplf) { // strip next character if it's a linefeed
				
					if (((**hf).ixbuffer < (**hf).ctbufferbytes) || fileloadbuffer (hf)) { // more chars
					
						if ((*(**hf).hbuffer) [(**hf).ixbuffer] == chlinefeed) {
							
							++(**hf).ixbuffer;
							
							--(**hf).ctbytesleft;
							}
						}
					}
				}
			
			return (true);
			}
		
		/*
		buffer is now depleted; try to move on to next
		*/
		
		if ((**hf).ctbytesleft <= 0) /*we've emptied last buffer*/
			return (true);
		} /*while*/
	} /*fifread*/



boolean fifreadline (const tyfilespec *fs, Handle *linestring) {
	
	/*
	read a line of text from the indicated file.
	*/
	
	return (fifread (fs, chreturn, longinfinity, linestring));
	} /*fifreadline*/


boolean fifreadhandle (const tyfilespec *fs, long ctbytes, Handle *x) {
	
	/*
	read a chunk of data from the indicated file.
	*/
	
	return (fifread (fs, chnul, ctbytes, x));
	} /*fifreadhandle*/


boolean fifwritehandle (const tyfilespec *fs, Handle x) {
	
	/*
	write some data at the end of the indicated file. it may be slow, but it's
	very easy to use! then again it might not be slow.
	*/
	
	boolean fl;
	hdlopenfile hfile;
	hdlfilenum fnum;
	long ctbytes = gethandlesize (x);
	long eofPos;
	
	if (ctbytes == 0) /*nothing to write*/
		return (true);
	
	if (!fileopener (fs, &hfile, &fnum))
		return (false);
	
	filegeteof (fnum, &eofPos);

	filesetposition (fnum, eofPos); /*position at the end of the file*/
	
	fl = filewrite (fnum, ctbytes, *x); /*this call does not move memory*/
	
	filecloser (hfile, fnum);
	
	return (fl);
	} /*fifwritehandle*/


boolean fifwriteline (const tyfilespec *fs, Handle linestring) {
	
	/*
	write a line at the end of the indicated file.
	*/
	
	#ifdef WIN95VERSION
		byte bseol [] = "\002\r\n";
	#endif
	#ifdef MACVERSION
		byte bseol [] = "\001\r";
	#endif
	
	if (!pushtexthandle (bseol, linestring))
		return (false);
	
	return (fifwritehandle (fs, linestring));
	} /*fifwriteline*/


boolean fifsetposition (const tyfilespec *fs, long pos) {
	
	/*
	5.0.1 dmb: new function. don't bother optimizing case where we're
	repositioning withing the current buffer. In most cases, we'll be
	called once, before any read operations.
	*/

	hdlopenfile hfile, hprev;
	register hdlopenfile hf;
	long eof;
	
	if (!findopenfile (fs, &hfile, &hprev)) { /*file isn't open*/
		
		fifopenfileerror (fs);

		return (false);
		}
	
	hf = hfile; /*copy into register*/
	
	if (!filesetposition ((**hf).fnum, pos))
		return (false);

	if (filegeteof ((**hf).fnum, &eof))
		(**hf).ctbytesleft = eof - pos;
	
	(**hf).ixbuffer = filebuffersize; /*force file read on first getline/char*/
	
	(**hf).ctbufferbytes = filebuffersize; //so getposition will work
	
	return (true);
	} /*fifsetposition*/


boolean fifgetposition (const tyfilespec *fs, long *pos) {
	
	/*
	5.0.1 dmb: new function
	
	5.1.5 dmb: account for unread buffer bytes
	*/

	hdlopenfile hfile, hprev;
	register hdlopenfile hf;
	
	if (!findopenfile (fs, &hfile, &hprev)) { /*file isn't open*/
		
		fifopenfileerror (fs);

		return (false);
		}
	
	hf = hfile; /*copy into register*/
	
	if (!filegetposition ((**hf).fnum, pos)) // next read will go from current file pos
		return (false);
	
	*pos -= (**hf).ctbufferbytes - (**hf).ixbuffer; // virtual pos is within already-read buffer
	
	return (true);
	} /*fifgetposition*/


boolean fifsetendoffile (const tyfilespec *fs, long eof) {
	
	/*
	set the eof for the file, opening & closing it if necessary
	
	5.0.1 dmb: reset ctbytesleft after changing the eof
	
	5.0.2 dmb: pin fpos after changing eof
	
	2003-05-26 AR: only update ctbytesleft and fpos if hfile is not nil.
	It can be nil if the file was not already open.
	*/
	
	boolean fl;
	hdlopenfile hfile;
	hdlfilenum fnum;
	
	if (!fileopener (fs, &hfile, &fnum))
		return (false);
	
	fl = fileseteof (fnum, eof);
	
	if (fl && hfile != nil) {
		
		(**hfile).ctbytesleft = max (0, eof - (**hfile).fpos);
		
		(**hfile).fpos = min ((**hfile).fpos, eof);
		}
	
	filecloser (hfile, fnum);
	
	return (fl);
	} /*fifseteof*/


boolean fifgetendoffile (const tyfilespec *fs, long *eof) {
	
	/*
	set the eof for the file, opening & closing it if necessary
	
	5.0.1 dmb: reset ctbytesleft after changing the eof
	*/
	
	boolean fl;
	hdlopenfile hfile;
	hdlfilenum fnum;
	
	if (!fileopener (fs, &hfile, &fnum))
		return (false);
	
	fl = filegeteof (fnum, eof);
	
	filecloser (hfile, fnum);
	
	return (fl);
	} /*fifseteof*/





