
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


data 'WDEF' (130, preload) {
	$"600E 0000 5744 4546 0082 0000 0000 0000"            /* `...WDEF.‚...... */
	$"41FA FFEE 4E71 4E71 6000 0002 4E56 0000"            /* AúÿîNqNq`...NV.. */
	$"48E7 0310 3E2E 0012 266E 000E 2C2E 0008"            /* Hç..>...&n..,... */
	$"302E 000C 0C40 0006 6260 43FA 0072 D040"            /* 0....@..b`Cú.rÐ@ */
	$"D2F1 0000 4ED1 2F06 3F07 2F0B 4EBA 03D6"            /* Òñ..NÑ/.?./.Nº.Ö */
	$"4FEF 000A 6044 2F06 3F07 2F0B 4EBA 0502"            /* Oï.Â`D/.?./.Nº.. */
	$"2D40 0014 6038 2F0B 4EBA 02D0 588F 602A"            /* -@..`8/.Nº.ÐX`* */
	$"2F0B 4EBA 0686 588F 6020 2F0B 4EBA 06A4"            /* /.Nº.†X` /.Nº.¤ */
	$"588F 6016 2F06 3F07 4EBA 0572 5C8F 600A"            /* X`./.?.Nº.r\`Â */
	$"3F07 2F0B 4EBA 05DA 5C8F 42AE 0014 4CEE"            /* ?./.Nº.Ú\B®..Lî */
	$"08C0 FFF4 4E5E 205F 4FEF 000C 4ED0 FF98"            /* .ÀÿôN^ _Oï..NÐÿ˜ */
	$"FFA8 FFB8 FFC2 FFCC FFD6 FFE2 4E56 0000"            /* ÿ¨ÿ¸ÿÂÿÌÿÖÿâNV.. */
	$"302E 0008 48C0 81FC 0002 4840 4E5E 4E75"            /* 0...HÀü..H@N^Nu */
	$"4E56 0000 2F2E 0008 A870 206E 0008 4868"            /* NV../...¨p n..Hh */
	$"0004 A870 4E5E 4E75 4E56 0000 2F2E 0008"            /* ..¨pN^NuNV../... */
	$"A871 206E 0008 4868 0004 A871 4E5E 4E75"            /* ¨q n..Hh..¨qN^Nu */
	$"4E56 FFF4 48E7 1F10 266B 0086 486E FFF4"            /* NVÿôHç..&k.†Hnÿô */
	$"A874 206E FFF4 3E28 0044 3C28 004A 7A00"            /* ¨t nÿô>(.D<(.Jz. */
	$"1A28 0046 3F3C 0003 A887 3F3C 0009 A88A"            /* .(.F?<..¨‡?<.Æ¨Š */
	$"4267 A888 486E FFF8 A88B 204B A029 4267"            /* Bg¨ˆHnÿø¨‹ K )Bg */
	$"2F13 A88C 382E 0012 986E 000E 985F E244"            /* /.¨Œ8...˜n..˜_âD */
	$"D86E 000E 362E 0010 966E 000C 966E FFF8"            /* Øn..6...–n..–nÿø */
	$"966E FFFA E243 302E 000C D06E FFF8 D640"            /* –nÿúâC0...ÐnÿøÖ@ */
	$"3003 906E FFF8 3D40 000C 302E FFFA D043"            /* 0.nÿø=@..0.ÿúÐC */
	$"3D40 0010 3D44 000E 4267 2F13 A88C 301F"            /* =@..=D..Bg/.¨Œ0. */
	$"D044 3D40 0012 486E 000C 2F3C 0000 FFFE"            /* ÐD=@..Hn../<..ÿþ */
	$"A8A9 486E 000C A8A3 3F04 3F03 A893 2F13"            /* ¨©Hn..¨£?.?.¨“/. */
	$"A884 204B A02A 206E FFF4 3147 0044 206E"            /* ¨„ K * nÿô1G.D n */
	$"FFF4 3146 004A 206E FFF4 1145 0046 4CDF"            /* ÿô1F.J nÿô.E.FLß */
	$"08F8 4E5E 4E75 4E56 FFF0 48E7 0030 266E"            /* .øN^NuNVÿðHç.0&n */
	$"0008 47EB 0082 2453 2053 A029 2052 2D50"            /* ..Gë.‚$S S ) R-P */
	$"FFF0 2D68 0004 FFF4 4A6E 000E 6620 486E"            /* ÿð-h..ÿôJn..f Hn */
	$"FFF0 A8A1 486E FFF0 2F3C 0001 0001 A8A9"            /* ÿð¨¡Hnÿð/<....¨© */
	$"486E FFF0 A8A3 2053 A02A 6000 0088 3F2E"            /* Hnÿð¨£ S *`..ˆ?. */
	$"FFF0 4EBA FE98 4A40 548F 6726 3F2E FFF2"            /* ÿðNºþ˜J@Tg&?.ÿò */
	$"4EBA FE8A 4A40 548F 670C 486E FFF8 487A"            /* NºþŠJ@Tg.HnÿøHz */
	$"00A2 A866 6030 486E FFF8 487A 0084 A866"            /* .¢¨f`0HnÿøHz.„¨f */
	$"6024 3F2E FFF2 4EBA FE64 4A40 548F 670C"            /* `$?.ÿòNºþdJ@Tg. */
	$"486E FFF8 487A 0058 A866 600A 486E FFF8"            /* HnÿøHz.X¨f`ÂHnÿø */
	$"487A 003A A866 486E FFF0 2F3C 0001 0001"            /* Hz.:¨fHnÿð/<.... */
	$"A8A9 486E FFF0 486E FFF8 A8A5 2F12 A8A1"            /* ¨©HnÿðHnÿø¨¥/.¨¡ */
	$"2052 4868 0008 A8A3 2052 4868 0008 A8A1"            /*  RHh..¨£ RHh..¨¡ */
	$"2053 A02A 4CDF 0C00 4E5E 4E75 1041 4130"            /*  S *Lß..N^Nu.AA0 */
	$"3041 4130 3041 4130 3041 4130 3000 1035"            /* 0AA00AA00AA00..5 */
	$"3530 3035 3530 3035 3530 3035 3530 3000"            /* 500550055005500. */
	$"1030 3041 4130 3041 4130 3041 4130 3041"            /* .00AA00AA00AA00A */
	$"4100 1030 3035 3530 3035 3530 3035 3530"            /* A..0055005500550 */
	$"3035 3500 4E56 0000 48E7 0030 206E 0008"            /* 055.NV..Hç.0 n.. */
	$"2668 0072 42A7 A8D8 245F 2F0B 486E 000C"            /* &h.rB§¨Ø$_/.Hn.. */
	$"A8DF 486E 000C 2F3C 0001 0001 A8A8 2F0A"            /* ¨ßHn../<....¨¨/Â */
	$"486E 000C A8DF 2F0A 2F0B 2F0B A8E5 2F0A"            /* Hn..¨ß/Â/./.¨å/Â */
	$"A8D9 4CDF 0C00 4E5E 4E75 4E56 FFF0 48E7"            /* ¨ÙLß..N^NuNVÿðHç */
	$"0130 246E 0008 47EA 0082 2053 A029 2E13"            /* .0$n..Gê.‚ S ).. */
	$"2D6A 0010 FFF8 2D6A 0014 FFFC 4A6A 0006"            /* -j..ÿø-j..ÿüJj.. */
	$"6F0E 2D6A 0008 FFF0 2D6A 000C FFF4 6012"            /* o.-j..ÿð-j..ÿô`. */
	$"206A 0002 2050 2D68 0006 FFF0 2D68 000A"            /*  j.. P-h..ÿð-h.Â */
	$"FFF4 486E FFF8 302E FFF2 4440 3F00 302E"            /* ÿôHnÿø0.ÿòD@?.0. */
	$"FFF0 4440 3F00 A8A8 2F2A 0076 486E FFF8"            /* ÿðD@?.¨¨/*.vHnÿø */
	$"A8DF 70FF D06E FFFA 3D40 FFFA 70F1 D06E"            /* ¨ßpÿÐnÿú=@ÿúpñÐn */
	$"FFF8 3D40 FFF8 7001 D06E FFFE 3D40 FFFE"            /* ÿø=@ÿøp.Ðnÿþ=@ÿþ */
	$"7001 D06E FFFC 3D40 FFFC 2F2E FFFC 2F2E"            /* p.Ðnÿü=@ÿü/.ÿü/. */
	$"FFF8 2F0A 4EBA FF1E 700F D06E FFF8 3D40"            /* ÿø/ÂNºÿ.p.Ðnÿø=@ */
	$"FFFC 2047 2050 20AE FFF8 216E FFFC 0004"            /* ÿü G P ®ÿø!nÿü.. */
	$"486E FFF8 2F3C 0002 0006 A8A9 700B D06E"            /* Hnÿø/<....¨©p.Ðn */
	$"FFFA 3D40 FFFE 2047 2050 216E FFF8 0008"            /* ÿú=@ÿþ G P!nÿø.. */
	$"216E FFFC 000C 2053 A02A 4CEE 0C80 FFE4"            /* !nÿü.. S *Lî.€ÿä */
	$"4E5E 4E75 4E56 FFF0 48E7 0130 266E 0008"            /* N^NuNVÿðHç.0&n.. */
	$"2E2E 000E 4A2B 006E 6700 00AE 246B 0082"            /* ....J+.ng..®$k.‚ */
	$"4267 2F07 A86B 3E1F 48C7 7004 B087 6618"            /* Bg/.¨k>.HÇp.°‡f. */
	$"206B 0082 A029 2052 4868 0008 A8A4 206B"            /*  k.‚ ) RHh..¨¤ k */
	$"0082 A02A 6000 0082 4A87 667C 2F0B 4EBA"            /* .‚ *`..‚J‡f|/.Nº */
	$"FECA 7000 102B 006F 3E80 3F2E 000C 2F0B"            /* þÊp..+.o>€?.../. */
	$"4EBA FD54 486E FFF0 487A 0068 A866 486E"            /* NºýTHnÿðHz.h¨fHn */
	$"FFF0 A89D 2EAB 0072 A8D2 206B 0072 2050"            /* ÿð¨.«.r¨Ò k.r P */
	$"2D68 0002 FFF8 2D68 0006 FFFC 3EAE FFFA"            /* -h..ÿø-h..ÿü>®ÿú */
	$"70FE D06E FFFC 3F00 A893 70FE D06E FFFE"            /* pþÐnÿü?.¨“pþÐnÿþ */
	$"3E80 70FE D06E FFFC 3F00 A891 70FE D06E"            /* >€pþÐnÿü?.¨‘pþÐn */
	$"FFFE 3E80 3F2E FFF8 A891 2052 2F28 0004"            /* ÿþ>€?.ÿø¨‘ R/(.. */
	$"2F10 2F0B 4EBA FC1A 4CEE 0C80 FFE4 4E5E"            /* /./.Nºü.Lî.€ÿäN^ */
	$"4E75 1046 4646 4646 4646 4646 4646 4646"            /* Nu.FFFFFFFFFFFFF */
	$"4646 4600 4E56 FFFC 2F0B 266E 000C 206E"            /* FFF.NVÿü/.&n.. n */
	$"0008 26A8 0010 2768 0014 0004 70F4 D06B"            /* ..&¨..'h....pôÐk */
	$"0004 3680 70F4 D06B 0006 3740 0002 7001"            /* ..6€pôÐk..7@..p. */
	$"D06B 0004 3740 0004 7001 D06B 0006 3740"            /* Ðk..7@..p.Ðk..7@ */
	$"0006 486E FFFC A874 2F2E 0008 A873 2F0B"            /* ..Hnÿü¨t/...¨s/. */
	$"4EBA FB7E 2EAE FFFC A873 265F 4E5E 4E75"            /* Nºû~.®ÿü¨s&_N^Nu */
	$"4E56 FFF4 48E7 0030 266E 0008 2F0B 4EBA"            /* NVÿôHç.0&n../.Nº */
	$"FDCA 4257 2F2E 000E A86B 3D5F FFFE 4257"            /* ýÊBW/...¨k=_ÿþBW */
	$"2F2E 000E A86A 3D5F FFFC 246B 0082 4227"            /* /...¨j=_ÿü$k.‚B' */
	$"2F2E FFFC 2052 4868 0008 A8AD 4A1F 6704"            /* /.ÿü RHh..¨­J.g. */
	$"7004 6050 4227 2F2E FFFC 2F12 A8AD 4A1F"            /* p.`PB'/.ÿü/.¨­J. */
	$"6704 7002 603E 0C6E 0002 000C 6720 486E"            /* g.p.`>.n....g Hn */
	$"FFF4 2F0B 4EBA FF3E 4217 2F2E FFFC 486E"            /* ÿô/.Nºÿ>B./.ÿüHn */
	$"FFF4 A8AD 4A1F 5C8F 6704 7003 6016 4227"            /* ÿô¨­J.\g.p.`.B' */
	$"2F2E FFFC 2F2B 0076 A8E8 4A1F 6704 7001"            /* /.ÿü/+.v¨èJ.g.p. */
	$"6002 7000 4CDF 0C00 4E5E 4E75 4E56 FFF8"            /* `.p.Lß..N^NuNVÿø */
	$"2F0B 0C6E 0002 0008 6760 266E 000A 2D53"            /* /..n....g`&n.Â-S */
	$"FFF8 2D6B 0004 FFFC 70FF D06E FFFA 3D40"            /* ÿø-k..ÿüpÿÐnÿú=@ */
	$"FFFA 70F1 D06E FFF8 3D40 FFF8 7002 D06E"            /* ÿúpñÐnÿø=@ÿøp.Ðn */
	$"FFFE 3D40 FFFE 7002 D06E FFFC 3D40 FFFC"            /* ÿþ=@ÿþp.Ðnÿü=@ÿü */
	$"486E FFF8 A8A1 70F4 D06B 0006 3F00 7001"            /* Hnÿø¨¡pôÐk..?.p. */
	$"D06E FFF8 3F00 A893 70F4 D06B 0006 3F00"            /* Ðnÿø?.¨“pôÐk..?. */
	$"70FF D06E FFFC 3F00 A891 265F 4E5E 4E75"            /* pÿÐnÿü?.¨‘&_N^Nu */
	$"4E56 FFF8 0C6E 0002 000C 677A 486E FFF8"            /* NVÿø.n....gzHnÿø */
	$"2F2E 0008 4EBA FE7E 2EAE 0008 A873 486E"            /* /...Nºþ~.®..¨sHn */
	$"FFF8 4EBA FA54 A89E 486E FFF8 A8A3 486E"            /* ÿøNºúT¨žHnÿø¨£Hn */
	$"FFF8 A8A1 486E FFF8 2F3C 0002 0002 A8A9"            /* ÿø¨¡Hnÿø/<....¨© */
	$"7002 D06E FFF8 3D40 FFF8 7002 D06E FFFA"            /* p.Ðnÿø=@ÿøp.Ðnÿú */
	$"3D40 FFFA 486E FFF8 A8A1 486E FFF8 2F3C"            /* =@ÿúHnÿø¨¡Hnÿø/< */
	$"FFFE FFFE A8A8 70FF D06E FFFC 3D40 FFFC"            /* ÿþÿþ¨¨pÿÐnÿü=@ÿü */
	$"70FF D06E FFFE 3D40 FFFE 486E FFF8 A8A3"            /* pÿÐnÿþ=@ÿþHnÿø¨£ */
	$"486E FFF8 A8A1 4E5E 4E75 4E56 FFFC 2F0B"            /* Hnÿø¨¡N^NuNVÿü/. */
	$"266E 0008 47EB 0082 7010 A122 2688 2D53"            /* &n..Gë.‚p.¡"&ˆ-S */
	$"FFFC 2F2E 0008 4EBA FC22 266E FFF8 4E5E"            /* ÿü/...Nºü"&nÿøN^ */
	$"4E75 4E56 FFFC 2F0B 266E 0008 47EB 0082"            /* NuNVÿü/.&n..Gë.‚ */
	$"2D53 FFFC 2053 A023 265F 4E5E 4E75"                 /* -Sÿü S #&_N^Nu */
};

