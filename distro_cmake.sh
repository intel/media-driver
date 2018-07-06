#!/bin/bash

# Full_Open_Source_Support flag is to control full-open-source build.
# Full-open-source means only using open source EU kernels and fixed function hardware.

cmake ../media-driver \
-DGEN8_Supported="no" \
-DGEN8_BDW_Supported="no" \
-DGEN9_Supported="no" \
-DGEN9_SKL_Supported="no" \
-DFull_Open_Source_Support="yes" $@
