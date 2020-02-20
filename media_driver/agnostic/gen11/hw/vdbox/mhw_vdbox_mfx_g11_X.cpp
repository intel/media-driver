/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mhw_vdbox_mfx_g11_X.cpp
//! \brief    Constructs VdBox MFX commands on Gen11-based platforms

#include "mhw_vdbox_mfx_g11_X.h"
#include "mhw_mi_hwcmd_g11_X.h"
#include "mhw_mmio_g11.h"

void MhwVdboxMfxInterfaceG11::InitMmioRegisters()
{
    MmioRegistersMfx *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

    mmioRegisters->generalPurposeRegister0LoOffset            = GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister0HiOffset            = GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister4LoOffset            = GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister4HiOffset            = GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister11LoOffset           = GENERAL_PURPOSE_REGISTER11_LO_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister11HiOffset           = GENERAL_PURPOSE_REGISTER11_HI_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister12LoOffset           = GENERAL_PURPOSE_REGISTER12_LO_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->generalPurposeRegister12HiOffset           = GENERAL_PURPOSE_REGISTER12_HI_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcImageStatusMaskRegOffset                = MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcImageStatusCtrlRegOffset                = MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcAvcNumSlicesRegOffset                   = MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcQPStatusCountOffset                     = MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxErrorFlagsRegOffset                     = MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxFrameCrcRegOffset                       = MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxMBCountRegOffset                        = MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcBitstreamBytecountFrameRegOffset        = MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset       = MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcBitstreamBytecountSliceRegOffset        = MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcVP8BitstreamBytecountFrameRegOffset     = MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcVP8ImageStatusMaskRegOffset             = MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfcVP8ImageStatusCtrlRegOffset             = MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcDQIndexRegOffset                  = MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcDLoopFilterRegOffset              = MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset      = MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcCumulativeDQIndex23RegOffset      = MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcCumulativeDLoopFilter01RegOffset  = MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcCumulativeDLoopFilter23RegOffset  = MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxVP8BrcConvergenceStatusRegOffset        = MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxLra0RegOffset                           = MFX_LRA0_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxLra1RegOffset                           = MFX_LRA1_REG_OFFSET_NODE_1_INIT_G11;
    mmioRegisters->mfxLra2RegOffset                           = MFX_LRA2_REG_OFFSET_NODE_1_INIT_G11;

    m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
}

void MhwVdboxMfxInterfaceG11::InitRowstoreUserFeatureSettings()
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    if (MEDIA_IS_SKU(m_skuTable, FtrSimulationMode))
    {
        // Disable RowStore Cache on simulation by default
        userFeatureData.u32Data = 1;
    }
    else
    {
        userFeatureData.u32Data = 0;
    }

    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
        &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
    m_rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

    if (m_rowstoreCachingSupported)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_INTRAROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_intraRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_DEBLOCKINGFILTERROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_deblockingFilterRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_BSDMPCROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_bsdMpcRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MPRROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_mprRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;
    }
}

MOS_STATUS MhwVdboxMfxInterfaceG11::GetRowstoreCachingAddrs(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(rowstoreParams);

    if (m_intraRowstoreCache.bSupported)
    {
        m_intraRowstoreCache.bEnabled = true;
        if (!rowstoreParams->bMbaff)
        {
            if (rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_2K)
            {
                m_intraRowstoreCache.dwAddress = INTRAROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_LESS_THAN_2K;
            }
            else if (rowstoreParams->dwPicWidth >= MHW_VDBOX_PICWIDTH_2K && rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_3K)
            {
                m_intraRowstoreCache.dwAddress = INTRAROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_3K;
            }
            else if (rowstoreParams->dwPicWidth >= MHW_VDBOX_PICWIDTH_3K &&
                     rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_4K &&
                    (rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP8VLD ||
                     rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC))
            {
                m_intraRowstoreCache.dwAddress = INTRAROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_BETWEEN_3K_AND_4K;
            }
            else
            {
                m_intraRowstoreCache.dwAddress = 0;
                m_intraRowstoreCache.bEnabled = false;
            }
        }
        else     //mbaff
        {
            if (rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_2K)
            {
                m_intraRowstoreCache.dwAddress = INTRAROWSTORE_MBAFF_BASEADDRESS_PICWIDTH_LESS_THAN_2K;
            }
            else
            {
                m_intraRowstoreCache.dwAddress = 0;
                m_intraRowstoreCache.bEnabled = false;
            }
        }
    }

    if (m_deblockingFilterRowstoreCache.bSupported)
    {
        m_deblockingFilterRowstoreCache.bEnabled = true;
        if (!rowstoreParams->bMbaff && rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_2K)
        {
            m_deblockingFilterRowstoreCache.dwAddress = DEBLOCKINGROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_LESS_THAN_2K;
        }
        else
        {
            m_deblockingFilterRowstoreCache.dwAddress = 0;
            m_deblockingFilterRowstoreCache.bEnabled = false;
        }
    }

    if (m_bsdMpcRowstoreCache.bSupported && rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_4K)         // mbaff and non mbaff mode for all resolutions
    {
        m_bsdMpcRowstoreCache.bEnabled = true;
        m_bsdMpcRowstoreCache.dwAddress = BSDMPCROWSTORE_BASEADDRESS;
    }
    else
    {
        m_bsdMpcRowstoreCache.dwAddress = 0;
        m_bsdMpcRowstoreCache.bEnabled = false;
    }

    if (m_mprRowstoreCache.bSupported && rowstoreParams->Mode == CODECHAL_DECODE_MODE_AVCVLD)
    {
        m_mprRowstoreCache.bEnabled = true;
        if (!rowstoreParams->bMbaff)
        {
            if (rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_2K)
            {
                m_mprRowstoreCache.dwAddress = MPRROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_LESS_THAN_2K;
            }
            else if (rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_2K && rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_3K)
            {
                m_mprRowstoreCache.dwAddress = MPRROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_3K;
            }
            else
            {
                m_mprRowstoreCache.dwAddress = MPRROWSTORE_FRAME_FIELD_BASEADDRESS_PICWIDTH_GREATER_THAN_3K;
            }
        }
        else  //Mbaff 
        {
            if (rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_2K)
            {
                m_mprRowstoreCache.dwAddress = MPRROWSTORE_MBAFF_BASEADDRESS_PICWIDTH_LESS_THAN_2K;
            }
            else
            {
                m_mprRowstoreCache.dwAddress = 0;
                m_mprRowstoreCache.bEnabled = false;
            }
        }
    }

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS MhwVdboxMfxInterfaceG11::CheckScalabilityOverrideValidity()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MEDIA_SYSTEM_INFO       *gtSystemInfo;
    uint32_t                forceVdbox;
    bool                    scalableDecMode;
    bool                    useVD1, useVD2;

    MHW_MI_CHK_NULL(m_osInterface);
    scalableDecMode = m_osInterface->bHcpDecScalabilityMode ? true : false;
    forceVdbox = m_osInterface->eForceVdbox;
    gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    MHW_MI_CHK_NULL(gtSystemInfo);

    if (forceVdbox != MOS_FORCE_VDBOX_NONE    &&
        forceVdbox != MOS_FORCE_VDBOX_1       &&
        forceVdbox != MOS_FORCE_VDBOX_2       &&
        // 2 pipes, VDBOX1-BE1, VDBOX2-BE2
        forceVdbox != MOS_FORCE_VDBOX_1_1_2   &&
        forceVdbox != MOS_FORCE_VDBOX_2_1_2)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("user feature forceVdbox value is invalid.");
        return eStatus;
    }

    if (!scalableDecMode &&
        (forceVdbox == MOS_FORCE_VDBOX_1_1_2 ||
            forceVdbox == MOS_FORCE_VDBOX_2_1_2))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("user feature forceVdbox valude does not consistent with regkey scalability mode.");
        return eStatus;
    }

    if (scalableDecMode && !m_scalabilitySupported)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("user feature scalability mode is not allowed on current platform!");
        return eStatus;
    }

    useVD1 = useVD2 = false;
    if (forceVdbox == 0)
    {
        useVD1 = true;
    }
    else
    {
        MHW_VDBOX_IS_VDBOX_SPECIFIED(forceVdbox, MOS_FORCE_VDBOX_1,
            MOS_FORCEVDBOX_VDBOXID_BITSNUM,
            MOS_FORCEVDBOX_MASK,
            useVD1);
        MHW_VDBOX_IS_VDBOX_SPECIFIED(forceVdbox, MOS_FORCE_VDBOX_2,
            MOS_FORCEVDBOX_VDBOXID_BITSNUM,
            MOS_FORCEVDBOX_MASK,
            useVD2);
    }

    if (!gtSystemInfo->VDBoxInfo.IsValid ||
        (useVD1 && !gtSystemInfo->VDBoxInfo.Instances.Bits.VDBox0Enabled) ||
        (useVD2 && !gtSystemInfo->VDBoxInfo.Instances.Bits.VDBox2Enabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("the forced VDBOX is not enabled in current platform.");
        return eStatus;
    }

    return eStatus;
}
#endif

MOS_STATUS MhwVdboxMfxInterfaceG11::FindGpuNodeToUse(
    PMHW_VDBOX_GPUNODE_LIMIT       gpuNodeLimit)
{
    bool setVideoNode = false;
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    MOS_GPU_NODE videoGpuNode = MOS_GPU_NODE_VIDEO;

    if (MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface))
    {
        if (GetNumVdbox() == 1)
        {
            videoGpuNode = MOS_GPU_NODE_VIDEO;
        }
        else
        {
            MHW_MI_CHK_STATUS(m_osInterface->pfnCreateVideoNodeAssociation(
                m_osInterface,
                setVideoNode,
                &videoGpuNode));
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface != nullptr && m_osInterface->bEnableDbgOvrdInVE &&
        (!m_osInterface->bSupportVirtualEngine || !m_scalabilitySupported))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("not support DebugOverrid on current OS or Platform.");
        return eStatus;
    }

    if (m_osInterface != nullptr && m_osInterface->bEnableDbgOvrdInVE)
    {
        MHW_MI_CHK_STATUS(CheckScalabilityOverrideValidity());
    }
#endif

    gpuNodeLimit->dwGpuNodeToUse = videoGpuNode;

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::GetMfxStateCommandsDataSize(
    uint32_t mode,
    uint32_t *commandsSize,
    uint32_t *patchListSize,
    bool isShortFormat)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t maxSize =
        mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize +
        mhw_vdbox_mfx_g11_X::MFX_PIPE_MODE_SELECT_CMD::byteSize +
        mhw_vdbox_mfx_g11_X::MFX_SURFACE_STATE_CMD::byteSize +
        mhw_vdbox_mfx_g11_X::MFX_PIPE_BUF_ADDR_STATE_CMD::byteSize +
        mhw_vdbox_mfx_g11_X::MFX_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
        2 * mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize +
        2 * mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD::byteSize +
        8 * mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize;

    uint32_t patchListMaxSize =
        PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
        PATCH_LIST_COMMAND(MFX_PIPE_MODE_SELECT_CMD) +
        PATCH_LIST_COMMAND(MFX_SURFACE_STATE_CMD) +
        PATCH_LIST_COMMAND(MFX_PIPE_BUF_ADDR_STATE_CMD) +
        PATCH_LIST_COMMAND(MFX_IND_OBJ_BASE_ADDR_STATE_CMD) +
        (2 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD)) +
        (2 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD));

    uint32_t standard = CodecHal_GetStandardFromMode(mode);
    if (standard == CODECHAL_AVC)
    {
        maxSize +=
            mhw_vdbox_mfx_g11_X::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFD_AVC_PICID_STATE_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFX_AVC_DIRECTMODE_STATE_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFX_QM_STATE_CMD::byteSize * 4;                     // QM_State sent 4 times

        patchListMaxSize +=
            PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(MFD_AVC_PICID_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_AVC_DIRECTMODE_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_AVC_IMG_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_QM_STATE_CMD) * 4;

        if (mode == CODECHAL_ENCODE_MODE_AVC)
        {
            maxSize +=
                mhw_mi_g11_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +
                mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize * 3 +   // 3 extra MI_FLUSH_DWs for encode
                mhw_vdbox_mfx_g11_X::MFX_FQM_STATE_CMD::byteSize * 4 +   // FQM_State sent 4 times
                mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 8 +   // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize * 3 +   // slice level commands for StatusReport, BrcPakStatistics
                MHW_VDBOX_PAK_BITSTREAM_OVERFLOW_SIZE;                // accounting for the max DW payload for PAK_INSERT_OBJECT, for frame header payload

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 3 +   // 3 extra MI_FLUSH_DWs for encode
                PATCH_LIST_COMMAND(MFX_FQM_STATE_CMD) * 4 +   // FQM_State sent 4 times
                PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) * 8 +   // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) * 3;// slice level commands for StatusReport, BrcPakStatistics
        }
    }
    else if (standard == CODECHAL_VC1)
    {
        maxSize +=
            mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFD_VC1_LONG_PIC_STATE_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFX_VC1_PRED_PIPE_STATE_CMD::byteSize;

        patchListMaxSize +=
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(MFD_VC1_LONG_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_VC1_PRED_PIPE_STATE_CMD);

        if (mode == CODECHAL_DECODE_MODE_VC1VLD)
        {
            maxSize +=
                mhw_vdbox_mfx_g11_X::MFX_VC1_DIRECTMODE_STATE_CMD::byteSize +
                mhw_vdbox_mfx_g11_X::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MFX_VC1_DIRECTMODE_STATE_CMD) +
                PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD);
        }
        else if (mode == CODECHAL_DECODE_MODE_VC1IT)
        {
            maxSize +=
                mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize * 2;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2;
        }
    }
    else if (standard == CODECHAL_MPEG2)
    {
        maxSize += mhw_vdbox_mfx_g11_X::MFX_MPEG2_PIC_STATE_CMD::byteSize;
        patchListMaxSize += PATCH_LIST_COMMAND(MFX_MPEG2_PIC_STATE_CMD);

        if (mode == CODECHAL_DECODE_MODE_MPEG2VLD)
        {
            maxSize +=
                mhw_vdbox_mfx_g11_X::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                ((mhw_vdbox_mfx_g11_X::MFX_QM_STATE_CMD::byteSize + (m_mpeg2WeightScaleSize * sizeof(uint32_t))) * 2);

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(MFX_QM_STATE_CMD) * 2;
        }
        else if (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
        {
            maxSize +=
                mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize * 2;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2;
        }
    }
    else if (standard == CODECHAL_VP8)
    {
        maxSize +=
            mhw_vdbox_mfx_g11_X::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFX_VP8_PIC_STATE_CMD::byteSize;

        patchListMaxSize +=
            PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_VP8_PIC_STATE_CMD);
    }
    else if (standard == CODECHAL_JPEG)
    {
        // Added to prevent error for JPEG 
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported decode mode.");
        maxSize = 0;
        patchListMaxSize = 0;
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::GetMfxPrimitiveCommandsDataSize(
    uint32_t mode,
    uint32_t *commandsSize,
    uint32_t *patchListSize,
    bool  isModeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t maxSize = 0, patchListMaxSize = 0;
    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_AVC)
    {
        if (mode == CODECHAL_DECODE_MODE_AVCVLD)
        {
            maxSize =
                mhw_vdbox_mfx_g11_X::MFX_AVC_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_mfx_g11_X::MFD_AVC_BSD_OBJECT_CMD::byteSize +
                mhw_vdbox_mfx_g11_X::MFD_AVC_DPB_STATE_CMD::byteSize +
                mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(MFX_AVC_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(MFD_AVC_BSD_OBJECT_CMD) +
                PATCH_LIST_COMMAND(MFD_AVC_DPB_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);

            if (isModeSpecific)
            {
                // isModeSpecific = bShortFormat for AVC decode
                maxSize +=
                    mhw_vdbox_mfx_g11_X::MFD_AVC_DPB_STATE_CMD::byteSize +
                    mhw_vdbox_mfx_g11_X::MFD_AVC_SLICEADDR_CMD::byteSize;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFD_AVC_DPB_STATE_CMD) +
                    PATCH_LIST_COMMAND(MFD_AVC_SLICEADDR_CMD);
            }
            else
            {
                maxSize +=
                    (2 * mhw_vdbox_mfx_g11_X::MFX_AVC_REF_IDX_STATE_CMD::byteSize) +
                    (2 * mhw_vdbox_mfx_g11_X::MFX_AVC_WEIGHTOFFSET_STATE_CMD::byteSize);

                patchListMaxSize +=
                    (2 * PATCH_LIST_COMMAND(MFX_AVC_REF_IDX_STATE_CMD)) +
                    (2 * PATCH_LIST_COMMAND(MFX_AVC_WEIGHTOFFSET_STATE_CMD));
            }
        }
        else // CODECHAL_ENCODE_MODE_AVC
        {
            // 1 PAK_INSERT_OBJECT inserted for every end of frame/stream with 1 DW payload
            maxSize = mhw_vdbox_mfx_g11_X::MFX_PAK_INSERT_OBJECT_CMD::byteSize + sizeof(uint32_t);
            patchListMaxSize = PATCH_LIST_COMMAND(MFC_AVC_PAK_INSERT_OBJECT_CMD);

            if (isModeSpecific)
            {
                // isModeSpecific = bSingleTaskPhaseSupported for AVC encode
                maxSize += (2 * mhw_mi_g11_X::MI_BATCH_BUFFER_START_CMD::byteSize);
                patchListMaxSize += (2 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD));
            }
            else
            {
                maxSize +=
                    mhw_vdbox_mfx_g11_X::MFX_AVC_SLICE_STATE_CMD::byteSize +
                    (2 * mhw_vdbox_mfx_g11_X::MFX_AVC_REF_IDX_STATE_CMD::byteSize) +
                    (2 * mhw_vdbox_mfx_g11_X::MFX_AVC_WEIGHTOFFSET_STATE_CMD::byteSize) +
                    mhw_vdbox_mfx_g11_X::MFX_PAK_INSERT_OBJECT_CMD::byteSize +
                    MHW_VDBOX_PAK_BITSTREAM_OVERFLOW_SIZE + // slice header payload
                    mhw_mi_g11_X::MI_BATCH_BUFFER_END_CMD::byteSize;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFX_AVC_SLICE_STATE_CMD) +
                    (2 * PATCH_LIST_COMMAND(MFX_AVC_REF_IDX_STATE_CMD)) +
                    (2 * PATCH_LIST_COMMAND(MFX_AVC_WEIGHTOFFSET_STATE_CMD)) +
                    PATCH_LIST_COMMAND(MFC_AVC_PAK_INSERT_OBJECT_CMD);
            }
        }
    }
    else if (standard == CODECHAL_VC1)
    {
        if (mode == CODECHAL_DECODE_MODE_VC1VLD)
        {
            maxSize =
                mhw_vdbox_mfx_g11_X::MFD_VC1_BSD_OBJECT_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(MFD_VC1_BSD_OBJECT_CMD);
        }
        else if (mode == CODECHAL_DECODE_MODE_VC1IT)
        {
            maxSize =
                sizeof(MFD_VC1_IT_OBJECT_CMD);

            patchListMaxSize =
                PATCH_LIST_COMMAND(MFD_VC1_IT_OBJECT_CMD);
        }
    }
    else if (standard == CODECHAL_MPEG2)
    {
        if (mode == CODECHAL_DECODE_MODE_MPEG2VLD)
        {
            maxSize =
                mhw_vdbox_mfx_g11_X::MFD_MPEG2_BSD_OBJECT_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(MFD_MPEG2_BSD_OBJECT_CMD);
        }
        else if (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
        {
            maxSize =
                sizeof(MFD_MPEG2_IT_OBJECT_CMD);

            patchListMaxSize =
                PATCH_LIST_COMMAND(MFD_MPEG2_IT_OBJECT_CMD);
        }
    }
    else if (standard == CODECHAL_VP8)
    {
        maxSize =
            mhw_vdbox_mfx_g11_X::MFD_VP8_BSD_OBJECT_CMD::byteSize;

        patchListMaxSize +=
            PATCH_LIST_COMMAND(MFD_VP8_BSD_OBJECT_CMD);
    }
    else if (standard == CODECHAL_JPEG)
    {
        maxSize +=
            mhw_vdbox_mfx_g11_X::MFX_FQM_STATE_CMD::byteSize * 3 +
            mhw_vdbox_mfx_g11_X::MFC_JPEG_HUFF_TABLE_STATE_CMD::byteSize * 2 +
            mhw_vdbox_mfx_g11_X::MFC_JPEG_SCAN_OBJECT_CMD::byteSize +
            mhw_vdbox_mfx_g11_X::MFX_PAK_INSERT_OBJECT_CMD::byteSize * 10;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported decode mode.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    //for gen 11, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select...
    MHW_MI_CHK_STATUS(m_MiInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    mhw_vdbox_mfx_g11_X::MFX_PIPE_MODE_SELECT_CMD cmd;

    MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMfxPipeModeSelect((uint32_t *)&cmd));

    cmd.DW1.StreamOutEnable = params->bStreamOutEnabled;
    cmd.DW1.DeblockerStreamOutEnable = params->bDeblockerStreamOutEnable;

    if (m_decodeInUse)
    {
        cmd.DW1.PreDeblockingOutputEnablePredeblockoutenable = params->bPreDeblockOutEnable;
        cmd.DW1.PostDeblockingOutputEnablePostdeblockoutenable = params->bPostDeblockOutEnable;
        cmd.DW1.CodecSelect = decoderCodec;
        cmd.DW1.DecoderShortFormatMode = !params->bShortFormatInUse;
        cmd.DW2.Vlf720IOddHeightInVc1Mode = params->bVC1OddFrameHeight;

        if (CodecHalIsDecodeModeVLD(params->Mode))
        {
            cmd.DW1.DecoderModeSelect = mfxDecoderModeVld;
        }
        else if (CodecHalIsDecodeModeIT(params->Mode))
        {
            cmd.DW1.DecoderModeSelect = mfxDecoderModeIt;
        }
    }
    else
    {
        cmd.DW1.DeblockerStreamOutEnable = params->bDeblockerStreamOutEnable;
        cmd.DW1.PostDeblockingOutputEnablePostdeblockoutenable = params->bPostDeblockOutEnable;
        cmd.DW1.PreDeblockingOutputEnablePredeblockoutenable = params->bPreDeblockOutEnable;
        cmd.DW1.CodecSelect = encoderCodec;
        cmd.DW1.DecoderShortFormatMode = !params->bShortFormatInUse;  // This bit is set to be long format in order for HW to not change next slice X and Y position in encoder mode
    }

    cmd.DW1.StandardSelect = CodecHal_GetStandardFromMode(params->Mode);

    if (params->bVdencEnabled)
    {
        cmd.DW1.VdencMode = 1;
        // Enable 4xDS in PAK for VDENC HME
        cmd.DW1.ScaledSurfaceEnable = 1;
        // Disable PAK streamout from previous PAK pass, as VDEnc does not support standalone PAK 
        cmd.DW1.StreamOutEnable = 0;
        // Enable PAK statistics streamout 
        cmd.DW1.FrameStatisticsStreamoutEnable = 1;
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    //for gen 11, we need to add MFX wait for both KIN and VRT before and after MFX Pipemode select...
    MHW_MI_CHK_STATUS(m_MiInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxSurfaceCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    uint32_t uvPlaneAlignment;
    if (params->ucSurfaceStateId == CODECHAL_MFX_SRC_SURFACE_ID)
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9;
    }
    else if ((params->ucSurfaceStateId == CODECHAL_MFX_REF_SURFACE_ID) || params->ucSurfaceStateId == CODECHAL_MFX_DSRECON_SURFACE_ID)
    {
        uvPlaneAlignment = params->dwUVPlaneAlignment ? params->dwUVPlaneAlignment : MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;
    }
    else
    {
        uvPlaneAlignment = MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY;
    }

    mhw_vdbox_mfx_g11_X::MFX_SURFACE_STATE_CMD cmd;
    cmd.DW1.SurfaceId = params->ucSurfaceStateId;

    cmd.DW2.Height = params->psSurface->dwHeight - 1;
    cmd.DW2.Width = params->psSurface->dwWidth - 1;

    cmd.DW3.TileWalk = mhw_vdbox_mfx_g11_X::MFX_SURFACE_STATE_CMD::TILE_WALK_YMAJOR;
    cmd.DW3.TiledSurface = 1;
    cmd.DW3.InterleaveChroma = 1;
    cmd.DW3.SurfacePitch = params->psSurface->dwPitch - 1;
    cmd.DW3.SurfaceFormat = MosToMediaStateFormat(params->psSurface->Format);
    cmd.DW3.TiledSurface = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

    if (cmd.DW3.TiledSurface)
    {
        cmd.DW3.TileWalk = (params->psSurface->TileType);
    }

    if (params->psSurface->Format == Format_P8) // monochrome format
    {
        cmd.DW3.InterleaveChroma = 0;
    }

    if (params->Mode == CODECHAL_DECODE_MODE_JPEG)
    {
        // this parameter must always be 0 for JPEG regardless of the YUV format
        cmd.DW3.InterleaveChroma = 0;

        // Separate function for JPEG decode because this surface format should match with that programmed 
        // in JPEG Picture State
        cmd.DW3.SurfaceFormat = GetJpegDecodeFormat(params->psSurface->Format);

    }

    cmd.DW4.YOffsetForUCb = cmd.DW5.YOffsetForVCr =
        MOS_ALIGN_CEIL(params->psSurface->UPlaneOffset.iYOffset, uvPlaneAlignment);

    if (IsVPlanePresent(params->psSurface->Format))
    {
        cmd.DW5.YOffsetForVCr =
            MOS_ALIGN_CEIL(params->psSurface->VPlaneOffset.iYOffset, uvPlaneAlignment);
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    mhw_vdbox_mfx_g11_X::MFX_PIPE_BUF_ADDR_STATE_CMD cmd;

    // Encoding uses both surfaces regardless of deblocking status
    if (params->psPreDeblockSurface != nullptr)
    {
        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;
        cmd.DW3.PreDeblockingMemoryCompressionEnable = (params->PreDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW3.PreDeblockingMemoryCompressionMode = (params->PreDeblockSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.DW3.PreDeblockingTiledResourceMode = Mhw_ConvertToTRMode(params->psPreDeblockSurface->TileType);

        resourceParams.presResource = &(params->psPreDeblockSurface->OsResource);
        resourceParams.dwOffset = params->psPreDeblockSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->PreDeblockSurfMmcState));
    }

    if (params->psPostDeblockSurface != nullptr)
    {
        cmd.DW6.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_POST_DEBLOCKING_CODEC].Value;
        cmd.DW6.PostDeblockingMemoryCompressionEnable = (params->PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW6.PostDeblockingMemoryCompressionMode = (params->PostDeblockSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.DW6.PostDeblockingTiledResourceMode = Mhw_ConvertToTRMode(params->psPostDeblockSurface->TileType);

        resourceParams.presResource = &(params->psPostDeblockSurface->OsResource);
        resourceParams.dwOffset = params->psPostDeblockSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->PostDeblockSurfMmcState));
    }

    if (params->psRawSurface != nullptr)
    {
        if (!m_decodeInUse)
        {
            MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &params->psRawSurface->OsResource, &mmcMode));

            cmd.DW9.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;
            cmd.DW9.OriginalUncompressedPictureMemoryCompressionEnable =
                mmcMode != MOS_MEMCOMP_DISABLED ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
            cmd.DW9.OriginalUncompressedPictureMemoryCompressionMode =
                mmcMode == MOS_MEMCOMP_HORIZONTAL ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;
        }
        else
        {
            cmd.DW9.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE].Value;
        }

        cmd.DW9.OriginalUncompressedPictureTiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW7.Value);
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presStreamOutBuffer != nullptr)
    {
        cmd.DW12.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

        cmd.DW12.StreamoutDataDestinationMemoryCompressionEnable = params->StreamOutBufMmcState != MOS_MEMCOMP_DISABLED ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW12.StreamoutDataDestinationMemoryCompressionMode = params->StreamOutBufMmcState == MOS_MEMCOMP_HORIZONTAL ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        resourceParams.presResource = params->presStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW10.Value);
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (!m_decodeInUse)
        {
            cmd.DW54.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

            resourceParams.presResource = params->presStreamOutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = &(cmd.DW52.Value);
            resourceParams.dwLocationInCmd = 52;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    if (m_intraRowstoreCache.bEnabled)
    {
        cmd.DW15.IntraRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
        cmd.DW13.IntraRowStoreScratchBufferBaseAddress = m_intraRowstoreCache.dwAddress;
    }
    else if (params->presMfdIntraRowStoreScratchBuffer != nullptr)
    {
        cmd.DW15.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        cmd.DW15.IntraRowStoreScratchBufferMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

        resourceParams.presResource = params->presMfdIntraRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW13.Value);
        resourceParams.dwLocationInCmd = 13;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (m_deblockingFilterRowstoreCache.bEnabled)
    {
        cmd.DW18.DeblockingFilterRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
        cmd.DW16.DeblockingFilterRowStoreScratchBaseAddress =
            m_deblockingFilterRowstoreCache.dwAddress;
    }
    else if (params->presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
    {
        cmd.DW18.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;
        cmd.DW18.DeblockingFilterRowStoreScratchMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

        resourceParams.presResource = params->presMfdDeblockingFilterRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW16.Value);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    bool firstRefPic = true;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        if (params->presReferences[i] != nullptr)
        {
            MOS_SURFACE details;
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->presReferences[i], &details));

            MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, params->presReferences[i], &mmcMode));

            if (mmcMode == MOS_MEMCOMP_HORIZONTAL)
            {
                cmd.DW61.Value |= (MHW_MEDIA_MEMCOMP_ENABLED << (i * 2)) | (MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL << (i * 2 + 1));
            }
            else if (mmcMode == MOS_MEMCOMP_VERTICAL)
            {
                cmd.DW61.Value |= (MHW_MEDIA_MEMCOMP_ENABLED << (i * 2)) | (MHW_MEDIA_MEMCOMP_MODE_VERTICAL << (i * 2 + 1));
            }
            else
            {
                cmd.DW61.Value |= MHW_MEDIA_MEMCOMP_DISABLED << (i * 2);
            }

            if (firstRefPic)
            {
                cmd.DW51.ReferencePictureTiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                firstRefPic = false;
            }

            resourceParams.presResource = params->presReferences[i];
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.pdwCmd = &(cmd.Refpicbaseaddr[i].DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = (i * 2) + 19; // * 2 to account for QW rather than DW
            resourceParams.bIsWritable = false;

            resourceParams.dwSharedMocsOffset = 51 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW51

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    // There is only one control DW51 for all references
    cmd.DW51.MemoryObjectControlState =
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    if (params->presMacroblockIldbStreamOutBuffer1 != nullptr)
    {
        cmd.DW57.MacroblockIldbStreamoutBufferMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW57.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presMacroblockIldbStreamOutBuffer1;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW55.Value);
        resourceParams.dwLocationInCmd = 55;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presMacroblockIldbStreamOutBuffer2 != nullptr)
    {
        cmd.DW60.SecondMacroblockIldbStreamoutBufferMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW60.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presMacroblockIldbStreamOutBuffer2;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW58.Value);
        resourceParams.dwLocationInCmd = 58;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Add 4xDS surface for VDENC
    if (params->bVdencEnabled && params->ps4xDsSurface != nullptr)
    {
        resourceParams.presResource = &params->ps4xDsSurface->OsResource;
        resourceParams.dwOffset = params->ps4xDsSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW62.Value);
        resourceParams.dwLocationInCmd = 62;
        resourceParams.bIsWritable = true;

        MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
        MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, resourceParams.presResource, &mmcMode));

        if (mmcMode == MOS_MEMCOMP_DISABLED)
        {
            params->Ps4xDsSurfMmcState = MOS_MEMCOMP_DISABLED;
            cmd.DW64.ScaledReferenceSurfaceMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
        }
        else
        {
            cmd.DW64.ScaledReferenceSurfaceMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_ENABLED;
        }
        cmd.DW64.ScaledReferenceSurfaceMemoryCompressionMode = (params->Ps4xDsSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.DW64.ScaledReferenceSurfaceTiledResourceMode = Mhw_ConvertToTRMode(params->ps4xDsSurface->TileType);

        cmd.DW64.ScaledReferenceSurfaceIndexToMemoryObjectControlStateMocsTables =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value >> 1;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->Ps4xDsSurfMmcState));
    }

    if (params->presSliceSizeStreamOutBuffer != nullptr)
    {
        cmd.DW67.SlicesizeStreamoutDataDestinationIndexToMemoryObjectControlStateMocsTables =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;
        cmd.DW67.SlicesizeStreamoutDataDestinationMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

        resourceParams.presResource = params->presSliceSizeStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW65.Value);
        resourceParams.dwLocationInCmd = 65;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxIndObjBaseAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_UPPER_BOUND_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

    mhw_vdbox_mfx_g11_X::MFX_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

    // mode specific settings
    if (CodecHalIsDecodeModeVLD(params->Mode) || (params->Mode == CODECHAL_ENCODE_MODE_VP8))
    {
        MHW_MI_CHK_NULL(params->presDataBuffer);

        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Value;

        resourceParams.presResource = params->presDataBuffer;
        resourceParams.dwOffset = params->dwDataOffset;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwSize = params->dwDataSize;
        resourceParams.bIsWritable = false;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }
    else if (CodecHalIsDecodeModeIT(params->Mode))
    {
        MHW_MI_CHK_NULL(params->presDataBuffer);

        cmd.DW13.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE].Value;

        resourceParams.presResource = params->presDataBuffer;
        resourceParams.dwOffset = params->dwDataOffset;
        resourceParams.pdwCmd = &(cmd.DW11.Value);
        resourceParams.dwLocationInCmd = 11;
        resourceParams.dwSize = params->dwDataSize;
        resourceParams.bIsWritable = false;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presMvObjectBuffer)
    {
        cmd.DW8.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC].Value;

        resourceParams.presResource = params->presMvObjectBuffer;
        resourceParams.dwOffset = params->dwMvObjectOffset;
        resourceParams.pdwCmd = &(cmd.DW6.Value);
        resourceParams.dwLocationInCmd = 6;
        resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwMvObjectSize, 0x1000);
        resourceParams.bIsWritable = false;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presPakBaseObjectBuffer)
    {
        cmd.DW23.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

        resourceParams.presResource = params->presPakBaseObjectBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW21.Value);
        resourceParams.dwLocationInCmd = 21;
        resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwPakBaseObjectSize, 0x1000);
        resourceParams.bIsWritable = true;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxBspBufBaseAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_BSP_BUF_BASE_ADDR;

    mhw_vdbox_mfx_g11_X::MFX_BSP_BUF_BASE_ADDR_STATE_CMD cmd;

    if (m_bsdMpcRowstoreCache.bEnabled)         // mbaff and non mbaff mode for all resolutions
    {
        cmd.DW3.BsdMpcRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
        cmd.DW1.BsdMpcRowStoreScratchBufferBaseAddressReadWrite = m_bsdMpcRowstoreCache.dwAddress;
    }
    else if (params->presBsdMpcRowStoreScratchBuffer)
    {
        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;
        cmd.DW1.BsdMpcRowStoreScratchBufferBaseAddressReadWrite = 0;

        resourceParams.presResource = params->presBsdMpcRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (m_mprRowstoreCache.bEnabled)
    {
        cmd.DW6.MprRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
        cmd.DW4.MprRowStoreScratchBufferBaseAddressReadWriteDecoderOnly = m_mprRowstoreCache.dwAddress;
    }
    else if (params->presMprRowStoreScratchBuffer)
    {
        cmd.DW6.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;
        cmd.DW4.MprRowStoreScratchBufferBaseAddressReadWriteDecoderOnly = 0;

        resourceParams.presResource = params->presMprRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presBitplaneBuffer)
    {
        cmd.DW9.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_BITPLANE_READ_CODEC].Value;

        resourceParams.presResource = params->presBitplaneBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW7.Value);
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxDecodeAvcImgCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MOS_UNUSED(batchBuffer);

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pAvcPicParams);

    auto avcPicParams = params->pAvcPicParams;

    mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD cmd;

    uint32_t numMBs =
        (avcPicParams->pic_height_in_mbs_minus1 + 1) *
        (avcPicParams->pic_width_in_mbs_minus1 + 1);

    cmd.DW1.FrameSize = numMBs;
    cmd.DW2.FrameHeight = avcPicParams->pic_height_in_mbs_minus1;
    cmd.DW2.FrameWidth = avcPicParams->pic_width_in_mbs_minus1;
    cmd.DW3.SecondChromaQpOffset = avcPicParams->second_chroma_qp_index_offset;
    cmd.DW3.FirstChromaQpOffset = avcPicParams->chroma_qp_index_offset;
    cmd.DW3.WeightedPredFlag = avcPicParams->pic_fields.weighted_pred_flag;
    cmd.DW3.WeightedBipredIdc = avcPicParams->pic_fields.weighted_bipred_idc;
    cmd.DW3.ImgstructImageStructureImgStructure10 =
        ((avcPicParams->CurrPic.PicFlags == PICTURE_FRAME) ?
            avcFrame : (CodecHal_PictureIsTopField(avcPicParams->CurrPic) ?
                avcTopField : avcBottomField));
    cmd.DW4.Chromaformatidc = avcPicParams->seq_fields.chroma_format_idc;
    cmd.DW4.Entropycodingflag = avcPicParams->pic_fields.entropy_coding_mode_flag;
    cmd.DW4.Imgdisposableflag = !avcPicParams->pic_fields.reference_pic_flag;
    cmd.DW4.Constrainedipredflag = avcPicParams->pic_fields.constrained_intra_pred_flag;
    cmd.DW4.Direct8X8Infflag = avcPicParams->seq_fields.direct_8x8_inference_flag;
    cmd.DW4.Transform8X8Flag = avcPicParams->pic_fields.transform_8x8_mode_flag;
    cmd.DW4.Framembonlyflag = avcPicParams->seq_fields.frame_mbs_only_flag;
    cmd.DW4.Mbaffflameflag =
        avcPicParams->seq_fields.mb_adaptive_frame_field_flag && !avcPicParams->pic_fields.field_pic_flag;
    cmd.DW4.Fieldpicflag = avcPicParams->pic_fields.field_pic_flag;

    cmd.DW5.TrellisQuantizationChromaDisableTqchromadisable = true;
    cmd.DW5.TrellisQuantizationRoundingTqr = 0;

    cmd.DW13.CurrentPictureHasPerformedMmco5 = 0;
    cmd.DW13.NumberOfReferenceFrames = params->ucActiveFrameCnt;
    // Specifies these fields to be NumRefIdx(L0 or L1) Active - 1 at picture level
    // But hardware expects the data without the minus 1 to keep this field consistent with
    // NumRefIdxL0 and NumRefIdxL1 which appears at slice level. Thus the addition of 1
    cmd.DW13.NumberOfActiveReferencePicturesFromL0 = avcPicParams->num_ref_idx_l0_active_minus1 + 1;
    cmd.DW13.NumberOfActiveReferencePicturesFromL1 = avcPicParams->num_ref_idx_l1_active_minus1 + 1;

    cmd.DW13.InitialQpValue = avcPicParams->pic_init_qp_minus26;

    cmd.DW14.Log2MaxFrameNumMinus4 = avcPicParams->seq_fields.log2_max_frame_num_minus4;
    cmd.DW14.Log2MaxPicOrderCntLsbMinus4 = avcPicParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4;
    cmd.DW14.DeblockingFilterControlPresentFlag = avcPicParams->pic_fields.deblocking_filter_control_present_flag;
    cmd.DW14.NumSliceGroupsMinus1 = avcPicParams->num_slice_groups_minus1;
    cmd.DW14.RedundantPicCntPresentFlag = avcPicParams->pic_fields.redundant_pic_cnt_present_flag;
    cmd.DW14.PicOrderPresentFlag = avcPicParams->pic_fields.pic_order_present_flag;
    cmd.DW14.SliceGroupMapType = avcPicParams->slice_group_map_type;
    cmd.DW14.PicOrderCntType = avcPicParams->seq_fields.pic_order_cnt_type;
    cmd.DW14.DeltaPicOrderAlwaysZeroFlag = avcPicParams->seq_fields.delta_pic_order_always_zero_flag;
    cmd.DW15.CurrPicFrameNum = avcPicParams->frame_num;
    cmd.DW15.SliceGroupChangeRate = avcPicParams->slice_group_change_rate_minus1;

    auto mvcExtPicParams = params->pMvcExtPicParams;
    if (mvcExtPicParams)
    {
        cmd.DW16.CurrentFrameViewId = mvcExtPicParams->CurrViewID;
        cmd.DW16.MaxViewIdxl0 = mvcExtPicParams->NumInterViewRefsL0;
        cmd.DW16.MaxViewIdxl1 = mvcExtPicParams->NumInterViewRefsL1;
        cmd.DW16.InterViewOrderDisable = 0;
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxEncodeAvcImgCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pEncodeAvcSeqParams);
    MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto avcSeqParams = params->pEncodeAvcSeqParams;
    auto avcPicParams = params->pEncodeAvcPicParams;

    mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD cmd;

    uint32_t numMBs = params->wPicWidthInMb * params->wPicHeightInMb;
    cmd.DW1.FrameSize = (numMBs > 0xFFFF) ? 0xFFFF : numMBs;

    cmd.DW2.FrameHeight = params->wPicHeightInMb - 1;
    cmd.DW2.FrameWidth = params->wPicWidthInMb - 1;

    cmd.DW3.ImgstructImageStructureImgStructure10 =
        (CodecHal_PictureIsFrame(avcPicParams->CurrOriginalPic) ?
            avcFrame : (CodecHal_PictureIsTopField(avcPicParams->CurrOriginalPic) ?
                avcTopField : avcBottomField));

    cmd.DW3.WeightedBipredIdc = avcPicParams->weighted_bipred_idc;
    cmd.DW3.WeightedPredFlag = avcPicParams->weighted_pred_flag;

    cmd.DW3.FirstChromaQpOffset = avcPicParams->chroma_qp_index_offset;
    cmd.DW3.SecondChromaQpOffset = avcPicParams->second_chroma_qp_index_offset;

    // Enable RhoDomain RC in PAK statistics
    cmd.DW3.RhodomainRateControlEnable = params->bVdencEnabled;

    cmd.DW4.Fieldpicflag =
        CodecHal_PictureIsField(avcPicParams->CurrOriginalPic);
    cmd.DW4.Mbaffflameflag = avcSeqParams->mb_adaptive_frame_field_flag;
    cmd.DW4.Framembonlyflag = avcSeqParams->frame_mbs_only_flag;
    cmd.DW4.Transform8X8Flag = avcPicParams->transform_8x8_mode_flag;
    cmd.DW4.Direct8X8Infflag = avcSeqParams->direct_8x8_inference_flag;
    cmd.DW4.Constrainedipredflag = avcPicParams->constrained_intra_pred_flag;
    cmd.DW4.Entropycodingflag = avcPicParams->entropy_coding_mode_flag;
    cmd.DW4.Chromaformatidc = avcSeqParams->chroma_format_idc;
    cmd.DW4.Mbmvformatflag = 1;
    cmd.DW4.Mvunpackedflag = 1;

    cmd.DW4.Loadslicepointerflag = 0;
    cmd.DW4.Mbstatenabled = 0; // Disable for the first pass
    if ((params->dwMaxFrameSize > 0) && params->ucCurrPass && params->pDeltaQp)
    {
        cmd.DW4.Mbstatenabled = 1;
    }
    cmd.DW4.Minframewsize = 0;

    cmd.DW5.IntrambmaxbitflagIntrambmaxsizereportmask = 1;
    cmd.DW5.IntermbmaxbitflagIntermbmaxsizereportmask = 1;
    cmd.DW5.FrameszoverflagFramebitratemaxreportmask = 1;
    cmd.DW5.FrameszunderflagFramebitrateminreportmask = 1;
    cmd.DW5.IntraIntermbipcmflagForceipcmcontrolmask = 1;
    cmd.DW5.MbratectrlflagMbLevelRateControlEnablingFlag = 0;
    cmd.DW5.Nonfirstpassflag = 0;
    cmd.DW5.TrellisQuantizationChromaDisableTqchromadisable = true;

    if (params->dwMaxFrameSize && params->ucCurrPass)
    {
        cmd.DW5.Nonfirstpassflag = 1;
    }

    if (params->dwTqEnabled &&
        cmd.DW4.Entropycodingflag)
    {
        cmd.DW5.TrellisQuantizationEnabledTqenb = params->dwTqEnabled;

        cmd.DW5.TrellisQuantizationRoundingTqr = params->dwTqRounding;
    }
    else
    {
        cmd.DW5.TrellisQuantizationEnabledTqenb = cmd.DW5.TrellisQuantizationRoundingTqr = 0;
    }

    //DW6
    cmd.DW6.Intrambmaxsz = m_avcIntraMbMaxSize;
    cmd.DW6.Intermbmaxsz = m_avcInterMbMaxSize;
    //DW8
    cmd.DW8.Slicedeltaqppmax0 =
        cmd.DW8.Slicedeltaqpmax1 =
        cmd.DW8.Slicedeltaqpmax2 =
        cmd.DW8.Slicedeltaqpmax3 = 0;
    //DW9
    cmd.DW9.Slicedeltaqpmin0 =
        cmd.DW9.Slicedeltaqpmin1 =
        cmd.DW9.Slicedeltaqpmin2 =
        cmd.DW9.Slicedeltaqpmin3 = 0;

    //DW10
    MHW_VDBOX_AVC_IMG_BITRATE_PARAMS bitrateParams;
    CalcAvcImgStateMinMaxBitrate(bitrateParams);
    cmd.DW10.Framebitratemin = bitrateParams.frameBitRateMin;
    cmd.DW10.Framebitrateminunit = bitrateParams.frameBitRateMinUnit;
    cmd.DW10.Framebitrateminunitmode = bitrateParams.frameBitRateMinUnitMode;
    cmd.DW10.Framebitratemax = bitrateParams.frameBitRateMax;
    cmd.DW10.Framebitratemaxunit = bitrateParams.frameBitRateMaxUnit;
    cmd.DW10.Framebitratemaxunitmode = bitrateParams.frameBitRateMaxUnitMode;

    //DW11
    cmd.DW11.Framebitratemindelta = bitrateParams.frameBitRateMinDelta;
    cmd.DW11.Framebitratemaxdelta = bitrateParams.frameBitRateMaxDelta;
    cmd.DW11.SliceStatsStreamoutEnable = params->bSliceSizeStreamOutEnabled;

    //add for multiple pass
    if (params->dwMaxFrameSize > 0 && params->pDeltaQp && (!params->bIPCMPass))
    {
        cmd.DW8.Slicedeltaqppmax0 =
            cmd.DW8.Slicedeltaqpmax1 =
            cmd.DW8.Slicedeltaqpmax2 =
            cmd.DW8.Slicedeltaqpmax3 = params->pDeltaQp[params->ucCurrPass];
        cmd.DW10.Framebitratemaxunit = 0;
        cmd.DW10.Framebitratemaxunitmode = 0;
        //when FrameBitrateMaxUnit & FrameBitrateMaxUnitMode both are 0, the frame size unit is 128bytes.
        cmd.DW10.Framebitratemax = params->dwMaxFrameSize >> 7;
        cmd.DW11.Framebitratemaxdelta = params->dwMaxFrameSize >> 8;

        // In compatibility mode (DW10.FrameBitrateMaxUnitMode = 0), only 12 bits is used.
        // If the calulated value of max frame size exceeded 12 bits, need change unit from 128 bytes to 16K bytes.
        if (params->dwMaxFrameSize >= (0x1 << 12) * 128)
        {
            // use 16K bytes unit mode in compatibility mode.
            cmd.DW10.Framebitratemaxunit = 1;
            cmd.DW10.Framebitratemaxunitmode = 0;
            cmd.DW10.Framebitratemax = params->dwMaxFrameSize >> 14;
            cmd.DW11.Framebitratemaxdelta = params->dwMaxFrameSize >> 15;
        }
    }

    if (params->bIPCMPass)
    {
        // InterMbConfFlag, IntraMbConfFlag: not being used in HW
        cmd.DW4.Mbstatenabled = true;
        cmd.DW5.IntraIntermbipcmflagForceipcmcontrolmask = true;
    }

    cmd.DW17.ExtendedRhodomainStatisticsEnable = m_rhoDomainStatsEnabled;

    if (avcSeqParams->EnableSliceLevelRateCtrl)
    {
        uint8_t qpY = avcPicParams->QpY;
        if (params->dwSliceThresholdTable == NO_SLICE_THRESHOLD_TABLE) // Do not use any Slice Threshold Table
        {
            cmd.DW19.ThresholdSizeInBytes = (avcPicParams->SliceSizeInBytes > params->dwVdencSliceMinusBytes) ?
                (avcPicParams->SliceSizeInBytes - params->dwVdencSliceMinusBytes) : 0;
        }
        else if (params->dwSliceThresholdTable == USE_SLICE_THRESHOLD_TABLE_100_PERCENT) // Use 100 Percent Slice Threshold Table
        {
            cmd.DW19.ThresholdSizeInBytes = avcPicParams->SliceSizeInBytes -
                MOS_MIN(avcPicParams->SliceSizeInBytes,
                ((avcPicParams->CodingType == I_TYPE) ? m_vdEncFrameDelta100PercentTab[qpY].IFrameDelta : m_vdEncFrameDelta100PercentTab[qpY].PFrameDelta));
        }
        else if (params->dwSliceThresholdTable == USE_SLICE_THRESHOLD_TABLE_90_PERCENT) // Use 90 Percent Slice Threshold Table
        {
            cmd.DW19.ThresholdSizeInBytes = avcPicParams->SliceSizeInBytes -
                MOS_MIN(avcPicParams->SliceSizeInBytes,
                ((avcPicParams->CodingType == I_TYPE) ? m_vdEncFrameDelta90PercentTab[qpY].IFrameDelta : m_vdEncFrameDelta90PercentTab[qpY].PFrameDelta));
        }
        cmd.DW20.TargetSliceSizeInBytes = avcPicParams->SliceSizeInBytes;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxAvcDirectmodeCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_AVC_DIRECTMODE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_AVC_DIRECT_MODE;

    mhw_vdbox_mfx_g11_X::MFX_AVC_DIRECTMODE_STATE_CMD cmd;

    if (!params->bDisableDmvBuffers)
    {
        MHW_MI_CHK_NULL(params->presAvcDmvBuffers);
        MHW_MI_CHK_NULL(params->pAvcDmvList);

        cmd.DW36.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC].Value;

        // current picture
        resourceParams.presResource = &params->presAvcDmvBuffers[params->ucAvcDmvIdx];
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DirectMvBufferBaseAddressForWrite[0].DW0_1.Value[0]);
        resourceParams.dwLocationInCmd = 34;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    CODEC_REF_LIST** refList;
    MHW_MI_CHK_NULL(refList = (CODEC_REF_LIST**)params->avcRefList);

    if (CodecHal_PictureIsBottomField(params->CurrPic))
    {
        cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_TOP] = 0;
        cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
            refList[params->CurrPic.FrameIdx]->iFieldOrderCnt[1];
    }
    else
    {
        cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_TOP] = cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
            refList[params->CurrPic.FrameIdx]->iFieldOrderCnt[0];
        if (CodecHal_PictureIsFrame(params->CurrPic))
        {
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
                refList[params->CurrPic.FrameIdx]->iFieldOrderCnt[1];
        }
    }

    if (!params->bDisableDmvBuffers)
    {
        // there is only one control DW33 for all references
        cmd.DW33.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC].Value;
    }

    bool dmvPresent[CODEC_MAX_NUM_REF_FRAME] = { false };
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        if (params->pAvcPicIdx[i].bValid)
        {
            uint8_t idx = params->pAvcPicIdx[i].ucPicIdx;
            uint8_t picID = params->bPicIdRemappingInUse ? i : refList[idx]->ucFrameId;
            uint8_t mvIdx = refList[idx]->ucDMVIdx[0];

            uint8_t validRef = ((params->uiUsedForReferenceFlags >> (i * 2)) >> 0) & 1;
            uint8_t frameID = picID << 1;
            if (frameID < CODEC_AVC_NUM_REF_DMV_BUFFERS * 2)
            {
                if (!params->bDisableDmvBuffers)
                {
                    dmvPresent[picID] = true;

                    resourceParams.presResource = &params->presAvcDmvBuffers[mvIdx];
                    resourceParams.dwOffset = 0;
                    resourceParams.pdwCmd = &(cmd.DirectMvBufferBaseAddress[picID].DW0_1.Value[0]);
                    resourceParams.dwLocationInCmd = picID * 2 + 1;
                    resourceParams.bIsWritable = false;

                    resourceParams.dwSharedMocsOffset = 33 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW33

                    MHW_MI_CHK_STATUS(AddResourceToCmd(
                        m_osInterface,
                        cmdBuffer,
                        &resourceParams));
                }

                cmd.PocList[frameID] = refList[idx]->iFieldOrderCnt[0] * validRef;
            }
            else
            {
                return MOS_STATUS_UNKNOWN;
            }

            validRef = ((params->uiUsedForReferenceFlags >> (i * 2)) >> 1) & 1;
            frameID = (picID << 1) + 1;
            if (frameID < CODEC_AVC_NUM_REF_DMV_BUFFERS * 2)
            {
                cmd.PocList[frameID] = refList[idx]->iFieldOrderCnt[1] * validRef;
            }
            else
            {
                return MOS_STATUS_UNKNOWN;
            }
        }
    }

    if (!params->bDisableDmvBuffers)
    {
        // Use a valid address for remaining DMV buffers
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
        {
            if (dmvPresent[i] == false)
            {
                //Give default buffer to the MV
                resourceParams.presResource = &params->presAvcDmvBuffers[CODEC_AVC_NUM_REF_DMV_BUFFERS];
                resourceParams.dwOffset = 0;
                resourceParams.pdwCmd = &(cmd.DirectMvBufferBaseAddress[i].DW0_1.Value[0]);
                resourceParams.dwLocationInCmd = i * 2 + 1;
                resourceParams.bIsWritable = false;

                resourceParams.dwSharedMocsOffset = 33 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW33

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfdAvcSliceAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(avcSliceState);

    mhw_vdbox_mfx_g11_X::MFD_AVC_SLICEADDR_CMD cmd;

    if (avcSliceState->bFullFrameData)
    {
        cmd.DW1.IndirectBsdDataLength       = avcSliceState->dwNextLength;
        cmd.DW2.IndirectBsdDataStartAddress = avcSliceState->dwNextOffset;
    }
    else
    {
        cmd.DW1.IndirectBsdDataLength       = (avcSliceState->dwNextLength + 1 - m_osInterface->dwNumNalUnitBytesIncluded);
        cmd.DW2.IndirectBsdDataStartAddress = (avcSliceState->dwNextOffset - 1 + m_osInterface->dwNumNalUnitBytesIncluded);
    }

    MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
    sliceInfoParam.presDataBuffer = avcSliceState->presDataBuffer;
    sliceInfoParam.dwSliceIndex = avcSliceState->dwSliceIndex;
    sliceInfoParam.dwTotalBytesConsumed = avcSliceState->dwTotalBytesConsumed;
    sliceInfoParam.dwDataStartOffset[0] = cmd.DW2.IndirectBsdDataStartAddress;
    sliceInfoParam.dwDataStartOffset[1] = avcSliceState->pAvcSliceParams->slice_data_offset;

    MHW_MI_CHK_STATUS(m_cpInterface->SetMfxProtectionState(
        m_decodeInUse,
        cmdBuffer,
        nullptr,
        &sliceInfoParam));

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfdAvcBsdObjectCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(avcSliceState);
    MHW_MI_CHK_NULL(avcSliceState->pAvcSliceParams);

    mhw_vdbox_mfx_g11_X::MFD_AVC_BSD_OBJECT_CMD cmd;
    auto sliceParams = avcSliceState->pAvcSliceParams;

    cmd.DW4.LastsliceFlag = avcSliceState->bLastSlice;

    cmd.DW3.IntraPredmode4X48X8LumaErrorControlBit = 1;
    cmd.DW5.IntraPredictionErrorControlBitAppliedToIntra16X16Intra8X8Intra4X4LumaAndChroma = 1;
    cmd.DW5.Intra8X84X4PredictionErrorConcealmentControlBit = 1;
    cmd.DW5.ISliceConcealmentMode = 1;

    if (avcSliceState->bShortFormatInUse)
    {
        if (avcSliceState->bFullFrameData)
        {
            cmd.DW1.IndirectBsdDataLength       = avcSliceState->dwLength;
            cmd.DW2.IndirectBsdDataStartAddress = sliceParams->slice_data_offset;
        }
        else
        {
            cmd.DW1.IndirectBsdDataLength = avcSliceState->dwLength + 1 - m_osInterface->dwNumNalUnitBytesIncluded;
            cmd.DW2.IndirectBsdDataStartAddress =
                sliceParams->slice_data_offset - 1 + m_osInterface->dwNumNalUnitBytesIncluded;
        }
        cmd.DW4.FirstMbByteOffsetOfSliceDataOrSliceHeader = 0;
    }
    else
    {
        // Long format
        cmd.DW1.IndirectBsdDataLength = avcSliceState->dwLength;
        cmd.DW2.IndirectBsdDataStartAddress = sliceParams->slice_data_offset + avcSliceState->dwOffset;
        cmd.DW4.FirstMacroblockMbBitOffset = sliceParams->slice_data_bit_offset;

        if (!avcSliceState->bIntelEntrypointInUse)
        {
            // NAL Header Unit must be passed to HW in the compressed bitstream buffer
            avcSliceState->dwOffset -= (m_osInterface->dwNumNalUnitBytesIncluded - 1);
            cmd.DW1.IndirectBsdDataLength += avcSliceState->dwOffset;
            cmd.DW2.IndirectBsdDataStartAddress -= avcSliceState->dwOffset;
            cmd.DW4.FirstMbByteOffsetOfSliceDataOrSliceHeader = avcSliceState->dwOffset;
        }
    }

    cmd.DW4.FixPrevMbSkipped = 1;

    MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
    sliceInfoParam.presDataBuffer = avcSliceState->presDataBuffer;
    sliceInfoParam.dwSliceIndex = avcSliceState->dwSliceIndex;
    sliceInfoParam.dwTotalBytesConsumed = avcSliceState->dwTotalBytesConsumed;
    sliceInfoParam.dwDataStartOffset[0] = cmd.DW2.IndirectBsdDataStartAddress;
    sliceInfoParam.dwDataStartOffset[1] = sliceParams->slice_data_offset;
    sliceInfoParam.dwDataLength[1] = sliceParams->slice_data_size;

    MHW_MI_CHK_STATUS(m_cpInterface->SetMfxProtectionState(
        m_decodeInUse,
        cmdBuffer,
        nullptr,
        &sliceInfoParam));

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxPakInsertObject(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_PAK_INSERT_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    mhw_vdbox_mfx_g11_X::MFX_PAK_INSERT_OBJECT_CMD cmd;
    uint32_t dwordsUsed = mhw_vdbox_mfx_g11_X::MFX_PAK_INSERT_OBJECT_CMD::dwSize;

    cmd.DW1.SliceHeaderIndicator = params->bSliceHeaderIndicator;

    if (params->bLastPicInSeq || params->bLastPicInStream)    // used by AVC, MPEG2
    {
        dwordsUsed += params->bLastPicInSeq + params->bLastPicInStream;

        cmd.DW0.DwordLength = mhw_vdbox_mfx_g11_X::GetOpLength(dwordsUsed);
        cmd.DW1.BitstreamstartresetResetbitstreamstartingpos = 0;
        cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = 1;
        cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = 1;
        cmd.DW1.EmulationflagEmulationbytebitsinsertenable = 0;
        cmd.DW1.SkipemulbytecntSkipEmulationByteCount = 0;
        // use dwBitSize to pass SrcDataEndingBitInclusion
        cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = params->dwBitSize;
        cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
        cmd.DW1.Headerlengthexcludefrmsize = cmd.DW1.EmulationflagEmulationbytebitsinsertenable ? false : params->bHeaderLengthExcludeFrmSize; // Cannot be set to true if emulation byte bit insertion is enabled

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        if (params->bLastPicInSeq) // only used by AVC, not used by MPEG2
        {
            uint32_t lastPicInSeqData = params->dwLastPicInSeqData;

            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
                cmdBuffer,
                batchBuffer,
                &lastPicInSeqData,
                sizeof(lastPicInSeqData)));
        }

        if (params->bLastPicInStream)  // used by AVC, MPEG2
        {
            uint32_t lastPicInStreamData = params->dwLastPicInStreamData;

            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
                cmdBuffer,
                batchBuffer,
                &lastPicInStreamData,
                sizeof(lastPicInStreamData)));
        }
    }
    else // used by AVC, MPEG2, JPEG
    {
        uint32_t byteSize = (params->dwBitSize + 7) >> 3;
        uint32_t dataBitsInLastDw = params->dwBitSize % 32;

        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        dwordsUsed += ((byteSize + 3) >> 2);
        cmd.DW0.DwordLength = mhw_vdbox_mfx_g11_X::GetOpLength(dwordsUsed);
        cmd.DW1.BitstreamstartresetResetbitstreamstartingpos = params->bResetBitstreamStartingPos;
        cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = params->bEndOfSlice;
        cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = params->bLastHeader;
        cmd.DW1.EmulationflagEmulationbytebitsinsertenable = params->bEmulationByteBitsInsert;
        cmd.DW1.SkipemulbytecntSkipEmulationByteCount = params->uiSkipEmulationCheckCount;
        cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;
        cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
        cmd.DW1.Headerlengthexcludefrmsize = cmd.DW1.EmulationflagEmulationbytebitsinsertenable ? false :
            params->bHeaderLengthExcludeFrmSize; // Cannot be set to true if emulation byte bit insertion is enabled
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        // Add actual data 
        uint8_t* data = (uint8_t*)(params->pBsBuffer->pBase + params->dwOffset);
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, data, byteSize));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxJpegPicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_JPEG_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pJpegPicParams);

    mhw_vdbox_mfx_g11_X::MFX_JPEG_PIC_STATE_CMD cmd;
    auto picParams = params->pJpegPicParams;

    if (picParams->m_chromaType == jpegRGB || picParams->m_chromaType == jpegBGR)
    {
        cmd.DW1.Obj1.InputFormatYuv = jpegYUV444;
    }
    else
    {
        cmd.DW1.Obj1.InputFormatYuv = picParams->m_chromaType;
    }
    cmd.DW1.Obj1.Rotation = picParams->m_rotation;
    cmd.DW1.Obj1.OutputFormatYuv = GetJpegDecodeFormat((MOS_FORMAT)params->dwOutputFormat);

    if (params->dwOutputFormat == Format_NV12)
    {
        if (picParams->m_chromaType == jpegYUV422H2Y ||
            picParams->m_chromaType == jpegYUV422H4Y)
        {
            cmd.DW1.Obj1.VerticalDownSamplingEnable = 1;
        }
        else if (picParams->m_chromaType == jpegYUV422V2Y ||
            picParams->m_chromaType == jpegYUV422V4Y)
        {
            cmd.DW1.Obj1.HorizontalDownSamplingEnable = 1;
        }
    }
    else if (params->dwOutputFormat == Format_UYVY ||
        params->dwOutputFormat == Format_YUY2)
    {
        if (picParams->m_chromaType == jpegYUV420)
        {
            cmd.DW1.Obj1.VerticalUpSamplingEnable = 1;
        }
    }

    cmd.DW2.Obj0.FrameWidthInBlocksMinus1 = params->dwWidthInBlocks;
    cmd.DW2.Obj0.FrameHeightInBlocksMinus1 = params->dwHeightInBlocks;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxJpegEncodePicStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    MhwVdboxJpegEncodePicState *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pJpegEncodePicParams);

    mhw_vdbox_mfx_g11_X::MFX_JPEG_PIC_STATE_CMD cmd;
    auto picParams = params->pJpegEncodePicParams;

    cmd.DW1.Obj0.InputSurfaceFormatYuv = picParams->m_inputSurfaceFormat;

    if (picParams->m_inputSurfaceFormat == codechalJpegY8)
    {
        cmd.DW1.Obj0.OutputMcuStructure = jpegYUV400;
        cmd.DW1.Obj0.PixelsInHorizontalLastMcu = picParams->m_picWidth % 8;
        cmd.DW1.Obj0.PixelsInVerticalLastMcu = picParams->m_picHeight % 8;
    }
    else if (picParams->m_inputSurfaceFormat == codechalJpegNV12)
    {
        cmd.DW1.Obj0.OutputMcuStructure = jpegYUV420;

        if (picParams->m_picWidth % 2 == 0)
        {
            cmd.DW1.Obj0.PixelsInHorizontalLastMcu = picParams->m_picWidth % 16;
        }
        else
        {
            cmd.DW1.Obj0.PixelsInHorizontalLastMcu = ((picParams->m_picWidth % 16) + 1) % 16;
        }

        if (picParams->m_picHeight % 2 == 0)
        {
            cmd.DW1.Obj0.PixelsInVerticalLastMcu = picParams->m_picHeight % 16;
        }
        else
        {
            cmd.DW1.Obj0.PixelsInVerticalLastMcu = ((picParams->m_picHeight % 16) + 1) % 16;
        }
    }
    else if (picParams->m_inputSurfaceFormat == codechalJpegYUY2 ||
        picParams->m_inputSurfaceFormat == codechalJpegUYVY)
    {
        cmd.DW1.Obj0.OutputMcuStructure = jpegYUV422H2Y;

        if (picParams->m_picWidth % 2 == 0)
        {
            cmd.DW1.Obj0.PixelsInHorizontalLastMcu = picParams->m_picWidth % 16;
        }
        else
        {
            cmd.DW1.Obj0.PixelsInHorizontalLastMcu = ((picParams->m_picWidth % 16) + 1) % 16;
        }

        cmd.DW1.Obj0.PixelsInVerticalLastMcu = picParams->m_picHeight % 8;
    }
    else if (picParams->m_inputSurfaceFormat == codechalJpegRGB)
    {
        cmd.DW1.Obj0.OutputMcuStructure = jpegYUV444;
        cmd.DW1.Obj0.PixelsInHorizontalLastMcu = picParams->m_picWidth % 8;
        cmd.DW1.Obj0.PixelsInVerticalLastMcu = picParams->m_picHeight % 8;
    }

    uint32_t horizontalSamplingFactor = GetJpegHorizontalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)picParams->m_inputSurfaceFormat);
    uint32_t verticalSamplingFactor = GetJpegVerticalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)picParams->m_inputSurfaceFormat);
    cmd.DW2.Obj0.FrameWidthInBlocksMinus1 = (((picParams->m_picWidth + (horizontalSamplingFactor * 8 - 1)) / (horizontalSamplingFactor * 8)) * horizontalSamplingFactor) - 1;
    cmd.DW2.Obj0.FrameHeightInBlocksMinus1 = (((picParams->m_picHeight + (verticalSamplingFactor * 8 - 1)) / (verticalSamplingFactor * 8)) * verticalSamplingFactor) - 1;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxJpegFqmCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_QM_PARAMS params,
    uint32_t numQuantTables)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_mfx_g11_X::MFX_FQM_STATE_CMD cmd;

    for (uint32_t i = 0; i < numQuantTables; i++)
    {
        cmd.DW1.Obj0.Avc = i;

        MOS_ZeroMemory(&cmd.ForwardQuantizerMatrix, sizeof(cmd.ForwardQuantizerMatrix));

        auto j = 0;
        // Copy over 32 uint32_t worth of values - Each uint32_t will contain 2 16 bit quantizer values
        // where for the DWordx Bits [15: 0] = 1/QM[0][x] Bits[32:16] = 1/QM[1][x] 
        for (auto k = 0; k < 8; k++)
        {
            for (auto l = k; l < 64; l += 16)
            {
                cmd.ForwardQuantizerMatrix[j] = (((GetReciprocalScalingValue(params->pJpegQuantMatrix->m_quantMatrix[i][l + 8]) & 0xFFFF) << 16)
                    | (GetReciprocalScalingValue(params->pJpegQuantMatrix->m_quantMatrix[i][l]) & 0xFFFF));
                j++;
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfcJpegHuffTableStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_mfx_g11_X::MFC_JPEG_HUFF_TABLE_STATE_CMD cmd;

    cmd.DW1.HuffTableId = params->HuffTableID;

    // cmd DWORDS 2:13 for DC Table
    // Format- 3Bytes: Byte0 for Code length, Byte1 and Byte2 for Code word, and Byte3 for dummy 
    for (auto j = 0; j < JPEG_NUM_HUFF_TABLE_DC_HUFFVAL; j++)
    {
        cmd.DcTable[j] = 0;
        cmd.DcTable[j] = (params->pDCCodeLength[j] & 0xFF) | ((params->pDCCodeValues[j] & 0xFFFF) << 8);
    }

    // cmd DWORDS 14:175 for AC table 
    // Format- 3Bytes: Byte0 for Code length, Byte1 and Byte2 for Code word, and Byte3 for dummy 
    for (auto j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; j++)
    {
        cmd.AcTable[j] = 0;
        cmd.AcTable[j] = (params->pACCodeLength[j] & 0xFF)
            | ((params->pACCodeValues[j] & 0xFFFF) << 8);
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfcJpegScanObjCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    MhwVdboxJpegScanParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pJpegEncodeScanParams);

    mhw_vdbox_mfx_g11_X::MFC_JPEG_SCAN_OBJECT_CMD cmd;

    uint32_t horizontalSamplingFactor = GetJpegHorizontalSamplingFactorForY(params->inputSurfaceFormat);
    uint32_t verticalSamplingFactor = GetJpegVerticalSamplingFactorForY(params->inputSurfaceFormat);
    cmd.DW1.McuCount = ((params->dwPicWidth + (horizontalSamplingFactor * 8 - 1)) / (horizontalSamplingFactor * 8))
        * ((params->dwPicHeight + (verticalSamplingFactor * 8 - 1)) / (verticalSamplingFactor * 8));
    cmd.DW2.RestartInterval = params->pJpegEncodeScanParams->m_restartInterval;
    cmd.DW2.IsLastScan = 1; // Always 1 since there is only 1 scan in the JPEG frame 
    cmd.DW2.HeadPresentFlag = 1; // There will always be MFC_JPEG_PAK_INSERT_OBJECT commands sent 

    for (auto i = 0; i < jpegNumComponent; i++)
    {
        cmd.DW2.HuffmanDcTable |= (params->pJpegEncodeScanParams->m_dcCodingTblSelector[i]) << i;
        cmd.DW2.HuffmanAcTable |= (params->pJpegEncodeScanParams->m_acCodingTblSelector[i]) << i;
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxDecodeVp8PicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_VP8_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_mfx_g11_X::MFX_VP8_PIC_STATE_CMD cmd;
    auto vp8PicParams = params->pVp8PicParams;
    auto vp8IqMatrixParams = params->pVp8IqMatrixParams;

    cmd.DW1.FrameWidthMinus1 = vp8PicParams->wFrameWidthInMbsMinus1;
    cmd.DW1.FrameHeightMinus1 = vp8PicParams->wFrameHeightInMbsMinus1;
    cmd.DW2.McFilterSelect = (vp8PicParams->version != 0);
    cmd.DW2.ChromaFullPixelMcFilterMode = (vp8PicParams->version == 3);
    cmd.DW2.Dblkfiltertype = vp8PicParams->filter_type;
    cmd.DW2.Skeyframeflag = vp8PicParams->key_frame;
    cmd.DW2.SegmentationIdStreamoutEnable =
        (vp8PicParams->segmentation_enabled) && (vp8PicParams->update_mb_segmentation_map);
    cmd.DW2.SegmentationIdStreaminEnable =
        (vp8PicParams->segmentation_enabled) && !(vp8PicParams->update_mb_segmentation_map);
    cmd.DW2.SegmentEnableFlag = vp8PicParams->segmentation_enabled;
    cmd.DW2.UpdateMbsegmentMapFlag =
        (vp8PicParams->segmentation_enabled) ? vp8PicParams->update_mb_segmentation_map : 0;
    cmd.DW2.MbNocoeffSkipflag = vp8PicParams->mb_no_coeff_skip;
    cmd.DW2.ModeReferenceLoopFilterDeltaEnabled = vp8PicParams->loop_filter_adj_enable;
    cmd.DW2.GoldenRefPictureMvSignbiasFlag = vp8PicParams->sign_bias_golden;
    cmd.DW2.AlternateRefPicMvSignbiasFlag = vp8PicParams->sign_bias_alternate;
    cmd.DW2.DeblockSharpnessLevel = vp8PicParams->ucSharpnessLevel;
    cmd.DW3.DblkfilterlevelForSegment3 = vp8PicParams->ucLoopFilterLevel[3];
    cmd.DW3.DblkfilterlevelForSegment2 = vp8PicParams->ucLoopFilterLevel[2];
    cmd.DW3.DblkfilterlevelForSegment1 = vp8PicParams->ucLoopFilterLevel[1];
    cmd.DW3.DblkfilterlevelForSegment0 = vp8PicParams->ucLoopFilterLevel[0];

    uint32_t i = 0;
    uint32_t j = 0;
    cmd.DW4.dec.QuantizerValue0Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW4.dec.QuantizerValue0Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 0;
    j = 2;
    cmd.DW5.dec.QuantizerValue0Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW5.dec.QuantizerValue0Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 0;
    j = 4;
    cmd.DW6.dec.QuantizerValue0Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW6.dec.QuantizerValue0Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 1;
    j = 0;
    cmd.DW7.dec.QuantizerValue1Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW7.dec.QuantizerValue1Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 1;
    j = 2;
    cmd.DW8.QuantizerValue1Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW8.QuantizerValue1Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 1;
    j = 4;
    cmd.DW9.QuantizerValue1Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW9.QuantizerValue1Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 2;
    j = 0;
    cmd.DW10.QuantizerValue2Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW10.QuantizerValue2Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 2;
    j = 2;
    cmd.DW11.QuantizerValue2Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW11.QuantizerValue2Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 2;
    j = 4;
    cmd.DW12.QuantizerValue2Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW12.QuantizerValue2Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 3;
    j = 0;
    cmd.DW13.QuantizerValue3Blocktype0Y1Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW13.QuantizerValue3Blocktype1Y1Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 3;
    j = 2;
    cmd.DW14.QuantizerValue3Blocktype2Uvdc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW14.QuantizerValue3Blocktype3Uvac = vp8IqMatrixParams->quantization_values[i][j + 1];

    i = 3;
    j = 4;
    cmd.DW15.QuantizerValue3Blocktype4Y2Dc = vp8IqMatrixParams->quantization_values[i][j];
    cmd.DW15.QuantizerValue3Blocktype5Y2Ac = vp8IqMatrixParams->quantization_values[i][j + 1];

    cmd.DW19.Mbsegmentidtreeprobs2 = vp8PicParams->cMbSegmentTreeProbs[2];
    cmd.DW19.Mbsegmentidtreeprobs1 = vp8PicParams->cMbSegmentTreeProbs[1];
    cmd.DW19.Mbsegmentidtreeprobs0 = vp8PicParams->cMbSegmentTreeProbs[0];
    cmd.DW20.Mbnocoeffskipfalseprob = vp8PicParams->ucProbSkipFalse;
    cmd.DW20.Intrambprob = vp8PicParams->ucProbIntra;
    cmd.DW20.Interpredfromlastrefprob = vp8PicParams->ucProbLast;
    cmd.DW20.Interpredfromgrefrefprob = vp8PicParams->ucProbGolden;
    cmd.DW21.Ymodeprob3 = vp8PicParams->ucYModeProbs[3];
    cmd.DW21.Ymodeprob2 = vp8PicParams->ucYModeProbs[2];
    cmd.DW21.Ymodeprob1 = vp8PicParams->ucYModeProbs[1];
    cmd.DW21.Ymodeprob0 = vp8PicParams->ucYModeProbs[0];
    cmd.DW22.Uvmodeprob2 = vp8PicParams->ucUvModeProbs[2];
    cmd.DW22.Uvmodeprob1 = vp8PicParams->ucUvModeProbs[1];
    cmd.DW22.Uvmodeprob0 = vp8PicParams->ucUvModeProbs[0];

    i = 0;
    j = 0;
    cmd.DW23.Mvupdateprobs00 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW23.Mvupdateprobs01 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW23.Mvupdateprobs02 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW23.Mvupdateprobs03 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 4;
    cmd.DW24.Mvupdateprobs04 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW24.Mvupdateprobs05 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW24.Mvupdateprobs06 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW24.Mvupdateprobs07 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 8;
    cmd.DW25.Mvupdateprobs08 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW25.Mvupdateprobs09 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW25.Mvupdateprobs010 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW25.Mvupdateprobs011 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 12;
    cmd.DW26.Mvupdateprobs012 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW26.Mvupdateprobs013 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW26.Mvupdateprobs014 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW26.Mvupdateprobs015 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 0;
    j = 16;
    cmd.DW27.Mvupdateprobs016 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW27.Mvupdateprobs017 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW27.Mvupdateprobs018 = vp8PicParams->ucMvUpdateProb[i][j + 2];

    i = 1;
    j = 0;
    cmd.DW28.Mvupdateprobs10 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW28.Mvupdateprobs11 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW28.Mvupdateprobs12 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW28.Mvupdateprobs13 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 4;
    cmd.DW29.Mvupdateprobs14 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW29.Mvupdateprobs15 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW29.Mvupdateprobs16 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW29.Mvupdateprobs17 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 8;
    cmd.DW30.Mvupdateprobs18 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW30.Mvupdateprobs19 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW30.Mvupdateprobs110 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW30.Mvupdateprobs111 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 12;
    cmd.DW31.Mvupdateprobs112 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW31.Mvupdateprobs113 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW31.Mvupdateprobs114 = vp8PicParams->ucMvUpdateProb[i][j + 2];
    cmd.DW31.Mvupdateprobs115 = vp8PicParams->ucMvUpdateProb[i][j + 3];

    i = 1;
    j = 16;
    cmd.DW32.Mvupdateprobs116 = vp8PicParams->ucMvUpdateProb[i][j];
    cmd.DW32.Mvupdateprobs117 = vp8PicParams->ucMvUpdateProb[i][j + 1];
    cmd.DW32.Mvupdateprobs118 = vp8PicParams->ucMvUpdateProb[i][j + 2];

    cmd.DW33.Reflfdelta0ForIntraFrame = vp8PicParams->cRefLfDelta[0];
    cmd.DW33.Reflfdelta1ForLastFrame = vp8PicParams->cRefLfDelta[1];
    cmd.DW33.Reflfdelta2ForGoldenFrame = vp8PicParams->cRefLfDelta[2];
    cmd.DW33.Reflfdelta3ForAltrefFrame = vp8PicParams->cRefLfDelta[3];
    cmd.DW34.Modelfdelta0ForBPredMode = vp8PicParams->cModeLfDelta[0];
    cmd.DW34.Modelfdelta1ForZeromvMode = vp8PicParams->cModeLfDelta[1];
    cmd.DW34.Modelfdelta2ForNearestNearAndNewMode = vp8PicParams->cModeLfDelta[2];
    cmd.DW34.Modelfdelta3ForSplitmvMode = vp8PicParams->cModeLfDelta[3];

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_VP8_PIC;

    resourceParams.presResource = params->presCoefProbBuffer;
    resourceParams.dwOffset = params->dwCoefProbTableOffset;
    resourceParams.pdwCmd = &(cmd.DW16.Value);
    resourceParams.dwLocationInCmd = 16;
    resourceParams.bIsWritable = false;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    if (vp8PicParams->segmentation_enabled)
    {
        resourceParams.presResource = params->presSegmentationIdStreamBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW35.Value);
        resourceParams.dwLocationInCmd = 35;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxEncodeVp8PicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_VP8_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pEncodeVP8SeqParams);
    MHW_MI_CHK_NULL(params->pEncodeVP8PicParams);
    MHW_MI_CHK_NULL(params->pEncodeVP8QuantData);

    mhw_vdbox_mfx_g11_X::MFX_VP8_PIC_STATE_CMD cmd;
    auto vp8SeqParams = params->pEncodeVP8SeqParams;
    auto vp8PicParams = params->pEncodeVP8PicParams;
    auto vp8QuantData = params->pEncodeVP8QuantData;

    cmd.DW1.FrameWidthMinus1 = params->wPicWidthInMb - 1;
    cmd.DW1.FrameHeightMinus1 = params->wPicHeightInMb - 1;

    cmd.DW2.McFilterSelect = (vp8PicParams->version != 0);
    cmd.DW2.ChromaFullPixelMcFilterMode = (vp8PicParams->version == 3);
    cmd.DW2.Dblkfiltertype = vp8PicParams->filter_type;
    cmd.DW2.Skeyframeflag = !vp8PicParams->frame_type;
    cmd.DW2.SegmentEnableFlag = vp8PicParams->segmentation_enabled;
    cmd.DW2.UpdateMbsegmentMapFlag =
        (vp8PicParams->segmentation_enabled) ? vp8PicParams->update_mb_segmentation_map : 0;
    cmd.DW2.MbNocoeffSkipflag = vp8PicParams->mb_no_coeff_skip;
    cmd.DW2.ModeReferenceLoopFilterDeltaEnabled = vp8PicParams->loop_filter_adj_enable;
    cmd.DW2.GoldenRefPictureMvSignbiasFlag = vp8PicParams->sign_bias_golden;
    cmd.DW2.AlternateRefPicMvSignbiasFlag = vp8PicParams->sign_bias_alternate;
    cmd.DW2.DeblockSharpnessLevel = vp8PicParams->sharpness_level;
    cmd.DW2.Log2NumOfPartition = vp8PicParams->CodedCoeffTokenPartition;

    cmd.DW3.DblkfilterlevelForSegment3 = vp8PicParams->loop_filter_level[3];
    cmd.DW3.DblkfilterlevelForSegment2 = vp8PicParams->loop_filter_level[2];
    cmd.DW3.DblkfilterlevelForSegment1 = vp8PicParams->loop_filter_level[1];
    cmd.DW3.DblkfilterlevelForSegment0 = vp8PicParams->loop_filter_level[0];

    //Y1 AC is the reference.
    cmd.DW4.enc.Seg0Qindex = vp8QuantData->QIndex[0];
    cmd.DW4.enc.Seg1Qindex = vp8QuantData->QIndex[1];
    cmd.DW4.enc.Seg2Qindex = vp8QuantData->QIndex[2];
    cmd.DW4.enc.Seg3Qindex = vp8QuantData->QIndex[3];

    cmd.DW5.enc.Y2DcQindexDelta = vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_DC];
    cmd.DW5.enc.Y2AcQindexDeltaSign = (vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_DC] < 0) ? 1 : 0;
    cmd.DW5.enc.Y2AcQindexDelta = vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_AC];
    cmd.DW5.enc.Y2AcQindexSign = (vp8QuantData->QIndexDelta[VP8_QINDEX_Y2_AC] < 0) ? 1 : 0;
    cmd.DW5.enc.UvdcQindexDelta = vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC];
    cmd.DW5.enc.UvdcQindexDeltaSign = (vp8QuantData->QIndexDelta[VP8_QINDEX_UV_DC] < 0) ? 1 : 0;
    cmd.DW5.enc.UvacQindexdelta = vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC];
    cmd.DW5.enc.UvacQindexDeltaSign = (vp8QuantData->QIndexDelta[VP8_QINDEX_UV_AC] < 0) ? 1 : 0;

    cmd.DW6.enc.Y1DcQindexDelta = vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC];
    cmd.DW6.enc.Y1DcQindexDeltaSign = (vp8QuantData->QIndexDelta[VP8_QINDEX_Y1_DC] < 0) ? 1 : 0;

    cmd.DW7.enc.ClampQindexLow = vp8PicParams->ClampQindexLow;
    cmd.DW7.enc.ClampQindexHigh = vp8PicParams->ClampQindexHigh;

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_VP8_PIC;

    cmd.DW19.Mbsegmentidtreeprobs2 = SegTreeProbs[2];
    cmd.DW19.Mbsegmentidtreeprobs1 = SegTreeProbs[1];
    cmd.DW19.Mbsegmentidtreeprobs0 = SegTreeProbs[0];

    //The following probs need to be updated by the kernel when ready.
    cmd.DW20.Mbnocoeffskipfalseprob = 224;
    cmd.DW20.Intrambprob = 63;
    cmd.DW20.Interpredfromlastrefprob = 255;
    cmd.DW20.Interpredfromgrefrefprob = 128;

    cmd.DW21.Ymodeprob3 = YModeProb[3];
    cmd.DW21.Ymodeprob2 = YModeProb[2];
    cmd.DW21.Ymodeprob1 = YModeProb[1];
    cmd.DW21.Ymodeprob0 = YModeProb[0];

    cmd.DW22.Uvmodeprob2 = UVModeProb[2];
    cmd.DW22.Uvmodeprob1 = UVModeProb[1];
    cmd.DW22.Uvmodeprob0 = UVModeProb[0];

    uint32_t i = 0;
    uint32_t j = 0;
    cmd.DW23.Mvupdateprobs00 = DefaultMvContext[i][j];
    cmd.DW23.Mvupdateprobs01 = DefaultMvContext[i][j + 1];
    cmd.DW23.Mvupdateprobs02 = DefaultMvContext[i][j + 2];
    cmd.DW23.Mvupdateprobs03 = DefaultMvContext[i][j + 3];

    i = 0;
    j = 4;
    cmd.DW24.Mvupdateprobs04 = DefaultMvContext[i][j];
    cmd.DW24.Mvupdateprobs05 = DefaultMvContext[i][j + 1];
    cmd.DW24.Mvupdateprobs06 = DefaultMvContext[i][j + 2];
    cmd.DW24.Mvupdateprobs07 = DefaultMvContext[i][j + 3];

    i = 0;
    j = 8;
    cmd.DW25.Mvupdateprobs08 = DefaultMvContext[i][j];
    cmd.DW25.Mvupdateprobs09 = DefaultMvContext[i][j + 1];
    cmd.DW25.Mvupdateprobs010 = DefaultMvContext[i][j + 2];
    cmd.DW25.Mvupdateprobs011 = DefaultMvContext[i][j + 3];

    i = 0;
    j = 12;
    cmd.DW26.Mvupdateprobs012 = DefaultMvContext[i][j];
    cmd.DW26.Mvupdateprobs013 = DefaultMvContext[i][j + 1];
    cmd.DW26.Mvupdateprobs014 = DefaultMvContext[i][j + 2];
    cmd.DW26.Mvupdateprobs015 = DefaultMvContext[i][j + 3];

    i = 0;
    j = 16;
    cmd.DW27.Mvupdateprobs016 = DefaultMvContext[i][j];
    cmd.DW27.Mvupdateprobs017 = DefaultMvContext[i][j + 1];
    cmd.DW27.Mvupdateprobs018 = DefaultMvContext[i][j + 2];

    i = 1;
    j = 0;
    cmd.DW28.Mvupdateprobs10 = DefaultMvContext[i][j];
    cmd.DW28.Mvupdateprobs11 = DefaultMvContext[i][j + 1];
    cmd.DW28.Mvupdateprobs12 = DefaultMvContext[i][j + 2];
    cmd.DW28.Mvupdateprobs13 = DefaultMvContext[i][j + 3];

    i = 1;
    j = 4;
    cmd.DW29.Mvupdateprobs14 = DefaultMvContext[i][j];
    cmd.DW29.Mvupdateprobs15 = DefaultMvContext[i][j + 1];
    cmd.DW29.Mvupdateprobs16 = DefaultMvContext[i][j + 2];
    cmd.DW29.Mvupdateprobs17 = DefaultMvContext[i][j + 3];

    i = 1;
    j = 8;
    cmd.DW30.Mvupdateprobs18 = DefaultMvContext[i][j];
    cmd.DW30.Mvupdateprobs19 = DefaultMvContext[i][j + 1];
    cmd.DW30.Mvupdateprobs110 = DefaultMvContext[i][j + 2];
    cmd.DW30.Mvupdateprobs111 = DefaultMvContext[i][j + 3];

    i = 1;
    j = 12;
    cmd.DW31.Mvupdateprobs112 = DefaultMvContext[i][j];
    cmd.DW31.Mvupdateprobs113 = DefaultMvContext[i][j + 1];
    cmd.DW31.Mvupdateprobs114 = DefaultMvContext[i][j + 2];
    cmd.DW31.Mvupdateprobs115 = DefaultMvContext[i][j + 3];

    i = 1;
    j = 16;
    cmd.DW32.Mvupdateprobs116 = DefaultMvContext[i][j];
    cmd.DW32.Mvupdateprobs117 = DefaultMvContext[i][j + 1];
    cmd.DW32.Mvupdateprobs118 = DefaultMvContext[i][j + 2];

    cmd.DW33.Reflfdelta0ForIntraFrame = vp8PicParams->ref_lf_delta[0];
    cmd.DW33.Reflfdelta1ForLastFrame = vp8PicParams->ref_lf_delta[1];
    cmd.DW33.Reflfdelta2ForGoldenFrame = vp8PicParams->ref_lf_delta[2];
    cmd.DW33.Reflfdelta3ForAltrefFrame = vp8PicParams->ref_lf_delta[3];

    cmd.DW34.Modelfdelta0ForBPredMode = vp8PicParams->mode_lf_delta[0];
    cmd.DW34.Modelfdelta1ForZeromvMode = vp8PicParams->mode_lf_delta[1];
    cmd.DW34.Modelfdelta2ForNearestNearAndNewMode = vp8PicParams->mode_lf_delta[2];
    cmd.DW34.Modelfdelta3ForSplitmvMode = vp8PicParams->mode_lf_delta[3];

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::InitMfxVp8EncoderCfgCmd(
    PMOS_RESOURCE cfgCmdBuffer,
    PMHW_VDBOX_VP8_ENCODER_CFG_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cfgCmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pEncodeVP8PicParams);
    MHW_MI_CHK_NULL(params->pEncodeVP8SeqParams);
    MHW_MI_CHK_NULL(params->pEncodeVP8QuantData);

    auto picParams = params->pEncodeVP8PicParams;
    auto seqParams = params->pEncodeVP8SeqParams;
    auto quantData = params->pEncodeVP8QuantData;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, cfgCmdBuffer, &lockFlags);
    MHW_MI_CHK_NULL(data);
    data += params->dwCfgCmdOffset;

    MOS_ZeroMemory(data, params->dwCfgBufferSize);

    mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD *cmd = (mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD *)data;

    cmd->DW0.DwordLength = mhw_vdbox_mfx_g11_X::GetOpLength(mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD::dwSize);
    cmd->DW0.SubOpcodeB = mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD::SUB_OPCODE_B_MFXVP8ENCODERCFG;
    cmd->DW0.SubOpcodeA = mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD::SUB_OPCODE_A_VP8COMMON;
    cmd->DW0.MediaCommandOpcode = mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD::MEDIA_COMMAND_OPCODE_VP8;
    cmd->DW0.Pipeline = mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD::PIPELINE_VIDEOCODEC;
    cmd->DW0.CommandType = mhw_vdbox_mfx_g11_X::MFX_VP8_Encoder_CFG_CMD::COMMAND_TYPE_PARALLELVIDEOPIPE;

    cmd->DW1.Value = 0;
    cmd->DW2.Value = 0;
    cmd->DW3.Value = 0;
    cmd->DW4.Value = 0;
    cmd->DW5.Value = 0;
    cmd->DW6.Value = 0;
    cmd->DW7.Value = 0;
    cmd->DW8.Value = 0;
    cmd->DW9.Value = 0;
    cmd->DW10.Value = 0;
    cmd->DW11.Value = 0;
    cmd->DW12.Value = 0;
    cmd->DW13.Value = 0;
    cmd->DW14.Value = 0;
    cmd->DW15.Value = 0;
    cmd->DW16.Value = 0;
    cmd->DW17.Value = 0;
    cmd->DW18.Value = 0;
    cmd->DW19.Value = 0;
    cmd->DW20.Value = 0;
    cmd->DW22.Value = 0;
    cmd->DW23.Value = 0;
    cmd->DW24.Value = 0;
    cmd->DW25.Value = 0;
    cmd->DW26.Value = 0;
    cmd->DW27.Value = 0;
    cmd->DW28.Value = 0;
    cmd->DW29.Value = 0;

    cmd->DW1.RateControlInitialPass = params->bFirstPass ? 1 : 0;
    cmd->DW1.PerSegmentDeltaQindexLoopfilterDisable = (params->bFirstPass || !params->bBRCEnabled);
    cmd->DW1.TokenStatisticsOutputEnable = 1;

    if (picParams->segmentation_enabled)
    {
        cmd->DW1.UpdateSegmentFeatureDataFlag = 1;
    }

    if (params->bBRCEnabled)
    {
        //Add BRC init below
        cmd->DW2.MaxFrameBitCountRateControlEnableMask = 1;
        cmd->DW2.MinFrameBitCountRateControlEnableMask = 1;
    }

    cmd->DW22.ShowFrame = picParams->show_frame;
    cmd->DW22.BitstreamFormatVersion = picParams->version;

    cmd->DW23.HorizontalSizeCode = ((seqParams->FrameWidthScale << 14) | seqParams->FrameWidth);
    cmd->DW23.VerticalSizeCode = ((seqParams->FrameHeightScale << 14) | seqParams->FrameHeight);

    //Add batch buffer end command
    data += sizeof(*cmd);

    mhw_mi_g11_X::MI_BATCH_BUFFER_END_CMD *miBatchBufferEndCmd = (mhw_mi_g11_X::MI_BATCH_BUFFER_END_CMD *)data;
    *miBatchBufferEndCmd = mhw_mi_g11_X::MI_BATCH_BUFFER_END_CMD();

    MHW_MI_CHK_STATUS(m_osInterface->pfnUnlockResource(
        m_osInterface,
        cfgCmdBuffer));

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceG11::AddMfxVp8BspBufBaseAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    if (params->dwPartitions == 0 || params->dwPartitions > m_vp8MaxNumPartitions)
    {
        MHW_ASSERTMESSAGE("Invalid Partitions.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    mhw_vdbox_mfx_g11_X::MFX_VP8_BSP_BUF_BASE_ADDR_STATE_CMD cmd;

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_BSP_BUF_BASE_ADDR;

    if (params->presFrameHeaderBuffer)
    {
        cmd.DW3.Value =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;

        resourceParams.presResource = params->presFrameHeaderBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presPakIntermediateBuffer)
    {
        cmd.DW6.Value =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

        resourceParams.presResource = params->presPakIntermediateBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        uint32_t partitionSize = (params->dwPakIntermediateTokenSize / params->dwPartitions);
        cmd.DW7.IntermediateBufferPartition1Offset = MOS_ALIGN_CEIL(params->dwPakIntermediatePartition0Size, 64);
        cmd.DW8.IntermediateBufferPartition2Offset = MOS_ALIGN_CEIL(cmd.DW7.IntermediateBufferPartition1Offset + partitionSize, 64);
        cmd.DW9.IntermediateBufferPartition3Offset = MOS_ALIGN_CEIL(cmd.DW8.IntermediateBufferPartition2Offset + partitionSize, 64);
        cmd.DW10.IntermediateBufferPartition4Offset = MOS_ALIGN_CEIL(cmd.DW9.IntermediateBufferPartition3Offset + partitionSize, 64);
        cmd.DW11.IntermediateBufferPartition5Offset = MOS_ALIGN_CEIL(cmd.DW10.IntermediateBufferPartition4Offset + partitionSize, 64);
        cmd.DW12.IntermediateBufferPartition6Offset = MOS_ALIGN_CEIL(cmd.DW11.IntermediateBufferPartition5Offset + partitionSize, 64);
        cmd.DW13.IntermediateBufferPartition7Offset = MOS_ALIGN_CEIL(cmd.DW12.IntermediateBufferPartition6Offset + partitionSize, 64);
        cmd.DW14.IntermediateBufferPartition8Offset = MOS_ALIGN_CEIL(cmd.DW13.IntermediateBufferPartition7Offset + partitionSize, 64);

        cmd.DW15.IntermediateBufferMaxSize = params->dwPakIntermediateTokenSize + params->dwPakIntermediatePartition0Size;
    }

    if (params->presPakFinalFrameBuffer)
    {
        cmd.DW18.Value =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

        resourceParams.presResource = params->presPakFinalFrameBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW16.Value);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presCoeffProbsBuffer)
    {
        cmd.DW25.Value =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;

        resourceParams.presResource = params->presCoeffProbsBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW23.Value);
        resourceParams.dwLocationInCmd = 23;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presTokenStatisticsBuffer)
    {
        cmd.DW28.Value =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;

        resourceParams.presResource = params->presTokenStatisticsBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW26.Value);
        resourceParams.dwLocationInCmd = 26;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presBsdMpcRowStoreScratchBuffer)
    {
        cmd.DW31.Value =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presBsdMpcRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW29_30.Value[0]);
        resourceParams.dwLocationInCmd = 29;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}


