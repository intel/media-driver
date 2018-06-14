#!/bin/bash

cmake ../media-driver \
-DCMAKE_INSTALL_PREFIX=/usr \
-DMEDIA_VERSION="2.0.0" \
-DBS_DIR_GMMLIB=`pwd`/../gmmlib/Source/GmmLib/ \
-DBS_DIR_COMMON=`pwd`/../gmmlib/Source/Common/ \
-DBS_DIR_INC=`pwd`/../gmmlib/Source/inc/ \
-DBS_DIR_MEDIA=`pwd`/../media-driver $@
