/*  Copyright 2019 The MathWorks, Inc. */

#include "mex.h"
#include "string.h"

#include "rdkafka.h"
#include "mw_kafka_utils.h"
#include "mx_kafka_utils.h"

static char errstr[512]; /* librdkafka API error reporting buffer */
static int verbose = 1;
static int wait_eof = 0; /* number of partitions awaiting EOF */

static void help(void);
static void usageError(const char *msg);
static void init(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs);
static void term(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs);
static void consume(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs);

static mxArray *setNewInt8Arr(uint32_T len, int8_T *ptr);
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
        if (nrhs < 4)
        {
            usageError("You must provide at least the strings\n\tinit <brokers> <topic> <group>");
        }
        init(nlhs, plhs, nrhs, prhs);
        return;
    }

    if (strcmp(cmd, "term") == 0)
    {
        if (nrhs < 2)
        {
            usageError("You must provide at least the arguments\n\tterm <rk> <rk_topic>");
        }
        term(nlhs, plhs, nrhs, prhs);
        return;
    }

    if (strcmp(cmd, "consume") == 0)
    {
        if (nrhs < 2)
        {
            usageError("You must provide at least the arguments\n\tconsume <rkt>");
        }
        consume(nlhs, plhs, nrhs, prhs);
        return;
    }

    sprintf(errstr, "Unknown command: '%s'\n", cmd);
    usageError(errstr);
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
    if (mxGetClassID(prhs[3]) != mxCHAR_CLASS)
    {
        usageError("The third argument (group) must be a character array\n");
    }
   if (mxGetClassID(prhs[4]) != mxCELL_CLASS)
    {
        usageError("The fifth argument (conf) must be a cell array of strings\n");
    }
    if (mxGetClassID(prhs[5]) != mxCELL_CLASS)
    {
        usageError("The sixth argument (topicConf) must be a cell array of strings\n");
    }

    char *brokers = getStringFromParam(prhs[1], "second argument <brokers>");
    char *topic = getStringFromParam(prhs[2], "third argument <topic>");
    char *group = getStringFromParam(prhs[3], "fourth argument <group>");

    rd_kafka_t *rk = NULL;

    int confCount = mxGetNumberOfElements(prhs[4]);
    int topicConfCount = mxGetNumberOfElements(prhs[5]);

    if (confCount % 2 != 0)
    {
        usageError("The elements in the conf argument must be key/value pairs");
    }

    if (topicConfCount % 2 != 0)
    {
        usageError("The elements in the topicConf argument must be key/value pairs");
    }


    mwLogInit("mw_kafka_consumer");

    char **confArray = getConfArrayFromMX(confCount, prhs[4], topicConfCount, prhs[5]);
    if (confArray == NULL)
        usageError("Problem creating strings for conf array");


    int ret = mwInitializeKafkaConsumer(&rk, brokers, group, topic, confCount, topicConfCount, (const char **)confArray);
    if (ret)
    {
        mwLog(MW_ERROR, "KafkaConsumer", "Problems initializing Kafka Consumer\n");
    }
    else
    {
        mwLog(MW_INFO, "KafkaConsumer", "Just started consumer");
        mwLog(MW_INFO, "Broker(s)", brokers);
        mwLog(MW_INFO, "Topic", topic);
        mwLog(MW_INFO, "Group", group);
    }

    freeConfArray(confArray, confCount + topicConfCount);

    delete[] brokers;
    delete[] topic;
    delete[] group;

    plhs[0] = setNewMXU64((void *)rk);
}

static void term(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs)
{
    if (mxGetClassID(prhs[1]) != mxUINT64_CLASS)
    {
        usageError("Second argument must be of type uint64_T\n");
    }

    rd_kafka_t *rk = (rd_kafka_t *)getMXU64Value(prhs[1]);
    mwTerminateKafkaConsumer(rk);
    mwLogTerminate();
}

static void consume(int nlhs, mxArray **plhs, const int nrhs, const mxArray **prhs)
{
    if (mxGetClassID(prhs[1]) != mxUINT64_CLASS)
    {
        usageError("Second argument (rk*) must be of type uint64_T\n");
    }

    rd_kafka_t *rk = (rd_kafka_t *)getMXU64Value(prhs[1]);

    rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rk, 100);
    if (rkmessage)
    {
        if (rkmessage->err)
        {
            // Error somehow
            plhs[0] = setNewInt8Arr(0, NULL);
            plhs[1] = setNewInt8Arr(0, NULL);
            plhs[2] = mxCreateString("Still an error, somehow ... ");
        }
        else
        {
            uint32_T mLen, kLen;

            kLen = rkmessage->key_len;
            ;
            plhs[0] = setNewInt8Arr(kLen, (int8_T *)rkmessage->key);

            mLen = (uint32_T)rkmessage->len;
            plhs[1] = setNewInt8Arr(mLen, (int8_T *)rkmessage->payload);

            if (nlhs > 2)
                plhs[2] = mxCreateString("");
        }

        rd_kafka_message_destroy(rkmessage);
    }
    else
    {
        plhs[0] = setNewInt8Arr(0, NULL);
        plhs[1] = setNewInt8Arr(0, NULL);
        plhs[2] = mxCreateString("### No message!!");
    }
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

static mxArray *setNewInt8Arr(uint32_T len, int8_T *ptr)
{
    mwSize nDims = 1;
    mwSize dims[] = {len};
    mxArray *R = mxCreateNumericArray(nDims, dims, mxINT8_CLASS, mxREAL);
    if (R == NULL)
    {
        usageError("Couldn't allocate numeric array\n");
    }
    if (len > 0)
    {
        mxInt8 *pr = mxGetInt8s(R);
        memcpy(pr, ptr, len * sizeof(mxInt8));
    }
    return R;
}

static void usageError(const char *msg)
{
    help();
    mexErrMsgTxt(msg);
}

static void help(void)
{
    mexPrintf("=========================================\n");
    mexPrintf("mx_kafka_consumer: Usage\n");
}

static void print_partition_list(const rd_kafka_topic_partition_list_t
                                     *partitions)
{
    int i;
    for (i = 0; i < partitions->cnt; i++)
    {
        mexPrintf("%s %s [%" PRId32 "] offset %" PRId64,
                  i > 0 ? "," : "",
                  partitions->elems[i].topic,
                  partitions->elems[i].partition,
                  partitions->elems[i].offset);
    }
    mexPrintf("\n");
}
