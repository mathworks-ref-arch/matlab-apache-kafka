/*  Copyright 2019 The MathWorks, Inc. */

#ifndef _SL_KAFKA_FUNS_H_
#define _SL_KAFKA_FUNS_H_

#include "rdkafka.h"

int initKafkaConsumer(rd_kafka_t **prk, rd_kafka_topic_t **prkt, 
    const char *brokers,
    const char *group,
    const char *topic );

int initKafkaProducer(rd_kafka_t **prk, rd_kafka_topic_t **prkt, 
    const char *brokers,
    const char *topic );
    
#endif //  _SL_KAFKA_FUNS_H_
