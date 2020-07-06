/*
* Copyright (c) 2020, Intel Corporation
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

//!
//! \file     decode_av1_feature_defs_g12.h
//! \brief    Defines for av1 decoding features
//!
#ifndef __DECODE_AV1_FEATURE_DEFS_G12_H__
#define __DECODE_AV1_FEATURE_DEFS_G12_H__

#include "media_feature_manager.h"

namespace decode
{
struct Av1FeatureIDs : public FeatureIDs
{
    enum av1FeatureIDs
    {
        av1SwFilmGrain = CONSTRUCTFEATUREID(FEATURE_COMPONENT_DECODE, reserve0, 0),
    };
};
}
#endif // !__DECODE_AV1_FEATURE_DEFS_G12_H__