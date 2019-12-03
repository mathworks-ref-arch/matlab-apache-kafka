/*  Copyright 2019 The MathWorks, Inc. */

#ifndef _MX_KAFKA_UTILS_H
#define _MX_KAFKA_UTILS_H

#include "mex.h"

#ifdef __cplusplus
extern "C"
{
#endif
    char **getConfArrayFromMX(int confCount, const mxArray *confMX,
                              int topicConfCount, const mxArray *topicConfMX);
    void freeConfArray(char **arr, int numEls);

#ifdef __cplusplus
}
#endif

#endif /* _MX_KAFKA_UTILS_H */
