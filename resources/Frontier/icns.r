
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


data 'icns' (128) {
	$"6963 6E73 0000 CEEE 6963 7323 0000 0048"            /* icns..Îîics#...H */
	$"00E0 0170 1AF0 3EFC 2DFE 7DFE 7DFE 5FFE"            /* .à.p.ð>ü-þ}þ}þ_þ */
	$"3BFE 1BFE 0BFC 0BF8 0BC0 0BC0 0BC0 1FF8"            /* ;þ.þ.ü.ø.À.À.À.ø */
	$"00E0 01F0 1BF0 3FFC 3FFE 7FFE 7FFE 7FFE"            /* .à.ð.ð?ü?þ.þ.þ.þ */
	$"3FFE 1FFE 0FFC 0FF8 0FC0 0FC0 0FC0 1FF8"            /* ?þ.þ.ü.ø.À.À.À.ø */
	$"6963 7334 0000 0088 0000 0000 FFF0 0000"            /* ics4...ˆ....ÿð.. */
	$"0000 000F 88AF 0000 000F F0FB 88AF 0000"            /* ....ˆ¯....ðûˆ¯.. */
	$"00FB 8F98 889F FF00 00FB 8FB8 889F 8AF0"            /* .û˜ˆŸÿ..û¸ˆŸŠð */
	$"0FB8 AFB8 8AFB 88F0 0FB8 A9B8 8AFB 88F0"            /* .¸¯¸Šûˆð.¸©¸Šûˆð */
	$"0FB8 8888 8F98 88F0 00F8 AB88 A888 88F0"            /* .¸ˆˆ˜ˆð.ø«ˆ¨ˆˆð */
	$"000F 9B88 8888 8AF0 0000 FB88 888A 9F00"            /* ..›ˆˆˆŠð..ûˆˆŠŸ. */
	$"0000 FB88 A9FF F000 0000 FB88 AF00 0000"            /* ..ûˆ©ÿð...ûˆ¯... */
	$"0000 FB88 AF00 0000 0000 FB88 AF00 0000"            /* ..ûˆ¯.....ûˆ¯... */
	$"000F FFFF FFFF F000 6963 7338 0000 0108"            /* ..ÿÿÿÿð.ics8.... */
	$"0000 0000 0000 0000 FFFF FF00 0000 0000"            /* ........ÿÿÿ..... */
	$"0000 0000 0000 00FF 7D7D 7DFF 0000 0000"            /* .......ÿ}}}ÿ.... */
	$"0000 00FF FF00 FF58 7D7D 7DFF 0000 0000"            /* ...ÿÿ.ÿX}}}ÿ.... */
	$"0000 FF7C 7DFF A77D 7D7D A7FF FFFF 0000"            /* ..ÿ|}ÿ§}}}§ÿÿÿ.. */
	$"0000 FF7C 7DFF 7C7D 7D7D A7FF 7D7D FF00"            /* ..ÿ|}ÿ|}}}§ÿ}}ÿ. */
	$"00FF 827D 7DFF 527D 7D7D FF7C 7D7D FF00"            /* .ÿ‚}}ÿR}}}ÿ|}}ÿ. */
	$"00FF 7C7D 7DA7 7C7D 7D83 FF7C 7D7D FF00"            /* .ÿ|}}§|}}ƒÿ|}}ÿ. */
	$"00FF 7C7D 7D7D 7D7D 7DFF A77D 7D7D FF00"            /* .ÿ|}}}}}}ÿ§}}}ÿ. */
	$"0000 FF7D 7D7C 7D7D 7D7D 7D7D 7D7D FF00"            /* ..ÿ}}|}}}}}}}}ÿ. */
	$"0000 00FF A758 7D7D 7D7D 7D7D 7D7D FF00"            /* ...ÿ§X}}}}}}}}ÿ. */
	$"0000 0000 FF58 7D7D 7D7D 7D7D A7FF 0000"            /* ....ÿX}}}}}}§ÿ.. */
	$"0000 0000 FF52 7D7D 7DA7 FFFF FF00 0000"            /* ....ÿR}}}§ÿÿÿ... */
	$"0000 0000 FF52 7D7D 83FF 0000 0000 0000"            /* ....ÿR}}ƒÿ...... */
	$"0000 0000 FF52 7D7D 83FF 0000 0000 0000"            /* ....ÿR}}ƒÿ...... */
	$"0000 0000 FF58 7D7D 83FF 0000 0000 0000"            /* ....ÿX}}ƒÿ...... */
	$"0000 00FF FFFF FFFF FFFF FFFF FF00 0000"            /* ...ÿÿÿÿÿÿÿÿÿÿ... */
	$"6973 3332 0000 0204 85FF 8000 89FF 0400"            /* is32....…ÿ€.‰ÿ.. */
	$"7766 5900 84FF 0800 00FF 0083 6666 5600"            /* wfY.„ÿ...ÿ.ƒffV. */
	$"83FF 0800 6C66 004B 7466 664F 8000 81FF"            /* ƒÿ..lf.KtffO€.ÿ */
	$"1800 7964 0077 6666 6146 0066 5C00 FFFF"            /* ..yd.wffaF.f\.ÿÿ */
	$"005E 665E 0088 6666 5B00 8066 0C00 FFFF"            /* .^f^.ˆff[.€f..ÿÿ */
	$"0074 665B 437C 6666 5200 8066 1000 FFFF"            /* .tf[C|ffR.€f..ÿÿ */
	$"007F 6666 5E66 6664 003D 6966 6600 80FF"            /* ..ff^ffd.=iff.€ÿ */
	$"0600 6659 7966 665A 8266 0000 81FF 0200"            /* ..fYyffZ‚f..ÿ.. */
	$"3983 8466 015C 0082 FF01 0083 8266 0260"            /* 9ƒ„f.\.‚ÿ..ƒ‚f.` */
	$"4F00 83FF 0500 8E66 6658 4680 0084 FF05"            /* O.ƒÿ..ŽffXF€.„ÿ. */
	$"008E 6666 5200 87FF 0500 8E66 6652 0087"            /* .ŽffR.‡ÿ..ŽffR.‡ */
	$"FF05 0083 6666 5200 86FF 8700 80FF 85FF"            /* ÿ..ƒffR.†ÿ‡.€ÿ…ÿ */
	$"8000 89FF 0400 A499 8600 84FF 0800 00FF"            /* €.‰ÿ..¤™†.„ÿ...ÿ */
	$"00AD 9999 8000 83FF 0800 8F99 006A A399"            /* .­™™€.ƒÿ..™.j£™ */
	$"9977 8000 81FF 3C00 A697 009F 9999 9269"            /* ™w€.ÿ<.¦—.Ÿ™™’i */
	$"0099 8900 FFFF 007D 998D 00B0 9999 8800"            /* .™‰.ÿÿ.}™.°™™ˆ. */
	$"8699 9900 FFFF 0099 9988 60A8 9999 7C00"            /* †™™.ÿÿ.™™ˆ`¨™™|. */
	$"8699 9900 FFFF 00A9 9999 8199 9997 0057"            /* †™™.ÿÿ.©™™™™—.W */
	$"9199 9900 80FF 0600 9986 A699 9986 8299"            /* ‘™™.€ÿ..™†¦™™†‚™ */
	$"0000 81FF 0200 54AD 8499 018B 0082 FF01"            /* ..ÿ..T­„™.‹.‚ÿ. */
	$"00AD 8299 028F 7600 83FF 0500 B399 9984"            /* .­‚™.v.ƒÿ..³™™„ */
	$"6A80 0084 FF05 00B3 9999 7C00 87FF 0500"            /* j€.„ÿ..³™™|.‡ÿ.. */
	$"B399 997C 0087 FF05 00AD 9999 7C00 86FF"            /* ³™™|.‡ÿ..­™™|.†ÿ */
	$"8700 80FF 85FF 8000 89FF 0100 1C80 0084"            /* ‡.€ÿ…ÿ€.‰ÿ...€.„ */
	$"FF04 0000 FF00 3181 0083 FF05 0025 0000"            /* ÿ...ÿ.1.ƒÿ..%.. */
	$"0F18 8300 81FF 0400 2000 0025 8500 03FF"            /* ..ƒ.ÿ.. ..%…..ÿ */
	$"FF00 2080 0000 3981 0000 2380 0007 FFFF"            /* ÿ. €..9..#€..ÿÿ */
	$"0029 0000 0825 8100 0023 8000 06FF FF00"            /* .)...%..#€..ÿÿ. */
	$"2900 0018 8100 010A 1880 0080 FF80 0000"            /* ).....Â.€.€ÿ€.. */
	$"2086 0081 FF02 0002 3186 0082 FF01 0031"            /*  †.ÿ...1†.‚ÿ..1 */
	$"8500 83FF 0100 4284 0084 FF01 0042 8100"            /* ….ƒÿ..B„.„ÿ..B. */
	$"87FF 0100 4281 0087 FF01 0031 8100 86FF"            /* ‡ÿ..B.‡ÿ..1.†ÿ */
	$"8700 80FF 7338 6D6B 0000 0108 0000 0000"            /* ‡.€ÿs8mk........ */
	$"0000 0000 FFFF FF00 0000 0000 0000 0000"            /* ....ÿÿÿ......... */
	$"0000 00FF FFFF FFFF 0000 0000 0000 00FF"            /* ...ÿÿÿÿÿ.......ÿ */
	$"FF00 FFFF FFFF FFFF 0000 0000 0000 FFFF"            /* ÿ.ÿÿÿÿÿÿ......ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ....ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 00FF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ..ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿ...ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 00FF"            /* ÿÿÿÿÿÿÿÿÿÿÿ....ÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"FFFF FFFF FFFF FFFF FFFF 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿ...... */
	$"FFFF FFFF FFFF FFFF FF00 0000 0000 0000"            /* ÿÿÿÿÿÿÿÿÿ....... */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 00FF"            /* ÿÿÿÿÿÿ.........ÿ */
	$"FFFF FFFF FFFF FFFF FF00 0000 4943 4E23"            /* ÿÿÿÿÿÿÿÿÿ...ICN# */
	$"0000 0108 0000 0000 000F C000 001B E000"            /* ..........À...à. */
	$"0033 E000 0037 F000 006F F000 1F6F F000"            /* .3à..7ð..oð..oð. */
	$"37DF F700 2FDF FF80 6F9F F7C0 5FBF EFC0"            /* 7ß÷./ßÿ€oŸ÷À_¿ïÀ */
	$"5FBF EFC0 DF7F EFC0 BF7F EFC0 BF7F EFC0"            /* _¿ïÀß.ïÀ¿.ïÀ¿.ïÀ */
	$"BF7F EFC0 BEFF FFC0 7EFF FFC0 3EFF FFE0"            /* ¿.ïÀ¾ÿÿÀ~ÿÿÀ>ÿÿà */
	$"1EFD FFF0 06F9 FF90 06F9 DF10 04F8 0020"            /* .ýÿð.ùÿ.ùß..ø.  */
	$"05FC 19C0 05FF 3860 05FF B01C 05FF 8002"            /* .ü.À.ÿ8`.ÿ°..ÿ€. */
	$"05FF 8077 05FF 801D 07FF 8001 07FF C402"            /* .ÿ€w.ÿ€..ÿ€..ÿÄ. */
	$"3FFF FBFC 0000 0000 000F C000 001F E000"            /* ?ÿûü......À...à. */
	$"003F E000 003F F000 007F F000 1F7F F000"            /* .?à..?ð...ð...ð. */
	$"3FFF F700 3FFF FF80 7FFF FFC0 7FFF FFC0"            /* ?ÿ÷.?ÿÿ€.ÿÿÀ.ÿÿÀ */
	$"7FFF FFC0 FFFF FFC0 FFFF FFC0 FFFF FFC0"            /* .ÿÿÀÿÿÿÀÿÿÿÀÿÿÿÀ */
	$"FFFF FFC0 FFFF FFC0 7FFF FFC0 3FFF FFE0"            /* ÿÿÿÀÿÿÿÀ.ÿÿÀ?ÿÿà */
	$"1FFF FFF0 07FF FFF0 07FF FFF0 07FF FFE0"            /* .ÿÿð.ÿÿð.ÿÿð.ÿÿà */
	$"07FF FFC0 07FF FFE0 07FF FFFC 07FF FFFE"            /* .ÿÿÀ.ÿÿà.ÿÿü.ÿÿþ */
	$"07FF FFFF 07FF FFFF 07FF FFFF 07FF FFFE"            /* .ÿÿÿ.ÿÿÿ.ÿÿÿ.ÿÿþ */
	$"3FFF FBFC 6963 6C34 0000 0208 0000 0000"            /* ?ÿûüicl4........ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 FFFF FF00 0000 0000 0000 0000 0000"            /* ..ÿÿÿ........... */
	$"000F AB88 89F0 0000 0000 0000 0000 0000"            /* ..«ˆ‰ð.......... */
	$"00F9 8B88 88F0 0000 0000 0000 0000 0000"            /* .ù‹ˆˆð.......... */
	$"00F8 B888 889F 0000 0000 0000 0000 0000"            /* .ø¸ˆˆŸ.......... */
	$"0FAB 8888 889F 0000 0000 0000 000F FFFF"            /* .«ˆˆˆŸ........ÿÿ */
	$"0FBB 8888 899F 0000 0000 0000 00FA B88A"            /* .»ˆˆ‰Ÿ.......ú¸Š */
	$"F9B8 8888 89AF 0FFF 0000 0000 00FB 8889"            /* ù¸ˆˆ‰¯.ÿ.....ûˆ‰ */
	$"F8B8 8888 89AF F999 F000 0000 0FBB 88A9"            /* ø¸ˆˆ‰¯ù™ð....»ˆ© */
	$"F8B8 8888 99FF B889 9F00 0000 0FB8 88A9"            /* ø¸ˆˆ™ÿ¸‰Ÿ....¸ˆ© */
	$"FB88 8888 99FB 8888 9F00 0000 0FB8 889F"            /* ûˆˆˆ™ûˆˆŸ....¸ˆŸ */
	$"AB88 8888 99FB 8888 9F00 0000 F9B8 889F"            /* «ˆˆˆ™ûˆˆŸ...ù¸ˆŸ */
	$"B888 8888 9FEB 8888 9F00 0000 FB88 889F"            /* ¸ˆˆˆŸëˆˆŸ...ûˆˆŸ */
	$"B888 8889 9F9B 8888 AF00 0000 FB88 88AF"            /* ¸ˆˆ‰Ÿ›ˆˆ¯...ûˆˆ¯ */
	$"B888 8889 9F9B 8888 AF00 0000 FB88 889A"            /* ¸ˆˆ‰Ÿ›ˆˆ¯...ûˆˆš */
	$"B888 889F F99B 8888 AF00 0000 FB88 889B"            /* ¸ˆˆŸù›ˆˆ¯...ûˆˆ› */
	$"8888 88FA 8888 8888 AF00 0000 0F88 88FB"            /* ˆˆˆúˆˆˆˆ¯....ˆˆû */
	$"8888 8A88 8888 8888 9F00 0000 00FA 88FB"            /* ˆˆŠˆˆˆˆˆŸ....úˆû */
	$"8888 8AFF 8888 8888 9FF0 0000 000F F9FB"            /* ˆˆŠÿˆˆˆˆŸð....ùû */
	$"8888 AF0F 8888 888A FFEF 0000 0000 0F9B"            /* ˆˆ¯.ˆˆˆŠÿï.....› */
	$"8888 FD0F 8FFF A8A9 FDCF 0000 0000 0FAB"            /* ˆˆý.ÿ¨©ýÏ.....« */
	$"8888 FC0D FEDD FFFF C0CF 0000 0000 0FB8"            /* ˆˆü.þÝÿÿÀÏ.....¸ */
	$"8888 FC00 0000 00C0 CCF0 0000 0000 0FBB"            /* ˆˆü....ÀÌð.....» */
	$"8888 AF00 00CF D0CD FF00 0000 0000 0FB8"            /* ˆˆ¯..ÏÐÍÿ......¸ */
	$"8888 89FF 00FF F000 0DF0 0000 0000 0FB8"            /* ˆˆ‰ÿ.ÿð..ð.....¸ */
	$"8888 899F F0EF D000 00CF FF00 0000 0FB8"            /* ˆˆ‰ŸðïÐ..Ïÿ....¸ */
	$"8888 A99A F000 0000 0000 0DF0 0000 0FB8"            /* ˆˆ©šð......ð...¸ */
	$"8888 A99A F000 0000 0FFF DEDF 0000 0FB8"            /* ˆˆ©šð....ÿÞß...¸ */
	$"8888 A99F E000 0000 00CF FDCF 0000 0F88"            /* ˆˆ©Ÿà....ÏýÏ...ˆ */
	$"8888 A99A FC00 0000 0000 C0CF 0000 0F88"            /* ˆˆ©šü.....ÀÏ...ˆ */
	$"8888 A99A AFC0 0FC0 0000 00F0 00EE FFFF"            /* ˆˆ©š¯À.À...ð.îÿÿ */
	$"FFFF FFFF FAFF F0FF FFFF FF00 6963 6C38"            /* ÿÿÿÿúÿðÿÿÿÿ.icl8 */
	$"0000 0408 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"0000 0000 0000 0000 0000 0000 0000 00FF"            /* ...............ÿ */
	$"8352 7D7D 7DA7 FF00 0000 0000 0000 0000"            /* ƒR}}}§ÿ......... */
	$"0000 0000 0000 0000 0000 0000 0000 FFA7"            /* ..............ÿ§ */
	$"7D52 7D7D 7D7D FF00 0000 0000 0000 0000"            /* }R}}}}ÿ......... */
	$"0000 0000 0000 0000 0000 0000 0000 FF7D"            /* ..............ÿ} */
	$"527D 7D7D 7D7D A7FF 0000 0000 0000 0000"            /* R}}}}}§ÿ........ */
	$"0000 0000 0000 0000 0000 0000 00FF 7D52"            /* .............ÿ}R */
	$"7D7D 7D7D 7D7D A7FF 0000 0000 0000 0000"            /* }}}}}}§ÿ........ */
	$"0000 0000 0000 00FF FFFF FFFF 00FF 7D52"            /* .......ÿÿÿÿÿ.ÿ}R */
	$"7D7D 7D7D 7DA7 A7FF 0000 0000 0000 0000"            /* }}}}}§§ÿ........ */
	$"0000 0000 0000 FF83 527D 7DAD FFA7 527D"            /* ......ÿƒR}}­ÿ§R} */
	$"7D7D 7D7D 7DA7 ADFF 00FF FFFF 0000 0000"            /* }}}}}§­ÿ.ÿÿÿ.... */
	$"0000 0000 0000 FF52 7D7D 7DA7 FF7D 527D"            /* ......ÿR}}}§ÿ}R} */
	$"7D7D 7D7D 7DA7 ADFF FFA7 A7A7 FF00 0000"            /* }}}}}§­ÿÿ§§§ÿ... */
	$"0000 0000 00FF 7D52 7D7D 7DA7 FF7D 527D"            /* .....ÿ}R}}}§ÿ}R} */
	$"7D7D 7D7D A7A7 FFFF 527D 7DA7 A7FF 0000"            /* }}}}§§ÿÿR}}§§ÿ.. */
	$"0000 0000 00FF 527D 7D7D 7DA7 FF52 7D7D"            /* .....ÿR}}}}§ÿR}} */
	$"7D7D 7D7D A7A7 FF52 7D7D 7D7D A7FF 0000"            /* }}}}§§ÿR}}}}§ÿ.. */
	$"0000 0000 00FF 527D 7D7D A7FF 7D52 7D7D"            /* .....ÿR}}}§ÿ}R}} */
	$"7D7D 7D7D A7A7 FF52 7D7D 7D7D A7FF 0000"            /* }}}}§§ÿR}}}}§ÿ.. */
	$"0000 0000 FF83 527D 7D7D A7FF 527D 7D7D"            /* ....ÿƒR}}}§ÿR}}} */
	$"7D7D 7D7D A7FF AC52 7D7D 7D7D A7FF 0000"            /* }}}}§ÿ¬R}}}}§ÿ.. */
	$"0000 0000 FF52 7D7D 7D7D A7FF 527D 7D7D"            /* ....ÿR}}}}§ÿR}}} */
	$"7D7D 7DA7 A7FF A77C 7D7D 7D7D 7DFF 0000"            /* }}}§§ÿ§|}}}}}ÿ.. */
	$"0000 0000 FF52 7D7D 7D7D 7DFF 527D 7D7D"            /* ....ÿR}}}}}ÿR}}} */
	$"7D7D 7DA7 A7FF A77C 7D7D 7D7D 7DFF 0000"            /* }}}§§ÿ§|}}}}}ÿ.. */
	$"0000 0000 FF52 7D7D 7D7D A77D 527D 7D7D"            /* ....ÿR}}}}§}R}}} */
	$"7D7D A7FF FFAD A758 7D7D 7D7D 7DFF 0000"            /* }}§ÿÿ­§X}}}}}ÿ.. */
	$"0000 0000 FF52 7D7D 7D7D A752 7D7D 7D7D"            /* ....ÿR}}}}§R}}}} */
	$"7D7D FF7D 7D7D 7D7D 7D7D 7D7D 7DFF 0000"            /* }}ÿ}}}}}}}}}}ÿ.. */
	$"0000 0000 00FF 7D7D 7D7D FF52 7D7D 7D7D"            /* .....ÿ}}}}ÿR}}}} */
	$"7D7D 7D7D 7D7D 7D7D 7D7D 7D7D A7FF 0000"            /* }}}}}}}}}}}}§ÿ.. */
	$"0000 0000 0000 FF7D 7D7D FF52 7D7D 7D7D"            /* ......ÿ}}}ÿR}}}} */
	$"7DAD FFFF 7D7D 7D7D 7D7D 7D7D A7FF FF00"            /* }­ÿÿ}}}}}}}}§ÿÿ. */
	$"0000 0000 0000 00FF FFA7 FF52 7D7D 7D7D"            /* .......ÿÿ§ÿR}}}} */
	$"7DFF 00FF 7D7D 7D7D 7D7D 7D7D E9FF ACFF"            /* }ÿ.ÿ}}}}}}}}éÿ¬ÿ */
	$"0000 0000 0000 0000 00FF A752 7D7D 7D7D"            /* .........ÿ§R}}}} */
	$"FFF8 00FF 7DFF FFFF 7D7D 7DA7 FFF9 F8FF"            /* ÿø.ÿ}ÿÿÿ}}}§ÿùøÿ */
	$"0000 0000 0000 0000 00FF 8352 7D7D 7D7D"            /* .........ÿƒR}}}} */
	$"FFF7 00FA FF81 F9FA FFFF FFFF F800 F7FF"            /* ÿ÷.úÿùúÿÿÿÿø.÷ÿ */
	$"0000 0000 0000 0000 00FF 527D 7D7D 7D7D"            /* .........ÿR}}}}} */
	$"FFF7 0000 0000 00F5 0000 F600 F6F6 FF00"            /* ÿ÷.....õ..ö.ööÿ. */
	$"0000 0000 0000 0000 00FF 527D 7D7D 7D7D"            /* .........ÿR}}}}} */
	$"7DFF 0000 0000 F8FF FA00 F6FA FFFF 0000"            /* }ÿ....øÿú.öúÿÿ.. */
	$"0000 0000 0000 0000 00FF 527D 7D7D 7D7D"            /* .........ÿR}}}}} */
	$"7DA7 FFFF 0000 FFFF FF00 00F5 00FA FF00"            /* }§ÿÿ..ÿÿÿ..õ.úÿ. */
	$"0000 0000 0000 0000 00FF 527D 7D7D 7D7D"            /* .........ÿR}}}}} */
	$"7DA7 A7FE FF00 FBFF F900 0000 0000 F7FF"            /* }§§þÿ.ûÿù.....÷ÿ */
	$"FFFF 0000 0000 0000 00FF 527D 7D7D 7D7D"            /* ÿÿ.......ÿR}}}}} */
	$"7DA7 A7AD FF00 F5F5 0000 0000 0000 0000"            /* }§§­ÿ.õõ........ */
	$"00F9 FF00 0000 0000 00FF 527D 7D7D 7D7D"            /* .ùÿ......ÿR}}}}} */
	$"7DA7 A7AD FF00 0000 0000 0000 00FF FFFF"            /* }§§­ÿ........ÿÿÿ */
	$"F9FB 81FF 0000 0000 00FF 527D 7D7D 7D7D"            /* ùûÿ.....ÿR}}}}} */
	$"7DA7 A7FF FB00 0000 0000 0000 0000 F8FF"            /* }§§ÿû.........øÿ */
	$"FF81 2BFF 0000 0000 00FF 7D7D 7D7D 7D7D"            /* ÿ+ÿ.....ÿ}}}}}} */
	$"7DA7 A7AD FF2B 0000 0000 0000 0000 00F5"            /* }§§­ÿ+.........õ */
	$"F600 F8FF 0000 0000 00FF 7D7D 7D7D 7D7D"            /* ö.øÿ.....ÿ}}}}}} */
	$"7DA7 A7AD ADFF 2B00 00FF F700 0000 0000"            /* }§§­­ÿ+..ÿ÷..... */
	$"0000 FF00 0000 ACAC FFFF FFFF FFFF FFFF"            /* ..ÿ...¬¬ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFAD FFFF FF00 FFFF FFFF FFFF"            /* ÿÿÿÿÿ­ÿÿÿ.ÿÿÿÿÿÿ */
	$"FFFF 0000 696C 3332 0000 06C7 A9FF 8300"            /* ÿÿ..il32...Ç©ÿƒ. */
	$"96FF 0200 549F 8066 014C 0094 FF03 004E"            /* –ÿ..TŸ€f.L.”ÿ..N */
	$"749F 8166 0000 94FF 0200 6B9F 8266 014C"            /* tŸf..”ÿ..kŸ‚f.L */
	$"0092 FF02 0059 9F83 6601 4C00 8CFF 8200"            /* .’ÿ..YŸƒf.L.Œÿ‚. */
	$"03FF 0070 9F82 6602 4C4C 008B FF08 0056"            /* .ÿ.pŸ‚f.LL.‹ÿ..V */
	$"9F66 6633 0045 9F83 6603 4C34 00FF 8000"            /* Ÿff3.EŸƒf.L4.ÿ€. */
	$"87FF 0100 9F80 6603 4F00 629F 8366 034C"            /* ‡ÿ..Ÿ€f.O.bŸƒf.L */
	$"2C00 0080 4C00 0085 FF09 0063 9F66 6660"            /* ,..€L..…ÿÆ.cŸff` */
	$"4600 779F 8266 094C 4C00 009F 6666 4C4C"            /* F.wŸ‚fÆLL..ŸffLL */
	$"0084 FF01 009F 8066 0356 3A00 9F83 6603"            /* .„ÿ..Ÿ€f.V:.Ÿƒf. */
	$"4C4C 009F 8166 014C 0084 FF01 009F 8066"            /* LL.Ÿf.L.„ÿ..Ÿ€f */
	$"034F 005C 9F83 6603 4C4C 009F 8166 014C"            /* .O.\Ÿƒf.LL.Ÿf.L */
	$"0083 FF08 0057 9F66 6663 4C00 9F84 6603"            /* .ƒÿ..WŸffcL.Ÿ„f. */
	$"4C00 329F 8166 014C 0083 FF01 009F 8066"            /* L.2Ÿf.L.ƒÿ..Ÿ€f */
	$"0361 4C00 9F83 6604 4C4C 004C 7981 6601"            /* .aL.Ÿƒf.LL.Lyf. */
	$"5900 83FF 0100 9F81 6602 5B00 9F83 6604"            /* Y.ƒÿ..Ÿf.[.Ÿƒf. */
	$"4C4C 004C 7F81 6601 5900 83FF 0100 9F81"            /* LL.L.f.Y.ƒÿ..Ÿ */
	$"6602 4D5C 9F82 6605 4C00 0028 4C86 8166"            /* f.M\Ÿ‚f.L..(L†f */
	$"0159 0083 FF01 009F 8166 013D 9F83 6601"            /* .Y.ƒÿ..Ÿf.=Ÿƒf. */
	$"0060 8566 0157 0084 FF00 0081 6601 009F"            /* .`…f.W.„ÿ..f..Ÿ */
	$"8266 025F 6363 8566 014C 0085 FF05 0059"            /* ‚f._cc…f.L.…ÿ..Y */
	$"6666 009F 8266 0230 0000 8466 0364 4C00"            /* ff.Ÿ‚f.0..„f.dL. */
	$"0085 FF04 0000 3C00 9F81 6603 5E00 FF00"            /* .…ÿ...<.Ÿf.^.ÿ. */
	$"8466 0459 1B00 3E00 86FF 0200 4C9F 8166"            /* „f.Y..>.†ÿ..LŸf */
	$"0400 A0FF 0066 8000 075E 6056 3900 8FA6"            /* .. ÿ.f€..^`V9.¦ */
	$"0086 FF02 0053 9F81 6607 00BF FF73 0061"            /* .†ÿ..SŸf..¿ÿs.a */
	$"8777 8100 03A2 FFB5 0086 FF02 009F 7781"            /* ‡w..¢ÿµ.†ÿ..Ÿw */
	$"6601 00B0 82FF 07EF FFFF DFFF D5DF 0087"            /* f..°‚ÿ.ïÿÿßÿÕß.‡ */
	$"FF02 009F 7081 6601 5900 81FF 07AF 0070"            /* ÿ..Ÿpf.Y.ÿ.¯.p */
	$"FFD0 7F00 0088 FF02 009F 6B82 6604 3700"            /* ÿÐ...ˆÿ..Ÿk‚f.7. */
	$"00FF FF80 0005 FFFF E0FF 7000 87FF 0100"            /* .ÿÿ€..ÿÿàÿp.‡ÿ.. */
	$"9F83 6607 4C4C 1500 FF50 0080 82FF 00B0"            /* Ÿƒf.LL..ÿP.€‚ÿ.° */
	$"8000 84FF 0100 9F82 6607 604C 4C2C 00FF"            /* €.„ÿ..Ÿ‚f.`LL,.ÿ */
	$"E0E0 86FF 0184 0083 FF01 009F 8266 0460"            /* àà†ÿ.„.ƒÿ..Ÿ‚f.` */
	$"4C4C 2C00 85FF 8000 038F 5060 0082 FF01"            /* LL,.…ÿ€..P`.‚ÿ. */
	$"009F 8266 0460 4C4C 0056 86FF 05A0 0000"            /* .Ÿ‚f.`LL.V†ÿ. .. */
	$"60CF 0082 FF01 006D 8266 0560 4C4C 2C00"            /* `Ï.‚ÿ..m‚f.`LL,. */
	$"CF86 FF04 EFDF FFAF 0082 FF00 0083 660A"            /* Ï†ÿ.ïßÿ¯.‚ÿ..ƒfÂ */
	$"604C 4C2C 2C00 C0FF FF00 BF84 FF00 0080"            /* `LL,,.Àÿÿ.¿„ÿ..€ */
	$"FF01 3F3F 8A00 002C 8000 00FF 8500 01FF"            /* ÿ.??Š..,€..ÿ…..ÿ */
	$"FFA9 FF83 0096 FF02 007D C980 9901 7100"            /* ÿ©ÿƒ.–ÿ..}É€™.q. */
	$"94FF 0300 74A3 C981 9900 0094 FF02 009C"            /* ”ÿ..t£É™..”ÿ..œ */
	$"C982 9901 7100 92FF 0200 86C9 8399 0171"            /* É‚™.q.’ÿ..†Éƒ™.q */
	$"008C FF82 0003 FF00 9FC9 8299 0271 7100"            /* .Œÿ‚..ÿ.ŸÉ‚™.qq. */
	$"8BFF 0800 7BC9 9999 4C00 64C9 8399 0371"            /* ‹ÿ..{É™™L.dÉƒ™.q */
	$"4A00 FF80 0087 FF01 00C9 8099 0377 0091"            /* J.ÿ€.‡ÿ..É€™.w.‘ */
	$"C983 9903 713D 0000 8071 0000 85FF 0900"            /* Éƒ™.q=..€q..…ÿÆ. */
	$"88C9 9999 8F69 00A4 C982 9909 7171 0000"            /* ˆÉ™™i.¤É‚™Æqq.. */
	$"C999 9971 7100 84FF 0100 C980 9903 8057"            /* É™™qq.„ÿ..É€™.€W */
	$"00C9 8399 0371 7100 C981 9901 7100 84FF"            /* .Éƒ™.qq.É™.q.„ÿ */
	$"0100 C980 9903 7700 88C9 8399 0371 7100"            /* ..É€™.w.ˆÉƒ™.qq. */
	$"C981 9901 7100 83FF 0800 7CC9 9999 9471"            /* É™.q.ƒÿ..|É™™”q */
	$"00C9 8499 0371 0032 C981 9901 7100 83FF"            /* .É„™.q.2É™.q.ƒÿ */
	$"0100 C980 9903 9271 00C9 8399 0471 7100"            /* ..É€™.’q.Éƒ™.qq. */
	$"71A6 8199 0186 0083 FF01 00C9 8199 0288"            /* q¦™.†.ƒÿ..É™.ˆ */
	$"00C9 8399 0471 7100 71A9 8199 0186 0083"            /* .Éƒ™.qq.q©™.†.ƒ */
	$"FF01 00C9 8199 0273 88C9 8299 0571 0000"            /* ÿ..É™.sˆÉ‚™.q.. */
	$"3171 AE81 9901 8600 83FF 0100 C981 9901"            /* 1q®™.†.ƒÿ..É™. */
	$"5AC9 8399 0100 8F85 9901 8300 84FF 0000"            /* ZÉƒ™..…™.ƒ.„ÿ.. */
	$"8199 0100 C982 9902 8D95 9485 9901 7100"            /* ™..É‚™.•”…™.q. */
	$"85FF 0500 8699 9900 C982 9902 4800 0084"            /* …ÿ..†™™.É‚™.H..„ */
	$"9903 9771 0000 85FF 0400 0058 00C9 8199"            /* ™.—q..…ÿ...X.É™ */
	$"038D 00FF 0084 9904 8628 003E 0086 FF02"            /* ..ÿ.„™.†(.>.†ÿ. */
	$"0073 C981 9904 00A1 FF00 9980 0007 8D90"            /* .sÉ™..¡ÿ.™€.. */
	$"8157 008F A600 86FF 0200 7CC9 8199 0700"            /* W.¦.†ÿ..|É™.. */
	$"BFFF 7400 628B 7B81 0003 A2FF B500 86FF"            /* ¿ÿt.b‹{..¢ÿµ.†ÿ */
	$"0200 C9A4 8199 0100 B182 FF07 EFFF FFDF"            /* ..É¤™..±‚ÿ.ïÿÿß */
	$"FFD5 DF00 87FF 0200 C99F 8199 0186 0081"            /* ÿÕß.‡ÿ..ÉŸ™.†. */
	$"FF07 AF00 70FF D07F 0000 88FF 0200 C99C"            /* ÿ.¯.pÿÐ...ˆÿ..Éœ */
	$"8299 0452 0000 FFFF 8000 05FF FFE0 FF70"            /* ‚™.R..ÿÿ€..ÿÿàÿp */
	$"0087 FF01 00C9 8399 0771 7115 00FF 5000"            /* .‡ÿ..Éƒ™.qq..ÿP. */
	$"8082 FF00 B080 0084 FF01 00C9 8299 078F"            /* €‚ÿ.°€.„ÿ..É‚™. */
	$"7171 3D00 FFE0 E086 FF01 8400 83FF 0100"            /* qq=.ÿàà†ÿ.„.ƒÿ.. */
	$"C982 9904 8F71 713D 0085 FF80 0003 8F50"            /* É‚™.qq=.…ÿ€..P */
	$"6000 82FF 0100 C982 9904 8F71 7100 5686"            /* `.‚ÿ..É‚™.qq.V† */
	$"FF05 A000 0060 CF00 82FF 0100 9E82 9905"            /* ÿ. ..`Ï.‚ÿ..ž‚™. */
	$"8F71 713D 00CF 86FF 04EF DFFF AF00 82FF"            /* qq=.Ï†ÿ.ïßÿ¯.‚ÿ */
	$"0000 8399 0A8F 7171 3D3D 00C0 FFFF 00BF"            /* ..ƒ™Âqq==.Àÿÿ.¿ */
	$"84FF 0000 80FF 013F 3F8A 0000 3D80 0000"            /* „ÿ..€ÿ.??Š..=€.. */
	$"FF85 0001 FFFF A9FF 8300 96FF 0200 014A"            /* ÿ…..ÿÿ©ÿƒ.–ÿ...J */
	$"8200 94FF 0300 0218 4A82 0094 FF02 0008"            /* ‚.”ÿ....J‚.”ÿ... */
	$"4A84 0092 FF02 0000 4A85 008C FF82 0003"            /* J„.’ÿ...J….Œÿ‚.. */
	$"FF00 104A 8500 8BFF 0200 0D4A 8100 0105"            /* ÿ..J….‹ÿ...J... */
	$"4A84 0002 0700 FF80 0087 FF01 004A 8200"            /* J„....ÿ€.‡ÿ..J‚. */
	$"0104 4A84 0000 0983 0085 FF02 0018 4A82"            /* ..J„..Æƒ.…ÿ...J‚ */
	$"0001 1C4A 8600 004A 8200 84FF 0100 4A83"            /* ...J†..J‚.„ÿ..Jƒ */
	$"0000 4A86 0000 4A83 0084 FF01 004A 8200"            /* ..J†..Jƒ.„ÿ..J‚. */
	$"0104 4A86 0000 4A83 0083 FF02 0010 4A82"            /* ..J†..Jƒ.ƒÿ...J‚ */
	$"0000 4A86 0001 324A 8300 83FF 0100 4A83"            /* ..J†..2Jƒ.ƒÿ..Jƒ */
	$"0000 4A87 0000 2083 0083 FF01 004A 8300"            /* ..J‡.. ƒ.ƒÿ..Jƒ. */
	$"004A 8700 0029 8300 83FF 0100 4A82 0001"            /* .J‡..)ƒ.ƒÿ..J‚.. */
	$"044A 8500 0215 0035 8300 83FF 0100 4A82"            /* .J…....5ƒ.ƒÿ..J‚ */
	$"0000 4A8F 0084 FF83 0000 4A8F 0085 FF82"            /* ..J.„ÿƒ..J.…ÿ‚ */
	$"0000 4A90 0085 FF04 0000 0300 4A83 0000"            /* ..J.…ÿ.....Jƒ.. */
	$"FF88 0001 3E00 86FF 0200 004A 8200 019E"            /* ÿˆ..>.†ÿ...J‚..ž */
	$"FF87 0002 8FA6 0086 FF02 0000 4A82 0006"            /* ÿ‡..¦.†ÿ...J‚.. */
	$"BFFF 6F00 5F80 7081 0003 A2FF B500 86FF"            /* ¿ÿo._€p..¢ÿµ.†ÿ */
	$"0200 4A1C 8200 00AE 82FF 07EF FFFF DFFF"            /* ..J.‚..®‚ÿ.ïÿÿßÿ */
	$"D5DF 0087 FF02 004A 1083 0081 FF07 AF00"            /* Õß.‡ÿ..J.ƒ.ÿ.¯. */
	$"70FF D07F 0000 88FF 0200 4A08 8500 01FF"            /* pÿÐ...ˆÿ..J.…..ÿ */
	$"FF80 0005 FFFF E0FF 7000 87FF 0100 4A85"            /* ÿ€..ÿÿàÿp.‡ÿ..J… */
	$"0005 1500 FF50 0080 82FF 00B0 8000 84FF"            /* ....ÿP.€‚ÿ.°€.„ÿ */
	$"0100 4A85 0004 0900 FFE0 E086 FF01 8400"            /* ..J…..Æ.ÿàà†ÿ.„. */
	$"83FF 0100 4A85 0001 0900 85FF 8000 038F"            /* ƒÿ..J…..Æ.…ÿ€.. */
	$"5060 0082 FF01 004A 8600 0056 86FF 05A0"            /* P`.‚ÿ..J†..V†ÿ.  */
	$"0000 60CF 0082 FF01 000C 8500 0209 00CF"            /* ..`Ï.‚ÿ...…..Æ.Ï */
	$"86FF 04EF DFFF AF00 82FF 8700 0709 0900"            /* †ÿ.ïßÿ¯.‚ÿ‡..ÆÆ. */
	$"C0FF FF00 BF84 FF00 0080 FF01 3F3F 8A00"            /* Àÿÿ.¿„ÿ..€ÿ.??Š. */
	$"0009 8000 00FF 8500 01FF FF6C 386D 6B00"            /* .Æ€..ÿ…..ÿÿl8mk. */
	$"0004 0800 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 00FF"            /* ...............ÿ */
	$"FFFF FFFF FF00 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿ........... */
	$"0000 0000 0000 0000 0000 0000 0000 FFFF"            /* ..............ÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"0000 0000 0000 0000 0000 0000 00FF FFFF"            /* .............ÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"0000 0000 0000 0000 0000 0000 00FF FFFF"            /* .............ÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 0000 0000 0000 0000 0000 FFFF FFFF"            /* ............ÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 0000 0000 FFFF FFFF FF00 FFFF FFFF"            /* ......ÿÿÿÿÿ.ÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿ.ÿÿÿ..... */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿ.......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿÿ......ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿ.....ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿ.....ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 FFFF FFFF FFFF FFFF"            /* ÿÿÿ.....ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿ...ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿ.ÿÿÿÿÿÿÿ */
	$"FF00 0069 6368 2300 0002 48FF FFFF FFFF"            /* ÿ..ich#...Hÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFF8 0000 0000 0FF8"            /* ÿÿÿÿÿÿÿÿÿø.....ø */
	$"0000 0000 0FF8 0000 0000 0FF8 0000 0000"            /* .....ø.....ø.... */
	$"0FF8 000F C000 0FF8 001B E000 0FF8 0033"            /* .ø..À..ø..à..ø.3 */
	$"E000 0FF8 0037 F000 0FF8 006F F000 0FF8"            /* à..ø.7ð..ø.oð..ø */
	$"1F6F F000 0FF8 37DF F700 0FF8 2FDF FF80"            /* .oð..ø7ß÷..ø/ßÿ€ */
	$"0FF8 6F9F F7C0 0FF8 5FBF EFC0 0FF8 5FBF"            /* .øoŸ÷À.ø_¿ïÀ.ø_¿ */
	$"EFC0 0FF8 DF7F EFC0 0FF8 BF7F EFC0 0FF8"            /* ïÀ.øß.ïÀ.ø¿.ïÀ.ø */
	$"BF7F EFC0 0FF8 BF7F EFC0 0FF8 BEFF FFC0"            /* ¿.ïÀ.ø¿.ïÀ.ø¾ÿÿÀ */
	$"0FF8 7EFF FFC0 0FF8 3EFF FFE0 0FF8 1EFD"            /* .ø~ÿÿÀ.ø>ÿÿà.ø.ý */
	$"FFF0 0FF8 06F9 FF90 0FF8 06F9 DF10 0FF8"            /* ÿð.ø.ùÿ.ø.ùß..ø */
	$"04F8 0020 0FF8 05FC 19C0 0FF8 05FF 3860"            /* .ø. .ø.ü.À.ø.ÿ8` */
	$"0FF8 05FF B01C 0FF8 05FF 8002 0FF8 05FF"            /* .ø.ÿ°..ø.ÿ€..ø.ÿ */
	$"8077 0FF8 05FF 801D 0FF8 07FF 8001 0FF8"            /* €w.ø.ÿ€..ø.ÿ€..ø */
	$"07FF C402 0FF8 3FFF FBFC 0FF8 0000 0000"            /* .ÿÄ..ø?ÿûü.ø.... */
	$"0FF8 0000 0000 0FF8 0000 0000 0FFF FFFF"            /* .ø.....ø.....ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿ..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 000F C000 0000 001F E000 0000 003F"            /* ....À.....à....? */
	$"E000 0000 003F F000 0000 007F F000 0000"            /* à....?ð.....ð... */
	$"1F7F F000 0000 3FFF F700 0000 3FFF FF80"            /* ..ð...?ÿ÷...?ÿÿ€ */
	$"0000 7FFF FFC0 0000 7FFF FFC0 0000 7FFF"            /* ...ÿÿÀ...ÿÿÀ...ÿ */
	$"FFC0 0000 FFFF FFC0 0000 FFFF FFC0 0000"            /* ÿÀ..ÿÿÿÀ..ÿÿÿÀ.. */
	$"FFFF FFC0 0000 FFFF FFC0 0000 FFFF FFC0"            /* ÿÿÿÀ..ÿÿÿÀ..ÿÿÿÀ */
	$"0000 7FFF FFC0 0000 3FFF FFE0 0000 1FFF"            /* ...ÿÿÀ..?ÿÿà...ÿ */
	$"FFF0 0000 07FF FFF0 0000 07FF FFF0 0000"            /* ÿð...ÿÿð...ÿÿð.. */
	$"07FF FFE0 0000 07FF FFC0 0000 07FF FFE0"            /* .ÿÿà...ÿÿÀ...ÿÿà */
	$"0000 07FF FFFC 0000 07FF FFFE 0000 07FF"            /* ...ÿÿü...ÿÿþ...ÿ */
	$"FFFF 0000 07FF FFFF 0000 07FF FFFF 0000"            /* ÿÿ...ÿÿÿ...ÿÿÿ.. */
	$"07FF FFFE 0000 3FFF FBFC 0000 0000 0000"            /* .ÿÿþ..?ÿûü...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0069 6368 3400"            /* ...........ich4. */
	$"0004 88FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ..ˆÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFF0 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿð.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"00FF FFFF FFF0 0000 0000 0000 0000 0000"            /* .ÿÿÿÿð.......... */
	$"0000 0000 0000 0000 00FF FFFF FFF0 0000"            /* .........ÿÿÿÿð.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"00FF FFFF FFF0 0000 0000 0000 0000 0000"            /* .ÿÿÿÿð.......... */
	$"0000 0000 0000 0000 00FF FFFF FFF0 0000"            /* .........ÿÿÿÿð.. */
	$"0000 0000 00FF FFFF 0000 0000 0000 0000"            /* .....ÿÿÿ........ */
	$"00FF FFFF FFF0 0000 0000 0000 0FAB 8889"            /* .ÿÿÿÿð.......«ˆ‰ */
	$"F000 0000 0000 0000 00FF FFFF FFF0 0000"            /* ð........ÿÿÿÿð.. */
	$"0000 0000 F98B 8888 F000 0000 0000 0000"            /* ....ù‹ˆˆð....... */
	$"00FF FFFF FFF0 0000 0000 0000 F8B8 8888"            /* .ÿÿÿÿð......ø¸ˆˆ */
	$"9F00 0000 0000 0000 00FF FFFF FFF0 0000"            /* Ÿ........ÿÿÿÿð.. */
	$"0000 000F AB88 8888 9F00 0000 0000 0000"            /* ....«ˆˆˆŸ....... */
	$"00FF FFFF FFF0 0000 0FFF FF0F BB88 8889"            /* .ÿÿÿÿð...ÿÿ.»ˆˆ‰ */
	$"9F00 0000 0000 0000 00FF FFFF FFF0 0000"            /* Ÿ........ÿÿÿÿð.. */
	$"FAB8 8AF9 B888 8889 AF0F FF00 0000 0000"            /* ú¸Šù¸ˆˆ‰¯.ÿ..... */
	$"00FF FFFF FFF0 0000 FB88 89F8 B888 8889"            /* .ÿÿÿÿð..ûˆ‰ø¸ˆˆ‰ */
	$"AFF9 99F0 0000 0000 00FF FFFF FFF0 000F"            /* ¯ù™ð.....ÿÿÿÿð.. */
	$"BB88 A9F8 B888 8899 FFB8 899F 0000 0000"            /* »ˆ©ø¸ˆˆ™ÿ¸‰Ÿ.... */
	$"00FF FFFF FFF0 000F B888 A9FB 8888 8899"            /* .ÿÿÿÿð..¸ˆ©ûˆˆˆ™ */
	$"FB88 889F 0000 0000 00FF FFFF FFF0 000F"            /* ûˆˆŸ.....ÿÿÿÿð.. */
	$"B888 9FAB 8888 8899 FB88 889F 0000 0000"            /* ¸ˆŸ«ˆˆˆ™ûˆˆŸ.... */
	$"00FF FFFF FFF0 00F9 B888 9FB8 8888 889F"            /* .ÿÿÿÿð.ù¸ˆŸ¸ˆˆˆŸ */
	$"EB88 889F 0000 0000 00FF FFFF FFF0 00FB"            /* ëˆˆŸ.....ÿÿÿÿð.û */
	$"8888 9FB8 8888 899F 9B88 88AF 0000 0000"            /* ˆˆŸ¸ˆˆ‰Ÿ›ˆˆ¯.... */
	$"00FF FFFF FFF0 00FB 8888 AFB8 8888 899F"            /* .ÿÿÿÿð.ûˆˆ¯¸ˆˆ‰Ÿ */
	$"9B88 88AF 0000 0000 00FF FFFF FFF0 00FB"            /* ›ˆˆ¯.....ÿÿÿÿð.û */
	$"8888 9AB8 8888 9FF9 9B88 88AF 0000 0000"            /* ˆˆš¸ˆˆŸù›ˆˆ¯.... */
	$"00FF FFFF FFF0 00FB 8888 9B88 8888 FA88"            /* .ÿÿÿÿð.ûˆˆ›ˆˆˆúˆ */
	$"8888 88AF 0000 0000 00FF FFFF FFF0 000F"            /* ˆˆˆ¯.....ÿÿÿÿð.. */
	$"8888 FB88 888A 8888 8888 889F 0000 0000"            /* ˆˆûˆˆŠˆˆˆˆˆŸ.... */
	$"00FF FFFF FFF0 0000 FA88 FB88 888A FF88"            /* .ÿÿÿÿð..úˆûˆˆŠÿˆ */
	$"8888 889F F000 0000 00FF FFFF FFF0 0000"            /* ˆˆˆŸð....ÿÿÿÿð.. */
	$"0FF9 FB88 88AF 0F88 8888 8AFF EF00 0000"            /* .ùûˆˆ¯.ˆˆˆŠÿï... */
	$"00FF FFFF FFF0 0000 000F 9B88 88FD 0F8F"            /* .ÿÿÿÿð....›ˆˆý. */
	$"FFA8 A9FD CF00 0000 00FF FFFF FFF0 0000"            /* ÿ¨©ýÏ....ÿÿÿÿð.. */
	$"000F AB88 88FC 0DFE DDFF FFC0 CF00 0000"            /* ..«ˆˆü.þÝÿÿÀÏ... */
	$"00FF FFFF FFF0 0000 000F B888 88FC 0000"            /* .ÿÿÿÿð....¸ˆˆü.. */
	$"0000 C0CC F000 0000 00FF FFFF FFF0 0000"            /* ..ÀÌð....ÿÿÿÿð.. */
	$"000F BB88 88AF 0000 CFD0 CDFF 0000 0000"            /* ..»ˆˆ¯..ÏÐÍÿ.... */
	$"00FF FFFF FFF0 0000 000F B888 8889 FF00"            /* .ÿÿÿÿð....¸ˆˆ‰ÿ. */
	$"FFF0 000D F000 0000 00FF FFFF FFF0 0000"            /* ÿð..ð....ÿÿÿÿð.. */
	$"000F B888 8889 9FF0 EFD0 0000 CFFF 0000"            /* ..¸ˆˆ‰ŸðïÐ..Ïÿ.. */
	$"00FF FFFF FFF0 0000 000F B888 88A9 9AF0"            /* .ÿÿÿÿð....¸ˆˆ©šð */
	$"0000 0000 000D F000 00FF FFFF FFF0 0000"            /* ......ð..ÿÿÿÿð.. */
	$"000F B888 88A9 9AF0 0000 000F FFDE DF00"            /* ..¸ˆˆ©šð....ÿÞß. */
	$"00FF FFFF FFF0 0000 000F B888 88A9 9FE0"            /* .ÿÿÿÿð....¸ˆˆ©Ÿà */
	$"0000 0000 CFFD CF00 00FF FFFF FFF0 0000"            /* ....ÏýÏ..ÿÿÿÿð.. */
	$"000F 8888 88A9 9AFC 0000 0000 00C0 CF00"            /* ..ˆˆˆ©šü.....ÀÏ. */
	$"00FF FFFF FFF0 0000 000F 8888 88A9 9AAF"            /* .ÿÿÿÿð....ˆˆˆ©š¯ */
	$"C00F C000 0000 F000 00FF FFFF FFF0 0000"            /* À.À...ð..ÿÿÿÿð.. */
	$"EEFF FFFF FFFF FFFA FFF0 FFFF FFFF 0000"            /* îÿÿÿÿÿÿúÿðÿÿÿÿ.. */
	$"00FF FFFF FFF0 0000 0000 0000 0000 0000"            /* .ÿÿÿÿð.......... */
	$"0000 0000 0000 0000 00FF FFFF FFF0 0000"            /* .........ÿÿÿÿð.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"00FF FFFF FFF0 0000 0000 0000 0000 0000"            /* .ÿÿÿÿð.......... */
	$"0000 0000 0000 0000 00FF FFFF FFFF FFFF"            /* .........ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF69 6368 3800 0009 08FF FFFF FFFF"            /* ÿÿÿich8..Æ.ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 00FF"            /* ...............ÿ */
	$"FFFF FFFF FF00 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿ........... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 FF83"            /* ..............ÿƒ */
	$"527D 7D7D A7FF 0000 0000 0000 0000 0000"            /* R}}}§ÿ.......... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 00FF A77D"            /* .............ÿ§} */
	$"527D 7D7D 7DFF 0000 0000 0000 0000 0000"            /* R}}}}ÿ.......... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 00FF 7D52"            /* .............ÿ}R */
	$"7D7D 7D7D 7DA7 FF00 0000 0000 0000 0000"            /* }}}}}§ÿ......... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 FF7D 527D"            /* ............ÿ}R} */
	$"7D7D 7D7D 7DA7 FF00 0000 0000 0000 0000"            /* }}}}}§ÿ......... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 FFFF FFFF FF00 FF7D 527D"            /* ......ÿÿÿÿÿ.ÿ}R} */
	$"7D7D 7D7D A7A7 FF00 0000 0000 0000 0000"            /* }}}}§§ÿ......... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 00FF 8352 7D7D ADFF A752 7D7D"            /* .....ÿƒR}}­ÿ§R}} */
	$"7D7D 7D7D A7AD FF00 FFFF FF00 0000 0000"            /* }}}}§­ÿ.ÿÿÿ..... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 00FF 527D 7D7D A7FF 7D52 7D7D"            /* .....ÿR}}}§ÿ}R}} */
	$"7D7D 7D7D A7AD FFFF A7A7 A7FF 0000 0000"            /* }}}}§­ÿÿ§§§ÿ.... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 FF7D 527D 7D7D A7FF 7D52 7D7D"            /* ....ÿ}R}}}§ÿ}R}} */
	$"7D7D 7DA7 A7FF FF52 7D7D A7A7 FF00 0000"            /* }}}§§ÿÿR}}§§ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 FF52 7D7D 7D7D A7FF 527D 7D7D"            /* ....ÿR}}}}§ÿR}}} */
	$"7D7D 7DA7 A7FF 527D 7D7D 7DA7 FF00 0000"            /* }}}§§ÿR}}}}§ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 FF52 7D7D 7DA7 FF7D 527D 7D7D"            /* ....ÿR}}}§ÿ}R}}} */
	$"7D7D 7DA7 A7FF 527D 7D7D 7DA7 FF00 0000"            /* }}}§§ÿR}}}}§ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 00FF 8352 7D7D 7DA7 FF52 7D7D 7D7D"            /* ...ÿƒR}}}§ÿR}}}} */
	$"7D7D 7DA7 FFAC 527D 7D7D 7DA7 FF00 0000"            /* }}}§ÿ¬R}}}}§ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 00FF 527D 7D7D 7DA7 FF52 7D7D 7D7D"            /* ...ÿR}}}}§ÿR}}}} */
	$"7D7D A7A7 FFA7 7C7D 7D7D 7D7D FF00 0000"            /* }}§§ÿ§|}}}}}ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 00FF 527D 7D7D 7D7D FF52 7D7D 7D7D"            /* ...ÿR}}}}}ÿR}}}} */
	$"7D7D A7A7 FFA7 7C7D 7D7D 7D7D FF00 0000"            /* }}§§ÿ§|}}}}}ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 00FF 527D 7D7D 7DA7 7D52 7D7D 7D7D"            /* ...ÿR}}}}§}R}}}} */
	$"7DA7 FFFF ADA7 587D 7D7D 7D7D FF00 0000"            /* }§ÿÿ­§X}}}}}ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 00FF 527D 7D7D 7DA7 527D 7D7D 7D7D"            /* ...ÿR}}}}§R}}}}} */
	$"7DFF 7D7D 7D7D 7D7D 7D7D 7D7D FF00 0000"            /* }ÿ}}}}}}}}}}ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 FF7D 7D7D 7DFF 527D 7D7D 7D7D"            /* ....ÿ}}}}ÿR}}}}} */
	$"7D7D 7D7D 7D7D 7D7D 7D7D 7DA7 FF00 0000"            /* }}}}}}}}}}}§ÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 00FF 7D7D 7DFF 527D 7D7D 7D7D"            /* .....ÿ}}}ÿR}}}}} */
	$"ADFF FF7D 7D7D 7D7D 7D7D 7DA7 FFFF 0000"            /* ­ÿÿ}}}}}}}}§ÿÿ.. */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 FFFF A7FF 527D 7D7D 7D7D"            /* ......ÿÿ§ÿR}}}}} */
	$"FF00 FF7D 7D7D 7D7D 7D7D 7DE9 FFAC FF00"            /* ÿ.ÿ}}}}}}}}éÿ¬ÿ. */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FFA7 527D 7D7D 7DFF"            /* ........ÿ§R}}}}ÿ */
	$"F800 FF7D FFFF FF7D 7D7D A7FF F9F8 FF00"            /* ø.ÿ}ÿÿÿ}}}§ÿùøÿ. */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF83 527D 7D7D 7DFF"            /* ........ÿƒR}}}}ÿ */
	$"F700 FAFF 81F9 FAFF FFFF FFF8 00F7 FF00"            /* ÷.úÿùúÿÿÿÿø.÷ÿ. */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7DFF"            /* ........ÿR}}}}}ÿ */
	$"F700 0000 0000 F500 00F6 00F6 F6FF 0000"            /* ÷.....õ..ö.ööÿ.. */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7D7D"            /* ........ÿR}}}}}} */
	$"FF00 0000 00F8 FFFA 00F6 FAFF FF00 0000"            /* ÿ....øÿú.öúÿÿ... */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7D7D"            /* ........ÿR}}}}}} */
	$"A7FF FF00 00FF FFFF 0000 F500 FAFF 0000"            /* §ÿÿ..ÿÿÿ..õ.úÿ.. */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7D7D"            /* ........ÿR}}}}}} */
	$"A7A7 FEFF 00FB FFF9 0000 0000 00F7 FFFF"            /* §§þÿ.ûÿù.....÷ÿÿ */
	$"FF00 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿ......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7D7D"            /* ........ÿR}}}}}} */
	$"A7A7 ADFF 00F5 F500 0000 0000 0000 0000"            /* §§­ÿ.õõ......... */
	$"F9FF 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* ùÿ.....ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7D7D"            /* ........ÿR}}}}}} */
	$"A7A7 ADFF 0000 0000 0000 0000 FFFF FFF9"            /* §§­ÿ........ÿÿÿù */
	$"FB81 FF00 0000 00FF FFFF FFFF FFFF FFFF"            /* ûÿ....ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF52 7D7D 7D7D 7D7D"            /* ........ÿR}}}}}} */
	$"A7A7 FFFB 0000 0000 0000 0000 00F8 FFFF"            /* §§ÿû.........øÿÿ */
	$"812B FF00 0000 00FF FFFF FFFF FFFF FFFF"            /* +ÿ....ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF7D 7D7D 7D7D 7D7D"            /* ........ÿ}}}}}}} */
	$"A7A7 ADFF 2B00 0000 0000 0000 0000 F5F6"            /* §§­ÿ+.........õö */
	$"00F8 FF00 0000 00FF FFFF FFFF FFFF FFFF"            /* .øÿ....ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 FF7D 7D7D 7D7D 7D7D"            /* ........ÿ}}}}}}} */
	$"A7A7 ADAD FF2B 0000 FFF7 0000 0000 0000"            /* §§­­ÿ+..ÿ÷...... */
	$"00FF 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .ÿ.....ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 00AC ACFF FFFF FFFF FFFF FFFF"            /* .....¬¬ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF ADFF FFFF 00FF FFFF FFFF FFFF"            /* ÿÿÿÿ­ÿÿÿ.ÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* ÿ......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 00FF FFFF FFFF FFFF FFFF"            /* .......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF69 6833 3200"            /* ÿÿÿÿÿÿÿÿÿÿÿih32. */
	$"0008 98FF 00F0 00A4 FF86 00A4 FF86 00A4"            /* ..˜ÿ.ð.¤ÿ†.¤ÿ†.¤ */
	$"FF86 00A4 FF86 008C FF83 008F FF86 008B"            /* ÿ†.¤ÿ†.Œÿƒ.ÿ†.‹ */
	$"FF02 0054 9F80 6601 4C00 8EFF 8600 8AFF"            /* ÿ..TŸ€f.L.Žÿ†.Šÿ */
	$"0300 4E74 9F81 6600 008E FF86 008A FF02"            /* ..NtŸf..Žÿ†.Šÿ. */
	$"006B 9F82 6601 4C00 8DFF 8600 89FF 0200"            /* .kŸ‚f.L.ÿ†.‰ÿ.. */
	$"599F 8366 014C 008D FF86 0083 FF82 0003"            /* YŸƒf.L.ÿ†.ƒÿ‚.. */
	$"FF00 709F 8266 024C 4C00 8DFF 8600 82FF"            /* ÿ.pŸ‚f.LL.ÿ†.‚ÿ */
	$"0800 569F 6666 3300 459F 8366 034C 3400"            /* ..VŸff3.EŸƒf.L4. */
	$"FF80 0089 FF86 0082 FF01 009F 8066 034F"            /* ÿ€.‰ÿ†.‚ÿ..Ÿ€f.O */
	$"0062 9F83 6603 4C2C 0000 804C 0000 88FF"            /* .bŸƒf.L,..€L..ˆÿ */
	$"8600 81FF 0900 639F 6666 6046 0077 9F82"            /* †.ÿÆ.cŸff`F.wŸ‚ */
	$"6609 4C4C 0000 9F66 664C 4C00 87FF 8600"            /* fÆLL..ŸffLL.‡ÿ†. */
	$"81FF 0100 9F80 6603 563A 009F 8366 034C"            /* ÿ..Ÿ€f.V:.Ÿƒf.L */
	$"4C00 9F81 6601 4C00 87FF 8600 81FF 0100"            /* L.Ÿf.L.‡ÿ†.ÿ.. */
	$"9F80 6603 4F00 5C9F 8366 034C 4C00 9F81"            /* Ÿ€f.O.\Ÿƒf.LL.Ÿ */
	$"6601 4C00 87FF 8600 80FF 0800 579F 6666"            /* f.L.‡ÿ†.€ÿ..WŸff */
	$"634C 009F 8466 034C 0032 9F81 6601 4C00"            /* cL.Ÿ„f.L.2Ÿf.L. */
	$"87FF 8600 80FF 0100 9F80 6603 614C 009F"            /* ‡ÿ†.€ÿ..Ÿ€f.aL.Ÿ */
	$"8366 044C 4C00 4C79 8166 0159 0087 FF86"            /* ƒf.LL.Lyf.Y.‡ÿ† */
	$"0080 FF01 009F 8166 025B 009F 8366 044C"            /* .€ÿ..Ÿf.[.Ÿƒf.L */
	$"4C00 4C7F 8166 0159 0087 FF86 0080 FF01"            /* L.L.f.Y.‡ÿ†.€ÿ. */
	$"009F 8166 024D 5C9F 8266 054C 0000 284C"            /* .Ÿf.M\Ÿ‚f.L..(L */
	$"8681 6601 5900 87FF 8600 80FF 0100 9F81"            /* †f.Y.‡ÿ†.€ÿ..Ÿ */
	$"6601 3D9F 8366 0100 6085 6601 5700 87FF"            /* f.=Ÿƒf..`…f.W.‡ÿ */
	$"8600 81FF 0000 8166 0100 9F82 6602 5F63"            /* †.ÿ..f..Ÿ‚f._c */
	$"6385 6601 4C00 87FF 8600 82FF 0500 5966"            /* c…f.L.‡ÿ†.‚ÿ..Yf */
	$"6600 9F82 6602 3000 0084 6603 644C 0000"            /* f.Ÿ‚f.0..„f.dL.. */
	$"86FF 8600 83FF 0400 003C 009F 8166 035E"            /* †ÿ†.ƒÿ...<.Ÿf.^ */
	$"00FF 0084 6604 591B 003E 0085 FF86 0085"            /* .ÿ.„f.Y..>.…ÿ†.… */
	$"FF02 004C 9F81 6604 00A0 FF00 6680 0007"            /* ÿ..LŸf.. ÿ.f€.. */
	$"5E60 5639 008F A600 85FF 8600 85FF 0200"            /* ^`V9.¦.…ÿ†.…ÿ.. */
	$"539F 8166 0700 BFFF 7300 6187 7781 0003"            /* SŸf..¿ÿs.a‡w.. */
	$"A2FF B500 85FF 8600 85FF 0200 9F77 8166"            /* ¢ÿµ.…ÿ†.…ÿ..Ÿwf */
	$"0100 B082 FF07 EFFF FFDF FFD5 DF00 86FF"            /* ..°‚ÿ.ïÿÿßÿÕß.†ÿ */
	$"8600 85FF 0200 9F70 8166 0159 0081 FF07"            /* †.…ÿ..Ÿpf.Y.ÿ. */
	$"AF00 70FF D07F 0000 87FF 8600 85FF 0200"            /* ¯.pÿÐ...‡ÿ†.…ÿ.. */
	$"9F6B 8266 0437 0000 FFFF 8000 05FF FFE0"            /* Ÿk‚f.7..ÿÿ€..ÿÿà */
	$"FF70 0086 FF86 0085 FF01 009F 8366 074C"            /* ÿp.†ÿ†.…ÿ..Ÿƒf.L */
	$"4C15 00FF 5000 8082 FF00 B080 0083 FF86"            /* L..ÿP.€‚ÿ.°€.ƒÿ† */
	$"0085 FF01 009F 8266 0760 4C4C 2C00 FFE0"            /* .…ÿ..Ÿ‚f.`LL,.ÿà */
	$"E086 FF01 8400 82FF 8600 85FF 0100 9F82"            /* à†ÿ.„.‚ÿ†.…ÿ..Ÿ‚ */
	$"6604 604C 4C2C 0085 FF80 0003 8F50 6000"            /* f.`LL,.…ÿ€..P`. */
	$"81FF 8600 85FF 0100 9F82 6604 604C 4C00"            /* ÿ†.…ÿ..Ÿ‚f.`LL. */
	$"5686 FF05 A000 0060 CF00 81FF 8600 85FF"            /* V†ÿ. ..`Ï.ÿ†.…ÿ */
	$"0100 6D82 6605 604C 4C2C 00CF 86FF 04EF"            /* ..m‚f.`LL,.Ï†ÿ.ï */
	$"DFFF AF00 81FF 8600 85FF 0000 8366 0A60"            /* ßÿ¯.ÿ†.…ÿ..ƒfÂ` */
	$"4C4C 2C2C 00C0 FFFF 00BF 84FF 0000 82FF"            /* LL,,.Àÿÿ.¿„ÿ..‚ÿ */
	$"8600 82FF 013F 3F8A 0000 2C80 0000 FF85"            /* †.‚ÿ.??Š..,€..ÿ… */
	$"0083 FF86 00A4 FF86 00A4 FF86 00A4 FFFF"            /* .ƒÿ†.¤ÿ†.¤ÿ†.¤ÿÿ */
	$"00EF 00FF 00F0 00A4 FF86 00A4 FF86 00A4"            /* .ï.ÿ.ð.¤ÿ†.¤ÿ†.¤ */
	$"FF86 00A4 FF86 008C FF83 008F FF86 008B"            /* ÿ†.¤ÿ†.Œÿƒ.ÿ†.‹ */
	$"FF02 007D C980 9901 7100 8EFF 8600 8AFF"            /* ÿ..}É€™.q.Žÿ†.Šÿ */
	$"0300 74A3 C981 9900 008E FF86 008A FF02"            /* ..t£É™..Žÿ†.Šÿ. */
	$"009C C982 9901 7100 8DFF 8600 89FF 0200"            /* .œÉ‚™.q.ÿ†.‰ÿ.. */
	$"86C9 8399 0171 008D FF86 0083 FF82 0003"            /* †Éƒ™.q.ÿ†.ƒÿ‚.. */
	$"FF00 9FC9 8299 0271 7100 8DFF 8600 82FF"            /* ÿ.ŸÉ‚™.qq.ÿ†.‚ÿ */
	$"0800 7BC9 9999 4C00 64C9 8399 0371 4A00"            /* ..{É™™L.dÉƒ™.qJ. */
	$"FF80 0089 FF86 0082 FF01 00C9 8099 0377"            /* ÿ€.‰ÿ†.‚ÿ..É€™.w */
	$"0091 C983 9903 713D 0000 8071 0000 88FF"            /* .‘Éƒ™.q=..€q..ˆÿ */
	$"8600 81FF 0900 88C9 9999 8F69 00A4 C982"            /* †.ÿÆ.ˆÉ™™i.¤É‚ */
	$"9909 7171 0000 C999 9971 7100 87FF 8600"            /* ™Æqq..É™™qq.‡ÿ†. */
	$"81FF 0100 C980 9903 8057 00C9 8399 0371"            /* ÿ..É€™.€W.Éƒ™.q */
	$"7100 C981 9901 7100 87FF 8600 81FF 0100"            /* q.É™.q.‡ÿ†.ÿ.. */
	$"C980 9903 7700 88C9 8399 0371 7100 C981"            /* É€™.w.ˆÉƒ™.qq.É */
	$"9901 7100 87FF 8600 80FF 0800 7CC9 9999"            /* ™.q.‡ÿ†.€ÿ..|É™™ */
	$"9471 00C9 8499 0371 0032 C981 9901 7100"            /* ”q.É„™.q.2É™.q. */
	$"87FF 8600 80FF 0100 C980 9903 9271 00C9"            /* ‡ÿ†.€ÿ..É€™.’q.É */
	$"8399 0471 7100 71A6 8199 0186 0087 FF86"            /* ƒ™.qq.q¦™.†.‡ÿ† */
	$"0080 FF01 00C9 8199 0288 00C9 8399 0471"            /* .€ÿ..É™.ˆ.Éƒ™.q */
	$"7100 71A9 8199 0186 0087 FF86 0080 FF01"            /* q.q©™.†.‡ÿ†.€ÿ. */
	$"00C9 8199 0273 88C9 8299 0571 0000 3171"            /* .É™.sˆÉ‚™.q..1q */
	$"AE81 9901 8600 87FF 8600 80FF 0100 C981"            /* ®™.†.‡ÿ†.€ÿ..É */
	$"9901 5AC9 8399 0100 8F85 9901 8300 87FF"            /* ™.ZÉƒ™..…™.ƒ.‡ÿ */
	$"8600 81FF 0000 8199 0100 C982 9902 8D95"            /* †.ÿ..™..É‚™.• */
	$"9485 9901 7100 87FF 8600 82FF 0500 8699"            /* ”…™.q.‡ÿ†.‚ÿ..†™ */
	$"9900 C982 9902 4800 0084 9903 9771 0000"            /* ™.É‚™.H..„™.—q.. */
	$"86FF 8600 83FF 0400 0058 00C9 8199 038D"            /* †ÿ†.ƒÿ...X.É™. */
	$"00FF 0084 9904 8628 003E 0085 FF86 0085"            /* .ÿ.„™.†(.>.…ÿ†.… */
	$"FF02 0073 C981 9904 00A1 FF00 9980 0007"            /* ÿ..sÉ™..¡ÿ.™€.. */
	$"8D90 8157 008F A600 85FF 8600 85FF 0200"            /* W.¦.…ÿ†.…ÿ.. */
	$"7CC9 8199 0700 BFFF 7400 628B 7B81 0003"            /* |É™..¿ÿt.b‹{.. */
	$"A2FF B500 85FF 8600 85FF 0200 C9A4 8199"            /* ¢ÿµ.…ÿ†.…ÿ..É¤™ */
	$"0100 B182 FF07 EFFF FFDF FFD5 DF00 86FF"            /* ..±‚ÿ.ïÿÿßÿÕß.†ÿ */
	$"8600 85FF 0200 C99F 8199 0186 0081 FF07"            /* †.…ÿ..ÉŸ™.†.ÿ. */
	$"AF00 70FF D07F 0000 87FF 8600 85FF 0200"            /* ¯.pÿÐ...‡ÿ†.…ÿ.. */
	$"C99C 8299 0452 0000 FFFF 8000 05FF FFE0"            /* Éœ‚™.R..ÿÿ€..ÿÿà */
	$"FF70 0086 FF86 0085 FF01 00C9 8399 0771"            /* ÿp.†ÿ†.…ÿ..Éƒ™.q */
	$"7115 00FF 5000 8082 FF00 B080 0083 FF86"            /* q..ÿP.€‚ÿ.°€.ƒÿ† */
	$"0085 FF01 00C9 8299 078F 7171 3D00 FFE0"            /* .…ÿ..É‚™.qq=.ÿà */
	$"E086 FF01 8400 82FF 8600 85FF 0100 C982"            /* à†ÿ.„.‚ÿ†.…ÿ..É‚ */
	$"9904 8F71 713D 0085 FF80 0003 8F50 6000"            /* ™.qq=.…ÿ€..P`. */
	$"81FF 8600 85FF 0100 C982 9904 8F71 7100"            /* ÿ†.…ÿ..É‚™.qq. */
	$"5686 FF05 A000 0060 CF00 81FF 8600 85FF"            /* V†ÿ. ..`Ï.ÿ†.…ÿ */
	$"0100 9E82 9905 8F71 713D 00CF 86FF 04EF"            /* ..ž‚™.qq=.Ï†ÿ.ï */
	$"DFFF AF00 81FF 8600 85FF 0000 8399 0A8F"            /* ßÿ¯.ÿ†.…ÿ..ƒ™Â */
	$"7171 3D3D 00C0 FFFF 00BF 84FF 0000 82FF"            /* qq==.Àÿÿ.¿„ÿ..‚ÿ */
	$"8600 82FF 013F 3F8A 0000 3D80 0000 FF85"            /* †.‚ÿ.??Š..=€..ÿ… */
	$"0083 FF86 00A4 FF86 00A4 FF86 00A4 FFFF"            /* .ƒÿ†.¤ÿ†.¤ÿ†.¤ÿÿ */
	$"00EF 00FF 00F0 00A4 FF86 00A4 FF86 00A4"            /* .ï.ÿ.ð.¤ÿ†.¤ÿ†.¤ */
	$"FF86 00A4 FF86 008C FF83 008F FF86 008B"            /* ÿ†.¤ÿ†.Œÿƒ.ÿ†.‹ */
	$"FF02 0001 4A82 008E FF86 008A FF03 0002"            /* ÿ...J‚.Žÿ†.Šÿ... */
	$"184A 8200 8EFF 8600 8AFF 0200 084A 8400"            /* .J‚.Žÿ†.Šÿ...J„. */
	$"8DFF 8600 89FF 0200 004A 8500 8DFF 8600"            /* ÿ†.‰ÿ...J….ÿ†. */
	$"83FF 8200 03FF 0010 4A85 008D FF86 0082"            /* ƒÿ‚..ÿ..J….ÿ†.‚ */
	$"FF02 000D 4A81 0001 054A 8400 0207 00FF"            /* ÿ...J...J„....ÿ */
	$"8000 89FF 8600 82FF 0100 4A82 0001 044A"            /* €.‰ÿ†.‚ÿ..J‚...J */
	$"8400 0009 8300 88FF 8600 81FF 0200 184A"            /* „..Æƒ.ˆÿ†.ÿ...J */
	$"8200 011C 4A86 0000 4A82 0087 FF86 0081"            /* ‚...J†..J‚.‡ÿ†. */
	$"FF01 004A 8300 004A 8600 004A 8300 87FF"            /* ÿ..Jƒ..J†..Jƒ.‡ÿ */
	$"8600 81FF 0100 4A82 0001 044A 8600 004A"            /* †.ÿ..J‚...J†..J */
	$"8300 87FF 8600 80FF 0200 104A 8200 004A"            /* ƒ.‡ÿ†.€ÿ...J‚..J */
	$"8600 0132 4A83 0087 FF86 0080 FF01 004A"            /* †..2Jƒ.‡ÿ†.€ÿ..J */
	$"8300 004A 8700 0020 8300 87FF 8600 80FF"            /* ƒ..J‡.. ƒ.‡ÿ†.€ÿ */
	$"0100 4A83 0000 4A87 0000 2983 0087 FF86"            /* ..Jƒ..J‡..)ƒ.‡ÿ† */
	$"0080 FF01 004A 8200 0104 4A85 0002 1500"            /* .€ÿ..J‚...J….... */
	$"3583 0087 FF86 0080 FF01 004A 8200 004A"            /* 5ƒ.‡ÿ†.€ÿ..J‚..J */
	$"8F00 87FF 8600 81FF 8300 004A 8F00 87FF"            /* .‡ÿ†.ÿƒ..J.‡ÿ */
	$"8600 82FF 8200 004A 9000 86FF 8600 83FF"            /* †.‚ÿ‚..J.†ÿ†.ƒÿ */
	$"0400 0003 004A 8300 00FF 8800 013E 0085"            /* .....Jƒ..ÿˆ..>.… */
	$"FF86 0085 FF02 0000 4A82 0001 9EFF 8700"            /* ÿ†.…ÿ...J‚..žÿ‡. */
	$"028F A600 85FF 8600 85FF 0200 004A 8200"            /* .¦.…ÿ†.…ÿ...J‚. */
	$"06BF FF6F 005F 8070 8100 03A2 FFB5 0085"            /* .¿ÿo._€p..¢ÿµ.… */
	$"FF86 0085 FF02 004A 1C82 0000 AE82 FF07"            /* ÿ†.…ÿ..J.‚..®‚ÿ. */
	$"EFFF FFDF FFD5 DF00 86FF 8600 85FF 0200"            /* ïÿÿßÿÕß.†ÿ†.…ÿ.. */
	$"4A10 8300 81FF 07AF 0070 FFD0 7F00 0087"            /* J.ƒ.ÿ.¯.pÿÐ...‡ */
	$"FF86 0085 FF02 004A 0885 0001 FFFF 8000"            /* ÿ†.…ÿ..J.…..ÿÿ€. */
	$"05FF FFE0 FF70 0086 FF86 0085 FF01 004A"            /* .ÿÿàÿp.†ÿ†.…ÿ..J */
	$"8500 0515 00FF 5000 8082 FF00 B080 0083"            /* …....ÿP.€‚ÿ.°€.ƒ */
	$"FF86 0085 FF01 004A 8500 0409 00FF E0E0"            /* ÿ†.…ÿ..J…..Æ.ÿàà */
	$"86FF 0184 0082 FF86 0085 FF01 004A 8500"            /* †ÿ.„.‚ÿ†.…ÿ..J…. */
	$"0109 0085 FF80 0003 8F50 6000 81FF 8600"            /* .Æ.…ÿ€..P`.ÿ†. */
	$"85FF 0100 4A86 0000 5686 FF05 A000 0060"            /* …ÿ..J†..V†ÿ. ..` */
	$"CF00 81FF 8600 85FF 0100 0C85 0002 0900"            /* Ï.ÿ†.…ÿ...…..Æ. */
	$"CF86 FF04 EFDF FFAF 0081 FF86 0085 FF87"            /* Ï†ÿ.ïßÿ¯.ÿ†.…ÿ‡ */
	$"0007 0909 00C0 FFFF 00BF 84FF 0000 82FF"            /* ..ÆÆ.Àÿÿ.¿„ÿ..‚ÿ */
	$"8600 82FF 013F 3F8A 0000 0980 0000 FF85"            /* †.‚ÿ.??Š..Æ€..ÿ… */
	$"0083 FF86 00A4 FF86 00A4 FF86 00A4 FFFF"            /* .ƒÿ†.¤ÿ†.¤ÿ†.¤ÿÿ */
	$"00EF 0068 386D 6B00 0009 0800 0000 0000"            /* .ï.h8mk..Æ...... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 00FF"            /* ...............ÿ */
	$"FFFF FFFF FF00 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿ........... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 FFFF"            /* ..............ÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 00FF FFFF"            /* .............ÿÿÿ */
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 00FF FFFF"            /* .............ÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 FFFF FFFF"            /* ............ÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 FFFF FFFF FF00 FFFF FFFF"            /* ......ÿÿÿÿÿ.ÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿÿ......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF00 FFFF FF00 0000 0000"            /* ÿÿÿÿÿÿÿ.ÿÿÿ..... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF 0000 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ...ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ....ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 FFFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FF00"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF00 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿ.. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 0000 0000 0000 0000"            /* ÿ............... */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 0000 0000 0000 0000 0000 0000"            /* ÿÿ.............. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ............. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ............. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF00 0000 0000 0000 0000 0000 0000"            /* ÿÿÿ............. */
	$"0000 0000 0000 0000 FFFF FFFF FFFF FFFF"            /* ........ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF 0000 0000 0000 0000 0000 0000 0000"            /* ÿÿ.............. */
	$"0000 0000 00FF FFFF FFFF FFFF FFFF FFFF"            /* .....ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 00FF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿ.ÿÿÿÿÿÿÿ */
	$"FF00 0000 0000 0000 0000 0000 0000 0000"            /* ÿ............... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0069 7433 3200"            /* ...........it32. */
	$"0056 9B00 0000 00FF FFFF FFFF FFFF FFAE"            /* .V›....ÿÿÿÿÿÿÿÿ® */
	$"FF0A D39F 7C5D 5150 5B78 9ECA FDEF FF03"            /* ÿÂÓŸ|]QP[xžÊýïÿ. */
	$"FCC9 6107 8500 0303 44B0 EEEC FF0C AD3C"            /* üÉa.…...D°îìÿ.­< */
	$"0100 000F 1F2C 2F2E 291C 0D80 0002 1E7D"            /* .....,/.)..€...} */
	$"E9E8 FF08 D661 0000 0C33 5364 6880 6B0A"            /* éèÿ.Öa...3Sdh€kÂ */
	$"6965 5F4E 320E 0000 269E FBE5 FF09 BF18"            /* ie_N2...&žûåÿÆ¿. */
	$"0001 3E63 6D6C 6867 8466 0964 583E 0E00"            /* ..>cmlhg„fÆdX>.. */
	$"0063 FAFF FEE1 FF09 C718 0011 4D6D 6C6B"            /* .cúÿþáÿÆÇ...Mmlk */
	$"6A68 8466 0C65 625E 5E4D 1B00 007E FEF4"            /* jh„f.eb^^M...~þô */
	$"F5FE D9FF 00FB 80F8 0BFC C516 002B 5F6E"            /* õþÙÿ.û€ø.üÅ..+_n */
	$"6C70 716E 6986 660A 625D 5953 2300 0DB0"            /* lpqni†fÂb]YS#..° */
	$"E1D6 F0D8 FF10 FEBD A8AB B27D 0C00 366C"            /* áÖðØÿ.þ½¨«²}..6l */
	$"6B6E 7579 766E 6887 6609 625C 5552 1F00"            /* knuyvnh‡fÆb\UR.. */
	$"44DE D6E7 D8FF 10FA B3AD B4A3 1C00 226A"            /* DÞÖçØÿ.ú³­´£.."j */
	$"696C 767D 7D76 6C67 8766 0965 6058 5443"            /* ilv}}vlg‡fÆe`XTC */
	$"0304 A5DD E1D8 FF0F F1C3 C4CC 4700 1460"            /* ..¥ÝáØÿ.ñÃÄÌG..` */
	$"6A6B 7580 837C 7168 8966 0963 5C53 531D"            /* jku€ƒ|qh‰fÆc\SS. */
	$"003B C1C7 FCD7 FF0E F1DC E77F 0013 5D6A"            /* .;ÁÇü×ÿ.ñÜç...]j */
	$"6B75 8188 8276 6B8A 6609 655F 5654 3A00"            /* kuˆ‚vkŠfÆe_VT:. */
	$"07AA CEFC D7FF 0EF2 E9DC 2300 496D 6972"            /* .ªÎü×ÿ.òéÜ#.Imir */
	$"818A 8A7C 6F68 8A66 0965 6058 534A 0800"            /* ŠŠ|ohŠfÆe`XSJ.. */
	$"76B9 FAD7 FF0E D4A8 4500 2469 686E 7D8B"            /* v¹ú×ÿ.Ô¨E.$ihn}‹ */
	$"8F85 736A 678B 6608 625A 5451 1A00 4FAF"            /* …sjg‹f.bZTQ..O¯ */
	$"FAD7 FF0D EF82 0008 5A6B 6B78 8A93 8D7C"            /* ú×ÿ.ï‚..ZkkxŠ“| */
	$"6C67 8C66 0863 5C55 542A 0035 AEF7 D7FF"            /* lgŒf.c\UT*.5®÷×ÿ */
	$"0CE1 3700 336D 6972 8493 9285 7268 8D66"            /* .á7.3mir„“’…rhf */
	$"0863 5D55 5435 001B 6FEF D7FF 0CA6 020C"            /* .c]UT5..oï×ÿ.¦.. */
	$"5C6A 6E7B 8E96 8D7C 6D67 8D66 0863 5D56"            /* \jn{Ž–|mgf.c]V */
	$"533D 0011 B0FA D7FF 0B59 0046 6D6B 7788"            /* S=..°ú×ÿ.Y.Fmkwˆ */
	$"9794 8373 698E 6608 635D 5652 4300 0DA9"            /* —”ƒsiŽf.c]VRC..© */
	$"F4D6 FF0B C50B 1266 6971 8494 9A8C 786C"            /* ôÖÿ.Å..fiq„”šŒxl */
	$"8F66 0863 5D56 5145 0406 6EF3 D6FF 0B56"            /* f.c]VQE..nóÖÿ.V */
	$"0038 6D6B 788E 9B97 8370 688F 6608 635C"            /* .8mkxŽ›—ƒphf.c\ */
	$"5551 4605 0462 F8C4 FF05 FDF2 E3D9 E2F1"            /* UQF..bøÄÿ.ýòãÙâñ */
	$"88FF 0BDD 0D04 5A6B 7184 979C 8F79 6B90"            /* ˆÿ.Ý..Zkq„—œyk */
	$"6608 635B 5451 4605 0463 F7C1 FF0B F5B8"            /* f.c[TQF..c÷Áÿ.õ¸ */
	$"6F36 1507 0307 1647 8EDB 85FF 0B86 0026"            /* o6.....GŽÛ…ÿ.†.& */
	$"6B6C 7A90 9D98 8571 6790 6608 615A 5350"            /* klz˜…qgf.aZSP */
	$"4503 0886 FAC0 FF0D BB38 0104 1D2C 312B"            /* E..†úÀÿ.»8...,1+ */
	$"1B07 0000 15A3 84FF 0A37 0047 6D71 8398"            /* .....£„ÿÂ7.Gmqƒ˜ */
	$"9D92 7C6B 9166 0860 5952 5043 0010 C1FE"            /* ’|k‘f.`YRPC..Áþ */
	$"BEFF 06DA 560C 2A52 676A 806B 0668 6042"            /* ¾ÿ.ÚV.*Rgj€k.h`B */
	$"1300 007F 82FF 0BCF 0411 636D 7A8D 9E99"            /* ....‚ÿ.Ï..cmzž™ */
	$"8874 6890 6608 655F 5751 503E 000D AAB8"            /* ˆthf.e_WQP>..ª¸ */
	$"FF0C FDDD DADB E1D9 7811 1D61 7672 6A81"            /* ÿ.ýÝÚÛáÙx..avrj */
	$"6680 6704 5A33 0200 8781 FF0B 7300 436E"            /* f€g.Z3..‡ÿ.s.Cn */
	$"7285 979F 917D 6E67 9066 0864 5D54 4F51"            /* r…—Ÿ‘}ngf.d]TOQ */
	$"3700 1B93 B8FF 0CFC CFCA CBD8 5D00 3576"            /* 7..“¸ÿ.üÏÊËØ].5v */
	$"7B76 6F69 8366 1464 605D 3500 00B6 FFFF"            /* {voiƒf.d`]5..¶ÿÿ */
	$"F720 0460 6C79 909E 9C87 746A 9166 0863"            /* ÷ .`lyžœ‡tj‘f.c */
	$"5B53 4F52 3100 3EE7 B9FF 0BDF D5C3 8300"            /* [SOR1.>ç¹ÿ.ßÕÃƒ. */
	$"3A7A 7F7E 766E 6883 6614 6561 5A55 1A00"            /* :z.~vnhƒf.eaZU.. */
	$"5CFF FFC0 0020 6A6F 8297 A096 7F6F 6791"            /* \ÿÿÀ. jo‚— –.og‘ */
	$"6608 6159 514D 5128 0030 C3B9 FF0A D9D0"            /* f.aYQMQ(.0Ã¹ÿÂÙÐ */
	$"7B00 2A7A 8187 8073 6B85 6612 625B 5642"            /* {.*z‡€sk…f.b[VB */
	$"0119 F0FE 4D00 4470 768D 9E9F 8C76 6A91"            /* ..ðþM.DpvžŸŒvj‘ */
	$"6609 655F 564F 4D51 1D00 63EE B9FF 0AE8"            /* fÆe_VOMQ..cî¹ÿÂè */
	$"9A06 106D 838A 897B 6D68 8566 1262 5C54"            /* š..mƒŠ‰{mh…f.b\T */
	$"5112 01D4 C400 0760 717E 96A1 9982 6E67"            /* Q..ÔÄ..`q~–¡™‚ng */
	$"9166 0963 5C54 4E4D 4E10 0078 EC84 FF05"            /* ‘fÆc\TNMN..xì„ÿ. */
	$"E4CE C8CC D1EE ACFF 09EC 4600 437F 898F"            /* äÎÈÌÑî¬ÿÆìF.C.‰ */
	$"8674 6986 6611 615A 5254 2400 BA7F 0021"            /* †ti†f.aZRT$.º..! */
	$"6C74 869C A092 7A6A 9266 0962 5A52 4D4D"            /* lt†œ ’zj’fÆbZRMM */
	$"4906 0274 DE81 FF0B DA95 4317 0502 0307"            /* I..tÞÿ.Ú•C..... */
	$"2468 A7E5 A9FF 09C8 041E 7084 908E 806F"            /* $h§å©ÿÆÈ..p„Ž€o */
	$"6785 6612 6560 5751 5332 01A0 3300 4072"            /* g…f.e`WQS2. 3.@r */
	$"7C8F A09B 8973 6891 660F 6560 5850 4C50"            /* | ›‰sh‘f.e`XPLP */
	$"3E00 1397 F2FF FFBF 4709 8000 0302 0504"            /* >..—òÿÿ¿GÆ€..... */
	$"0180 0002 1465 CFA7 FF08 6900 5B7F 8C93"            /* .€...eÏ§ÿ.i.[.Œ“ */
	$"8877 6A86 6611 645D 554F 5136 0965 0505"            /* ˆwj†f.d]UOQ6Æe.. */
	$"5C75 8698 A093 7F6D 9266 2163 5D56 4F4C"            /* \u†˜ “.m’f!c]VOL */
	$"512F 0038 B2FF F57B 0500 0221 3D4E 585D"            /* Q/.8²ÿõ{...!=NX] */
	$"5C56 4B36 2003 0006 59A8 DAC5 ECA1 FF09"            /* \VK6 ...Y¨ÚÅì¡ÿÆ */
	$"F215 1775 8392 9080 6F67 8666 1162 5A53"            /* ò..uƒ’€og†f.bZS */
	$"4D53 2B05 1C00 1B6B 788F 9E9D 8A77 6992"            /* MS+....kxžŠwi’ */
	$"6615 625B 534D 4C51 1F00 56BD D342 0000"            /* f.b[SMLQ..V½ÓB.. */
	$"2C5B 696D 6A69 6868 8069 0964 5427 0000"            /* ,[imjihh€iÆdT'.. */
	$"16B2 B2E1 FAA0 FF08 B700 3C80 8A94 8979"            /* .²²áú ÿ.·.<€Š”‰y */
	$"6B86 6612 655F 5751 4D51 2000 0400 3B72"            /* k†f.e_WQMQ ...;r */
	$"7F95 A099 8271 6891 6611 655F 5851 4C4D"            /* .• ™‚qh‘f.e_XQLM */
	$"4E0E 078F D664 0001 3B6E 6D68 8566 0965"            /* N..Öd..;nmh…fÆe */
	$"645E 390D 0032 A8E6 F69F FF09 FB4B 0163"            /* d^9..2¨æöŸÿÆûK.c */
	$"8692 9281 7168 8666 1163 5D54 4F4D 4D12"            /* †’’qh†f.c]TOMM. */
	$"0000 0F62 7488 9CA0 9079 6C92 6611 635D"            /* ...btˆœ yl’f.c] */
	$"544F 4C4F 4400 2CC3 A404 0344 7873 6D68"            /* TOLOD.,Ã¤..Dxsmh */
	$"8666 0865 615F 4B0F 0035 CCF3 9FFF 09CA"            /* †f.ea_K..5ÌóŸÿÆÊ */
	$"001D 7A8B 948D 796C 6786 6611 615A 514D"            /* ..z‹”ylg†f.aZQM */
	$"4D48 0600 002A 6E78 8F9F 9C88 7269 9266"            /* MH...*nxŸœˆri’f */
	$"1162 5A52 4E4C 5233 0055 D147 0041 7C7D"            /* .bZRNLR3.UÑG.A|} */
	$"7870 6887 6607 635E 5B40 0200 83FB 9FFF"            /* xph‡f.c^[@..ƒûŸÿ */
	$"0889 003E 8590 9485 7268 8666 1265 6057"            /* .‰.>…”…rh†f.e`W */
	$"504D 4E3E 0200 0042 727E 96A0 9880 6F68"            /* PMN>...Br~– ˜€oh */
	$"9166 1265 6058 514D 4C52 2000 73B8 0F10"            /* ‘f.e`XQMLR .s¸.. */
	$"7082 857B 7068 8766 0665 615A 551A 004C"            /* p‚…{ph‡f.eaZU..L */
	$"9FFF 09FC 3C00 618A 9490 7D6C 6786 6612"            /* ŸÿÆü<.aŠ”}lg†f. */
	$"635D 544F 4C51 3000 0006 5D73 869C 9E90"            /* c]TOLQ0...]s†œž */
	$"786A 6791 6612 645E 5550 4C4D 4E09 0DB2"            /* xjg‘f.d^UPLMNÆ.² */
	$"7A00 3B84 8A89 7A6C 6788 6606 635E 5936"            /* z.;„Š‰zlgˆf.c^Y6 */
	$"0018 EE9E FF08 DB08 1779 8E94 8A76 6987"            /* ..îžÿ.Û..yŽ”Švi‡ */
	$"6611 615A 524D 4C50 2300 001D 6D78 8E9E"            /* f.aZRMLP#...mxŽž */
	$"9A87 7168 9266 1163 5A53 4E4C 5234 0042"            /* š‡qh’f.cZSNLR4.B */
	$"AB41 015B 898F 8773 6889 6606 6460 594A"            /* «A.[‰‡sh‰f.d`YJ */
	$"0601 BC9E FF08 A900 3884 9192 8471 6886"            /* ..¼žÿ.©.8„‘’„qh† */
	$"6611 655F 5750 4C4C 4E17 0000 3474 7E93"            /* f.e_WPLLN...4t~“ */
	$"9E94 806D 9266 1265 6158 514D 4C51 1B00"            /* ž”€m’f.eaXQMLQ.. */
	$"63A2 1604 6B8C 9184 7067 8966 0665 6259"            /* c¢..kŒ‘„pg‰f.ebY */
	$"541E 008A 9EFF 0869 0166 8793 8E7E 6D67"            /* T..Šžÿ.i.f‡“Ž~mg */
	$"8666 1163 5D56 4F4C 4D4A 0B00 014F 7784"            /* †f.c]VOLMJ...Ow„ */
	$"999D 8C79 6A92 6611 645F 564F 4C4E 4A08"            /* ™Œyj’f.d_VOLNJ. */
	$"016B C125 0976 8E91 826F 8B66 0563 5A57"            /* .kÁ%ÆvŽ‘‚o‹f.cZW */
	$"3600 579D FF08 F827 1D7B 8C93 8878 6987"            /* 6.Wÿ.ø'.{Œ“ˆxi‡ */
	$"6611 625A 534E 4C4E 3F02 000E 667A 8C9C"            /* f.bZSNLN?...fzŒœ */
	$"9883 7168 9266 1162 5B53 4E4C 5138 0012"            /* ˜ƒqh’f.b[SNLQ8.. */
	$"A3A9 1B0D 7990 9281 6E8B 6606 645C 5545"            /* £©..y’n‹f.d\UE */
	$"0025 F49C FF08 D702 3E82 8F91 8273 6886"            /* .%ôœÿ.×.>‚‘‚sh† */
	$"6611 655F 5750 4C4C 5133 0000 2470 7F92"            /* f.e_WPLLQ3..$p.’ */
	$"9D92 7B6D 9266 1265 6058 504D 4C51 2500"            /* ’{m’f.e`XPMLQ%. */
	$"39BC 9B18 0F7A 9294 836E 8B66 0664 5D55"            /* 9¼›..z’”ƒn‹f.d]U */
	$"4E09 07D9 9CFF 07A5 0058 8691 8F7D 6E87"            /* NÆ.Ùœÿ.¥.X†‘}n‡ */
	$"6611 645D 554F 4C4C 5127 0000 3976 8497"            /* f.d]UOLLQ'..9v„— */
	$"9B8C 766A 9266 1263 5E56 4F4C 4D4E 1100"            /* ›Œvj’f.c^VOLMN.. */
	$"5DC4 9117 107B 9496 856F 8B66 0665 5E56"            /* ]Ä‘..{”–…o‹f.e^V */
	$"5117 00B8 9CFF 076C 0371 8892 8B78 6C87"            /* Q..¸œÿ.l.qˆ’‹xl‡ */
	$"6611 635B 534F 4C4C 4E17 0002 5379 8C9B"            /* f.c[SOLLN...SyŒ› */
	$"9984 7068 9266 1362 5A54 4E4C 4F41 0200"            /* ™„ph’f.bZTNLOA.. */
	$"75A6 7E14 117C 969A 8871 678A 6606 655F"            /* u¦~..|–šˆqgŠf.e_ */
	$"5655 2600 899B FF08 FE3D 197C 8B93 8774"            /* VU&.‰›ÿ.þ=.|‹“‡t */
	$"6987 6611 6359 524E 4C4D 490A 0014 697B"            /* i‡f.cYRNLMIÂ..i{ */
	$"919C 957D 6C67 9166 1465 6057 514D 4C51"            /* ‘œ•}lg‘f.e`WQMLQ */
	$"2D00 088F 8668 1013 7D97 9C8B 7267 8A66"            /* -..†h..}—œ‹rgŠf */
	$"0665 5F56 5432 005C 9BFF 08F0 1731 828D"            /* .e_VT2.\›ÿ.ð.1‚ */
	$"9183 7068 8766 1062 5850 4D4C 4F40 0200"            /* ‘ƒph‡f.bXPMLO@.. */
	$"3672 7F95 9C90 786A 9266 1464 5E55 504C"            /* 6r.•œxj’f.d^UPL */
	$"4C50 1B00 3AD8 9052 0C15 7D98 9E8D 7367"            /* LP..:ØR..}˜žsg */
	$"8A66 0765 6057 533B 002A F29A FF08 B900"            /* Šf.e`WS;.*òšÿ.¹. */
	$"4B86 9090 7E6D 6786 6611 6560 5750 4D4C"            /* K†~mg†f.e`WPML */
	$"502D 0000 5174 8599 9A89 7368 9266 1463"            /* P-..Qt…™š‰sh’f.c */
	$"5A51 4F4C 4E48 0700 71BF 5B35 0819 7E9A"            /* ZQOLNH..q¿[5..~š */
	$"A191 7668 8B66 0660 5753 4501 05D7 9AFF"            /* ¡‘vh‹f.`WSE..×šÿ */
	$"0776 0061 8691 8D7A 6B87 6610 6560 5750"            /* .v.a†‘zk‡f.e`WP */
	$"4D4C 5023 000C 6477 8C9B 9782 6E92 6615"            /* MLP#..dwŒ›—‚n’f. */
	$"6560 5750 4D4C 5136 0005 8DA1 603D 021D"            /* e`WPMLQ6..¡`=.. */
	$"7F9B A393 7769 8B66 0660 5852 4A08 00B5"            /* .›£“wi‹f.`XRJ..µ */
	$"9AFF 0756 016D 8791 8B77 6987 6610 6560"            /* šÿ.V.m‡‘‹wi‡f.e` */
	$"5750 4D4C 502F 001D 6C7A 909B 937D 6B92"            /* WPMLP/..lz›“}k’ */
	$"6615 645D 544F 4C4C 501D 001E EFF9 D09E"            /* f.d]TOLLP...ïùÐž */
	$"0021 819C A495 7769 8B66 0660 5852 4C10"            /* .!œ¤•wi‹f.`XRL. */
	$"00A7 99FF 08FE 3D0E 7487 9088 7468 8866"            /* .§™ÿ.þ=.t‡ˆthˆf */
	$"0F62 5951 4E4C 5033 0033 737E 949A 8E78"            /* .bYQNLP3.3s~”šŽx */
	$"6992 6615 625B 534E 4C4F 4203 0053 FFFB"            /* i’f.b[SNLOB..Sÿû */
	$"8760 0027 839C A494 7769 8B66 0660 5852"            /* ‡`.'ƒœ¤”wi‹f.`XR */
	$"4F16 008A 99FF 08F7 221F 7888 8F84 7267"            /* O..Š™ÿ.÷".xˆ„rg */
	$"8866 0F64 5D55 504D 4F1D 004C 7784 9897"            /* ˆf.d]UPMO..Lw„˜— */
	$"8773 6891 6616 645F 5850 4D4C 512E 0000"            /* ‡sh‘f.d_XPMLQ... */
	$"8DFF F665 3B00 2C84 9BA2 9175 688B 6606"            /* ÿöe;.,„›¢‘uh‹f. */
	$"6058 5251 1F00 7899 FF07 F212 2B7B 878D"            /* `XRQ..x™ÿ.ò.+{‡ */
	$"8170 8A66 0E61 5B55 5247 0404 5F78 899A"            /* pŠf.a[URG.._x‰š */
	$"9481 6F67 9166 1663 5D55 4F4C 4C4F 1B00"            /* ”og‘f.c]UOLLO.. */
	$"00BE FFE9 AD61 0034 8798 9E8C 7267 8B66"            /* .¾ÿé­a.4‡˜žŒrg‹f */
	$"0660 5851 5125 006E 99FF 07ED 0C32 7C87"            /* .`XQQ%.n™ÿ.í.2|‡ */
	$"8B7F 6E8A 660D 6560 5B59 3D00 1369 7B8E"            /* ‹.nŠf.e`[Y=..i{Ž */
	$"9A8F 7D6C 9266 1562 5B52 4E4C 4D4B 0B00"            /* š}l’f.b[RNLMK.. */
	$"12E8 FFD6 763A 003D 8895 9885 6F8C 6606"            /* .èÿÖv:.=ˆ•˜…oŒf. */
	$"6058 5151 2800 5F99 FF07 E709 377D 8689"            /* `XQQ(._™ÿ.çÆ7}†‰ */
	$"7C6D 8B66 0C64 6263 3100 2972 7F92 9A8B"            /* |m‹f.dbc1.)r.’š‹ */
	$"776A 9166 1665 5F58 514D 4C50 3B01 0045"            /* wj‘f.e_XQMLP;..E */
	$"FEFF F0C3 3200 4985 908E 7D6C 8C66 0660"            /* þÿðÃ2.I…Ž}lŒf.` */
	$"5751 512C 0054 99FF 07E6 0939 7D84 877A"            /* WQQ,.T™ÿ.æÆ9}„‡z */
	$"6C8E 6609 2100 3E77 8396 9885 7369 9166"            /* lŽfÆ!.>wƒ–˜…si‘f */
	$"1664 5C55 504C 4C52 2900 007E FFFF F4D2"            /* .d\UPLLR)..~ÿÿôÒ */
	$"0D00 5881 8984 7469 8B66 0765 6057 5051"            /* ..X‰„ti‹f.e`WPQ */
	$"2E00 4F99 FF07 F10C 387C 8285 796B 8C66"            /* ..O™ÿ.ñ.8|‚…ykŒf */
	$"0B67 6512 004E 7888 9896 8170 6791 6616"            /* .ge..Nxˆ˜–pg‘f. */
	$"6259 524F 504F 4B17 0000 A1FB DAB6 6C00"            /* bYROPOK...¡ûÚ¶l. */
	$"0665 7D81 7B6F 678B 6607 655F 5650 5130"            /* .e}{og‹f.e_VPQ0 */
	$"004E 99FF 07D2 0C34 797F 8177 6B8C 660A"            /* .N™ÿ.Ò.4y.wkŒfÂ */
	$"695B 0307 617A 8E99 927C 6D91 6607 655E"            /* i[..azŽ™’|m‘f.e^ */
	$"5654 4D33 1909 8000 0B1C 363B 2E00 001E"            /* VTM3.Æ€...6;.... */
	$"7077 7872 6A8C 6607 655E 5650 5130 004E"            /* pwxrjŒf.e^VPQ0.N */
	$"94FF 0CF0 F0F1 F5DA 921F 2A75 7B7D 7569"            /* ”ÿ.ððñõÚ’.*u{}ui */
	$"8C66 0A6B 4800 176C 7D92 998E 776A 9166"            /* ŒfÂkH..l}’™Žwj‘f */
	$"0463 5C58 4213 8100 0206 0A04 8100 0606"            /* .c\XB....Â.... */
	$"5270 706F 6B67 8C66 0765 5E56 4F51 2F00"            /* RppokgŒf.e^VOQ/. */
	$"5194 FF0C FBFB E3AC ABB2 321B 7177 7972"            /* Q”ÿ.ûûã¬«²2.qwyr */
	$"698C 660A 6D38 002A 7281 9599 8A74 6990"            /* iŒfÂm8.*r•™Šti */
	$"6616 6561 5F3E 0100 152C 3F4A 5A62 614F"            /* f.ea_>...,?JZbaO */
	$"3925 1541 6D6A 6C6B 688D 6608 645E 554F"            /* 9%.Amjlkhf.d^UO */
	$"512C 004E F395 FF0A F9C5 9FA9 4B08 6B72"            /* Q,.Nó•ÿÂùÅŸ©K.kr */
	$"736E 688C 660A 6A27 0042 7585 9798 8671"            /* snhŒfÂj'.Bu…—˜†q */
	$"6890 6615 6464 4A09 204B 6269 6A6A 6867"            /* hf.ddJÆ Kbijjhg */
	$"686A 6C69 6569 6768 6867 8E66 0964 5D54"            /* hjlieighhgŽfÆd]T */
	$"4F51 2800 2557 D996 FF08 CBB7 9C0D 566F"            /* OQ(.%WÙ–ÿ.Ë·œ.Vo */
	$"6C6A 688B 660A 6766 1306 6076 8A99 9581"            /* ljh‹fÂgf..`vŠ™• */
	$"6D91 6606 655D 2444 6668 6785 6601 6767"            /* m‘f.e]$Dfhg…f.gg */
	$"9366 0964 5C53 4E51 2400 429F EC97 FF07"            /* “fÆd\SNQ$.BŸì—ÿ. */
	$"ECD4 2F33 6E68 6767 8B66 0A68 6004 176C"            /* ìÔ/3nhgg‹fÂh`..l */
	$"778E 9991 7B6B 9166 0367 484E 68A0 6608"            /* wŽ™‘{k‘f.gHNh f. */
	$"635B 524E 501F 0032 D298 FF05 FCCA 5113"            /* c[RNP..2Ò˜ÿ.üÊQ. */
	$"6968 8D66 0A69 5400 276E 7B91 998F 7869"            /* ihfÂiT.'n{‘™xi */
	$"9166 0362 5466 65A0 6608 6259 514E 4E17"            /* ‘f.bTfe f.bYQNN. */
	$"0065 F399 FF04 FBC4 0A45 6D8D 660A 6C45"            /* .eó™ÿ.ûÄÂEmfÂlE */
	$"003A 737E 9499 8B75 6891 6601 6565 A166"            /* .:s~”™‹uh‘f.ee¡f */
	$"0865 6057 504E 4B0D 00B2 9CFF 0377 055C"            /* .e`WPNK..²œÿ.w.\ */
	$"6A8C 660A 6D35 004A 7482 9697 8671 67B7"            /* jŒfÂm5.Jt‚–—†qg· */
	$"6608 645E 564F 4E44 010A DB9C FF04 E913"            /* f.d^VOND.ÂÛœÿ.é. */
	$"1467 698B 6609 6B29 0058 7486 9795 826F"            /* .gi‹fÆk).Xt†—•‚o */
	$"B866 0863 5C54 4E4F 3A00 39F6 9DFF 05AC"            /* ¸f.c\TNO:.9öÿ.¬ */
	$"071E 656B 6788 660A 6769 1B06 6576 8998"            /* ..ekgˆfÂgi..ev‰˜ */
	$"917F 6DB7 6608 6562 5A52 4D51 2B00 819F"            /* ‘.m·f.ebZRMQ+.Ÿ */
	$"FF05 C015 114E 6A6A 8766 0A67 650C 166B"            /* ÿ.À..Njj‡fÂge..k */
	$"798C 988D 7B6A B766 0864 5F57 504D 4E16"            /* yŒ˜{j·f.d_WPMN. */
	$"01C6 A0FF 07BB 1100 275B 6A6C 6884 660A"            /* .Æ ÿ.»..'[jlh„fÂ */
	$"695A 0324 707B 8F97 8B78 69B7 6608 635C"            /* iZ.$p{—‹xi·f.c\ */
	$"554F 4D49 041D F1A1 FF09 D356 0700 1E3F"            /* UOMI..ñ¡ÿÆÓV...? */
	$"5B6A 6C68 8166 0A6C 4A00 3375 7E92 9787"            /* [jlhfÂlJ.3u~’—‡ */
	$"7568 B766 0760 5952 4E51 3600 66A4 FF16"            /* uh·f.`YRNQ6.f¤ÿ. */
	$"CE69 1600 0020 425C 6A6B 6766 6B39 0044"            /* Îi... B\jkgfk9.D */
	$"7682 9595 8371 67B6 6608 645C 5450 4E4A"            /* v‚••ƒqg¶f.d\TPNJ */
	$"0E02 C1A6 FF14 EBB5 5E04 0000 1D4B 686B"            /* ..Á¦ÿ.ëµ^....Khk */
	$"6E2B 0051 7785 9693 7F6F 67B5 6609 6560"            /* n+.Qw…–“.ogµfÆe` */
	$"5851 4E51 3000 38FD A9FF 11D6 9457 1100"            /* XQNQ0.8ý©ÿ.Ö”W.. */
	$"0B33 5622 015D 7788 9691 7C6D 67B5 6608"            /* .3V".]wˆ–‘|mgµf. */
	$"635C 554F 4E4C 1000 9BAD FF0D E497 4200"            /* c\UONL..›­ÿ.ä—B. */
	$"0003 0E65 7A8B 968D 796B B566 0963 5E57"            /* ...ez‹–ykµfÆc^W */
	$"514D 5129 0038 F7B0 FF0A A30E 001B 6B7C"            /* QMQ).8÷°ÿÂ£...k| */
	$"8E96 8A75 6AB4 6609 635E 5752 4D51 3C02"            /* Ž–Šuj´fÆc^WRMQ<. */
	$"27DC B2FF 098F 0024 6F7E 9095 8772 69B2"            /* 'Ü²ÿÆ.$o~•‡ri² */
	$"660A 6563 5E57 524E 5046 0C00 A9B3 FF09"            /* fÂec^WRNPF..©³ÿÆ */
	$"DD03 2E73 8192 9484 7168 B066 0D65 6360"            /* Ý..s’”„qh°f.ec` */
	$"5C57 524F 523E 0908 1654 E8B2 FF09 BB00"            /* \WROR>Æ..Tè²ÿÆ». */
	$"3C75 8393 9381 6E67 AD66 1165 6462 605D"            /* <uƒ““ng­f.edb`] */
	$"5954 5151 5030 051B BCD2 1227 D8B1 FF09"            /* YTQQP0..¼Ò.'Ø±ÿÆ */
	$"A200 4676 8594 927E 6D67 9666 0368 6B6B"            /* ¢.Fv…”’~mg–f.hkk */
	$"688D 6615 6563 6360 5E5B 5855 5251 5244"            /* hf.ecc`^[XURQRD */
	$"2000 002C E3FF B200 2EF0 B0FF 097F 0053"            /*  ..,ãÿ²..ð°ÿÆ..S */
	$"7687 948F 7B6B 6795 6605 6B59 2F29 5C69"            /* v‡”{kg•f.kY/)\i */
	$"8966 1865 6462 605D 5B59 5553 5253 5248"            /* ‰f.edb`][YUSRSRH */
	$"2500 0043 9804 3CFC FFAF 008D B0FF 0862"            /* %..C˜.<üÿ¯.°ÿ.b */
	$"005E 768A 948C 7869 9566 076B 4715 4D21"            /* .^vŠ”Œxi•f.kG.M! */
	$"1967 6783 661E 6565 6462 615F 5D5A 5755"            /* .ggƒf.eedba_]ZWU */
	$"5251 4F50 503F 2103 003F 9DF7 FF6A 00D5"            /* RQOPP?!..?÷ÿj.Õ */
	$"FFFF 8312 E7AF FF08 430C 6777 8B93 8A75"            /* ÿÿƒ.ç¯ÿ.C.gw‹“Šu */
	$"6894 6608 6958 08AC FFC1 075E 6880 6616"            /* h”f.iX.¬ÿÁ.^h€f. */
	$"6564 6362 605D 5B59 5654 5250 4F4F 504F"            /* edcb`][YVTRPOOPO */
	$"4119 0000 22AD FD80 FF07 D404 8DFF FFFA"            /* A..."­ý€ÿ.Ô.ÿÿú */
	$"228F AEFF 09F5 271D 6D78 8C93 8874 6894"            /* "®ÿÆõ'.mxŒ“ˆth” */
	$"6620 6C24 4CFF FFEC 1550 6965 6463 615F"            /* f l$Lÿÿì.Piedca_ */
	$"5D5C 5956 5553 514F 4F51 4F48 3313 0000"            /* ]\YVUSQOOQOH3... */
	$"3F9A E582 FF08 FA1B 49FE FFFF 7F38 F9AD"            /* ?šå‚ÿ.ú.Iþÿÿ.8ù­ */
	$"FF09 EA0D 2D6F 7A8D 9285 7267 9366 1F6A"            /* ÿÆê.-oz’…rg“f.j */
	$"5705 BFFF FFF6 2642 655F 5D5B 5956 5452"            /* W.¿ÿÿö&Be_][YVTR */
	$"5150 4F50 514F 4835 1600 0024 6EC1 FE84"            /* QPOPQOH5...$nÁþ„ */
	$"FF08 FC1F 2BFB FFFF D01A E0AD FF09 D303"            /* ÿ.ü.+ûÿÿÐ.à­ÿÆÓ. */
	$"3F72 7B8F 9082 6F67 9366 1C6B 251E F7FF"            /* ?r{‚og“f.k%.÷ÿ */
	$"FFFC 3633 5E57 5453 514F 4E4F 5052 4E41"            /* ÿü63^WTSQONOPRNA */
	$"2B16 0000 2466 B1F1 86FF 03C7 5200 6480"            /* +...$f±ñ†ÿ.ÇR.d€ */
	$"FF02 F528 B6AD FF08 C000 4571 7D8E 8F7F"            /* ÿ.õ(¶­ÿ.À.Eq}Ž. */
	$"6E92 6604 6564 630D 4881 FF12 4927 5851"            /* n’f.edc.Hÿ.I'XQ */
	$"5050 5250 4D4A 3D2B 0F00 0008 3992 F386"            /* PPRPMJ=+....9’ó† */
	$"FF06 F3C7 6B00 0043 E080 FF02 FB2A 89AC"            /* ÿ.óÇk..Cà€ÿ.û*‰¬ */
	$"FF09 FD99 004F 717E 8E8E 7E6C 9166 0564"            /* ÿÆý™.Oq~ŽŽ~l‘f.d */
	$"615F 5800 6B81 FF0F 6310 524C 463D 301E"            /* a_X.kÿ.c.RLF=0. */
	$"0C00 000D 2F6D B3F0 81FF 0DFC FBFA F5EF"            /* ..../m³ðÿ.üûúõï */
	$"D7A3 5619 010D 55B1 FB81 FF02 F81B 5AAB"            /* ×£V...U±ûÿ.ø.Z« */
	$"FF0A A83B 0D06 5871 808E 8C7B 6B90 6606"            /* ÿÂ¨;..Xq€ŽŒ{kf. */
	$"6560 5A57 4800 8781 FF03 8E00 1E08 8000"            /* e`ZWH.‡ÿ.Ž...€. */
	$"1719 497C B1D3 E0D4 B597 8167 4E41 2C29"            /* ..I|±ÓàÔµ—gNA,) */
	$"2317 120D 194D 69A5 E684 FF02 F512 48A9"            /* #....Mi¥æ„ÿ.õ.H© */
	$"FF0C F867 0000 0907 5C71 818E 8A79 6A90"            /* ÿ.øg..Æ.\qŽŠyj */
	$"6606 635C 5553 4000 9381 FF1A C700 092F"            /* f.c\US@.“ÿ.Ç.Æ/ */
	$"527C BAED ECB9 6824 0F01 0000 0615 2A3B"            /* R|ºíì¹h$......*; */
	$"4B4F 5667 89BD EC88 FF02 DF00 63A9 FF0C"            /* KOVg‰½ìˆÿ.ß.c©ÿ. */
	$"F203 0005 0E0B 6271 828D 8776 6990 6606"            /* ò.....bq‚‡vif. */
	$"6259 5251 3C00 9981 FF12 F93D 0184 B9B2"            /* bYRQ<.™ÿ.ù=.„¹² */
	$"894E 1E29 496C 8AA4 C3D9 E9F6 FB88 FF01"            /* ‰N.)IlŠ¤ÃÙéöûˆÿ. */
	$"F5E5 82FF 05D2 570C A592 D4A7 FF0C E80C"            /* õå‚ÿ.ÒW.¥’Ô§ÿ.è. */
	$"0003 0612 6772 828C 8474 6890 6606 6058"            /* ....gr‚Œ„thf.`X */
	$"5150 3A00 9B82 FF08 E355 1015 213C 72B1"            /* QP:.›‚ÿ.ãU..!<r± */
	$"F38F FF10 FEFF F88A D3FF FFF6 A30F 1062"            /* óÿ.þÿøŠÓÿÿö£..b */
	$"4000 1A92 FDA5 FF0C 5F00 0006 1015 6873"            /* @..’ý¥ÿ._.....hs */
	$"838B 8373 678F 6607 655F 5650 5038 0092"            /* ƒ‹ƒsgf.e_VPP8.’ */
	$"84FF 02E7 E5F2 90FF 0EE1 9992 F8FF FFD9"            /* „ÿ.çåòÿ.á™’øÿÿÙ */
	$"49F1 EF2F 003D 490B 8100 0156 F9A2 FF01"            /* Iñï/.=I...Vù¢ÿ. */
	$"FB66 8000 0904 0A1D 6A73 838B 8071 678F"            /* ûf€.Æ.Â.jsƒ‹€qg */
	$"6607 655F 5650 4F41 0080 92FF 01E9 FE83"            /* f.e_VPOA.€’ÿ.éþƒ */
	$"FF02 F14D 5281 FF06 9D54 322C 7C57 0783"            /* ÿ.ñMRÿ.T2,|W.ƒ */
	$"0001 42CE A0FF 01FE 6481 0009 0301 256C"            /* ..BÎ ÿ.þd.Æ..%l */
	$"7383 897E 6F67 8F66 0764 5E55 504D 4D06"            /* sƒ‰~ogf.d^UPMM. */
	$"5C91 FF03 E13F 6BF4 83FF 02D6 77FA 81FF"            /* \‘ÿ.á?kôƒÿ.Öwúÿ */
	$"0497 0033 4201 8500 0112 929F FF00 6D82"            /* .—.3B.…...’Ÿÿ.m‚ */
	$"0008 1B06 2B6E 7583 867C 6E90 6608 645D"            /* ....+nuƒ†|nf.d] */
	$"544F 4C50 2C24 F48E FF06 D364 1600 0035"            /* TOLP,$ôŽÿ.Ód...5 */
	$"D883 FF01 DFE9 82FF 04B2 2900 0C0C 8600"            /* Øƒÿ.ßé‚ÿ.²)...†. */
	$"015B ED9C FF00 9283 0008 3508 2E6F 7583"            /* .[íœÿ.’ƒ..5..ouƒ */
	$"8479 6C90 6608 635D 544E 4C4D 4E0B B88D"            /* „ylf.c]TNLMN.¸ */
	$"FF01 B817 8200 0154 FE83 FF02 E9D4 FA81"            /* ÿ.¸.‚..Tþƒÿ.éÔú */
	$"FF08 EC69 030E 1B2B 3417 0482 0001 46DA"            /* ÿ.ìi...+4..‚..FÚ */
	$"9AFF 007F 8300 0918 3A00 3470 7582 8377"            /* šÿ..ƒ.Æ.:.4pu‚ƒw */
	$"6B90 6609 635C 534E 4C4C 5028 41FB 8BFF"            /* kfÆc\SNLLP(Aû‹ÿ */
	$"01F2 1D83 0001 11E8 84FF 01F0 F782 FF09"            /* .ò.ƒ...è„ÿ.ð÷‚ÿÆ */
	$"E806 0235 505E 5A50 2609 8100 0116 9F98"            /* è..5P^ZP&Æ...Ÿ˜ */
	$"FF00 8983 000A 03AF A800 3A71 7581 8176"            /* ÿ.‰ƒ.Â.¯¨.:quv */
	$"6A90 6609 635B 534E 4C4C 4D4E 147A 8BFF"            /* jfÆc[SNLLMN.z‹ÿ */
	$"01D6 0683 0001 06D2 8BFF 0CDF 4425 0700"            /* .Ö.ƒ...Ò‹ÿ.ßD%.. */
	$"000F 1F2E 3529 1804 8000 0169 EF95 FF00"            /* ....5)..€..iï•ÿ. */
	$"A484 000A 21A0 7500 3F71 7480 7F74 6990"            /* ¤„.Â! u.?qt€.ti */
	$"6603 635B 524E 804C 0450 3D03 53F6 89FF"            /* f.c[RN€L.P=.Sö‰ÿ */
	$"00B8 8400 0107 D48C FF11 FEF6 DCB5 8137"            /* .¸„...ÔŒÿ.þöÜµ7 */
	$"0400 0001 1310 1004 0000 2DC2 93FF 01AB"            /* ..........-Â“ÿ.« */
	$"1484 000A 4072 4C00 4171 747F 7C72 6990"            /* .„.Â@rL.Aqt.|ri */
	$"6603 625A 524E 804C 064F 3908 0040 B9FB"            /* f.bZRN€L.O9..@¹û */
	$"83FF 00FA 80FF 009A 8400 0110 E591 FF0D"            /* ƒÿ.ú€ÿ.š„...å‘ÿ. */
	$"FBDA AB78 4824 0700 030F 0000 17AC 85FF"            /* ûÚ«xH$.......¬…ÿ */
	$"0364 ADE4 E485 FF01 A302 8400 0B0B 4145"            /* .d­ää…ÿ.£.„...AE */
	$"3000 4570 747E 7B71 698F 6604 6562 5A52"            /* 0.Ept~{qif.ebZR */
	$"4D80 4C0D 5038 0004 2317 346F A3CB DDE0"            /* M€L.P8..#.4o£ËÝà */
	$"9FE0 80FF 0091 8400 012B F896 FF0A F6D2"            /* Ÿà€ÿ.‘„..+ø–ÿÂöÒ */
	$"9A32 0310 0000 0175 EE83 FF06 0000 120E"            /* š2.....uîƒÿ..... */
	$"4390 ED81 FF01 BD07 8500 0B0F 201E 1700"            /* Cíÿ.½.…... ... */
	$"4970 737C 786F 688F 6604 6562 5A51 4D80"            /* Ips|xohf.ebZQM€ */
	$"4C0D 5037 0000 3044 3106 0003 0A0C 68FD"            /* L.P7..0D1...Â.hý */
	$"80FF 01DD 0A83 0000 5E9A FF03 EC2E 1226"            /* €ÿ.ÝÂƒ..^šÿ.ì..& */
	$"8000 0126 CE82 FF83 0005 194E 8ED2 D324"            /* €..&Î‚ÿƒ...NŽÒÓ$ */
	$"8500 0C22 6261 6639 004D 6E73 7B76 6E67"            /* ….."baf9.Mns{vng */
	$"8F66 0465 625A 514D 804C 0C50 3600 030A"            /* f.ebZQM€L.P6..Â */
	$"031B 201D 1600 24EE 82FF 005A 8200 0123"            /* .. ...$î‚ÿ.Z‚..# */
	$"C99B FF03 C511 451D 8000 051D B6FF F5C8"            /* É›ÿ.Å.E.€...¶ÿõÈ */
	$"9C86 0001 070A 8500 0D02 94DC D2E0 7000"            /* œ†...Â…...”ÜÒàp. */
	$"526E 7279 756D 678F 6604 6562 5A51 4D80"            /* Rnryumgf.ebZQM€ */
	$"4C04 5038 0005 1280 0004 0111 0F0E DE82"            /* L.P8...€......Þ‚ */
	$"FF06 DA20 0002 45A3 EC8D FF13 FAD1 FEFF"            /* ÿ.Ú ..E£ìÿ.úÑþÿ */
	$"FFFD EACC A5B5 F6FF FFFD 9435 031D 6317"            /* ÿýêÌ¥µöÿÿý”5..c. */
	$"8000 0306 4D20 0491 000C 2DB0 ADAC B85B"            /* €...M .‘..-°­¬¸[ */
	$"0053 6D71 7673 6C90 6604 6562 5A51 4D80"            /* .Smqvslf.ebZQM€ */
	$"4C04 5038 0005 1482 0003 0C06 48D3 82FF"            /* L.P8...‚....HÓ‚ÿ */
	$"02D7 89BC 90FF 14F9 3B42 6159 3B18 0500"            /* .×‰¼ÿ.ù;BaY;... */
	$"0032 B3FF DE02 0114 042E 753E 9700 0C5E"            /* .2³ÿÞ.....u>—..^ */
	$"9E94 959F 4F00 536C 6F75 716B 9066 0465"            /* ž”•ŸO.Slouqkf.e */
	$"625A 514D 804C 044F 3A00 0410 8200 0401"            /* bZQM€L.O:...‚... */
	$"1100 14C8 97FF 01FB 2F86 000B 0DC3 8100"            /* ...È—ÿ.û/†...Ã. */
	$"096D 4400 2B9A 3B01 9400 0D15 8185 8383"            /* ÆmD.+š;.”...…ƒƒ */
	$"8C45 0054 6C6E 726F 6A90 6604 6562 5A52"            /* ŒE.Tlnrojf.ebZR */
	$"4D80 4C04 4F3C 0101 0784 0002 1100 8C98"            /* M€L.O<...„....Œ˜ */
	$"FF00 6787 0000 0380 0006 0A9E A90C 3C52"            /* ÿ.g‡...€..Âž©.<R */
	$"0E94 000D 4A7E 7677 777F 3E00 556B 6D6F"            /* .”..J~vww.>.Ukmo */
	$"6D68 9066 0465 625A 524D 804C 044F 3F02"            /* mhf.ebZRM€L.O?. */
	$"0104 8300 030A 2C18 DA98 FF03 ECA2 6622"            /* ..ƒ..Â,.Ú˜ÿ.ì¢f" */
	$"8500 0916 1602 22C0 FF5F 0006 0693 0002"            /* ….Æ..."Àÿ_...“.. */
	$"0F68 6C80 6A08 7137 0056 6B6B 6C6A 6890"            /* .hl€j.q7.Vkkljh */
	$"6604 6562 5A52 4D80 4C04 4E42 0202 1183"            /* f.ebZRM€L.NB...ƒ */
	$"0002 4116 7C9C FF02 ED8F 1582 0007 47E9"            /* ..A.|œÿ.í.‚..Gé */
	$"EBC3 E5FF FF8A 9500 0301 3B69 6580 6408"            /* ëÃåÿÿŠ•...;ie€d. */
	$"6B34 0056 6A69 6A69 6791 6603 625A 524E"            /* k4.Vjijig‘f.bZRN */
	$"804C 054E 4303 011C 0982 0002 3002 A79E"            /* €L.NC...Æ‚..0.§ž */
	$"FF02 DA86 2D80 0000 7483 FF00 9F95 0002"            /* ÿ.Ú†-€..tƒÿ.Ÿ•.. */
	$"1F69 6781 6508 6C34 0057 6968 6867 6791"            /* .ige.l4.Wihhgg‘ */
	$"6603 635B 524E 804C 0D4E 4404 001A 1904"            /* f.c[RN€L.ND..... */
	$"0000 0219 1C00 80A0 FF04 F4B0 6754 CE83"            /* ......€ ÿ.ô°gTÎƒ */
	$"FF00 B795 000B 1861 6A65 6465 656B 3400"            /* ÿ.·•...ajedeek4. */
	$"5769 9566 0363 5B52 4E80 4C0E 4E46 0400"            /* Wi•f.c[RN€L.NF.. */
	$"86BC B94B 0035 7D74 0F20 F3AA FF02 8300"            /* †¼¹K.5}t. óªÿ.ƒ. */
	$"1580 1601 170C 8F00 0A0D 4D74 6F6D 6D74"            /* .€.....Â.Mtommt */
	$"3900 5669 9566 0363 5B52 4E80 4C0E 4D49"            /* 9.Vi•f.c[RN€L.MI */
	$"0700 A2E7 E5B6 2581 989A 4400 ACA9 FF09"            /* ..¢çå¶%˜šD.¬©ÿÆ */
	$"E312 3B9F 999A 9B9D 8E10 8F00 0902 3B77"            /* ã.;Ÿ™š›Ž..Æ.;w */
	$"7D77 7F3E 0056 6995 6603 635B 524E 804C"            /* }w.>.Vi•f.c[RN€L */
	$"0F4D 4C0C 0075 A6A1 835C A2B2 B595 0434"            /* .ML..u¦¡ƒ\¢²µ•.4 */
	$"F9A8 FF09 5C03 91AE AEAD A8A8 AE43 9100"            /* ù¨ÿÆ\.‘®®­¨¨®C‘. */
	$"071E 668E 8F46 0054 6995 6603 635B 524E"            /* ..fŽF.Ti•f.c[RN */
	$"804C 0F4D 4D0F 004D 6DA2 4546 B477 B7E4"            /* €L.MM..Mm¢EF´w·ä */
	$"6B00 8AA7 FF0A 8900 5AC2 BCA2 5534 3437"            /* k.Š§ÿÂ‰.ZÂ¼¢U447 */
	$"2892 0006 0675 B355 0056 6B80 6803 696A"            /* (’...u³U.Vk€h.ij */
	$"6B6B 8E6C 0369 6158 5480 5110 5252 1400"            /* kkŽl.iaXT€Q.RR.. */
	$"2E66 C24A 47B5 3964 D4D0 350E E3A6 FF05"            /* .fÂJGµ9dÔÐ5.ã¦ÿ. */
	$"5400 607B 7015 9800 1205 7A67 004A 5D5B"            /* T.`{p.˜...zg.J][ */
	$"5B5A 5952 4B46 433E 3937 3232 852F 0030"            /* [ZYRKFC>9722…/.0 */
	$"8032 1733 2F30 3135 3739 3E14 0027 70BB"            /* €2.3/01579>..'p» */
	$"484A BB43 6E8A 79A8 153D DE8A FF00 F396"            /* HJ»CnŠy¨.=ÞŠÿ.ó– */
	$"FF05 F583 0015 1204 9A00 0305 1D00 0480"            /* ÿ.õƒ....š......€ */
	$"0502 0404 0299 0010 0100 0816 240F 1E48"            /* .....™......$..H */
	$"1F40 4D2F 834D 0921 C188 FF05 DD2E 4576"            /* .@M/ƒMÆ!Áˆÿ.Ý.Ev */
	$"B4FA 80FF 02F9 CCE9 89FF 07CF A275 2908"            /* ´ú€ÿ.ùÌé‰ÿ.Ï¢u). */
	$"0927 05CC 0006 0201 0915 030A 9486 FF01"            /* Æ'.Ì....Æ..Â”†ÿ. */
	$"DC2C 8100 0630 6CED DA3B 0443 85FF 04FE"            /* Ü,..0líÚ;.C…ÿ.þ */
	$"9E4B 4B0E 8000 020A 0702 9400 0308 1A10"            /* žKK.€..Â..”..... */
	$"0080 01B3 0021 0805 0048 92D3 F1F1 E5CB"            /* .€.³.!...H’ÓññåË */
	$"A977 1B00 0B0B 0906 0000 2215 0004 0870"            /* ©w....Æ..."....p */
	$"8B97 9DA0 9894 833F 8000 0408 0F0A 0701"            /* ‹— ˜”ƒ?€....Â.. */
	$"9600 061C 5E3A 0004 0302 B400 1705 1507"            /* –...^:....´..... */
	$"0004 1517 0B01 0000 150E 0300 0103 0206"            /* ................ */
	$"0806 0412 1780 0002 0101 0380 0005 192C"            /* .....€.....€..., */
	$"2F41 2208 9800 061C 5E3B 0004 0302 B500"            /* /A".˜...^;....µ. */
	$"0B24 5344 271A 1626 1C15 395B 1083 0013"            /* .$SD'..&..9[.ƒ.. */
	$"130D 0021 5236 4B52 3B3D 3522 0000 2F43"            /* ...!R6KR;=5"../C */
	$"3643 2008 8900 FFFF FBFF FFFF FFFF 03FF"            /* 6C .‰.ÿÿûÿÿÿÿÿ.ÿ */
	$"FFFE ED9C E001 DFED 88FF 02FB E4DF 82E0"            /* ÿþíœà.ßíˆÿ.ûäß‚à */
	$"03E1 E2E2 E18A E001 DFF2 87FF 02F7 E3DF"            /* .áââáŠà.ßò‡ÿ.÷ãß */
	$"96E0 02DE E8FE 8DFF 02F9 EAE9 8BEA 01E4"            /* –à.Þèþÿ.ùêé‹ê.ä */
	$"DF89 E001 DEF3 89FF 0EFB EAE9 EAF0 F6F7"            /* ß‰à.Þó‰ÿ.ûêéêðö÷ */
	$"F0ED ECEF F7F7 ECE2 86E0 01E1 FB88 FF01"            /* ðíìï÷÷ìâ†à.áûˆÿ. */
	$"F5E8 87EA 01E5 DF89 E002 DFE5 FB8F FF03"            /* õè‡ê.åß‰à.ßåûÿ. */
	$"F9ED EDEE 80ED 86EE 01EB E189 E001 DFE9"            /* ùííî€í†î.ëá‰à.ßé */
	$"8AFF 0FF3 F6F3 C596 7458 4C4B 5571 95B7"            /* Šÿ.óöóÅ–tXLKUq•· */
	$"DFEC E483 E002 DFE2 FD88 FF01 F5ED 87EE"            /* ßìäƒà.ßâýˆÿ.õí‡î */
	$"01E4 DF89 E001 DFF7 91FF 02F4 ECED 84EE"            /* .äß‰à.ß÷‘ÿ.ôìí„î */
	$"82ED 02EE E5DF 89E0 01E1 FC89 FF03 F2BE"            /* ‚í.îåß‰à.áü‰ÿ.ò¾ */
	$"5B06 8500 0503 3E9E D4EA E581 E002 DFE4"            /* [.…...>žÔêåà.ßä */
	$"FE88 FF01 F1EC 86ED 01EC E189 E002 DFE5"            /* þˆÿ.ñì†í.ìá‰à.ßå */
	$"FE91 FF02 FEF1 ED88 EE03 EDEE EBE1 89E0"            /* þ‘ÿ.þñíˆî.íîëá‰à */
	$"01DF F388 FF01 AD3A 8000 0717 2F41 4746"            /* .ßóˆÿ.­:€.../AGF */
	$"3D29 1380 0008 1C70 D1EC E2E0 E0DF E988"            /* =).€...pÑìâààßéˆ */
	$"FF01 FCEE 86ED 01EE E98A E001 DEF4 93FF"            /* ÿ.üî†í.îéŠà.Þô“ÿ */
	$"01FB EF89 EE02 EDEE E689 E002 DFE2 FD85"            /* .ûï‰î.íîæ‰à.ßâý… */
	$"FF1A D661 0000 134D 7C96 9CA0 A09F 9E98"            /* ÿ.Öa...M|–œ  Ÿž˜ */
	$"8E75 4A17 0000 228F E1E4 DFDF E988 FF01"            /* ŽuJ..."áäßßéˆÿ. */
	$"F6EC 86ED 02EE E5DF 88E0 02DF E8FE 94FF"            /* öì†í.îåßˆà.ßèþ”ÿ */
	$"01F6 ED8A EE02 EDE3 DF88 E001 DFF4 84FF"            /* .öíŠî.íãßˆà.ßô„ÿ */
	$"09BF 1800 035D 94A2 A09B 9A80 9900 9A80"            /* Æ¿...]”¢ ›š€™.š€ */
	$"990A 9683 5D15 0000 59E2 E4E0 EA88 FF01"            /* ™Â–ƒ]...Yâäàêˆÿ. */
	$"F2EC 87ED 00E2 89E0 01DF F695 FF07 FEF1"            /* òì‡í.â‰à.ßö•ÿ.þñ */
	$"EEEF EFEE EFEF 85EE 01E9 DF88 E001 DFE7"            /* îïïîïï…î.éßˆà.ßç */
	$"83FF 09C7 1800 1973 A39F 9C9C 9B84 990C"            /* ƒÿÆÇ...s£Ÿœœ›„™. */
	$"9793 8D8C 7329 0000 7CFD F2F3 FE87 FF00"            /* —“Œs)..|ýòóþ‡ÿ. */
	$"F086 ED01 EEEA 89E0 02DF E5FD 96FF 01FC"            /* ð†í.îê‰à.ßåý–ÿ.ü */
	$"F182 EF86 EE01 E3DF 88E0 01E1 F780 F80B"            /* ñ‚ï†î.ãßˆà.á÷€ø. */
	$"FCC5 1600 408F A39D A0A0 9E9B 8599 0B98"            /* üÅ..@£  ž›…™.˜ */
	$"938B 867C 3500 0DB1 E2D7 F086 FF00 FC87"            /* “‹†|5..±â×ð†ÿ.ü‡ */
	$"ED02 EEE5 DF88 E001 DFF4 98FF 00FB 84EF"            /* í.îåßˆà.ßô˜ÿ.û„ï */
	$"83EE 01EF EA88 E011 E1DF B6A9 ABB2 7D0C"            /* ƒî.ïêˆà.áß¶©«²}. */
	$"0050 A29F 9EA3 A6A4 9E9A 8699 0A98 9389"            /* .P¢Ÿž£¦¤žš†™Â˜“‰ */
	$"807C 2E00 44DE D6E7 86FF 01F8 EC85 ED02"            /* €|..DÞÖç†ÿ.øì…í. */
	$"EEEB E188 E001 DEEC 9AFF 01F4 EE83 EF84"            /* îëáˆà.Þìšÿ.ôîƒï„ */
	$"EE01 E4DF 86E0 10E1 DDB1 ADB4 A31C 0032"            /* î.äß†à.áÝ±­´£..2 */
	$"9F9C 9DA4 A9A9 A39D 8899 0997 8E83 7D64"            /* Ÿœ¤©©£ˆ™Æ—Žƒ}d */
	$"0403 A5DD E186 FF01 F5EC 85ED 02EE E8DF"            /* ..¥Ýá†ÿ.õì…í.îèß */
	$"89E0 00F9 9AFF 01FD F184 EF82 EE01 EFEB"            /* ‰à.ùšÿ.ýñ„ï‚î.ïë */
	$"87E0 10E1 D9C3 C4CC 4700 1E90 9E9D A3AB"            /* ‡à.áÙÃÄÌG..ž£« */
	$"ACA8 A09A 8999 0993 897D 7B2B 003B C1C7"            /* ¬¨ š‰™Æ“‰}{+.;ÁÇ */
	$"FC85 FF01 F2EC 85ED 02EE E5DF 87E0 01DE"            /* ü…ÿ.òì…í.îåß‡à.Þ */
	$"EC9C FF00 FA85 EF82 EE01 EFE8 88E0 0DDE"            /* ìœÿ.ú…ï‚î.ïèˆà.Þ */
	$"E77F 001D 8C9F 9DA3 ABB0 ACA3 9C8A 9909"            /* ç...ŒŸ£«°¬£œŠ™Æ */
	$"968E 817E 5800 07AA CEFC 84FF 01FE EF86"            /* –Ž~X..ªÎü„ÿ.þï† */
	$"ED01 ECE1 89E0 00F8 9DFF 00F7 85EF 82EE"            /* í.ìá‰à.øÿ.÷…ï‚î */
	$"01ED E286 E00E E2EB DC22 006E A49B A0AB"            /* .íâ†à.âëÜ".n¤› « */
	$"B1B1 A89F 9A8A 9909 9890 847C 6F0C 0076"            /* ±±¨ŸšŠ™Æ˜„|o..v */
	$"B9FA 84FF 00FC 86ED 01EE E988 E001 DFEB"            /* ¹ú„ÿ.ü†í.îéˆà.ßë */
	$"8BFF 01F7 F68F FF00 F485 EF81 EE02 EFE8"            /* ‹ÿ.÷öÿ.ô…ïî.ïè */
	$"DF84 E00E E4C5 A945 0036 9E9B 9EA8 B2B4"            /* ß„à.äÅ©E.6ž›ž¨²´ */
	$"AEA2 9C8C 9908 9387 7D79 2600 4FAF FA84"            /* ®¢œŒ™.“‡}y&.O¯ú„ */
	$"FF01 F9EC 85ED 02EE E6DF 88E0 00F7 8AFF"            /* ÿ.ùì…í.îæßˆà.÷Šÿ */
	$"02F7 E0E0 8FFF 01FD F185 EF82 EE00 E384"            /* .÷ààÿ.ýñ…ï‚î.ã„ */
	$"E00D E2E0 8500 0D87 9F9D A5B1 B7B3 A79D"            /* à.âà…..‡Ÿ¥±·³§ */
	$"8D99 0894 8A7E 7E3F 0035 AEF7 84FF 02F5"            /* ™.”Š~~?.5®÷„ÿ.õ */
	$"ECEE 83ED 02EE EDE2 87E0 01DF EE8B FF05"            /* ìîƒí.îíâ‡à.ßî‹ÿ. */
	$"E9DF E0F0 EEF7 8DFF 00F8 85EF 81EE 01EF"            /* éßàðî÷ÿ.ø…ïî.ï */
	$"EA84 E00D E1D6 3900 4DA3 9CA1 ACB7 B7AE"            /* ê„à.áÖ9.M£œ¡¬··® */
	$"A19B 8D99 0895 8B7F 7E4E 001B 6FEF 84FF"            /* ¡›™.•‹.~N..oï„ÿ */
	$"01F2 ED81 EE00 ED80 EE00 EA87 E002 DFE1"            /* .òíî.í€î.ê‡à.ßá */
	$"FC8A FF00 F780 E003 DEDD E2F5 8CFF 01FE"            /* üŠÿ.÷€à.ÞÝâõŒÿ.þ */
	$"F386 EF81 EE01 E2DF 82E0 0CE8 9D02 128A"            /* ó†ïî.âß‚à.è..Š */
	$"9E9F A7B3 B9B4 A89E 8E99 0895 8C80 7B5B"            /* žŸ§³¹´¨žŽ™.•Œ€{[ */
	$"0010 B0FA 83FF 01FE EF86 EE01 E7DF 86E0"            /* ..°úƒÿ.þï†î.çß†à */
	$"01DF F08B FF01 EDDE 81E0 02DF E0F3 8CFF"            /* .ßð‹ÿ.íÞà.ßàóŒÿ */
	$"02FD F2EE 86EF 03EE EEEB E181 E00D E1EB"            /* .ýòî†ï.îîëáà.áë */
	$"5000 69A3 9CA5 B0BA B8AC A29B 8E99 0895"            /* P.i£œ¥°º¸¬¢›Ž™.• */
	$"8C80 7A64 020C A9F4 83FF 00FB 87EE 00E3"            /* Œ€zd..©ôƒÿ.û‡î.ã */
	$"86E0 02DF E2FB 8AFF 02FB E7DF 84E0 00F3"            /* †à.ßâûŠÿ.ûçß„à.ó */
	$"8CFF 01FB F087 EF03 EEEF E8DF 80E0 0DE8"            /* Œÿ.ûð‡ï.îïèß€à.è */
	$"B00A 1D99 9CA0 ADB8 BCB2 A59D 9A8E 9908"            /* °Â.™œ ­¸¼²¥šŽ™. */
	$"958B 8079 6806 056E F383 FF01 F7ED 84EE"            /* •‹€yh..nóƒÿ.÷í„î */
	$"01EF E987 E001 DFEE 8AFF 03FD F0E6 DF83"            /* .ïé‡à.ßîŠÿ.ýðæßƒ */
	$"E002 DFE1 FB8C FF07 F6F1 F3F4 F6F5 F3F1"            /* à.ßáûŒÿ.öñóôöõóñ */
	$"82EF 11ED E1E0 E0E1 E94D 0055 A39C A6B4"            /* ‚ï.íáààáéM.U£œ¦´ */
	$"BCB9 ACA0 9B8F 9908 948A 7F78 6909 0362"            /* ¼¹¬ ›™.”Š.xiÆ.b */
	$"F883 FF01 F3ED 85EE 01E4 DF86 E001 E1FB"            /* øƒÿ.óí…î.äß†à.áû */
	$"8AFF 03F4 EDE7 DF84 E002 DFE4 F48C FF08"            /* Šÿ.ôíçß„à.ßäôŒÿ. */
	$"F0E3 D6CD D5E2 F0FC F680 EF11 F0E8 E0E0"            /* ðãÖÍÕâðüö€ï.ðèàà */
	$"E6C4 0C08 879E A0AD BABD B5A7 9C9A 8F99"            /* æÄ..‡ž ­º½µ§œš™ */
	$"0894 897E 7869 0903 63F7 82FF 01FD EF85"            /* .”‰~xiÆ.c÷‚ÿ.ýï… */
	$"EE01 EDE2 86E0 01DE ED8A FF04 F8ED EEE7"            /* î.íâ†à.ÞíŠÿ.øíîç */
	$"DF85 E002 DFE0 F388 FF1F F5B8 6E32 1104"            /* ß…à.ßàóˆÿ.õ¸n2.. */
	$"0004 1443 86CF FAF0 EFEF EEE3 E0EC 7700"            /* ...C†Ïúðïïîãàìw. */
	$"3AA0 9EA6 B5BE BBAE A19A 9099 0893 877C"            /* : ž¦µ¾»®¡š™.“‡| */
	$"7867 0607 86FA 82FF 00FA 85EE 02EF E9DF"            /* xg..†ú‚ÿ.ú…î.ïéß */
	$"85E0 02DF E7FE 89FF 05FA EEED EEE7 DF86"            /* …à.ßçþ‰ÿ.úîíîçß† */
	$"E002 DFE0 F986 FF1F BA35 010E 3045 4C42"            /* à.ßàù†ÿ.º5..0ELB */
	$"2B0E 0000 149B F9F2 EFF0 E8E1 E131 006A"            /* +....›ùòïðèáá1.j */
	$"A3A0 ACBA BEB6 A89D 9099 0998 9185 7B77"            /* £ ¬º¾¶¨™Æ˜‘…{w */
	$"6502 0FC1 FE82 FF01 F7ED 84EE 02EF E6DF"            /* e..Áþ‚ÿ.÷í„î.ïæß */
	$"85E0 01DE F389 FF06 FEF0 EDEE EEE7 DF87"            /* …à.Þó‰ÿ.þðíîîçß‡ */
	$"E002 DFE6 FA83 FF21 D954 1142 7A98 9FA1"            /* à.ßæúƒÿ!ÙT.Bz˜Ÿ¡ */
	$"A1A0 9D90 631D 0000 78F8 F2EF EFEC B603"            /* ¡ c...xøòïïì¶. */
	$"1A93 A0A6 B4BF BBB0 A39B 9099 0898 8F82"            /* .“ ¦´¿»°£›™.˜‚ */
	$"7978 5E00 0CAA 83FF 03F5 EEEF EF83 EE00"            /* yx^..ªƒÿ.õîïïƒî. */
	$"E285 E002 DFE5 FC89 FF01 F5ED 80EE 01E6"            /* â…à.ßåü‰ÿ.õí€î.æ */
	$"DF88 E00E DFE2 DCDB DBE1 D977 102D 8BA9"            /* ßˆà.ßâÜÛÛáÙw.-‹© */
	$"A59D 9A80 9917 9A9B 9A86 4C04 007F F6F0"            /* ¥š€™.š›š†L...öð */
	$"F1F8 6600 65A4 A2AE BBC0 B5A8 9E9A 9099"            /* ñøf.e¤¢®»Àµ¨žš™ */
	$"0896 8B7E 767A 5200 1B93 83FF 00F2 81EF"            /* .–‹~vzR..“ƒÿ.òï */
	$"80EE 01EF EB87 E000 F589 FF00 F981 EE02"            /* €î.ïë‡à.õ‰ÿ.ùî. */
	$"EFE6 DF89 E00C DECC CBCB D85D 004D A7AB"            /* ïæß‰à.ÞÌËËØ].M§« */
	$"A49F 9B83 9914 958F 8B50 0000 ABFB F2E8"            /* ¤Ÿ›ƒ™.•‹P..«ûòè */
	$"1E09 909F A6B5 BFBD AFA3 9B91 9908 9388"            /* .ÆŸ¦µ¿½¯£›‘™.“ˆ */
	$"7C75 7B49 003E E782 FF01 FCF0 81EF 80EE"            /* |u{I.>ç‚ÿ.üðï€î */
	$"02EF E7DF 84E0 01DF E789 FF01 FBEF 81EE"            /* .ïçß„à.ßç‰ÿ.ûïî */
	$"02EF E7DF 8AE0 0BD6 D7C4 8300 54AB AAA9"            /* .ïçßŠà.Ö×Äƒ.T«ª© */
	$"A49F 9A83 9914 9890 877F 2700 58F7 FCB5"            /* ¤Ÿšƒ™.˜‡.'.X÷üµ */
	$"0032 9FA0 ACBA C0B8 AA9F 9A90 9909 9891"            /* .2Ÿ ¬ºÀ¸ªŸš™Æ˜‘ */
	$"867A 757B 3C00 30C3 82FF 01FA EE83 EF03"            /* †zu{<.0Ã‚ÿ.úîƒï. */
	$"EEEE E3DF 84E0 01DE F188 FF01 FEF1 82EE"            /* îîãß„à.Þñˆÿ.þñ‚î */
	$"02EF E7DF 8AE0 0AD0 D17A 003D AAAC AFAA"            /* .ïçßŠàÂÐÑz.=ª¬¯ª */
	$"A29D 8599 1293 8881 6302 18E1 F64A 0066"            /* ¢…™.“ˆc..áöJ.f */
	$"A4A4 B3BF BFB2 A49C 9199 0996 8F82 7774"            /* ¤¤³¿¿²¤œ‘™Æ–‚wt */
	$"782C 0063 EE82 FF0A F7F0 F4F7 F8F7 F7F3"            /* x,.cî‚ÿÂ÷ðô÷ø÷÷ó */
	$"F1EC E184 E002 DFE5 FC88 FF01 F4ED 82EE"            /* ñìá„à.ßåüˆÿ.ôí‚î */
	$"02EF E7DF 89E0 0BE1 DB9C 0618 9AAF B1B1"            /* .ïçß‰à.áÛœ..š¯±± */
	$"A79E 9A85 9912 9388 7D7A 1E00 C8B7 000D"            /* §žš…™.“ˆ}z..È·.. */
	$"8FA2 AABA C0BB AC9F 9A91 9909 948B 7E75"            /* ¢ªºÀ»¬Ÿš‘™Æ”‹~u */
	$"7475 1800 78EC 83FF 0AEF D6C2 BCC0 C4E0"            /* tu..xìƒÿÂïÖÂ¼ÀÄà */
	$"F9F3 E4E1 83E0 01E1 F988 FF01 F7ED 83EE"            /* ùóäáƒà.áùˆÿ.÷íƒî */
	$"02EF E7DF 89E0 0AE3 DB47 0061 AFB1 B5AF"            /* .ïçß‰àÂãÛG.a¯±µ¯ */
	$"A39B 8599 1298 9288 7C7E 3700 AF77 0031"            /* £›…™.˜’ˆ|~7.¯w.1 */
	$"9FA3 AFBD BFB6 A79C 9199 0A98 9287 7B74"            /* Ÿ£¯½¿¶§œ‘™Â˜’‡{t */
	$"756D 0A01 74DE 81FF 0EDA 8E40 1604 0102"            /* umÂ.tÞÿ.ÚŽ@.... */
	$"0623 6399 CBEA E8E1 80E0 01DF EA88 FF01"            /* .#c™Ëêèá€à.ßêˆÿ. */
	$"FDEF 84EE 02EF E7DF 89E0 09E9 B303 2CA0"            /* ýï„î.ïçß‰àÆé³.,  */
	$"AEB5 B4AA 9F86 9912 978F 847A 7D4B 0097"            /* ®µ´ªŸ†™.—„z}K.— */
	$"3000 5FA6 A7B4 C0BD B0A2 9A91 990F 9690"            /* 0._¦§´À½°¢š‘™.– */
	$"8479 7377 5D00 1397 F2FF FFBF 4708 8000"            /* „ysw]..—òÿÿ¿G.€. */
	$"0305 0908 0480 0004 115A BAE7 E680 E000"            /* ..Æ..€...Zºçæ€à. */
	$"F587 FF02 FDF2 ED84 EE02 EFE7 DF88 E00A"            /* õ‡ÿ.ýòí„î.ïçßˆàÂ */
	$"E1EB 5D00 84AD B3B7 B0A5 9C86 9911 958B"            /* áë].„­³·°¥œ†™.•‹ */
	$"8077 7A52 0861 0508 8AA5 AEBB C0B7 AA9E"            /* €wzR.a..Š¥®»À·ªž */
	$"9299 2194 8C80 7672 7A46 0037 B2FF F57B"            /* ’™!”Œ€vrzF.7²ÿõ{ */
	$"0400 0531 5C74 848C 8982 7150 3007 0005"            /* ...1\t„Œ‰‚qP0... */
	$"519E D2BC E587 FF02 FEF3 ED85 EE02 EFE6"            /* QžÒ¼å‡ÿ.þóí…î.ïæ */
	$"DF88 E009 E4D6 1025 A5AE B7B4 AA9F 8699"            /* ßˆàÆäÖ.%¥®·´ªŸ†™ */
	$"1298 9287 7C74 7C40 041C 002A 9EA7 B4BE"            /* .˜’‡|t|@...*ž§´¾ */
	$"BEB2 A59C 9199 2398 9287 7C74 727A 2F00"            /* ¾²¥œ‘™#˜’‡|trz/. */
	$"56BD D342 0000 4288 9EA3 A09D 9C9C 9D9E"            /* V½ÓB..Bˆž£ œœž */
	$"9D96 7D3A 0000 18B3 B4E2 FB86 FF01 F7ED"            /* –}:...³´âû†ÿ.÷í */
	$"85EE 02EF EBE1 84E0 00DF 81E0 09EC A200"            /* …î.ïëá„à.ßàÆì¢. */
	$"57AF B2B8 B0A6 9C86 9912 978F 8279 7479"            /* W¯²¸°¦œ†™.—‚yty */
	$"3000 0300 58A5 AAB9 C0BA ACA1 9A91 9911"            /* 0...X¥ª¹Àº¬¡š‘™. */
	$"978F 8479 7373 7517 068F D664 0002 56A2"            /* —„yssu..Öd..V¢ */
	$"A19B 8599 0997 958B 5613 0032 A8E6 F685"            /* ¡›…™Æ—•‹V..2¨æö… */
	$"FF01 FDEF 85EE 03EF EDE3 DF83 E001 E3E3"            /* ÿ.ýï…î.ïíãßƒà.ãã */
	$"80E0 0AE2 E344 048C B1B6 B7AB A09A 8699"            /* €àÂâãD.Œ±¶·« š†™ */
	$"1195 8B7E 7673 731B 0000 1692 A4B0 BDC0"            /* .•‹~vss....’¤°½À */
	$"B6A6 9D92 9911 958B 8077 7276 6701 2BC3"            /* ¶¦’™.•‹€wrvg.+Ã */
	$"A403 0360 AAA3 9E9B 8699 0897 918E 7016"            /* ¤..`ª£ž›†™.—‘Žp. */
	$"0035 CCF3 84FF 02FD F1ED 84EE 03EF EEE3"            /* .5Ìó„ÿ.ýñí„î.ïîã */
	$"DF83 E00F DFE6 F0E5 DFE0 E8B3 002B A8B2"            /* ßƒà.ßæðåßàè³.+¨² */
	$"B8B3 A69C 8699 1298 9387 7A74 746C 0900"            /* ¸³¦œ†™.˜“‡zttlÆ. */
	$"003F A4A5 B4BF BEB0 A29B 9299 1192 887C"            /* .?¤¥´¿¾°¢›’™.’ˆ| */
	$"7472 7A4C 0055 D147 005B ADA9 A6A0 9B87"            /* trzL.UÑG.[­©¦ ›‡ */
	$"9907 958C 8961 0300 83FB 84FF 01F2 ED85"            /* ™.•Œ‰a..ƒû„ÿ.òí… */
	$"EE02 EDE3 DF84 E00F DFE4 F1EF E7E1 ED7A"            /* î.íãß„à.ßäñïçáíz */
	$"0058 B2B5 B7AF A19B 8699 1296 9083 7874"            /* .X²µ·¯¡›†™.–ƒxt */
	$"765D 0300 0063 A6A9 B9C0 BAAA 9F9B 9199"            /* v]...c¦©¹ÀºªŸ›‘™ */
	$"1297 9084 7973 737B 3200 74B8 0F16 9DAE"            /* .—„yss{2.t¸..® */
	$"AEA7 A09A 8799 0697 9187 7F27 004C 84FF"            /* ®§ š‡™.—‘‡.'.L„ÿ */
	$"01F8 ED86 EE01 E5DF 85E0 0EDF E4F0 F0F1"            /* .øí†î.åß…à.ßäððñ */
	$"EEE3 3501 88B4 B8B6 A89D 8799 1194 8B7E"            /* îã5.ˆ´¸¶¨‡™.”‹~ */
	$"7572 7947 0000 0A8A A5AE BDBF B5A5 9C92"            /* uryG..ÂŠ¥®½¿µ¥œ’ */
	$"9911 968D 8077 7275 7410 0CB2 7A00 53B2"            /* ™.–€wrut..²z.S² */
	$"B1B1 A69D 8999 0695 8C85 5200 17EE 82FF"            /* ±±¦‰™.•Œ…R..î‚ÿ */
	$"01FB EF85 EE01 EFE6 87E0 01DF E480 F009"            /* .ûï…î.ïæ‡à.ßä€ðÆ */
	$"F8CD 0621 A6B5 B8B2 A49B 8699 1297 9187"            /* øÍ.!¦µ¸²¤›†™.—‘‡ */
	$"7A74 7278 3400 002C A0A6 B4BE BBAF A19A"            /* ztrx4.., ¦´¾»¯¡š */
	$"9199 1298 9488 7C75 727B 4E00 42AB 4001"            /* ‘™.˜”ˆ|ur{N.B«@. */
	$"7EB5 B5AF A39B 8999 0696 9085 6F0A 01BC"            /* ~µµ¯£›‰™.–…oÂ.¼ */
	$"81FF 01FE F185 EE01 EFE7 88E0 01DF E480"            /* ÿ.þñ…î.ïçˆà.ßä€ */
	$"F009 FDA1 0050 B1B6 B7AD A09A 8699 1196"            /* ðÆý¡.P±¶·­ š†™.– */
	$"8E84 7973 7374 2200 004E A7A8 B8BF B8AA"            /* Ž„ysst"..N§¨¸¿¸ª */
	$"9E92 9912 9791 8479 7373 7929 0063 A215"            /* ž’™.—‘„yssy).c¢. */
	$"0793 B4B6 ADA0 9A89 9906 9892 867F 2D00"            /* .“´¶­ š‰™.˜’†.-. */
	$"8A81 FF01 F5ED 84EE 01EF E889 E00D DFE4"            /* Šÿ.õí„î.ïè‰à.ßä */
	$"F0F0 F1FB 6305 8FB2 B7B4 A99E 8799 1194"            /* ððñûc.²·´©ž‡™.” */
	$"8A80 7672 736E 0F00 0175 A8AD BABE B3A6"            /* Š€vrsn...u¨­º¾³¦ */
	$"9D92 9911 968E 8177 7375 6F0D 006B C124"            /* ’™.–Žwsuo..kÁ$ */
	$"0EA1 B5B6 AC9F 8B99 0593 8782 5100 5780"            /* .¡µ¶¬Ÿ‹™.“‡‚Q.W€ */
	$"FF02 F7EE EF83 EE01 EFE8 8AE0 0DDF E4F0"            /* ÿ.÷îïƒî.ïèŠà.ßäð */
	$"F0F4 E923 2EAA B2B7 B0A5 9C86 9912 9892"            /* ðôé#.ª²·°¥œ†™.˜’ */
	$"877C 7472 755E 0400 1596 A7B2 BDBB ACA2"            /* ‡|tru^...–§²½»¬¢ */
	$"9A92 9911 938A 7D75 7279 5400 11A3 A91A"            /* š’™.“Š}uryT..£©. */
	$"13A4 B6B7 AC9E 8B99 0B95 8A80 6801 25F4"            /* .¤¶·¬ž‹™.•Š€h.%ô */
	$"FFFA EFEE EF82 EE01 EFE9 8BE0 0DDF E5F0"            /* ÿúïîï‚î.ïé‹à.ßåð */
	$"F0F7 CB00 5AB2 B5B6 ACA1 9A86 9911 978F"            /* ð÷Ë.Z²µ¶¬¡š†™.— */
	$"827A 7471 784C 0000 36A3 AAB6 BEB6 A89E"            /* ‚ztqxL..6£ª¶¾¶¨ž */
	$"9299 1297 9085 7974 727A 3700 39BC 9B17"            /* ’™.—…ytrz7.9¼›. */
	$"15A4 B7B8 AD9F 8B99 0996 8C80 740F 06DA"            /* .¤·¸­Ÿ‹™Æ–Œ€t..Ú */
	$"FFF0 EE80 EF80 EE01 EFEB 8CE0 0CDF E4EF"            /* ÿðî€ï€î.ïëŒà.ßäï */
	$"F0FC 9B00 7DB2 B6B4 A99F 8799 1196 8C80"            /* ðü›.}²¶´©Ÿ‡™.–Œ€ */
	$"7772 7178 3A00 0055 A9AD BABD B2A4 9C92"            /* wrqx:..U©­º½²¤œ’ */
	$"9912 958C 8177 7273 751A 005D C491 1616"            /* ™.•Œwrsu..]Ä‘.. */
	$"A5B8 B9AE 9F8B 9907 978D 8079 2300 B8FF"            /* ¥¸¹®Ÿ‹™.—€y#.¸ÿ */
	$"81EF 80EE 02EF EAE2 8CE0 0CDF E4EF F0FC"            /* ï€î.ïêâŒà.ßäïðü */
	$"6508 9CB1 B7B1 A59D 8799 1194 897D 7672"            /* e.œ±·±¥‡™.”‰}vr */
	$"7274 2200 037C A9B2 BDBC ADA0 9A92 9913"            /* rt"..|©²½¼­ š’™. */
	$"9287 7D75 7277 6103 0075 A67E 1318 A6BA"            /* ’‡}urwa..u¦~..¦º */
	$"BCAF A19A 8A99 0797 8E82 7F39 0083 FC81"            /* ¼¯¡šŠ™.—Ž‚.9.ƒü */
	$"EF04 EEEE EFEB E18D E00C DFE4 EFF2 EF38"            /* ï.îîïëáà.ßäïòï8 */
	$"29AA B2B7 AFA2 9B87 9910 9387 7A74 7273"            /* )ª²·¯¢›‡™.“‡ztrs */
	$"6D0F 001D 9AA8 B5BD B8A9 9E92 9914 978F"            /* m...š¨µ½¸©ž’™.— */
	$"847A 7472 7A44 0008 8F86 680F 1AA6 BBBD"            /* „ztrzD..†h..¦»½ */
	$"B2A1 9A8A 9908 978F 827F 4B00 57F6 F081"            /* ²¡šŠ™.—‚.K.Wöð */
	$"EF02 EEED E187 E000 DF83 E00C DFE3 EFF4"            /* ï.îíá‡à.ßƒà.ßãïô */
	$"E113 49B1 B4B6 ADA0 9A86 9911 9891 8579"            /* á.I±´¶­ š†™.˜‘…y */
	$"7471 755F 0300 50A7 AAB8 BDB5 A59B 9299"            /* tqu_..P§ª¸½µ¥›’™ */
	$"1496 8C80 7872 7278 2800 3AD8 9052 0B1E"            /* .–Œ€xrrx(.:ØR.. */
	$"A7BC BEB4 A29A 8A99 0898 9082 7D58 0028"            /* §¼¾´¢šŠ™.˜‚}X.( */
	$"E3F3 81EF 02EE E4DF 86E0 02DE E7F6 80F7"            /* ãóï.îäß†à.Þçö€÷ */
	$"80DE 0AE6 F1F9 AF00 6CB3 B5B5 A99E 8799"            /* €ÞÂæñù¯.l³µµ©ž‡™ */
	$"1197 9083 7873 7178 4400 0178 A6AE BBBC"            /* .—ƒxsqxD..x¦®»¼ */
	$"B0A1 9A91 9915 9893 887B 7572 756C 0A00"            /* °¡š‘™.˜“ˆ{urulÂ. */
	$"72BF 5B35 0623 A9BD C1B6 A49B 8A99 0E98"            /* r¿[5.#©½Á¶¤›Š™.˜ */
	$"9083 7C68 0204 CAF6 EFEF F0EE E3DF 85E0"            /* ƒ|h..Êöïïðîãß…à */
	$"03DF DFE9 FD81 FF0D F0EF F3FB FDFF 6F00"            /* .ßßéýÿ.ðïóûýÿo. */
	$"89B1 B5B4 A79C 8799 1097 9083 7873 7277"            /* ‰±µ´§œ‡™.—ƒxsrw */
	$"3500 1293 A5B2 BCBA AC9E 9299 1597 9083"            /* 5..“¥²¼º¬ž’™.—ƒ */
	$"7874 727A 5100 048A A060 3D01 29AA BDC2"            /* xtrzQ..Š `=.)ª½Â */
	$"B8A5 9B8A 990D 9890 847B 6F0E 00AA FAEF"            /* ¸¥›Š™.˜„{o..ªúï */
	$"F0EF E3DF 85E0 02DF E4F2 89FF 0754 0499"            /* ðïãß…à.ßäò‰ÿ.T.™ */
	$"B1B5 B2A5 9B87 9910 9790 8378 7372 7846"            /* ±µ²¥›‡™.—ƒxsrxF */
	$"002C 9EA6 B5BC B7A8 9D92 9915 958D 7F76"            /* .,ž¦µ¼·¨’™.•.v */
	$"7272 782B 001B D3DF CE9F 002F ACBE C2B8"            /* rrx+..ÓßÎŸ./¬¾Â¸ */
	$"A59B 8A99 0C98 9084 7B73 1800 9DFB EFF0"            /* ¥›Š™.˜„{s..ûïð */
	$"E6DF 85E0 02DE E3FA 89FF 08FE 3B18 A3B1"            /* æß…à.Þãú‰ÿ.þ;.£± */
	$"B5B0 A39B 8799 1098 9186 7B74 7278 4C00"            /* µ°£›‡™.˜‘†{trxL. */
	$"4CA7 A9B8 BCB4 A69C 9199 1698 9389 7C75"            /* L§©¸¼´¦œ‘™.˜“‰|u */
	$"7276 6305 004C E8DD 8560 0036 ADBE C2B8"            /* rvc..LèÝ…`.6­¾Â¸ */
	$"A59B 8A99 0A98 9084 7B76 2200 82FC F0E7"            /* ¥›Š™Â˜„{v".‚üðç */
	$"86E0 02DE E6F9 8AFF 08F7 2030 A8B0 B4AD"            /* †à.ÞæùŠÿ.÷ 0¨°´­ */
	$"A29A 8899 0F95 8B80 7875 762C 006F A8AD"            /* ¢šˆ™.•‹€xuv,.o¨­ */
	$"BBBA AFA2 9B91 9916 9790 8478 7272 7A44"            /* »º¯¢›‘™.—„xrrzD */
	$"0000 7EF1 D963 3C00 3FB0 BDC2 B5A4 9B8A"            /* ..~ñÙc<.?°½Âµ¤›Š */
	$"9909 9890 847A 792E 0072 FEE7 87E0 01EE"            /* ™Æ˜„zy..rþç‡à.î */
	$"FD8B FF07 F20F 3FAB B0B3 AB9F 8999 0E98"            /* ý‹ÿ.ò.?«°³«Ÿ‰™.˜ */
	$"9187 7F7B 6A06 078C A7B1 BBB7 AB9F 9299"            /* ‘‡.{j..Œ§±»·«Ÿ’™ */
	$"1694 8C7F 7672 7377 2800 01A9 EBD2 AC61"            /* .”Œ.vrsw(..©ëÒ¬a */
	$"004A B3BB BEB1 A29A 8A99 0898 9084 7A7B"            /* .J³»¾±¢šŠ™.˜„z{ */
	$"3600 69F4 85E0 02DF E0F4 8DFF 07ED 094B"            /* 6.iô…à.ßàôÿ.íÆK */
	$"ADAF B2AA 9E8A 990D 968F 8886 5C00 1C99"            /* ­¯²ªžŠ™.–ˆ†\..™ */
	$"A9B4 BCB5 A89D 9299 1691 887C 7572 7370"            /* ©´¼µ¨’™.‘ˆ|ursp */
	$"1100 11CD EAC0 763A 0057 B5B9 BAAE A09A"            /* ...ÍêÀv:.Wµ¹º® š */
	$"8A99 0998 9084 7A7A 3B00 57E9 E183 E002"            /* Š™Æ˜„zz;.Wéáƒà. */
	$"DFE4 F58E FF07 E706 53AE AEB0 A89D 8B99"            /* ßäõŽÿ.ç.S®®°¨‹™ */
	$"0C96 9295 4900 3DA5 AAB7 BCB2 A59C 9199"            /* .–’•I.=¥ª·¼²¥œ‘™ */
	$"1697 8E83 7974 7177 5901 0041 E5E3 DBC4"            /* .—ŽƒytqwY..AåãÛÄ */
	$"3200 67B3 B6B5 A99D 8B99 0998 9083 797A"            /* 2.g³¶µ©‹™Æ˜ƒyz */
	$"4100 4BE5 E282 E002 DFED FE8F FF07 E606"            /* A.Kåâ‚à.ßíþÿ.æ. */
	$"57AE ADAF A79D 8E99 0932 005B A9AC B9BB"            /* W®­¯§Ž™Æ2.[©¬¹» */
	$"AEA2 9B91 9916 958A 7F76 7272 7B3D 0000"            /* ®¢›‘™.•Š.vrr{=.. */
	$"77F6 E1EA D40D 007D AFB0 ADA3 9B8B 9909"            /* wöáêÔ..}¯°­£›‹™Æ */
	$"9890 8379 7A44 0047 E4E2 81E0 01DE EB89"            /* ˜ƒyzD.Gäâà.Þë‰ */
	$"FF02 FBE9 E882 FF07 F109 56AD ACAD A69C"            /* ÿ.ûéè‚ÿ.ñÆV­¬­¦œ */
	$"8C99 0B9A 961B 0074 A9B0 BAB9 ABA0 9A90"            /* Œ™.š–..t©°º¹« š */
	$"9917 9892 867C 7679 7670 2100 0098 EDCB"            /* ™.˜’†|vyvp!..˜íË */
	$"B76C 000A 91AA ABA7 9F9A 8B99 0F97 9082"            /* ·l.Â‘ª«§Ÿš‹™.—‚ */
	$"7979 4700 45E3 E2E0 E0DF E0EE FE89 FF02"            /* yyG.Eãâààßàîþ‰ÿ. */
	$"EEDE DF82 FF07 D20A 4EAB AAAB A49B 8C99"            /* îÞß‚ÿ.ÒÂN«ª«¤›Œ™ */
	$"0A9D 8804 098E A8B4 BBB6 A89D 9199 0797"            /* Âˆ.ÆŽ¨´»¶¨‘™.— */
	$"8E81 7D75 4C25 0E80 000B 1931 3B2E 0000"            /* Ž}uL%.€...1;... */
	$"2CA1 A6A5 A19B 8C99 0D97 8F81 7879 4800"            /* ,¡¦¥¡›Œ™.—xyH. */
	$"45E3 E2E0 DEE6 F88A FF10 F4DF E0E0 EEEE"            /* EãâàÞæøŠÿ.ôßààîî */
	$"F0F5 DA92 1C40 A7A7 A9A3 9B8C 990A A16B"            /* ðõÚ’.@§§©£›Œ™Â¡k */
	$"0023 9EA9 B7BC B3A4 9C91 9904 958A 8563"            /* .#ž©·¼³¤œ‘™.•Š…c */
	$"1C81 0002 090F 0981 0005 0A7A A3A0 9F9D"            /* ...Æ.Æ..Âz£ Ÿ */
	$"8D99 0C97 8D80 7879 4500 47E6 E2DF E9FD"            /* ™.—€xyE.Gæâßéý */
	$"89FF 12FE F7E8 DFE0 E0DD E0D3 AEAB B22F"            /* ‰ÿ.þ÷èßààÝàÓ®«²/ */
	$"2CA2 A6A6 A29B 8C99 0AA4 5300 3EA4 ABB9"            /* ,¢¦¦¢›Œ™Â¤S.>¤«¹ */
	$"BBB1 A39B 9099 1698 928F 5D01 001F 415F"            /* »±£›™.˜’]...A_ */
	$"6E87 9390 7556 381F 61A3 9C9D 9C9A 8D99"            /* n‡“uV8.a£œœš™ */
	$"0B96 8C7F 7679 4200 45D9 E2E8 FE89 FF04"            /* .–Œ.vyB.EÙâèþ‰ÿ. */
	$"FBF2 EEE7 DF80 E00B E1DC B5A0 A949 119B"            /* ûòîçß€à.áÜµ ©I.› */
	$"A3A2 9F9B 8B99 0B9A 9F3B 0063 A8AD BABA"            /* £¢Ÿ›‹™.šŸ;.c¨­ºº */
	$"AEA0 9A90 9912 9696 6F0E 3070 939D 9F9E"            /* ® š™.––o.0p“Ÿž */
	$"9C9B 9CA0 A29D 989D 9980 9A8E 9909 968C"            /* œ›œ ¢˜™€šŽ™Æ–Œ */
	$"7F76 793B 0025 54CE 89FF 06FB F3EE EEEF"            /* .vy;.%TÎ‰ÿ.ûóîîï */
	$"E7DF 81E0 0AE1 E3BD B89B 0F81 A19D 9C9A"            /* çßàÂáã½¸›.¡œš */
	$"8B99 0A9B 991D 098E A6B1 BBB9 AA9E 9199"            /* ‹™Â›™.ÆŽ¦±»¹ªž‘™ */
	$"0697 8B36 6798 9C9A 8599 019A 9B93 9909"            /* .—‹6g˜œš…™.š›“™Æ */
	$"958A 7C76 7936 0042 A0EE 87FF 08FB F5EF"            /* •Š|vy6.B î‡ÿ.ûõï */
	$"EDEE EEEF E7DF 83E0 07E5 DED5 2D51 A49A"            /* íîîïçßƒà.åÞÕ-Q¤š */
	$"9A8C 990A 9C8F 0623 9FA5 B4BC B6A8 9C91"            /* šŒ™Âœ.#Ÿ¥´¼¶¨œ‘ */
	$"9904 9B6C 759D 989F 9908 9488 7B75 772E"            /* ™.›lu˜Ÿ™.”ˆ{uw. */
	$"0032 D286 FF02 FEF5 EF82 EE02 EFE7 DF83"            /* .2Ò†ÿ.þõï‚î.ïçßƒ */
	$"E006 E1DF C151 219D 9A8D 990A 9E7E 003A"            /* à.áßÁQ!š™Âž~.: */
	$"A2A7 B6BB B4A5 9B91 9902 947F 98A0 9909"            /* ¢§¶»´¥›‘™.”.˜ ™Æ */
	$"9892 867A 7574 2200 65F3 84FF 03FE FAF2"            /* ˜’†zut".eó„ÿ.þúò */
	$"EE81 EF04 EEEE EFE7 DF85 E004 E2B6 0A6A"            /* îï.îîïçß…à.â¶Âj */
	$"A38D 990A A267 0056 A8A9 B8BB B2A4 9B91"            /* £™Â¢g.V¨©¸»²¤›‘ */
	$"9901 9797 A199 0897 9183 7875 6F14 00B2"            /* ™.——¡™.—‘ƒxuo..² */
	$"84FF 03FA F2EF EE84 EF01 EEE6 86E0 05E2"            /* „ÿ.úòïî„ï.îæ†à.â */
	$"EB69 0E8A 9F8C 990A A350 006E A7AB B9BA"            /* ëi.ŠŸŒ™Â£P.n§«¹º */
	$"AFA1 9AB7 9908 968E 8076 7566 0309 DB82"            /* ¯¡š·™.–Ž€vuf.ÆÛ‚ */
	$"FF03 FAF3 EFEE 83EF 03F0 EEE8 E388 E005"            /* ÿ.úóïîƒï.ðîèãˆà. */
	$"E6CF 0E22 9A9E 8A99 0A9A 9F3E 0082 A5AE"            /* æÏ."šžŠ™ÂšŸ>.‚¥® */
	$"BAB8 AC9F B899 0894 8A7F 7577 5600 38F6"            /* º¸¬Ÿ¸™.”Š.uwV.8ö */
	$"80FF 03FD F5EF EE82 EF05 F0F0 EDE8 E2DF"            /* €ÿ.ýõïî‚ï.ððíèâß */
	$"8AE0 06EA 9905 3197 A19A 8899 0A9A 9D29"            /* Šà.ê™.1—¡šˆ™Âš) */
	$"0994 A5B1 BAB6 AA9E B799 0E98 9288 7B74"            /* Æ”¥±º¶ªž·™.˜’ˆ{t */
	$"7940 0081 FFFF FDF7 F1EE 82EF 05F0 EFEB"            /* y@.ÿÿý÷ñî‚ï.ðïë */
	$"E7E2 DF8D E007 ECAD 111E 759E 9E9A 8699"            /* çâßà.ì­..užžš†™ */
	$"0A9A 9812 229C A6B3 BBB4 A79B B799 0C96"            /* Âš˜."œ¦³»´§›·™.– */
	$"8E83 7873 7523 01C7 FFF8 F0EE 82EF 05F0"            /* Žƒxsu#.Çÿøðî‚ï.ð */
	$"EEEA E3E0 DF90 E009 F0A6 0D00 3D88 9FA2"            /* îêãàßàÆð¦..=ˆŸ¢ */
	$"9D9A 8399 0A9E 8703 36A2 A8B5 BAB2 A49B"            /* šƒ™Âž‡.6¢¨µº²¤› */
	$"B799 0A94 8A7F 7674 6D07 1CEC F8EE 81EF"            /* ·™Â”Š.vtm..ìøîï */
	$"06F0 F0EE E9E2 E0DF 8CE0 01DF DF82 E00B"            /* .ððîéâàßŒà.ßß‚à. */
	$"EABD 4E05 0031 5F89 9FA2 9D9A 8099 0AA1"            /* ê½N..1_‰Ÿ¢š€™Â¡ */
	$"6E00 4DA9 A9B6 BAAF A29A B699 0A98 9186"            /* n.M©©¶º¯¢š¶™Â˜‘† */
	$"7C74 7951 0061 F9F0 80EF 04F0 EFEC E8E2"            /* |tyQ.aùð€ï.ðïìèâ */
	$"8FE0 21E2 E7E5 E2E0 E0DF E0E0 E8E7 BA5D"            /* à!âçåâààßààèçº] */
	$"1100 0132 6489 9EA0 9A99 A155 0064 A8AC"            /* ...2d‰ž š™¡U.d¨¬ */
	$"B8B8 ACA1 9AB6 9911 958B 8078 756E 1602"            /* ¸¸¬¡š¶™.•‹€xun.. */
	$"B6FA EFEF F0EE EAE4 E0DF 8FE0 22DF E4EF"            /* ¶úïïðîêäàßà"ßäï */
	$"EEEE ECE9 E6E2 E0DF E0E8 E9D0 A254 0200"            /* îîìéæâàßàèéÐ¢T.. */
	$"002D 729C A2A6 4000 77A8 AEB9 B7A9 9F9A"            /* .-rœ¢¦@.w¨®¹·©Ÿš */
	$"B599 1097 9085 7A74 7A48 0035 F0F2 F0EE"            /* µ™.—…ztzH.5ðòðî */
	$"E7E2 E0DF 8AE0 01DF DF82 E001 DFE4 81EE"            /* çâàßŠà.ßß‚à.ßäî */
	$"1BEF EFEE EBE9 E6E3 E2E4 EAE8 BE84 4E0D"            /* .ïïîëéæãâäêè¾„N. */
	$"0017 4E81 3201 89A7 B0B9 B6A8 9EB6 990D"            /* ..N2.‰§°¹¶¨ž¶™. */
	$"948B 7F77 7572 1800 92FC EDE8 E2DF 8AE0"            /* ”‹.wur..’üíèâßŠà */
	$"05DF DEE0 E2E4 E181 E001 DFE4 81EE 80EF"            /* .ßÞàâäáà.ßäî€ï */
	$"80F0 15EF EDEA E8E6 E9EE E8CB 8739 0001"            /* €ð.ïíêèæéîèË‡9.. */
	$"0415 95A7 B2BA B4A6 9DB5 990C 948D 8378"            /* ..•§²º´¦µ™.”ƒx */
	$"747A 3D00 35E8 E8E0 DF88 E00F DFDE DFE1"            /* tz=.5èèàßˆà.ßÞßá */
	$"E6F0 F6FB FEFA F6F7 E0E0 DFE4 82EE 84EF"            /* æðöûþúö÷ààßä‚î„ï */
	$"13F0 F0EF EEED ECEC F0E7 920C 0029 9CA9"            /* .ððïîíììðç’..)œ© */
	$"B4BA B2A3 9BB3 990C 9894 8C82 7A74 7959"            /* ´º²£›³™.˜”Œ‚ztyY */
	$"0424 C6E8 DF86 E008 DFDE DEE0 E5ED F3FA"            /* .$Æèß†à.ßÞÞàåíóú */
	$"FE84 FF03 E0E0 DFE4 83EE 88EF 0EF0 F0EF"            /* þ„ÿ.ààßäƒîˆï.ððï */
	$"F1FD 8300 36A2 A9B5 B9B0 A19B B299 0C97"            /* ñýƒ.6¢©µ¹°¡›²™.— */
	$"948D 837B 7578 6913 0097 F5DF 83E0 08DF"            /* ”ƒ{uxi..—õßƒà.ß */
	$"DEDF E0E2 EAF0 F9FE 88FF 03E0 E0DF E483"            /* Þßàâêðùþˆÿ.ààßäƒ */
	$"EE8C EF0A F7D0 0046 A7AB B6B8 AEA0 9BAF"            /* îŒïÂ÷Ð.F§«¶¸® ›¯ */
	$"991A 9897 9490 8B83 7B77 7B5E 0F07 164A"            /* ™.˜—”‹ƒ{w{^...J */
	$"D2E9 E0DF DFDE E0E1 E4EA F3F8 FD8C FF03"            /* ÒéàßßÞàáäêóøýŒÿ. */
	$"E0E0 DFE4 84EE 8BEF 0AF9 B000 5BA8 ADB7"            /* ààßä„î‹ïÂù°.[¨­· */
	$"B7AB 9F9A AD99 1897 9593 8F8A 857F 7979"            /* ·«Ÿš­™.—•“Š….yy */
	$"7849 0819 BCD2 1321 C3E7 E2E7 EDF6 FAFE"            /* xI..¼Ò.!Ãçâçíöúþ */
	$"90FF 03E0 E0DF E384 ED02 EEEF EE88 EF0A"            /* ÿ.ààßã„í.îïîˆïÂ */
	$"FB98 0069 A8AE B7B6 A99D 9A96 9903 9DA1"            /* û˜.i¨®·¶©š–™.¡ */
	$"A19D 8D99 1697 9593 908C 8985 7F7B 797B"            /* ¡™.—•“Œ‰….{y{ */
	$"6632 0000 2CE3 FFB2 012C E4FD 95FF 0DDE"            /* f2..,ãÿ².,äý•ÿ.Þ */
	$"DEDF F0FB F9F7 F6F4 F2F1 F0EF EF82 EE83"            /* Þßðûù÷öôòñðïï‚îƒ */
	$"EF09 FD77 007B A7B0 B8B5 A89C 9699 05A0"            /* ïÆýw.{§°¸µ¨œ–™.  */
	$"8449 3E86 9D88 9909 9897 9592 8F8C 8986"            /* „I>†ˆ™Æ˜—•’Œ‰† */
	$"807D 807B 0C6C 3800 003B 8A04 3CFC FFAF"            /* €}€{.l8..;Š.<üÿ¯ */
	$"018F 94FF 04FD FDF0 EFF5 84FF 18FE FDFC"            /* .”ÿ.ýýðïõ„ÿ.þýü */
	$"FAF9 F7F5 F4F3 F1F0 EFEF EEEF F95C 028C"            /* úù÷õôóñðïïîïù\.Œ */
	$"A6B1 B7B2 A59C 9599 07A0 6917 4B1E 2398"            /* ¦±·²¥œ•™. i.K.#˜ */
	$"9B83 991E 9897 9594 928E 8A87 837F 7C79"            /* ›ƒ™.˜—•”’ŽŠ‡ƒ.|y */
	$"7778 785E 3207 003E 9CEC FF6A 00D5 FFFF"            /* wxx^2..>œìÿj.Õÿÿ */
	$"8312 E792 FF02 F3E2 E290 FF0F FEFD FCFA"            /* ƒ.ç’ÿ.óââÿ.þýüú */
	$"F8F8 F53D 1998 A6B2 B7B1 A49B 9499 089E"            /* øøõ=.˜¦²·±¤›”™.ž */
	$"8209 ABFF C008 8A9D 8099 1697 9694 928F"            /* ‚Æ«ÿÀ.Š€™.—–”’ */
	$"8C88 8682 7D7B 7876 7679 7761 2600 0021"            /* Œˆ†‚}{xvvywa&..! */
	$"ADFD 80FF 07D4 048D FFFF FA22 8F91 FF03"            /* ­ý€ÿ.Ô.ÿÿú"‘ÿ. */
	$"F7E0 DFDF 96FF 09F5 242F A1A6 B2B7 AFA2"            /* ÷àßß–ÿÆõ$/¡¦²·¯¢ */
	$"9A94 9920 A136 49FF FFEB 1677 9D97 9694"            /* š”™ ¡6Iÿÿë.w—–” */
	$"928F 8C89 8581 7E7B 7977 777A 776C 4D20"            /* ’Œ‰…~{ywwzwlM  */
	$"0000 3B93 E482 FF08 FA1B 49FE FFFF 7F38"            /* ..;“ä‚ÿ.ú.Iþÿÿ.8 */
	$"FA84 FF0F FEFE FDFC FBFB F9F8 F8F6 F5F5"            /* ú„ÿ.þþýüûûùøøöõõ */
	$"E9DE E0E0 96FF 09EB 0B46 A4A6 B3B7 AEA1"            /* éÞàà–ÿÆë.F¤¦³·®¡ */
	$"9A93 9921 9F7C 05BE FFFF F526 6498 8F8C"            /* š“™!Ÿ|.¾ÿÿõ&d˜Œ */
	$"8985 817F 7C7A 7676 787A 766C 5025 0100"            /* ‰….|zvvxzvlP%.. */
	$"2069 B6F0 FDFD 82FF 12FC 1F2B FBFF FFD0"            /*  i¶ðýý‚ÿ.ü.+ûÿÿÐ */
	$"1ADC FDF8 F7F6 F6F4 F3F2 F2F1 80F0 82EF"            /* .Üýø÷ööôóòòñ€ð‚ï */
	$"04F0 E8DF E0E0 96FF 08D3 015F A6A7 B4B5"            /* .ðèßàà–ÿ.Ó._¦§´µ */
	$"AB9F 9499 299E 351B F7FF FFFB 354F 8C82"            /* «Ÿ”™)ž5.÷ÿÿû5OŒ‚ */
	$"7F7C 7977 7675 777A 7561 4223 0500 2060"            /* .|ywvuwzuaB#.. ` */
	$"A8E4 FBF9 F1EE F2FC FEFF FFC3 5100 6480"            /* ¨äûùñîòüþÿÿÃQ.d€ */
	$"FF03 F528 AAFA 82EF 89F0 04F1 E8DF E0E0"            /* ÿ.õ(ªú‚ï‰ð.ñèßàà */
	$"96FF 08C0 0069 A6A8 B4B4 AA9E 9199 0598"            /* –ÿ.À.i¦¨´´ªž‘™.˜ */
	$"9797 9214 4681 FF22 473B 837A 7979 7A79"            /* ——’.Fÿ"G;ƒzyyzy */
	$"746F 5C42 1B00 0006 358B E6FC FDF6 F2F2"            /* to\B....5‹æüýöòò */
	$"F3F6 FBF9 E5BF 6400 0043 E080 FF05 FB2A"            /* óöûùå¿d..Cà€ÿ.û* */
	$"7EF9 ECEC 81ED 84EE 81EF 04F0 E8DF E0E0"            /* ~ùììí„îï.ðèßàà */
	$"95FF 09FD 9800 77A3 A9B4 B3A9 9E91 9905"            /* •ÿÆý˜.w£©´³©ž‘™. */
	$"9591 8F7F 006B 81FF 2161 1A79 7268 5B49"            /* •‘..kÿ!a.yrh[I */
	$"3018 0200 0B2B 67AA E4FB FFF9 F2EC ECEB"            /* 0....+gªäûÿùòììë */
	$"E6E0 CC9A 5017 000D 55B1 FB81 FF09 F81D"            /* æàÌšP...U±ûÿÆø. */
	$"4FE9 E1E0 E1E0 E1E0 84E1 82E2 00E1 80E0"            /* Oéáàáàáà„á‚â.á€à */
	$"94FF 0AA8 3B0C 0884 A2AB B4B2 A79D 9099"            /* ”ÿÂ¨;..„¢«´²§™ */
	$"0696 8E86 8467 0086 81FF 038E 0128 0F80"            /* .–Ž†„g.†ÿ.Ž.(.€ */
	$"0017 1544 76A7 C8D5 C8AC 8D79 6148 3C29"            /* ...Dv§ÈÕÈ¬yaH<) */
	$"2721 1411 0D19 4E6A A5E6 84FF 04F5 143F"            /* '!....Nj¥æ„ÿ.õ.? */
	$"E4E2 92E0 02F7 F6FA 87FF 81FD 10FA FBFD"            /* äâ’à.÷öú‡ÿý.úûý */
	$"F8EF 6700 0008 0A8A A2AB B4B1 A59B 9099"            /* øïg...ÂŠ¢«´±¥›™ */
	$"0694 8A80 7C5C 0093 81FF 0CC7 0007 2C4E"            /* .”Š€|\.“ÿ.Ç..,N */
	$"75B0 E0E2 B162 210E 8000 0A07 162B 3C4B"            /* u°àâ±b!.€.Â..+<K */
	$"4F57 688A BDEC 88FF 04DF 0058 F6E7 94E0"            /* OWhŠ½ìˆÿ.ß.Xöç”à */
	$"04E1 EAF0 EBE8 82E9 00E8 81E2 10E1 E1E2"            /* .áêðëè‚é.èâ.ááâ */
	$"E4D3 0200 050D 1193 A2AC B3AF A49B 9099"            /* äÓ.....“¢¬³¯¤›™ */
	$"0692 867B 7857 0099 81FF 12F9 3D01 7DB0"            /* .’†{xW.™ÿ.ù=.}° */
	$"A881 491D 294A 6C8A A5C3 D9E9 F6FB 88FF"            /* ¨I.)JlŠ¥ÃÙéöûˆÿ */
	$"01F5 E582 FF07 D257 0B92 85BB EFE3 93E0"            /* .õå‚ÿ.ÒW.’…»ïã“à */
	$"80DE 88DF 10E0 E0DF E7CC 0B00 0305 1A99"            /* €Þˆß.ààßçÌ.....™ */
	$"A2AC B3AE A39A 8F99 0797 9084 7978 5500"            /* ¢¬³®£š™.—„yxU. */
	$"9B82 FF08 E355 1015 213D 73B2 F48F FF11"            /* ›‚ÿ.ãU..!=s²ôÿ. */
	$"FEFF F88A D3FF FFF6 A30F 0E58 3900 1782"            /* þÿøŠÓÿÿö£..X9..‚ */
	$"E2E5 A2E0 0EE5 E754 0000 060E 219B A2AC"            /* âå¢à.åçT....!›¢¬ */
	$"B2AC A19A 8F99 0797 9082 7978 5500 9284"            /* ²¬¡š™.—‚yxU.’„ */
	$"FF02 E7E5 F290 FF0E E199 92F8 FFFF D949"            /* ÿ.çåòÿ.á™’øÿÿÙI */
	$"F1EF 2F00 3D48 0A81 0004 4CDF E9DF DF9E"            /* ñï/.=HÂ..Lßéßßž */
	$"E002 E5E2 5980 0009 0408 2D9D A2AD B1AA"            /* à.åâY€.Æ..-¢­±ª */
	$"A09A 8F99 0797 8F80 7776 6000 8092 FF01"            /*  š™.—€wv`.€’ÿ. */
	$"E9FE 83FF 02F1 4D52 81FF 069D 5432 2C7C"            /* éþƒÿ.ñMRÿ.T2,| */
	$"5707 8300 073A C2F7 E7E1 E0DE DE80 DF96"            /* W.ƒ..:Â÷çáàÞÞ€ß– */
	$"E002 E4E5 5881 0008 0300 39A0 A2AD AFA9"            /* à.äåX....9 ¢­¯© */
	$"9F90 9907 978D 7F77 7370 0B5B 91FF 03E1"            /* Ÿ™.—.wsp.[‘ÿ.á */
	$"3F6B F483 FF02 D677 FA81 FF04 9700 3342"            /* ?kôƒÿ.Öwúÿ.—.3B */
	$"0185 000E 1393 FFFF F7F2 ECE8 E9E4 E0E0"            /* .…...“ÿÿ÷òìèéäàà */
	$"DEDF DF90 E002 E3E9 5F82 0008 1B03 3FA2"            /* Þßßà.ãé_‚....?¢ */
	$"A2AC AEA8 9E90 9908 968C 7E76 7276 3D21"            /* ¢¬®¨ž™.–Œ~vrv=! */
	$"F48E FF06 D364 1600 0035 D883 FF01 DFE9"            /* ôŽÿ.Ód...5Øƒÿ.ßé */
	$"82FF 04B2 2900 0C0C 8600 015B EE82 FF06"            /* ‚ÿ.²)...†..[î‚ÿ. */
	$"FEF8 F6F0 E9E5 DF8E E002 E3EA 8083 0008"            /* þøöðéåßŽà.ãê€ƒ.. */
	$"3505 45A3 A2AC AEA6 9D90 9908 958B 7D75"            /* 5.E£¢¬®¦™.•‹}u */
	$"7272 6F0E B78D FF01 B817 8200 0154 FE83"            /* rro.·ÿ.¸.‚..Tþƒ */
	$"FF02 E9D4 FA81 FF08 EC69 030E 1B2B 3417"            /* ÿ.éÔúÿ.ìi...+4. */
	$"0482 0001 46DA 86FF 02FE EEDF 8CE0 02E3"            /* .‚..FÚ†ÿ.þîßŒà.ã */
	$"EA70 8300 0918 3A00 50A5 A2AC ACA5 9D90"            /* êpƒ.Æ.:.P¥¢¬¬¥ */
	$"9909 958A 7D75 7271 773C 3FFB 8BFF 01F2"            /* ™Æ•Š}urqw<?û‹ÿ.ò */
	$"1D83 0001 11E8 84FF 01F0 F782 FF09 E806"            /* .ƒ...è„ÿ.ð÷‚ÿÆè. */
	$"0235 505E 5A50 2609 8100 0116 9F87 FF01"            /* .5P^ZP&Æ...Ÿ‡ÿ. */
	$"F7F6 8AE0 02E1 E978 8300 0A03 AFA8 0058"            /* ÷öŠà.áéxƒ.Â.¯¨.X */
	$"A6A2 ABAB A29B 8F99 0A98 948A 7C75 7271"            /* ¦¢««¢›™Â˜”Š|urq */
	$"7272 1D78 8BFF 01D6 0683 0001 06D2 8BFF"            /* rr.x‹ÿ.Ö.ƒ...Ò‹ÿ */
	$"0CDF 4425 0700 000F 1F2E 3529 1804 8000"            /* .ßD%......5)..€. */
	$"0169 EF87 FF00 E185 E005 DFE0 E0E2 F793"            /* .iï‡ÿ.á…à.ßààâ÷“ */
	$"8400 0A21 A075 005E A6A2 ABA9 A29B 9099"            /* „.Â! u.^¦¢«©¢›™ */
	$"0B94 887B 7572 7171 7758 0253 F689 FF00"            /* .”ˆ{urqqwX.Sö‰ÿ. */
	$"B884 0001 07D4 8CFF 11FE F6DC B581 3704"            /* ¸„...ÔŒÿ.þöÜµ7. */
	$"0000 0113 1010 0400 002D C286 FF0E E8EC"            /* .........-Â†ÿ.èì */
	$"E5E5 E2E0 E0E1 E3E5 E6E9 F59E 1384 000A"            /* ååâààáãåæéõž.„.Â */
	$"4072 4C00 62A5 A2AA A8A1 9B90 990D 9488"            /* @rL.b¥¢ª¨¡›™.”ˆ */
	$"7B74 7271 7176 5408 0040 B9FB 83FF 00FA"            /* {trqqvT..@¹ûƒÿ.ú */
	$"80FF 009A 8400 0110 E591 FF0D FBDA AB78"            /* €ÿ.š„...å‘ÿ.ûÚ«x */
	$"4824 0700 030F 0000 17AC 85FF 0D57 9CCA"            /* H$.......¬…ÿ.WœÊ */
	$"CAE6 F3ED EBEA EAEB F797 0284 000B 0B41"            /* Êæóíëêêë÷—.„...A */
	$"4530 0068 A5A2 A8A7 A09A 9099 0393 887B"            /* E0.h¥¢¨§ š™.“ˆ{ */
	$"7480 710D 7652 0004 2317 346F A3CB DDE0"            /* t€q.vR..#.4o£ËÝà */
	$"9FE0 80FF 0091 8400 012B F896 FF0A F6D2"            /* Ÿà€ÿ.‘„..+ø–ÿÂöÒ */
	$"9A32 0310 0000 0175 EE83 FF0C 0000 110D"            /* š2.....uîƒÿ..... */
	$"3F86 DBEF F8F2 FAAE 0685 000B 0F20 1E17"            /* ?†Ûïøòú®.…... .. */
	$"006E A4A2 A8A6 A09A 9099 0393 877A 7480"            /* .n¤¢¨¦ š™.“‡zt€ */
	$"710D 7751 0000 3044 3106 0003 0A0C 68FD"            /* q.wQ..0D1...Â.hý */
	$"80FF 01DD 0A83 0000 5E9A FF03 EC2E 1226"            /* €ÿ.ÝÂƒ..^šÿ.ì..& */
	$"8000 0126 CE82 FF83 0005 1748 84C4 C521"            /* €..&Î‚ÿƒ...H„ÄÅ! */
	$"8500 0C22 6261 6638 0074 A3A1 A7A5 9F9A"            /* ….."baf8.t£¡§¥Ÿš */
	$"9099 0393 877A 7480 710C 7751 0003 0A03"            /* ™.“‡zt€q.wQ..Â. */
	$"1B20 1D16 0024 EE82 FF00 5A82 0001 23C9"            /* . ...$î‚ÿ.Z‚..#É */
	$"9BFF 03C5 1145 1D80 0005 1DB6 FFF5 C89C"            /* ›ÿ.Å.E.€...¶ÿõÈœ */
	$"8600 0107 0985 000D 0294 DCD2 E070 007A"            /* †...Æ…...”ÜÒàp.z */
	$"A1A1 A6A3 9E9A 9099 0393 877A 7480 7104"            /* ¡¡¦£žš™.“‡zt€q. */
	$"7653 0005 1280 0004 0111 0F0E DE82 FF06"            /* vS...€......Þ‚ÿ. */
	$"DA20 0002 45A3 EC8D FF13 FAD1 FEFF FFFD"            /* Ú ..E£ìÿ.úÑþÿÿý */
	$"EACC A5B5 F6FF FFFD 9435 031D 6317 8000"            /* êÌ¥µöÿÿý”5..c.€. */
	$"0306 4D20 0491 000D 2DB0 ADAC B85B 007C"            /* ..M .‘..-°­¬¸[.| */
	$"A1A1 A4A1 9D9A 9099 0393 877A 7480 7104"            /* ¡¡¤¡š™.“‡zt€q. */
	$"7654 0005 1482 0003 0C06 48D3 82FF 02D7"            /* vT...‚....HÓ‚ÿ.× */
	$"89BC 90FF 14F9 3B42 6159 3B18 0500 0032"            /* ‰¼ÿ.ù;BaY;....2 */
	$"B3FF DE02 0114 042E 753E 9700 0C5E 9E94"            /* ³ÿÞ.....u>—..^ž” */
	$"959F 4E00 7DA1 A0A3 A09C 9199 0393 877A"            /* •ŸN.}¡ £ œ‘™.“‡z */
	$"7480 7104 7656 0004 1082 0004 0111 0014"            /* t€q.vV...‚...... */
	$"C897 FF01 FB2F 8600 0B0D C381 0009 6D44"            /* È—ÿ.û/†...Ã.ÆmD */
	$"002B 9A3B 0194 000D 1581 8583 838C 4500"            /* .+š;.”...…ƒƒŒE. */
	$"7EA0 9EA1 9F9C 9199 0393 887A 7480 7104"            /* ~ ž¡Ÿœ‘™.“ˆzt€q. */
	$"765A 0101 0784 0002 1100 8C98 FF00 6787"            /* vZ...„....Œ˜ÿ.g‡ */
	$"0000 0380 0006 0A9E A90C 3C52 0E94 000D"            /* ...€..Âž©.<R.”.. */
	$"4A7E 7677 777F 3E00 80A0 9E9F 9E9A 9199"            /* J~vww.>.€ žŸžš‘™ */
	$"0394 887B 7480 7104 755E 0300 0483 0003"            /* .”ˆ{t€q.u^...ƒ.. */
	$"0A2C 18DA 98FF 03EC A266 2285 0009 1616"            /* Â,.Ú˜ÿ.ì¢f"….Æ.. */
	$"0222 C0FF 5F00 0606 9300 020F 686C 806A"            /* ."Àÿ_...“...hl€j */
	$"0471 3600 819F 809D 009A 9199 0394 887B"            /* .q6.Ÿ€.š‘™.”ˆ{ */
	$"7480 7104 7463 0402 1183 0002 4116 7C9C"            /* t€q.tc...ƒ..A.|œ */
	$"FF02 ED8F 1582 0007 47E9 EBC3 E5FF FF8A"            /* ÿ.í.‚..GéëÃåÿÿŠ */
	$"9500 0301 3B69 6580 6408 6B34 0081 9E9B"            /* •...;ie€d.k4.ž› */
	$"9C9B 9A91 9903 9488 7B74 8071 0574 6504"            /* œ›š‘™.”ˆ{t€q.te. */
	$"011C 0982 0002 3002 A79E FF02 DA86 2D80"            /* ..Æ‚..0.§žÿ.Ú†-€ */
	$"0000 7483 FF00 9F95 0002 1F69 6781 6504"            /* ..tƒÿ.Ÿ•...ige. */
	$"6C34 0182 9D80 9A92 9903 9488 7B74 8071"            /* l4.‚€š’™.”ˆ{t€q */
	$"0D74 6607 001A 1904 0000 0219 1C00 80A0"            /* .tf...........€  */
	$"FF04 F4B0 6754 CE83 FF00 B795 000B 1861"            /* ÿ.ô°gTÎƒÿ.·•...a */
	$"6A65 6465 656B 3401 829D 9599 0394 887B"            /* jedeek4.‚•™.”ˆ{ */
	$"7480 710E 7468 0800 86BC B94B 0035 7D74"            /* t€q.th..†¼¹K.5}t */
	$"0F20 F3AA FF02 8300 1580 1601 170C 8F00"            /* . óªÿ.ƒ..€..... */
	$"0A0D 4D74 6F6D 6D74 3800 819D 9499 0498"            /* Â.Mtommt8.”™.˜ */
	$"9489 7B74 8071 0E73 6D0C 00A2 E7E5 B625"            /* ”‰{t€q.sm..¢çå¶% */
	$"8198 9A44 00AC A9FF 09E3 123B 9F99 9A9B"            /* ˜šD.¬©ÿÆã.;Ÿ™š› */
	$"9D8E 108F 0009 023B 777D 777F 3E00 819E"            /* Ž..Æ.;w}w.>.ž */
	$"9499 0498 9489 7B74 8071 0F72 7212 0075"            /* ”™.˜”‰{t€q.rr..u */
	$"A6A1 835C A2B2 B595 0434 F9A8 FF09 5C03"            /* ¦¡ƒ\¢²µ•.4ù¨ÿÆ\. */
	$"91AE AEAD A8A8 AE43 9100 071E 668E 8F46"            /* ‘®®­¨¨®C‘...fŽF */
	$"007F 9E94 9904 9894 897B 7480 710F 7273"            /* ..ž”™.˜”‰{t€q.rs */
	$"1600 4D6D A245 46B4 77B7 E46B 008A A7FF"            /* ..Mm¢EF´w·äk.Š§ÿ */
	$"0A89 005A C2BC A255 3434 3728 9200 0606"            /* Â‰.ZÂ¼¢U447(’... */
	$"75B3 5400 81A2 819D 019E A080 A18B A204"            /* u³T.¢.ž €¡‹¢. */
	$"A19D 9184 7D81 7A0F 7B1E 002E 66C2 4A47"            /* ¡‘„}z.{...fÂJG */
	$"B539 64D4 D035 0EE3 A6FF 0554 0060 7B70"            /* µ9dÔÐ5.ã¦ÿ.T.`{p */
	$"1598 0012 057A 6700 6F8D 8888 8786 7B70"            /* .˜...zg.oˆˆ‡†{p */
	$"6963 5D55 514B 4B85 451B 484B 4B4A 4C47"            /* ic]UQKK…E.HKKJLG */
	$"484A 4F52 555D 1E00 2770 BB48 4ABB 436E"            /* HJORU]..'p»HJ»Cn */
	$"8A79 A815 3DDE 8AFF 00F3 96FF 05F5 8300"            /* Šy¨.=ÞŠÿ.ó–ÿ.õƒ. */
	$"1512 049A 0004 051D 0006 0880 0701 0603"            /* ...š.......€.... */
	$"9800 1101 0100 0816 240F 1E48 1F40 4D2F"            /* ˜.......$..H.@M/ */
	$"834D 0921 C188 FF05 DD2E 4576 B4FA 80FF"            /* ƒMÆ!Áˆÿ.Ý.Ev´ú€ÿ */
	$"02F9 CCE9 89FF 07CF A275 2908 0927 05CC"            /* .ùÌé‰ÿ.Ï¢u).Æ'.Ì */
	$"0006 0201 0915 030A 9486 FF01 DC2C 8100"            /* ....Æ..Â”†ÿ.Ü,. */
	$"0630 6CED DA3B 0443 85FF 04FE 9E4B 4B0E"            /* .0líÚ;.C…ÿ.þžKK. */
	$"8000 020A 0702 9400 0308 1A10 0080 01B3"            /* €..Â..”......€.³ */
	$"0021 0805 0048 92D3 F1F1 E5CB A977 1B00"            /* .!...H’ÓññåË©w.. */
	$"0B0B 0906 0000 2215 0004 0870 8B97 9DA0"            /* ..Æ..."....p‹—  */
	$"9894 833F 8000 0408 0F0A 0701 9600 061C"            /* ˜”ƒ?€....Â..–... */
	$"5E3A 0004 0302 B400 1705 1507 0004 1517"            /* ^:....´......... */
	$"0B01 0000 150E 0300 0103 0206 0806 0412"            /* ................ */
	$"1780 0002 0101 0380 0005 192C 2F41 2208"            /* .€.....€...,/A". */
	$"9800 061C 5E3B 0004 0302 B500 0B24 5344"            /* ˜...^;....µ..$SD */
	$"271A 1626 1C15 395B 1083 0013 130D 0021"            /* '..&..9[.ƒ.....! */
	$"5236 4B52 3B3D 3522 0000 2F43 3643 2008"            /* R6KR;=5"../C6C . */
	$"8900 FFFF FBFF FFFF FFFF 06FF FFF8 7112"            /* ‰.ÿÿûÿÿÿÿÿ.ÿÿøq. */
	$"0B0F 820E 010F 0F81 0E03 0F0F 0E0D 8A0F"            /* ..‚..........Š. */
	$"0103 7088 FF02 DA2A 0A81 0F00 0E81 0F00"            /* ..pˆÿ.Ú*Â..... */
	$"0E88 0F02 0E04 9687 FF02 C021 0B80 0E80"            /* .ˆ....–‡ÿ.À!.€.€ */
	$"0F04 0E0E 0F0F 108B 0F02 0146 F88D FF03"            /* .......‹...Føÿ. */
	$"CC5D 5458 8059 005A 8259 8058 025A 2C0B"            /* Ì]TX€Y.Z‚Y€X.Z,. */
	$"890F 0102 9989 FF0E DC58 5053 5558 5C59"            /* ‰...™‰ÿ.ÜXPSUX\Y */
	$"5656 585C 5C2A 0A85 0F02 0C1A E288 FF02"            /* VVX\\*Â…....âˆÿ. */
	$"AE4C 5581 5606 5757 5657 5B37 0C89 0F02"            /* ®LUV.WWVW[7.‰.. */
	$"0830 DB8F FF07 CB73 7A7B 7B7C 7B7B 807C"            /* .0Ûÿ.Ësz{{|{{€| */
	$"077B 7B7A 7A7D 6211 0D88 0F02 0552 FD89"            /* .{{zz}b..ˆ...Rý‰ */
	$"FF0F 8E6F 7866 5041 342A 2A32 4153 3114"            /* ÿ.ŽoxfPA4**2AS1. */
	$"0F0E 830F 020B 1DF2 88FF 03A9 6E76 7681"            /* ..ƒ....òˆÿ.©nvv */
	$"7705 7878 797A 300B 880F 020E 0ABD 90FF"            /* w.xxyz0.ˆ...Â½ÿ */
	$"07FE A870 7778 7777 7980 7880 7704 7677"            /* .þ¨pwxwwy€x€w.vw */
	$"7830 0A88 0F02 0C18 E789 FF03 A15A 3608"            /* x0Âˆ....ç‰ÿ.¡Z6. */
	$"8500 0504 1A1F 1711 0E81 0F02 092D F487"            /* …..........Æ-ô‡ */
	$"FF06 FC8C 6C71 7272 7380 7404 7376 6D17"            /* ÿ.üŒlqrrs€t.svm. */
	$"0D88 0F02 0637 FB91 FF02 F891 7484 7A00"            /* .ˆ...7û‘ÿ.ø‘t„z. */
	$"7980 7804 777B 5F11 0D87 0F02 0E07 9B88"            /* y€x.w{_..‡....›ˆ */
	$"FF02 AD33 0489 0009 040C 1919 0E0E 0F0F"            /* ÿ.­3.‰.Æ........ */
	$"0452 88FF 04E4 7670 7272 8073 0574 7575"            /* .Rˆÿ.ävprr€s.tuu */
	$"7A55 0D88 0F02 0E01 A493 FF02 E07E 7A80"            /* zU.ˆ....¤“ÿ.à~z€ */
	$"7C05 7D7C 7C7B 7A79 8078 027B 3D0B 880F"            /* |.}||{zy€x.{=.ˆ. */
	$"0209 20F2 85FF 01D6 618F 0006 0F1B 150C"            /* .Æ ò…ÿ.Öa...... */
	$"0B00 5088 FF04 B86A 7172 7280 7305 7475"            /* ..Pˆÿ.¸jqrr€s.tu */
	$"7579 350A 880F 0206 46F9 94FF 06B9 767E"            /* uy5Âˆ...Fù”ÿ.¹v~ */
	$"7F7D 7E7D 807C 077B 7A79 797A 7425 0B87"            /* .}~}€|.{zyyzt%.‡ */
	$"0F02 0E06 A384 FF01 BF18 8000 0301 0202"            /* ....£„ÿ.¿.€..... */
	$"018A 0005 0415 1D1E 135C 88FF 0198 6B81"            /* .Š.......\ˆÿ.˜k */
	$"7207 7374 7575 7770 1C0D 870F 020E 05B0"            /* r.stuuwp..‡....° */
	$"95FF 11F4 927A 8080 7F7F 7E7E 7D7C 7B7A"            /* •ÿ.ô’z€€..~~}|{z */
	$"7978 7D51 0B88 0F01 0543 83FF 01C7 1980"            /* yx}Q.ˆ...Cƒÿ.Ç.€ */
	$"0005 0205 0807 0401 8A00 0501 71F0 E5EA"            /* ........Š...qðåê */
	$"FE86 FF0E F884 6E72 7273 7273 7475 757A"            /* þ†ÿ.ø„nrrsrstuuz */
	$"5D0F 0E87 0F02 0532 EC96 FF11 E88F 7E82"            /* ]..‡...2ì–ÿ.è~‚ */
	$"8281 807F 7E7E 7C7A 7A79 7A78 290B 870F"            /* ‚€.~~|zzyzx).‡. */
	$"080C 18DB F9F8 F8FC C517 8000 0603 0B11"            /* ...ÛùøøüÅ.€..... */
	$"130C 0401 8B00 0411 B5E6 D9F0 86FF 07E5"            /* ....‹...µæÙð†ÿ.å */
	$"7570 7272 7373 7480 7502 7937 0A87 0F02"            /* uprrsst€u.y7Â‡.. */
	$"0D07 A798 FF00 DA81 830C 8181 807F 7D7C"            /* ..§˜ÿ.Úƒ.€.}| */
	$"7C7B 7A7D 5D0F 0E86 0F07 0D10 88B0 ABB2"            /* |{z}]..†....ˆ°«² */
	$"7D0C 8000 0703 0D19 1F1A 0E03 018C 0003"            /* }.€..........Œ.. */
	$"44DE D6E7 86FF 02CA 6E72 8073 0774 7475"            /* DÞÖç†ÿ.Ênr€s.ttu */
	$"7579 6513 0D87 0F01 026A 9AFF 10AC 7E85"            /* uye..‡...jšÿ.¬~… */
	$"8483 8281 7F7E 7E7D 7B7A 7B7B 2C09 860F"            /* „ƒ‚.~~}{z{{,Æ†. */
	$"060C 19A4 B1B4 A31D 8000 0702 0A1A 2727"            /* ...¤±´£.€...Â.'' */
	$"190A 028D 0003 04A5 DDE1 86FF 01B1 6C84"            /* .Â....¥Ýá†ÿ.±l„ */
	$"7403 757C 4C0A 870F 020C 10CD 9AFF 11EE"            /* t.u|LÂ‡....Íšÿ.î */
	$"9182 8584 8382 817F 7F7E 7D7C 7B7E 5E10"            /* ‘‚…„ƒ‚..~}|{~^. */
	$"0E85 0F05 0936 C0C7 CC47 8000 0702 0A19"            /* .…..Æ6ÀÇÌG€...Â. */
	$"2B30 2512 058F 0003 3BC1 C7FC 84FF 06FE"            /* +0%....;ÁÇü„ÿ.þ */
	$"976F 7475 7474 8075 0376 7830 0A87 0F01"            /* —otutt€u.vx0Â‡.. */
	$"0167 9CFF 00D6 8085 0C84 8482 817F 7E7E"            /* .gœÿ.Ö€….„„‚.~~ */
	$"7C7C 7B7F 480C 850F 0402 6DE9 E87F 8000"            /* ||{.H.…...méè.€. */
	$"0801 091A 2D39 2D1A 0801 8F00 0307 AACE"            /* ..Æ.-9-......ªÎ */
	$"FC84 FF02 F583 7383 7504 7679 6B16 0D86"            /* ü„ÿ.õƒsƒu.vyk..† */
	$"0F02 0D0E C29D FF10 BA81 8685 8482 8281"            /* ....Âÿ.º†…„‚‚ */
	$"807E 7E7D 7C7E 741B 0C84 0F04 007C F9DC"            /* €~~}|~t..„...|ùÜ */
	$"2380 0007 0414 2B3C 3B24 0F03 9100 0276"            /* #€....+<;$..‘..v */
	$"B9FA 84FF 02E4 7975 8476 027B 550C 870F"            /* ¹ú„ÿ.äyu„v.{U.‡. */
	$"0102 5C8A FF02 FEC1 BB8E FF10 FAA2 8186"            /* ..\Šÿ.þÁ»Žÿ.ú¢† */
	$"8584 8381 817F 7E7E 7D7C 814C 0A84 0F0E"            /* …„ƒ.~~}|LÂ„.. */
	$"045B B945 0000 0102 0D26 3E44 3316 0692"            /* .[¹E.....&>D3..’ */
	$"0002 4FAF FA84 FF02 CA72 7684 7702 7B38"            /* ..O¯ú„ÿ.Êrv„w.{8 */
	$"0986 0F02 0C0B BD8A FF02 BF0D 0C8F FF10"            /* Æ†....½Šÿ.¿..ÿ. */
	$"E98D 8485 8583 8281 807F 7F7E 7D7E 7925"            /* é„……ƒ‚€..~}~y% */
	$"0B83 0F02 0277 9380 0008 0208 1E3C 4A40"            /* .ƒ...w“€.....<J@ */
	$"240A 0192 0002 35AE F784 FF02 AF70 7883"            /* $Â.’..5®÷„ÿ.¯pxƒ */
	$"7703 7970 1C0C 860F 0206 79FE 8AFF 0552"            /* w.yp..†...yþŠÿ.R */
	$"020D 877F C28D FF09 C680 8685 8483 8281"            /* ..‡.ÂÿÆÆ€†…„ƒ‚ */
	$"807F 807E 0283 580C 820F 030E 0B8F 4480"            /* €.€~.ƒX.‚....D€ */
	$"0007 0613 314A 4A33 1404 9300 021C 6FEF"            /* ....1JJ3..“...oï */
	$"83FF 02FE 9774 8178 8077 037C 5A0E 0E85"            /* ƒÿ.þ—tx€w.|Z..… */
	$"0F02 0B1A E78A FF07 BE0C 0D0F 0000 1AAF"            /* ....çŠÿ.¾......¯ */
	$"8CFF 10F6 9D81 8684 8382 8281 8080 7F7E"            /* Œÿ.ö†„ƒ‚‚€€.~ */
	$"807A 240A 810F 0E0E 1562 0A00 0002 0E24"            /* €z$Â....bÂ....$ */
	$"4250 4024 0A01 9300 0212 B0FA 83FF 0CF4"            /* BP@$Â.“...°úƒÿ.ô */
	$"8677 7A79 7879 7978 787E 4109 860F 0102"            /* †wzyxyyxx~AÆ†... */
	$"818B FF01 7100 800F 0310 0808 A08C FF06"            /* ‹ÿ.q.€..... Œÿ. */
	$"ED97 8185 8483 8280 8106 807F 7F83 6615"            /* í—…„ƒ‚€.€..ƒf. */
	$"0D80 0F02 0E17 1F80 0007 071D 3A52 4C2F"            /* .€.....€....:RL/ */
	$"1504 9400 020E A9F4 83FF 03DE 7B79 7B80"            /* ..”...©ôƒÿ.Þ{y{€ */
	$"7A05 797A 7B77 230C 850F 020A 1EDE 8AFF"            /* z.yz{w#.…..Â.ÞŠÿ */
	$"02DB 4107 820F 020A 0AA0 8CFF 04E0 8683"            /* .ÛA.‚..ÂÂ Œÿ.à†ƒ */
	$"8483 8082 0781 8080 7F7F 8649 0A81 0F0B"            /* „ƒ€‚.€€..†IÂ.. */
	$"2007 0000 0312 314D 563E 1E0A 9500 0207"            /*  .....1MV>.Â•... */
	$"6EF3 83FF 01C3 7584 7B02 8053 0D86 0F01"            /* nóƒÿ.Ãu„{.€S.†.. */
	$"0479 8AFF 03EE 893B 0883 0F02 0815 E08C"            /* .yŠÿ.î‰;.ƒ....àŒ */
	$"FF13 B481 8685 8584 8382 8180 807F 8274"            /* ÿ.´†……„ƒ‚€€.‚t */
	$"190C 0F0E 1516 8000 0707 1E41 5751 3012"            /* ......€....AWQ0. */
	$"0495 0002 0562 F882 FF0C FEA1 777D 7D7C"            /* .•...bø‚ÿ.þ¡w}}| */
	$"7C7B 7B7C 7D30 0985 0F02 0B13 DB8A FF03"            /* |{{|}0Æ…....ÛŠÿ. */
	$"A971 4108 840F 020A 2DA3 8BFF 09FE 967C"            /* ©qA.„..Â-£‹ÿÆþ–| */
	$"7C77 7A7E 8489 8480 8011 8548 090F 0E1A"            /* |wz~„‰„€€.…HÆ... */
	$"0800 0004 1131 525A 4420 0801 9500 0206"            /* .....1RZD ..•... */
	$"63F7 82FF 02EC 847B 817D 057C 7C7F 7117"            /* c÷‚ÿ.ì„{}.||.q. */
	$"0D85 0F01 0070 8AFF 04C9 747A 4108 850F"            /* .…...pŠÿ.ÉtzA.…. */
	$"0205 09A1 88FF 1FF5 B975 330F 0807 0810"            /* ..Æ¡ˆÿ.õ¹u3..... */
	$"2A4E 7687 8180 8279 200B 1115 0100 010A"            /* *Nv‡€‚y ......Â */
	$"2045 5B53 3311 0396 0002 0A86 FA82 FF02"            /*  E[S3..–..Â†ú‚ÿ. */
	$"D37A 7D80 7E80 7D02 8356 0B85 0F02 0642"            /* Óz}€~€}.ƒV.…...B */
	$"F689 FF05 D176 757D 4108 860F 0209 0ECD"            /* ö‰ÿ.Ñvu}A.†..Æ.Í */
	$"86FF 01BD 3D87 0013 105B 8782 8087 4D07"            /* †ÿ.½=‡...[‡‚€‡M. */
	$"130D 0000 0413 2F53 5C48 2409 9700 0211"            /* ....../S\H$Æ—... */
	$"C1FE 82FF 02C0 787F 817E 047D 7D82 3A09"            /* Áþ‚ÿ.Àx.~.}}‚:Æ */
	$"850F 0100 9989 FF06 F58B 7478 7E42 0887"            /* …...™‰ÿ.õ‹tx~B.‡ */
	$"0F02 053B D783 FF05 DA5A 0400 0302 8600"            /* ...;×ƒÿ.ÚZ....†. */
	$"1202 4887 8383 7C2A 1204 0000 0921 415C"            /* ..H‡ƒƒ|*....Æ!A\ */
	$"5539 1703 9700 010E AA83 FF0B AA79 8080"            /* U9..—...ªƒÿ.ªy€€ */
	$"7E7F 7E7E 8078 1E0C 840F 0206 31EA 89FF"            /* ~.~~€x..„...1ê‰ÿ */
	$"06AC 757B 797E 4308 870F 0E10 032D D3DD"            /* .¬u{y~C.‡....-ÓÝ */
	$"DBE1 D979 1300 0C10 0C04 8800 0F4C 8782"            /* ÛáÙy......ˆ..L‡‚ */
	$"8671 1700 0003 1434 525E 4727 0C98 0001"            /* †q.....4R^G'.˜.. */
	$"1C93 82FF 02FA 917C 8080 807F 0384 5F0E"            /* .“‚ÿ.ú‘|€€€..„_. */
	$"0E83 0F02 0E0A B189 FF07 CA78 7B7B 7A80"            /* .ƒ...Â±‰ÿ.Êx{{z€ */
	$"4308 880F 0D0C 16BC CFCB D85E 0004 161E"            /* C.ˆ....¼ÏËØ^.... */
	$"190F 0488 000F 0162 8783 8217 0000 0720"            /* ...ˆ...b‡ƒ‚....  */
	$"455D 5937 1805 9800 013E E782 FF02 E783"            /* E]Y7..˜..>ç‚ÿ.çƒ */
	$"7E80 8080 7F02 8440 0984 0F01 0541 89FF"            /* ~€€€..„@Æ„...A‰ÿ */
	$"08DB 817B 7E7B 7A81 4308 880F 0D0D 0F9F"            /* .Û{~{zC.ˆ....Ÿ */
	$"E0C3 8403 0717 2628 1B0D 0389 000E 3688"            /* àÃ„...&(...‰..6ˆ */
	$"8869 0300 020E 2E53 604F 290E 0398 0001"            /* ˆi.....S`O)..˜.. */
	$"30C3 82FF 01D3 7D84 8002 7D26 0B84 0F01"            /* 0Ã‚ÿ.Ó}„€.}&.„.. */
	$"0089 88FF 09F1 8F79 7E7D 7D7B 8144 0888"            /* .‰ˆÿÆñy~}}{D.ˆ */
	$"0F0C 0D0E 94DA 7B04 0318 2C36 2B15 078A"            /* ....”Ú{...,6+..Š */
	$"000D 127D 8932 0000 0619 405E 5E3F 1906"            /* ...}‰2....@^^?.. */
	$"9900 0163 EE82 FF03 BC7A 8385 8084 0482"            /* ™..cî‚ÿ.¼zƒ…€„.‚ */
	$"8569 110E 830F 0207 33E8 87FF 0AFC A478"            /* …i..ƒ...3è‡ÿÂü¤x */
	$"7F7E 7D7E 7C82 4408 880F 0C0E 0580 A907"            /* .~}~|‚D.ˆ....€©. */
	$"0011 2B3D 3B23 0C03 8A00 0D03 7369 0100"            /* ..+=;#..Š...si.. */
	$"020E 2850 6254 2F0F 0299 0001 78EC 82FF"            /* ..(PbT/..™..xì‚ÿ */
	$"02AA 7F79 816D 037E 8A43 0883 0F02 0C13"            /* .ª.ym.~ŠC.ƒ.... */
	$"CA88 FF0A C077 7E7E 7F7E 7D7D 8345 0889"            /* ÊˆÿÂÀw~~.~}}ƒE.‰ */
	$"0F0A 0466 5500 071E 3B45 3619 058C 000B"            /* .Â.fU...;E6..Œ.. */
	$"6648 0000 0516 3659 6049 2207 9900 0204"            /* fH....6Y`I".™... */
	$"74DE 81FF 13D5 5D2A 1205 0404 061C 3D21"            /* tÞÿ.Õ]*......=! */
	$"1311 0E0F 0E0D 0E00 5988 FF02 EB83 7B82"            /* ........Yˆÿ.ëƒ{‚ */
	$"7E03 7D83 4507 890F 0A0C 2B0A 0211 2F46"            /* ~.}ƒE.‰.Â.+Â../F */
	$"432A 0D01 8B00 0C01 5B1F 0000 0A24 455F"            /* C*..‹...[...Â$E_ */
	$"593A 1504 9900 0714 97F2 FFFF BF47 0A86"            /* Y:..™...—òÿÿ¿GÂ† */
	$"0009 0108 1118 130F 0E0D 0BB1 87FF 02EC"            /* .Æ.........±‡ÿ.ì */
	$"9279 837E 037D 8446 0789 0F09 1210 000B"            /* ’yƒ~.}„F.‰.Æ.... */
	$"2140 4A39 1B06 8C00 0B09 4804 0002 1434"            /* !@J9..Œ..ÆH....4 */
	$"5460 4B29 0C9A 0005 38B2 FFF5 7B05 8B00"            /* T`K).š..8²ÿõ{.‹. */
	$"0505 1252 9981 AE87 FF04 F99B 757E 7D81"            /* ...R™®‡ÿ.ù›u~} */
	$"7E04 7D7E 843F 0888 0F0A 0E16 0803 1530"            /* ~.}~„?.ˆ.Â.....0 */
	$"4A45 2B10 018C 000B 0A1E 0000 061E 435D"            /* JE+..Œ..Â.....C] */
	$"5C3C 1C06 9A00 0356 BDD3 428F 0004 22BC"            /* \<..š..V½ÓB.."¼ */
	$"BDEB FC86 FF02 BC74 7C82 7D05 7E7D 8363"            /* ½ëü†ÿ.¼t|‚}.~}ƒc */
	$"130D 820F 010D 0B82 0F08 1803 0920 3D4C"            /* ..‚....‚....Æ =L */
	$"3B1F 098D 000B 0404 0000 0A29 4F61 542F"            /* ;.Æ......Â)OaT/ */
	$"1302 9900 090A 8FD6 6400 0004 0604 018B"            /* ..™.ÆÂÖd......‹ */
	$"0003 32A8 E6F6 85FF 04E7 827A 7D7C 827D"            /* ..2¨æö…ÿ.ç‚z}|‚} */
	$"0382 7220 0B82 0F10 0D22 260A 0F0F 0E17"            /* .‚r .‚..."&Â.... */
	$"1901 1330 4749 2D12 0390 0007 0313 3859"            /* ...0GI-......8Y */
	$"6046 200A 9A00 092D C3A4 0401 0A13 120B"            /* `F Âš.Æ-Ã¤..Â... */
	$"048C 0002 35CC F384 FF03 F094 777C 807D"            /* .Œ..5Ìó„ÿ.ð”w|€} */
	$"067C 7C7D 8076 2709 830F 0F09 3787 3106"            /* .||}€v'Æƒ..Æ7‡1. */
	$"0D0E 1B06 051E 3D4C 401F 0991 0007 051D"            /* ......=L@.Æ‘.... */
	$"455F 5B38 1404 9A00 0956 D147 000B 1A25"            /* E_[8..š.ÆVÑG...% */
	$"1F11 048D 0001 83FB 83FF 03FC 9A76 7D83"            /* .....ƒûƒÿ.üšv}ƒ */
	$"7C03 8073 260A 840F 0F09 3394 8640 100D"            /* |.€s&Â„..Æ3”†@.. */
	$"1901 0B29 464C 3514 0491 0007 0A27 4F61"            /* ...)FL5..‘..Â'Oa */
	$"522B 0D02 9A00 0974 B812 0217 2A33 2310"            /* R+..š.Æt¸...*3#. */
	$"048D 0000 4D84 FF03 C378 7C7D 827C 037F"            /* ...M„ÿ.Ãx|}‚|.. */
	$"7C30 0885 0F0F 0A33 8D91 9368 2510 0114"            /* |0.…..Â3‘“h%... */
	$"364C 4627 0B01 9000 0704 1235 585E 451E"            /* 6LF'......5X^E. */
	$"079A 000A 10B2 7A00 0C26 3B3A 200A 018D"            /* .š.Â.²z..&;: Â. */
	$"0001 18EE 82FF 05DF 807C 7E7D 7D80 7C03"            /* ...î‚ÿ.ß€|~}}€|. */
	$"7E7E 3809 860F 0E0A 328C 8D8D 9474 0705"            /* ~~8Æ†..Â2Œ”t.. */
	$"1F40 4C3B 1A05 9100 0708 1C42 5D56 3613"            /* .@L;..‘....B]V6. */
	$"029A 0009 42AB 4200 1634 4437 1704 8E00"            /* .š.ÆB«B..4D7..Ž. */
	$"0102 BC81 FF0C F391 797E 7E7D 7D7C 7C7D"            /* ..¼ÿ.ó‘y~~}}||} */
	$"7E3F 0A87 0F0E 0A32 8B8C 8A91 6501 0A28"            /* ~?Â‡..Â2‹ŒŠ‘e.Â( */
	$"4749 3211 0291 0006 0D26 4C5E 4C2A 0C9B"            /* GI2..‘...&L^L*.› */
	$"0009 63A2 1700 1D3B 4732 1101 8F00 008A"            /* .Æc¢...;G2....Š */
	$"80FF 02FD AC78 807E 077D 7D7C 7D80 430B"            /* €ÿ.ý¬x€~.}}|}€C. */
	$"0E87 0F0D 0A31 8A8A 8991 4100 1333 4B42"            /* .‡..Â1ŠŠ‰‘A..3KB */
	$"270B 9100 0702 1432 545A 3F1F 079A 0009"            /* '.‘....2TZ?..š.Æ */
	$"016B C127 0021 4048 2E0D 9000 0058 80FF"            /* .kÁ'.!@H....X€ÿ */
	$"05BD 797F 7F7E 7E80 7D03 814A 0C0E 880F"            /* .½y..~~€}.J..ˆ. */
	$"0D0A 3188 898A 881E 031D 3D4B 381C 0591"            /* .Â1ˆ‰Šˆ...=K8..‘ */
	$"0007 061E 3F59 5330 1303 9A00 0912 A3A9"            /* ....?YS0..š.Æ.£© */
	$"1D01 2443 4A2E 0D90 000F 26F4 FFD7 7E7E"            /* ..$CJ....&ôÿ×~~ */
	$"807F 7F7E 7D7E 8253 0D0D 890F 0D0A 3188"            /* €..~}~‚S..‰..Â1ˆ */
	$"888A 7A06 0B25 4348 2F14 0291 0006 0A28"            /* ˆŠz..%CH/..‘..Â( */
	$"495B 4923 0C9B 0009 39BC 9B1A 0226 474C"            /* I[I#.›.Æ9¼›..&GL */
	$"300D 9000 040A DDF8 8E7C 8080 067F 7E7E"            /* 0...ÂÝøŽ|€€..~~ */
	$"8460 100D 8A0F 0C0A 3187 878E 5F01 122D"            /* „`..Š..Â1‡‡Ž_..- */
	$"4844 270E 9100 0701 1032 515A 3F1B 079B"            /* HD'.‘....2QZ?..› */
	$"0009 5DC4 9119 0327 4950 340F 9000 0301"            /* .Æ]Ä‘..'IP4.... */
	$"B9C0 7A80 8006 7F7F 7E84 5F17 0C8B 0F0C"            /* ¹Àz€€...~„_..‹.. */
	$"0A30 8587 8F41 001A 364A 3D1D 0991 0007"            /* Â0…‡A..6J=.Æ‘.. */
	$"0418 3D58 5532 1103 9B00 0A75 A67E 1604"            /* ..=XU2..›.Âu¦~.. */
	$"284D 5639 1201 9000 0B5F 8A80 8180 807F"            /* (MV9...._Š€€€. */
	$"7F83 6615 0B86 0F81 0D0E 0F0F 0A2F 8587"            /* .ƒf..†.....Â/…‡ */
	$"8729 0020 3D4A 3616 0591 0007 0621 475A"            /* ‡). =J6..‘...!GZ */
	$"4D26 0A01 9A00 0B08 8F86 6813 0529 4F5A"            /* M&Â.š...†h..)OZ */
	$"3D14 0290 0003 3987 8181 8080 0384 741B"            /* =....9‡€€.„t. */
	$"0A86 0F01 080D 800E 0E0F 0F09 2B81 8682"            /* Â†....€....Æ+†‚ */
	$"1505 2641 492F 1103 9100 060B 294E 5A45"            /* ..&AI/..‘...)NZE */
	$"1D06 9B00 0B3A D890 520F 052A 515D 4116"            /* ..›..:ØR..*Q]A. */
	$"0290 0009 1F7F 8281 8080 8479 2E0A 850F"            /* ..Æ..‚€€„y.Â…. */
	$"030E 0045 B980 BC80 000B 3D8E 816B 030C"            /* ...E¹€¼€..=Žk.. */
	$"2C45 4528 0B01 9000 0702 1133 5457 3A15"            /* ,EE(......3TW:. */
	$"029B 000B 76C3 5C35 0B06 2A54 6247 1A04"            /* .›..vÃ\5..*TbG.. */
	$"9000 0805 7484 8081 8475 2609 850F 0307"            /* ...t„€„u&Æ…... */
	$"0053 EA81 FF0D 8784 9EDD EFB5 4300 1131"            /* .Sêÿ.‡„žÝïµC..1 */
	$"4640 2107 9100 0605 183E 5851 2F0D 9B00"            /* F@!.‘....>XQ/.›. */
	$"0C03 7795 5F3E 0506 2B56 664B 1D05 9000"            /* ..w•_>..+VfK... */
	$"0701 6286 8084 7B29 0985 0F03 0630 95FD"            /* ..b†€„{)Æ…...0•ý */
	$"88FF 0750 0014 3447 3C1C 0591 0006 081F"            /* ˆÿ.P..4G<..‘.... */
	$"4658 4B27 099B 000C 0B1C 22C3 A203 062B"            /* FXK'Æ›...."Ã¢..+ */
	$"5868 4D1D 0591 0005 5C87 8386 3E08 850F"            /* XhM..‘..\‡ƒ†>.…. */
	$"0202 22D9 89FF 08FE 4200 1636 4637 1704"            /* .."Ù‰ÿ.þB..6F7.. */
	$"9000 0701 0C27 4D56 411E 059B 000C 1210"            /* ....'MVA..›.... */
	$"1278 6401 072D 5867 4C1D 0591 0005 4D8A"            /* .xd..-XgL..‘..MŠ */
	$"8145 0D0E 830F 030D 003D CD8A FF08 F728"            /* E..ƒ....=ÍŠÿ.÷( */
	$"0019 3744 3213 0290 0007 0313 3253 5237"            /* ..7D2......2SR7 */
	$"1502 9A00 0D01 1610 1456 3F00 082D 5764"            /* ..š......V?..-Wd */
	$"461A 0491 0004 438B 3D08 0E83 0F03 0A0D"            /* F..‘..C‹=..ƒ..Â. */
	$"76EB 8BFF 07F2 1800 1A37 412D 1091 0006"            /* vë‹ÿ.ò...7A-.‘.. */
	$"051A 3A56 4C2E 0F9B 000D 0419 092E A963"            /* ..:VL..›....Æ.©c */
	$"0009 2D54 5C3E 1402 9100 0344 5309 0E83"            /* .Æ-T\>..‘..DSÆ.ƒ */
	$"0F02 050B AB8D FF07 ED12 001B 363E 2A0D"            /* ....«ÿ.í...6>*. */
	$"9100 0608 2142 5745 260B 9B00 0C0A 2009"            /* ‘...!BWE&.›..Â Æ */
	$"2A77 3C00 0A2C 4F53 340F 9200 0221 190D"            /* *w<.Â,OS4.’..!.. */
	$"820F 030E 0528 B08E FF07 E80F 001B 343A"            /* ‚....(°Žÿ.è...4: */
	$"260B 9100 060D 2949 563D 1D07 9B00 0C28"            /* &.‘...)IV=..›..( */
	$"2803 4DC9 3300 0C2B 4643 2508 9200 010B"            /* (.MÉ3..+FC%.’... */
	$"1082 0F03 0E03 6EF4 8FFF 07E7 0F00 1932"            /* .‚....nôÿ.ç...2 */
	$"3622 0A90 0007 0111 314F 5334 1504 9B00"            /* 6"Â....1OS4..›. */
	$"0C48 3600 9BDF 0E00 0D27 3A31 1804 9200"            /* .H6.›ß...':1..’. */
	$"010B 1181 0F02 0C00 6389 FF02 E055 5082"            /* .......c‰ÿ.àUP‚ */
	$"FF07 F212 0018 2F32 1F08 9000 0702 1639"            /* ÿ.ò.../2......9 */
	$"534F 2C10 029B 000C 5B3B 67BB 6E00 000E"            /* SO,..›..[;g»n... */
	$"232D 230E 0192 0001 0A11 800F 0305 0E7A"            /* #-#..’..Â.€....z */
	$"FB89 FF02 7400 0482 FF07 D211 0015 292D"            /* û‰ÿ.t..‚ÿ.Ò...)- */
	$"1B08 9000 0605 1D41 5549 240A 9C00 0B13"            /* ......AUI$Âœ... */
	$"1940 3000 0003 0E1B 1D14 0693 0006 0A11"            /* .@0........“..Â. */
	$"0F0F 003E C88A FF10 A206 0D0F E2E0 EDF8"            /* ...>ÈŠÿ.¢...âàíø */
	$"D992 2500 1122 2617 0690 0006 0925 4955"            /* Ù’%.."&....Æ%IU */
	$"411C 07A2 0005 030B 1110 0802 9300 050A"            /* A..¢........“..Â */
	$"100D 0452 EB89 FF12 F6C2 4803 0F0F 190F"            /* ...Rë‰ÿ.öÂH..... */
	$"61BC ADB2 3700 0D1B 1F13 0690 0006 0C2C"            /* a¼­²7........., */
	$"4D55 3B16 04A2 0004 0207 0A07 0394 0004"            /* MU;..¢....Â..”.. */
	$"0D10 044C F989 FF13 E098 7B42 070F 0F0B"            /* ...Lù‰ÿ.à˜{B.... */
	$"0B08 4EAB AC4D 0008 1215 0E04 8F00 0701"            /* ..N«¬M......... */
	$"1033 5252 3311 03A2 0004 0103 0402 0194"            /* .3RR3..¢.......” */
	$"0003 233E 83F9 88FF 06E2 A47D 7883 4507"            /* ..#>ƒùˆÿ.â¤}xƒE. */
	$"810F 0A0D 0D5F BFA0 0A01 080A 0702 8F00"            /* .Â.._¿ Â..Â... */
	$"0603 163B 554E 2B0B BF00 0243 A5F9 87FF"            /* ...;UN+.¿..C¥ù‡ÿ */
	$"08E1 AE81 787C 7D84 4607 820F 090C 0A79"            /* .á®x|}„F.‚.Æ.Ây */
	$"DE35 0002 0302 018F 0006 051C 4156 4923"            /* Þ5.........AVI# */
	$"08BF 0001 32D2 86FF 0AF7 B180 797D 7F7E"            /* .¿..2Ò†ÿÂ÷±€y}.~ */
	$"7E84 4607 830F 030B 1385 6794 0006 0821"            /* ~„F.ƒ....…g”...! */
	$"4656 441E 05BF 0001 65F3 84FF 0CF9 D099"            /* FVD..¿..eó„ÿ.ùÐ™ */
	$"7A7F 8080 7F7F 7E88 4807 840F 030B 125A"            /* z.€€..~ˆH.„....Z */
	$"1292 0007 010B 284C 553D 1803 BF00 00B2"            /* .’....(LU=..¿..² */
	$"83FF 04FE D498 7C7C 8081 0680 8082 857E"            /* ƒÿ.þÔ˜||€.€€‚…~ */
	$"3909 850F 030C 101F 0191 0007 020F 2F50"            /* 9Æ…......‘..../P */
	$"5135 1202 BE00 010B DB82 FF04 D4A1 7E7D"            /* Q5..¾...Û‚ÿ.Ô¡~} */
	$"8081 8105 8385 754D 220E 870F 020D 170A"            /* €.ƒ…uM".‡....Â */
	$"9100 0604 1334 524D 2F0E BF00 013A F680"            /* ‘....4RM/.¿..:ö€ */
	$"FF0F F1B0 7D7C 8182 8182 8387 8676 4C1C"            /* ÿ.ñ°}|‚‚ƒ‡†vL. */
	$"090B 890F 0210 1706 9000 0606 183A 5348"            /* Æ.‰.........:SH */
	$"280A BF00 1281 FFFF EFC1 8F7D 8182 8382"            /* (Â¿..ÿÿïÁ}‚ƒ‚ */
	$"8389 8264 3F1A 0A0C 8C0F 0212 1A09 8F00"            /* ƒ‰‚d?.Â.Œ....Æ. */
	$"0608 1D41 5342 2207 BE00 1104 CDFB C38D"            /* ...ASB".¾...ÍûÃ */
	$"7C80 8382 8386 887B 5527 0D09 0C8F 0F02"            /* |€ƒ‚ƒ†ˆ{U'.Æ... */
	$"1016 098E 0006 0A22 4552 3D1D 05BE 000F"            /* ..ÆŽ..Â"ER=..¾.. */
	$"20D1 A57B 7F83 8384 8788 7B52 1E0A 0A0E"            /*  Ñ¥{.ƒƒ„‡ˆ{R.ÂÂ. */
	$"8A0F 0B0E 090A 0C0D 0E0F 0F10 1815 088C"            /* Š...ÆÂ.........Œ */
	$"0006 0E27 4951 3718 03BE 000C 3F8A 7D82"            /* ...'IQ7..¾..?Š}‚ */
	$"8385 8983 6A46 1E0B 0C8C 0F0F 0C23 4730"            /* ƒ…‰ƒjF...Œ...#G0 */
	$"1D13 0C0A 0C0E 0E13 1915 0B04 8800 0702"            /* ...Â........ˆ... */
	$"122E 4D4E 2F13 02BD 000B 0468 8682 8588"            /* ..MN/..½...h†‚…ˆ */
	$"7D5A 2D10 090C 8E0F 0D0A 2F81 7E77 6B56"            /* }Z-.Æ.Ž..Â/~wkV */
	$"381B 0F09 080A 1080 1701 0701 8500 0602"            /* 8..Æ.Â.€....…... */
	$"1634 4F4A 290F BE00 0921 8687 897A 441D"            /* .4OJ).¾.Æ!†‡‰zD. */
	$"0B0A 0E88 0F1D 0D0B 090C 0D0D 0F0F 0A2D"            /* .Â.ˆ....Æ.....Â- */
	$"7C7E 7F82 8583 7764 4D39 2615 0C08 0F14"            /* |~.‚…ƒwdM9&..... */
	$"1110 0802 8200 0604 1938 5046 240C BE00"            /* ....‚....8PF$.¾. */
	$"075A 8B6F 4C21 090C 0E86 0F0E 0E0C 0700"            /* .Z‹oL!Æ..†...... */
	$"0C1D 2914 0D0E 0F0F 0A2D 7A80 7D11 7F81"            /* ..).....Â-z€}.. */
	$"8386 8986 8074 5E4B 351E 1311 0E12 0D02"            /* ƒ†‰†€t^K5....... */
	$"8000 0605 1E3D 5142 1F08 BD00 0521 6133"            /* €....=QB..½..!a3 */
	$"1209 0B86 0F30 0C08 0005 123B 87B8 E3F4"            /* .Æ.†.0.....;‡¸ãô */
	$"D6B9 BC0F 0F0A 2D7A 7C7C 7D7F 7F80 8182"            /* Ö¹¼..Â-z||}..€‚ */
	$"8383 8589 8A86 7E71 5E45 2F21 1603 0000"            /* ƒƒ…‰Š†~q^E/!.... */
	$"0824 4250 3C19 05BC 0004 0D22 0F09 0D83"            /* .$BP<..¼...".Æ.ƒ */
	$"0F0A 0E0C 0B03 0011 3875 9ED5 F984 FF23"            /* .Â......8užÕù„ÿ# */
	$"0F0F 0A2C 797B 7C7D 7F7F 8081 8281 8282"            /* ..Â,y{|}..€‚‚‚ */
	$"8383 8485 8789 8984 7A63 3901 0009 2746"            /* ƒƒ„…‡‰‰„zc9..Æ'F */
	$"4F37 1504 BC00 021C 0E0E 810F 0A0D 0C09"            /* O7..¼......Â..Æ */
	$"0205 111E 5D87 D2F8 88FF 100F 0F0A 2C78"            /* ....]‡Òøˆÿ...Â,x */
	$"7A7B 7C7E 7F7F 8081 8182 8182 8083 0F84"            /* z{|~..€‚‚€ƒ.„ */
	$"8383 8485 8A79 0700 0C2C 494E 3211 03BA"            /* ƒƒ„…Šy...,IN2..º */
	$"000F 0B18 0A17 0F0E 0B06 020C 132A 5E9F"            /* ....Â........*^Ÿ */
	$"CAF1 8CFF 100F 0F0A 2C77 7979 7B7C 7D7E"            /* ÊñŒÿ...Â,wyy{|}~ */
	$"8081 8182 8181 8083 0084 8183 0A87 6501"            /* €‚€ƒ.„ƒÂ‡e. */
	$"000E 304A 4B2C 0D01 B900 0C1C BCD2 1A07"            /* ..0JK,..¹...¼Ò.. */
	$"1C04 1B3F 70B5 D5F4 90FF 130F 0F08 2478"            /* ...?pµÕôÿ....$x */
	$"7572 7475 787A 7D7E 7F80 8181 8283 8282"            /* urtuxz}~.€‚ƒ‚‚ */
	$"830A 895B 0000 1133 4C48 270B 0198 0000"            /* ƒÂ‰[...3LH'..˜.. */
	$"019D 0008 2BE3 FFB2 0621 85EA FB94 FF23"            /* ...+ãÿ².!…êû”ÿ# */
	$"0000 0784 DECF BFB3 A89B 9088 827F 7B7A"            /* ...„ÞÏ¿³¨›ˆ‚.{z */
	$"7A7B 7E80 8082 8283 838B 4700 0114 374D"            /* z{~€€‚‚ƒƒ‹G...7M */
	$"4423 0801 9600 0303 0000 089A 0008 0827"            /* D#..–......š...' */
	$"013D FCFF AF04 9C94 FF04 F2F0 8781 B183"            /* .=üÿ¯.œ”ÿ.òð‡±ƒ */
	$"FF19 FEFA F2E7 D9CD C0B4 AA9C 9189 8581"            /* ÿ.þúòçÙÍÀ´ªœ‘‰… */
	$"7D7D 8639 0003 183B 4C40 1E05 9500 0601"            /* }}†9...;L@..•... */
	$"050F 5329 0703 9700 0B42 939A D36D 00D5"            /* ..S)..—..B“šÓm.Õ */
	$"FFFF 8312 E792 FF02 A11A 1C8F FF10 FDF9"            /* ÿÿƒ.ç’ÿ.¡..ÿ.ýù */
	$"F1E5 D7CA BDA3 2E00 041B 3D4C 3C1A 0495"            /* ñå×Ê½£....=L<..• */
	$"0006 0706 B0FF C306 0795 0002 26AD FD80"            /* ....°ÿÃ..•..&­ý€ */
	$"FF07 D304 8DFF FFFA 228F 91FF 03BB 0C07"            /* ÿ.Ó.ÿÿú"‘ÿ.».. */
	$"0B96 FF09 F42C 0005 1D3F 4B38 1602 9400"            /* .–ÿÆô,...?K8..”. */
	$"0702 0152 FFFF ED14 0392 0003 0325 57D5"            /* ...Rÿÿí..’...%WÕ */
	$"82FF 08FA 1B49 FEFF FF7F 38FE 83FF 10FD"            /* ‚ÿ.ú.Iþÿÿ.8þƒÿ.ý */
	$"FBF9 F2E8 E2DA D2C8 C1BA B2B0 5001 0F0F"            /* ûùòèâÚÒÈÁº²°P... */
	$"96FF 09EC 1200 0720 4149 3412 0193 0007"            /* –ÿÆì... AI4..“.. */
	$"010D 06C0 FFFF F727 9100 051A 4168 8484"            /* ...Àÿÿ÷'‘...Ah„„ */
	$"C682 FF16 FB1F 2BFB FFFF D119 C2D6 C9C0"            /* Æ‚ÿ.û.+ûÿÿÑ.ÂÖÉÀ */
	$"BAB4 AAA0 9A95 918C 8A89 8681 8404 8D49"            /* º´ª š•‘ŒŠ‰†„.I */
	$"070F 0F96 FF08 D406 0008 2442 472F 0F94"            /* ...–ÿ.Ô...$BG/.” */
	$"0007 0204 24F7 FFFF FC39 8E00 101A 3D62"            /* ....$÷ÿÿü9Ž...=b */
	$"7F8A 8783 7E98 DFD8 D4CE A24D 0064 80FF"            /* .Š‡ƒ~˜ßØÔÎ¢M.d€ÿ */
	$"0CF6 295E 8C83 8284 8485 8788 898A 808C"            /* .ö)^Œƒ‚„„…‡ˆ‰Š€Œ */
	$"018D 8D80 8E04 944D 060F 0F96 FF08 C100"            /* .€Ž.”M...–ÿ.Á. */
	$"000A 2643 442A 0C94 0002 0400 4A81 FF02"            /* .Â&CD*.”....Jÿ. */
	$"4F01 0189 0006 0923 5583 8B89 8580 8409"            /* O..‰..Æ#Uƒ‹‰…€„Æ */
	$"8588 897C 6634 0000 43E0 80FF 19FB 3239"            /* …ˆ‰|f4..Cà€ÿ.û29 */
	$"706C 6E71 7374 7476 787A 7B7D 7E7F 7F81"            /* plnqsttvxz{}~.. */
	$"8281 8A48 070F 0F95 FF09 FD9A 0000 0B28"            /* ‚ŠH...•ÿÆýš...( */
	$"4342 260A 9400 0209 006D 81FF 0266 0105"            /* CB&Â”..Æ.mÿ.f.. */
	$"8500 160B 1E3E 6280 8A8F 8785 8181 807D"            /* …....>b€Š‡…€} */
	$"7C71 5328 0700 1256 B1FB 81FF 06F8 2805"            /* |qS(...V±ûÿ.ø(. */
	$"1311 1112 8013 8014 0415 1717 1819 801A"            /* ....€.€.......€. */
	$"041E 160E 0F0F 94FF 0AA8 3B0E 0100 0C2B"            /* ......”ÿÂ¨;....+ */
	$"433F 2308 9400 0208 0088 81FF 028F 0009"            /* C?#.”....ˆÿ..Æ */
	$"8000 1801 142E 4762 7278 705D 4C42 3425"            /* €.....Gbrxp]LB4% */
	$"1F12 120E 0608 0A1C 546C A6E6 84FF 05F5"            /* ......Â.Tl¦æ„ÿ.õ */
	$"1E04 100E 0E89 0D81 0C00 0E80 0F02 BCB8"            /* .....‰....€..¼¸ */
	$"D687 FF00 F280 F010 DBDF EBC0 B763 0000"            /* Ö‡ÿ.ò€ð.ÛßëÀ·c.. */
	$"0B00 010E 2D42 3C1F 0694 0002 0700 9481"            /* ....-B<..”....” */
	$"FF1A C700 0B20 3148 697F 7D62 330B 0500"            /* ÿ.Ç.. 1Hi.}b3... */
	$"0004 0E1D 3041 4F53 5A6B 8CBF EC88 FF03"            /* ....0AOSZkŒ¿ìˆÿ. */
	$"DF08 0D13 930F 060E 0C17 5E87 6250 8252"            /* ß...“.....^‡bP‚R */
	$"154F 211D 1E1E 1718 1C0F 0C00 0005 1100"            /* .O!............. */
	$"0211 2E41 371B 0594 0002 0700 9A81 FF12"            /* ...A7..”....šÿ. */
	$"F93E 0049 625C 4324 132B 5070 8CA6 C4D9"            /* ù>.Ib\C$.+PpŒ¦ÄÙ */
	$"E9F6 FB88 FF01 F5E5 82FF 06D2 5803 1609"            /* éöûˆÿ.õå‚ÿ.ÒX..Æ */
	$"0D10 910F 060D 0D0C 0300 0305 8204 0105"            /* ..‘.........‚... */
	$"0A80 0B10 0C0B 0B0D 0C01 0003 0A00 0212"            /* Â€..........Â... */
	$"2F40 3317 0394 0002 0500 9C82 FF08 E356"            /* /@3..”....œ‚ÿ.ãV */
	$"1215 2343 78B4 F48F FF0F FEFF F88A D3FF"            /* ..#Cx´ôÿ.þÿøŠÓÿ */
	$"FFF6 A30F 0111 0700 0209 A50F 0D10 0600"            /* ÿö£......Æ¥..... */
	$"0006 1400 0213 303E 2F14 0296 0000 9384"            /* ......0>/..–..“„ */
	$"FF02 E8E6 F390 FF0E E199 92F8 FFFF D949"            /* ÿ.èæóÿ.á™’øÿÿÙI */
	$"F1EF 2F00 3D41 0381 0007 050F 0703 080C"            /* ñï/.=A......... */
	$"0D0E 9C0F 0110 0780 0009 040F 0003 1431"            /* ..œ....€.Æ.....1 */
	$"3C2C 1201 9400 0201 0081 92FF 01E9 FE83"            /* <,..”....’ÿ.éþƒ */
	$"FF02 F14D 5281 FF06 9D54 322C 7C57 0883"            /* ÿ.ñMRÿ.T2,|W.ƒ */
	$"000C 0457 6D38 160E 0201 0504 090D 0D95"            /* ...Wm8......Æ..• */
	$"0F01 1007 8100 0803 0600 0415 3139 2810"            /* ...........19(. */
	$"9500 0204 005D 91FF 03E1 3F6B F483 FF02"            /* •....]‘ÿ.á?kôƒÿ. */
	$"D677 FA81 FF04 9700 3342 0185 000F 2298"            /* Öwúÿ.—.3B.….."˜ */
	$"F9E5 BC96 6A4F 522C 100C 0004 080E 900F"            /* ùå¼–jOR,....... */
	$"0110 0782 0008 1B0B 0005 1730 3624 0D95"            /* ...‚.......06$.• */
	$"0003 020A 28F4 8EFF 06D3 6416 0000 35D8"            /* ...Â(ôŽÿ.Ód...5Ø */
	$"83FF 01DF E982 FF04 B229 000C 0C86 0001"            /* ƒÿ.ßé‚ÿ.²)...†.. */
	$"5EF3 82FF 08F5 CAB6 8755 3709 0C0D 8D0F"            /* ^ó‚ÿ.õÊ¶‡U7Æ... */
	$"0110 0983 0008 350E 0006 1730 3220 0A96"            /* ..Æƒ..5....02 Â– */
	$"0002 0B06 BA8D FF01 B817 8200 0154 FE83"            /* ....ºÿ.¸.‚..Tþƒ */
	$"FF02 E9D4 FA81 FF08 EC69 030E 1B2B 3417"            /* ÿ.éÔúÿ.ìi...+4. */
	$"0482 0001 46DA 86FF 03F8 7A07 0D8C 0F01"            /* .‚..FÚ†ÿ.øz..Œ.. */
	$"1008 8300 0918 3A04 0006 182E 301D 0996"            /* ..ƒ.Æ.:.....0.Æ– */
	$"0003 0101 46FC 8BFF 01F2 1D83 0001 11E8"            /* ....Fü‹ÿ.ò.ƒ...è */
	$"84FF 01F0 F782 FF09 E806 0235 505E 5A50"            /* „ÿ.ð÷‚ÿÆè..5P^ZP */
	$"2609 8100 0116 9F86 FF02 FDBF BB88 0F04"            /* &Æ...Ÿ†ÿ.ý¿»ˆ.. */
	$"0E0D 0B0F 0983 000A 03AF A801 0007 182D"            /* ....Æƒ.Â.¯¨....- */
	$"2C1A 0897 0002 0301 7C8B FF01 D606 8300"            /* ,..—....|‹ÿ.Ö.ƒ. */
	$"0106 D28B FF0C DF44 2507 0000 0F1F 2E35"            /* ..Ò‹ÿ.ßD%......5 */
	$"2918 0480 0001 69EF 87FF 820F 090E 0E0D"            /* )..€..iï‡ÿ‚.Æ... */
	$"0C0B 0D0F 1830 1E84 000A 21A0 7600 0007"            /* .....0.„.Â! v... */
	$"182B 2816 0698 0003 0704 54F6 89FF 00B8"            /* .+(..˜....Tö‰ÿ.¸ */
	$"8400 0107 D48C FF11 FEF6 DCB5 8137 0400"            /* „...ÔŒÿ.þöÜµ7.. */
	$"0001 1310 1004 0000 2DC2 86FF 0E10 100F"            /* ........-Â†ÿ.... */
	$"0E0C 0E11 1825 333E 4A58 400B 8400 0A40"            /* .....%3>JX@.„.Â@ */
	$"724D 0000 0717 2925 1404 9900 0409 0140"            /* rM....)%..™..Æ.@ */
	$"B9FB 83FF 00FA 80FF 009A 8400 0110 E591"            /* ¹ûƒÿ.ú€ÿ.š„...å‘ */
	$"FF0D FBDA AB78 4824 0700 030F 0000 17AC"            /* ÿ.ûÚ«xH$.......¬ */
	$"85FF 0D06 0B0C 1537 4A52 575B 5D5D 613B"            /* …ÿ.....7JRW[]]a; */
	$"0184 000B 0B41 4531 0000 0716 2722 1103"            /* .„...AE1....'".. */
	$"9A00 0A05 2317 346F A3CB DDE0 9FE0 80FF"            /* š.Â.#.4o£ËÝàŸà€ÿ */
	$"0091 8400 012B F896 FF0A F6D2 9A32 0310"            /* .‘„..+ø–ÿÂöÒš2.. */
	$"0000 0175 EE83 FF0C 0000 0104 1D39 595F"            /* ...uîƒÿ......9Y_ */
	$"605C 5E41 0285 000B 0F20 1E18 0000 0816"            /* `\^A.…... ...... */
	$"251E 1003 9B00 0930 4431 0600 030A 0C68"            /* %...›.Æ0D1...Â.h */
	$"FD80 FF01 DD0A 8300 005E 9AFF 03EC 2E12"            /* ý€ÿ.ÝÂƒ..^šÿ.ì.. */
	$"2680 0001 26CE 82FF 8300 0509 1C33 4B4A"            /* &€..&Î‚ÿƒ..Æ.3KJ */
	$"0C85 000C 2262 6166 3A00 0007 1522 1C0E"            /* .….."baf:....".. */
	$"039A 0009 040A 031B 201D 1600 24EE 82FF"            /* .š.Æ.Â.. ...$î‚ÿ */
	$"005A 8200 0123 C99B FF03 C511 451D 8000"            /* .Z‚..#É›ÿ.Å.E.€. */
	$"051D B6FF F5C8 9C86 0001 0303 8500 0D02"            /* ..¶ÿõÈœ†....…... */
	$"94DC D2E0 7200 0007 141F 180C 029A 0001"            /* ”ÜÒàr........š.. */
	$"0612 8000 0401 110F 0EDE 82FF 06DA 2000"            /* ..€......Þ‚ÿ.Ú . */
	$"0245 A3EC 8DFF 13FA D1FE FFFF FDEA CCA5"            /* .E£ìÿ.úÑþÿÿýêÌ¥ */
	$"B5F6 FFFF FD94 3503 1D63 1780 0003 064D"            /* µöÿÿý”5..c.€...M */
	$"2004 9100 0D2D B0AD ACB8 5D00 0007 111B"            /*  .‘..-°­¬¸]..... */
	$"1409 019A 0001 0614 8200 030C 0648 D382"            /* .Æ.š....‚....HÓ‚ */
	$"FF02 D789 BC90 FF14 F93B 4261 593B 1805"            /* ÿ.×‰¼ÿ.ù;BaY;.. */
	$"0000 32B3 FFDE 0201 1404 2E75 3E97 000D"            /* ..2³ÿÞ.....u>—.. */
	$"5E9E 9495 9F50 0000 060E 1811 0701 9A00"            /* ^ž”•ŸP........š. */
	$"0105 1082 0004 0111 0014 C897 FF01 FB2F"            /* ...‚......È—ÿ.û/ */
	$"8600 0B0D C381 0009 6D44 002B 9A3B 0194"            /* †...Ã.ÆmD.+š;.” */
	$"000D 1581 8583 838C 4700 0005 0D14 0E06"            /* ...…ƒƒŒG....... */
	$"9B00 0102 0784 0002 1100 8C98 FF00 6787"            /* ›....„....Œ˜ÿ.g‡ */
	$"0000 0380 0006 0A9E A90C 3C52 0E94 000D"            /* ...€..Âž©.<R.”.. */
	$"4A7E 7677 777F 4000 0004 0A0F 0B04 9B00"            /* J~vww.@...Â...›. */
	$"0102 0483 0003 0A2C 18DA 98FF 03EC A266"            /* ...ƒ..Â,.Ú˜ÿ.ì¢f */
	$"2285 0009 1616 0222 C0FF 5F00 0606 9300"            /* "….Æ..."Àÿ_...“. */
	$"020F 686C 806A 0871 3900 0003 070A 0703"            /* ..hl€j.q9....Â.. */
	$"9B00 0103 1183 0002 4116 7C9C FF02 ED8F"            /* ›....ƒ..A.|œÿ.í */
	$"1582 0007 47E9 EBC3 E5FF FF8A 9500 0301"            /* .‚..GéëÃåÿÿŠ•... */
	$"3B69 6580 6408 6B36 0000 0205 0704 019B"            /* ;ie€d.k6.......› */
	$"0002 021C 0982 0002 3002 A79E FF02 DA86"            /* ....Æ‚..0.§žÿ.Ú† */
	$"2D80 0000 7483 FF00 9F95 0002 1F69 6781"            /* -€..tƒÿ.Ÿ•...ig */
	$"6501 6C36 8000 0202 0302 9C00 0A02 1A19"            /* e.l6€.....œ.Â... */
	$"0400 0002 191C 0080 A0FF 04F4 B067 54CE"            /* .......€ ÿ.ô°gTÎ */
	$"83FF 00B7 9500 0818 616A 6564 6565 6B36"            /* ƒÿ.·•...ajedeek6 */
	$"A300 0A86 BCB9 4B00 357D 740F 20F3 AAFF"            /* £.Â†¼¹K.5}t. óªÿ */
	$"0283 0015 8016 0117 0C8F 0007 0D4D 746F"            /* .ƒ..€.......Mto */
	$"6D6D 743A A300 0AA2 E7E5 B625 8198 9A44"            /* mmt:£.Â¢çå¶%˜šD */
	$"00AC A9FF 09E3 123B 9F99 9A9B 9D8E 108F"            /* .¬©ÿÆã.;Ÿ™š›Ž. */
	$"0006 023B 777D 777F 40A3 000B 75A6 A183"            /* ...;w}w.@£..u¦¡ƒ */
	$"5CA2 B2B5 9504 34F9 A8FF 095C 0391 AEAE"            /* \¢²µ•.4ù¨ÿÆ\.‘®® */
	$"ADA8 A8AE 4391 0004 1E66 8E8F 48A3 000B"            /* ­¨¨®C‘...fŽH£.. */
	$"4D6D A245 46B4 77B7 E46B 008A A7FF 0A89"            /* Mm¢EF´w·äk.Š§ÿÂ‰ */
	$"005A C2BC A255 3434 3728 9200 0306 75B3"            /* .ZÂ¼¢U447(’...u³ */
	$"56A3 000C 2E66 C24A 47B5 3964 D4D0 350E"            /* V£...fÂJGµ9dÔÐ5. */
	$"E3A6 FF05 5400 607B 7015 9800 0205 7A69"            /* ã¦ÿ.T.`{p.˜...zi */
	$"A300 0D27 70BB 484A BB43 6E8A 79A8 153D"            /* £..'p»HJ»CnŠy¨.= */
	$"DE8A FF00 F396 FF05 F583 0015 1204 9A00"            /* ÞŠÿ.ó–ÿ.õƒ....š. */
	$"0105 1DA3 000E 0816 240F 1E48 1F40 4D2F"            /* ...£....$..H.@M/ */
	$"834D 0921 C188 FF05 DD2E 4576 B4FA 80FF"            /* ƒMÆ!Áˆÿ.Ý.Ev´ú€ÿ */
	$"02F9 CCE9 89FF 07CF A275 2908 0927 05CC"            /* .ùÌé‰ÿ.Ï¢u).Æ'.Ì */
	$"0006 0201 0915 030A 9486 FF01 DC2C 8100"            /* ....Æ..Â”†ÿ.Ü,. */
	$"0630 6CED DA3B 0443 85FF 04FE 9E4B 4B0E"            /* .0líÚ;.C…ÿ.þžKK. */
	$"8000 020A 0702 9400 0308 1A10 0080 01B3"            /* €..Â..”......€.³ */
	$"0021 0805 0048 92D3 F1F1 E5CB A977 1B00"            /* .!...H’ÓññåË©w.. */
	$"0B0B 0906 0000 2215 0004 0870 8B97 9DA0"            /* ..Æ..."....p‹—  */
	$"9894 833F 8000 0408 0F0A 0701 9600 061C"            /* ˜”ƒ?€....Â..–... */
	$"5E3A 0004 0302 B400 1705 1507 0004 1517"            /* ^:....´......... */
	$"0B01 0000 150E 0300 0103 0206 0806 0412"            /* ................ */
	$"1780 0002 0101 0380 0005 192C 2F41 2208"            /* .€.....€...,/A". */
	$"9800 061C 5E3B 0004 0302 B500 0B24 5344"            /* ˜...^;....µ..$SD */
	$"271A 1626 1C15 395B 1083 0013 130D 0021"            /* '..&..9[.ƒ.....! */
	$"5236 4B52 3B3D 3522 0000 2F43 3643 2008"            /* R6KR;=5"../C6C . */
	$"8900 FFFF FBFF 7438 6D6B 0000 4008 0000"            /* ‰.ÿÿûÿt8mk..@... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0001 2F4A 4646 4646 4646 4647"            /* ....../JFFFFFFFG */
	$"4849 4949 4A4A 4B4C 4D4D 4E4E 4F4F 5154"            /* HIIIJJKLMMNNOOQT */
	$"4405 0000 0000 0000 0000 0000 0000 0000"            /* D............... */
	$"0000 0014 5759 5858 5757 5756 5656 5757"            /* ....WYXXWWWVVVWW */
	$"5757 5757 5656 5754 1000 0000 0000 0000"            /* WWWWVVWT........ */
	$"0000 0000 0000 0000 114D 4E4C 4B4A 4A49"            /* .........MNLKJJI */
	$"4947 4746 4747 4646 4646 4645 4646 4816"            /* IGGFGGFFFFFEFFH. */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0009 1F1F 1F1F 2B5E 6563 6363 6464 6566"            /* .Æ....+^ecccddef */
	$"6768 6868 696A 6C5E 5456 5758 5959 5B5C"            /* ghhhijl^TVWXYY[\ */
	$"6022 0000 0000 0000 0000 000C 2524 2424"            /* `"..........%$$$ */
	$"2424 232D 747C 7A76 7474 7879 7978 7574"            /* $$#-t|zvttxyyxut */
	$"6660 6363 6362 6361 1800 0000 0000 0002"            /* f`cccbca........ */
	$"2025 2424 2424 2420 4071 6E6C 6B6A 6967"            /*  %$$$$$ @qnlkjig */
	$"6766 6465 594B 4C4C 4C4C 4C4C 4B4F 3D04"            /* gfdeYKLLLLLLKO=. */
	$"0000 0000 0000 0000 031D 0F00 0000 0000"            /* ................ */
	$"000D 2D2E 2E2E 2B44 6E6D 6D6D 6F70 7071"            /* ..-...+Dnmmmoppq */
	$"7172 7474 7576 7970 5858 5A5B 5C5D 5E5F"            /* qrttuvypXXZ[\]^_ */
	$"6547 0100 0000 0000 0000 0006 3238 3737"            /* eG..........2877 */
	$"3737 3638 7587 88A0 B8C9 D7DB DBD8 CBB8"            /* 7768u‡ˆ ¸É×ÛÛØË¸ */
	$"9269 6065 6767 6862 1200 0000 0000 0005"            /* ’i`egghb........ */
	$"3338 3737 3636 3532 5F80 7A79 7876 7573"            /* 38776652_€zyxvus */
	$"7271 6F6E 5649 4B4B 4B4B 4B4B 4B4D 1C00"            /* rqonVIKKKKKKKM.. */
	$"0000 0000 0000 0000 152F 1500 0000 0000"            /* ........./...... */
	$"000C 2C2C 2C2E 2D2C 516F 6D6E 6F70 7072"            /* ..,,,.-,Qomnoppr */
	$"7374 7576 7778 7A7B 655A 5D5E 6061 6263"            /* stuvwxz{eZ]^`abc */
	$"6663 1000 0000 0000 0000 0000 2639 3737"            /* fc..........&977 */
	$"3737 352B 6FAC D8FD FFFF FFFF FFFF FFFF"            /* 775+o¬Øýÿÿÿÿÿÿÿÿ */
	$"FEDD 9D76 6769 6E61 0700 0000 0000 0008"            /* þÝvgina........ */
	$"3538 3736 3636 3434 6D82 7D7C 7A78 7674"            /* 58766644m‚}|zxvt */
	$"7372 706B 504B 4C4B 4B4B 4B4B 4D42 0500"            /* srpkPKLKKKKKMB.. */
	$"0000 0000 0000 0003 242C 1400 0000 0000"            /* ........$,...... */
	$"000C 2C2D 2E2E 2F2F 3161 7270 7071 7374"            /* ..,-..//1arppqst */
	$"7577 7879 7B7D 7E80 7960 6063 6466 6769"            /* uwxy{}~€y``cdfgi */
	$"6A70 3400 0000 0000 0000 0000 183B 393A"            /* jp4..........;9: */
	$"3A33 3475 DAFF FFFF FFFF FFFF FFFF FFFF"            /* :34uÚÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFEF BC7F 6E64 0600 0000 0000 000C"            /* ÿÿÿï¼.nd........ */
	$"393B 3939 3838 3541 8186 8280 7F7D 7B79"            /* 9;99885A†‚€.}{y */
	$"7674 7566 4E4D 4D4C 4B4B 4B4B 5027 0000"            /* vtufNMMLKKKKP'.. */
	$"0000 0000 0000 0017 2A2B 1400 0000 0000"            /* ........*+...... */
	$"000D 2D2F 2F30 3131 2F3E 6E73 7375 7777"            /* ..-//011/>nssuww */
	$"797B 7C7E 8081 8384 8774 6569 6A6B 6D6F"            /* y{|~€ƒ„‡teijkmo */
	$"7176 6004 0000 0000 0000 0000 0939 3E3D"            /* qv`.........Æ9>= */
	$"3657 B6FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* 6W¶ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFEE B168 0300 0000 0000 0011"            /* ÿÿÿÿÿî±h........ */
	$"3D3D 3D3C 3C3B 365E 908B 8887 8482 807E"            /* ===<<;6^‹ˆ‡„‚€~ */
	$"7B79 775F 5050 4E4D 4C4B 4B4D 4208 0000"            /* {yw_PPNMLKKMB... */
	$"0000 0000 0000 0E2A 292B 1400 0000 0000"            /* .......*)+...... */
	$"000D 2F30 3031 3233 3331 5479 7677 797A"            /* ../0012331Tyvwyz */
	$"7C7F 8082 8486 8789 8C8B 726C 6F71 7375"            /* |.€‚„†‡‰Œ‹rloqsu */
	$"7678 7C23 0000 0000 0000 0000 002F 4334"            /* vx|#........./C4 */
	$"71EE FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* qîÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFBB 0000 0000 0000 0017"            /* ÿÿÿÿÿÿÿ»........ */
	$"4240 3F3F 3E3D 3B77 9791 8E8D 8A88 8582"            /* B@??>=;w—‘ŽŠˆ…‚ */
	$"807D 785A 5352 504F 4D4C 4C4D 1C00 0000"            /* €}xZSRPOMLLM.... */
	$"0000 0000 0001 212A 282B 1400 0000 0000"            /* ......!*(+...... */
	$"000D 3031 3132 3334 3535 3A6C 7C7A 7D7E"            /* ..01123455:l|z}~ */
	$"8082 8486 888B 8C8E 9094 8671 7476 787A"            /* €‚„†ˆ‹ŒŽ”†qtvxz */
	$"7C7D 864D 0000 0000 0000 0000 0020 3A6D"            /* |}†M......... :m */
	$"EDFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* íÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFE 6800 0000 0000 001D"            /* ÿÿÿÿÿÿÿþh....... */
	$"4643 4241 413F 438A 9C97 9492 918D 8A86"            /* FCBAA?CŠœ—”’‘Š† */
	$"8483 7659 5654 5251 4E4D 4F42 0500 0000"            /* „ƒvYVTRQNMOB.... */
	$"0000 0000 000E 2A29 282B 1400 0000 0000"            /* ......*)(+...... */
	$"000D 3132 3334 3637 3738 3744 7381 8182"            /* ..123467787Ds‚ */
	$"8587 898B 8D90 9294 9799 9981 7A7C 7F81"            /* …‡‰‹’”—™™z|. */
	$"8384 897D 1600 0000 0000 0000 0003 70F0"            /* ƒ„‰}..........pð */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF F330 0000 0000 0025"            /* ÿÿÿÿÿÿÿÿó0.....% */
	$"4A46 4545 4541 519E A19E 9B99 9794 918E"            /* JFEEEAQž¡ž›™—”‘Ž */
	$"8A88 6E5C 5B58 5553 514F 5223 0000 0000"            /* Šˆn\[XUSQOR#.... */
	$"0000 0000 0322 2B29 292B 1400 0000 0000"            /* ....."+))+...... */
	$"000E 3233 3436 3739 3A3B 3C39 5082 8687"            /* ..234679:;<9P‚†‡ */
	$"898C 8E90 9395 979A 9C9E A299 8183 8588"            /* ‰ŒŽ“•—šœž¢™ƒ…ˆ */
	$"898B 8D96 5700 0000 0000 0000 004C F3FF"            /* ‰‹–W........Lóÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFB1 0000 0000 002C"            /* ÿÿÿÿÿÿÿÿÿ±....., */
	$"4E49 4948 4742 67AB A7A5 A2A0 9D9A 9794"            /* NIIHGBg«§¥¢ š—” */
	$"9286 6763 5F5C 5955 5354 4706 0000 0000"            /* ’†gc_\YUSTG..... */
	$"0000 0000 132C 2929 292B 1400 0000 0000"            /* .....,)))+...... */
	$"000F 3434 3637 383B 3C3D 3F3F 3E6D 8C8B"            /* ..44678;<=??>mŒ‹ */
	$"8D90 9394 989A 9C9E A2A4 A6A8 9088 8B8D"            /* “”˜šœž¢¤¦¨ˆ‹ */
	$"8F91 9298 870A 0000 0000 0000 13DD FFFF"            /* ‘’˜‡Â.......Ýÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFC 4100 0000 0031"            /* ÿÿÿÿÿÿÿÿÿüA....1 */
	$"514C 4C4B 4A44 7CB3 ACAA A8A5 A4A0 9D9A"            /* QLLKJD|³¬ª¨¥¤ š */
	$"9883 6968 635F 5C59 5658 2E00 0000 0000"            /* ˜ƒihc_\YVX...... */
	$"0000 0003 242C 2A2A 292C 1400 0000 0000"            /* ....$,**),...... */
	$"000A 3238 3739 3B3D 3E40 4142 414C 8491"            /* .Â2879;=>@ABAL„‘ */
	$"9195 989A 9DA0 A2A5 A8A9 ABAF A68F 9093"            /* ‘•˜š ¢¥¨©«¯¦“ */
	$"9596 979A A03A 0000 0000 0000 A0FF FFFF"            /* •–—š :...... ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF BA01 0000 0039"            /* ÿÿÿÿÿÿÿÿÿÿº....9 */
	$"5350 4F4E 4D4B 95B9 B2B0 AEAC AAA6 A3A1"            /* SPONMK•¹²°®¬ª¦£¡ */
	$"9D80 6F6D 6864 615D 5B53 0E00 0000 0000"            /* €omhda][S...... */
	$"0000 001C 2D2A 2A2A 292C 1500 0000 0000"            /* ....-***),...... */
	$"0000 1338 3A3B 3D3E 4042 4445 4643 5F93"            /* ...8:;=>@BDEFC_“ */
	$"979A 9DA0 A3A5 A8AA ADAF B0B3 B6A6 9699"            /* —š £¥¨ª­¯°³¶¦–™ */
	$"9B9C 9E9F A777 0100 0000 005D FFFF FFFF"            /* ›œžŸ§w.....]ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF F826 0000 0243"            /* ÿÿÿÿÿÿÿÿÿÿø&...C */
	$"5552 5351 4F56 AEBC B9B7 B5B2 B0AD A9A8"            /* URSQOV®¼¹·µ²°­©¨ */
	$"9F7E 7873 6E6A 6762 6238 0000 0000 0000"            /* Ÿ~xsnjgbb8...... */
	$"0000 0C2D 2C2B 2A2A 2A2C 1500 0000 0000"            /* ...-,+***,...... */
	$"0000 0020 3F3D 3F41 4244 4647 4949 4773"            /* ... ?=?ABDFGIIGs */
	$"9E9E A1A3 A7AA ACAF B1B3 B5B7 BAB8 9F9C"            /* žž¡£§ª¬¯±³µ·º¸Ÿœ */
	$"9FA1 A2A4 A79F 1800 0000 0FD7 FFFF FFFF"            /* Ÿ¡¢¤§Ÿ.....×ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF5A 0000 044A"            /* ÿÿÿÿÿÿÿÿÿÿÿZ...J */
	$"5754 5454 5063 BCC0 BDBB B9B7 B5B2 AFAE"            /* WTTTPc¼À½»¹·µ²¯® */
	$"9D81 7E79 746F 6A68 5B0D 0000 0000 0000"            /* ~ytojh[....... */
	$"0000 202F 2C2C 2A2A 2A2D 1500 0000 0000"            /* .. /,,***-...... */
	$"0000 0002 2641 4243 4546 484A 4B4D 4C4F"            /* ....&ABCEFHJKMLO */
	$"8BA7 A5A8 ABAE B0B3 B5B7 BABC BDC0 B3A1"            /* ‹§¥¨«®°³µ·º¼½À³¡ */
	$"A3A5 A7A8 A9B2 5300 0000 A6FF FFFF FFFF"            /* £¥§¨©²S...¦ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF8B 0000 0850"            /* ÿÿÿÿÿÿÿÿÿÿÿ‹...P */
	$"5957 5756 507C C7C4 C1C0 BEBC BBB7 B5B4"            /* YWWVP|ÇÄÁÀ¾¼»·µ´ */
	$"9986 8480 7975 7070 3800 0000 0000 0000"            /* ™†„€yupp8....... */
	$"000D 2F2E 2D2C 2B2A 2A2D 1500 0000 0000"            /* ../.-,+**-...... */
	$"0000 0000 0022 4546 4749 4B4D 4E4F 514F"            /* ....."EFGIKMNOQO */
	$"5FA1 ACAD B0B3 B5B8 BBBD BFC1 C2C4 C4AD"            /* _¡¬­°³µ¸»½¿ÁÂÄÄ­ */
	$"A7AA ABAC ACB4 9308 0049 FEFF FFFF FFFF"            /* §ª«¬¬´“..Iþÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFB4 0100 0E57"            /* ÿÿÿÿÿÿÿÿÿÿÿ´...W */
	$"5C5A 5A59 5397 CFC8 C6C5 C3C2 C0BD BBB5"            /* \ZZYS—ÏÈÆÅÃÂÀ½»µ */
	$"978D 8B86 807C 796B 0F00 0000 0000 0000"            /* —‹†€|yk........ */
	$"0126 322F 2E2C 2C2B 2A2D 1404 0900 0000"            /* .&2/.,,+*-..Æ... */
	$"0000 0000 0000 2D4C 494B 4D4E 5052 5354"            /* ......-LIKMNPRST */
	$"4F7B B3B1 B4B7 B9BC BFC1 C2C4 C5C7 C9BE"            /* O{³±´·¹¼¿ÁÂÄÅÇÉ¾ */
	$"ABAD AEB0 B0B3 B631 00C0 FFFF FFFF FFFF"            /* «­®°°³¶1.Àÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFD6 0700 135B"            /* ÿÿÿÿÿÿÿÿÿÿÿÖ...[ */
	$"5D5C 5C5A 5AAE D1CB CAC8 C7C5 C3C1 C0B3"            /* ]\\ZZ®ÑËÊÈÇÅÃÁÀ³ */
	$"9894 908C 8681 844A 0000 0000 0000 0000"            /* ˜”Œ†„J........ */
	$"1235 3230 2F2E 2C2C 2B2D 1228 1900 0000"            /* .520/.,,+-.(.... */
	$"0000 0000 0000 073B 504D 4F50 5253 5556"            /* .......;PMOPRSUV */
	$"555B 9EB9 B8BA BCBF C1C3 C5C7 C8C9 CBCA"            /* U[ž¹¸º¼¿ÁÃÅÇÈÉËÊ */
	$"B4AF B1B3 B3B5 BD72 3AFA FFFF FFFF FFFF"            /* ´¯±³³µ½r:úÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFE6 0E00 1B5F"            /* ÿÿÿÿÿÿÿÿÿÿÿæ..._ */
	$"5F5E 5D5B 63C1 D1CE CDCC CAC9 C7C5 C4B0"            /* _^][cÁÑÎÍÌÊÉÇÅÄ° */
	$"9B99 9591 8C88 821F 0000 0000 0000 0007"            /* ›™•‘Œˆ‚......... */
	$"2F36 3331 302F 2C2C 2C2A 3247 1600 0000"            /* /6310/,,,*2G.... */
	$"0000 0000 0000 000A 3A53 5152 5556 5758"            /* .......Â:SQRUVWX */
	$"5A57 64A9 BFBD C0C2 C5C6 C8CA CCCC CDCF"            /* ZWd©¿½ÀÂÅÆÈÊÌÌÍÏ */
	$"C9B5 B4B6 B6B7 B8AF CCFF FFFF FFFF FFFF"            /* Éµ´¶¶·¸¯Ìÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF3 1E00 2463"            /* ÿÿÿÿÿÿÿÿÿÿÿó..$c */
	$"6160 605B 77D0 D2D0 D0CF CECC CBC9 C6AC"            /* a``[wÐÒÐÐÏÎÌËÉÆ¬ */
	$"A29E 9B97 9193 6100 0000 0000 0000 0023"            /* ¢ž›—‘“a........# */
	$"3B36 3332 3130 2E2D 293D 5A4B 1500 0000"            /* ;63210.-)=ZK.... */
	$"0000 0000 0000 0000 053D 5755 5658 595A"            /* .........=WUVXYZ */
	$"5B5D 5872 BCC3 C2C6 C8C9 CACC CECF D0D1"            /* []Xr¼ÃÂÆÈÉÊÌÎÏÐÑ */
	$"D4C4 B6B7 B9B9 B6CF FFFF FFFF FFFF FFFF"            /* ÔÄ¶·¹¹¶Ïÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF7 2A00 2B66"            /* ÿÿÿÿÿÿÿÿÿÿÿ÷*.+f */
	$"6161 615A 90D8 D4D3 D2D1 D0CF CECD C0AA"            /* aaaZØÔÓÒÑÐÏÎÍÀª */
	$"A8A4 A19C 9995 2700 0000 0000 0000 0D3B"            /* ¨¤¡œ™•'........; */
	$"3A38 3634 3130 302C 3263 5D48 1500 0000"            /* :864100,2c]H.... */
	$"0000 0000 0000 0000 000D 4E59 5859 5B5C"            /* ..........NYXY[\ */
	$"5D5E 5E5A 98C8 C4C6 C8CA CCCD CFD0 D1D3"            /* ]^^Z˜ÈÄÆÈÊÌÍÏÐÑÓ */
	$"D4D1 BBB8 BABA B9EC FFFF FFFF FFFF FFFF"            /* ÔÑ»¸ºº¹ìÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF8 2D00 3168"            /* ÿÿÿÿÿÿÿÿÿÿÿø-.1h */
	$"6363 615F ADDB D5D5 D4D3 D2D1 D0CF B9AC"            /* cca_­ÛÕÕÔÓÒÑÐÏ¹¬ */
	$"ABA8 A5A1 A36A 0200 0000 0000 0000 2841"            /* «¨¥¡£j........(A */
	$"3B39 3735 3332 302B 5170 5B49 1500 0000"            /* ;975320+Qp[I.... */
	$"0000 0000 0000 0000 0000 1654 5D5B 5C5E"            /* ...........T][\^ */
	$"5E5F 5D57 62B7 CFD2 D6D5 D2D0 CFD1 D3D4"            /* ^_]Wb·ÏÒÖÕÒÐÏÑÓÔ */
	$"D4D7 C8BA BCBB C9FD FFFF FFFF FFFF FFFF"            /* Ô×Èº¼»Éýÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF8 2C00 3A6A"            /* ÿÿÿÿÿÿÿÿÿÿÿø,.:j */
	$"6464 6170 CCD9 D7D6 D5D5 D4D3 D3CE B5B1"            /* ddapÌÙ×ÖÕÕÔÓÓÎµ± */
	$"AFAB A9A7 9D21 0000 0000 0000 000E 4140"            /* ¯«©§!........A@ */
	$"3D3B 3836 3533 2E45 6B6D 5B49 1500 0002"            /* =;8653.Ekm[I.... */
	$"0C00 0000 0000 0000 0000 0015 5160 5D5F"            /* ............Q`]_ */
	$"5E59 6790 BBE7 FCFD FEFE FCF4 E8DC D3D5"            /* ^Yg»çüýþþüôèÜÓÕ */
	$"D6D7 D5C1 BCBB E0FF FFFF FFFF FFFF FFFF"            /* Ö×ÕÁ¼»àÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF5 2400 426A"            /* ÿÿÿÿÿÿÿÿÿÿÿõ$.Bj */
	$"6565 6087 DAD9 D8D7 D7D6 D6D6 D6C9 B5B4"            /* ee`‡ÚÙØ××ÖÖÖÖÉµ´ */
	$"B2AF ACB2 6E00 0000 0000 0000 0032 4741"            /* ²¯¬²n........2GA */
	$"3F3C 3A38 3631 456E 6E6D 5B49 1500 0001"            /* ?<:861Ennm[I.... */
	$"381E 0000 0000 0000 0000 0000 1B5D 615A"            /* 8............]aZ */
	$"5E8E E0FF FFFF FFFF FFFF FFFF FFFD E6D5"            /* ^ŽàÿÿÿÿÿÿÿÿÿÿýæÕ */
	$"D7D7 D9CB BBC0 F3FF FFFF FFFF FFFF FFFF"            /* ××ÙË»Àóÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFF0 1800 4A6A"            /* ÿÿÿÿÿÿÿÿÿÿÿð..Jj */
	$"6565 5F99 DFD9 D8D8 D8D8 D7D7 D6C4 B7B6"            /* ee_™ßÙØØØØ××ÖÄ·¶ */
	$"B5B2 B0B1 3300 0000 0000 0000 1B4A 4642"            /* µ²°±3........JFB */
	$"403F 3C39 3638 6772 6D6E 5C49 1500 0000"            /* @?<968grmn\I.... */
	$"3F59 2100 0000 0000 0000 0000 0024 5779"            /* ?Y!..........$Wy */
	$"CBFF FFFF FFFF FFFF FFFF FFFF FFFF FFED"            /* Ëÿÿÿÿÿÿÿÿÿÿÿÿÿÿí */
	$"D7D7 D9D8 C3CD FEFF FFFF FFFF FFFF FFFF"            /* ××ÙØÃÍþÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFE4 0B00 536A"            /* ÿÿÿÿÿÿÿÿÿÿÿä..Sj */
	$"6766 62AF E0D9 DAD9 D9D9 D8D8 D6C0 BAB8"            /* gfb¯àÙÚÙÙÙØØÖÀº¸ */
	$"B7B5 BA8C 0600 0000 0000 0007 444D 4744"            /* ·µºŒ........DMGD */
	$"4341 3E3B 365D 7671 6E6E 5C49 1500 0000"            /* CA>;6]vqnn\I.... */
	$"3D5D 5B25 0000 0000 0000 0000 0000 92F7"            /* =][%..........’÷ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"EDD8 D9DA D2E6 FFFF FFFF FFFF FFFF FFFF"            /* íØÙÚÒæÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFD2 0503 5A6A"            /* ÿÿÿÿÿÿÿÿÿÿÿÒ..Zj */
	$"6765 6BC6 DEDA DADA DADA D9DA D1BE BCBB"            /* gekÆÞÚÚÚÚÚÙÚÑ¾¼» */
	$"B9B8 BA40 0000 0000 0000 002B 534D 4A47"            /* ¹¸º@.......+SMJG */
	$"4542 3F39 537C 7672 7170 5C49 1500 0000"            /* EB?9S|vrqp\I.... */
	$"3E5B 5E61 2700 0000 0000 0000 0089 FFFF"            /* >[^a'........‰ÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFE6 D8D9 DCFB FFFF FFFF FFFF FFFF FFFF"            /* ÿæØÙÜûÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFB4 000B 6169"            /* ÿÿÿÿÿÿÿÿÿÿÿ´..ai */
	$"6864 78D7 DCDB DBDA DADA DADB CCBF BFBD"            /* hdx×ÜÛÛÚÚÚÚÛÌ¿¿½ */
	$"BBC0 9C06 0000 0000 0000 0B4E 524E 4C49"            /* »Àœ........NRNLI */
	$"4644 3D4E 7D7E 7774 7171 5C49 1500 0000"            /* FD=N}~wtqq\I.... */
	$"3F5E 5D63 662E 0000 0000 0000 60FE FFFF"            /* ?^]cf.......`þÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFF3 DAD8 E5FF FFFF FFFF FFFF FFFF FFFF"            /* ÿóÚØåÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF9E 0016 6769"            /* ÿÿÿÿÿÿÿÿÿÿÿž..gi */
	$"6862 8BDD DCDB DBDB DBDB DADA C6C0 BFBF"            /* hb‹ÝÜÛÛÛÛÛÚÚÆÀ¿¿ */
	$"BEC3 5200 0000 0000 0000 2D59 524F 4D4A"            /* ¾ÃR.......-YROMJ */
	$"4843 487C 877E 7976 7372 5D49 1500 0000"            /* HCH|‡~yvsr]I.... */
	$"405F 5F63 6A6E 3400 0000 0061 F7FF FFFF"            /* @__cjn4....a÷ÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFD DDDB F6FF FFFF FFFF FFFF FFFF FFFF"            /* ÿýÝÛöÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF7A 0026 6B69"            /* ÿÿÿÿÿÿÿÿÿÿÿz.&ki */
	$"6862 A0E1 DADA D9DA DADA DBD5 C2C1 C1C0"            /* hb áÚÚÙÚÚÚÛÕÂÁÁÀ */
	$"C78D 0400 0000 0000 000E 5458 5452 4F4C"            /* Ç........TXTROL */
	$"4946 7891 8882 7C79 7574 5F4A 1500 0000"            /* IFx‘ˆ‚|yut_J.... */
	$"4160 6165 6970 7638 0000 34F5 FFFF FFFF"            /* A`aeipv8..4õÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF E3E5 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿãåÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FF55 0034 6F69"            /* ÿÿÿÿÿÿÿÿÿÿÿU.4oi */
	$"6763 B5E2 E0E3 E5E4 E2DF DBC9 BFC1 C1C2"            /* gcµâàãåäâßÛÉ¿ÁÁÂ */
	$"C43A 0000 0000 0000 0544 5D57 5553 514E"            /* Ä:.......D]WUSQN */
	$"476E 9A90 8B85 7F7B 7776 604B 1500 0000"            /* Gnš‹….{wv`K.... */
	$"4262 6267 6B6F 777F 3E00 A6FF FFFF FFFF"            /* Bbbgkow.>.¦ÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF E7EF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿçïÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FC32 003E 6F62"            /* ÿÿÿÿÿÿÿÿÿÿü2.>ob */
	$"6785 DFFA FDFF FFFF FEFC F2DB C9BF BEC7"            /* g…ßúýÿÿÿþüòÛÉ¿¾Ç */
	$"A206 0000 0000 0000 2562 5B59 5754 524B"            /* ¢.......%b[YWTRK */
	$"589B 9D93 8E88 827D 7977 604B 1500 0000"            /* X›“Žˆ‚}yw`K.... */
	$"4363 6369 6E72 777F 7F72 F5FF FFFF FFFF"            /* Cccinrw..rõÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF EBF9 FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿëùÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF E610 0041 6990"            /* ÿÿÿÿÿÿÿÿÿÿæ..Ai */
	$"D8FB FFFF FFFF FFFF FFFF FFFF FBE9 CEC6"            /* ØûÿÿÿÿÿÿÿÿÿÿûéÎÆ */
	$"4F00 0000 0000 0005 4F61 5C5B 5856 505A"            /* O.......Oa\[XVPZ */
	$"98A8 9E97 928B 8580 7B79 634C 1600 0000"            /* ˜¨ž—’‹…€{ycL.... */
	$"4466 666A 7074 7A7F 81D7 FFFF FFFF FFFF"            /* Dffjptz.×ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF F6FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿöÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF B901 0051 BFFE"            /* ÿÿÿÿÿÿÿÿÿÿ¹..Q¿þ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FEE5"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿþå */
	$"2E00 0000 0000 002E 655F 5D5C 5A56 5698"            /* ........e_]\ZVV˜ */
	$"B5A9 A29B 968F 8884 7E7C 624D 1600 0000"            /* µ©¢›–ˆ„~|bM.... */
	$"4567 686D 7176 7C80 8FF5 FFFF FFFF FFFF"            /* Eghmqv|€õÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF 8400 0BCE FFFF"            /* ÿÿÿÿÿÿÿÿÿÿ„..Îÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"DA23 0000 0000 085A 645F 5E5D 5A52 86BD"            /* Ú#.....Zd_^]ZR†½ */
	$"B3AC A59F 9992 8B86 8372 5451 1600 0000"            /* ³¬¥Ÿ™’‹†ƒrTQ.... */
	$"4768 686E 7478 7E7E ADFF FFFF FFFF FFFF"            /* Ghhntx~~­ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFF7 3400 88FF FFFF"            /* ÿÿÿÿÿÿÿÿÿ÷4.ˆÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFC2 1700 0000 2E67 6261 5F5E 566A B8BD"            /* ÿÂ.....gba_^Vj¸½ */
	$"B5AF A8A2 9C95 8E8B 7F5D 5452 1700 0000"            /* µ¯¨¢œ•Ž‹.]TR.... */
	$"4772 736F 767B 8088 E1FF FFFF FFFF FFFF"            /* Grsov{€ˆáÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFC7 013B FBFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÇ.;ûÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF C109 0013 5E65 6261 605C 65AF C7BD"            /* ÿÿÁÆ..^eba`\e¯Ç½ */
	$"B8B3 ACA6 A099 9489 6859 5854 1700 0000"            /* ¸³¬¦ ™”‰hYXT.... */
	$"467B 997E 747C 7EA6 FFFF FFFF FFFF FFFF"            /* F{™~t|~¦ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF8E 00AD FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿŽ.­ÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF5B 004E 6A64 6362 5F5F AECE C4C0"            /* ÿÿÿ[.Njdcb__®ÎÄÀ */
	$"BBB6 AFA9 A39E 9270 615E 5956 1700 0000"            /* »¶¯©£ž’pa^YV.... */
	$"477B A0A0 8A7E 7DC5 FFFF FFFF FFFF FFFF"            /* G{  Š~}Åÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FF53 10EF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿS.ïÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFA7 1866 6664 6462 5B8F D1CB C6C1"            /* ÿÿÿ§.ffddb[ÑËÆÁ */
	$"BEB9 B2AC A79F 7D6A 6560 5C57 1800 0000"            /* ¾¹²¬§Ÿ}je`\W.... */
	$"497D A0A6 AC9F 8EE6 FFFF FFFF FFFF FFFF"            /* I} ¦¬ŸŽæÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF EF11 54FF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿï.Tÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFE8 6167 6564 645E 7ACD D1CC C8C4"            /* ÿÿÿèagedd^zÍÑÌÈÄ */
	$"C0BC B5B1 AA8C 746F 6963 5F5A 1800 0000"            /* À¼µ±ªŒtoic_Z.... */
	$"4A7F A1A7 ACAF BDFD FFFF FFFF FFFF FFFF"            /* J.¡§¬¯½ýÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF B900 9CFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿ¹.œÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF 9460 6766 616A C0D7 CFCD CAC6"            /* ÿÿÿÿ”`gfajÀ×ÏÍÊÆ */
	$"C2BE B9B4 9880 7973 6D66 615D 1A00 0000"            /* Â¾¹´˜€ysmfa].... */
	$"4C80 A2A7 ACAC D2FF FFFF FFFF FFFF FFFF"            /* L€¢§¬¬Òÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 8300 C8FF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿƒ.Èÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF B05E 6764 62A7 DBD3 D1CF CBC8"            /* ÿÿÿÿ°^gdb§ÛÓÑÏËÈ */
	$"C4C0 BCA2 8B84 7D76 7069 645F 1A00 0000"            /* ÄÀ¼¢‹„}vpid_.... */
	$"4D82 A3A9 ADAF E4FF FFFF FFFF FFFF FFFF"            /* M‚£©­¯äÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF 4805 DFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿH.ßÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF CF64 6560 9CDC D6D4 D2D0 CDCA"            /* ÿÿÿÿÏde`œÜÖÔÒÐÍÊ */
	$"C7C4 AE94 9088 817A 736C 6665 1B00 0000"            /* ÇÄ®”ˆzslfe.... */
	$"4E84 A4AA ADB7 F6FF FFFF FFFF FFFF FFFF"            /* N„¤ª­·öÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFEB 1210 E4FF FFFF FFFF"            /* ÿÿÿÿÿÿÿë..äÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF EC70 5F86 D8DA D6D5 D3D1 CFCC"            /* ÿÿÿÿìp_†ØÚÖÕÓÑÏÌ */
	$"CAB8 9F98 938C 857E 7770 6F51 0C00 0000"            /* Ê¸Ÿ˜“Œ…~wpoQ.... */
	$"4F85 A5AB ADC3 FEFF FFFF FFFF FFFF FFFF"            /* O…¥«­Ãþÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFBE 0016 E5FF FFFF FFFF"            /* ÿÿÿÿÿÿÿ¾..åÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FB7D 69CA DDD7 D6D5 D4D2 D0CE"            /* ÿÿÿÿû}iÊÝ×ÖÕÔÒÐÎ */
	$"C1A7 A19B 978F 8882 7C78 4609 0000 0000"            /* Á§¡›—ˆ‚|xFÆ.... */
	$"5087 A6AC ADD1 FFFF FFFF FFFF FFFF FFFF"            /* P‡¦¬­Ñÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF90 0019 E7FF FFFF FFFF"            /* ÿÿÿÿÿÿÿ..çÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF93 A0DF D9D8 D7D6 D4D3 D2C6"            /* ÿÿÿÿÿ“ ßÙØ×ÖÔÓÒÆ */
	$"AFA8 A49E 9992 8A87 8347 0200 0000 0000"            /* ¯¨¤ž™’Š‡ƒG...... */
	$"5189 A7AD AFE2 FFFF FFFF FFFF FFFF FFFF"            /* Q‰§­¯âÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FF57 001D EAFF FFFF FFFF"            /* ÿÿÿÿÿÿÿW..êÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFDA D6DB D9D9 D8D7 D5D5 CCB5"            /* ÿÿÿÿÿÚÖÛÙÙØ×ÕÕÌµ */
	$"AEAC A6A1 9B95 9288 3E00 0000 0000 0000"            /* ®¬¦¡›•’ˆ>....... */
	$"548A A8AD B4EF FFFF FFFF FFFF FFFF FFFF"            /* TŠ¨­´ïÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF F925 0021 EDFF FFFF FFFF"            /* ÿÿÿÿÿÿù%.!íÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFF8 DCDA DAD9 D8D7 D7D1 BAB3"            /* ÿÿÿÿÿøÜÚÚÙØ××Ñº³ */
	$"B0AD A8A4 9E9F 8628 0000 0000 0000 0000"            /* °­¨¤žŸ†(........ */
	$"4E90 ABAD BBF9 FFFF FFFF FFFF FFFF FFFF"            /* N«­»ùÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF C104 0024 F0FF FFFF FFFF"            /* ÿÿÿÿÿÿÁ..$ðÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFB DED9 DAD9 D8D9 D5C1 B6B4"            /* ÿÿÿÿÿûÞÙÚÙØÙÕÁ¶´ */
	$"B2AE AAA6 A988 2200 0000 0000 0000 0000"            /* ²®ª¦©ˆ"......... */
	$"0B4A A3B2 CFFF FFFF FFFF FFFF FFFF FFFF"            /* .J£²Ïÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 6B00 0029 F3FF FFFF FFFF"            /* ÿÿÿÿÿÿk..)óÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E3D9 DAD9 DAD5 C2BA B9B6"            /* ÿÿÿÿÿÿãÙÚÙÚÕÂº¹¶ */
	$"B3AF AEB2 7E16 0000 0000 0000 0000 0000"            /* ³¯®²~........... */
	$"000F 5B8A DEFF FFFF FFFF FFFF FFFF FFFF"            /* ..[ŠÞÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFD 3B00 0030 F8FF FFFF FFFF"            /* ÿÿÿÿÿý;..0øÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E7D9 DADA D8C4 BCBC B9B8"            /* ÿÿÿÿÿÿçÙÚÚØÄ¼¼¹¸ */
	$"B5B5 AF5A 0A00 0000 0000 0000 0000 0000"            /* µµ¯ZÂ........... */
	$"0016 4D51 CDFF FFFF FFFF FFFF FFFF FFFF"            /* ..MQÍÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFF2 2000 003D FDFF FFFF FFFF"            /* ÿÿÿÿÿò ..=ýÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EAD9 DBDB CABD BDBD BAB8"            /* ÿÿÿÿÿÿêÙÛÛÊ½½½º¸ */
	$"BBBA 5500 0000 0000 0003 0A00 0000 0000"            /* »ºU.......Â..... */
	$"0016 5056 D7FF FFFF FFFF FFFF FFFF FFFF"            /* ..PV×ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFE3 1700 0050 FFFF FFFF FFFF"            /* ÿÿÿÿÿã...Pÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EEDA DBCC C0BF BDBD BCC2"            /* ÿÿÿÿÿÿîÚÛÌÀ¿½½¼Â */
	$"B34A 0000 0000 0000 0939 2600 0000 0000"            /* ³J......Æ9&..... */
	$"0016 4E5B E8FF FFFF FFFF FFFF FFFF FFFF"            /* ..N[èÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFCF 0F00 0061 FFFF FFFF FFFF"            /* ÿÿÿÿÿÏ...aÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF F0D9 CCBF C1C0 BFBE C592"            /* ÿÿÿÿÿÿðÙÌ¿ÁÀ¿¾Å’ */
	$"2A00 0000 0000 001B 4556 2500 0000 0000"            /* *.......EV%..... */
	$"0016 4E5D F0FF FFFF FFFF FFFF FFFF FFFF"            /* ..N]ðÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFC3 0C00 0077 FFFF FFFF FFFF"            /* ÿÿÿÿÿÃ...wÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF F3CE C0C1 C1C0 C0CA 8C0C"            /* ÿÿÿÿÿÿóÎÀÁÁÀÀÊŒ. */
	$"0000 0000 0002 3656 5352 2500 0000 0000"            /* ......6VSR%..... */
	$"0016 4D64 F6FF FFFF FFFF FFFF FFFF FFFF"            /* ..Mdöÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFB5 0A00 0094 FFFF FFFF FFFF"            /* ÿÿÿÿÿµÂ..”ÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EEC2 C1C2 C1C4 C98A 1200"            /* ÿÿÿÿÿÿîÂÁÂÁÄÉŠ.. */
	$"0000 0000 0A3C 5B55 5254 2600 0000 0000"            /* ....Â<[URT&..... */
	$"0016 4D69 F8FF FFFF FFFF FFFF FFFF FFFF"            /* ..Miøÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FCAC 0900 02C2 FFFF FFFF FFFF"            /* ÿÿÿÿü¬Æ..Âÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF ECC1 C2C2 C6BB 5806 0000"            /* ÿÿÿÿÿÿìÁÂÂÆ»X... */
	$"0000 0023 505F 5754 5254 2600 0000 0000"            /* ...#P_WTRT&..... */
	$"0017 506C F8FF FFFF FFFF FFFF FFFF FFFF"            /* ..Pløÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F6A4 0300 1EF1 FFFF FFFF FFFF"            /* ÿÿÿÿö¤...ñÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EDC2 C2C8 C13B 0000 0000"            /* ÿÿÿÿÿÿíÂÂÈÁ;.... */
	$"0000 3963 605B 5855 5354 2700 0000 0000"            /* ..9c`[XUST'..... */
	$"0008 2C5E F7FF FFFF FFFF FFFF FFFF FFFF"            /* ..,^÷ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F1A5 0001 7FFF FFFF FFFF FFFF"            /* ÿÿÿÿñ¥...ÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EDC2 CBB9 4400 0000 0000"            /* ÿÿÿÿÿÿíÂË¹D..... */
	$"0B40 6560 5D5C 5956 5455 2400 0000 0000"            /* .@e`]\YVTU$..... */
	$"0000 0016 E9FF FFFF FFFF FFFF FFFF FFFF"            /* ....éÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FEEB B6BE FCFF FFFF FFFF FFFF"            /* ÿÿÿÿþë¶¾üÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF F0CC 8F24 0000 0000 0027"            /* ÿÿÿÿÿÿðÌ$.....' */
	$"5A68 625F 5E5C 5957 5250 3237 1C00 0000"            /* Zhb_^\YWRP27.... */
	$"0000 0005 D1FF FFFF FFFF FFFF FFFF FFFF"            /* ....Ñÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF F274 0700 0000 0006 4068"            /* ÿÿÿÿÿÿòt......@h */
	$"6763 6260 5E5C 5952 5781 9487 2600 0000"            /* gcb`^\YRW”‡&... */
	$"0000 0000 B4FF FFFF FFFF FFFF FFFF FFFF"            /* ....´ÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF C200 0000 0000 0E49 6D66"            /* ÿÿÿÿÿÿÂ......Imf */
	$"6463 6260 5F58 576A 9FBD A182 2400 0000"            /* dcb`_XWjŸ½¡‚$... */
	$"0000 0000 8EFF FFFF FFFF FFFF FFFF FFFF"            /* ....Žÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF A900 0000 0027 5F6B 6765"            /* ÿÿÿÿÿÿ©....'_kge */
	$"6463 625D 5A70 A1BE BEB7 9E82 2400 0000"            /* dcb]Zp¡¾¾·ž‚$... */
	$"0000 0000 38F8 FFFF FFFF FFFF FFFF FFFF"            /* ....8øÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 9A00 0008 406B 6A67 6766"            /* ÿÿÿÿÿÿš...@kjggf */
	$"6562 5E74 9EC4 CAC1 BBB8 9E82 2400 0000"            /* eb^tžÄÊÁ»¸ž‚$... */
	$"0000 0000 01C7 FFFF FFFF FFFF FFFF FFFF"            /* .....Çÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 8800 1B5A 6E69 6767 6764"            /* ÿÿÿÿÿÿˆ..Znigggd */
	$"5E66 A0CC D1CA C4C0 BBB7 9D82 2400 0001"            /* ^f ÌÑÊÄÀ»·‚$... */
	$"3A37 0F00 007E FFFF FFFF FFFF FFFF FFFF"            /* :7...~ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 711B 626E 6968 6866 6168"            /* ÿÿÿÿÿÿq.bnihhfah */
	$"87B9 D6D0 CBC8 C4BF BAB8 9D81 2300 0000"            /* ‡¹ÖÐËÈÄ¿º¸#... */
	$"6694 845F 3031 ECFF FFFF FFFF FFFF FFFF"            /* f”„_01ìÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 8360 6C69 6967 6265 88BE"            /* ÿÿÿÿÿÿƒ`liigbeˆ¾ */
	$"D8D7 D0CD CBC7 C4C0 BCB3 9780 2300 0000"            /* Ø×ÐÍËÇÄÀ¼³—€#... */
	$"6090 929D 9D88 C9FF FFFF FFFF FFFF FFFF"            /* `’ˆÉÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFB 8566 6969 6666 88B7 DADB"            /* ÿÿÿÿÿû…fiiffˆ·ÚÛ */
	$"D5D3 D0CD CAC8 C4BB A994 8880 2300 0000"            /* ÕÓÐÍÊÈÄ»©”ˆ€#... */
	$"5F8F 8F95 9BA0 AFF9 FFFF FFFF FFFF FFFF"            /* _•› ¯ùÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFE5 7067 6663 70AA DCDE D8D6"            /* ÿÿÿÿÿåpgfcpªÜÞØÖ */
	$"D4D3 D0CE CCC3 B19E 928B 8680 2200 0000"            /* ÔÓÐÎÌÃ±ž’‹†€"... */
	$"5F8E 8F95 9A9F A0C7 FDFF FFFF FFFF FFFF"            /* _Ž•šŸ Çýÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFBA 6163 739C CBDF DAD8 D7D6"            /* ÿÿÿÿÿºacsœËßÚØ×Ö */
	$"D5D4 CFC3 B6A7 9D98 928B 857E 2200 0000"            /* ÕÔÏÃ¶§˜’‹…~"... */
	$"5F8E 8F95 9A9E A3A4 C2F9 FFFF FFFF FFFF"            /* _Ž•šž£¤Âùÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FD88 649A CFE1 DDDA D8D8 D7D6"            /* ÿÿÿÿýˆdšÏáÝÚØØ×Ö */
	$"D1C5 B5AB A6A2 9C97 9188 827C 2100 0000"            /* ÑÅµ«¦¢œ—‘ˆ‚|!... */
	$"5F8E 8E94 9A9E A3A7 A5C6 FAFF FFFF FFFF"            /* _ŽŽ”šž£§¥Æúÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF EE8A B7E2 DEDA DAD9 D9D9 D4C8"            /* ÿÿÿÿîŠ·âÞÚÚÙÙÙÔÈ */
	$"B9B1 ADAA A6A1 9A95 8F87 817A 2000 0000"            /* ¹±­ª¦¡š•‡z ... */
	$"5F8D 8C92 999D A3A7 ABAA C1E9 FDFF FFFF"            /* _Œ’™£§«ªÁéýÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF EDD7 E0DB DBDA DBD9 D2C8 BDB7"            /* ÿÿÿÿí×àÛÛÚÛÙÒÈ½· */
	$"B4B2 ADA8 A49F 9994 8D86 807B 2100 0000"            /* ´²­¨¤Ÿ™”†€{!... */
	$"5E93 9F9E 9C9D A1A5 A9AD AFB3 C6E4 F9FF"            /* ^“Ÿžœ¡¥©­¯³Æäùÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF E6DB DBDC DCD9 D0C4 BCB9 B9B6"            /* ÿÿÿÿæÛÛÜÜÙÐÄ¼¹¹¶ */
	$"B3B0 ACA7 A39C 9792 8E89 7D6A 1900 0000"            /* ³°¬§£œ—’Ž‰}j.... */
	$"5C96 B2B7 B8B8 B7B1 ADAD B0B1 B2B6 C2D1"            /* \–²·¸¸·±­­°±²¶ÂÑ */
	$"E9FE FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* éþÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFF9 DCDC DDD9 CCC2 BDBC BCBA B8B6"            /* ÿÿÿùÜÜÝÙÌÂ½¼¼º¸¶ */
	$"B3AF AAA6 A4A1 9D93 7A48 210B 0000 0000"            /* ³¯ª¦¤¡“zH!..... */
	$"5B95 AFB6 BABF C4C7 C6C4 C1BE BCB9 B8B6"            /* [•¯¶º¿ÄÇÆÄÁ¾¼¹¸¶ */
	$"BACA DBEB FBFF FFFF FFFF FFFF FFFF FFFF"            /* ºÊÛëûÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFED DAD7 CEC4 BEBE BDBD BBBA B7B5"            /* ÿÿÿíÚ×ÎÄ¾¾½½»º·µ */
	$"B4B3 B2A9 9979 502F 0F00 0000 0000 0000"            /* ´³²©™yP/........ */
	$"5A94 AEB4 B9BD C2C6 CACD D0D2 D2D1 CDC9"            /* Z”®´¹½ÂÆÊÍÐÒÒÑÍÉ */
	$"C5BF BBBE C6DB F0FF FFFF FFFF FFFF FFFF"            /* Å¿»¾ÆÛðÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF F8D5 C9C2 BFBF BFBE BDBC BBBD C0BA"            /* ÿÿøÕÉÂ¿¿¿¾½¼»½Àº */
	$"AB90 643A 1A02 0000 0000 0000 0000 0000"            /* «d:............ */
	$"5A93 ADB3 B8BC C1C5 C8CB CED0 D2D5 D7D8"            /* Z“­³¸¼ÁÅÈËÎÐÒÕ×Ø */
	$"D8D7 D5D1 CBC4 C4D9 FBFF FFFF FFFF FFFF"            /* Ø×ÕÑËÄÄÙûÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFF7 CEBF C0C1 C0C0 BFBF C1C5 BEA3 7747"            /* ÿ÷Î¿ÀÁÀÀ¿¿ÁÅ¾£wG */
	$"2206 0000 0000 0005 1324 1500 0000 0000"            /* "........$...... */
	$"5A91 ACB2 B7BB C0C4 C8CB CED0 D2D4 D5D6"            /* Z‘¬²·»ÀÄÈËÎÐÒÔÕÖ */
	$"D7D9 DADB DBDA D8CE E7FF FFFF FFFF FFFF"            /* ×ÙÚÛÛÚØÎçÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFDA BCC1 C1C2 C3C8 C8BF A87E 4312 0000"            /* ÿÚ¼ÁÁÂÃÈÈ¿¨~C... */
	$"0000 0004 0E1E 314A 5458 2900 0000 0000"            /* ......1JTX)..... */
	$"5991 ABB1 B6BA BFC4 C7CA CDCF D2D4 D5D6"            /* Y‘«±¶º¿ÄÇÊÍÏÒÔÕÖ */
	$"D7D8 D9D9 DADA DCDB E2FF FFFF FFFF FFFF"            /* ×ØÙÙÚÚÜÛâÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFEC C7C4 CBC4 B48E 5C33 0E00 0000 010F"            /* ÿìÇÄËÄ´Ž\3...... */
	$"1E2F 4052 5B5D 5D59 5455 2600 0000 0000"            /* ./@R[]]YTU&..... */
	$"5A91 AAB0 B5B9 BEC3 C5C9 CCCF D1D3 D5D6"            /* Z‘ª°µ¹¾ÃÅÉÌÏÑÓÕÖ */
	$"D7D8 D9D9 DADA DBD9 E6FF FFFF FFFF FFFF"            /* ×ØÙÙÚÚÛÙæÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF F7AB 653A 1600 0000 071E 3341 5360"            /* ÿÿ÷«e:......3AS` */
	$"6568 6663 5F5C 5855 5254 2600 0000 0000"            /* ehfc_\XURT&..... */
	$"4C86 A6B0 B8BD C3C6 C8CB CDCF D1D3 D4D5"            /* L†¦°¸½ÃÆÈËÍÏÑÓÔÕ */
	$"D7D8 D9D9 DADA DBD9 EBFF FFFF FFFF FFFF"            /* ×ØÙÙÚÚÛÙëÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFB3 0000 0218 3147 5E6A 6C6D 6965"            /* ÿÿÿ³....1G^jlmie */
	$"6463 615F 5D5A 5855 5253 2600 0000 0000"            /* dca_]ZXURS&..... */
	$"0421 5B6A 7985 94A3 AFBD C7CD D3D7 DADB"            /* .![jy…”£¯½ÇÍÓ×ÚÛ */
	$"DBDB DBDB DBDB DAD9 EFFF FFFF FFFF FFFF"            /* ÛÛÛÛÛÛÚÙïÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF D796 ECFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿ×–ìÿ */
	$"FFFF FFFF 8647 5D69 6E6D 6A68 6766 6664"            /* ÿÿÿÿ†G]inmjhgffd */
	$"6362 615F 5D5A 5754 5153 2500 0000 0000"            /* cba_]ZWTQS%..... */
	$"0010 4245 474A 4F55 5A63 6E7B 8795 A2AC"            /* ..BEGJOUZcn{‡•¢¬ */
	$"B9C7 D0D5 DADE E0DF F4FF FFFF FFFF FFFF"            /* ¹ÇÐÕÚÞàßôÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF DE8B 1709 9DFF"            /* ÿÿÿÿÿÿÿÿÿÿÞ‹.Æÿ */
	$"FFFF FFFF F67D 686A 6968 6867 6766 6564"            /* ÿÿÿÿö}hjihhggfed */
	$"6362 605E 5C59 5553 5051 2400 0000 0000"            /* cb`^\YUSPQ$..... */
	$"0012 4448 4B4E 5154 5558 595A 5A5C 5E61"            /* ..DHKNQTUXYZZ\^a */
	$"656B 717B 8895 9FB6 F0FF FFFF FFFF FFFF"            /* ekq{ˆ•Ÿ¶ðÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF EC9C 6B5B 5860 80F8"            /* ÿÿÿÿÿÿÿÿìœk[X`€ø */
	$"FFFF FFFF FFA9 6069 6868 6767 6665 6463"            /* ÿÿÿÿÿ©`ihhggfedc */
	$"615E 5C59 5653 504E 4D50 2502 0200 0000"            /* a^\YVSPNMP%..... */
	$"0012 4246 494D 5053 5659 5B5D 5F61 6263"            /* ..BFIMPSVY[]_abc */
	$"6464 6564 6362 616E EAFF FFFF FFFF FFFF"            /* ddedcbanêÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF F8E7 885E 676A 6C68 6CE6"            /* ÿÿÿÿÿÿøçˆ^gjlhlæ */
	$"FFFF FFFF FFD2 6462 6262 6263 6566 6667"            /* ÿÿÿÿÿÒdbbbbceffg */
	$"6B6F 7274 787C 7C7D 7D80 6448 1600 0000"            /* kortx||}}€dH.... */
	$"0012 4145 484A 4E51 5457 5A5C 5E60 6263"            /* ..AEHJNQTWZ\^`bc */
	$"6465 6667 6767 6576 F6FF FFFF FFFF FFFF"            /* defgggevöÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFB F1E6 DCDE 995F 6463 625F 6DE9"            /* ÿÿÿûñæÜÞ™_dcb_mé */
	$"FFFF FFFF FFF3 9088 939A 9FA4 ABB1 B4B6"            /* ÿÿÿÿÿóˆ“šŸ¤«±´¶ */
	$"B8B9 B6B3 B0AC A6A1 9B95 785B 1900 0000"            /* ¸¹¶³°¬¦¡›•x[.... */
	$"0011 3E42 4548 4C50 5255 585A 5C5F 6062"            /* ..>BEHLPRUXZ\_`b */
	$"6365 6666 6767 6387 FDFF FFFF FFFF FFFF"            /* ceffggc‡ýÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FBF2 E8DE DAD9 DBE0 C381 888E 97AB D3FD"            /* ûòèÞÚÙÛàÃˆŽ—«Óý */
	$"FFFF FFFF FFFF E3D7 D9D9 D8D6 D2CF CBC6"            /* ÿÿÿÿÿÿã×ÙÙØÖÒÏËÆ */
	$"C2BD B7B2 ADA6 A09A 9591 7355 1800 0000"            /* Â½·²­¦ š•‘sU.... */
	$"0011 3F43 4649 4D51 5457 5A5D 5F61 6466"            /* ..?CFIMQTWZ]_adf */
	$"6769 6A6A 6C6D 6694 FFFF FFFF FFFF FFFF"            /* gijjlmf”ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFE F8ED"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿþøí */
	$"DED9 D9DA DADA DADA DAD9 DEE6 F5FF FFFF"            /* ÞÙÙÚÚÚÚÚÚÙÞæõÿÿÿ */
	$"FFFF FFFF FFFF E6CC CECC CAC8 C4C1 BEBA"            /* ÿÿÿÿÿÿæÌÎÌÊÈÄÁ¾º */
	$"B7B3 ADA8 A39D 9793 8D8A 6F54 1700 0000"            /* ·³­¨£—“ŠoT.... */
	$"000C 2C2E 3032 3436 3839 3A3B 3C3E 3F3F"            /* ..,.024689:;<>?? */
	$"3F3F 4040 3F40 3887 FFFF FFFF FFFF FFFF"            /* ??@@?@8‡ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFD F9F1 E7DE DAD8"            /* ÿÿÿÿÿÿÿÿÿýùñçÞÚØ */
	$"DADB DCDC DCDD DEE1 E8F3 FBFF FFFF FFFF"            /* ÚÛÜÜÜÝÞáèóûÿÿÿÿÿ */
	$"FFFF FFFF FFFF E5B4 B4B2 AFAB A7A3 9E98"            /* ÿÿÿÿÿÿå´´²¯«§£ž˜ */
	$"928E 8781 7B74 6F6A 6560 5B55 1700 0000"            /* ’Ž‡{toje`[U.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 029C FFFF FFFF FFFF FFFF"            /* .......œÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFC F6EF E8E2 E0E2 E5E9 EDF0"            /* ÿÿÿÿÿüöïèâàâåéíð */
	$"F3F5 F8F8 F9FA FCFE FFFF FFFF FFFF FFFF"            /* óõøøùúüþÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EAB5 B1AF ABA7 A39E 9893"            /* ÿÿÿÿÿÿêµ±¯«§£ž˜“ */
	$"8D87 807A 746E 6762 5E58 5654 1700 0000"            /* ‡€ztngb^XVT.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0009 4CC4 FFFF FFFF FFFF FFFF"            /* .....ÆLÄÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFA F5EF E7DE DEE5 F0F8 FCFE FFFF FFFF"            /* ÿúõïçÞÞåðøüþÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E6B4 B2AD AAA6 A19C 9690"            /* ÿÿÿÿÿÿæ´²­ª¦¡œ– */
	$"8B85 7D77 716B 6560 5C58 5552 1700 0000"            /* ‹…}wqke`\XUR.... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0856 76CB FFFF FFFF FFFF FFFF"            /* .....VvËÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FEEE E5E7 EAF3 FCFF FFFF FFFF FFFF FFFF"            /* þîåçêóüÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF CA8D A1AE A8A2 9E99 938C"            /* ÿÿÿÿÿÿÊ¡®¨¢ž™“Œ */
	$"8782 7B74 6E68 635F 5A56 5451 1600 0000"            /* ‡‚{tnhc_ZVTQ.... */
	$"0000 0000 0000 0000 0000 0000 0000 0409"            /* ...............Æ */
	$"090C 161E 5379 72D5 FFFF FFFF FFFF FFFF"            /* Æ...SyrÕÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF F6BB 7D6E 768E A3A1 9B97 9089"            /* ÿÿÿÿö»}nvŽ£¡›—‰ */
	$"837D 7671 6B65 605C 5854 5250 1600 0000"            /* ƒ}vqke`\XTRP.... */
	$"0000 0207 1012 1929 3645 5766 707D 8B95"            /* .......)6EWfp}‹• */
	$"9BA7 AF84 7471 76E3 FFFF FFFF FFFF FFFF"            /* ›§¯„tqvãÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFDA 806B 7173 726F 7F95 9697 938B"            /* ÿÿÿÚ€kqsro.•–—“‹ */
	$"857C 746E 6963 5D5A 5653 524F 1600 0000"            /* …|tnic]ZVSRO.... */
	$"3452 5966 7176 808C 949C A4A9 ACAF B2B4"            /* 4RYfqv€Œ”œ¤©¬¯²´ */
	$"B8B7 8C73 7371 79EC FFFF FFFF FFFF FFFF"            /* ¸·Œssqyìÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFE2 A273 6D73 7376 7473 7367 413E 4A54"            /* ÿâ¢smssvtssgA>JT */
	$"5D65 6A6A 6865 605B 5754 514F 1600 0000"            /* ]ejjhe`[WTQO.... */
	$"4C71 7177 7B80 868B 8F94 999E A2A7 ABAF"            /* Lqqw{€†‹”™ž¢§«¯ */
	$"B188 6F73 7372 7EF3 FFFF FFFF FFFF FFFF"            /* ±ˆossr~óÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFDC 9673 6F75 7762 7179 7373 674E 3529"            /* ÿÜ–souwbqyssgN5) */
	$"2016 141B 202A 353D 454A 4D4F 1600 0000"            /*  ... *5=EJMO.... */
	$"476B 6C71 777C 8188 8C91 979B A0A4 AAAD"            /* Gklqw|ˆŒ‘—› ¤ª­ */
	$"8970 7373 7471 61F4 FFFF FFFF FFFF FFFF"            /* ‰psstqaôÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFDB 8772 4007 205A 7974 7576 6C5C"            /* ÿÿÿÛ‡r@. Zytuvl\ */
	$"564F 4232 2416 0A04 0105 0F18 0900 0000"            /* VOB2$.Â.....Æ... */
	$"476A 6A70 757A 8086 8A90 959A 9EA4 A98E"            /* Gjjpuz€†Š•šž¤©Ž */
	$"7173 7374 7A3F 29F7 FFFF FFFF FFFF FFFF"            /* qsstz?)÷ÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF BB14 0000 0002 4878 7773 7569"            /* ÿÿÿÿ».....Hxwsui */
	$"5751 5151 4F49 4137 2314 0300 0000 0000"            /* WQQQOIA7#....... */
	$"4668 696E 7378 7D83 888E 9498 9DA2 8871"            /* Fhinsx}ƒˆŽ”˜¢ˆq */
	$"7373 737A 5001 35FD FFFF FFFF FFFF FFFF"            /* ssszP.5ýÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF D979 5928 0700 001F 6176 7475"            /* ÿÿÿÿÙyY(....avtu */
	$"705B 4B4A 4845 4241 3F3E 1B00 0000 0000"            /* p[KJHEBA?>...... */
	$"4566 666C 7175 7B81 858B 9095 9B87 7173"            /* Efflqu{…‹•›‡qs */
	$"7373 7A56 0600 46FF FFFF FFFF FFFF FFFF"            /* sszV..Fÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFF6 D3AA 7940 1F3D 666F"            /* ÿÿÿÿÿÿÿöÓªy@.=fo */
	$"7374 6049 4443 403E 3A3B 1B00 0000 0000"            /* st`IDC@>:;...... */
	$"4365 656A 6E72 787D 8289 919E 8C71 7373"            /* Ceejnrx}‚‰‘žŒqss */
	$"737A 5309 0000 58FF FFFF FFFF FFFF FFFF"            /* szSÆ..Xÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF E9C2 A68C"            /* ÿÿÿÿÿÿÿÿÿÿÿÿéÂ¦Œ */
	$"7570 7569 4B3E 3D3B 3939 1A00 0000 0000"            /* upuiK>=;99...... */
	$"4363 6268 6D74 7D87 8F98 A395 7672 7373"            /* Ccbhmt}‡˜£•vrss */
	$"7A5D 0A00 0000 62FF FFFF FFFF FFFF FFFF"            /* z]Â...bÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFA"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿú */
	$"E297 6C76 6D4C 3939 3738 1900 0000 0000"            /* â—lvmL9978...... */
	$"4165 6D77 7F85 8C92 979D 8E72 7273 7379"            /* Aemw.…Œ’—Žrrssy */
	$"610E 0000 0000 6AFF FFFF FFFF FFFF FFFF"            /* a.....jÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFCC DDFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÌÝÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFEF 8271 7673 5538 3437 1800 0000 0016"            /* ÿï‚qvsU847...... */
	$"6674 7577 7F84 8A8F 958C 7272 7373 796C"            /* ftuw.„Š•Œrrssyl */
	$"1200 0000 0000 6FFF FFFF FFFF FFFF FFFF"            /* ......oÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF6A 0F77"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿj.w */
	$"DCFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* Üÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF CF48 7278 7666 3932 1800 0000 336D"            /* ÿÿÏHrxvf92....3m */
	$"7974 7372 7276 7C86 8976 7273 7378 681E"            /* ytsrrv|†‰vrssxh. */
	$"0000 0000 0000 74FF FFFF FFFF FFFF FFFF"            /* ......tÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF96 2C19"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ–,. */
	$"2E5D A9FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* .]©ÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FF94 2B65 7A78 6840 1200 0000 4362"            /* ÿÿÿ”+ezxh@....Cb */
	$"7178 7A76 7472 7273 7372 7373 786B 1700"            /* qxzvtrrssrssxk.. */
	$"0000 0000 0000 85FF FFFF FFFF FFFF FFFF"            /* ......…ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFAE 7175"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ®qu */
	$"6C58 55D7 FFFF FFFF FFFF FFFF FFFF FFFF"            /* lXU×ÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFD7 0904 4B75 7B72 3E06 0000 0003"            /* ÿÿÿ×Æ.Ku{r>..... */
	$"152D 4764 777A 7674 7373 7377 6E1C 0000"            /* .-Gdwzvtssswn... */
	$"0000 0000 0000 95FF FFFF FFFF FFFF FFFF"            /* ......•ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFA4 6C74"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ¤lt */
	$"7476 718A F6FF FFFF FFFF FFFF FFFF FFFF"            /* tvqŠöÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFEC 9A19 001D 5C7B 7A5C 1500 0000"            /* ÿÿÿìš...\{z\.... */
	$"0000 0005 294A 6573 7979 7877 2D00 0000"            /* ....)Jesyyxw-... */
	$"0000 0000 0000 9EFF FFFF FFFF FFFF FFFF"            /* ......žÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFA0 6C73"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ ls */
	$"7373 736D 9AF6 FFFF FFFF FFFF FFFF FFFF"            /* sssmšöÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFC5 0D00 0851 767B 6716 0000"            /* ÿÿÿÿÿÅ...Qv{g... */
	$"0000 0000 0000 061C 3550 6628 0000 0000"            /* ........5Pf(.... */
	$"0000 0000 0000 A5FF FFFF FFFF FFFF FFFF"            /* ......¥ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF9F 7074"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿŸpt */
	$"7373 7373 6B95 FCFF FFFF FFFF FFFF FFFF"            /* ssssk•üÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 6700 0001 1D21 2418 0000"            /* ÿÿÿÿÿÿg....!$... */
	$"0000 0000 0000 0000 0000 0400 0000 0000"            /* ................ */
	$"0000 0000 0000 AAFF FFFF FFFF FFFF FFFF"            /* ......ªÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FE7C 5F73"            /* ÿÿÿÿÿÿÿÿÿÿÿÿþ|_s */
	$"7977 7477 5E81 FDFF FFFF FFFF FFFF FFFF"            /* ywtw^ýÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF 9E00 0000 0000 0000 0000"            /* ÿÿÿÿÿÿž......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 AFFF FFFF FFFF FFFF FFFF"            /* ......¯ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FE3E 051D"            /* ÿÿÿÿÿÿÿÿÿÿÿÿþ>.. */
	$"3662 7573 3EDA FFFF FFFF FFFF FFFF FFFF"            /* 6bus>Úÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF B400 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ´......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 B1FF FFFF FFFF FFFF FFFF"            /* ......±ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FE3F 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿþ?.. */
	$"0006 1D1E 3FFE FFFF FFFF FFFF FFFF FFFF"            /* ....?þÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF CB05 0000 0000 0000 0000"            /* ÿÿÿÿÿÿË......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 B1FF FFFF FFFF FFFF FFFF"            /* ......±ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FE40 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿþ@.. */
	$"0000 0000 34FC FFFF FFFF FFFF FFFF FFFF"            /* ....4üÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF E30C 0000 0000 0000 0000"            /* ÿÿÿÿÿÿã......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 B1FF FFFF FFFF FFFF FFFF"            /* ......±ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF43 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿC.. */
	$"0000 0000 0BDB FFFF FFFF FFFF FFFF FFFF"            /* .....Ûÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF EF0E 0000 0000 0000 0000"            /* ÿÿÿÿÿÿï......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 AEFF FFFF FFFF FFFF FFFF"            /* ......®ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF4E 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿN.. */
	$"0000 0000 0092 FFFF FFFF FFFF FFFF FFFF"            /* .....’ÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF A502 0000 0000 0000 0000"            /* ÿÿÿÿÿÿ¥......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 A9FF FFFF FFFF FFFF FFFF"            /* ......©ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF5A 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿZ.. */
	$"0000 0000 002B F5FF FFFF FFFF FFFF FFFF"            /* .....+õÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFF1 2600 0000 0000 0000 0000"            /* ÿÿÿÿÿñ&......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 A4FF FFFF FFFF FFFF FFFF"            /* ......¤ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF65 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿe.. */
	$"0000 0000 0000 73FF FFFF FFFF FFFF FFFF"            /* ......sÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF83 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿƒ.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 9AFF FFFF FFFF FFFF FFFF"            /* ......šÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF6A 0000"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿj.. */
	$"0000 0000 0000 00B5 FFFF FFFF FFFF FFFF"            /* .......µÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FD37 0000 0000 0000 0000 0000"            /* ÿÿÿÿý7.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 8FFF FFFF FFFF FFFF FFFF"            /* ......ÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF85 120B"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ….. */
	$"0601 0000 0000 001C DBFF FFFF FFFF FFFF"            /* ........Ûÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF4C 0000 0000 0000 0000 0000"            /* ÿÿÿÿÿL.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 103B 75AB DDFF FFFF FFFF FFFF FFFF"            /* ...;u«Ýÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFF7 E6D9"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿ÷æÙ */
	$"D0BE A798 856A 5835 5EEF FFFF FFFF FFFF"            /* Ð¾§˜…jX5^ïÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FF62 110F 0F08 0000 0000 0000"            /* ÿÿÿÿÿb.......... */
	$"0000 0000 0000 0000 0000 0000 0000 0820"            /* ...............  */
	$"7ABF F1FF FFFF FFFF FFFF FFFF FFFF FFFF"            /* z¿ñÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFC ECF6 FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿüìöÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFEF DAA9 5C1A 0000 0000 0000"            /* ÿÿÿÿÿïÚ©\....... */
	$"0000 0000 0000 0000 0000 0001 0C42 93B6"            /* .............B“¶ */
	$"CED2 D4D1 CED3 DAD9 D9DF E5E5 E4E9 F0F0"            /* ÎÒÔÑÎÓÚÙÙßååäéðð */
	$"F0F1 F3F3 F3F5 F6F6 F6F9 FAFA FAFC FDFD"            /* ðñóóóõöööùúúúüýý */
	$"FDFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ýÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ */
	$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FBE2"            /* ÿÿÿÿÿÿÿÿÿÿÿÿÿÿûâ */
	$"BFA3 846A 5239 1F0F 0100 0000 0000 0000"            /* ¿£„jR9.......... */
	$"0000 0000 0000 0000 0000 0000 040F 1209"            /* ...............Æ */
	$"0303 0605 0507 0909 090B 0C0C 0C0D 0F0F"            /* ......ÆÆÆ....... */
	$"0F15 1C1B 1A21 2726 262E 3231 313A 3D3C"            /* .....!'&&.211:=< */
	$"3D46 4848 4952 5353 535D 5E5E 5F69 6A69"            /* =FHHIRSSS]^^_iji */
	$"6B74 7574 7880 8080 848B 8B89 99DD FFFF"            /* ktutx€€€„‹‹‰™Ýÿÿ */
	$"FFFF FFFF FFFB C8A3 A0A4 ADAC ACAC AFAE"            /* ÿÿÿÿÿûÈ£ ¤­¬¬¬¯® */
	$"ACB1 D0DC E3E6 E6E4 D8D4 C1A4 8762 350E"            /* ¬±ÐÜãææäØÔÁ¤‡b5. */
	$"0100 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 000F 4984"            /* ..............I„ */
	$"A7AF A588 6635 0400 0000 0000 0000 0000"            /* §¯¥ˆf5.......... */
	$"0000 060A 0E0F 110E 0A07 0200 0000 0000"            /* ...Â....Â....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000"                 /* .............. */
};

