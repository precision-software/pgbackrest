//
// Created by micro on 7/25/2022.
//

#ifndef PGBACKREST_NETDB_H
#define PGBACKREST_NETDB_H

#include "missing.h"

static int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res) {MISSING;}

static void freeaddrinfo(struct addrinfo *res) {MISSING;}
static const char *gai_strerror(int errcode){MISSING;}

#endif //PGBACKREST_NETDB_H
