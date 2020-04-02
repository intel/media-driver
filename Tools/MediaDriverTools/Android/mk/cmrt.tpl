# Copyright(c) 2018 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files(the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and / or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

LOCAL_PATH:= $(call my-dir)


$(info value of CMRT_BUILD_TYPE is: $(CMRT_BUILD_TYPE))
$(info value of CMRT_ARCH is: $(CMRT_ARCH))

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
@LOCAL_SRC_FILES

LOCAL_C_INCLUDES += \
@LOCAL_C_INCLUDES

LOCAL_CFLAGS += \
    -Wno-non-virtual-dtor \
    -DANDROID=1 \
@LOCAL_CFLAGS

ifeq ($(CMRT_BUILD_TYPE), debug)
    LOCAL_CFLAGS += \
        -D_DEBUG \
        -D__DEBUG \
        -O0
else
    LOCAL_CFLAGS += \
        -fno-strict-aliasing \
        -D_FORTIFY_SOURCE=2
endif


LOCAL_MODULE := libigfxcmrt
LOCAL_PROPRIETARY_MODULE := true

LOCAL_LDLIBS := -lc -lva -lva-android
LOCAL_SHARED_LIBRARIES := libc libdl libcutils liblog libutils libm libva libva-android

include $(BUILD_SHARED_LIBRARY)
