/*  Copyright 2019 The MathWorks, Inc. */

#ifndef _SL_JANSSON_FUNS_H_
#define _SL_JANSSON_FUNS_H_

#include "rtwtypes.h"
#include "jansson.h"

#if defined(__cplusplus)
extern "C" {
#endif

real_T getRealFromJSONField(const char *fieldName, json_t *obj);

#if defined(__cplusplus)
}
#endif

#endif  //_SL_JANSSON_FUNS_H_
