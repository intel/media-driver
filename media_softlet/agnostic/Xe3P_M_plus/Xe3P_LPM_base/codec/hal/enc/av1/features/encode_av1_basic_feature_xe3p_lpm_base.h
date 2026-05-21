/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_av1_basic_feature_xe3p_lpm_base.h
//! \brief    Defines the Xe3P_LPM_Base common class for encode av1 basic feature
//!
#ifndef __ENCODE_AV1_BASIC_FEATURE_XE3P_LPM_BASE_H__
#define __ENCODE_AV1_BASIC_FEATURE_XE3P_LPM_BASE_H__

#include "encode_av1_basic_feature.h"
#include "encode_huc.h"

namespace encode
{
class Av1BasicFeatureXe3P_Lpm_Base : public Av1BasicFeature
{
public:
    Av1BasicFeatureXe3P_Lpm_Base(MediaFeatureManager *featureManager,
                     EncodeAllocator                *allocator,
                     CodechalHwInterfaceNext        *hwInterface,
                     TrackedBuffer                  *trackedBuf,
                     RecycleResource                *recycleBuf,
                     void                           *constSettings) :
                     Av1BasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, constSettings){ m_featureManager = featureManager; m_hwInterface = hwInterface;};

    virtual ~Av1BasicFeatureXe3P_Lpm_Base();

    virtual MOS_STATUS UpdateFormat(void *params) override;

    virtual MOS_STATUS Update(void *params) override;

    virtual MOS_STATUS Init(void *setting) override;

    MOS_RESOURCE* GetVdencReadBatchBufferOrigin(uint32_t recycledBufIdx, uint32_t brcPass);
    MOS_RESOURCE* GetVdencReadBatchBufferTU7(uint32_t recycledBufIdx, uint32_t brcPass);

    //!
    //! \brief    Get VDEnc 2nd level batch buffer
    //! \details  Returns pointer to the VDEnc 2nd level batch buffer for SLBB update output
    //! \param    [in] recycledBufIdx
    //!           Recycled buffer index
    //! \return   MHW_BATCH_BUFFER*
    //!           Pointer to the VDEnc 2nd level batch buffer
    //!
    MHW_BATCH_BUFFER* GetVdenc2ndLevelBatchBuffer(uint32_t recycledBufIdx);
    MHW_BATCH_BUFFER* GetVdenc2ndLevelBatchBufferTU7(uint32_t recycledBufIdx);

    virtual MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

    virtual MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    virtual MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    virtual MHW_SETPAR_DECL_HDR(AVP_INLOOP_FILTER_STATE);

protected:
    MOS_STATUS AllocateVdencBatchBuffers();

    CodechalHwInterfaceNext *m_hwInterface = nullptr;

    MOS_RESOURCE m_vdencReadBatchBufferOrigin[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {};
    MOS_RESOURCE m_vdencReadBatchBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {};
    //!< VDEnc 2nd level batch buffer for SLBB update output
    MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};
    //!< VDEnc 2nd level batch buffer for TU7 SLBB update output
    MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};

MEDIA_CLASS_DEFINE_END(encode__Av1BasicFeatureXe3P_Lpm_Base)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_BASIC_FEATURE_XE3P_LPM_BASE_H__
