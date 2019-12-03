/*  Copyright 2019 The MathWorks, Inc. */

#define S_FUNCTION_NAME sf_decode_flat_json_object
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

// Needed for JSON decoding
#include "jansson.h"

enum
{
    EP_JSON_ENCODE = 0,
    EP_JSON_LEN,
    EP_OUT_LENGTH,
    EP_IN_LENGTH,
    EP_STRING_LIST,
    EP_STRING_LIST_RTW,
    EP_NumParams
};

typedef enum
{
    SF_DIR_DECODE = 1,
    SF_DIR_ENCODE
} SFCodeDir_T;

#define P_JSON_ENCODE ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_JSON_ENCODE))))
#define P_JSON_LEN ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_JSON_LEN))))
#define P_OUT_LENGTH ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_OUT_LENGTH))))
#define P_IN_LENGTH ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_IN_LENGTH))))
#define P_STRING_LIST (ssGetSFcnParam(S, EP_STRING_LIST))
#define P_STRING_LIST_RTW (ssGetSFcnParam(S, EP_STRING_LIST_RTW))

static char errstr[512];

static real_T getRealFromJSONField(const char *fieldName, json_t *obj)
{
    json_t *F;
    real_T FV = 0.0;

    F = json_object_get(obj, fieldName);

    if (!F)
    {
        mexPrintf("No such JSON field, %s\n", fieldName);
        return FV;
    }
    if (json_is_real(F))
    {
        FV = json_real_value(F);
    }
    else if (json_is_integer(F))
    {
        FV = (real_T)json_integer_value(F);
    }
    else
    {
        mexPrintf("Bad type for JSON value\n");
    }

    return FV;
}

static char *getStringFromParamCellString(SimStruct *S, const mxArray *P, int idx)
{
    static char gsfpErr[1024];

    if (mxGetClassID(P) != mxCELL_CLASS)
    {
        sprintf(gsfpErr, "The parameter must be a cell array\n");
        ssSetErrorStatus(S, gsfpErr);
        return NULL;
    }

    const mxArray *Pel = mxGetCell(P, idx);
    if (mxGetClassID(Pel) != mxCHAR_CLASS)
    {
        sprintf(gsfpErr, "All elements must be character arrays. Element [%d] isn't.\n", idx);
        ssSetErrorStatus(S, gsfpErr);
        return NULL;
    }

    mwSize N = (mwSize)1 + mxGetNumberOfElements(Pel);
    char *newStr = new char[N];
    if (newStr == NULL)
    {
        sprintf(gsfpErr, "Couldn't allocate memory for element [%d]\n", idx);
        ssSetErrorStatus(S, gsfpErr);
        return NULL;
    }

    if (mxGetString(Pel, newStr, N))
    {
        delete[] newStr;
        newStr = NULL;
        sprintf(gsfpErr, "Couldn't read string element [%d]]\n", idx);
        ssSetErrorStatus(S, gsfpErr);
    }
    return newStr;
}

static int getParamString(SimStruct *S, char **strPtr, const mxArray *prm, int epwIdx, char *errorHelp)
{
    int N = (int)mxGetNumberOfElements(prm) + 1;
    char *tmp = new char[N];
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
    ssSetPWorkValue(S, epwIdx, tmp);
    *strPtr = tmp;

    return 0;
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
    ssSetNumSFcnParams(S, EP_NumParams); /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S))
    {
        /* Return if number of expected != number of actual parameters */
        return;
    }

    int k, numFields = mxGetNumberOfElements(P_STRING_LIST);

    ssSetSFcnParamNotTunable(S, EP_JSON_ENCODE);
    ssSetSFcnParamNotTunable(S, EP_JSON_LEN);
    ssSetSFcnParamNotTunable(S, EP_OUT_LENGTH);
    ssSetSFcnParamNotTunable(S, EP_IN_LENGTH);
    ssSetSFcnParamNotTunable(S, EP_STRING_LIST);
    ssSetSFcnParamNotTunable(S, EP_STRING_LIST_RTW);

    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (P_JSON_ENCODE == SF_DIR_DECODE)
    {
        // DECODING
        int nIn = (P_IN_LENGTH != 0) ? 2 : 1;
        if (!ssSetNumInputPorts(S, nIn))
            return;
        ssSetInputPortDataType(S, 0, SS_INT8);
        ssSetInputPortWidth(S, 0, P_JSON_LEN);
        ssSetInputPortRequiredContiguous(S, 0, true); /*direct input signal access*/
        ssSetInputPortDirectFeedThrough(S, 0, 1);

        if (P_IN_LENGTH != 0)
        {
            ssSetInputPortDataType(S, 1, SS_UINT32);
            ssSetInputPortWidth(S, 1, 1);
            ssSetInputPortRequiredContiguous(S, 1, true); /*direct input signal access*/
            ssSetInputPortDirectFeedThrough(S, 1, 1);
        }

        if (!ssSetNumOutputPorts(S, numFields))
            return;
        for (k = 0; k < numFields; ++k)
        {
            ssSetOutputPortWidth(S, k, 1);
        }
    }
    else
    {
        // ENCODING
        int nOut = (P_OUT_LENGTH != 0) ? 2 : 1;
        if (!ssSetNumInputPorts(S, numFields))
            return;
        for (k = 0; k < numFields; ++k)
        {
            ssSetInputPortDataType(S, k, SS_DOUBLE);
            ssSetInputPortWidth(S, k, 1);
            ssSetInputPortRequiredContiguous(S, k, true); /*direct input signal access*/
            ssSetInputPortDirectFeedThrough(S, k, 1);
        }

        if (!ssSetNumOutputPorts(S, nOut))
            return;
        ssSetOutputPortWidth(S, 0, P_JSON_LEN);
        ssSetOutputPortDataType(S, 0, SS_INT8);
        if (P_OUT_LENGTH != 0)
        {
            ssSetOutputPortWidth(S, 1, 1);
            ssSetOutputPortDataType(S, 1, SS_UINT32);
        }
    }

    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    if (ssRTWGenIsCodeGen(S))
    {
        ssSetNumPWork(S, 0);
    }
    else
    {
        ssSetNumPWork(S, numFields);
    }
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);

    /* Specify the sim state compliance to be same as a built-in block */
    ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

    ssSetOptions(S, 0 | SS_OPTION_CALL_TERMINATE_ON_EXIT);
}

/* Function: mdlInitializeSampleTimes =========================================
 * Abstract:
 *    This function is used to specify the sample time(s) for your
 *    S-function. You must register the same number of sample times as
 *    specified in ssSetNumSampleTimes.
 */
static void mdlInitializeSampleTimes(SimStruct *S)
{
    ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
    ssSetOffsetTime(S, 0, 0.0);
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
    if (!ssRTWGenIsCodeGen(S))
    {
        int k, numFields = mxGetNumberOfElements(P_STRING_LIST);
        for (k = 0; k < numFields; ++k)
        {
            ssSetPWorkValue(S, k, getStringFromParamCellString(S, P_STRING_LIST, k));
            // mexPrintf("Found string '%s'\n", ssGetPWorkValue(S, k));
        }
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

    if (P_JSON_ENCODE == SF_DIR_DECODE)
    {
        // DECODING
        json_t *root;
        json_error_t error;

        const char *u = (const char *)ssGetInputPortSignal(S, 0);
        int k, numFields = mxGetNumberOfElements(P_STRING_LIST);

        if (P_IN_LENGTH != 0)
        {
            uint32_T *pu = (uint32_T *)ssGetInputPortSignal(S, 1);
            uint32_T len = *pu;
            root = json_loadb(u, len, 0, &error);
        }
        else
        {
            root = json_loads(u, 0, &error);
        }
        if (!root)
        {
            sprintf(errstr, "Error: on line %d: %s\n", error.line, error.text);
            // ssSetErrorStatus(S, errstr);
            mexPrintf(errstr);
            // mexPrintf("## %s\n", (char*) u);
            return;
        }

        if (!json_is_object(root))
        {
            mexPrintf("The toplevel element should be an object in our case\n");
            goto jsondecode_exit;
        }

        for (k = 0; k < numFields; ++k)
        {
            const char *fieldName = (const char *)ssGetPWorkValue(S, k);
            real_T *Y = (real_T *)ssGetOutputPortSignal(S, k);
            Y[0] = getRealFromJSONField(fieldName, root);
        }

    jsondecode_exit:
        json_decref(root);
    }
    else
    {
        // ENCODING
        int8_T *Y = (int8_T *)ssGetOutputPortSignal(S, 0);
        int count = 0;
        int k, numFields = mxGetNumberOfElements(P_STRING_LIST);
        char *JS = (char *)Y;
        for (k = 0; k < numFields; ++k)
        {
            char *fieldName = (char *)ssGetPWorkValue(S, k);
            real_T *uk = (real_T *)ssGetInputPortSignal(S, k);
            if (k == 0)
            {
                count += sprintf(JS + count, "{\"%s\":%.10f", fieldName, uk[0]);
            }
            else
            {
                count += sprintf(JS + count, ",\"%s\":%.10f", fieldName, uk[0]);
            }
        }
        JS[count++] = '}';
        JS[count++] = '\0';
        if (P_OUT_LENGTH != 0)
        {
            uint32_T *msgLen = (uint32_T *)ssGetOutputPortSignal(S, 1);
            *msgLen = count - 1;
        }
        // mexPrintf("count == %d\n", count);
    }
}

static void mdlTerminate(SimStruct *S)
{
    void **pwp = ssGetPWork(S);
    if (pwp == NULL)
    {
        // This was just a model update, no need to free resources.
        return;
    }

    int k, numFields = mxGetNumberOfElements(P_STRING_LIST);
    for (k = 0; k < numFields; ++k)
    {
        char *msg = (char *)ssGetPWorkValue(S, k);
        if (msg != NULL)
        {
            delete[] msg;
            ssSetPWorkValue(S, k, NULL);
        }
    }
}

#define MDL_RTW /* Change to #undef to remove function */
#if defined(MDL_RTW) && defined(MATLAB_MEX_FILE)
static void mdlRTW(SimStruct *S)
{
    int N = 1 + mxGetNumberOfElements(P_STRING_LIST_RTW);
    char *str = new char[N];
    int32_T useInLength = (int32_T) P_IN_LENGTH;
    int32_T useOutLength = (int32_T) P_OUT_LENGTH;
    if (str == NULL)
    {
        ssSetErrorStatus(S, "Couldn''t allocate string in mdlRTW");
        return;
    }

    if (mxGetString(P_STRING_LIST_RTW, str, N))
    {
        ssSetErrorStatus(S, "Couldn't read string in mdlRTW");
        delete[] str;
        return;
    }

    int numFields = mxGetNumberOfElements(P_STRING_LIST);

    int32_T isEncoding = P_JSON_ENCODE == SF_DIR_ENCODE;
    if (!ssWriteRTWParamSettings(S, 4,
                                 SSWRITE_VALUE_VECT_STR, "JSONFieldList", (const char_T *)str, numFields,
                                 SSWRITE_VALUE_DTYPE_NUM, "IsEncoding", (const void *)&isEncoding, SS_INT32,
                                 SSWRITE_VALUE_DTYPE_NUM, "UseInLength", (const void *)&useInLength, SS_INT32,
                                 SSWRITE_VALUE_DTYPE_NUM, "UseOutLength", (const void *)&useOutLength, SS_INT32))
    {
        return; // (error reporting will be handled by SL)
    }

    delete[] str;
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
