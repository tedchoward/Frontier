
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iac.h>
#include "outlinesharing.h"




hdloutlinerecord outlinedata; /*global, points to "current" structure*/


#define diskfontnamelength 32 /*number of bytes for a font name stored on disk*/

typedef char diskfontstring [diskfontnamelength + 1];


#define defaultfont geneva
#define defaultsize 12
#define defaultstyle 0

#define infinity 32767

#define longsizeof(x) (long)sizeof(x)

#define max(x,y) ((x) > (y)? (x) : (y))

#define min(x,y) ((x) < (y)? (x) : (y))



typedef struct tydiskoutlineheader { /*header for packed outline*/
	
	short versionnumber; /*Frontier 1.0 sets this to 2*/
	
	long sizelinetable; /*number of bytes in the linetable section of handle*/
	
	long sizetext; /*number of bytes in the text portion of handle*/
	
	short lnumcursor; /*the line number the outline cursor is on*/
	
	opLineSpacing linespacing; /*indicates how many pixels to skip between lines*/
	
	short lineindent; /*how much to indent for each level*/
	
	diskfontstring fontname; /*the font used to display this outline, can be saved on disk*/
	
	short fontsize, fontstyle;
	
	short vertmin, vertmax, vertcurrent; /*scrollbar info last time this outline was in a window*/
	
	short horizmin, horizmax, horizcurrent;
	
	long timecreated, timelastsave; /*time the outline was created/last saved*/
	
	long ctsaves; /*the number of times this outline has been packed*/
	
	Boolean fltextmode: 1; /*last time it was edited was it in structure mode or edit mode?*/
	
	Rect windowrect; /*the size and position of window that last displayed outline*/
	
	char waste [32]; /*room to grow*/
	} tydiskoutlineheader;


typedef struct tylinetableitem {
	
	Boolean flexpanded: 1; /*was the line expanded when the outline was closed?*/
	
	Boolean flopenwindow: 1; /*was the linked window open when its owner was closed?*/
	
	Boolean reserved1bit: 1; /*reserved for future versions of Frontier*/
	
	Boolean reserved2bit: 1; /*reserved for future versions of Frontier*/
	
	Boolean reserved3bit: 1; /*reserved for future versions of Frontier*/
	
	Boolean flcomment: 1; /*is this line a comment?*/
	
	Boolean flbreakpoint: 1; /*is a breakpoint set on this line?*/
	
	long lenrefcon; /*number of bytes that follow -- the refcon information*/
	} tylinetableitem, *ptrlinetable, **hdllinetable;
	

static Handle packhandle; /*global needed for packing and unpacking*/

static long ixpackhandle; /*index into packhandle*/




#define ctoutlinestack 5 /*we can remember outline contexts up to 5 levels deep*/

short topoutlinestack = 0;

hdloutlinerecord outlinestack [ctoutlinestack];





static void opFillChar (void *pfill, long ctfill, char chfill) {
	
	/*
	do a mass memory fill -- copy ctfill chfills at pfill.
	*/
	
	char *p = pfill;
	long ct = ctfill;
	char ch = chfill;
	
	while (ct--) *p++ = (char) ch; /*tight loop*/
	} /*opFillChar*/
	

static void opClearBytes (void *pclear, long ctclear) {
	
	/*
	fill memory with 0's.
	*/
	
	opFillChar (pclear, ctclear, (char) 0);
	} /*opClearBytes*/
	

static Boolean opNewClearHandle (long ctbytes, Handle *hreturned) {
	
	long ct = ctbytes;
	Handle h;
	
	*hreturned = h = NewHandle (ct);
	
	if (h == nil) 
		return (false);
		
	opClearBytes (*h, ct);
	
	*hreturned = h;
	
	return (true);
	} /*opNewClearHandle*/
	
	
static Boolean opEqualStrings (Str255 bs1, Str255 bs2) {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal.  return false otherwise.
	*/

	char *p1 = (char *) bs1, *p2 = (char *) bs2;
	char ct = *p1 + 1;
	
	while (ct--) 
		
		if (*p1++ != *p2++)
		
			return (false);
		
	return (true); /*loop terminated*/
	} /*opEqualStrings*/
	
	
static void opCopyString (void *source, void *dest) {

	/*
	create a copy of source in dest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	char *s = source, *d = dest;
	unsigned short i, len;
	
	len = (unsigned char) s [0];
	
	for (i = 0; i <= len; i++) 
		*d++ = *s++;
	} /*opCopyString*/


static void opCopyHeapString (StringHandle hstring, Str255 bs) {
	
	if (hstring == nil) {
		
		bs [0] = 0;
		
		return;
		}
	
	HLock ((Handle) hstring);
	
	opCopyString (*hstring, bs);
	
	HUnlock ((Handle) hstring);
	} /*opCopyHeapString*/
	

static void opMoveLeft (void *psource, void *pdest, long length) {
	
	/*
	do a mass memory move with the left edge leading.  good for closing
	up a gap in a buffer, among other thingsÉ
	*/
	
	char *ps, *pd;
	long ctloops;
	
	ctloops = length;
	
	if (ctloops > 0) {
	
		ps = psource; /*copy into a register*/
	
		pd = pdest; /*copy into a register*/
	
		while (ctloops--) *pd++ = *ps++;
		}
	} /*opMoveLeft*/
	
	
static Boolean opNewFilledHandle (void *pdata, long size, Handle *hreturned) {
	
	Handle h;
	long ctbytes;
	
	ctbytes = size; 
	
	h = NewHandle (ctbytes);
	
	if (h == nil) {
		
		*hreturned = nil;
		
		return (false);
		}
	
	opMoveLeft (pdata, *h, ctbytes);
		
	*hreturned = h;
	
	return (true);
	} /*opNewFilledHandle*/
	
	
static Boolean opEnlargeHandle (Handle hgrow, long ctgrow, char *newdata) {
	
	/*
	make the handle big enough to hold the new data, and move the new data in
	at the end of the newly enlarged handle.
	*/
	
	Handle h = hgrow;
	long ct = ctgrow;
	char *p = newdata;
	long origsize;
	
	origsize = GetHandleSize (h);
	
	SetHandleSize (h, origsize + ct);
		
	if (MemError () != noErr)
		return (false);
		
	opMoveLeft (p, *h + origsize, ct);
	
	return (true);
	} /*opEnlargeHandle*/ 
	
	
static Boolean opLoadFromHandle (Handle hload, long *ixload, long ctload, char *pdata) {
	
	/*
	copy the next ctload bytes from hload into pdata and increment the index.
	
	return false if there aren't enough bytes.
	
	start ixload at 0.
	*/
	
	Handle h = hload;
	char *p = pdata;
	long ct = ctload;
	long ix = *ixload;
	long size;
	
	size = GetHandleSize (h);
	
	if ((ix + ct) > size) /*asked for more bytes than there are*/
		return (false); 
		
	opMoveLeft (*h + ix, p, ct); /*copy out of the handle*/
	
	*ixload = ix + ct; /*increment the index into the handle*/
	
	return (true);
	} /*opLoadFromHandle*/
	

static Boolean opPushTextHandle (Str255 bs, Handle htext) {
	
	/*
	htext is a handle created with newtexthandle.
	
	increase the size of the handle so we can push the text of bs at 
	the end of the handle (not including length byte).
	*/
	
	return (opEnlargeHandle (htext, (unsigned char) bs [0], (char *) bs + 1));
	} /*opPushTextHandle*/


static Boolean opNewHeapString (Str255 bs, StringHandle *hstring) {

	return (opNewFilledHandle (bs, ((unsigned char) bs [0]) + 1, (Handle *) hstring));
	} /*opNewHeapString*/
	
	
static Boolean opPushString (void *source, void *dest) {

	/*
	insert the source string at the end of the destination string.
	
	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	char *s = source, *d = dest;
	unsigned char lensource = s [0];
	unsigned char lendest = d [0];
	unsigned short newlen = lensource + lendest;
	
	if (newlen > 255)
		return (false);
		
	d [0] = (unsigned char) newlen;
	
	d += (unsigned char) (lendest + 1); /*point at the position we're copying into*/
	
	s++; /*point at 1st char after length byte*/
	
	while (lensource--) 
		*d++ = *s++;
	
	return (true);
	} /*opPushString*/
	
	
static Boolean opPushChar (char ch, Str255 bs) {
	
	/*
	insert the character at the end of a pascal string.
	*/
	
	unsigned char len;
	
	len = bs [0]; 
	
	if (len >= 255)
		return (false);
	
	bs [++len] = ch;
	
	bs [0] = len;
	
	return (true);
	} /*opPushChar*/
	
	
static void opAllLower (Str255 bs) {
	
	unsigned char len = bs [0];
	char *p = (char *) &bs [1];
	char ch;
	
	while (len--) {
		
		ch = *p;
		
		if ((ch >= 'A') && (ch <= 'Z'))
			*p += 32;
			
		p++;
		} /*while*/
	} /*opAllLower*/
	
	
static void opDiskGetFontName (short fontnum, diskfontstring fontname) {
	
	/*
	Apple recommends that fonts be stored on disk as strings.  we return the
	fontname, limited in length to 32, based on the indicated font number.
	*/
	
	Str255 bs;
	
	GetFontName (fontnum, bs);
	
	if (bs [0] > diskfontnamelength)
		bs [0] = diskfontnamelength;
	
	opCopyString (bs, fontname);
	} /*opDiskGetFontName*/


static void opDiskGetFontNum (diskfontstring fontname, short *fontnum) {
	
	if (fontname [0] == 0)
		*fontnum = geneva;
	else
		GetFNum ((StringPtr) fontname, fontnum);
	} /*opDiskGetFontNum*/
	
	
pascal void opGetHeadString (hdlheadrecord hnode, Str255 bs) {
	
	if (hnode == nil) { /*defensive driving*/
		
		bs [0] = 0;
		
		return;
		}
		
	opCopyHeapString ((**hnode).hstring, bs);
	} /*opGetHeadString*/


pascal Boolean opSetHeadString (hdlheadrecord hnode, Str255 bs) {
	
	StringHandle hstring;
	
	if (!opNewHeapString (bs, &hstring))
		return (false);
	
	DisposHandle ((Handle) (**hnode).hstring);
	
	(**hnode).hstring = hstring;
	
	return (true);
	} /*opSetHeadString*/


pascal Boolean opIsFirstInList (hdlheadrecord hnode) {
	
	/*
	return true if the node is the first at its level.
	*/
	
	return ((**hnode).headlinkup == hnode);
	} /*opIsFirstInList*/
	
	
pascal Boolean opIsLastInList (hdlheadrecord hnode) {
	
	/*
	return true if the node is the last at its level.
	*/
	
	return ((**hnode).headlinkdown == hnode);
	} /*opIsLastInList*/
	

pascal Boolean opHasSubheads (hdlheadrecord hnode) {
	
	/*
	return true if the node has subheads.
	*/
	
	return ((**hnode).headlinkright != hnode);
	} /*opHasSubheads*/
	

pascal Boolean opSubheadsExpanded (hdlheadrecord hnode) {
	
	/*
	return true if the node has subheads and they are expanded.
	*/
	
	hdlheadrecord hkid = (**hnode).headlinkright;
	
	if (hkid == hnode) /*no subheads*/
		return (false);
		
	return ((**hkid).flexpanded);
	} /*opSubheadsExpanded*/
	
	
pascal Boolean opIsSubordinateTo (hdlheadrecord h1, hdlheadrecord h2) {
	
	/*
	return true if h1 is part of h2's structure.
	
	another way of saying the same thing: return true if h2 is in 
	the path back to a summit from h1.
	*/
	
	hdlheadrecord x = h1, y = h2;
	
	if (x == y) /*a node is not subordinate to itself*/
		return (false);
	
	while (true) {
		
		if ((**x).headlevel == 0) /*at a summit, h1 is not subordinate to h2*/
			return (false);
		
		x = (**x).headlinkleft; /*move towards a summit*/
		
		if (x == y) /*x is subordinate to y*/
			return (true);
		} /*while*/
	} /*opIsSubordinateTo*/
	
	
pascal Boolean opIsNestedInComment (hdlheadrecord hnode) {
	
	/*
	return true if the node is subordinate to a comment line, or if it is
	a comment line itself.
	*/
	
	hdlheadrecord x = hnode;
	
	while (true) {
		
		if ((**x).flcomment) /*it is nested inside a comment*/
			return (true);
			
		if ((**x).headlevel <= 0) /*can't surface any further*/
			return (false);
			
		x = (**x).headlinkleft;
		} /*while*/
	} /*opIsNestedInComment*/
	
      
pascal hdlheadrecord opGetAncestor (hdlheadrecord hnode, short level) {
	
	/*
	surface out from hnode until you reach a node at the indicated level.
	
	interesting function -- it's used in implementing dragging move.
	*/
	
	hdlheadrecord x = hnode;
	
	while (true) {
		
		short headlevel = (**x).headlevel;
		
		if (headlevel <= level) /*surfaced out far enough*/
			return (x);
			
		if (headlevel <= 0) /*internal error*/
			return (nil);
			
		x = (**x).headlinkleft;
		} /*while*/
	} /*opGetAncestor*/
	
	
pascal hdlheadrecord opGetLastSubhead (hdlheadrecord hnode) {
	
	/*
	return a handle to the last child subordinate to the node.
	
	if the node has no children, return nil.
	*/
	
	hdlheadrecord x = (**hnode).headlinkright;
	hdlheadrecord nextx;
	
	if (x == hnode) /*has no subheads*/
		return (nil);
		
	while (true) {
		
		nextx = (**x).headlinkdown;
		
		if (nextx == x) /*reached the last subhead*/
			return (x);
			
		x = nextx; /*advance to next node*/
		} /*while*/
	} /*opGetLastSubhead*/
	
      
pascal hdlheadrecord opGetNthSubhead (hdlheadrecord hnode, short n) {
	
	/*
	return the nth subhead of the indicated node, nil if he doesn't have
	that many subheads.
	
	the number is 1-based, so the first sub is returned when n == 1.
	*/
	
	hdlheadrecord h = hnode;
	hdlheadrecord nomad = (**h).headlinkright;
	hdlheadrecord nextnomad;
	short i;
	
	if (n <= 0) /*special case*/
		return (h);
	
	if (nomad == h) /*no subs*/
		return (nil);
	
	for (i = 1; i < n; i++) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad) /*aren't that many subs*/
			return (nil);
			
		nomad = nextnomad; /*advance to next sub*/
		} /*for*/
		
	return (nomad);
	} /*opGetNthSubhead*/
	
	
pascal hdlheadrecord opGetNthSummit (short n) {
	
	/*
	return the nth summit of the current structure. 
	
	if there aren't enough summits, return nil.
	*/
	
	hdlheadrecord x = (**outlinedata).hsummit;
	short ctloops = n - 1;
	short i;
	
	if (outlinedata == nil) /*defensive driving*/
		return (nil);
	
	for (i = 1; i <= ctloops; i++) {
		
		hdlheadrecord nextx = (**x).headlinkdown;
		
		if (nextx == x) /*reached the end of the list, less than n summits*/
			return (nil);
			
		x = nextx;
		} /*for*/
	
	return (x);
	} /*opGetNthSummit*/


pascal hdlheadrecord opGetLastExpanded (hdlheadrecord hnode) {
	
	/*
	return the last expanded subhead under hnode.  
	
	if it has no subheads or it is not expanded, we return the node itself.
	*/
	
	hdlheadrecord h = hnode;
	
	while (true) {
	
		if (!opSubheadsExpanded (h))
			return (h);
			
		h = opGetLastSubhead (h);
		} /*while*/
	} /*opGetLastExpanded*/
	
	
pascal hdlheadrecord opGetFirstAtLevel (hdlheadrecord hnode) {
	
	/*
	shoot from hnode up to the first guy at his level, returning with
	a handle to the first guy.
	*/
	
	hdlheadrecord h = hnode;
	hdlheadrecord next;
	
	if (h == nil) /*defensive driving*/
		return (nil);
		
	while (true) {
		
		next = (**h).headlinkup;
		
		if (next == h) /*no way to go*/
			return (h);
			
		h = next;
		} /*while*/
	} /*opGetFirstAtLevel*/
	

pascal short opCountAtLevel (hdlheadrecord hnode) {
	
	/*
	return the number of nodes at the same level as the node.
	*/
	
	short ct = 1;
	
	hnode = opGetFirstAtLevel (hnode);
	
	while (opGo (&hnode, opDown)) 
		ct++;
		
	return (ct);
	} /*opCountAtLevel*/
	
	
pascal short opGetSiblingNumber (hdlheadrecord hnode) {
	
	/*
	if hnode is the third head at its level, return 3. 
	*/
	
	short ct = 1;
	
	while (opGo (&hnode, opUp))
		ct++;
	
	return (ct);
	} /*opGetSiblingNumber*/
	
	
pascal short opCountSummits (void) {
	
	/*
	return the number of summits in the outline.
	*/
	
	if (outlinedata == nil)
		return (0);
		
	return (opCountAtLevel ((**outlinedata).hsummit));
	} /*opCountSummits*/
	
	
pascal short opCountSubs (hdlheadrecord hnode) {
	
	/*
	returns the number of first level subs underneath the indicated headrecord.
	*/
	
	hdlheadrecord hright = (**hnode).headlinkright;
	
	if (hright == hnode) /*no subs*/
		return (0);
		
	return (opCountAtLevel (hright));
	} /*opCountSubs*/
	
	
pascal Boolean opRecursivelyVisit (hdlheadrecord h, short lev, opvisitcallback visit) {

	/*
	visit all the children of the headrecord in level-first order, ie first visit
	the node, then visit its children and so on...
	*/
	
	hdlheadrecord nomad;
	
	if (h == nil)
		return (true);
		
	nomad = (**h).headlinkright;
	
	if (nomad == h) /*nothing to the right*/
		return (true);
		
	while (true) {
		
		if (!(*visit) (nomad))
			return (false);
			
		if (lev > 1)
			if (!opRecursivelyVisit (nomad, lev - 1, visit))
				return (false);
			
		if ((**nomad).headlinkdown == nomad) /*just processed last subhead*/
			return (true);
			
		nomad = (**nomad).headlinkdown;
		} /*while*/
	} /*opRecursivelyVisit*/


pascal Boolean opRecursivelyVisitKidsFirst (hdlheadrecord h, short lev, opvisitcallback visit) {
	
	/*
	visit all the children of the headrecord in depth-first order, ie first visit the 
	children, then visit the node itself. useful for operations that delete nodes, or 
	might delete nodes.
	*/
		
	hdlheadrecord nomad, nextnomad;
	
	if (h == nil)
		return (true);
		
	nomad = (**h).headlinkright;
	
	if (nomad == h) /*nothing to the right*/
		return (true);
		
	while (true) {
		
		if (lev > 1)
			if (!opRecursivelyVisitKidsFirst (nomad, lev - 1, visit))
				return (false);
		
		nextnomad = (**nomad).headlinkdown; /*visit may dealloc nomad*/
		
		if (!(*visit) (nomad))
			return (false);
			
		if (nextnomad == nomad) /*just processed last subhead*/
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	} /*opRecursivelyVisitKidsFirst*/
	

pascal Boolean opSiblingVisiter (hdlheadrecord hnode, Boolean flkidsfirst, opvisitcallback visit) {

	/*
	visit the node and all the siblings down from it and all sub-nodes.
	*/
	
	hdlheadrecord nomad = hnode;
	hdlheadrecord nextnomad;
	
	if (nomad == nil) /*defensive driving*/
		return (false);
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (flkidsfirst) {
			
			if (!opRecursivelyVisitKidsFirst (nomad, infinity, visit))
				return (false);
				
			if (!(*visit) (nomad))
				return (false);
			}
			
		else {
			if (!(*visit) (nomad))
				return (false);
			
			if (!opRecursivelyVisit (nomad, infinity, visit))
				return (false);
			}
			
		if (nextnomad == nomad) /*have visited the last sibling*/
			return (true);
			
		if (nextnomad == nil) /*11/10/88: houtlinescrap sometimes has a nil down ptr*/
			return (true);
			
		nomad = nextnomad; /*advance to the next sibling*/
		} /*while*/
	} /*opSiblingVisiter*/
	
	
pascal Boolean opVisitOutline (opvisitcallback visit) {

	return (opSiblingVisiter ((**outlinedata).hsummit, false, visit));
	} /*opVisitOutline*/


static Boolean opResetLevelsVisit (hdlheadrecord hnode) {
	
	hdlheadrecord h = hnode;
	
	(**h).headlevel = (**(**h).headlinkleft).headlevel + 1;
	
	return (true);
	} /*opResetLevelsVisit*/
	
	
pascal void opResetLevels (hdlheadrecord hnode) {
	
	/*
	the headlevel field is dependent on the structure of the outline and
	can always be calculated from the structure. it's included in the 
	headrcord for efficiency and convenience.
	
	call this routine after you've reorganized the outline, or added a 
	new headline, or deleted one. we recalc the headlevel fields for all
	the lines subordinate to the node. make sure it has the correct
	headlevel value before calling.
	*/

	opRecursivelyVisit (hnode, infinity, &opResetLevelsVisit);
	} /*opResetLevels*/
	
	
short intforcount = 0;


static Boolean opCountVisit (hdlheadrecord hnode) {
	
	intforcount++; /*the simplest visit routine*/
	
	return (true);
	} /*opCountVisit*/
	
	
static short opCountSubheads (hdlheadrecord hnode, short level) {
	
	intforcount = 0;
	
	opRecursivelyVisit (hnode, level, &opCountVisit);
	
	return (intforcount);
	} /*opCountSubheads*/
	
	
pascal short opCountAllHeads (void) {
	
	/*
	return the number of headlines linked into the current outlinerecord.
	*/
	
	hdlheadrecord nomad = (**outlinedata).hsummit;
	short ct = 0;
	
	while (true) {
		
		ct += opCountSubheads (nomad, infinity) + 1;
		
		if (!opGo (&nomad, opDown))
			return (ct);
		} /*while*/
	} /*opCountAllHeads*/
	

pascal Boolean opSetTarget (hdloutlinerecord houtline) {
	
	/*
	call this routine to set the global if you don't care about preserving the
	current outlinerecord. patterned after the target.set verb in Frontier.
	*/
	
	outlinedata = houtline;
	
	return (true);
	} /*opSetTarget*/
	
		
pascal Boolean opPushOutline (hdloutlinerecord houtline) {
	
	/*
	when you want to temporarily work with a different outlinerecord, call this
	routine, do your stuff and then call opPopOutline.
	*/
	
	if (topoutlinestack >= ctoutlinestack) {
		
		DebugStr ("\poutline stack overflow!");
		
		return (false);
		}
	
	outlinestack [topoutlinestack++] = outlinedata;
	
	outlinedata = houtline;
	
	return (true);
	} /*opPushOutline*/
		

pascal Boolean opPopOutline (void) {
	
	if (topoutlinestack <= 0)
		return (false);
	
	outlinedata = outlinestack [--topoutlinestack];
	
	return (true);
	} /*opPopOutline*/


static hdlheadrecord opBumpFlatDown (hdlheadrecord hnode, Boolean flexpanded) {
	
	/*
	return the node that is flatdown from the indicated node.  if the Boolean
	is true, then only expanded nodes are visited.
	
	if hnode points to the last node, return the node itself.
	*/
	
	hdlheadrecord h = hnode;
	hdlheadrecord hcheck;
	
	hcheck = (**h).headlinkright; /*first check subhead #1*/
	
	if (hcheck != h) { /*has subheads*/
		
		if (!flexpanded) /*caller doesn't care if it's expanded*/
			return (hcheck);
		
		if ((**hcheck).flexpanded)
			return (hcheck);
		}
		
	hcheck = (**h).headlinkdown; /*check next sibling*/
	
	if (hcheck != h) { /*has subheads*/
		
		if (!flexpanded) /*caller doesn't care if it's expanded*/
			return (hcheck);
			
		if ((**hcheck).flexpanded)
			return (hcheck);
		}
	
	while (true) {
		
		if ((**h).headlinkleft == h) /*at a summit*/
			return (hnode); 
		
		h = (**h).headlinkleft; /*move out to parent*/
		
		hcheck = (**h).headlinkdown; /*check next sibling*/
		
		if (hcheck != h) { /*has subheads*/
			
			if (!flexpanded) /*caller doesn't care if it's expanded*/
				return (hcheck);
				
			if ((**hcheck).flexpanded)
				return (hcheck);
			}
		} /*while*/
	} /*opBumpFlatDown*/
	
	
static hdlheadrecord opBumpFlatUp (hdlheadrecord hnode, Boolean flexpanded) {
	
	hdlheadrecord h = hnode;
	hdlheadrecord origh = h;
	hdlheadrecord lasth;
	
	if ((**h).headlinkup == h) { /*no way up*/
		
		if ((**h).headlinkleft != h) /*not at summit*/
			return ((**h).headlinkleft);
			
		return (origh); /*hnode is on the summit -- no movement*/
		}
	
	h = (**h).headlinkup; /*go to previous sibling*/
	
	if (flexpanded && !(**h).flexpanded) { /*special case*/
		
		if (h != origh) /*we moved, though still collapsed*/
			return (h);
		
		return ((**h).headlinkleft); /*bump out a level -- hopefully expanded*/
		}
	
	while (true) {
		
		lasth = h;
		
		h = opBumpFlatDown (h, flexpanded);
		
		if (h == origh)
			return (lasth);
		} /*while*/
	} /*opBumpFlatUp*/


static Boolean opGoUp (hdlheadrecord *hnode) {
	
	hdlheadrecord h = *hnode;
	hdlheadrecord hup;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hup = (**h).headlinkup;
	
	if (hup == h)
		return (false);
		
	*hnode = hup;
	
	return (true);
	} /*opGoUp*/
	
	
static Boolean opGoDown (hdlheadrecord *hnode) {
	
	hdlheadrecord h = *hnode;
	hdlheadrecord hdown;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hdown = (**h).headlinkdown;
	
	if (hdown == h)
		return (false);
		
	*hnode = hdown;
	
	return (true);
	} /*opGoDown*/
	
	
static Boolean opGoLeft (hdlheadrecord *hnode) {
	
	hdlheadrecord h = *hnode;
	hdlheadrecord hleft;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hleft = (**h).headlinkleft;
	
	if (hleft == h)
		return (false);
		
	*hnode = hleft;
	
	return (true);
	} /*opGoLeft*/
	
	
static Boolean opGoRight (hdlheadrecord *hnode) {
	
	hdlheadrecord h = *hnode;
	hdlheadrecord hright;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hright = (**h).headlinkright;
	
	if (hright == h)
		return (false);
		
	*hnode = hright;
	
	return (true);
	} /*opGoRight*/
	
	
pascal Boolean opGo (hdlheadrecord *hnode, opDirection dir) {
	
	/*
	flatten out the navigation routines to emulate Frontier's "op.go" verb.
	*/

	switch (dir) {
		
		case opUp:
			return (opGoUp (hnode));
			
		case opDown:
			return (opGoDown (hnode));
			
		case opLeft:
			return (opGoLeft (hnode));
			
		case opRight:
			return (opGoRight (hnode));
			
		case opFlatup: {
			hdlheadrecord x = *hnode;
			
			x = opBumpFlatUp (x, false);
			
			if (x == *hnode) /*no motion*/
				return (false);
				
			*hnode = x;
			
			return (true);
			}
			
		case opFlatdown: {
			hdlheadrecord x = *hnode;
			
			x = opBumpFlatDown (x, false);
			
			if (x == *hnode) /*no motion*/
				return (false);
				
			*hnode = x;
			
			return (true);
			}
			
		} /*switch*/
		
	return (false);
	} /*opGo*/
	
	
static hdlheadrecord opRepeatedBump (opDirection dir, short ctbumps, hdlheadrecord hstart, Boolean flexpanded) {
	
	/*
	navigate from hstart in the indicated opDirection, ctbumps times.
	
	if flexpanded is true only visit nodes that are expanded.
	
	return the resulting node.
	
	this was designed early on, and doesn't return a Boolean indicating whether
	it was able to move in the desired opDirection.
	*/
	
	hdlheadrecord nomad = hstart;
	short ct = ctbumps;
	Boolean fl = flexpanded;
	hdlheadrecord lastnomad;
	short i;
	
	if (nomad == nil) /*defensive driving*/
		return (nil);
	
	if (ct < 0) {
		
		ct = -ct;
		
		switch (dir) {
			
			case opFlatup:
				dir = opFlatdown;
				
				break;
				
			case opFlatdown:
				dir = opFlatup;
				
				break;
				
			case opLeft:
				dir = opRight;
				
				break;
				
			case opRight:
				dir = opLeft;
				
				break;
				
			case opUp:
				dir = opDown;
				
				break;
				
			case opDown:
				dir = opUp;
				
				break;
			} /*switch*/
		} /*ct < 0*/
		
	switch (dir) {
		
		case opFlatup:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = opBumpFlatUp (nomad, fl);
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case opFlatdown:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = opBumpFlatDown (nomad, fl);
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case opLeft:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkleft;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case opRight:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkright;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case opUp:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkup;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case opDown:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkdown;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		} /*switch*/
	
	L1:
			
	return (nomad);
	} /*opRepeatedBump*/


pascal short opSetCountExpanded (void) {
	
	/*
	when you load an outline, or do something too complex to maintain
	arithmetically, call this routine to brute force compute the number
	of expanded headlines.
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hsummit;
	short ct = 1; /*always at least one line expanded*/
	hdlheadrecord x;
	
	while (true) {
		
		x = nomad;
		
		nomad = opRepeatedBump (opFlatdown, 1, nomad, true);
		
		if (nomad == x) { /*could go no further*/
			
			(**ho).ctexpanded = ct;
			
			return (ct);
			}
			
		ct++;
		} /*while*/
	} /*opSetCountExpanded*/
	
	
static void opSetExpandedBits (hdlheadrecord hsummit, Boolean fl) {
	
	/*
	set the expanded bits of the node and all its siblings as indicated
	by the Boolean.
	*/

	Boolean bit = fl;
	hdlheadrecord nomad = hsummit;
	hdlheadrecord lastnomad;
	
	if (nomad == nil) /*defensive driving*/
		return;
	
	while (true) {
		
		(**nomad).flexpanded = bit;
		
		lastnomad = nomad;
		
		nomad = (**nomad).headlinkdown;
		
		if (nomad == lastnomad) 
			return;
		} /*while*/
	} /*opSetExpandedBits*/


static void opGetFlatLineNum (hdlheadrecord hnode, short *lnum) {
	
	/*
	get the node line number for the indicated node.  the first summit is
	node 0.  its first sub, if expanded, is node 1.
	
	we compute the number by bumping our way flatup until we can bump no more,
	counting nodes as we go.
	*/
	
	hdlheadrecord nomad = hnode;
	hdlheadrecord lastnomad;
	short ct = 0;
	
	while (true) {
		
		lastnomad = nomad;
		
		nomad = opBumpFlatUp (nomad, true);
		
		if (nomad == lastnomad) {
			
			*lnum = ct;
			
			return;
			}
			
		ct++;
		} /*while*/
	} /*opGetFlatLineNum*/


static Boolean opNewHeadRecord (Str255 bshead, hdlheadrecord *hnewnode) {
 	
 	/*
 	create a new headline, without linking into any structure.
 	*/
 	
 	hdlheadrecord h;
 	StringHandle hstring;
 	Handle hnew;
 	
 	if (!opNewClearHandle (longsizeof (tyheadrecord), &hnew))
 		return (false);
 		
 	h = *hnewnode = (hdlheadrecord) hnew; /*copy into register*/
 	
 	if (!opNewHeapString (bshead, &hstring)) {
 	
 		DisposHandle ((Handle) h);
 		
 		return (false);
 		}
 		
 	(**h).hstring = hstring;
 	
 	(**h).headlinkup = (**h).headlinkdown = (**h).headlinkleft = (**h).headlinkright = h;
 	
 	(**h).fldirty = true; /*in need of display*/
 	
 	return (true);
 	} /*opNewHeadRecord*/
 

static Boolean opNewStructure (Str255 bssummit, hdlheadrecord *hsummit) {

	/*
	create a new summit for the current outline record.
	*/
	
	hdlheadrecord hnode;
	StringHandle hstring;
	Handle hnew;
	
	if (!opNewClearHandle (longsizeof (tyheadrecord), &hnew))
		return (false);
	
	hnode = (hdlheadrecord) hnew; /*copy into register*/
	
	if (!opNewHeapString (bssummit, &hstring)) {
	
		DisposHandle ((Handle) hnode);
		
		return (false);
		}
		
	(**hnode).hstring = hstring;
	
	(**hnode).headlinkleft = (**hnode).headlinkright = hnode;
	
	(**hnode).headlinkup = (**hnode).headlinkdown = hnode;
	
	(**hnode).fldirty = true;
	
	(**hnode).flexpanded = true;
	
	*hsummit = hnode;
	
	return (true);
	} /*opNewStructure*/


static Boolean opReleaseHeadline (hdlheadrecord hnode) {
	
	hdlheadrecord h = hnode;
	Handle hrefcon = (Handle) (**h).headrecordrefcon;
	
	if (hrefcon != nil) /*node has a refcon handle attached*/	
		DisposHandle (hrefcon); 
	
	DisposHandle ((Handle) (**h).hstring); /*get rid of the head string*/
	
	DisposHandle ((Handle) h); 
	
	return (true);
	} /*opReleaseHeadline*/
	
	
static Boolean opSetSummit (hdloutlinerecord houtline, hdlheadrecord hnode) {
	
	/*
	establish hnode as the summit of the indicated outline.  dispose of 
	the existing summit, if one exists.  also, make sure that the levels 
	of the outline are set correctly, starting at zero.
	*/
	
	hdloutlinerecord ho = houtline;
	hdlheadrecord h = hnode;
	
	if ((**h).headlevel != 0) {
		
		(**h).headlevel = 0;
		
		opResetLevels (h);
		}
	
	if ((**ho).hsummit != nil)
		opDisposeStructure ((**ho).hsummit);
	
	(**ho).hsummit = (**ho).hbarcursor = (**ho).hline1 = h;
	
	return (true);
	} /*opSetSummit*/


static Boolean opNewSummit (void) {
	
	/*
	create a new, blank summit for the current outline.  link it into the
	structure accordingly.
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord hnewsummit;
	
	if (!opNewStructure ("\p", &hnewsummit))
		return (false);
	
	return (opSetSummit (ho, hnewsummit));
	} /*opNewSummit*/


pascal Boolean opNewOutlineRecord (hdloutlinerecord *houtline) {
	
	/*
	create a new outline record, returned in houtline.  we assume nothing
	about outlinewindowinfo or outlinewindow, and we preserve outlinedata.
	
	the rectangles and displayinfo are all zero after we're called.
	*/
	
	hdloutlinerecord ho;
	Boolean fl; 
	unsigned long time;
	
	if (!opNewClearHandle (longsizeof (tyoutlinerecord), (Handle *) houtline))
		return (false);
		
	ho = *houtline; /*copy into register*/
	
	opPushOutline (ho);
	
	fl = opNewSummit ();
	
	opPopOutline ();
		
	if (!fl) {	
	
		DisposHandle ((Handle) ho);
	
		return (false);
		}
		
	(**ho).ctexpanded = 1; /*the summit line is expanded*/
	
	(**ho).lineindent = 15;
	
	(**ho).linespacing = opOneandalittlespaced;
	
	GetDateTime (&time);
	
	(**ho).timecreated = time;
	
	return (true);
	} /*opNewOutlineRecord*/
	
	
pascal void opDisposeStructure (hdlheadrecord hnode) {
	
	/*
	dispose of the structure pointed to by hnode, including all his
	submaterial and all of his siblings' submaterial.
	
	be sure to unlink the node first, if you don't want all his siblings
	to be disposed of too...
	
	if fldisk, then all disk storage used by the structure is dealloc'd
	too.
	*/
	
	opSiblingVisiter (hnode, true, &opReleaseHeadline);
	} /*opDisposeStructure*/
	
	
pascal void opDisposeOutlineRecord (hdloutlinerecord houtline) {
	
	hdloutlinerecord ho = houtline;
	Handle hrefcon;
	
	if (ho == nil) /*defensive driving*/
		return;
	
	opPushOutline (ho); /*set the current outline to this one*/
	
	opDisposeStructure ((**ho).hsummit);
	
	opPopOutline (); /*finished popping all hoists*/
	
	hrefcon = (Handle) (**ho).outlinerefcon;
	
	if (hrefcon != nil)
		DisposHandle (hrefcon);
		
	DisposHandle ((Handle) ho);
	} /*opDisposeOutlineRecord*/
	

static Boolean outTableVisit (hdlheadrecord hnode) {
	
	hdlheadrecord hn = hnode;
	Handle hrefcon = (**hn).headrecordrefcon;
	long lenrefcon;
	tylinetableitem item;
	
	opClearBytes (&item, longsizeof (item)); /*clear all unused bits*/
	
	item.flexpanded = (**hn).flexpanded;
	
	item.flcomment = (**hn).flcomment;
	
	item.flbreakpoint = (**hn).flbreakpoint;
	
	item.lenrefcon = lenrefcon = GetHandleSize (hrefcon); /*0 if it's nil*/
	
	(**hnode).tmpbit = false;
	
	if (!opEnlargeHandle (packhandle, longsizeof (tylinetableitem), (char *) &item)) {
	
		DisposHandle (packhandle);
		
		return (false);
		}
	
	if (lenrefcon > 0) { /*something stored in the refcon field*/
	
		HLock (hrefcon);
		
		if (!opEnlargeHandle (packhandle, item.lenrefcon, *hrefcon)) {
			
			HUnlock (hrefcon);
			
			DisposHandle (packhandle);
			
			return (false);
			}
			
		HUnlock (hrefcon);
		}
		
	return (true);
	} /*outTableVisit*/


static Boolean outlineToTable (hdlheadrecord hnode, Handle h, long *ctbytes) {
	
	/*
	traverse the current outline, producing a table of information 
	with one element for each line in the outline.  append that table
	to the end of the indicated handle and return the number of bytes
	added to the handle.
	
	return false if there was a memory error.
	*/
	
	long origsize;
	
	*ctbytes = 0;
	
	origsize = GetHandleSize (h);
	
	packhandle = h;
	
	if (!opSiblingVisiter (hnode, false, &outTableVisit))
		return (false);
		
	*ctbytes = GetHandleSize (h) - origsize;
	
	return (true);
	} /*outlineToTable*/


static Boolean outTextVisit (hdlheadrecord hnode) {
	
	short level = (**hnode).headlevel;
	Str255 bs;
	
	bs [0] = level;
	
	opFillChar (&bs [1], (long) level, (char) 9);
	
	opPushString (*(**hnode).hstring, bs);
	
	opPushChar ((char) 13, bs);
	
	if (!opPushTextHandle (bs, packhandle)) {
	
		DisposHandle (packhandle);
		
		return (false);
		}
	
	return (true);
	} /*outTextVisit*/
	

static Boolean outlineToText (hdlheadrecord hnode, Handle htext, long *ctbytes) {
	
	/*
	convert the outline into a block of tab-indented text, each
	line ended by a carriage return.  suitable for saving to disk
	because we convert the whole outline, all level-0 heads in
	the current outline.
	
	push the resulting text at the end of the indicated handle and
	return in ctbytes the number of bytes added to the handle.
	
	return false if there was a memory allocation error.
	*/
	
	Handle h = htext;
	long origbytes;
	
	origbytes = GetHandleSize (h);
	
	*ctbytes = 0;
	
	packhandle = h;
	
	if (!opSiblingVisiter (hnode, false, &outTextVisit))
		return (false);
	
	*ctbytes = GetHandleSize (h) - origbytes;
	
	return (true);
	} /*outlineToText*/


pascal Boolean opPack (Handle *hpackedoutline) {
	
	/*
	create a packed, contiguous version of the current outline record.
	
	DW 3/25/90: if hpackedoutline comes in non-nil, we just append our
	stuff to the end of the handle, we don't allocate a new one.
	
	dmb 10/16/90: don't dispose of handle if we didn't allocate it
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord hsummit;
	Handle h;
	short ixheader;
	tydiskoutlineheader header;
	Boolean flallocated = false;
	Boolean flerror = false;
	
	h = *hpackedoutline; /*copy into register*/
	
	opClearBytes (&header, longsizeof (header)); /*assure all bits set to 0*/
	
	if (h == nil) { /*the normal case, allocate a new handle*/
	
		if (!opNewClearHandle (longsizeof (header), hpackedoutline))
			return (false);
		
		flallocated = true;
		
		h = *hpackedoutline;
		
		ixheader = 0;
		}
	else {
		ixheader = GetHandleSize (h); /*where we move the header in*/
		
		if (!opEnlargeHandle (h, longsizeof (header), (char *) &header))
			return (false);
		}
	
	header.versionnumber = 2;
	
	opGetFlatLineNum ((**ho).hbarcursor, &header.lnumcursor);

	header.linespacing = (**ho).linespacing;
	
	header.lineindent = (**ho).lineindent;
	
	header.vertmin = (**ho).vertmin;
	
	header.vertmax = (**ho).vertmax;
	
	header.vertcurrent = (**ho).vertcurrent;
	
	header.horizmin = (**ho).horizmin;
	
	header.horizmax = (**ho).horizmax;
	
	header.horizcurrent = (**ho).horizcurrent;
	
	header.timecreated = (**ho).timecreated;
	
	GetDateTime ((unsigned long *) &header.timelastsave); /*stamp it*/
	
	header.ctsaves = ++(**ho).ctsaves;
	
	opDiskGetFontName ((**ho).fontnum, header.fontname);
	
	header.fontsize = (**ho).fontsize;
	
	header.fontstyle = (**ho).fontstyle;
	
	header.windowrect = (**ho).windowrect;
	
	hsummit = (**ho).hsummit; /*copy into register*/
	
	if (!outlineToText (hsummit, h, &header.sizetext)) {
		
		flerror = true;
		
		goto exit;
		}
		
	if (!outlineToTable (hsummit, h, &header.sizelinetable)) {
	
		flerror = true;
		
		goto exit;
		}
	
	/*move the header into handle*/ {
		
		char *p;
		
		HLock (h); /*about to do a moveleft*/
		
		p = *h;
		
		p += ixheader;
		
		opMoveLeft (&header, p, longsizeof (header));
		
		HUnlock (h);
		}
	
	exit:
	
	if (flerror) {
		
		if (flallocated)
			DisposHandle (h);
		
		return (false);
		}
	
	return (true);
	} /*opPack*/


static long ixtable; /*must be global for recursion*/
	

static Boolean inTableVisit (hdlheadrecord hnode) {
	
	hdlheadrecord hn = hnode;
	char *p;
	long lenrefcon;
	Handle hrefcon;
	tylinetableitem item;
	
	p = *packhandle + ixtable + ixpackhandle;
	
	ixpackhandle += longsizeof (tylinetableitem);

	opMoveLeft (p, &item, longsizeof (tylinetableitem));
	
	(**hn).flexpanded = item.flexpanded;
	
	(**hn).flcomment = item.flcomment;
	
	(**hn).flbreakpoint = item.flbreakpoint;
	
	lenrefcon = item.lenrefcon; /*copy into register*/
	
	if (lenrefcon > 0) { /*link in a refcon node*/
	
		if (!opNewClearHandle (lenrefcon, &hrefcon))
			return (false);
			
		p = *packhandle + ixtable + ixpackhandle;
		
		ixpackhandle += lenrefcon;
		
		opMoveLeft (p, *hrefcon, lenrefcon);
		
		(**hn).headrecordrefcon = hrefcon;
		}
	
	return (true);
	} /*inTableVisit*/
	
	
static Boolean tableToOutline (Handle htable, long ixstart, hdlheadrecord hsummit) { 

	/*
	apply values in table to hsummit outline
	*/
	
	packhandle = htable;
	
	ixtable = ixstart;
	
	ixpackhandle = 0; /*pointing to first item in the table*/
	
	return (opSiblingVisiter (hsummit, false, &inTableVisit));
	} /*tableToOutline*/
	

static short spacesforlevel;

static short firstindent;

static Boolean fltabindent;


static void clearIndentValues (Boolean flmusttabindent) {
	
	spacesforlevel = 0;
	
	firstindent = -1;
	
	fltabindent = flmusttabindent;
	} /*clearIndentValues*/


static long divRound (long n, long d) {
	
	/*
	divide numerator n by divisor d, rouding as closely as possible
	*/
	
	if (n >= 0)
		return ((n + d / 2) / d);
	
	return ((n - d / 2) / d);
	} /*divRound*/


static Boolean opGetLineText (Handle htext, long ixlast, long *ix, short *level, Str255 bs) {

	/*
	extract a line of text from the handle at offset ix.  ix gets bumped to
	point at the beginning of the next line.
	
	the level is the number of ascii 9's at the head of the string.
	
	12/13/91 dmb: added support for space-indented text import
	*/
	
	long sizetext = ixlast;
	long ixtext = *ix;
	long ct = 0;
	char *p;
	
	bs [0] = 0; /*in case we return early*/
	
	if (ixtext >= sizetext) /*no more characters available*/
		return (false);
		
	HLock (htext);
	
	p = *htext + ixtext;
	
	while (*p == (char) 9) { /*count the leading tab chars*/
	
		ct++;
		
		p++;
		
		ixtext++;
		
		if (ixtext >= sizetext) { /*no text on the line*/
			
			HUnlock (htext);
			
			return (false);
			}
		
		fltabindent = true;
		} /*while*/
	
	if (!fltabindent) {
		
		while (*p == ' ') { /*count the leading tab chars*/
			
			ct++;
			
			p++;
			
			ixtext++;
			
			if (ixtext >= sizetext) { /*no text on the line*/
				
				HUnlock (htext);
				
				return (false);
				}
			} /*while*/
		
		if (ct > 0) {
			
			if (spacesforlevel == 0) /*first run of spaces*/
				spacesforlevel = ct;
			
			ct = divRound (ct, spacesforlevel);
			}
		}
	
	if (firstindent < 0) /*first line of text*/
		firstindent = ct;
	
	*level = max (0, ct - firstindent);
	
	ct = 0;
	
	while (*p != (char) 13) { /*copy the text chars into the string*/
		
		if (ct >= 255) /*don't overwrite the buffer*/
			break;
			
		bs [0] = ++ct;
		
		bs [ct] = *p;
		
		p++;
		
		ixtext++;
		
		if (ixtext >= sizetext) /*ran out of characters in the buffer*/
			break;
		} /*while*/
	
	*ix = ixtext + 1;
	
	HUnlock (htext);
	
	return (true);
	} /*opGetLineText*/


pascal void opUnlink (hdlheadrecord hnode) {
	
	/*
	point all elements of the structure around the indicated node, and 
	set his "outbound" pointers (ie everything but his right pointer)  
	to point at himself.
	*/
	
	hdlheadrecord h = hnode;
	hdlheadrecord hdown = (**h).headlinkdown;
	hdlheadrecord hup = (**h).headlinkup;
	hdlheadrecord hleft = (**h).headlinkleft;
	
	if (h == nil) /*defensive driving*/
		return;

	if (hup == h) { /*unlinking first guy at this level*/

		if (hdown == h) { /*only child*/

			(**hleft).headlinkright = hleft; /*eliminate parent's child list*/
			
			goto exit;
			}

		if (hleft != h) /*has a parent*/
			(**hleft).headlinkright = hdown;
		
		(**hdown).headlinkup = hdown; /*he's now the 1st kid*/
		
		goto exit;
		}

	/*has a sibling above*/

	if (hdown == h) { /*last guy in his list*/
	
		(**hup).headlinkdown = hup; /*he's now the last kid*/

		goto exit;
		}

	(**hup).headlinkdown = hdown; /*point around me*/
	
	(**hdown).headlinkup = hup;
	
	exit: /*goto here to exit*/

	(**h).headlinkup = (**h).headlinkdown = (**h).headlinkleft = h;
	} /*opUnlink*/


static void opDepositDown (hdlheadrecord hpre, hdlheadrecord hdeposit) {
	
	/*
	deposit the indicated node down from hpre.
	
	modified 11/9/88 to support multiple level-0 nodes DW.
	
	11/10/88 handles moving the first summit down, updates 
	(**outlinedata).hsummit.
	
	12/23/88 more efficient code, use registers better.
	*/
	
	hdlheadrecord hp = hpre;
	hdlheadrecord hd = hdeposit;
	short headlevel = (**hp).headlevel;
	
	if (headlevel == 0)
		(**hd).headlinkleft = hd; /*points back at himself*/
	else
		(**hd).headlinkleft = (**hp).headlinkleft; /*inherits parent from pre*/
	
	(**hd).headlevel = headlevel; /*inherits level from pre*/
	
	(**hd).flexpanded = (**hp).flexpanded; /*must be consistent*/
	
	if ((**hp).headlinkdown == hp) /*inserting as the last in the list*/
	
		(**hd).headlinkdown = hd; /*point back at himself*/
		
	else { /*inserting in the middle of the list*/
		
		hdlheadrecord x = (**hp).headlinkdown;
		
		(**hd).headlinkdown = x;
		
		(**x).headlinkup = hd;
		}
	
	(**hd).headlinkup = hp;
	
	(**hp).headlinkdown = hd;

	return; /*the following code could hide serious bugs, it's irrelevant here*/
	
	/*
	if (((**hp).headlinkup == hp) && (headlevel == 0)) /*make sure hsummit is correct%/
	
		(**outlinedata).hsummit = hp;
	*/
	} /*opDepositDown*/


static void opDepositRight (hdlheadrecord hparent, hdlheadrecord hdeposit) {

	hdlheadrecord hp = hparent;
	hdlheadrecord hd = hdeposit;
	hdlheadrecord hr = (**hp).headlinkright;
	
	(**hp).fldirty = true;
	
	(**hd).headlevel = (**hp).headlevel + 1;
	
	(**hd).headlinkleft = hp;
	
	(**hd).headlinkup = hd; /*points at himself*/
	
	if (hr == hp) { /*first kid*/
		
		(**hd).headlinkdown = hd; /*points at himself*/
		
		(**hp).fldirty = true; /*might need to re-display the icon on the line*/
		}
	else {
		
		(**hd).headlinkdown = hr;
		
		(**hr).headlinkup = hd;
		
		(**hd).flexpanded = (**hr).flexpanded; /*must be consistent*/
		}
	
	(**hp).headlinkright = hd;
	} /*opDepositRight*/


static void opDepositUp (hdlheadrecord hpre, hdlheadrecord hdeposit) {
	
	/*
	deposit the indicated node up from hpre.  special case considerations if
	we're moving at the very topmost level of the structure.
	*/
	
	hdlheadrecord hp = hpre;
	hdlheadrecord hd = hdeposit;
	hdlheadrecord hleft;
	hdloutlinerecord ho = outlinedata;
			
	if (!opIsFirstInList (hp)) { /*simple in every case*/
	
		opDepositDown ((**hp).headlinkup, hd);
		
		return;
		}
	
	hleft = (**hp).headlinkleft;
	
	if (hleft != hp) { /*pre has a parent*/
	
		opDepositRight (hleft, hd); /*insert to right of pre's parent*/
		}
		
	else { /*insert as the new 0-level summit*/
		
		(**hd).headlevel = 0;
		
		(**hd).headlinkup = hd; 
		
		(**hd).headlinkdown = hp;
		
		(**hd).flexpanded = true; /*summits are always expanded*/
		
		(**hp).headlinkup = hd;
		
		if (hp == (**ho).hsummit) /*becomes the new summit*/
			(**ho).hsummit = hd;		
		}
		
	if (hp == (**ho).hline1) /*becomes the new first line*/
		(**ho).hline1 = hd;
	} /*opDepositUp*/


pascal Boolean opDeposit (hdlheadrecord hpre, opDirection dir, hdlheadrecord hdeposit) {
	
	switch (dir) {
	
		case opDown:
			opDepositDown (hpre, hdeposit);
			
			break;
			
		case opRight:
			opDepositRight (hpre, hdeposit);
			
			break;
			
		case opUp:
			opDepositUp (hpre, hdeposit);
			
			break;
			
		default:
			return (false); /*invalid direction for deposit*/
		} /*switch*/
	
	return (true);
	} /*opDeposit*/
	
	
pascal Boolean opPromoteSubheads (hdlheadrecord hnode) {
	
	while (true) {
		
		hdlheadrecord hsub = hnode;
		
		if (!opGo (&hsub, opRight)) /*has no subs*/
			return (true);
	
		while (opGo (&hsub, opDown)) {} /*move to the last sub*/
		
		opUnlink (hsub);
		
		opDepositDown (hnode, hsub);
		} /*while*/
	} /*opPromoteSubheads*/


pascal Boolean opAddHeadline (hdlheadrecord hpre, opDirection dir, Str255 bshead, hdlheadrecord *hnew) {
	
	/*
	create a new headrecord. it's position is relative to hpre, it's depositited in 
	the direction indicated by dir. return true if it worked, false if it didn't.
	*/
	
	if (!opNewHeadRecord (bshead, hnew))
		return (false);
	
	return (opDeposit (hpre, dir, *hnew));
	} /*opAddHeadline*/


static Boolean textToOutline (Handle htext, long ixstart, long ixlast, hdlheadrecord *hnode) {
	
	hdlheadrecord h = nil;
	short lastlevel;
	opDirection dir;
	hdlheadrecord hnewnode;
	long ixtext = ixstart;
	short level;
	Str255 bs;
	Boolean fl = false; /*guilty until proven innocent*/
	
	clearIndentValues (true);
	
	while (true) {
		
		if (!opGetLineText (htext, ixlast, &ixtext, &level, bs)) { /*consumed the text*/
			
			if (h != nil) /*success -- we got something*/
				fl = true;
			
			break;
			}
		
		if (h == nil) { /*first line*/
			
			if (!opNewStructure (bs, hnode))
				break;
			
			h = *hnode;
			
			lastlevel = 0;
			}
		else {
			if (level > lastlevel) 
				dir = opRight;
		
			else {
				h = opRepeatedBump (opLeft, lastlevel - level, h, true);
		
				dir = opDown;
				}
			
			if (!opAddHeadline (h, dir, bs, &hnewnode)) {
				
				opDisposeStructure (*hnode);
				
				break;
				}
			
			h = hnewnode;
			
			lastlevel = level;
			}
		} /*while*/
	
	return (fl);
	} /*textToOutline*/


static Boolean unpackVersion2 (Handle hpackedoutline, long *ixload) {
	
	hdloutlinerecord ho;
	Handle h = hpackedoutline;
	long ixstart;
	hdlheadrecord hsummit, hline1, hcursor;
	tydiskoutlineheader header;
	short fontnum;
	
	ho = outlinedata; /*copy into register*/
	
	if (!opLoadFromHandle (h, ixload, longsizeof (header), (char *) &header))
		return (false);
		
	(**ho).linespacing = header.linespacing;
	
	(**ho).lineindent = header.lineindent;
	
	(**ho).vertmin = header.vertmin;
	
	(**ho).vertmax = header.vertmax;
	
	(**ho).vertcurrent = header.vertcurrent;
	
	(**ho).horizmin = header.horizmin;
	
	(**ho).horizmax = header.horizmax;
	
	(**ho).horizcurrent = header.horizcurrent;
	
	(**ho).timecreated = header.timecreated;
	
	(**ho).timelastsave = header.timelastsave;
	
	(**ho).ctsaves = header.ctsaves;
	
	opDiskGetFontNum (header.fontname, &fontnum);
	
	(**ho).fontnum = fontnum;
	
	(**ho).fontsize = header.fontsize;

	(**ho).fontstyle = header.fontstyle;
	
	(**ho).windowrect = header.windowrect;
	
	ixstart = *ixload; /*first character in text*/
	
	if (!textToOutline (h, ixstart, ixstart + header.sizetext, &hsummit))
		return (false);
	
	*ixload += header.sizetext;
	
	opSetSummit (ho, hsummit);
	
	opSetExpandedBits (hsummit, true); /*all 1st level items are expanded*/
	
	if (!tableToOutline (h, ixstart + header.sizetext, hsummit))
		return (false);
	
	*ixload += header.sizelinetable;
	
	hline1 = opRepeatedBump (opFlatdown, header.vertcurrent, hsummit, true);
	
	(**ho).hline1 = hline1;
	
	hcursor = opRepeatedBump (opFlatdown, header.lnumcursor, hsummit, true);
	
	(**ho).hbarcursor = hcursor;
	
	opSetCountExpanded (); /*don't bother saving this on disk, we re-compute*/
	
	return (true);
	} /*unpackVersion2*/
	
	
pascal Boolean opUnpack (Handle hpackedoutline, long *ixload) {
	
	Handle h = hpackedoutline;
	long ixorig = *ixload;
	short versionnumber;
	hdloutlinerecord houtline;
	Boolean fl;
	
	if (!opNewOutlineRecord (&houtline))
		return (false);
	
	outlinedata = houtline;
	
	if (!opLoadFromHandle (h, ixload, longsizeof (versionnumber), (char *) &versionnumber))
		return (false);
	
	*ixload = ixorig; /*the header record includes the version number*/
	
	switch (versionnumber) {
		
		case 2:
			fl = unpackVersion2 (h, ixload);
			
			break;
		
		default:
			DebugStr ("\pbad outline version number");
			
			fl = false;
		} /*switch*/
	
	if (!fl)
		opDisposeOutlineRecord (houtline);
	
	return (fl);
	} /*opUnpack*/


pascal Boolean IACgetoutlineparam (OSType keyword, hdloutlinerecord *houtline) {
	
	/*
	get the outline parameter from the current Apple Event -- as indicated by the
	keyword. return true if it worked, false otherwise.
	
	follows the same pattern as the routines implemented in iac.c.
	*/
	
	AEDesc result;
	OSErr ec;
	Boolean fl;
	long ix;
	
	ec = AEGetParamDesc (IACglobals.event, (AEKeyword) keyword, 'optx', &result);
	
	if (ec != noErr) {
		
		IACparamerror (ec, "\poutline", keyword);
		
		return (false);
		}
	
	opPushOutline (nil); /*preserve outlinedata*/
	
	ix = 4; /*start loading at offset 4*/
	
	fl = opUnpack (result.dataHandle, &ix);
	
	*houtline = outlinedata;
	
	opPopOutline ();
	
	AEDisposeDesc (&result);
	
	if (!fl)
		IACreturnerror (-1, "\pError unpacking outline parameter");
	
	return (fl);
	} /*IACgetoutlineparam*/
	
	
pascal Boolean IACpushoutlineparam (hdloutlinerecord val, OSType keyword) {
	
	/*
	push the outline on the current Apple Event with the indicated key.
	
	follows the same pattern as the routines implemented in iac.c.
	*/

	AEDesc desc;
	OSErr ec;
	Handle hpackedoutline;
	Boolean fl;
	long headerbytes;
	
	hpackedoutline = NewHandle (4);
	
	if (hpackedoutline == nil)
		return (false);
		
	headerbytes = 0x00010000; /*magic incantation to make Frontier happy*/
	
	BlockMove (&headerbytes, *hpackedoutline, 4);
	
	opPushOutline (val); /*preserve outlinedata*/
	
	fl = opPack (&hpackedoutline);
	
	opPopOutline ();
	
	if (!fl)
		return (false);
	
	desc.descriptorType = 'optx';
	
	desc.dataHandle = hpackedoutline;
	
	ec = AEPutParamDesc (IACglobals.event, (AEKeyword) keyword, &desc);
	
	AEDisposeDesc (&desc);
	
	return (ec == noErr);
	} /*IACpushoutlineparam*/


pascal Boolean IACreturnoutline (hdloutlinerecord houtline) {
	
	/*
	return the outline as the result of the current Apple Event.
		
	follows the same pattern as the routines implemented in iac.c.
	*/
	
	AppleEvent *oldevent = IACglobals.event;
	Boolean fl = false;
	
	IACglobals.event = IACglobals.reply; /*push params on the reply record*/
	
	fl = IACpushoutlineparam (houtline, keyDirectObject);
	
	IACglobals.reply = oldevent; /*restore*/
	
	return (fl);
	} /*IACreturnoutline*/
	
	
