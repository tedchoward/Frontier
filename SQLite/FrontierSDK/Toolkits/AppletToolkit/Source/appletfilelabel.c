
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletbits.h"
#include "appletfilesinternal.h"
#include "appletfilelabel.h"
#include <Icons.h>




typedef struct tylabelinfo {
	
	hdlstring hstring;
	
	RGBColor rgb;
	} tylabelinfo;
	
#define ctlabels 8
	
static tylabelinfo labelcache [ctlabels];

static boolean needinit = true;


boolean filegetlabelinfo (short ixlabel, RGBColor *rgb, bigstring bslabel) {
	
	if (needinit) {
		
		short i;
		
		needinit = false;
		
		clearbytes (&labelcache, longsizeof (labelcache));
		
		for (i = 0; i < ctlabels; i++) {
			
			OSErr ec;
			RGBColor rgb;
			bigstring bs;
			
			ec = GetLabel (i, &rgb, bs);
			
			if (ec == noErr) {
				
				newheapstring (bs, &labelcache [i].hstring);
				
				labelcache [i].rgb = rgb;
				}
			} /*for*/
		}
	
	/*get the string and rgb from the label cache*/ {
		
		hdlstring hstring = labelcache [ixlabel].hstring;
		
		if (hstring == nil)
			return (false);
			
		*rgb = labelcache [ixlabel].rgb;
		
		copyheapstring (hstring, bslabel);
		
		return (true);
		}
	} /*filegetlabelinfo*/
	
	
static boolean label2index (bigstring bslabel, short *ixlabel) {
	
	short i;
	bigstring bs;
	RGBColor rgb;
	
	for (i = 0; i < 8; i++) {
		
		filegetlabelinfo (i, &rgb, bs);
		
		if (equalstrings (bs, bslabel)) {
			
			*ixlabel = i;
			
			return (true);
			}
		} /*for*/
		
	return (false);
	} /*label2index*/


boolean filesetlabel (ptrfilespec pfs, short ixlabel) {
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (pfs, &pb))
		return (false);
		
	pb.hFileInfo.ioFlFndrInfo.fdFlags &= 0xFFF1; /*clear out old index*/
	
	pb.hFileInfo.ioFlFndrInfo.fdFlags |= ixlabel << 1; /*slam in new index*/
	
	return (setmacfileinfo (pfs, &pb));
	} /*filesetlabel*/




