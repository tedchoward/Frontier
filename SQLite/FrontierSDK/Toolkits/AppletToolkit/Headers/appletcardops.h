
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletcardopsinclude

#ifndef iowaruntimeinclude

	#include <iowaruntime.h>

#endif

#ifndef ioainclude

	#include <ioa.h>

#endif

#ifndef ioaiconinclude

	#include <ioaicon.h>
	
#endif

#ifndef iowaprefsinclude

	#include <iowaprefs.h>
	
#endif


typedef char cardfontstring [33];

	
extern tydialoginfo appdialoginfo [ctdialogs];
	
	
extern hdlcard currentcard;

boolean cardobjectexists (bigstring);

boolean setcardstring (bigstring, void *);

boolean setcardflag (bigstring, boolean);

boolean setcardtmpbit (bigstring, boolean);

boolean setcardappbit (bigstring, short, boolean);

boolean getcardstring (bigstring, void *);

boolean getcardpassword (bigstring, void *);

boolean getcardflag (bigstring, boolean *);

boolean getcardbit (bigstring);

boolean setcardenable (bigstring, boolean);

boolean setcardpopup (bigstring, bigstring, Handle, short);

boolean setcardpopupfrommenu (bigstring, MenuHandle, short);

boolean setcardpopupfromfolder (bigstring, ptrfilespec, bigstring);

boolean setcardpopupcheckeditem (bigstring, short);

short getcardpopup (bigstring);

boolean getcardpopupstring (bigstring, bigstring);

boolean setradiotext (bigstring, Handle);

boolean setcardtextcolor (bigstring, RGBColor *);

boolean emptycardpassword (bigstring);

boolean setcardpassword (bigstring, void *);

boolean setcardicon (bigstring, short, short, Handle, tyiconclickcallback);

boolean getcardcolorpopup (bigstring, RGBColor *);

boolean setcardcolorpopup (bigstring, RGBColor *);

boolean setcardnumber (bigstring, long);

boolean getcardshort (bigstring, short *);

boolean getcardnumber (bigstring, long *);

short getfontsizepopup (bigstring);

void setfontsizepopup (bigstring, short);

void setfontpopup (bigstring, cardfontstring);

void getfontpopup (bigstring, cardfontstring);

boolean getcardshort (bigstring, short *);

boolean getcardrect (bigstring, Rect *);

boolean setcardrect (bigstring, Rect);

boolean setcarddate (bigstring, tydaterecord);

boolean getcarddate (bigstring, tydaterecord *);

boolean getcardtexthandle (bigstring, Handle *);

boolean setcardtexthandle (bigstring, Handle);

boolean replacecardtexthandle (bigstring, Handle *);

boolean enableallcardobjects (boolean);

void updatecard (void);

void appcarddefaulteventhandler (EventRecord *);
	
void appscriptedmodalcardfilter (EventRecord *);

void appopencardresource (short, tycardeventcallback);

void apprunmodalresource (short, tycardeventcallback);

void appsavedialoginfo (short);

void appcardtofront (hdlcard);

