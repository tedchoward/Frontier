
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

#include "landinternal.h"



ATNBPRecHandle hNBPParms; 

Handle hnamebuf; 


pascal boolean landfindfirstnetname (bsname, bstype, bszone) bigstring bsname, bstype, bszone; {

	/* 
	requires name, type, and zone of network objects (accepts appropriate wildcards)
	returns the first matching object in those same strings.  If no match, the strings
	are nil and the function returns FALSE
	*/
	
	EntityName searchname;
	AddrBlock address;
	Boolean flgotnames;
	
	if ((stringlength (bsname) > 31) || (stringlength (bstype) > 31) || (stringlength (bszone) > 31)) 
		return (false);	
		
	/* our name limit is 254 */

	hnamebuf  = NewHandle ((long)(254 * sizeof(EntityName)));
	hNBPParms = (ATNBPRecHandle)NewHandle ((long)sizeof (ATNBPRec));
	
	if (hNBPParms == nil || hnamebuf == nil)
		return(false);
	
	/* 
	note: we have our own type for recognition over then network, and we currently
	only work on one zone (the local one)
	*/
	landcopystring(bsname, searchname.objStr);
	landcopystring("\pUserLand IAC Net Protocol", searchname.typeStr);
	searchname.zoneStr[0] = 1;
	searchname.zoneStr[1] = '*';
	
	/* Do the name binding protocol call. */

	(*hNBPParms)->nbpEntityPtr = &searchname;
	MoveHHi (hnamebuf);
	HLock (hnamebuf);
	(*hNBPParms)->nbpBufPtr = *hnamebuf;
	(*hNBPParms)->nbpBufSize = (long)(254 * sizeof(EntityName));
	(*hNBPParms)->nbpDataField = 254;
	(*hNBPParms)->nbpRetransmitInfo.retransInterval = 4;  /* Half a second. */
	(*hNBPParms)->nbpRetransmitInfo.retransCount = 2;

	/* 
	Get the names, check for error. If we had an error, or if we didn't get any
	names, release memory and return false
	*/
	if (NBPLookup (hNBPParms, false) != noErr || (*hNBPParms)->nbpDataField == 0)
	{
		flgotnames = false;
		bsname[0] = bstype[0] = bszone[0] = 0;
		
	}	
	else
	{
		NBPExtract (*hnamebuf, (*hNBPParms)->nbpDataField, 1, &searchname, &address);
		landcopystring(searchname.objStr, bsname);
		flgotnames = true;
	} 

	/* unlock the name buffer no matter what happened */
	HUnlock(hnamebuf);
	
	/*
	if we didn't get any names, or we only got one, then trash everything: this will
	insure that landfindnextnetname will return appropriately
	*/
	if (flgotnames == false || (*hNBPParms)->nbpDataField == 1)
	{
		DisposHandle((Handle)hnamebuf);
		DisposHandle((Handle)hNBPParms);
		hnamebuf = nil;
		hNBPParms = nil;
	}
	
	return(flgotnames);
	
} 	


pascal boolean landfindnextnetname(bsname, bstype, bszone)
bigstring bsname;
bigstring bstype; 
bigstring bszone;
{

	/*
	called successively after the landfindfirstname call. The strings are filled in
	with the appropriate data for the matching network object. If there are no more
	matches, the strings are nil and the function returns FALSE.
	*/
	
	static int ixname = 1;	/* the last name we returned */
	AddrBlock address;		/* dummy address block */	
	EntityName searchname;	/* the name struct our stuff gets returned in */
	
	if (hnamebuf == nil || hNBPParms == nil)
	{
		bsname[0] = bstype[0] = bszone[0] = 0;
		return(false);
	}
	
	/* if we got all the names there are to get, then clean up and return */
	if (ixname > (*hNBPParms)->nbpDataField)
	{
		bsname[0] = bstype[0] = bszone[0] = 0;
		DisposHandle((Handle)hnamebuf);
		DisposHandle((Handle)hNBPParms);
		hnamebuf = nil;
		hNBPParms = nil;
		return(false);
	}

	/* 
	get the next name out of our buffer. note: don't copy the other strings for
	now, since they have no real meaning
	*/
	NBPExtract (*hnamebuf, (*hNBPParms)->nbpDataField, ixname, &searchname, &address);
	landcopystring(searchname.objStr, bsname);
	
	/* update our index of returned names */
	ixname++;
	
	return(true);

}



static boolean landvisitmachineports (LocationNameRec *ln, landcallback visit) {
	
	IPCListPortsPBRec pb;
	PortInfoRec buf;
	PPCPortRec portname;
	bigstring bs;
	OSErr errcode;
		
 	setstringlength (bs, 1);
	
	bs [1] = '=';
	
	pb.startIndex = 0;
		
	while (true) {
	
		portname.portKindSelector = ppcNBPLocation; 
	
		landcopystring (bs, portname.name);
	
		landcopystring (bs, portname.u.portTypeStr);
		
		pb.portName = &portname;
		
		pb.requestCount = 1;
		
		pb.bufferPtr = (PortInfoArrayPtr) &buf; 
		
		pb.locationName = ln;
		
		errcode = IPCListPorts (&pb, nil);
		
		if (errcode != noErr)
			return (false);
			
		if (pb.actualCount == 0)
			return (true);
			
		if (!visit (buf.name.name))
			return (false);
		
		pb.startIndex++;
		} /*while*/
	} /*landvisitmachineports*/
  

boolean landvisitport (name) Str32 name; {
	
	return (true);
	} /*landvisitport*/
	
	
landvisitallports (landcallback visit) {
	
	/*
	See Inside Macintosh, volume VI, page 32-37.
	*/
	
	XPPParamBlock pb;
	LocationNameRec ln;
	char zipbuf [578]; 
	OSErr errcode;
	
	landclearbytes (&pb, longsizeof (pb));
	
	pb.XCALL.csCode = xCall;
	
	pb.XCALL.xppSubCode = zipGetZoneList;
	
	pb.XCALL.xppTimeout = 4; /*four seconds*/
	
	pb.XCALL.xppRetry = 4; /*four retries*/
	
	pb.XCALL.zipBuffPtr = (Ptr) &zipbuf;
	
	pb.XCALL.zipInfoField [0] = (char) 0;
	
	pb.XCALL.zipInfoField [1] = (char) 0;
	
	/*
	errcode = GetMyZone (&pb, false);
	*/
	
	/*
 	ln.locationKindSelector = ppcNoLocation;
 	
 	landvisitmachineports (&ln, &landvisitport);
 	*/
	} /*landvisitallports*/
	

landtestlistports (void) {
	
	bigstring bsname, bstype, bszone;
	
	return (true);
	
	Debugger ();
	
	setstringlength (bsname, 0);
	
	setstringlength (bstype, 0);
	
	setstringlength (bszone, 0);
	
	landfindfirstnetname (bsname, bstype, bszone);
	
	landvisitallports (&landvisitport);
	} /*landtestlistports*/
	

        
