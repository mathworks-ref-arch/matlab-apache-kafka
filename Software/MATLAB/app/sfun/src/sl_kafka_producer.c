/*
 * sl_kafka_producer
 *
 * Copyright 2019 The MathWorks, Inc.
 */

/**
 * Simple Apache Kafka producer using the Kafka driver from librdkafka
 * (https://github.com/edenhill/librdkafka)
 */

#define S_FUNCTION_NAME sl_kafka_producer
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"
#include "fixedpoint.h"

#include "rdkafka.h"

#include "mw_kafka_utils.h"
#include "mx_kafka_utils.h"

enum
{
    EP_BROKERS = 0,
    EP_TOPIC,
    EP_USE_EXTERNAL_KEY,
    EP_EXT_KEY_LEN,
    EP_KEY,
    EP_MSG_LEN,
    EP_USE_EXT_TIMESTAMP,
    EP_CONF,
    EP_TOPIC_CONF,
    EP_COMBINED_CONF_STR,
    EP_TS,
    EP_NumParams
};

#define P_BROKER (ssGetSFcnParam(S, EP_BROKERS))
#define P_TOPIC (ssGetSFcnParam(S, EP_TOPIC))
#define P_USE_EXT_KEY ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_USE_EXTERNAL_KEY))))
#define P_EXT_KEY_LEN ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_EXT_KEY_LEN))))
#define P_KEY (ssGetSFcnParam(S, EP_KEY))
#define P_MSG_LEN ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_MSG_LEN))))
#define P_USE_EXT_TIMESTAMP ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_USE_EXT_TIMESTAMP))))
#define P_CONF (ssGetSFcnParam(S, EP_CONF))
#define P_TOPIC_CONF (ssGetSFcnParam(S, EP_TOPIC_CONF))
#define P_COMBINED_CONF_STR (ssGetSFcnParam(S, EP_COMBINED_CONF_STR))
#define P_TS (ssGetSFcnParam(S, EP_TS))

enum
{
    EPW_KAFKA_PRODUCER = 0,
    EPW_KAFKA_TOPIC,
    EPW_BROKERS,
    EPW_TOPIC,
    EPW_KEY,
    EPW_NumPWorks
};

static char errstr[512]; /* librdkafka API error reporting buffer */

static int getParamString(SimStruct *S, char **strPtr, const mxArray *prm, int epwIdx, char *errorHelp)
{
    int N = mxGetNumberOfElements(prm);
    char *tmp = (char *)malloc((N + 1) * sizeof(char));
    if (tmp == NULL)
    {
        sprintf(errstr, "Couldn't allocate string for '%s'\n", errorHelp);
        ssSetErrorStatus(S, errstr);
        return 1;
    }
    if (mxGetString(prm, tmp, N + 1))
    {
        sprintf(errstr, "Couldn't retrieve '%s' string\n", errorHelp);
        ssSetErrorStatus(S, errstr);
        return 2;
    }
    if (epwIdx >= 0)
    {
        ssSetPWorkValue(S, epwIdx, tmp);
    }
    *strPtr = tmp;

    return 0;
}

void initKafkaProducer(SimStruct *S)
{
    rd_kafka_t *rk = NULL;        /* Producer instance handle */
    rd_kafka_topic_t *rkt = NULL; /* Topic object */

    char *brokers = NULL; /* Argument: broker list */
    char *topic = NULL;   /* Argument: topic to produce to */
    char *key = NULL;     /* The key for the topic, may be NULL */

    int ret;
    int nConf, nTopicConf;

    mwLogInit("simulink");
    // Get broker string and topics string
    int N = mxGetNumberOfElements(P_BROKER);
    if (getParamString(S, &brokers, P_BROKER, EPW_BROKERS, "brokers"))
        return;

    if (getParamString(S, &topic, P_TOPIC, EPW_TOPIC, "topic"))
        return;

    if (getParamString(S, &key, P_KEY, EPW_KEY, "key"))
        return;

    nConf = mxGetNumberOfElements(P_CONF);
    nTopicConf = mxGetNumberOfElements(P_TOPIC_CONF);
    const char **confArray = getConfArrayFromMX(nConf, P_CONF, nTopicConf, P_TOPIC_CONF);

    if (confArray == NULL)
    {
        ssSetErrorStatus(S, "Couldn't retrieve confArray from parameters");
        return;
    }

    ret = mwInitializeKafkaProducer(&rk, &rkt, brokers, topic, nConf, nTopicConf, confArray);

    if (confArray != NULL)
    {
        freeConfArray((char **)confArray, nConf + nTopicConf);
    }

    if (ret)
    {
        ssSetErrorStatus(S, "Couldn't initialize Kafka Producer");
    }
    ssSetPWorkValue(S, EPW_KAFKA_PRODUCER, rk);
    ssSetPWorkValue(S, EPW_KAFKA_TOPIC, rkt);
}

/*====================*
 * S-function methods *
 *====================*/

static void mdlInitializeSizes(SimStruct *S)
{
    int_T curPort = 0;
    int_T numInports = 1 + P_USE_EXT_KEY + P_USE_EXT_TIMESTAMP;
    ssSetNumSFcnParams(S, EP_NumParams); /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    {
        /* Return if number of expected != number of actual parameters */
        return;
    }

    ssSetSFcnParamNotTunable(S, EP_BROKERS);
    ssSetSFcnParamNotTunable(S, EP_TOPIC);
    ssSetSFcnParamNotTunable(S, EP_USE_EXTERNAL_KEY);
    ssSetSFcnParamNotTunable(S, EP_EXT_KEY_LEN);
    ssSetSFcnParamNotTunable(S, EP_KEY);
    ssSetSFcnParamNotTunable(S, EP_MSG_LEN);
    ssSetSFcnParamNotTunable(S, EP_USE_EXT_TIMESTAMP);
    ssSetSFcnParamNotTunable(S, EP_CONF);
    ssSetSFcnParamNotTunable(S, EP_TOPIC_CONF);
    ssSetSFcnParamNotTunable(S, EP_COMBINED_CONF_STR);
    ssSetSFcnParamNotTunable(S, EP_TS);

    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (!ssSetNumInputPorts(S, numInports))
        return;
    curPort = 0;
    ssSetInputPortWidth(S, curPort, P_MSG_LEN);
    ssSetInputPortDataType(S, curPort, SS_INT8);
    ssSetInputPortRequiredContiguous(S, curPort, true); /*direct input signal access*/
    ssSetInputPortDirectFeedThrough(S, curPort, 1);
    if (P_USE_EXT_KEY)
    {
        curPort++;
        ssSetInputPortWidth(S, curPort, P_EXT_KEY_LEN);
        ssSetInputPortDataType(S, curPort, SS_INT8);
        ssSetInputPortRequiredContiguous(S, curPort, true); /*direct input signal access*/
        ssSetInputPortDirectFeedThrough(S, curPort, 1);
    }
    if (P_USE_EXT_TIMESTAMP)
    {
        DTypeId dataTypeIdReg;

        dataTypeIdReg = ssRegisterDataTypeFxpBinaryPoint(S, 1, 64, 0, 1);
        if (dataTypeIdReg == INVALID_DTYPE_ID)
            return;

        curPort++;
        ssSetInputPortWidth(S, curPort, 1);
        ssSetInputPortDataType(S, curPort, dataTypeIdReg);
        ssSetInputPortRequiredContiguous(S, curPort, true); /*direct input signal access*/
        ssSetInputPortDirectFeedThrough(S, curPort, 1);
    }

    if (!ssSetNumOutputPorts(S, 0))
        return;

    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    ssSetNumPWork(S, EPW_NumPWorks);
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);

    /* Specify the sim state compliance to be same as a built-in block */
    ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

    ssSetOptions(S,
                 SS_OPTION_CALL_TERMINATE_ON_EXIT);
}

/* Function: mdlInitializeSampleTimes =========================================
 * Abstract:
 *    This function is used to specify the sample time(s) for your
 *    S-function. You must register the same number of sample times as
 *    specified in ssSetNumSampleTimes.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    real_T *pr, ts, offset = 0.0;
    pr = mxGetPr(P_TS);
    ts = pr[0];
    if (mxGetNumberOfElements(P_TS) > 1)
    {
        offset = pr[1];
    }
    ssSetSampleTime(S, 0, ts);
    ssSetOffsetTime(S, 0, offset);
}

#define MDL_START /* Change to #undef to remove function */
#if defined(MDL_START)
/* Function: mdlStart =======================================================
   * Abstract:
   *    This function is called once at start of model execution. If you
   *    have states that should be initialized once, this is the place
   *    to do it.
   */
static void mdlStart(SimStruct *S)
{
    if (ssGetSimMode(S) == SS_SIMMODE_NORMAL)
    {
        // Only initialize Kafka when we're actually running in Simulink
        initKafkaProducer(S);
    }
}
#endif /*  MDL_START */

/* Function: mdlOutputs =======================================================
 * Abstract:
 *    In this function, you compute the outputs of your S-function
 *    block.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
    const int8_T *u = (const int8_T *)ssGetInputPortSignal(S, 0);
    rd_kafka_t *rk = ssGetPWorkValue(S, EPW_KAFKA_PRODUCER);     /* Producer instance handle */
    rd_kafka_topic_t *rkt = ssGetPWorkValue(S, EPW_KAFKA_TOPIC); /* Topic object */
    char *buf = (char *)u;                                       /* Message value temporary buffer */
    char *brokers;                                               /* Argument: broker list */
    char *topic;                                                 /* Argument: topic to produce to */
    char *key;
    int keylen;
    int N = strlen(buf);
    int ret;

    int inIdx = 0;
    if (P_USE_EXT_KEY)
    {
        inIdx++;
        int8_T *u2 = (const int8_T *)ssGetInputPortSignal(S, inIdx);
        key = (char *)u2;
    }
    else
    {
        key = ssGetPWorkValue(S, EPW_KEY); /* The key for the topic, may be NULL */
    }
    keylen = strlen(key);

    if (P_USE_EXT_TIMESTAMP)
    {
        inIdx++;
        int64_T* timestamp = (int64_T*) ssGetInputPortSignal(S, inIdx);
        ret = mwProduceKafkaMessageWithTimestamp(rk, rkt, key, keylen, buf, N, timestamp[0]);
    }
    else
    {
        ret = mwProduceKafkaMessage(rk, rkt, key, keylen, buf, N);
    }
    if (ret)
    {
        ssSetErrorStatus(S, "Failed producing message\n");
    }
}

/* Function: mdlTerminate =====================================================
 * Abstract:
 *    In this function, you should perform any actions that are necessary
 *    at the termination of a simulation.  For example, if memory was
 *    allocated in mdlStart, this is the place to free it.
 */
static void mdlTerminate(SimStruct *S)
{
    if (ssGetSimMode(S) == SS_SIMMODE_NORMAL)
    {
        // We only need to terminate properly in normal simulation mode
        static int verbose = 0;
        void **pwp = ssGetPWork(S);
        if (pwp == NULL)
        {
            // This was just a model update, no need to free resources.
            return;
        }
        mexPrintf("sl_kafka_producer@mdlTerminate(): Freeing up used resources\n");

        rd_kafka_topic_t *rkt = (rd_kafka_topic_t *)ssGetPWorkValue(S, EPW_KAFKA_TOPIC);
        rd_kafka_t *rk = (rd_kafka_t *)ssGetPWorkValue(S, EPW_KAFKA_PRODUCER);

        mwTerminateKafkaProducer(rk, rkt);

        ssSetPWorkValue(S, EPW_KAFKA_TOPIC, NULL);
        ssSetPWorkValue(S, EPW_KAFKA_PRODUCER, NULL);

        char *brokers = (char *)ssGetPWorkValue(S, EPW_BROKERS);
        if (brokers != NULL)
        {
            if (verbose)
                mexPrintf("\tFreeing brokers string\n");
            free(brokers);
            ssSetPWorkValue(S, EPW_BROKERS, NULL);
        }
        char *topic = (char *)ssGetPWorkValue(S, EPW_TOPIC);
        if (topic != NULL)
        {
            if (verbose)
                mexPrintf("\tFreeing topic string\n");
            free(topic);
            ssSetPWorkValue(S, EPW_TOPIC, NULL);
        }
        char *key = (char *)ssGetPWorkValue(S, EPW_KEY);
        if (key != NULL)
        {
            if (verbose)
                mexPrintf("\tFreeing key string\n");
            free(key);
            ssSetPWorkValue(S, EPW_KEY, NULL);
        }
        mwLogTerminate();
    }
}

#define MDL_RTW /* Change to #undef to remove function */
#if defined(MDL_RTW) && defined(MATLAB_MEX_FILE)
static void mdlRTW(SimStruct *S)
{
    char *brokers = NULL, *topic = NULL, *key = NULL, *confArray = NULL;
    int32_T useExt = P_USE_EXT_KEY;
    int32_T useExtTimestamp = P_USE_EXT_TIMESTAMP;
    int32_T nConf = mxGetNumberOfElements(P_CONF);
    int32_T nTopicConf = mxGetNumberOfElements(P_TOPIC_CONF);

    if (getParamString(S, &brokers, P_BROKER, -1, "brokers"))
        goto sl_kafka_producer_mdl_rtw_exit;
    if (getParamString(S, &topic, P_TOPIC, -1, "topic"))
        goto sl_kafka_producer_mdl_rtw_exit;
    if (getParamString(S, &key, P_KEY, -1, "key"))
        goto sl_kafka_producer_mdl_rtw_exit;
    if (getParamString(S, &confArray, P_COMBINED_CONF_STR, -1, "confArray"))
        goto sl_kafka_producer_mdl_rtw_exit;

    if (!ssWriteRTWParamSettings(S, 8,
                                 SSWRITE_VALUE_QSTR, "Brokers", (const void *)brokers,
                                 SSWRITE_VALUE_QSTR, "Topic", (const void *)topic,
                                 SSWRITE_VALUE_QSTR, "Key", (const void *)key,
                                 SSWRITE_VALUE_DTYPE_NUM, "UseExtKey", (const void *)&useExt, SS_INT32,
                                 SSWRITE_VALUE_DTYPE_NUM, "UseExtTimestamp", (const void *)&useExtTimestamp, SS_INT32,
                                 SSWRITE_VALUE_DTYPE_NUM, "nConf", (const void *)&nConf, SS_INT32,
                                 SSWRITE_VALUE_DTYPE_NUM, "nTopicConf", (const void *)&nTopicConf, SS_INT32,
                                 SSWRITE_VALUE_VECT_STR, "ConfArray", (const char_T *)confArray, nConf + nTopicConf))
    {
        // (error reporting will be handled by SL)
    }
sl_kafka_producer_mdl_rtw_exit:
    if (brokers != NULL)
        free(brokers);
    if (topic != NULL)
        free(topic);
    if (key != NULL)
        free(key);
    if (confArray != NULL)
        free(confArray);

    // static int getParamString(SimStruct *S, char **strPtr, const mxArray *prm, int epwIdx, char *errorHelp)
}
#endif /* MDL_RTW */

/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#include "simulink.c"  /* MEX-file interface mechanism */
#include "fixedpoint.c"
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
