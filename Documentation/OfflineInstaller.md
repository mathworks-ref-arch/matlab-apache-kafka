# Offline installer

## Introduction
The build process for this package requires a few tools, e.g. compilers,
linkers, build tools, etc.

The package is only delivered with sources, so there's no way past doing
the build, at least once.

However, in case a user wants to install the package on several machines,
or if the machine with the build tools is not the same machine where the
package will be used, there is an *Offline Installer*.

This will enable the user to build the package on one machine, and then
zip this together, to make it easier to install on a second machine.

## Create offline installer
In order for the offline installer to be created, one correct installation
must first be created.
Do this by going through the steps in the [Installation](Installation.md)
document.

When this is done, call this function in MATLAB:

```matlab
>> kafka_create_offline_installer;
Elapsed time is 29.129166 seconds.
Created zip file "C:\Some\Path\matlab-apache-kafka_20210406T154142.zip"
```

It will take about half a minute, and will then return the name of the
zip-file that was created.

When this is unzipped on a second computer, please note that it's still
necessary to run the `startup` function in order to set the paths.
If the user wish to keep these on the path, they can simply issue
a `savepath` command in MATLAB, .i.e.

```matlab
cd Software/MATLAB
startup
savepath % Only if the path should be persistent between sessionss
```




