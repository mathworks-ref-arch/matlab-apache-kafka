# Development Dockerfile MathWorks Embedded Coder target for Apache Kafka
# 
#  Run build.sh in same directory
#
#  Copyright 2019 The MathWorks, Inc.

ARG BASEIMAGE
FROM ${BASEIMAGE}

RUN apt-get update \
     && apt-get install -y \
     curl \
     g++ \
     libjansson-dev \ 
     libsasl2-dev \
     libzstd-dev \
     make \
     zlib1g-dev

RUN curl -OL https://github.com/edenhill/librdkafka/archive/v1.0.0.tar.gz

RUN mkdir -p /src/rdkafka && \
    tar xzf v1.0.0.tar.gz -C /src/rdkafka/ && \
    cd /src/rdkafka/librdkafka-1.0.0 && \
    ./configure && \
    make && \
    make install

RUN ldconfig && \
    rm -rf /src

CMD ["bash"]