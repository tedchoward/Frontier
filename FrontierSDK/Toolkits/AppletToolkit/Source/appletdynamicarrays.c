
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletdynamicarrays.h"




short sizedynamicarray (tydynamicarray *dynam) {
	
	if ((*dynam).hstorage == nil)
		return (0);
		
	return ((short) (GetHandleSize ((*dynam).hstorage) / (*dynam).elementsize));
	} /*sizedynamicarray*/
	
	
boolean indexdynamicarray (short ix, tydynamicarray *dynam, void *pdata) {
	
	Handle h = (*dynam).hstorage;
	short elementsize = (*dynam).elementsize;
	char *p;
	
	if (ix >= sizedynamicarray (dynam)) /*index is 0-based*/
		return (false);
	
	p = ((char *) *(*dynam).hstorage) + (ix * elementsize);
	
	moveleft (p, pdata, elementsize);
	
	return (true);
	} /*indexdynamicarray*/
	
	
void cleardynamicarrayelement (short ix, tydynamicarray *dynam) {

	Handle h = (*dynam).hstorage;
	short elementsize = (*dynam).elementsize;
	char *p;
	
	if (ix >= sizedynamicarray (dynam)) /*index is 0-based*/
		return;
	
	p = ((char *) *(*dynam).hstorage) + (ix * elementsize);
	
	clearbytes (p, elementsize);
	} /*cleardynamicarrayelement*/
	
	
boolean addtodynamicarray (void *pdata, tydynamicarray *dynam) {
	
	Handle h = (*dynam).hstorage;
	short elementsize = (*dynam).elementsize;
	char *p;
	short ct, i;
	
	if (h == nil)
		return (newfilledhandle (pdata, elementsize, &(*dynam).hstorage));
		
	p = (char *) *h;
	
	ct = sizedynamicarray (dynam);
	
	for (i = 0; i < ct; i++) { /*look for an empty slot in the array*/
		
		long x;
		
		moveleft (p, &x, longsizeof (x));
		
		if (x == 0) { /*the first four bytes are 0, it's empty*/
		
			moveleft (pdata, p, elementsize);
			
			return (true);
			}
		
		p += (*dynam).elementsize;
		} /*for*/
	
	/*no empty slots, make the array bigger*/
	
	SetHandleSize (h, (ct + 1) * elementsize);

	if (MemError () != noErr)
		return (false);
		
	p = *h + (ct * elementsize);
	
	moveleft (pdata, p, elementsize);
	
	return (true);
	} /*addtodynamicarray*/
	
	
boolean indynamicarray (long firstfourbytes, tydynamicarray *dynam, ptrchar *pdata) {

	Handle h = (*dynam).hstorage;
	short elementsize = (*dynam).elementsize;
	char *p;
	short ct, i;

	p = (char *) *h;
	
	ct = sizedynamicarray (dynam);
	
	for (i = 0; i < ct; i++) { 
		
		long x;
		
		moveleft (p, &x, longsizeof (x));
		
		if (x == firstfourbytes) { /*the first four bytes match*/
			
			*pdata = p;
			
			return (true);
			}
		
		p += (*dynam).elementsize;
		} /*for*/
	
	return (false);
	} /*indynamicarray*/
	
	
boolean removefromdynamicarray (long firstfourbytes, tydynamicarray *dynam) {
	
	char *p;
	
	if (!indynamicarray (firstfourbytes, dynam, &p))
		return (false);
		
	clearbytes (p, (*dynam).elementsize);
	
	return (true);
	} /*removefromdynamicarray*/
	
	
boolean disposedynamicarray (tydynamicarray *dynam) {
	
	disposehandle ((*dynam).hstorage);
	
	(*dynam).hstorage = nil;
	
	return (true);
	} /*disposedynamicarray*/
	
	

