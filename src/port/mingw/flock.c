//
// Created by micro on 8/11/2022.
//
#include "port/mingw/include/port.h"
#include <windows.h>
#include <fileapi.h>
#include <io.h>
#include <sys/file.h>
#include <errno.h>


#undef flock
/**********************************************************************************************************************************
Implement the unix flock() call using Windows primitives.
    Unlike POSIX flock(), which locks the entire file, windows locks any range of bytes.
    We simply lock the first byte (which doesn't have to exist), although it might be better to lock the entire 64 bit range.
*/
int
pgflock(int fd, int operation) {

    // Get a Windows handle to the file.
    HANDLE handle = (HANDLE)_get_osfhandle(fd);

    // Map POSIX flock operations onto Windows flags.
    int op = operation & LOCK_OP;
    DWORD flags = (operation&LOCK_NB)? LOCKFILE_FAIL_IMMEDIATELY: 0;
    OVERLAPPED overlapped = {0};

    // Process according to the lock request.
    int success;
    if (op == LOCK_EX)
        success = LockFileEx(handle, LOCK_EXCLUSIVE|flags, 0, 1, 0, &overlapped);
    else if (op == LOCK_NB)
        success = LockFileEx(handle, flags, 0, 1, 0, &overlapped);
    else if (op == LOCK_UN)
        success = UnlockFileEx(handle, 0, 1, 0, &overlapped);
    else {
        errno = EINVAL;
        return -1;
    }

    // If we failed to get the lock, map ERROR_IO_PENDING to EWOULDBLK.
    if (!success) {
        errno = _dosmaperr(GetLastError());
        return -1;
    }

    // Successful.
    return 0;
}