/*  Copyright 2019 The MathWorks, Inc. */

#ifndef _MW_KAFKA_UTILS_H_
#define _MW_KAFKA_UTILS_H_

enum
{
    MW_TRACE = 0,
    MW_DEBUG,
    MW_INFO,
    MW_WARN,
    MW_ERROR,
    MW_FATAL,
    MW_OFF,
    MW_NUM_LOG_LEVELS
};

#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#include "mex.h"
#include "tmwtypes.h"
#else
#include "rtwtypes.h"
#endif

#include "librdkafka/rdkafka.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void mwLog(int level, const char *producer, const char *msg);
    void mwLogInit(const char *fileName);
    void mwLogTerminate();

    int mwInitializeKafkaConsumer(rd_kafka_t **prk,
                                  const char *brokers, const char *group, const char *topic,
                                  int confCount, int topicConfCount, const char **confArray);

    int mwInitializeKafkaProducer(rd_kafka_t **prk, rd_kafka_topic_t **prkt,
                                  const char *brokers, const char *topic,
                                  int confCount, int topicConfCount, const char **confArray);

    int mwConsumeKafkaMessage(rd_kafka_t *rk,
                              int8_T *msg, uint32_T *msgLen, const uint32_T P_MSG_LEN,
                              int8_T *key, uint32_T *keyLen, const uint32_T P_KEY_LEN,
                              int64_T *timestamp);

    int mwProduceKafkaMessage(rd_kafka_t *rk, rd_kafka_topic_t *rkt,
                              const char *key, int keyLen,
                              const char *buf, int bufLen);

    void mwTerminateKafkaConsumer(rd_kafka_t *rk);

    void mwTerminateKafkaProducer(rd_kafka_t *rk, rd_kafka_topic_t *rkt);

    void mwLogKafkaConfig(rd_kafka_conf_t *conf);
    void mwLogKafkaTopicConfig(rd_kafka_topic_conf_t *conf);

#ifdef __cplusplus
}
#endif

#endif // _MW_KAFKA_UTILS_H_
