
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define ioainclude


#ifndef appletdefsinclude

	#include <appletdefs.h>

#endif


#ifndef __APPLEEVENTS__

	#include <AppleEvents.h>

#endif




#define idUserTalk 'LAND' /*identifies Frontier as a scripting component*/

#define idAppleScript 'ascr' /*identifies AppleScript as a scripting component*/


typedef long tyobjecttype;


/*UserLand-supplied object types*/

	#define notype 0 
	
	#define grouptype 1
	
	#define checkboxtype 2
	
	#define radiobuttontype 3
	
	#define picturetype 4
	
	#define statictexttype 5
	
	#define edittexttype 6
	
	#define buttontype 7
	
	#define scrollbartype 8
	
	#define unusedtype2 9
	
	#define recttype 10
	
	#define ovaltype 11
	
	#define linetype 12
	
	#define icontype 13
	
	#define formulatype 14
	
	#define popuptype 15
	
	#define clonetype 16


/*recalc options for objects*/

	#define neverrecalc 1	

	#define initrecalc 2 /*recalc only when card starts up*/

	#define changerecalc 3 /*recalc when another object changes*/

	#define timerecalc 4 /*recalc every x seconds*/
	
	
typedef struct tyobject {

	short versionnumber;
	
	struct tyobject **nextobject; /*next object in the object list*/
	
	struct tyobject **nextselectedobject; /*part of the selection list structure*/
	
	struct tyobject **childobjectlist; /*for group objects*/
	
	struct tyobject **nextinthread; /*flat list of objects, for run mode*/
	
	struct tycard **owningcard; /*each object points back to the card that it appears in*/
	
	tyobjecttype objecttype; /*a button, checkbox, static text, etc.*/
	
	Rect objectrect; /*the display rectangle for the object*/
	
	Handle objectname; /*the variable name for this object*/
	
	Handle objectvalue; /*the string that's displayed in the object*/
	
	Handle objectscript; /*the script that runs when the object is hit, the object's action script*/
	
	Handle objecterrormessage; /*error that was generated when the object was calculated*/
	
	Handle objectdata; /*a refcon handle, must be a contiguous block, no dangling handles*/
	
	unsigned short objectflag: 1; /*for buttons, true --> bold, checkboxes, the box is checked*/
	
	unsigned short objectvisible: 1; /*if false, the object isn't drawn*/
	
	unsigned short objectenabled: 1; /*if false the object is disabled*/
	
	unsigned short objecttransparent: 1; /*if true, don't erase the rect before drawing object*/
	
	unsigned short objectautosize: 1; /*enables auto-resizing -- when you single-click to create new object*/
	
	unsigned short objectinval: 1; /*indicates the object needs re-display*/
	
	unsigned short objecthasframe: 1; /*do we draw a frame for the object?*/
	
	unsigned short objecttmpbit: 1; /*for any temporary use at all*/
	
	unsigned short objectselected: 1; /*if true, the object is part of the selection list*/
	
	unsigned short reservedbit1: 1; /*room for growth*/
	
	unsigned short reservedbit2: 1; /*room for growth*/
	
	unsigned short reservedbit3: 1; /*room for growth*/
	
	unsigned short appbit1: 1; /*other modules can use this bit*/
	
	unsigned short appbit2: 1; /*other modules can use this bit*/
	
	unsigned short appbit3: 1; /*other modules can use this bit*/
	
	unsigned short appbit4: 1; /*other modules can use this bit*/
	
	OSType objectlanguage; /*component subtype for linked script's interpreter*/ 
	
	Handle objecteditrecord; 
	
	short objectfont;
	
	short objectfontsize;
	
	short objectstyle;
	
	tyjustification objectjustification;
	
	RGBColor objectfillcolor, objecttextcolor, objectframecolor;
	
	short objectdropshadowdepth;
	
	short objectlinespacing;
	
	short objectindentation;
	
	short sorttag; /*used for sorting the object list by location*/
	
	Handle objectrecalcscript; /*the object's recalc script*/
	
	short objectrecalcstatus; /*when does this object recalc?*/
	
	short objectrecalcperiod; /*if it recalcs every x seconds, this is x*/
	
	unsigned long objectnextruntime; /*number of secs since 1/1/04 for the next recalc*/
	
	char waste [6];
	} tyobject, *ptrobject, **hdlobject;
	
	
typedef boolean (*tyobjectvisitroutine) (hdlobject);
		
typedef void (*tyupdatecallback) (void);


#define cttemphandles 25


typedef boolean (*tyobjectcallback) (hdlobject);

typedef boolean (*tystackedobjectcallback) (tyobject *);

typedef boolean (*tycleanobjectcallback) (hdlobject, short, short, Rect *);

typedef boolean (*tyobjectbooleancallback) (hdlobject, boolean);

typedef boolean (*tyobjectcharcallback) (hdlobject, char);

typedef boolean (*tyobjectrectcallback) (hdlobject, Rect *);

typedef boolean (*tyobjectptrhandlecallback) (hdlobject, Handle *);

typedef boolean (*tyobjecthandlecallback) (hdlobject, Handle);

typedef boolean (*tyobjectstringcallback) (hdlobject, Str255);

typedef boolean (*tymouseclickcallback) (hdlobject, hdlobject, Point, boolean, boolean);

typedef boolean (*tyobjectpointcallback) (hdlobject, Point);

typedef void (*tygetobjectsizecallback) (hdlobject, short *, short *);

typedef boolean (*tyshortcallback) (short);

typedef boolean (*tyvoidcallback) (void);

typedef void (*tydrawtexthandlecallback) (Handle, Rect, tyjustification);

typedef boolean (*tyhandlecallback) (Handle);

typedef boolean (*tyeditclickcallback) (Point, boolean, Handle);

typedef boolean (*tycharhandlecallback) (char, Handle);

typedef boolean (*tyRGBcallback) (RGBColor *);

typedef Handle (*tyreturnshandlecallback) (void);

typedef boolean (*tygetnthstringcallback) (hdlobject, short, bigstring);

typedef boolean (*tyappleeventcallback) (hdlobject, AppleEvent *);

typedef boolean (*typackdatacallback) (hdlobject, Handle *);

typedef boolean (*tyunpackdatacallback) (hdlobject, Handle);

typedef boolean (*tyevalscriptcallback) (hdlobject, Handle, Handle *, OSType, bigstring);


 
typedef struct tycard {
	
	short versionnumber; /*must be the first object in the struct, must be 2 bytes*/
	
	hdlobject objectlist; /*the first object in the display list, must be 2nd item in this struct*/
	
	hdlobject selectionlist; /*the first selected object, nil if nothing selected*/
	
	Handle windowtitle; /*the string displayed in the window's title bar, if it has one*/
	
	boolean runmode; /*if true, the card is being run, not being edited*/
	
	boolean flactive; /*true if the window containing this card is active*/
	
	boolean tracking; /*if true we're in a very special mode, tracking the mouse*/
	
	hdlobject trackerobject; /*the object that's being tracked*/
	
	boolean trackerpressed; /*if true, the mouse is in the right rectangle*/
	
	boolean bitmapactive; /*if true, drawing routines don't use offscreen bitmaps*/
	
	Handle updatethiseditbuffer; /*used for deferred edit buffer updating*/
	
	hdlobject activetextobject; /*the current edittext object being edited*/
	
	boolean flskiptext; /*see drawobject*/
	
	boolean dontdraw; /*set true if we're just finding out how big the drawing is*/
	
	Rect pictrect; /*this is how big the drawing is*/
	
	PicHandle macpicture; /*used when we're printing the card*/
	
	boolean needsupdate; /*set true if some object has been inval'd*/
	
	RGBColor backcolor; /*the card's background color*/
	
	WindowPtr drawwindow; /*allows runmode to override the window that's drawn into*/
	
	Rect updaterect; /*the portion of the card that needs updating*/
	
	tyobject defaults; /*defaults for colors, other styles*/
	
	tyupdatecallback updatecallback; /*updating is handled differently in runtime*/
	
	boolean flgrid; /*if true, snap-to-grid is on*/
	
	short gridunits; /*the number of pixels for each grid unit*/
	
	Handle temphandles [cttemphandles]; /*temporary handles generated when eval'ing expressions*/
	
	boolean recalcneeded; /*if true some value changed, all formulas need recalcing*/
	
	long recalctickcount; /*the time to do the recalc, non-zero when user is typing, for example*/
	
	short rightborder, bottomborder; /*allows designer to determine the size of the running card*/
	
	long refcon; /*used by editor to store other stuff*/
	
	Handle tablename; /*the sub-table of system.cards that holds this card's table*/
	
	Handle embeddedtable; /*packed Frontier table that goes where the card goes*/
	
	boolean tablestoredinfrontier; /*if false, Frontier wasn't running when the card was opened*/
	
	FontInfo fontinfo; /*a copy of globalfontinfo, available to IOA callback routines*/

	tygetobjectsizecallback IOAgetobjectsizeCallback;
	
	tyobjectptrhandlecallback IOAgetstringvalueCallback;
	
	tyRGBcallback IOApushforecolorCallback;
	
	tyvoidcallback IOApopforecolorCallback;
	
	tyRGBcallback IOApushbackcolorCallback;
	
	tyvoidcallback IOApopbackcolorCallback;
	
	tyobjectcallback IOArunbuttonscriptCallback;
	
	tyobjectptrhandlecallback IOAgetbooleanvalueCallback;
	
	tyobjecthandlecallback IOAsetbooleanvalueCallback;
	
	tydrawtexthandlecallback IOAeditdrawtexthandleCallback;
	
	tyobjectcallback IOArecalcobjectvalueCallback;
	
	tyevalscriptcallback IOAevalscriptCallback;
	
	tyobjectcallback IOAinvalobjectCallback;
	
	tyobjecthandlecallback IOAsetobjectvalueCallback;
	
	tyvoidcallback IOAclearactivetextobjectCallback;
	
	tyobjectcallback IOAsetactivetextobjectCallback;
	
	tyreturnshandlecallback IOAgetactiveeditrecordCallback;
	
	tyeditclickcallback IOAeditclickCallback;
	
	tyhandlecallback IOAeditselectallCallback;
	
	tyhandlecallback IOAeditidleCallback;
	
	tycharhandlecallback IOAeditkeystrokeCallback;
	
	tygetnthstringcallback IOAgetnthstringCallback;
	
	boolean flinvisiblegrid; /*move me! then rebuild all the IOAs*/
	
	boolean flautoname; /*move me! then rebuild all the IOAs*/
	
	EventRecord macevent; /*move me! then rebuild all the IOAs*/
	
	boolean floater; /*true if the window is a floating window in the host*/
	
	long idwindow; /*determines what kind of window is used to display card*/
	
	boolean fullupdate; /*true if a global has been changed, requiring a full update*/
	
	Point defaultnewobjectlocation; /*the next place a new object is created by a script*/
	
	Rect deferredinvalrect; /*allows system event handlers to inval a rectangle*/
	
	boolean notscriptedcard: 1; /*for clay basket and others, that drive the cards from C*/
	
	boolean runtimevaluechanged: 1; /*DW 9/19/95 -- at least 1 value in card changed, you might want to enable an Update button*/
	
	boolean reservedbit2: 1; /*room for growth*/
	
	boolean reservedbit3: 1; /*room for growth*/
	
	boolean reservedbit4: 1; /*room for growth*/
	
	boolean reservedbit5: 1; /*room for growth*/
	
	boolean reservedbit6: 1; /*room for growth*/
	
	boolean reservedbit7: 1; /*room for growth*/
	
	boolean reservedbit8: 1; /*room for growth*/
	
	boolean runmodechanges: 1; /*set true when one of the objects changes, can be cleared by client after changes are saved or discarded*/
	
	boolean appbit1: 1; /*a bit that the application can use*/
	
	boolean appbit2: 1; /*a bit that the application can use*/
	
	boolean appbit3: 1; /*a bit that the application can use*/
	
	boolean appbit4: 1; /*a bit that the application can use*/
	
	boolean appbit5: 1; /*a bit that the application can use*/
	
	boolean appbit6: 1; /*a bit that the application can use*/
	
	hdlobject recalcobject; /*dmb 1.0b20 - the object that changed, triggering a card recalc*/
	
	char waste [4]; /*room for growth*/
	} tycard, **hdlcard;
	

typedef struct tyconfigrecord {
	
	long objectTypeID; 
	
	char objectTypeName [32];
	
	char objectFlagName [32];
	
	boolean frameWhenEditing;
	
	boolean canEditValue;
	
	boolean toggleFlagWhenHit;
	
	boolean mutuallyExclusive;
	
	boolean speaksForGroup;
	
	boolean handlesMouseTrack;
	
	boolean editableInRunMode;
	
	boolean isFontAware;
	
	boolean alwaysIdle;
	
	boolean hasSpecialCard;
	
	boolean cantDrawInBitmap;
	
	tystackedobjectcallback initObjectCallback;
	
	tyobjectcallback disposeDataCallback;
	
	tyobjectcallback drawObjectCallback;
	
	tyobjectbooleancallback editObjectCallback;
	
	tyobjectcharcallback keystrokeObjectCallback;
	
	tyobjectcallback idleObjectCallback;
	
	tymouseclickcallback clickObjectCallback;
	
	tycleanobjectcallback cleanupObjectCallback;
	
	tyobjectbooleancallback recalcObjectCallback;
	
	tyobjectcallback canReplicateObjectCallback;
	
	tyobjectcallback catchReturnCallback;
	
	tyobjectrectcallback getObjectInvalRectCallback;
	
	tyobjectrectcallback getObjectEditRectCallback;
	
	tyobjectptrhandlecallback getValueForScriptCallback;
	
	tyobjecthandlecallback setValueFromScriptCallback;
	
	tyobjectpointcallback setObjectCursorCallback;
	
	tyobjectstringcallback debugObjectCallback;
	
	tyappleeventcallback getAttributesCallback;
	
	tyappleeventcallback setAttributesCallback;
	
	typackdatacallback packDataCallback;
	
	tyobjectcallback unpackDataCallback;
	} tyconfigrecord, *ptrconfigrecord, **hdlconfigrecord;
	
	
void setupconfig (tyconfigrecord *); /*every IOA component must define this routine*/


#define IOAgetconfigcommand 1
#define IOAinitobjectcommand 2
#define IOAdrawobjectcommand 3
#define IOAeditobjectcommand 4
#define IOAkeystrokecommand 5
#define IOAidlecommand 6
#define IOAclickcommand 7
#define IOAcleanupcommand 8
#define IOArecalccommand 9
#define IOAcanreplicatcommand 10
#define IOAcatchreturncommand 11
#define IOAgetobjectinvalrectcommand 12
#define IOAgetobjecteditrectcommand 13
#define IOAgetvaluecommand 14
#define IOAsetvaluecommand 15
#define IOAsetcursorcommand 16
#define IOAunpackdatacommand 17
#define IOAdebugobjectcommand 18
#define IOAgetattributescommand 19
#define IOAsetattributescommand 20
#define IOAgeticoncommand 21
#define IOAgetcardcommand 22
#define IOApackdatacommand 23
#define IOAdisposedatacommand 24

	
	
void IOAfillchar (void *, long, char);

void IOAclearbytes (void *, long);

short IOAmakemultiple (short, short);

short IOAclosestmultiple (short, short);

void IOAgetobjectsize (hdlobject, short *, short *);

boolean IOAgetstringvalue (hdlobject, Handle *);

boolean IOApushforecolor (RGBColor *);

boolean IOApopforecolor (void);

boolean IOApushbackcolor (RGBColor *); 

boolean IOApopbackcolor (void);

boolean IOAvisitobjects (hdlobject, tyobjectvisitroutine);

boolean IOArunbuttonscript (hdlobject);

void IOAcopystring (void *, void *);

boolean IOAgetbooleanvalue (hdlobject, Handle *);

boolean IOAsetbooleanvalue (hdlobject, Handle);

void IOAeditdrawtexthandle (Handle, Rect, tyjustification);

boolean IOArecalcobjectvalue (hdlobject);

boolean IOAevalscript (hdlobject, Handle, Handle *, OSType, bigstring);

boolean IOAsetobjectvalue (hdlobject, Handle);

boolean IOAgetnthstring (hdlobject, short, bigstring);

void IOAinvalobject (hdlobject);

void IOAtexthandletostring (Handle, bigstring);

boolean IOAnewtexthandle (bigstring, Handle *);

void IOAcenterstring (Rect, bigstring);

void IOAcenterrect (Rect *, Rect);

boolean IOAclearactivetextobject (void);

boolean IOAsetactivetextobject (hdlobject);

Handle IOAgetactiveeditrecord (void);

boolean IOAeditclick (Point, boolean, Handle);

boolean IOAeditselectall (Handle);

boolean IOAeditidle (Handle);

boolean IOAeditkeystroke (char, Handle);

void IOAellipsize (Str255, short);

boolean IOAincolorwindow (hdlobject);

boolean IOAopenresfile (void);

void IOAcloseresfile (void);

boolean IOApushstring (bigstring bssource, bigstring bsdest);

boolean IOAregistercomponent (void);

void IOAunregistercomponent (void);
