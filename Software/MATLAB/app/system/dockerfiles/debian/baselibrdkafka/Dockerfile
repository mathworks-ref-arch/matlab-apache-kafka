# Base Dockerfile MathWorks Embedded Coder target for Apache Kafka
# 
#  Run build.sh in same directory
#
#  Copyright 2019 The MathWorks, Inc.

ARG BASEIMAGE
FROM ${BASEIMAGE} AS base-image

RUN apt-get update 

RUN apt-get install -y \
   libjansson4 \
   libsasl2-2 \
   libssl-dev \
   libzstd1 \
   zlib1g \
   && apt-get clean \
  	&& apt-get -y autoremove

ARG TAG
LABEL version="$TAG"
LABEL maintainer="mwlab@mathworks.com"

CMD ["bash"]
