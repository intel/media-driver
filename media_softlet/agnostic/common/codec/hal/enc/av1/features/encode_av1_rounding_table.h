/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_av1_rounding_table.h
//! \brief    Defines the common interface for av1 vdenc rounding table
//!

#ifndef __ENCODE_AV1_ROUNDING_TABLE_H__
#define __ENCODE_AV1_ROUNDING_TABLE_H__
#include "encode_basic_feature.h"
#include "encode_pipeline.h"

namespace encode
{
   const int8_t Par65Values[] =
   {
      -21, -21, -21, -21, -21, -21, -21, -21, -25, -25, -25, -25, -25,
      -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25,
      -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25,
      -25, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -9,
      -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9,
      -9, -9, -9, -9, -9, -9, -5, -5, -5, -5, -5, -5, -5,
      -5, -5, -5, -5, -5, -1, -1, -1, -1, -1, -1, -1, -1,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -9, -9, -9,
      -9, -9, -9, -9, -9, -9, -9, -9, -9, -5, -5, -5, -5,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9,
      -9, -9, -9, -5, -5, -5, -5, -5, -5, -1, -1, -1, -1,
      -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17,
      -17, -17, -17, -17, -17, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -9, -9, -9, -9, -9, -9,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -13, -13, -13, -13, -13, -13, -13, -13, -9, -9, -9, -9, -9,
      -9, -5, -5, -5, -5, -5, -5, -1, -1, -1, -1, -1, -1,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17,
      -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -17, -17, -17, -17, -17,
      -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17,
      -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -17, -17, -17,
      -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -17, -17, -17, -17, -17, -17, -17, -17,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21,
      -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -17, -17, -17,
      -17, -17, -17, -17, -17, -17, -17, -17, -17, -13, -13, -13, -13,
      -21, -21, -21, -21, -21, -21, -21, -21, -25, -25, -25, -25, -25,
      -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25,
      -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25,
      -25, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21
   };

class Av1EncodeRoundingTable : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting
{

public:
    Av1EncodeRoundingTable(MediaFeatureManager *featureManager,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    EncodeBasicFeature       *m_basicFeature      = nullptr;  //!< EncodeBasicFeature
    CodechalHwInterfaceNext  *m_hwInterface       = nullptr;  //!< Hw interface as utilities
    MediaFeatureManager      *m_featureManager    = nullptr;  //!< Pointer to feature manager

MEDIA_CLASS_DEFINE_END(encode__Av1EncodeRoundingTable)
};
    
}
#endif // !__ENCODE_AV1_ROUNDING_TABLE_H__
