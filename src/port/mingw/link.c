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


extern int _dosmaperr(DWORD err);

int
link(const char *src, const char *dst)
{
	if (CreateHardLinkA(dst, src, NULL) == 0)
	{
		_dosmaperr(GetLastError());
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
symlink(const char *target, const char *linkpath) {

    // Set up the flags, including a quick check if it is a directory or not.
    DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    if (is_directory(linkpath))
        flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;

    // Create the link and check for errors.
    int ret = CreateSymbolicLinkA(linkpath, target, flags);
    if (ret == 0) {
        errno = _dosmaperr(GetLastError());
        return -1;
    }

    // All is fine.
    return 0;
}
