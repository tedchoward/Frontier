
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

#include "memory.h"
#include "strings.h"
#include "cursor.h"
#include "ops.h"
#include "quickdraw.h"
#include "oplist.h"
#include "shell.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "tablestructure.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */
#include "file.h" // 2006-09-15 creedon


#define ctsigbytes 19 /*length of string + 1 byte for length*/

#define signaturestring (ptrstring) "\x12" "packed binary data"


#pragma pack(2)
typedef struct tyoldpackedvalue {
	
	byte sigbytes [ctsigbytes]; /*a signature -- keeps errors from causing crashes*/
	
	tyvaluetype valuetype;
	
	/*depending on type, any number of bytes following contain the value data*/
	} tyoldpackedvalue, *ptroldpackedvalue, **hdloldpackedvalue;



typedef struct typackedvalue {
	
	OSType typeid;
	
	/*depending on type, any number of bytes following contain the value data*/
	} typackedvalue, *ptrpackedvalue, **hdlpackedvalue;
#pragma options align=reset

/*

static hdlpackedvalue hpackedvalue; //6.2a13 AR: eliminated for better thread-safety

static long ixunpack; //6.2a13 AR: eliminated for better thread-safety

static boolean floldformat; //6.2a13 AR: eliminated for better thread-safety
*/


static boolean langpackdata (long lendata, ptrvoid pdata, hdlpackedvalue hpackedvalue) {
	
	return (enlargehandle ((Handle) hpackedvalue, lendata, pdata));
	} /*langpackdata*/


static boolean langpackhandle (Handle hdata, hdlpackedvalue hpackedvalue) {

	boolean fl;

	if (hdata == nil)
		return (true);
	
	lockhandle (hdata);
	
	fl = langpackdata (gethandlesize (hdata), *hdata, hpackedvalue);
	
	unlockhandle (hdata);

	return (fl);
	} /*langpackhandle*/


boolean langpackvalue (tyvaluerecord val, Handle *h, hdlhashnode hnode) {
	
	/*
	4/8/93 dmb: save/restore hdlpackedvalue to allow reentrancy needed for code values
	
	5.0.2b10 dmb: don't disable langerror when getting an address path.
	
	2006-04-20 sethdill & aradke: convert rgb values from native byte order to big-endian
	*/
	
	register boolean fl;
	typackedvalue header;
	hdlpackedvalue hpackedvalue;
	Handle hdata;
	
	/*
	copystring (signaturestring, header.sigbytes); /%prevents crashes on bad data%/
	
	header.valuetype = val.valuetype;
	*/
	
	header.typeid = langexternalgettypeid (val);
	
	memtodisklong (header.typeid);

	if (!newfilledhandle (&header, sizeof (header), (Handle *) &hpackedvalue))
		return (false);
	
	switch (val.valuetype) {
	
		case novaluetype:
			fl = true; /*nothing to pack*/
			
			break;
		
		case booleanvaluetype:
			fl = langpackdata (sizeof (val.data.flvalue), &val.data.flvalue, hpackedvalue);
			
			break;
		
		case charvaluetype:
		case directionvaluetype:
			fl = langpackdata (sizeof (val.data.chvalue), &val.data.chvalue, hpackedvalue);
			
			break;
		
		case intvaluetype:
		case tokenvaluetype:
			memtodiskshort (val.data.intvalue);

			fl = langpackdata (sizeof (val.data.intvalue), &val.data.intvalue, hpackedvalue);
			
			break;
		
		case longvaluetype:
		case ostypevaluetype:
		case enumvaluetype:
		case fixedvaluetype:
			memtodisklong (val.data.longvalue);

			fl = langpackdata (sizeof (val.data.longvalue), &val.data.longvalue, hpackedvalue);
			
			break;
		
		case pointvaluetype:
			memtodiskshort (val.data.pointvalue.h);
			memtodiskshort (val.data.pointvalue.v);

			fl = langpackdata (sizeof (val.data.pointvalue), &val.data.pointvalue, hpackedvalue);
			
			break;
		
		case datevaluetype:
			memtodisklong (val.data.longvalue);

			fl = langpackdata (sizeof (val.data.datevalue), &val.data.datevalue, hpackedvalue);
			
			break;
		
		case addressvaluetype: {
			bigstring bs;
			
			/*
			copyheapstring (val.data.addressvalue, bs);
			*/
			
		//	disablelangerror ();
			
			fl = getaddresspath (val, bs);
			
		//	enablelangerror ();
			
			if (!fl)
				break;
			
			fl = langpackdata ((long) stringlength (bs), bs + 1, hpackedvalue);
			
			break;
			}
		
		case singlevaluetype:
			// ??? need swapping ???
			fl = langpackdata (sizeof (val.data.singlevalue), &val.data.singlevalue, hpackedvalue);
			
			break;
		
		case rectvaluetype: {
			diskrect rdisk;
			 
			recttodiskrect (*val.data.rectvalue, &rdisk);
			
			fl = langpackdata (sizeof (rdisk), &rdisk, hpackedvalue);
			
			break;
			}
		
		case rgbvaluetype: {	// 2006-04-20 sethdill & aradke
			diskrgb rgbdiskk;
			 
			rgbtodiskrgb (*val.data.rgbvalue, &rgbdiskk);
			
			fl = langpackdata (sizeof (rgbdiskk), &rgbdiskk, hpackedvalue);
			
			break;
			}
		
		#if noextended
		
			case doublevaluetype: {
				long double x = **val.data.doublevalue;
				extended80 x80;
				 
					safeldtox80 (&x, &x80);
									 
				fl = langpackdata (sizeof (x80), &x80, hpackedvalue);
				
				break;
				}
		#else
		
			case doublevaluetype:
			
		#endif
		
		case stringvaluetype:
		case passwordvaluetype:
		case patternvaluetype:
		case objspecvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
		case binaryvaluetype:
			fl = langpackhandle (val.data.binaryvalue, hpackedvalue);
			
			break;
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
			fl = oppacklist (val.data.listvalue, &hdata);
			
			if (!fl)
				break;
			
			fl = langpackhandle (hdata, hpackedvalue);
			
			disposehandle (hdata);

			break;
	#endif
	
	
		case filespecvaluetype:
		case aliasvaluetype:
			fl = langpackfileval (&val, &hdata);
			
			if (!fl)
				break;
			
			langpackhandle (hdata, hpackedvalue);
			
			disposehandle (hdata);
			
			break;

		case codevaluetype:
			fl = langpacktree (val.data.codevalue, &hdata);
			
			if (!fl)
				break;
			
			langpackhandle (hdata, hpackedvalue);
			
			disposehandle (hdata);
			
			break;
		
		case externalvaluetype: {
			register Handle lh = val.data.externalvalue;
			Handle hpacked;
			
			initbeachball (left);
			
			fl = langexternalmemorypack ((hdlexternalvariable) lh, &hpacked, hnode);
			
			if (!fl)
				break;
			
			langpackhandle (hpacked, hpackedvalue);
			
			disposehandle (hpacked);
			
			break;
			}
		
		default:
			langerror (cantpackerror);
			
			fl = false;
			
			break;	
		} /*switch*/
	
	if (fl)
		*h = (Handle) hpackedvalue;
	else
		disposehandle ((Handle) hpackedvalue);
	
	return (fl);
	
	} // langpackvalue


boolean langpackverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	tyvaluerecord val;
	register boolean fl;
	hdlhashtable htable;
	bigstring bsvarname;
	Handle hpacked;
	
	setbooleanvalue (false, vreturned); /*default returned value of verb*/
		
	if (!getparamvalue (hparam1, 1, &val)) /*the value to be packed*/
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bsvarname)) /*the place to put the binary value*/
		return (false);
	
	if (!langpackvalue (val, &hpacked, HNoNode))
		return (false);
	
	fl = langsetbinaryval (htable, bsvarname, hpacked);
	
	(*vreturned).data.flvalue = fl;
	
	return (fl);
	} /*langpackverb*/


boolean langpackwindowverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	6.16.97 dmb: new verb for standalone window handling.
	*/
	
	hdlwindowinfo hinfo;
	tyvaluerecord val;
	register boolean fl;
	hdlhashtable htable;
	bigstring bsvarname;
	Handle hpacked;
	Handle x;
	
	
	setbooleanvalue (false, vreturned); /*default returned value of verb*/
	
	if (!getwinparam (hparam1, 1, &hinfo)) /*the value to be packed*/
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bsvarname)) /*the place to put the binary value*/
		return (false);
	
	if (hinfo == nil) {
		
		langparamerror (badwindowerror, BIGSTRING ("\x04" "pack"));
		
		return (false);
		}
	
	if (!shellgetexternaldata (hinfo, &x))
		return (false);
	
	setexternalvalue (x, &val);
	
	if (!langpackvalue (val, &hpacked, HNoNode))
		return (false);
	
	fl = langsetbinaryval (htable, bsvarname, hpacked);
	
	(*vreturned).data.flvalue = fl;
	
	return (fl);
	} /*langpackwindowverb*/


static boolean langunpackdata (long lendata, ptrvoid pdata, hdlpackedvalue hpackedvalue, long* ptrixunpack) {
	
	return (loadfromhandle ((Handle) hpackedvalue, ptrixunpack, lendata, pdata));
	} /*langunpackdata*/


static boolean langunpackstring (hdlstring *hstring, hdlpackedvalue hpackedvalue, long* ptrixunpack) {
	
	register Handle h = (Handle) hpackedvalue;
	register long len;
	bigstring bs;
			
	if (!loadfromhandle (h, ptrixunpack, (long) 1, bs)) /*get string length*/
		return (false);
	
	len = (long) stringlength (bs);
	
	if (len > 0)
		if (!loadfromhandle (h, ptrixunpack, len, &bs [1]))
			return (false);
	
	return (newheapstring (bs, hstring));
	} /*langunpackstring*/


static boolean langunpackhandle (boolean fltemp, Handle *hbinary, hdlpackedvalue hpackedvalue, long *ptrixunpack) {
	
	/*
	load all the bytes following the header into the handle.
	*/
	
	register Handle h = (Handle) hpackedvalue;
	register long ctbytes;
	
	ctbytes = gethandlesize (h) - *ptrixunpack;
	
	return (loadfromhandletohandle (h, ptrixunpack, ctbytes, fltemp, hbinary));
	} /*langunpackhandle*/


static boolean langunpackexternal (hdlexternalhandle *hexternal, hdlpackedvalue hpackedvalue, long *ptrixunpack) {
	
	register boolean fl;
	Handle hpacked;
	
	if (!langunpackhandle (true, &hpacked, hpackedvalue, ptrixunpack))
		return (false);
	
	initbeachball (right);
	
	fl = langexternalmemoryunpack (hpacked, hexternal);
	
	disposehandle (hpacked);
	
	return (fl);
	} /*langunpackexternal*/


static boolean langunpackoldheader (tyvaluetype *valuetype, hdlpackedvalue hpackedvalue, long *ptrixunpack) {
	
	long ixorig = *ptrixunpack;
	tyoldpackedvalue oldheader;
	
	if (!langunpackdata (sizeof (oldheader), &oldheader, hpackedvalue, ptrixunpack))
		return (false);
	
	if (!equalstrings (oldheader.sigbytes, signaturestring)) {
		
		*ptrixunpack = ixorig; /*restore*/
		
		return (false);
		}
	
	*valuetype = oldheader.valuetype;
	
	return (true);
	} /*langunpackoldheader*/


boolean langunpackvalue (Handle hpacked, tyvaluerecord *val) {
	
	/*
	6/4/91 dmb: new header is just type id, but retain backward compatibility.
	
	4/8/93 dmb: save/restore hdlpackedvalue to allow reentrancy needed for code values
	
	5.0.2b3 dmb: unpacking addresses, if stringtoaddress fails, set valuetype to string
	
	2006-04-20 sethdill & aradke: convert rgb values to native byte order
	*/
	
	tyvaluerecord v;
	register hdlpackedvalue h;
	boolean fl, flpush;
	typackedvalue header;
	Handle hdata;
	long ixunpack = 0;
	
	initvalue (&v, novaluetype);
	
	h = (hdlpackedvalue) hpacked; /*copy into register*/
	
	if (langunpackoldheader (&v.valuetype, h, &ixunpack))
		goto unpack;
	
	if (!langunpackdata (sizeof (header), &header, h, &ixunpack))
		goto formaterror;
	
	disktomemlong (header.typeid);

	v.valuetype = langexternalgetvaluetype (header.typeid);
	
	langunpackoldheader (&v.valuetype, h, &ixunpack); /*may have added new header before old*/
	
unpack:
	
	switch (v.valuetype) {
	
		case novaluetype:
			fl = true; /*nothing to unpack*/
			
			break;
		
		case booleanvaluetype:
			fl = langunpackdata (sizeof (v.data.flvalue), &v.data.flvalue, h, &ixunpack);
			
			break;
			
		case charvaluetype:
		case directionvaluetype:
			fl = langunpackdata (sizeof (v.data.chvalue), &v.data.chvalue, h, &ixunpack);
			
			break;
		
		case intvaluetype:
		case tokenvaluetype:
			fl = langunpackdata (sizeof (v.data.intvalue), &v.data.intvalue, h, &ixunpack);
			
			disktomemshort (v.data.intvalue);
			break;
		
		case longvaluetype:
		case ostypevaluetype:
		case enumvaluetype:
		case fixedvaluetype:
			fl = langunpackdata (sizeof (v.data.longvalue), &v.data.longvalue, h, &ixunpack);
			
			disktomemlong (v.data.longvalue);
			break;
		
		case pointvaluetype:
			fl = langunpackdata (sizeof (v.data.pointvalue), &v.data.pointvalue, h, &ixunpack);
			
			disktomemshort (v.data.pointvalue.h);
			disktomemshort (v.data.pointvalue.v);
			break;
		
		case datevaluetype:
			fl = langunpackdata (sizeof (v.data.datevalue), &v.data.datevalue, h, &ixunpack);
			
			disktomemlong (v.data.datevalue);
			break;
		
		case singlevaluetype:
			fl = langunpackdata (sizeof (v.data.singlevalue), &v.data.singlevalue, h, &ixunpack);
			// ??? need swapping ???
			
			break;
		
		case oldstringvaluetype: 
			fl = langunpackstring ((hdlstring *) &v.data.stringvalue, h, &ixunpack);
			
			if (fl)
				pullfromhandle (v.data.stringvalue, 0, 1, nil); /*shed length byte*/
			
			break;
		
		case addressvaluetype:
			fl = langunpackhandle (false, &v.data.stringvalue, h, &ixunpack);
			
			if (!fl)
				break;
			
			flpush = (currenthashtable == nil);
			
			if (flpush)
				flpush = pushhashtable (roottable);
			
			if (stringtoaddress (&v))
				exemptfromtmpstack (&v);
			else
				v.valuetype = stringvaluetype;
			
			if (flpush)
				pophashtable ();
			
			break;
		
		case rectvaluetype: {
			Rect r;
			diskrect rdisk;
			 
			fl = langunpackdata (sizeof (rdisk), &rdisk, h, &ixunpack);
			
			if (fl) {
				
				diskrecttorect (&rdisk, &r);
				
				fl = newheapvalue (&r, sizeof (r), rectvaluetype, &v);
				
				if (fl)
					exemptfromtmpstack (&v);
				}
			
			break;
			}
		
		case rgbvaluetype: { /* 2006-04-20 sethdill & aradke */
			RGBColor rgb;
			diskrgb rgbdisk;
			 
			fl = langunpackdata (sizeof (rgbdisk), &rgbdisk, h, &ixunpack);
			
			if (fl) {
				
				diskrgbtorgb (&rgbdisk, &rgb);
				
				fl = newheapvalue (&rgb, sizeof (rgb), rgbvaluetype, &v);
				
				if (fl)
					exemptfromtmpstack (&v);
				}
			
			break;
			}
		
		#if noextended
		
			case doublevaluetype: {
				long double x;
				extended80 x80;
				 
				fl = langunpackdata (sizeof (x80), &x80, h, &ixunpack);
				
				if (fl) {
					
						safex80told (&x80, &x);
					 
					fl = setdoublevalue (x, &v);
					
					if (fl)
						exemptfromtmpstack (&v);
					}
				
				break;
				}
		#else
		
			case doublevaluetype:
			
		#endif
		
		case stringvaluetype:
		case passwordvaluetype: 
		case patternvaluetype:
		case binaryvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
			fl = langunpackhandle (false, &v.data.binaryvalue, h, &ixunpack);
			
			break;
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
			fl = langunpackhandle (true, &hdata, h, &ixunpack);
			
			if (!fl)
				break;
			
			fl = opunpacklist (hdata, &v.data.listvalue);
			
			break;
	#endif

		case objspecvaluetype: {
			Handle hobjspec;
			
			fl = langunpackhandle (false, &hobjspec, h, &ixunpack);
			
			if (!fl)
				break;
			
			if (gethandlesize (hobjspec) == 0) {
				
				disposehandle (hobjspec);
				
				hobjspec = nil;
				}
			
			v.data.objspecvalue = hobjspec;
			
			break;
			}
		
		
		case filespecvaluetype:
		case aliasvaluetype:
			fl = langunpackhandle (true, &hdata, h, &ixunpack);
			
			if (!fl)
				break;
			
			fl = langunpackfileval (hdata, &v);
			
			break;

		case codevaluetype:
			fl = langunpackhandle (true, &hdata, h, &ixunpack);
			
			if (!fl)
				break;
			
			fl = langunpacktree (hdata, &v.data.codevalue);
			
			break;
		
		case externalvaluetype:
			fl = langunpackexternal ((hdlexternalvariable *) &v.data.externalvalue, h, &ixunpack);
			
			break;
		
		default:
			langerror (cantunpackerror);
			
			return (false);		
		} /*switch*/
	
	*val = v;
	
	return (fl);
	
	formaterror:
	
	langerror (unpackformaterror);
	
	return (false);
	} /*langunpackvalue*/


boolean langunpackverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	tyvaluerecord val;
	hdlhashtable htable;
	bigstring bsvarname;
	Handle hpacked;
	
	setbooleanvalue (false, vreturned); /*default returned value of verb*/
	
	if (!getbinaryvalue (hparam1, 1, true, (Handle *) &hpacked)) /*the value to be unpacked*/
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bsvarname)) /*the place to put the unpacked value*/
		return (false);
	
	if (!langunpackvalue ((Handle) hpacked, &val))
		return (false);
	
	if (!langsetsymboltableval (htable, bsvarname, val)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	(*vreturned).data.flvalue = true;
	
	return (true);
	} /*langunpackverb*/


boolean langunpackwindowverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	//
	// 2006-09-18 creedon: FSRef-ized
	//

	tyvaluerecord val;
	tyfilespec fspec;
	ptrfilespec fs;
	Handle hpacked;
	
	setbooleanvalue (false, vreturned); /*default returned value of verb*/
	
	if (!getbinaryvalue (hparam1, 1, true, (Handle *) &hpacked)) /*the value to be unpacked*/
		return (false);
	
	flnextparamislast = true;
	
	if (!getfilespecvalue (hparam1, 2, &fspec)) /*the file the value came from*/
		return (false);
	
	if (!langunpackvalue ((Handle) hpacked, &val))
		return (false);
	
	if (val.valuetype != externalvaluetype) {
		
		langerror (cantunpackthisexternalerror);
		
		return (false);
		}
	
	fs = &fspec;
	

		if (macfilespecisvalid(fs))
			langexternalsetdirty ((hdlexternalhandle) val.data.externalvalue, false);
		else
			fs = nil;
		
	

	if (!langexternalzoomfilewindow (&val, fs, true)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	(*vreturned).data.flvalue = true;
	
	return (true);
	
	} // langunpackwindowverb



boolean langvaluetotextscrap (tyvaluerecord val, Handle htext) {
	
	/*
	convert the given value to text, appending to the indicated handle
	*/
	
	bigstring bstype, bsvalue, bssize;
	
	if (!hashvaltostrings (val, bstype, bsvalue, bssize))
		return (false);
	
	pushchar (':', bstype);
	
	if (val.valuetype == externalvaluetype) {
		pushchar (chreturn, bstype);

		#ifdef xxxWIN95VERSION
			pushchar (chlinefeed, bstype);
		#endif
		}
	else {
		pushchar (chtab, bstype);
		}

	if (!pushtexthandle (bstype, htext))
		return (false);
	
	if (val.valuetype == externalvaluetype)
		return (langexternalpacktotext ((hdlexternalvariable) val.data.externalvalue, htext));
	
	pushchar (chreturn, bsvalue);

	#ifdef xxxWIN95VERSION
		pushchar (chlinefeed, bsvalue);
	#endif
	
	return (pushtexthandle (bsvalue, htext));
	} /*langvaluetotextscrap*/




