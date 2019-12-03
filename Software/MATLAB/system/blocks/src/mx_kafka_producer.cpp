/*  Copyright 2019 The MathWorks, Inc. */

#include "mex.h"
#include "string.h"

#include "librdkafka/rdkafka.h"
#include "mw_kafka_utils.h"
#include "mx_kafka_utils.h"

static char errstr[512]; /* librdkafka API error reporting buffer */
static int verbose = 1;

static void help(void);
static void usageError(const char *msg);
static void init(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs);
static void term(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs);
static void publish(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs);

static mxArray *setNewMXU64(void *ptr);
static void *getMXU64Value(const mxArray *P);
static char *getStringFromParam(const mxArray *P, const char *msg);

void mexFunction(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs)
{
    if (nrhs < 1)
    {
        usageError("You have to provide at least one argument!");
    }

    if (mxGetClassID(prhs[0]) != mxCHAR_CLASS)
    {
        usageError("The first argument must be a character array\n");
    }

    char cmd[20];
    if (mxGetString(prhs[0], cmd, 19))
    {
        usageError("Wrong command\n");
    }

    if (strcmp(cmd, "init") == 0)
    {
        if (nrhs < 3)
        {
            usageError("You must provide at least the strings\n\tinit <brokers> <topic>");
        }
        init(nlhs, plhs, nrhs, prhs);
        return;
    }

    if (strcmp(cmd, "term") == 0)
    {
        if (nrhs < 3)
        {
            usageError("You must provide at least the arguments\n\tterm <rk> <rk_topic>");
        }
        term(nlhs, plhs, nrhs, prhs);
        return;
    }

    if (strcmp(cmd, "publish") == 0)
    {
        if (nrhs < 4)
        {
            usageError("You must provide at least the arguments\n\tpublish <rkt> <key> <value>");
        }
        publish(nlhs, plhs, nrhs, prhs);
        return;
    }
}

static void init(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs)
{
    if (mxGetClassID(prhs[1]) != mxCHAR_CLASS)
    {
        usageError("The second argument (broker) must be a character array\n");
    }
    if (mxGetClassID(prhs[2]) != mxCHAR_CLASS)
    {
        usageError("The third argument (topic) must be a character array\n");
    }
    if (mxGetClassID(prhs[3]) != mxCELL_CLASS)
    {
        usageError("The fourth argument (conf) must be a cell array of strings\n");
    }
    if (mxGetClassID(prhs[4]) != mxCELL_CLASS)
    {
        usageError("The fifth argument (topicConf) must be a cell array of strings\n");
    }
    char *brokers = getStringFromParam(prhs[1], "second argument <brokers>");
    char *topic = getStringFromParam(prhs[2], "third argument <topic>");

    int confCount = mxGetNumberOfElements(prhs[3]);
    int topicConfCount = mxGetNumberOfElements(prhs[4]);

    if (confCount % 2 != 0)
    {
        usageError("The elements in the conf argument must be key/value pairs");
    }

    if (topicConfCount % 2 != 0)
    {
        usageError("The elements in the topicConf argument must be key/value pairs");
    }

    mwLogInit("mw_kafka_producer");

    char **confArray = getConfArrayFromMX(confCount, prhs[3], topicConfCount, prhs[4]);
    if (confArray == NULL)
        usageError("Problem creating strings for conf array");


    rd_kafka_t *rk = NULL;
    rd_kafka_topic_t *rkt = NULL;

    int ret = mwInitializeKafkaProducer(&rk, &rkt, brokers, topic,
                                        confCount, topicConfCount, (const char **)confArray);
    freeConfArray(confArray, confCount + topicConfCount);

    if (ret)
    {
        mwLog(MW_ERROR, "KafkaProducer", "Problems initializing Kafka Producer");
    }
    else
    {
        mwLog(MW_INFO, "KafkaProducer", "Just started Kafka Producer");
    }

    delete[] brokers;
    delete[] topic;

    plhs[0] = setNewMXU64((void *)rk);
    plhs[1] = setNewMXU64((void *)rkt);
}

static void term(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs)
{
    if (mxGetClassID(prhs[1]) != mxUINT64_CLASS)
    {
        usageError("Second argument must be of type uint64_T\n");
    }
    if (mxGetClassID(prhs[2]) != mxUINT64_CLASS)
    {
        usageError("Third argument must be of type uint64_T\n");
    }

    rd_kafka_t *rk = (rd_kafka_t *)getMXU64Value(prhs[1]);
    rd_kafka_topic_t *rkt = (rd_kafka_topic_t *)getMXU64Value(prhs[2]);
    mwTerminateKafkaProducer(rk, rkt);
    mwLogTerminate();
}

static void publish(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs)
{
    if (mxGetClassID(prhs[1]) != mxUINT64_CLASS)
    {
        usageError("Second argument (rk*) must be of type uint64_T\n");
    }
    if (mxGetClassID(prhs[2]) != mxUINT64_CLASS)
    {
        usageError("Third argument (rk_topic*) must be of type uint64_T\n");
    }
    if (mxGetClassID(prhs[3]) != mxINT8_CLASS)
    {
        usageError("Fourth argument (key) must be of type int8_T\n");
    }
    if (mxGetClassID(prhs[4]) != mxINT8_CLASS)
    {
        usageError("Fifth argument (value) must be of type int8_T\n");
    }
    rd_kafka_t *rk = (rd_kafka_t *)getMXU64Value(prhs[1]);
    rd_kafka_topic_t *rkt = (rd_kafka_topic_t *)getMXU64Value(prhs[2]);

    int keylen = mxGetNumberOfElements(prhs[3]);
    mxInt8 *key = mxGetInt8s(prhs[3]);

    int vallen = mxGetNumberOfElements(prhs[4]);
    mxInt8 *val = mxGetInt8s(prhs[4]);

    int ret = mwProduceKafkaMessage(rk, rkt,
                                    (const char *)key, keylen,
                                    (const char *)val, vallen);
}

static char *getStringFromParam(const mxArray *P, const char *msg)
{
    static char gsfpErr[1024];

    if (mxGetClassID(P) != mxCHAR_CLASS)
    {
        sprintf(gsfpErr, "The %s must be a character array\n", msg);
        usageError(gsfpErr);
    }

    mwSize N = 1 + mxGetNumberOfElements(P);
    char *newStr = new char[N];
    if (newStr == NULL)
    {
        sprintf(gsfpErr, "Couldn't allocate memory for string (%s)\n", msg);
        usageError(gsfpErr);
    }
    if (mxGetString(P, newStr, N))
    {
        delete[] newStr;
        sprintf(gsfpErr, "Couldn't read string (%s)\n", msg);
        usageError(gsfpErr);
    }
    return newStr;
}

static void *getMXU64Value(const mxArray *P)
{
    mxUint64 *pr = mxGetUint64s(P);
    return ((void *)pr[0]);
}

static mxArray *setNewMXU64(void *ptr)
{
    mwSize nDims = 1;
    mwSize dims[] = {1};
    mxArray *R = mxCreateNumericArray(nDims, dims, mxUINT64_CLASS, mxREAL);
    if (R == NULL)
    {
        usageError("Couldn't allocate numeric array\n");
    }
    mxUint64 *pr = mxGetUint64s(R);
    pr[0] = (mxUint64)ptr;
    return R;
}

static void usageError(const char *msg)
{
    help();
    mexErrMsgTxt(msg);
}

static void help(void)
{
    mexPrintf("mx_kafka_producer: Usage\n");
}