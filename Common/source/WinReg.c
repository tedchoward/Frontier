
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

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#endif

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "lang.h"
#include "winreg.h"

boolean setRegKeyString (Handle regkey, bigstring subkey, bigstring itemname, bigstring bsval) {
    HKEY hkey;
	DWORD what;
	char key[256];
	char item[256];
	char val[256];
	char * itemptr;

	if ((subkey == NULL) || (bsval == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);
	copyptocstring (bsval, val);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegCreateKeyEx ((HKEY) regkey, key, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &what) == ERROR_SUCCESS) {
		
		RegSetValueEx (hkey, itemptr, 0, REG_SZ, val, strlen (val)+1);
			
		RegCloseKey (hkey);

		return (true);
		}

	return (false);
	} /*setRegKeyString*/

boolean setRegKeyLong (Handle regkey, bigstring subkey, bigstring itemname, unsigned long val) {
    HKEY hkey;
	DWORD what;
	char key[256];
	char item[256];
	char * itemptr;

	if (subkey == NULL)
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegCreateKeyEx ((HKEY) regkey, key, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &what) == ERROR_SUCCESS) {
		
		RegSetValueEx (hkey, itemptr, 0, REG_DWORD_LITTLE_ENDIAN, (unsigned char *) &val, sizeof(long));
			
		RegCloseKey (hkey);

		return (true);
		}

	return (false);
	} /*setRegKeyLong*/

boolean setRegKeyBinary (Handle regkey, bigstring subkey, bigstring itemname, unsigned char * val, unsigned long len) {
    HKEY hkey;
	DWORD what;
	char key[256];
	char item[256];
	char * itemptr;

	if ((subkey == NULL) || (val == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegCreateKeyEx ((HKEY) regkey, key, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &what) == ERROR_SUCCESS) {
		
		RegSetValueEx (hkey, itemptr, 0, REG_BINARY, val, len);
			
		RegCloseKey (hkey);

		return (true);
		}

	return (false);
	} /*setRegKeyBinary*/

boolean getRegKeyInfo (Handle regkey, bigstring subkey, bigstring itemname, unsigned long *regType, unsigned long * len) {
    HKEY hkey;
	char key[256];
	char item[256];
	char * itemptr;
	long err;

	if ((subkey == NULL) || (regType == NULL) || (len == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegOpenKeyEx ((HKEY) regkey, key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		
		*len = 0;

		err = RegQueryValueEx (hkey, itemptr, NULL, regType, NULL, len);
			
		RegCloseKey (hkey);

		if (err != ERROR_SUCCESS)
			return (false);

		return (true);
		}

	return (false);
	} /*getRegKeyInfo*/

boolean getRegKeyString (Handle regkey, bigstring subkey, bigstring itemname, bigstring bsval) {
    HKEY hkey;
	char key[256];
	char item[256];
	char * itemptr;
	DWORD regtype;
	long err;
	DWORD len;

	if ((subkey == NULL) || (bsval == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegOpenKeyEx ((HKEY) regkey, key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		
		len = sizeof(bigstring) - 1;

		err = RegQueryValueEx (hkey, itemptr, NULL, &regtype, stringbaseaddress(bsval), &len);
			
		RegCloseKey (hkey);

		if (err != ERROR_SUCCESS)
			return (false);

		if (regtype != REG_SZ)
			return (false);

		setstringlength (bsval, len - 1);

		return (true);
		}

	return (false);
	} /*getRegKeyString*/

boolean getRegKeyLong (Handle regkey, bigstring subkey, bigstring itemname, unsigned long * val) {
    HKEY hkey;
	char key[256];
	char item[256];
	char * itemptr;
	DWORD regtype;
	long err;
	DWORD len;

	if ((subkey == NULL) || (val == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegOpenKeyEx ((HKEY) regkey, key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		
		len = sizeof(long);

		err = RegQueryValueEx (hkey, itemptr, NULL, &regtype, (unsigned char *) val, &len);
			
		RegCloseKey (hkey);

		if (err != ERROR_SUCCESS)
			return (false);

		if (regtype != REG_DWORD_LITTLE_ENDIAN)
			return (false);

		if (len != sizeof(long))
			return (false);

		return (true);
		}

	return (false);
	} /*getRegKeyLong*/

boolean getRegKeyBinary (Handle regkey, bigstring subkey, bigstring itemname, unsigned char * val, unsigned long  * len) {
    HKEY hkey;
	char key[256];
	char item[256];
	char * itemptr;
	DWORD regtype;
	long err;

	if ((subkey == NULL) || (val == NULL) || (len == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegOpenKeyEx ((HKEY) regkey, key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		
		err = RegQueryValueEx (hkey, itemptr, NULL, &regtype, val, len);
			
		RegCloseKey (hkey);

		if (err != ERROR_SUCCESS)
			return (false);

		if (regtype != REG_BINARY)
			return (false);

		return (true);
		}

	return (false);
	} /*getRegKeyBinary*/

#ifdef PIKE
#define ProgramKey "\x16" "Software\\UserLand\\Pike"
#else
#define ProgramKey "\x1a" "Software\\UserLand\\Frontier"
#endif

boolean setProfileString (bigstring itemname, bigstring bsval) {
	return (setRegKeyString ((Handle) HKEY_CURRENT_USER, ProgramKey, itemname, bsval)); 
	} /*setProfileString*/

boolean setProfileLong (bigstring itemname, unsigned long val) {
	return (setRegKeyLong ((Handle) HKEY_CURRENT_USER, ProgramKey, itemname, val)); 
	} /*setProfileLong*/

boolean setProfileData (bigstring itemname, unsigned char * val, unsigned long len) {
	return (setRegKeyBinary ((Handle) HKEY_CURRENT_USER, ProgramKey, itemname, val, len));
	} /*setProfileData*/

boolean getProfileString (bigstring itemname, bigstring bsval) {
	return (getRegKeyString ((Handle) HKEY_CURRENT_USER, ProgramKey, itemname, bsval)); 
	} /*getProfileString*/

boolean getProfileLong (bigstring itemname, unsigned long * val) {
	return (getRegKeyLong ((Handle) HKEY_CURRENT_USER, ProgramKey, itemname, val)); 
	} /*getProfileLong*/

boolean getProfileData (bigstring itemname, unsigned char * val, unsigned long * len) {
	return (getRegKeyBinary ((Handle) HKEY_CURRENT_USER, ProgramKey, itemname, val, len));
	} /*getProfileData*/


static boolean setRegKeyValue (bigstring bskey, bigstring bsitem, bigstring bsval) {

    HKEY hkey;
	DWORD what;
	char key[256];
	char item[256];
	char val[256];
	char * itemptr;

	if ((bskey == NULL) || (bsval == NULL))
		return (false);

	if (stringlength (bskey) == 0)
		return (false);

	copyptocstring (bskey, key);
	copyptocstring (bsval, val);

	if (bsitem != NULL) {
		itemptr = item;
		copyptocstring (bsitem, item);
		}
	else
		itemptr = NULL;

	if (RegCreateKeyEx (HKEY_CLASSES_ROOT, key, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &what) == ERROR_SUCCESS) {
		
		RegSetValueEx (hkey, itemptr, 0, REG_SZ, val, strlen (val)+1);
			
		RegCloseKey (hkey);

		return (true);
		}

	return (false);
	}


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

boolean registerFileType (bigstring extension, bigstring filetype, bigstring contenttype, bigstring filedescription, bigstring iconfile, short iconid, bigstring shellopen) {
	bigstring sval, skey;

	if ((extension == NULL) || (filetype == NULL))
		return (false);

	if (stringlength (extension) < 2)
		return (false);

	if (stringlength (filetype) == 0)
		return (false);

	/* Register with the Registry */
	setRegKeyValue (extension, NULL, filetype);

	if (contenttype != NULL) {
		setRegKeyValue (extension, "\x0c" "Content Type", contenttype);
		}

	if (filedescription == NULL)
		return (true);				/* nothing more to do */

	setRegKeyValue (filetype, NULL, filedescription);

	if (iconfile != NULL) {
		copystring (iconfile, sval);
		pushchar (',', sval);
		pushint (iconid, sval);

		copystring (filetype, skey);
		pushstring ("\x0c" "\\DefaultIcon", skey);

		setRegKeyValue (skey, NULL, sval);
		}

	if (shellopen != NULL) {
		copystring (filetype, skey);
		pushstring ("\x13" "\\shell\\open\\command", skey);

		setRegKeyValue (skey, NULL, shellopen);
		}

	return (true);
	}


static boolean winregpullhkeyfromtext (Handle htext, HKEY *hkey) {

	/*
	7.0.2b1 Radio PBS: get the base key from the text string. Remove
	it from the text string.

	If a string is HKCU/foo/bar/, then htext becomes foo\bar\, and *hkey
	becomes HKEY_CLASSES_USER.
	*/

	short ixslash;
	bigstring bsbase;
	boolean fl = false;
	
	ixslash = textpatternmatch (*htext, gethandlesize (htext), "\x01\\", true);

	if (ixslash == -1) /*didn't find?*/
		return (false);

	pullfromhandle (htext, 0, ixslash, stringbaseaddress (bsbase));

	pullfromhandle (htext, 0, 1, nil); /*remove leading /*/

	setstringlength (bsbase, ixslash);

	allupper (bsbase); /*case insensitive*/

	if ((equalstrings (bsbase, STR_P_HKCU_SHORT)) || (equalstrings (bsbase, STR_P_HKCU_LONG))) {
		*hkey = HKEY_CURRENT_USER;
		fl = true;
		}

	if ((equalstrings (bsbase, STR_P_HKLM_SHORT)) || (equalstrings (bsbase, STR_P_HKLM_LONG))) {
		*hkey = HKEY_LOCAL_MACHINE;
		fl = true;
		}

	if ((equalstrings (bsbase, STR_P_HKCR_SHORT)) || (equalstrings (bsbase, STR_P_HKCR_LONG))) {
		*hkey = HKEY_CLASSES_ROOT;
		fl = true;
		}

	if ((equalstrings (bsbase, STR_P_HKU_SHORT)) || (equalstrings (bsbase, STR_P_HKU_LONG))) {
		*hkey = HKEY_USERS;
		fl = true;
		}

	if ((equalstrings (bsbase, STR_P_HKCC_SHORT)) || (equalstrings (bsbase, STR_P_HCC_LONG))) {
		*hkey = HKEY_CURRENT_CONFIG;
		fl = true;
		}
	
	return (fl);
	} /*winregpullhkeyfromtext*/


static boolean winregiskey (Handle h) {

	/*
	7.0.2.b1 Radio PBS: return true if the string specifies a key.
	Otherwise, the string specifies a value.
	*/

	char ch;
	long lentext = gethandlesize (h);

	ch = (*h) [lentext - 1];

	if (ch == '\\')
		return (true);

	return (false);
	} /*winregiskey*/


static boolean winregtypestringtotype (bigstring bstype, DWORD *regtype) {

	/*
	7.0.2b1 Radio PBS: Given a string identifying a type, return the numerical type.
	*/

	bigstring uppertype;
	boolean fl = false;

	copystring (bstype, uppertype);

	allupper (uppertype); /*case insensitive*/

	if (equalstrings (bstype, STR_P_REG_BINARY)) {

		*regtype = REG_BINARY;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_DWORD)) {

		*regtype = REG_DWORD;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_DWORD_BIG_ENDIAN)) {

		*regtype = REG_DWORD_BIG_ENDIAN;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_EXPAND_SZ)) {

		*regtype = REG_EXPAND_SZ;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_LINK)) {

		*regtype = REG_LINK;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_MULTI_SZ)) {

		*regtype = REG_MULTI_SZ;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_NONE)) {

		*regtype = REG_NONE;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_RESOURCE_LIST)) {

		*regtype = REG_RESOURCE_LIST;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_SZ)) {

		*regtype = REG_SZ;

		fl = true;
		} /*if*/

	if (equalstrings (bstype, STR_P_REG_BINARY)) {

		*regtype = REG_BINARY;

		fl = true;
		} /*if*/

	return (fl);
	} /*winregtypestringtotype*/


static void winregtypetostring (DWORD regtype, bigstring bstype) {

	/*
	7.0.2b1 Radio PBS: Given the numerical type of a registry entry, return the string
	representing that type.
	*/

	switch (regtype) {
		
		case REG_BINARY:

			copystring (STR_P_REG_BINARY, bstype);

			break;
			
		case REG_DWORD:

			copystring (STR_P_REG_DWORD, bstype);

			break;
			
		case REG_DWORD_BIG_ENDIAN:

			copystring (STR_P_REG_DWORD_BIG_ENDIAN, bstype);
 
			break;

		case REG_EXPAND_SZ:

			copystring (STR_P_REG_EXPAND_SZ, bstype);

			break;
			
		case REG_LINK:

			copystring (STR_P_REG_LINK, bstype);

			break;
			
		case REG_MULTI_SZ:

			copystring (STR_P_REG_MULTI_SZ, bstype);

			break;
			
		case REG_NONE:

			copystring (STR_P_REG_NONE, bstype);

			break;

		case REG_RESOURCE_LIST:

			copystring (STR_P_REG_RESOURCE_LIST, bstype);

			break;
			
		case REG_SZ:

			copystring (STR_P_REG_SZ, bstype);

			break;

		default:

			copystring (STR_P_UNKNOWNTYPE, bstype);

			break;
		} /*switch*/
	} /*winregtypetostring*/


static boolean winreggetkeytype (HKEY hkey, Handle h, DWORD *regtype) {

	/*
	7.0.2b1 Radio PBS: get the data type of a key.
	*/

	bigstring subkey;
	unsigned long len;

	texthandletostring (h, subkey);

	return (getRegKeyInfo ((Handle) hkey, subkey, emptystring, regtype, &len));
	} /*winreggetkeptype*/


static boolean winreggetvaluetype (HKEY hkey, Handle h, DWORD *regtype) {

	/*
	7.0.2b1 Radio PBS: get the data type of a key.
	*/

	bigstring subkey, itemname;
	unsigned long len;

	texthandletostring (h, subkey);

	pullstringsuffix (subkey, itemname, '\\');

	return (getRegKeyInfo ((Handle) hkey, subkey, itemname, regtype, &len));
	} /*winreggetvaluetype*/


static boolean winreggetvaluelength (HKEY hkey, Handle h, unsigned long *length) {
	
	/*
	7.0.2b1 Radio PBS: get the size of a registry value.
	*/

	bigstring subkey, itemname;
	DWORD regtype;

	texthandletostring (h, subkey);

	pullstringsuffix (subkey, itemname, '\\');

	return (getRegKeyInfo ((Handle) hkey, subkey, itemname, &regtype, length));
	} /*winreggetvaluelength*/


boolean winreggettype (Handle h, bigstring bstype) {

	/*
	7.0.2b1 Radio PBS: given a string of type "HKCU/foo/bar," get the data type.
	*/

	HKEY hbasekey = HKEY_CURRENT_USER;
	DWORD type = 0;
	boolean fl = false;

	if (!winregpullhkeyfromtext (h, &hbasekey))
		return (false);

	if (winregiskey (h))

		fl = winreggetkeytype (hbasekey, h, &type);

	else

		fl = winreggetvaluetype (hbasekey, h, &type);

	if (fl)
		winregtypetostring (type, bstype);

	return (fl);
	} /*winreggettype*/


static boolean winreggethandlevalue (Handle regkey, bigstring subkey, bigstring itemname, Handle hval, unsigned long  *len, DWORD *regtype) {
 
	HKEY hkey;
	char key[256];
	char item[256];
	char * itemptr;
	long err;

	if ((subkey == NULL) || (hval == NULL) || (len == NULL))
		return (false);

	if (stringlength (subkey) == 0)
		return (false);

	copyptocstring (subkey, key);

	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	if (RegOpenKeyEx ((HKEY) regkey, key, 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
		
		lockhandle (hval);

		err = RegQueryValueEx (hkey, itemptr, NULL, regtype, *hval, len);

		unlockhandle (hval);
			
		RegCloseKey (hkey);

		if (err != ERROR_SUCCESS)
			return (false);

		return (true);
		}

	return (false);
	} /*winreggethandlevalue*/


static boolean winreggetvalue (HKEY hkey, Handle h, Handle hvalue, DWORD *regtype, unsigned long * len) {

	/*
	7.0.2b1 Radio PBS: get a value from the registry.
	*/

	bigstring subkey, itemname;
	boolean fl = false;
	unsigned long lenhandle;

	texthandletostring (h, subkey);

	pullstringsuffix (subkey, itemname, '\\');
	
	fl = winreggethandlevalue ((Handle) hkey, subkey, itemname, hvalue, len, regtype);

	lenhandle = gethandlesize (hvalue);

	if ((*hvalue) [lenhandle - 1] == '\0') /*pop trailing null terminator*/
		sethandlesize (hvalue, lenhandle - 1);

	return (fl);
	} /*winreggetvaluetype*/


static void winregcoercetofrontiertype (Handle hvalue, DWORD regtype, tyvaluerecord* v) {

	/*
	7.0.2b1 Radio PBS: convert a registry value to a Frontier value record, coercing the type.
	*/

	initvalue (v, novaluetype);

	switch (regtype) {
		
		case REG_DWORD:
		case REG_DWORD_BIG_ENDIAN:

			setlongvalue ((**hvalue), v);

			disposehandle (hvalue); /*7.0.1 (Frontier) PBS: In this case hvalue should be consumed.*/

			break;
			
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:			
		case REG_SZ:
		
			setheapvalue (hvalue, stringvaluetype, v);

			break;

		case REG_BINARY:
		case REG_LINK:
		case REG_NONE:
		case REG_RESOURCE_LIST:

		default:
			
			setbinaryvalue (hvalue, '\?\?\?\?', v);

			break;
		} /*switch*/
	} /*winregcoercetofrontiertype*/


boolean winregread (Handle h, tyvaluerecord *v) {

	/*
	7.0.2b1 Radio PBS: read a registry value.
	*/

	HKEY hbasekey = HKEY_CURRENT_USER;
	Handle hvalue;
	unsigned long length;
	DWORD regtype;

	if (!winregpullhkeyfromtext (h, &hbasekey))
		return (false);

	if (!winreggetvaluelength (hbasekey, h, &length))
		return (false);

	if (!newhandle (length, &hvalue))
		return (false);
	
	if (!winreggetvalue (hbasekey, h, hvalue, &regtype, &length))
		return (false);

	winregcoercetofrontiertype (hvalue, regtype, v);

	return (true);
	} /*winregread*/


static boolean winregdeletekey (HKEY hkey, Handle h) {

	/*
	7.0.2b1 Radio PBS: delete a key.
	*/

	long lentext;
	boolean fl = true;

	lentext = gethandlesize (h);

	sethandlesize (h, lentext + 1);

	(*h) [lentext] = '\0'; /*null terminate*/

	lockhandle (h);

	if (RegOpenKeyEx (hkey, *h, 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS) {

		if (RegDeleteKey (hkey, *h) == ERROR_SUCCESS) {

			fl = true;

			RegFlushKey (hkey);
			} /*if*/

		RegCloseKey (hkey);
		} /*if*/

	unlockhandle (h);

	return (fl);
	} /*winregdeletekey*/


static boolean winregdeletevalue (HKEY hkey, Handle h, bigstring bsitem) {

	/*
	7.0.2b1 Radio PBS: delete a value.
	*/

	long lentext;
	char item [256];
	char * itemptr;
	boolean fl = true;

	lentext = gethandlesize (h);

	sethandlesize (h, lentext + 1);

	(*h) [lentext] = '\0'; /*null terminate*/

	copyptocstring (bsitem, item);

	itemptr = item;

	lockhandle (h);

	if (RegOpenKeyEx (hkey, *h, 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS) {

		if (RegDeleteValue (hkey, itemptr) == ERROR_SUCCESS)

			fl = true;

		RegCloseKey (hkey);
		} /*if*/

	unlockhandle (h);

	return (fl);
	} /*winregdeletevalue*/


boolean winregdelete (Handle h) {

	/*
	7.0.2b1 Radio PBS: delete from the registry.
	*/

	HKEY hbasekey = HKEY_CURRENT_USER;
	bigstring subkey, bsitem;
	boolean fl = false;

	if (!winregpullhkeyfromtext (h, &hbasekey))
		return (false);

	if (winregiskey (h))
		
		fl = winregdeletekey (hbasekey, h);

	else {

		texthandletostring (h, subkey);

		if (!pullstringsuffix (subkey, bsitem, '\\'))
			return (false);

		fl = winregdeletevalue (hbasekey, h, bsitem);
		} /*else*/

	return (fl);
	} /*winregdelete*/


static void winregfrontiertypetotype (tyvaluetype type, DWORD *regtype) {

	/*
	7.0.2b1 Radio PBS: Given a Frontier type, get the registry type.
	*/

	switch (type) {
		
		case binaryvaluetype: /*binary data*/
		case addressvaluetype:
		case codevaluetype:
		case externalvaluetype:
		case passwordvaluetype:
		case unused2valuetype:
		case rectvaluetype:
		case rgbvaluetype:
		case menuvaluetype:
		case patternvaluetype:
		case pointvaluetype:
		case olddoublevaluetype:
		case wordvaluetype:
		case pictvaluetype:
		case headvaluetype:
		case tablevaluetype:
		case oldstringvaluetype:
		case tokenvaluetype:
		case objspecvaluetype:

			*regtype = REG_BINARY;

			break;

		case outlinevaluetype: /*strings*/
		case stringvaluetype:
		case ostypevaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
		case enumvaluetype:
		case scriptvaluetype:
		case listvaluetype:
		case recordvaluetype:
	
			*regtype = REG_SZ;

			break;

		case uninitializedvaluetype: /*numbers*/
		case novaluetype:
		case charvaluetype:
		case intvaluetype:
		case longvaluetype:
		case booleanvaluetype:
		case datevaluetype:
		case directionvaluetype:
		case fixedvaluetype:
		case singlevaluetype:
		case doublevaluetype:

			*regtype = REG_DWORD;
				
			break;

		default: /*default is binary data*/

			*regtype = REG_BINARY;

			break;
		} /*switch*/
	} /*winregfrontiertypetotype*/


static void winregcoercevalue (tyvaluerecord *v, DWORD regtype) {
	
	/*
	7.0.2b1 Radio PBS: coerce a Frontier value to a binary, string, or long,
	so it can be written to the registry.
	*/

	switch (regtype) {
	
		case REG_DWORD:
		case REG_DWORD_BIG_ENDIAN:

			coercetolong (v);

			break;
			
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:			
		case REG_SZ:
		
			coercetostring (v);

			break;

		case REG_BINARY:
		case REG_LINK:
		case REG_NONE:
		case REG_RESOURCE_LIST:
		default:

			coercetobinary (v);
		} /*switch*/

	} /*winregcoercevalue*/


static boolean winregdowrite (HKEY regkey, Handle h, bigstring itemname, tyvaluerecord *v, DWORD regtype) {

	/*
	7.0.2b1 Radio PBS: perform the write to the registry.
	*/
							  
	HKEY hkey;
	DWORD what;
	char item[256];
	char * itemptr;
	byte *val;
	boolean fl = false, flbinary = false, flstring = false;
	long lendata, lentext;
  
	switch (regtype) {

		case REG_DWORD:
		case REG_DWORD_BIG_ENDIAN:

			(long*) val = &((*v).data.longvalue);

			lendata = sizeof (long);

			break;
			
		case REG_EXPAND_SZ:
		case REG_MULTI_SZ:			
		case REG_SZ: {

			long lentext = gethandlesize ((*v).data.stringvalue);

			sethandlesize ((*v).data.stringvalue, lentext + 1);

			(*((*v).data.stringvalue)) [lentext] = '\0'; /*null terminate*/			

			val = *((*v).data.stringvalue);

			lendata = lentext + 1;

			flstring = true;

			break;
			}

		case REG_BINARY:
		case REG_LINK:
		case REG_NONE:
		case REG_RESOURCE_LIST:
		default: {
						
			/*Pull first four bytes*/

			pullfromhandle ((*v).data.binaryvalue, 0, 4, nil);

			val = *((*v).data.binaryvalue);

			lendata = gethandlesize ((*v).data.binaryvalue);

			if (lendata < 0)
				lendata = 0;

			flbinary = true;

			break;
			}
		} /*switch*/


	if (itemname != NULL) {
		itemptr = item;
		copyptocstring (itemname, item);
		}
	else
		itemptr = NULL;

	lentext = gethandlesize (h);

	sethandlesize (h, lentext + 1);

	(*h) [lentext] = '\0'; /*null terminate*/

	lockhandle (h);

	if (flbinary)
		lockhandle ((*v).data.binaryvalue);

	if (flstring)
		lockhandle ((*v).data.stringvalue);

	if (RegCreateKeyEx (regkey, *h, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &what) == ERROR_SUCCESS) {
		
		RegSetValueEx (hkey, itemptr, 0, regtype, val, lendata);
			
		RegCloseKey (hkey);

		fl = true;
		} /*if*/

	unlockhandle (h);

	if (flbinary)
		unlockhandle ((*v).data.binaryvalue);

	if (flstring)
		unlockhandle ((*v).data.stringvalue);

	return (fl);
	} /*winregdowrite*/
							  

boolean winregwrite (Handle h, tyvaluerecord *val, bigstring bstype) {

	/*
	7.0.2b1 Radio PBS: write a value to the registry.
	*/
	
	DWORD regtype;
	HKEY hbasekey = HKEY_CURRENT_USER;
	bigstring subkey, bsitem;
	tyvaluerecord vcopy;
	boolean fl = false;

	if (equalstrings (bstype, emptystring)) /*no type specified*/
	
		winregfrontiertypetotype ((*val).valuetype, &regtype);

	else
		
		winregtypestringtotype (bstype, &regtype);

	if (!winregpullhkeyfromtext (h, &hbasekey))
		return (false);

	if (!winregiskey (h)) {

		texthandletostring (h, subkey);

		if (!pullstringsuffix (subkey, bsitem, '\\'))
			return (false);

		disposehandle (h);
		
		newtexthandle (subkey, &h); 
		} /*if*/

	initvalue (&vcopy, novaluetype);

	if (!copyvaluerecord (*val, &vcopy))
		return (false);

	winregcoercevalue (&vcopy, regtype);

	fl = winregdowrite (hbasekey, h, bsitem, &vcopy, regtype);

	disposevaluerecord (vcopy, false);

	return (fl);
	} /*winregwrite*/