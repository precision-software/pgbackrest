//
// Created by micro on 7/25/2022.
//

#ifndef PGBACKREST_SOCKET_H
#define PGBACKREST_SOCKET_H

// TODO: use winsock

#include "missing.h"

typedef int socklen_t;

struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next;
};

#define AF_UNSPEC (MISSING, -1)
#define AI_PASSIVE (MISSING, -1)
/*
static
int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen) {MISSING;}
            */

#endif //PGBACKREST_SOCKET_H
