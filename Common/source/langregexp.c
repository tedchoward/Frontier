
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

#include "frontier.h"
#include "standard.h"

#ifdef flregexpverbs

#include "error.h"
#include "memory.h"
#include "ops.h"
#include "resources.h"
#include "strings.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "process.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "op.h"
#include "oplist.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "search.h"

#include "langregexp.h"

/*
	How to add the files from the Perl-Compatible Regular Expression (PCRE)
	library to the CodeWarrior project for Frontier on Mac OS X:
	
	1. Download the source code from http://pcre.org/ and
	decompress the archive file
	
	2. Run configure and then make on OS X so that all the
	missing .h and .c files will be created
	
	3. Copy all the following files into the PCRE directory of
	the Frontier source code tree, change the end-of-line
	terminator to cr (Mac) and make the changes
	listed for each file:
	
	config.h
	- rename file to pcre_config.h
	- line 45: #define NEWLINE to '\x0D' (cr) instead of '\n'
	
	internal.h
	- rename file to pcre_internal.h
	- line 40: #include pcre_config instead of config.h
	- line 227: #undef TRUE and FALSE before defining them
		because they are also defined in Frontier's standard headers
	- line 659: #define POSIX_MALLOC_THRESHOLD to 10 to make
		pcre.c compile
	
	pcre.h
	- line 163: extend the function template for pcre_exec to
		match the call from regexpexechandle, i.e. add an
		int parameter for the match length and a pointer to a
		const unsigned char for the character table
	
	chartables.c
	[no changes]
	
	get.c:
	- line 43: #include pcre_internal.h instead of internal.h
	
	maketables.c:
	- line 42: #include pcre_internal.h instead of internal.h
	
	pcre.c:
	- line 52: #include pcre_internal.h instead of internal.h
	- line 2648, 4167, 4193, 4223: add space between closing
		parenthesis and semicolon to avoid compiler warning
	- line 7193: add comment about match_length parameter
	- line 7201: add comments about changes
	- line 7213: add an int parameter for the match length
		and a pointer to a const unsigned char for the character
		table
	- line 7222: define a const uschar ptr named stop_match
	- line 7263: initialize stop_match to
		match_block.start_subject + start_offset + match_length
	- line 7283, 7284, 7359: replace reference to re->tables
		with name of new character parameter
	- line 7379, 7383, 7393, 7402, 7526: replace end_subject
		with stop_match
	
	study.c:
	- line 39: #include pcre_internal.h instead of internal.h
	
	4. Add get.c, maketables.c, pcre.c, and study.c to the
	CodeWarrior project
	
	5. #define PCRE_STATIC before #including pcre.h anywhere
*/

#include "pcre_internal.h"


static unsigned char regexp_default_tables[] = {

#ifdef MACVERSION

/* This table is a lower casing table. */

	  0,  1,  2,  3,  4,  5,  6,  7,
	  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23,
	 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39,
	 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55,
	 56, 57, 58, 59, 60, 61, 62, 63,
	 64, 97, 98, 99,100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122, 91, 92, 93, 94, 95,
	 96, 97, 98, 99,100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122,123,124,125,126,127,
	138,140,141,142,150,154,159,135,
	136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,
	152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,
	168,169,170,171,172,173,190,191,
	176,177,178,179,180,181,182,183,
	184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,
	200,201,202,136,139,155,207,207,
	208,209,210,211,212,213,214,215,
	216,216,218,219,220,221,222,223,
	224,225,226,227,228,137,144,135,
	145,143,146,148,149,147,151,153,
	240,152,156,158,157,245,246,247,
	248,249,250,251,252,253,254,255,

/* This table is a case flipping table. */

	  0,  1,  2,  3,  4,  5,  6,  7,
	  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23,
	 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39,
	 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55,
	 56, 57, 58, 59, 60, 61, 62, 63,
	 64, 97, 98, 99,100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122, 91, 92, 93, 94, 95,
	 96, 65, 66, 67, 68, 69, 70, 71,
	 72, 73, 74, 75, 76, 77, 78, 79,
	 80, 81, 82, 83, 84, 85, 86, 87,
	 88, 89, 90,123,124,125,126,127,
	138,140,141,142,150,154,159,231,
	203,229,128,204,129,130,131,233,
	230,232,234,237,235,236,132,238,
	241,239,133,205,242,244,243,134,
	160,161,162,163,164,165,166,167,
	168,169,170,171,172,173,190,191,
	176,177,178,179,180,181,182,183,
	184,185,186,187,188,189,174,175,
	192,193,194,195,196,197,198,199,
	200,201,202,136,139,155,207,206,
	208,209,210,211,212,213,214,215,
	217,216,218,219,220,221,222,223,
	224,225,226,227,228,137,144,135,
	145,143,146,148,149,147,151,153,
	240,152,156,158,157,245,246,247,
	248,249,250,251,252,253,254,255,

/* This table contains bit maps for various character classes.
Each map is 32 bytes long and the bits run from the least
significant end of each byte. The classes that have their own
maps are: space, xdigit, digit, upper, lower, word, graph
print, punct, and cntrl. Other classes are built from combinations. */

	0x00,0x3E,0x00,0x00,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x03,
	0x7E,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x03,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFE,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,
	0x7F,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,
	0x00,0x78,0x00,0x02,0xE0,0xFF,0x1E,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x07,
	0x80,0xFF,0xFF,0xFF,0x80,0x00,0x00,0xC0,
	0x00,0x80,0x00,0xC1,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x03,
	0xFE,0xFF,0xFF,0x87,0xFE,0xFF,0xFF,0x07,
	0xFF,0xFF,0xFF,0xFF,0x80,0xC0,0x00,0xC0,
	0x00,0xF8,0x00,0xC3,0xE0,0xFF,0x1E,0x00,

	0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

	0x00,0x3E,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

	0x00,0x00,0x00,0x00,0xFE,0xFF,0x00,0xFC,
	0x01,0x00,0x00,0xF8,0x01,0x00,0x00,0x78,
	0x00,0x00,0x00,0x00,0x7F,0x3F,0xFF,0x3F,
	0xFF,0x03,0xFF,0x3C,0x1F,0x00,0xE1,0xFF,

	0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/* This table identifies various classes of character by individual bits:
  0x01   white space character
  0x02   letter
  0x04   decimal digit
  0x08   hexadecimal digit
  0x10   alphanumeric or '_'
  0x80   regular expression metacharacter or binary zero
*/

	0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x01,0x00,0x01,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x80,0x80,0x80,0x00,0x00,0x80,0x00,
	0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
	0x1C,0x1C,0x00,0x00,0x00,0x00,0x00,0x80,
	0x00,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x80,0x00,0x00,0x80,0x10,
	0x00,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x80,0x80,0x00,0x00,0x00,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,
	0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x12,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x12,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x01,0x12,0x12,0x12,0x12,0x12,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x12,0x12,0x00,0x00,0x00,0x00,0x12,0x12,
	0x00,0x00,0x00,0x00,0x00,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x00,0x12,0x12,0x12,0x12,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

#endif


#ifdef WIN95VERSION

/* This table is a lower casing table. */

	  0,  1,  2,  3,  4,  5,  6,  7,
	  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23,
	 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39,
	 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55,
	 56, 57, 58, 59, 60, 61, 62, 63,
	 64, 97, 98, 99,100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122, 91, 92, 93, 94, 95,
	 96, 97, 98, 99,100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122,123,124,125,126,127,
	128,129,130,131,132,133,134,135,
	136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,
	152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,
	168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,
	184,185,186,187,188,189,190,191,
	224,225,226,227,228,229,230,231,
	232,233,234,235,236,237,238,239,
	208,241,242,243,244,245,246,215,
	248,249,250,251,252,221,222,223,
	224,225,226,227,228,229,230,231,
	232,233,234,235,236,237,238,239,
	240,241,242,243,244,245,246,247,
	248,249,250,251,252,253,254,255,

/* This table is a case flipping table. */

	  0,  1,  2,  3,  4,  5,  6,  7,
	  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23,
	 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39,
	 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55,
	 56, 57, 58, 59, 60, 61, 62, 63,
	 64, 97, 98, 99,100,101,102,103,
	104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,
	120,121,122, 91, 92, 93, 94, 95,
	 96, 65, 66, 67, 68, 69, 70, 71,
	 72, 73, 74, 75, 76, 77, 78, 79,
	 80, 81, 82, 83, 84, 85, 86, 87,
	 88, 89, 90,123,124,125,126,127,
	128,129,130,131,132,133,134,135,
	136,137,138,139,140,141,142,143,
	144,145,146,147,148,149,150,151,
	152,153,154,155,156,157,158,159,
	160,161,162,163,164,165,166,167,
	168,169,170,171,172,173,174,175,
	176,177,178,179,180,181,182,183,
	184,185,186,187,188,189,190,191,
	224,225,226,227,228,229,230,231,
	232,233,234,235,236,237,238,239,
	208,241,242,243,244,245,246,215,
	248,249,250,251,252,221,222,223,
	192,193,194,195,196,197,198,199,
	200,201,202,203,204,205,206,207,
	240,209,210,211,212,213,214,247,
	216,217,218,219,220,253,254,141,

/* This table contains bit maps for various character classes.
Each map is 32 bytes long and the bits run from the least
significant end of each byte. The classes that have their own
maps are: space, xdigit, digit, upper, lower, word, graph
print, punct, and cntrl. Other classes are built from combinations. */

	0x00,0x3E,0x00,0x00,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x03,
	0x7E,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x03,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xFE,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,
	0x00,0x14,0x00,0x80,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0x7F,0x7F,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0x07,
	0x08,0x00,0x00,0x14,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x80,0xFF,0xFF,0x7F,0xFF,

	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x03,
	0xFE,0xFF,0xFF,0x87,0xFE,0xFF,0xFF,0x07,
	0x08,0x14,0x00,0x94,0x00,0x00,0x00,0x00,
	0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0x7F,0xFF,

	0x00,0x00,0x00,0x00,0xFE,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

	0x00,0x3E,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

	0x00,0x00,0x00,0x00,0xFE,0xFF,0x00,0xFC,
	0x01,0x00,0x00,0xF8,0x01,0x00,0x00,0x78,
	0xF7,0xEB,0xFF,0x6B,0xFF,0xFF,0xFF,0xFF,
	0x00,0x00,0x80,0x00,0x00,0x00,0x80,0x00,

	0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

/* This table identifies various classes of character by individual bits:
  0x01   white space character
  0x02   letter
  0x04   decimal digit
  0x08   hexadecimal digit
  0x10   alphanumeric or '_'
  0x80   regular expression metacharacter or binary zero
*/

	0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x01,0x00,0x01,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
	0x80,0x80,0x80,0x80,0x00,0x00,0x80,0x00,
	0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
	0x1C,0x1C,0x00,0x00,0x00,0x00,0x00,0x80,
	0x00,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x80,0x00,0x00,0x80,0x10,
	0x00,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x80,0x80,0x00,0x00,0x00,
	0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x00,
	0x00,0x00,0x12,0x00,0x12,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x12,0x00,0x12,0x00,0x00,0x12,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x00,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x00,
	0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12

#endif

	};

/* pascal strings for regexp.compile */

#define STR_flCaseSensitive			"\x0f" "flCaseSensitive"
#define STR_flDotMatchesAll			"\x0f" "flDotMatchesAll"
#define STR_flMultiLine				"\x0b" "flMultiLine"
#define STR_flAutoCapture			"\x0d" "flAutoCapture"
#define STR_flGreedyQuantifiers		"\x13" "flGreedyQuantifiers"
#define STR_flMatchEmptyString		"\x12" "flMatchEmptyString"
#define STR_flExtendedMode			"\x0e" "flExtendedMode"

/* pascal strings for regexp.match */

#define STR_ix						"\x02" "ix"
#define STR_ct						"\x02" "ct"
#define STR_adrMatchInfoTable		"\x11" "adrMatchInfoTable"
#define STR_flMakeGroups			"\x0c" "flMakeGroups"
#define STR_flMakeNamedGroups		"\x11" "flMakeNamedGroups"

/* pascal strings for the match info table */

#define STR_matchOffset				"\x0b" "matchOffset"
#define STR_matchLength				"\x0b" "matchLength"
#define STR_matchString				"\x0b" "matchString"
#define STR_groupOffsets			"\x0c" "groupOffsets"
#define STR_groupLengths			"\x0c" "groupLengths"
#define STR_groupStrings			"\x0c" "groupStrings"
#define STR_namedGroups				"\x0b" "namedGroups"
#define STR_groupNumber				"\x0b" "groupNumber"

/* pascal strings for regexp.replace */

#define STR_maxReplacements			"\x0f" "maxReplacements"
#define STR_adrReplacementCount		"\x13" "adrReplacementCount"
#define STR_adrCallback				"\x0b" "adrCallback"
#define STR_matchInfo				"\x09" "matchInfo"
#define STR_replacementString		"\x11" "replacementString"

/* pascal strings for regexp.extract */

#define STR_groups					"\x06" "groups"

/* pascal strings for regexp.split */

#define STR_maxSplits				"\x09" "maxSplits"

/* pascal strings for regexp.visit */

#define STR_maxRuns					"\x07" "maxRuns"
#define STR_matchInfoTable			"\x0e" "matchInfoTable"

/* pascal strings for regexp.grep */

#define STR_flIncludeMatches		"\x10" "flIncludeMatches"

/* pascal strings for regexp.getPatternInfo */

#define STR_backRefMax				"\x0a" "backRefMax"
#define STR_captureCount			"\x0c" "captureCount"
#define STR_firstByte				"\x09" "firstByte"
#define STR_firstByteTable			"\x0e" "firstByteTable"
#define STR_lastLiteral				"\x0b" "lastLiteral"
#define STR_nameTable				"\x09" "nameTable"
#define STR_options					"\x07" "options"
#define STR_size					"\x04" "size"
#define STR_studySize				"\x09" "studySize"

/* identifiers for use in packed patterns */

#define SYSTEM_ID_MAC	0
#define SYSTEM_ID_WIN	1

#ifdef MACVERSION
#define SYSTEM_ID	SYSTEM_ID_MAC
#endif

#ifdef WIN95VERSION
#define SYSTEM_ID	SYSTEM_ID_WIN
#endif

#define CURRENT_VERSION		0


/* typedefs */

typedef enum tyregexptoken { /*verbs that are processed by langregexp.c*/
	
	compilefunc,
	
	matchfunc,
	
	replacefunc,
	
	extractfunc,
	
	splitfunc,
	
	joinfunc,
	
	visitfunc,

	grepfunc,
	
	getpatterninfofunc,
	
	expandfunc,
	
	ctregexpverbs
	
	} tyregexptoken;


typedef enum tyreplacementtoken { /*identifies parts of a replacement string*/

	REGEXP_TERMINATOR,	/*signals end of sequence of replacement parts*/
	
	REGEXP_LITERAL,		/*literal string*/
	
	REGEXP_NUMBERED,	/*numbered reference to a captured substring*/
	
	REGEXP_NAMED		/*named reference to a captured substring*/

	} tyreplacementtoken;


typedef struct tycompiledpattern { /*internal representation of a compiled pattern*/

	OSType type; 			/*type of binary object, required to be 'PCRE'*/
	
	short systemid;			/* 0 = Mac, 1 = Win */
	
	short version;			/*version of this record, increment to have older versions
								of Frontier recognize newer patterns as invalid*/
	
	short pcreversionmajor;		/*PCRE version major*/
	
	short pcreversionminor;		/*PCRE version minor*/
	
	short unused[4];		/*reserved for future use*/
	
	int options;			/*options supplied to regexpcompileverb*/
	
	int capturecount;		/*number of captured substrings, determines size of ovector passed into pcre_exec*/
	
	int ovecsize;			/*number of slots in the ovector*/
	
	int pattern_offset;		/*offset of pattern structure in this record*/
	
	int pattern_size;		/*size allocated for pattern structure*/
	
	int study_offset;		/*offset of study_data structure in this record*/
	
	int study_size;			/*size allocated for study_data structure*/
	
	int unused2[4];			/*reserved for future use*/
	} tycompiledpattern;


typedef struct tyreplacepart { /*describes parts of a replacement string*/

	tyreplacementtoken type;	/*type of part*/
	
	int ix;						/*index into replacement string*/
	
	int len;					/*length in replacement string*/
	
	int ref;					/*numbered substring*/
	
	} tyreplacepart, *ptrreplacepart, **hdlreplacepart;


typedef struct tyreplscancompileinfo {
	
	handlestream s;
	
	Handle hcp;
	
	} tyreplscancompileinfo;


typedef struct tyreplscanexpandinfo {
	
	handlestream s;
	
	Handle hsubject;
	
	hdllistrecord hgroups;
	
	hdlhashtable htnames;
	
	} tyreplscanexpandinfo;


typedef struct tyregexpsearchinfo {
	
	handlestream s;
	
	char *p;
	
	Handle hcp;
	
	Handle hovector;
	
	} tyregexpsearchinfo;


typedef struct tywritelistinfo {

	handlestream *s;
	
	Handle hglue;
	} tywritelistinfo;


typedef struct tygreplistinfo {

	Handle hcp;
	
	Handle hovec;

	hdllistrecord hresult;
	
	boolean flincludematches;
	
	ptrstring bserrorptr;
	} tygreplistinfo;


typedef boolean (*tyreplscanliteralcallback) (int ix, int len, bigstring bserror, void *refcon);

typedef boolean (*tyreplscannumberedcallback) (int ix, int len, int ref, bigstring bserror, void *refcon);

typedef boolean (*tyreplscannamedcallback) (int ix, int len, const char *cptr, int clen, bigstring bserror, void *refcon);

typedef void (*tyreplscanbadgroupnamecallback) (int pos, bigstring bsmsg);


static unsigned char *chartableptr = nil;


#ifdef MACVERSION
#pragma mark === lang errors ===
#endif


static void regexpcompilationerror (const char *errmsg, int errpos, bigstring bserror) {
	
	bigstring bserrmsg;
	bigstring bserrpos;
	
	copyctopstring (errmsg, bserrmsg);
	
	numbertostring (errpos, bserrpos);
	
	getstringlist (langerrorlist, regexpcompileerror, bserror);
	
	parsedialogstring (bserror, bserrmsg, bserrpos, nil, nil, bserror);

	return;
	} /*regexpcompilationerror*/


static void regexpverberrorwithnumber (short stringnum, int x, bigstring bserror) {

	bigstring bsnum;
	
	getstringlist (langerrorlist, stringnum, bserror);
	
	numbertostring (x, bsnum);
	
	parsedialogstring (bserror, "\x02" "^0", bsnum, nil, nil, bserror);

	return;
	} /*regexpverberrorwithnumber*/


static void regexperrorwithnumber (short stringnum, int x, bigstring bserror) {

	bigstring bsnum;
	
	getstringlist (langerrorlist, stringnum, bserror);
	
	numbertostring (x, bsnum);
	
	parsedialogstring (bserror, bsnum, nil, nil, nil, bserror);

	return;
	} /*regexperrorwithnumber*/


#ifdef MACVERSION
#pragma mark === pattern value accessors ===
#endif


static int getoptions (Handle hcp) {
	return ((**((tycompiledpattern**)hcp)).options);
	} /*getoptions*/


static int getcapturecount (Handle hcp) {
	return ((**((tycompiledpattern**)hcp)).capturecount);
	} /*getcapturecount*/


static int getovectorsize (Handle hcp) {
	return ((**((tycompiledpattern**)hcp)).ovecsize);
	} /*getovectorsize*/


static int getgroupoffset (Handle hovec, int ix) {
	return (*((int*)(*hovec) + 2 * ix));
	} /*getgroupoffset*/


static int getgroupend (Handle hovec, int ix) {
	return (*((int*)(*hovec) + 2 * ix + 1));
	} /*getgroupend*/


static int getgrouplength (Handle hovec, int ix) {
	return (getgroupend (hovec, ix) - getgroupoffset (hovec, ix));
	} /*getgrouplength*/


static pcre * getpatternref (Handle hcp) {
	
	/*
	Compute a ptr to the pcre part of the handle that is only valid
	until the memory manager relocates our block of memory.
	*/
	
	return ((pcre *) ((char *) *hcp + (**((tycompiledpattern**)hcp)).pattern_offset));
	} /*getpatternref*/


static real_pcre * getrealpatternref (Handle hcp) {
	
	/*
	Compute a ptr to the pcre part of the handle that is only valid
	until the memory manager relocates our block of memory.
	*/
	
	return ((real_pcre *) ((char *) *hcp + (**((tycompiledpattern**)hcp)).pattern_offset));
	} /*getrealpatternref*/


static void * getstudydata (Handle hcp) {
	
	/*
	Compute a ptr to the study_data part of the handle that is only valid
	until the memory manager relocates our block of memory.
	*/
	
	return ((void *) (((**((tycompiledpattern**)hcp)).study_size != nil) ? ((char *) *hcp + ((**((tycompiledpattern**)hcp)).study_offset)) : nil));
	} /*getstudydata*/


static void initpcreextra (Handle hcp, pcre_extra *extraref) {
	
	void *study_data = getstudydata (hcp);
	
	clearbytes (extraref, sizeof (pcre_extra));
	
	if (study_data != nil) {
		extraref->flags |= PCRE_EXTRA_STUDY_DATA;
		extraref->study_data = study_data;
		}
	} /*initpcreextra*/


#ifdef MACVERSION
#pragma mark === param handling ===
#endif


static boolean getoptionaladdressparam (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, hdlhashtable *ht, bigstring bsname) {
	
	/*
	Get an optional parameter that is expected to be an address value.
	If the param is not specified or if it is nil, we don't return anything.
	*/
	
	tyvaluerecord vparam;
	
	setaddressvalue (nil, emptystring, &vparam);

	if (!getoptionalparamvalue (hfirst, ctconsumed, ctpositional, bsparam, &vparam))
		return (false);
	
	return (getaddressvalue (vparam, ht, bsname));
	} /*getoptionaladdressparam*/


static boolean getoptionaltableparam (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, hdlhashtable *htable) {
	
	/*
	Get an optional parameter that is expected to be the address of a table.
	If the param is not specified or if it is nil, we don't return anything.
	If the param is a valid address, we make sure a table exists at the
	indicated location and return a handle to it.
	*/
	
	hdlhashtable ht;
	bigstring bsname;
	
	if (!getoptionaladdressparam (hfirst, ctconsumed, ctpositional, bsparam, &ht, bsname))
		return (false);
	
	if (ht == nil && isemptystring (bsname))
		return (true);
	
	if (!langassignnewtablevalue  (ht, bsname, htable))
		return (false);

	return (true);
	} /*getoptionaltableparam*/


static boolean getoptionallistparam (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, hdllistrecord *hlist) {
	
	/*
	Get an optional parameter of type list that defaults to an empty list.
	*/
	
	tyvaluerecord vparam;
	
	initvalue (&vparam, listvaluetype);

	if (!getoptionalparamvalue (hfirst, ctconsumed, ctpositional, bsparam, &vparam))
		return (false);
	
	if (vparam.data.binaryvalue == nil) {
		
		if (!opnewlist (hlist, false))
			return (false);
		
		if (!setheapvalue ((Handle) *hlist, listvaluetype, &vparam))
			return (false);
		}

	*hlist = (hdllistrecord) vparam.data.binaryvalue;

	return (true);
	} /*getoptionallistparam*/


#ifdef MACVERSION
#pragma mark === utils ===
#endif


static boolean regexpgetpatternvalue (hdltreenode hp1, short pnum, boolean flreadonly, bigstring bserror, Handle *h, Handle *hovec) {

	/*
	Get a compiled pattern from the code tree for the params.
	
	If we don't yield control to other threads, it's safe
	to request a read-only copy to reduce memory overhead.
	However, if we run a callback script for example, it
	just might destroy the original pattern value we are
	looking at and cause us to crash. For those kernel verbs,
	we make our own copy of the pattern value. (Currently,
	these verbs are regexp.replace and regexp.visit.)
	
	Since the usual case is to request a readonly value,
	we should never modify any part of the pattern value.
	*/
	
	tyvaluerecord val;
	Handle hcp;
	boolean flvalid;	
	
	if (!getreadonlyparamvalue (hp1, pnum, &val))
		return (false);
	
	if ((!flreadonly) || val.valuetype != binaryvaluetype) {
		
		if (!copyvaluerecord (val, &val))
			return (false);
		
		if (!coercetobinary (&val))
			return (false);
		}
	
	hcp = val.data.binaryvalue;
	
	/* consistency checks */
	
	flvalid = gethandlesize (hcp) >= sizeof (tycompiledpattern);
	
	flvalid = flvalid && conditionallongswap ((**((tycompiledpattern**)hcp)).type) == MAGIC_NUMBER;
		
	flvalid = flvalid && conditionalshortswap ((**((tycompiledpattern**)hcp)).systemid) == SYSTEM_ID;
		
	flvalid = flvalid && conditionalshortswap ((**((tycompiledpattern**)hcp)).version) == CURRENT_VERSION;
		
	flvalid = flvalid && conditionalshortswap ((**((tycompiledpattern**)hcp)).pcreversionmajor) == PCRE_MAJOR;
		
	flvalid = flvalid && conditionalshortswap ((**((tycompiledpattern**)hcp)).pcreversionminor) == PCRE_MINOR;
		
	flvalid = flvalid && (**((tycompiledpattern**)hcp)).pattern_offset == sizeof (tycompiledpattern);
		
	flvalid = flvalid && ((**((tycompiledpattern**)hcp)).study_offset == 0
							|| (**((tycompiledpattern**)hcp)).study_offset == (sizeof (tycompiledpattern) + (**((tycompiledpattern**)hcp)).pattern_size));
	
	flvalid = flvalid && gethandlesize (hcp) == (sizeof (tycompiledpattern) + (**((tycompiledpattern**)hcp)).pattern_size + (**((tycompiledpattern**)hcp)).study_size);
		
	flvalid = flvalid && (**((tycompiledpattern**)hcp)).pattern_size == getrealpatternref (hcp) -> size;
		
	flvalid = flvalid && ((**((tycompiledpattern**)hcp)).study_size == 0
							|| ((**((tycompiledpattern**)hcp)).study_offset > 0
									&& (**((tycompiledpattern**)hcp)).study_size == ((pcre_study_data *) getstudydata (hcp)) -> size));
	
	if (!flvalid) {
		getstringlist (langerrorlist, regexpinvaliderror, bserror);
		return (false);
		}
	
	if (hovec != nil) {
		
		if (!regexpnewovector (hcp, hovec))
			return (false);
	
		if (!pushtmpstack (*hovec))
			return (false);
		}
	
	/* success! */
	
	*h = hcp;
	
	return (true);
	} /*regexpgetpatternvalue*/


static int regexpexec (Handle hcp, char *subject, int length,
						int ix, int matchlen, Handle hovector,
						bigstring bserror) {

	pcre_extra extra;
	int res;
	
	/* consistency checks */
	
	if (ix < 0)
		ix = 0;
	
	if (matchlen > length - ix)
		matchlen = length - ix;
	
	if (matchlen < 0)
		matchlen = 0;
	
	/* run pcre engine */
	
	initpcreextra (hcp, &extra);

	res = pcre_exec (getpatternref (hcp), &extra,
						subject, length, ix, matchlen,
						getoptions (hcp) & PUBLIC_EXEC_OPTIONS,
						(int *)*hovector, getovectorsize (hcp), chartableptr); 

	/* process execution errors here, presumably indicative of a bug in the PCRE library */
	
	if (res <= 0 && res != PCRE_ERROR_NOMATCH) {
		
		// "Can't do regexp.foo because an internal regexp error occurred (code X)."
	
		regexpverberrorwithnumber (regexpinternalerror, res, bserror);
		}

	return (res);
	} /*regexpexec*/


static int regexpexechandle (Handle hcp, Handle h,
								int ix, int matchlen, Handle hovector,
								bigstring bserror) {

	return (regexpexec (hcp, *h, gethandlesize (h), ix, matchlen, hovector, bserror));
	} /*regexpexechandle*/


static boolean regexpgetpatterninfo (Handle hcp, int what, bigstring bserror, void *where) {

	pcre_extra extra;
	int res;
	
	initpcreextra (hcp, &extra);
	
	res = pcre_fullinfo (getpatternref (hcp), &extra, what, where);
	
	if (res != 0) {
		regexpverberrorwithnumber (regexpinternalerror, res, bserror);
		return (false);
		}
	
	return (true);
	} /*regexpgetpatterninfo*/


static boolean regexpbuildmatchinfotable (Handle hsubject, Handle hcp, Handle hovec,
											boolean flmakegroups, boolean flmakenamedgroups,
											bigstring bserror, hdlhashtable ht) {

	long offset = getgroupoffset (hovec, 0);
	long length = getgrouplength (hovec, 0);
	hdllistrecord hoffsets = nil;
	hdllistrecord hlengths = nil;
	hdllistrecord hstrings = nil;
	Handle h;
	
	/* matchOffset */
	
	if (!langassignlongvalue (ht, STR_matchOffset, offset + 1))
		goto exit;
	
	/* matchLength */
	
	if (!langassignlongvalue (ht, STR_matchLength, length))
		goto exit;
	
	/* matchString */

	if (!loadfromhandletohandle (hsubject, &offset, length, false, &h))
		goto exit;
	
	if (!langassigntextvalue (ht, STR_matchString, h)) {
		disposehandle (h);
		goto exit;
		}
	
	/* make groups */
	
	if (flmakegroups) {
	
		tyvaluerecord vtemp;
		long j = 0;
		
		if (!opnewlist (&hoffsets, false))
			goto exit;
		
		if (!opnewlist (&hlengths, false))
			goto exit;
		
		if (!opnewlist (&hstrings, false))
			goto exit;
		
		while ((++j) <= getcapturecount (hcp)) {
			
			offset = getgroupoffset (hovec, j);
			
			if (offset >= 0) { /* this group matched */
				
				length = getgrouplength (hovec, j);
			
				/* groupOffsets [j] */
				
				if (!langpushlistlong (hoffsets, offset + 1))
					goto exit;
			
				/* groupLengths [j] */
				
				if (!langpushlistlong (hlengths, length))
					goto exit;
			
				/* groupStrings [j] */
				
				if (!loadfromhandletohandle (hsubject, &offset, length, false, &h))
					goto exit;
				
				if (!langpushlisttext (hstrings, h))
					goto exit;
				}
			else { /* this group didn't match */
			
				/* groupOffsets [j] */
				
				if (!langpushlistlong (hoffsets, 0))
					goto exit;
			
				/* groupLengths [j] */
				
				if (!langpushlistlong (hlengths, 0))
					goto exit;
			
				/* groupStrings [j] */
				
				if (!langpushliststring (hstrings, emptystring))
					goto exit;
				
				}
			}
		
		/* groupOffsets */
		
		initvalue (&vtemp, listvaluetype);
		
		vtemp.data.binaryvalue = (Handle) hoffsets;
		
		if (!hashtableassign (ht, STR_groupOffsets, vtemp))
			goto exit;
		
		hoffsets = nil;
		
		/* groupLengths */
		
		initvalue (&vtemp, listvaluetype);
		
		vtemp.data.binaryvalue = (Handle) hlengths;
		
		if (!hashtableassign (ht, STR_groupLengths, vtemp))
			goto exit;
		
		hlengths = nil;
		
		/* groupStrings */
		
		initvalue (&vtemp, listvaluetype);
		
		vtemp.data.binaryvalue = (Handle) hstrings;
		
		if (!hashtableassign (ht, STR_groupStrings, vtemp))
			goto exit;
		
		hstrings = nil;
		}
	
	if (flmakenamedgroups) {

		hdlhashtable htgroups, htname;
		int res, ct, sz, k, groupnum;
		unsigned char *cptr;
		bigstring bs;
		
		res = pcre_fullinfo (getpatternref (hcp), nil, PCRE_INFO_NAMECOUNT, (void *) &ct);
		
		if (res == 0)
			res = pcre_fullinfo (getpatternref (hcp), nil, PCRE_INFO_NAMEENTRYSIZE, (void *) &sz);
		
		if (res == 0)
			res = pcre_fullinfo (getpatternref (hcp), nil, PCRE_INFO_NAMETABLE, (void *) &cptr);
		
		if (res != 0) {
			regexpverberrorwithnumber (regexpinternalerror, res, bserror);
			goto exit;
			}
		
		if (!langassignnewtablevalue (ht, STR_namedGroups, &htgroups))
			goto exit;

		if (cptr != nil) {
			
			for (k = 0; k < ct; k++) {
				
				groupnum = (cptr[k*sz] << 8) + cptr[k*sz+1];
				
				copyctopstring ((char *) (cptr + k * sz + 2), bs);
				
				if (!langassignnewtablevalue (htgroups, bs, &htname))
					goto exit;
				
				offset = getgroupoffset (hovec, groupnum);
				
				length = getgrouplength (hovec, groupnum);

				if (!langassignlongvalue (htname, STR_groupNumber, groupnum))
					goto exit;

				if (!langassignlongvalue (htname, STR_matchOffset, offset + 1)) /*1-based*/
					goto exit;

				if (!langassignlongvalue (htname, STR_matchLength, length))
					goto exit;

				if (!loadfromhandletohandle (hsubject, &offset, length, false, &h))
					goto exit;
				
				if (!langassigntextvalue (htname, STR_matchString, h)) {
					disposehandle (h);
					goto exit;
					}
				} /*for*/
			}
		}
		
	return (true);

exit:
	
	opdisposelist (hoffsets);
	
	opdisposelist (hlengths);
	
	opdisposelist (hstrings);
	
	return (false);
	} /*regexpbuildmatchinfotable*/


static boolean writehandlestreamreplpart (handlestream *s, tyreplacementtoken type, int ix, int len, int ref) {
	
	/*
	helper function for regexpscanreplacement
	*/

	tyreplacepart rp;
	
	if (type == REGEXP_LITERAL && len == 0) /*never write empty literals*/
		return (true);
	
	rp.type = type;
	rp.ix   = ix;
	rp.len  = len;
	rp.ref  = ref;
	
	return (writehandlestream (s, &rp, sizeof (rp)));
	} /*writehandlestreamreplpart*/


static int regexpstringnumberfrompattern (const char *cptr, int len, Handle hcp) {
	
	int res, top, mid, bot, entrysize, c;
	unsigned char *entry, *nametable;
	
	res = pcre_fullinfo (getpatternref (hcp), nil, PCRE_INFO_NAMECOUNT,  (void *) &top);

	if (res == 0)
		res = pcre_fullinfo (getpatternref (hcp), nil, PCRE_INFO_NAMEENTRYSIZE, (void *) &entrysize);	

	if (res == 0)
		res = pcre_fullinfo (getpatternref (hcp), nil, PCRE_INFO_NAMETABLE, (void *) &nametable);	
	
	if (res != 0) {
		return (res);
		}
		
	bot = 0;
	
	while (top > bot) {
	
	  mid = (top + bot) / 2;
	  
	  entry = nametable + entrysize * mid;
	  
	  if (strlen ((char *)(entry + 2)) == len)
		c = memcmp (cptr, (char *)(entry + 2), len);
	  
	  if (c == 0) {
	  	return (entry[0] << 8) + entry[1];
	  	}
	  
	  if (c > 0)
	  	bot = mid + 1;
	  else
	  	top = mid;
	  }

	return (PCRE_ERROR_NOSUBSTRING);
	} /*regexpstringnumberfrompattern*/


static boolean replscancompileliteral (int ix, int len, bigstring bserror, void *refcon) {
	
	tyreplscancompileinfo *info = (tyreplscancompileinfo *) refcon;
	
	return (writehandlestreamreplpart (&(info->s), REGEXP_LITERAL, ix, len, 0));
	} /*replscancompileliteral*/


static boolean replscancompilenumbered (int ix, int len, int ref, bigstring bserror, void *refcon) {
	
	tyreplscancompileinfo *info = (tyreplscancompileinfo *) refcon;
						
	if (ref > getcapturecount (info->hcp)) {
		regexpverberrorwithnumber (regexpnonexistantgroupnumbererror, ix+1, bserror);
		return (false);
		}
	
	return (writehandlestreamreplpart (&(info->s), REGEXP_NUMBERED, ix, len, ref));
	} /*replscancompilenumbered*/


static boolean replscancompilenamed (int ix, int len, const char *cptr, int clen, bigstring bserror, void *refcon) {
	
	tyreplscancompileinfo *info = (tyreplscancompileinfo *) refcon;
	int ref;
	
	ref = regexpstringnumberfrompattern (cptr, clen, info->hcp);
						
	if (ref <= 0) {
		if (ref == PCRE_ERROR_NOSUBSTRING)
			regexpverberrorwithnumber (regexpnonexistantgroupnameerror, ix+1, bserror);
		else
			regexpverberrorwithnumber (regexpinternalerror, ref, bserror);
		return (false);
		}
	
	return (writehandlestreamreplpart (&(info->s), REGEXP_NAMED, ix, len, 0));
	} /*replscancompilenamed*/

	
static boolean replscanwriteliteral (int ix, int len, bigstring bserror, void *refcon) {
	
	tyreplscanexpandinfo *info = (tyreplscanexpandinfo *) refcon;
	
	return (writehandlestreamhandlepart (&(info->s), info->hsubject, ix, len));
	} /*replscanwriteliteral*/


static boolean replscanwritenumbered (int ix, int len, int ref, bigstring bserror, void *refcon) {
	
	tyreplscanexpandinfo *info = (tyreplscanexpandinfo *) refcon;
	tyvaluerecord val;
	
	if (ref > opcountlistitems (info->hgroups)) {
		regexpverberrorwithnumber (regexpnonexistantgroupnumbererror, ix+1, bserror);
		return (false);
		}

	if (!getnthlistval (info->hgroups, ref, nil, &val))
		return (false);
	
	if (!coercetostring (&val))
		return (false);
	
	if (!writehandlestreamhandle (&(info->s), val.data.stringvalue))
		return (false);
	
	releaseheaptmp (val.data.stringvalue);
	
	return (true);
	} /*replscanwritenumbered*/


static boolean replscanwritenamed (int ix, int len, const char *cptr, int clen, bigstring bserror, void *refcon) {
	
	tyreplscanexpandinfo *info = (tyreplscanexpandinfo *) refcon;
	hdlhashnode hnode;
	hdlhashtable ht;
	tyvaluerecord vname, vstr;
	bigstring bsname;
	boolean fl;
	
	moveleft ((void *) cptr, stringbaseaddress (bsname), clen);
	setstringlength (bsname, clen);

	if (!langhashtablelookup (info->htnames, bsname, &vname, &hnode))
		return (false);	

	if (!langexternalvaltotable (vname, &ht, hnode)) {
		return (false);
		}

	if (!langhashtablelookup (ht, STR_matchString, &vstr, &hnode))
		return (false);
		
	if (vstr.valuetype != stringvaluetype)
		if (!copyvaluerecord (vstr, &vstr) || !coercetostring (&vstr))
			return (false);

	fl = writehandlestreamhandle (&(info->s), vstr.data.stringvalue);

	if (vstr.fltmpstack)
		releaseheaptmp (vstr.data.stringvalue);
	
	return (fl);
	} /*replscanwritenamed*/


static boolean regexpcheckliteral (int ix, int len, bigstring bserror, void *refcon) {

	/*nothing to check*/
	
	return (true);
	} /*regexpcheckliteral*/


static boolean regexpchecknumbered (int ix, int len, int ref, bigstring bserror, void *refcon) {
	
	Handle hcp = (Handle) refcon;
						
	if (ref > getcapturecount (hcp)) {
		regexperrorwithnumber (frnonexistantgroupnumbererror, ix+1, bserror);
		return (false);
		}
	
	return (true);
	} /*regexpchecknumbered*/


static boolean regexpchecknamed (int ix, int len, const char *cptr, int clen, bigstring bserror, void *refcon) {
	
	Handle hcp = (Handle) refcon;
	int ref;
	
	ref = regexpstringnumberfrompattern (cptr, clen, hcp);
						
	if (ref <= 0) {
		if (ref == PCRE_ERROR_NOSUBSTRING)
			regexperrorwithnumber (frnonexistantgroupnameerror, ix+1, bserror);
		else
			regexperrorwithnumber (frinternalerror, ref, bserror);
		return (false);
		}
	
	return (true);
	} /*regexpchecknamed*/
	

static boolean regexptextsearchwriteliteral (int ix, int len, bigstring bserror, void *refcon) {
	
	tyregexpsearchinfo *info = (tyregexpsearchinfo *) refcon;
	
	return (writehandlestream (&(info->s), stringbaseaddress (searchparams.bsreplace) + ix, len));
	} /*regexptextsearchwriteliteral*/


static boolean regexptextsearchwritenumbered (int ix, int len, int ref, bigstring bserror, void *refcon) {
	
	tyregexpsearchinfo *info = (tyregexpsearchinfo *) refcon;

	return (writehandlestream (&(info->s), info->p + getgroupoffset (info->hovector, ref), getgrouplength (info->hovector, ref)));
	} /*regexptextsearchwritenumbered*/


static boolean regexptextsearchwritenamed (int ix, int len, const char *cptr, int clen, bigstring bserror, void *refcon) {
	
	tyregexpsearchinfo *info = (tyregexpsearchinfo *) refcon;
	int ref;
	
	ref = regexpstringnumberfrompattern (cptr, clen, info->hcp);
						
	if (ref <= 0) {
		return (false);
		}

	return (writehandlestream (&(info->s), info->p + getgroupoffset (info->hovector, ref), getgrouplength (info->hovector, ref)));
	} /*regexptextsearchwritenamed*/


static void replscanerror (int pos, bigstring bsmsg) {
	
	regexpverberrorwithnumber (regexpbadgroupnameerror, pos, bsmsg);
	} /*replscanerror*/


static void regexpcheckerror (int pos, bigstring bsmsg) {
	
	regexperrorwithnumber (frbadgroupnameerror, pos, bsmsg);
	} /*regexpcheckerror*/
	

static boolean regexpscanreplacement (const char *pstart, long len, bigstring bserror,
										tyreplscanliteralcallback literalfunc,
										tyreplscannumberedcallback numberedfunc,
										tyreplscannamedcallback namedfunc,
										tyreplscanbadgroupnamecallback errorfunc,
										void *refcon) {

	/*
	Syntax for references in replacement string:
	
	\nn where nn is a number, possibly consisting of multiple digits, refers to a numbered substring.
	
	\g<nn> where nn is a number, possibly consisting of multiple digits, refers to a numbered substring.
	
	\g<nn> where nn is an alpha string, refers to a named substring.
	
	This follows the syntax used by the sub method in the Python re module.
	*/
	
	const char *p = pstart;
	const char *plast = p;
	const char *pend = p + len;
	
	while (p < pend - 1) { /*leave trailing backslash alone*/
		
		if (*p == '\\') {
			
			const char *p1 = p + 1; /*still in handle due to break-off condition*/
			
			if (*p1 == 'g') {
				
				p1++;
				
				if (p1 < pend && *p1 == '<') {
					
					const char *p2 = ++p1;
					
					if (p1 == pend) {
						errorfunc (p2 - pstart, bserror);
						return (false);
						}
					
					if (isdigit(*p1)) { /*should be a numbered group*/
					
						long ref = 0;
						
						while (p1 < pend && isdigit (*p1)) {
							
							ref = 10 * ref + (*p1 - '0');
							
							p1++;
							}
						
						if (p1 == pend || *p1 != '>') {
							errorfunc (p2 - pstart + 1, bserror);
							return (false);
							}
					
						if (!literalfunc (plast - pstart, p - plast, bserror, refcon))
							return (false);
						
						if (!numberedfunc (p - pstart, p1 - p, ref, bserror, refcon))
							return (false);
						
						}
					else { /*might be a named group*/

						while (p1 < pend && *p1 != '>')
							p1++;
						
						if (p1 == pend || p1 == p2) { /*unterminated or empty group name*/
							errorfunc (p2 - pstart + 1, bserror);
							return (false);				
							}
						
						if (!literalfunc (plast - pstart, p - plast, bserror, refcon))
							return (false);
						
						if (!namedfunc (p - pstart, p1 - p, p2, p1 - p2, bserror, refcon))
							return (false);
						}
					
					p = plast = ++p1; /*point past closing angle bracket*/
					
					continue; /*avoid incrementation of x*/
					}
				else {
					errorfunc (p1 - pstart, bserror);				
					return (false);				
					}
				}
			else if (*p1 == '\\') {
				/* 2004/12/11 smd: double backslashes should be collapsed into singles */
				
				if (!literalfunc (plast - pstart, (p - plast) + 1, bserror, refcon))
					return (false);
				
				p = plast = ++p1;
				
				continue; /*avoid incrementation of p*/				
				}
			else if (isdigit (*p1)) {
				
				long ref = 0;
				
				while (p1 < pend && isdigit (*p1)) {
					
					ref = 10 * ref + (*p1 - '0');
					
					p1++;
					}
				
				if (!literalfunc (plast - pstart, p - plast, bserror, refcon))
					return (false);
				
				if (!numberedfunc (p - pstart, p1 - p, ref, bserror, refcon))
					return (false);
				
				p = plast = p1;
				
				continue; /*avoid incrementation of x*/				
				}
			else {
				p++; /*step over following char*/
				}
			}
		
		p++;
		}/*while*/
	
	/* push remaining literal if any */
	
	if (plast < pend)
		if (!literalfunc (plast - pstart, pend - plast, bserror, refcon))
			return (false);
	
	return (true);
	} /*regexpscanreplacement*/


static boolean regexpcompilereplacement (Handle hcp, char *p, long len, bigstring bserror, hdlreplacepart *hreplaceparts) {

	tyreplscancompileinfo info;
	
	/* scan repl parameter */
	
	info.hcp = hcp;

	openhandlestream (nil, &info.s);

	if (!regexpscanreplacement (p, len, bserror,
									&replscancompileliteral, &replscancompilenumbered, &replscancompilenamed,
									&replscanerror,
									(void *) &info)) {
		disposehandlestream (&info.s);
		return (false);
		}

	if (!writehandlestreamreplpart (&info.s, REGEXP_TERMINATOR, 0, 0, 0)) {
		disposehandlestream (&info.s);
		return (false);
		}
	
	*hreplaceparts = (hdlreplacepart) closehandlestream (&info.s);

	return (*hreplaceparts != nil);
	} /*regexpcompilereplacement*/


static boolean writereplacementhandlestream (handlestream *s, Handle hsubject, Handle hreplace, hdlreplacepart hreplaceparts, Handle hovec) {

	long k = 0;
	
	assert (hreplaceparts != nil);
	
	while (true) {
	
		switch ((*hreplaceparts)[k].type) {
		
			case REGEXP_LITERAL:
			{
				if (!writehandlestreamhandlepart (s, hreplace, (*hreplaceparts)[k].ix, (*hreplaceparts)[k].len))
					return (false);

				break;
			}
			
			case REGEXP_NUMBERED:
			case REGEXP_NAMED:
			{
				int ref = (*hreplaceparts)[k].ref;
				
				if (!writehandlestreamhandlepart (s, hsubject, getgroupoffset (hovec, ref), getgrouplength (hovec, ref)))
					return (false);

				break;
			}
				
			default:
				return (true);
		
			} /*switch*/
	
		k++;
		
		} /*while*/

	return (true);
	} /*writereplacementhandlestream*/


static boolean getcodetreefromscriptaddress (hdlhashtable htable, bigstring bsverb, hdltreenode *hcode) {
	
	/*
	Given the address of an object, assume it's a script or code object and get its code tree
	
	Code cribbed from langrunscript in lang.c
	*/
	
	tyvaluerecord vhandler;
	hdlhashnode handlernode;
	
	if (!hashtablelookupnode (htable, bsverb, &handlernode)) {
		
		langparamerror (unknownfunctionerror, bsverb);
		
		return (false);
		}
	
	vhandler = (**handlernode).val;
	
	/*build a code tree and call the handler, with our error hook in place*/
	
	*hcode = nil;
	
	if (vhandler.valuetype == codevaluetype) {

		*hcode = vhandler.data.codevalue;
	}
	else if ((**htable).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, hcode)) {

			langparamerror (notfunctionerror, bsverb);

			return (false);
			}
		
		if (*hcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, hcode))
				return (false);
			}
		}
	
	return (true);
	} /*getcodetreefromscriptaddress*/


static boolean regexprunreplacecallback (tyaddress *adrcallback, tyvaluerecord *vrepl, tyvaluerecord *vmatchinfo, Handle *hrepl, boolean *flcallbackresult) {
	
	hdlhashtable htlocals = nil;
	hdllistrecord hparams;
	tyvaluerecord vparams, vresult, val;
	hdltreenode hcode;
	hdlhashnode hdummy;
	boolean fl;
	
	*hrepl = nil;
	
	/* set up table of local variables */
	
	exemptfromtmpstack (vrepl);
	
	exemptfromtmpstack (vmatchinfo);
	
	fl = langpushlocalchain (&htlocals);
	
	if (fl)
		fl = hashtableassign (htlocals, STR_replacementString, *vrepl);
	
	if (!fl)
		disposevaluerecord (*vrepl, false);
	
	if (fl)
		fl = hashtableassign (htlocals, STR_matchInfo, *vmatchinfo);
	
	if (!fl)
		disposevaluerecord (*vmatchinfo, false);
	
	/* build list of parameters */
	
	if (fl)
		fl = opnewlist (&hparams, false);
	
	if (fl)
		fl = setheapvalue ((Handle) hparams, listvaluetype, &vparams);
	
	if (fl)
		fl = langpushlistaddress (hparams, htlocals, STR_matchInfo);
	
	if (fl)
		fl = langpushlistaddress (hparams, htlocals, STR_replacementString);
		
	/* obtain code tree for callback script */
	
	if (fl)
		fl = getcodetreefromscriptaddress ((*adrcallback).ht, (*adrcallback).bs, &hcode);
	
	/* run callback script */
	
	if (fl)
		fl = langrunscriptcode ((*adrcallback).ht, (*adrcallback).bs, hcode, &vparams, nil, &vresult);
	
	if (fl)
		fl = coercetoboolean (&vresult);

	if (fl)
		*flcallbackresult = vresult.data.flvalue;
	
	/* get replacement string from locals table */
	
	if (fl)
		fl = langhashtablelookup (htlocals, STR_replacementString, &val, &hdummy);
	
	if (fl)
		fl = copyvaluerecord (val, &val);
	
	if (fl)
		if (val.valuetype != stringvaluetype)
			fl = coercetostring (&val);
	
	if (fl) {
		exemptfromtmpstack (&val);
		*hrepl = val.data.stringvalue;
		}
	
	/* dispose table for local variables */
	
	return (langpoplocalchain (htlocals) && fl);
	} /*regexprunreplacecallback*/


static boolean regexpreplacematch (handlestream *s, Handle hsubject, Handle hreplace, hdlreplacepart hreplaceparts,
									Handle hcp, Handle hovec, tyaddress *adrcallback,
									bigstring bserror, boolean *flresult) {

	hdlhashtable htmatchinfo = nil;
	tyvaluerecord vmatchinfo;
	handlestream hsrepl;
	tyvaluerecord vrepl;
	Handle hrepl;
	boolean fl;
	
	*flresult = true; /*default return value*/
	
	/* if adrcallback was not specified just perform the replacement and return */
	
	if ((*adrcallback).ht == nil && isemptystring ((*adrcallback).bs))
		return (writereplacementhandlestream (s, hsubject, hreplace, hreplaceparts, hovec));
	
	/* build default replacement string */
	
	openhandlestream (nil, &hsrepl);
	
	if (!writereplacementhandlestream (&hsrepl, hsubject, hreplace, hreplaceparts, hovec))
		return (false);
	
	hrepl = closehandlestream (&hsrepl);
	
	if (hrepl == nil) {
		if (!newemptyhandle (&hrepl))
			return (false);
		}
	
	if (!setheapvalue (hrepl, stringvaluetype, &vrepl))
		return (false);
	
	/* build matchInfo table */
	
	if (!tablenewtablevalue (&htmatchinfo, &vmatchinfo))
		return (false);
	
	if (!pushtmpstackvalue (&vmatchinfo))
		return (false);
	
	if (!regexpbuildmatchinfotable (hsubject, hcp, hovec, true, true, bserror, htmatchinfo))
		return (false);
	
	/* run callback script */
	
	if (!regexprunreplacecallback (adrcallback, &vrepl, &vmatchinfo, &hrepl, flresult)) {
		disposehandle (hrepl);
		return (false);
		}

	if (*flresult)
		fl = writehandlestreamhandle (s, hrepl);
	else
		fl = writehandlestreamhandlepart (s, hsubject, getgroupoffset (hovec, 0), getgrouplength (hovec, 0));

	disposehandle (hrepl);
	
	return (fl);
	} /*regexpreplacematch*/


static boolean pushhandlepartonlist (hdllistrecord hlist, Handle hsrc, long offset, long length) {

	Handle h;

	if (!loadfromhandletohandle (hsrc, &offset, length, false, &h))
		return (false);

	return (langpushlisttext (hlist, h));
	} /*pushhandlepartonlist*/


static boolean regexppushonematchonlist (Handle hsubject, Handle hovec, long ixgroup, hdllistrecord hlist) {

	return (pushhandlepartonlist (hlist, hsubject, getgroupoffset (hovec, ixgroup), getgrouplength (hovec, ixgroup)));
	} /*regexppushonematchonlist*/


static boolean regexpextractmatch (Handle hsubject, Handle hovec, int **hgroups, long ctgroups, hdllistrecord hresult) {

	boolean fl;
	
	if (ctgroups == 1) {
		
		/* add string item to result list */
		
		fl = regexppushonematchonlist (hsubject, hovec, (*hgroups)[0], hresult);
		}
	else {
	
		/* add sublist of strings to result list */

		hdllistrecord hsublist = nil;
		tyvaluerecord vtemp;
		long k;
		
		if (!opnewlist (&hsublist, false))
			return (false);
		
		for (k = 0; k < ctgroups; k++) {
			
			if (!regexppushonematchonlist (hsubject, hovec, (*hgroups)[k], hsublist)) {
				opdisposelist (hsublist);
				return (false);
				}
			} /*for*/
		
		initvalue (&vtemp, listvaluetype);
		
		vtemp.data.binaryvalue = (Handle) hsublist;
		
		fl = langpushlistval (hresult, nil, &vtemp);
		
		opdisposelist (hsublist);
		}

	return (fl);
	} /*regexpextractmatch*/


static boolean regexpwritelistitemscallback (tyvaluerecord *v, ptrvoid refcon) {
	
	tywritelistinfo *info = (tywritelistinfo *) refcon;
	
	if (!coercetostring (v))
		return (false);
	
	if (!writehandlestreamhandle ((*info).s, (*v).data.stringvalue))
		return (false);
	
	if (!writehandlestreamhandle ((*info).s, (*info).hglue))
		return (false);

	return (true);
	} /*regexpwritelistitemscallback*/


static boolean regexprunvisitcallback (Handle hsubject, Handle hcp, Handle hovec,
										boolean flmakegroups, boolean flmakenamedgroups, tyaddress *adrcallback,
										bigstring bserror, tyvaluerecord *vresult) {
	
	hdlhashtable htlocals = nil;
	hdlhashtable htmatchinfo;
	hdllistrecord hparams;
	tyvaluerecord vparams;
	hdltreenode hcode;
	boolean fl = true;
		
	/* obtain code tree for callback script */
	
	if (!getcodetreefromscriptaddress ((*adrcallback).ht, (*adrcallback).bs, &hcode))
		return (false);
	
	/* set up table for local variables */
	
	if (!langpushlocalchain (&htlocals))
		return (false);
		
	/* build match info table */
	
	fl = langassignnewtablevalue (htlocals, STR_matchInfo, &htmatchinfo);
	
	if (fl)
		fl = regexpbuildmatchinfotable (hsubject, hcp, hovec, flmakegroups, flmakenamedgroups, bserror, htmatchinfo);
	
	/* build list of parameters */
	
	if (fl)
		fl = opnewlist (&hparams, false);
	
	if (fl)
		fl = setheapvalue ((Handle) hparams, listvaluetype, &vparams);
	
	if (fl)
		fl = langpushlistaddress (hparams, htlocals, STR_matchInfo);
	
	/* run callback script */
	
	if (fl)
		fl = langrunscriptcode ((*adrcallback).ht, (*adrcallback).bs, hcode, &vparams, nil, vresult);
	
	/* dispose table for local variables */
	
	fl = langpoplocalchain (htlocals) && fl;

	return (fl);
	} /*regexprunvisitcallback*/


static boolean regexpgreplistitemscallback (tyvaluerecord *v, ptrvoid refcon) {
	
	tygreplistinfo *info = (tygreplistinfo *) refcon;
	int res;
	
	if (!coercetostring (v))
		return (false);
	
	res = regexpexechandle ((*info).hcp, (*v).data.stringvalue, 0, longinfinity, (*info).hovec, (*info).bserrorptr);
	
	if (res <= 0 && res != PCRE_ERROR_NOMATCH)
		return (false);
	
	if ((*info).flincludematches ? (res > 0) : (res == PCRE_ERROR_NOMATCH))
		if (!langpushlistval ((*info).hresult, nil, v))
			return (false);
	
	return (true);
	} /*regexpgreplistitemscallback*/


#ifdef MACVERSION
#pragma mark === public functions ===
#endif


boolean regexpcompile (const char *patternstr, int options, bigstring bserror, Handle *hcp) {
	
	/*
	Compile a null-terminated string as a regular expression
	*/

	pcre *patternref = nil;
	pcre_extra *extraref = nil;
	const char *errmsg;
	int errpos;
	tycompiledpattern rec;
	Handle h;
	real_pcre *pat;
	int capturecount, res;
	boolean flsuccess = false;
	
	/* compile pattern */
	
	patternref = pcre_compile (patternstr, options & PUBLIC_OPTIONS, &errmsg, &errpos, chartableptr);
	
	if (patternref == nil) {
		regexpcompilationerror (errmsg, errpos, bserror);
		goto exit;
		}
		
	/* study compiled pattern */
	
	extraref = pcre_study (patternref, options & PUBLIC_STUDY_OPTIONS, &errmsg);
	
	if (errmsg != nil) {
		regexpcompilationerror (errmsg, 0, bserror);
		goto exit;
		}

	/* determine number of captured substrings in pattern */
	
	res = pcre_fullinfo (patternref, extraref, PCRE_INFO_CAPTURECOUNT, &capturecount);
	
	if (res != 0) {
		regexpverberrorwithnumber (regexpinternalerror, res, bserror);
		goto exit;
		}
	
	/* nuke tables ptr in real_pcre struct */
	
	pat = (real_pcre *) patternref;
	
	pat->tables = nil;
	
	/* fill in compiled pattern struct*/
	
	rec.type = MAGIC_NUMBER;
	rec.systemid = SYSTEM_ID;
	rec.version = CURRENT_VERSION;
	rec.pcreversionmajor = PCRE_MAJOR;
	rec.pcreversionminor = PCRE_MINOR;
	
	memtodisklong (rec.type);
	memtodiskshort (rec.systemid);
	memtodiskshort (rec.version);			/* don't need to byte-swap anything else because we	*/
	memtodiskshort (rec.pcreversionmajor);	/* reject the pattern if it wasn't compiled on the	*/
	memtodiskshort (rec.pcreversionminor);	/* same platfrom and with the same PCRE version		*/

	rec.options = options;

	rec.capturecount = capturecount;

	rec.ovecsize = max (30, 3 * (capturecount + 1));

	rec.pattern_size	= pat->size;
	rec.pattern_offset	= sizeof (tycompiledpattern);

	if (extraref != nil) {
		if ((extraref->flags & PCRE_EXTRA_STUDY_DATA) != 0 && extraref->study_data != nil) {
			rec.study_size = ((pcre_study_data *) (extraref->study_data)) -> size;
			rec.study_offset = rec.pattern_offset + rec.pattern_size;
			}
		}
	else {
		rec.study_size = 0;
		rec.study_offset = 0;
		}

	/* allocate handle */
	
	if (!newclearhandle (sizeof (tycompiledpattern) + rec.pattern_size + rec.study_size, &h))
		goto exit;
	
	/* move data into handle */
	
	moveleft (&rec, *h, sizeof (rec));
	
	if (rec.pattern_size > 0)
		moveleft (patternref, *h + rec.pattern_offset, rec.pattern_size);
	
	if (rec.study_size > 0)
		moveleft (extraref->study_data, *h + rec.study_offset, rec.study_size);

	*hcp = h;
	
	flsuccess = true;
	
exit:
	
	if (patternref != nil)
		pcre_free (patternref);
	
	if (extraref != nil)
		pcre_free (extraref);
		
	return (flsuccess);
	} /*regexpcompile*/


boolean regexpcheckreplacement (Handle hcp, const char *p, int len) {

	/*
	Just check whether the replacement string contains a syntax error,
	but don't actually build any in-memory structures.
	*/
	
	bigstring bsmsg;
	boolean fl;
	
	assert (hcp != nil && *hcp != nil);
	
	assert (conditionallongswap ((**((tycompiledpattern**)hcp)).type) == MAGIC_NUMBER);
	
	setemptystring (bsmsg);
	
	fl = regexpscanreplacement (p, len,
						bsmsg,
						&regexpcheckliteral,
						&regexpchecknumbered,
						&regexpchecknamed,
						&regexpcheckerror,
						(void *) hcp);

	if (!isemptystring (bsmsg))
		langerrormessage (bsmsg);
	
	return (fl);
	} /*regexpcheckreplacement*/


boolean regexpnewovector (Handle hcp, Handle *hovec) {
	return (newclearhandle (sizeof (int) * getovectorsize (hcp), hovec));
	} /*regexpnewovector*/


boolean regexptextsearch (byte *ptext, long lentext, long *offset, long *lenmatch) {

	/*
	Called from textsearch (which it mimicks) in search.c
	to support regular expressions in Find & Replace dialogs.
	
	If we find a match, set up the replacement string right here,
	but leave inserting it up to the caller.
	
	Return value indicates whether we found a match or not.
	
	Don't throw langerrors.
	*/

	Handle hcp = searchparams.hcompiledpattern;
	Handle hovector = searchparams.hovector;
	long ixstart = *offset;
	pcre_extra extra;
	int res;
	bigstring bsdummy;
	boolean fl;
	
	assert (hcp != nil && *hcp != nil);
	
	assert (conditionallongswap ((**((tycompiledpattern**)hcp)).type) == MAGIC_NUMBER);
		
	initpcreextra (hcp, &extra);
	
	while (true) {
		
		res = pcre_exec (getpatternref (hcp), &extra,
							(const char *)ptext, lentext, ixstart, lentext - ixstart,
							getoptions (hcp) & PUBLIC_EXEC_OPTIONS,
							(int *) *hovector, getovectorsize (hcp), chartableptr); 

		if (res <= 0)
			return (false);
		
		if (!searchparams.flwholewords || isword (ptext, lentext, getgroupoffset (hovector, 0), getgrouplength (hovector, 0))) {
		
			tyregexpsearchinfo info;
			
			info.p = ptext;
			info.hcp = hcp;
			info.hovector = hovector;
			openhandlestream (nil, &info.s);
			
			disablelangerror ();
			
			fl = regexpscanreplacement (stringbaseaddress (searchparams.bsorigreplace),
								stringlength (searchparams.bsorigreplace),
								bsdummy,
								&regexptextsearchwriteliteral,
								&regexptextsearchwritenumbered,
								&regexptextsearchwritenamed,
								&replscanerror,
								(void *) &info);
			
			enablelangerror ();
			
			if (!fl) {
				disposehandlestream (&info.s);
				return (false);
				}
			
			texthandletostring (closehandlestream (&info.s), searchparams.bsreplace);
		
			*offset = getgroupoffset (hovector, 0);
			*lenmatch = getgrouplength (hovector, 0);

			return (true);
			}
		
		ixstart = getgroupoffset (hcp, 0) + 1; /*continue after beginning of false match*/
		}
	} /*regexptextsearch*/


#ifdef MACVERSION
#pragma mark === kernel verbs ===
#endif


static boolean regexpcompileverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {
	
	/*
	compile (pattern, flCaseSensitive = true) //opaque: pattern reference
	*/
	
	Handle hpattern;
	tyvaluerecord vtemp;
	int options = 0;
	short ctconsumed = 1;
	short ctpositional = 1;
	char chterminate = chnul;
	Handle hcp;
	boolean fl;
		
	/* get pattern param */
	
	if (!gettextvalue (hp1, 1, &hpattern))
		return (false);

	/* get flCaseSensitive param */
	
	setbooleanvalue (false, &vtemp); /* defaults to false */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flCaseSensitive, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? 0L : PCRE_CASELESS);

	/* get flDotMatchesAll param */
	
	setbooleanvalue (true, &vtemp); /* defaults to true */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flDotMatchesAll, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? PCRE_DOTALL : 0L);

	/* get flMultiLine param */
	
	setbooleanvalue (true, &vtemp); /* defaults to true */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flMultiLine, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? PCRE_MULTILINE : 0L);

	/* get flAutoCapture param */
	
	setbooleanvalue (true, &vtemp); /* defaults to true */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flAutoCapture, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? 0 : PCRE_NO_AUTO_CAPTURE);

	/* get flGreedyQuantifiers param */
	
	setbooleanvalue (true, &vtemp); /* defaults to true */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flGreedyQuantifiers, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? 0 : PCRE_UNGREEDY);

	/* get flMatchEmptyString param */
	
	setbooleanvalue (true, &vtemp); /* defaults to true */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flMatchEmptyString, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? 0 : PCRE_NOTEMPTY);

	/* get flExtendedMode param */
	
	flnextparamislast = true;
	
	setbooleanvalue (false, &vtemp); /* defaults to false */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flExtendedMode, &vtemp)) 
		return (false);
	
	options |= (vtemp.data.flvalue ? PCRE_EXTENDED : 0);
	
	/* prepare params */
	
	if (!enlargehandle (hpattern, 1, &chterminate)) /* null-terminate pattern */
		return (false);
	
	/* compile pattern */
	
	lockhandle (hpattern);
	
	fl = regexpcompile (*hpattern, options, bserror, &hcp);
	
	unlockhandle (hpattern);
	
	return (fl && setheapvalue (hcp, binaryvaluetype, v));
	} /*regexpcompileverb*/


static boolean regexpmatchverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	match (patternRef, s, adrMatchInfoTable=nil, ix = 1, ct = infinity, flMakeGroups=false)
		 //number: position of match or 0
	*/

	Handle hcp;
	Handle hovec;
	Handle hsubject;
	long ixstart;
	unsigned long matchlength;
	hdlhashtable hmatchinfotable;
	boolean flmakegroups;
	boolean flmakenamedgroups;
	tyvaluerecord vtemp;
	short ctconsumed = 2;
	short ctpositional = 2;
	long res;
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, true, bserror, &hcp, &hovec))
		return (false);
		
	/* get s parameter */
	
	if (!getreadonlytextvalue (hp1, 2, &hsubject))
		return (false);
	
	/* get ix parameter */
	
	setlongvalue (1, &vtemp); /* defaults to 1 */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_ix, &vtemp)) 
		return (false);
	
	ixstart = vtemp.data.longvalue - 1; /* convert from 1-based to 0-based*/
	
	/* get ct parameter */
	
	setlongvalue (longinfinity, &vtemp); /* defaults to 0x7fffffff */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_ct, &vtemp)) 
		return (false);
	
	matchlength = (unsigned long) vtemp.data.longvalue;
	
	/* get adrMatchInfoTable parameter */
	
	hmatchinfotable = nil;
	
	if (!getoptionaltableparam (hp1, &ctconsumed, &ctpositional, STR_adrMatchInfoTable, &hmatchinfotable))
		return (false);
		
	/* get flMakeGroups parameter */
	
	setbooleanvalue (false, &vtemp); /* defaults to false */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flMakeGroups, &vtemp)) 
		return (false);
	
	flmakegroups = vtemp.data.flvalue;
		
	/* get flmakenamedgroups parameter */

	flnextparamislast = true;
	
	setbooleanvalue (false, &vtemp); /* defaults to false */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flMakeNamedGroups, &vtemp)) 
		return (false);
	
	flmakenamedgroups = vtemp.data.flvalue;

	/* execute compiled pattern */
	
	res = regexpexechandle (hcp, hsubject, ixstart, matchlength, hovec, bserror);

	/* if it did not match, we are done! */
		
	if (res == PCRE_ERROR_NOMATCH) {
	
		releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	

		setlongvalue (0, v);
		
		return (true);
		}

	/* catch execution errors, presumably indicative of a bug in the PCRE library */
	
	if (res <= 0)
		return (false);
	
	/* build match info table */
	
	if (hmatchinfotable != nil)
		if (!regexpbuildmatchinfotable (hsubject, hcp, hovec, flmakegroups, flmakenamedgroups, bserror, hmatchinfotable))
			return (false);
	
	/* return match position */
	
	setlongvalue (getgroupoffset (hovec, 0) + 1, v);
	
	releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	
	
	return (true);
	} /*regexpmatchverb*/


static boolean regexpreplaceverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	replace (patternRef, repl, s, maxReplacements = infinity)
		 //string: modified string
	*/

	Handle hcp;
	Handle hovec = nil;
	Handle hreplace;
	long replacelength;
	Handle hsubject;
	long subjectlength;
	long maxrepl;
	tyaddress adrctrepl;
	tyaddress adrcallback;
	tyvaluerecord vtemp;
	short ctconsumed = 3;
	short ctpositional = 3;
	hdlreplacepart hreplaceparts = nil;
	handlestream hs;
	Handle hresult;
	int res;
	int ix, lastix;
	int ctrepl = 0;
	boolean fl, flresult;
	
	clearbytes (&hs, sizeof (hs));
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, false, bserror, &hcp, &hovec))
		goto exit;
		
	/* get repl parameter */
	
	if (!getreadonlytextvalue (hp1, 2, &hreplace))
		goto exit;
	
	replacelength = gethandlesize (hreplace);
		
	/* get s parameter */
	
	if (!getreadonlytextvalue (hp1, 3, &hsubject))
		goto exit;
	
	subjectlength = gethandlesize (hsubject);

	/* get maxReplacements parameter */
	
	setlongvalue (longinfinity, &vtemp); /* defaults to 0x7fffffff */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_maxReplacements, &vtemp)) 
		goto exit;
	
	maxrepl = vtemp.data.longvalue;

	/* get adrReplacementCount parameter */
	
	adrctrepl.ht = nil; /*defaults to a nil address*/
	setemptystring (adrctrepl.bs);

	if (!getoptionaladdressparam (hp1, &ctconsumed, &ctpositional, STR_adrReplacementCount, &adrctrepl.ht, adrctrepl.bs)) 
		goto exit;

	/* get adrCallback parameter */

	flnextparamislast = true;
	
	adrcallback.ht = nil; /*defaults to a nil address*/
	setemptystring (adrcallback.bs);

	if (!getoptionaladdressparam (hp1, &ctconsumed, &ctpositional, STR_adrCallback, &adrcallback.ht, adrcallback.bs)) 
		goto exit;

	/* return early if the caller hasn't asked for at least one replacement */
	
	if (maxrepl <= 0) {
		
		if (!copyhandle (hsubject, &hresult))
			return (false);
		
		goto done;
		}
	
	/* scan repl parameter */
	
	lockhandle (hreplace);
	
	fl = regexpcompilereplacement (hcp, *hreplace, replacelength, bserror, &hreplaceparts);
	
	unlockhandle (hreplace);
	
	if (!fl)
		goto exit;
	
	pushtmpstack ((Handle) hreplaceparts);

	/* run engine */
		
	openhandlestream (nil, &hs);
	
	lastix = ix = 0;
	
	while (true) { /*execute loop at least once*/	
		
		res = regexpexechandle (hcp, hsubject, ix, longinfinity, hovec, bserror);
			
		/* catch internal errors */
		
		if (res <= 0 && res != PCRE_ERROR_NOMATCH) 
			goto exit;
		
		/* if it did not match, copy remainder and we are done */
		
		if (res == PCRE_ERROR_NOMATCH) {
			
			if (!writehandlestreamhandlepart (&hs, hsubject, lastix, subjectlength - lastix))
				goto exit;
			
			break;
			}
		
		/* write fragment between last match and current match */
		
		if (!writehandlestreamhandlepart (&hs, hsubject, lastix, getgroupoffset (hovec, 0) - lastix))
			goto exit;

		/* process repl string, including captured substrings */
		
		if (!regexpreplacematch (&hs, hsubject, hreplace, hreplaceparts, hcp, hovec, &adrcallback, bserror, &flresult))
			goto exit;
		
		/* if the callback returned false or we reached max replacements, copy remainder and we are done */
		
		if (!flresult || ++ctrepl >= maxrepl) {
			
			int groupend = getgroupend (hovec, 0);
			
			if (!writehandlestreamhandlepart (&hs, hsubject, groupend, subjectlength - groupend))
				goto exit;
		
			break;
			}
		
		/* determine position of next match attempt */
		
		lastix = ix = getgroupend (hovec, 0);
		
		if (getgrouplength (hovec, 0) == 0)
			ix++; /* prevent infinite loop if we matched the empty string */
		} /*while*/	

	hresult = closehandlestream (&hs);
	
	if (hresult == nil) {
		if (!newemptyhandle (&hresult))
			goto exit;
		}

done:
	
	releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	
	
	releaseheaptmp ((Handle) hreplaceparts);
	
	/* set result */

	if (!isemptystring (adrctrepl.bs)) {
		
		setlongvalue (ctrepl, &vtemp);
		
		if (!langsetsymboltableval (adrctrepl.ht, adrctrepl.bs, vtemp)) {
			goto exit;
			}
		}
	
	return (setheapvalue (hresult, stringvaluetype, v));
	
exit:

	disposehandlestream (&hs);
	
	return (false);
	} /*regexpreplaceverb*/


static boolean regexpextractverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	extract (patternRef, s, groups = {})
		 //list: matched strings or group strings
	*/
	Handle hcp;
	Handle hovec;
	Handle hsubject;
	hdllistrecord hgroups;
	short ctconsumed = 2;
	short ctpositional = 2;
	long ctgroups;
	int **hgroupsarray;
	long ref;
	long k;
	tyvaluerecord vtemp;
	boolean fl;
	hdllistrecord hresult;
	long ix, lastix, res;
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, true, bserror, &hcp, &hovec))
		return (false);
		
	/* get s parameter */
	
	if (!getreadonlytextvalue (hp1, 2, &hsubject))
		return (false);
	
	/* get groups parameter */
	
	if (!getoptionallistparam (hp1, &ctconsumed, &ctpositional, STR_groups, &hgroups))
		return (false);
	
	/* convert groups list to internal format */
	
	ctgroups = opcountlistitems (hgroups);
	
	if (ctgroups == 0) {
	
		if (!newhandle (sizeof (int), (Handle *) &hgroupsarray))
			return (false);
		
		if (!pushtmpstack ((Handle) hgroupsarray))
			return (false);
		
		(*hgroupsarray)[0] = 0;
		
		ctgroups = 1;
		}
	else {
	
		if (!newclearhandle (ctgroups * sizeof (int), (Handle *) &hgroupsarray))
			return (false);
		
		if (!pushtmpstack ((Handle) hgroupsarray))
			return (false);
		
		for (k = 0; k < ctgroups; k++) {
		
			if (!getnthlistval (hgroups, k+1, nil, &vtemp))
				return (false);
			
			disablelangerror ();
			
			fl = coercetolong (&vtemp);
			
			enablelangerror ();
			
			if (fl) {
				ref = vtemp.data.longvalue;
				}
			else {
				
				if (!coercetostring (&vtemp))
					return (false);
				
				if (!enlargehandle (vtemp.data.binaryvalue, 1, nil)) /*terminate string with nil char*/
					return (false);
				
				ref = pcre_get_stringnumber(getpatternref (hcp), *vtemp.data.binaryvalue);
				}

			if (ref < 0 || ref > getcapturecount (hcp)) {
				regexpverberrorwithnumber (regexpnonexistantgrouperror, k+1, bserror);
				return (false);
				}
			
			(*hgroupsarray)[k] = ref;
			} /*for*/	
		}
	
	/* prepare result list */
	
	if (!opnewlist (&hresult, false))
		return (false);
	
	if (!setheapvalue ((Handle) hresult, listvaluetype, v))
		return (false);
	
	/* run engine */
	
	lastix = ix = 0;
	
	while (true) { /*execute loop at least once*/	
		
		res = regexpexechandle (hcp, hsubject, ix, longinfinity, hovec, bserror);
			
		/* catch internal errors */
		
		if (res <= 0 && res != PCRE_ERROR_NOMATCH) 
			return (false);
		
		/* if it did not match, we are done */
		
		if (res == PCRE_ERROR_NOMATCH)
			break;

		/* extract requested substrings */
		
		if (!regexpextractmatch (hsubject, hovec, hgroupsarray, ctgroups, hresult))
			return (false);
		
		/* determine position of next match attempt */
		
		lastix = ix = getgroupend (hovec, 0);
		
		if (getgrouplength (hovec, 0) == 0)
			ix++; /* prevent infinite loop if we matched the empty string */
		} /*while*/	
	
	releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	
	
	releaseheaptmp ((Handle) hgroupsarray); /*no need to accumulate these on the tmp stack*/	
	
	return (true);
	} /*regexpextractverb*/


static boolean regexpsplitverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	split (patternRef, s, maxChunks = infinity)
		//list: unmatched fragments
	*/
	Handle hcp;
	Handle hovec;
	Handle hsubject;
	long subjectlength;
	long maxchunks, ctchunks;
	tyvaluerecord vtemp;
	short ctconsumed = 2;
	short ctpositional = 2;
	hdllistrecord hresult;
	int res;
	int ix, lastix, k;
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, true, bserror, &hcp, &hovec))
		return (false);
		
	/* get s parameter */
	
	if (!getreadonlytextvalue (hp1, 2, &hsubject))
		return (false);
	
	subjectlength = gethandlesize (hsubject);

	/* get STR_maxSplits parameter */

	flnextparamislast = true;
	
	setlongvalue (longinfinity, &vtemp); /* defaults to 0x7fffffff */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_maxSplits, &vtemp)) 
		return (false);
	
	maxchunks = vtemp.data.longvalue;
	
	/* prepare result list */
	
	if (!opnewlist (&hresult, false))
		return (false);
	
	if (!setheapvalue ((Handle) hresult, listvaluetype, v))
		return (false);

	/* return early if the caller hasn't asked for at least one replacement */
	
	if (maxchunks <= 1) {
		
		initvalue (&vtemp, stringvaluetype);
		
		vtemp.data.stringvalue = hsubject;
		
		return (langpushlistval (hresult, nil, &vtemp));
		}
	
	/* run engine */
	
	lastix = ix = 0;
	
	ctchunks = 0;
	
	while (true) { /*execute loop at least once*/
		
		/* if we are about to reach maxchunks, add remainder and return */
		
		if (++ctchunks == maxchunks) {
			if (!pushhandlepartonlist (hresult, hsubject, lastix, subjectlength - lastix))
				return (false);
			break;
			}
		
		/* attempt match */
		
		res = regexpexechandle (hcp, hsubject, ix, longinfinity, hovec, bserror);
			
		/* catch internal errors */
		
		if (res <= 0 && res != PCRE_ERROR_NOMATCH) 
			return (false);
		
		/* if it did not match, add remainder and return */
		
		if (res == PCRE_ERROR_NOMATCH) {
			if (!pushhandlepartonlist (hresult, hsubject, lastix, subjectlength - lastix))
				return (false);
			break;
			}
		
		/* add fragment between last match and current match */
		
		if (!pushhandlepartonlist (hresult, hsubject, lastix, getgroupoffset (hovec, 0) - lastix))
			return (false);
		
		/* add any captured substrings of the delimiter */
		
		for (k = 1; k <= getcapturecount (hcp); k++) {
			
			if (!regexppushonematchonlist (hsubject, hovec, k, hresult))
				return (false);
		
			} /*for*/
		
		/* determine position of next match attempt */
		
		lastix = ix = getgroupend (hovec, 0);
		
		if (getgrouplength (hovec, 0) == 0)
			ix++; /* prevent infinite loop if we matched the empty string */
		} /*while*/	
	
	releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	
	
	return (true);
	} /*regexpsplitverb*/


static boolean regexpjoinverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	join (s, strList)
		//string: elements of strList glued together with s
	*/

	Handle hglue;
	tyvaluerecord vlist;
	handlestream hs;
	Handle hresult;
	tywritelistinfo info;
	
	clearbytes (&hs, sizeof (hs));
		
	/* get s parameter */
	
	if (!getreadonlytextvalue (hp1, 1, &hglue))
		return (false);
	
	/* get strList parameter */
	
	if (!getreadonlyparamvalue (hp1, 2, &vlist))
		return (false);
	
	if (vlist.valuetype != listvaluetype) {
	
		if (!vlist.fltmpstack && !copyvaluerecord (vlist, &vlist))
			return (false);
		
		if (!coercetolist (&vlist, listvaluetype))
			return (false);
		}
	
	/* prepare result */

	openhandlestream (nil, &hs);
	
	/* run engine */
	
	info.s = &hs;
	info.hglue = hglue;
	
	if (!langvisitlistvalues (&vlist, &regexpwritelistitemscallback, (ptrvoid) &info))
		goto exit;
	
	/* set result, shrink handlestream to remove final copy of hglue */
	
	hresult = closehandlestream (&hs);
	
	if (hresult != nil) {
		if (!sethandlesize (hresult, gethandlesize (hresult) - gethandlesize (hglue)))
			goto exit;
		}
	else {
		if (!newemptyhandle (&hresult))
			goto exit;
		}

	return (setheapvalue (hresult, stringvaluetype, v));

exit:

	disposehandlestream (&hs);
	
	return (false);
	} /*regexpjoinverb*/


static boolean regexpvisitverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	visit (patternRef, s, adrCallBack, flMakeGroups = false, maxRuns = infinity)
		//boolean: false if callback returned false
	*/

	Handle hcp;
	Handle hovec;
	Handle hsubject;
	tyaddress adrcallback;
	boolean flmakegroups, flmakenamedgroups;
	long maxruns;
	short ctconsumed = 3;
	short ctpositional = 3;
	tyvaluerecord vtemp;
	int ix, lastix, res;
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, false, bserror, &hcp, &hovec))
		return (false);
		
	/* get s parameter */
	
	if (!gettextvalue (hp1, 2, &hsubject)) /*better get a copy, who knows what the callback script might do to the original*/
		return (false);

	/* get adrCallback parameter */
	
	if (!getaddressparam (hp1, 3, &vtemp))
		return (false);
	
	if (!getaddressvalue (vtemp, &adrcallback.ht, adrcallback.bs))
		return (false);
		
	/* get flMakeGroups parameter */
	
	setbooleanvalue (false, &vtemp); /* defaults to false */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flMakeGroups, &vtemp)) 
		return (false);
	
	flmakegroups = vtemp.data.flvalue;
		
	/* get flmakenamedgroups parameter */
	
	setbooleanvalue (false, &vtemp); /* defaults to false */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flMakeNamedGroups, &vtemp)) 
		return (false);
	
	flmakenamedgroups = vtemp.data.flvalue;

	/* get maxRuns parameter */

	flnextparamislast = true;
	
	setlongvalue (longinfinity, &vtemp); /* defaults to 0x7fffffff */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_maxRuns, &vtemp)) 
		return (false);
	
	maxruns = vtemp.data.longvalue;
	
	/* run engine */
	
	lastix = ix = 0;
	
	setbooleanvalue (true, v);
	
	while (true) { /*execute loop at least once*/	
		
		res = regexpexechandle (hcp, hsubject, ix, longinfinity, hovec, bserror);
			
		/* catch internal errors */
		
		if (res <= 0 && res != PCRE_ERROR_NOMATCH)
			return (false);
		
		/* if it did not match, we are done */
		
		if (res == PCRE_ERROR_NOMATCH)
			break;

		/* run callback script, break if it returns false */

		if (!regexprunvisitcallback (hsubject, hcp, hovec, flmakegroups, flmakenamedgroups, &adrcallback, bserror, v))
			return (false);
		
		if (!coercetoboolean (v))
			return (false);
		
		if (!(*v).data.flvalue)
			break;
		
		/* if we reached max runs, copy remainder and we are done */
		
		if (--maxruns <= 0)
			break;
		
		/* determine position of next match attempt */
		
		lastix = ix = getgroupend (hovec, 0);
		
		if (getgrouplength (hovec, 0) == 0)
			ix++; /* prevent infinite loop if we matched the empty string */
		} /*while*/	
	
	releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	

	return (true);
	} /*regexpvisitverb*/


static boolean regexpgrepverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	grep (patternRef, s, flIncludeMatches = true)
		//list: filtered copy of s
	*/
	
	Handle hcp;
	Handle hovec;
	tyvaluerecord vsubject;
	Handle hsubject;
	long subjectlength;
	boolean flincludematches;	
	short ctconsumed = 2;
	short ctpositional = 2;
	tyvaluerecord vtemp;
	hdllistrecord hresult;
	char *p, *pstart, *pend;
	int res, ix, len;
	pcre_extra extra;
	boolean flsuccess = false;
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, true, bserror, &hcp, &hovec))
		return (false);

	/* get s parameter (list or string) */
	
	if (!getreadonlyparamvalue (hp1, 2, &vsubject))
		return (false);
	
	if (vsubject.valuetype != listvaluetype) {
		if (!coercetostring (&vsubject))
			return (false);
		}
	
	/* get flIncludeMatches parameter */

	flnextparamislast = true;
	
	setbooleanvalue (true, &vtemp); /* defaults to true */

	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, STR_flIncludeMatches, &vtemp)) 
		return (false);
	
	flincludematches = vtemp.data.flvalue;
	
	/* prepare result list */
	
	if (!opnewlist (&hresult, false))
		return (false);
	
	if (!setheapvalue ((Handle) hresult, listvaluetype, v))
		return (false);
	
	/* handle subject as list */
	
	if (vsubject.valuetype == listvaluetype) {
		
		tygreplistinfo info;
		
		info.hcp = hcp;
		info.hovec = hovec;
		info.hresult = hresult;
		info.flincludematches = flincludematches;
		info.bserrorptr = (ptrstring) &bserror;
		
		flsuccess = langvisitlistvalues (&vsubject, &regexpgreplistitemscallback, (ptrvoid) &info);
	
		releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/
		
		return (flsuccess);
		}
	
	/* handle subject as string */
	
	hsubject = vsubject.data.stringvalue;
	
	subjectlength = gethandlesize (hsubject);

	lockhandle (hsubject); /*pointers into hsubject need to survive pushhandlepartonlist call*/
	lockhandle (hcp);
	lockhandle (hovec);
	
	initpcreextra (hcp, &extra);
	
	p = *hsubject;
	
	pend = *hsubject + subjectlength;

	while (true) {
		
		/* get next line */
		
		pstart = p;
		
		while (p < pend) {
			
			if (*p == '\r')
				break;
			
			p++;
			} /*while*/
		
		/* try to match line */
		
		ix = pstart - *hsubject;
		
		len = p - pstart;

		res = regexpexec (hcp, pstart, len, 0, len, hovec, bserror);
			
		/* catch internal errors */
		
		if (res <= 0 && res != PCRE_ERROR_NOMATCH) {
			regexpverberrorwithnumber (regexpinternalerror, res, bserror);
			break;
			}
	
		/* possibly add line to result list */

		if (flincludematches ? (res > 0) : (res == PCRE_ERROR_NOMATCH))
			if (!pushhandlepartonlist (hresult, hsubject, ix, len)) /* function call might move memory */
				break;
		
		/* advance and skip newline char */
		
		p++;
		
		if (*p == '\n')
			p++;
		
		/* check wether we are done */
		
		if (p >= pend) {
			flsuccess = true;
			break;
			}
		
		} /*while*/
	
	unlockhandle (hovec);
	unlockhandle (hcp);
	unlockhandle (hsubject);
	
	releaseheaptmp (hovec); /*no need to accumulate these on the tmp stack*/	
	
	return (flsuccess);
	} /*regexpgrepverb*/


static boolean regexpgetpatterninfoverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	on getPatternInfo (patternRef, adrInfoTable)
		//boolean: true
	*/
	
	Handle hcp;
	hdlhashtable htable, htinfo, htfirstbyte, htnames, htoptions;
	bigstring bsname, bsnum;
	unsigned char *ucptr;
	char *cptr;
	int x, k, ct, sz;
	
	/* get patternRef parameter */
	
	if (!regexpgetpatternvalue (hp1, 1, true, bserror, &hcp, nil))
		return (false);
	
	/* get adrInfoTable parameter */

	if (!getvarparam (hp1, 2, &htable, bsname))
		return (false);

	if (!langsuretablevalue (htable, bsname, &htinfo))
		return (false);

	/* highest back reference */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_BACKREFMAX, bserror, (void *) &x))
		return (false);
	
	if (!langassignlongvalue (htinfo, STR_backRefMax, x))
		return (false);

	/* capture count */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_CAPTURECOUNT, bserror, (void *) &x))
		return (false);
	
	if (!langassignlongvalue (htinfo, STR_captureCount, x))
		return (false);

	/* first byte */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_FIRSTBYTE, bserror, (void *) &x))
		return (false);
	
	if (x < 0) {
		if (!langassignlongvalue (htinfo, STR_firstByte, x))
			return (false);
		}
	else {
		if (!langassigncharvalue (htinfo, STR_firstByte, (unsigned char) x))
			return (false);
		}

	/* first byte table*/
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_FIRSTTABLE, bserror, (void *) &ucptr))
		return (false);
	
	if (!langassignnewtablevalue (htinfo, STR_firstByteTable, &htfirstbyte))
		return (false);
	
	if (ucptr != nil) {
	
		for (k = 0; k < 256; k++) {
		
			numbertostring (k, bsnum);
			
			padwithzeros (bsnum, 3);
			
			if (!langassignbooleanvalue (htfirstbyte, bsnum, ucptr[k/8] & (1 << (k % 8))))
				return (false);
			} /*for*/
		}

	/* last literal */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_LASTLITERAL, bserror, (void *) &x))
		return (false);
	
	if (x < 0) {
		if (!langassignlongvalue (htinfo, STR_lastLiteral, x))
			return (false);
		}
	else {
		if (!langassigncharvalue (htinfo, STR_lastLiteral, (unsigned char) x))
			return (false);
		}

	/* name table */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_NAMECOUNT, bserror, (void *) &ct))
		return (false);
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_NAMEENTRYSIZE, bserror, (void *) &sz))
		return (false);
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_NAMETABLE, bserror, (void *) &cptr))
		return (false);

	if (!langassignnewtablevalue (htinfo, STR_nameTable, &htnames))
		return (false);

	if (cptr != nil) {
		
		bigstring bs;
		int ix;
		
		for (k = 0; k < ct; k++) {
			
			ix = (cptr[k*sz] << 8) + cptr[k*sz+1];
			
			copyctopstring (&(cptr[k*sz+2]), bs);

			if (!langassignlongvalue (htnames, bs, ix))
				return (false);
		
			} /*for*/
		}

	/* options */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_OPTIONS, bserror, (void *) &x))
		return (false);
	
	if (!langassignnewtablevalue (htinfo, STR_options, &htoptions))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flCaseSensitive, (x & PCRE_CASELESS) == 0))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flDotMatchesAll, (x & PCRE_DOTALL) != 0))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flMultiLine, (x & PCRE_MULTILINE) != 0))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flAutoCapture, (x & PCRE_NO_AUTO_CAPTURE) == 0))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flGreedyQuantifiers, (x & PCRE_UNGREEDY) == 0))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flMatchEmptyString, (x & PCRE_NOTEMPTY) == 0))
		return (false);

	if (!langassignbooleanvalue (htoptions, STR_flExtendedMode, (x & PCRE_EXTENDED) == 0))
		return (false);

	/* size */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_SIZE, bserror, (void *) &x))
		return (false);
	
	if (!langassignlongvalue (htinfo, STR_size, x))
		return (false);

	/* study size */
	
	if (!regexpgetpatterninfo (hcp, PCRE_INFO_STUDYSIZE, bserror, (void *) &x))
		return (false);
	
	if (!langassignlongvalue (htinfo, STR_studySize, x))
		return (false);

	/* clean up and return */
	
	setbooleanvalue (true, v);
			
	return (true);
	} /*regexpgetpatterninfoverb*/


static boolean regexpexpandverb (hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {
	
	/*
	on expand (s, adrMatchInfoTable)
		//string: processed copy of s
	*/

	Handle hsubject;
	long subjectlength;
	tyvaluerecord vtemp;
	hdlhashtable htmatchinfo;
	hdlhashnode hnode;
	tyreplscanexpandinfo info;
	Handle hresult;
	boolean fl;
		
	/* get s parameter */
	
	if (!getreadonlytextvalue (hp1, 1, &hsubject))
		return (false);
	
	subjectlength = gethandlesize (hsubject);
	
	/* get adrMatchInfoTable parameter */
	
	if (!gettablevalue (hp1, 2, &htmatchinfo))
		return (false);

	/* get groupStrings list from info table*/
		
	if (!langhashtablelookup (htmatchinfo, STR_groupStrings, &vtemp, &hnode))
		return (false);
	
	if (vtemp.valuetype != listvaluetype) {
	
		if (!copyvaluerecord (vtemp, &vtemp) || !coercetolist (&vtemp, listvaluetype)) {
			return (false);
			}
		}
			
	info.hgroups = (hdllistrecord) vtemp.data.binaryvalue;

	/* get namedGroups sub-table from info table*/

	if (hashtablelookup (htmatchinfo, STR_namedGroups, &vtemp, &hnode)) {
		if (!langexternalvaltotable (vtemp, &info.htnames, hnode)) {
			return (false);
			}
		}
	
	/* perform replacement */
	
	info.hsubject = hsubject;

	openhandlestream (nil, &info.s);
	
	lockhandle (hsubject);

	fl = regexpscanreplacement (*hsubject, subjectlength, bserror,
									&replscanwriteliteral,
									&replscanwritenumbered,
									&replscanwritenamed,
									&replscanerror,
									(void *) &info);
	
	unlockhandle (hsubject);
	
	if (!fl) {
		disposehandlestream (&info.s);
		return (false);
		}
		
	hresult = closehandlestream (&info.s);
	
	if (hresult == nil) {
		if (!newemptyhandle (&hresult))
			return (false);
		}
		
	return (setheapvalue (hresult, stringvaluetype, v));
	} /*regexpexpandverb*/


static boolean regexpfunctionvalue (short token, hdltreenode hp1, tyvaluerecord *v, bigstring bserror) {

	/*
	2003-04-22 AR: kernel verbs for regular expressions
	*/
	
	setbooleanvalue (false, v); /*by default, regexp functions return false*/
	
	switch (token) {

		case compilefunc:
			return (regexpcompileverb (hp1, v, bserror));

		case matchfunc:
			return (regexpmatchverb (hp1, v, bserror));

		case replacefunc:
			return (regexpreplaceverb (hp1, v, bserror));

		case extractfunc:
			return (regexpextractverb (hp1, v, bserror));

		case splitfunc:
			return (regexpsplitverb (hp1, v, bserror));

		case joinfunc:
			return (regexpjoinverb (hp1, v, bserror));

		case visitfunc:
			return (regexpvisitverb (hp1, v, bserror));

		case grepfunc:
			return (regexpgrepverb (hp1, v, bserror));
		
		case getpatterninfofunc:
			return (regexpgetpatterninfoverb (hp1, v, bserror));
		
		case expandfunc:
			return (regexpexpandverb (hp1, v, bserror));

		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			return (false);
		} /*switch*/
	} /*regexpfunctionvalue*/


boolean regexpinitverbs (void) {

	/*
	2003-04-22 AR: new verbs. Call me from shellinit in shell.c.
	*/
	
	assert (sizeof (regexp_default_tables) == tables_length); /*if this fails, something changed in the PCRE library*/
	
	chartableptr = regexp_default_tables; //was: pcre_maketables (); /*build char tables using current locale*/
	
	return (loadfunctionprocessor (idregexpverbs, &regexpfunctionvalue));
	} /*regexpinitverbs*/


#endif /* flregexpverbs */

