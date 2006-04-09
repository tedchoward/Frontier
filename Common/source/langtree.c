
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
#include "lang.h"
#include "langinternal.h"
#include "tablestructure.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */


typedef struct tydisktreenode {

#ifdef MACVERSION
	tytreetype nodetype; /*add, subtract, if, etc.*/
#endif
#ifdef WIN95VERSION
	short nodetype;	/*low byte will coincide with MAC enum*/
#endif

	long nodevalsize;
	
	short lnum; /*which line number in the source was this node generated from?*/
	
	short charnum; /*at what character offset? max is 255*/
	
	short paraminfo;
	
	/*
	byte ctparams; //number of params actually allocated, max is 4
	
	byte hasparam1; //is param1 not nil?
	
	byte hasparam2;
	
	byte hasparam3;
	
	byte hasparam4;
	
	byte haslink; //is there another node linked to this one?
	*/
	} tydisktreenode, *ptrdisktreenode, **hdldisktreenode;

typedef enum tydisktreenodeparaminfo {

#ifdef MACVERSION
	ctparams_mask = 0xf000,
	ctparams_1 = 0x1000,
	hasparam1_mask = 0x0800,
	hasparam2_mask = 0x0400,
	hasparam3_mask = 0x0200,
	hasparam4_mask = 0x0100,
	haslink_mask = 0x0080
#endif

#ifdef WIN95VERSION
	ctparams_mask = 0x00f0,
	ctparams_1 = 0x0010,
	hasparam1_mask = 0x0008,
	hasparam2_mask = 0x0004,
	hasparam3_mask = 0x0002,
	hasparam4_mask = 0x0001,
	haslink_mask = 0x8000
#endif
	} tydisktreenodeparaminfo;


typedef struct tydisktreerec {
	
	short version;
	
	long ctnodes;
	
	long flags; /*currently unused*/
	
	byte waste [8];
	
	tydisktreenode nodes [];
	} tydisktreerec, *ptrdisktreerec, **hdldisktreerec;


typedef struct tyOLD42disktreenode {

	#ifdef MACVERSION
	tytreetype nodetype; /*add, subtract, if, etc.*/
	#endif
	#ifdef WIN95VERSION
	short nodetype;
	#endif

	long nodevalsize;
	
	short lnum; /*which line number in the source was this node generated from?*/
	
	short charnum; /*at what character offset? max is 255*/
	
	byte ctparams : 4; /*number of params actually allocated, max is 4*/
	
	byte hasparam1 : 1; /*is param1 not nil?*/
	
	byte hasparam2 : 1;
	
	byte hasparam3 : 1;
	
	byte hasparam4 : 1;
	
	byte haslink : 1; /*is there another node linked to this one?*/
	
	byte unused : 7; /*a little room for expansion*/
	} tyOLD42disktreenode, *ptrOLD42disktreenode, **hdlOLD42disktreenode;


#ifdef fldebug

	static long cttreenodes = 0;

#endif



#define treenodeallocator true

#define treenodeblocksize 1024


typedef struct tytreenodeblock {
	
	ptrtreenode pnode; // points to next field, simulating handle
	
	tytreenode node;
	} tytreenodeblock, *ptrtreenodeblock;


#ifdef WIN95VERSION
	
static CRITICAL_SECTION treenodesection;

static boolean treenodesectioninitialized = false;

static void _entercriticaltreenodesection (void) {

	if (!treenodesectioninitialized) {

		InitializeCriticalSection (&treenodesection);

		treenodesectioninitialized = true;
		}
	
	EnterCriticalSection (&treenodesection);
	}

static void _leavecriticaltreenodesection (void) {

	LeaveCriticalSection (&treenodesection);
	}

#else

#define _entercriticaltreenodesection()

#define _leavecriticaltreenodesection()

#endif


static hdltreenode hfirstfreetreenode = nil;


static boolean alloctreenodeblock (hdltreenode *hfirstfreenode) {
	
	ptrtreenodeblock pblock, p, pend;
	hdltreenode hprev;
	
	pblock = (ptrtreenodeblock) malloc (sizeof (tytreenodeblock) * treenodeblocksize);
	
	if (pblock == nil) {
		
		memoryerror ();
		
		return (false);
		}
	
	clearbytes (pblock, sizeof (tytreenodeblock) * treenodeblocksize);
	
	for (hprev = nil, p = pblock, pend = p + treenodeblocksize; p < pend; ++p) {
		
		(*p).node.link = hprev; // free list is reverse order within block for easy coding
		
		(*p).pnode = &(*p).node; // pointer to this is a handle to real tree node
		
		hprev = &(*p).pnode;
		}
	
	*hfirstfreenode = hprev;
	
	return (true);
	} /*alloctreenodeblock*/


static boolean alloctreenode (hdltreenode *hnode) {

	hdltreenode h;
	
	_entercriticaltreenodesection();
	
	if (hfirstfreetreenode == nil)
		if (!alloctreenodeblock (&hfirstfreetreenode)) {
		
			_leavecriticaltreenodesection();
			
			return (false);
			}
	
	h = hfirstfreetreenode;
	
	hfirstfreetreenode = (**h).link; // remove from free list
	
	(**h).link = nil;	// zero non-zero fields
	
	*hnode = h;
	
	_leavecriticaltreenodesection();
	
	return (true);
	} /*alloctreenode*/


static void freetreenode (hdltreenode hnode) {

	clearbytes (*hnode, sizeof (tytreenode));
	
	_entercriticaltreenodesection();

	(**hnode).link = hfirstfreetreenode;
	
	hfirstfreetreenode = hnode;
	
	_leavecriticaltreenodesection();
	} /*freetreenode*/


boolean langvisitcodetree (hdltreenode htree, langtreevisitcallback visit, ptrvoid refcon) {
	
	/*
	10/23/91 dmb: visit the entire code tree, links and children last
	
	we only all the visit routine for non-nil nodes
	
	for now, we ignore visit's return value, visit everything, and return true.
	
	4/7/93 dmb: change order of visiting, and now pay attention to callback's
	return value. use a while loop instead of linear recursion for the links.
	*/
	
	register hdltreenode h;
	register short ctparams;
	
	for (h = htree; h != nil; h = (**h).link) {
		
		if (!(*visit) (h, refcon))
			return (false);
		
		ctparams = (**h).ctparams;
		
		if (ctparams == 0)
			continue;
		
		if (!langvisitcodetree ((**h).param1, visit, refcon))
			return (false);
		
		if (ctparams == 1)
			continue;
		
		if (!langvisitcodetree ((**h).param2, visit, refcon))
			return (false);
		
		if (ctparams == 2)
			continue;
		
		if (!langvisitcodetree ((**h).param3, visit, refcon))
			return (false);
		
		if (ctparams == 3)
			continue;
		
		if (!langvisitcodetree ((**h).param4, visit, refcon))
			return (false);
		}
	
	return (true);
	} /*langvisitcodetree*/


static boolean counttreenodevisit (hdltreenode hnode, ptrvoid refcon) {
#pragma unused (hnode)

	short *ctnodes = (short *) refcon;
	
	++*ctnodes;
	
	return (true);
	} /*counttreenodevisit*/


short langcounttreenodes (hdltreenode hnode) {
	
	short cttreenodesvisited = 0;
	
	langvisitcodetree (hnode, &counttreenodevisit, &cttreenodesvisited);
	
	return (cttreenodesvisited);
	} /*langcounttreenodes*/
			

boolean langdisposetree (hdltreenode htree) {

	/*
	5.7.97 dmb: protect herrornode global
	*/

	register hdltreenode h = htree;
	register short ctparams;
	
	if (h == nil)
		return (true);

	if (h == herrornode)
		herrornode = NULL;
	
	#ifdef fldebug
	
	--cttreenodes;
	
	#endif
	
	langdisposetree ((**h).link);
	
	ctparams = (**h).ctparams;
	
	if (ctparams == 0)
		goto exit;
		
	langdisposetree ((**h).param1);
	
	if (ctparams == 1)
		goto exit;
		
	langdisposetree ((**h).param2);
	
	if (ctparams == 2)
		goto exit;
		
	langdisposetree ((**h).param3);
	
	if (ctparams == 3)
		goto exit;
		
	langdisposetree ((**h).param4);
	
	exit:
	
	disposevaluerecord ((**h).nodeval, false);
	
	#ifndef treenodeallocator
		disposehandle ((Handle) h);
	#else
		freetreenode (h);
	#endif
	
	return (true);
	} /*langdisposetree*/


static boolean equaltrees (hdltreenode h1, hdltreenode h2) {
	
	short ctparams;
	tyvaluetype valuetype;
	
	if ((h1 == nil) && (h2 == nil))
		return (true);

	if ((h1 == nil) || (h2 == nil))
		return (false);

	ctparams = (**h1).ctparams;
	
	if (ctparams != (**h2).ctparams)
		return (false);
	
	if ((**h1).nodetype != (**h2).nodetype)
		return (false);
	
	valuetype = (**h1).nodeval.valuetype;

	if (valuetype != (**h2).nodeval.valuetype)
		return (false);
	
	if (valuetype != novaluetype) {
		
		if (langheaptype (valuetype)) {
			
			if (!equalhandles ((**h1).nodeval.data.stringvalue, (**h2).nodeval.data.stringvalue))
				return (false);
			}
		else {
			return (false); // we're not concerned with scalar value nodes
			}
		}
	
	if (!equaltrees ((**h1).link, (**h2).link))
		return (false);
	
	switch (ctparams) {
		
		case 4:
			if (!equaltrees ((**h1).param4, (**h2).param4))
				return (false);
			// cascade

		case 3:
			if (!equaltrees ((**h1).param3, (**h2).param3))
				return (false);
			// cascade

		case 2:
			if (!equaltrees ((**h1).param2, (**h2).param2))
				return (false);

		case 1:
			if (!equaltrees ((**h1).param1, (**h2).param1))
				return (false);

		case 0:
		default:
			return (true);
		}
	} /*equaltrees*/


static boolean langnewtreenode (hdltreenode *hnode, short ctparams) {
	
	/*
	allocate a new tree node.
	
	8/29/90 DW: added ctparams to optimize node size.  we only allocate room for
	the indicated number of parameter handles.  the four slots must be declared
	at the end of the treenode struct.
	*/
	
	register hdltreenode h;
	//register long ctbytes;
	
	#ifdef fldebug
	
	++cttreenodes;
	
	#endif
	
	#ifndef treenodeallocator
		ctbytes = longsizeof (tytreenode) - ((4 - ctparams) * longsizeof (Handle));
		
		if (!newclearhandle (ctbytes, (Handle *) hnode)) {
			
			*hnode = nil;
			
			return (false);
			}
	#else
		if (!alloctreenode (hnode)) {
		
			*hnode = nil;
			
			return (false);
			}
	#endif
	
	h = *hnode; /*copy into register*/
	
	(**h).lnum = ctscanlines;
	
	(**h).charnum = ctscanchars;
	
	(**h).ctparams = ctparams;
	
	return (true);
	} /*langnewtreenode*/


static boolean pushvalue (tytreetype op, tyvaluerecord val, hdltreenode *hreturned) {
	
	/*
	12/26/91 dmb: dispose value when newtreenode fails
	*/
	
	register hdltreenode h;
	
	if (!langnewtreenode (hreturned, 0)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	h = *hreturned; /*copy into register*/
	
	(**h).nodetype = op;
	
	(**h).nodeval = val;
	
	return (true);
	} /*pushvalue*/
	

boolean newconstnode (tyvaluerecord val, hdltreenode *hreturned) {
	
	return (pushvalue (constop, val, hreturned));
	} /*newconstnode*/
	

boolean newidnode (tyvaluerecord val, hdltreenode *hreturned) {
	
	return (pushvalue (identifierop, val, hreturned));
	} /*newidnode*/
	

boolean pushquadruplet (tytreetype op, hdltreenode hp1, hdltreenode hp2, hdltreenode hp3, hdltreenode hp4, hdltreenode *hreturned) {
	
	register hdltreenode h;
	
	if (!langnewtreenode (hreturned, 4)) {
		
		langdisposetree (hp1); /*we must consume it*/
		
		langdisposetree (hp2);
		
		langdisposetree (hp3);
		
		langdisposetree (hp4);
		
		return (false);
		}
		
	h = *hreturned; /*copy into register*/
	
	(**h).nodetype = op;
	
	(**h).param1 = hp1;
	
	(**h).param2 = hp2;
	
	(**h).param3 = hp3;
	
	(**h).param4 = hp4;
	
	return (true);
	} /*pushquadruplet*/
	
	
boolean pushtriplet (tytreetype op, hdltreenode hp1, hdltreenode hp2, hdltreenode hp3, hdltreenode *hreturned) {
	
	register hdltreenode h;
	
	if (!langnewtreenode (hreturned, 3)) {
		
		langdisposetree (hp1); /*we must consume it*/
		
		langdisposetree (hp2);
		
		langdisposetree (hp3);
		
		return (false);
		}
		
	h = *hreturned; /*copy into register*/
	
	(**h).nodetype = op;
	
	(**h).param1 = hp1;
	
	(**h).param2 = hp2;
	
	(**h).param3 = hp3;
	
	return (true);
	} /*pushtriplet*/
	
	
boolean pushbinaryoperation (tytreetype op, hdltreenode hp1, hdltreenode hp2, hdltreenode *hreturned) {
	
	/*
	5.0a19 dmb: first cut at compiler-time assignment optimizations:

	1: x = x + y  =>  x += y
	
	later:
	2: x = x - y  =>  x -= y
	3: x = string.delete (x, y, z) => string.xdelete (@x, y, z)
	4: x = string.mid (x, y, z)    => string.xmid (@x, y, z)
	5: x = string.popleading (x, y) => string.xpopleading (@x, y)
	6. x = string.replace (x, y, z) => string.xreplace[all] (@x, y, z)
	7. x = string.upp[low]er (x)    => string.xupp[low]er (@x)
	*/

	register hdltreenode h;
	
	if (!langnewtreenode (hreturned, 2)) {
		
		langdisposetree (hp1); /*we must consume it*/
		
		langdisposetree (hp2);
		
		return (false);
		}
	
	#ifdef version5orgreater
	
	if (op == assignop) { // look for assignment optimizations when rhs includes lhs
		
		h = hp2;

		switch ((**h).nodetype) {
		
			case addop:
				if (equaltrees (hp1, (**h).param1)) { // turn + into +=

					op = addvalueop;
					
					hp2 = (**h).param2;
					
					(**h).param2 = nil;

					langdisposetree (h);
					}
				
				break;
			
			case subtractop:
				if (equaltrees (hp1, (**h).param1)) { // turn - into -=

					op = subtractvalueop;
					
					hp2 = (**h).param2;
					
					(**h).param2 = nil;

					langdisposetree (h);
					}
				
				break;
			
			default:
				/* do nothing */
				break;
			}
		}

	#endif

	h = *hreturned; /*copy into register*/
	
	(**h).nodetype = op;
	
	(**h).param1 = hp1;
	
	(**h).param2 = hp2;
	
	return (true);
	} /*pushbinaryoperation*/
	
	
boolean pushunaryoperation (tytreetype op, hdltreenode hp1, hdltreenode *hreturned) {
	
	register hdltreenode h;
	
	if (!langnewtreenode (hreturned, 1)) {
		
		langdisposetree (hp1); /*we must consume it*/
		
		return (false);
		}
	
	h = *hreturned; /*copy into register*/
	
	(**h).nodetype = op;
	
	(**h).param1 = hp1;
	
	return (true);
	} /*pushunaryoperation*/


boolean pushoperation (tytreetype op, hdltreenode *hreturned) {
	
	if (!langnewtreenode (hreturned, 0))
		return (false);
	
	(**(*hreturned)).nodetype = op;
	
	return (true);
	} /*pushoperation*/


boolean pushloop (hdltreenode hp1, hdltreenode hp2, hdltreenode hp3, hdltreenode *hreturned) {
	
	return (pushquadruplet (loopop, hp1, hp2, hp3, nil, hreturned));
	} /*pushloop*/


boolean pushloopbody (hdltreenode hp4, hdltreenode hlist) {
	
	register hdltreenode h = hlist;
	
	(**h).param4 = hp4;
	
	return (true);
	} /*pushloopbody*/


boolean pushunarystatementlist (hdltreenode hp1, hdltreenode hlist) {
	
	register hdltreenode h = hlist;
	
	(**h).param1 = hp1;
	
	return (true);
	} /*pushunarystatementlist*/


boolean pushtripletstatementlists (hdltreenode hp2, hdltreenode hp3, hdltreenode hlist) {
	
	/*
	add the statement lists to the previously-created triplet.
	
	the triplet was created earlier so that the line/character position 
	information could be meaningful.
	
	since the triplet was created with unassigned parameters set to 
	nil, we only set non-nil parameters.  this is more than an optimization; 
	it adds flexibility so that for some callers (like fileloop), one 
	of these parameters can already be set
	*/
	
	register hdltreenode h = hlist;
	
	if (hp2)
		(**h).param2 = hp2;
	
	if (hp3)
		(**h).param3 = hp3;
	
	return (true);
	} /*pushtripletstatementlists*/


boolean pushfunctioncall (hdltreenode hp1, hdltreenode hp2, hdltreenode *hreturned) {
	
	/*
	hp1 represents the name of the function call.  hp2 represents the parameter
	list (it can be nil, indicating a function taking no parameters).
	
	before pushing the name onto the code tree, first see if it's a built-in
	function.  if so, substitute the tokennumber for the built-in.
	
	note: since we always call pushbinaryoperation, we consume both parameters.
	
	7/8/90 DW: only do the optimization if hp1 is a simple, undotted string.
	this has been a bug lurking in here for a long, long time!  it took the 
	whole day of splitting the baby to find this one.  thank you.
	
	7/8/90 DW: wishful thinking.  this really was a problem, but the nasty
	behavior continues...
	
	7/10/90 DW: turns out we were disposing of an already-disposed-of handle
	in scripts.c.
	*/
	
	register hdltreenode h = hp1; /*we do a lot of work on this guy*/
	register Handle hstring;
	register boolean fl;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if ((**h).nodetype != identifierop) /*more complex than an ID, skip optimization*/
		goto L1;
	
	assert ((**h).nodeval.valuetype == stringvaluetype);
	
	/*
	if ((**h).nodeval.valuetype != stringvaluetype) /%skip optimization%/
		goto L1;
	*/
	
	hstring = (**h).nodeval.data.stringvalue;
	
	texthandletostring (hstring, bs); /*get the identifier name*/
	
	fl = hashtablelookup (hbuiltinfunctions, bs, &val, &hnode); /*check the built-in functions*/
	
	if (fl && (val.valuetype == tokenvaluetype)) { /*a pre-defined built-in*/
		
		disposehandle ((Handle) hstring); /*don't need the string*/
		
		(**h).nodeval = val; /*replace with token value*/
		}
	
	L1:
	
	return (pushbinaryoperation (functionop, h, hp2, hreturned));
	} /*pushfunctioncall*/


boolean pushkernelcall (hdltreenode hp1, hdltreenode *hreturned) {
	
	/*
	7/18/91 dmb: resolve the efp reference immediately, both for 
	performance and to catch errors at compile time.  we stick the 
	resolved token in a constant node that becomes the one parameter 
	in the kernel code tree.
	
	8/21/91 dmb: langvalue now supports calling kernel code directly, with 
	kernelfunctionvalue, which doesn't work off the the code tree.  so there's 
	no point in stashing sway  the token value, which was kind of a waste of 
	memory anyway. 
	
	5.7.97 dmb: protect herrornode global
	*/
	
	register hdltreenode h = hp1;
	register boolean fl;
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	boolean flerrornodewasset;
	hdlhashnode hnode;
	
	assert ((**h).nodetype == dotop);
	
	pushhashtable (efptable);
	
	fl = langgetdotparams (h, &htable, bs);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	fl = (**htable).valueroutine != nil;
	
	if (fl)
		fl = hashtablelookup (htable, bs, &val, &hnode);
	
	if (!fl || (val.valuetype != tokenvaluetype)) { /*didn't get the token value*/
		
		langparamerror (notefperror, bs);
		
		return (false);
		}
	
	if (!setaddressvalue (htable, bs, &val))
		return (false);
	
	exemptfromtmpstack (&val); /*pushvalue will consume it on failure*/
	
	if (!pushvalue (kernelop, val, hreturned))
		return (false);
	
	/*
	if (!newconstnode (val, &htokenparam))
		return (false);
	
	if (!pushunaryoperation (kernelop, htokenparam, hreturned))
		return (false);
	
	if (!setaddressvalue (htable, bs, &val))
		return (false);
	
	(***hreturned).nodeval = val;
	*/
	
	flerrornodewasset = herrornode != NULL;
	
	langdisposetree (h); /*don't need the tree anymore*/

	if (flerrornodewasset  && (herrornode == NULL)) // it was pointing to our old code
		herrornode = *hreturned;
	
	return (true);
	} /*pushkernelcall*/


boolean pushfunctionreference (tyvaluerecord val, hdltreenode *htree) {
	
	/*
	code factored from dialog callbacks and ipc handlers
	
	val should be an address value, the address of the function to be called.
	
	we build a code tree for the function reference -- just the address 
	dereferenced.  this corresponds to hp1 in pushfunctioncall
	*/
	
	hdltreenode haddress;
	
	exemptfromtmpstack (&val); /*will be disposed along w/code tree*/
	
	if (!newconstnode (val, &haddress))
		return (false);
	
	if (!pushunaryoperation (dereferenceop, haddress, htree))
		return (false);
	
	return (true);
	} /*pushfunctionreference*/


boolean pushlastlink (hdltreenode hnewlast, hdltreenode hlist) {
	
	/*
	hnewlast is a newly parsed object, for example a statement in a statement
	list or a parameter expression in a parameter list.
	
	tack it on at the end of the list headed up by hlist.  we travel from
	hlist until we hit nil, then backup and insert hnewlast as the last node
	in the list.
	
	the link field of the treenode is used to connect the list.
	
	12/9/91 dmb: don't treat nil newlast as an error
	*/
	
	register hdltreenode nomad = hlist;
	
	if ((hnewlast == nil) || (nomad == nil))
		return (true);
	
	while (true) { /*find the end of the list*/
		
		register hdltreenode nextnomad = (**nomad).link;
		
		if (nextnomad == nil) { /*found it*/
			
			(**nomad).link = hnewlast; /*link in at the end of the list*/
			
			assert ((**hnewlast).link == nil || (**hnewlast).nodetype == fieldop);
			
			return (true);
			}
		
		nomad = nextnomad; 
		} /*while*/
	} /*pushlastlink*/


#if 0

boolean pushlastoptionallink (hdltreenode hnewlast, hdltreenode hlist, hdltreenode *hresult) {
	
	tyvaluerecord val;
	
	initvalue (&val, novaluetype);
	
	if (hnewlast == nil) {
		
		if (!newconstnode (val, &hnewlast))
			return (false);
		}
	
	if (hlist == nil) {
		
		if (!newconstnode (val, &hlist))
			return (false);
		}
	
	pushlastlink (hnewlast, hlist);
	
	*hresult = hlist;
	
	return (true);
	} /*pushlastoptionallink*/

#endif


typedef struct packtreeinfo {
	
	hdldisktreerec hdisktree;
	
	Handle htreenodevalues;
	
	long ctvisited;
	} typacktreeinfo, *ptrpacktreeinfo;


static boolean langpacktreevisit (hdltreenode htree, ptrvoid refcon) {
	
	/*
	4/8/93 dmb: pack a code tree. be careful not to retain value of a modulop 
	node, which is really a handle will not persist.
	*/
	
	register hdltreenode hn = htree;
	ptrpacktreeinfo lpi = (ptrpacktreeinfo) refcon;
	register ptrdisktreenode pn;
	tyvaluerecord val = (**hn).nodeval;
	Handle hpackedval = nil;
	short ctparams;
	short paraminfo;
	
	/*first pack value, which can move memory*/
	
	if ((val.valuetype != novaluetype) && ((**hn).nodetype != moduleop)) {
		
		if (!langpackvalue ((**hn).nodeval, &hpackedval, HNoNode))
			return (false);
		
		if (!pushhandle (hpackedval, (*lpi).htreenodevalues)) {
			
			disposehandle (hpackedval);
			
			return (false);
			}
		}
	
	pn = &(**(*lpi).hdisktree).nodes [(*lpi).ctvisited++];
	
	(*pn).nodetype = (**hn).nodetype;
	
	(*pn).nodevalsize = gethandlesize (hpackedval);
	
	(*pn).lnum = (**hn).lnum;
	
	(*pn).charnum = (**hn).charnum;
	
	ctparams = (**hn).ctparams;
	
	paraminfo = ctparams * ctparams_1;
	
	if ((ctparams > 0) && ((**hn).param1 != nil))
		paraminfo |= hasparam1_mask;
	
	if ((ctparams > 1) && ((**hn).param2 != nil))
		paraminfo |= hasparam2_mask;
	
	if ((ctparams > 2) && ((**hn).param3 != nil))
		paraminfo |= hasparam3_mask;
	
	if ((ctparams > 3) && ((**hn).param4 != nil))
		paraminfo |= hasparam4_mask;
	
	if ((**hn).link != nil)
		paraminfo |= haslink_mask;
	
	(*pn).paraminfo = paraminfo;
	
	memtodisklong ((*pn).nodevalsize);
	memtodiskshort ((*pn).lnum);
	memtodiskshort ((*pn).charnum);

	disposehandle (hpackedval);
	
	return (true);
	} /*langpacktreevisit*/


boolean langpacktree (hdltreenode htree, Handle *hpacked) {
	
	long ctnodes;
	typacktreeinfo info;
	
	ctnodes = langcounttreenodes (htree);
	
	if (!newclearhandle (sizeof (tydisktreerec) + ctnodes * sizeof (tydisktreenode), (Handle *) &info.hdisktree))
		return (false);
	
	if (!newemptyhandle (&info.htreenodevalues)) {
		
		disposehandle ((Handle) info.hdisktree);
		
		return (false);
		}
	
	(**info.hdisktree).ctnodes = ctnodes;
	
	info.ctvisited = 0;
	
	if (!langvisitcodetree (htree, &langpacktreevisit, &info)) {
		
		disposehandle ((Handle) info.hdisktree);
		
		disposehandle (info.htreenodevalues);
		
		return (false);
		}
	
	memtodiskshort ((**info.hdisktree).version);	/*I can not find where this is set*/
	memtodisklong ((**info.hdisktree).ctnodes);
	memtodisklong ((**info.hdisktree).flags);
	
	return (mergehandles ((Handle) info.hdisktree, info.htreenodevalues, hpacked));
} /*langpacktree*/


static boolean
langunpacktreenode (
		tydisktreenode	*rec,
		hdltreenode		*hnode,
		ptrpacktreeinfo	 ppi)
{
	/*
	unpack a single tree node.
	*/

	register ptrdisktreenode pn = rec;
	register hdltreenode hn;
	Handle hpackedval = nil;
	tyvaluerecord val;
	short ctparams;
	
	ctparams = ((*pn).paraminfo & ctparams_mask) / ctparams_1;
	
	if (!langnewtreenode (hnode, ctparams))
		return (false);
	
	hn = *hnode;
	
	if ((*pn).nodevalsize > 0) {
		
		if (!newhandle ((*pn).nodevalsize, &hpackedval))
			goto error;
		
		if (!pullfromhandle ((*ppi).htreenodevalues, 0, (*pn).nodevalsize, *hpackedval)) /*does _not_ move memory*/
			goto error;
		
		if (!langunpackvalue (hpackedval, &val))
			goto error;
		
		disposehandle (hpackedval);
		
		(**hn).nodeval = val;
		}
	
	(**hn).nodetype = (*pn).nodetype;
	
	(**hn).lnum = (*pn).lnum;
	
	(**hn).charnum = (*pn).charnum;

	return (true);

error:
	{
#ifndef treenodeallocator
		disposehandle ((Handle) *hnode);
#else
		freetreenode (*hnode);
#endif
		disposehandle (hpackedval);
		
		return (false);
	}
} /*langunpacktreenode*/


static boolean
langunpacktreevisit (
		hdltreenode		*htree,
		ptrpacktreeinfo	 ppi)
{
	/*
	unpack a code tree recursively
	*/
	
	register hdltreenode hn = nil;
	hdltreenode hfirst = nil;
	tydisktreenode rec;
	hdltreenode hnode;
	
	while (true)
	{
		rec = (**(*ppi).hdisktree).nodes [(*ppi).ctvisited++];
		
		disktomemlong (rec.nodevalsize);
		disktomemshort (rec.lnum);
		disktomemshort (rec.charnum);
		/* remaining fields are byte values */

		assert ((*ppi).ctvisited <= (**(*ppi).hdisktree).ctnodes);
		
		if (!langunpacktreenode (&rec, &hnode, ppi))
			goto error;
		
		if (hn == nil) /*first node created*/
			hfirst = hnode;
		else
			(**hn).link = hnode;
		
		hn = hnode;
		
		if (rec.paraminfo & hasparam1_mask)
		{
			if (!langunpacktreevisit (&hnode, ppi))
				goto error;

			(**hn).param1 = hnode;
		}

		if (rec.paraminfo & hasparam2_mask)
		{
			if (!langunpacktreevisit (&hnode, ppi))
				goto error;

			(**hn).param2 = hnode;
		}

		if (rec.paraminfo & hasparam3_mask)
		{
			if (!langunpacktreevisit (&hnode, ppi))
				goto error;

			(**hn).param3 = hnode;
		}

		if (rec.paraminfo & hasparam4_mask)
		{
			if (!langunpacktreevisit (&hnode, ppi))
				goto error;

			(**hn).param4 = hnode;
		}
		
		if ((rec.paraminfo & haslink_mask) == 0)
			break;
		}
	
	*htree = hfirst;
	
	return (true);
	
	error: {
		
		langdisposetree (hfirst);
		
		return (false);
		}
	} /*langunpacktreevisit*/


boolean langunpacktree (Handle hpacked, hdltreenode *htree) {
	
	/*
	unpack a code tree, consuming the packed handle
	
	2002-11-11 AR: Added asserts to make sure the C compiler chose the
	proper byte alignment for the tydisktreenode and tydisktreerec structs.
	If it did not, we would end up corrupting any database files we saved.
	*/
	
	typacktreeinfo info;
	long ctnodes;
	boolean fl;
	
	assert (sizeof (tydisktreenode) == 12);
	
	assert (sizeof (tydisktreenode) == sizeof (tyOLD42disktreenode));
	
	assert (sizeof (tydisktreerec) == 18);
	
	*htree = nil; /*default return*/
	
	if (!unmergehandles (hpacked, (Handle *) &info.hdisktree, &info.htreenodevalues))
		return (false);
	
	/* platformize the header first */
	disktomemshort ((**info.hdisktree).version);
	disktomemlong((**info.hdisktree).ctnodes);
	disktomemlong((**info.hdisktree).flags);

	info.ctvisited = 0;
	
	ctnodes = (**info.hdisktree).ctnodes;
	
	if (ctnodes == 0)
		fl = true;
	else
		fl = langunpacktreevisit (htree, &info);
	
	assert (info.ctvisited == ctnodes);
	
	disposehandle ((Handle) info.hdisktree);
	
	disposehandle (info.htreenodevalues);
	
	return (fl);
	} /*langunpacktree*/


boolean langcopytree (hdltreenode hin, hdltreenode *hout) {
	
	/*
	Duplicate the code tree without using langpacktree/langunpacktree
	so this verb can be called even when not in control of the lang globals.
	*/
	
	Handle hpacked;
	
	return (langpacktree (hin, &hpacked) && langunpacktree (hpacked, hout));
	
/*
	register short ctparams;
	register hdltreenode h;
	hdltreenode hcopy = nil;
	hdltreenode hprev = nil;
	
	*hout = nil;
		
	for (h = hin; h != nil; hprev = h, h = (**h).link) {
		
		ctparams = (**h).ctparams;

		if (!langnewtreenode (&hcopy, ctparams))
			goto error;
		
		if (hprev != nil)
			(**hprev).link = hcopy;
		
		if (*hout == nil)
			*hout = hcopy;
		
		//copy stuff

		(**hcopy).nodetype = (**h).nodetype;
		
		(**hcopy).lnum = (**h).lnum;
		
		(**hcopy).charnum = (**h).charnum;
			
		//copy valuerecord
		
		copyvaluerecord ((**h).nodeval, &(**hcopy).nodeval);
		
		//copy params
		
		if (ctparams == 0)
			continue;
		
		if (!langcopytree ((**h).param1, &(**hcopy).param1))
			goto error;
		
		if (ctparams == 1)
			continue;
			
		if (!langcopytree ((**h).param2, &(**hcopy).param2))
			goto error;
		
		if (ctparams == 2)
			continue;
		
		if (!langcopytree ((**h).param3, &(**hcopy).param3))
			goto error;
		
		if (ctparams == 3)
			continue;
		
		if (!langcopytree ((**h).param4, &(**hcopy).param4))
			goto error;
		}
	
	return (true);

error:

	langdisposetree (*hout);
	
	return (false);
*/
	}/*langcopytree*/
