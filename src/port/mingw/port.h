//
// Created by micro on 7/22/2022.
//

#ifndef PGBACKREST_PORT_H
#define PGBACKREST_PORT_H


#include <assert.h>
static const int NOT_IMPLEMENTED=0;
#define MISSING assert(NOT_IMPLEMENTED)

// Don't use the basic unix functions. (ywt)
//#define NO_OLDNAMES

// windows defines mkdir with a single parameter.  redefine mkdir to have two paramaters ala posix.
#define mkdir mkdir_with_one_arg
#include <io.h>
#undef mkdir
static int mkdir(const char *path, int mode) {MISSING;}

// COM defines "interface" as a macro (struct) which interferes with pgbackrest "interface" name.
//   Our solution is to include <windows.h> now, and then  to undefine "interface".
#include <windows.h>
#undef interface


typedef long suseconds_t;

#define S_ISLNK(mode) (MISSING, 0)
#define S_ISREG(mode) (MISSING, 0)
#define S_ISDIR(mode) (MISSING, 0)

static int lstat(const char *path, void *buf){MISSING;}
static  long readlink (const char *path, char *buf, long bufsiz) {MISSING;}
static int fsync()      {MISSING;};
static int chown()      {MISSING;};


typedef int gid_t;
typedef int uid_t;

static uid_t getuid() {return 0;}
static gid_t getgid() {return 0;}

// Signals are not in windows, although signal.h exists in mingw
#define SIGPIPE  (-1)
#define SIGHUP  (-1)
#define SIGCHLD (-1)

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

// File locks - not included in mingw sys/file.h
#define LOCK_EX (MISSING, 0)
#define LOCK_NB (MISSING, 0)

// fcntl
static int fcntl(int fd, int cmd, ... /* arg */ ) {MISSING;}
#define F_GETFL (MISSING, -1)
#define F_SETFL (MISSING, -1)
#define O_NONBLOCK (MISSING, -1)

// For debugging macros
#define MESSAGE(x) PRAGMA(message #x)
#define PRAGMA(x) _Pragma(#x)

#endif //PGBACKREST_PORT_H
