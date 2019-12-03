# Base Dockerfile MathWorks Embedded Coder target for Apache Kafka
# 
#  Run build.sh in same directory
#
#  Copyright 2019 The MathWorks, Inc.

ARG BASEIMAGE
ARG BASEDEVIMAGE
FROM ${BASEDEVIMAGE} as compile-step

COPY src /src/

WORKDIR /src

# Change one instance of bool to boolean_T
RUN sed -i.bak -e 's/\bbool\b/boolean_T/g' ssc_core/ssc_core_c8d83e88.c

RUN make -f pm.mk

ARG BASENAME
ARG TAG
FROM ${BASEIMAGE}

RUN mkdir /pmlibs

COPY --from=compile-step /src/*.a /pmlibs/

ENTRYPOINT [ "sh" ]

