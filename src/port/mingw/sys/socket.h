//
// Created by micro on 7/25/2022.
//

#ifndef PGBACKREST_SOCKET_H
#define PGBACKREST_SOCKET_H

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

#endif //PGBACKREST_SOCKET_H
