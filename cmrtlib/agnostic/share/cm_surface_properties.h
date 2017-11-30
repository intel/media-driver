/*
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_SURFACE_PROPERTIES_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_SURFACE_PROPERTIES_H_

#include "cm_include.h"

enum CM_ROTATION {
  CM_ROTATION_IDENTITY = 0,      //!< Rotation 0 degrees
  CM_ROTATION_90,                //!< Rotation 90 degrees
  CM_ROTATION_180,               //!< Rotation 180 degrees
  CM_ROTATION_270,               //!< Rotation 270 degrees
};

#define    CM_CHROMA_SITING_NONE           0
#define    CM_CHROMA_SITING_HORZ_LEFT      1 << 0
#define    CM_CHROMA_SITING_HORZ_CENTER    1 << 1
#define    CM_CHROMA_SITING_HORZ_RIGHT     1 << 2
#define    CM_CHROMA_SITING_VERT_TOP       1 << 4
#define    CM_CHROMA_SITING_VERT_CENTER    1 << 5
#define    CM_CHROMA_SITING_VERT_BOTTOM    1 << 6

// to support new flag with current API
// new flag/field could be added to the end of this structure
struct CM_FLAG {
  CM_RT_API CM_FLAG(): rotationFlag(CM_ROTATION_IDENTITY),
                       chromaSiting(0) {};

  CM_ROTATION rotationFlag;
  int32_t chromaSiting;
};

typedef enum _CM_SAMPLER8x8_SURFACE_ {
  CM_AVS_SURFACE = 0,
  CM_VA_SURFACE = 1
}CM_SAMPLER8x8_SURFACE;

typedef enum _CM_SURFACE_ADDRESS_CONTROL_MODE_ {
  CM_SURFACE_CLAMP = 0,
  CM_SURFACE_MIRROR = 1
} CM_SURFACE_ADDRESS_CONTROL_MODE;

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_SURFACE_PROPERTIES_H_
