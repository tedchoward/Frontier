
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

#include "memory.h"
#include "font.h"
#include "quickdraw.h"
#include "strings.h"
#include "ops.h"
#include "op.h"
#include "opinternal.h"




typedef struct tylinetableitem {
	
	short flags;
	
	long lenrefcon; /*number of bytes that follow -- the refcon information*/
	} tylinetableitem, *ptrlinetable, **hdllinetable;

typedef enum tylinetableitemflags {

#ifdef MACVERSION
	flexpanded = 0x8000, /*was the line expanded when the outline was closed?*/
	flopenwindow = 0x4000, /*was the linked window open when its owner was closed?*/
	appbit0 = 0x2000, /*application-defined bit*/
	appbit1 = 0x1000, /*application-defined bit*/
	fllocked = 0x0800, /*is the line locked?*/
	flcomment = 0x0400, /*is this line a comment?*/
	flbreakpoint = 0x0200 /*is a breakpoint set on this line?*/
#endif

#ifdef WIN95VERSION
	flexpanded = 0x0080, /*was the line expanded when the outline was closed?*/
	flopenwindow = 0x0040, /*was the linked window open when its owner was closed?*/
	appbit0 = 0x0020, /*application-defined bit*/
	appbit1 = 0x0010, /*application-defined bit*/
	fllocked = 0x0008, /*is the line locked?*/
	flcomment = 0x0004, /*is this line a comment?*/
	flbreakpoint = 0x0002 /*is a breakpoint set on this line?*/
#endif
	} tylinetableitemflags;

#define macplatform 'mac '
#define winplatform 'win'

#ifdef MACVERSION
	#define thisplatform macplatform
	#define diskchcomment			((byte) 0xab)	/* '«' */
	#define diskchendcomment		((byte) 0xbb)
	#define diskchopencurlyquote	((byte) 0x93)
	#define diskchclosecurlyquote	((byte) 0x94)
	#define diskchtrademark			((byte) 0x99)
	#define diskchnotequals			((byte) 0xad)	/* '­' */
	#define diskchdivide			((byte) 0xf7)	/* '÷' */
#endif
#ifdef WIN95VERSION
	#define thisplatform winplatform
	#define diskchcomment			((byte) 0xc7)	/* 'Ç' */
	#define diskchendcomment		((byte) 0xc8)	/* 'È' */
	#define diskchopencurlyquote	((byte) 0xd2)	/* 'Ò' */
	#define diskchclosecurlyquote	((byte) 0xd3)	/* 'Ó' */
	#define diskchtrademark			((byte) 0xaa)	/* 'ª' */
	#define diskchnotequals			((byte) 0xad)	/* '­' */
	#define diskchdivide			((byte) 0xd6)	/* 'Ö' */
	#define diskchellipses			((byte) 0xc9)	/* 'É' */

	static byte bsellipses [] = "\x03...";
#endif

#define opversionnumber 2

#define hibyte(x) (x & 0xff00)
	
	
typedef struct tyversion2diskheader {
	
	short versionnumber; /*important, this structure is saved on disk*/
	
	long sizelinetable; /*number of bytes in the linetable section of handle*/
	
	long sizetext; /*number of bytes in the text portion of handle*/
	
	short lnumcursor; 
	
#ifdef MACVERSION
	tylinespacing linespacing;
#endif
#ifdef WIN95VERSION
	short linespacing;
#endif
	
	short lineindent;
	
	diskfontstring fontname; 
	
	short fontsize, fontstyle;
	
	short vertmin, vertmax, vertcurrent; /*for structs that don't get their own file*/
	
	short horizmin, horizmax, horizcurrent;
	
	unsigned long timecreated, timelastsave;
	
	unsigned long ctsaves;
	
	boolean fltextmode;
	
	diskrect windowrect; /*the size and position of the window that displays the outline*/
	
	long outlinesignature; /*client info*/
	
	RGBColor backcolor;
	
	RGBColor forecolor;
	
	OSType platform; /*Mac or Win*/
	
	short lnumcursor_hiword;
	
	short vertcurrent_hiword; //vert min, max aren't really used
	
	short horizcurrent_hiword; //horiz min, max aren't really used
	
	short waste [3]; /*room to grow*/
	} tyversion2diskheader;


typedef struct tyoppackinfo {

	handlestream *packstream;
	
	boolean flpackcomments;
	} tyoppackinfo, *ptroppackinfo;


static boolean outtablevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	4.1b7 dmb: we should never dispose of the packhandle.
	oppack takes care of that.
	
	5.1.3 dmb: use pushhandle to add refcon data; don't lock refcon while 
	enlarging packhandle
	*/

	register hdlheadrecord hn = hnode;
	register Handle hrefcon = (**hn).hrefcon;
	register long lenrefcon;
	ptroppackinfo packinfo = (ptroppackinfo) refcon;
	tylinetableitem item;
	
	clearbytes (&item, sizeof (item));
	
	item.flags |= flexpanded * (**hn).flexpanded;
	
	item.flags |= flopenwindow * (**hn).tmpbit;
	
	item.flags |= appbit0 * (**hn).appbit0;
	
	item.flags |= appbit1 * (**hn).appbit1;
	
	item.flags |= fllocked * (**hn).fllocked;
	
	item.flags |= flcomment * (**hn).flcomment;
	
	item.flags |= flbreakpoint * (**hn).flbreakpoint;
	
	lenrefcon = gethandlesize (hrefcon); /*0 if it's nil*/

	item.lenrefcon = conditionallongswap (lenrefcon);

	(**hnode).tmpbit = false;
	
	if (!writehandlestream ((*packinfo).packstream, &item, sizeof (item))) {
	
		/*4.1b7 dmb: our caller manages this - disposehandle (packhandle);*/
		
		return (false);
		}
	
	if (lenrefcon > 0) { /*something stored in the refcon field*/
		
		if (!writehandlestreamhandle ((*packinfo).packstream, hrefcon)) {
			
			/*4.1b7 dmb: our caller manages this - disposehandle (packhandle);*/
			
			return (false);
			}
		}
		
	return (true);
	} /*outtablevisit*/


static boolean opoutlinetotable (hdlheadrecord hnode, handlestream *packstream, long *ctbytes) {
	
	/*
	traverse the current outline, producing a table of information 
	with one element for each line in the outline.  append that table
	to the end of the indicated handle and return the number of bytes
	added to the handle.
	
	return false if there was a memory error.
	*/
	
	register long origsize;
	tyoppackinfo packinfo;
	
	*ctbytes = 0;
	
	origsize = (*packstream).eof;
	
	packinfo.packstream = packstream;
	
	if (!opsiblingvisiter (hnode, false, &outtablevisit, &packinfo))
		return (false);
		
	*ctbytes = (*packstream).eof - origsize;
	
	return (true);
	} /*opoutlinetotable*/


static boolean pushdiskchar (byte ch, handlestream *deststream) {

	/*
	insert the character at the end of a pascal string.
	map cross-platform disk characters to machine-specific chars

	5.0b11 dmb: we're now only called when mapping from the other
	platform the diskch constants are the other flatform's
	*/

	switch (ch) {
		case diskchcomment:
			ch = chcomment;
			break;
		
		case diskchendcomment:
			ch = chendcomment;
			break;
		
		case diskchopencurlyquote:
			ch = chopencurlyquote;
			break;

		case diskchclosecurlyquote:
			ch = chclosecurlyquote;
			break;
		
		case diskchtrademark:
			ch = chtrademark;
			break;

		case diskchdivide:
			ch = chdivide;
			break;
		
	#ifndef MACVERSION
		case diskchnotequals:
			if (!writehandlestreamchar (deststream, '!'))
				return (false);
			
			ch = '=';
			break;
		
		case diskchellipses:
			return (writehandlestreamstring (deststream, bsellipses));
	#endif

		default:
			break;
		}
	
	return (writehandlestreamchar (deststream, ch));
	} /*pushdiskchar*/


#if 0

static boolean xxxpushstringtodisk (bigstring bssource, bigstring bsdest) {

	/*
	insert the source string at the end of the destination string.
	map machine-specific disk characters to cross-platform chars
	*/
	
	short lensource = stringlength (bssource);
	short lendest = stringlength (bsdest);
	register byte *psource, *pdest;
	byte ch;
	
	if ((lensource + lendest) > lenbigstring) /*resulting string would be too long*/
		return (false);
		
	pdest = stringbaseaddress (bsdest) + lendest;
	
	psource = stringbaseaddress (bssource);
	
	setstringlength (bsdest, lendest + lensource);
	
	while (lensource--) {
		
		ch = *psource++;

		/*
		switch (ch) {
			case chcomment:
				ch = diskchcomment;
				break;
			
			case chendcomment:
				ch = diskchendcomment;
				break;
			
			case chopencurlyquote:
				ch = diskchopencurlyquote;
				break;

			case chclosecurlyquote:
				ch = diskchclosecurlyquote;
				break;
			
			case chtrademark:
				ch = diskchtrademark;
				break;

			case chnotequals:
				ch = diskchnotequals;
				break;
			
			case chdivide:
				ch = diskchdivide;
				break;
			
			default:
				break;
			}
		*/

		*pdest++ = ch;
		}
	
	return (true);
	} /*pushstringtodisk*/

#endif


static boolean outtextvisit (hdlheadrecord hnode, ptroppackinfo packinfo) {
	
	/*
	2.1b3 dmb: account for the fact that the leading tabs and trailing 
	return may overflow our 255-character limit
	
	2.1b5 dmb: added option to skip comment text
	
	4.1b7 dmb: we should never dispose of the packhandle.
	oppack takes care of that.

	5.0b11 dmb: don't map character when packing. can lose info, screw 
	clipboard
	*/
	
	register short level = (**hnode).headlevel;
	bigstring bs;
	
	filledstring (chtab, level, bs);
	
	if (!writehandlestreamstring ((*packinfo).packstream, bs)) //push the tabs by themself
		goto error;
	
	if ((*packinfo).flpackcomments || !opnestedincomment (hnode)) {
		
		if (!writehandlestreamhandle ((*packinfo).packstream, (**hnode).headstring)) //push the head text
			goto error;
		}
	
	if (!writehandlestreamchar ((*packinfo).packstream, chreturn)) //push the cr terminator
		goto error;

	return (true);
	
	error: {
		
		/*4.1b7 dmb: our caller manages this - disposehandle (packhandle);*/
		
		return (false);
		}
	} /*outtextvisit*/
	

static boolean opoutlinetotext (hdlheadrecord hnode, handlestream *textstream, long *ctbytes) {
	
	/*
	convert the outline into a block of tab-indented text, each
	line ended by a carriage return.  suitable for saving to disk
	because we convert the whole outline, all level-0 heads in
	the current outline.
	
	push the resulting text at the end of the indicated handle and
	return in ctbytes the number of bytes added to the handle.
	
	return false if there was a memory allocation error.
	*/
	
	register long origbytes;
	tyoppackinfo packinfo;
	
	origbytes = (*textstream).eof;
	
	*ctbytes = 0;
	
	packinfo.packstream = textstream;
	
	packinfo.flpackcomments = true;
	
	if (!opsiblingvisiter (hnode, false, (opvisitcallback) &outtextvisit, &packinfo))
		return (false);
	
	*ctbytes = (*textstream).eof - origbytes;
	
	return (true);
	} /*opoutlinetotext*/


boolean oppack (Handle *hpackedoutline) {
	
	/*
	create a packed, contiguous version of the current outline record.
	
	DW 3/25/90: if hpackedoutline comes in non-nil, we just append our
	stuff to the end of the handle, we don't allocate a new one.
	
	dmb 10/16/90: don't dispose of handle if we didn't allocate it
	
	dmb 2/8/91: flush edit buffer if in text mode
	
	dmb 3/1/91: deal with hoists
	
	4/9/93 dmb: resize the packedoutline handle really large and back again 
	before starting to avoid potentially many, many heap compactions while 
	expanding it for real
	
	4.1b7 dmb: fixed double dispose bug. if our caller allocated the handle
	we do not dispose it on error, as was originally intended.

	5.0b11 dmb: added platform logic. don't map characters when packing
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hsummit;
	register Handle h;
	register long ixheader;
	handlestream packstream;
	tyversion2diskheader header;
	boolean flallocated = false;
	boolean flpoppedhoists = false;
	boolean flerror = false;
	long lnumcursor;
	
	h = *hpackedoutline; /*copy into register*/
	
	clearbytes (&header, sizeof (header)); /*assure all bits set to 0*/
	
	if (h == nil) { /*the normal case, allocate a new handle*/
		
		if (!newgrowinghandle (5 * 1024, hpackedoutline)) //start with non-empty handle
			return (false);
		
		h = *hpackedoutline;
		
		flallocated = true;
		
		openhandlestream (h, &packstream);
		
		packstream.pos = packstream.eof = sizeof (header);
		}
	else {
		
		openhandlestream (h, &packstream);
		
		packstream.pos = packstream.eof;
		
		if (!writehandlestream (&packstream, &header, sizeof (header)))
			return (false);
		}
	
	ixheader = packstream.pos - sizeof (header); //we're pointing past header now
	
	flpoppedhoists = oppopallhoists ();
	
	header.versionnumber = conditionalshortswap(opversionnumber);
	
	header.platform = conditionallongswap (thisplatform);

	opgetnodeline ((**ho).hbarcursor, &lnumcursor);
	
	memlongtodiskwords (lnumcursor, header.lnumcursor, header.lnumcursor_hiword);
	
	header.linespacing = conditionalenumswap((**ho).linespacing);
	
	header.lineindent = conditionalshortswap((**ho).lineindent);
	
	header.fltextmode = (**ho).fltextmode;
	
	header.vertmin = conditionalshortswap((**ho).vertscrollinfo.min);
	
	header.vertmax = conditionalshortswap((**ho).vertscrollinfo.max);
	
	memlongtodiskwords ((**ho).vertscrollinfo.cur, header.vertcurrent, header.vertcurrent_hiword);
	
	header.horizmin = conditionalshortswap((**ho).horizscrollinfo.min);
	
	header.horizmax = conditionalshortswap((**ho).horizscrollinfo.max);
	
	memlongtodiskwords ((**ho).horizscrollinfo.cur, header.horizcurrent, header.horizcurrent_hiword);
	
	header.timecreated = conditionallongswap((**ho).timecreated);
	
	header.timelastsave = conditionallongswap((**ho).timelastsave);
	
	/*timestamp (&header.timelastsave);*/ /*dmb 4.1b13: don't stamp it; opdirty sets it as true mode date*/
	
	header.ctsaves = conditionallongswap(++(**ho).ctsaves);
	
	header.forecolor = (**ho).forecolor;
	
	memtodiskshort (header.forecolor.red);
	memtodiskshort (header.forecolor.green);
	memtodiskshort (header.forecolor.blue);
	
	header.backcolor = (**ho).backcolor;
	
	memtodiskshort (header.backcolor.red);
	memtodiskshort (header.backcolor.green);
	memtodiskshort (header.backcolor.blue);
	
	diskgetfontname ((**ho).fontnum, header.fontname);
	
	header.fontsize = conditionalshortswap((**ho).fontsize);
	
	header.fontstyle = conditionalshortswap((**ho).fontstyle);
	
	recttodiskrect (&(**ho).windowrect, &header.windowrect);
	
	header.outlinesignature = conditionallongswap((**ho).outlinesignature);
	
	hsummit = (**ho).hsummit; /*copy into register*/
	
	opwriteeditbuffer (); /*if a headline is being edited, update text handle*/
	
	if (!opoutlinetotext (hsummit, &packstream, &header.sizetext)) {
		
		flerror = true;
		
		goto exit;
		}
		
	if (!opoutlinetotable (hsummit, &packstream, &header.sizelinetable)) {
	
		flerror = true;
		
		goto exit;
		}
	
	memtodisklong (header.sizetext);
	memtodisklong (header.sizelinetable);
	
	/*move the header into handle*/ {
		
		register ptrbyte p;
		
		p = *h;
		
		p += ixheader;
		
		moveleft (&header, p, sizeof (header));
		}
	
	exit:
	
	if (flpoppedhoists)
		oprestorehoists ();
	
	closehandlestream (&packstream);
	
	if (flerror) {
		
		if (flallocated)
			disposehandle (h);
		
		return (false);
		}
	
	return (true);
	} /*oppack*/


boolean oppackoutline (hdloutlinerecord houtline, Handle *hpackedoutline) {
	
	boolean fl;
	
	oppushoutline (houtline);
	
	fl = oppack (hpackedoutline);
	
	oppopoutline ();
	
	return (fl);
	} /*oppackoutline*/


static boolean intablevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	5.1.5b9 dmb: validate expanded bit in case structure changes with overflow
	*/
	
	register hdlheadrecord hn = hnode;
	ptroppackinfo packinfo = (ptroppackinfo) refcon;
	register long lenrefcon;
	Handle hrefcon;
	tylinetableitem item;
	hdlheadrecord hup, hleft;
	
	if (athandlestreameof ((*packinfo).packstream)) // ran out of line items
		return (true);
	
	readhandlestream ((*packinfo).packstream, &item, sizeof (item));
	
	hup = (**hn).headlinkup;
	
	if (hup == hn) { //first in list, can set expandedness
	
		hleft = (**hn).headlinkleft;
		
		if ((hleft != hn) && !(**hleft).flexpanded)
			(**hn).flexpanded = false;
		else
			(**hn).flexpanded = (item.flags & flexpanded) != 0;
		}
	else
		(**hn).flexpanded = (**hup).flexpanded;
	
	(**hn).fllocked = (item.flags & fllocked) != 0;
	
	(**hn).flcomment = (item.flags & flcomment) != 0;
	
	(**hn).flbreakpoint = (item.flags & flbreakpoint) != 0;
	
	(**hn).tmpbit = (item.flags & flopenwindow) != 0;
	
	(**hn).appbit0 = (item.flags & appbit0) != 0;
	
	(**hn).appbit1 = (item.flags & appbit1) != 0;
	
	lenrefcon = conditionallongswap(item.lenrefcon); /*copy into register*/
	
	if (lenrefcon > 0) { /*link in a refcon node*/
	
		if (!newclearhandle (lenrefcon, &hrefcon))
			return (false);
		
		readhandlestream ((*packinfo).packstream, *hrefcon, lenrefcon);

		(**hn).hrefcon = hrefcon;
		}
	
	return (true);
	} /*intablevisit*/


static boolean optabletooutline (handlestream *packstream, hdlheadrecord hsummit) { 

	/*
	apply values in table to hsummit outline
	*/
	
	tyoppackinfo packinfo;
	
	packinfo.packstream = packstream;
	
	return (opsiblingvisiter (hsummit, false, &intablevisit, &packinfo));
	} /*optabletooutline*/
	

static short spacesforlevel;

static short firstindent;

static boolean fltabindent;

static void clearindentvalues (boolean flmusttabindent) {
	
	spacesforlevel = 0;
	
	firstindent = -1;
	
	fltabindent = flmusttabindent;
	} /*clearindentvalues*/


static boolean opgetlinetext (boolean flmapchars, handlestream *textstream, short *level, Handle *htext) {

	/*
	extract a line of text from the handle at offset ix.  ix gets bumped to
	point at the beginning of the next line.
	
	the level is the number of ascii 9's at the head of the string.
	
	12/13/91 dmb: added support for space-indented text import
	
	3/11/92 dmb: fixed off-by-one error when filling lenbigstring w/out hitting a cr.
	
	2.1b8 dmb: ignore null characters
	
	5.0a2 dmb: when overflowing a headline, try to break after a word
	
	5.0a4 dmb: added *level+1 limit to constrain to correct structure, avoid hang
			   skip linefeeds after returns

	5.0b11 dmb: map characters when they're from the other platform
	*/
	
	handlestream s;
	long ct = 0;
	register ptrbyte p;
	boolean fl;
	
	if (athandlestreameof (textstream))
		return (false);
	
	if (!newhandle (lenbigstring, htext))
		return (false);
	
	openhandlestream (*htext, &s);
	
	s.eof = 0; //nothing has actually been written to the 255-byte buffer
	
	ct = skiphandlestreamchars (textstream, chtab);
	
	if (ct > 0)
		fltabindent = true;
	
	else {
	
		ct = skiphandlestreamchars (textstream, chspace);
		
		if (ct > 0) {
			
			if (spacesforlevel == 0) /*first run of spaces*/
				spacesforlevel = (short) ct;
			
			ct = divround (ct, spacesforlevel);
			}
		}
	
/*
	while (*p == chtab) { /%count the leading tab chars

		ct++;
		
		p++;
		
		ixtext++;
		
		if (ixtext >= sizetext) /%no text on the line
			return (false);
		
		fltabindent = true;
		}

	if (!fltabindent) {
		
		while (*p == chspace) { /%count the leading tab chars
			
			ct++;
			
			p++;
			
			ixtext++;
			
			if (ixtext >= sizetext) /%no text on the line
				return (false);
			}
		
		if (ct > 0) {
			
			if (spacesforlevel == 0) /%first run of spaces
				spacesforlevel = (short) ct;
			
			ct = divround (ct, spacesforlevel);
			}
		}
*/

	if (firstindent < 0) { /*first line of text*/
	
		firstindent = (short) ct;
		
		*level = 0;
		}
	else {
		*level = min (*level + 1, max (0, ct - firstindent));
		}
	
	while ((*textstream).pos < (*textstream).eof) { /*copy the text chars into the string*/
		
		p = *(*textstream).data + (*textstream).pos;
		
		if (*p == chreturn) {
			
			(*textstream).pos++; /*skip over the return; we'll start a new headline*/
			
			if (*++p == chlinefeed)
				(*textstream).pos++;
			
			break;
			}
		
		if (*p != chnul) { /*ignore null characters*/
			
			if (flmapchars)
				fl = pushdiskchar (*p, &s);
			else
				fl = writehandlestreamchar (&s, *p);
			
			if (!fl) {
				
				disposehandle (*htext);
				
				return (false);
				}
			}
		
		(*textstream).pos++;
		} /*while*/
	
	closehandlestream (&s);
	
	return (true);
	} /*opgetlinetext*/


static boolean opunpacktexttooutline (long platform, handlestream *packstream, hdlheadrecord *hnode) {
	
	/*
	2/19/91 dmb: call opstart/endinternalchange around this routine to 
	avoid building bogus undo.
	
	8/22/91 dmb: no longer attempt to attach structure to current outline.  used to 
	cause crash if an error occurred; cleaner to leave it to caller.
	*/
	
	register hdlheadrecord h = nil;
	register short lastlevel = 0;
	register tydirection dir;
	Handle hlinetext;
	hdlheadrecord hnewnode;
	short level;
	boolean fl = false; /*guilty until proven innocent*/
	
	opstartinternalchange ();
	
	clearindentvalues (true);
	
	while (true) {
		
		if (!opgetlinetext (platform != thisplatform, packstream, &level, &hlinetext)) { /*consumed the text*/
			
			if (h != nil) /*success -- we got something*/
				fl = true;
			
			break;
			}
		
		if (h == nil) { /*first line*/
			
			if (!opnewstructure (hlinetext, hnode))
				break;
			
			h = *hnode;
			
			lastlevel = 0;
			}
		else {
			if (level > lastlevel) 
				dir = right;
		
			else {
				h = oprepeatedbump (left, lastlevel - level, h, true);
		
				dir = down;
				}
			
			if (!opdepositnewheadline (h, dir, hlinetext, &hnewnode)) {
				
				opdisposestructure (*hnode, false);
				
				break;
				}
			
			h = hnewnode;
			
			lastlevel = level;
			}
		} /*while*/
	
	opendinternalchange ();
	
	return (fl);
	} /*opunpacktexttooutline*/

	
static boolean opunpackversion2 (handlestream *packstream) {
	
	/*
	12/28/90 dmb: set hline1 according to vertcurrent -- used to be left at summit
	
	2/9/93 dmb: push/popstyle around texttooutline so that recalc isn't needed
	
	2.1b4 dmb: pushscratchport before measuring text
	
	3.0.4b8 dmb: use of scratchport is now more thorough

	5.0b11 dmb: added platform logic. map character if platform changes
	*/
	
	register hdloutlinerecord ho;
	register long ixstart;
	handlestream stream;
	hdlheadrecord hsummit, hline1, hcursor;
	tyversion2diskheader header;
	short fontnum;
	long lnumcursor;
	boolean fl;
	
	ho = outlinedata; /*copy into register*/
	
	if (!readhandlestream (packstream, &header, sizeof (header)))
		return (false);
	
	(**ho).linespacing = conditionalenumswap (header.linespacing);
	
	(**ho).lineindent = conditionalshortswap (header.lineindent);
	
	(**ho).fltextmode = (boolean) conditionalshortswap (header.fltextmode);
	
	(**ho).vertscrollinfo.min = conditionalshortswap (header.vertmin);
	
	(**ho).vertscrollinfo.max = conditionalshortswap (header.vertmax);
	
	(**ho).vertscrollinfo.cur = diskwordstomemlong (header.vertcurrent, header.vertcurrent_hiword);
	
	(**ho).horizscrollinfo.min = conditionalshortswap (header.horizmin);
	
	(**ho).horizscrollinfo.max = conditionalshortswap (header.horizmax);
	
	(**ho).horizscrollinfo.cur = diskwordstomemlong (header.horizcurrent, header.horizcurrent_hiword);
	
	(**ho).timecreated = conditionallongswap (header.timecreated);
	
	(**ho).timelastsave = conditionallongswap (header.timelastsave);
	
	(**ho).ctsaves = conditionallongswap (header.ctsaves);
	
	disktomemshort (header.backcolor.red);
	disktomemshort (header.backcolor.green);
	disktomemshort (header.backcolor.blue);
	
	if (memcmp (&header.backcolor, &blackcolor, sizeof (RGBColor)) == 0)
		header.backcolor = whitecolor;
	
	// (**ho).backcolor = header.backcolor;
	
	diskgetfontnum (header.fontname, &fontnum);
	
	if (isemptystring (header.fontname)) //it was munged in beta
		(**ho).fontnum = config.defaultfont;
	else
		(**ho).fontnum = fontnum;
	
	if (header.fontsize == 0) //it was munged in beta
		(**ho).fontsize = config.defaultsize;
	else
		(**ho).fontsize = conditionalshortswap (header.fontsize);
	
	(**ho).fontstyle = conditionalshortswap (header.fontstyle);
	
	diskrecttorect (&header.windowrect, &(**ho).windowrect);
	
	disktomemlong (header.platform);
	
	disktomemlong (header.sizetext);
	
	disktomemlong (header.sizelinetable);
	
	if (header.platform == 0)
		header.platform = macplatform;
	
	if (header.outlinesignature == 0)
		header.outlinesignature = conditionallongswap ('LAND');

	(**ho).outlinesignature = conditionallongswap (header.outlinesignature);
	
	ixstart = (*packstream).pos;
	
	pushscratchport ();
	
	pushstyle ((**ho).fontnum, (**ho).fontsize, (**ho).fontstyle);
	
	stream = *packstream;
	
	stream.eof = stream.pos + header.sizetext;
	
	fl = opunpacktexttooutline (header.platform, &stream, &hsummit);
	
	popstyle ();
	
	popport ();
	
	if (!fl)
		return (false);
	
	(*packstream).pos += header.sizetext;
	
	stream = *packstream;
	
	stream.eof = stream.pos + header.sizelinetable;
	
	opsetsummit (ho, hsummit);
	
	opsetexpandedbits (hsummit, true); /*all 1st level items are expanded*/
	
	if (!optabletooutline (&stream, hsummit))
		return (false);
	
	(*packstream).pos += header.sizelinetable;
	
	hline1 = oprepeatedbump (flatdown, (**ho).vertscrollinfo.cur, hsummit, true);
	
	(**ho).hline1 = hline1;
	
	lnumcursor = diskwordstomemlong (header.lnumcursor, header.lnumcursor_hiword);

	hcursor = oprepeatedbump (flatdown, lnumcursor, hsummit, true);
	
	(**ho).hbarcursor = hcursor;
	
	opsetctexpanded (ho); /*don't bother saving this on disk, we re-compute*/
	
	return (true);
	} /*opunpackversion2*/
	
	
boolean opunpack (Handle hpackedoutline, long *ixload, hdloutlinerecord *houtline) {
	
	/*
	9/25/91 dmb: added call to testheapspace to try to improve low-mem handling
	
	9/15/92 dmb: removed support for version1 format; it never shipped
	
	12/17/96 dmb: tolerate version numbers new than now, unless high byte changes
	*/
	
	handlestream packstream;
	hdloutlinerecord ho;
	short versionnumber;
	boolean fl;
	
	*houtline = nil;
	
	if (!newoutlinerecord (&ho))
		return (false);
	
	openhandlestream (hpackedoutline, &packstream);
	
	packstream.pos = *ixload;
	
	if (!readhandlestream (&packstream, &versionnumber, sizeof (versionnumber))) {
	
		shellerrormessage ("\x3d" "Can't unpack outline because unexpected data was encountered.");
		
		opdisposeoutline (ho, false);
		
		return (false);
		}
	
	packstream.pos = *ixload;
	
	disktomemshort (versionnumber);
	
	oppushoutline (ho);
	
	switch (versionnumber) {
		
		case 2:
		case 3:
			fl = opunpackversion2 (&packstream);
			
			break;
		
		default:
			if ((hibyte (versionnumber) != hibyte (opversionnumber)) || versionnumber < 2) {
			
				shellinternalerror (idbadopversionnumber, STR_bad_outline_version_number);
				
				fl = false;
				}
			else
				fl = opunpackversion2 (&packstream);
			
			break;
		
		} /*switch*/
	
	oppopoutline ();
	
	if (fl) { /*so far so good -- let's make sure we're not leaving memory dangerously low*/
		
		fl = testheapspace (0x800); /*2K gives us some room to play with*/
		}
	
	*ixload = packstream.pos;
	
	closehandlestream (&packstream);
	
	if (!fl) {
		
		opdisposeoutline (ho, false);
		
		return (false);
		}
	
	*houtline = ho;
	
	return (true);
	} /*opunpack*/


boolean opunpackoutline (Handle hpackedoutline, hdloutlinerecord *houtline) {
	
	long ixload = 0;
	
	return (opunpack (hpackedoutline, &ixload, houtline));
	} /*opunpackoutline*/


boolean optextscraptooutline (hdloutlinerecord houtline, Handle htext, hdlheadrecord *hnode) {
	
	/*
	there seems to be an opportunity to factor code here.
	
	2.1b8 dmb: added houtline parameter to satisfy texttooutlinecallback 
	conventions. currently ignored here.
	
	5.0a4 dmb: added logic so empty lines don't dictate structure.
	*/
	
	register hdlheadrecord hlast = nil;
	register short lastlevel = 0;
	tydirection dir;
	hdlheadrecord htree = nil;
	hdlheadrecord hnewnode;
	short level;
	Handle hlinetext;
	handlestream textstream;
	boolean fl = false; /*guilty until proven innocent*/
	
	opstartinternalchange ();
	
	openhandlestream (htext, &textstream);
	
	clearindentvalues (false);
	
	while (true) {
		
		if (!opgetlinetext (false, &textstream, &level, &hlinetext)) { /*no more text*/
			
			fl = true; /*success*/
			
			break;
			}
			
		if (hlast == nil) { /*first line*/
			
			if (!opnewstructure (hlinetext, &htree))
				break;
			
			hlast = htree;
			
			lastlevel = 0;
			
			continue; /*finished with this one*/
			}
		
		if (gethandlesize (hlinetext) == 0) // 5.0a4 dmb: empty lines don't dictate structure
			level = lastlevel;
		
		if (level > lastlevel) {
			
			dir = right;
			}
		else { /*surface 0 or more times*/
		
			hlast = oprepeatedbump (left, lastlevel - level, hlast, true);
	
			dir = down;
			}
		
		if (!opdepositnewheadline (hlast, dir, hlinetext, &hnewnode))
			break;
		
		hlast = hnewnode;
		
		lastlevel = level;
		} /*while*/
	
	closehandlestream (&textstream);
	
	opendinternalchange ();
	
	if (!fl) { /*didn't succeed -- toss accumulated structure and return false*/
		
		if (htree != nil)
			opdisposestructure (htree, false);
		
		return (false);
		}
	
	*hnode = htree;
	
	return (htree != nil);
	} /*optextscraptooutline*/


static short outscraplevel = 0; /*for communications while sending to scrap*/


static boolean outscrapvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	6/12/91 dmb: validate outlinedata.  currently, menubar scraps will only 
	export properly while a menubar outline is active.  the problem is that 
	the scrap is a standalone headline; it needs to have an outline record too.
	*/
	
	register hdlheadrecord h = hnode;
	ptroppackinfo packinfo = (ptroppackinfo) refcon;
	handlestream *s;
	
	if (!outtextvisit (h, packinfo))
		return (false);
	
	if ((**h).hrefcon != nil) {
		
		assert (outlinedata != nil);
		
		if (outlinedata == nil)
			return (false);
		
		s = (*packinfo).packstream;
		
		closehandlestream (s);
		
		if (!(*(**outlinedata).textualizerefconcallback) (h, (*s).data))
			return (false);
		
		openhandlestream ((*s).data, s);
		
		(*s).pos = (*s).eof;
		}
	
	return (true);
	} /*outscrapvisit*/


boolean opoutlinetotextstream (hdloutlinerecord houtline, boolean flomitcomments, handlestream *s) {
	
	/*
	convert the outline into a block of tab-indented text, each
	line ended by a carriage return.  suitable for passing through 
	the clipboard to a text-based application.
	*/
	
	tyoppackinfo packinfo;
	boolean fl;
	
	outscraplevel = 0;
	
	packinfo.packstream = s;
	
	packinfo.flpackcomments = !flomitcomments;
	
	oppushoutline (houtline);
	
	opwriteeditbuffer (); /*if a headline is being edited, update text handle*/
	
	fl = opsiblingvisiter ((**houtline).hsummit, false, &outscrapvisit, &packinfo);
	
	oppopoutline ();
	
	return (fl);
	} /*opoutlinetotextstream*/


boolean opoutlinetotextscrap (hdloutlinerecord houtline, boolean flomitcomments, Handle htext) {
	
	/*
	stick the handle into a handlestream and pass it on through...
	*/
	
	handlestream packstream;
	boolean fl;
	
	openhandlestream (htext, &packstream);
	
	fl = opoutlinetotextstream (houtline, flomitcomments, &packstream);
	
	closehandlestream (&packstream);
	
	return (fl);
	} /*opoutlinetotextscrap*/


boolean opoutlinetonewtextscrap (hdloutlinerecord houtline, Handle *htext) {
	
	/*
	similar to opoutlinetotextscrap above, but allocated new handle
	*/
	
	if (!newgrowinghandle (0, htext))
		return (false);
	
	if (opoutlinetotextscrap (houtline, false, *htext))
		return (true);
	
	disposehandle (*htext);
	
	*htext = nil;
	
	return (false);
	} /*opoutlinetonewtextscrap*/


/*
boolean opsuboutlinetonewtextscrap (hdlheadrecord hnode, Handle *htext) {
	
	/%
	similar to opoutlinetotextscrap above, but allocated new handle and 
	only visits hnode and its subheads
	%/
	
	register hdlheadrecord h = hnode;
	
	if (!newemptyhandle (htext))
		return (false);
	
	outscraplevel = 0;
	
	packhandle = *htext;
	
	if (outscrapvisit (h) && oprecursivelyvisit (h, infinity, &outscrapvisit))
		return  (true);
	
	disposehandle (*htext);
	
	return (false);
	} /%opsuboutlinetonewtextscrap%/
*/


