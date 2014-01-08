
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

#include "filealias.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "mac.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/

#import <Foundation/Foundation.h>

boolean equalfilespecs(ptrfilespec fs1, ptrfilespec fs2) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSString *name1 = [(NSURL *)fs1->url absoluteString];
	NSString *name2 = [(NSURL *)fs2->url absoluteString];
	
	boolean equals = [name1 compare:name2] == NSOrderedSame;
	
	[pool release];
	
	return equals;
}

boolean filesetposition(hdlfilenum fnum, long position) {
	if (lseek(fnum, position, SEEK_SET) == -1) {
		switch (errno) {
			case EBADF:
				shellerrormessage(BIGSTRING("\x21" "fd is not an open file descriptor"));
				break;
			case EINVAL:
				shellerrormessage(BIGSTRING("\x52" "the resulting file offset would be negative or beyond the end of a seekable device"));
				break;
			case EOVERFLOW:
				shellerrormessage(BIGSTRING("\x26" "the resulting file offset is too large"));
				break;
			case ESPIPE:
				shellerrormessage(BIGSTRING("\x2d" "fd is associated with a pipe, socket, or fifo"));
				break;
		}
		
		return false;
	}
	return true;
}

boolean filegetposition(hdlfilenum fnum, long *position) {
	int pos = lseek(fnum, 0, SEEK_CUR);
	
	if (pos == -1) {
		switch (errno) {
			case EBADF:
				shellerrormessage(BIGSTRING("\x21" "fd is not an open file descriptor"));
				break;
		}
		
		return false;
	}
	
	*position = pos;
	return true;
}

boolean filegeteof(hdlfilenum fnum, long *position) {
	int pos = lseek(fnum, 0, SEEK_END);
	
	if (pos == -1) {
		switch (errno) {
			case EBADF:
				shellerrormessage(BIGSTRING("\x21" "fd is not an open file descriptor"));
				break;
		}
		
		return false;
	}
	
	*position = pos;
	
	return true;
}

boolean fileseteof(hdlfilenum fnum, long position) {
	if (ftruncate(fnum, position) == -1) {
		switch (errno) {
			case EBADF:
				shellerrormessage(BIGSTRING("\x1c" "fd is not a valid descriptor"));
				break;
			case EINVAL:
				shellerrormessage(BIGSTRING("\x1a" "fd is not open for writing"));
				break;
		}
		
		return false;
	}
	
	return true;
}

long filegetsize(hdlfilenum fnum) {
	long fileSize;
	
	filegeteof(fnum, &fileSize);
	
	return fileSize;
}

boolean filetruncate(hdlfilenum fnum) {
	return fileseteof(fnum, 0L);
}

boolean filewrite(hdlfilenum fnum, long ctwrite, void *buffer) {
	if (write(fnum, buffer, ctwrite) == -1) {
		// TODO error reporting
		return false;
	}
	
	return true;
}

boolean filereaddata(hdlfilenum fnum, long ctread, long *ctactual, void *buffer) {
	
	ssize_t actualBytes = read(fnum, buffer, ctread);
	
	if (actualBytes == -1) {
		//TODO: error reporting
		return false;
	}
	
	*ctactual = actualBytes;
	
	return true;
}

boolean fileread(hdlfilenum fnum, long ctread, void *buffer) {
	long ctactual;
	
	if (!filereaddata(fnum, ctread, ctactual, buffer)) {
		return false;
	}
	
	if (ctactual < ctread) {
		oserror(eofErr);
		return false;
	}
	
	return true;
}

boolean filegetchar(hdlfilenum fnum, char *ch) {
	return fileread(fnum, 1l, ch);
}

boolean fileputchar(hdlfilenum fnum, char ch) {
	return filewrite(fnum, 1l, &ch);
}

boolean filewritehandle(hdlfilenum fnum, Handle h) {
	return filewrite(fnum, gethandlesize(h), *h);
}

boolean filereadhandle(hdlfilenum fnum, Handle *hreturned) {
	long fileSize;
	Handle h;
	
	fileSize = filegetsize(fnum);
	
	if (!newclearhandle(fileSize, hreturned)) {
		return false;
	}
	
	h = *hreturned;
	
	if (!fileread(fnum, fileSize, *h)) {
		disposehandle(h);
		return false;
	}
	
	return true;
}

boolean flushvolumechanges(const ptrfilespec fs, hdlfilenum fnum) {
	if (fnum != NULL) {
		fsync(fnum);
	}
	
	return true;
}

void fileinit() {
}

void fileshutdown() {
}

static boolean fileCreateAndOpen(ptrfilespec fs, OSType creator, OSType fileType, hdlfilenum *fnum) {
	
	UInt8 buffer[1024];
	CFURLGetFileSystemRepresentation(fs->url, true, buffer, 1024);
	
	int fd = open(buffer, O_RDWR | O_CREAT | O_EXCL);
	
	if (fd == -1) {
		//TODO proper error reporting
		return false
	}
	
	*fnum = fd;
	
	return true;
}

boolean opennewfile(ptrfilespec fs, OSType creator, OSType fileType, hdlfilenum *fnum) {
	boolean flfolder;
	
	if (fileexists(fs, &flfolder)) {
		if (!deletefile(fs)) {
			return false;
		}
	}
	
	return fileCreateAndOpen(fs, creator, fileType, fnum);
}

boolean openfile(const ptrfilespec fs, hdlfilenum *fnum, boolean flreadonly) {
	int fileDescriptor = 0;
	int flags = flreadonly ? O_RDONLY : O_RDWR;
	UInt8 buffer[1024];
	CFURLGetFileSystemRepresentation(fs->url, true, buffer, 1024);

	
	fileDescriptor = open(buffer, flags);
	
	if (fileDescriptor == -1) {
		//TODO: error reporting
		return false;
	}
	
	*fnum = fileDescriptor;
	
	return true;
}

boolean closefile(hdlfilenum fnum) {
	if (fnum != 0) {
		if (close(fnum) == -1) {
			// TODO: error reporting
			return false;
		}
	}
	
	return true;
}

boolean deletefile(const ptrfilespec fs) {
	boolean success = false;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSURL *fileURL = (NSURL *)fs->url;
	NSString *path = [fileURL path];
	NSError *err;
	
	success = [[NSFileManager defaultManager] removeItemAtPath:path error:err];
	
	if (!success) {
		//TODO: error reporting
		NSLog(@"Error deleting file: %@\n%@", [err localizedDescription], [err userInfo]);
	}
	
	[pool release];
	
	return success;
}

boolean fileexists(const ptrfilespec fs, boolean *flfolder) {
	boolean exists = false;
	
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSURL *fileURL = (NSURL *)fs->url;
	NSString *path = [fileURL path];
	
	exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:flfolder];
	
	[pool release];
	
	return exists;
}