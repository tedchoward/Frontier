
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

/*
	here's the scoop: when Frontier is servicing a component call, 
	running a script, it's A5 is set up.  when running a dialog, the 
	system gets confused if A5 isn't the same as that of the current 
	application.  (one way to make it flake out is to click in the 
	menu bar to pull down the dimmed menus.)  fortunately, we always 
	have the "correct" value at hand -- the low-mem global CurrentA5.
	
	these macros expand on those in Think C's SetUpA4.h by adding two 
	flavors of setting up A5, one to set it to CurrentA4, the other to 
	set it to the A5 that has been stashed away by calling RememberA5. 
	the latter value is Frontier's A5, which must be remembered before 
	setting A5 to CurrentA5.
	
	get it?  it works!
*/

	
#ifdef THINK_C

	static void __GetA5(void) {
		
		asm {
			bsr.s	@1
			dc.l	0			;  store A5 here
		@1	move.l	(sp)+,a1
			}
		}
	
	#define RememberA5()	do { __GetA5(); asm { move.l a5,(a1) } } while (0)
	
	#define SetUpAppA5()	do { asm { move.l a5,-(sp) } __GetA5(); asm { move.l (a1),a5 } } while (0)
	
	#define SetUpCurA5()	do { asm { move.l a5,-(sp) } asm { movea.l 0x904,a5 } } while (0)
	
	#define RestoreA5()		do { asm { move.l (sp)+,a5 } } while (0)

	#define pushA5()		asm { move.l a5,-(a7) }
	
	#define popA5()			asm { move.l (a7)+,a5 }
	
	#define pushA0_D7()		asm { movem.l a0-a5/d1-d7,-(a7) }
	
	#define popA0_D7()		asm { movem.l (a7)+,a0-a5/d1-d7 }
	
	#define setA5to8offA6()	asm { movea.l 8(a6),a5 }

#else

	/*
	dmb 4.0.2b1: statics are referenced through A5, so we must use asm stubs
	to store our data relative to the PC for 68k code
	*/
	
	#if !defined(__POWERPC__) && !defined(__CFM68K__)
	
		static asm long *getAppA5 (void):__A0
		{
			lea		__storage,a0
			rts
		
		__storage:	dc.l	0	/* this storage is only referenced thru data cache */
		}
	
		#define	__appA5	(*getAppA5())

	#else
	
		static long __appA5;
	
	#endif
	
	//Code change by Timothy Paustian Wednesday, July 12, 2000 1:59:26 PM
	//A5 worlds have no relvance in Carbon so just define them away.
	#if TARGET_API_MAC_CARBON == 1
	#define RememberA5()
	
	#define SetUpThisA5(A5) nil
	
	#define SetUpAppA5() nil
		
	#define SetUpCurA5() nil
		
	#define RestoreA5(savedA5)
	
	#define pushA5()
	
	#define popA5()
	#else
		
	#define RememberA5()	do {__appA5 = (long) LMGetCurrentA5 ();} while (0)
	
	#define SetUpThisA5(A5)	SetA5 (A5)
	
	#define SetUpAppA5()	SetA5 (__appA5);
	
//	#define SetUpCurA5()	do { savedA5 = SetA5 ((long) LMGetCurrentA5 ()); } while (0)
	
	#define SetUpCurA5()	SetCurrentA5 ();
	
	#define RestoreA5(savedA5)	SetA5 (savedA5)
	
	#define pushA5()		SetUpCurA5 ()
	
	#define popA5()			RestoreA5 ()
	#endif
	
#endif
	
