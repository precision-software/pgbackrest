//
// Created by micro on 7/22/2022.
//

#ifndef PGBACKREST_PWD_H
#define PGBACKREST_PWD_H
#include <port.h>  // pid_t

struct passwd {
    uid_t pw_uid;
    char *pw_name;
};

static struct passwd dummyPassword = {.pw_name="dummyPassword"};

static struct passwd *
getpwnam(const char *name)
{
    dummyPassword.pw_uid = getuid();
    return &dummyPassword;
}

static struct passwd *
getpwuid(uid_t uid)
{
    dummyPassword.pw_uid = uid;
    return &dummyPassword;
}


#endif //PGBACKREST_PWD_H
