
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

	/*128*/ "&Auml;",

	/*129*/ "&Aring;",

	/*130*/ "&Ccedil;",

	/*131*/ "&Eacute;",

	/*132*/ "&Ntilde;",

	/*133*/ "&Ouml;",

	/*134*/ "&Uuml;",

	/*135*/ "&aacute;",

	/*136*/ "&agrave;",

	/*137*/ "&acirc;",

	/*138*/ "&auml;",

	/*139*/ "&atilde;",

	/*140*/ "&aring;",

	/*141*/ "&ccedil;",

	/*142*/ "&eacute;",

	/*143*/ "&egrave;",

	/*144*/ "&ecirc;",

	/*145*/ "&euml;",

	/*146*/ "&iacute;",

	/*147*/ "&igrave;",

	/*148*/ "&icirc;",

	/*149*/ "&iuml;",

	/*150*/ "&ntilde;",

	/*151*/ "&oacute;",

	/*152*/ "&ograve;",

	/*153*/ "&ocirc;",

	/*154*/ "&ouml;",

	/*155*/ "&otilde;",

	/*156*/ "&uacute;",

	/*157*/ "&ugrave;",

	/*158*/ "&ucirc;",

	/*159*/ "&uuml;",

	/*160*/ "[sgl dagger]",

	/*161*/ "&#176;",

	/*162*/ "&#162;",

	/*163*/ "&#163;",

	/*164*/ "&#167;",

	/*165*/ "o", //"[bullet]",

	/*166*/ "&#182;",

	/*167*/ "&szlig;",

	/*168*/ "&reg;",

	/*169*/ "&copy;",

	/*170*/ "[trademark]",

	/*171*/ "&#180;",

	/*172*/ "&#168;",

	/*173*/ "[not equal]",

	/*174*/ "&AElig;",

	/*175*/ "&Oslash;",

	/*176*/ "°",

	/*177*/ "&#177;",

	/*178*/ "[less equal]",

	/*179*/ "[greater equal]",

	/*180*/ "&#165;",

	/*181*/ "&#181;",

	/*182*/ "[partial diff]",

	/*183*/ "[sigma]",

	/*184*/ "[product]",

	/*185*/ "[pi]",

	/*186*/ "[integral]",

	/*187*/ "&#186;",

	/*188*/ "&#170;",

	/*189*/ "[omega]",

	/*190*/ "&aelig;",

	/*191*/ "&oslash;",

	/*192*/ "&#191;",

	/*193*/ "&#161;",

	/*194*/ "&#172;",

	/*195*/ "[radical]",

	/*196*/ "[florin]",

	/*197*/ "[approx equal]",

	/*198*/ "[delta]",

	/*199*/ "&#171;",

	/*200*/ "&#187;",

	/*201*/ "...", // "[...]",

	/*202*/ "&nbsp;",

	/*203*/ "&Agrave;",

	/*204*/ "&Atilde;",

	/*205*/ "&Otilde;",

	/*206*/ "[OE]",

	/*207*/ "[oe]",

	/*208*/ "-", //"[en dash]",

	/*209*/ "--", //"[em dash]",

	/*210*/ "\"", //"[open \"]",

	/*211*/ "\"", //"[close \"]",

	/*212*/ "'", //"[open ']",

	/*213*/ "'", //"[close ']",

	/*214*/ "&#247;",

	/*215*/ "[lozenge]",

	/*216*/ "&yuml;",

	/*217*/ "[Y&#168;]",

	/*218*/ "/",

	/*219*/ "&#164;",

	/*220*/ "[&lt;]",

	/*221*/ "[&gt;]",

	/*222*/ "[fi]",

	/*223*/ "[fl]",

	/*224*/ "[dbl dagger]",

	/*225*/ "&#183;",

	/*226*/ "[base ']",

	/*227*/ "[base \"]",

	/*228*/ "[per thou]",

	/*229*/ "&Acirc;",

	/*230*/ "&Ecirc;",

	/*231*/ "&Aacute;",

	/*232*/ "&Euml;",

	/*233*/ "&Egrave;",

	/*234*/ "&Iacute;",

	/*235*/ "&Icirc;",

	/*236*/ "&Iuml;",

	/*237*/ "&Igrave;",

	/*238*/ "&Oacute;",

	/*239*/ "&Ocirc;",

	/*240*/ "[apple]",

	/*241*/ "&Ograve;",

	/*242*/ "&Uacute;",

	/*243*/ "&Ucirc;",

	/*244*/ "&Ugrave;",

	/*245*/ "[dotless i]",

	/*246*/ "[^]",

	/*247*/ "[~]",

	/*248*/ "[macron]",

	/*249*/ "[breve]",

	/*250*/ "[dot accent]",

	/*251*/ "[ring]",

	/*252*/ "[cedilla]",

	/*253*/ "[hungarian umlaut]",

	/*254*/ "[ogonek]",

	/*255*/ "[caron]"

	}; /*iso8859table*/


