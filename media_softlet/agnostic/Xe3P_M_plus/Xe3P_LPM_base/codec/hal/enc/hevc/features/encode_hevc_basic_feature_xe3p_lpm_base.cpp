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
//! \file     encode_hevc_basic_feature_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for encode hevc Xe3p_LPM base parameter
//!

#include "encode_hevc_basic_feature_xe3p_lpm_base.h"

namespace encode
{

HevcBasicFeatureXe3p_Lpm_Base::~HevcBasicFeatureXe3p_Lpm_Base()
{
    ENCODE_FUNC_CALL();
    
    if (m_hwInterface != nullptr)
    {
        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            MOS_STATUS eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_vdenc2ndLevelBatchBuffer[j], nullptr);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to free m_vdenc2ndLevelBatchBuffer[%d], status = 0x%x", j, eStatus);
            }
            eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_vdenc2ndLevelBatchBufferTU7[j], nullptr);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to free m_vdenc2ndLevelBatchBufferTU7[%d], status = 0x%x", j, eStatus);
            }
        }
    }
}

MOS_STATUS HevcBasicFeatureXe3p_Lpm_Base::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);
    
    ENCODE_CHK_STATUS_RETURN(HevcBasicFeature::Init(setting));
    ENCODE_CHK_STATUS_RETURN(AllocateVdencBatchBuffers());
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeatureXe3p_Lpm_Base::AllocateVdencBatchBuffers()
{
    ENCODE_FUNC_CALL();
    
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;
    
    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
        {
            // VDEnc read batch buffer Origin
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer Origin";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            MOS_RESOURCE *allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencReadBatchBufferOrigin[k][i] = *allocatedbuffer;
            
            // VDEnc read batch buffer TU7
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer TU7";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencReadBatchBufferTU7[k][i] = *allocatedbuffer;
        }
    }
    
    // VDEnc 2nd level batch buffer for SLBB update output
    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBuffer[j].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_vdenc2ndLevelBatchBuffer[j],
            nullptr,
            m_hwInterface->m_vdenc2ndLevelBatchBufferSize));

        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBufferTU7[j], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBufferTU7[j].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_vdenc2ndLevelBatchBufferTU7[j],
            nullptr,
            m_hwInterface->m_vdenc2ndLevelBatchBufferSize));
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_RESOURCE* HevcBasicFeatureXe3p_Lpm_Base::GetVdencReadBatchBufferOrigin(uint32_t recycledBufIdx, uint32_t brcPass)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM || brcPass >= VDENC_BRC_NUM_OF_PASSES)
    {
        return nullptr;
    }
    return &m_vdencReadBatchBufferOrigin[recycledBufIdx][brcPass];
}

MOS_RESOURCE* HevcBasicFeatureXe3p_Lpm_Base::GetVdencReadBatchBufferTU7(uint32_t recycledBufIdx, uint32_t brcPass)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM || brcPass >= VDENC_BRC_NUM_OF_PASSES)
    {
        return nullptr;
    }
    return &m_vdencReadBatchBufferTU7[recycledBufIdx][brcPass];
}

MHW_BATCH_BUFFER* HevcBasicFeatureXe3p_Lpm_Base::GetVdenc2ndLevelBatchBuffer(uint32_t recycledBufIdx)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
    {
        return nullptr;
    }
    return &m_vdenc2ndLevelBatchBuffer[recycledBufIdx];
}

MHW_BATCH_BUFFER* HevcBasicFeatureXe3p_Lpm_Base::GetVdenc2ndLevelBatchBufferTU7(uint32_t recycledBufIdx)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
    {
        return nullptr;
    }
    return &m_vdenc2ndLevelBatchBufferTU7[recycledBufIdx];
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcBasicFeatureXe3p_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(HevcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params));

    params.chromaPrefetchDisable = m_chromaPrefetchDisable;

    params.verticalShift32Minus1 = 0;
    params.numVerticalReqMinus1  = 11;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcBasicFeatureXe3p_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(HevcBasicFeature::MHW_SETPAR_F(VDENC_CMD2)(params));

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR ||
        (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR && m_hevcSeqParams->LookaheadDepth > 0))
    {
        params.minQp = m_hevcPicParams->BRCMinQp < CODEC_HEVC_MIN_QP1 ? CODEC_HEVC_MIN_QP1 : m_hevcPicParams->BRCMinQp;
    }

#if !(_MEDIA_RESERVED)
    const bool ibcIFrame = m_hevcPicParams->pps_curr_pic_ref_enabled_flag &&
                           (m_hevcPicParams->CodingType == I_TYPE);
    params.extSettings.emplace_back(
        [ibcIFrame](uint32_t *data) {
            data[2]  |= ibcIFrame ? 0x00000002u : 0x00000003u;
            data[5]  |= 0x00c0a000u;
            data[7]  |= 0x00060003u;
            data[16] |= 0x0f000000u;
            data[19] |= 0x98000000u;
            return MOS_STATUS_SUCCESS;
        });
#endif

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
