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

/* Created 15 Aug 2011 Brent Simmons */

#include <pthread.h>
#include "mutex.h"

int mutexCreateRecursive(pthread_mutex_t *lockToCreate) {
	pthread_mutexattr_t lockAttributes;
	int pthreadErrorCode = pthread_mutexattr_init(&lockAttributes);
	if (pthreadErrorCode != 0)
		return pthreadErrorCode;
	pthread_mutexattr_settype(&lockAttributes, PTHREAD_MUTEX_RECURSIVE);
	pthreadErrorCode = pthread_mutex_init(lockToCreate, &lockAttributes);
	pthread_mutexattr_destroy(&lockAttributes);
	return pthreadErrorCode;
}


