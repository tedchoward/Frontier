
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletinternal.h"




#define noaction 0



static boolean flmultipleundo = false;

hdlundostack undostack = nil;

hdlundostack redostack = nil;


typedef boolean (*globalscallback) (long, boolean);


static boolean initstack (short basesize, short elemsize, hdlstack *hstack) {

	/*
	generic handle-based stack initialization
	*/

	ptrstack ps;

	if (!newclearhandle ((long) basesize, (Handle *) hstack))
		return (false);

	ps = **hstack;

	ps->topstack = 0;
	
	ps->basesize = basesize;
	
	ps->elemsize = elemsize;
	
	return (true);
	} /*initstack*/


static boolean pushstack (hdlstack hstack, ptrvoid pelem) {

	/*
	push a new element, pelem, onto the top of the handle-based 
	stack hstack
	*/

	if (!enlargehandle ((Handle) hstack, (long) (**hstack).elemsize, pelem))
		return (false);

	++(**hstack).topstack;

	return (true);
	} /*pushstack*/


static boolean popstack (hdlstack hstack, ptrvoid pelem) {

	/*
	pop the top element off of the handle-based stack hstack, copying
	into pelem
	*/

	long elemsize = (**hstack).elemsize;
	long newsize;

	if (!(**hstack).topstack) /*check for empty stack*/
		return (false);

	newsize = gethandlesize ((Handle) hstack) - (long) elemsize;

	moveleft (* (char **) hstack + newsize, pelem, elemsize);

	sethandlesize ((Handle) hstack, newsize); /*should never fail; going smaller*/

	--(**hstack).topstack;

	return (true);
	} /*popstack*/


static void swapundostacks (void) {

	/*
	redo is often identical to undo, but sometimes operates on the 
	opposite stacks.  swapping stacks avoids having to duplicate a lot 
	of code, or having to pass the current stack as a parameter everywhere
	
	12/11/90 dmb: must change fields in windowinfo too, in case undo 
	routines cause globals to be pushed and popped
	*/
	
	hdlappwindow ha = app.appwindow;
	hdlundostack htempstack = undostack;
	
	undostack = redostack;
	
	redostack = htempstack;
	
	/*keep true globals in sync*/
	
	(**ha).undostack = (Handle) undostack;
	
	(**ha).redostack = (Handle) redostack;
	} /*swapundostacks*/


static boolean pushstep (undocallback pundo, Handle hundo, boolean flaction) {

	/*
	push a new step, defined by an undo routine and its data, onto 
	the undo stack.  any number of steps can accumulate to undo a 
	single action.  if no action is pending, just toss the undo by 
	calling the undo routine with flundo false
	*/
	
	tyundorecord step;
	
	if (!undostack || (**undostack).ixaction == noaction) {
		
		(*pundo) (hundo, false);
		
		return (true);
		}
		
	step.undoroutine = pundo;
	
	step.hundodata = hundo;
	
	step.flactionstep = flaction;
	
	return (pushstack ((hdlstack) undostack, &step));
	} /*pushstep*/


boolean pushundostep (undocallback pundo, Handle hundo) {

	/*
	push a new step, defined by an undo routine and its data, onto 
	the undo stack.  any number of steps can accumulate to undo a 
	single action.  if no action is pending, just toss the undo by 
	calling the undo routine with flundo false
	*/
	
	return (pushstep (pundo, hundo, false));
	} /*pushundostep*/


static boolean pushactionstep (short ixaction, long globaldata) {
	
	return (pushstep ((undocallback) ixaction, (Handle) globaldata, true));
	} /*pushactionstep*/


static boolean popundostep (tyundorecord *pstep) {
	
	/*
	pop the next step off of the undo stack.  this is usually done only 
	within the undo code (in this file), but may be called from a 
	higher level under certain circumstances.
	*/
	
	return (popstack ((hdlstack) undostack, pstep));
	} /*popundostep*/


static boolean pushaction (short ixaction, long globaldata) {

	/*
	start recording undos for a new action.  ixaction is a non-zero index 
	into a stringlist of undoable actions, used to build the menu item.
	
	to mark the juncture between the previous action steps (if any), the 
	index of the previous action is pushed onto the stack as a psuedo-step.
	*/
	
	hdlundostack hstack = undostack;
	short ixlastaction = (**hstack).ixaction;
	
	if (!hstack)
		return (false);
	
	/* if previous action exists, try pushing it */
	
	if (ixlastaction != noaction  &&  !pushactionstep (ixlastaction, (**hstack).globaldata))
		return (false);
	
	/* set the new action */
	
	(**hstack).globaldata = globaldata;
	
	(**hstack).ixaction = ixaction;
	
	return (true);
	} /*pushaction*/


static void killactions (void) {

	/*
	release all pending undo steps, for all actions.
	
	7/9/91 dmb: need to set up globals, but only if an undo step is actually 
	tossed
	*/
	
	hdlundostack hstack = undostack;
	long globaldata = (**hstack).globaldata;
	tyundorecord step;
	
	while (popundostep (&step)) { /*pop stack until empty; ignore action records*/
		
		if (step.flactionstep)
			globaldata = (long) step.hundodata;
		
		else {
			
			(*(globalscallback) app.setundoglobalscallback) (globaldata, false); /*set up environment*/
			
			(*step.undoroutine) (step.hundodata, false); /*ignore errors*/
			}
		}
	
	(**hstack).ixaction = noaction;
	} /*killactions*/


boolean pushundoaction (short ixaction) {

	/*
	start recording undos for a new action.  ixaction is a non-zero index 
	into a stringlist of undoable actions, used to build the menu item.
	
	any pending redo must be killed; since we're making a new change, we're 
	creating a new future than cannot be comingled with the old future that 
	was undone

	for single-level undo, this routine also kills any existing undo.  
	for multiple-level undo, subsequent undos are added to the same undo stack.
	*/
	
	long globaldata;
	
	if (undostack == nil)
		return (false);
	
	if (!flmultipleundo)
		killactions ();
	
	swapundostacks (); /*to redo*/
	
	killactions ();
	
	swapundostacks (); /*restore*/
	
	if ((ixaction == noaction) || !(*(boolean (*)(long *))app.getundoglobalscallback) (&globaldata))
		globaldata = 0L;
	
	return (pushaction (ixaction, globaldata));
	} /*pushundoaction*/


boolean popundoaction (void) {

	/*
	remove the topmost action's steps from the undo stack, releasing the 
	undo by calling the undo routines with flundo false.
	
	7/9/91 dmb: need to set up globals, but only if an undo step is actually 
	tossed
	*/
	
	hdlundostack hstack = undostack;
	tyundorecord step;
	short ixaction;
	long globaldata;
	
	ixaction = noaction;
	
	globaldata = 0L;
	
	while (popundostep (&step)) { /*pop stack until empty; ignore action records*/
	
		if (step.flactionstep) { /*new action record; finished with this undo*/
			
			ixaction = (short) step.undoroutine;
			
			globaldata = (long) step.hundodata;
			
			break;
			}
		
		(*(globalscallback) app.setundoglobalscallback) ((**hstack).globaldata, false); /*set up environment*/
		
		(*step.undoroutine) (step.hundodata, false); /*ignore errors*/
		}
	
	(**hstack).ixaction = ixaction; /*update current action*/
	
	(**hstack).globaldata = globaldata;
	
	return (true);
	} /*popundoaction*/


boolean undolastaction (boolean flbuildredo) {
	
	hdlundostack hstack = undostack;
	short ixaction;
	long globaldata;
	tyundorecord step;
	
	ixaction = (**hstack).ixaction;
	
	if (ixaction == noaction)
		return (false); /*nothing to undo*/
	
	globaldata = (**hstack).globaldata;
	
	swapundostacks ();
	
	if (flbuildredo)
		pushaction (ixaction, globaldata);
	else
		pushaction (noaction, 0L);
	
	swapundostacks ();
	
	(*(globalscallback) app.setundoglobalscallback) (globaldata, true); /*set up environment, as required*/
	
	ixaction = noaction; /*default if new action isn't found*/
	
	globaldata = 0L;
	
	while (popundostep (&step)) { /*pop stack until empty or action record is found*/
	
		if (step.flactionstep) { /*new action record; finished with this undo*/
			
			ixaction = (short) step.undoroutine;
			
			globaldata = (long) step.hundodata;
			
			break;
			}
		
		swapundostacks ();
		
		(*step.undoroutine) (step.hundodata, true); /*xxx error checking here*/
		
		swapundostacks ();	
		}
	
	(**hstack).ixaction = ixaction; /*update current action*/
	
	(**hstack).globaldata = globaldata; /*update current action*/
	
	return (true); /*xxx*/
	} /*undolastaction*/


boolean redolastaction (boolean flbuildundo) {

	swapundostacks ();

	undolastaction (flbuildundo);

	swapundostacks ();
	
	return (true);
	} /*redolastaction*/


boolean getundoaction (short *ixaction) {

	*ixaction = (**undostack).ixaction;
	
	return (*ixaction != noaction);
	} /*getundoaction*/


boolean getredoaction (short *ixaction) {

	*ixaction = (**redostack).ixaction;
	
	return (*ixaction != noaction);
	} /*getredoaction*/


boolean getundomenuitem (bigstring bsundo, boolean *flenabled) {
	
	short ixbase, ixaction;
	bigstring bsaction;
	
	if (app.appwindow == nil) {
		
		getstringlist (undolistnumber, undostring, bsundo);
		
		*flenabled = true;
		}
	else { /* try to get redo first, else try to get undo */
		
		getstringlist (undolistnumber, cantundoitem, bsundo); /*default*/
		
		*flenabled = false;
		
		if (getredoaction (&ixaction))
			ixbase = redostring;
		else {
			if (getundoaction (&ixaction))
				ixbase = undostring;
			}
		
		if (ixaction > 0) { /*it's an undoable operation*/
			
			getstringlist (undolistnumber, ixbase, bsundo);
			
			if (getstringlist (undolistnumber, ixaction, bsaction))
				pushstring (bsaction, bsundo);
			
			*flenabled = true;
			}
		}
	
	return (stringlength (bsundo) > 0);
	} /*getundomenuitem*/

 
void killundo (void) {

	/*
	release all pending undo and redo steps, for all actions.
	*/
	
	killactions (); /*kill undo*/
	
	swapundostacks ();
	
	killactions (); /*kill redo*/
	
	swapundostacks ();
	} /*killundo*/


boolean newundostack (hdlundostack *hstack) {
	
	if (!initstack (sizeof (tyundostack), sizeof (tyundorecord), (hdlstack *) hstack))
		return (false);
	
	(***hstack).ixaction = noaction;
	
	return (true);
	} /*newundostack*/


boolean disposeundostack (hdlundostack hstack) {
	
	#ifdef notdefined	
		if (hstack != nil)
			if ((**hstack).ixaction != noaction)
				alertdialog ("\pDisposing non-empty undo!");
	#endif
	
	disposehandle ((Handle) hstack);
	
	return (true);
	} /*disposeundostack*/



