# Intel(R) C for Media Runtime


## Introduction

cmrtlib is a runtime library needed when user wants to execute their own GPU kernels on render engine. It calls iHD media driver to load the kernels and allocate the resources. It provides a set of APIs for user to call directly from application.

## License

The Intel(R) C for Media Runtime is distributed under the MIT license.
You may obtain a copy of the License at:

https://opensource.org/licenses/MIT

## Building and Install

cmrtlib is recommended to be built and installed together with media driver. Using the CMakeLists.txt in the root path will involve the building and installation of cmrtlib by default. Please refer to the readme in the root path. Besides, cmrtlib/CMakeLists.txt can also be used as an entry to build and install.
For API usage, please refer to the cm_rt*.h headers installed.
For application that uses this library, please refer to igfxcmrt.pc installed for the compiling and linking flags.

## Backward compatibility

cmrtlib maintains the backward compatibility in both API level and ABI level

## Notes

This cmrtlib library is a separate effort from a similar library (https://github.com/intel/cmrt). They may provide the same functionalities but this is not intended to replace the other. This cmrtlib works and is maintained to support the following platforms: https://github.com/intel/media-driver#supported-platforms.
