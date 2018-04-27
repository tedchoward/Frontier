
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

#include "memory.h"
#include "quickdraw.h"
#include "strings.h"
#include "ops.h"
#include "resources.h"
#include "shell.rsrc.h"
#include "langinternal.h"



#define dberrorlist 256

static byte * dberrorstrings [] = {
		/* [1] */
		STR_File_was_created_by_an_incompatible_version_of_this_program,
		/* [2] */
		STR_Internal_error_attempted_to_read_a_free_block
	};


static byte * langmiscstrings [] = {
		/* [1] */
		STR_unknown,
		/* [2] */
		STR_error
	};


static byte * stacknames [] = {
		/* [1] */
		STR_hash_table
	};


static byte * langerrorstrings [] = {
		/* [1] */
		STR_Cant_delete_XXX_because_it_hasnt_been_defined,
		/* [2] */
		STR_Stack_overflow_XXX_stack,
		/* [3] */
		STR_The_name_XXX_hasnt_been_defined,
		/* [4] */
		STR_Address_value_doesnt_refer_to_a_valid_table
	};










