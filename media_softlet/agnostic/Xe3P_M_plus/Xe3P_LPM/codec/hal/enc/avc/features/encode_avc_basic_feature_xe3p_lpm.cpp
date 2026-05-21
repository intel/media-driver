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
//! \file     encode_avc_basic_feature_xe3p_lpm.cpp
//! \brief    Defines the common interface for encode avc Xe3P_LPM parameter
//!

#include "encode_avc_basic_feature_xe3p_lpm.h"
#include "encode_avc_brc.h"
#include "media_avc_feature_defs.h"
#include "mhw_vdbox_vdenc_hwcmd_xe3p_lpm.h"

namespace encode
{

AvcBasicFeatureXe3P_Lpm::AvcBasicFeatureXe3P_Lpm(EncodeAllocator * allocator,
                       CodechalHwInterfaceNext *hwInterface,
                       TrackedBuffer *trackedBuf,
                       RecycleResource *recycleBuf,
                       MediaCopyWrapper *mediaCopyWrapper,
                       void *constSettings) :
                       AvcBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf, mediaCopyWrapper, constSettings),
                       m_hwInterface(hwInterface) {}

AvcBasicFeatureXe3P_Lpm::~AvcBasicFeatureXe3P_Lpm()
{
    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        if (m_hwInterface && m_hwInterface->GetOsInterface())
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

uint8_t AvcBasicFeatureXe3P_Lpm::GetMinAvcQp()
{
    ENCODE_FUNC_CALL();

    // For VBR rate control, use minimum QP of 5 (CODEC_AVC_MIN_QP5)
    // For other rate control methods, use base class default of 10 (CODEC_AVC_MIN_QP10)
    if (m_seqParam->RateControlMethod == RATECONTROL_VBR)
    {
        return CODEC_AVC_MIN_QP5;  // Returns 5 for VBR
    }

    return AvcBasicFeature::GetMinAvcQp();  // Returns CODEC_AVC_MIN_QP10 (10) from base class
}

void AvcBasicFeatureXe3P_Lpm::UpdateMinMaxQp()
{
    ENCODE_FUNC_CALL();

    if (m_seqParam->RateControlMethod != RATECONTROL_VBR)
    {
        return AvcBasicFeature::UpdateMinMaxQp();
    }

    uint8_t minQp = GetMinAvcQp();  // VBR returns CODEC_AVC_MIN_QP5 (5)

    m_minMaxQpControlEnabled = true;
    if (m_picParam->CodingType == I_TYPE)
    {
        m_iMaxQp = MOS_MIN(MOS_MAX(m_picParam->ucMaximumQP, minQp), CODEC_AVC_MAX_QP);
        m_iMinQp = MOS_MIN(MOS_MAX(m_picParam->ucMinimumQP, minQp), m_iMaxQp);
        if (!m_pFrameMinMaxQpControl)
        {
            m_pMinQp = m_iMinQp;
            m_pMaxQp = m_iMaxQp;
        }
        if (!m_bFrameMinMaxQpControl)
        {
            m_bMinQp = m_iMinQp;
            m_bMaxQp = m_iMaxQp;
        }
    }
    else if (m_picParam->CodingType == P_TYPE)
    {
        m_pFrameMinMaxQpControl = true;
        m_pMaxQp                = MOS_MIN(MOS_MAX(m_picParam->ucMaximumQP, minQp), CODEC_AVC_MAX_QP);
        m_pMinQp                = MOS_MIN(MOS_MAX(m_picParam->ucMinimumQP, minQp), m_pMaxQp);
        if (!m_bFrameMinMaxQpControl)
        {
            m_bMinQp = m_pMinQp;
            m_bMaxQp = m_pMaxQp;
        }
    }
    else  // B_TYPE
    {
        m_bFrameMinMaxQpControl = true;
        m_bMaxQp                = MOS_MIN(MOS_MAX(m_picParam->ucMaximumQP, minQp), CODEC_AVC_MAX_QP);
        m_bMinQp                = MOS_MIN(MOS_MAX(m_picParam->ucMinimumQP, minQp), m_bMaxQp);
    }
    // Zero out the QP values, so we don't update the AVCState settings until new values are sent in MiscParamsRC
    m_picParam->ucMinimumQP = 0;
    m_picParam->ucMaximumQP = 0;
}

uint32_t AvcBasicFeatureXe3P_Lpm::GetVdencOneSliceStateSize()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    
    auto mfxItf = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
    auto vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    auto miItf = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
    
    ENCODE_CHK_NULL_RETURN(mfxItf);
    ENCODE_CHK_NULL_RETURN(vdencItf);
    ENCODE_CHK_NULL_RETURN(miItf);
    
    return mfxItf->MHW_GETSIZE_F(MFX_AVC_SLICE_STATE)() +
           vdencItf->MHW_GETSIZE_F(VDENC_AVC_SLICE_STATE)() +
           miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
}

MHW_BATCH_BUFFER* AvcBasicFeatureXe3P_Lpm::GetVdenc2ndLevelBatchBuffer(uint32_t recycledBufIdx)
{
    ENCODE_FUNC_CALL();

    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
    {
        return nullptr;
    }
    return &m_vdenc2ndLevelBatchBuffer[recycledBufIdx];
}

MHW_BATCH_BUFFER* AvcBasicFeatureXe3P_Lpm::GetVdenc2ndLevelBatchBufferTU7(uint32_t recycledBufIdx)
{
    ENCODE_FUNC_CALL();

    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
    {
        return nullptr;
    }
    return &m_vdenc2ndLevelBatchBufferTU7[recycledBufIdx];
}

MOS_STATUS AvcBasicFeatureXe3P_Lpm::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    ENCODE_CHK_STATUS_RETURN(AvcBasicFeature::Init(setting));
    ENCODE_CHK_NULL_RETURN(m_hwInterface);

    // Compute AVC-specific SLBB buffer size from actual AVC command sizes
    auto mfxItf   = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
    auto vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    auto miItf    = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
    ENCODE_CHK_NULL_RETURN(mfxItf);
    ENCODE_CHK_NULL_RETURN(vdencItf);
    ENCODE_CHK_NULL_RETURN(miItf);

    // Group1 (image-level): MFX_AVC_IMG_STATE + VDENC_CMD3 + VDENC_AVC_IMG_STATE + BB_END, cacheline-aligned
    // Group2 (per-slice × max slices): MFX_AVC_SLICE_STATE + VDENC_AVC_SLICE_STATE + BB_END
    uint32_t group1Size = mfxItf->MHW_GETSIZE_F(MFX_AVC_IMG_STATE)()
        + vdencItf->MHW_GETSIZE_F(VDENC_CMD3)()
        + vdencItf->MHW_GETSIZE_F(VDENC_AVC_IMG_STATE)()
        + miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    m_slbbBufferSize = MOS_ALIGN_CEIL(
        MOS_ALIGN_CEIL(group1Size, CODECHAL_CACHELINE_SIZE)
        + CODECHAL_ENCODE_AVC_MAX_SLICES_SUPPORTED
          * (mfxItf->MHW_GETSIZE_F(MFX_AVC_SLICE_STATE)()
             + vdencItf->MHW_GETSIZE_F(VDENC_AVC_SLICE_STATE)()
             + miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)()),
        CODECHAL_PAGE_SIZE);

    // Allocate SLBB resources for xe3p_lpm platform
    PMOS_RESOURCE allocatedbuffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    for (uint32_t k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        // VDENC IMG STATE read buffer - Origin
        allocParamsForBufferLinear.dwBytes      = GetSlbbBufferSize();
        allocParamsForBufferLinear.pBufName     = "VDENC BRC IMG State Read Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocatedbuffer                         = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcImageStatesReadBufferOrigin[k] = allocatedbuffer;

        // VDENC IMG STATE read buffer - TU7
        allocParamsForBufferLinear.dwBytes      = GetSlbbBufferSize();
        allocParamsForBufferLinear.pBufName     = "VDENC BRC IMG State Read Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocatedbuffer                         = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcImageStatesReadBufferTU7[k] = allocatedbuffer;

        // VDEnc 2nd level batch buffer for SLBB update output
        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBuffer[k], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBuffer[k].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_vdenc2ndLevelBatchBuffer[k],
            nullptr,
            m_slbbBufferSize));

        // VDEnc 2nd level batch buffer for TU7 SLBB update output
        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBufferTU7[k], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBufferTU7[k].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_vdenc2ndLevelBatchBufferTU7[k],
            nullptr,
            m_slbbBufferSize));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcBasicFeatureXe3P_Lpm)
{
    AvcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params);

    params.verticalShift32Minus1 = 0;
    params.numVerticalReqMinus1  = 11;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcBasicFeatureXe3P_Lpm)
{
    ENCODE_FUNC_CALL();

    if (m_seqParam->RateControlMethod == RATECONTROL_VBR)
    {
        params.minQp = CODEC_AVC_MIN_QP5;
    }

    AvcBasicFeature::MHW_SETPAR_F(VDENC_AVC_IMG_STATE)(params);

#if !(_MEDIA_RESERVED)
    params.extSettings.emplace_back(
        [](uint32_t *data) {
            data[1]  |= (1u) | (3u << 8);
            data[2]  |= (2u << 14) | (2u << 16);
            data[7]  |= (0xFFu << 16) | (0xFFu << 24);
            data[8]  |= 0x2000u;
            data[12] |= (15u << 24);
            data[14] |= (1u << 21) | (0xFFu << 24);
            return MOS_STATUS_SUCCESS;
        });
#endif

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
