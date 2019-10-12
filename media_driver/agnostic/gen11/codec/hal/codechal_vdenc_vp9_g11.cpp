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

/*
*  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

//!
//! \file     codechal_vdenc_vp9_g11.cpp
//! \brief    VP9 VDENC encoder for GEN11.
//!
#include "codechal_vdenc_vp9_g11.h"
#include "codechal_kernel_header_g11.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g10.h"
#include "igcodeckrn_g11.h"
#endif
#include "mhw_vdbox_hcp_g11_X.h"
#include "mhw_vdbox_vdenc_g11_X.h"
#include "mhw_vdbox_g11_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g11_X.h"
#include "codechal_huc_cmd_initializer_g11.h"

const uint32_t CodechalVdencVp9StateG11::meCurbeInit[48] =
    {
    0x00000000, 0x00200010, 0x00003939, 0x77a43000, 0x00000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};

MOS_STATUS CodechalVdencVp9StateG11::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncodeWriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_VDBOX_NUM_ID, m_numPipe);
    CodecHalEncodeWriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_VE_ID, m_useVirtualEngine);
    CodecHalEncodeWriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_HW_STITCH, m_enableTileStitchByHW);
    CodecHalEncodeWriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));
#endif

    return eStatus;
}

CodechalVdencVp9StateG11::CodechalVdencVp9StateG11(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalVdencVp9State(hwInterface, debugInterface, standardInfo)
{
    m_useCommonKernel = true;
    m_isTilingSupported      = true;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t *)IGCODECKRN_G11;
#endif

    // KUID for HME + DS + SW SCOREBOARD Kernel
    m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;

    // We need the DYS kernel inside AllVP9Enc_CNLA0, for SHME we need kernels inside
    // HME_DS_SCOREBOARD_KERNEL, so we need to allocate enough size in ISH for both.
    pfnGetKernelHeaderAndSize = GetCommonKernelHeaderAndSizeG11;

    uint8_t* binary = nullptr;
    uint32_t combinedKernelSize = 0;
    m_scalabilityState = nullptr;

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_VP9_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_ENCODE_VP9_INIT_DSH_SIZE;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (m_useCommonKernel)
    {
        m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
        eStatus = CodecHalGetKernelBinaryAndSize(
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
            (uint8_t*)IGCODECKRN_G11,
#else
            nullptr,
#endif
            m_kuidCommon,
            &binary,
            &combinedKernelSize);
        CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

        m_hwInterface->GetStateHeapSettings()->dwIshSize +=
            MOS_ALIGN_CEIL(combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    }

    // Initialize to 0
    MOS_ZeroMemory(&m_realCmdBuffer, sizeof(m_realCmdBuffer));
    MOS_ZeroMemory(m_veBatchBuffer, sizeof(m_veBatchBuffer));
    MOS_ZeroMemory(&m_hcpScalabilitySyncBuffer, sizeof(m_hcpScalabilitySyncBuffer));

    for (auto i = 0; i < m_numUncompressedSurface; i++)
    {
        MOS_ZeroMemory(&m_tileRecordBuffer[i].sResource, sizeof(m_tileRecordBuffer[i].sResource));
    }
    for (auto i = 0; i < m_numUncompressedSurface; i++)
    {
        MOS_ZeroMemory(&m_tileStatsPakIntegrationBuffer[i].sResource, sizeof(m_tileStatsPakIntegrationBuffer[i].sResource));
    }
    MOS_ZeroMemory(&m_frameStatsPakIntegrationBuffer.sResource, sizeof(m_frameStatsPakIntegrationBuffer.sResource));
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        for (auto j = 0; j < m_brcMaxNumPasses; j++)
        {
            MOS_ZeroMemory(&m_hucPakIntDmemBuffer[i][j], sizeof(m_hucPakIntDmemBuffer[i][j]));
        }
    }
    MOS_ZeroMemory(&m_hucPakIntDummyBuffer, sizeof(m_hucPakIntDummyBuffer));
    MOS_ZeroMemory(&m_hucPakIntBrcDataBuffer, sizeof(m_hucPakIntBrcDataBuffer));
    MOS_ZeroMemory(&m_resPipeStartSync, sizeof(m_resPipeStartSync));
    MOS_ZeroMemory(&m_resDelayMinus, sizeof(m_resDelayMinus));
    for (auto i = 0; i < m_maxNumPipes; i++)
    {
        MOS_ZeroMemory(&m_stitchWaitSemaphoreMem[i], sizeof(m_stitchWaitSemaphoreMem[i]));
    }

    for (auto i = 0; i < 3; i++)
    {
        MOS_ZeroMemory(&m_refPicList0[i], sizeof(m_refPicList0[i]));
    }

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);
}

MOS_STATUS CodechalVdencVp9StateG11::GetSystemPipeNumberCommon()
{
    MOS_STATUS eStatus   = MOS_STATUS_SUCCESS;
    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_DISABLE_SCALABILITY,
        &userFeatureData);

    //Disable scalability temporarily
    bool disableScalability = true;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = userFeatureData.i32Data ? true : false;
    }

    MEDIA_SYSTEM_INFO *gtSystemInfo = m_gtSystemInfo;

    if (gtSystemInfo && disableScalability == false)
    {
        // Both VE mode and media solo mode should be able to get the VDBOX number via the same interface
        m_numVdbox = (uint8_t)(gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        m_numVdbox = 1;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ExecuteDysSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_nalUnitParams);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        PerfTagSetting perfTag;
        CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);
    }

    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
    secondLevelBatchBuffer.dwOffset = 0;
    secondLevelBatchBuffer.bSecondLevel = true;
    if (!m_hucEnabled)
    {
        secondLevelBatchBuffer.OsResource = m_resHucPakInsertUncompressedHeaderReadBuffer;
    }
    else
    {
        secondLevelBatchBuffer.OsResource = m_resHucPakInsertUncompressedHeaderWriteBuffer;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &secondLevelBatchBuffer));

    // Setup Tile level PAK commands
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_tileParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9StateG11::SetTileData());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG11 *>(m_hcpInterface)->AddHcpTileCodingCmd(&cmdBuffer, &m_tileParams[0]));

    //Disbale Frame Tracking Header for this submission as this is not the last submission
    bool isFrameTrackingHeaderSet = cmdBuffer.Attributes.bEnableMediaFrameTracking;
    cmdBuffer.Attributes.bEnableMediaFrameTracking = false;

    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    secondLevelBatchBuffer.OsResource = m_resMbCodeSurface;
    secondLevelBatchBuffer.dwOffset = 0;
    secondLevelBatchBuffer.bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &secondLevelBatchBuffer));

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    // MFXPipeDone should not be set for tail insertion
    vdPipelineFlushParams.Flags.bWaitDoneMFX =
        (m_lastPicInStream || m_lastPicInSeq) ? 0 : 1;
    vdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
    vdPipelineFlushParams.Flags.bFlushHEVC = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
        
    if (!m_scalableMode) // single pipe mode can read the info from MMIO register. Otherwise, we have to use the tile size statistic buffer
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));
    }

    if (m_currPass >= (m_numPasses - 1))    // Last pass and the one before last
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string currPassName = "PAK_PASS" + std::to_string((int)m_currPass);
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        CODECHAL_NUM_MEDIA_STATES,
        currPassName.data())));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (m_waitForEnc &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_waitForEnc = false;
    }

    if (m_currPass >= (m_numPasses - 1))    // Last pass and the one before last
    {
        bool renderFlags;

        renderFlags = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderFlags));
    }

    //Restore the frame tracking header for the further passes and submissions
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));
    cmdBuffer.Attributes.bEnableMediaFrameTracking = isFrameTrackingHeaderSet;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        if (m_vp9PicParams->PicFlags.fields.segmentation_enabled) {
            //CodecHal_DbgDumpEncodeVp9SegmentStreamout(m_debugInterface, m_encoder);
            //m_debugInterface->DumpBuffer(
            //    (PCODECHAL_ENCODE_VP9_STATE)pvStandardState.resVdencSegmentMapStreamOut,
            //    CodechalDbgAttr::attrOutput,
            //    "SegMap_Out",
            //    CODECHAL_CACHELINE_SIZE * MOS_ROUNDUP_DIVIDE(pEncoder->dwFrameHeight, 64) * MOS_ROUNDUP_DIVIDE(pEncoder->dwFrameWidth, 64),
            //    0,
            //    CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_RECON);
        } if (m_mmcState) {
            m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
        });

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderEngineInterface->GetHwCaps());

    uint32_t combinedKernelSize = 0;
    uint8_t *binary             = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuidCommon,
        &binary,
        &combinedKernelSize));

    for (uint32_t krnStateIdx = 0; krnStateIdx < CodechalEncoderState::CODECHAL_ENCODE_ME_IDX_NUM; krnStateIdx++)
    {
        CODECHAL_KERNEL_HEADER currKrnHeader;
        PMHW_KERNEL_STATE      kernelStatePtr = &m_meKernelStates[krnStateIdx];
        uint32_t               kernelSize     = combinedKernelSize;
        // For dual pipe HME-P kernel state is loaded for both ids
        // Non legacy streamin is a new hevc vp9 streamin kernel
        EncOperation encOperation = (krnStateIdx > 0 && m_vdencEnabled) ? (m_useNonLegacyStreamin ? VDENC_STREAMIN_HEVC : VDENC_ME) : ENC_ME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG11(
            binary,
            encOperation,
            (encOperation == ENC_ME) ? krnStateIdx : 0,
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount     = CODECHAL_ENCODE_ME_NUM_SURFACES_G11;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(MeCurbe);
        kernelStatePtr->KernelParams.iBlockWidth  = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount     = 1;

        kernelStatePtr->dwCurbeOffset        = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = binary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize   = kernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        if (m_noMeKernelForPFrame)
        {
            m_meKernelStates[1] = m_meKernelStates[0];
            break;
        }
    }

    // Until a better way can be found, maintain old binding table structures
    MeKernelBindingTable *bindingTable        = &m_meBindingTable;
    bindingTable->dwMEMVDataSurface           = CODECHAL_ENCODE_ME_MV_DATA_SURFACE_G11;
    bindingTable->dw16xMEMVDataSurface        = CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_G11;
    bindingTable->dw32xMEMVDataSurface        = CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_G11;
    bindingTable->dwMEDist                    = CODECHAL_ENCODE_ME_DISTORTION_SURFACE_G11;
    bindingTable->dwMEBRCDist                 = CODECHAL_ENCODE_ME_BRC_DISTORTION_G11;
    bindingTable->dwMECurrForFwdRef           = CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_G11;
    bindingTable->dwMEFwdRefPicIdx[0]         = CODECHAL_ENCODE_ME_FWD_REF_IDX0_G11;
    bindingTable->dwMEFwdRefPicIdx[1]         = CODECHAL_ENCODE_ME_FWD_REF_IDX1_G11;
    bindingTable->dwMEFwdRefPicIdx[2]         = CODECHAL_ENCODE_ME_FWD_REF_IDX2_G11;
    bindingTable->dwMEFwdRefPicIdx[3]         = CODECHAL_ENCODE_ME_FWD_REF_IDX3_G11;
    bindingTable->dwMEFwdRefPicIdx[4]         = CODECHAL_ENCODE_ME_FWD_REF_IDX4_G11;
    bindingTable->dwMEFwdRefPicIdx[5]         = CODECHAL_ENCODE_ME_FWD_REF_IDX5_G11;
    bindingTable->dwMEFwdRefPicIdx[6]         = CODECHAL_ENCODE_ME_FWD_REF_IDX6_G11;
    bindingTable->dwMEFwdRefPicIdx[7]         = CODECHAL_ENCODE_ME_FWD_REF_IDX7_G11;
    bindingTable->dwMECurrForBwdRef           = CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_G11;
    bindingTable->dwMEBwdRefPicIdx[0]         = CODECHAL_ENCODE_ME_BWD_REF_IDX0_G11;
    bindingTable->dwMEBwdRefPicIdx[1]         = CODECHAL_ENCODE_ME_BWD_REF_IDX1_G11;
    bindingTable->dwVdencStreamInSurface      = CODECHAL_ENCODE_ME_VDENC_STREAMIN_OUTPUT_G11;
    bindingTable->dwVdencStreamInInputSurface = CODECHAL_ENCODE_ME_VDENC_STREAMIN_INPUT_G11;
#endif

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::InitKernelStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    // DYS
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateDys());

    // G11 VDEnc SHME (16x) and 4x/streamin
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
#endif

    return eStatus;
}

uint32_t CodechalVdencVp9StateG11::GetMaxBtCount()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    uint32_t maxBtCount = 0;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    if (m_hmeSupported)
    {
        uint32_t scalingBtCount = 0;
        uint32_t numKernelsToLoad = m_interlacedFieldDisabled ? 1 : CODEC_NUM_FIELDS_PER_FRAME;
        uint16_t btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();
        for(uint32_t krnStateIdx = 0; krnStateIdx < numKernelsToLoad; krnStateIdx++)
        {
            scalingBtCount += MOS_ALIGN_CEIL(
                m_scaling4xKernelStates[krnStateIdx].KernelParams.iBTCount,
                btIdxAlignment);
        }
        uint32_t meBtCount = 0;
        // 4xME + Streamin kernel btcount
        meBtCount += MOS_ALIGN_CEIL(m_meKernelStates[CODECHAL_ENCODE_ME_IDX_VDENC].KernelParams.iBTCount, btIdxAlignment);

        //16xME streamin kernel count added to ME count and scaling kernel 16x added to scaling count
        if (m_16xMeSupported)
        {
            meBtCount += MOS_ALIGN_CEIL(m_meKernelStates[CODECHAL_ENCODE_ME_IDX_P].KernelParams.iBTCount, btIdxAlignment);
            for (uint32_t krnStateIdx = 0; krnStateIdx < numKernelsToLoad; krnStateIdx++)
            {
                scalingBtCount += MOS_ALIGN_CEIL(
                    m_scaling4xKernelStates[krnStateIdx].KernelParams.iBTCount,
                    btIdxAlignment);
            }
        }
        maxBtCount = scalingBtCount + meBtCount;
    }
#endif

    return maxBtCount;
}

// DYS kernel state init
MOS_STATUS CodechalVdencVp9StateG11::InitKernelStateDys()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    uint32_t combinedKernelSize = 0;
    uint8_t* binary = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        (uint8_t*)IGCODECKRN_G11,
        m_kuidCommon,
        &binary,
        &combinedKernelSize));

    uint32_t kernelSize = combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG11(
        binary,
        ENC_DYS,
        0,
        &currKrnHeader,
        &kernelSize));

    PMHW_KERNEL_STATE kernelState = &m_dysKernelState;
    kernelState->KernelParams.iBTCount = MOS_ALIGN_CEIL(m_dysNumSurfaces, m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    kernelState->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelState->KernelParams.iCurbeLength = MOS_ALIGN_CEIL(m_dysStaticDataSize, m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    kernelState->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;// just assign this to 16, the block resolution for the kernel is decided when its launched depending on the std.
    kernelState->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;// just assign this to 16, the block resolution for the kernel is decided when its launched depending on the std.
    kernelState->KernelParams.iIdCount = 1;
    kernelState->KernelParams.iSamplerCount = 1;
    kernelState->KernelParams.iSamplerLength = m_stateHeapInterface->pStateHeapInterface->GetSizeofSamplerStateAvs();

    kernelState->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelState->dwSamplerOffset = MOS_ALIGN_CEIL(kernelState->dwCurbeOffset + kernelState->KernelParams.iCurbeLength, MHW_SAMPLER_STATE_AVS_ALIGN_G9);
    kernelState->KernelParams.pBinary =
        binary +
        (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelState->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelState->KernelParams.iBTCount,
        &kernelState->dwSshSize,
        &kernelState->dwBindingTableSize));

    m_dysDshSize = kernelState->dwSamplerOffset +
        MOS_ALIGN_CEIL(kernelState->KernelParams.iSamplerLength * kernelState->KernelParams.iSamplerCount, MHW_SAMPLER_STATE_AVS_ALIGN);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelState));
#endif

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetupSegmentationStreamIn()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_segmentMapProvided && !m_hmeEnabled) // If we're not going to use the streamin surface leave now
    {
        return eStatus;
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;

    mhw_vdbox_vdenc_g11_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD *
    streamIn = (mhw_vdbox_vdenc_g11_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(streamIn);

    // align to cache line size is OK since streamin state is padded to cacheline size - HW uses cacheline size to read, not command size
    uint32_t blockWidth   = MOS_ALIGN_CEIL(m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32;
    uint32_t blockHeight  = MOS_ALIGN_CEIL(m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32;
    uint32_t streamInSize = blockHeight * blockWidth * CODECHAL_CACHELINE_SIZE;
    MOS_ZeroMemory(streamIn, streamInSize);

    // If segment map isn't provided then we unlock surface and exit function here.
    // Reason why check isn't done before function call is to take advantage of the fact that
    // we need the surface locked here if seg map is provided and we want it 0'd either way.
    // This saves us from doing 2 locks on this buffer per frame.
    if (!m_segmentMapProvided)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resVdencStreamInBuffer[m_currRecycledBufIdx]));
        return eStatus;
    }

    char *data = (char *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mbSegmentMapSurface.OsResource,
        &lockFlagsReadOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    // Rasterization is done within a tile and then for each tile within the frame in raster order.
    uint32_t numTileColumns          = (1 << m_vp9PicParams->log2_tile_columns);
    uint32_t numTileRows             = (1 << m_vp9PicParams->log2_tile_rows);
    uint32_t numTiles = numTileColumns * numTileRows;
    uint32_t currTileStartX64Aligned = 0, dwCurrTileStartY64Aligned = 0;         //Set tile Y coordinate 0
    m_32BlocksRasterized = 0;   //Count of rasterized blocks for this frame
    uint32_t tileX = 0;
    uint32_t tileY = 0;
    for (uint32_t tileIdx = 0; tileIdx < numTiles; tileIdx++)
    {
        tileX = tileIdx % numTileColumns; //Current tile column position
        tileY = tileIdx / numTileColumns; //Current tile row position

        currTileStartX64Aligned   = ((tileX * m_picWidthInSb) >> m_vp9PicParams->log2_tile_columns) * CODEC_VP9_SUPER_BLOCK_WIDTH;
        dwCurrTileStartY64Aligned = ((tileY * m_picHeightInSb) >> m_vp9PicParams->log2_tile_rows) * CODEC_VP9_SUPER_BLOCK_HEIGHT;

        uint32_t tileWidth64Aligned = (((tileX == (numTileColumns - 1)) ? m_picWidthInSb : (((tileX + 1) * m_picWidthInSb) >> m_vp9PicParams->log2_tile_columns)) *
                                          CODEC_VP9_SUPER_BLOCK_WIDTH) -
                                      currTileStartX64Aligned;

        uint32_t tileHeight64Aligned = (((tileY == (numTileRows - 1)) ? m_picHeightInSb : (((tileY + 1) * m_picHeightInSb) >> m_vp9PicParams->log2_tile_rows)) *
                                           CODEC_VP9_SUPER_BLOCK_HEIGHT) -
                                       dwCurrTileStartY64Aligned;

        // last tile col raw width and raw height not necessarily 64 aligned, use this length to duplicate values from segmap for empty padding blocks in last tiles.
        uint32_t lastTileColWidth = (tileX == (numTileColumns - 1)) ? (m_frameWidth - currTileStartX64Aligned) : tileWidth64Aligned;
        uint32_t lastTileRowHeight = (tileY == (numTileRows - 1)) ? (m_frameHeight - dwCurrTileStartY64Aligned) : tileHeight64Aligned;

        uint32_t tileWidth = (tileX == (numTileColumns - 1)) ? lastTileColWidth : tileWidth64Aligned;
        uint32_t tileHeight = (tileY == (numTileRows - 1)) ? lastTileRowHeight : tileHeight64Aligned;

        // Recreate the mapbuffer and remap it if, for this frame, tile height and width have changed from previous tile
        // which was processed from this frame or previous,
        // or if map buffer is created for previous frame and tile map has changed from previous frame (numtilerows and cols)
        if (!m_mapBuffer ||
            tileHeight != m_segStreamInHeight ||
            tileWidth != m_segStreamInWidth ||
            numTileColumns != m_tileParams[tileIdx].NumOfTileColumnsInFrame ||
            m_tileParams[tileIdx].NumOfTilesInFrame != numTiles)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitZigZagToRasterLUTPerTile(tileHeight,
                tileWidth,
                dwCurrTileStartY64Aligned,
                currTileStartX64Aligned));
        }
        m_tileParams[tileIdx].NumOfTileColumnsInFrame = numTileColumns;
        m_tileParams[tileIdx].NumOfTilesInFrame       = numTiles;
    }


    uint32_t dwPitch = m_mbSegmentMapSurface.dwPitch;
    if (GetResType(&m_mbSegmentMapSurface.OsResource) == MOS_GFXRES_BUFFER)
    {
        //application can send 1D or 2D buffer, based on that change the pitch to correctly access the map buffer
        //driver reads the seg ids from the buffer for each 16x16 block. Reads 4 values for each 32x32 block
        dwPitch = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH) / CODECHAL_MACROBLOCK_WIDTH;
    }
    // set seg ID's of streamin states
    for (uint32_t i = 0 ; i < blockHeight * blockWidth ; ++i)
    {
        uint32_t addrOffset = CalculateBufferOffset(
            m_mapBuffer[i],
            m_frameWidth,
            m_vp9PicParams->PicFlags.fields.seg_id_block_size,
            dwPitch);
        uint32_t segId  = *(data + addrOffset);
        streamIn[i].DW7.SegidEnable = 1;
        streamIn[i].DW7.Segid32X32016X1603Vp9Only = segId | (segId << 4) | (segId << 8) | (segId << 12);

        // TU functions copied from there.
        streamIn[i].DW0.Maxtusize = 3;

        streamIn[i].DW0.Maxcusize = 3;
        // For InterFrames we change the CUsize to 32x32 if we have sub 32 blocks with different segids in superblock
        if ((i % 4) == 3 && m_pictureCodingType == P_TYPE)
        {
            if (!(streamIn[i - 3].DW7.Segid32X32016X1603Vp9Only == streamIn[i - 2].DW7.Segid32X32016X1603Vp9Only &&
                streamIn[i - 2].DW7.Segid32X32016X1603Vp9Only == streamIn[i - 1].DW7.Segid32X32016X1603Vp9Only &&
                streamIn[i - 1].DW7.Segid32X32016X1603Vp9Only == streamIn[i].DW7.Segid32X32016X1603Vp9Only))
            {
                streamIn[i - 3].DW0.Maxcusize = streamIn[i - 2].DW0.Maxcusize = streamIn[i - 1].DW0.Maxcusize = streamIn[i].DW0.Maxcusize = 2;
            }
        }

        streamIn[i].DW0.Numimepredictors = CODECHAL_VDENC_NUMIMEPREDICTORS;

        switch (m_vp9SeqParams->TargetUsage)
        {
        case 1:     // Quality mode
        case 4:     // Normal mode
            streamIn[i].DW6.Nummergecandidatecu8X8 = 1;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 2;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 3;
            streamIn[i].DW6.Nummergecandidatecu64X64 = 4;
            break;
        case 7:     // Speed mode
            streamIn[i].DW0.Numimepredictors = 4;
            streamIn[i].DW6.Nummergecandidatecu8X8 = 0;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 2;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 2;
            streamIn[i].DW6.Nummergecandidatecu64X64 = 2;
            break;
        default:
            MHW_ASSERTMESSAGE("Invalid TU provided!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mbSegmentMapSurface.OsResource));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resVdencStreamInBuffer[m_currRecycledBufIdx]));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetMeSurfaceParams(MeSurfaceParams *meSurfaceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(meSurfaceParams);

    meSurfaceParams->bMbaff = false;
    meSurfaceParams->b4xMeDistortionBufferSupported = true;
    meSurfaceParams->dwNumRefIdxL0ActiveMinus1      = (m_vp9PicParams->PicFlags.fields.frame_type) ? m_numRefFrames - 1 : 0;
    meSurfaceParams->dwNumRefIdxL1ActiveMinus1 = 0;

    MOS_ZeroMemory(&m_refPicList0, sizeof(m_refPicList0));

    if (m_lastRefPic)
    {
        m_refPicList0[0].FrameIdx = m_vp9PicParams->RefFlags.fields.LastRefIdx;
        m_refPicList0[0].PicFlags = PICTURE_FRAME;
    }
    if (m_goldenRefPic)
    {
        m_refPicList0[1].FrameIdx = m_vp9PicParams->RefFlags.fields.GoldenRefIdx;
        m_refPicList0[1].PicFlags = PICTURE_FRAME;
    }
    if (m_altRefPic)
    {
        m_refPicList0[2].FrameIdx = m_vp9PicParams->RefFlags.fields.AltRefIdx;
        m_refPicList0[2].PicFlags = PICTURE_FRAME;
    }

    meSurfaceParams->pL0RefFrameList = &(m_refPicList0[0]);
    meSurfaceParams->ppRefList                  = &m_refList[0];
    meSurfaceParams->pPicIdx = &m_picIdx[0];
    meSurfaceParams->pCurrOriginalPic = &m_currOriginalPic;
    meSurfaceParams->ps4xMeMvDataBuffer = &m_4xMeMvDataBuffer;
    meSurfaceParams->ps16xMeMvDataBuffer = &m_16xMeMvDataBuffer;
    meSurfaceParams->psMeDistortionBuffer = &m_4xMeDistortionBuffer;
    meSurfaceParams->dwVerticalLineStride = m_verticalLineStride;
    meSurfaceParams->dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    meSurfaceParams->b32xMeEnabled = m_32xMeSupported;
    meSurfaceParams->b16xMeEnabled = m_16xMeEnabled;
    meSurfaceParams->pMeBindingTable = &m_meBindingTable;
    meSurfaceParams->bVdencStreamInEnabled = true;
    meSurfaceParams->psMeVdencStreamInBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
    meSurfaceParams->dwVDEncStreamInSurfaceSize = MOS_BYTES_TO_DWORDS((MOS_ALIGN_CEIL(m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32) *
                                                  (MOS_ALIGN_CEIL(m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32) *
                                                  CODECHAL_CACHELINE_SIZE);
    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetMeCurbeParams(MeCurbeParams *meParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(meParams);

    meParams->b16xMeEnabled = m_16xMeEnabled;
    meParams->b32xMeEnabled = m_32xMeSupported;
    meParams->TargetUsage = TU_QUALITY;
    meParams->MaxMvLen = m_hmeMaxMvLength;
    meParams->CurrOriginalPic.FrameIdx     = m_vp9PicParams->CurrOriginalPic.FrameIdx;
    meParams->CurrOriginalPic.PicEntry     = m_vp9PicParams->CurrOriginalPic.PicEntry;
    meParams->CurrOriginalPic.PicFlags     = m_vp9PicParams->CurrOriginalPic.PicFlags;
    meParams->pic_init_qp_minus26          = m_vp9PicParams->LumaACQIndex - 26;
    meParams->num_ref_idx_l0_active_minus1 = (m_vp9PicParams->PicFlags.fields.frame_type) ? m_numRefFrames - 1 : 0;
    meParams->num_ref_idx_l1_active_minus1 = 0;

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SendMeSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer,
    MeSurfaceParams *   params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeDistortionBuffer);

    if (!params->bVdencStreamInEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeBrcDistortionBuffer);
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeVdencStreamInBuffer);
    }

    CODECHAL_MEDIA_STATE_TYPE encMediaStateType = (params->b32xMeInUse) ? CODECHAL_MEDIA_STATE_32X_ME : params->b16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    if (params->bVdencStreamInEnabled && encMediaStateType == CODECHAL_MEDIA_STATE_4X_ME)
    {
        encMediaStateType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMeBindingTable);
    MeKernelBindingTable *meBindingTable = params->pMeBindingTable;

    bool    isFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool    isBottomField  = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    uint8_t currVDirection = (!isFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((isBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    PMOS_SURFACE currScaledSurface = nullptr, meMvDataBuffer = nullptr;
    uint32_t     meMvBottomFieldOffset = 0, currScaledBottomFieldOffset = 0;
    if (params->b32xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps32xMeMvDataBuffer);
        currScaledSurface           = m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer              = params->ps32xMeMvDataBuffer;
        meMvBottomFieldOffset       = params->dw32xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw32xScaledBottomFieldOffset;
    }
    else if (params->b16xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps16xMeMvDataBuffer);
        currScaledSurface           = m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer              = params->ps16xMeMvDataBuffer;
        meMvBottomFieldOffset       = params->dw16xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw16xScaledBottomFieldOffset;
    }
    else
    {
        currScaledSurface           = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer              = params->ps4xMeMvDataBuffer;
        meMvBottomFieldOffset       = params->dw4xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw4xScaledBottomFieldOffset;
    }

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.


    uint32_t width  = MOS_ALIGN_CEIL(params->dwDownscaledWidthInMb * 32, 64);
    uint32_t height = params->dwDownscaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    meMvDataBuffer->dwWidth  = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch  = width;

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface          = true;
    surfaceParams.bMediaBlockRW         = true;
    surfaceParams.psSurface             = meMvDataBuffer;
    surfaceParams.dwOffset              = meMvBottomFieldOffset;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEMVDataSurface;
    surfaceParams.bIsWritable           = true;
    surfaceParams.bRenderTarget         = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    if (params->b16xMeInUse && params->b32xMeEnabled)
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface  = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface     = params->ps32xMeMvDataBuffer;
        surfaceParams.dwOffset =
            isBottomField ? params->dw32xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dw32xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }
    else if (!params->b32xMeInUse && params->b16xMeEnabled)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface  = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface     = params->ps16xMeMvDataBuffer;
        surfaceParams.dwOffset =
            isBottomField ? params->dw16xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dw16xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (!params->b32xMeInUse && !params->b16xMeInUse)
    {
        if (!params->bVdencStreamInEnabled)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bIs2DSurface          = true;
            surfaceParams.bMediaBlockRW         = true;
            surfaceParams.psSurface             = params->psMeBrcDistortionBuffer;
            surfaceParams.dwOffset              = params->dwMeBrcDistortionBottomFieldOffset;
            surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEBRCDist;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
            surfaceParams.bIsWritable           = true;
            surfaceParams.bRenderTarget         = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));
        }

        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface          = true;
        surfaceParams.bMediaBlockRW         = true;
        surfaceParams.psSurface             = params->psMeDistortionBuffer;
        surfaceParams.dwOffset              = params->dwMeDistortionBottomFieldOffset;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEDist;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable           = true;
        surfaceParams.bRenderTarget         = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    // Setup references 1...n
    // LIST 0 references (not optional)
    CODEC_PICTURE refPic;
    bool          isRefFieldPicture = false, isRefBottomField = false;
    uint8_t       refPicIdx = 0;
    if (params->pL0RefFrameList)
    {
        for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
        {
            refPic = params->pL0RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
            {
                if (refIdx == 0)
                {
                    // Current Picture Y - VME
                    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                    surfaceParams.bUseAdvState          = true;
                    surfaceParams.psSurface             = currScaledSurface;
                    surfaceParams.dwOffset              = isBottomField ? currScaledBottomFieldOffset : 0;
                    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                    surfaceParams.dwBindingTableOffset  = meBindingTable->dwMECurrForFwdRef;
                    surfaceParams.ucVDirection          = currVDirection;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                        m_hwInterface,
                        cmdBuffer,
                        &surfaceParams,
                        params->pKernelState));
                }

                isRefFieldPicture                   = CodecHal_PictureIsField(refPic) ? 1 : 0;
                isRefBottomField                    = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
                refPicIdx                           = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
                uint8_t  scaledIdx                  = params->ppRefList[refPicIdx]->ucScalingIdx;
                uint32_t refScaledBottomFieldOffset = 0;
                MOS_SURFACE *refScaledSurface;
                if (params->b32xMeInUse)
                {
                    refScaledSurface                = m_trackedBuf->Get32xDsSurface(scaledIdx);
                }
                else if (params->b16xMeInUse)
                {
                    refScaledSurface                = m_trackedBuf->Get16xDsSurface(scaledIdx);
                }
                else
                {
                    refScaledSurface                = m_trackedBuf->Get4xDsSurface(scaledIdx);
                }
                refScaledBottomFieldOffset          = isRefBottomField ? currScaledBottomFieldOffset : 0;
                

                // L0 Reference Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState          = true;
                surfaceParams.psSurface             = refScaledSurface;
                surfaceParams.dwOffset              = isRefBottomField ? refScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEFwdRefPicIdx[refIdx];
                surfaceParams.ucVDirection          = !isFieldPicture ? CODECHAL_VDIRECTION_FRAME : ((isRefBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }
        }
    }
    else
    {
        return MOS_STATUS_NULL_POINTER;
    }

    // Setup references 1...n
    // LIST 1 references (optional)
    if (params->pL1RefFrameList)
    {
        for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL1ActiveMinus1; refIdx++)
        {
            refPic = params->pL1RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
            {
                if (refIdx == 0)
                {
                    // Current Picture Y - VME
                    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                    surfaceParams.bUseAdvState          = true;
                    surfaceParams.psSurface             = currScaledSurface;
                    surfaceParams.dwOffset              = isBottomField ? currScaledBottomFieldOffset : 0;
                    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                    surfaceParams.dwBindingTableOffset  = meBindingTable->dwMECurrForBwdRef;
                    surfaceParams.ucVDirection          = currVDirection;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                        m_hwInterface,
                        cmdBuffer,
                        &surfaceParams,
                        params->pKernelState));
                }

                isRefFieldPicture                   = CodecHal_PictureIsField(refPic) ? 1 : 0;
                isRefBottomField                    = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
                refPicIdx                           = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
                uint8_t  scaledIdx                  = params->ppRefList[refPicIdx]->ucScalingIdx;
                uint32_t refScaledBottomFieldOffset = 0;
                MOS_SURFACE *refScaledSurface;
                if (params->b32xMeInUse)
                {
                    refScaledSurface                = m_trackedBuf->Get32xDsSurface(scaledIdx);
                }
                else if (params->b16xMeInUse)
                {
                    refScaledSurface                = m_trackedBuf->Get16xDsSurface(scaledIdx);
                }
                else
                {
                    refScaledSurface                = m_trackedBuf->Get4xDsSurface(scaledIdx);
                }
                refScaledBottomFieldOffset          = isRefBottomField ? currScaledBottomFieldOffset : 0;
                

                // L1 Reference Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState          = true;
                surfaceParams.psSurface             = refScaledSurface;
                surfaceParams.dwOffset              = isRefBottomField ? refScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEBwdRefPicIdx[refIdx];
                surfaceParams.ucVDirection          = (!isFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((isRefBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }
        }
    }
    if (encMediaStateType == CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN)
    {
        // Output buffer
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize                = params->dwVDEncStreamInSurfaceSize;
        surfaceParams.bIs2DSurface          = false;
        surfaceParams.presBuffer            = params->psMeVdencStreamInBuffer;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dwVdencStreamInSurface;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable           = true;
        surfaceParams.bRenderTarget         = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));

        // Input buffer (for AVC case we only read the surface and update data)
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize                = params->dwVDEncStreamInSurfaceSize;
        surfaceParams.bIs2DSurface          = false;
        surfaceParams.presBuffer            = params->psMeVdencStreamInBuffer;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dwVdencStreamInInputSurface;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable           = true;
        surfaceParams.bRenderTarget         = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    return eStatus;
}


//------------------------------------------------------------------------------
//| Purpose:    Setup curbe for common ME kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalVdencVp9StateG11::SetCurbeMe(
    MeCurbeParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CODECHAL_ENCODE_ASSERT(params->TargetUsage <= NUM_TARGET_USAGE_MODES);

    uint32_t scaleFactor       = 0;
    bool     useMvFromPrevStep = false, writeDistortions = false;
    uint8_t  mvShiftFactor = 0, prevMvReadPosFactor = 0;
    switch (params->hmeLvl)
    {
    case HME_LEVEL_32x:
        useMvFromPrevStep = CODECHAL_ENCODE_HME_FIRST_STEP_G11;
        writeDistortions  = false;
        scaleFactor       = SCALE_FACTOR_32x;
        mvShiftFactor     = CODECHAL_ENCODE_MV_SHIFT_FACTOR_32x_G11;
        break;
    case HME_LEVEL_16x:
        useMvFromPrevStep   = (params->b32xMeEnabled) ? CODECHAL_ENCODE_HME_FOLLOWING_STEP_G11 : CODECHAL_ENCODE_HME_FIRST_STEP_G11;
        writeDistortions    = false;
        scaleFactor         = SCALE_FACTOR_16x;
        mvShiftFactor       = CODECHAL_ENCODE_MV_SHIFT_FACTOR_16x_G11;
        prevMvReadPosFactor = CODECHAL_ENCODE_PREV_MV_READ_POSITION_16x_G11;
        break;
    case HME_LEVEL_4x:
        useMvFromPrevStep   = (params->b16xMeEnabled) ? CODECHAL_ENCODE_HME_FOLLOWING_STEP_G11 : CODECHAL_ENCODE_HME_FIRST_STEP_G11;
        writeDistortions    = true;
        scaleFactor         = SCALE_FACTOR_4x;
        mvShiftFactor       = CODECHAL_ENCODE_MV_SHIFT_FACTOR_4x_G11;
        prevMvReadPosFactor = CODECHAL_ENCODE_PREV_MV_READ_POSITION_4x_G11;
        break;
    default:
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MeCurbe cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(MeCurbe),
        meCurbeInit,
        sizeof(MeCurbe)));

    cmd.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        cmd.DW3.SrcAccess =
            cmd.DW3.RefAccess    = CodecHal_PictureIsField(params->CurrOriginalPic) ? 1 : 0;
        cmd.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(params->CurrOriginalPic) ? 1 : 0;
    }

    bool framePicture = CodecHal_PictureIsFrame(params->CurrOriginalPic);
    char qpPrimeY     = (params->pic_init_qp_minus26 + 26) + params->slice_qp_delta;

    cmd.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth        = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    cmd.DW5.QpPrimeY            = qpPrimeY;
    cmd.DW6.WriteDistortions    = writeDistortions;
    cmd.DW6.UseMvFromPrevStep   = useMvFromPrevStep;

    cmd.DW6.SuperCombineDist = m_superCombineDistGeneric[params->TargetUsage];
    cmd.DW6.MaxVmvR          = (framePicture) ? params->MaxMvLen * 4 : (params->MaxMvLen >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32 to match
        cmd.DW1.BiWeight             = 32;
        cmd.DW13.NumRefIdxL1MinusOne = params->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        if (params->hmeLvl == HME_LEVEL_4x && m_useNonLegacyStreamin)
        {
            cmd.DW30.ActualMBHeight = m_frameHeight;
            cmd.DW30.ActualMBWidth  = m_frameWidth;
        }
        else if (m_vdencEnabled && m_16xMeSupported)
        {
            cmd.DW30.ActualMBHeight = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);
            cmd.DW30.ActualMBWidth  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth);
        }
        cmd.DW13.NumRefIdxL0MinusOne =
            params->num_ref_idx_l0_active_minus1;
    }

    cmd.DW13.RefStreaminCost = 5;
    // This flag is to indicate the ROI source type instead of indicating ROI is enabled or not
    cmd.DW13.ROIEnable = 0;

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW14.List0RefID0FieldParity = params->List0RefID0FieldParity;
            cmd.DW14.List0RefID1FieldParity = params->List0RefID1FieldParity;
            cmd.DW14.List0RefID2FieldParity = params->List0RefID2FieldParity;
            cmd.DW14.List0RefID3FieldParity = params->List0RefID3FieldParity;
            cmd.DW14.List0RefID4FieldParity = params->List0RefID4FieldParity;
            cmd.DW14.List0RefID5FieldParity = params->List0RefID5FieldParity;
            cmd.DW14.List0RefID6FieldParity = params->List0RefID6FieldParity;
            cmd.DW14.List0RefID7FieldParity = params->List0RefID7FieldParity;
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.DW14.List1RefID0FieldParity = params->List1RefID0FieldParity;
            cmd.DW14.List1RefID1FieldParity = params->List1RefID1FieldParity;
        }
    }

    cmd.DW15.MvShiftFactor       = mvShiftFactor;
    cmd.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    // r3 & r4
    uint8_t targetUsage = params->TargetUsage;
    uint8_t meMethod    = 0;
    if (m_pictureCodingType == B_TYPE)
    {
        meMethod = params->pBMEMethodTable ?  // use the ME table dependent on codec standard
                       params->pBMEMethodTable[targetUsage]
                                           : m_bMeMethodGeneric[targetUsage];
    }
    else
    {
        meMethod = params->pMEMethodTable ?  // use the ME table dependent on codec standard
                       params->pMEMethodTable[targetUsage]
                                          : m_meMethodGeneric[targetUsage];
    }

    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus          = MOS_SecureMemcpy(&(cmd.SPDelta), 14 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // Non legacy stream in is for hevc vp9 streamin kernel
    if (params->hmeLvl == HME_LEVEL_4x && m_useNonLegacyStreamin)
    {
        //StreamIn CURBE
        cmd.DW6.LCUSize                    = 1;  //Only LCU64 supported by the VDEnc HW
        cmd.DW6.InputStreamInSurfaceEnable = params->segmapProvided;
        cmd.DW31.MaxCuSize                 = 3;
        cmd.DW31.MaxTuSize                 = 3;
        switch (params->TargetUsage)
        {
        case 1:
        case 4:
            cmd.DW36.NumMergeCandidateCu64x64 = 4;
            cmd.DW36.NumMergeCandidateCu32x32 = 3;
            cmd.DW36.NumMergeCandidateCu16x16 = 2;
            cmd.DW36.NumMergeCandidateCu8x8   = 1;
            cmd.DW31.NumImePredictors         = 8;
            break;
        case 7:
            cmd.DW36.NumMergeCandidateCu64x64 = 2;
            cmd.DW36.NumMergeCandidateCu32x32 = 2;
            cmd.DW36.NumMergeCandidateCu16x16 = 2;
            cmd.DW36.NumMergeCandidateCu8x8   = 0;
            cmd.DW31.NumImePredictors         = 4;
            break;
        }
    }

    // r5
    cmd.DW40._4xMeMvOutputDataSurfIndex      = CODECHAL_ENCODE_ME_MV_DATA_SURFACE_G11;
    cmd.DW41._16xOr32xMeMvInputDataSurfIndex = (params->hmeLvl == HME_LEVEL_32x) ? CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_G11 : CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_G11;
    cmd.DW42._4xMeOutputDistSurfIndex        = CODECHAL_ENCODE_ME_DISTORTION_SURFACE_G11;
    cmd.DW43._4xMeOutputBrcDistSurfIndex     = CODECHAL_ENCODE_ME_BRC_DISTORTION_G11;
    cmd.DW44.VMEFwdInterPredictionSurfIndex  = CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_G11;
    cmd.DW45.VMEBwdInterPredictionSurfIndex  = CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_G11;
    cmd.DW46.VDEncStreamInOutputSurfIndex    = CODECHAL_ENCODE_ME_VDENC_STREAMIN_OUTPUT_G11;
    cmd.DW47.VDEncStreamInInputSurfIndex     = CODECHAL_ENCODE_ME_VDENC_STREAMIN_INPUT_G11;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ExecuteMeKernel(
    MeCurbeParams *  meParams,
    MeSurfaceParams *meSurfaceParams,
    HmeLevel         hmeLevel)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(meParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(meSurfaceParams);

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL : CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    // Each ME kernel buffer counts as a separate perf task
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = (hmeLevel == HME_LEVEL_32x) ? CODECHAL_MEDIA_STATE_32X_ME : (hmeLevel == HME_LEVEL_16x) ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    bool vdencMeInUse = false;
    if (m_vdencEnabled && (encFunctionType == CODECHAL_MEDIA_STATE_4X_ME))
    {
        vdencMeInUse = true;
        // Non legacy stream in is for hevc vp9 streamin kernel
        encFunctionType = m_useNonLegacyStreamin ? CODECHAL_MEDIA_STATE_4X_ME : CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    uint32_t krnStateIdx = vdencMeInUse ? CODECHAL_ENCODE_ME_IDX_VDENC : ((m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_ME_IDX_P : CODECHAL_ENCODE_ME_IDX_B);

    PMHW_KERNEL_STATE kernelState = &m_meKernelStates[krnStateIdx];

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ? m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));
    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // Setup Additional MeParams (Most of them set up in codec specific function, so don't zero out here)
    meParams->hmeLvl       = hmeLevel;
    meParams->pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMe(meParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));)
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
    }
    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams                 = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState    = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Setup Additional ME surface params (Most of them set up in codec specific function, so don't zero out here)
    meSurfaceParams->dwDownscaledWidthInMb  = (hmeLevel == HME_LEVEL_32x) ? m_downscaledWidthInMb32x : (hmeLevel == HME_LEVEL_16x) ? m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    meSurfaceParams->dwDownscaledHeightInMb = (hmeLevel == HME_LEVEL_32x) ? m_downscaledFrameFieldHeightInMb32x : (hmeLevel == HME_LEVEL_16x) ? m_downscaledFrameFieldHeightInMb16x : m_downscaledFrameFieldHeightInMb4x;
    meSurfaceParams->b32xMeInUse            = (hmeLevel == HME_LEVEL_32x) ? true : false;
    meSurfaceParams->b16xMeInUse            = (hmeLevel == HME_LEVEL_16x) ? true : false;
    meSurfaceParams->pKernelState           = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(&cmdBuffer, meSurfaceParams));

    // Dump SSH for ME kernel
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState)));

    /* zero out the mv data memory and me distortion buffer for the driver ULT
    kernel only writes out this data used for current frame, in some cases the the data used for
    previous frames would be left in the buffer (for example, the L1 mv for B frame would still show
    in the P frame mv data buffer */

    // Zeroing out the buffers has perf impact, so zero it out only when dumps are actually enabled
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);
        uint8_t *data                = NULL;
        uint32_t size                = 0;
        bool     driverMeDumpEnabled = m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrOutput, encFunctionType);

        if (driverMeDumpEnabled) {
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;

            switch (hmeLevel)
            {
            case HME_LEVEL_32x:
                data = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &meSurfaceParams->ps32xMeMvDataBuffer->OsResource,
                    &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                size = MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) *
                       (m_downscaledHeightInMb32x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
                MOS_ZeroMemory(data, size);
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &meSurfaceParams->ps32xMeMvDataBuffer->OsResource);
                break;
            case HME_LEVEL_16x:
                data = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &meSurfaceParams->ps16xMeMvDataBuffer->OsResource,
                    &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                size = MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) *
                       (m_downscaledHeightInMb16x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
                MOS_ZeroMemory(data, size);
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &meSurfaceParams->ps16xMeMvDataBuffer->OsResource);
                break;
            case HME_LEVEL_4x:
                if (!m_vdencEnabled)
                {
                    data = (uint8_t *)m_osInterface->pfnLockResource(
                        m_osInterface,
                        &meSurfaceParams->ps4xMeMvDataBuffer->OsResource,
                        &lockFlags);
                    CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                    size = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) *
                           (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
                    MOS_ZeroMemory(data, size);
                    m_osInterface->pfnUnlockResource(
                        m_osInterface,
                        &meSurfaceParams->ps4xMeMvDataBuffer->OsResource);
                }
                break;
            default:
                return MOS_STATUS_INVALID_PARAMETER;
            }

            // zeroing out ME dist buffer
            if (meSurfaceParams->b4xMeDistortionBufferSupported)
            {
                data = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface, &meSurfaceParams->psMeDistortionBuffer->OsResource, &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);
                size = meSurfaceParams->psMeDistortionBuffer->dwHeight * meSurfaceParams->psMeDistortionBuffer->dwPitch;
                MOS_ZeroMemory(data, size);
                m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &meSurfaceParams->psMeDistortionBuffer->OsResource);
            }
        });

    uint32_t scalingFactor = (hmeLevel == HME_LEVEL_32x) ? SCALE_FACTOR_32x : (hmeLevel == HME_LEVEL_16x) ? SCALE_FACTOR_16x : SCALE_FACTOR_4x;

    uint32_t resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scalingFactor);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode              = m_walkerMode;
    walkerCodecParams.dwResolutionX           = resolutionX;
    walkerCodecParams.dwResolutionY           = resolutionY;
    walkerCodecParams.bNoDependency           = true;
    walkerCodecParams.bMbaff                  = meSurfaceParams->bMbaff;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId               = m_groupId;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    uint32_t dumpFormat = 0;
    CODECHAL_DEBUG_TOOL(
     //   CodecHal_DbgMapSurfaceFormatToDumpFormat(m_rawSurfaceToEnc->Format, &dumpFormat);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf"));
    if (m_lastRefPic)
    {
      //  CodecHal_DbgMapSurfaceFormatToDumpFormat(m_lastRefPic->Format, &dumpFormat);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_lastRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            "LastRefSurface"));
    }

    if (m_goldenRefPic)
    {
      //  CodecHal_DbgMapSurfaceFormatToDumpFormat(m_goldenRefPic->Format, &dumpFormat);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_goldenRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            "GoldenRefSurface"));
    }

    if (m_altRefPic)
    {
      //  CodecHal_DbgMapSurfaceFormatToDumpFormat(m_altRefPic->Format, &dumpFormat);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_altRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            "_AltRefSurface"));
    }
    );


    m_setRequestedEUSlices = ((m_frameHeight * m_frameWidth) >= m_ssdResolutionThreshold &&
        m_targetUsage <= m_ssdTargetUsageThreshold) ? true : false;

    m_hwInterface->m_numRequestedEuSlices = (m_setRequestedEUSlices) ?
        m_sliceShutdownRequestState : m_sliceShutdownDefaultState;

    // While this streamin isn't a kernel function, we 0 the surface here which is needed before HME kernel
    SetupSegmentationStreamIn();

    // Super HME
    if (m_16xMeSupported)
    {
        //4x Downscaling
        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhaseCSC =
            cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_16xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase16xDS = !(m_32xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase32xDS = !m_hmeEnabled;

        m_firstTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }

    if (m_16xMeEnabled)
    {
        //Initialize the ME struct for HME kernel calls
        MeCurbeParams meParams;
        MOS_ZeroMemory(&meParams, sizeof(MeCurbeParams));
        SetMeCurbeParams(&meParams);

        MeSurfaceParams meSurfaceParams;
        MOS_ZeroMemory(&meSurfaceParams, sizeof(MeSurfaceParams));
        SetMeSurfaceParams(&meSurfaceParams);

        // P_HME kernel (16x HME)
        m_lastTaskInPhase = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ExecuteMeKernel(&meParams, &meSurfaceParams, HME_LEVEL_16x));

        //StreamIn kernel, 4xME
        m_lastTaskInPhase = true;
        meParams.segmapProvided = m_segmentMapProvided;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ExecuteMeKernel(&meParams, &meSurfaceParams, HME_LEVEL_4x));
    }
    //CODECHAL_DEBUG_TOOL
    // (
        /*CodecHal_DbgDumpVp9VdEncHMEData(
            m_encoder,
            dumpFormat,
            MeSurfaceParams.ps4xMeMvDataBuffer,
            MeSurfaceParams.ps16xMeMvDataBuffer,
            MeSurfaceParams.psMeDistortionBuffer);
        */

    //if (m_scalingEnabled)
    //{
    //    // Dump 4x scaling and HME buffers
    //    m_debugInterface->DumpYUVSurface(
    //        m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER),
    //        CodechalDbgAttr::attrReferenceSurfaces,
    //        "4xScaledSurf");
    //    m_debugInterface->DumpBuffer(
    //        &meSurfaceParams.ps4xMeMvDataBuffer->OsResource,
    //        CodechalDbgAttr::attrOutput,
    //        "MvData",
    //        meSurfaceParams.ps4xMeMvDataBuffer->dwHeight * meSurfaceParams.ps4xMeMvDataBuffer->dwPitch,
    //        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
    //        CODECHAL_MEDIA_STATE_4X_ME);
    //    m_debugInterface->DumpBuffer(
    //        &meSurfaceParams.psMeDistortionBuffer->OsResource,
    //        CodechalDbgAttr::attrOutput,
    //        "MeDist",
    //        meSurfaceParams.psMeDistortionBuffer->dwHeight *meSurfaceParams.psMeDistortionBuffer->dwPitch,
    //        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
    //        CODECHAL_MEDIA_STATE_4X_ME);

    //    if (m_16xMeSupported)
    //    {
    //        // Dump 16x scaling and HME buffers
    //        m_debugInterface->DumpYUVSurface(
    //            m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER),
    //            CodechalDbgAttr::attrReferenceSurfaces,
    //            "16xScaledSurf");
    //        if (m_16xMeEnabled)
    //        {
    //            m_debugInterface->DumpBuffer(
    //                &meSurfaceParams.ps16xMeMvDataBuffer->OsResource,
    //                CodechalDbgAttr::attrOutput,
    //                "MvData",
    //                meSurfaceParams.ps16xMeMvDataBuffer->dwHeight *meSurfaceParams.ps16xMeMvDataBuffer->dwPitch,
    //                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
    //                CODECHAL_MEDIA_STATE_16X_ME);
    //        }
    //    }
    //}
    // dump VDEncStreamin
    /* m_debugInterface->DumpBuffer(
        &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
        CodechalDbgAttr::attrOutput,
        "Output",
        (MOS_ALIGN_CEIL(m_frameHeight, 32) * (MOS_ALIGN_CEIL(m_frameFieldHeight, 32)) / 16),
        0,
        CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN);*/

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        m_waitForEnc = true;
    }
#endif

    return eStatus;
}

static void StatusReportCleanup(
    EncodeStatusReport* encodeStatusReport,
    HCPPakHWTileSizeRecord_G11* tileStatusReport,
    CODECHAL_ENCODE_BUFFER* tileSizeStreamoutBuffer,
    PMOS_INTERFACE osInterface,
    uint8_t* tempBsBuffer,
    uint8_t* bitstream)
{

    if (tempBsBuffer)
    {
        MOS_FreeMemory(tempBsBuffer);
    }

    if (bitstream)
    {
        osInterface->pfnUnlockResource(osInterface, &encodeStatusReport->pCurrRefList->resBitstreamBuffer);
    }

    if (tileStatusReport)
    {
        // clean-up the tile status report buffer
        if (encodeStatusReport->CodecStatus == CODECHAL_STATUS_SUCCESSFUL)
        {
            for (uint32_t i = 0; i < encodeStatusReport->NumberTilesInFrame; i++)
            {
                MOS_ZeroMemory(&tileStatusReport[i], sizeof(tileStatusReport[i]));
            }
        }

        osInterface->pfnUnlockResource(osInterface, &tileSizeStreamoutBuffer->sResource);
    }
}

CodechalVdencVp9StateG11::~CodechalVdencVp9StateG11()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_scalabilityState)
    {
        MOS_FreeMemAndSetNull(m_scalabilityState);
    }
    //Note: virtual engine interface destroy is done in MOS layer

    return;
}

MOS_STATUS CodechalVdencVp9StateG11::GetStatusReport(
    EncodeStatus*       encodeStatus,
    EncodeStatusReport* encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport);

    if (encodeStatusReport->UsedVdBoxNumber == 1)
    {
        encodeStatusReport->bitstreamSize = encodeStatus->dwMFCBitstreamByteCountPerFrame + encodeStatus->dwHeaderBytesInserted;
        encodeStatusReport->NumberPasses  = (uint8_t)encodeStatus->dwNumberPasses;

        encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
        return eStatus;
    }

    // Tile record always in m_tileRecordBuffer even in scala mode
    PCODECHAL_ENCODE_BUFFER presTileSizeStatusReport = &m_tileRecordBuffer[encodeStatusReport->CurrOriginalPic.FrameIdx];

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    HCPPakHWTileSizeRecord_G11* tileStatusReport = (HCPPakHWTileSizeRecord_G11*)m_osInterface->pfnLockResource(
        m_osInterface,
        &presTileSizeStatusReport->sResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileStatusReport);

    encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
    encodeStatusReport->PanicMode = false;
    encodeStatusReport->AverageQp = 0;
    encodeStatusReport->QpY = 0;
    encodeStatusReport->SuggestedQpYDelta = 0;
    encodeStatusReport->NumberPasses = 1;
    encodeStatusReport->bitstreamSize = 0;
    encodeStatus->ImageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQp = 0;

    double sum_qp = 0.0;
    uint32_t totalCU = 0;
    for (uint32_t i = 0; i < encodeStatusReport->NumberTilesInFrame; i++)
    {
        if (tileStatusReport[i].Length == 0)
        {
            encodeStatusReport->CodecStatus = CODECHAL_STATUS_INCOMPLETE;
            StatusReportCleanup(encodeStatusReport, tileStatusReport, presTileSizeStatusReport, m_osInterface, nullptr, nullptr);
            return eStatus;
        }

        encodeStatusReport->bitstreamSize += tileStatusReport[i].Length;
        totalCU += (m_tileParams[i].TileHeightInMinCbMinus1 + 1) * (m_tileParams[i].TileWidthInMinCbMinus1 + 1);
        sum_qp += tileStatusReport[i].Hcp_Qp_Status_Count;
    }

    encodeStatusReport->QpY = encodeStatusReport->AverageQp =
        (uint8_t)((sum_qp / (double)totalCU) / 4.0); // due to TU is 4x4 and there are 4 TUs in one CU

    if (m_enableTileStitchByHW)
    {
        StatusReportCleanup(encodeStatusReport, tileStatusReport, presTileSizeStatusReport, m_osInterface, nullptr, nullptr);
        return eStatus;
    }

    uint8_t* bufPtr = (uint8_t*)MOS_AllocAndZeroMemory(encodeStatusReport->bitstreamSize);
    uint8_t* tempBsBuffer = bufPtr;
    CODECHAL_ENCODE_CHK_NULL_RETURN(tempBsBuffer);

    CODEC_REF_LIST currRefList = *(encodeStatus->encodeStatusReport.pCurrRefList);
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    uint8_t* bitstream = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &currRefList.resBitstreamBuffer,
        &lockFlags);
    if (bitstream == nullptr)
    {
        MOS_SafeFreeMemory(tempBsBuffer);
        CODECHAL_ENCODE_CHK_NULL_RETURN(nullptr);
    }

    for (uint32_t i = 0; i < encodeStatusReport->NumberTilesInFrame; i++)
    {
        uint32_t offset = m_tileParams[i].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
        uint32_t len = tileStatusReport[i].Length;

        if (offset + len >= m_bitstreamUpperBound)
        {
            eStatus = MOS_STATUS_INVALID_FILE_SIZE;
            CODECHAL_ENCODE_ASSERTMESSAGE("Error: Tile offset and length add up to more than bitstream upper bound");
            encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
            encodeStatusReport->bitstreamSize = 0;
            StatusReportCleanup(encodeStatusReport, tileStatusReport, presTileSizeStatusReport, m_osInterface, tempBsBuffer, bitstream);
            return eStatus;
        }

        MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
        bufPtr += len;
    }

    MOS_SecureMemcpy(bitstream, encodeStatusReport->bitstreamSize, tempBsBuffer, encodeStatusReport->bitstreamSize);
    MOS_ZeroMemory(&bitstream[encodeStatusReport->bitstreamSize],
        m_bitstreamUpperBound - encodeStatusReport->bitstreamSize);

    StatusReportCleanup(encodeStatusReport, tileStatusReport, presTileSizeStatusReport, m_osInterface, tempBsBuffer, bitstream);

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::DecideEncodingPipeNumber()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_numPipe = m_numVdbox;

    uint8_t num_tile_columns = (1 << m_vp9PicParams->log2_tile_columns);

    if (num_tile_columns > m_numPipe)
    {
        m_numPipe = 1;
    }

    if (num_tile_columns < m_numPipe)
    {
        if (num_tile_columns >= 1 && num_tile_columns <= 4)
        {
            m_numPipe = num_tile_columns;
        }
        else
        {
            m_numPipe = 1; // invalid tile column test cases and switch back to the single VDBOX mode
        }
    }

    if (m_numPipe == 0 || m_numPipe > CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE)
    {
        m_numPipe = 1;
    }

    if (m_numPipe > 1)
    {
        m_scalableMode = true; // KMD VE is now enabled by default. Mediasolo can also use the VE interface.
    }
    else
    {
        m_scalableMode = false;
    }

    if (m_scalabilityState)
    {
        // Create/ re-use a GPU context with 2 pipes
        m_scalabilityState->ucScalablePipeNum = m_numPipe;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::PlatformCapabilityCheck()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DecideEncodingPipeNumber());

    if (MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeScalability_ChkGpuCtxReCreation(this, m_scalabilityState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    if (m_numPipe > 1)
    {
        m_singleTaskPhaseSupported = m_singleTaskPhaseSupportedInPak = false;
    }

    //so far only validate Tiling for VDEnc VP9
    uint8_t col = (1 << (m_vp9PicParams->log2_tile_columns));
    uint8_t row = (1 << (m_vp9PicParams->log2_tile_rows));

    // Single pipe cannot handle N X M or M X N tile row/column cases, either one has to be 1, return error
    if ((col > 1) && (row > 1) && (m_numPipe == 1))
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Single pipe cannot handle N X M or M X N tile row/column cases, either one has to be 1");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Handling invalid tiling and scalability cases. When NumTilingColumn does not match NumPipe fall back to single pipe mode
    if (m_numPipe > 1 && (col != m_numPipe))
    {
        if ((col == 1) || (row == 1))
        {
            m_numPipe = 1; // number of tile columns cannot be greater than number of pipes (VDBOX), run in single pipe mode
            m_scalableMode = false;
        }
        else
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Number of tile columns cannot be greater than number of pipes (VDBOX) when number of rows > 1");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    //num columns must be either 2 or 4 for scalability mode, H/W limitation
    if ((m_numPipe > 1) && (m_numPipe != 2) && (m_numPipe != 4))
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Num pipes must be either 2 or 4 for scalability mode, H/W limitation");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Tile width needs to be minimum size 256, error out if less
    if ((col != 1) && ((m_vp9PicParams->SrcFrameWidthMinus1 + 1) < col * CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH))
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Incorrect number of columns input parameter, Tile width is < 256");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (row > 4)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Max number of rows cannot exceeds 4 by VP9 Spec.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_vdencEnabled && VP9_ENCODED_CHROMA_FORMAT_YUV444 == m_vp9SeqParams->SeqFlags.fields.EncodedFormat && m_vp9SeqParams->TargetUsage == 7)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Speed mode is not supported in VDENC 444, resetting TargetUsage to Normal mode\n");
        m_vp9SeqParams->TargetUsage = 4;
    }

    // number of tiles for this frame
    m_numberTilesInFrame = col * row;
    m_numUsedVdbox = m_numPipe;

    if (!m_newSeq)
    {
        // If there is no new SEQ header, then the number of passes is decided here.
        // Otherwise, it is done in SetSequenceStructs. For example, BRC setting may be changed.
        m_numPasses = (m_numPassesInOnePipe + 1) * m_numPipe - 1;
    }
    // Last place where scalable mode is decided
    if (m_frameNum == 0)
    {
        m_lastFrameScalableMode = m_scalableMode;
    }
    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CodechalEncoderState::SetGpuCtxCreatOption();
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeScalability_ConstructParmsForGpuCtxCreation(
            m_scalabilityState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    CODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS scalSetParms;
    MOS_ZeroMemory(&scalSetParms, sizeof(CODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS));

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        scalSetParms.bNeedSyncWithPrevious = true;
    }

    int32_t currentPass = GetCurrentPass();
    uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
    // Scalable mode only
    if (m_scalableMode)
    {
        for (auto i = 0; i < m_numPipe; i++)
        {
            scalSetParms.veBatchBuffer[i] = m_veBatchBuffer[m_virtualEngineBBIndex][i][passIndex].OsResource;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_SetHintParams(this, m_scalabilityState, &scalSetParms));
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_PopulateHintParams(m_scalabilityState, cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetTileData()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 *tileCodingParams = m_tileParams;

    tileCodingParams->Mode = CODECHAL_ENCODE_MODE_VP9;

    uint32_t numTileRows    = (1 << m_vp9PicParams->log2_tile_rows);
    uint32_t numTileColumns = (1 << m_vp9PicParams->log2_tile_columns);
    uint32_t numTiles = numTileRows * numTileColumns;

    uint32_t bitstreamSizePerTile = m_bitstreamUpperBound / (numTiles * CODECHAL_CACHELINE_SIZE);
    uint32_t numLcusInTiles = 0, numCuRecord = 64;
    uint32_t cuLevelStreamoutOffset = 0, sliceSizeStreamoutOffset = 0, bitstreamByteOffset = 0, sseRowstoreOffset = 0;

    for (uint32_t tileCntr = 0; tileCntr < numTiles; tileCntr++)
    {
        uint32_t tileX, tileY, tileStartSbX, tileStartSbY, tileWidthInSb, tileHeightInSb, lastTileColWidth, lastTileRowHeight, numLcuInTile;
        bool isLastTileCol, isLastTileRow;

        tileX = tileCntr % numTileColumns;
        tileY = tileCntr / numTileColumns;

        isLastTileCol = ((numTileColumns - 1) == tileX);
        isLastTileRow = ((numTileRows - 1) == tileY);

        tileStartSbX = (tileX * m_picWidthInSb) >> m_vp9PicParams->log2_tile_columns;
        tileStartSbY = (tileY * m_picHeightInSb) >> m_vp9PicParams->log2_tile_rows;

        tileWidthInSb  = (isLastTileCol ? m_picWidthInSb : (((tileX + 1) * m_picWidthInSb) >> m_vp9PicParams->log2_tile_columns)) - tileStartSbX;
        tileHeightInSb = (isLastTileRow ? m_picHeightInSb : (((tileY + 1) * m_picHeightInSb) >> m_vp9PicParams->log2_tile_rows)) - tileStartSbY;

        lastTileColWidth  = (MOS_ALIGN_CEIL((m_vp9PicParams->SrcFrameWidthMinus1 + 1 - tileStartSbX * CODEC_VP9_SUPER_BLOCK_WIDTH), CODEC_VP9_MIN_BLOCK_WIDTH) / CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
        lastTileRowHeight = (MOS_ALIGN_CEIL((m_vp9PicParams->SrcFrameHeightMinus1 + 1 - tileStartSbY * CODEC_VP9_SUPER_BLOCK_HEIGHT), CODEC_VP9_MIN_BLOCK_HEIGHT) / CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

        numLcuInTile = tileWidthInSb * tileHeightInSb;
        tileCodingParams[tileCntr].NumberOfActiveBePipes     = m_numPipe;
        tileCodingParams[tileCntr].NumOfTilesInFrame         = numTiles;
        tileCodingParams[tileCntr].NumOfTileColumnsInFrame   = numTileColumns;
        tileCodingParams[tileCntr].TileStartLCUX             = tileStartSbX;
        tileCodingParams[tileCntr].TileStartLCUY             = tileStartSbY;
        tileCodingParams[tileCntr].IsLastTileofColumn        = isLastTileRow;
        tileCodingParams[tileCntr].IsLastTileofRow           = isLastTileCol;

        tileCodingParams[tileCntr].TileWidthInMinCbMinus1    = isLastTileCol ? lastTileColWidth : (tileWidthInSb * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
        tileCodingParams[tileCntr].TileHeightInMinCbMinus1   = isLastTileRow ? lastTileRowHeight : (tileHeightInSb * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

        if (m_scalableMode)
        {
            sseRowstoreOffset = (tileStartSbX + (3 * tileX)) << 5;

            tileCodingParams[tileCntr].CuRecordOffset = MOS_ALIGN_CEIL(((numCuRecord * numLcusInTiles) * 64),
                CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;
            tileCodingParams[tileCntr].presHcpSyncBuffer        = &m_hcpScalabilitySyncBuffer.sResource;
            tileCodingParams[tileCntr].SliceSizeStreamoutOffset = sliceSizeStreamoutOffset;
            tileCodingParams[tileCntr].SseRowstoreOffset        = sseRowstoreOffset;
            tileCodingParams[tileCntr].BitstreamByteOffset      = bitstreamByteOffset;
            tileCodingParams[tileCntr].CuLevelStreamoutOffset   = cuLevelStreamoutOffset;

            cuLevelStreamoutOffset   += (tileCodingParams[tileCntr].TileWidthInMinCbMinus1 + 1) * (tileCodingParams[tileCntr].TileHeightInMinCbMinus1 + 1);
            sliceSizeStreamoutOffset += (tileCodingParams[tileCntr].TileWidthInMinCbMinus1 + 1) * (tileCodingParams[tileCntr].TileHeightInMinCbMinus1 + 1);
            sseRowstoreOffset        += (numLcuInTile * m_sizeOfSseSrcPixelRowStoreBufferPerLcu) / CODECHAL_CACHELINE_SIZE;
            bitstreamByteOffset      += bitstreamSizePerTile;
            numLcusInTiles           += numLcuInTile;

            tileCodingParams[tileCntr].TileSizeStreamoutOffset = (tileCntr*m_hcpInterface->GetPakHWTileSizeRecordSize() + CODECHAL_CACHELINE_SIZE - 1) / CODECHAL_CACHELINE_SIZE;

            //DW5
            const uint32_t frameStatsStreamoutSize = m_brcPakStatsBufSize;
            tileCodingParams[tileCntr].PakTileStatisticsOffset = (tileCntr*frameStatsStreamoutSize + CODECHAL_CACHELINE_SIZE - 1) / CODECHAL_CACHELINE_SIZE;

            //DW12
            tileCodingParams[tileCntr].Vp9ProbabilityCounterStreamoutOffset = ((tileCntr * m_probabilityCounterBufferSize) + (CODECHAL_CACHELINE_SIZE - 1)) / CODECHAL_CACHELINE_SIZE;
        }
        else
        {
            tileCodingParams[tileCntr].CuRecordOffset = 0;
            tileCodingParams[tileCntr].presHcpSyncBuffer = nullptr;
            tileCodingParams[tileCntr].SliceSizeStreamoutOffset = 0;
            tileCodingParams[tileCntr].SseRowstoreOffset = 0;
            tileCodingParams[tileCntr].BitstreamByteOffset = 0;
            tileCodingParams[tileCntr].CuLevelStreamoutOffset = 0;
            tileCodingParams[tileCntr].TileSizeStreamoutOffset = 0;

            //DW5
            tileCodingParams[tileCntr].PakTileStatisticsOffset = 0;

            //DW12
            tileCodingParams[tileCntr].Vp9ProbabilityCounterStreamoutOffset = 0;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetTileCommands(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11 vdencWalkerStateParams;
    vdencWalkerStateParams.Mode             = CODECHAL_ENCODE_MODE_VP9;
    vdencWalkerStateParams.pVp9EncPicParams = m_vp9PicParams;
    vdencWalkerStateParams.dwNumberOfPipes  = VDENC_PIPE_SINGLE_PIPE;

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    // MFXPipeDone should not be set for tail insertion
    vdPipelineFlushParams.Flags.bWaitDoneMFX =
        (m_lastPicInStream || m_lastPicInSeq) ? 0 : 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
    vdPipelineFlushParams.Flags.bFlushVDENC    = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

    if (IsFirstPipe() && IsFirstPass())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTileData());
    }

    MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS vdencWeightOffsetParams;
    uint32_t                             numTileColumns  = (1 << m_vp9PicParams->log2_tile_columns);
    uint32_t                             numTileRows     = (1 << m_vp9PicParams->log2_tile_rows);
    int currentPipe     = GetCurrentPipe();
    for (uint32_t tileRow = 0, tileIdx = 0; tileRow < numTileRows; tileRow++)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++, tileIdx++)
        {
            if (m_numPipe > 1)
            {
                if (tileCol != currentPipe)
                {
                    continue;
                }
            }

            // HCP_TILE_CODING commmand
            CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG11 *>(m_hcpInterface)->AddHcpTileCodingCmd(cmdBuffer, &m_tileParams[tileIdx]));

            MOS_ZeroMemory(&vdencWeightOffsetParams, sizeof(vdencWeightOffsetParams));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWeightsOffsetsStateCmd(cmdBuffer, nullptr, &vdencWeightOffsetParams));

            vdencWalkerStateParams.pTileCodingParams = &m_tileParams[tileIdx];
            vdencWalkerStateParams.dwTileId          = tileIdx;
            switch (m_numPipe)
            {
            case 0:
            case 1:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_SINGLE_PIPE;
                break;
            case 2:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_TWO_PIPE;
                break;
            case 4:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_FOUR_PIPE;
                break;
            default:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_INVALID;
                CODECHAL_ENCODE_ASSERTMESSAGE("Num Pipes invalid");
                return eStatus;
                break;
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(cmdBuffer, &vdPipelineFlushParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ExecuteTileLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    int currentPipe = GetCurrentPipe();
    int currentPass  = GetCurrentPass();

    if(currentPipe < 0 || currentPass < 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (IsFirstPipe())
    {
        MHW_BATCH_BUFFER secondLevelBatchBuffer;
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
        secondLevelBatchBuffer.dwOffset = 0;
        secondLevelBatchBuffer.bSecondLevel = true;

        if (!m_hucEnabled)
        {
            secondLevelBatchBuffer.OsResource = m_resHucPakInsertUncompressedHeaderReadBuffer;
        }
        else
        {
            secondLevelBatchBuffer.OsResource = m_resHucPakInsertUncompressedHeaderWriteBuffer;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            &secondLevelBatchBuffer));
    }

    // Setup Tile level PAK commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTileCommands(&cmdBuffer));

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    if (!Mos_ResourceIsNull(&m_stitchWaitSemaphoreMem[currentPipe].sResource))
    {
        flushDwParams.pOsResource = &m_stitchWaitSemaphoreMem[currentPipe].sResource;
        flushDwParams.dwDataDW1 = currentPass + 1;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (IsFirstPipe())
    {
        if (m_numPipe > 1 && m_enableTileStitchByHW)
        {
            for (auto i = 1; i < m_numPipe; i++)
            {
                if (!Mos_ResourceIsNull(&m_stitchWaitSemaphoreMem[i].sResource) && m_hucEnabled)
                {
                    // This semaphore waits for all pipes except pipe 1 vdenc+pak to finish processing before stitching bitstream
                    SendHWWaitCommand(&m_stitchWaitSemaphoreMem[i].sResource, &cmdBuffer, (currentPass + 1));
                }
            }

            uint32_t index = m_virtualEngineBBIndex;
            HucCopyParams copyParams;

            copyParams.size = m_tileRecordBuffer[index].sResource.iSize;
            copyParams.presSrc = &m_tileRecordBuffer[index].sResource;

            copyParams.presDst = &m_resBitstreamBuffer;
            copyParams.lengthOfTable = (uint8_t)(m_numberTilesInFrame);

            auto hucCmdInitializer = static_cast<CodechalCmdInitializerG11*>(m_hucCmdInitializer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(hucCmdInitializer->AddCopyCmds(&cmdBuffer, &copyParams));
        }
        // PAK integration kernel to integrate stats for next HUC pass
        if (m_scalableMode && m_hucEnabled && m_isTilingSupported && IsFirstPipe())
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCVp9PakInt(&cmdBuffer));
        }

        if (m_scalableMode && IsLastPass())
        {
            // In scalablemode, singletaskPhaseSupported is off. On last pass we have to make sure to signal
            // m_lastTaskInPhase flag to true so that end status report can send a sync tag
            m_lastTaskInPhase = true;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

        if (!m_scalableMode) // single pipe mode can read the info from MMIO register. Otherwise, we have to use the tile size statistic buffer
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));
        }
    }

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    if (m_singleTaskPhaseSupported && m_hucEnabled && IsLastPass())
    {
        m_lastTaskInPhase = true; //HPU singletask phase mode only
    }

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string currPassName = "PAK_PASS" + std::to_string((int)m_currPass);
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        CODECHAL_NUM_MEDIA_STATES,
        currPassName.data())));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));
    
    if (IsFirstPipe() &&
        m_waitForEnc &&
        IsFirstPass() &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_waitForEnc = false;
    }

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        bool renderFlags = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderFlags));
        m_lastTaskInPhase = false;

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                m_resVdencPakObjCmdStreamOutBuffer,
                CodechalDbgAttr::attrPakObjStreamout,
                currPassName.data(),
                m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
                0,
                CODECHAL_NUM_MEDIA_STATES));

            if (m_vp9PicParams->PicFlags.fields.segmentation_enabled) {
                //CodecHal_DbgDumpEncodeVp9SegmentStreamout(m_debugInterface, m_encoder);
                //m_debugInterface->DumpBuffer(
                //    (PCODECHAL_ENCODE_VP9_STATE)pvStandardState.resVdencSegmentMapStreamOut,
                //    CodechalDbgAttr::attrOutput,
                //    "SegMap_Out",
                //    CODECHAL_CACHELINE_SIZE * MOS_ROUNDUP_DIVIDE(pEncoder->dwFrameHeight, 64) * MOS_ROUNDUP_DIVIDE(pEncoder->dwFrameWidth, 64),
                //    0,
                //    CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_RECON);
            }

            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            });
    }

    if (IsFirstPipe() && IsLastPass())
    {
        if (m_vp9PicParams->PicFlags.fields.super_frame && m_tsEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructSuperFrame());
        }
    }

    // Increment the second level batch buffer index at the end of every pass
    if (IsLastPipe())
    {
        if (m_hucEnabled)
        {
            // We save the index of the 2nd level batch buffer in case there is a pass that needs the last SLBB
            m_lastVdencPictureState2ndLevelBBIndex = m_vdencPictureState2ndLevelBBIndex;
        }
        m_vdencPictureState2ndLevelBBIndex = (m_vdencPictureState2ndLevelBBIndex + 1) % CODECHAL_VP9_ENCODE_RECYCLED_BUFFER_NUM;
    }

    // Reset parameters for next PAK execution
    if (IsLastPipe() && IsLastPass())
    {

        if ((currentPipe == 0) &&
            m_signalEnc &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // signal semaphore
            MOS_SYNC_PARAMS syncParams;
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_semaphoreObjCount++;
        }

        m_prevFrameInfo.KeyFrame    = !m_vp9PicParams->PicFlags.fields.frame_type;
        m_prevFrameInfo.IntraOnly   = (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME) || m_vp9PicParams->PicFlags.fields.intra_only;
        m_prevFrameInfo.ShowFrame   = m_vp9PicParams->PicFlags.fields.show_frame;
        m_prevFrameInfo.FrameWidth  = m_oriFrameWidth;
        m_prevFrameInfo.FrameHeight = m_oriFrameHeight;
        m_currMvTemporalBufferIndex ^= 0x01;
        m_contextFrameTypes[m_vp9PicParams->PicFlags.fields.frame_context_idx] = m_vp9PicParams->PicFlags.fields.frame_type;
        m_prevFrameSegEnabled                                                  = m_vp9PicParams->PicFlags.fields.segmentation_enabled;

        // Reset parameters for next PAK execution
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
        // Save the last frame's scalable mode flag to prevent switching buffers when doing next pass
        m_lastFrameScalableMode = m_scalableMode;

    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    return ExecuteTileLevel();
}

PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CodechalVdencVp9StateG11::CreateMhwVdboxPipeModeSelectParams()
{
    auto pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11);

    return pipeModeSelectParams;
}

void CodechalVdencVp9StateG11::SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalVdencVp9State::SetHcpPipeModeSelectParams(pipeModeSelectParams);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11& pipeModeSelectParamsG11 = static_cast<MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11&>(pipeModeSelectParams);

    pipeModeSelectParamsG11.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    pipeModeSelectParamsG11.PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    if (m_scalableMode)
    {
        // Running in the multiple VDBOX mode
        if (IsFirstPipe())
        {
            pipeModeSelectParamsG11.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
        }
        else
        {
            if (IsLastPipe())
            {
                pipeModeSelectParamsG11.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
            }
            else
            {
                pipeModeSelectParamsG11.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
            }
        }

        pipeModeSelectParamsG11.PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
    }

    return;
}

void CodechalVdencVp9StateG11::SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams)
{
    CodechalVdencVp9State::SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);

    PCODECHAL_ENCODE_BUFFER tileRecordBuffer = &m_tileRecordBuffer[m_virtualEngineBBIndex];
    bool useTileRecordBuffer = !Mos_ResourceIsNull(&tileRecordBuffer->sResource);

    if (m_scalableMode && m_hucEnabled && m_isTilingSupported)
    {
        // overwrite presProbabilityCounterBuffer and it's params for scalable mode
        indObjBaseAddrParams.presProbabilityCounterBuffer = &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource;
        indObjBaseAddrParams.dwProbabilityCounterOffset = m_tileStatsOffset.counterBuffer;
        indObjBaseAddrParams.dwProbabilityCounterSize = m_statsSize.counterBuffer;
    }

    indObjBaseAddrParams.presPakTileSizeStasBuffer = useTileRecordBuffer? &tileRecordBuffer->sResource : nullptr;
    indObjBaseAddrParams.dwPakTileSizeStasBufferSize = useTileRecordBuffer? ((m_statsSize.tileSizeRecord) * GetNumTilesInFrame()) : 0;
    indObjBaseAddrParams.dwPakTileSizeRecordOffset = useTileRecordBuffer? m_tileStatsOffset.tileSizeRecord: 0;

}

MOS_STATUS CodechalVdencVp9StateG11::VerifyCommandBufferSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (UseLegacyCommandBuffer())   // legacy mode & resize CommandBuffer Size for every BRC pass
    {
        if (!m_singleTaskPhaseSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
        }
    }
    else    // virtual engine
    {
        uint32_t requestedSize =
            m_pictureStatesSize +
            m_picturePatchListSize +
            m_extraPictureStatesSize +
            (m_sliceStatesSize * m_numSlices);
        requestedSize += requestedSize*m_numPassesInOnePipe;
        if (m_hucEnabled && m_brcEnabled)
        {
            requestedSize += m_brcMaxNumPasses*(m_defaultHucCmdsSize+m_defaultHucPatchListSize);
        }
        // Running in the multiple VDBOX mode
        int currentPipe = GetCurrentPipe();
        int currentPass = GetCurrentPass();
        if (currentPipe < 0 || currentPipe >= m_numPipe)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        if (currentPass < 0 || currentPass >= m_brcMaxNumPasses)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (IsFirstPipe() && m_osInterface->bUsesPatchList)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
        }
        uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
        PMOS_COMMAND_BUFFER cmdBuffer;
        if (m_osInterface->phasedSubmission)
        {
            cmdBuffer = &m_realCmdBuffer;
        }
        else
        {
            cmdBuffer = &m_veBatchBuffer[m_virtualEngineBBIndex][(uint32_t)currentPipe][passIndex];
        }

        if (Mos_ResourceIsNull(&cmdBuffer->OsResource) ||
            m_sizeOfVEBatchBuffer < requestedSize)
        {
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;
            allocParamsForBufferLinear.dwBytes = requestedSize;
            allocParamsForBufferLinear.pBufName = "Batch buffer for each VDBOX";

            if (!Mos_ResourceIsNull(&cmdBuffer->OsResource))
            {
                if (cmdBuffer->pCmdBase)
                {
                    m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
                }
                m_osInterface->pfnFreeResource(m_osInterface, &cmdBuffer->OsResource);
            }

            eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &cmdBuffer->OsResource);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

            m_sizeOfVEBatchBuffer = requestedSize;
        }

        if (cmdBuffer->pCmdBase == 0)
        {
            MOS_LOCK_PARAMS lockParams;
            MOS_ZeroMemory(&lockParams, sizeof(lockParams));
            lockParams.WriteOnly = true;
            cmdBuffer->pCmdPtr = cmdBuffer->pCmdBase = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &cmdBuffer->OsResource, &lockParams);
            cmdBuffer->iRemaining = m_sizeOfVEBatchBuffer;
            cmdBuffer->iOffset = 0;

            if (cmdBuffer->pCmdBase == nullptr)
            {
                return MOS_STATUS_NULL_POINTER;
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::GetCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (UseLegacyCommandBuffer())        // legacy mode
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, 0));
    }
    else    // virtual engine
    {
        if (m_osInterface->phasedSubmission)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_realCmdBuffer, 0));
            *cmdBuffer = m_realCmdBuffer;
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_realCmdBuffer, 0));

            int currentPipe = GetCurrentPipe();
            int currentPass = GetCurrentPass();
            if (currentPipe < 0 || currentPipe >= m_numPipe)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
            uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
            *cmdBuffer = m_veBatchBuffer[m_virtualEngineBBIndex][currentPipe][passIndex];
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ReturnCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (UseLegacyCommandBuffer())        // legacy mode
    {
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);
    }
    else    // virtual engine
    {
        if (m_osInterface->phasedSubmission)
        {
            m_realCmdBuffer = *cmdBuffer;
            m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_realCmdBuffer, 0);
        }
        else
        {
            int currentPipe = GetCurrentPipe();
            int currentPass = GetCurrentPass();
            if (currentPipe < 0 || currentPipe >= m_numPipe)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (eStatus == MOS_STATUS_SUCCESS)
            {
                uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
                m_veBatchBuffer[m_virtualEngineBBIndex][currentPipe][passIndex] = *cmdBuffer;
                m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_realCmdBuffer, 0);
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (UseLegacyCommandBuffer())        // legacy mode
    {
        if (!IsRenderContext()) // Set VE Hints for video contexts only
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
    }
    else // virtual engine
    {
        if (m_osInterface->phasedSubmission)
        {
            CodecHalEncodeScalability_EncodePhaseToSubmissionType(IsFirstPipe(),&m_realCmdBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_realCmdBuffer, nullRendering));
        }
        else
        {
            if (!IsLastPipe())
            {
                return eStatus;
            }
            int currentPass = GetCurrentPass();
            for (auto i = 0; i < m_numPipe; i++)
            {
                uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
                PMOS_COMMAND_BUFFER cmdBuffer = &m_veBatchBuffer[m_virtualEngineBBIndex][i][passIndex];

                if (cmdBuffer->pCmdBase)
                {
                    m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
                }

                cmdBuffer->pCmdBase = 0;
                cmdBuffer->iOffset = cmdBuffer->iRemaining = 0;
            }

            if (eStatus == MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&m_realCmdBuffer));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_realCmdBuffer, nullRendering));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool frameTrackingRequested,
    MHW_MI_MMIOREGISTERS *mmioRegister)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (IsRenderContext()) //Render context only
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTrackingRequested));
        return eStatus;
    }
    else // Legacy mode or virtual engine mode
    {
        if (!IsLastPipe())
        {
            return eStatus;
        }
        PMOS_COMMAND_BUFFER commandBufferInUse;
        if (m_realCmdBuffer.pCmdBase && m_scalableMode)
        {
            commandBufferInUse = &m_realCmdBuffer; //virtual engine mode
        }
        else
        {
            if (cmdBuffer && cmdBuffer->pCmdBase)
            {
                commandBufferInUse = cmdBuffer; //legacy mode
            }
            else
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        commandBufferInUse->Attributes.bTurboMode = m_hwInterface->m_turboMode;
        commandBufferInUse->Attributes.dwNumRequestedEUSlices = m_hwInterface->m_numRequestedEuSlices;
        commandBufferInUse->Attributes.dwNumRequestedSubSlices = m_hwInterface->m_numRequestedSubSlices;
        commandBufferInUse->Attributes.dwNumRequestedEUs = m_hwInterface->m_numRequestedEus;
        commandBufferInUse->Attributes.bValidPowerGatingRequest = true;
        if (frameTrackingRequested && m_frameTrackingEnabled)
        {
            commandBufferInUse->Attributes.bEnableMediaFrameTracking = true;
            commandBufferInUse->Attributes.resMediaFrameTrackingSurface =
                m_encodeStatusBuf.resStatusBuffer;
            commandBufferInUse->Attributes.dwMediaFrameTrackingTag = m_storeData;
            // Set media frame tracking address offset(the offset from the encoder status buffer page)
            commandBufferInUse->Attributes.dwMediaFrameTrackingAddrOffset = 0;
        }

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface = m_hwInterface->GetOsInterface();
        genericPrologParams.pvMiInterface = m_hwInterface->GetMiInterface();
        genericPrologParams.bMmcEnabled = CodecHalMmcState::IsMmcEnabled();
        genericPrologParams.dwStoreDataValue = m_storeData - 1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(commandBufferInUse, &genericPrologParams));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetSemaphoreMem(
    PMOS_RESOURCE               semaphoreMem,
    PMOS_COMMAND_BUFFER         cmdBuffer,
    uint32_t                    value)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(semaphoreMem);
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = semaphoreMem;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue = value;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SendHWWaitCommand(
    PMOS_RESOURCE               semaphoreMem,
    PMOS_COMMAND_BUFFER         cmdBuffer,
    uint32_t                    value)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(semaphoreMem);

    MHW_MI_SEMAPHORE_WAIT_PARAMS semaphoreWaitParams;
    MOS_ZeroMemory(&semaphoreWaitParams, sizeof(semaphoreWaitParams));
    semaphoreWaitParams.presSemaphoreMem = semaphoreMem;
    semaphoreWaitParams.bPollingWaitMode = true;
    semaphoreWaitParams.dwSemaphoreData = value;
    semaphoreWaitParams.CompareOperation = MHW_MI_SAD_EQUAL_SDD;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiSemaphoreWaitCmd(cmdBuffer, &semaphoreWaitParams));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetDmemHuCPakInt()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t currPass = (uint8_t)GetCurrentPass();

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    // All bytes in below dmem for fields not used by VP9 to be set to 0xFF.
    HucPakIntDmem* dmem = (HucPakIntDmem*)m_osInterface->pfnLockResource(
        m_osInterface, &m_hucPakIntDmemBuffer[m_currRecycledBufIdx][currPass], &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dmem);

    MOS_ZeroMemory(dmem, sizeof(HucPakIntDmem));
    // CODECHAL_VDENC_VP9_PAK_INT_DMEM_OFFSETS_SIZE size of offsets in the CODECHAL_VDENC_VP9_HUC_PAK_INT_DMEM struct.
    // Reset offsets to 0xFFFFFFFF as unavailable
    memset(dmem, 0xFF, m_pakIntDmemOffsetsSize);

    dmem->totalSizeInCommandBuffer = 0;
    dmem->offsetInCommandBuffer = 0xFFFF; // Not used for VP9, all bytes in dmem for fields not used are 0xFF
    dmem->picWidthInPixel = (uint16_t)m_frameWidth;
    dmem->picHeightInPixel = (uint16_t)m_frameHeight;
    dmem->totalNumberOfPaks = m_numPipe;
    dmem->codec = m_pakIntVp9CodecId;
    dmem->maxPass = m_brcMaxNumPasses; // Only VDEnc CQP and BRC
    dmem->currentPass = currPass + 1;
    dmem->lastTileBSStartInBytes = 0xFFFF;
    dmem->picStateStartInBytes = 0xFFFF;

    // Offset 0 is for region 1 - output of integrated frame stats from PAK integration kernel

    dmem->tileSizeRecordOffset[0] = m_frameStatsOffset.tileSizeRecord;
    dmem->vdencStatOffset[0] = m_frameStatsOffset.vdencStats;
    dmem->vp9PakStatOffset[0] = m_frameStatsOffset.pakStats;
    dmem->vp9CounterBufferOffset[0] = m_frameStatsOffset.counterBuffer;

    //Offset 1 - 4 is for region 0 - Input to PAK integration kernel for all tile statistics per pipe
    for (auto i = 1; i <= m_numPipe; i++)
    {
        dmem->numTiles[i - 1] = (GetNumTilesInFrame()) / m_numPipe;
        dmem->tileSizeRecordOffset[i] = m_tileStatsOffset.tileSizeRecord + ((i - 1)*(dmem->numTiles[i - 1])*m_statsSize.tileSizeRecord);
        dmem->vdencStatOffset[i] = m_tileStatsOffset.vdencStats + ((i - 1)*(dmem->numTiles[i - 1])*m_statsSize.vdencStats);
        dmem->vp9PakStatOffset[i] = m_tileStatsOffset.pakStats + ((i - 1)*(dmem->numTiles[i - 1])*m_statsSize.pakStats);
        dmem->vp9CounterBufferOffset[i] = m_tileStatsOffset.counterBuffer + ((i - 1)*(dmem->numTiles[i - 1])*m_statsSize.counterBuffer);
    }
    m_osInterface->pfnUnlockResource(m_osInterface, &m_hucPakIntDmemBuffer[m_currRecycledBufIdx][currPass]);

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::HuCVp9PakInt(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!IsFirstPipe())
    {
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        uint32_t    hucRegionSize[16]   = { 0 };
        const char* hucRegionName[16] = { "\0" };

        hucRegionName[0] = "_MultiPakStreamout_input";
        hucRegionSize[0] = m_tileStatsPakIntegrationBufferSize;
        hucRegionName[1] = "_IntegratedStreamout_output";
        hucRegionSize[1] = m_frameStatsPakIntegrationBufferSize;
        hucRegionName[4] = "_BitStream_input";
        hucRegionSize[4] = MOS_ALIGN_CEIL(m_bitstreamUpperBound, CODECHAL_PAGE_SIZE);
        hucRegionName[5] = "_BitStream_output";
        hucRegionSize[5] = MOS_ALIGN_CEIL(m_bitstreamUpperBound, CODECHAL_PAGE_SIZE);
        hucRegionName[6] = "_HistoryBufferOutput";
        hucRegionSize[6] = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        hucRegionName[7] = "_HCPPICSTATEInputDummy";
        hucRegionSize[7] = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        hucRegionName[8] = "_HCPPICSTATEOutputDummy";
        hucRegionSize[8] = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        hucRegionName[9] = "_BrcDataOutputBuffer"; // This is the pak MMIO region 7 , not 4, of BRC update
        hucRegionSize[9] = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);
        hucRegionName[15] = "_TileRecordBuffer"; // This is the pak MMIO region 7 , not 4, of BRC update
        hucRegionSize[15] = m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterface->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE);
    )

    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor =  m_vdboxHucPakIntegrationKernelDescriptor;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCPakInt());

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_hucPakIntDmemBuffer[m_currRecycledBufIdx][GetCurrentPass()];
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(sizeof(HucPakIntDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));
    virtualAddrParams.regionParams[0].presRegion = &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource; // Region 0 - Tile based input statistics from PAK/ VDEnc
    virtualAddrParams.regionParams[0].dwOffset = 0;
    virtualAddrParams.regionParams[1].presRegion = &m_frameStatsPakIntegrationBuffer.sResource; // Region 1 - HuC Frame statistics output
    virtualAddrParams.regionParams[1].isWritable = true;
    virtualAddrParams.regionParams[4].presRegion = &m_hucPakIntDummyBuffer;             // Region 4 - Not used for VP9
    virtualAddrParams.regionParams[5].presRegion = &m_hucPakIntDummyBuffer;             // Region 5 - Not used for VP9
    virtualAddrParams.regionParams[5].isWritable = true;
    virtualAddrParams.regionParams[6].presRegion = &m_hucPakIntDummyBuffer;             // Region 6 - Not used for VP9
    virtualAddrParams.regionParams[6].isWritable = true;
    virtualAddrParams.regionParams[7].presRegion = &m_hucPakIntDummyBuffer;             // Region 7 - Not used for VP9
    virtualAddrParams.regionParams[8].presRegion = &m_hucPakIntDummyBuffer;             // Region 8 - Not used for VP9
    virtualAddrParams.regionParams[8].isWritable = true;
    virtualAddrParams.regionParams[9].presRegion = &m_hucPakIntBrcDataBuffer;           // Region 9 - HuC outputs BRC data
    virtualAddrParams.regionParams[9].isWritable = true;
    virtualAddrParams.regionParams[15].presRegion = &m_tileRecordBuffer[m_virtualEngineBBIndex].sResource;          // Region 15 [In/Out] - Tile Record Buffer
    virtualAddrParams.regionParams[15].dwOffset   = 0; 

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ConstructPicStateBatchBuf(
    PMOS_RESOURCE picStateBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(picStateBuffer);

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hucCmdInitializer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucCmdInitializer->CommandInitializerSetVp9Params(this));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = false;
        //For Superframes, there is an extra submission at the end, so submit with frame tracking there
        if (!m_vp9PicParams->PicFlags.fields.super_frame) {
            requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
        m_firstTaskInPhase = false;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucCmdInitializer->CmdInitializerVp9Execute(&cmdBuffer, picStateBuffer));

    if (!m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd) && !m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    ReturnCommandBuffer(&cmdBuffer);

    if (!m_singleTaskPhaseSupported)
    {
        bool renderFlags = m_videoContextUsesNullHw;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderFlags));
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, picStateBuffer, &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase  = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr   = (uint32_t *)data;
    constructedCmdBuf.iOffset   = 0;
    constructedCmdBuf.iRemaining = m_vdencPicStateSecondLevelBatchBufferSize;

    // HCP_VP9_PIC_STATE
    MHW_VDBOX_VP9_ENCODE_PIC_STATE picState;
    MOS_ZeroMemory(&picState, sizeof(picState));
    picState.pVp9PicParams                    = m_vp9PicParams;
    picState.pVp9SeqParams                    = m_vp9SeqParams;
    picState.ppVp9RefList                     = &(m_refList[0]);
    picState.PrevFrameParams.fields.KeyFrame  = m_prevFrameInfo.KeyFrame;
    picState.PrevFrameParams.fields.IntraOnly = m_prevFrameInfo.IntraOnly;
    picState.PrevFrameParams.fields.Display   = m_prevFrameInfo.ShowFrame;
    picState.dwPrevFrmWidth                   = m_prevFrameInfo.FrameWidth;
    picState.dwPrevFrmHeight                  = m_prevFrameInfo.FrameHeight;
    picState.ucTxMode                         = m_txMode;
    picState.bSSEEnable                       = m_vdencBrcEnabled;
    picState.bUseDysRefSurface                = (m_dysRefFrameFlags != DYS_REF_NONE) && m_dysVdencMultiPassEnabled;
    picState.bVdencPakOnlyPassFlag            = m_vdencPakonlyMultipassEnabled;
    picState.uiMaxBitRate                     = m_vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    picState.uiMinBitRate                     = m_vp9SeqParams->MinBitRate * CODECHAL_ENCODE_BRC_KBPS;
    constructedCmdBuf.iOffset += m_cmd1Size;
    m_hucPicStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    constructedCmdBuf.pCmdPtr += constructedCmdBuf.iOffset/sizeof(uint32_t);
    eStatus = m_hcpInterface->AddHcpVp9PicStateEncCmd(&constructedCmdBuf, nullptr, &picState);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add HCP_VP9_PIC_STATE command.");
        return eStatus;
    }

    // HCP_VP9_SEGMENT_STATE
    MHW_VDBOX_VP9_SEGMENT_STATE segmentState;
    MOS_ZeroMemory(&segmentState, sizeof(segmentState));
    segmentState.Mode                    = m_mode;
    segmentState.pVp9EncodeSegmentParams = m_vp9SegmentParams;
    uint8_t segmentCount                 = (m_vp9PicParams->PicFlags.fields.segmentation_enabled) ? CODEC_VP9_MAX_SEGMENTS : 1;

    for (uint8_t i = 0; i < segmentCount; i++)
    {
        segmentState.ucCurrentSegmentId = i;
        eStatus = m_hcpInterface->AddHcpVp9SegmentStateCmd(&constructedCmdBuf, nullptr, &segmentState);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add MHW_VDBOX_VP9_SEGMENT_STATE command.");
            return eStatus;
        }
    }

    // Adjust cmd buffer offset to have 8 segment state blocks
    if (segmentCount < CODEC_VP9_MAX_SEGMENTS)
    {
        // Max 7 segments, 32 bytes each
        uint8_t zeroBlock[m_segmentStateBlockSize * (CODEC_VP9_MAX_SEGMENTS - 1)];
        MOS_ZeroMemory(zeroBlock, sizeof(zeroBlock));
        Mhw_AddCommandCmdOrBB(&constructedCmdBuf, nullptr, zeroBlock, (CODEC_VP9_MAX_SEGMENTS - segmentCount) * m_segmentStateBlockSize);
    }
    m_slbbImgStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    constructedCmdBuf.iOffset += m_cmd2Size;
    constructedCmdBuf.pCmdPtr += m_cmd2Size/ sizeof(uint32_t);

    // BB_END
    eStatus = m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add MI Batch Buffer End command.");
        return eStatus;
    }
    m_hucSlbbSize = (uint16_t)constructedCmdBuf.iOffset;

    m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::HuCVp9Prob()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (!IsFirstPipe())
    {
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        uint32_t hucRegionSize[16] = { 0 };
        const char* hucRegionName[16] = { "\0" };

        hucRegionName[0] = "_UpdatedProbBuffer";   // hucRegionName[0] is used to dump region 0 after HuC is run, which has updated probabilities. Input Region 0 is dumped separetely before HuC.
        hucRegionSize[0] = 32 * CODECHAL_CACHELINE_SIZE;
        hucRegionName[1] = "_CountersBuffer";
        hucRegionSize[1] = 193 * CODECHAL_CACHELINE_SIZE;
        hucRegionName[2] = "_ProbBuffer";
        hucRegionSize[2] = 32 * CODECHAL_CACHELINE_SIZE;
        hucRegionName[3] = "_ProbDeltaBuffer";
        hucRegionSize[3] = 29 * CODECHAL_CACHELINE_SIZE;
        hucRegionName[4] = "_UncompressedHdr";
        hucRegionSize[4] = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;
        hucRegionName[5] = "_CompressedHdr";
        hucRegionSize[5] = 32 * CODECHAL_CACHELINE_SIZE;
        hucRegionName[6] = "_SecondLevelBatchBuffer";
        hucRegionSize[6] = m_vdencPicStateSecondLevelBatchBufferSize;
        hucRegionName[7] = "_SecondLevelBatchBuffer";
        hucRegionSize[7] = m_vdencPicStateSecondLevelBatchBufferSize;
        hucRegionName[8] = "_UncompressedHdr";
        hucRegionSize[8] = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;
        hucRegionName[9] = "_DefaultProbs";
        hucRegionSize[9] = sizeof(Keyframe_Default_Probs) + sizeof(Inter_Default_Probs);
        hucRegionName[10] = "_SuperFrameBuffer";
        hucRegionSize[10] = CODECHAL_ENCODE_VP9_BRC_SUPER_FRAME_BUFFER_SIZE;
        hucRegionName[11] = "_DataExtension";
        hucRegionSize[11] = CODECHAL_ENCODE_VP9_VDENC_DATA_EXTENSION_SIZE;
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported || m_firstTaskInPhase) && !m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
        m_firstTaskInPhase = false;
    }
    int currPass = GetCurrentPass();
    
    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucVp9VdencProbKernelDescriptor;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCVp9Prob());

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_resHucProbDmemBuffer[currPass];
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(sizeof(HucProbDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    // Add Virtual addr
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    // Input regions
    virtualAddrParams.regionParams[0].presRegion = &m_resProbBuffer[m_vp9PicParams->PicFlags.fields.frame_context_idx];
    virtualAddrParams.regionParams[0].isWritable = true;        // Region 0 is both read and write for HuC. Has input probabilities before running HuC and updated probabilities after running HuC, which will then be input to next pass
    if (m_scalableMode)
    {
        virtualAddrParams.regionParams[1].presRegion = &m_frameStatsPakIntegrationBuffer.sResource;
        virtualAddrParams.regionParams[1].dwOffset = m_frameStatsOffset.counterBuffer;
    }
    else
    {
        virtualAddrParams.regionParams[1].presRegion = &m_resProbabilityCounterBuffer;
        virtualAddrParams.regionParams[1].dwOffset = 0;
    }
    // If BRC enabled, BRC Pass 2 output SLBB -> input SLBB for HPU on pass 3 (HPU pass 1 and 3. BRC Update pass 1 and 2)
    //                 BRC Pass 1 output SLBB -> input SLBB for HPU on pass 1
    // If BRC not on , Driver prepared SLBB   -> input to HPU on both passes

    if (m_vdencBrcEnabled)
    {
        virtualAddrParams.regionParams[7].presRegion = &m_resVdencPictureState2NdLevelBatchBufferWrite[0];
    }
    else
    {
        virtualAddrParams.regionParams[7].presRegion = &m_resVdencPictureState2NdLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex];
    }
    
    virtualAddrParams.regionParams[8].presRegion = &m_resHucPakInsertUncompressedHeaderReadBuffer;
    virtualAddrParams.regionParams[9].presRegion = &m_resHucDefaultProbBuffer;

    // Output regions
    virtualAddrParams.regionParams[2].presRegion  = &m_resHucProbOutputBuffer;  // Final probability output from HuC after each pass
    virtualAddrParams.regionParams[2].isWritable = true;
    virtualAddrParams.regionParams[3].presRegion  = &m_resProbabilityDeltaBuffer;
    virtualAddrParams.regionParams[3].isWritable = true;
    virtualAddrParams.regionParams[4].presRegion  = &m_resHucPakInsertUncompressedHeaderWriteBuffer;
    virtualAddrParams.regionParams[4].isWritable = true;
    virtualAddrParams.regionParams[5].presRegion  = &m_resCompressedHeaderBuffer;
    virtualAddrParams.regionParams[5].isWritable = true;
    virtualAddrParams.regionParams[6].presRegion  = &m_resVdencPictureState2NdLevelBatchBufferWrite[0];
    virtualAddrParams.regionParams[6].isWritable = true;
    virtualAddrParams.regionParams[10].presRegion = &m_resBitstreamBuffer;
    virtualAddrParams.regionParams[10].isWritable = true;
    virtualAddrParams.regionParams[11].presRegion = &m_resVdencDataExtensionBuffer;
    virtualAddrParams.regionParams[11].isWritable = true;

    m_hpuVirtualAddrParams = virtualAddrParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));
    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Write HUC_STATUS mask: DW1 (mask value)
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resHucPakMmioBuffer;
    storeDataParams.dwResourceOffset = sizeof(uint32_t);
    storeDataParams.dwValue = 1 << 31; //Repak bit for HUC is bit 31
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    // store HUC_STATUS register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resHucPakMmioBuffer;
    storeRegParams.dwOffset = 0;
    storeRegParams.dwRegister = m_hucInterface->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    // For superframe pass, after HuC executes, write the updated size (combined frame size) to status report
    // So app knows total size instead of just the showframe size
    if (m_superFrameHucPass)
    {
        EncodeStatusBuffer* encodeStatusBuf = &m_encodeStatusBuf;
        uint32_t baseOffset =
            (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            sizeof(uint32_t) * 2;  // encodeStatus is offset by 2 DWs in the resource

        MHW_MI_COPY_MEM_MEM_PARAMS copyMemMemParams;
        MOS_ZeroMemory(&copyMemMemParams, sizeof(copyMemMemParams));

        copyMemMemParams.presSrc = virtualAddrParams.regionParams[11].presRegion;
        copyMemMemParams.dwSrcOffset = 0; // Updated framesize is 1st DW in buffer
        copyMemMemParams.presDst = &encodeStatusBuf->resStatusBuffer;
        copyMemMemParams.dwDstOffset = baseOffset + encodeStatusBuf->dwBSByteCountOffset;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
            &cmdBuffer,
            &copyMemMemParams));
    }

    if ((!m_singleTaskPhaseSupported && !m_scalableMode) || m_superFrameHucPass)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    // Dump input probabilites before running HuC
    CODECHAL_DEBUG_TOOL(
       m_debugInterface->DumpHucRegion(
            virtualAddrParams.regionParams[0].presRegion,
            0,
            hucRegionSize[0],
            0,
            "_ProbBuffer",
            (virtualAddrParams.regionParams[0].isWritable ? true : false),
            currPass,
            CodechalHucRegionDumpType::hucRegionDumpHpu);
    )

    ReturnCommandBuffer(&cmdBuffer);

    // For Temporal scaling, super frame pass is initiated after the command buffer submission in ExecuteSliceLevel.
    // So if Single Task Phase is enabled, then we need to explicitly submit the command buffer here to call HuC
    if ((!m_singleTaskPhaseSupported && !m_scalableMode) || m_superFrameHucPass)
    {
        bool renderFlags = m_videoContextUsesNullHw;

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            ((currPass == 0)? "HPU_Pass0":"HPU_Pass1"))));

        if (m_superFrameHucPass) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));
            //For superframe submission, this is the last submission so add frame tracking header
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, m_vp9PicParams->PicFlags.fields.super_frame));
            ReturnCommandBuffer(&cmdBuffer);
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderFlags));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
                &m_resHucProbDmemBuffer[currPass],
                sizeof(HucProbDmem),
                currPass,
                CodechalHucRegionDumpType::hucRegionDumpHpu));

            for (auto i = 0; i < 16; i++) {
                if (virtualAddrParams.regionParams[i].presRegion)
                {
                    if (m_scalableMode && m_isTilingSupported && virtualAddrParams.regionParams[i].isWritable && i != 11)
                    {
                        continue;
                    }
                    m_debugInterface->DumpHucRegion(
                        virtualAddrParams.regionParams[i].presRegion,
                        virtualAddrParams.regionParams[i].dwOffset,
                        hucRegionSize[i],
                        i,
                        hucRegionName[i],
                        !virtualAddrParams.regionParams[i].isWritable,
                        currPass,
                        CodechalHucRegionDumpType::hucRegionDumpHpu);
                }
            })
    }

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : HuCBrcUpdate
| Purpose   : Start/Submit VP9 HuC BrcUpdate kernel to HW
|
| Returns   : MOS_STATUS
\---------------------------------------------------------------------------*/
MOS_STATUS CodechalVdencVp9StateG11::HuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    int currPass = GetCurrentPass();

    CODECHAL_DEBUG_TOOL(
        uint32_t hucRegionSize[16];
        const char* hucRegionName[16];

        hucRegionName[0] = "_BrcHistory";
        hucRegionSize[0] = m_brcHistoryBufferSize;
        hucRegionName[1] = "_VDEncStats";
        hucRegionSize[1] = m_vdencBrcStatsBufferSize;
        hucRegionName[2] = "_PAKStats";
        hucRegionSize[2] = m_vdencBrcPakStatsBufferSize;
        hucRegionName[3] = "_InputSLBB";
        hucRegionSize[3] = m_vdencPicStateSecondLevelBatchBufferSize;
        hucRegionName[4] = "_BRCData";
        hucRegionSize[4] = CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE;
        hucRegionName[5] = "_ConstData";
        hucRegionSize[5] = m_brcConstantSurfaceSize;
        hucRegionName[6] = "_OutputSLBB";
        hucRegionSize[6] = m_vdencPicStateSecondLevelBatchBufferSize;
        hucRegionName[7] = "_PAKMMIO";
        hucRegionSize[7] = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);
    )

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_swBrcMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcUpdate());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitBrcConstantBuffer(&m_brcBuffers.resBrcConstantDataBuffer, m_pictureCodingType));
        // Set region params for dumping only
        MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
        virtualAddrParams.regionParams[0].presRegion = &m_brcBuffers.resBrcHistoryBuffer;
        virtualAddrParams.regionParams[0].isWritable = true;
        virtualAddrParams.regionParams[1].presRegion = &m_resVdencBrcStatsBuffer;
        virtualAddrParams.regionParams[2].presRegion = &m_resFrameStatStreamOutBuffer;
        virtualAddrParams.regionParams[3].presRegion = &m_resVdencPictureState2NdLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex];
        virtualAddrParams.regionParams[4].presRegion = &m_brcBuffers.resBrcHucDataBuffer;
        virtualAddrParams.regionParams[4].isWritable = true;
        virtualAddrParams.regionParams[5].presRegion = &m_brcBuffers.resBrcConstantDataBuffer;
        virtualAddrParams.regionParams[6].presRegion = &m_resVdencPictureState2NdLevelBatchBufferWrite[0];
        virtualAddrParams.regionParams[6].isWritable = true;
        virtualAddrParams.regionParams[7].presRegion = &m_brcBuffers.resBrcBitstreamSizeBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(// Dump history IN since it's both IN/OUT, OUT will dump at end of function, rest of buffers are IN XOR OUT (not both)
            virtualAddrParams.regionParams[0].presRegion,
            virtualAddrParams.regionParams[0].dwOffset,
            hucRegionSize[0],
            0,
            hucRegionName[0],
            true,
            currPass,
            CodechalHucRegionDumpType::hucRegionDumpUpdate));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SoftwareBRC(true));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
                &m_resVdencBrcUpdateDmemBuffer[currPass],
                sizeof(HucBrcUpdateDmem),  // Change buffer and size to update dmem
                currPass,
                CodechalHucRegionDumpType::hucRegionDumpUpdate));

            for (auto i = 0; i < 16; i++) {
                if (virtualAddrParams.regionParams[i].presRegion)
                {
                    m_debugInterface->DumpHucRegion(
                        virtualAddrParams.regionParams[i].presRegion,
                        virtualAddrParams.regionParams[i].dwOffset,
                        hucRegionSize[i],
                        i,
                        hucRegionName[i],
                        !virtualAddrParams.regionParams[i].isWritable,
                        currPass,
                        CodechalHucRegionDumpType::hucRegionDumpUpdate);
                }
            });
        // We increment by the average frame value once for each frame
        if (IsFirstPass())
        {
            m_curTargetFullness += m_inputBitsPerFrame;
        }

        return eStatus;
    }
#endif

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported || (m_firstTaskInPhase && !m_brcInit)) && !m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

        m_firstTaskInPhase = false;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitBrcConstantBuffer(&m_brcBuffers.resBrcConstantDataBuffer, m_pictureCodingType));

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucVp9VdencBrcUpdateKernelDescriptor;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcUpdate());

    // set HuC DMEM param
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_resVdencBrcUpdateDmemBuffer[currPass];
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(sizeof(HucBrcUpdateDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS; // how to set?
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    // Set surfaces to HuC regions
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));

    // History Buffer - IN/OUT
    virtualAddrParams.regionParams[0].presRegion = &m_brcBuffers.resBrcHistoryBuffer;
    virtualAddrParams.regionParams[0].isWritable = true;
    if (IsFirstPass()) // First BRC pass needs stats from last frame
    {
        if (m_lastFrameScalableMode) // Frame (n-1) Scalable mode stats output -> input for frame n, BRC Pass 0
        {
            // VDEnc Stats Buffer - IN
            virtualAddrParams.regionParams[1].presRegion = &m_frameStatsPakIntegrationBuffer.sResource;
            virtualAddrParams.regionParams[1].dwOffset = m_tileStatsOffset.vdencStats;
            // Frame (not PAK) Stats Buffer - IN
            virtualAddrParams.regionParams[2].presRegion = &m_frameStatsPakIntegrationBuffer.sResource;
            virtualAddrParams.regionParams[2].dwOffset = m_frameStatsOffset.pakStats;
            // PAK MMIO - IN
            virtualAddrParams.regionParams[7].presRegion = &m_hucPakIntBrcDataBuffer;
        }
        else
        {
            virtualAddrParams.regionParams[1].presRegion = &m_resVdencBrcStatsBuffer;
            virtualAddrParams.regionParams[1].dwOffset = 0;
            virtualAddrParams.regionParams[2].presRegion = &m_resFrameStatStreamOutBuffer;
            virtualAddrParams.regionParams[2].dwOffset = 0;
            virtualAddrParams.regionParams[7].presRegion = &m_brcBuffers.resBrcBitstreamSizeBuffer;
        }
    }
    else // Second BRC Update Pass
    {
        if (m_scalableMode)
        {
            // VDEnc Stats Buffer - IN
            virtualAddrParams.regionParams[1].presRegion = &m_frameStatsPakIntegrationBuffer.sResource;
            virtualAddrParams.regionParams[1].dwOffset = m_tileStatsOffset.vdencStats;
            // Frame (not PAK) Stats Buffer - IN
            virtualAddrParams.regionParams[2].presRegion = &m_frameStatsPakIntegrationBuffer.sResource;
            virtualAddrParams.regionParams[2].dwOffset = m_frameStatsOffset.pakStats;
            // PAK MMIO - IN
            virtualAddrParams.regionParams[7].presRegion = &m_hucPakIntBrcDataBuffer;
        }
        else
        {
            virtualAddrParams.regionParams[1].presRegion = &m_resVdencBrcStatsBuffer;
            virtualAddrParams.regionParams[1].dwOffset = 0;
            virtualAddrParams.regionParams[2].presRegion = &m_resFrameStatStreamOutBuffer;
            virtualAddrParams.regionParams[2].dwOffset = 0;
            virtualAddrParams.regionParams[7].presRegion = &m_brcBuffers.resBrcBitstreamSizeBuffer;
        }
    }

    //For Dys + BRC Pass 0, use the resVdencDysPictureState2ndLevelBatchBuffer as input buffer
    virtualAddrParams.regionParams[3].presRegion = (m_dysRefFrameFlags != DYS_REF_NONE && m_dysVdencMultiPassEnabled) ? &m_resVdencDysPictureState2NdLevelBatchBuffer : &m_resVdencPictureState2NdLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex];
    // BRC Data - OUT
    virtualAddrParams.regionParams[4].presRegion = &m_brcBuffers.resBrcHucDataBuffer;
    virtualAddrParams.regionParams[4].isWritable = true;

    // Const Data - IN
    virtualAddrParams.regionParams[5].presRegion = &m_brcBuffers.resBrcConstantDataBuffer;

    // Output SLBB - OUT
    virtualAddrParams.regionParams[6].presRegion = &m_resVdencPictureState2NdLevelBatchBufferWrite[0];
    virtualAddrParams.regionParams[6].isWritable = true;

    // Load HuC Regions into Cmd Buf
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (!m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd) && !m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    ReturnCommandBuffer(&cmdBuffer);

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        // Dump history input before HuC runs
        CODECHAL_DEBUG_TOOL(
            m_debugInterface->DumpHucRegion(
                virtualAddrParams.regionParams[0].presRegion,
                0,
                hucRegionSize[0],
                0,
                hucRegionName[0],
                true,
                currPass,
                CodechalHucRegionDumpType::hucRegionDumpUpdate);
        );

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
                &m_resVdencBrcUpdateDmemBuffer[currPass],
                sizeof(HucBrcUpdateDmem),  // Change buffer and size to update dmem
                currPass,
                CodechalHucRegionDumpType::hucRegionDumpUpdate));

            for (auto i = 0; i < 16; i++) {
                if (virtualAddrParams.regionParams[i].presRegion)
                {
                    m_debugInterface->DumpHucRegion(
                        virtualAddrParams.regionParams[i].presRegion,
                        virtualAddrParams.regionParams[i].dwOffset,
                        hucRegionSize[i],
                        i,
                        hucRegionName[i],
                        !virtualAddrParams.regionParams[i].isWritable,
                        currPass,
                        CodechalHucRegionDumpType::hucRegionDumpUpdate);
                }
            })
    }

    // We increment by the average frame value once for each frame
    if (IsFirstPass())
    {
        m_curTargetFullness += m_inputBitsPerFrame;
    }

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : HuCBrcInitReset
| Purpose   : Start/Submit VP9 HuC BrcInit kernel to HW
|
| Returns   : MOS_STATUS
\---------------------------------------------------------------------------*/
MOS_STATUS CodechalVdencVp9StateG11::HuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    int currPass = GetCurrentPass();

    CODECHAL_DEBUG_TOOL(
        uint32_t hucRegionSize[16];
        const char* hucRegionName[16];

        hucRegionName[0] = "_BrcHistoryBuffer";
        hucRegionSize[0] = m_brcHistoryBufferSize;
    )

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_swBrcMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcInitReset());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SoftwareBRC(false));
        // Set region params for dumping only
        MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
        virtualAddrParams.regionParams[0].presRegion = &m_brcBuffers.resBrcHistoryBuffer;
        virtualAddrParams.regionParams[0].isWritable = true;
        m_inputBitsPerFrame                          = ((m_vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS) * 100.) / ((m_vp9SeqParams->FrameRate[m_vp9SeqParams->NumTemporalLayersMinus1].uiNumerator * 100.) / m_vp9SeqParams->FrameRate[m_vp9SeqParams->NumTemporalLayersMinus1].uiDenominator);
        m_curTargetFullness                          = m_vp9SeqParams->TargetBitRate[m_vp9SeqParams->NumTemporalLayersMinus1] * CODECHAL_ENCODE_BRC_KBPS;

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
                &m_resVdencBrcInitDmemBuffer,
                sizeof(HucBrcInitDmem),
                0,
                CodechalHucRegionDumpType::hucRegionDumpInit));

            for (auto i = 0; i < 16; i++) {
                if (virtualAddrParams.regionParams[i].presRegion)
                {
                    m_debugInterface->DumpHucRegion(
                        virtualAddrParams.regionParams[i].presRegion,
                        virtualAddrParams.regionParams[i].dwOffset,
                        hucRegionSize[i],
                        i,
                        hucRegionName[i],
                        !virtualAddrParams.regionParams[i].isWritable,
                        currPass,
                        CodechalHucRegionDumpType::hucRegionDumpInit);
                }
            })
        return eStatus;
    }
#endif
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported || m_firstTaskInPhase) && !m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

        m_firstTaskInPhase = false;
    }

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucVp9VdencBrcInitKernelDescriptor;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcInitReset());

    m_inputBitsPerFrame = ((m_vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS) * 100.) / ((m_vp9SeqParams->FrameRate[m_vp9SeqParams->NumTemporalLayersMinus1].uiNumerator * 100.) / m_vp9SeqParams->FrameRate[m_vp9SeqParams->NumTemporalLayersMinus1].uiDenominator);
    m_curTargetFullness = m_vp9SeqParams->TargetBitRate[m_vp9SeqParams->NumTemporalLayersMinus1] * CODECHAL_ENCODE_BRC_KBPS;

    // set HuC DMEM param
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_resVdencBrcInitDmemBuffer;
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(sizeof(HucBrcInitDmem), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_brcBuffers.resBrcHistoryBuffer;
    virtualAddrParams.regionParams[0].isWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (!m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd) && !m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    ReturnCommandBuffer(&cmdBuffer);

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
                &m_resVdencBrcInitDmemBuffer,
                sizeof(HucBrcInitDmem),
                0,
                CodechalHucRegionDumpType::hucRegionDumpInit));

            for (auto i = 0; i < 16; i++) {
                if (virtualAddrParams.regionParams[i].presRegion)
                {
                    m_debugInterface->DumpHucRegion(
                        virtualAddrParams.regionParams[i].presRegion,
                        virtualAddrParams.regionParams[i].dwOffset,
                        hucRegionSize[i],
                        i,
                        hucRegionName[i],
                        !virtualAddrParams.regionParams[i].isWritable,
                        0,
                        CodechalHucRegionDumpType::hucRegionDumpInit);
                }
            })
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::SetSequenceStructs());

    // All pipe need to go through the picture-level and slice-level commands
    m_numPassesInOnePipe = m_numPasses;
    m_numPasses = (m_numPasses + 1) * m_numPipe - 1;

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::SetPictureStructs());

    m_virtualEngineBBIndex = m_currOriginalPic.FrameIdx;

    if (m_dysRefFrameFlags != DYS_REF_NONE && m_dysVdencMultiPassEnabled)
    {
        if (!m_hucEnabled)
        {
            m_numPassesInOnePipe = (m_dysRefFrameFlags != DYS_REF_NONE);
        }
        if (m_vdencBrcEnabled)
        {
            //Reduce per pipe passes by 1, as m_numPassesInOnePipe == 1 becomes m_numPassesInOnePipe = 0 for Huc to run
            m_dysBrc = true;
            m_numPassesInOnePipe = (m_numPassesInOnePipe > 0 ) ? m_numPassesInOnePipe - 1 : m_numPassesInOnePipe;
        }
        else
        {
            m_dysCqp = true;
        }
        m_numPasses = (m_numPassesInOnePipe + 1) * m_numPipe - 1;
    }
    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifyCommandBufferSize());

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    // Scalable Mode header
    if (m_scalableMode)
    {
        MOS_COMMAND_BUFFER cmdBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

        bool requestFrameTracking = m_singleTaskPhaseSupported ? IsFirstPass() : IsLastPass();
        // In scalable mode, command buffer header is sent on last pipe only
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));
    }

    if (IsFirstPass() && IsFirstPipe())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructPakInsertObjBatchBuf(&m_resHucPakInsertUncompressedHeaderReadBuffer));
    }

    // For VDENC dynamic scaling, here are the steps we need to process
    //   Pass 0. VDENC + PAK Pass
    //     a. If this is Dys + BRC case, then run BRC Pass 0
    //     b. Ref frame scaling
    //    c. VDENC + PAK pass to stream out PakObjCmd
    //   Pass 1 -> Reset to Pass 0 so as to run HPU Pass 0
    //     a. If this is Dys + BRC case, then run BRC Pass 1
    //     b. Run HPU Pass 0
    //     c. Lite Pass (Pak only multi pass enabled) to stream in
    //        PakObjCmd from previous pass
    //   Pass 1 -> Only run HPU Pass 1 to update the probabilities for
    //   next frame. Repak is disabled for performance reasons
    if (m_dysRefFrameFlags != DYS_REF_NONE)
    {
        if (m_currPass == 0)
        {
            if (m_dysVdencMultiPassEnabled)
            {
                if (Mos_ResourceIsNull(&m_resVdencDysPictureState2NdLevelBatchBuffer))
                {
                    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

                    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
                    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
                    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
                    allocParamsForBufferLinear.Format = Format_Buffer;
                    allocParamsForBufferLinear.dwBytes = m_vdencPicStateSecondLevelBatchBufferSize;
                    allocParamsForBufferLinear.pBufName = "VDEnc DYS Picture Second Level Batch Buffer";

                    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_resVdencDysPictureState2NdLevelBatchBuffer);

                    if (eStatus != MOS_STATUS_SUCCESS)
                    {
                        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc DYS Picture Second Level Batch Buffer.");
                        return eStatus;
                    }
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
            }
        }
        else if (m_currPass == 1)
        {
            m_hucEnabled = m_dysHucEnabled; // recover huc state
            m_vdencPakonlyMultipassEnabled = true;
            m_dysRefFrameFlags = DYS_REF_NONE;
            m_currPass = 0; // reset ucCurrPass = 0 to run the Huc
            m_lastTaskInPhase = false;
        }
    }
    else
    {
        if (IsFirstPass() && m_vdencBrcEnabled)
        {
            m_vdencPakObjCmdStreamOutEnabled = true;
            m_resVdencPakObjCmdStreamOutBuffer = &m_resMbCodeSurface;
        }
        else
        {
            m_vdencPakObjCmdStreamOutEnabled = false;
        }
    }
    if (m_isTilingSupported)
    {
        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        uint8_t* tileStatsData = nullptr;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;
        if (Mos_ResourceIsNull(&m_tileRecordBuffer[m_virtualEngineBBIndex].sResource))
        {
            // Allocate Tile Stats Buffer for PAK integration and to be used everywhere for tile stats
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;
            auto size = m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterface->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.dwBytes = size;
            allocParamsForBufferLinear.pBufName = "Tile Record Buffer";

            CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_tileRecordBuffer[m_virtualEngineBBIndex].sResource));
            m_tileRecordBuffer[m_virtualEngineBBIndex].dwSize = size;

            auto tileRecordData = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_tileRecordBuffer[m_virtualEngineBBIndex].sResource, &lockFlagsWriteOnly);

            MOS_ZeroMemory(tileRecordData, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_tileRecordBuffer[m_virtualEngineBBIndex].sResource);
        }
    }
    // Running in the multiple VDBOX mode, Allocate Required Buffers for Tile based operation. Do this only once per frame.
    if (m_isTilingSupported && m_scalableMode && m_hucEnabled && IsFirstPipe() && IsFirstPass())
    {
        // Max row is 4 by VP9 Spec
        uint32_t m_maxScalableModeRows = 4;
        uint32_t m_maxScalableModeTiles = m_numVdbox * m_maxScalableModeRows;

        // Fill Pak integration kernel input tile stats structure
        MOS_ZeroMemory(&m_tileStatsOffset, sizeof(StatsInfo));
        // TileSizeRecord has to be 4k aligned
        m_tileStatsOffset.tileSizeRecord = 0; // TileReord is in a separated resource
        // VdencStats has to be 4k aligned
        m_tileStatsOffset.vdencStats = 0; // vdencStats is head of m_tileStatsPakIntegrationBuffer
        // VP9PAKStats has to be 64 byte aligned
        m_tileStatsOffset.pakStats = MOS_ALIGN_CEIL((m_tileStatsOffset.vdencStats + (m_maxScalableModeTiles * m_statsSize.vdencStats)), CODECHAL_PAGE_SIZE);
        // VP9CounterBuffer has to be 4k aligned
        m_tileStatsOffset.counterBuffer = MOS_ALIGN_CEIL((m_tileStatsOffset.pakStats + (m_maxScalableModeTiles * m_statsSize.pakStats)), CODECHAL_PAGE_SIZE);

        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        uint8_t* tileStatsData = nullptr;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;

        if (Mos_ResourceIsNull(&m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource))
        {
            // Allocate Tile Stats Buffer for PAK integration and to be used everywhere for tile stats
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL((m_tileStatsOffset.counterBuffer + (m_maxScalableModeTiles * m_statsSize.counterBuffer)), CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "GEN11 Tile Level Statistics Buffer";

            m_tileStatsPakIntegrationBufferSize = allocParamsForBufferLinear.dwBytes;

            CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource));
            m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].dwSize = allocParamsForBufferLinear.dwBytes;

            tileStatsData = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource, &lockFlagsWriteOnly);

            MOS_ZeroMemory(tileStatsData, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource);
        }
    }

    int currPass = GetCurrentPass();
    if ((m_dysRefFrameFlags != DYS_REF_NONE) && m_dysVdencMultiPassEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructPicStateBatchBuf(&m_resVdencDysPictureState2NdLevelBatchBuffer));
    }
    else
    {
        if (IsFirstPipe())
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructPicStateBatchBuf(&m_resVdencPictureState2NdLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex]));
        }
    }

    if (IsFirstPipe() && m_vdencBrcEnabled)
    {
        // Invoke BRC init/reset FW
        if (m_brcInit || m_brcReset)
        {
            if (!m_singleTaskPhaseSupported)
            {
                //Reset earlier set PAK perf tag
                m_osInterface->pfnResetPerfBufferID(m_osInterface);
                CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET);
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcInitReset());
            m_brcInit = m_brcReset = false;
        }
        // For multipass and singlepass+RePAK we call BRC update for all passes except last pass (RePAK)
        // For single pass w/o RePAK (1 total pass) we call BRC update on one and only pass
        if (!IsLastPass() || (m_currPass == 0 && m_numPasses == 0))
        {
            bool origSingleTaskPhase = m_singleTaskPhaseSupported;
            bool origFrameTrackingHeader = false;

            // If this is the case of Dynamic Scaling + BRC Pass 0'  VDENC + Pak  pass
            // Disable SingleTaskPhase before running 1st BRC update
            // To run HPU0 on the next pass i.e Pak only pass, we make Pass 1 as Pass 0 in which case the
            // BRC dmem buffer( resVdencBrcUpdateDmemBuffer[0] ) will get overridden if we do not submit BRC command now.
            if (m_dysBrc && m_dysRefFrameFlags != DYS_REF_NONE)
            {
                m_singleTaskPhaseSupported = false;

                //Reset Frame Tracking Header for this submission
                MOS_COMMAND_BUFFER cmdBuffer;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));
                origFrameTrackingHeader = cmdBuffer.Attributes.bEnableMediaFrameTracking;
                cmdBuffer.Attributes.bEnableMediaFrameTracking = false;
                ReturnCommandBuffer(&cmdBuffer);
            }

            if (!m_singleTaskPhaseSupported)
            {
                //Reset performance buffer used for BRC init
                m_osInterface->pfnResetPerfBufferID(m_osInterface);
                CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE);
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcUpdate());
            //Restore the original state of SingleTaskPhaseSupported flag
            m_singleTaskPhaseSupported = origSingleTaskPhase;

            //Restore Original Frame Tracking Header
            if (m_dysBrc && m_dysRefFrameFlags != DYS_REF_NONE)
            {
                MOS_COMMAND_BUFFER cmdBuffer;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));
                cmdBuffer.Attributes.bEnableMediaFrameTracking = origFrameTrackingHeader;
                ReturnCommandBuffer(&cmdBuffer);
            }
        }
    }

    // run HuC_VP9Prob first pass (it runs in parallel with ENC)
    if (m_hucEnabled)
    {
        if (IsFirstPipe() && (IsFirstPass() || IsLastPass() || (m_vdencBrcEnabled)))  // Before the first PAK pass, for RePak pass and for BRC case, HuC_VP9Prob needs to be called on Pass 1 as well
        {
            if (!m_singleTaskPhaseSupported)
            {
                //Reset earlier set PAK perf tag
                m_osInterface->pfnResetPerfBufferID(m_osInterface);
                // Add Hpu tag here after updated
                CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_8X8_PU);
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCVp9Prob());
            if (!m_singleTaskPhaseSupported)
            {
                //reset performance buffer used for HPU update
                m_osInterface->pfnResetPerfBufferID(m_osInterface);
            }
        }
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RefreshFrameInternalBuffers());
    }

    if (m_dysRefFrameFlags != DYS_REF_NONE && IsFirstPass())
    {
        // Turn off scalability and Tiling for Dynamic scaling pass 0 for reference scaling
        uint8_t logTileRows = m_vp9PicParams->log2_tile_rows;
        uint8_t logTileColumns = m_vp9PicParams->log2_tile_columns;
        bool scalableMode = m_scalableMode;
        uint8_t numPipe = m_numPipe;
        m_vp9PicParams->log2_tile_rows = 0;
        m_vp9PicParams->log2_tile_columns = 0;
        m_scalableMode = false;
        m_numPipe = 1;
        // Execute Reference scaling pass
        CODECHAL_ENCODE_CHK_STATUS_RETURN(DysRefFrames());

        // Restore scalability and Tiling status for subsequent passes
        m_vp9PicParams->log2_tile_rows = logTileRows;
        m_vp9PicParams->log2_tile_columns = logTileColumns;
        m_scalableMode = scalableMode;
        m_numPipe = numPipe;

        if (m_dysVdencMultiPassEnabled)
        {
            m_singleTaskPhaseSupported = true;
            m_firstTaskInPhase = true;
            m_vdencPakObjCmdStreamOutEnabled = true;
            m_resVdencPakObjCmdStreamOutBuffer = &m_resMbCodeSurface;
        }
        else
        {
            m_hucEnabled = m_dysHucEnabled; //recover huc state
        }
    }

    // set HCP_SURFACE_STATE values
    MHW_VDBOX_SURFACE_PARAMS surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID + 1];
    for (uint8_t i = 0; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; i++)
    {
        MOS_ZeroMemory(&surfaceParams[i], sizeof(surfaceParams[i]));
        surfaceParams[i].Mode = m_mode;
        surfaceParams[i].ucSurfaceStateId = i;
        surfaceParams[i].ChromaType = m_outputChromaFormat;
        surfaceParams[i].bSrc8Pak10Mode   = (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth) && (!m_vp9SeqParams->SeqFlags.fields.SourceBitDepth);

        switch (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
        {
            case VP9_ENCODED_BIT_DEPTH_10: //10 bit encoding
            {
                surfaceParams[i].ucBitDepthChromaMinus8 = 2;
                surfaceParams[i].ucBitDepthLumaMinus8 = 2;
                break;
            }
            default:
            {
                surfaceParams[i].ucBitDepthChromaMinus8 = 0;
                surfaceParams[i].ucBitDepthLumaMinus8 = 0;
                break;
            }
        }
    }

    // For PAK engine, we do NOT use scaled reference images even if dynamic scaling is enabled
    PMOS_SURFACE refSurface[3], refSurfaceNonScaled[3], dsRefSurface4x[3], dsRefSurface8x[3];
    for (auto i = 0; i < 3; i++)
    {
        refSurface[i] = refSurfaceNonScaled[i] = dsRefSurface4x[i] = dsRefSurface8x[i] = nullptr;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetHcpSrcSurfaceParams(surfaceParams, refSurface, refSurfaceNonScaled, dsRefSurface4x, dsRefSurface8x));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);
    }

    // Non scalable mode header
    if ((!m_singleTaskPhaseSupported || m_firstTaskInPhase) && !m_scalableMode)
    {
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    // Place hw semaphore on all other pipe to wait for first pipe HUC to finish. Apply for all passes after extend the Dmen HPU buffer size
    int currPipe = GetCurrentPipe();
    if (m_scalableMode)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));

        //HW Semaphore cmd to make sure all pipes start encode at the same time
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resPipeStartSync, 1, MHW_MI_ATOMIC_INC, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(
            &m_resPipeStartSync,
            &cmdBuffer,
            m_numPipe));

        // Program some placeholder cmds to resolve the hazard between pipe sync
        MHW_MI_STORE_DATA_PARAMS dataParams;
        dataParams.pOsResource = &m_resDelayMinus;
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue = 0xDE1A;
        for (uint32_t i = 0; i < m_numDelay; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                &cmdBuffer,
                &dataParams));
        }

        //clean HW semaphore memory
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resPipeStartSync, 1, MHW_MI_ATOMIC_DEC, &cmdBuffer));

        //Start Watchdog Timer
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStartCmd(&cmdBuffer));
    }

    // clean-up per VDBOX semaphore memory, only in the first BRC pass. Same semaphore is re-used across BRC passes for stitch command
    if (IsFirstPass())
    {
        if (!Mos_ResourceIsNull(&m_stitchWaitSemaphoreMem[currPipe].sResource))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                SetSemaphoreMem(
                    &m_stitchWaitSemaphoreMem[currPipe].sResource,
                    &cmdBuffer,
                    false));
        }
    }

    // Repak conditional batch buffer end based on repak flag written by Huc to HUC_STATUS regster
    if (m_hucEnabled && (m_numPasses > 0) && IsLastPass())
    {
        // Insert conditional batch buffer end
        // Bit 30 has been added as a success condition, therefore this needs to be masked to only check 31 for RePAK
        // or else if HuC decides not to do RePAK for conditional RePAK yet terminates successfully RePAK will still happen.
        // Success = bit 30 set to 1, Do RePAK = bit 31 set to 1, value is always 0; if 0 < memory, continue
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_resHucPakMmioBuffer;
        // Make the DisableCompareMask 0, so that the HW will do AND operation on DW0 with Mask DW1, refer to HuCVp9Prob() for the settings
        // and compare the result against the Semaphore data which in our case dwValue = 0.
        // If result > dwValue then continue execution otherwise terminate the batch buffer
        miConditionalBatchBufferEndParams.bDisableCompareMask = false;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    if (IsFirstPipe())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
    }

    // set HCP_PIPE_BUF_ADDR_STATE values
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams = nullptr;
    pipeBufAddrParams = CreateHcpPipeBufAddrParams(pipeBufAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetHcpPipeBufAddrParams(*pipeBufAddrParams, refSurface, refSurfaceNonScaled, dsRefSurface4x, dsRefSurface8x));
    pipeBufAddrParams->pRawSurfParam = &surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID];
    pipeBufAddrParams->pDecodedReconParam = &surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID];
#ifdef _MMC_SUPPORTED
    // In case of dynamic scaling refSurface is scaled for pass 0 and nonscaled for last pass
    // This ensures correct references are passed in for MMC
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPipeBufAddr(pipeBufAddrParams, refSurface, &cmdBuffer));
#endif

    // set HCP_PIPE_MODE_SELECT values
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = nullptr;
    pipeModeSelectParams = CreateMhwVdboxPipeModeSelectParams();
    SetHcpPipeModeSelectParams(*pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));

    // This wait cmd is needed to make sure copy is done as suggested by HW folk
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(&cmdBuffer, nullptr, false));

    // Decoded picture
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID]));

    // Source input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID]));

    // Last reference picture
    if (refSurface[0])
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID]));
    }

    // Golden reference picture
    if (refSurface[1])
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID]));
    }

    // Alt reference picture
    if (refSurface[2])
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID]));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(&cmdBuffer, pipeBufAddrParams));

    // set HCP_IND_OBJ_BASE_ADDR_STATE values
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));
    if (pipeModeSelectParams)
    {
        MOS_Delete(pipeModeSelectParams);
        pipeModeSelectParams = nullptr;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSrcSurfaceStateCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID]));
    if (m_pictureCodingType == I_TYPE)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID]));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID]));
        if ((m_dysRefFrameFlags != DYS_REF_NONE) && !m_dysVdencMultiPassEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID]));
        }
    }

    MHW_VDBOX_SURFACE_PARAMS dsSurfaceParams[2];     // 8x and 4x DS surfaces
    SetHcpDsSurfaceParams(&dsSurfaceParams[0]);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencDsRefSurfaceStateCmd(&cmdBuffer, &dsSurfaceParams[0], 2));

    if (pipeBufAddrParams)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeBufAddrCmd(&cmdBuffer, pipeBufAddrParams));
        MOS_Delete(pipeBufAddrParams);
        pipeBufAddrParams = nullptr;
    }

    MHW_BATCH_BUFFER secondLevelBatchBuffer;
    MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
    secondLevelBatchBuffer.dwOffset = 0;
    secondLevelBatchBuffer.bSecondLevel = true;
    if (m_hucEnabled)
    {
        secondLevelBatchBuffer.OsResource = m_resVdencPictureState2NdLevelBatchBufferWrite[0];
    }
    else
    {
        if (m_dysRefFrameFlags != DYS_REF_NONE && m_dysVdencMultiPassEnabled)
        {
            // For DyS + BRC case, we run BRC on Pass 0, so although we dont run HPU on Pass 0
            // (VDENC + PAK pass) we will still use the write buffer here
            if (m_dysBrc)
            {
                secondLevelBatchBuffer.OsResource = m_resVdencPictureState2NdLevelBatchBufferWrite[0];
            }
            else //CQP case for Pass 0 , HPU has not run yet.. so use this buffer
            {
                secondLevelBatchBuffer.OsResource = m_resVdencDysPictureState2NdLevelBatchBuffer;
            }
        }
        else
        {
            secondLevelBatchBuffer.OsResource = m_resVdencPictureState2NdLevelBatchBufferRead[currPass][m_vdencPictureState2ndLevelBBIndex];
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &secondLevelBatchBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams,
    PMOS_SURFACE* refSurface,
    PMOS_SURFACE* refSurfaceNonScaled,
    PMOS_SURFACE* dsRefSurface4x,
    PMOS_SURFACE* dsRefSurface8x)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    pipeBufAddrParams = {};
    pipeBufAddrParams.Mode = m_mode;
    pipeBufAddrParams.psPreDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psPostDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psRawSurface = m_rawSurfaceToPak;

    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &m_resDeblockingFilterLineBuffer;

    pipeBufAddrParams.presDeblockingFilterTileRowStoreScratchBuffer =
        &m_resDeblockingFilterTileLineBuffer;

    pipeBufAddrParams.presDeblockingFilterColumnRowStoreScratchBuffer =
        &m_resDeblockingFilterTileColumnBuffer;

    pipeBufAddrParams.presMetadataLineBuffer       = &m_resMetadataLineBuffer;
    pipeBufAddrParams.presMetadataTileLineBuffer   = &m_resMetadataTileLineBuffer;
    pipeBufAddrParams.presMetadataTileColumnBuffer = &m_resMetadataTileColumnBuffer;
    pipeBufAddrParams.presCurMvTempBuffer = m_trackedBuf->GetMvTemporalBuffer(m_currMvTemporalBufferIndex);

    // Huc first pass doesn't write probabilities to output prob region but only updates to the input region. HuC run before repak writes to the ouput region.
    uint8_t frameCtxIdx = 0;
    if (m_hucEnabled && IsLastPass())
    {
        pipeBufAddrParams.presVp9ProbBuffer = &m_resHucProbOutputBuffer;
    }
    else
    {
        frameCtxIdx = m_vp9PicParams->PicFlags.fields.frame_context_idx;
        CODECHAL_ENCODE_ASSERT(frameCtxIdx < CODEC_VP9_NUM_CONTEXTS);
        pipeBufAddrParams.presVp9ProbBuffer = &m_resProbBuffer[frameCtxIdx];
    }

    pipeBufAddrParams.presVp9SegmentIdBuffer              = &m_resSegmentIdBuffer;
    pipeBufAddrParams.presHvdTileRowStoreBuffer           = &m_resHvcTileRowstoreBuffer;
    pipeBufAddrParams.ps4xDsSurface = m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    pipeBufAddrParams.ps8xDsSurface = m_trackedBuf->Get8xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    pipeBufAddrParams.presVdencIntraRowStoreScratchBuffer = &m_resVdencIntraRowStoreScratchBuffer;
    pipeBufAddrParams.dwNumRefIdxL0ActiveMinus1           = (m_vp9PicParams->PicFlags.fields.frame_type) ? m_numRefFrames - 1 : 0;
    if (m_scalableMode && m_hucEnabled && m_isTilingSupported)
    {
        pipeBufAddrParams.presVdencStreamOutBuffer = &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource;
        pipeBufAddrParams.dwVdencStatsStreamOutOffset = m_tileStatsOffset.vdencStats;
    }
    else
    {
        pipeBufAddrParams.presVdencStreamOutBuffer    = &m_resVdencBrcStatsBuffer;
        pipeBufAddrParams.dwVdencStatsStreamOutOffset = 0;
    }
    pipeBufAddrParams.presStreamOutBuffer = nullptr;
    pipeBufAddrParams.presFrameStatStreamOutBuffer  = &m_resFrameStatStreamOutBuffer;
    pipeBufAddrParams.presSseSrcPixelRowStoreBuffer = &m_resSseSrcPixelRowStoreBuffer;
    pipeBufAddrParams.presVdencStreamInBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
    pipeBufAddrParams.presSegmentMapStreamOut       = &m_resVdencSegmentMapStreamOut;
    pipeBufAddrParams.presPakCuLevelStreamoutBuffer =
        Mos_ResourceIsNull(&m_resPakcuLevelStreamoutData.sResource) ? nullptr : &m_resPakcuLevelStreamoutData.sResource;
    if (m_dysRefFrameFlags != DYS_REF_NONE)
    {
        pipeBufAddrParams.presVdencPakObjCmdStreamOutBuffer =
            (m_vdencPakObjCmdStreamOutEnabled) ? m_resVdencPakObjCmdStreamOutBuffer : nullptr;
    }
    else
    {
        pipeBufAddrParams.presVdencPakObjCmdStreamOutBuffer = m_resVdencPakObjCmdStreamOutBuffer = &m_resMbCodeSurface;
    }

    if (m_scalableMode && m_hucEnabled && m_isTilingSupported)
    {
        PCODECHAL_ENCODE_BUFFER tileStatisticsBuffer = &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex];
        bool useTileStatisticsBuffer = tileStatisticsBuffer && !Mos_ResourceIsNull(&tileStatisticsBuffer->sResource);
        // the new framestats streamout will now be the tile level stats buffer because each pak is spewing out tile level stats
        pipeBufAddrParams.presFrameStatStreamOutBuffer = useTileStatisticsBuffer ? &tileStatisticsBuffer->sResource : nullptr;
        pipeBufAddrParams.dwFrameStatStreamOutOffset = useTileStatisticsBuffer ? m_tileStatsOffset.pakStats : 0;
        //Main Frame Stats are integrated by PAK integration kernel
    }
    else
    {
        pipeBufAddrParams.presFrameStatStreamOutBuffer = &m_resFrameStatStreamOutBuffer;
        pipeBufAddrParams.dwFrameStatStreamOutOffset = 0;
    }

    if (m_pictureCodingType != I_TYPE)
    {
        for (auto i = 0; i < 3; i++)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(refSurface[i]);
            CODECHAL_ENCODE_CHK_NULL_RETURN(dsRefSurface4x[i]);
            CODECHAL_ENCODE_CHK_NULL_RETURN(dsRefSurface8x[i]);

            pipeBufAddrParams.presReferences[i] = &refSurface[i]->OsResource;
            pipeBufAddrParams.presVdencReferences[i] = &refSurface[i]->OsResource;
            pipeBufAddrParams.presVdenc4xDsSurface[i] = &dsRefSurface4x[i]->OsResource;
            pipeBufAddrParams.presVdenc8xDsSurface[i] = &dsRefSurface8x[i]->OsResource;

            if ((m_dysRefFrameFlags != DYS_REF_NONE) && !m_dysVdencMultiPassEnabled)
            {
                pipeBufAddrParams.presReferences[i + 4] = &refSurfaceNonScaled[i]->OsResource;
            }
        }

        pipeBufAddrParams.presColMvTempBuffer[0] = m_trackedBuf->GetMvTemporalBuffer(m_currMvTemporalBufferIndex ^ 0x01);
    }

    return eStatus;
}

uint16_t CodechalVdencVp9StateG11::GetNumTilesInFrame()
{
    return ((1 << m_vp9PicParams->log2_tile_rows) * (1 << m_vp9PicParams->log2_tile_columns));
}

MOS_STATUS CodechalVdencVp9StateG11::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::AllocateResources());

    // create the tile coding state parameters
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_tileParams =
                                        (PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11)MOS_AllocAndZeroMemory(sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11) * m_maxTileNumber));

    if (m_isTilingSupported)
    {
        uint32_t maxPicWidthInSb = MOS_ROUNDUP_DIVIDE(m_maxPicWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
        uint32_t maxPicHeightInSb = MOS_ROUNDUP_DIVIDE(m_maxPicHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);

        //PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
        uint32_t size = maxPicWidthInSb * maxPicHeightInSb * 64 * CODECHAL_CACHELINE_SIZE; // One CU has 16-byte, and there are 64 CU in one SB. But, each tile needs to be aliged to the cache line
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPakcuLevelStreamoutData.sResource);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

        //PAK Slice Level Streamut Data. DW60-DW62 in HCP pipe buffer address command
        // one LCU has one cache line. Use CU as LCU during creation
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "PAK Slice Level Streamout Data";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPakSliceLevelStreamutData.sResource);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

        //HCP scalability Sync buffer
        size = CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE * CODECHAL_CACHELINE_SIZE;
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "Hcp scalability Sync buffer ";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_hcpScalabilitySyncBuffer.sResource);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        m_hcpScalabilitySyncBuffer.dwSize = size;

        // PAK integration related
        if (m_isTilingSupported && m_scalableMode && m_hucEnabled)
        {
            // HUC Pak Int DMEM buffer
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucPakIntDmem), CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "Huc Pak Int Dmem Buffer";
            for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
            {
                for (auto j = 0; j < m_brcMaxNumPasses; j++)
                {
                    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_hucPakIntDmemBuffer[i][j]);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            // HuC PAK Int region 7, 8
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "HUC PAK Int Dummy Buffer";

            eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_hucPakIntDummyBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            uint8_t* data = nullptr;

            data = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_hucPakIntDummyBuffer,
                &lockFlags);

            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(
                data,
                allocParamsForBufferLinear.dwBytes);

            m_osInterface->pfnUnlockResource(m_osInterface, &m_hucPakIntDummyBuffer);

            // HuC PAK Int Region 1 programming related stats
            MOS_ZeroMemory(&m_frameStatsOffset, sizeof(StatsInfo));
            MOS_ZeroMemory(&m_statsSize, sizeof(StatsInfo));

            //Sizes of each buffer to be loaded into the region 0 as input and 1 loaded out as output.
            m_statsSize.tileSizeRecord = m_hcpInterface->GetPakHWTileSizeRecordSize();
            m_statsSize.vdencStats = m_brcStatsBufSize;
            m_statsSize.pakStats = m_brcPakStatsBufSize;
            m_statsSize.counterBuffer = m_probabilityCounterBufferSize;

            //Offsets for output of all integrated frame statistics (region 1) from PAK integration kernel
            m_frameStatsOffset.tileSizeRecord = 0;
            // Vdenc stats has to be 4K aligned
            m_frameStatsOffset.vdencStats = 0;
            // VP9 PAK stats/ BRC pak stats / Frame Stats have to be 4K aligned
            m_frameStatsOffset.pakStats = MOS_ALIGN_CEIL((m_frameStatsOffset.vdencStats + m_statsSize.vdencStats), CODECHAL_PAGE_SIZE);
            // VP9 CounterBuffer goes as input to HUC region so it has to be 4k aligned
            m_frameStatsOffset.counterBuffer = MOS_ALIGN_CEIL((m_frameStatsOffset.pakStats + m_statsSize.pakStats), CODECHAL_PAGE_SIZE);

            // HuC PAK Int DMEM region 1 buffer allocation
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_frameStatsOffset.counterBuffer + m_statsSize.counterBuffer, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "PAK HUC Integrated Frame Stats Buffer";
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;

            m_frameStatsPakIntegrationBufferSize = allocParamsForBufferLinear.dwBytes;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_frameStatsPakIntegrationBuffer.sResource));
            m_frameStatsPakIntegrationBuffer.dwSize = allocParamsForBufferLinear.dwBytes;

            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_frameStatsPakIntegrationBuffer.sResource, &lockFlags);
            MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_frameStatsPakIntegrationBuffer.sResource);

            // Allocate region 9 of pak integration to be fed as input to HUC BRC region 7
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "GEN11 PAK Integration FrameByteCount output";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_hucPakIntBrcDataBuffer));

            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_hucPakIntBrcDataBuffer, &lockFlags);
            MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_hucPakIntBrcDataBuffer);

            // Allocate Semaphore memory for VDEnc/PAK on all pipes to signal stitch command to stop waiting
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;
            allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
            allocParamsForBufferLinear.pBufName = "GEN11 VDEnc PAK done Semaphore Memory";

            for (auto i = 0; i < m_numPipe; i++)
            {
                uint32_t* data = nullptr;

                CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_stitchWaitSemaphoreMem[i].sResource));

                m_stitchWaitSemaphoreMem[i].dwSize = allocParamsForBufferLinear.dwBytes;

                data = (uint32_t*)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_stitchWaitSemaphoreMem[i].sResource,
                    &lockFlags);

                CODECHAL_ENCODE_CHK_NULL_RETURN(data);

                *data = 1;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &m_stitchWaitSemaphoreMem[i].sResource));
            }

        }
        uint32_t* data = nullptr;
        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.pBufName = "Pipe Start Sync memory";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPipeStartSync));

        data = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resPipeStartSync,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, sizeof(uint32_t));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resPipeStartSync));

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.pBufName = "Frame Start Sync memory";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resFrameStartSync));

        data = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resFrameStartSync,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, sizeof(uint32_t));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resFrameStartSync));

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.pBufName = "DelayMinusMemory";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resDelayMinus));

        data = (uint32_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resDelayMinus,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, sizeof(uint32_t));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resDelayMinus));
    }

    return eStatus;
}

void CodechalVdencVp9StateG11::FreeResources()
{
    CodechalVdencVp9State::FreeResources();

    MOS_FreeMemory(m_tileParams);
    if (m_isTilingSupported)
    {
        if (!Mos_ResourceIsNull(&m_resPakcuLevelStreamoutData.sResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resPakcuLevelStreamoutData.sResource);
        }

        if (!Mos_ResourceIsNull(&m_resPakSliceLevelStreamutData.sResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resPakSliceLevelStreamutData.sResource);
        }

        // Release Hcp scalability Sync buffer
        if (!Mos_ResourceIsNull(&m_hcpScalabilitySyncBuffer.sResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_hcpScalabilitySyncBuffer.sResource);
        }

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_tileRecordBuffer); i++)
        {
            if (!Mos_ResourceIsNull(&m_tileRecordBuffer[i].sResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_tileRecordBuffer[i].sResource);
            }
        }

        for (auto i = 0; i < m_numUncompressedSurface; i++)
        {
            for (auto j = 0; j < CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE; j++)
            {
                for (auto k = 0; k < 3; k++)
                {
                    PMOS_COMMAND_BUFFER cmdBuffer = &m_veBatchBuffer[i][j][k];

                    if (!Mos_ResourceIsNull(&cmdBuffer->OsResource))
                    {
                        if (cmdBuffer->pCmdBase)
                        {
                            m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
                        }
                        m_osInterface->pfnFreeResource(m_osInterface, &cmdBuffer->OsResource);
                    }
                }
            }
        }
        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            for (auto j = 0; j < m_brcMaxNumPasses; j++)
            {
                if (!Mos_ResourceIsNull(&m_hucPakIntDmemBuffer[i][j]))
                {
                    m_osInterface->pfnFreeResource(
                        m_osInterface,
                        &m_hucPakIntDmemBuffer[i][j]);
                }
            }
        }

        if (!Mos_ResourceIsNull(&m_hucPakIntDummyBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_hucPakIntDummyBuffer);
        }

        if (!Mos_ResourceIsNull(&m_frameStatsPakIntegrationBuffer.sResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_frameStatsPakIntegrationBuffer.sResource);
        }

        if (!Mos_ResourceIsNull(&m_hucPakIntBrcDataBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_hucPakIntBrcDataBuffer);
        }

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_tileStatsPakIntegrationBuffer); i++)
        {
            if (!Mos_ResourceIsNull(&m_tileStatsPakIntegrationBuffer[i].sResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_tileStatsPakIntegrationBuffer[i].sResource);
            }
        }

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_stitchWaitSemaphoreMem); i++)
        {
            if (!Mos_ResourceIsNull(&m_stitchWaitSemaphoreMem[i].sResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_stitchWaitSemaphoreMem[i].sResource);
            }
        }

        if (!Mos_ResourceIsNull(&m_resPipeStartSync))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resPipeStartSync);
        }

        if (!Mos_ResourceIsNull(&m_resFrameStartSync))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resFrameStartSync);
        }

        if (!Mos_ResourceIsNull(&m_resDelayMinus))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resDelayMinus);
        }
    }

    return;
}

MOS_STATUS CodechalVdencVp9StateG11::SendMIAtomicCmd(
    PMOS_RESOURCE               semaMem,
    uint32_t                    immData,
    MHW_COMMON_MI_ATOMIC_OPCODE opCode,
    PMOS_COMMAND_BUFFER         cmdBuffer
)
{
    MHW_MI_ATOMIC_PARAMS       atomicParams;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory((&atomicParams), sizeof(atomicParams));
    atomicParams.pOsResource = semaMem;
    atomicParams.dwDataSize = sizeof(uint32_t);
    atomicParams.Operation = opCode;
    atomicParams.bInlineData = true;
    atomicParams.dwOperand1Data[0] = immData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(cmdBuffer, &atomicParams));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    uint32_t   maxRows = 1;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    //Create and register huc Cmd Initializer
    m_hucCmdInitializer = MOS_New(CodechalCmdInitializerG11, this);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::Initialize(settings));

    GetSystemPipeNumberCommon();

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_scalabilityState = (PCODECHAL_ENCODE_SCALABILITY_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SCALABILITY_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalabilityState);
        //scalability initialize
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_InitializeState(m_scalabilityState, m_hwInterface));
    }

    maxRows = MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT) / CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT;
    //Max num of rows = 4 by VP9 Spec
    maxRows = MOS_MIN(maxRows, 4);
    //Max tile numbers = max of number tiles for single pipe or max muber of tiles for scalable pipes
    m_maxTileNumber = MOS_MAX((MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH) / CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH), m_numVdbox * maxRows);

    m_dysVdencMultiPassEnabled = true;

    m_numPipe = m_numVdbox;

    m_scalableMode = (m_numPipe > 1);
    m_useVirtualEngine = true;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_STATUS eStatusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ENABLE_HW_STITCH,
        &userFeatureData);
    m_enableTileStitchByHW = userFeatureData.i32Data ? true : false;

    userFeatureData.i32Data = 1;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_HUC_ENABLE_ID,
        &userFeatureData);
    m_hucEnabled = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    userFeatureData.i32Data = 1;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;
    m_singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
    // For dynamic scaling, the SingleTaskPhaseSupported is set to true and it does not get restored
    // to the original value after encoding of the frame. So need to restore to the original state
    m_storeSingleTaskPhaseSupported = m_singleTaskPhaseSupported; //Save the SingleTaskPhase state here

    // Multi-Pass BRC: currently disabled by default, plan to enable by default
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        &userFeatureData);
    m_multipassBrcSupported = (userFeatureData.i32Data) ? true : false;
    m_vdencBrcStatsBufferSize     = m_brcStatsBufSize;
    m_vdencBrcPakStatsBufferSize  = m_brcPakStatsBufSize;
    m_brcHistoryBufferSize        = m_brcHistoryBufSize;

    // HME enabled by default for VP9
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ME_ENABLE_ID,
        &userFeatureData);
    m_hmeSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_16xME_ENABLE_ID,
        &userFeatureData);
    m_16xMeSupported = (userFeatureData.i32Data) ? true : false;

    // disable superHME when HME is disabled
    if (m_hmeSupported == false)
    {
        m_16xMeSupported = false;
    }

    // UHME disabled
    m_32xMeSupported = false;
    // VP9 uses a different streamin kernel
    m_useNonLegacyStreamin = true;

    // Initialize kernel State
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStates());

    // Get max binding table count
    m_maxBtCount = GetMaxBtCount();    // Need to add the correct BTcount when HME is enabled

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : GetSegmentBlockIndexInFrame
| Purpose   : Returns the offset of 32x32 block in the frame based on current x,y 32 block location in current tile
|
| Returns   : MOS_STATUS
\---------------------------------------------------------------------------*/
uint32_t CodechalVdencVp9StateG11::GetSegmentBlockIndexInFrame(
    uint32_t frameWidth,
    uint32_t curr32XInTile,
    uint32_t curr32YInTile,
    uint32_t currTileStartY64aligned,
    uint32_t currTileStartX64aligned)
{
    uint32_t frameWidthIn32 = MOS_ALIGN_CEIL(frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32;
    uint32_t curr32XInFrame = currTileStartX64aligned / 32 + curr32XInTile;
    uint32_t curr32YInFrame = currTileStartY64aligned / 32 + curr32YInTile;
    uint32_t curr32BlockInFrame = curr32YInFrame * frameWidthIn32 + curr32XInFrame;
    return curr32BlockInFrame;
}

/*----------------------------------------------------------------------------
| Name      : InitZigZagToRasterLUTPerTile
| Purpose   : Rasterize a tile's 32 blocks' segmap indices, add to frame mapbuffer created for these indices
|
| Returns   : MOS_STATUS
\---------------------------------------------------------------------------*/
MOS_STATUS CodechalVdencVp9StateG11::InitZigZagToRasterLUTPerTile(
    uint32_t tileHeight,
    uint32_t tileWidth,
    uint32_t currTileStartYInFrame,
    uint32_t currTileStartXInFrame)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    // Allocate space for zig-zag to raster LUT used for vdenc streamin (1 int32_t for every 32x32 block (pic 64 aligned))
    // We only do this when the 1st tile of new frame is being processed and keep it the same unless tile resolutions changed.
    // We keep this map around until sequence is finished, it's deleted at device destruction.
    if (currTileStartXInFrame == 0 && currTileStartYInFrame == 0)
    {
        if (m_mapBuffer) // free previous if it exists - it may exist if this isn't first seg streamin frame, but it's a new tile with different res
        {
            MOS_FreeMemory(m_mapBuffer);
        }
        // Allocate one integer space for each 32*32 block in the whole frame to hold the segmentation index.
        m_mapBuffer = (uint32_t*)MOS_AllocAndZeroMemory(
            (MOS_ALIGN_CEIL(m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32) *
            (MOS_ALIGN_CEIL(m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32) *
            sizeof(int32_t)); //Framewidth and height are 64 aligned already
    }
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mapBuffer);

    uint32_t align64Width32 = MOS_ALIGN_CEIL(tileWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32;
    uint32_t align64Height32 = MOS_ALIGN_CEIL(tileHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32;
    uint32_t* mapBufferZigZagPerTile = (uint32_t*)MOS_AllocAndZeroMemory(align64Width32*align64Height32 * sizeof(uint32_t));
    CODECHAL_ENCODE_CHK_NULL_RETURN(mapBufferZigZagPerTile);

    m_segStreamInHeight = m_frameHeight;
    m_segStreamInWidth = m_frameWidth;

    uint32_t count32 = 0; //Number of 32 by 32 blocks that will be processed here
    for (uint32_t curr32YInTile = 0; curr32YInTile< align64Height32; curr32YInTile++)
    {
        for (uint32_t curr32XInTile = 0; curr32XInTile < align64Width32; curr32XInTile++)
        {
            mapBufferZigZagPerTile[count32++] = GetSegmentBlockIndexInFrame(
                m_frameWidth,
                curr32XInTile,
                curr32YInTile,
                currTileStartYInFrame,
                currTileStartXInFrame);
        }
    }

    //    mapBufferZigZagPerTile --->   m_mapBuffer
    //  | a b c d ...               ---> | a b W X c d Y Z ....
    //  | W X Y Z ...
    uint32_t num32blocks = align64Width32 * align64Height32;
    uint32_t tileOffsetIndex = m_32BlocksRasterized;
    for (uint32_t i = 0, dwRasterCount = 0; i < num32blocks; i += (align64Width32 * 2))
    {
        for (uint32_t j = i; j < i + (align64Width32 * 2); j += 4)
        {
            m_mapBuffer[j + tileOffsetIndex] = mapBufferZigZagPerTile[dwRasterCount++];
            m_mapBuffer[j + tileOffsetIndex + 1] = mapBufferZigZagPerTile[dwRasterCount++];
        }
        for (uint32_t j = i + 2; j < i + (align64Width32 * 2); j += 4)
        {
            m_mapBuffer[j + tileOffsetIndex] = mapBufferZigZagPerTile[dwRasterCount++];
            m_mapBuffer[j + tileOffsetIndex + 1] = mapBufferZigZagPerTile[dwRasterCount++];
        }
    }
    if (mapBufferZigZagPerTile) // free per tile map buffer as it has been rasterized and copied into the mapbuffer
    {
        MOS_FreeMemory(mapBufferZigZagPerTile);
    }

    // ^ Zig-zag pattern filled to SB aligned (CEIL), if unaligned then we base seg ID address on previous row/column (data replication)
    uint32_t width32 = CODECHAL_GET_WIDTH_IN_BLOCKS(tileWidth, 32);
    if (width32 != align64Width32) // replicate last column
    {
        for (auto i = (align64Width32 * 2) - 1 - 2; i < num32blocks; i += (align64Width32 * 2))
        {
            m_mapBuffer[i + tileOffsetIndex] = m_mapBuffer[i + tileOffsetIndex - 1];
            m_mapBuffer[i + tileOffsetIndex + 2] = m_mapBuffer[i + tileOffsetIndex + 1];
        }
    }

    uint32_t height32 = CODECHAL_GET_HEIGHT_IN_BLOCKS(tileHeight, 32);
    if (height32 != align64Height32) // replicate last row
    {
        for (auto i = num32blocks - (align64Width32 * 2) + 2; i < num32blocks; i += 4)
        {
            m_mapBuffer[i + tileOffsetIndex] = m_mapBuffer[i + tileOffsetIndex - 2];
            m_mapBuffer[i + tileOffsetIndex + 1] = m_mapBuffer[i + tileOffsetIndex + 1 - 2];
        }
    }
    //Index offset to be added to the buffer for the next tile depending on how many blocks were rasterized already in this tile
    m_32BlocksRasterized += count32;

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateG11::CalculateVdencPictureStateCommandSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11 stateCmdSizeParams;
    uint32_t vdencPictureStatesSize = 0, vdencPicturePatchListSize = 0;
    stateCmdSizeParams.bHucDummyStream = true;
    m_hwInterface->GetHxxStateCommandSize(
        CODECHAL_ENCODE_MODE_VP9,
        &vdencPictureStatesSize,
        &vdencPicturePatchListSize,
        &stateCmdSizeParams);

    m_defaultPictureStatesSize += vdencPictureStatesSize;
    m_defaultPicturePatchListSize += vdencPicturePatchListSize;

    m_hwInterface->GetVdencStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_VP9,
        &vdencPictureStatesSize,
        &vdencPicturePatchListSize);

    m_defaultPictureStatesSize += vdencPictureStatesSize;
    m_defaultPicturePatchListSize += vdencPicturePatchListSize;

    return eStatus;
}

PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS CodechalVdencVp9StateG11::CreateHcpPipeBufAddrParams(PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams)
{
    pipeBufAddrParams = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11);

    return pipeBufAddrParams;
}

MOS_STATUS CodechalVdencVp9StateG11::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
            (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

        memset(attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
        attriExt->bUseVirtualEngineHint =
            attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
    }

    return eStatus;
}
