
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

#define langinternalinclude


#ifndef langinclude

	#include "lang.h"
	
#endif



#define langerrorlist 257

#define undefinederror 1
#define ostypecoerceerror 2 /*maxstringlengtherror*/
#define badentrypointnameerror 3 /*nilparametererror*/
#define stringcharerror 4
#define stringlongerror 5
#define unknownidentifiererror 6
#define notenoughparameterserror 7
#define unexpectedopcodeerror 8 /*used to be builtinrequiresstringerror*/
#define unknownfunctionerror 9
#define cantdeleteerror 10
#define charoutofrangeerror 11
#define inttoolargeerror 12
#define inttoosmallerror 13
#define stringnotterminatederror 14
#define badcharconsterror 15
#define illegaltokenerror 16
#define stringnotbooleanerror 17
#define dividebyzeroerror 18
#define parsererror 19
#define tmpstackoverflowerror 20
#define notvarparamerror 21
#define multiplesymbolerror 22
#define intcoerceerror 23
#define charcoerceerror 24
#define longcoerceerror 25
#define datecoerceerror 26
#define stringcoerceerror 27
#define booleancoerceerror 28
#define notfunctionerror 29
#define toomanyparameterserror 30
#define badreplacecontexterror 31 /*nohfswindowerror*/
#define badaddresserror 32 /*undeclaredapperror*/
#define ipcerror 33
#define appnotloadederror 34
#define badexternalassignmenterror 35 /*texttoolongerror*/
#define cantcoercetobinaryerror 36
#define coercionnotpossibleerror 37
#define unaryminusnotpossibleerror 38
#define unarynotnotpossibleerror 39
#define foragentsonlyerror 40 /*nowindowopenerror*/
#define invaliddirectionerror 41
#define badipclistvalerror 42 /*notarrayerror*/
#define badipclistposerror 43 /*arrayreferror*/
#define numbertoolargeerror 44 /*cantcoercetostringerror*/
#define ipcappleerror 45 /*badostypeerror*/
#define nosuchtableerror 46
#define notefperror 47
#define arraynottableerror 48
#define arrayindexerror 49
#define arraystringindexerror 50
#define cantsizeerror 51
#define tabletoosmallerror 52
#define numbernotpositiveerror 53 /*badindexname*/
#define cantpackerror 54
#define cantunpackerror 55
#define unpackformaterror 56
#define cantpackthisexternalerror 57
#define cantunpackthisexternalerror 58
#define iacaddresserror 59 /*verbnotacceptederror*/
#define addresscoerceerror 60 /*onedotperexternalmessageerror*/
#define iactoolkitnotintializederror 61
#define badwithstatementerror 62 /*stringtoolongerror*/
#define nopatherror 63
#define externalreturnerror 64 /*addressrequirederror*/
#define externalgetvalueerror 65
#define externalassignerror 66
#define trapnotrunningerror 67
#define dialognotrunningerror 68
#define cantloaddialogerror 69
#define dialogitemnumerror 70
#define cantnestdialogserror 71
#define badexternaloperationerror 72
#define additionnotpossibleerror 73
#define subtractionnotpossibleerror 74
#define multiplicationnotpossibleerror 75
#define divisionnotpossibleerror 76
#define modulusnotpossibleerror 77
#define comparisonnotpossibleerror 78
#define toomanywithtableserror 79
#define stackoverflowerror 80
#define badoutgoingipctypeerror 81
#define cantruncarderror 82 /*badincomingipctypeerror*/
#define binarycoerceerror 83
#define pointcoerceerror 84
#define rectcoerceerror 85
#define rgbcoerceerror 86
#define patterncoerceerror 87
#define filespeccoerceerror 88
#define aliascoerceerror 89
#define badrandomboundserror 90 /*objspeccoerceerror*/
#define cantusealiaseserror 91
#define cantuseobjspecserror 92
#define badkeyformerror 93
#define binaryrequirederror 94
#define floatcoerceerror 95
#define notxcmderror 96
#define notucmderror 97
#define notcarderror 98
#define badrangeoperationerror 99
#define badobjectspecificationerror 100
#define cantopencomponenterror 101
#define cantbackgroundclipboard 102
#define listcoerceerror 103
#define badfieldoperationerror 104
#define unknownparametererror 105
#define duplicateparametererror 106
#define binarytypecoerceerror 107
#define nocomponentmanagererror 108
#define bitindexerror 109
#define badnetworkvolumespecificationerror 110
#define nofileopenerror 111
#define floattolongerror 112
#define semaphoretimeouterror 113
#define badthreadiderror 114
#define dbnotopenederror 115
#define dbopenedreadonlyerror 116
#define unimplementedverberror 117
#define needopendberror 118
#define externalvaluerequirederror 119
#define niladdresserror 120
#define tableloadingerror 121
#define filenotopenederror 122
#define cantconnecttodllerror 123
#define cantfindprocinfoerror 124
#define cantfinddllfunctionerror 125
#define cantfindprocinfofunctionerror 126
#define noattributestableerror 127
#define cantfindattributeerror 128
#define cantgetxmladdresserror 129
#define badxmltexterror 130
#define missingxmlattributeserror 131
#define cantdecompilerror 132
#define frontierxmldatatypeerror 133
#define hashpackerror 134
#define hashunpackerror 135
#define evaldirectiveerror 136
#define emptydefinedirective 137
#define illegalnameerror 138
#define tablesavingerror 139
#define badrenameerror 140
#define cantencodeaddress 141
#define cantdecodeaddress 142
#define badwindowerror 143
#define parseaddresserror 144
#define urlspliterror 145
#define cloudspecerror 146
#define cloudelementerror 147
#define bitshiftdisterror 148
#define cantfindprocinfoloaderror 149
#define regexpcompileerror 150
#define regexpinvaliderror 151
#define regexpinternalerror 152
#define regexpbadgroupnameerror 153
#define regexpnonexistantgroupnumbererror 154
#define regexpnonexistantgroupnameerror 155
#define regexpnonexistantgrouperror 156
#define frinternalerror 157
#define frbadgroupnameerror 158
#define frnonexistantgroupnumbererror 159
#define frnonexistantgroupnameerror 160

#define langstacklist 137

typedef enum tystackid { /*stack names*/
	
	idtablestack = 1,
	
	iderrorcallbackstack,
	
	idsourcestack,
	
	idlocalsstack,
	
	idprogramstack
	} tystackid;


#define machinePPC	"\x07" "PowerPC"
#define machine68K	"\x03" "68K"
#define machinex86	"\x03" "x86"

#define osMacOS		"\x05" "MacOS"
#define osWin95		"\x05" "Win95"
#define osWinNT		"\x05" "WinNT"
//Code change by Timothy Paustian Tuesday, July 11, 2000 9:42:50 PM
//Add a const to return for the carbon OS
#define osCarbon	"\x05" "MacCn"

typedef boolean (*langtreevisitcallback) (hdltreenode, ptrvoid);


#define idusertalkstring 128
#define idusertalkdescriptionstring 129


#define langinterfacelist 138

enum { /*lang interface strings*/
	
	useparenthesisstring = 1,
	
	unresolvedaliasstring,
	
	unavailablestring,
	
	appwhoseidstring,
	
	appinfotablestring,
	
	appidfieldstring,
	
	appnamefieldstring,
	
	onthisnetstring,
	
	onthismacstring,
	
	nomaintorunstring,
	
	needmodulecallstring,
	
	useaddressoperatorstring,
	
	usertalkcomponentstring,
		
	anomynousthreadstring,

	appleeventthreadstring
	};


#define langmiscstringlist 158

enum { /*lang misc display strings*/

	unknownstring = 1,
	
	errorstring,
	
	
	#if !flruntime
	
	ondiskstring,
	
	treesizestring,
	
	treesizestring2,
	
	tokennumberstring,
	
	breakpointstring,
	
	levelstring,
	
	enumstring,
	
	#endif
	
	
	numberstring,
	
	nonestring,
	
	nilstring,
	
	justnilstring
	};


extern boolean fllangerror; /*if true, the langerror dialog has already appeared*/

extern unsigned short langerrordisable; /*6.1.1b2 AR: if > 0, don't execute langerrors*/

extern Handle tryerror; /*non-nil after try error, until else is evaluated*/

extern Handle tryerrorstack; /*non-nil after try error, until else is evaluated*/

extern boolean flparamerrorenabled; /*if true, no error dialog from getparam*/

extern boolean flbreak; /*for break op*/

extern boolean flcontinue; /*for continue op*/

extern boolean flreturn; /*for return op*/

extern hdlhashtable hmagictable; /*for communication with evaluatelist*/

extern DialogPtr langmodaldialog;

extern bigstring bsfunctionname;

extern boolean flstackoverflow; /*langops.c*/

extern boolean flfindanyspecialsymbol;

extern bigstring bscontainername; /*langtree.c*/

extern boolean flcoerceexternaltostring; /*langvalue.c*/

extern boolean flinhibitnilcoercion; /*langvalue.c*/

//extern short flextendedsymbolsearch;


/*prototypes*/

extern boolean langpushlocalchain (hdlhashtable *); /*langcallbacks.c*/

extern boolean langpoplocalchain (hdlhashtable);

extern boolean languserescaped (boolean);

extern langprogramkilled (void);

extern boolean langdebuggercall (hdltreenode);

extern boolean langsaveglobals (void);

extern boolean langrestoreglobals (void);

extern boolean langpushsourcecode (hdlhashtable, hdlhashnode, bigstring);

extern boolean langpopsourcecode (void);

extern boolean langgetsourcecodeaddress (hdlhashtable *htable, bigstring bsname);

extern boolean langerrormessage (bigstring);


#ifdef flnewfeatures

extern boolean langruncard (hdltreenode hparam1, boolean flmodal, tyvaluerecord *vreturned); /*langcard.c*/

extern boolean langismodalcard (hdltreenode, tyvaluerecord *);

extern boolean langsetmodalcardtimeout (hdltreenode, tyvaluerecord *);

extern boolean langcardstart (void);

#endif


extern boolean langdialogrunning (void); /*langdialog.c*/

extern boolean langgetdialogvalue (hdltreenode, tyvaluerecord *);

extern boolean langsetdialogvalue (hdltreenode, tyvaluerecord *);

extern boolean langsetdialogitemenable (hdltreenode, tyvaluerecord *);

extern boolean langsetdialogitemvis (hdltreenode, boolean, tyvaluerecord *);

extern boolean langrundialog (hdltreenode, tyvaluerecord *);


extern boolean langdialogstart (void); /*langdialog.c*/


extern void parseerror (bigstring); /*langerror.c*/


extern hdlerrorstack langerrorgetstack (void); /*langerrorwindow.c*/

extern boolean langerrorstart (void);


extern void langseterrorline (hdltreenode); /*langevaluate.c*/

extern boolean langrunscript (bigstring, tyvaluerecord *, hdlhashtable, tyvaluerecord *);

extern boolean evaluatetree (hdltreenode, tyvaluerecord *);

extern boolean evaluatelist (hdltreenode, tyvaluerecord *);


extern boolean hashflushcache (long *); /*langhash.h*/

extern short hashfunction (const bigstring);

extern boolean hashresolvevalue (hdlhashtable, hdlhashnode);


extern boolean langrunmodeless (hdltreenode, tyvaluerecord *); /*langmodeless.c*/


extern boolean langheaptype (tyvaluetype); /*langops.c*/

extern boolean langheapallocated (tyvaluerecord *, Handle *);

extern void langgetwithvaluename (short, bigstring);

extern boolean langgetvalsize (tyvaluerecord, long *);

extern boolean langgetstringlist (short, bigstring);

extern boolean langgetmiscstring (short, bigstring);

extern boolean langcheckstacklimit (tystackid, short, short);

extern void langbadexternaloperror (short, tyvaluerecord);

extern void langarrayreferror (short, bigstring, const tyvaluerecord *, tyvaluerecord *);

extern boolean langfindexternalwindow (const bigstring, Handle *);

extern boolean langgetuserflag (short, boolean);

extern boolean langsetuserflag (short, boolean);

extern boolean langgetthisaddress (hdlhashtable *, bigstring);

extern boolean langsetthisvalue (hdlhashtable, hdlhashtable, bigstring);

extern boolean langfastaddresstotable (hdlhashtable, bigstring, hdlhashtable *);

extern boolean langvaltocode (tyvaluerecord *vcode, hdltreenode *hcode); //02/04/02 dmb

extern boolean langfollowifaddressvalue (tyvaluerecord *v);


extern boolean isfirstidentifierchar (byte ch); /*langscan.c*/

extern boolean isidentifierchar (byte ch);

extern boolean langisidentifier (bigstring);

extern boolean langdeparsestring (bigstring, byte);

extern boolean langstriptextsyntax (Handle);

extern boolean langaddapplescriptsyntax (Handle);


extern void langstarttrace (void); /*langtrace.c*/

extern void langendtrace (void);

extern void langtrace (bigstring);

extern void langsyntaxtrace (boolean);


extern boolean langvisitcodetree (hdltreenode, langtreevisitcallback, ptrvoid); /*langtree.c*/

extern short langcounttreenodes (hdltreenode);

extern boolean newconstnode (tyvaluerecord, hdltreenode *);

extern boolean newidnode (tyvaluerecord, hdltreenode *);

extern boolean pushquadruplet (tytreetype, hdltreenode, hdltreenode, hdltreenode, hdltreenode, hdltreenode *);

extern boolean pushtriplet (tytreetype, hdltreenode, hdltreenode, hdltreenode, hdltreenode *);

extern boolean pushbinaryoperation (tytreetype, hdltreenode, hdltreenode, hdltreenode *);

extern boolean pushunaryoperation (tytreetype, hdltreenode, hdltreenode *);

extern boolean pushoperation (tytreetype, hdltreenode *);

extern boolean pushloop (hdltreenode, hdltreenode, hdltreenode, hdltreenode *);

extern boolean pushloopbody (hdltreenode, hdltreenode);

extern boolean pushunarystatementlist (hdltreenode, hdltreenode);

extern boolean pushtripletstatementlists (hdltreenode, hdltreenode, hdltreenode);

extern boolean pushfunctioncall (hdltreenode, hdltreenode, hdltreenode *);

extern boolean pushkernelcall (hdltreenode, hdltreenode *);

extern boolean pushfunctionreference (tyvaluerecord, hdltreenode *);

extern boolean pushlastlink (hdltreenode, hdltreenode);

extern boolean pushlastoptionallink (hdltreenode, hdltreenode, hdltreenode *);

extern boolean langpacktree (hdltreenode, Handle *);

extern boolean langunpacktree (Handle, hdltreenode *);

extern boolean langcopytree (hdltreenode, hdltreenode *);


extern boolean stringtoaddress (tyvaluerecord *); /*langvalue.c*/

extern boolean stringisboolean (bigstring bs, boolean *flboolean);

extern void pullstringvalue (const tyvaluerecord *, bigstring);

extern boolean setaddressencoding (tyvaluerecord *, boolean);

extern boolean coercebinaryval (tyvaluerecord *, tyvaluetype, long, tyvaluetype);

extern boolean getobjspecparam (hdltreenode, short, tyvaluerecord *);

extern boolean newheapvalue (ptrvoid, long, tyvaluetype, tyvaluerecord *);

extern boolean langcoerceerror (tyvaluerecord *, tyvaluetype);

extern boolean langgetdottedsymbolval (hdltreenode, hdlhashtable *, bigstring, tyvaluerecord *, hdlhashnode *);

extern boolean idvalue (hdltreenode, tyvaluerecord *);

extern boolean dotvalue (hdltreenode, tyvaluerecord *);

extern boolean addressofvalue (hdltreenode, tyvaluerecord *);

extern boolean dereferencevalue (hdltreenode, tyvaluerecord *);

extern boolean assignvalue (hdltreenode, tyvaluerecord);

extern boolean deletevalue (hdltreenode, tyvaluerecord *);

extern boolean modifyassignvalue (hdltreenode, tyvaluerecord, tytreetype, tyvaluerecord *, boolean);

extern boolean arrayvalue (hdltreenode, tyvaluerecord *);

extern boolean incrementvalue (boolean, boolean, hdltreenode, tyvaluerecord *);

extern boolean langgetnodecode (hdlhashtable, bigstring, hdlhashnode, hdltreenode *);

extern boolean evaluatereadonlyparam (hdltreenode hparam, tyvaluerecord *vparam);


extern boolean langinitbuiltins (void); /*langverbs.h*/

extern boolean langreleasesemaphores (struct typrocessrecord **);


// langxml.c

extern boolean langisremotefunction (hdltreenode htree);

extern boolean langremotefunctioncall (hdltreenode htree, hdltreenode hparam1, tyvaluerecord *vreturned);

// lang.c

extern boolean langrunscriptcode (hdlhashtable htable, bigstring bsverb, hdltreenode hcode, tyvaluerecord *vparams, hdlhashtable hcontext, tyvaluerecord *vreturned);


