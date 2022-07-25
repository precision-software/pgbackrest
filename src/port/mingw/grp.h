//
// Created by micro on 7/22/2022.
//

#ifndef PGBACKREST_GRP_H
#define PGBACKREST_GRP_H

#include "port.h"


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
    dummyGroup.gr_gid = getgid();
    return &dummyGroup;
}


#endif //PGBACKREST_GRP_H
