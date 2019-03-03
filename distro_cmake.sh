#!/bin/bash

cmake ../media-driver \
-DGEN8=OFF \
-DGEN9=OFF \
-DENABLE_NONFREE_KERNELS=OFF $@
