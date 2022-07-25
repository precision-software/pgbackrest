//
// Created by micro on 7/22/2022.
//

#ifndef PGBACKREST_GRP_H
#define PGBACKREST_GRP_H

#include "port.h"


struct group {
    gid_t gr_gid;
    char *gr_name;
};

static struct group dummyGroup = {.gr_name="dummyGroup"};

static struct group
*getgrnam(const char *name)
{
    dummyGroup.gr_gid = getgid();
    return &dummyGroup;
}

static struct group
*getgrgid(int gid)
{
    dummyGroup.gr_gid = gid;
    return &dummyGroup;
}


#endif //PGBACKREST_GRP_H
