/*  Copyright 2019 The MathWorks, Inc. */

#include "mx_kafka_utils.h"
#include "mw_kafka_utils.h"

char **getConfArrayFromMX(int confCount, const mxArray *confMX,
                          int topicConfCount, const mxArray *topicConfMX)
{
    char **arr = NULL;
    int N = (confCount + topicConfCount);
    arr = (char **)malloc(N * sizeof(char *));
    int ai = -1;
    if (arr != NULL)
    {
        int i;
        for (i = 0; i < confCount; i++)
        {
            const mxArray *mxS = mxGetCell(confMX, i);
            int nels = mxGetNumberOfElements(mxS);
            char *str = (char *)malloc((nels + 1) * sizeof(char));
            if (str == NULL)
            {
                freeConfArray(arr, ai);
                return NULL;
            }
            else
            {
                ai++;
                arr[ai] = str;
            }
            if (mxGetString(mxS, str, nels + 1))
            {
                freeConfArray(arr, ai);
                return NULL;
            }
        }
        for (i = 0; i < topicConfCount; i++)
        {
            const mxArray *mxS = mxGetCell(topicConfMX, i);
            int nels = mxGetNumberOfElements(mxS);
            char *str = (char *)malloc((nels + 1) * sizeof(char));
            if (str == NULL)
            {
                freeConfArray(arr, ai);
                return NULL;
            }
            else
            {
                ai++;
                arr[ai] = str;
            }
            if (mxGetString(mxS, str, nels + 1))
            {
                freeConfArray(arr, ai);
                return NULL;
            }
        }
    }
    return arr;
}

void freeConfArray(char **arr, int numEls)
{
    int i;
    for (i = 0; i < numEls; i++)
    {
        free((void *)arr[i]);
    }
    free(arr);
}

