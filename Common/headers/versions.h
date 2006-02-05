
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
	versions.h
	
	Defines version numbers for Frontier and Radio UserLand for all platforms.
	
	JES 12/04/2002
*/ 


/* Radio version information */

#define	radio_major_version				10
#define	radio_major_version_bcd			0x10 	/* major version in BCD notation */

#define	radio_sub_version				1
#define	radio_minor_version				0
#define	radio_subminor_version_bcd		0x10  	/* sub and minor version in BCD notation */

#define	radio_stage_code				0x40		/* dev = 0x20, alpha = 0x40, beta = 0x60, final = 0x80 */
#define	radio_revision_level			5		/* for non-final releases only */
#define	radio_build_number				5		/* increment by one for every release, final or not */

#define	radio_version_string				"10.1a5"


/* Frontier version information */

#define	frontier_major_version			10
#define	frontier_major_version_bcd		0x10 	/* major version in BCD notation */

#define	frontier_sub_version			1
#define	frontier_minor_version			0
#define	frontier_subminor_version_bcd	0x10  	/* sub and minor version in BCD notation */

#define	frontier_stage_code				0x40		/* dev = 0x20, alpha = 0x40, beta = 0x60, final = 0x80 */
#define	frontier_revision_level			5		/* for non-final releases only */
#define	frontier_build_number			5		/* increment by one for every release, final or not */

#define	frontier_version_string			"10.1a5"


/* OPML Editor version information */

#define	opml_major_version				10
#define	opml_major_version_bcd			0x10 	/* major version in BCD notation */

#define	opml_sub_version				1
#define	opml_minor_version				0
#define	opml_subminor_version_bcd		0x10  	/* sub and minor version in BCD notation */

#define opml_stage_code					0x40	/* dev = 0x20, alpha = 0x40, beta = 0x60, final = 0x80 */
#define	opml_revision_level				5		/* for non-final releases only */
#define	opml_build_number				5		/* increment by one for every release, final or not */

#define	opml_version_string			"10.1a5"


/* strings for all apps */

#define	copyright_year_string	"2006"


/* Define app name and related strings */
#ifdef PIKE
	#ifndef OPMLEDITOR
		#define APPNAME		"Radio"
	#else
		#define APPNAME		"OPML"
	#endif
#else
	#define APPNAME			"Frontier"
#endif

#define APPNAME_SHORT	APPNAME		/* 2006-02-04 aradke */

/* app name for display -- ends with the trademark character */
/* 2005-01-12 aradke: app names no longer include trademark character */
#define APPNAME_TM		APPNAME


/* 2006-02-04 aradke: Product slogan and url as shown in about window */

#ifdef PIKE
	#ifndef OPMLEDITOR
		#define app_name		"\x05" "Radio"
		#define app_slogan		"\x2b" "The power of Web publishing on your desktop"
		#define app_copyright	"\x23" "© 1992-" copyright_year_string " UserLand Software, Inc."
		#define app_url			"\x26" "http://frontierkernel.sourceforge.net/"
	#else
		#define app_name		"\x04" "OPML"
		#define app_slogan		"\x25" "Powerful OPML editing on your desktop"
		#define app_copyright	"\x20" "© 1992-" copyright_year_string " Scripting News, Inc."
		#define app_url			"\x18" "http://support.opml.org/"
	#endif
#else
	#define app_name			"\x08" "Frontier"
	#define app_slogan			"\x25" "Powerful cross-platform web scripting"
	#define app_copyright		"\x23" "© 2004-" copyright_year_string " Frontier Kernel Project"
	#define app_url				"\x26" "http://frontierkernel.sourceforge.net/"
#endif
