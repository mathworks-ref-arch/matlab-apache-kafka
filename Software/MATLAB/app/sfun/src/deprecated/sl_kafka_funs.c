/*
 * sl_kafka_consumer
 *
 * Several examples taken from librdkafka, please see librdkafka disclaimer below.
 * 
 * Copyright 2019 The MathWorks, Inc.
 */


/*
 * Need to include simstruc.h for the definition of the SimStruct and
 * its associated macro definitions.
 */

#include <stdlib.h>
#include <time.h>

#include "sl_kafka_funs.h"

static char errstr[512];       /* librdkafka API error reporting buffer */


static void setNowString(char *tStr, int len) {
  time_t rawtime;
  struct tm * ptm;

  time ( &rawtime );

  ptm = gmtime ( &rawtime );

  strftime(tStr, len, "%Y-%m-%d %H:%M:%S", ptm);
}


static int wait_eof = 0; /* number of partitions awaiting EOF */

static char errstr[512]; /* librdkafka API error reporting buffer */

static void print_partition_list(const rd_kafka_topic_partition_list_t
                                     *partitions)
{
    int i;
    for (i = 0; i < partitions->cnt; i++)
    {
        fprintf(stdout, "%s %s [%" PRId32 "] offset %" PRId64,
                  i > 0 ? "," : "",
                  partitions->elems[i].topic,
                  partitions->elems[i].partition,
                  partitions->elems[i].offset);
    }
    fprintf(stdout, "\n");
}

static void rebalance_cb(rd_kafka_t *rk,
                         rd_kafka_resp_err_t err,
                         rd_kafka_topic_partition_list_t *partitions,
                         void *opaque)
{

    fprintf(stdout, "%% Consumer group rebalanced: ");

    switch (err)
    {
    case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
        fprintf(stdout, "assigned:\n");
        print_partition_list(partitions);
        rd_kafka_assign(rk, partitions);
        wait_eof += partitions->cnt;
        break;

    case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
        fprintf(stdout, "revoked:\n");
        print_partition_list(partitions);
        rd_kafka_assign(rk, NULL);
        wait_eof = 0;
        break;

    default:
        fprintf(stdout, "failed: %s\n",
                  rd_kafka_err2str(err));
        rd_kafka_assign(rk, NULL);
        break;
    }
}

/**
 * Kafka logger callback (optional)
 */
static void logger(const rd_kafka_t *rk, int level,
                   const char *fac, const char *buf)
{
    char tmbuf[64];
    setNowString(tmbuf, 64);
    fprintf(stdout, "%s RDKAFKA-%i-%s: %s: %s\n",
              tmbuf, level, fac, rd_kafka_name(rk), buf);
}

int initKafkaConsumer(rd_kafka_t **prk, rd_kafka_topic_t **prkt, 
    const char *brokers,
    const char *group,
    const char *topic )
{
    rd_kafka_t *rk = NULL;        /* Consumer instance handle */
    rd_kafka_topic_t *rkt = NULL; /* Topic object */
    rd_kafka_conf_t *conf = NULL; /* Temporary configuration object */
    rd_kafka_topic_conf_t *topic_conf = NULL;
    rd_kafka_topic_partition_list_t *topics;
    int partition = RD_KAFKA_PARTITION_UA;
    int64_t start_offset = RD_KAFKA_OFFSET_BEGINNING;

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
 

    /* Set default topic config for pattern-matched topics. */
    /* This seems to make the app core dump */
    rd_kafka_conf_set_default_topic_conf(conf, topic_conf);

    /* Callback called on partition assignment changes -
      * TODO: Not used in rdkafka_example.c */
    rd_kafka_conf_set_rebalance_cb(conf, rebalance_cb);


    /*
      * Create consumer instance.
      *
      * NOTE: rd_kafka_new() takes ownership of the conf object
      *       and the application must not reference it again after
      *       this call.
      */
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
    
    // doConfDump(S, conf, topic_conf);
    // LOG(S, "Conf_dump executed");
    /* Redirect rd_kafka_poll() to consumer_poll() */
    /* TODO: Necessary??? rd_kafka_poll_set_consumer(rk); */
    rd_kafka_poll_set_consumer(rk);

    // TODO: Make this accept lists of topics to listen to.
    topics = rd_kafka_topic_partition_list_new(1);

    rd_kafka_topic_partition_list_add(topics, topic, partition);


    fprintf(stdout, "%% Subscribing to %d topics\n", topics->cnt);
    {
        rd_kafka_resp_err_t err; 
        if ((err = rd_kafka_subscribe(rk, topics))) {
            fprintf(stderr, 
                    "%% Failed to start consuming topics: %s\n",
                    rd_kafka_err2str(err));
            return 6;
        }
    }

    *prk = rk;
    *prkt = rkt;

    return 0;
}

int initKafkaProducer(rd_kafka_t **prk, rd_kafka_topic_t **prkt, 
    const char *brokers,
    const char *topic ) {
    rd_kafka_t *rk = NULL;         /* Producer instance handle */
    rd_kafka_topic_t *rkt = NULL;  /* Topic object */
    rd_kafka_conf_t *conf = NULL;  /* Temporary configuration object */


    conf = rd_kafka_conf_new();
   
    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            fprintf(stderr, "%s\n", errstr);
            return 1;
    }

    /* Set the delivery report callback.
      * This callback will be called once per message to inform
      * the application if delivery succeeded or failed.
      * See dr_msg_cb() above. */
     // TODO
    // rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    /*
      * Create producer instance.
      *
      * NOTE: rd_kafka_new() takes ownership of the conf object
      *       and the application must not reference it again after
      *       this call.
      */
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!rk) {
            fprintf(stderr, "%s\n", errstr);
            return 2;
    }

    /* Create topic object that will be reused for each message
      * produced.
      *
      * Both the producer instance (rd_kafka_t) and topic objects (topic_t)
      * are long-lived objects that should be reused as much as possible.
      */
    rkt = rd_kafka_topic_new(rk, topic, NULL);
    if (!rkt) {
            fprintf(stderr, "%% Failed to create topic object: %s\n",
                    rd_kafka_err2str(rd_kafka_last_error()));
            return 3; 
    }

    // When we're here, all went well
    *prk = rk;
    *prkt = rkt;


    return 0;

}

