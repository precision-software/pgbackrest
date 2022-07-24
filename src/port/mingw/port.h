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
//   Our soluction is to include <windows.h> now, and then  to undefine "interface".
#include <windows.h>
#undef interface


//typedef unsigned long off_t;

typedef long suseconds_t;

//typedef int mode_t;
//#define S_IRWXG 0
//#define S_IRWXU 0
//#define S_IRWXO 0

#define S_ISLNK(mode) 0
#define S_ISREG(mode) 0
#define S_ISDIR(mode) 0

//#define O_CREAT 0
//#define O_APPEND 0
//#define O_WRONLY 0
//#define O_RDWR 0
//#define O_TRUNC 0
//#define O_RDONLY 0

static int lstat(const char *path, void *buf){return -1;}
static  long readlink (const char *path, char *buf, long bufsiz) {return -1;}
static int fsync()      {MISSING;};
static int chown()      {MISSING;};


typedef long long pid_t; // Matches mingw, but we need it earlier.
typedef int gid_t;
typedef int uid_t;


/******
 * From grp.h.
 */
struct group {
    gid_t gr_gid;
    char *gr_name;
};

struct passwd {
    uid_t pw_uid;
    char *pw_name;
};

static const uid_t dummyUid = 0;
static const gid_t dummyGid = 0;
static const struct passwd dummyPassword = {.pw_uid=dummyUid, .pw_name="dummyPassword"};
static const struct group dummyGroup = {.gr_gid = dummyGid, .gr_name="dummyGroup"};

//static pid_t getpid() {MISSING;}
static uid_t getuid() {return dummyUid;}
static gid_t getgid() {return dummyGid;}
static struct group *getgrnam(const char *name) {return &dummyGroup;}
static struct group *getgrgid(int gid) {return &dummyGroup;}
static struct passwd *getpwnam(const char *name) {return &dummyPassword;}
static struct passwd *getpwuid(uid_t uid) {return &dummyPassword;}


#endif //PGBACKREST_PORT_H
