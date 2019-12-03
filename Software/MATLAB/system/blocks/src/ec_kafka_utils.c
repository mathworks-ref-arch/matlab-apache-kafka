/*  Copyright 2019 The MathWorks, Inc. */

#include "ec_kafka_utils.h"
#include "mw_kafka_utils.h"
#include "stdio.h"
#include "stdlib.h"

char *slKafkaBroker = 0;
char *slKafkaGroup = 0;

#define BILLION_L 1000000000

void ec_kafka_init(void)
{
    static int done = 0;
    if (!done)
    {
        slKafkaBroker = getenv("SL_KAFKA_BROKER");
        slKafkaGroup = getenv("SL_KAFKA_GROUP");
        done = 1;
    }
}

/**
 * Returns 
 *  0 - timespec is zero
 *  1 - TS is postive
 * -1 - TS is negative
 */
static int timespec_sgn(const struct timespec *TS)
{
    if (TS->tv_sec > 0)
        return 1;
    else if (TS->tv_sec < 0)
        return -1;
    else
    {
        if (TS->tv_nsec > 0)
            return 1;
        else if (TS->tv_nsec < 0)
            return -1;
        else
            return 0;
    }
}


long timespec_diff(const struct timespec *t0, const struct timespec *t1, struct timespec *res)
{
    long nanoDiff;
    if ((t1->tv_nsec - t0->tv_nsec) < 0)
    {
        res->tv_sec = t1->tv_sec - t0->tv_sec - 1;
        res->tv_nsec = t1->tv_nsec - t0->tv_nsec + BILLION_L;
    }
    else
    {
        res->tv_sec = t1->tv_sec - t0->tv_sec;
        res->tv_nsec = t1->tv_nsec - t0->tv_nsec;
    }


    nanoDiff = res->tv_sec * 1000000000.0 + res->tv_nsec;

    return nanoDiff;
}

void sec_to_timespec(double tSec, struct timespec *TS)
{
    long sec;
    sec = (long)tSec;
    TS->tv_sec = sec;
    TS->tv_nsec = (long)((tSec - (double)sec) * BILLION_L);
}

void print_timespec(const struct timespec *TS, const char *msg)
{
    static char msgBuf[256];
    sprintf(msgBuf, "%s: %ld.%09ld\n", msg, TS->tv_sec, TS->tv_nsec);
    mwLog(MW_INFO, "TimeSpec", msgBuf);
}