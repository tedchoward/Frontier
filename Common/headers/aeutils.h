
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

/*PBS 03/14/02: AE OS X fix.*/

extern boolean datahandletostring (AEDesc* desc, bigstring bs);

extern boolean copydatahandle (AEDesc *desc, Handle *h);

extern OSErr putdescdatapointer (AEDesc *desc, DescType typeCode, ptrvoid pvoid, long len);

extern boolean putdeschandle (AEDesc *desc, DescType typeCode, Handle h);

extern boolean newdescwithhandle (AEDesc *desc, DescType typeCode, Handle h);

extern boolean newdescnull (AEDesc *desc, DescType typeCode); // MJL 08/12/03: Broken boolean obj spec fix

extern boolean nildatahandle (AEDesc *desc);