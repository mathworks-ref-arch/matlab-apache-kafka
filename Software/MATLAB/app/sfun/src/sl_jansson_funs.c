/*  Copyright 2019 The MathWorks, Inc. */


#include "sl_jansson_funs.h"
#include "stdio.h"

real_T getRealFromJSONField(const char *fieldName, json_t *obj)
{
  json_t *F;
  real_T FV = 0.0;
  F = json_object_get(obj, fieldName);
  if (!F) {
    fprintf(stderr, "No such JSON field, %s\n", fieldName);
    return FV;
  }

  if (json_is_real(F)) {
    FV = json_real_value(F);
  } else if (json_is_integer(F)) {
    FV = (real_T) json_integer_value(F);
  } else {
    fprintf(stderr, "Bad type for JSON value\n");
  }

  return FV;
}



