/*  Copyright 2019 The MathWorks, Inc. */

#include "mw_kafka_utils.h"
#include <time.h>
#include <string.h>

static char errstr[512]; /* librdkafka API error reporting buffer */
static int wait_eof = 0; /* number of partitions awaiting EOF */

static void setNowString(char *tStr, int len);

static const char *defaultLogFileName = "mw_application.log";
char logFileName[1024];

static int logCounter = 0;
static FILE *logFH = NULL;

static const char *mwLogLevelStrings[] = {
    "MW_TRACE",
    "MW_DEBUG",
    "MW_INFO",
    "MW_WARN",
    "MW_ERROR",
    "MW_FATAL",
    "MW_OFF"};

void mwLog(int level, const char *producer, const char *msg)
{
    static char timeStr[128];
    setNowString(timeStr, 128);
    FILE *loc = logFH;
    if (loc == NULL)
        loc = stderr;
    fprintf(loc, "[%s](%s){%s} %s\n", timeStr, mwLogLevelStrings[level], producer, msg);
    fflush(loc);
}

void mwLogInit(const char *fileName)
{
    const char *fn;
    if (logFH != NULL)
    {
        // Already intialized
        char msg[512];
        sprintf(msg, "Reusing Logger '%s', %d users", logFileName, ++logCounter);
        mwLog(MW_INFO, "Logger", msg);

        return;
    }
    if (fileName != NULL)
    {
        time_t rawtime;
        struct tm *ptm;
        char tStr[256];
        time(&rawtime);

        ptm = gmtime(&rawtime);

        // strftime(tStr, 256, "%Y%m%d_%H%M%S", ptm);
        strftime(tStr, 256, "%Y%m%d", ptm);

        sprintf(logFileName, "%s_%s.log", fileName, tStr);
    }
    else
        strcpy(logFileName, defaultLogFileName);

    logFH = fopen(logFileName, "a+");
    if (logFH == NULL)
    {
        fprintf(stderr, "Problems starting logging\n");
        return;
    }
    else
    {
        char msg[512];
        sprintf(msg, "Started Logger '%s', %d users", logFileName, ++logCounter);
        mwLog(MW_INFO, "Logger", msg);
    }
}

void mwLogTerminate()
{
    if (logFH != NULL)
    {
        logCounter--;
        if (logCounter == 0)
        {
            fclose(logFH);
            logFH = NULL;
        }
    }
}

static void setNowString(char *tStr, int len)
{
    time_t rawtime;
    struct tm *ptm;

    time(&rawtime);

    ptm = gmtime(&rawtime);

    strftime(tStr, len, "%Y-%m-%d %H:%M:%S", ptm);
}

static void print_partition_list(const rd_kafka_topic_partition_list_t
                                     *partitions)
{
    int i;
    static char partitionInfo[256];
    for (i = 0; i < partitions->cnt; i++)
    {
        sprintf(partitionInfo, "%s %s [%" PRId32 "] offset %" PRId64,
                i > 0 ? "," : "",
                partitions->elems[i].topic,
                partitions->elems[i].partition,
                partitions->elems[i].offset);
        mwLog(MW_INFO, "PartionList", partitionInfo);
    }
}

static void rebalance_cb(rd_kafka_t *rk,
                         rd_kafka_resp_err_t err,
                         rd_kafka_topic_partition_list_t *partitions,
                         void *opaque)
{
    switch (err)
    {
    case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
        mwLog(MW_INFO, "%% Consumer group rebalanced", "assigned:");
        print_partition_list(partitions);
        rd_kafka_assign(rk, partitions);
        wait_eof += partitions->cnt;
        break;

    case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
        mwLog(MW_INFO, "%% Consumer group rebalanced", "revoked:");
        print_partition_list(partitions);
        rd_kafka_assign(rk, NULL);
        wait_eof = 0;
        break;

    default:
        mwLog(MW_INFO, "%% Consumer group rebalanced failed", rd_kafka_err2str(err));
        rd_kafka_assign(rk, NULL);
        break;
    }
}

/**
 * @brief Message delivery report callback.
 *
 * This callback is called exactly once per message, indicating if
 * the message was succesfully delivered
 * (rkmessage->err == RD_KAFKA_RESP_ERR_NO_ERROR) or permanently
 * failed delivery (rkmessage->err != RD_KAFKA_RESP_ERR_NO_ERROR).
 *
 * The callback is triggered from rd_kafka_poll() and executes on
 * the application's thread.
 */
static void dr_msg_cb(rd_kafka_t *rk,
                      const rd_kafka_message_t *rkmessage, void *opaque)
{
    if (rkmessage->err)
        mwLog(MW_WARN, "%% Message delivery failed", rd_kafka_err2str(rkmessage->err));
    else
    {
        static char msg[512];
        sprintf(msg, "%% Message delivered (%zd bytes, "
                     "partition %" PRId32 ")\n",
                rkmessage->len, rkmessage->partition);
        mwLog(MW_TRACE, "DeliveryReport", msg);
    }

    /* The rkmessage is destroyed automatically by librdkafka */
}

/**
 * Kafka logger callback (optional)
 */
static void logger(const rd_kafka_t *rk, int level,
                   const char *fac, const char *buf)
{
    char msg[256];
    sprintf(msg, "RDKAFKA-%i-%s: %s: %s", level, fac, rd_kafka_name(rk), buf);
    mwLog(MW_INFO, "KafkaLogger", msg);
}

int mwInitializeKafkaConsumer(rd_kafka_t **prk,
                              const char *brokers, const char *group, const char *topic,
                              int confCount, int topicConfCount, const char **confArray)
{
    rd_kafka_t *rk = NULL;        /* Consumer instance handle */
    rd_kafka_conf_t *conf = NULL; /* Temporary configuration object */
    rd_kafka_topic_conf_t *topic_conf = NULL;
    rd_kafka_topic_partition_list_t *topics;
    int partition = RD_KAFKA_PARTITION_UA;
    int64_t start_offset = RD_KAFKA_OFFSET_BEGINNING;
    int i;
    int ret = 0;
    conf = rd_kafka_conf_new();
    if (conf == NULL)
    {
        fprintf(stderr, "Couldn't instantiate Kafka config object.\n");
        return 1;
    }
    /* Set logger (this is optional, may want to use setting for this) */
    rd_kafka_conf_set_log_cb(conf, logger);

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();
    if (topic_conf == NULL)
    {
        fprintf(stderr, "Couldn't instantiate topic configuration\n");
        return 2;
    }

    if (rd_kafka_conf_set(conf, "group.id", group,
                          errstr, sizeof(errstr)) !=
        RD_KAFKA_CONF_OK)
    {
        fprintf(stderr, "%s\n", errstr);
        return 3;
    }

    rd_kafka_conf_set(conf, "enable.partition.eof", "true",
                      NULL, 0);

    /* Set additional user defined configuration values */
    for (i = 0; i < confCount; i += 2)
    {
        char *key = (char *)confArray[i];
        char *value = (char *)(confArray[i + 1]);
        if (rd_kafka_conf_set(conf, key, value,
                              errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
        {
            mwLog(MW_ERROR, "mwInitializeKafkaConsumer", errstr);
            return 1;
        }
    }

    mwLogKafkaConfig(conf);

    if (topicConfCount > 0)
    {
        /* Set additional user defined configuration values */
        for (i = 0; i < topicConfCount; i += 2)
        {
            char *key = (char *)confArray[confCount + i];
            char *value = (char *)(confArray[confCount + i + 1]);
            if (rd_kafka_topic_conf_set(topic_conf, key, value,
                                        errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
            {
                mwLog(MW_ERROR, "mwInitializeKafkaConsumer", errstr);
                return 2;
            }
        }
    }
    mwLogKafkaTopicConfig(topic_conf);

    /* Set default topic config for pattern-matched topics. */
    /* This seems to make the app core dump */
    rd_kafka_conf_set_default_topic_conf(conf, topic_conf);

    /* Callback called on partition assignment changes -
    rd_kafka_conf_set_rebalance_cb(conf, rebalance_cb);

    /*
      * Create consumer instance.
      *
      * NOTE: rd_kafka_new() takes ownership of the conf object
      *       and the application must not reference it again after
      *       this call.
      */
    mwLogKafkaConfig(conf);
    mwLogKafkaTopicConfig(topic_conf);
    rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!rk)
    {
        fprintf(stderr, "%s\n", errstr);
        return 4;
    }
    conf = NULL;

    /* Add brokers */
    printf("Adding brokers %s\n", brokers);
    if (rd_kafka_brokers_add(rk, brokers) == 0)
    {
        fprintf(stderr, "%% No valid brokers specified\n");
        return 5;
    }

    rd_kafka_poll_set_consumer(rk);

    topics = rd_kafka_topic_partition_list_new(1);

    rd_kafka_topic_partition_list_add(topics, topic, partition);

    {
        rd_kafka_resp_err_t err;
        if ((err = rd_kafka_subscribe(rk, topics)))
        {
            static char msg[256];

            sprintf(msg,
                    "%% Failed to start consuming topics: %s\n",
                    rd_kafka_err2str(err));
            mwLog(MW_ERROR, "KafkaConsumer", msg);
            return 6;
        }
    }

    *prk = rk;
    return 0;
}

int mwInitializeKafkaProducer(rd_kafka_t **prk, rd_kafka_topic_t **prkt,
                              const char *brokers, const char *topic,
                              int confCount, int topicConfCount, const char **confArray)
{
    rd_kafka_t *rk = NULL;        /* Producer instance handle */
    rd_kafka_topic_t *rkt = NULL; /* Topic object */
    rd_kafka_conf_t *conf = NULL; /* Temporary configuration object */
    rd_kafka_topic_conf_t *topicConf = NULL;

    int i;
    int ret = 0;
    conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        mwLog(MW_ERROR, "KafkaProducer", errstr);
        ret = 1;
        goto init_kafka_producer_exit;
    }

    /* Set additional user defined configuration values */
    for (i = 0; i < confCount; i += 2)
    {
        char *key = (char *)confArray[i];
        char *value = (char *)(confArray[i + 1]);
        if (rd_kafka_conf_set(conf, key, value,
                              errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
        {
            mwLog(MW_ERROR, "mwInitializeKafkaProducer", errstr);
            ret = 1;
            goto init_kafka_producer_exit;
        }
    }

    /* Set the delivery report callback.
      * This callback will be called once per message to inform
      * the application if delivery succeeded or failed.
      * See dr_msg_cb() above. */
    // rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    mwLogKafkaConfig(conf);
    /*
      * Create producer instance.
      *
      * NOTE: rd_kafka_new() takes ownership of the conf object
      *       and the application must not reference it again after
      *       this call.
      */
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!rk)
    {
        mwLog(MW_FATAL, "KafkaProducer", errstr);
        rd_kafka_conf_destroy(conf);

        ret = 2;
    }
    conf = NULL;

    topicConf = rd_kafka_topic_conf_new();
    if (topicConfCount > 0)
    {
        /* Set additional user defined configuration values */
        for (i = 0; i < topicConfCount; i += 2)
        {
            char *key = (char *)confArray[confCount + i];
            char *value = (char *)(confArray[confCount + i + 1]);
            if (rd_kafka_topic_conf_set(topicConf, key, value,
                                        errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
            {
                mwLog(MW_ERROR, "mwInitializeKafkaProducer", errstr);
                ret = 1;
                goto init_kafka_producer_exit;
            }
        }
    }
    mwLogKafkaTopicConfig(topicConf);

    /* Create topic object that will be reused for each message
      * produced.
      *
      * Both the producer instance (rd_kafka_t) and topic objects (topic_t)
      * are long-lived objects that should be reused as much as possible.
      */
    rkt = rd_kafka_topic_new(rk, topic, topicConf);
    if (!rkt)
    {
        mwLog(MW_ERROR, "Failed to create topic object", rd_kafka_err2str(rd_kafka_last_error()));
        ret = 3;
    }

init_kafka_producer_exit:
    *prk = rk;
    *prkt = rkt;

    return ret;
}

int mwConsumeKafkaMessage(rd_kafka_t *rk,
                          int8_T *msg, uint32_T *msgLen, const uint32_T P_MSG_LEN,
                          int8_T *key, uint32_T *keyLen, const uint32_T P_KEY_LEN,
                          int64_T *timestamp)
{
    int ret = 0;
    rd_kafka_message_t *rkmessage;

    rkmessage = rd_kafka_consumer_poll(rk, 0);
    if (rkmessage)
    {
        if (rkmessage->err)
        {
            if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF)
            {
                /*  Not a problem, only end of messages in partition */
                msgLen[0] = (uint32_T)0;
            }
            else
            {
                mwLog(MW_ERROR, "Error consuming message", rd_kafka_err2str(rkmessage->err));
                msgLen[0] = (uint32_T)0;
            }
        }
        else
        {
            uint32_T mLen, kLen;

            ret = 1;
            msgLen[0] = (uint32_T)rkmessage->len;
            mLen = msgLen[0] > P_MSG_LEN ? P_MSG_LEN : msgLen[0];
            memcpy(msg, rkmessage->payload, mLen * sizeof(int8_T));

            keyLen[0] = rkmessage->key_len;
            kLen = keyLen[0] > P_KEY_LEN ? P_KEY_LEN : keyLen[0];
            memcpy(key, rkmessage->key, kLen);

            if (timestamp != NULL)
            {
                *timestamp = rd_kafka_message_timestamp(rkmessage, NULL);
            }
        }
        rd_kafka_message_destroy(rkmessage);
    }
    return ret;
}

int mwProduceKafkaMessage(rd_kafka_t *rk, rd_kafka_topic_t *rkt,
                          const char *key, int keyLen,
                          const char *buf, int bufLen)
{
    int ret = 1;
    int retry = 10;
    while (retry > 0)
    {
        if (rd_kafka_produce(
                /* Topic object */
                rkt,
                /* Use builtin partitioner to select partition*/
                RD_KAFKA_PARTITION_UA,
                /* Make a copy of the payload. */
                RD_KAFKA_MSG_F_COPY,
                /* Message payload (value) and length */
                (void *)buf, bufLen,
                /* Optional key and its length */
                key, keyLen,
                /* Message opaque, provided in
                * delivery report callback as
                * msg_opaque. */
                NULL) == -1)
        {
            /**
            * Failed to *enqueue* message for producing.
            */
            mwLog(MW_WARN, "Failed to produce to topic", rd_kafka_topic_name(rkt));
            mwLog(MW_WARN, "Failed to produce to topic (errmsg)", rd_kafka_err2str(rd_kafka_last_error()));

            /* Poll to handle delivery reports */
            if (rd_kafka_last_error() ==
                RD_KAFKA_RESP_ERR__QUEUE_FULL)
            {
                /* If the internal queue is full, wait for
                    * messages to be delivered and then retry.
                    * The internal queue represents both
                    * messages to be sent and messages that have
                    * been sent or failed, awaiting their
                    * delivery report callback to be called.
                    *
                    * The internal queue is limited by the
                    * configuration property
                    * queue.buffering.max.messages */
                rd_kafka_poll(rk, 1000 /*block for max 1000ms*/);
                retry -= 1;
            }
        }
        else
        {
            // Message successfully produced
            retry = 0;
            ret = 0;
        }
    }
    return ret;
}

void mwTerminateKafkaConsumer(rd_kafka_t *rk)
{
    int verbose = 1;
    if (rk != NULL)
    {
        if (verbose)
        {
            mwLog(MW_TRACE, "KafkaConsumer", "Closing kafka consumer");
        }
        rd_kafka_consumer_close(rk);
        if (verbose)
        {
            mwLog(MW_TRACE, "KafkaConsumer", "Destroying kafka consumer");
        }
        rd_kafka_destroy(rk);
    }
}

void mwTerminateKafkaProducer(rd_kafka_t *rk, rd_kafka_topic_t *rkt)
{
    static int verbose = 1;
    /* 1) Make sure all outstanding requests are transmitted and handled. */
    int N;
    while ((N = rd_kafka_outq_len(rk)) > 0)
        rd_kafka_poll(rk, 50);

    /* 2) Destroy the topic and handle objects */
    if (rkt != NULL)
    {
        if (verbose)
            mwLog(MW_TRACE, "KafkaProducer", "Destroying kafka topic");
        rd_kafka_topic_destroy(rkt);
    }
    if (rk != NULL)
    {
        if (verbose)
            mwLog(MW_TRACE, "KafkaProducer", "Destroying kafka producer");
        rd_kafka_destroy(rk);
    }
}

void mwLogKafkaConfig(rd_kafka_conf_t *conf)
{
    const char **arr;
    size_t i, cnt = 0;
    arr = rd_kafka_conf_dump(conf, &cnt);
    for (i = 0; i < cnt; i += 2)
    {
        char msg[4096];
        sprintf(msg, "%s: %s", arr[i], arr[i + 1]);
        mwLog(MW_INFO, "KafkaConf", msg);
    }
    rd_kafka_conf_dump_free(arr, cnt);
}

void mwLogKafkaTopicConfig(rd_kafka_topic_conf_t *conf)
{
    const char **arr;
    size_t i, cnt = 0;
    arr = rd_kafka_topic_conf_dump(conf, &cnt);
    for (i = 0; i < cnt; i += 2)
    {
        char msg[4096];
        sprintf(msg, "%s: %s", arr[i], arr[i + 1]);
        mwLog(MW_INFO, "KafkaTopicConf", msg);
    }
    rd_kafka_conf_dump_free(arr, cnt);
}
