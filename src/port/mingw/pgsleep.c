/*-------------------------------------------------------------------------
 *
 * pgsleep.c
 *	   Portable delay handling.
 *
 *
 * Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
 *
 * src/port/pgsleep.c
 *
 *-------------------------------------------------------------------------
 */
#include "port.h"



/*
 * pg_usleep --- delay the specified number of microseconds.
 *
 * NOTE: although the delay is specified in microseconds, the effective
 * resolution is only 1/HZ, or 10 milliseconds, on most Unixen.  Expect
 * the requested delay to be rounded up to the next resolution boundary.
 *
 * On machines where "long" is 32 bits, the maximum delay is ~2000 seconds.
 *
 * CAUTION: the behavior when a signal arrives during the sleep is platform
 * dependent.  On most Unix-ish platforms, a signal does not terminate the
 * sleep; but on some, it will (the Windows implementation also allows signals
 * to terminate pg_usleep).  And there are platforms where not only does a
 * signal not terminate the sleep, but it actually resets the timeout counter
 * so that the sleep effectively starts over!  It is therefore rather hazardous
 * to use this for long sleeps; a continuing stream of signal events could
 * prevent the sleep from ever terminating.  Better practice for long sleeps
 * is to use WaitLatch() with a timeout.
 */
void
pg_usleep(long microsec) {
    if (microsec > 0) {
        SleepEx((microsec < 500 ? 1 : (microsec + 500) / 1000), FALSE);
    }
}