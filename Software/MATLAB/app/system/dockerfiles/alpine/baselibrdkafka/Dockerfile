# Base Dockerfile MathWorks Embedded Coder target for Apache Kafka
# 
#  Run build.sh in same directory
#
#  Copyright 2019 The MathWorks, Inc.

ARG BASEIMAGE
FROM ${BASEIMAGE} AS base-image

RUN apk update

RUN apk add \
    jansson \
    librdkafka \
    zlib

LABEL maintainer="mwlab@mathworks.com"

CMD ["sh"]
