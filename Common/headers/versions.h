
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
	versions.h
	
	Defines version numbers for Frontier and Radio UserLand for all platforms.
	
	JES 12/04/2002
*/ 


#define	radio_major_version				8
#define	radio_major_version_bcd			0x08 	/* major version in BCD notation */

#define	radio_sub_version				1
#define	radio_minor_version				0
#define	radio_subminor_version_bcd		0x10  	/* sub and minor version in BCD notation */

#define radio_stage_code				0x60	/* dev = 0x20, alpha = 0x40, beta = 0x60, final = 0x80 */
#define	radio_revision_level			4		/* for non-final releases only */
#define	radio_build_number				4		/* increment by one for every release, final or not */

#define radio_version_string			"8.1b4"


#define	frontier_major_version			10
#define	frontier_major_version_bcd		0x10 	/* major version in BCD notation */

#define	frontier_sub_version			0
#define	frontier_minor_version			0
#define	frontier_subminor_version_bcd	0x00  	/* sub and minor version in BCD notation */

#define frontier_stage_code				0x40	/* dev = 0x20, alpha = 0x40, beta = 0x60, final = 0x80 */
#define	frontier_revision_level			2		/* for non-final releases only */
#define	frontier_build_number			2		/* increment by one for every release, final or not */

#define frontier_version_string			"10.0a2"


#define	copyright_year_string	"2004"


/* Define app name and related strings */
#ifdef PIKE
	#define APPNAME						"Radio UserLand"
	#define APPNAME_SHORT				"Radio"
#else
	#define APPNAME						"UserLand Frontier"
	#define APPNAME_SHORT				"Frontier"
#endif

/* app name for display -- ends with the trademark character */
#ifdef MACVERSION
	#define APPNAME_TM					APPNAME "\xAA"
#endif
#ifdef WIN95VERSION
	#define APPNAME_TM					APPNAME "\x99"
#endif
