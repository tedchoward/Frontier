
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

#include "frontier.r"
#include "kernelverbdefs.h"


type 'EFP#' {

	integer = $$countof(efps);
	
	wide array efps {
		
		pstring;			/*function processor name*/
		
		integer;			/*verbs require window?*/
		
		integer = $$countof(verbs);
		
		array verbs {
			
			pstring;		/*individual verb name*/
			};
		};
	};


resource 'EFP#' (idopverbs, "op, opattributes, script, osa") {
	
	{
	
	"op", true, {
		
		"getlinetext",
		"level",
		"countsubs",
		"countsummits",
		"go",
		"firstsummit",
		"expand",
		"collapse",
		"subsexpanded",
		"insert",
		"find",
		"sort",
		"setlinetext",
		"reorg",
		"promote",
		"demote",
		"hoist",
		"dehoist",
		"deletesubs",
		"deleteline",
		"tabkeyreorg",
		"flatcursorkeys",
		"getdisplay",
		"setdisplay",
		"getcursor",
		"setcursor",
		"getrefcon",
		"setrefcon",
		"getexpansionstate",
		"setexpansionstate",
		"getscrollstate",
		"setscrollstate",
		"getsuboutline",
		"insertoutline",
		"setmodified",				/*7.0b5 PBS*/
		"getselection",				/*7.0b15 PBS*/
		"index",					/*7.0b17 PBS*/
		"visitall",					/*7.0b17 PBS*/
		"getselectedsuboutlines",	/*7.0b18 PBS*/
		"xmltooutline",				/*7.0b21 PBS*/
		"outlinetoxml",				/*7.0b21 PBS*/
		"sethtmlformatting",
		"gethtmlformatting",
		"setdynamic",
		"getdynamic"
		},
	
	"opattributes", true, {
		
		"addgroup",
		"getall",
		"getone",
		"makeempty",
		"setone"
		},
	
	"script", true, {
		
		"compile",
		"uncompile",
		"getcode",
		"getlanguage",
		"setlanguage",
		"makecomment",
		"uncomment",
		"iscomment",
		"getbreakpoint",
		"setbreakpoint",
		"clearbreakpoint",
		"startprofile",
		"stopprofile"
		},
	
	"osa", false, {
		
		"compile",
		"getsource"
		}
	}
};


resource 'EFP#' (idtableverbs, "table") {
	
	{
	"table", true, {
		
		"move",
		"copy",
		"rename",
		"moveandrename",
		
		/*
		"lock",
		"islocked",
		*/
		
		"assign",
		"validate",
		"sortby",
		"getcursor",
		"getselection",
		"go",
		"goto",
		"gotoname",
		"jettison",
		"packtable",
		"emptytable",
		"getdisplaysettings",
		"setdisplaysettings",
		"getsortorder"
		}
	}
};


resource 'EFP#' (idmenuverbs, "menu") {
	
	{
	"menu", true, {
		
		"zoomscript",
		"buildmenubar",
		"clearmenubar",
		"isinstalled",
		"install",
		"remove",
		"getscript",
		"setscript",
		"addmenucommand",
		"deletemenucommand",
		"addsubmenu",
		"deletesubmenu",
		"getcommandkey",
		"setcommandkey"
		}
	}
};


resource 'EFP#' (idwpverbs, "wp") {
	
	{
	"wp", true, {
		
		"intextmode",
		"settextmode",
		"gettext",
		"settext",
		"getseltext",
		"getdisplay",
		"setdisplay",
		"getruler",
		"setruler",
		"getindent",
		"setindent",
		"getleftmargin",
		"setleftmargin",
		"getrightmargin",
		"setrightmargin",
		"setspacing",
		"setjustification",
		"settab",
		"cleartabs",
		"getselect",
		"setselect",
		"insert",
		
		#ifdef flvariables
		
		"newvariable",
		"deletevariable",
		"getnthvariable",
		"setvariablevalue",
		"getvariablevalue",
		"insertvariable",
		"getevaluate",
		"setevaluate",
		"hilitevariables",
		
		#endif
		
		"rulerlength",
		"go",
		"selectword",
		"selectline",
		"selectparagraph"
		}
	}
};


resource 'EFP#' (idpictverbs, "pict") {
	
	{
	"pict", true, {
		
		"scheduleupdate",
		"expressions",
		"getpicture",
		"setpicture"
		
		/*
		"getbounds"
		*/
		}
	}
};


resource 'EFP#' (idlangverbs, "lang") {
	
	{
	"lang", true, {
		
		"scripterror",
		"new",
		"delete",
		"edit",
		"close",
		"timecreated",
		"timemodified",
		"settimecreated",
		"settimemodified",
		"boolean", 
		
		"char",
		"short",
		"long",
		"date",
		"direction",
		"string4",
		"string",
		"displaystring",
		"address",
		"binary",
		"getbinarytype",
		"setbinarytype",
		"point",
		"rect",
		"rgb",
		"pattern",
		"fixed",
		"single",
		"double",
		"filespec",
		"alias",
		"list",
		"record",
		"enum",
		"memavail",
		"flushmemory",
		"random",
		"evaluate",
		"evaluatethread",
		"rollbeachball",
		"abs",
		"seteventtimeout",
		"seteventtransactionid",
		"seteventinteraction",
		"geteventattribute",
		"coerceappleitem",
		"getapplelistitem",
		"putapplelistitem",
		"countapplelistitems",
		"systemevent",
		"DDEevent",
		"transactionEvent",
		"msg",
		"callxcmd",
		"calldll",
		"packwindow",
		"unpackwindow",
		"callscript"
		},
	
	"clock", false, {
		
		"now",
		"set",
		"sleepfor",
		"ticks",
		"milliseconds",  /* 2005-01-08 SMD */
		"waitseconds",
		"waitsixtieths"
		},
	
	"date", false, {
		
		"get",
		"set",
		"abbrevstring",
		"dayofweek",
		"daysinmonth",
		"daystring",
		"firstofmonth",
		"lastofmonth",
		"longstring",
		"nextmonth",
		"nextweek",
		"nextyear",
		"prevmonth",
		"prevweek",
		"prevyear",
		"shortstring",
		"tomorrow",
		"weeksinmonth",
		"yesterday",
		"getcurrenttimezone",
		"netstandardstring",
		"monthtostring",
		"dayofweektostring",
		"versionlessthan",
		"day",			/* 2005-04-07 SMD (day, month, year, hour, minute, second) */
		"month",
		"year",
		"hour",
		"minute",
		"seconds"
		},
	
	"dialog", true, {
		
		"alert",
		"run",
		"runmodeless",
		"runcard",
		"runmodalcard",
		"ismodalcard",
		"setmodalcardtimeout",
		"getvalue",
		"setvalue",
		"setitemenable",
		"showitem",
		"hideitem",
		"twoway",
		"threeway",
		"ask",
		"getint",
		"notify",
		"getuserinfo",
		"getpassword",
		},
	
	"kb", false, {
		
		"optionkey",
		"cmdkey",
		"shiftkey",
		"controlkey",
		},
	
	"mouse", false, {
		
		"button",
		"location",
		},
	
	"point", false, {
		
		"get",
		"set",
		},
	
	"rectangle", false, {
		
		"get",
		"set",
		},
	
	"rgb", false, {
		
		"get",
		"set",
		},
	
	"speaker", false, {
		
		"beep",
		"sound",
		"playnamedsound",
		},
	
	"target", true, {
		
		"get",
		"set",
		"clear",
		},
	
	"bit", false, {
		
		"get",
		"set",
		"clear",
		"logicaland",
		"logicalor",
		"logicalxor",
		"shiftleft",
		"shiftright",
		},
		
	"semaphore", false, {
		
		"lock",
		"unlock",
		},
		
	"base64", false, {
		
		"encode",
		"decode",
		},
	
	"tcp", false, {
		
		"addressdecode",
		"addressencode",
		"addresstoname",
		"nametoaddress",
		"myaddress",
		"abortstream",
		"closestream",
		"closelisten",
		"openaddrstream",
		"opennamestream",
		"readstream",
		"writestream",
		"listenstream",
		"statusstream",
		"getpeeraddress",
		"getpeerport",
		"writestringtostream",
		"writefiletostream",
		"readstreamuntil",
		"readstreambytes",
		"readstreamuntilclosed",
		"getstats",
		"countconnections",
		},

	"dll", false, {
		
		"call",
		"load",
		"unload",
		"isloaded",
		},
	
	"python", false, {
	
		"doscript",
		},
	
	"htmlcontrol", false, {
		"bacK",
		"forward",
		"refresh",
		"home",
		"stop",
		"navigate",
		"isoffline",
		"setoffline",
		},
	
	"statusbar", false, {
		"msg",
		"setsections",
		"getsections",
		"getsectionone",
		"getmessage",
		},
	
	"winregistry", false, {
		"delete",
		"read",
		"gettype",
		"write",
		}		
	}
};


resource 'EFP#' (idstringverbs, "string") {
	
	{
	"string", false, {
		
		"delete",
		"insert",
		"popleading",
		"poptrailing",
		"trimwhitespace",
		"popsuffix",
		"hassuffix",
		"mid",
		"nthchar",
		"nthfield",
		"countfields",
		"setwordchar",
		"getwordchar",
		"firstword",
		"lastword",
		"nthword",
		"countwords",
		"commentdelete",
		"firstsentence",
		"patternmatch",
		"hex",
		"timestring",
		"datestring",
		"upper",
		"lower",
		"filledstring",
		"addcommas",
		"replace",
		"replaceall",
		"length",
		"isalpha",
		"isnumeric",
		"ispunctuation",
		"processhtmlmacros",
		"urldecode",
		"urlencode",
		"parsehttpargs",
		"iso8859encode",
		"getgifheightwidth",
		"getjpegheightwidth",
		"wrap",
		"davenetmassager",
		"parseaddress",
		"dropnonalphas",
		"padwithzeros",
		"ellipsize",
		"innercasename",
		"urlsplit",
		"hashMD5",
		"latintomac",
		"mactolatin",
		"utf16toansi",
		"utf8toansi",
		"ansitoutf8",
		"ansitoutf16",
		"multipleReplaceAll",
		"macromantoutf8", // 2006-02-25 creedon
		"utf8tomacroman" // 2006-02-25 creedon
		}
	}
};


resource 'EFP#' (idfileverbs, "file") {
	
	{
	"file", false, {
		
		"created",
		"modified",
		"type",
		"creator",
		"setcreated",
		"setmodified",
		"settype",
		"setcreator",
		"isfolder",
		"isvolume",
		"islocked",
		"lock",
		"unlock",
		"copy",
		"copydatafork",
		"copyresourcefork",
		"delete",
		"rename",
		"exists",
		"size",
		"fullpath",
		"getpath",
		"setpath",
		"filefrompath",
		"folderfrompath",
		"getsystemfolderpath",
		"getspecialfolderpath",
		"new",
		"newfolder",
		"newalias",
		"getfiledialog",
		"putfiledialog",
		"getfolderdialog",
		"getdiskdialog",
		"geticonpos",
		"seticonpos",
		"getversion",
		"setversion",
		"getfullversion",
		"setfullversion",
		"getcomment",
		"setcomment",
		"getlabel",
		"setlabel",
		"findapplication",
		"isbusy",
		"hasbundle",
		"setbundle",
		"isalias",
		"isvisible",
		"setvisible",
		"followalias",
		"move",
		"eject",
		"isejectable",
		"freespaceonvolume",
		"volumesize",
		"volumeblocksize",
		"filesonvolume",
		"foldersonvolume",
		"unmountvolume",
		"mountservervolume",
		"findinfile",
		"countlines",
		"open",
		"close",
		"endoffile",
		"setendoffile",
		"getendoffile",
		"setposition",
		"getposition",
		"readline",
		"writeline",
		"read",
		"write",
		"compare",
		"writewholefile",
		"getpathchar",
		"freespaceonvolumedouble",
		"volumesizedouble",
		"getmp3info",
		"readwholefile"		/* 2006-04-11 aradke */
		}
	}
};


resource 'EFP#' (idrezverbs, "rez") {
	
	{
	"rez", false, {
		
		"getresource",
		"putresource",
		"getnamedresource",
		"putnamedresource",
		"countrestypes",
		"getnthrestype",
		"countresources",
		"getnthresource",
		"getnthresinfo",
		"resourceexists",
		"namedresourceexists",
		"deleteresource",
		"deletenamedresource",
		"getresourceattributes",
		"setresourceattributes"
		
		}
	}
};


resource 'EFP#' (idwindowverbs, "window") {
	
	{
	"window", true, {
		
		"isopen",
		"open",
		"isfront",
		"bringtofront",
		"sendtoback",
		"frontmost",
		"next",
		"isvisible",
		"show",
		"hide",
		"close",
		"update",
		"ismenuscript",
		"getposition",
		"setposition",
		"getsize",
		"setsize",
		"zoom",
		"runselection",
		"scroll",
		"msg",
		"dbstats",
		"quickscript",
		"ismodified",
		"setmodified",
		"gettitle",
		"settitle",
		"about",
		"quicktime", /*7.0b4 PBS: QuickTime Player window*/
		"getfile",
		"isreadonly" /*7.0b6 PBS: is a window read-only*/
		}
	}
};


resource 'EFP#' (idsearchverbs, "search") {
	
	{
	"search", true, {
		
		"reset",
		"findnext",
		"replace",
		"replaceall"
		}
	}
};


resource 'EFP#' (idfilemenuverbs, "filemenu") {
	
	{
	"filemenu", true, {
		
		"new",
		"open",
		"close",
		"closeall",
		"save",
		"savecopy", /* 2006-02-05 aradke: re-enabled, don't change order of tokens */
		"revert",
		"print",
		"quit",
		"saveas" /* 2005-10-26 creedon */
		}
	}
};


resource 'EFP#' (ideditmenuverbs, "editmenu") {
	
	{
	"editmenu", true, {
		
		"undo",
		"cut",
		"copy",
		"paste",
		"clear",
		"selectall",
		"getfont",
		"getfontsize",
		"setfont",
		"setfontsize",
		"plaintext",
		"setbold",
		"setitalic",
		"setunderline",
		"setoutline",
		"setshadow"
		}
	}
};


resource 'EFP#' (idsysverbs, "sys") {
	
	{
	"sys", false, {
		
		"osversion", /*sys*/
		
		"systemtask",
		"browsenetwork",
		"appisrunning",
		"frontmostapp",
		"bringapptofront",
		"countapps",
		"getnthapp",
		"getapppath",
		"memavail",
		"machine",
		"os",
		"getenvironmentvariable",
		"setenvironmentvariable",
		"unixshellcommand",
		"winshellcommand"		/* 2006-03-09 aradke */
		}
	}
};


resource 'EFP#' (idlaunchverbs, "launch") {
	
	{
	"launch", false, {
		
		"applemenu",
		"application",
		"appwithdocument",
		"resource",
		"anything"
		}
	}
};


resource 'EFP#' (idclipboardverbs, "clipboard") {
	
	{
	"clipboard", false, {
		
		"get",
		"put"
		}
	}
};


resource 'EFP#' (idfrontierverbs, "frontier") {
	
	{
	"frontier", true, {
		
		"getprogrampath",
		"getfilepath",
		"enableagents",
		"requesttofront",
		"isruntime",
		"countthreads",
		"ispowerpc",
		"reclaimmemory",
		"version",
		"hashstats",
		"gethashloopcount",
		"hideapplication",
		"isvalidserialnumber",
		"showapplication"
		}
	}
};


resource 'EFP#' (idthreadverbs, "thread") {
	
	{
	"thread", true, {
		
		"exists",
		"evaluate",
		"callscript",
		"getcurrentid",
		"getcount",
		"getnthid",
		"sleep",
		"sleepfor",
		"sleepticks",
		"issleeping",
		"wake",
		"kill",
		"gettimeslice",
		"settimeslice",
		"getdefaulttimeslice",
		"setdefaulttimeslice",
		/*
		"begincritical",
		"endcritical",
		*/
		"getstats"
		}
	}
};


resource 'EFP#' (idcancoonverbs, "cancoon") {
	
	{
	"mainwindow", false, {
		
		"showflag",
		"hideflag",
		"showpopup",
		"hidepopup",
		"showbuttons",
		"hidebuttons",
		"showserverstats"
		
		/*
		"setmsg"
		*/
		}
	}
};


resource 'EFP#' (iddbverbs, "db") {
	
	{
	"db", false, {
		
		"new",
		"open",
		"save",
		"close",
		"defined",
		
	//	"gettype",
		
		"getvalue",
		"setvalue",
		"delete",
		"newTable",
		"isTable",
		"countitems",
		"getnthitem",
		"getmoddate"
		}
	}
};


resource 'EFP#' (idxmlverbs, "xml") {
	
	{
	"xml", false, {
		
		"addtable",
		"addvalue",
		"compile",
		"decompile",
		"getaddress",
		"getaddresslist",
		"getattribute",
		"getattributevalue",
		"getvalue",
		"valtostring",
		"frontiervaluetotaggedtext",
		"structtofrontiervalue",
		"getpathaddress",
		"converttodisplayname"
		}
	}
};


resource 'EFP#' (idhtmlverbs, "html") {
	
	{
	"html", false, {
		
		"processmacros",
		"urldecode",
		"urlencode",
		"parsehttpargs",
		"iso8859encode",
		"getgifheightwidth",
		"getjpegheightwidth",
		"buildpagetable",
		"refglossary",
		"getpref",
		"getonedirective",
		"rundirective",
		"rundirectives",
		"runoutlinedirectives",
		"cleanforexport",
		"normalizename",
		"glossarypatcher",
		"expandurls",
		"traversalskip",
		"getpagetableaddress",
		"neutermacros",
		"neutertags",
		"drawcalendar"
		},
	
	"searchengine", false, {
	
		"stripmarkup",
		"deindexpage",
		"indexpage",
		"cleanindex",
		"mergeresults"
		},
	
	"mrcalendar", false, {
		
		"getaddressday",
		"getdayaddress",
		"getfirstaddress",
		"getfirstday",
		"getlastaddress",
		"getlastday",
		"getmostrecentaddress",
		"getmostrecentday",
		"getnextaddress",
		"getnextday",
		"navigate"
		},
		
	"webserver", false, {
		
		"server",
		"dispatch",
		"parseheaders",
		"parsecookies",
		"buildresponse",
		"builderrorpage",
		"getserverstring",
		},
	
	"inetd", false, {
	
		"supervisor"
		}
	}
};


resource 'EFP#' (idquicktimeverbs, "quicktime") { /*7.0b4 PBS: QuickTime player verbs*/
		
		{
		"quicktime", false, {
			
			"open",
			"play",
			"stop",
			"isPlaying"
			}
		}
	};


resource 'EFP#' (idregexpverbs, "re") { /*2003-04-22 AR: regular expression verbs*/
	
	{
	"re", false, {
		
		"compile",
		"match",
		"replace",
		"extract",
		"split",
		"join",
		"visit",
		"grep",
		"getpatterninfo",
		"expand"
		}
	}
};

resource 'EFP#' (idmathverbs, "math") { /*2004-12-29 SMD: math verbs*/
	
	{
	"math", false, {
		
		"min",
		"max",
		"sqrt"
		}
	}
};

resource 'EFP#' (idcryptverbs, "crypt") { /* 2006-03-07 creedon: crypt verbs */
	{
	"crypt", false, {
	
		"whirlpool",
		"hmacMD5",
		"MD5",
		"SHA1",
		"hmacSHA1"
		}
	}
};

