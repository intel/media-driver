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
include $(CLEAR_VARS)

LOCAL_MODULE := libgmm_umd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES = \
@LOCAL_SRC_FILES

LOCAL_CFLAGS = \
@LOCAL_CFLAGS

LOCAL_CPPFLAGS = \
    -std=c++11

LOCAL_C_INCLUDES = \
@LOCAL_C_INCLUDES

LOCAL_EXPORT_C_INCLUDE_DIRS = \
    $(LOCAL_PATH)/Source/GmmLib/inc \
    $(LOCAL_PATH)/Source/inc \
    $(LOCAL_PATH)/Source/inc/common \

include $(BUILD_STATIC_LIBRARY)
