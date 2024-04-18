
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

LOCAL_SRC_FILES := \
@LOCAL_SRC_FILES

LOCAL_HEADER_LIBRARIES := libva_headers

LOCAL_SHARED_LIBRARIES := \
    libsync \
    libcutils \
    libutils \
    libdrm \
    libva \
    liblog \
    libgmm_umd \

LOCAL_CPPFLAGS = \
    -DDRV_I915 \
    -DOTC_GRALLOC \
    -fexceptions \
    -frtti \
    -std=c++14 \
    -Wno-error \
    -Wno-ignored-qualifiers \
    -Wno-unused-parameter \
    -Wno-missing-braces \
    -Wno-overloaded-virtual \
    -Wno-unused-variable \
    -Wno-missing-field-initializers \
    -Wno-unused-function \
    -Wno-logical-op-parentheses \
    -Wno-implicit-fallthrough \
    -Wno-comment \
    -Wno-unused-private-field \
    -Wno-unused-value \
    -Wno-parentheses-equality \
    -Wno-unused-label \
    -Wno-parentheses \
    -Wno-c++11-narrowing \
    -Wno-unused-lambda-capture \
    -Wno-unreachable-code-loop-increment \
    -Wno-delete-incomplete \
    -DGMM_LIB_DLL \
@LOCAL_CFLAGS

LOCAL_CONLYFLAGS = -x c++
LOCAL_CFLAGS = $(LOCAL_CPPFLAGS)

LOCAL_C_INCLUDES  = \
@LOCAL_C_INCLUDES
    $(LOCAL_PATH)/../cmrtlib/linux/hardware \

#LOCAL_CPP_FEATURES := rtti exceptions

LOCAL_MODULE := i965_drv_video
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)