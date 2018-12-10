#!/bin/bash

cmake ../media-driver \
-DGEN8=OFF \
-DGEN9=OFF \
-DFREE_KERNELS=ON $@
