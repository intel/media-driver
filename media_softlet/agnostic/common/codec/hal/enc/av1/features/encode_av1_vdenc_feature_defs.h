/*
* Copyright (c) 2020 - 2024, Intel Corporation
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
//! \file     encode_av1_vdenc_feature_defs.h
//! \brief    Defines for av1 coding features
//!
#ifndef __ENCODE_AV1_VDENC_FEATURE_DEFS_H__
#define __ENCODE_AV1_VDENC_FEATURE_DEFS_H__

#include "media_feature_manager.h"

namespace encode
{
struct Av1FeatureIDs : public FeatureIDs
{
    enum av1FeatureIDs
    {
        av1SuperRes   = basicFeature - 1,
        av1CqpFeature = CONSTRUCTFEATUREID(FEATURE_COMPONENT_ENCODE, FEATURE_SUBCOMPONENT_AV1, 0),
        av1Segmentation,
        av1BrcFeature,
        av1LplaEncFeature,
        av1FullEncFeature,
        av1Scc,
        av1Aqm,
        av1FastPass,
        av1LookupTableRounding,
#if _MEDIA_RESERVED
#define AV1_FEATURE_IDS_EXT
#include "encode_av1_vdenc_feature_defs_ext.h"
#undef AV1_FEATURE_IDS_EXT
#endif
    };
};


}
#endif // !__ENCODE_AV1_VDENC_FEATURE_DEFS_H__
