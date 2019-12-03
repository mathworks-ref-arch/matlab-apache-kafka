# Base Dockerfile MathWorks Embedded Coder target for Apache Kafka
# 
#  Run build.sh in same directory
#
#  Copyright 2019 The MathWorks, Inc.

ARG BASEIMAGE
FROM ${BASEIMAGE}

RUN apk update

RUN apk add \
    g++ \
    make \
    jansson-dev \
    librdkafka-dev \
    openssl-dev \
    zlib-dev

CMD ["sh"]
