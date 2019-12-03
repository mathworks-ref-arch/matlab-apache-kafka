/* Some static utils used by both producer and consumer 
 * Use this file via include in your project
 *
 * Copyright 2019 The MathWorks, Inc.
 */



#ifndef _SL_KAFKA_UTILS_C_
#define _SL_KAFKA_UTILS_C_

#include "simstruc.h"

static int getParamString(SimStruct *S, char *strPtr, mxArray *prm, int epwIdx, char *errorHelp) {
    int N = mxGetNumberOfElements(prm);
    strPtr = (char*) malloc(N+1);
    if (strPtr == NULL) {
        sprintf(errstr,"Couldn't allocate string for '%s'\n", errorHelp);
        ssSetErrorStatus(S,errstr);
        return 1;
    }    
    if (mxGetString(prm, strPtr, N+1)) {
        sprintf(errstr,"Couldn't retrieve '%s' string\n", errorHelp);
        ssSetErrorStatus(S,errstr);
        return 2;
    }
    ssSetPWorkValue(S, epwIdx, strPtr);

    return 0;
}



#endif    // _SL_KAFKA_UTILS_C_
