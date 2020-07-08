# MATLAB Interface *for Apache Kafka*

MATLAB® interface for Apache Kafka®. This package provides Kafka clients for
MATLAB, Simulink and Embedded Coder.

Kafka® is used for building real-time data pipelines and streaming apps.
It is horizontally scalable, fault-tolerant, fast and widely used.

## Requirements

Requires MATLAB release R2018a or later. Also requires Simulink when using the Kafka Client blocks.
The package is currently only supported for Windows and Linux. The Embedded Coder solution is only supported
on Linux. 

### MathWorks Products (http://www.mathworks.com)

* MATLAB
* [Optional] Simulink
* [Optional] MATLAB Coder
* [Optional] Simulink Coder
* [Optional] Embedded Coder

### 3rd Party Products

For building the mex functions and S-functions.
* A C/C++ compiler for your platform, in accordance with
[MATLAB supported compilers](https://www.mathworks.com/support/requirements/supported-compilers.html).
* [librdkafka](https://github.com/edenhill/librdkafka), version 1.0 or higher
* Some other libraries, as declared in the installation of librdkafka.
* [Optional] [jansson](http://www.digip.org/jansson/) JSON library, if you're using the Simulink JSON-Converter.

## Introduction

[Apache Kafka®](http://kafka.apache.org/)  is a community distributed event streaming platform capable of handling trillions of events a day. Initially conceived as a messaging queue, Kafka is based on an abstraction of a distributed commit log. Since being created and open sourced by LinkedIn in 2011, Kafka has quickly evolved from messaging queue to a full-fledged event streaming platform.

This project contains C/C++-based Kafka Clients, a producer for MATLAB, and producers and consumers for Simulink. In Simulink,
code generation with Embedded Coder is also supported.

The usage in MATLAB is for prototyping, and if you want to use it in a production system, we recommend compiling your code for *MATLAB Production Server*.

For usage in Simulink, this is also prototyping. Generate C code using Embedded Coder to use in a production environment.

## Installation

First install *librdkafka*, and optionally *jansson*, see [the documentation](Documentation/Installation.md).

To install the MATLAB part, just do
```matlab
cd Software/MATLAB
startup
```
This will add the needed paths. If you want them to be saved for future sessions, run
```matlab
savepath
```

You will need to compile the Mex function and optionally the S-functions. This is done with the two commands:
```matlab
kafka_build_mex
kafka_build_sfuns
```

If you intend to generate dockerfiles too, you need to build the base images before you can build from these dockerfiles. To build the base images, run
```matlab
kafka_build_dockerfiles
```

For help refer to [the documentation](Documentation/Installation.md).

## Usage

### MATLAB Kafka Producer

Using the Kafka producer is straightforward.

```matlab
P = kafka.Producer('<mybroker>', '<mytopic>');
P.public('mykey', 'my message');
```
The same producer can, and should be used for sending several
messages to the same topic.

The key and message arguments should be in a form that can automatically be converted to ```int8```. If you have a structure,
you should first convert it to JSON.
```matlab
>> S = struct('item', '3422', 'value', [3,4,5])
S =
  struct with fields:

     item: '3422'
    value: [3 4 5]
>> jsstr = jsonencode(S)
jsstr =
    '{"item":"3422","value":[3,4,5]}'
```

Please see the [documentation](Documentation/README.md) for more information.

### Simulink blocks
The current version contains 3 blocks for Kafka communication. A consumer block, a producer block,
and a very simple block to convert flat JSON structures.

The Kafka functionality works both for simulation and code generation.

Read more in the [corresponding documentation ](Documentation/BasicUsage.md#simulink-clients)

### Embedded Coder
This repository also contains an Embedded Coder target for use with Kafka. It will generate code
from the Simulink model and the Kafka blocks. This code can also be dockerized,
with the use of a Dockerfile that is generated with the code.

Read more in the [corresponding documentation](Documentation/BasicUsage.md#embedded-coder-target)


## Documentation
See [documentation](Documentation/README.md) for more information.


## License
The license for the MATLAB Interface *for Apache Kafka* is available in the [LICENSE.md](LICENSE.md) file in this GitHub repository.
This package uses certain third-party content which is licensed under separate license agreements.
See the 3rd party packages for the respective license details.

## Enhancement Request
Provide suggestions for additional features or capabilities using the following link:   
https://www.mathworks.com/products/reference-architectures/request-new-reference-architectures.html

## Support
Email: `mwlab@mathworks.com`
