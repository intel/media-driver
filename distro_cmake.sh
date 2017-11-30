#!/bin/bash

# Full_Open_Source_Support flag is to control full-open-source build.
# Full-open-source means only using open source EU kernels and fixed function hardware.

cmake ../media-driver \
-DMEDIA_VERSION="2.0.0" \
-DGEN8_Supported="no" \
-DGEN8_BDW_Supported="no" \
-DGEN9_Supported="no" \
-DGEN9_SKL_Supported="no" \
-DFull_Open_Source_Support=”yes" \
-DBUILD_ALONG_WITH_CMRTLIB=1 \
-DBS_DIR_GMMLIB=`pwd`/../gmmlib/Source/GmmLib/ \
-DBS_DIR_COMMON=`pwd`/../gmmlib/Source/Common/ \
-DBS_DIR_INC=`pwd`/../gmmlib/Source/inc/ \
-DBS_DIR_MEDIA=`pwd`/../media-driver $@
