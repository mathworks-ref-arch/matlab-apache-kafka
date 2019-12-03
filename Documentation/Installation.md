# Installation

## External dependencies

For the external dependencies, please refer to the external web-sites found here:

* [librdkafka](https://github.com/edenhill/librdkafka), version 1.0 or higher
* [Optional] [jansson](http://www.digip.org/jansson/) JSON library, if you're using the Simulink JSON-Converter.

### librdkafka

`librdkafka` must have version 1.0 or higher. Use a package manager if possible, and build from source if not.

#### Linux
Get librdkafka from github and checkout a stable version (`>=1.0`), and follow the README file. Basically
```bash
./configure
make
sudo make install
```

#### Windows
Get librdkafka from github and checkout a stable version (`>=1.0`), and follow the README file. Basically
```none
mkdir build.win64
cd build.win64
cmake -LH -G "Visual Studio 15 2017 Win64" ..
cd ..
```
After doing so, the `RdKafka.sln` can be used to build the libraries using the MSVC IDE. To perform the install, it will probably be necessary to start MSVC as Administrator.

For the runtime on Windows, you must add the location of your DLL to your PATH variable, or copy it to the folder where your mex-file was created.

### Jansson

#### Linux
On Ubuntu, Jansson is available through `apt`. 

#### Windows
On Windows, build Jansson from source using the following steps.
Download the sources from the link above, or from github. When using github, make sure to checkout release 2.12.

In the source directory of Jansson:
```none
mkdir build.win64
cd build.win64
cmake -LH -G "Visual Studio 15 2017 Win64" ..
cd ..
```

After doing so, the `jansson.sln` can be used to build the libraries using the MSVC IDE. To perform the install, it will probably be necessary to start MSVC as Administrator.


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


