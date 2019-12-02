#!/bin/sh

# Please update the paths below in accordance with your setup
# if you do not have gcc 7.0+ and libva installed somewhere in
# the system directories like /usr/share

# ACLOCAL and PKG_CONFIG should point to the directories with libva and dependent files installed
#export ACLOCAL_PATH="/usr/intel/pkgs/pkgconfig/0.23-64/share/aclocal"
#export PKG_CONFIG_PATH=""

export CC=/usr/intel/bin/gcc
export CXX=/usr/intel/bin/g++
export CFLAGS="-m32 -Wno-register"
export CXXFLAGS="-m32 -Wno-register"


# CPLUS_INCLUDE_PATH and LD_LIBRARY_PATH should include
# correct paths to the custom libva includes location if it is not installed
# in your system directories
#export CPLUS_INCLUDE_PATH=""


