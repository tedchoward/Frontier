
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

/* String Definitions */



	#define STR_Cant_delete_XXX_because_it_hasnt_been_defined	BIGSTRING ("\x31" "Can't delete \"^0\" because it hasn't been defined.")
	#define STR_The_name_XXX_hasnt_been_defined					BIGSTRING ("\x22" "The name \"^0\" hasn't been defined.")
	#define STR_Address_value_doesnt_refer_to_a_valid_table		BIGSTRING ("\x2d" "Address value doesn't refer to a valid table.")

	#define STR_Assertion	BIGSTRING ("\x5e" "Assertion failed in file ^1, at line ^2.  Probably no big deal, but please tell Doug.  Thanks!")

	#define STR_unknown		BIGSTRING ("\x07" "unknown")       
	#define STR_char		BIGSTRING ("\x04" "char")
	#define STR_int			BIGSTRING ("\x03" "int")
	#define STR_long		BIGSTRING ("\x04" "long")
	#define STR_unused1		BIGSTRING ("\x07" "unused1")
	#define STR_binary		BIGSTRING ("\x06" "binary")
	#define STR_boolean		BIGSTRING ("\x07" "boolean")
	#define STR_token		BIGSTRING ("\x05" "token")
	#define STR_date		BIGSTRING ("\x04" "date")
	#define STR_address		BIGSTRING ("\x07" "address")
	#define STR_compiled_code	BIGSTRING ("\x0d" "compiled code")
	#define STR_string		BIGSTRING ("\x06" "string")
	#define STR_external	BIGSTRING ("\x08" "external")
	#define STR_direction	BIGSTRING ("\x09" "direction")
	#define STR_unused2		BIGSTRING ("\x07" "unused2")
	#define STR_string4		BIGSTRING ("\x07" "string4")
	#define STR_unused3		BIGSTRING ("\x07" "unused3")	
	#define STR_point		BIGSTRING ("\x05" "point")	
	#define STR_rect		BIGSTRING ("\x04" "rect")	
	#define STR_pattern		BIGSTRING ("\x07" "pattern")	
	#define STR_rgb			BIGSTRING ("\x03" "rgb")	
	#define STR_fixed		BIGSTRING ("\x05" "fixed")	
	#define STR_single		BIGSTRING ("\x06" "single")	
	#define STR_double		BIGSTRING ("\x06" "double")	
	#define STR_objspec		BIGSTRING ("\x07" "objspec")	
	#define STR_filespec	BIGSTRING ("\x08" "filespec")	
	#define STR_alias		BIGSTRING ("\x05" "alias")	
	#define STR_enumerator	BIGSTRING ("\x0a" "enumerator")	
	#define STR_list		BIGSTRING ("\x04" "list")
	#define STR_record		BIGSTRING ("\x06" "record")
	#define STR_outline		BIGSTRING ("\x07" "outline")
	#define STR_wptext		BIGSTRING ("\x06" "wptext")
	#define STR_interface	BIGSTRING ("\x09" "interface")
	#define STR_headline	BIGSTRING ("\x08" "headline")
	#define STR_table		BIGSTRING ("\x05" "table")
	#define STR_script		BIGSTRING ("\x06" "script")
	#define STR_menubar		"\x07" "menubar"
	#define STR_picture		BIGSTRING ("\x07" "picture")

	#define STR_with		BIGSTRING ("\x04" "with")
	#define STR_this		BIGSTRING ("\x04" "this")
	#define STR_temp		"\x04" "temp"
	#define STR_environment	"\x0b" "environment"

	#define STR_Stack_overflow	BIGSTRING ("\x0f" "Stack overflow!")

	#define STR_too_many_pophashtables	BIGSTRING ("\x16" "too many pophashtables")
	#define STR_trying_to_dispose_global_symbol_table	BIGSTRING ("\x26" "trying to dispose global symbol table!")
	#define STR_pushpackstack_no_room_on_stack		BIGSTRING ("\x1f" "pushpackstack: no room on stack")
	#define STR_poppackstack_nothing_on_stack		BIGSTRING ("\x1e" "poppackstack: nothing on stack")
	#define STR_Error_unpacking		BIGSTRING ("\x11" "Error unpacking: ")

	#define STR_The_version_number_of_this_database_file_is_not_recognized_by_this_version_of_Frontier	BIGSTRING ("\x57" "The version number of this database file is not recognized by this version of Frontier.")

	#define STR_move			BIGSTRING ("\x04" "move")
	#define STR_copy			BIGSTRING ("\x04" "copy")
	#define STR_rename			BIGSTRING ("\x06" "rename")
	#define STR_moveandrename	BIGSTRING ("\x0d" "moveandrename")
	#define STR_assign			BIGSTRING ("\x06" "assign")
	#define STR_nil_table_address__Creating_empty_table		BIGSTRING ("\x2b" "nil table address.  (Creating empty table.)")


	#define STR_compiler		"\x08" "compiler"
	#define STR_menubar			"\x07" "menubar"
	#define STR_builtins		"\x08" "builtins"
	#define STR_paths			"\x05" "paths"
	#define STR_verbs			"\x05" "verbs"
	#define STR_apps			"\x04" "apps"
	#define STR_traps			"\x05" "traps"
	#define STR_agents			"\x06" "agents"
	#define STR_misc			"\x04" "misc"
	#define STR_kernel			"\x06" "kernel"
	#define STR_language		"\x08" "language"
	#define STR_stack			"\x05" "stack"
	#define STR_semaphores		"\x0a" "semaphores"
	#define STR_threads			"\x07" "threads"
	#define STR_filewindows		"\x05" "files"
	#define STR_root			"\x04" "root"
	#define STR_startup			"\x07" "startup"
	#define STR_suspend			"\x07" "suspend"
	#define STR_resume			"\x06" "resume"
	#define STR_shutdown		"\x08" "shutdown"
	#define STR_system			"\x06" "system"
	#define STR_menus			BIGSTRING ("\x05" "menus")
	#define STR_menubars		"\x0b" "sharedmenus"
	#define STR_macintosh		"\x09" "macintosh"
	#define STR_objectmodel		"\x0b" "objectmodel"
	#define STR_else			"\x04" "else"


	#define STR_File_was_created_by_an_incompatible_version_of_this_program		BIGSTRING ("\x3c" "File was created by an incompatible version of this program.")
	#define STR_Internal_error_attempted_to_read_a_free_block					BIGSTRING ("\x30" "Internal error:  attempted to read a free block.")
	#define STR_Internal_error_unimplemented_function_call						BIGSTRING ("\x24" "Verb unimplemented on this platform.")
	#define STR_Internal_error_bad_type_for_temp_data							BIGSTRING ("\x28" "Internal error:  bad type for temp data.")
	#define STR_error							BIGSTRING ("\x05" "error")
	#define STR_hash_table						BIGSTRING ("\x0a" "hash table")
	#define STR_Stack_overflow_XXX_stack		BIGSTRING ("\x1a" "Stack overflow:  ^0 stack.")
	#define STR_outline_stack_overflow			BIGSTRING ("\x17" "outline stack overflow!")
	#define	STR_wp_stack_overflow				BIGSTRING ("\x12" "wp stack overflow!")
	#define STR_database_stack_overflow			BIGSTRING ("\x18" "database stack overflow!")
	#define STR_bad_outline_version_number		BIGSTRING ("\x1a" "bad outline version number")
	#define STR_bad_list_version_number			BIGSTRING ("\x17" "bad list version number")
	#define STR_ourline_windowopen_inconsistency	BIGSTRING ("\x20" "outline windowopen inconsistency")
	#define STR_increase_cteditors				BIGSTRING ("\x24" "increase cteditors in shellprivate.h")
	#define STR_failed_to_load_a_button_list	BIGSTRING ("\x1c" "failed to load a button list")
	#define STR_globals_stack_overflow			BIGSTRING ("\x17" "globals stack overflow!")
	#define STR_increase_maxkeyboardhooks		BIGSTRING ("\x19" "increase maxkeyboardhooks")
	#define STR_increase_maxdirtyhooks			BIGSTRING ("\x16" "increase maxdirtyhooks")
	#define STR_increase_maxmenuhooks			BIGSTRING ("\x15" "increase maxmenuhooks")
	#define STR_increase_maxeventhooks			BIGSTRING ("\x16" "increase maxeventhooks")
	#define STR_increase_maxerrorhooks			BIGSTRING ("\x16" "increase maxerrorhooks")
	#define STR_increase_maxscraphooks			BIGSTRING ("\x16" "increase maxscraphooks")
	#define STR_increase_maxmemoryhooks			BIGSTRING ("\x17" "increase maxmemoryhooks")
	#define STR_increase_maxfilehooks			BIGSTRING ("\x15" "increase maxfilehooks")

	#define STR_Unknown_error					BIGSTRING ("\x0e" "Unknown error.")
	#define STR_Mac_OS_Error					BIGSTRING ("\x0e" "Mac OS Error: ")

	#define	STR_menuseparator					BIGSTRING ("\x02" "(-")
	#define	STR_hexprefix						"\x02" "0x"
