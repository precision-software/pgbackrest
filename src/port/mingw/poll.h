//
// Created by micro on 7/25/2022.
//

#ifndef PGBACKREST_POLL_H
#define PGBACKREST_POLL_H
#include "missing.h"


typedef int nfds_t;

struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};
static int poll(struct pollfd *fds, nfds_t nfds, int timeout) {MISSING;}

#define POLLIN (MISSING, -1)
#define POLLOUT (MISSING, -1)
#endif //PGBACKREST_POLL_H
