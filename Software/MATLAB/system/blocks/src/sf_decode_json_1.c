/*  Copyright 2019 The MathWorks, Inc. */

#define S_FUNCTION_NAME  sf_decode_json_1
#define S_FUNCTION_LEVEL 2

#include "simstruc.h"

// Needed for JSON decoding
#include "jansson.h"

enum
{
    EP_JSON_LEN = 0,
    EP_NumParams
};

#define P_JSON_LEN ((int_T)mxGetScalar((ssGetSFcnParam(S, EP_JSON_LEN))))


static char errstr[512];

static real_T getRealFromJSONField(const char *fieldName, json_t *obj) {
    json_t *F;
    real_T FV = 0.0;

    F = json_object_get(obj, fieldName);

    if (!F) {
      mexPrintf("No such JSON field, %s\n", fieldName);
      return FV;
    }
    if (json_is_real(F)) {
        FV = json_real_value(F);
    } else if (json_is_integer(F)) {
        FV = (real_T) json_integer_value(F);
    } else {
      mexPrintf("Bad type for JSON value\n");
    }

    return FV;
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
    ssSetNumSFcnParams(S, EP_NumParams);  /* Number of expected parameters */
    if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
        /* Return if number of expected != number of actual parameters */
        return;
    }

    ssSetNumContStates(S, 0);
    ssSetNumDiscStates(S, 0);

    if (!ssSetNumInputPorts(S, 1)) return;
    ssSetInputPortDataType(S, 0, SS_INT8);
    ssSetInputPortWidth(S, 0, P_JSON_LEN);
    ssSetInputPortRequiredContiguous(S, 0, true); /*direct input signal access*/
    ssSetInputPortDirectFeedThrough(S, 0, 1);

    if (!ssSetNumOutputPorts(S, 5)) return;
    ssSetOutputPortWidth(S, 0, 1);
    ssSetOutputPortWidth(S, 1, 1);
    ssSetOutputPortWidth(S, 2, 1);
    ssSetOutputPortWidth(S, 3, 1);
    ssSetOutputPortWidth(S, 4, 1);

    ssSetNumSampleTimes(S, 1);
    ssSetNumRWork(S, 0);
    ssSetNumIWork(S, 0);
    ssSetNumPWork(S, 0);
    ssSetNumModes(S, 0);
    ssSetNumNonsampledZCs(S, 0);

    /* Specify the sim state compliance to be same as a built-in block */
    ssSetSimStateCompliance(S, USE_DEFAULT_SIM_STATE);

    ssSetOptions(S, 0);
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


/* Function: mdlOutputs =======================================================
 * Abstract:
 *    In this function, you compute the outputs of your S-function
 *    block.
 */
static void mdlOutputs(SimStruct *S, int_T tid)
{
  json_t *root;
  json_error_t error;

  const int8_T *u = (const int8_T*) ssGetInputPortSignal(S,0);
  real_T       *TS = ssGetOutputPortSignal(S,0);
  real_T       *millis = ssGetOutputPortSignal(S,1);
  real_T       *flow = ssGetOutputPortSignal(S,2);
  real_T       *pressure = ssGetOutputPortSignal(S,3);
  real_T       *current = ssGetOutputPortSignal(S,4);


  root = json_loads(u, 0, &error);
  if (!root) {
    sprintf(errstr, "Error: on line %d: %s\n", error.line, error.text);
    // ssSetErrorStatus(S, errstr);
    mexPrintf(errstr);
    // mexPrintf("## %s\n", (char*) u);
    return;
  }

  if (!json_is_object(root)) {
    mexPrintf("The toplevel element should be an object in our case\n");
    goto jsondecode_exit;
  }

    TS[0] = getRealFromJSONField("TS", root);
    millis[0] = getRealFromJSONField("M", root);
    flow[0] = getRealFromJSONField("Flow", root);
    pressure[0] = getRealFromJSONField("Pressure", root);
    current[0] = getRealFromJSONField("Current", root);

    // if (flow[0] < 5.0) {
    //   mexPrintf("The string: %s\n\tvalue: %.5g\n", (char*)u, flow[0]);
    // }





jsondecode_exit:
  json_decref(root);
  // y[0] = u[0];
  
}

static void mdlTerminate(SimStruct *S)
{
}


/*=============================*
 * Required S-function trailer *
 *=============================*/

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif
