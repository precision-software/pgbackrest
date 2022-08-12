/*-------------------------------------------------------------------------
 *
 * dirmod.c
 *	  directory handling functions
 *
 * Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *	This includes replacement versions of functions that work on
 *	Win32 (NT4 and newer).
 *
 * IDENTIFICATION
 *	  src/port/dirmod.c
 *
 *-------------------------------------------------------------------------
 */


#include "port.h"
#include <stdio.h>  // TODO: error logging instead.

/* Don't modify declarations in system headers */
#undef rename
#undef unlink

#include <unistd.h>
#include <sys/stat.h>
#include <winioctl.h>

/*
 *	pgrename
 */
int
pgrename(const char *from, const char *to)
{
	int			loops = 0;

	/*
	 * We need to loop because even though PostgreSQL uses flags that allow
	 * rename while the file is open, other applications might have the file
	 * open without those flags.  However, we won't wait indefinitely for
	 * someone else to close the file, as the caller might be holding locks
	 * and blocking other backends.
	 */
	while (!MoveFileEx(from, to, MOVEFILE_REPLACE_EXISTING))
	{
		DWORD		err = GetLastError();

		_dosmaperr(err);

		/*
		 * Modern NT-based Windows versions return ERROR_SHARING_VIOLATION if
		 * another process has the file open without FILE_SHARE_DELETE.
		 * ERROR_LOCK_VIOLATION has also been seen with some anti-virus
		 * software. This used to check for just ERROR_ACCESS_DENIED, so
		 * presumably you can get that too with some OS versions. We don't
		 * expect real permission errors where we currently use rename().
		 */
		if (err != ERROR_ACCESS_DENIED &&
			err != ERROR_SHARING_VIOLATION &&
			err != ERROR_LOCK_VIOLATION)
			return -1;

		if (++loops > 100)		/* time out after 10 sec */
			return -1;
		pg_usleep(100000);		/* us */
	}
	return 0;
}


/*
 *	pgunlink
 */
int
pgunlink(const char *path)
{
	int			loops = 0;

	/*
	 * We need to loop because even though PostgreSQL uses flags that allow
	 * unlink while the file is open, other applications might have the file
	 * open without those flags.  However, we won't wait indefinitely for
	 * someone else to close the file, as the caller might be holding locks
	 * and blocking other backends.
	 */
	while (unlink(path))
	{
		if (errno != EACCES)
			return -1;
		if (++loops > 100)		/* time out after 10 sec */
			return -1;
		pg_usleep(100000);		/* us */
	}
	return 0;
}

/* We undefined these above; now redefine for possible use below */
#define rename(from, to)		pgrename(from, to)
#define unlink(path)			pgunlink(path)

/*-------------------------------------------------------------------------
 *
 * link.c
 *
 * Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/port/link.c
 *
 *-------------------------------------------------------------------------
 */

#include <winsock2.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#define stat _stat64

int
link(const char *src, const char *dst)
{
    if (CreateHardLinkA(dst, src, NULL) == 0)
    {
        errno = (int)GetLastError();  // TODO: map dos error
        return -1;
    }

    return 0;
}



bool
is_directory(const char *path) {
    struct stat status[1];
    if (stat(path, status) == -1)
        return false;
    return S_ISDIR(status->st_mode);
}

int
pgsymlink(const char *target, const char *linkpath) {

    // Set up the flags, including a quick check if it is a directory or not.
    DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    if (is_directory(linkpath))
        flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;

    // Create the link and check for errors.
    int ret = CreateSymbolicLinkA(linkpath, target, flags);
    if (ret == 0) {
        errno = (int)GetLastError();  // TODO: map error.
        return -1;
    }

    // All is fine.
    return 0;
}


/*
 *	pgreadlink - uses Win32 junction points
 */
int
pgreadlink(const char *path, char *buf, size_t size)
{
    MISSING;
    return -1;
}

/*
 * Assumes the file exists, so will return false if it doesn't
 * (since a nonexistent file is not a junction)
 * This will be used to augment stat and lstat to handle symbolic links.
 */
bool
is_symlink(const char *path)
{
	DWORD		attr = GetFileAttributes(path);
	return  attr != INVALID_FILE_ATTRIBUTES &&
         (attr & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT;
}

// Remember the previous value of umask.
static int pgmask = 0;

/***********************************************************************************************************************************
 * umask sets a new user mask and returns the old.
 *    On Windows, umask is a noop and the mask is effectively always 0.
 *    However, applications expect to set and read umasks, so we
 *    make sure umask() always returns the last set value, even though the value is otherwise ignored.
 *    Later, we can coordinate with the open/creat stubs to use the mask, as much as can be under Windows.
 */
int pgumask(int newMask) {
#undef umask

    // Update the mask
    int oldMask = pgmask;
    if (umask(newMask) == -1)
        return -1;

    // Remember the new mask.
    pgmask = newMask;
    return oldMask;

}