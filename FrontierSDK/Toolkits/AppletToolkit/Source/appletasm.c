
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ctype.h>

#include "appletdefs.h"
#include "appletasm.h"


#ifdef THINK_C
boolean fastunicaseequalstrings (bigstring bs1, bigstring bs2) {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal, ignoring case.
	*/
	
	asm {
		/*
		register usage:
			d0 - scratch, return value
			d1 - *bs1
			d2 - *bs2
			d3 - "ct"
			
			a0 - scratch
			a1 - "bs1"
			a2 - "bs2"
		*/
		
		movem.l	d3/a2,-(a7)
		
#ifdef THINK_C
		movea.l	bs1,a1
		movea.l	bs2,a2
#endif
		
		moveq	#0,d1		; clear high bytes
		moveq	#0,d2
		
		moveq	#0,d3		; length of bs1
		move.b	(a1)+,d3
		
		cmp.b	(a2)+,d3	; compare lengths
		beq.s	@1
		
		moveq	#0,d0		; different lengths, return false
		bra.s	@9
		
	@1	subq.w	#1,d3		; length exhausted?
		bge.s	@2
		
		moveq	#1,d0		; yes, strings are equal
		bra.s	@9
		
	@2	move.b	(a1)+,d1	; get *bs1++
		
		lea		__ctype,a0	; isupper?
		adda.l	d1,a0
		btst	#6,(a0)
		beq.s	@3
		
		moveq	#0x20,d0	; tolower
		eor.w	d0,d1
		
	@3	move.b	(a2)+,d2	; get *bs2++
		
		lea		__ctype,a0	; isupper?
		adda.l	d2,a0
		btst	#6,(a0)
		beq.s	@4
		
		moveq	#0x20,d0	; tolower
		eor.w	d0,d2
		
	@4	cmp.w	d1,d2		; characters equal?
		beq.s	@1
		
		moveq	#0,d0
		
	@9
		movem.l	(a7)+,d3/a2
	}
	} /*fastunicaseequalstrings*/
#else
boolean fastunicaseequalstrings (bigstring bs1, bigstring bs2)
{
	char	c1, c2;
	short	i = 1;
	
	// if the lengths aren't equal, fail!
	if (bs1[0] != bs2[0])	return false;
	
	while (i <= bs1[0]) {
		c1 = bs1[i];	c2 = bs2[i];
		
		if (c1 != c2) {	// if not equal as is, fix the case
			if ((c1 >=97) && (c1 <=122))	c1 -= 0x020;
			if ((c2 >=97) && (c2 <=122))	c2 -= 0x020;
			
			if (c1 != c2)	return false;	// they're not equal, so fail!
		}
	}
	
	// we get this far, we're equal!
	return true;
}
#endif

