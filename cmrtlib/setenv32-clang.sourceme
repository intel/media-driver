#!/bin/sh

# Please update the paths below in accordance with your setup
# if you do not have clang 8.0+, gcc 9.1+, libva and libc++ installed somewhere in
# the system directories like /usr/share

# ACLOCAL and PKG_CONFIG should point to the directories with libva and dependent files installed
#export ACLOCAL_PATH="/usr/intel/pkgs/pkgconfig/0.23-64/share/aclocal"
#export PKG_CONFIG_PATH=""

export CC=/usr/intel/bin/clang
export CXX=/usr/intel/bin/clang++
export CFLAGS="-m32 -fcoroutines-ts -stdlib=libc++ -Wno-register"
export CXXFLAGS="-m32 -fcoroutines-ts -stdlib=libc++ -Wno-register"

# LDFLAGS should include -Wl,-L/path/to/libc++ and -Wl,-L/path/to/libgcc
# if clang's libc++ and libgcc are not installed 
# in the system directories
export LDFLAGS="-lc++abi"

# COMPILER_PATH should include the path to gcc 9.1+ installation directory
# if for some reason clang picks up wrong GCC toolchain path into its
# --gcc-toolchain option
#export COMPILER_PATH=/usr/intel/pkgs/gcc/9.1.0

# LIBRARY_PATH, CPLUS_INCLUDE_PATH and LD_LIBRARY_PATH should include
# correct paths to the custom libc++ location if it is not installed
# in your system directories
#export LIBRARY_PATH=""
#export CPLUS_INCLUDE_PATH=""
#export LD_LIBRARY_PATH=""
