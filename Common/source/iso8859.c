
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

/*
This mapping based on table provided by Alan Legg:

		At work: alan@asset-graphics.com
		At play: alan@damocles.demon.co.uk
		http://www.asset-graphics.com
*/

unsigned char * iso8859table [256] = {

	/*0*/	nil,

	/*1*/	nil,

	/*2*/	nil,

	/*3*/	nil,

	/*4*/	nil,

	/*5*/	nil,

	/*6*/	nil,

	/*7*/	nil,

	/*8*/	nil,

	/*9*/	nil,

	/*10*/	nil,

	/*11*/	nil,

	/*12*/	nil,

	/*13*/	nil,

	/*14*/	nil,

	/*15*/	nil,

	/*16*/	nil,

	/*17*/	nil,

	/*18*/	nil,

	/*19*/	nil,

	/*20*/	nil,

	/*21*/	nil,

	/*22*/	nil,

	/*23*/	nil,

	/*24*/	nil,

	/*25*/	nil,

	/*26*/	nil,

	/*27*/	nil,

	/*28*/	nil,

	/*29*/	nil,

	/*30*/	nil,

	/*31*/	nil,

	/*32*/	nil,

	/*33*/	nil,

	/*34*/	nil,

	/*35*/	nil,

	/*36*/	nil,

	/*37*/	nil,

	/*38*/	nil,

	/*39*/	nil,

	/*40*/	nil,

	/*41*/	nil,

	/*42*/	nil,

	/*43*/	nil,

	/*44*/	nil,

	/*45*/	nil,

	/*46*/	nil,

	/*47*/	nil,

	/*48*/	nil,

	/*49*/	nil,

	/*50*/	nil,

	/*51*/	nil,

	/*52*/	nil,

	/*53*/	nil,

	/*54*/	nil,

	/*55*/	nil,

	/*56*/	nil,

	/*57*/	nil,

	/*58*/	nil,

	/*59*/	nil,

	/*60*/	nil,

	/*61*/	nil,

	/*62*/	nil,

	/*63*/	nil,

	/*64*/	nil,

	/*65*/	nil,

	/*66*/	nil,

	/*67*/	nil,

	/*68*/	nil,

	/*69*/	nil,

	/*70*/	nil,

	/*71*/	nil,

	/*72*/	nil,

	/*73*/	nil,

	/*74*/	nil,

	/*75*/	nil,

	/*76*/	nil,

	/*77*/	nil,

	/*78*/	nil,

	/*79*/	nil,

	/*80*/	nil,

	/*81*/	nil,

	/*82*/	nil,

	/*83*/	nil,

	/*84*/	nil,

	/*85*/	nil,

	/*86*/	nil,

	/*87*/	nil,

	/*88*/	nil,

	/*89*/	nil,

	/*90*/	nil,

	/*91*/	nil,

	/*92*/	nil,

	/*93*/	nil,

	/*94*/	nil,

	/*95*/	nil,

	/*96*/	nil,

	/*97*/	nil,

	/*98*/	nil,

	/*99*/	nil,

	/*100*/	nil,

	/*101*/	nil,

	/*102*/	nil,

	/*103*/	nil,

	/*104*/	nil,

	/*105*/	nil,

	/*106*/	nil,

	/*107*/	nil,

	/*108*/	nil,

	/*109*/	nil,

	/*110*/	nil,

	/*111*/	nil,

	/*112*/	nil,

	/*113*/	nil,

	/*114*/	nil,

	/*115*/	nil,

	/*116*/	nil,

	/*117*/	nil,

	/*118*/	nil,

	/*119*/	nil,

	/*120*/	nil,

	/*121*/	nil,

	/*122*/	nil,

	/*123*/	nil,

	/*124*/	nil,

	/*125*/	nil,

	/*126*/	nil,

	/*127*/	nil,

	/*128*/ BIGSTRING ("&Auml;"),

	/*129*/ BIGSTRING ("&Aring;"),

	/*130*/ BIGSTRING ("&Ccedil;"),

	/*131*/ BIGSTRING ("&Eacute;"),

	/*132*/ BIGSTRING ("&Ntilde;"),

	/*133*/ BIGSTRING ("&Ouml;"),

	/*134*/ BIGSTRING ("&Uuml;"),

	/*135*/ BIGSTRING ("&aacute;"),

	/*136*/ BIGSTRING ("&agrave;"),

	/*137*/ BIGSTRING ("&acirc;"),

	/*138*/ BIGSTRING ("&auml;"),

	/*139*/ BIGSTRING ("&atilde;"),

	/*140*/ BIGSTRING ("&aring;"),

	/*141*/ BIGSTRING ("&ccedil;"),

	/*142*/ BIGSTRING ("&eacute;"),

	/*143*/ BIGSTRING ("&egrave;"),

	/*144*/ BIGSTRING ("&ecirc;"),

	/*145*/ BIGSTRING ("&euml;"),

	/*146*/ BIGSTRING ("&iacute;"),

	/*147*/ BIGSTRING ("&igrave;"),

	/*148*/ BIGSTRING ("&icirc;"),

	/*149*/ BIGSTRING ("&iuml;"),

	/*150*/ BIGSTRING ("&ntilde;"),

	/*151*/ BIGSTRING ("&oacute;"),

	/*152*/ BIGSTRING ("&ograve;"),

	/*153*/ BIGSTRING ("&ocirc;"),

	/*154*/ BIGSTRING ("&ouml;"),

	/*155*/ BIGSTRING ("&otilde;"),

	/*156*/ BIGSTRING ("&uacute;"),

	/*157*/ BIGSTRING ("&ugrave;"),

	/*158*/ BIGSTRING ("&ucirc;"),

	/*159*/ BIGSTRING ("&uuml;"),

	/*160*/ BIGSTRING ("[sgl dagger]"),

	/*161*/ BIGSTRING ("&#176;"),

	/*162*/ BIGSTRING ("&#162;"),

	/*163*/ BIGSTRING ("&#163;"),

	/*164*/ BIGSTRING ("&#167;"),

	/*165*/ BIGSTRING ("o"), //"[bullet]",

	/*166*/ BIGSTRING ("&#182;"),

	/*167*/ BIGSTRING ("&szlig;"),

	/*168*/ BIGSTRING ("&reg;"),

	/*169*/ BIGSTRING ("&copy;"),

	/*170*/ BIGSTRING ("[trademark]"),

	/*171*/ BIGSTRING ("&#180;"),

	/*172*/ BIGSTRING ("&#168;"),

	/*173*/ BIGSTRING ("[not equal]"),

	/*174*/ BIGSTRING ("&AElig;"),

	/*175*/ BIGSTRING ("&Oslash;"),

	/*176*/ BIGSTRING ("°"),

	/*177*/ BIGSTRING ("&#177;"),

	/*178*/ BIGSTRING ("[less equal]"),

	/*179*/ BIGSTRING ("[greater equal]"),

	/*180*/ BIGSTRING ("&#165;"),

	/*181*/ BIGSTRING ("&#181;"),

	/*182*/ BIGSTRING ("[partial diff]"),

	/*183*/ BIGSTRING ("[sigma]"),

	/*184*/ BIGSTRING ("[product]"),

	/*185*/ BIGSTRING ("[pi]"),

	/*186*/ BIGSTRING ("[integral]"),

	/*187*/ BIGSTRING ("&#186;"),

	/*188*/ BIGSTRING ("&#170;"),

	/*189*/ BIGSTRING ("[omega]"),

	/*190*/ BIGSTRING ("&aelig;"),

	/*191*/ BIGSTRING ("&oslash;"),

	/*192*/ BIGSTRING ("&#191;"),

	/*193*/ BIGSTRING ("&#161;"),

	/*194*/ BIGSTRING ("&#172;"),

	/*195*/ BIGSTRING ("[radical]"),

	/*196*/ BIGSTRING ("[florin]"),

	/*197*/ BIGSTRING ("[approx equal]"),

	/*198*/ BIGSTRING ("[delta]"),

	/*199*/ BIGSTRING ("&#171;"),

	/*200*/ BIGSTRING ("&#187;"),

	/*201*/ BIGSTRING ("..."), // "[...]",

	/*202*/ BIGSTRING ("&nbsp;"),

	/*203*/ BIGSTRING ("&Agrave;"),

	/*204*/ BIGSTRING ("&Atilde;"),

	/*205*/ BIGSTRING ("&Otilde;"),

	/*206*/ BIGSTRING ("[OE]"),

	/*207*/ BIGSTRING ("[oe]"),

	/*208*/ BIGSTRING ("-"), //"[en dash]",

	/*209*/ BIGSTRING ("--"), //"[em dash]",

	/*210*/ BIGSTRING ("\""), //"[open \"]",

	/*211*/ BIGSTRING ("\""), //"[close \"]",

	/*212*/ BIGSTRING ("'"), //"[open ']",

	/*213*/ BIGSTRING ("'"), //"[close ']",

	/*214*/ BIGSTRING ("&#247;"),

	/*215*/ BIGSTRING ("[lozenge]"),

	/*216*/ BIGSTRING ("&yuml;"),

	/*217*/ BIGSTRING ("[Y&#168;]"),

	/*218*/ BIGSTRING ("/"),

	/*219*/ BIGSTRING ("&#164;"),

	/*220*/ BIGSTRING ("[&lt;]"),

	/*221*/ BIGSTRING ("[&gt;]"),

	/*222*/ BIGSTRING ("[fi]"),

	/*223*/ BIGSTRING ("[fl]"),

	/*224*/ BIGSTRING ("[dbl dagger]"),

	/*225*/ BIGSTRING ("&#183;"),

	/*226*/ BIGSTRING ("[base ']"),

	/*227*/ BIGSTRING ("[base \"]"),

	/*228*/ BIGSTRING ("[per thou]"),

	/*229*/ BIGSTRING ("&Acirc;"),

	/*230*/ BIGSTRING ("&Ecirc;"),

	/*231*/ BIGSTRING ("&Aacute;"),

	/*232*/ BIGSTRING ("&Euml;"),

	/*233*/ BIGSTRING ("&Egrave;"),

	/*234*/ BIGSTRING ("&Iacute;"),

	/*235*/ BIGSTRING ("&Icirc;"),

	/*236*/ BIGSTRING ("&Iuml;"),

	/*237*/ BIGSTRING ("&Igrave;"),

	/*238*/ BIGSTRING ("&Oacute;"),

	/*239*/ BIGSTRING ("&Ocirc;"),

	/*240*/ BIGSTRING ("[apple]"),

	/*241*/ BIGSTRING ("&Ograve;"),

	/*242*/ BIGSTRING ("&Uacute;"),

	/*243*/ BIGSTRING ("&Ucirc;"),

	/*244*/ BIGSTRING ("&Ugrave;"),

	/*245*/ BIGSTRING ("[dotless i]"),

	/*246*/ BIGSTRING ("[^]"),

	/*247*/ BIGSTRING ("[~]"),

	/*248*/ BIGSTRING ("[macron]"),

	/*249*/ BIGSTRING ("[breve]"),

	/*250*/ BIGSTRING ("[dot accent]"),

	/*251*/ BIGSTRING ("[ring]"),

	/*252*/ BIGSTRING ("[cedilla]"),

	/*253*/ BIGSTRING ("[hungarian umlaut]"),

	/*254*/ BIGSTRING ("[ogonek]"),

	/*255*/ BIGSTRING ("[caron]")

	}; /*iso8859table*/


