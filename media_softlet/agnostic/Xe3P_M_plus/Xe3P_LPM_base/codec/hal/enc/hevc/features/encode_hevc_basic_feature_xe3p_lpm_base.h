/*
* Copyright (c) 2025-2026, Intel Corporation
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
//! \file     encode_hevc_basic_feature_xe3p_lpm_base.h
//! \brief    Defines the common interface for encode hevc Xe3p_LPM basic feature
//!
#ifndef __ENCODE_HEVC_BASIC_FEATURE_XE3P_LPM_BASE_H__
#define __ENCODE_HEVC_BASIC_FEATURE_XE3P_LPM_BASE_H__

#include "encode_hevc_basic_feature.h"
#include "encode_allocator.h"
#include "encode_huc.h"

namespace encode
{

class HevcBasicFeatureXe3p_Lpm_Base : public HevcBasicFeature
{
public:
    HevcBasicFeatureXe3p_Lpm_Base(EncodeAllocator *allocator,
        CodechalHwInterfaceNext                   *hwInterface,
        TrackedBuffer                             *trackedBuf,
        RecycleResource                           *recycleBuf,
        void                                      *constSettings = nullptr) : HevcBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, constSettings) { m_hwInterface = hwInterface; }

    virtual ~HevcBasicFeatureXe3p_Lpm_Base();

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

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    //! SLBB layout offsets populated by HEVCHucSLBBUpdatePkt::ConstructBatchBuffer()
    //! and consumed by BRC Update to set DMEM without re-constructing the SLBB.
    uint32_t m_slbbCmd2StartInBytes              = 0;
    uint32_t m_slbbSlbDataSizeInBytes            = 0;
    uint32_t m_slbbHcpSliceStateCmdSize          = 0;
    uint32_t m_slbbHcpWeightOffsetStateCmdSize   = 0;
    uint32_t m_slbbVdencWeightOffsetStateCmdSize = 0;
    uint32_t m_slbbMiBatchBufferEndCmdSize       = 0;
    uint32_t m_slbbAlignSize[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = {0};

protected:
    MOS_STATUS AllocateVdencBatchBuffers();
    CodechalHwInterfaceNext *m_hwInterface = nullptr;

    MOS_RESOURCE m_vdencReadBatchBufferOrigin[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {};
    MOS_RESOURCE m_vdencReadBatchBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {};

    //!< VDEnc 2nd level batch buffer for SLBB update output
    MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};
    //!< VDEnc 2nd level batch buffer for TU7 SLBB update output
    MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};

MEDIA_CLASS_DEFINE_END(encode__HevcBasicFeatureXe3p_Lpm_Base)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_BASIC_FEATURE_XE3P_LPM_BASE_H__
