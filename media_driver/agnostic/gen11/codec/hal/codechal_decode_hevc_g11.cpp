/*
* Copyright (c) 2012-2018, Intel Corporation
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
//! \file     codechal_decode_hevc_g11.cpp
//! \brief    Implements the decode interface extension for HEVC.
//! \details  Implements all functions required by CodecHal for HEVC decoding.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_decode_hevc_g11.h"
#include "codechal_mmc_decode_hevc.h"
#include "mhw_vdbox_hcp_g11_X.h"
#include "mhw_vdbox_mfx_g11_X.h"
#include "mhw_vdbox_g11_X.h"
#include "codechal_hw_g11_X.h"
#include "hal_oca_interface.h"

//==<Functions>=======================================================
MOS_STATUS CodechalDecodeHevcG11::AllocateResourcesVariableSizes ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint8_t maxBitDepth     = (m_is10BitHevc) ? 10 : 8;
    uint8_t chromaFormatPic = m_hevcPicParams->chroma_format_idc;
    uint8_t chromaFormat    = m_chromaFormatinProfile;
    CODECHAL_DECODE_ASSERT(chromaFormat >= chromaFormatPic);

    uint32_t widthMax     = MOS_MAX(m_width, m_widthLastMaxAlloced);
    uint32_t heightMax    = MOS_MAX(m_height, m_heightLastMaxAlloced);
    uint32_t frameSizeMax = MOS_MAX((m_copyDataBufferInUse ? m_copyDataBufferSize : m_dataSize), m_frameSizeMaxAlloced);

    uint32_t ctbLog2SizeYPic = m_hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                               m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3;
    uint32_t ctbLog2SizeY = MOS_MAX(ctbLog2SizeYPic, m_ctbLog2SizeYMax);

    //for Scalability
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        MHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam;
        MOS_ZeroMemory(&reallocParam, sizeof(reallocParam));
        reallocParam.ucMaxBitDepth     = maxBitDepth;
        reallocParam.ucChromaFormat    = chromaFormat;
        reallocParam.dwCtbLog2SizeY    = ctbLog2SizeY;
        reallocParam.dwCtbLog2SizeYMax  = m_ctbLog2SizeYMax;
        reallocParam.dwPicWidth         = widthMax;
        reallocParam.dwPicWidthAlloced  = m_widthLastMaxAlloced;
        reallocParam.dwPicHeight        = heightMax;
        reallocParam.dwPicHeightAlloced = m_heightLastMaxAlloced;
        reallocParam.dwFrameSize        = frameSizeMax;
        reallocParam.dwFrameSizeAlloced = m_frameSizeMaxAlloced;

        MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
        MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
        hcpBufSizeParam.ucMaxBitDepth  = maxBitDepth;
        hcpBufSizeParam.ucChromaFormat = chromaFormat;
        hcpBufSizeParam.dwCtbLog2SizeY = ctbLog2SizeY;
        hcpBufSizeParam.dwPicWidth      = widthMax;
        hcpBufSizeParam.dwPicHeight     = heightMax;
        hcpBufSizeParam.dwMaxFrameSize  = frameSizeMax;

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_AllocateResources_VariableSizes(
            m_scalabilityState,
            &hcpBufSizeParam,
            &reallocParam));

        m_frameSizeMaxAlloced = frameSizeMax;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::AllocateResourcesVariableSizes());

    return eStatus;
}

CodechalDecodeHevcG11::~CodechalDecodeHevcG11 ()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }
    if (m_scalabilityState)
    {
        CodecHalDecodeScalability_Destroy(m_scalabilityState);
        MOS_FreeMemAndSetNull(m_scalabilityState);
    }
    //Note: virtual engine interface destroy is done in MOS layer

    return;
}

MOS_STATUS CodechalDecodeHevcG11::CheckLCUSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint16_t LCUSize = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size + 
        m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

    if (m_width > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU || m_height > CODECHAL_HEVC_MAX_DIM_FOR_MIN_LCU)
    {
        if (LCUSize == CODECHAL_HEVC_MIN_LCU)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Invalid LCU size.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SetGpuCtxCreatOption(
    CodechalSetting *          codecHalSetting)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CodechalDecode::SetGpuCtxCreatOption(codecHalSetting);
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_DECODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeScalability_ConstructParmsForGpuCtxCreation(
                m_scalabilityState,
                (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
                codecHalSetting));

            if (((PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt)->LRCACount == 2)
            {
                m_videoContext = MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VIDEO5 : MOS_GPU_CONTEXT_VDBOX2_VIDEO;

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    m_videoContext,
                    MOS_GPU_NODE_VIDEO,
                    m_gpuCtxCreatOpt));

                MOS_GPUCTX_CREATOPTIONS createOption;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    MOS_GPU_CONTEXT_VIDEO,
                    m_videoGpuNode,
                    &createOption));
            }
            else if (((PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt)->LRCACount == 3)
            {
                m_videoContext = MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VIDEO7 : MOS_GPU_CONTEXT_VDBOX2_VIDEO2;

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    m_videoContext,
                    MOS_GPU_NODE_VIDEO,
                    m_gpuCtxCreatOpt));

                MOS_GPUCTX_CREATOPTIONS createOption;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                    m_osInterface,
                    MOS_GPU_CONTEXT_VIDEO,
                    m_videoGpuNode,
                    &createOption));
            }
            else
            {
                m_videoContext = MOS_GPU_CONTEXT_VIDEO;
            }
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
                m_sinlgePipeVeState,
                (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
                false));

            m_videoContext = MOS_GPU_CONTEXT_VIDEO;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SetHucDmemS2LPictureBss(
    PHUC_HEVC_S2L_PIC_BSS       hucHevcS2LPicBss)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(hucHevcS2LPicBss);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::SetHucDmemS2LPictureBss(hucHevcS2LPicBss));

    if (m_hevcExtPicParams)
    {
        hucHevcS2LPicBss->high_precision_offsets_enabled_flag =
            m_hevcExtPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag;
        hucHevcS2LPicBss->chroma_qp_offset_list_enabled_flag =
            m_hevcExtPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag;
    }
    else
    {
        hucHevcS2LPicBss->high_precision_offsets_enabled_flag  = 0;
        hucHevcS2LPicBss->chroma_qp_offset_list_enabled_flag   = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SetFrameStates ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    m_frameIdx++;

    // Check HuC_status2 Imem loaded bit, if 0,return error
    // As driver doesn't know when can get reg value afer storing HuC_Status2 register,
    // Check the reg value here at the beginning of next frame
    // Check twice, first entry and second entry
    if (m_shortFormatInUse &&
        m_frameIdx < 3 &&
        m_statusQueryReportingEnabled &&
        (((m_decodeStatusBuf.m_decodeStatus->m_hucErrorStatus2 >> 32) & m_hucInterface->GetHucStatus2ImemLoadedMask()) == 0))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("HuC IMEM Loaded fails");
        return MOS_STATUS_UNKNOWN;
    }

    m_cencBuf = m_decodeParams.m_cencBuf;
    
    if (IsFirstExecuteCall())    // For DRC Multiple Execution Call, no need to update every value in pHevcState except first execute
    {
        m_dataSize   = m_decodeParams.m_dataSize;
        m_dataOffset = m_decodeParams.m_dataOffset;
        m_numSlices  = m_decodeParams.m_numSlices;

        if (m_numSlices > CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Slice number doesn't support!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        m_hevcPicParams    = (PCODEC_HEVC_PIC_PARAMS)m_decodeParams.m_picParams;
        m_hevcExtPicParams = (PCODEC_HEVC_EXT_PIC_PARAMS)m_decodeParams.m_extPicParams;
        CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_sliceParams);
        m_hevcSliceParams    = (PCODEC_HEVC_SLICE_PARAMS)m_decodeParams.m_sliceParams;
        m_hevcExtSliceParams = (PCODEC_HEVC_EXT_SLICE_PARAMS)m_decodeParams.m_extSliceParams;
        m_hevcIqMatrixParams = (PCODECHAL_HEVC_IQ_MATRIX_PARAMS)m_decodeParams.m_iqMatrixBuffer;
        m_destSurface        = *(m_decodeParams.m_destSurface);
        m_resDataBuffer      = *(m_decodeParams.m_dataBuffer);

        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeBitstreamCat());
    }
    else
    {
        m_dataSize      = m_decodeParams.m_dataSize;
        m_dataOffset    = 0;
        m_resDataBuffer = *(m_decodeParams.m_dataBuffer);
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckAndCopyBitstream());

    PCODEC_REF_LIST destEntry = m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx];
    MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));

    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(m_hevcPicParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hevcIqMatrixParams);

    // Calculate bCurPicIntra
    m_curPicIntra = true;
    if (m_hevcPicParams->IntraPicFlag == 0)
    {
        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            uint8_t frameIdx = m_hevcPicParams->RefPicSetStCurrBefore[i];
            if (frameIdx < 15)
            {
                m_curPicIntra = false;
                break;
            }

            frameIdx = m_hevcPicParams->RefPicSetStCurrAfter[i];
            if (frameIdx < 15)
            {
                m_curPicIntra = false;
                break;
            }

            frameIdx = m_hevcPicParams->RefPicSetLtCurr[i];
            if (frameIdx < 15)
            {
                m_curPicIntra = false;
                break;
            }
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetPictureStructs());

    uint32_t i;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_frameUsedAsCurRef[i] = false;
        m_refIdxMapping[i]     = -1;
    }

    // Calculate RefIdxMapping
    for (i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        uint8_t frameIdx = m_hevcPicParams->RefPicSetStCurrBefore[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[frameIdx] = true;
        }

        frameIdx = m_hevcPicParams->RefPicSetStCurrAfter[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[frameIdx] = true;
        }

        frameIdx = m_hevcPicParams->RefPicSetLtCurr[i];
        if (frameIdx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            m_frameUsedAsCurRef[frameIdx] = true;
        }
    }

    uint8_t curRefIdx = 0;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_frameUsedAsCurRef[i])
        {
            m_refIdxMapping[i] = curRefIdx++;
        }
    }

    CODECHAL_DECODE_CHK_COND_RETURN(curRefIdx > 8, "bitstream has more than 8 references");

    m_minCtbSize = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    m_width      = m_hevcPicParams->PicWidthInMinCbsY * m_minCtbSize;
    m_height     = m_hevcPicParams->PicHeightInMinCbsY * m_minCtbSize;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CheckLCUSize());

    if (m_hcpInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        rowstoreParams.Mode             = CODECHAL_DECODE_MODE_HEVCVLD;
        rowstoreParams.dwPicWidth       = m_width;
        rowstoreParams.bMbaff           = false;
        rowstoreParams.ucBitDepthMinus8 = (m_is10BitHevc) ? 2 : 0;
        rowstoreParams.ucChromaFormat   = m_hevcPicParams->chroma_format_idc;
        rowstoreParams.ucLCUSize        = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3 +
                                          m_hevcPicParams->log2_diff_max_min_luma_coding_block_size);
        m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams);
    }

    // Calculate Tile info
    if (m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetAllTileInfo());
    }

    if (m_curPicIntra)
    {
        m_perfType = I_TYPE;
    }
    else
    {
        // Not possible to determine whether P or B is used for short format.
        // For long format iterating through all of the slices to determine P vs
        // B, so in order to avoid this, declare all other pictures MIXED_TYPE.
        m_perfType = MIXED_TYPE;
    }

    m_crrPic = m_hevcPicParams->CurrPic;
    m_secondField =
        CodecHal_PictureIsBottomField(m_hevcPicParams->CurrPic);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic     = m_crrPic;
        m_debugInterface->m_secondField = m_secondField;
        m_debugInterface->m_frameType   = m_perfType;

        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
            m_hevcPicParams,
            m_hevcExtPicParams));

        if (m_hevcIqMatrixParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(m_hevcIqMatrixParams));
        }

        if (m_hevcSliceParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSliceParams(
                m_hevcSliceParams,
                m_hevcExtSliceParams,
                m_numSlices,
                m_shortFormatInUse));
        })

    // Clear DMEM buffer program flag
    if (m_shortFormatInUse)
    {
        m_dmemBufferProgrammed = false;
        m_dmemBufferIdx++;
        m_dmemBufferIdx %= CODECHAL_HEVC_NUM_DMEM_BUFFERS;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    // Check if SFC can be supported
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->CheckAndInitialize((CODECHAL_DECODE_PROCESSING_PARAMS *)m_decodeParams.m_procParams, m_hevcPicParams));
#endif
    CODECHAL_DEBUG_TOOL(
        if (!m_incompletePicture && !IsFirstExecuteCall()) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resCopyDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                m_estiBytesInBitstream,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        })

    if (MOS_VE_SUPPORTED(m_osInterface) && static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
    {
        CODECHAL_DECODE_SCALABILITY_INIT_PARAMS  initParams;

        MOS_ZeroMemory(&initParams, sizeof(initParams));
        initParams.u32PicWidthInPixel   = m_width;
        initParams.u32PicHeightInPixel  = m_height;
        initParams.format = m_decodeParams.m_destSurface->Format;
        initParams.usingSFC             = false;
        initParams.gpuCtxInUse          = GetVideoContext();

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitScalableParams(
            m_scalabilityState,
            &initParams,
            &m_decodePassNum));

        if (MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeScalability_ChkGpuCtxReCreation(
                    m_scalabilityState,
                    (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
            SetVideoContext(m_scalabilityState->VideoContext);
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesVariableSizes());

    m_hcpDecPhase = CodechalHcpDecodePhaseInitialized;

    return eStatus;
}

//!
//! \brief    Determine Decode Phase
//! \details  Determine decode phase in hevc decode
//! \param    N/A
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalDecodeHevcG11::DetermineDecodePhase()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported() && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DetermineDecodePhase(
            m_scalabilityState,
            &m_hcpDecPhase));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::DetermineDecodePhase());
    }

    CODECHAL_DECODE_VERBOSEMESSAGE("Current Decode Phase: %d.", m_hcpDecPhase);

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::EndStatusReport(
    CodechalDecodeStatusReport &decodeStatusReport,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    auto mmioRegistersMfx = m_mfxInterface->GetMmioRegisters(m_vdboxIndex);
    uint32_t currIndex = m_decodeStatusBuf.m_currIndex;

    MHW_MI_STORE_REGISTER_MEM_PARAMS regParams;

    //Frame CRC
    if (m_reportFrameCrc)
    {
        uint32_t frameCrcOffset =
            currIndex * sizeof(CodechalDecodeStatus) +
            m_decodeStatusBuf.m_decFrameCrcOffset +
            sizeof(uint32_t) * 2;

        regParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
        regParams.dwOffset = frameCrcOffset;
        regParams.dwRegister = mmioRegistersMfx->mfxFrameCrcRegOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
            cmdBuffer,
            &regParams));
    }

    // First copy all the SW data in to the eStatus buffer
    m_decodeStatusBuf.m_decodeStatus[currIndex].m_swStoredData = m_decodeStatusBuf.m_swStoreData;
    m_decodeStatusBuf.m_decodeStatus[currIndex].m_decodeStatusReport = decodeStatusReport;

    uint32_t storeDataOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_storeDataOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_DATA_PARAMS dataParams;
    dataParams.pOsResource = &m_decodeStatusBuf.m_statusBuffer;
    dataParams.dwResourceOffset = storeDataOffset;
    dataParams.dwValue = CODECHAL_STATUS_QUERY_END_FLAG;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &dataParams));

    m_decodeStatusBuf.m_currIndex = (m_decodeStatusBuf.m_currIndex + 1) % CODECHAL_DECODE_STATUS_NUM;

    CodechalDecodeStatus *decodeStatus = &m_decodeStatusBuf.m_decodeStatus[m_decodeStatusBuf.m_currIndex];
    MOS_ZeroMemory(decodeStatus, sizeof(CodechalDecodeStatus));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, cmdBuffer));

    if (!m_osInterface->bEnableKmdMediaFrameTracking && m_osInterface->bInlineCodecStatusUpdate)
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        // Send MI_FLUSH with protection bit off, which will FORCE exit protected mode for MFX
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        flushDwParams.pOsResource = &m_decodeStatusBuf.m_statusBuffer;
        flushDwParams.dwDataDW1 = m_decodeStatusBuf.m_swStoreData;
        MHW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::EndStatusReportForFE(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegistersMfx = m_mfxInterface->GetMmioRegisters(m_vdboxIndex);
    auto mmioRegistersHcp = m_hcpInterface ? m_hcpInterface->GetMmioRegisters(m_vdboxIndex) : nullptr;

    uint32_t currIndex = m_decodeStatusBuf.m_currIndex;
    //Error Status report
    uint32_t errStatusOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_decErrorStatusOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_REGISTER_MEM_PARAMS regParams;
    regParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
    regParams.dwOffset = errStatusOffset;
    regParams.dwRegister = mmioRegistersHcp ?
        mmioRegistersHcp->hcpCabacStatusRegOffset : mmioRegistersMfx->mfxErrorFlagsRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &regParams));

    //MB Count
    uint32_t mbCountOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_decMBCountOffset +
        sizeof(uint32_t) * 2;

    regParams.presStoreBuffer = &m_decodeStatusBuf.m_statusBuffer;
    regParams.dwOffset = mbCountOffset;
    regParams.dwRegister = mmioRegistersHcp ?
        mmioRegistersHcp->hcpDecStatusRegOffset : mmioRegistersMfx->mfxMBCountRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &regParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER       primCmdBuf)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported() && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_SCALABILITY_SETHINT_PARMS scalSetParms;
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            scalSetParms.bNeedSyncWithPrevious       = true;
            scalSetParms.bSameEngineAsLastSubmission = false;
            scalSetParms.bSFCInUse                   = false;
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SetHintParams(m_scalabilityState, &scalSetParms));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_PopulateHintParams(m_scalabilityState, primCmdBuf));
    }
    else
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
            MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
            vesetParams.bNeedSyncWithPrevious       = true;
            vesetParams.bSameEngineAsLastSubmission = false;
            vesetParams.bSFCInUse                   = false;
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, primCmdBuf, true));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::DetermineSendProlgwithFrmTracking(
    bool                        *sendPrologWithFrameTracking)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(sendPrologWithFrameTracking);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        if (CodecHalDecodeScalability1stPhaseofSubmission(m_scalabilityState))
        {
            *sendPrologWithFrameTracking = true;
        }
    }
    else
    {
        if (m_shortFormatInUse)
        {
            *sendPrologWithFrameTracking = m_enableSf2DmaSubmits;
        }
        else
        {
            *sendPrologWithFrameTracking = true;
        }
    }

    return eStatus;
}

uint32_t CodechalDecodeHevcG11::RequestedSpaceSize(uint32_t requestedSize)
{
    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
    {
        //primary cmd buffer only including cmd buffer header .
        return COMMAND_BUFFER_RESERVED_SPACE * 2;
    }
    else
    {
        return requestedSize;
    }
}

MOS_STATUS CodechalDecodeHevcG11::VerifyExtraSpace(
    uint32_t requestedSize,
    uint32_t additionalSizeNeeded)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
    {
        eStatus = MOS_STATUS_NO_SPACE;

        // Try a maximum of 3 attempts to request the required sizes from OS
        // OS could reset the sizes if necessary, therefore, requires to re-verify
        for (auto i = 0; (i < 3) && (eStatus != MOS_STATUS_SUCCESS); i++)
        {
            // Verify secondary cmd buffer
            eStatus = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSize,
                MOS_VE_HAVE_SECONDARY_CMDBUFFER);

            // Resize command buffer if not enough
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(
                    m_osInterface,
                    requestedSize + additionalSizeNeeded,
                    0,
                    MOS_VE_HAVE_SECONDARY_CMDBUFFER));
                // Set status to NO_SPACE to enter the commaned buffer size verification on next loop.
                eStatus = MOS_STATUS_NO_SPACE;
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    CODECHAL_DECODE_FUNCTION_ENTER;

    //HCP Decode Phase State Machine
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineDecodePhase());

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        //Switch GPU context when necessary
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SwitchGpuContext(m_scalabilityState));
    }

    // Set HEVC Decode Phase, and execute it.
    if (m_shortFormatInUse && m_hcpDecPhase == CodechalHcpDecodePhaseLegacyS2L)
    {
        if (m_secureDecoder)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPictureS2L());
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPictureLongFormat());
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SendPictureS2L()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_enableSf2DmaSubmits)
    {
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((CODECHAL_DECODE_MODE_HUC << 4) & 0xF0) | (m_perfType & 0xF)));
    }

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &primCmdBuffer, true));

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));

        if (cmdBufferInUse == &scdryCmdBuffer)
        {
            //send prolog at the start of a secondary cmd buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBufferInUse, false));
        }
    }

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && CodecHalDecodeScalability1stDecPhase(m_scalabilityState))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitSemaMemResources(m_scalabilityState, cmdBufferInUse));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureS2LCmds(cmdBufferInUse));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer(m_scalabilityState, &scdryCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::InitPicLongFormatMhwParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Reset all pic Mhw Params
    auto pipeModeSelectParams =
        static_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11>(m_picMhwParams.PipeModeSelectParams);
    *pipeModeSelectParams = {};
    auto pipeBufAddrParams =
        static_cast<PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11>(m_picMhwParams.PipeBufAddrParams);
    *pipeBufAddrParams = {};
    auto hevcPicStateParams =
        static_cast<PMHW_VDBOX_HEVC_PIC_STATE_G11>(m_picMhwParams.HevcPicState);

    *hevcPicStateParams = {};
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::InitPicLongFormatMhwParams());

        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            CodecHalDecodeScalablity_DecPhaseToHwWorkMode(
                pipeModeSelectParams->MultiEngineMode,
                pipeModeSelectParams->PipeWorkMode);

            pipeBufAddrParams->presSliceStateStreamOutBuffer =
                &m_scalabilityState->resSliceStateStreamOutBuffer;
            pipeBufAddrParams->presMvUpRightColStoreBuffer =
                &m_scalabilityState->resMvUpRightColStoreBuffer;
            pipeBufAddrParams->presIntraPredUpRightColStoreBuffer =
                &m_scalabilityState->resIntraPredUpRightColStoreBuffer;
            pipeBufAddrParams->presIntraPredLeftReconColStoreBuffer =
                &m_scalabilityState->resIntraPredLeftReconColStoreBuffer;
            pipeBufAddrParams->presCABACSyntaxStreamOutBuffer =
                m_scalabilityState->presCABACStreamOutBuffer;
    }

    hevcPicStateParams->pHevcExtPicParams = m_hevcExtPicParams;

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SendPictureLongFormat()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_enableSf2DmaSubmits)
    {
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((CODECHAL_DECODE_MODE_HEVCVLD << 4) & 0xF0) | (m_perfType & 0xF)));
    }

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    bool sendPrologWithFrameTracking = false;
    CODECHAL_DECODE_CHK_STATUS_RETURN(DetermineSendProlgwithFrmTracking(&sendPrologWithFrameTracking));

    if (sendPrologWithFrameTracking)
    {
        //Frame tracking functionality is called at the start of a command buffer.
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
            &primCmdBuffer, true));
    }

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;
    auto                mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));

        if ((!m_shortFormatInUse && !CodecHalDecodeScalabilityIsFESeparateSubmission(m_scalabilityState)) ||
            CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
        {
            //send prolog at the start of a secondary cmd buffer
            CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBufferInUse, false));
        }

        HalOcaInterface::On1stLevelBBStart(scdryCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }
    else
    {
        HalOcaInterface::On1stLevelBBStart(primCmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicLongFormatMhwParams());

    CODECHAL_DEBUG_TOOL(
        for (uint16_t n = 0; n < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; n++)
        {
            if (m_picMhwParams.PipeBufAddrParams->presReferences[n])
            {
                MOS_SURFACE dstSurface;

                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.OsResource = *(m_picMhwParams.PipeBufAddrParams->presReferences[n]);
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->m_refIndex = n;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    "RefSurf"));
            }

            if (m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n])
            {
                m_debugInterface->m_refIndex = n;
                // dump mvdata
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    m_picMhwParams.PipeBufAddrParams->presColMvTempBuffer[n],
                    CodechalDbgAttr::attrMvData,
                    "_DEC",
                    m_mvBufferSize));
            }
        }
    );

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && CodecHalDecodeScalability1stDecPhase(m_scalabilityState))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitSemaMemResources(m_scalabilityState, cmdBufferInUse));
        }
    }
    
    //Send status report Start
    if (m_statusQueryReportingEnabled)
    {
        bool sendStatusReportStart = true;
        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            sendStatusReportStart = CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState);
        }
        if (sendStatusReportStart)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(cmdBufferInUse));
        }
    }

    if (m_shortFormatInUse && m_statusQueryReportingEnabled &&
        (!CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) ||
            CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState)))
    {
        uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
                        m_decodeStatusBuf.m_storeDataOffset +
                        sizeof(uint32_t) * 2;

        // Check HuC_STATUS bit15, HW continue if bit15 > 0, otherwise send COND BB END cmd.
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->SendCondBbEndCmd(
                &m_decodeStatusBuf.m_statusBuffer,
                statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusMaskOffset,
                0,
                false,
                cmdBufferInUse));
    }

    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_FEBESync(
            m_scalabilityState,
            cmdBufferInUse));
        if (m_perfFEBETimingEnabled && CodecHalDecodeScalabilityIsLastCompletePhase(m_scalabilityState))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, &scdryCmdBuffer));
        }
    }

    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(
            m_miInterface->AddWatchdogTimerStartCmd(cmdBufferInUse));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureLongFormatCmds(cmdBufferInUse, &m_picMhwParams));

    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
    {
        MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 hcpTileCodingParam;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_CalculateHcpTileCodingParams<MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11>(
            m_scalabilityState,
            m_hevcPicParams,
            &hcpTileCodingParam));
        CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG11*>(m_hcpInterface)->AddHcpTileCodingCmd(
            cmdBufferInUse,
            &hcpTileCodingParam));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer(m_scalabilityState, &scdryCmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::SendSliceLongFormat(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(hevcSliceState);
    CODECHAL_DECODE_CHK_NULL_RETURN(hevcSliceState->pHevcSliceParams);

    PCODEC_HEVC_SLICE_PARAMS slc           = hevcSliceState->pHevcSliceParams;
    PCODEC_HEVC_EXT_SLICE_PARAMS slcExt    =
        static_cast<PMHW_VDBOX_HEVC_SLICE_STATE_G11>(hevcSliceState)->pHevcExtSliceParams;

    // Disable the flag when ref list is empty for P/B slices to avoid the GPU hang
    if (m_curPicIntra &&
        !m_hcpInterface->IsHevcISlice(slc->LongSliceFlags.fields.slice_type))

    {
        slc->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag = 0;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSliceStateCmd(
        cmdBuffer,
        hevcSliceState));

    if (! m_hcpInterface->IsHevcISlice(slc->LongSliceFlags.fields.slice_type))
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS refIdxParams;
        refIdxParams.CurrPic         = m_hevcPicParams->CurrPic;
        refIdxParams.ucList = 0;
        refIdxParams.ucNumRefForList = slc->num_ref_idx_l0_active_minus1 + 1;
        eStatus = MOS_SecureMemcpy(
            &refIdxParams.RefPicList,
            sizeof(refIdxParams.RefPicList),
            &slc->RefPicList,
            sizeof(slc->RefPicList));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");
        refIdxParams.hevcRefList = (void**)m_hevcRefList;
        refIdxParams.poc_curr_pic  = m_hevcPicParams->CurrPicOrderCntVal;
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = m_hevcPicParams->PicOrderCntValList[i];
        }

        refIdxParams.pRefIdxMapping = hevcSliceState->pRefIdxMapping;
        refIdxParams.RefFieldPicFlag    = m_hevcPicParams->RefFieldPicFlag;
        refIdxParams.RefBottomFieldFlag = m_hevcPicParams->RefBottomFieldFlag;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
            cmdBuffer,
            nullptr,
            &refIdxParams));

        if (m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type))
        {
            refIdxParams.ucList = 1;
            refIdxParams.ucNumRefForList = slc->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
                cmdBuffer,
                nullptr,
                &refIdxParams));
        }
    }
    else if (MEDIA_IS_WA(m_waTable, WaDummyReference) && !m_osInterface->bSimIsActive)
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS refIdxParams;
        MOS_ZeroMemory(&refIdxParams, sizeof(MHW_VDBOX_HEVC_REF_IDX_PARAMS));
        refIdxParams.bDummyReference = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
            cmdBuffer,
            nullptr,
            &refIdxParams));
    }

    if ((m_hevcPicParams->weighted_pred_flag &&
            m_hcpInterface->IsHevcPSlice(slc->LongSliceFlags.fields.slice_type)) ||
        (m_hevcPicParams->weighted_bipred_flag &&
            m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type)))
    {
        MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS weightOffsetParams;

        weightOffsetParams.ucList = 0;

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[0],
            sizeof(weightOffsetParams.LumaWeights[0]),
            &slc->delta_luma_weight_l0,
            sizeof(slc->delta_luma_weight_l0));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[1],
            sizeof(weightOffsetParams.LumaWeights[1]),
            &slc->delta_luma_weight_l1,
            sizeof(slc->delta_luma_weight_l1));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        if (slcExt)//REXT
        {
            eStatus = MOS_SecureMemcpy(
                &weightOffsetParams.LumaOffsets[0],
                sizeof(weightOffsetParams.LumaOffsets[0]),
                &slcExt->luma_offset_l0,
                sizeof(slcExt->luma_offset_l0));
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

            eStatus = MOS_SecureMemcpy(
                &weightOffsetParams.LumaOffsets[1],
                sizeof(weightOffsetParams.LumaOffsets[1]),
                &slcExt->luma_offset_l1,
                sizeof(slcExt->luma_offset_l1));
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

            eStatus = MOS_SecureMemcpy(
                &weightOffsetParams.ChromaOffsets[0],
                sizeof(weightOffsetParams.ChromaOffsets[0]),
                &slcExt->ChromaOffsetL0,
                sizeof(slcExt->ChromaOffsetL0));
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

            eStatus = MOS_SecureMemcpy(
                &weightOffsetParams.ChromaOffsets[1],
                sizeof(weightOffsetParams.ChromaOffsets[1]),
                &slcExt->ChromaOffsetL1,
                sizeof(slcExt->ChromaOffsetL1));
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");
        }
        else
        {
            for (int32_t i = 0; i < 15; i++)
            {
                weightOffsetParams.LumaOffsets[0][i] = (int16_t)slc->luma_offset_l0[i];
                weightOffsetParams.LumaOffsets[1][i] = (int16_t)slc->luma_offset_l1[i];

                for (int32_t j = 0; j < 2; j++)
                {
                    weightOffsetParams.ChromaOffsets[0][i][j] = (int16_t)slc->ChromaOffsetL0[i][j];
                    weightOffsetParams.ChromaOffsets[1][i][j] = (int16_t)slc->ChromaOffsetL1[i][j];
                }
            }
        }

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[0],
            sizeof(weightOffsetParams.ChromaWeights[0]),
            &slc->delta_chroma_weight_l0,
            sizeof(slc->delta_chroma_weight_l0));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        eStatus = MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[1],
            sizeof(weightOffsetParams.ChromaWeights[1]),
            &slc->delta_chroma_weight_l1,
            sizeof(slc->delta_chroma_weight_l1));
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Failed to copy memory.");

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(
            cmdBuffer,
            nullptr,
            &weightOffsetParams));

        if (m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type))
        {
            weightOffsetParams.ucList = 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(
                cmdBuffer,
                nullptr,
                &weightOffsetParams));
        }
    }

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AddHcpSecureState(
            cmdBuffer,
            hevcSliceState));
    }

    MHW_VDBOX_HCP_BSD_PARAMS bsdParams;
    MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
    bsdParams.dwBsdDataLength = hevcSliceState->dwLength;
    bsdParams.dwBsdDataStartOffset = slc->slice_data_offset + hevcSliceState->dwOffset;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpBsdObjectCmd(
        cmdBuffer,
        &bsdParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    // Bitstream is incomplete, don't do any decoding work.
    if (m_incompletePicture)
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DECODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);

    uint32_t statusBufferOffset = (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
                    m_decodeStatusBuf.m_storeDataOffset +
                    sizeof(uint32_t) * 2;

    uint32_t renderingFlags = m_videoContextUsesNullHw;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    MOS_COMMAND_BUFFER primCmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &primCmdBuffer, 0));

    PMOS_COMMAND_BUFFER cmdBufferInUse = &primCmdBuffer;
    MOS_COMMAND_BUFFER  scdryCmdBuffer;
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_GetCmdBufferToUse(
            m_scalabilityState,
            &scdryCmdBuffer,
            &cmdBufferInUse));
    }

    // store CS ENGINE ID register
    if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported() && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReadCSEngineIDReg(
            m_scalabilityState,
            &m_decodeStatusBuf,
            cmdBufferInUse));
    }

    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
    {
        //no slice level command for scalability decode BE phases
    }
    else if (m_cencBuf)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetCencBatchBuffer(cmdBufferInUse));
    }
    else if (m_shortFormatInUse &&
             (m_hcpDecPhase == CodechalHcpDecodePhaseLegacyLong ||
                 CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState)))
    {
        // If S2L and 2nd pass, jump to 2nd level batch buffer
        if (m_enableSf2DmaSubmits)
        {
            #if (_DEBUG || _RELEASE_INTERNAL)
            m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iLastCurrent = m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex].iSize;
            #endif

            CODECHAL_DEBUG_TOOL(
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                    &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex],
                    CODECHAL_NUM_MEDIA_STATES,
                    "_DEC"));)
        }

        //S2L conversion
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            cmdBufferInUse,
            &m_secondLevelBatchBuffer[m_secondLevelBatchBufferIndex]));
    }
    else
    {
        //Short format 1st pass or long format
        // Setup static slice state parameters
        MHW_VDBOX_HEVC_SLICE_STATE_G11 hevcSliceState;
        hevcSliceState.presDataBuffer   = m_copyDataBufferInUse ? &m_resCopyDataBuffer : &m_resDataBuffer;
        hevcSliceState.pHevcPicParams   = m_hevcPicParams;
        hevcSliceState.pHevcExtPicParam = m_hevcExtPicParams;
        hevcSliceState.pRefIdxMapping   = &m_refIdxMapping[0];

        PCODEC_HEVC_SLICE_PARAMS     slc    = m_hevcSliceParams;
        PCODEC_HEVC_EXT_SLICE_PARAMS slcExt = m_hevcExtSliceParams;
        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            hevcSliceState.pHevcSliceParams = slc;
            hevcSliceState.pHevcExtSliceParams = slcExt;
            hevcSliceState.dwLength         = slc->slice_data_size;
            hevcSliceState.dwSliceIndex     = slcCount;
            hevcSliceState.bLastSlice          = (slcCount == (m_numSlices - 1));

            // If S2L and 1st pass, send HuC commands.
            if (m_shortFormatInUse)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(SendSliceS2L(cmdBufferInUse, &hevcSliceState));
            }
            else
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(SendSliceLongFormat(cmdBufferInUse, &hevcSliceState));
            }

            slc++;
            if (slcExt)
            {
                slcExt++;
            }
        }

        // If S2L and 1st pass
        if (m_shortFormatInUse && m_hcpDecPhase == CodechalHcpDecodePhaseLegacyS2L)
        {
            // Send VD Pipe Flush command for SKL+
            MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
            MOS_ZeroMemory(&vdpipeFlushParams , sizeof(vdpipeFlushParams));
            vdpipeFlushParams.Flags.bWaitDoneHEVC               = 1;
            vdpipeFlushParams.Flags.bFlushHEVC                  = 1;
            vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser     = 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
                cmdBufferInUse,
                &vdpipeFlushParams));

            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
                cmdBufferInUse,
                &flushDwParams));

            if (m_statusQueryReportingEnabled)
            {
                // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
                eStatus = m_hwInterface->SendCondBbEndCmd(
                        &m_decodeStatusBuf.m_statusBuffer,
                        statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatus2MaskOffset,
                        0,
                        false,
                        cmdBufferInUse);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);

                // Write HUC_STATUS mask
                MHW_MI_STORE_DATA_PARAMS storeDataParams;
                MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
                storeDataParams.pOsResource         = &m_decodeStatusBuf.m_statusBuffer;
                storeDataParams.dwResourceOffset    = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusMaskOffset;
                storeDataParams.dwValue             = m_hucInterface->GetHucStatusHevcS2lFailureMask();
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                    cmdBufferInUse,
                    &storeDataParams));

                // store HUC_STATUS register
                MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
                MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
                storeRegParams.presStoreBuffer      = &m_decodeStatusBuf.m_statusBuffer;
                storeRegParams.dwOffset             = statusBufferOffset + m_decodeStatusBuf.m_hucErrorStatusRegOffset;
                storeRegParams.dwRegister           = mmioRegisters->hucStatusRegOffset;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
                    cmdBufferInUse,
                    &storeRegParams));
            }

            if (m_enableSf2DmaSubmits)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
                    cmdBufferInUse,
                    nullptr));
            }

            m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
            if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer(m_scalabilityState, &scdryCmdBuffer));
            }

            if (m_enableSf2DmaSubmits)
            {
                CODECHAL_DEBUG_TOOL(
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                    cmdBufferInUse,
                    CODECHAL_NUM_MEDIA_STATES,
                    "_DEC"));

                //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
                //    m_debugInterface,
                //    cmdBufferInUse));
                );

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
                    m_osInterface,
                    cmdBufferInUse,
                    renderingFlags));
            }

            return eStatus;
        }
    }

    // Send VD Pipe Flush command for SKL+
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams , sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC               = 1;
    vdpipeFlushParams.Flags.bFlushHEVC                  = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser     = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(
        cmdBufferInUse,
        &vdpipeFlushParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBufferInUse,
        &flushDwParams));

    if (CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalablity_SetFECabacStreamoutOverflowStatus(
            m_scalabilityState,
            cmdBufferInUse));

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_SignalFE2BESemaphore(
            m_scalabilityState,
            cmdBufferInUse));

        //hcp decode status read at end of FE to support scalability
        if (m_statusQueryReportingEnabled)
        {
            EndStatusReportForFE(cmdBufferInUse);
            if (m_perfFEBETimingEnabled)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void *)this, m_osInterface, m_miInterface, &scdryCmdBuffer));
            }
        }
    }

    //Sync for decode completion in scalable mode
    if (CodecHalDecodeScalabilityIsBEPhase(m_scalabilityState))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_BEsCompletionSync(
            m_scalabilityState,
            cmdBufferInUse));
    }

    bool syncDestSurface = true;
    //if scalable decode,  BE0 finish means whole frame complete.
    // Check if destination surface needs to be synchronized
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        syncDestSurface = CodecHalDecodeScalabilityIsLastCompletePhase(m_scalabilityState);
    }

    if (syncDestSurface)
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_hevcPicParams->CurrPic))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                cmdBufferInUse,
                &syncParams));
        }

        if (m_statusQueryReportingEnabled)
        {
            CodechalDecodeStatusReport decodeStatusReport;
            decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
            decodeStatusReport.m_currDecodedPic     = m_hevcPicParams->CurrPic;
            decodeStatusReport.m_currDeblockedPic   = m_hevcPicParams->CurrPic;
            decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
            decodeStatusReport.m_currDecodedPicRes  = m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx]->resRefPic;
#ifdef _DECODE_PROCESSING_SUPPORTED
            CODECHAL_DEBUG_TOOL(
                if (m_downsampledSurfaces && m_sfcState && m_sfcState->m_sfcOutputSurface) {
                    m_downsampledSurfaces[m_hevcPicParams->CurrPic.FrameIdx].OsResource =
                        m_sfcState->m_sfcOutputSurface->OsResource;
                    decodeStatusReport.m_currSfcOutputPicRes =
                        &m_downsampledSurfaces[m_hevcPicParams->CurrPic.FrameIdx].OsResource;
                })
#endif
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(m_hevcPicParams->CurrPic);
                decodeStatusReport.m_frameType   = m_perfType;);

            if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
                    decodeStatusReport,
                    cmdBufferInUse));
            }
            else
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecode::EndStatusReport(
                    decodeStatusReport,
                    cmdBufferInUse));
            }
        }
    }

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBufferInUse,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        cmdBufferInUse,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &primCmdBuffer, 0);
    if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState) && MOS_VE_SUPPORTED(m_osInterface))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_ReturnSdryCmdBuffer(m_scalabilityState, &scdryCmdBuffer));
    }

    bool syncCompleteFrame = m_copyDataBufferInUse;
    if (MOS_VE_SUPPORTED(m_osInterface) && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        syncCompleteFrame = syncCompleteFrame && CodecHalDecodeScalabilityIsFEPhase(m_scalabilityState);
    }

    if (syncCompleteFrame)
    {
        //Sync up complete frame
        MOS_SYNC_PARAMS copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContextForWa;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &copyDataSyncParams));

        copyDataSyncParams = g_cInitSyncParams;
        copyDataSyncParams.GpuContext = m_videoContext;
        copyDataSyncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &copyDataSyncParams));
    }

    CODECHAL_DEBUG_TOOL(
        {
            if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_DbgDumpCmdBuffer(
                    this,
                    m_scalabilityState,
                    m_debugInterface,
                    &primCmdBuffer));
            }
            else
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                    &primCmdBuffer,
                    CODECHAL_NUM_MEDIA_STATES,
                    "_DEC"));

                //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
                //    m_debugInterface,
                //    &primCmdBuffer));
            }
        });

    bool submitCommand = true;
    //submit command buffer
    if (MOS_VE_SUPPORTED(m_osInterface) && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        submitCommand = CodecHalDecodeScalabilityIsToSubmitCmdBuffer(m_scalabilityState);
        HalOcaInterface::On1stLevelBBEnd(scdryCmdBuffer, *m_osInterface->pOsContext);
    }
    else
    {
        HalOcaInterface::On1stLevelBBEnd(primCmdBuffer, *m_osInterface->pOsContext);
    }
    if (submitCommand || m_osInterface->phasedSubmission)
    {
        //command buffer to submit is the primary cmd buffer.
        if (MOS_VE_SUPPORTED(m_osInterface))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&primCmdBuffer));
        }

        if(m_osInterface->phasedSubmission
           && MOS_VE_SUPPORTED(m_osInterface)
           && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            CodecHalDecodeScalability_DecPhaseToSubmissionType(m_scalabilityState,cmdBufferInUse);
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBufferInUse, renderingFlags));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &primCmdBuffer, renderingFlags));
        }
    }

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    // Reset status report
    if (m_statusQueryReportingEnabled)
    {
        bool resetStatusReport = true;

        //if scalable decode,  reset status report at final BE phase.
        if (CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
        {
            resetStatusReport = CodecHalDecodeScalabilityIsFinalBEPhase(m_scalabilityState);
        }

        if (resetStatusReport)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
                m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    m_hevcRefList[m_hevcPicParams->CurrPic.FrameIdx]->resRefPic =
        m_destSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!CodecHal_PictureIsField(m_hevcPicParams->CurrPic))
    {
        MOS_SYNC_PARAMS syncParams      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
    }
#ifdef _DECODE_PROCESSING_SUPPORTED
    // Send Vebox and SFC cmds
    bool sendSFC = m_sfcState->m_sfcPipeOut ? true : false;
    if (MOS_VE_SUPPORTED(m_osInterface) && CodecHalDecodeScalabilityIsScalableMode(m_scalabilityState))
    {
        sendSFC = (m_sfcState->m_sfcPipeOut ? true : false) && CodecHalDecodeScalabilityIsFinalBEPhase(m_scalabilityState);
    }
    if (sendSFC)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->RenderStart());
    }
#endif
    return eStatus;
}

MOS_STATUS CodechalDecodeHevcG11::AllocateStandard (
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width                         = settings->width;
    m_height                        = settings->height;
    m_is10BitHevc                   = (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? true : false;
    m_chromaFormatinProfile         = settings->chromaFormat;
    m_shortFormatInUse              = settings->shortFormatInUse;

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_sfcState = MOS_New(CodechalHevcSfcState);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_sfcState);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif

    MOS_ZeroMemory(&m_currPic, sizeof(m_currPic));

    m_frameIdx = 0;

    if (m_shortFormatInUse)
    {
        // Legacy SF has 2 passes, 1st pass is S2L, 2nd pass is HEVC Long decode
        // The pass number will be changed again if is scalable decode mode.
        m_decodePassNum            = 2;

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_SF_2_DMA_SUBMITS_ENABLE_ID,
            &userFeatureData);
        m_enableSf2DmaSubmits = userFeatureData.u32Data ? true : false;
    }

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11 stateCmdSizeParams;
    stateCmdSizeParams.bShortFormat    = m_shortFormatInUse;
    stateCmdSizeParams.bHucDummyStream = (m_secureDecoder ? m_secureDecoder->IsDummyStreamEnabled() : false);
    stateCmdSizeParams.bScalableMode   = static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported();

    // Picture Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetHxxStateCommandSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        &stateCmdSizeParams));

    // Primitive Level Commands
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetHxxPrimitiveCommandSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        m_shortFormatInUse));

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        if (static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->IsScalabilitySupported())
        {
            m_scalabilityState = (PCODECHAL_DECODE_SCALABILITY_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SCALABILITY_STATE));
            CODECHAL_DECODE_CHK_NULL_RETURN(m_scalabilityState);
            //scalability initialize
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeScalability_InitializeState(
                this,
                m_scalabilityState,
                m_hwInterface,
                m_shortFormatInUse));
        }
        else
        {
            //single pipe VE initialize
            m_sinlgePipeVeState = (PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE));
            CODECHAL_DECODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_InitInterface(m_osInterface, m_sinlgePipeVeState));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeHevc::AllocateResourcesFixedSizes());

    // Prepare Pic Params
    m_picMhwParams.PipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11);
    m_picMhwParams.SurfaceParams        = MOS_New(MHW_VDBOX_SURFACE_PARAMS);
    m_picMhwParams.PipeBufAddrParams    = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11);
    m_picMhwParams.IndObjBaseAddrParams = MOS_New(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS);
    m_picMhwParams.QmParams             = MOS_New(MHW_VDBOX_QM_PARAMS);
    m_picMhwParams.HevcPicState         = MOS_New(MHW_VDBOX_HEVC_PIC_STATE_G11);
    m_picMhwParams.HevcTileState        = MOS_New(MHW_VDBOX_HEVC_TILE_STATE);

    MOS_ZeroMemory(m_picMhwParams.SurfaceParams, sizeof(MHW_VDBOX_SURFACE_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.IndObjBaseAddrParams, sizeof(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.QmParams, sizeof(MHW_VDBOX_QM_PARAMS));
    MOS_ZeroMemory(m_picMhwParams.HevcTileState, sizeof(MHW_VDBOX_HEVC_TILE_STATE));

    return eStatus;
}

CodechalDecodeHevcG11::CodechalDecodeHevcG11(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecodeHevc(hwInterface, debugInterface, standardInfo),
                                            m_hevcExtPicParams(nullptr),
                                            m_hevcExtSliceParams(nullptr),
                                            m_frameSizeMaxAlloced(0),
                                            m_sinlgePipeVeState(nullptr),
                                            m_scalabilityState(nullptr)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

     Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeHevcG11::DumpPicParams(
    PCODEC_HEVC_PIC_PARAMS     picParams,
    PCODEC_HEVC_EXT_PIC_PARAMS extPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss << "PicWidthInMinCbsY: " << +picParams->PicWidthInMinCbsY << std::endl;
    oss << "PicHeightInMinCbsY: " << +picParams->PicHeightInMinCbsY << std::endl;
    //wFormatAndSequenceInfoFlags
    oss << "chroma_format_idc: " << +picParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag: " << +picParams->separate_colour_plane_flag << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "NoPicReorderingFlag: " << +picParams->NoPicReorderingFlag << std::endl;
    oss << "ReservedBits1: " << +picParams->ReservedBits1 << std::endl;
    oss << "wFormatAndSequenceInfoFlags: " << +picParams->wFormatAndSequenceInfoFlags << std::endl;
    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;
    oss << "sps_max_dec_pic_buffering_minus1: " << +picParams->sps_max_dec_pic_buffering_minus1 << std::endl;
    oss << "log2_min_luma_coding_block_size_minus3: " << +picParams->log2_min_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_luma_coding_block_size: " << +picParams->log2_diff_max_min_luma_coding_block_size << std::endl;
    oss << "log2_min_transform_block_size_minus2: " << +picParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "log2_diff_max_min_transform_block_size: " << +picParams->log2_diff_max_min_transform_block_size << std::endl;
    oss << "max_transform_hierarchy_depth_intra: " << +picParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter: " << +picParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "num_short_term_ref_pic_sets: " << +picParams->num_short_term_ref_pic_sets << std::endl;
    oss << "num_long_term_ref_pic_sps: " << +picParams->num_long_term_ref_pic_sps << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1: " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1: " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "init_qp_minus26: " << +picParams->init_qp_minus26 << std::endl;
    oss << "ucNumDeltaPocsOfRefRpsIdx: " << +picParams->ucNumDeltaPocsOfRefRpsIdx << std::endl;
    oss << "wNumBitsForShortTermRPSInSlice: " << +picParams->wNumBitsForShortTermRPSInSlice << std::endl;
    oss << "ReservedBits2: " << +picParams->ReservedBits2 << std::endl;
    //dwCodingParamToolFlags
    oss << "scaling_list_enabled_flag: " << +picParams->scaling_list_enabled_flag << std::endl;
    oss << "amp_enabled_flag: " << +picParams->amp_enabled_flag << std::endl;
    oss << "sample_adaptive_offset_enabled_flag: " << +picParams->sample_adaptive_offset_enabled_flag << std::endl;
    oss << "pcm_enabled_flag: " << +picParams->pcm_enabled_flag << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1: " << +picParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1: " << +picParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "log2_min_pcm_luma_coding_block_size_minus3: " << +picParams->log2_min_pcm_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_pcm_luma_coding_block_size: " << +picParams->log2_diff_max_min_pcm_luma_coding_block_size << std::endl;
    oss << "pcm_loop_filter_disabled_flag: " << +picParams->pcm_loop_filter_disabled_flag << std::endl;
    oss << "long_term_ref_pics_present_flag: " << +picParams->long_term_ref_pics_present_flag << std::endl;
    oss << "sps_temporal_mvp_enabled_flag: " << +picParams->sps_temporal_mvp_enabled_flag << std::endl;
    oss << "strong_intra_smoothing_enabled_flag: " << +picParams->strong_intra_smoothing_enabled_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag: " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "output_flag_present_flag: " << +picParams->output_flag_present_flag << std::endl;
    oss << "num_extra_slice_header_bits: " << +picParams->num_extra_slice_header_bits << std::endl;
    oss << "sign_data_hiding_enabled_flag: " << +picParams->sign_data_hiding_enabled_flag << std::endl;
    oss << "cabac_init_present_flag: " << +picParams->cabac_init_present_flag << std::endl;
    oss << "ReservedBits3: " << +picParams->ReservedBits3 << std::endl;
    oss << "dwCodingParamToolFlags: " << +picParams->dwCodingParamToolFlags << std::endl;
    //dwCodingSettingPicturePropertyFlags
    oss << "constrained_intra_pred_flag: " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag: " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag: " << +picParams->cu_qp_delta_enabled_flag << std::endl;
    oss << "diff_cu_qp_delta_depth: " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_slice_chroma_qp_offsets_present_flag: " << +picParams->pps_slice_chroma_qp_offsets_present_flag << std::endl;
    oss << "weighted_pred_flag: " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag: " << +picParams->weighted_bipred_flag << std::endl;
    oss << "transquant_bypass_enabled_flag: " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "tiles_enabled_flag: " << +picParams->tiles_enabled_flag << std::endl;
    oss << "entropy_coding_sync_enabled_flag: " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "uniform_spacing_flag: " << +picParams->uniform_spacing_flag << std::endl;
    oss << "loop_filter_across_tiles_enabled_flag: " << +picParams->loop_filter_across_tiles_enabled_flag << std::endl;
    oss << "pps_loop_filter_across_slices_enabled_flag: " << +picParams->pps_loop_filter_across_slices_enabled_flag << std::endl;
    oss << "deblocking_filter_override_enabled_flag: " << +picParams->deblocking_filter_override_enabled_flag << std::endl;
    oss << "pps_deblocking_filter_disabled_flag: " << +picParams->pps_deblocking_filter_disabled_flag << std::endl;
    oss << "lists_modification_present_flag: " << +picParams->lists_modification_present_flag << std::endl;
    oss << "slice_segment_header_extension_present_flag: " << +picParams->slice_segment_header_extension_present_flag << std::endl;
    oss << "IrapPicFlag: " << +picParams->IrapPicFlag << std::endl;
    oss << "IdrPicFlag: " << +picParams->IdrPicFlag << std::endl;
    oss << "IntraPicFlag: " << +picParams->IntraPicFlag << std::endl;
    oss << "ReservedBits4: " << +picParams->ReservedBits4 << std::endl;
    oss << "dwCodingSettingPicturePropertyFlags: " << +picParams->dwCodingSettingPicturePropertyFlags << std::endl;
    oss << "pps_cb_qp_offset: " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset: " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1: " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1: " << +picParams->num_tile_rows_minus1 << std::endl;
    //Dump column width
    oss << "column_width_minus1[19]:";
    for (uint8_t i = 0; i < 19; i++)
        oss << picParams->column_width_minus1[i] << " ";
    oss << std::endl;

    //Dump row height
    oss << "row_height_minus1[21]:";
    for (uint8_t i = 0; i < 21; i++)
        oss << picParams->row_height_minus1[i] << " ";
    oss << std::endl;

    oss << "pps_beta_offset_div2: " << +picParams->pps_beta_offset_div2 << std::endl;
    oss << "pps_tc_offset_div2: " << +picParams->pps_tc_offset_div2 << std::endl;
    oss << "log2_parallel_merge_level_minus2: " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "CurrPicOrderCntVal: " << +picParams->CurrPicOrderCntVal << std::endl;

    oss.setf(std::ios::dec, std::ios::basefield);
    //Dump RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    //Dump POC List
    oss << "PicOrderCntValList[15]:";
    for (uint8_t i = 0; i < 15; i++)
        oss << std::hex << picParams->PicOrderCntValList[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List
    oss << "RefPicSetStCurrBefore[8]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetStCurrBefore[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List
    oss << "RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetStCurrAfter[i] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List
    oss << "RefPicSetLtCurr[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->RefPicSetLtCurr[i] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrBefore List with POC
    oss << "RefPicSetStCurrBefore[8] (POC): ";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetStCurrBefore[i]] << " ";
    oss << std::endl;

    //Dump Ref RefPicSetStCurrAfter List with POC
    oss << "RefPicSetStCurrAfter[16] (POC):";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetStCurrAfter[i]] << " ";
    oss << std::endl;

    //Dump Ref PicSetStCurr List with POC
    oss << "RefPicSetLtCurr[16] (POC): ";
    for (uint8_t i = 0; i < 8; i++)
        oss << picParams->PicOrderCntValList[picParams->RefPicSetLtCurr[i]] << " ";
    oss << std::endl;

    oss << "RefFieldPicFlag: " << +picParams->RefFieldPicFlag << std::endl;
    oss << "RefBottomFieldFlag: " << +picParams->RefBottomFieldFlag << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    if (extPicParams)
    {
        //PicRangeExtensionFlags
        oss << "transform_skip_rotation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_rotation_enabled_flag << std::endl;
        oss << "transform_skip_context_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_context_enabled_flag << std::endl;
        oss << "implicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.implicit_rdpcm_enabled_flag << std::endl;
        oss << "explicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.explicit_rdpcm_enabled_flag << std::endl;
        oss << "extended_precision_processing_flag: " << +extPicParams->PicRangeExtensionFlags.fields.extended_precision_processing_flag << std::endl;
        oss << "intra_smoothing_disabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.intra_smoothing_disabled_flag << std::endl;
        oss << "high_precision_offsets_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag << std::endl;
        oss << "persistent_rice_adaptation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.persistent_rice_adaptation_enabled_flag << std::endl;
        oss << "cabac_bypass_alignment_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag << std::endl;
        oss << "cross_component_prediction_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag << std::endl;
        oss << "chroma_qp_offset_list_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag << std::endl;
        oss << "BitDepthLuma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthLuma16 << std::endl;
        oss << "BitDepthChroma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthChroma16 << std::endl;
        oss << "diff_cu_chroma_qp_offset_depth: " << +extPicParams->diff_cu_chroma_qp_offset_depth << std::endl;
        oss << "chroma_qp_offset_list_len_minus1: " << +extPicParams->chroma_qp_offset_list_len_minus1 << std::endl;
        oss << "log2_sao_offset_scale_luma: " << +extPicParams->log2_sao_offset_scale_luma << std::endl;
        oss << "log2_sao_offset_scale_chroma: " << +extPicParams->log2_sao_offset_scale_chroma << std::endl;
        oss << "log2_max_transform_skip_block_size_minus2: " << +extPicParams->log2_max_transform_skip_block_size_minus2 << std::endl;

        //Dump cb_qp_offset_list[6]
        oss << "cb_qp_offset_list[6]: ";
        for (uint8_t i = 0; i < 6; i++)
            oss << extPicParams->cb_qp_offset_list[i] << " ";
        oss << std::endl;

        //Dump cr_qp_offset_list[6]
        oss << "cr_qp_offset_list[6]: ";
        for (uint8_t i = 0; i < 6; i++)
            oss << extPicParams->cr_qp_offset_list[i] << " ";
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeHevcG11::DumpSliceParams(
    PCODEC_HEVC_SLICE_PARAMS     sliceParams,
    PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
    uint32_t                     numSlices,
    bool                         shortFormatInUse)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    PCODEC_HEVC_SLICE_PARAMS     hevcSliceControl    = nullptr;
    PCODEC_HEVC_EXT_SLICE_PARAMS hevcExtSliceControl = nullptr;

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint16_t j = 0; j < numSlices; j++)
    {
        hevcSliceControl = &sliceParams[j];
        if (extSliceParams)
        {
            hevcExtSliceControl = &extSliceParams[j];
        }

        oss << "====================================================================================================" << std::endl;
        oss << "Data for Slice number = " << +j << std::endl;
        oss << "slice_data_size: " << +hevcSliceControl->slice_data_size << std::endl;
        oss << "slice_data_offset: " << +hevcSliceControl->slice_data_offset << std::endl;

        if (!shortFormatInUse)
        {
            //Dump Long format specific
            oss << "ByteOffsetToSliceData: " << +hevcSliceControl->ByteOffsetToSliceData << std::endl;
            oss << "slice_segment_address: " << +hevcSliceControl->slice_segment_address << std::endl;

            //Dump RefPicList[2][15]
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[0][" << +i << "]";
                oss << "FrameIdx: " << +hevcSliceControl->RefPicList[0][i].FrameIdx;
                oss << ", PicFlags: " << +hevcSliceControl->RefPicList[0][i].PicFlags;
                oss << std::endl;
            }
            for (uint8_t i = 0; i < 15; ++i)
            {
                oss << "RefPicList[1][" << +i << "]";
                oss << "FrameIdx: " << +hevcSliceControl->RefPicList[1][i].FrameIdx;
                oss << ", PicFlags: " << +hevcSliceControl->RefPicList[1][i].PicFlags;
                oss << std::endl;
            }

            oss << "last_slice_of_pic: " << +hevcSliceControl->LongSliceFlags.fields.LastSliceOfPic << std::endl;
            oss << "dependent_slice_segment_flag: " << +hevcSliceControl->LongSliceFlags.fields.dependent_slice_segment_flag << std::endl;
            oss << "slice_type: " << +hevcSliceControl->LongSliceFlags.fields.slice_type << std::endl;
            oss << "color_plane_id: " << +hevcSliceControl->LongSliceFlags.fields.color_plane_id << std::endl;
            oss << "slice_sao_luma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_luma_flag << std::endl;
            oss << "slice_sao_chroma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_chroma_flag << std::endl;
            oss << "mvd_l1_zero_flag: " << +hevcSliceControl->LongSliceFlags.fields.mvd_l1_zero_flag << std::endl;
            oss << "cabac_init_flag: " << +hevcSliceControl->LongSliceFlags.fields.cabac_init_flag << std::endl;
            oss << "slice_temporal_mvp_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag << std::endl;
            oss << "slice_deblocking_filter_disabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag << std::endl;
            oss << "collocated_from_l0_flag: " << +hevcSliceControl->LongSliceFlags.fields.collocated_from_l0_flag << std::endl;
            oss << "slice_loop_filter_across_slices_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag << std::endl;
            oss << "reserved: " << +hevcSliceControl->LongSliceFlags.fields.reserved << std::endl;
            oss << "collocated_ref_idx: " << +hevcSliceControl->collocated_ref_idx << std::endl;
            oss << "num_ref_idx_l0_active_minus1: " << +hevcSliceControl->num_ref_idx_l0_active_minus1 << std::endl;
            oss << "num_ref_idx_l1_active_minus1: " << +hevcSliceControl->num_ref_idx_l1_active_minus1 << std::endl;
            oss << "slice_qp_delta: " << +hevcSliceControl->slice_qp_delta << std::endl;
            oss << "slice_cb_qp_offset: " << +hevcSliceControl->slice_cb_qp_offset << std::endl;
            oss << "slice_cr_qp_offset: " << +hevcSliceControl->slice_cr_qp_offset << std::endl;
            oss << "slice_beta_offset_div2: " << +hevcSliceControl->slice_beta_offset_div2 << std::endl;
            oss << "slice_tc_offset_div2: " << +hevcSliceControl->slice_tc_offset_div2 << std::endl;
            oss << "luma_log2_weight_denom: " << +hevcSliceControl->luma_log2_weight_denom << std::endl;
            oss << "delta_chroma_log2_weight_denom: " << +hevcSliceControl->delta_chroma_log2_weight_denom << std::endl;

            //Dump luma_offset[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "luma_offset_l0[" << +i << "]: " << (extSliceParams ? +hevcExtSliceControl->luma_offset_l0[i] : +hevcSliceControl->luma_offset_l0[i]) << std::endl;
                oss << "luma_offset_l1[" << +i << "]: " << (extSliceParams ? +hevcExtSliceControl->luma_offset_l1[i] : +hevcSliceControl->luma_offset_l1[i]) << std::endl;
            }
            //Dump delta_luma_weight[2][15]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_luma_weight_l0[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
                oss << "delta_luma_weight_l1[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
            }
            //Dump chroma_offset[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "ChromaOffsetL0[" << +i << "][0]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL0[i][0] : +hevcSliceControl->ChromaOffsetL0[i][0]) << std::endl;

                oss << "ChromaOffsetL0[" << +i << "][1]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL0[i][1] : +hevcSliceControl->ChromaOffsetL0[i][1]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][0]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL1[i][0] : +hevcSliceControl->ChromaOffsetL1[i][0]) << std::endl;

                oss << "ChromaOffsetL1[" << +i << "][1]: " << (extSliceParams ? +hevcExtSliceControl->ChromaOffsetL1[i][1] : +hevcSliceControl->ChromaOffsetL1[i][1]) << std::endl;
            }
            //Dump delta_chroma_weight[2][15][2]
            for (uint8_t i = 0; i < 15; i++)
            {
                oss << "delta_chroma_weight_l0[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l0[i][0] << std::endl;
                oss << "delta_chroma_weight_l0[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l0[i][1] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l1[i][0] << std::endl;
                oss << "delta_chroma_weight_l1[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l1[i][1] << std::endl;
            }

            oss << "five_minus_max_num_merge_cand: " << +hevcSliceControl->five_minus_max_num_merge_cand << std::endl;

            if (extSliceParams)
            {
                oss << "cu_chroma_qp_offset_enabled_flag: " << +hevcExtSliceControl->cu_chroma_qp_offset_enabled_flag << std::endl;
            }
        }

        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSlcParams,
            CodechalDbgExtType::txt);

        std::ofstream ofs;
        if (j == 0)
        {
            ofs.open(fileName, std::ios::out);
        }
        else
        {
            ofs.open(fileName, std::ios::app);
        }
        ofs << oss.str();
        ofs.close();
    }
    return MOS_STATUS_SUCCESS;
}
#endif
