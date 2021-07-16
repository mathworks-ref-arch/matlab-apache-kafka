/*  Copyright 2019 The MathWorks, Inc. */


#ifndef EC_KAFKA_UTILS_H
#define EC_KAFKA_UTILS_H

#define MAX_BROKER_STRING_LENGTH 2048

#include <time.h>

#if defined(__cplusplus)
extern "C" {
#endif

extern char *slKafkaBroker;
extern char *slKafkaGroup;

void ec_kafka_init(void);

long timespec_diff(const struct timespec *t0, const struct timespec *t1, struct timespec *res);

void sec_to_timespec(double tSec, struct timespec *TS);

void print_timespec(const struct  timespec *TS, const char *msg);
#if defined(__cplusplus)
}
#endif

#endif // EC_KAFKA_UTILS_H
