#!/bin/bash

cmake ../media-driver \
-DGEN8_Supported="no" \
-DGEN8_BDW_Supported="no" \
-DGEN9_Supported="no" \
-DGEN9_BXT_Supported="no" \
-DGEN9_SKL_Supported="no" \
-DGEN9_CFL_Supported="no" \
-DGEN9_GLK_Supported="no" \
-DGEN9_KBL_Supported="no" \
-DFREE_KERNELS=ON $@
