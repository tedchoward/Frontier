
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

/* Frontier Windows Registry defines */


#include "lang.h"

#ifdef REG_RESOURCE_LIST
	#define regtype_none						( REG_NONE )   // No value type
	#define regtype_string                      ( REG_SZ )   // Unicode nul terminated string
	#define regtype_expand_string               ( REG_EXPAND_SZ )   // Unicode nul terminated string
														// (with environment variable references)
	#define regtype_binary						( REG_BINARY )   // Free form binary
	#define regtype_ulong						( REG_DWORD )   // 32-bit number
	#define regtype_ulong_little_endian			( REG_DWORD_LITTLE_ENDIAN )   // 32-bit number (same as REG_DWORD)
	#define regtype_ulong_big_endian			( REG_DWORD_BIG_ENDIAN )   // 32-bit number
	#define regtype_link						( REG_LINK )   // Symbolic Link (unicode)
	#define regtype_unicode						( REG_MULTI_SZ )   // Multiple Unicode strings
	#define regtype_resource_list				( REG_RESOURCE_LIST )   // Resource list in the resource map
	#define regtype_full_resource_descriptor	( REG_FULL_RESOURCE_DESCRIPTOR )  // Resource list in the hardware description
	#define regtype_resource_requirements_list	( REG_RESOURCE_REQUIREMENTS_LIST )

#else

	#define regtype_none						( 0 )   // No value type
	#define regtype_string                      ( 1 )   // Unicode nul terminated string
	#define regtype_expand_string               ( 2 )   // Unicode nul terminated string
														// (with environment variable references)
	#define regtype_binary						( 3 )   // Free form binary
	#define regtype_ulong						( 4 )   // 32-bit number
	#define regtype_ulong_little_endian			( 4 )   // 32-bit number (same as REG_DWORD)
	#define regtype_ulong_big_endian			( 5 )   // 32-bit number
	#define regtype_link						( 6 )   // Symbolic Link (unicode)
	#define regtype_unicode						( 7 )   // Multiple Unicode strings
	#define regtype_resource_list				( 8 )   // Resource list in the resource map
	#define regtype_full_resource_descriptor	( 9 )  // Resource list in the hardware description
	#define regtype_resource_requirements_list	( 10 )
#endif

#define STR_P_HKCU_SHORT	"\x04" "HKCU"
#define STR_P_HKCU_LONG		"\x11" "HKEY_CURRENT_USER"
#define STR_P_HKLM_SHORT	"\x04" "HKLM"
#define STR_P_HKLM_LONG		"\x12" "HKEY_LOCAL_MACHINE"
#define STR_P_HKCR_SHORT	"\x04" "HKCR"
#define STR_P_HKCR_LONG		"\x11" "HKEY_CURRENT_USER"
#define STR_P_HKU_SHORT		"\x03" "HKU"
#define STR_P_HKU_LONG		"\x0a" "HKEY_USERS"
#define STR_P_HKCC_SHORT	"\x04" "HKCC"
#define STR_P_HCC_LONG		"\x13" "HKEY_CURRENT_CONFIG"

#define STR_P_REG_BINARY				"\x0a" "REG_BINARY"
#define STR_P_REG_DWORD					"\x09" "REG_DWORD"
#define STR_P_REG_DWORD_LITTLE_ENDIAN	"\x13" "DWORD_LITTLE_ENDIAN"
#define STR_P_REG_DWORD_BIG_ENDIAN		"\x10" "DWORD_BIG_ENDIAN"
#define STR_P_REG_EXPAND_SZ				"\x09" "EXPAND_SZ"
#define STR_P_REG_LINK					"\x08" "REG_LINK"
#define STR_P_REG_MULTI_SZ				"\x0c" "REG_MULTI_SZ"
#define STR_P_REG_NONE					"\x08" "REG_NONE"
#define STR_P_REG_QWORD					"\x09" "REG_QWORD"
#define STR_P_REG_QWORD_LITTLE_ENDIAN	"\x13" "QWORD_LITTLE_ENDIAN"
#define STR_P_REG_RESOURCE_LIST			"\x11" "REG_RESOURCE_LIST"
#define STR_P_REG_SZ					"\x06" "REG_SZ"
#define STR_P_UNKNOWNTYPE				"\x07" "UNKNOWN"	

/* Frontier Windows Registry functions */

/* This function is a single line registration of the extension, its content type, its filetype and its description
and the default icon and shell open command needed.  

  extension - is the .extension to register - must specify and will be a key in HKEY_CLASSES_ROOT
  filetype - this is the name of the other key in HKEY_CLASSES_ROOT and the defualt value of the extensions key - must be specified 
  contenttype - this is optional - if specified will value of the value "Content Type" for the extensions key
  filedescription - this is optional - if not specified the function will not create the filetype key also 
					iconfile, iconid, & shellopen are ignored.  If it is specified the filetype key is created with
					the default value set to this filedescription.
  iconfile - this is optional and only used if filedescription is specified.  If specified is the filename that contians
					the icon for this filetype.  a DefaultIcon subkey will be created and its value will be set to this
					value followed be a comma and the value of iconid. 
  iconid - optional - see above
  shellopen - optional - if specified creates a shell - open - command subkey and sets its value to this parameter.
*/

boolean registerFileType (bigstring extension, bigstring filetype, bigstring contenttype, bigstring filedescription, bigstring iconfile, short iconid, bigstring shellopen);


boolean setRegKeyString (Handle regkey, bigstring subkey, bigstring itemname, bigstring bsval);

boolean setRegKeyLong (Handle regkey, bigstring subkey, bigstring itemname, unsigned long val);

boolean setRegKeyBinary (Handle regkey, bigstring subkey, bigstring itemname, unsigned char * val, unsigned long len);

boolean getRegKeyInfo (Handle regkey, bigstring subkey, bigstring itemname, unsigned long *regType, unsigned long * len);

boolean getRegKeyString (Handle regkey, bigstring subkey, bigstring itemname, bigstring bsval);

boolean getRegKeyLong (Handle regkey, bigstring subkey, bigstring itemname, unsigned long * val);

boolean getRegKeyBinary (Handle regkey, bigstring subkey, bigstring itemname, unsigned char * val, unsigned long  * len);

boolean setProfileString (bigstring itemname, bigstring bsval);

boolean setProfileLong (bigstring itemname, unsigned long val);

boolean setProfileData (bigstring itemname, unsigned char * val, unsigned long len);

boolean getProfileString (bigstring itemname, bigstring bsval);

boolean getProfileLong (bigstring itemname, unsigned long * val);

boolean getProfileData (bigstring itemname, unsigned char * val, unsigned long * len);

extern boolean winreggettype (Handle h, bigstring bstype); /*7.0.2b1 Radio PBS*/

extern boolean winregread (Handle h, tyvaluerecord *v);

extern boolean winregdelete (Handle h);

extern boolean winregwrite (Handle h, tyvaluerecord *val, bigstring bstype);
