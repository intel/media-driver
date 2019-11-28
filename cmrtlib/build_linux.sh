#!/bin/bash

# Written by David Stuttard, Intel (david.stuttard@intel.com) 26 June, 2013
# This script will use cmake to build and install the linux cm and cmrt libraries

# Process the input options using built-in getopts
usage(){
    echo "Usage: `basename $0` -b <relative-path> -h for help";
    echo "    -b <relative-path> : Set the build dir to something other than the default (build.linux)";
    echo "    -h                 : Help (print out this usage)";
    echo "    -a                 : build jitter shared library and GenX_IR";
    echo "    -d                 : Debug build (default build.linux.debug)";
    echo "    -e                 : Enable EMU build";
    echo "    --32               : Build 32 bit variant (default on 32 bit systems)";
    echo "    --64               : Build 64 bit variant (default on 64 bit systems)";
}

function define_colours_on() {
    # Reset
    Colour_Off='\033[0m'      # Text Reset

    # Regular Colours
    Black='\033[0;30m'        # Black
    Red='\033[0;31m'          # Red
    Green='\033[0;32m'        # Green
    Yellow='\033[0;33m'       # Yellow
    Blue='\033[0;34m'         # Blue
    Purple='\033[0;35m'       # Purple
    Cyan='\033[0;36m'         # Cyan
    White='\033[0;37m'        # White
    
    # Bold
    BBlack='\033[1;30m'       # Black
    BRed='\033[1;31m'         # Red
    BGreen='\033[1;32m'       # Green
    BYellow='\033[1;33m'      # Yellow
    BBlue='\033[1;34m'        # Blue
    BPurple='\033[1;35m'      # Purple
    BCyan='\033[1;36m'        # Cyan
    BWhite='\033[1;37m'       # White
}

function define_colours_off() {
    # Reset
    Colour_Off=''      # Text Reset

    # Regular Colours
    Black=''        # Black
    Red=''          # Red
    Green=''        # Green
    Yellow=''       # Yellow
    Blue=''         # Blue
    Purple=''       # Purple
    Cyan=''         # Cyan
    White=''        # White
    
    # Bold
    BBlack=''       # Black
    BRed=''         # Red
    BGreen=''       # Green
    BYellow=''      # Yellow
    BBlue=''        # Blue
    BPurple=''      # Purple
    BCyan=''        # Cyan
    BWhite=''       # White
}

BUILD_DIR=build.linux
DEBUG=0
BUILD_SET=0
BUILD_32=0
BUILD_64=0
BUILD_EMU=0
CROSS_BUILD=0
BUILD_SIZE=0

# Set up build size
MACHINE_TYPE=`uname -m`
if [ ${MACHINE_TYPE} == 'x86_64' ]; then
    BUILD_SIZE=64
else
    BUILD_SIZE=32
fi

while getopts "b:edah-:" opt; do
    case $opt in
        -)
            case "${OPTARG}" in
                32)
                    BUILD_32=1
                    ;;
                64)
                    BUILD_64=1
                    ;;
            esac;;
        b) 
            BUILD_DIR=$OPTARG
            BUILD_SET=1
            ;;
        e)
            BUILD_EMU=1
            ;;
        d)
            DEBUG=1
            ;;
        a)
            export BUILD_ALL=1
            ;;            
        h) 
            usage
            exit 1
            ;;
        \?)
            echo "Unknown option"
            usage
            exit 1
            ;;
    esac
done

if [[ $BUILD_32 -eq 1 ]]; then
    if [[ $BUILD_64 -eq 1 ]]; then
        echo "Can't specify 32 *and* 64 bit at the same time"
        usage
        exit 1
    fi
fi

EXTRA_OPTIONS=
EXTRA_CMAKE_FLAGS=

case $BUILD_SIZE in
    32)
        if [[ $BUILD_64 -eq 1 ]]; then
            CROSS_BUILD=1
            BUILD_SIZE=64
            EXTRA_OPTIONS="export CFLAGS=-m64 CXXFLAGS=-m64"
        fi
        ;;
    64)
        if [[ $BUILD_32 -eq 1 ]]; then
            CROSS_BUILD=1
            BUILD_SIZE=32
            EXTRA_OPTIONS="export CFLAGS=-m32 CXXFLAGS=-m32"
        fi
        ;;
esac

if [[ $BUILD_SET -eq 0 ]]; then
    if [[ $DEBUG -eq 1 ]]; then
        BUILD_DIR=build.linux.debug
    fi
    # Add .x. if a cross compile build
    if [[ $CROSS_BUILD -eq 1 ]]; then
        BUILD_DIR="$BUILD_DIR.x"
    fi
    # Append build size to directory name
    BUILD_DIR="$BUILD_DIR.$BUILD_SIZE"
fi

if [[ $DEBUG -eq 1 ]]; then
    EXTRA_CMAKE_FLAGS="$EXTRA_CMAKE_FLAGS -DCMAKE_BUILD_TYPE=Debug"
else
    EXTRA_CMAKE_FLAGS="$EXTRA_CMAKE_FLAGS -DCMAKE_BUILD_TYPE=Release"
fi

define_colours_on

echo "We're going to do the build in " $BUILD_DIR        

# Do we have cmake installed?
if hash cmake 2>/dev/null; then
    echo "cmake has been detected ... proceeding"
else
    echo "cmake is required to perform a linux build. Please install the package (sudo apt-get install cmake)"
    exit 1
fi

# By default we will create and do the build in build.linux but this can overridden 
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p $BUILD_DIR
fi

CMAKE_ROOT=`pwd`

# The root CMakeLists.txt file is in the same directory as this script - remember this location
cd $CMAKE_ROOT
cd $BUILD_DIR
echo "executing : $EXTRA_OPTIONS cmake" $CMAKE_ROOT
(
if [[ $BUILD_EMU -eq 0 ]]; then
  $EXTRA_OPTIONS
fi
cmake $EXTRA_CMAKE_FLAGS $CMAKE_ROOT
)
if [[ $BUILD_EMU -eq 0 ]]; then
  echo "executing make"
  make
fi
