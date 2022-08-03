#ifndef PGBACKREST_PORT_H
#define PGBACKREST_PORT_H
/*-------------------------------------------------------------------------
 *
 * win32_port.h
 *	  Windows-specific compatibility stuff.
 *
 * Note this is read in MinGW as well as native Windows builds,
 * but not in Cygwin builds.
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/port/win32_port.h
 *
 *-------------------------------------------------------------------------
 */

// Temporary porting decisions - to be fixed before production.
#define WINDOWS_HACK 1

#include "missing.h"  // MISSING nacro.

// Features
#define FILE_OFFSET_BITS == 64

/* undefine and redefine after #include */
#undef mkdir
#undef ERROR

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#undef small
#include <process.h>
#include <signal.h>
#include <direct.h>
#undef near
#undef interface

#ifdef NOTYET
/* needed before sys/stat hacking below: */
#define fstat microsoft_native_fstat
#define stat microsoft_native_stat
#include <sys/stat.h>
#undef fstat
#undef stat
#endif

// Fill in some missing types.
#include <stdbool.h>  // C99's "bool"
typedef int gid_t;
typedef int uid_t;
typedef useconds_t suseconds_t;

// User permissions.  These are "no-op" stubs.
static uid_t getuid() {return 0;}
static gid_t getgid() {return 0;}
static int chown(const char *name, uid_t user, gid_t group) {return 0;};

// flock() not implemented on mingw, but gnu portable library may have an implementation.
static int flock(int fd, int operation) {MISSING;}
#define LOCK_EX (MISSING,0)
#define LOCK_SH (MISSING,0)
#define LOCK_NB (MISSING,0)

// fcntl operations
#define F_GETFL (MISSING,0)
#define F_SETFL (MISSING,0)
#define O_NONBLOCK (MISSING,0)
static int fcntl(int fd, int operation, ...){MISSING;}

// Simply missing and need to be implemented (or removed)
static int setsid(void) {MISSING;}
static pid_t fork(void) {MISSING;}
static int pipe(int pipefd[2]) {MISSING;}
static int link(const char *oldpath, const char *newpath) {MISSING;}


/* Windows mkdir has a single parameter. Ignore the second posix parameter until we deal with user permissions. */
#define mkdir(a,b)	mkdir(a)

// Simple wrappers not in winsock2, but could be.
#define ftruncate(a,b)	chsize(a,b)
#define fsync(fd) _commit(fd)

/*
 *	Signal stuff
 *
 *	For WIN32, there is no wait() call so there are no wait() macros
 *	to interpret the return value of system().  Instead, system()
 *	return values < 0x100 are used for exit() termination, and higher
 *	values are used to indicate non-exit() termination, which is
 *	similar to a unix-style signal exit (think SIGSEGV ==
 *	STATUS_ACCESS_VIOLATION).  Return values are broken up into groups:
 *
 *	https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/using-ntstatus-values
 *
 *		NT_SUCCESS			0 - 0x3FFFFFFF
 *		NT_INFORMATION		0x40000000 - 0x7FFFFFFF
 *		NT_WARNING			0x80000000 - 0xBFFFFFFF
 *		NT_ERROR			0xC0000000 - 0xFFFFFFFF
 *
 *	Effectively, we don't care on the severity of the return value from
 *	system(), we just need to know if it was because of exit() or generated
 *	by the system, and it seems values >= 0x100 are system-generated.
 *	See this URL for a list of WIN32 STATUS_* values:
 *
 *		Wine (URL used in our error messages) -
 *			http://source.winehq.org/source/include/ntstatus.h
 *		Descriptions -
 *			https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
 *
 *	The comprehensive exception list is included in ntstatus.h from the
 *	Windows	Driver Kit (WDK).  A subset of the list is also included in
 *	winnt.h from the Windows SDK.  Defining WIN32_NO_STATUS before including
 *	windows.h helps to avoid any conflicts.
 *
 *	Some day we might want to print descriptions for the most common
 *	exceptions, rather than printing an include file name.  We could use
 *	RtlNtStatusToDosError() and pass to FormatMessage(), which can print
 *	the text of error values, but MinGW does not support
 *	RtlNtStatusToDosError().
 */
#define WIFEXITED(w)	(((w) & 0XFFFFFF00) == 0)
#define WIFSIGNALED(w)	(!WIFEXITED(w))
#define WEXITSTATUS(w)	(w)
#define WTERMSIG(w)		(w)


// Signals. Sigaction is not supported by mingw, so these are just stubs to permit compilation.
#define CLD_EXITED (-1)
#define SA_NOCLDSTOP (-1)
#define SA_NOCLDWAIT (-1)
#define SA_SIGINFO (-1)

typedef struct siginfo {
    int si_pid;
    int si_code;
} siginfo_t;

typedef int sigset_t;

struct sigaction {
    void     (*sa_handler)(int);
    void     (*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void     (*sa_restorer)(void);
};

static int sigaction(int signum, const struct sigaction *restrict act, struct sigaction *restrict oldact) {MISSING;}
static int kill(pid_t pid, int signal) {MISSING;}

/* Some extra signals */
#define SIGHUP				1
#define SIGQUIT				3
#define SIGTRAP				5
#define SIGABRT				22	/* Set to match W32 value -- not UNIX value */
#define SIGKILL				9
#define SIGPIPE				13
#define SIGALRM				14
#define SIGSTOP				17
#define SIGTSTP				18
#define SIGCONT				19
#define SIGCHLD				20
#define SIGWINCH			28
#define SIGUSR1				30
#define SIGUSR2				31


/* For now, we don't support reading symbolic links on Windows. */
#define lstat(name, status)  stat(name, status)
#define lchown(name, uid, gid)  chown(name, uid, gid)
#define S_ISLNK(type) 0

#define symlink(oldpath, newpath)	pgsymlink(oldpath, newpath)
#define readlink(path, buf, size)	pgreadlink(path, buf, size)
extern int	pgsymlink(const char *oldpath, const char *newpath);
extern int	pgreadlink(const char *path, char *buf, size_t size);
extern bool pgwin32_is_junction(const char *path);



 /* Supplement to <errno.h>.
 *
 * We redefine network-related Berkeley error symbols as the corresponding WSA
 * constants. This allows strerror.c to recognize them as being in the Winsock
 * error code range and pass them off to win32_socket_strerror(), since
 * Windows' version of plain strerror() won't cope.  Note that this will break
 * if these names are used for anything else besides Windows Sockets errors.
 * See TranslateSocketError() when changing this list.
 */
#include <errno.h>
#undef EAGAIN
#define EAGAIN WSAEWOULDBLOCK
#undef EINTR
#define EINTR WSAEINTR
#undef EMSGSIZE
#define EMSGSIZE WSAEMSGSIZE
#undef EAFNOSUPPORT
#define EAFNOSUPPORT WSAEAFNOSUPPORT
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#undef ECONNABORTED
#define ECONNABORTED WSAECONNABORTED
#undef ECONNRESET
#define ECONNRESET WSAECONNRESET
#undef EINPROGRESS
#define EINPROGRESS WSAEINPROGRESS
#undef EISCONN
#define EISCONN WSAEISCONN
#undef ENOBUFS
#define ENOBUFS WSAENOBUFS
#undef EPROTONOSUPPORT
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#undef ECONNREFUSED
#define ECONNREFUSED WSAECONNREFUSED
#undef ENOTSOCK
#define ENOTSOCK WSAENOTSOCK
#undef EOPNOTSUPP
#define EOPNOTSUPP WSAEOPNOTSUPP
#undef EADDRINUSE
#define EADDRINUSE WSAEADDRINUSE
#undef EADDRNOTAVAIL
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#undef EHOSTDOWN
#define EHOSTDOWN WSAEHOSTDOWN
#undef EHOSTUNREACH
#define EHOSTUNREACH WSAEHOSTUNREACH
#undef ENETDOWN
#define ENETDOWN WSAENETDOWN
#undef ENETRESET
#define ENETRESET WSAENETRESET
#undef ENETUNREACH
#define ENETUNREACH WSAENETUNREACH
#undef ENOTCONN
#define ENOTCONN WSAENOTCONN


extern int	pgwin32_noblock;
extern void _dosmaperr(unsigned long);


// Envioronment variables
#define putenv(x) pgwin32_putenv(x)
#define setenv(x,y,z) pgwin32_setenv(x,y,z)
#define unsetenv(x) pgwin32_unsetenv(x)
extern int	pgwin32_putenv(const char *);
extern int	pgwin32_setenv(const char *name, const char *value, int overwrite);
extern int	pgwin32_unsetenv(const char *name);

/* in port/win32security.c */
extern int	pgwin32_is_service(void);
extern int	pgwin32_is_admin(void);

/* Windows security token manipulation (in src/common/exec.c) */
extern BOOL AddUserToTokenDacl(HANDLE hToken);




#endif //PGBACKREST_PORT_H