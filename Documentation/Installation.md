# Installation

This package has only been tested on Linux and Windows. No support for OSX is present, as of now,
but may well be possible.

## External dependencies

The *matlab-kafka-producer* package has two external dependencies:

* [librdkafka](https://github.com/edenhill/librdkafka), and
* [Optional] [jansson](http://www.digip.org/jansson/)

These are part of the repository, as submodules, referring to the repositories above.

To build them, follow these steps.

### librdkafka

#### Linux
1. `cd` into the folder `Software/CPP/librdkafka` and then

2. Configure and build the package
```bash
./configure
make
```

3. Copy the file `librdkafka.so` from `Software/CPP/librdkafka/src`
to `Software/MATLAB/app/sfun`, and
copy the file `librdkafka.a` from `Software/CPP/librdkafka/src`
to `Software/MATLAB/app/sfun/lib`.

#### Windows
To run this installation, an installation of [CMake](https://cmake.org/) will be necessary.

1. `cd` into the folder `Software\CPP\librdkafka`

2. Create a Visual Studio solution for the Software:
```none
mkdir build.win64
cd build.win64
cmake -LH -G "Visual Studio 15 2017 Win64" ..
```
If using a different version of *Visual Studio*, the argument will look different.
Run
```none
cmake -G
```
to see what generators are available, and map the correct name to the version
installed on the system.

3. Build the solution. This can be done either by
    1. opening the project with Visual Studio and then building it from there
    2. Running the build from the command line, using `msbuild`
    ```none
    msbuild RdKafka.sln /p:Configuration=Release
    ```
    A suitable environment for `msbuild` can easily be opened from the Visual Studio Folder in the
    Windows Applications menu by clicking something like *Developer Command Prompt for VS 2017*.

4. Copy the file `rdkafka.dll` from the folder
`Software\CPP\librdkafka\build.win64\src\Release` into
`Software\MATLAB\app\sfun`, and
copy the file `rdkafka.lib` from the folder
`Software\CPP\librdkafka\build.win64\src\Release` into
`Software\MATLAB\app\sfun\lib`

### Jansson

#### Linux

1. `cd` into the folder `Software/CPP/librdkafka` and then

2. Configure and build the package
```bash
./configure
make
```

3. Copy the file `libjansson.so` from `Software/CPP/jansson/src/.libs`
to `Software/MATLAB/app/sfun`, and
copy the file `libjansson.a` from `Software/CPP/jansson/src/.libs`
to `Software/MATLAB/app/sfun/lib/lib`.

#### Windows

To run this installation, an installation of [CMake](https://cmake.org/) will be necessary.

1. `cd` into the folder `Software\CPP\jansson`

2. Create a Visual Studio solution for the Software:
```none
mkdir build.win64
cd build.win64
cmake -LH -G "Visual Studio 15 2017 Win64" ..
```
If using a different version of *Visual Studio*, the argument will look different.
Run
```none
cmake -G
```
to see what generators are available, and map the correct name to the version
installed on the system.

3. Build the solution. This can be done either by
    1. opening the project with Visual Studio and then building it from there
    2. Running the build from the command line, using `msbuild`
    ```none
    msbuild jansson.sln /p:Configuration=Release
    ```
    A suitable environment for `msbuild` can easily be opened from the Visual Studio Folder in the
    Windows Applications menu by clicking something like *Developer Command Prompt for VS 2017*.

4. Copy the file `jansson.lib` from the folder
`Software\CPP\jansson\build.win64\lib\Release` into
`Software\MATLAB\app\sfun\lib`.

## MATLAB libraries

To install the MATLAB part, just do
```matlab
cd Software/MATLAB
startup
```
this will add the needed paths. If you want them to be saved for future sessions, run the following command in MATLAB.
```matlab
savepath
```

You will need to compile the Mex function and optionally the S-functions. This is done with the two commands:
```matlab
kafka_build_mex
kafka_build_sfuns
```

The first command is for the MATLAB clients, and the second one is for the Simulink/Embedded Coder clients.

To build the base docker images needed for Embedded Coder/Docker integration, run this command in MATLAB:
```matlab
kafka_build_dockerimages
```

If there are any problems with linking or include files, please take a look at the file
[kafka.utils.getMexLibArgs](/Software/MATLAB/system/+kafka/+utils/getMexLibArgs.m), and adapt file paths if needed.
