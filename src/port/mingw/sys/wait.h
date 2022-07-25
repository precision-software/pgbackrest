//
// Created by micro on 7/25/2022.
//

#ifndef PGBACKREST_WAIT_H
#define PGBACKREST_WAIT_H

#include <sys/types.h>
#include <port.h>  # For MISSING


static pid_t waitpid(pid_t pid, int *status, int options) {MISSING;}
#define WEXITSTATUS(code) (MISSING,0)
#define WIFEXITED(code) (MISSING,0)
#define WNOHANG (MISSING,0)


// flock()
#define LOCK_EX (MISSING, 0)
#define LOCK_NB (MISSING, 0)

#endif //PGBACKREST_WAIT_H
