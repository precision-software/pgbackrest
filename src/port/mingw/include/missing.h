//
// Created by micro on 7/25/2022.
//

#ifndef PGBACKREST_MISSING_H
#define PGBACKREST_MISSING_H

// A reference to MISSING will abort with a function-name not implemented message.
#include <assert.h>
static const int NOT_IMPLEMENTED=0;
#define MISSING (write(2, "   ERROR - ", 12), write(2, __func__, strlen(__func__)), write(2, " ", 1), assert(NOT_IMPLEMENTED), 0)

#endif //PGBACKREST_MISSING_H
