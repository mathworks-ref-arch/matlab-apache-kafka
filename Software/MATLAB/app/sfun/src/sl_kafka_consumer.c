/*
 * sl_kafka_consumer
 *
 * Copyright 2019 The MathWorks, Inc.
 */

/**
 * Simple Apache Kafka consumer using the Kafka driver from librdkafka
 * (https://github.com/edenhill/librdkafka)
 */

#define S_FUNCTION_NAME sl_kafka_consumer
#define S_FUNCTION_LEVEL 2

/*
 * Need to include simstruc.h for the definition of the SimStruct and
 * its associated macro definitions.
 */
#include "simstruc.h"
#include "fixedpoint.h"

#include "rdkafka.h"

#include <time.h>

#include "mw_kafka_utils.h"
#include "mx_kafka_utils.h"

enum
{
    EP_BROKERS = 0,
    EP_TOPIC,
    EP_GROUP,
    EP_MSG_LEN,
    EP_KEY_LEN,
    EP_OUTPUT_TIMESTAMP,
    EP_CONF,
    EP_TOPIC_CONF,
    EP_COMBINED_CONF_STR,
    EP_TS,
    EP_NumParams
};

#define P_BROKER (ssGetSFcnParam(S, EP_BROKERS))
#define P_TOPIC (ssGetSFcnParam(S, EP_TOPIC))
#define P_GROUP (ssGetSFcnParam(S, EP_GROUP))
#define P_MSG_LEN ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_MSG_LEN))))
#define P_KEY_LEN ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_KEY_LEN))))
#define P_OUTPUT_TIMESTAMP ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_OUTPUT_TIMESTAMP))))
#define P_CONF (ssGetSFcnParam(S, EP_CONF))
#define P_TOPIC_CONF (ssGetSFcnParam(S, EP_TOPIC_CONF))
#define P_COMBINED_CONF_STR (ssGetSFcnParam(S, EP_COMBINED_CONF_STR))
#define P_TS (ssGetSFcnParam(S, EP_TS))

enum
{
    EPW_KAFKA_CONSUMER = 0,
    EPW_NumPWorks
};

static int wait_eof = 0; /* number of partitions awaiting EOF */

static char errstr[512]; /* librdkafka API error reporting buffer */

static int getParamString(SimStruct *S, char **strPtr, const mxArray *prm, int epwIdx, char *errorHelp)
{
    int N = (int)mxGetNumberOfElements(prm);
    char *tmp = (char *)malloc(N + 1);
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

void initKafkaConsumer(SimStruct *S)
{
    rd_kafka_t *rk = NULL;        /* Consumer instance handle */
    rd_kafka_topic_t *rkt = NULL; /* Topic object */
    rd_kafka_conf_t *conf = NULL; /* Temporary configuration object */
    rd_kafka_topic_conf_t *topic_conf = NULL;
    rd_kafka_topic_partition_list_t *topics;
    int partition = RD_KAFKA_PARTITION_UA;
    int64_t start_offset = RD_KAFKA_OFFSET_BEGINNING;
    /*
     *  - RD_KAFKA_OFFSET_BEGINNING
     *  - RD_KAFKA_OFFSET_END
     *  - RD_KAFKA_OFFSET_STORED
     *  - RD_KAFKA_OFFSET_TAIL
     */

    char *brokers; /* Argument: broker list */
    char *topic;   /* Argument: topic to produce to */
    char *group;   /* The consumer group for the consumer, may be NULL */

    int nConf, nTopicConf;

    mwLogInit("simulink");

    if (getParamString(S, &brokers, P_BROKER, -1, "brokers"))
        goto exit_init_kafka;
    // LOG(S, brokers);
    if (getParamString(S, &topic, P_TOPIC, -1, "topic"))
        goto exit_init_kafka;
    // LOG(S, topic);

    if (getParamString(S, &group, P_GROUP, -1, "group"))
        goto exit_init_kafka;
    // LOG(S, group);
    mexPrintf("### brokers: %s, topic: %s, group: %s\n", brokers, topic, group);

    nConf = mxGetNumberOfElements(P_CONF);
    nTopicConf = mxGetNumberOfElements(P_TOPIC_CONF);
    mexPrintf("There are %d conf and %d topicConf entries.\n", nConf, nTopicConf);
    const char **confArray = getConfArrayFromMX(nConf, P_CONF, nTopicConf, P_TOPIC_CONF);

    if (confArray == NULL)
    {
        ssSetErrorStatus(S, "Couldn't retrieve confArray from parameters");
        return;
    }

    int res = mwInitializeKafkaConsumer(&rk, brokers, group, topic, nConf, nTopicConf, confArray);
    if (res)
    {
        ssSetErrorStatus(S, "Problems initializing Kafka Consumer\n");
        goto exit_init_kafka;
    }
    ssSetPWorkValue(S, EPW_KAFKA_CONSUMER, rk);

exit_init_kafka:
    if (brokers != NULL)
    {
        free(brokers);
    }
    if (group != NULL)
    {
        free(group);
    }
    if (topic != NULL)
    {
        free(topic);
    }
    if (conf != NULL)
    {
        rd_kafka_conf_destroy(conf);
    }
}

/*====================*
 * S-function methods *
 *====================*/

/* Function: mdlInitializeSizes ===============================================
 * Abstract:
 *    The sizes information is used by Simulink to determine the S-function
 *    block's characteristics (number of inputs, outputs, states, etc.).
 */
static void mdlInitializeSizes(SimStruct *S)
{
    int_T numOutports = 5;
    DTypeId f64_id;

    // printSimMode(S, "mdlInitializeSizes");

    ssSetNumSFcnParams(S, EP_NumParams); /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
        return;

    if (P_OUTPUT_TIMESTAMP != 0)
    {
        numOutports += 1;
    }
    ssSetSFcnParamNotTunable(S, EP_BROKERS);
    ssSetSFcnParamNotTunable(S, EP_TOPIC);
    ssSetSFcnParamNotTunable(S, EP_GROUP);
    ssSetSFcnParamNotTunable(S, EP_MSG_LEN);
    ssSetSFcnParamNotTunable(S, EP_KEY_LEN);
    ssSetSFcnParamNotTunable(S, EP_OUTPUT_TIMESTAMP);
    ssSetSFcnParamNotTunable(S, EP_CONF);
    ssSetSFcnParamNotTunable(S, EP_TOPIC_CONF);
    ssSetSFcnParamNotTunable(S, EP_COMBINED_CONF_STR);
    ssSetSFcnParamNotTunable(S, EP_TS);

    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (!ssSetNumInputPorts(S, 0))
        return;
    // ssSetInputPortWidth(S, 0, 1);
    // ssSetInputPortDataType(S, 0, SS_INT8);
    // ssSetInputPortRequiredContiguous(S, 0, true); /*direct input signal access*/
    // ssSetInputPortDirectFeedThrough(S, 0, 1);

    if (!ssSetNumOutputPorts(S, numOutports))
        return;
    // The function call output
    ssSetOutputPortWidth(S, 0, 1);
    ssSetOutputPortDataType(S, 0, SS_FCN_CALL);
    // The real message
    ssSetOutputPortWidth(S, 1, P_MSG_LEN);
    ssSetOutputPortDataType(S, 1, SS_INT8);
    // The message length
    ssSetOutputPortWidth(S, 2, 1);
    ssSetOutputPortDataType(S, 2, SS_UINT32);
    // The key
    ssSetOutputPortWidth(S, 3, P_KEY_LEN);
    ssSetOutputPortDataType(S, 3, SS_INT8);
    // The key length
    ssSetOutputPortWidth(S, 4, 1);
    ssSetOutputPortDataType(S, 4, SS_UINT32);

    if (P_OUTPUT_TIMESTAMP != 0)
    {
        // The timestamp
        f64_id =
            ssRegisterDataTypeFxpBinaryPoint(S,
                                             1,  // int isSigned,
                                             64, //int wordLength,
                                             0,  //int fractionLength,
                                             0   //int obeyDataTypeOverride
            );

        if (f64_id == INVALID_DTYPE_ID)
        {
            ssSetErrorStatus(S, "Couldn't register f64 datatype");
            return;
        }
        ssSetOutputPortWidth(S, 5, 1);
        ssSetOutputPortDataType(S, 5, f64_id);
    }
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
    // printSimMode(S, "mdlInitializeSampleTimes");
    pr = mxGetPr(P_TS);
    ts = pr[0];
    if (mxGetNumberOfElements(P_TS) > 1)
    {
        offset = pr[1];
    }
    ssSetSampleTime(S, 0, ts);
    ssSetOffsetTime(S, 0, offset);

    ssSetCallSystemOutput(S, 0); /* call on first element */
    ssSetModelReferenceSampleTimeDefaultInheritance(S);
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
        // printSimMode(S, "mdlStart");

        initKafkaConsumer(S);
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
    int ret;
    rd_kafka_t *rk = (rd_kafka_t *)ssGetPWorkValue(S, EPW_KAFKA_CONSUMER);

    int8_T *msg = (int8_T *)ssGetOutputPortSignal(S, 1);
    uint32_T *msgLen = (uint32_T *)ssGetOutputPortSignal(S, 2);
    int8_T *key = (int8_T *)ssGetOutputPortSignal(S, 3);
    uint32_T *keyLen = (uint32_T *)ssGetOutputPortSignal(S, 4);

    int64_T *timestamp = NULL;
    if (P_OUTPUT_TIMESTAMP)
    {
        timestamp = (int64_T *)ssGetOutputPortSignal(S, 5);
    }

    ret = mwConsumeKafkaMessage(rk, msg, msgLen, P_MSG_LEN,
                                key, keyLen, P_KEY_LEN, timestamp);

    if (ret != 0)
    {
        //Call the subsystem attached
        if (!ssCallSystemWithTid(S, 0, tid))
        {
            /* Error occurred which will be reported by Simulink */
            return;
        }
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
        static int verbose = 1;
        void **pwp = ssGetPWork(S);
        if (pwp == NULL)
        {
            // This was just a model update, no need to free resources.
            return;
        }
        mexPrintf("sl_kafka_consumer@mdlTerminate(): Freeing up used resources\n");

        rd_kafka_t *rk = (rd_kafka_t *)ssGetPWorkValue(S, EPW_KAFKA_CONSUMER);

        mwTerminateKafkaConsumer(rk);

        ssSetPWorkValue(S, EPW_KAFKA_CONSUMER, NULL);
    }
}

#define MDL_RTW /* Change to #undef to remove function */
#if defined(MDL_RTW) && defined(MATLAB_MEX_FILE)
static void mdlRTW(SimStruct *S)
{
    char *brokers = NULL, *group = NULL, *topic = NULL, *confArray = NULL;
    int32_T nConf = mxGetNumberOfElements(P_CONF);
    int32_T nTopicConf = mxGetNumberOfElements(P_TOPIC_CONF);

    if (getParamString(S, &brokers, P_BROKER, -1, "brokers"))
        goto sl_kafka_consumer_mdl_rtw_exit;
    if (getParamString(S, &topic, P_TOPIC, -1, "topic"))
        goto sl_kafka_consumer_mdl_rtw_exit;
    if (getParamString(S, &group, P_GROUP, -1, "group"))
        goto sl_kafka_consumer_mdl_rtw_exit;
    if (getParamString(S, &confArray, P_COMBINED_CONF_STR, -1, "confArray"))
        goto sl_kafka_consumer_mdl_rtw_exit;
    if (getParamString(S, &brokers, P_BROKER, -1, "brokers"))
        return;
    if (getParamString(S, &topic, P_TOPIC, -1, "topic"))
        return;
    if (getParamString(S, &group, P_GROUP, -1, "group"))
        return;

    if (!ssWriteRTWParamSettings(S, 6,
                                 SSWRITE_VALUE_QSTR, "Brokers", (const void *)brokers,
                                 SSWRITE_VALUE_QSTR, "Topic", (const void *)topic,
                                 SSWRITE_VALUE_QSTR, "Group", (const void *)group,
                                 SSWRITE_VALUE_DTYPE_NUM, "nConf", (const void *)&nConf, SS_INT32,
                                 SSWRITE_VALUE_DTYPE_NUM, "nTopicConf", (const void *)&nTopicConf, SS_INT32,
                                 SSWRITE_VALUE_VECT_STR, "ConfArray", (const char_T *)confArray, nConf+nTopicConf))
    {
        return; // (error reporting will be handled by SL)
    }
sl_kafka_consumer_mdl_rtw_exit:
    if (brokers != NULL)
        free(brokers);
    if (topic != NULL)
        free(topic);
    if (group != NULL)
        free(group);
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
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
