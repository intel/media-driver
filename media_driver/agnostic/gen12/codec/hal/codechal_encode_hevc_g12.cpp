/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_hevc_g12.cpp
//! \brief    HEVC dual-pipe encoder for GEN12.
//!

#include "codechal_encode_hevc_g12.h"
#include "codechal_encode_csc_ds_g12.h"
#include "codechal_mmc_encode_hevc_g12.h"
#include "codechal_encode_wp_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codechal_kernel_hme_g12.h"
#include "codechal_debug.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#include "codeckrnheader.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_mi_g12_X.h"
#include "mhw_render_g12_X.h"
#include "media_user_settings_mgr_g12.h"
#include "cm_queue_rt.h"
#include "codechal_debug.h"

//! \cond SKIP_DOXYGEN
#define CRECOST(lambda, mode, lcu, slice) (Map44LutValue((uint32_t)((lambda) * (m_modeBits[(lcu)][(mode)][(slice)]) * (m_modeBitsScale[(mode)][(slice)])), 0x8F))
#define RDEBITS62(mode, lcu, slice) (GetU62ModeBits((float)((m_modeBits[(lcu)][(mode)][(slice)]) * (m_modeBitsScale[(mode)][(slice)]))))
//! \endcond

MOS_STATUS CodechalEncHevcStateG12::SetGpuCtxCreatOption()
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
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // call MI_VD_CONTROL_STATE before HCP_PIPE_SELECT to init the pipe.
    {
        MHW_MI_VD_CONTROL_STATE_PARAMS vdControlStateParams;
        //set up VD_CONTROL_STATE command
        {
            MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
            vdControlStateParams.initialization = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                static_cast<MhwMiInterfaceG12 *>(m_miInterface)->AddMiVdControlStateCmd(cmdBuffer, &vdControlStateParams));
        }
    }

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
    SetHcpPipeModeSelectParams(pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    return eStatus;
}

void CodechalEncHevcStateG12::SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams)
{
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &pipeModeSelectParams =
        static_cast<MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &>(vdboxPipeModeSelectParams);
    pipeModeSelectParams = {};
    CodechalEncodeHevcBase::SetHcpPipeModeSelectParams(vdboxPipeModeSelectParams);

    pipeModeSelectParams.pakPiplnStrmoutEnabled = m_pakPiplStrmOutEnable;
    pipeModeSelectParams.pakFrmLvlStrmoutEnable = (m_brcEnabled && m_numPipe > 1);

    if (m_numPipe > 1)
    {
        // Running in the multiple VDBOX mode
        if (IsFirstPipe())
        {
            pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
        }
        else if (IsLastPipe())
        {
            pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
        }
        else
        {
            pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
        }
        pipeModeSelectParams.PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
    }
    else
    {
        pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        pipeModeSelectParams.PipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    }
}

void CodechalEncHevcStateG12::SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE &picStateParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpPicStateParams(picStateParams);
    picStateParams.sseEnabledInVmeEncode = m_sseEnabled;
}

MOS_STATUS CodechalEncHevcStateG12::AddHcpSurfaceStateCmds(MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_VDBOX_SURFACE_PARAMS srcSurfaceParams;
    SetHcpSrcSurfaceParams(srcSurfaceParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(cmdBuffer, &srcSurfaceParams));

    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    SetHcpReconSurfaceParams(reconSurfaceParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(cmdBuffer, &reconSurfaceParams));

    // Add the surface state for reference picture, GEN12 HW change
    reconSurfaceParams.ucSurfaceStateId = CODECHAL_HCP_REF_SURFACE_ID;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(cmdBuffer, &reconSurfaceParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AddHcpPictureStateCmd(MOS_COMMAND_BUFFER *cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_VDBOX_HEVC_PIC_STATE_G12 picStateParams;

    SetHcpPicStateParams(picStateParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(cmdBuffer, &picStateParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::UpdateYUY2SurfaceInfo(
    MOS_SURFACE &surface,
    bool         is10Bit)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (surface.Format == Format_YUY2V)
    {
        // surface has been updated
        return eStatus;
    }

    if (surface.Format != Format_YUY2 &&
        surface.Format != Format_Y210 &&
        surface.Format != Format_Y216)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (surface.dwWidth < m_oriFrameWidth / 2 || surface.dwHeight < m_oriFrameHeight * 2)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    surface.Format   = is10Bit ? Format_Y216V : Format_YUY2V;
    surface.dwWidth  = m_oriFrameWidth;
    surface.dwHeight = m_oriFrameHeight;

    surface.YPlaneOffset.iSurfaceOffset = 0;
    surface.YPlaneOffset.iXOffset       = 0;
    surface.YPlaneOffset.iYOffset       = 0;

    surface.UPlaneOffset.iSurfaceOffset = surface.dwHeight * surface.dwPitch;
    surface.UPlaneOffset.iXOffset       = 0;
    surface.UPlaneOffset.iYOffset       = surface.dwHeight;

    surface.VPlaneOffset.iSurfaceOffset = surface.dwHeight * surface.dwPitch;
    surface.VPlaneOffset.iXOffset       = 0;
    surface.VPlaneOffset.iYOffset       = surface.dwHeight;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::InitializePicture(const EncoderParams &params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::InitializePicture(params));

    if (m_resolutionChanged)
    {
        ResizeBufferOffset();
    }

    m_sseEnabled = false;
    // only 420 format support SSE output
    // see TDR in scalability case, disable SSE for now before HW confirm the capability.
    if (m_sseSupported &&
        m_hevcSeqParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV420 &&
        m_numPipe == 1)
    {
        m_sseEnabled = true;
    }

    // for HEVC VME, HUC based WP is not supported.
    m_hevcPicParams->bEnableGPUWeightedPrediction = false;

    m_pakPiplStrmOutEnable = m_sseEnabled || (m_brcEnabled && m_numPipe > 1);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTileData(m_tileParams, params.dwBitstreamSize));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateTileStatistics());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResourcesVariableSize());

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::SetPictureStructs());

    if (m_minMaxQpControlEnabled)
    {
        //if Min Max QP is on disable Frame Panic Mode
        m_enableFramePanicMode = false;
    }

    // This is an additional (the 5th) PAK pass for BRC panic mode. Enabled for the single pipe case only.
    // Panic mode is not supported with Min/Max QP
    if (m_brcEnabled && m_enableFramePanicMode && (false == m_hevcSeqParams->DisableHRDConformance) &&
        (I_TYPE != m_hevcPicParams->CodingType) &&
        (m_numPipe == 1))
    {
        m_numPasses++;
    }

    m_virtualEngineBbIndex = m_currOriginalPic.FrameIdx;

    if ((uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_chromaFormat &&
        (uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_outputChromaFormat)
    {
        uint8_t currRefIdx = m_hevcPicParams->CurrReconstructedPic.FrameIdx;
        UpdateYUY2SurfaceInfo(m_refList[currRefIdx]->sRefBuffer, m_is10BitHevc);

        if (m_pictureCodingType != I_TYPE)
        {
            for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                if (!m_picIdx[i].bValid || !m_currUsedRefPic[i])
                {
                    continue;
                }
                uint8_t picIdx = m_picIdx[i].ucPicIdx;
                CODECHAL_ENCODE_ASSERT(picIdx < 127);

                UpdateYUY2SurfaceInfo((m_refList[picIdx]->sRefBuffer), m_is10BitHevc);
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ConvertY210ToY210V(
    PMOS_SURFACE source,
    PMOS_SURFACE target)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(source);
    CODECHAL_ENCODE_CHK_NULL_RETURN(target);

    if (m_oriFrameWidth > target->dwWidth || m_oriFrameHeight > target->dwHeight ||
        m_oriFrameWidth > source->dwWidth || m_oriFrameHeight > source->dwHeight)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_LOCK_PARAMS lockRead, lockWrite;
    MOS_ZeroMemory(&lockRead, sizeof(lockRead));
    MOS_ZeroMemory(&lockWrite, sizeof(lockWrite));

    lockRead.ReadOnly   = 1;
    lockWrite.WriteOnly = 1;

    uint16_t *srcData = (uint16_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &source->OsResource,
        &lockRead);

    if (srcData == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    uint8_t *dstData = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &target->OsResource,
        &lockWrite);

    if (dstData == nullptr)
    {
        // release the lock on srcData acquired above before returning here
        m_osInterface->pfnUnlockResource(m_osInterface, &source->OsResource);
        return MOS_STATUS_NULL_POINTER;
    }

    uint32_t highBits = MOS_ALIGN_CEIL(target->dwWidth, 32);
    uint32_t srcPitch = source->dwPitch / sizeof(srcData[0]);
    uint32_t dstPitch = target->dwPitch / sizeof(dstData[0]);

    //Y
    for (uint32_t h = 0; h < m_oriFrameHeight; h++)
    {
        for (uint32_t w = 0; w < m_oriFrameWidth; w++)
        {
            uint16_t d = srcData[w * 2 + h * srcPitch];

            dstData[w + h * dstPitch + 0]        = (uint8_t)(d >> 8);
            dstData[w + h * dstPitch + highBits] = (uint8_t)(d >> 6) & 3;
        }
    }

    uint32_t uvOffset = target->dwPitch * target->dwHeight;

    //UV
    for (uint32_t h = 0; h < m_oriFrameHeight; h++)
    {
        for (uint32_t w = 0; w < m_oriFrameWidth; w++)
        {
            uint16_t d = srcData[w * 2 + 1 + h * srcPitch];

            dstData[uvOffset + w + h * dstPitch + 0]        = (uint8_t)(d >> 8);
            dstData[uvOffset + w + h * dstPitch + highBits] = (uint8_t)(d >> 6) & 3;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ConvertP010ToP010V(
    PMOS_SURFACE source,
    PMOS_SURFACE target)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(source);
    CODECHAL_ENCODE_CHK_NULL_RETURN(target);

    if (m_oriFrameWidth > target->dwWidth || m_oriFrameHeight > target->dwHeight ||
        m_oriFrameWidth > source->dwWidth || m_oriFrameHeight > source->dwHeight)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_LOCK_PARAMS lockRead, lockWrite;
    MOS_ZeroMemory(&lockRead, sizeof(lockRead));
    MOS_ZeroMemory(&lockWrite, sizeof(lockWrite));
    lockRead.ReadOnly   = 1;
    lockWrite.WriteOnly = 1;

    uint16_t *srcData = (uint16_t *)m_osInterface->pfnLockResource(m_osInterface,
        &source->OsResource,
        &lockRead);
    if (srcData == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    uint8_t *dstData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &target->OsResource, &lockWrite);
    if (dstData == nullptr)
    {
        // release the lock on srcData acquired above before returning here
        m_osInterface->pfnUnlockResource(m_osInterface, &source->OsResource);
        return MOS_STATUS_NULL_POINTER;
    }

    uint32_t highBits = MOS_ALIGN_CEIL(target->dwWidth, 32);
    uint32_t srcPitch = source->dwPitch / sizeof(srcData[0]);
    uint32_t dstPitch = target->dwPitch / sizeof(dstData[0]);

    //Y
    for (uint32_t h = 0; h < m_oriFrameHeight; h++)
    {
        for (uint32_t w = 0; w < m_oriFrameWidth; w++)
        {
            uint16_t d = srcData[w + h * srcPitch];

            dstData[w + h * dstPitch + 0]        = (uint8_t)(d >> 8);
            dstData[w + h * dstPitch + highBits] = (uint8_t)(d >> 6) & 3;
        }
    }

    uint32_t dstUvOffset = target->dwPitch * target->dwHeight;
    uint32_t srcUvOffset = srcPitch * source->dwHeight;

    //UV
    for (uint32_t h = 0; h < m_oriFrameHeight / 2; h++)
    {
        for (uint32_t w = 0; w < m_oriFrameWidth; w++)
        {
            uint16_t d = srcData[srcUvOffset + w + h * srcPitch];

            dstData[dstUvOffset + w + h * dstPitch + 0]        = (uint8_t)(d >> 8);
            dstData[dstUvOffset + w + h * dstPitch + highBits] = (uint8_t)(d >> 6) & 3;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ConvertYUY2ToYUY2V(
    PMOS_SURFACE source,
    PMOS_SURFACE target)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(source);
    CODECHAL_ENCODE_CHK_NULL_RETURN(target);

    if (m_oriFrameWidth > target->dwWidth || m_oriFrameHeight > target->dwHeight ||
        m_oriFrameWidth > source->dwWidth || m_oriFrameHeight > source->dwHeight)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_LOCK_PARAMS lockRead, lockWrite;
    MOS_ZeroMemory(&lockRead, sizeof(lockRead));
    MOS_ZeroMemory(&lockWrite, sizeof(lockWrite));

    lockRead.ReadOnly   = 1;
    lockWrite.WriteOnly = 1;

    uint8_t *srcData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &source->OsResource, &lockRead);
    if (srcData == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    uint8_t *dstData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &target->OsResource, &lockWrite);
    if (dstData == nullptr)
    {
        // release the lock on srcData acquired above before returning here
        m_osInterface->pfnUnlockResource(m_osInterface, &source->OsResource);
        return MOS_STATUS_NULL_POINTER;
    }

    //Y
    for (uint32_t h = 0; h < m_oriFrameHeight; h++)
    {
        for (uint32_t w = 0; w < m_oriFrameWidth; w++)
        {
            uint8_t d = srcData[w * 2 + h * source->dwPitch];

            dstData[w + h * target->dwPitch] = d;
        }
    }

    uint32_t uvOffset = target->dwPitch * target->dwHeight;

    //UV
    for (uint32_t h = 0; h < m_oriFrameHeight; h++)
    {
        for (uint32_t w = 0; w < m_oriFrameWidth; w++)
        {
            uint8_t d = srcData[w * 2 + 1 + h * source->dwPitch];

            dstData[uvOffset + w + h * target->dwPitch] = d;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::DownScaling2X(
    PMOS_SURFACE source,
    PMOS_SURFACE target)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(source);
    CODECHAL_ENCODE_CHK_NULL_RETURN(target);

    if ((source->Format != Format_NV12 && source->Format != Format_YUY2V && source->Format != Format_Y216V) ||
        (target->Format != Format_NV12 && target->Format != Format_YUY2V))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_LOCK_PARAMS lockRead, lockWrite;
    MOS_ZeroMemory(&lockRead, sizeof(lockRead));
    MOS_ZeroMemory(&lockWrite, sizeof(lockWrite));

    lockRead.ReadOnly   = 1;
    lockWrite.WriteOnly = 1;

    uint8_t *srcData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &source->OsResource, &lockRead);
    if (srcData == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    uint8_t *dstData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &target->OsResource, &lockWrite);
    if (dstData == nullptr)
    {
        // release the lock on srcData acquired above before returning here
        m_osInterface->pfnUnlockResource(m_osInterface, &source->OsResource);
        return MOS_STATUS_NULL_POINTER;
    }

    //Y
    for (uint32_t h = 0, h2 = 0; h < m_oriFrameHeight; h += 2, h2++)
    {
        for (uint32_t w = 0, w2 = 0; w < m_oriFrameWidth; w += 2, w2++)
        {
            int16_t sum =
                (int16_t)srcData[(h + 0) * source->dwPitch + w + 0] +
                (int16_t)srcData[(h + 0) * source->dwPitch + w + 1] +
                (int16_t)srcData[(h + 1) * source->dwPitch + w + 0] +
                (int16_t)srcData[(h + 1) * source->dwPitch + w + 1];

            sum                                = sum >> 2;
            dstData[h2 * target->dwPitch + w2] = (uint8_t)sum;
        }
    }

    srcData = srcData + source->dwHeight * source->dwPitch;
    dstData = dstData + target->dwHeight * target->dwPitch;

    uint32_t uvHeightRatio = (source->Format == Format_NV12) ? 2 : 1;

    //UV
    for (uint32_t h = 0, h2 = 0; h < m_oriFrameHeight / uvHeightRatio; h += 2, h2++)
    {
        for (uint32_t w = 0, w2 = 0; w < m_oriFrameWidth; w += 4, w2 += 2)
        {
            // U
            int16_t sum =
                (int16_t)srcData[(h + 0) * source->dwPitch + w + 0] +
                (int16_t)srcData[(h + 0) * source->dwPitch + w + 2] +
                (int16_t)srcData[(h + 1) * source->dwPitch + w + 0] +
                (int16_t)srcData[(h + 1) * source->dwPitch + w + 2];
            sum                                    = sum >> 2;
            dstData[h2 * target->dwPitch + w2 + 0] = (uint8_t)sum;

            // V
            sum =
                (int16_t)srcData[(h + 0) * source->dwPitch + w + 1] +
                (int16_t)srcData[(h + 0) * source->dwPitch + w + 3] +
                (int16_t)srcData[(h + 1) * source->dwPitch + w + 1] +
                (int16_t)srcData[(h + 1) * source->dwPitch + w + 3];
            sum                                    = sum >> 2;
            dstData[h2 * target->dwPitch + w2 + 1] = (uint8_t)sum;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetKernelParams(
    EncOperation      encOperation,
    MHW_KERNEL_PARAM *kernelParams,
    uint32_t          idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    kernelParams->iThreadCount = m_hwInterface->GetRenderInterface()->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount     = 1;

    uint32_t curbeAlignment = m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();
    switch (encOperation)
    {
    case ENC_MBENC:
    {
        switch (idx)
        {
        case MBENC_LCU32_KRNIDX:
            kernelParams->iBTCount     = MBENC_B_FRAME_END - MBENC_B_FRAME_BEGIN;
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MBENC_LCU32_BTI), (size_t)curbeAlignment);
            kernelParams->iBlockWidth  = CODECHAL_HEVC_MAX_LCU_SIZE_G9;
            kernelParams->iBlockHeight = CODECHAL_HEVC_MAX_LCU_SIZE_G9;
            break;

        case MBENC_LCU64_KRNIDX:
            kernelParams->iBTCount     = MBENC_B_FRAME_END - MBENC_B_FRAME_BEGIN;
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MBENC_LCU64_BTI), (size_t)curbeAlignment);
            kernelParams->iBlockWidth  = CODECHAL_HEVC_MAX_LCU_SIZE_G10;
            kernelParams->iBlockHeight = CODECHAL_HEVC_MAX_LCU_SIZE_G10;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported MBENC mode requested");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    break;

    case ENC_BRC:
    {
        switch (idx)
        {
        case CODECHAL_HEVC_BRC_INIT:
        case CODECHAL_HEVC_BRC_RESET:
            kernelParams->iBTCount     = BRC_INIT_RESET_END - BRC_INIT_RESET_BEGIN;
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(BRC_INITRESET_CURBE), (size_t)curbeAlignment);
            kernelParams->iBlockWidth  = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
            kernelParams->iBlockHeight = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
            break;

        case CODECHAL_HEVC_BRC_FRAME_UPDATE:
            kernelParams->iBTCount     = BRC_UPDATE_END - BRC_UPDATE_BEGIN;
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(BRCUPDATE_CURBE), (size_t)curbeAlignment);
            kernelParams->iBlockWidth  = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
            kernelParams->iBlockHeight = CODECHAL_HEVC_FRAME_BRC_BLOCK_SIZE;
            break;

        case CODECHAL_HEVC_BRC_LCU_UPDATE:
            kernelParams->iBTCount     = BRC_LCU_UPDATE_END - BRC_LCU_UPDATE_BEGIN;
            kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(BRCUPDATE_CURBE), (size_t)curbeAlignment);
            kernelParams->iBlockWidth  = CODECHAL_HEVC_LCU_BRC_BLOCK_SIZE;
            kernelParams->iBlockHeight = CODECHAL_HEVC_LCU_BRC_BLOCK_SIZE;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported BRC mode requested");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetBindingTable(
    EncOperation                           encOperation,
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC hevcEncBindingTable,
    uint32_t                               idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(hevcEncBindingTable);

    MOS_ZeroMemory(hevcEncBindingTable, sizeof(*hevcEncBindingTable));

    switch (encOperation)
    {
    case ENC_MBENC:
    {
        switch (idx)
        {
        case MBENC_LCU32_KRNIDX:
        case MBENC_LCU64_KRNIDX:
            hevcEncBindingTable->dwNumBindingTableEntries  = MBENC_B_FRAME_END - MBENC_B_FRAME_BEGIN;
            hevcEncBindingTable->dwBindingTableStartOffset = MBENC_B_FRAME_BEGIN;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported MBENC mode requested");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    break;

    case ENC_BRC:
    {
        switch (idx)
        {
        case CODECHAL_HEVC_BRC_INIT:
            hevcEncBindingTable->dwNumBindingTableEntries  = BRC_INIT_RESET_END - BRC_INIT_RESET_BEGIN;
            hevcEncBindingTable->dwBindingTableStartOffset = BRC_INIT_RESET_BEGIN;
            break;

        case CODECHAL_HEVC_BRC_RESET:
            hevcEncBindingTable->dwNumBindingTableEntries  = BRC_INIT_RESET_END - BRC_INIT_RESET_BEGIN;
            hevcEncBindingTable->dwBindingTableStartOffset = BRC_INIT_RESET_BEGIN;
            break;

        case CODECHAL_HEVC_BRC_FRAME_UPDATE:
            hevcEncBindingTable->dwNumBindingTableEntries  = BRC_UPDATE_END - BRC_UPDATE_BEGIN;
            hevcEncBindingTable->dwBindingTableStartOffset = BRC_UPDATE_BEGIN;
            break;

        case CODECHAL_HEVC_BRC_LCU_UPDATE:
            hevcEncBindingTable->dwNumBindingTableEntries  = BRC_LCU_UPDATE_END - BRC_LCU_UPDATE_BEGIN;
            hevcEncBindingTable->dwBindingTableStartOffset = BRC_LCU_UPDATE_BEGIN;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported BRC mode requested");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    break;

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < hevcEncBindingTable->dwNumBindingTableEntries; i++)
    {
        hevcEncBindingTable->dwBindingTableEntries[i] = i;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AllocateEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Surfaces used by I & B Kernels
    uint32_t width = 0, height = 0;
    uint32_t size = 0;

    if (!m_useMdf)
    {
        // Intermediate CU Record surface
        if (Mos_ResourceIsNull(&m_intermediateCuRecordSurfaceLcu32.OsResource))
        {
            width  = m_widthAlignedLcu32;
            height = m_heightAlignedLcu32 >> 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_intermediateCuRecordSurfaceLcu32,
                width,
                height,
                "Intermediate CU record surface",
                MOS_TILE_Y));
        }

        // Scratch Surface for I-kernel
        if (Mos_ResourceIsNull(&m_scratchSurface.OsResource))
        {
            width  = m_widthAlignedLcu32 >> 3;
            height = m_heightAlignedLcu32 >> 5;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_scratchSurface,
                width,
                height,
                "Scratch surface for I and B Kernels"));
        }

        // CU based QP surface
        if (Mos_ResourceIsNull(&m_16x16QpInputData.OsResource))
        {
            width  = MOS_ALIGN_CEIL(m_picWidthInMb, 64);
            height = MOS_ALIGN_CEIL(m_picHeightInMb, 64);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_16x16QpInputData,
                width,
                height,
                "16x16 QP Data Input surface"));
        }

        // Surfaces used by B Kernels
        // Enc constant table for B LCU32
        if (Mos_ResourceIsNull(&m_encConstantTableForB.sResource))
        {
            size = m_encConstantDataLutSize;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                &m_encConstantTableForB,
                size,
                "Enc Constant Table surface For LCU32/LCU64"));
        }

        //Debug surface
        for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_debugSurface); i++)
        {
            if (Mos_ResourceIsNull(&m_debugSurface[i].sResource))
            {
                size = m_debugSurfaceSize;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                    &m_debugSurface[i],
                    size,
                    "Kernel debug surface"));
            }
        }
    }

    // LCU Level Input Data
    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_lcuLevelInputDataSurface); i++)
    {
        if (Mos_ResourceIsNull(&m_lcuLevelInputDataSurface[i].OsResource))
        {
            width  = 16 * ((m_widthAlignedMaxLcu >> 6) << 1);
            height = ((m_heightAlignedMaxLcu >> 6) << 1);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_lcuLevelInputDataSurface[i],
                width,
                height,
                "Lcu Level Data Input surface"));
        }
    }

    m_brcInputForEncKernelBuffer = nullptr;

    //Current Picture Y with Reconstructed boundary pixels
    if (Mos_ResourceIsNull(&m_currPicWithReconBoundaryPix.OsResource))
    {
        width  = m_widthAlignedLcu32;
        height = m_heightAlignedLcu32;

        if (m_isMaxLcu64)
        {
            width  = m_widthAlignedMaxLcu;
            height = m_heightAlignedMaxLcu;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(
            &m_currPicWithReconBoundaryPix,
            width,
            height * m_alignReconFactor,
            "Current Picture Y with Reconstructed Boundary Pixels surface"));
    }

    // Encoder History Input Surface
    if (Mos_ResourceIsNull(&m_encoderHistoryInputBuffer.OsResource))
    {
        width  = 32 * ((m_widthAlignedMaxLcu >> 6) << 1);
        height = ((m_heightAlignedMaxLcu >> 6) << 1);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_encoderHistoryInputBuffer,
            width,
            height,
            "Encoder History Input surface"));
    }

    // Encoder History Output Surface
    if (Mos_ResourceIsNull(&m_encoderHistoryOutputBuffer.OsResource))
    {
        width  = 32 * ((m_widthAlignedMaxLcu >> 6) << 1);
        height = ((m_heightAlignedMaxLcu >> 6) << 1);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_encoderHistoryOutputBuffer,
            width,
            height,
            "Encoder History Output surface"));
    }

    if (m_hmeSupported && !m_useMdf)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->AllocateResources());
        // BRC Distortion surface
        if (Mos_ResourceIsNull(&m_brcBuffers.sMeBrcDistortionBuffer.OsResource))
        {
            width  = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 3), 64);
            height = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8) << 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_brcBuffers.sMeBrcDistortionBuffer,
                width,
                height,
                "Brc Distortion surface Buffer"));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateMeResources());
    }

    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_encBCombinedBuffer1); i++)
    {
        if (Mos_ResourceIsNull(&m_encBCombinedBuffer1[i].sResource))
        {
            size = sizeof(MBENC_COMBINED_BUFFER1);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                &m_encBCombinedBuffer1[i],
                size,
                "Enc B combined buffer1"));

            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_encBCombinedBuffer1[i].sResource,
                &lockFlags);
            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(data, size);

            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_encBCombinedBuffer1[i].sResource);
        }
    }

    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_encBCombinedBuffer2); i++)
    {
        if (Mos_ResourceIsNull(&m_encBCombinedBuffer2[i].sResource))
        {
            uint32_t               numLcu64 = m_widthAlignedMaxLcu * m_heightAlignedMaxLcu / 64 / 64;
            MBENC_COMBINED_BUFFER2 fixedBuf;

            m_historyOutBufferSize = MOS_ALIGN_CEIL(32 * numLcu64, CODECHAL_CACHELINE_SIZE);
            m_threadTaskBufferSize = MOS_ALIGN_CEIL(96 * numLcu64, CODECHAL_CACHELINE_SIZE);

            size = MOS_ALIGN_CEIL(sizeof(fixedBuf), CODECHAL_CACHELINE_SIZE) + m_historyOutBufferSize + m_threadTaskBufferSize;

            m_historyOutBufferOffset = MOS_ALIGN_CEIL(sizeof(fixedBuf), CODECHAL_CACHELINE_SIZE);
            m_threadTaskBufferOffset = m_historyOutBufferOffset + m_historyOutBufferSize;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
                &m_encBCombinedBuffer2[i],
                size,
                "Enc B combined buffer2"));

            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_encBCombinedBuffer2[i].sResource,
                &lockFlags);
            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(data, size);

            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_encBCombinedBuffer2[i].sResource);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::FreeEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_DeleteArray(m_mbEncKernelStates);
    m_mbEncKernelStates = nullptr;
    MOS_FreeMemory(m_mbEncKernelBindingTable);
    m_mbEncKernelBindingTable = nullptr;

    MOS_DeleteArray(m_brcKernelStates);
    m_brcKernelStates = nullptr;
    MOS_FreeMemory(m_brcKernelBindingTable);
    m_brcKernelBindingTable = nullptr;

    HmeParams hmeParams;
    MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
    hmeParams.presMvAndDistortionSumSurface = &m_mvAndDistortionSumSurface.sResource;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(DestroyMEResources(&hmeParams));

    // Surfaces used by I kernel
    // Release Intermediate CU Record Surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_intermediateCuRecordSurfaceLcu32.OsResource);

    // Release Scratch Surface for I-kernel
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_scratchSurface.OsResource);

    // Release CU based QP surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_16x16QpInputData.OsResource);

    // Release LCU Level Input Data
    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_lcuLevelInputDataSurface); i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_lcuLevelInputDataSurface[i].OsResource);
    }

    // Release Current Picture Y with Reconstructed boundary pixels surface
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_currPicWithReconBoundaryPix.OsResource);

    // Release Encoder History Input Data
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_encoderHistoryInputBuffer.OsResource);

    // Release Encoder History Output Data
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_encoderHistoryOutputBuffer.OsResource);

    // Release Debug surface
    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_debugSurface); i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_debugSurface[i].sResource);
    }

    // Surfaces used by B Kernels
    // Enc constant table for B LCU32
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_encConstantTableForB.sResource);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(FreeMeResources());

    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_encBCombinedBuffer1); i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encBCombinedBuffer1[i].sResource);
    }

    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_encBCombinedBuffer2); i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encBCombinedBuffer2[i].sResource);
    }

    if (m_swScoreboard)
    {
        MOS_FreeMemory(m_swScoreboard);
        m_swScoreboard = nullptr;
    }

    if (m_numDelay)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resDelayMinus);
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AllocateMeResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Mv and Distortion Summation Surface
    if (Mos_ResourceIsNull(&m_mvAndDistortionSumSurface.sResource))
    {
        uint32_t size = m_mvdistSummationSurfSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_mvAndDistortionSumSurface,
            size,
            "Mv and Distortion Summation surface"));

        // Initialize the surface to zero for now till HME is updated to output the data into this surface
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_mvAndDistortionSumSurface.sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, size);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_mvAndDistortionSumSurface.sResource);
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::FreeMeResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_brcBuffers.sMeBrcDistortionBuffer.OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AllocatePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t mvt_size        = MOS_ALIGN_CEIL(((m_frameWidth + 63) >> 6) * ((m_frameHeight + 15) >> 4), 2) * CODECHAL_CACHELINE_SIZE;
    uint32_t mvtb_size       = MOS_ALIGN_CEIL(((m_frameWidth + 31) >> 5) * ((m_frameHeight + 31) >> 5), 2) * CODECHAL_CACHELINE_SIZE;
    m_sizeOfMvTemporalBuffer = MOS_MAX(mvt_size, mvtb_size);

    const uint32_t minLcuSize        = 16;
    const uint32_t picWidthInMinLCU  = MOS_ROUNDUP_DIVIDE(m_frameWidth, minLcuSize);   //assume smallest LCU to get max width
    const uint32_t picHeightInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameHeight, minLcuSize);  //assume smallest LCU to get max height

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth  = m_bitDepth;
    hcpBufSizeParam.ucChromaFormat = m_chromaFormat;
    // We should move the buffer allocation to picture level if the size is dependent on LCU size
    hcpBufSizeParam.dwCtbLog2SizeY = 6;  //assume Max LCU size
    hcpBufSizeParam.dwPicWidth     = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE);
    hcpBufSizeParam.dwPicHeight    = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Deblocking Filter Row Store Scratch data surface
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Row Store Scratch Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "DeblockingScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        return eStatus;
    }

    // Deblocking Filter Tile Row Store Scratch data surface
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Tile Row Store Scratch Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "DeblockingTileRowScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterTileRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Tile Row Store Scratch Buffer.");
        return eStatus;
    }

    // Deblocking Filter Column Row Store Scratch data surface
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Tile Column Store Scratch Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "DeblockingColumnScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterColumnRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Tile Column Row Store Scratch Buffer.");
        return eStatus;
    }

    // Metadata Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Metadata Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "MetadataLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Line Buffer.");
        return eStatus;
    }

    // Metadata Tile Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Metadata Tile Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "MetadataTileLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataTileLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Tile Line Buffer.");
        return eStatus;
    }

    // Metadata Tile Column buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Metadata Tile Column Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "MetadataTileColumnBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataTileColumnBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Tile Column Buffer.");
        return eStatus;
    }

    // SAO Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Line Buffer.");
        return eStatus;
    }

    // SAO Tile Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Tile Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoTileLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoTileLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Tile Line Buffer.");
        return eStatus;
    }

    // SAO Tile Column buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Tile Column Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes  = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoTileColumnBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoTileColumnBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Tile Column Buffer.");
        return eStatus;
    }

    // Lcu ILDB StreamOut buffer
    // Allocate the buffer size
    // This is not enabled with HCP_PIPE_MODE_SELECT yet, placeholder here
    allocParamsForBufferLinear.dwBytes  = CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "LcuILDBStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resLcuIldbStreamOutBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate LCU ILDB StreamOut Buffer.");
        return eStatus;
    }

    // Lcu Base Address buffer
    // HEVC Encoder Mode: Slice size is written to this buffer when slice size conformance is enabled.
    // 1 CL (= 16 DWs = 64 bytes) per slice * Maximum number of slices in a frame.
    // Align to page for HUC requirement
    uint32_t maxLcu                     = picWidthInMinLCU * picHeightInMinLCU;
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(maxLcu * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "LcuBaseAddressBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resLcuBaseAddressBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate LCU Base Address Buffer.");
        return eStatus;
    }

    // SAO StreamOut buffer
    // size = MOS_ALIGN_CEIL(picWidthInMinLCU, 4) * 16
    uint32_t size = MOS_ALIGN_CEIL(picWidthInMinLCU, 4) * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU;
    //extra added size to cover tile enabled case, per tile width aligned to 4.  20: max tile column No.
    size += 3 * 20 * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "SaoStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoStreamOutBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO StreamOut Buffer.");
        return eStatus;
    }

    uint32_t maxTileNumber = (MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE) / CODECHAL_HEVC_MIN_TILE_SIZE) *
                             (MOS_ALIGN_CEIL(m_frameHeight, CODECHAL_HEVC_MIN_TILE_SIZE) / CODECHAL_HEVC_MIN_TILE_SIZE);

    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Allocate Frame Statistics Streamout Data Destination Buffer. DW98-100 in HCP pipe buffer address command
    allocParamsForBufferLinear.dwBytes  = m_sizeOfHcpPakFrameStats * maxTileNumber;  //Each tile has 8 cache size bytes of data
    allocParamsForBufferLinear.pBufName = "FrameStatStreamOutBuffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resFrameStatStreamOutBuffer));

    // PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
    // One CU has 16-byte. But, each tile needs to be aliged to the cache line
    uint32_t frameWidthInCus            = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, CODECHAL_HEVC_MIN_CU_SIZE);
    uint32_t frameHeightInCus           = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameHeight, CODECHAL_HEVC_MIN_CU_SIZE);
    size                                = MOS_ALIGN_CEIL(frameWidthInCus * frameHeightInCus * 16, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";

    CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resPakcuLevelStreamoutData.sResource));
    m_resPakcuLevelStreamoutData.dwSize = size;
    CODECHAL_ENCODE_VERBOSEMESSAGE("first allocate cu steam out buffer, size=0x%x.\n", size);

    // Allocate SSE Source Pixel Row Store Buffer. Implementation for each tile column is shown as below:
    //   tileWidthInLCU = ((tileWidthInLCU+3) * BYTES_PER_CACHE_LINE)*(4+4) ; tileWidthInLCU <<= 1; // double the size as RTL treats it as 10 bit data
    // Here, we consider each LCU column is one tile column.

    m_sizeOfSseSrcPixelRowStoreBufferPerLcu = (CODECHAL_CACHELINE_SIZE * (4 + 4)) << 1;                          //size per LCU plus 10-bit
    size                                    = m_sizeOfSseSrcPixelRowStoreBufferPerLcu * (picWidthInMinLCU + 3);  // already aligned to cacheline size
    allocParamsForBufferLinear.dwBytes      = size;
    allocParamsForBufferLinear.pBufName     = "SseSrcPixelRowStoreBuffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSseSrcPixelRowStoreBuffer));

    // SAO Row Store buffer, HSAO
    // Aligned to 4 for each tile column
    uint32_t maxTileColumn              = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE);
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(picWidthInMinLCU + 3 * maxTileColumn, 4) * 16;
    allocParamsForBufferLinear.pBufName = "SaoRowStoreBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_SAORowStoreBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO row store Buffer.");
        return eStatus;
    }

    //HCP scalability Sync buffer
    size                                = CODECHAL_HEVC_MAX_NUM_HCP_PIPE * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "GEN12 Hcp scalability Sync buffer ";

    CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resHcpScalabilitySyncBuffer.sResource));
    m_resHcpScalabilitySyncBuffer.dwSize = size;

    // create the tile coding state parameters
    m_tileParams = (PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12)MOS_AllocAndZeroMemory(sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12) * maxTileNumber);

    if (m_enableHWSemaphore)
    {
        // Create the HW sync objects which will be used by each reference frame and BRC in GEN12
        allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
        allocParamsForBufferLinear.pBufName = "SemaphoreMemory";

        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resBrcSemaphoreMem); i++)
        {
            eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resBrcSemaphoreMem[i].sResource);
            m_resBrcSemaphoreMem[i].dwSize = allocParamsForBufferLinear.dwBytes;
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Cannot create BRC HW Semaphore Memory.");

            uint32_t *data = (uint32_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_resBrcSemaphoreMem[i].sResource,
                &lockFlagsWriteOnly);

            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            *data = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_resBrcSemaphoreMem[i].sResource));
        }

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPipeStartSemaMem);
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Cannot create Scalability pipe start sync HW semaphore.");

        uint32_t *data = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resPipeStartSemaMem,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        *data = 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resPipeStartSemaMem));

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPipeCompleteSemaMem);
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Cannot create Scalability pipe completion sync HW semaphore.");

        data = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resPipeCompleteSemaMem,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        *data = 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resPipeCompleteSemaMem));
    }

    if (m_hucPakStitchEnabled)
    {
        if (Mos_ResourceIsNull(&m_resHucStatus2Buffer))
        {
            // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
            allocParamsForBufferLinear.dwBytes  = sizeof(uint64_t);
            allocParamsForBufferLinear.pBufName = "HUC STATUS 2 Buffer";

            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resHucStatus2Buffer),
                "%s: Failed to allocate HUC STATUS 2 Buffer\n",
                __FUNCTION__);
        }

        uint8_t *data;

        // Pak stitch DMEM
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemEncG12), CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "PAK Stitch Dmem Buffer";
        auto numOfPasses                    = CODECHAL_DP_MAX_NUM_BRC_PASSES;
        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            for (auto i = 0; i < numOfPasses; i++)
            {
                eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resHucPakStitchDmemBuffer[j][i]);

                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate PAK Stitch Dmem Buffer.");
                    return eStatus;
                }
            }
        }
        // BRC Data Buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "BRC Data Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resBrcDataBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate BRC Data Buffer Buffer.");
            return eStatus;
        }

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resBrcDataBuffer,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(
            data,
            allocParamsForBufferLinear.dwBytes);

        m_osInterface->pfnUnlockResource(m_osInterface, &m_resBrcDataBuffer);
        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            for (auto j = 0; j < CODECHAL_HEVC_MAX_NUM_BRC_PASSES; j++)
            {
                // HuC stitching Data buffer
                allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucCommandData), CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName = "HEVC HuC Stitch Data Buffer";
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_resHucStitchDataBuffer[i][j]));

                MOS_LOCK_PARAMS lockFlagsWriteOnly;
                MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
                lockFlagsWriteOnly.WriteOnly = 1;

                uint8_t *pData = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_resHucStitchDataBuffer[i][j],
                    &lockFlagsWriteOnly);
                CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
                MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
                m_osInterface->pfnUnlockResource(m_osInterface, &m_resHucStitchDataBuffer[i][j]);
            }
        }

        //Second level BB for huc stitching cmd
        MOS_ZeroMemory(&m_HucStitchCmdBatchBuffer, sizeof(m_HucStitchCmdBatchBuffer));
        m_HucStitchCmdBatchBuffer.bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_HucStitchCmdBatchBuffer,
            nullptr,
            m_hwInterface->m_HucStitchCmdBatchBufferSize));
    }

    // Pak obj and CU records for skip frame
    uint32_t mbCodeSize = m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE;  // Must reserve at least 8 cachelines after MI_BATCH_BUFFER_END_CMD since HW prefetch max 8 cachelines from BB everytime

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.Format   = Format_Buffer;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.dwBytes  = mbCodeSize;
    allocParams.pBufName = "skipFrameMbCodeSurface";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &m_skipFrameInfo.m_resMbCodeSkipFrameSurface);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate PAK object buffer for skip frame");
        return eStatus;
    }

    if (m_numDelay)
    {
        allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
        allocParamsForBufferLinear.pBufName = "DelayMinusMemory";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &m_resDelayMinus),
            "Failed to allocate delay minus memory.");

        uint8_t *       data;
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        data                = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resDelayMinus,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, sizeof(uint32_t));

        m_osInterface->pfnUnlockResource(m_osInterface, &m_resDelayMinus);
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::FreePakResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Release Frame Statistics Streamout Data Destination Buffer
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resFrameStatStreamOutBuffer);

    // PAK CU Level Stream out buffer
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resPakcuLevelStreamoutData.sResource);

    // Release SSE Source Pixel Row Store Buffer
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resSseSrcPixelRowStoreBuffer);

    // Release Hcp scalability Sync buffer
    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resHcpScalabilitySyncBuffer.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resPakcuLevelStreamoutData.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resPakSliceLevelStreamoutData.sResource);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_SAORowStoreBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_skipFrameInfo.m_resMbCodeSkipFrameSurface);

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resTileBasedStatisticsBuffer); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resTileBasedStatisticsBuffer[i].sResource);
    }
    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_tileRecordBuffer); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_tileRecordBuffer[i].sResource);
    }
    m_osInterface->pfnFreeResource(m_osInterface, &m_resHuCPakAggregatedFrameStatsBuffer.sResource);

    MOS_FreeMemory(m_tileParams);

    if (m_useVirtualEngine)
    {
        for (uint32_t i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; i++)
        {
            for (uint32_t j = 0; j < CODECHAL_HEVC_MAX_NUM_HCP_PIPE; j++)
            {
                for (auto k = 0; k < CODECHAL_HEVC_MAX_NUM_BRC_PASSES; k++)
                {
                    PMOS_COMMAND_BUFFER cmdBuffer = &m_veBatchBuffer[i][j][k];
                    if (cmdBuffer->pCmdBase)
                    {
                        m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
                    }
                    m_osInterface->pfnFreeResource(m_osInterface, &cmdBuffer->OsResource);
                }
            }
        }
    }

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_refSync); i++)
    {
        auto sync = &m_refSync[i];

        if (!Mos_ResourceIsNull(&sync->resSyncObject))
        {
            // if this object has been signaled before, we need to wait to ensure singal-wait is in pair.
            if (sync->uiSemaphoreObjCount || sync->bInUsed)
            {
                MOS_SYNC_PARAMS syncParams  = g_cInitSyncParams;
                syncParams.GpuContext       = m_renderContext;
                syncParams.presSyncResource = &sync->resSyncObject;
                syncParams.uiSemaphoreCount = sync->uiSemaphoreObjCount;
                m_osInterface->pfnEngineWait(m_osInterface, &syncParams);
            }
        }
        m_osInterface->pfnFreeResource(m_osInterface, &sync->resSemaphoreMem.sResource);
    }

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resBrcSemaphoreMem); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resBrcSemaphoreMem[i].sResource);
    }
    m_osInterface->pfnFreeResource(m_osInterface, &m_resPipeStartSemaMem);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resPipeCompleteSemaMem);

    if (m_hucPakStitchEnabled)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resHucStatus2Buffer);
        m_osInterface->pfnFreeResource(m_osInterface, &m_resBrcDataBuffer);

        for (int i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            for (int j = 0; j < CODECHAL_HEVC_MAX_NUM_BRC_PASSES; j++)
            {
                m_osInterface->pfnFreeResource(m_osInterface, &m_resHucPakStitchDmemBuffer[i][j]);
                m_osInterface->pfnFreeResource(m_osInterface, &m_resHucStitchDataBuffer[i][j]);
            }
        }
        Mhw_FreeBb(m_osInterface, &m_HucStitchCmdBatchBuffer, nullptr);
    }
    return CodechalEncHevcState::FreePakResources();
}

MOS_STATUS CodechalEncHevcStateG12::GetKernelHeaderAndSize(
    void *       binary,
    EncOperation operation,
    uint32_t     krnStateIdx,
    void *       krnHeader,
    uint32_t *   krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    PCODECHAL_HEVC_KERNEL_HEADER kernelHeaderTable = (PCODECHAL_HEVC_KERNEL_HEADER)binary;

    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;
    switch (operation)
    {
    case ENC_MBENC:
    {
        switch (krnStateIdx)
        {
        case MBENC_LCU32_KRNIDX:
            currKrnHeader = &kernelHeaderTable->HEVC_Enc_LCU32;
            break;

        case MBENC_LCU64_KRNIDX:
            currKrnHeader = &kernelHeaderTable->HEVC_Enc_LCU64;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported MBENC mode requested");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    break;

    case ENC_BRC:
    {
        switch (krnStateIdx)
        {
        case CODECHAL_HEVC_BRC_INIT:
            currKrnHeader = &kernelHeaderTable->HEVC_brc_init;
            break;

        case CODECHAL_HEVC_BRC_RESET:
            currKrnHeader = &kernelHeaderTable->HEVC_brc_reset;
            break;

        case CODECHAL_HEVC_BRC_FRAME_UPDATE:
            currKrnHeader = &kernelHeaderTable->HEVC_brc_update;
            break;

        case CODECHAL_HEVC_BRC_LCU_UPDATE:
            currKrnHeader = &kernelHeaderTable->HEVC_brc_lcuqp;
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported BRC mode requested, krnStateIdx=%d", krnStateIdx);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        break;
    }

    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    PCODECHAL_KERNEL_HEADER invalidEntry  = &(kernelHeaderTable->HEVC_brc_lcuqp) + 1;
    uint32_t                nextKrnOffset = *krnSize;
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    m_numMbEncEncKrnStates                       = MBENC_NUM_KRN;

    m_mbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);

    m_mbEncKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) * m_numMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelBindingTable);

    PMHW_KERNEL_STATE kernelStatePtr = m_mbEncKernelStates;

    for (uint32_t krnStateIdx = 0; krnStateIdx < m_numMbEncEncKrnStates; krnStateIdx++)
    {
        auto                   kernelSize = m_combinedKernelSize;
        CODECHAL_KERNEL_HEADER currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_MBENC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
            ENC_MBENC,
            &kernelStatePtr->KernelParams,
            krnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
            ENC_MBENC,
            &m_mbEncKernelBindingTable[krnStateIdx],
            krnStateIdx));

        kernelStatePtr->dwCurbeOffset = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary =
            m_kernelBinary +
            (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize   = kernelSize;
        kernelStatePtr->dwCurbeOffset        = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize   = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    m_numBrcKrnStates                            = CODECHAL_HEVC_BRC_NUM;

    m_brcKernelStates = MOS_NewArray(MHW_KERNEL_STATE, m_numBrcKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    m_brcKernelBindingTable = (PCODECHAL_ENCODE_BINDING_TABLE_GENERIC)MOS_AllocAndZeroMemory(
        sizeof(GenericBindingTable) * m_numBrcKrnStates);

    PMHW_KERNEL_STATE kernelStatePtr = m_brcKernelStates;

    kernelStatePtr++;  // Skipping BRC_COARSE_INTRA as it not in Gen11

    // KrnStateIdx initialization starts at 1 as Gen11 does not support BRC_COARSE_INTRA kernel in BRC. It is part of the Combined Common Kernel
    for (uint32_t krnStateIdx = 1; krnStateIdx < m_numBrcKrnStates; krnStateIdx++)
    {
        auto                   kernelSize = m_combinedKernelSize;
        CODECHAL_KERNEL_HEADER currKrnHeader;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_BRC,
            krnStateIdx,
            &currKrnHeader,
            (uint32_t *)&kernelSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
            ENC_BRC,
            &kernelStatePtr->KernelParams,
            krnStateIdx));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
            ENC_BRC,
            &m_brcKernelBindingTable[krnStateIdx],
            krnStateIdx));

        kernelStatePtr->dwCurbeOffset        = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize   = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::GetFrameBrcLevel()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    //if L0/L1 both points to previous frame, then its LBD otherwise its is level 1 RA B.
    auto                               B_or_LDB_brclevel = m_lowDelay ? HEVC_BRC_FRAME_TYPE_P_OR_LB : HEVC_BRC_FRAME_TYPE_B;
    std::map<int, HEVC_BRC_FRAME_TYPE> codingtype_to_brclevel{
        {I_TYPE, HEVC_BRC_FRAME_TYPE_I},
        {P_TYPE, HEVC_BRC_FRAME_TYPE_P_OR_LB},
        {B_TYPE, B_or_LDB_brclevel},
        {B1_TYPE, HEVC_BRC_FRAME_TYPE_B1},
        {B2_TYPE, HEVC_BRC_FRAME_TYPE_B2}};

    //Both I or P/LDB type at same HierarchLevelPlus1
    auto                               intra_LDBFrame_to_Brclevel = (m_pictureCodingType == I_TYPE) ? HEVC_BRC_FRAME_TYPE_I : HEVC_BRC_FRAME_TYPE_P_OR_LB;
    std::map<int, HEVC_BRC_FRAME_TYPE> hierchLevelPlus1_to_brclevel{
        {1, intra_LDBFrame_to_Brclevel},
        {2, HEVC_BRC_FRAME_TYPE_B},
        {3, HEVC_BRC_FRAME_TYPE_B1},
        {4, HEVC_BRC_FRAME_TYPE_B2}};

    if (m_hevcSeqParams->HierarchicalFlag && m_hevcSeqParams->GopRefDist > 1 && m_hevcSeqParams->GopRefDist <= 8)
    {
        if (m_hevcPicParams->HierarchLevelPlus1 > 0)  // LDB or RAB
        {
            m_currFrameBrcLevel = hierchLevelPlus1_to_brclevel.count(m_hevcPicParams->HierarchLevelPlus1) ? hierchLevelPlus1_to_brclevel[m_hevcPicParams->HierarchLevelPlus1] : HEVC_BRC_FRAME_TYPE_INVALID;
            //Invalid HierarchLevelPlus1 or LBD frames at level 3 eror check.
            if ((m_currFrameBrcLevel == HEVC_BRC_FRAME_TYPE_INVALID) ||
                (m_hevcSeqParams->LowDelayMode && m_currFrameBrcLevel == HEVC_BRC_FRAME_TYPE_B2))
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("HEVC_BRC_FRAME_TYPE_INVALID or LBD picture doesn't support Level 4\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            if (!m_hevcSeqParams->LowDelayMode)  // RA B
            {
                m_currFrameBrcLevel = codingtype_to_brclevel.count(m_pictureCodingType) ? codingtype_to_brclevel[m_pictureCodingType] : HEVC_BRC_FRAME_TYPE_INVALID;
                //Invalid CodingType.
                if (m_currFrameBrcLevel == HEVC_BRC_FRAME_TYPE_INVALID)
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Invalid CodingType\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else  // Low Delay mode: Flat case
            {
                m_currFrameBrcLevel = (m_pictureCodingType == I_TYPE) ? HEVC_BRC_FRAME_TYPE_I : HEVC_BRC_FRAME_TYPE_P_OR_LB;
            }
        }
    }
    else  // Flat B
    {
        m_currFrameBrcLevel = (m_pictureCodingType == I_TYPE) ? HEVC_BRC_FRAME_TYPE_I : B_or_LDB_brclevel;
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t CodechalEncHevcStateG12::GetMaxBtCount()
{
    uint16_t btIdxAlignment = m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    // BRC Init kernel
    uint32_t btCountPhase1 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_INIT].KernelParams.iBTCount, btIdxAlignment);

    // SwScoreboard kernel
    uint32_t btCountPhase2 = MOS_ALIGN_CEIL(m_swScoreboardState->GetBTCount(), btIdxAlignment);

    // Csc+Ds+Conversion kernel
    btCountPhase2 += MOS_ALIGN_CEIL(m_cscDsState->GetBTCount(), btIdxAlignment);

    // Intra Distortion kernel
    if (m_intraDistKernel)
    {
        btCountPhase2 += MOS_ALIGN_CEIL(m_intraDistKernel->GetBTCount(), btIdxAlignment);
    }

    // HME 4x, 16x, 32x kernel
    if (m_hmeKernel)
    {
        btCountPhase2 += (MOS_ALIGN_CEIL(m_hmeKernel->GetBTCount(), btIdxAlignment) * 3);
    }

    // Weighted prediction kernel
    btCountPhase2 += MOS_ALIGN_CEIL(m_wpState->GetBTCount(), btIdxAlignment);
    uint32_t btCountPhase3 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[MBENC_LCU32_KRNIDX].KernelParams.iBTCount, btIdxAlignment);

    uint32_t btCountPhase4 = MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE].KernelParams.iBTCount, btIdxAlignment) +
                             MOS_ALIGN_CEIL(m_mbEncKernelStates[MBENC_LCU64_KRNIDX].KernelParams.iBTCount, btIdxAlignment);

    uint32_t maxBtCount = MOS_MAX(btCountPhase1, btCountPhase2);
    maxBtCount          = MOS_MAX(maxBtCount, btCountPhase3);
    maxBtCount          = MOS_MAX(maxBtCount, btCountPhase4);

    return maxBtCount;
}

MOS_STATUS CodechalEncHevcStateG12::CalcScaledDimensions()
{
    return MOS_STATUS_SUCCESS;
}

void CodechalEncHevcStateG12::GetMaxRefFrames(uint8_t &maxNumRef0, uint8_t &maxNumRef1)
{
    maxNumRef0 = m_maxNumVmeL0Ref;
    maxNumRef1 = m_maxNumVmeL1Ref;

    return;
}

MOS_STATUS CodechalEncHevcStateG12::GetStatusReport(
    EncodeStatus *      encodeStatus,
    EncodeStatusReport *encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport);

    if (encodeStatusReport->UsedVdBoxNumber <= 1)
    {
        return CodechalEncodeHevcBase::GetStatusReport(encodeStatus, encodeStatusReport);
    }

    PCODECHAL_ENCODE_BUFFER tileSizeStatusReport = &m_tileRecordBuffer[encodeStatusReport->CurrOriginalPic.FrameIdx];

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    HCPPakHWTileSizeRecord_G12 *tileStatusReport = (HCPPakHWTileSizeRecord_G12 *)m_osInterface->pfnLockResource(
        m_osInterface,
        &tileSizeStatusReport->sResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileStatusReport);

    encodeStatusReport->CodecStatus                                      = CODECHAL_STATUS_SUCCESSFUL;
    encodeStatusReport->PanicMode                                        = false;
    encodeStatusReport->AverageQp                                        = 0;
    encodeStatusReport->QpY                                              = 0;
    encodeStatusReport->SuggestedQpYDelta                                = 0;
    encodeStatusReport->NumberPasses                                     = 1;
    encodeStatusReport->bitstreamSize                                    = 0;
    encodeStatus->ImageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQp = 0;

    uint32_t totalCU = 0;
    double   sumQp   = 0.0;
    for (uint32_t i = 0; i < encodeStatusReport->NumberTilesInFrame; i++)
    {
        if (tileStatusReport[i].Length == 0)
        {
            encodeStatusReport->CodecStatus = CODECHAL_STATUS_INCOMPLETE;
            return eStatus;
        }

        encodeStatusReport->bitstreamSize += tileStatusReport[i].Length;
        totalCU += (m_tileParams[i].TileHeightInMinCbMinus1 + 1) * (m_tileParams[i].TileWidthInMinCbMinus1 + 1);
        sumQp += tileStatusReport[i].Hcp_Qp_Status_Count;
    }

    encodeStatusReport->NumberPasses = (uint8_t)encodeStatus->dwNumberPasses + 1;
    CODECHAL_ENCODE_VERBOSEMESSAGE("BRC Scalability Mode Exectued PAK Pass number: %d.\n", encodeStatusReport->NumberPasses);

    if (encodeStatusReport->bitstreamSize == 0 ||
        encodeStatusReport->bitstreamSize > m_bitstreamUpperBound)
    {
        encodeStatusReport->CodecStatus   = CODECHAL_STATUS_ERROR;
        encodeStatusReport->bitstreamSize = 0;
        CODECHAL_ENCODE_ASSERTMESSAGE("Bit-stream size exceeds upper bound!");
        return MOS_STATUS_INVALID_FILE_SIZE;
    }

    if (m_sseEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CalculatePSNR(encodeStatus, encodeStatusReport));
    }

    encodeStatusReport->QpY = encodeStatusReport->AverageQp =
        (uint8_t)((sumQp / (double)totalCU) / 4.0);  // due to TU is 4x4 and there are 4 TUs in one CU

    if (m_enableTileStitchByHW)
    {
        return eStatus;
    }

    uint8_t *tempBsBuffer = nullptr, *bufPtr = nullptr;
    tempBsBuffer = bufPtr = (uint8_t *)MOS_AllocAndZeroMemory(encodeStatusReport->bitstreamSize);
    CODECHAL_ENCODE_CHK_NULL_RETURN(tempBsBuffer);

    CODEC_REF_LIST currRefList = *(encodeStatus->encodeStatusReport.pCurrRefList);
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    uint8_t *bitstream = (uint8_t *)m_osInterface->pfnLockResource(
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
        uint32_t len    = tileStatusReport[i].Length;

        MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
        bufPtr += len;
    }

    MOS_SecureMemcpy(bitstream, encodeStatusReport->bitstreamSize, tempBsBuffer, encodeStatusReport->bitstreamSize);
    MOS_ZeroMemory(&bitstream[encodeStatusReport->bitstreamSize],
        m_bitstreamUpperBound - encodeStatusReport->bitstreamSize);

    if (tempBsBuffer)
    {
        MOS_FreeMemory(tempBsBuffer);
    }

    if (m_osInterface && bitstream)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &currRefList.resBitstreamBuffer);
    }

    if (m_osInterface && tileStatusReport)
    {
        // clean-up the tile status report buffer
        MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * encodeStatusReport->NumberTilesInFrame);

        m_osInterface->pfnUnlockResource(m_osInterface, &tileSizeStatusReport->sResource);
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AllocateResourcesVariableSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        return eStatus;
    }

    uint32_t bufSize = 0;
    if (m_pakPiplStrmOutEnable)
    {
        // PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
        // One CU has 16-byte. But, each tile needs to be aliged to the cache line
        uint32_t tileWidthInCus  = 0;
        uint32_t tileHeightInCus = 0;
        uint32_t numTileColumns  = m_hevcPicParams->num_tile_columns_minus1 + 1;
        uint32_t numTileRows     = m_hevcPicParams->num_tile_rows_minus1 + 1;
        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                uint32_t idx = tileRow * numTileColumns + tileCol;

                tileHeightInCus = m_tileParams[idx].TileHeightInMinCbMinus1 + 1;
                tileWidthInCus  = m_tileParams[idx].TileWidthInMinCbMinus1 + 1;
                bufSize += (tileWidthInCus * tileHeightInCus * 16);
                bufSize = MOS_ALIGN_CEIL(bufSize, CODECHAL_CACHELINE_SIZE);
            }
        }
        if (Mos_ResourceIsNull(&m_resPakcuLevelStreamoutData.sResource) ||
            (bufSize > m_resPakcuLevelStreamoutData.dwSize))
        {
            if (!Mos_ResourceIsNull(&m_resPakcuLevelStreamoutData.sResource))
            {
                m_osInterface->pfnFreeResource(m_osInterface, &m_resPakcuLevelStreamoutData.sResource);
            }

            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = bufSize;
            allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";

            CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resPakcuLevelStreamoutData.sResource));
            m_resPakcuLevelStreamoutData.dwSize = bufSize;
            CODECHAL_ENCODE_VERBOSEMESSAGE("reallocate cu steam out buffer, size=0x%x.\n", bufSize);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_firstTaskInPhase = m_singleTaskPhaseSupported ? IsFirstPass() : true;
    m_lastTaskInPhase  = m_singleTaskPhaseSupported ? IsLastPass() : true;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifyCommandBufferSize());

    if (!m_singleTaskPhaseSupportedInPak)
    {
        // Command buffer or patch list size are too small and so we cannot submit multiple pass of PAKs together
        m_firstTaskInPhase = true;
        m_lastTaskInPhase  = true;
    }

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported) || m_firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool bRequestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, bRequestFrameTracking));
    }

    // clean-up per VDBOX semaphore memory
    int32_t currentPipe = GetCurrentPipe();
    if (currentPipe < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_numPipe >= 2 &&
        ((m_singleTaskPhaseSupported && IsFirstPass()) ||
            !m_singleTaskPhaseSupported))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));
        //HW Semaphore cmd to make sure all pipes start encode at the same time
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resPipeStartSemaMem, 1, MHW_MI_ATOMIC_INC, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(
            &m_resPipeStartSemaMem,
            &cmdBuffer,
            m_numPipe));

        // Program some placeholder cmds to resolve the hazard between BEs sync
        MHW_MI_STORE_DATA_PARAMS dataParams;
        dataParams.pOsResource      = &m_resDelayMinus;
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue          = 0xDE1A;
        for (uint32_t i = 0; i < m_numDelay; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                &cmdBuffer,
                &dataParams));
        }

        //clean HW semaphore memory
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resPipeStartSemaMem, 1, MHW_MI_ATOMIC_DEC, &cmdBuffer));

        //Start Watchdog Timer
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStartCmd(&cmdBuffer));

        //To help test media reset, this hw semaphore wait will never be reached.
        if (m_enableTestMediaReset)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(
                &m_resPipeStartSemaMem,
                &cmdBuffer,
                m_numPipe + 2));
        }
    }

    if (m_brcEnabled && !IsFirstPass())  // Only the regular BRC passes have the conditional batch buffer end
    {
        // Ensure the previous PAK BRC pass is done, mainly for pipes other than pipe0.
        if (m_singleTaskPhaseSupported && m_numPipe >= 2 &&
            !Mos_ResourceIsNull(&m_resBrcSemaphoreMem[currentPipe].sResource))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                SendHWWaitCommand(
                    &m_resBrcSemaphoreMem[currentPipe].sResource,
                    &cmdBuffer,
                    1));
        }

        // Insert conditional batch buffer end
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));
        uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                              sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource       ;

        if (m_hucPakStitchEnabled && m_numPipe >= 2)  //BRC scalability
        {
            CODECHAL_ENCODE_ASSERT((m_encodeStatusBuf.dwHuCStatusMaskOffset & 7) == 0);  // Make sure uint64_t aligned
            CODECHAL_ENCODE_ASSERT((m_encodeStatusBuf.dwHuCStatusMaskOffset + sizeof(uint32_t)) == m_encodeStatusBuf.dwHuCStatusRegOffset);

            miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miConditionalBatchBufferEndParams.dwOffset            = baseOffset + m_encodeStatusBuf.dwHuCStatusMaskOffset;
        }
        else
        {
            CODECHAL_ENCODE_ASSERT((m_encodeStatusBuf.dwImageStatusMaskOffset & 7) == 0);  // Make sure uint64_t aligned
            CODECHAL_ENCODE_ASSERT((m_encodeStatusBuf.dwImageStatusMaskOffset + sizeof(uint32_t)) == m_encodeStatusBuf.dwImageStatusCtrlOffset);

            miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miConditionalBatchBufferEndParams.dwOffset            = baseOffset + m_encodeStatusBuf.dwImageStatusMaskOffset;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));

        auto                             mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MHW_MI_COPY_MEM_MEM_PARAMS       miCpyMemMemParams;
        if (m_hucPakStitchEnabled && m_numPipe >= 2)
        {
            // Write back the HCP image control register with HUC PAK Int Kernel output
            MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
            MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));
            miLoadRegMemParams.presStoreBuffer = &m_resBrcDataBuffer;
            miLoadRegMemParams.dwOffset        = CODECHAL_OFFSETOF(PakIntegrationBrcData, HCP_ImageStatusControl);
            miLoadRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&cmdBuffer, &miLoadRegMemParams));

            if (IsFirstPipe())
            {
                MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
                miCpyMemMemParams.presSrc     = &m_resBrcDataBuffer;
                miCpyMemMemParams.dwSrcOffset = CODECHAL_OFFSETOF(PakIntegrationBrcData, HCP_ImageStatusControl);
                miCpyMemMemParams.presDst     = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
                miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(&cmdBuffer, &miCpyMemMemParams));

                MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
                miStoreRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
                miStoreRegMemParams.dwOffset        = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOfLastBRCPassOffset;
                miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));
            }
        }
        else
        {
            // Write back the HCP image control register for RC6 may clean it out
            MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
            MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));
            miLoadRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miLoadRegMemParams.dwOffset        = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOffset;
            miLoadRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&cmdBuffer, &miLoadRegMemParams));

            MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
            miStoreRegMemParams.presStoreBuffer = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
            miStoreRegMemParams.dwOffset        = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
            miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));

            MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
            miStoreRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miStoreRegMemParams.dwOffset        = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOfLastBRCPassOffset;
            miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));
        }
    }

    if (IsFirstPipe() && IsFirstPass() && m_osInterface->bTagResourceSync)
    {
        // This is a short term solution to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.

        MOS_RESOURCE globalGpuContextSyncTagBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource      = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        uint32_t value          = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue          = (value > 0) ? (value - 1) : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    if (IsFirstPipe())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
    }

    if (m_numPipe >= 2)
    {
        // clean up hw semaphore for BRC PAK pass sync, used only in single task phase.
        if (m_singleTaskPhaseSupported &&
            m_brcEnabled &&
            !Mos_ResourceIsNull(&m_resBrcSemaphoreMem[currentPipe].sResource))
        {
            MHW_MI_STORE_DATA_PARAMS storeDataParams;
            MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
            storeDataParams.pOsResource      = &m_resBrcSemaphoreMem[currentPipe].sResource;
            storeDataParams.dwResourceOffset = 0;
            storeDataParams.dwValue          = 0;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                &cmdBuffer,
                &storeDataParams));
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPipeModeSelectCmd(&cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpSurfaceStateCmds(&cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPipeBufAddrCmd(&cmdBuffer));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    MHW_VDBOX_QM_PARAMS fqmParams, qmParams;
    SetHcpQmStateParams(fqmParams, qmParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpFqmStateCmd(&cmdBuffer, &fqmParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpQmStateCmd(&cmdBuffer, &qmParams));

    if (m_brcEnabled)
    {
        uint32_t picStateCmdOffset;
        if (m_hucPakStitchEnabled && m_numPipe >= 2)
        {
            //for non fist PAK pass, always use the 2nd HCP PIC STATE cmd buffer
            picStateCmdOffset = IsFirstPass() ? 0 : 1;
        }
        else
        {
            picStateCmdOffset = GetCurrentPass();
        }

        MOS_RESOURCE &brcHcpStateWriteBuffer = m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx];
        if (IsPanicModePass())
        {
            // BRC kernel supports only 4 BrcImageStates read/write buffers.
            // So for panic PAK pass use HCP_PIC_STATE command from previous PAK pass.
            picStateCmdOffset -= 1;
        }

        MHW_BATCH_BUFFER batchBuffer;
        MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
        batchBuffer.OsResource   = brcHcpStateWriteBuffer;
        batchBuffer.dwOffset     = picStateCmdOffset * BRC_IMG_STATE_SIZE_PER_PASS_G12;
        batchBuffer.bSecondLevel = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            &batchBuffer));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPictureStateCmd(&cmdBuffer));
    }

    // Send HEVC_VP9_RDOQ_STATE command
    if (m_hevcRdoqEnabled)
    {
        MHW_VDBOX_HEVC_PIC_STATE picStateParams;
        SetHcpPicStateParams(picStateParams);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(&cmdBuffer, &picStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));
    return eStatus;
}

void CodechalEncHevcStateG12::SetHcpSliceStateCommonParams(
    MHW_VDBOX_HEVC_SLICE_STATE &sliceState)
{
    CodechalEncHevcState::SetHcpSliceStateCommonParams(sliceState);

    sliceState.RoundingIntra = m_roundingIntraInUse;
    sliceState.RoundingInter = m_roundingInterInUse;

    if ((m_hevcSliceParams->slice_type == CODECHAL_HEVC_P_SLICE && m_hevcPicParams->weighted_pred_flag) ||
        (m_hevcSliceParams->slice_type == CODECHAL_HEVC_B_SLICE && m_hevcPicParams->weighted_bipred_flag))
    {
        sliceState.bWeightedPredInUse = true;
    }
    else
    {
        sliceState.bWeightedPredInUse = false;
    }

    static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12 &>(sliceState).dwNumPipe = m_numPipe;

    sliceState.presDataBuffer = IsPanicModePass() ? &m_skipFrameInfo.m_resMbCodeSkipFrameSurface : &m_resMbCodeSurface;
}

void CodechalEncHevcStateG12::SetHcpSliceStateParams(
    MHW_VDBOX_HEVC_SLICE_STATE &          sliceState,
    PCODEC_ENCODER_SLCDATA                slcData,
    uint16_t                              slcCount,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileCodingParams,
    bool                                  lastSliceInTile,
    uint32_t                              idx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    sliceState.pEncodeHevcSliceParams                                           = &m_hevcSliceParams[slcCount];
    sliceState.dwDataBufferOffset                                               = slcData[slcCount].CmdOffset;
    sliceState.dwOffset                                                         = slcData[slcCount].SliceOffset;
    sliceState.dwLength                                                         = slcData[slcCount].BitSize;
    sliceState.uiSkipEmulationCheckCount                                        = slcData[slcCount].SkipEmulationByteCount;
    sliceState.dwSliceIndex                                                     = (uint32_t)slcCount;
    sliceState.bLastSlice                                                       = (slcCount == m_numSlices - 1);
    sliceState.bLastSliceInTile                                                 = lastSliceInTile;
    sliceState.bLastSliceInTileColumn                                           = (bool)lastSliceInTile & tileCodingParams[idx].IsLastTileofColumn;
    sliceState.bFirstPass                                                       = IsFirstPass();
    sliceState.bLastPass                                                        = IsLastPass();
    sliceState.bInsertBeforeSliceHeaders                                        = (slcCount == 0);
    sliceState.bSaoLumaFlag                                                     = (m_hevcSeqParams->SAO_enabled_flag) ? m_hevcSliceParams[slcCount].slice_sao_luma_flag : 0;
    sliceState.bSaoChromaFlag                                                   = (m_hevcSeqParams->SAO_enabled_flag) ? m_hevcSliceParams[slcCount].slice_sao_chroma_flag : 0;
    static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12 &>(sliceState).pTileCodingParams = tileCodingParams + idx;
    static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12 &>(sliceState).dwTileID          = idx;

    sliceState.DeblockingFilterDisable = m_hevcSliceParams[slcCount].slice_deblocking_filter_disable_flag;
    sliceState.TcOffsetDiv2            = m_hevcSliceParams[slcCount].tc_offset_div2;
    sliceState.BetaOffsetDiv2          = m_hevcSliceParams[slcCount].beta_offset_div2;

    CalcTransformSkipParameters(sliceState.EncodeHevcTransformSkipParams);
}

MOS_STATUS CodechalEncHevcStateG12::SetMfxVideoCopyCmdParams(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetCpInterface());
    MhwCpInterface *cpInterface = m_hwInterface->GetCpInterface();

    uint32_t index = m_virtualEngineBbIndex;

    MHW_CP_COPY_PARAMS cpCopyParams;
    MOS_ZeroMemory(&cpCopyParams, sizeof(cpCopyParams));

    cpCopyParams.size          = m_hwInterface->m_tileRecordSize;
    cpCopyParams.presSrc       = &m_tileRecordBuffer[index].sResource;
    cpCopyParams.presDst       = &m_resBitstreamBuffer;
    cpCopyParams.lengthOfTable = (uint8_t)(m_numTiles);
    cpCopyParams.isEncodeInUse = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->SetCpCopy(m_osInterface, cmdBuffer, &cpCopyParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_slcData);

    if (m_pakOnlyTest)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadPakCommandAndCuRecordFromFile());
    }

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::ExecuteSliceLevel());
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncTileLevel());
    }

    /*
    if ((m_useMdf) && (m_rawSurfaceToEnc))
    {
        m_osInterface->pfnWaitOnResource(m_osInterface,
                                         &m_rawSurfaceToEnc->OsResource);
    }
    */

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncTileLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    int32_t currentPipe = GetCurrentPipe();
    int32_t currentPass = GetCurrentPass();

    if (currentPipe < 0 || currentPass < 0)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid pipe number or pass number");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState;
    SetHcpSliceStateCommonParams(sliceState);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    uint32_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    uint32_t numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;

    for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
        {
            PCODEC_ENCODER_SLCDATA slcData = m_slcData;
            uint32_t               slcCount, idx, sliceNumInTile = 0;

            idx = tileRow * numTileColumns + tileCol;

            if ((m_numPipe > 1) && (tileCol != currentPipe))
            {
                continue;
            }

            // HCP_TILE_CODING commmand
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                static_cast<MhwVdboxHcpInterfaceG12 *>(m_hcpInterface)->AddHcpTileCodingCmd(&cmdBuffer, &m_tileParams[idx]));

            for (slcCount = 0; slcCount < m_numSlices; slcCount++)
            {
                bool lastSliceInTile = false, sliceInTile = false;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                    &m_tileParams[idx],
                    &sliceInTile,
                    &lastSliceInTile));

                if (!sliceInTile)
                {
                    continue;
                }

                if (IsFirstPass())
                {
                    uint32_t startLcu = 0;
                    for (uint32_t ii = 0; ii < slcCount; ii++)
                    {
                        startLcu += m_hevcSliceParams[ii].NumLCUsInSlice;
                    }
                    slcData[slcCount].CmdOffset = startLcu * (m_hwInterface->GetHcpInterface()->GetHcpPakObjSize()) * sizeof(uint32_t);
                }

                SetHcpSliceStateParams(sliceState, slcData, (uint16_t)slcCount, m_tileParams, lastSliceInTile, idx);

                CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(&cmdBuffer, &sliceState));

                sliceNumInTile++;
            }  // end of slice

            if (0 == sliceNumInTile)
            {
                // One tile must have at least one slice
                CODECHAL_ENCODE_ASSERT(false);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }  // end of row tile
    }      // end of column tile

    // Insert end of sequence/stream if set
    if ((m_lastPicInStream || m_lastPicInSeq) && IsLastPipe())
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq    = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&cmdBuffer, &pakInsertObjectParams));
    }

    // Send VD_PIPELINE_FLUSH command
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    vdPipelineFlushParams.Flags.bWaitDoneHEVC           = 1;
    vdPipelineFlushParams.Flags.bFlushHEVC              = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    //HW Semaphore cmd to make sure all pipes completion encode
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resPipeCompleteSemaMem, 1, MHW_MI_ATOMIC_INC, &cmdBuffer));

    if (IsFirstPipe())
    {
        // first pipe needs to ensure all other pipes are ready
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(
            &m_resPipeCompleteSemaMem,
            &cmdBuffer,
            m_numPipe));

        //clean HW semaphore memory
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource = &m_resPipeCompleteSemaMem;
        storeDataParams.dwValue     = 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            &cmdBuffer,
            &storeDataParams));

        // Use HW stitch commands only in the scalable mode
        if (m_numPipe > 1 && m_enableTileStitchByHW)
        {
            //call PAK Int Kernel in scalability case
            if (m_hucPakStitchEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(HucPakIntegrate(&cmdBuffer));
#if 0  // Need to enable this code once Gen12 becomes open source \
       // 2nd level BB buffer for stitching cmd                   \
       // current location to add cmds in 2nd level batch buffer
                m_HucStitchCmdBatchBuffer.iCurrent = 0;
                // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
                m_HucStitchCmdBatchBuffer.dwOffset = 0;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_HucStitchCmdBatchBuffer));
                // This wait cmd is needed to make sure copy command is done as suggested by HW folk in encode cases
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(&cmdBuffer, nullptr, m_osInterface->osCpInterface->IsCpEnabled() ? true : false));
#endif
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMfxVideoCopyCmdParams(&cmdBuffer));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSseStatistics(&cmdBuffer));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

        if (m_numPipe <= 1)  // single pipe mode can read the info from MMIO register. Otherwise, we have to use the tile size statistic buffer
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));

            // BRC PAK statistics different for each pass
            if (m_brcEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStats(&cmdBuffer));
            }
        }
        else
        {  //scalability mode
            if (m_brcEnabled)
            {
                //MMIO register is not used in scalability BRC case. all information is in TileSizeRecord stream out buffer
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatisticsForScalability(&cmdBuffer));
            }
            else
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        //this is to support BRC scalbility test to match with single pipe. Will be removed later after enhanced BRC Scalability is enabled.
        if (m_brcEnabled && m_forceSinglePakPass)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ResetImgCtrlRegInPAKStatisticsBuffer(&cmdBuffer));
        }
#endif

        if (m_singleTaskPhaseSupported &&
            m_brcEnabled && m_numPipe >= 2 && !IsLastPass())
        {
            // Signal HW semaphore for the BRC dependency (i.e., next BRC pass waits for the current BRC pass)
            for (auto i = 0; i < m_numPipe; i++)
            {
                if (!Mos_ResourceIsNull(&m_resBrcSemaphoreMem[i].sResource))
                {
                    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
                    storeDataParams.pOsResource = &m_resBrcSemaphoreMem[i].sResource;
                    storeDataParams.dwValue     = 1;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                        &cmdBuffer,
                        &storeDataParams));
                }
            }
        }
    }

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data()));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (IsFirstPipe() &&
        (m_pakOnlyTest == 0) &&  // In the PAK only test, no need to wait for ENC's completion
        IsFirstPass() &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams  = g_cInitSyncParams;
        syncParams.GpuContext       = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        bool nullRendering = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, nullRendering));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpHucDebugOutputBuffers());
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpPakOutput());
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            })

        if ((IsLastPipe()) &&
            (IsLastPass()) &&
            m_signalEnc &&
            m_currRefSync &&
            !Mos_ResourceIsNull(&m_currRefSync->resSyncObject))
        {
            // signal semaphore
            MOS_SYNC_PARAMS syncParams;
            syncParams                  = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount++;
            m_currRefSync->bInUsed = true;
        }
    }

    // Reset parameters for next PAK execution
    if (IsLastPipe() && IsLastPass())
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_currPakSliceIdx = (m_currPakSliceIdx + 1) % CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS;

        if (m_hevcSeqParams->ParallelBRC)
        {
            m_brcBuffers.uiCurrBrcPakStasIdxForWrite =
                (m_brcBuffers.uiCurrBrcPakStasIdxForWrite + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::DecideEncodingPipeNumber()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_numPipe = m_numVdbox;

    uint8_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;

    if (numTileColumns > m_numPipe)
    {
        m_numPipe = 1;
    }

    if (numTileColumns < m_numPipe)
    {
        if (numTileColumns >= 1 && numTileColumns <= 4)
        {
            m_numPipe = numTileColumns;
        }
        else
        {
            m_numPipe = 1;  // invalid tile column test cases and switch back to the single VDBOX mode
        }
    }

    m_useVirtualEngine = true;  //always use virtual engine interface for single pipe and scalability mode

    if (!m_forceScalability)
    {
        //resolution < 4K, always go with single pipe
        if (m_frameWidth * m_frameHeight < ENCODE_HEVC_4K_PIC_WIDTH * ENCODE_HEVC_4K_PIC_HEIGHT)
        {
            m_numPipe = 1;
        }
    }

    m_numUsedVdbox       = m_numPipe;
    m_numberTilesInFrame = (m_hevcPicParams->num_tile_rows_minus1 + 1) * (m_hevcPicParams->num_tile_columns_minus1 + 1);

    if (m_scalabilityState)
    {
        // Create/ re-use a GPU context with 2 pipes
        m_scalabilityState->ucScalablePipeNum = m_numPipe;
    }
    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::PlatformCapabilityCheck()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DecideEncodingPipeNumber());

    if (MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeScalability_ChkGpuCtxReCreation(this, m_scalabilityState, (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    if (m_frameWidth * m_frameHeight > ENCODE_HEVC_MAX_16K_PIC_WIDTH * ENCODE_HEVC_MAX_16K_PIC_HEIGHT)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Frame resolution greater than 16k not supported");
    }

    if (m_vdencEnabled && m_chromaFormat == HCP_CHROMA_FORMAT_YUV444 && m_hevcSeqParams->TargetUsage == 7)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Speed mode is not supported in VDENC 444, resetting TargetUsage to Normal mode\n");
        m_hevcSeqParams->TargetUsage = 4;
    }

    if ((uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_chromaFormat &&
        (uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_outputChromaFormat &&
        Format_YUY2 == m_reconSurface.Format)
    {
        if (m_reconSurface.dwHeight < m_oriFrameHeight * 2 ||
            m_reconSurface.dwWidth < m_oriFrameWidth / 2)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    // set RDOQ Intra blocks Threshold for Gen11+
    m_rdoqIntraTuThreshold = 0;
    if (m_hevcRdoqEnabled)
    {
        if (1 == m_hevcSeqParams->TargetUsage)
        {
            m_rdoqIntraTuThreshold = 0xffff;
        }
        else if (4 == m_hevcSeqParams->TargetUsage)
        {
            m_rdoqIntraTuThreshold = m_picWidthInMb * m_picHeightInMb;
            m_rdoqIntraTuThreshold = MOS_MIN(m_rdoqIntraTuThreshold / 10, 0xffff);
        }
    }

    return eStatus;
}

bool CodechalEncHevcStateG12::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool isColorFormatSupported = false;

    if (nullptr == surface)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return isColorFormatSupported;
    }

    switch (surface->Format)
    {
    case Format_NV12:
        isColorFormatSupported = IS_Y_MAJOR_TILE_FORMAT(surface->TileType);
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_A8R8G8B8:
    case Format_P010:
    case Format_P016:
    case Format_Y210:
    case Format_Y216:
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Input surface color format = %d not supported!", surface->Format);
        break;
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalEncHevcStateG12::GetSystemPipeNumberCommon()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;
    statusKey            = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_DISABLE_SCALABILITY_G12,
        &userFeatureData);

    bool disableScalability = true;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = userFeatureData.i32Data ? true : false;
    }

    MEDIA_SYSTEM_INFO *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(gtSystemInfo);

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

MOS_STATUS CodechalEncHevcStateG12::HucPakIntegrate(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    auto mmioRegisters = m_hwInterface->GetHucInterface()->GetMmioRegisters(m_vdboxIndex);

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = VDBOX_HUC_PAK_INTEGRATION_KERNEL_DESCRIPTOR;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    // DMEM set
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    if (m_brcEnabled && m_hevcSeqParams->RateControlMethod != RATECONTROL_ICQ)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCPakIntegrate(&dmemParams));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCPakIntegrateCqp(&dmemParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    if (m_brcEnabled && m_hevcSeqParams->RateControlMethod != RATECONTROL_ICQ)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCPakIntegrate(&virtualAddrParams));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCPakIntegrateCqp(&virtualAddrParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    // Write HUC_STATUS2 mask - bit 6 - valid IMEM loaded
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource      = &m_resHucStatus2Buffer;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue          = m_hwInterface->GetHucInterface()->GetHucStatus2ImemLoadedMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    // Store HUC_STATUS2 register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resHucStatus2Buffer;
    storeRegParams.dwOffset        = sizeof(uint32_t);
    storeRegParams.dwRegister      = mmioRegisters->hucStatus2RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC    = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVdencInterface()->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    EncodeStatusBuffer encodeStatusBuf = m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf.wCurrIndex * encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    // Write HUC_STATUS mask
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource      = &encodeStatusBuf.resStatusBuffer;
    storeDataParams.dwResourceOffset = baseOffset + encodeStatusBuf.dwHuCStatusMaskOffset;
    storeDataParams.dwValue          = m_hwInterface->GetHucInterface()->GetHucStatusReEncodeMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    // store HUC_STATUS register
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &encodeStatusBuf.resStatusBuffer;
    storeRegParams.dwOffset        = baseOffset + encodeStatusBuf.dwHuCStatusRegOffset;
    storeRegParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &storeRegParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::Initialize(CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_STATUS                  statusKey = MOS_STATUS_SUCCESS;

#if (_DEBUG || _RELEASE_INTERNAL)
    char stringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.StringData.pStringData = stringData;
    statusKey                              = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_PAK_ONLY_ID_G12,
        &userFeatureData);

    if (statusKey == MOS_STATUS_SUCCESS && userFeatureData.StringData.uSize > 0)
    {
        MOS_SecureStrcpy(m_pakOnlyDataFolder,
            sizeof(m_pakOnlyDataFolder) / sizeof(m_pakOnlyDataFolder[0]),
            stringData);

        uint32_t len = strlen(m_pakOnlyDataFolder);
        if (m_pakOnlyDataFolder[len - 1] == '\\')
        {
            m_pakOnlyDataFolder[len - 1] = 0;
        }

        m_pakOnlyTest = true;
        // PAK only mode does not need to init any kernel
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.StringData.pStringData = stringData;
    statusKey                              = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_LOAD_KERNEL_INPUT_ID_G12,
        &userFeatureData);

    if (statusKey == MOS_STATUS_SUCCESS && userFeatureData.StringData.uSize > 0)
    {
        MOS_SecureStrcpy(m_loadKernelInputDataFolder,
            sizeof(m_loadKernelInputDataFolder) / sizeof(m_loadKernelInputDataFolder[0]),
            stringData);

        uint32_t len = strlen(m_loadKernelInputDataFolder);
        if (m_loadKernelInputDataFolder[len - 1] == '\\')
        {
            m_loadKernelInputDataFolder[len - 1] = 0;
        }
        m_loadKernelInput = true;
    }
#endif

    // Common initialization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::Initialize(settings));

    m_numDelay                              = 15;  //Value suggested by HW team.
    m_bmeMethodTable                        = (uint8_t *)m_meMethod;
    m_b4XMeDistortionBufferSupported        = true;
    m_brcBuffers.dwBrcConstantSurfaceWidth  = HEVC_BRC_CONSTANT_SURFACE_WIDTH_G9;
    m_brcBuffers.dwBrcConstantSurfaceHeight = HEVC_BRC_CONSTANT_SURFACE_HEIGHT_G10;
    m_brcHistoryBufferSize                  = HEVC_BRC_HISTORY_BUFFER_SIZE_G12;
    m_maxNumSlicesSupported                 = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6;
    m_brcBuffers.dwBrcHcpPicStateSize       = BRC_IMG_STATE_SIZE_PER_PASS_G12 * CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID,
        &userFeatureData);
    // Region number must be greater than 1
    m_numberConcurrentGroup = (userFeatureData.i32Data < 1) ? 1 : userFeatureData.i32Data;

    if (m_numberConcurrentGroup > 16)
    {
        // Region number cannot be larger than 16
        m_numberConcurrentGroup = 16;
    }

    m_sizeOfHcpPakFrameStats = 9 * CODECHAL_CACHELINE_SIZE;  //Frame statistics occupying 9 caceline on gen12

    // Subthread number used in the ENC kernel
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_SUBTHREAD_NUM_ID_G12,
        &userFeatureData);
    m_numberEncKernelSubThread = (userFeatureData.i32Data < 1) ? 1 : userFeatureData.i32Data;

    if (m_numberEncKernelSubThread > m_hevcThreadTaskDataNum)
    {
        m_numberEncKernelSubThread = m_hevcThreadTaskDataNum;  // support up to 2 sub-threads in one LCU64x64
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID,
        &userFeatureData);
    m_enable26WalkingPattern = (userFeatureData.i32Data) ? false : true;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_ENABLE_ID,
        &userFeatureData);
    m_hevcRdoqEnabled = userFeatureData.i32Data ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VME_ENCODE_SSE_ENABLE_ID_G12,
        &userFeatureData);
    m_sseSupported = userFeatureData.i32Data ? true : false;

    // Overriding the defaults here with 32 aligned dimensions
    // 2x Scaling WxH
    m_downscaledWidth2x =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_frameWidth);
    m_downscaledHeight2x =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_frameHeight);

    // HME Scaling WxH
    m_downscaledWidth4x =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_frameWidth);
    m_downscaledHeight4x =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_frameHeight);
    m_downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth4x);
    m_downscaledHeightInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight4x);

    // SuperHME Scaling WxH
    m_downscaledWidth16x =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_downscaledWidth4x);
    m_downscaledHeight16x =
        CODECHAL_GET_4xDS_SIZE_32ALIGNED(m_downscaledHeight4x);
    m_downscaledWidthInMb16x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth16x);
    m_downscaledHeightInMb16x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight16x);

    // UltraHME Scaling WxH
    m_downscaledWidth32x =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_downscaledWidth16x);
    m_downscaledHeight32x =
        CODECHAL_GET_2xDS_SIZE_32ALIGNED(m_downscaledHeight16x);
    m_downscaledWidthInMb32x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth32x);
    m_downscaledHeightInMb32x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledHeight32x);

    // disable MMCD if we enable Codechal dump. Because dump code changes the surface state from compressed to uncompressed,
    // this causes mis-match issue between dump is enabled or disabled.
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState && m_debugInterface && m_debugInterface->m_dbgCfgHead){
            //m_mmcState->SetMmcDisabled();
        })

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetSystemPipeNumberCommon());

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_scalabilityState = (PCODECHAL_ENCODE_SCALABILITY_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SCALABILITY_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalabilityState);
        //scalability initialize
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_InitializeState(m_scalabilityState, m_hwInterface));
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_HW_STITCH_G12,
        &userFeatureData);
    m_enableTileStitchByHW = userFeatureData.i32Data ? true : false;

    statusKey = MOS_STATUS_SUCCESS;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_HW_SEMAPHORE_G12,
        &userFeatureData);
    m_enableHWSemaphore = userFeatureData.i32Data ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_WP_SUPPORT_ID,
        &userFeatureData);
    m_weightedPredictionSupported = userFeatureData.i32Data ? true : false;

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VE_DEBUG_OVERRIDE_G12,
        &userFeatureData);
    m_kmdVeOveride.Value = (uint64_t)userFeatureData.i64Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VME_FORCE_SCALABILITY_ID_G12,
        &userFeatureData);
    m_forceScalability = userFeatureData.i32Data ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VME_DISABLE_PANIC_MODE_ID_G12,
        &userFeatureData);
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        m_enableFramePanicMode = userFeatureData.i32Data ? false : true;
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VME_BRC_LTR_INTERVAL_ID,
        &userFeatureData);
    m_ltrInterval = (uint32_t)(userFeatureData.i32Data);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VME_BRC_LTR_DISABLE_ID,
        &userFeatureData);
    m_enableBrcLTR = (userFeatureData.i32Data) ? false : true;
#endif

    if (m_codecFunction != CODECHAL_FUNCTION_PAK)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID,
            &userFeatureData);
        m_hmeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID,
            &userFeatureData);
        m_16xMeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_32xME_ENABLE_ID,
            &userFeatureData);
        // Keeping UHME by Default ON for Gen12
        m_32xMeSupported = (userFeatureData.i32Data) ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_NUM_THREADS_PER_LCU_ID,
            &userFeatureData);
        m_totalNumThreadsPerLcu = (uint16_t)userFeatureData.i32Data;

        if (m_totalNumThreadsPerLcu < m_minThreadsPerLcuB || m_totalNumThreadsPerLcu > m_maxThreadsPerLcuB)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    if (m_frameWidth < 128 || m_frameHeight < 128)
    {
        m_16xMeSupported = false;
        m_32xMeSupported = false;
    }
    else if (m_frameWidth < 512 || m_frameHeight < 512)
    {
        m_32xMeSupported = false;
    }

    return eStatus;
}

void CodechalEncHevcStateG12::LoadCosts(uint8_t sliceType, uint8_t qp)
{
    if (sliceType >= CODECHAL_HEVC_NUM_SLICE_TYPES)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid slice type");
        sliceType = CODECHAL_HEVC_I_SLICE;
    }

    double  qpScale   = 0.60;
    int32_t qpMinus12 = qp - 12;
    double  lambda    = sqrt(qpScale * pow(2.0, MOS_MAX(0, qpMinus12) / 3.0));
    uint8_t lcuIdx    = ((m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3) == 6) ? 1 : 0;
    m_lambdaRD        = (uint16_t)(qpScale * pow(2.0, MOS_MAX(0, qpMinus12) / 3.0) * 4 + 0.5);

    m_modeCostCre[LUTCREMODE_INTRA_32X32]       = CRECOST(lambda, LUTMODEBITS_INTRA_32X32, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_16X16]       = CRECOST(lambda, LUTMODEBITS_INTRA_16X16, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_8X8]         = CRECOST(lambda, LUTMODEBITS_INTRA_8X8, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_CHROMA]      = CRECOST(lambda, LUTMODEBITS_INTRA_CHROMA, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_32X32]       = CRECOST(lambda, LUTMODEBITS_INTER_32X32, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_32X16]       = CRECOST(lambda, LUTMODEBITS_INTER_32X16, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_16X16]       = CRECOST(lambda, LUTMODEBITS_INTER_16X16, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_16X8]        = CRECOST(lambda, LUTMODEBITS_INTER_16X8, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_8X8]         = CRECOST(lambda, LUTMODEBITS_INTER_8X8, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_BIDIR]       = CRECOST(lambda, LUTMODEBITS_INTER_BIDIR, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTER_SKIP]        = CRECOST(lambda, LUTMODEBITS_INTER_SKIP, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_NONDC_32X32] = CRECOST(lambda, LUTMODEBITS_INTRA_NONDC_32X32, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_NONDC_16X16] = CRECOST(lambda, LUTMODEBITS_INTRA_NONDC_16X16, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_NONDC_8X8]   = CRECOST(lambda, LUTMODEBITS_INTRA_NONDC_8X8, lcuIdx, sliceType);
    m_modeCostCre[LUTCREMODE_INTRA_NONPRED]     = CRECOST(lambda, LUTMODEBITS_INTRA_MPM, lcuIdx, sliceType);

    m_modeCostRde[LUTRDEMODE_INTRA_64X64]       = RDEBITS62(LUTMODEBITS_INTRA_64X64, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_32X32]       = RDEBITS62(LUTMODEBITS_INTRA_32X32, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_16X16]       = RDEBITS62(LUTMODEBITS_INTRA_16X16, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_8X8]         = RDEBITS62(LUTMODEBITS_INTRA_8X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_NXN]         = RDEBITS62(LUTMODEBITS_INTRA_NXN, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_MPM]         = RDEBITS62(LUTMODEBITS_INTRA_MPM, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_DC_32X32]    = RDEBITS62(LUTMODEBITS_INTRA_DC_32X32, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_DC_8X8]      = RDEBITS62(LUTMODEBITS_INTRA_DC_8X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_NONDC_32X32] = RDEBITS62(LUTMODEBITS_INTRA_NONDC_32X32, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTRA_NONDC_8X8]   = RDEBITS62(LUTMODEBITS_INTRA_NONDC_8X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_BIDIR]       = RDEBITS62(LUTMODEBITS_INTER_BIDIR, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_REFID]       = RDEBITS62(LUTMODEBITS_INTER_REFID, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_SKIP_64X64]        = RDEBITS62(LUTMODEBITS_SKIP_64X64, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_SKIP_32X32]        = RDEBITS62(LUTMODEBITS_SKIP_32X32, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_SKIP_16X16]        = RDEBITS62(LUTMODEBITS_SKIP_16X16, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_SKIP_8X8]          = RDEBITS62(LUTMODEBITS_SKIP_8X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_MERGE_64X64]       = RDEBITS62(LUTMODEBITS_MERGE_64X64, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_MERGE_32X32]       = RDEBITS62(LUTMODEBITS_MERGE_32X32, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_MERGE_16X16]       = RDEBITS62(LUTMODEBITS_MERGE_16X16, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_MERGE_8X8]         = RDEBITS62(LUTMODEBITS_MERGE_8X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_32X32]       = RDEBITS62(LUTMODEBITS_INTER_32X32, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_32X16]       = RDEBITS62(LUTMODEBITS_INTER_32X16, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_16X16]       = RDEBITS62(LUTMODEBITS_INTER_16X16, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_16X8]        = RDEBITS62(LUTMODEBITS_INTER_16X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_INTER_8X8]         = RDEBITS62(LUTMODEBITS_INTER_8X8, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_TU_DEPTH_0]        = RDEBITS62(LUTMODEBITS_TU_DEPTH_0, lcuIdx, sliceType);
    m_modeCostRde[LUTRDEMODE_TU_DEPTH_1]        = RDEBITS62(LUTMODEBITS_TU_DEPTH_1, lcuIdx, sliceType);

    for (uint8_t i = 0; i < 8; i++)
    {
        m_modeCostRde[LUTRDEMODE_CBF + i] = RDEBITS62(LUTMODEBITS_CBF + i, lcuIdx, sliceType);
    }
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC MbEnc B Kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG12::SetCurbeMbEncBKernel()
{
    uint32_t        curIdx = m_currRecycledBufIdx;
    MOS_LOCK_PARAMS lockFlags;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    uint8_t tuMapping = ((m_hevcSeqParams->TargetUsage) / 3) % 3;  // Map TU 1,4,6 to 0,1,2

    // Initialize the CURBE data
    MBENC_CURBE curbe;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.QPType    = QP_TYPE_CONSTANT;
        curbe.ROIEnable = m_hevcPicParams->NumROI ? true : false;
    }
    else
    {
        curbe.QPType = m_lcuBrcEnabled ? QP_TYPE_CU_LEVEL : QP_TYPE_FRAME;
    }

    // TU based settings
    curbe.EnableCu64Check        = m_tuSettings[EnableCu64CheckTuParam][tuMapping];
    curbe.MaxNumIMESearchCenter  = m_tuSettings[MaxNumIMESearchCenterTuParam][tuMapping];
    curbe.MaxTransformDepthInter = m_tuSettings[Log2TUMaxDepthInterTuParam][tuMapping];
    curbe.MaxTransformDepthIntra = m_tuSettings[Log2TUMaxDepthIntraTuParam][tuMapping];
    curbe.Dynamic64Order         = m_tuSettings[Dynamic64OrderTuParam][tuMapping];
    curbe.DynamicOrderTh         = m_tuSettings[DynamicOrderThTuParam][tuMapping];
    curbe.Dynamic64Enable        = m_tuSettings[Dynamic64EnableTuParam][tuMapping];
    curbe.Dynamic64Th            = m_tuSettings[Dynamic64ThTuParam][tuMapping];
    curbe.IncreaseExitThresh     = m_tuSettings[IncreaseExitThreshTuParam][tuMapping];
    curbe.IntraSpotCheck         = m_tuSettings[IntraSpotCheckFlagTuParam][tuMapping];
    curbe.Fake32Enable           = m_tuSettings[Fake32EnableTuParam][tuMapping];

    curbe.FrameWidthInSamples  = m_frameWidth;
    curbe.FrameHeightInSamples = m_frameHeight;

    curbe.Log2MaxCUSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    curbe.Log2MinCUSize = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    curbe.Log2MaxTUSize = m_hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
    curbe.Log2MinTUSize = m_hevcSeqParams->log2_min_transform_block_size_minus2 + 2;

    curbe.ChromaFormatType = m_hevcSeqParams->chroma_format_idc;

    curbe.TUDepthControl = curbe.MaxTransformDepthInter;

    int32_t sliceQp   = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.FrameQP     = abs(sliceQp);
    curbe.FrameQPSign = (sliceQp > 0) ? 0 : 1;

#if 0  // no need in the optimized kernel because kernel does the table look-up
    LoadCosts(CODECHAL_HEVC_B_SLICE, (uint8_t)sliceQp);
    curbe.DW4_ModeIntra32x32Cost = m_modeCostCre[LUTCREMODE_INTRA_32X32];
    curbe.DW4_ModeIntraNonDC32x32Cost = m_modeCostCre[LUTCREMODE_INTRA_NONDC_32X32];

    curbe.DW5_ModeIntra16x16Cost = m_modeCostCre[LUTCREMODE_INTRA_16X16];
    curbe.DW5_ModeIntraNonDC16x16Cost = m_modeCostCre[LUTCREMODE_INTRA_NONDC_16X16];
    curbe.DW5_ModeIntra8x8Cost = m_modeCostCre[LUTCREMODE_INTRA_8X8];
    curbe.DW5_ModeIntraNonDC8x8Cost = m_modeCostCre[LUTCREMODE_INTRA_NONDC_8X8];

    curbe.DW6_ModeIntraNonPred = m_modeCostCre[LUTCREMODE_INTRA_NONPRED];

    curbe.DW7_ChromaIntraModeCost = m_modeCostCre[LUTCREMODE_INTRA_CHROMA];

    curbe.DW12_IntraModeCostMPM = m_modeCostRde[LUTRDEMODE_INTRA_MPM];

    curbe.DW13_IntraTUDept0Cost = m_modeCostRde[LUTRDEMODE_TU_DEPTH_0];
    curbe.DW13_IntraTUDept1Cost = m_modeCostRde[LUTRDEMODE_TU_DEPTH_1];

    curbe.DW14_IntraTU4x4CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_4X4];
    curbe.DW14_IntraTU8x8CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_8X8];
    curbe.DW14_IntraTU16x16CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_16X16];
    curbe.DW14_IntraTU32x32CBFCost = m_modeCostRde[LUTRDEMODE_INTRA_CBF_32X32];
    curbe.DW15_LambdaRD = (uint16_t)m_lambdaRD;
    curbe.DW17_IntraNonDC8x8Penalty = m_modeCostRde[LUTRDEMODE_INTRA_NONDC_8X8];
    curbe.DW17_IntraNonDC32x32Penalty = m_modeCostRde[LUTRDEMODE_INTRA_NONDC_32X32];
#endif

    curbe.NumofColumnTile = m_hevcPicParams->num_tile_columns_minus1 + 1;
    curbe.NumofRowTile    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    curbe.HMEFlag         = m_hmeSupported ? 3 : 0;

    curbe.MaxRefIdxL0  = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10 - 1;
    curbe.MaxRefIdxL1  = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G10 - 1;
    curbe.MaxBRefIdxL0 = CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10 - 1;

    // Check whether Last Frame is I frame or not
    if (m_frameNum == 0 || m_picHeightInMb == I_TYPE || (m_frameNum && m_lastPictureCodingType == I_TYPE))
    {
        // This is the flag to notify kernel not to use the history buffer
        curbe.LastFrameIsIntra = true;
    }
    else
    {
        curbe.LastFrameIsIntra = false;
    }

    curbe.SliceType             = PicCodingTypeToSliceType(m_hevcPicParams->CodingType);
    curbe.TemporalMvpEnableFlag = m_hevcSliceParams->slice_temporal_mvp_enable_flag;
    curbe.CollocatedFromL0Flag  = m_hevcSliceParams->collocated_from_l0_flag;
    curbe.theSameRefList        = m_sameRefList;
    curbe.IsLowDelay            = m_lowDelay;
    curbe.MaxNumMergeCand       = m_hevcSliceParams->MaxNumMergeCand;
    curbe.NumRefIdxL0           = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    curbe.NumRefIdxL1           = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;

    if (m_hevcSeqParams->TargetUsage == 1)
    {
        // MaxNumMergeCand C Model uses 4 for TU1,
        // for quality consideration, make sure not larger than the value from App as it will be used in PAK
        curbe.MaxNumMergeCand = MOS_MIN(m_hevcSliceParams->MaxNumMergeCand, 4);
    }
    else
    {
        // MaxNumMergeCand C Model uses 2 for TU4 and TU7,
        // for quality consideration, make sure not larger than the value from App as it will be used in PAK
        curbe.MaxNumMergeCand = MOS_MIN(m_hevcSliceParams->MaxNumMergeCand, 2);
    }

    int32_t tbRefListL0[CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L0_REF_G10] = {0}, tbRefListL1[CODECHAL_ENCODE_HEVC_NUM_MAX_VME_L1_REF_G10] = {0};
    curbe.FwdPocNumber_L0_mTb_0 = tbRefListL0[0] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][0]);
    curbe.BwdPocNumber_L1_mTb_0 = tbRefListL1[0] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][0]);
    curbe.FwdPocNumber_L0_mTb_1 = tbRefListL0[1] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][1]);
    curbe.BwdPocNumber_L1_mTb_1 = tbRefListL1[1] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][1]);

    curbe.FwdPocNumber_L0_mTb_2 = tbRefListL0[2] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][2]);
    curbe.BwdPocNumber_L1_mTb_2 = tbRefListL1[2] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][2]);
    curbe.FwdPocNumber_L0_mTb_3 = tbRefListL0[3] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[0][3]);
    curbe.BwdPocNumber_L1_mTb_3 = tbRefListL1[3] = ComputeTemporalDifferent(m_hevcSliceParams->RefPicList[1][3]);

    curbe.RefFrameWinHeight = m_frameHeight;
    curbe.RefFrameWinWidth  = m_frameWidth;

    // Hard coding for now from Gen10HEVC_TU4_default.par
    curbe.RoundingInter      = (m_roundingInter + 1) << 4;  // Should be an input from par(slice state)
    curbe.RoundingIntra      = (m_roundingIntra + 1) << 4;  // Should be an input from par(slice state)
    curbe.RDEQuantRoundValue = (m_roundingInter + 1) << 4;

    uint32_t gopP = (m_hevcSeqParams->GopRefDist) ? ((m_hevcSeqParams->GopPicSize - 1) / m_hevcSeqParams->GopRefDist) : 0;
    uint32_t gopB = m_hevcSeqParams->GopPicSize - 1 - gopP;

    curbe.CostScalingForRA = 1;  // default setting

    // get the min distance between current pic and ref pics
    uint32_t minPocDist     = 255;
    uint32_t costTableIndex = 0;
    if (curbe.CostScalingForRA == 1)
    {
        for (uint8_t ref = 0; ref < curbe.NumRefIdxL0; ref++)
        {
            if ((uint32_t)abs(tbRefListL0[ref]) < minPocDist)
                minPocDist = abs(tbRefListL0[ref]);
        }
        for (uint8_t ref = 0; ref < curbe.NumRefIdxL1; ref++)
        {
            if ((uint32_t)abs(tbRefListL1[ref]) < minPocDist)
                minPocDist = abs(tbRefListL1[ref]);
        }

        if (gopB == 4)
        {
            if (minPocDist == 1 || minPocDist == 2 || minPocDist == 4)
                costTableIndex = minPocDist;
        }
        if (gopB == 8)
        {
            if (minPocDist == 1 || minPocDist == 2 || minPocDist == 4 || minPocDist == 8)
                costTableIndex = minPocDist + 3;
        }
    }

    curbe.CostTableIndex = costTableIndex;

    // the following fields are needed by the new optimized kernel in v052417
    curbe.Log2ParallelMergeLevel  = m_hevcPicParams->log2_parallel_merge_level_minus2 + 2;
    curbe.MaxIntraRdeIter         = 1;
    curbe.CornerNeighborPixel     = 0;
    curbe.IntraNeighborAvailFlags = 0;
    curbe.SubPelMode              = 3;  // qual-pel search
    curbe.InterSADMeasure         = 2;  // Haar transform
    curbe.IntraSADMeasure         = 2;  // Haar transform
    curbe.IntraPrediction         = 0;  // enable 32x32, 16x16, and 8x8 luma intra prediction
    curbe.RefIDCostMode           = 1;  // 0: AVC and 1: linear method
    curbe.TUBasedCostSetting      = 0;
    curbe.ConcurrentGroupNum      = m_numberConcurrentGroup;
    curbe.NumofUnitInWaveFront    = m_numWavefrontInOneRegion;
    curbe.LoadBalenceEnable       = 0;  // when this flag is false, kernel does not use LoadBalance (or MBENC_B_FRAME_CONCURRENT_TG_DATA) buffe
    curbe.ThreadNumber            = MOS_MIN(2, m_numberEncKernelSubThread);
    curbe.Pic_init_qp_B           = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.Pic_init_qp_P           = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.Pic_init_qp_I           = m_hevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY;
    curbe.WaveFrontSplitsEnable   = (m_numberConcurrentGroup == 1) ? false : true;
    curbe.SuperHME                = m_16xMeSupported;
    curbe.UltraHME                = m_32xMeSupported;
    curbe.PerBFrameQPOffset       = 0;

    switch (m_hevcSeqParams->TargetUsage)
    {
    case 1:
        curbe.Degree45          = 0;
        curbe.Break12Dependency = 0;
        break;
    case 4:
    default:
        curbe.Degree45          = 1;
        curbe.Break12Dependency = 1;
        break;
    }

    curbe.LongTermReferenceFlags_L0 = 0;
    for (uint32_t i = 0; i < curbe.NumRefIdxL0; i++)
    {
        curbe.LongTermReferenceFlags_L0 |= (m_hevcSliceParams->RefPicList[0][i].PicFlags & PICTURE_LONG_TERM_REFERENCE) << i;
    }
    curbe.LongTermReferenceFlags_L1 = 0;
    for (uint32_t i = 0; i < curbe.NumRefIdxL1; i++)
    {
        curbe.LongTermReferenceFlags_L1 |= (m_hevcSliceParams->RefPicList[1][i].PicFlags & PICTURE_LONG_TERM_REFERENCE) << i;
    }

    curbe.Stepping           = 0;
    curbe.Cu64SkipCheckOnly  = 0;
    curbe.Cu642Nx2NCheckOnly = 0;
    curbe.EnableCu64AmpCheck = 1;
    curbe.IntraSpeedMode     = 0;  // 35 mode
    curbe.DisableIntraNxN    = 0;

    if (m_hwInterface->GetPlatform().usRevId == 0)
    {
        curbe.Stepping               = 1;
        curbe.TUDepthControl         = 1;
        curbe.MaxTransformDepthInter = 1;
        curbe.MaxTransformDepthIntra = 0;
        //buf->curbe.EnableCu64Check       = 1;
        curbe.Cu64SkipCheckOnly  = 0;
        curbe.Cu642Nx2NCheckOnly = 1;
        curbe.EnableCu64AmpCheck = 0;
        curbe.IntraSpeedMode     = 0;  // 35 mode
        curbe.DisableIntraNxN    = 1;
        curbe.MaxNumMergeCand    = 1;
    }

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto buf            = (PMBENC_COMBINED_BUFFER1)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_encBCombinedBuffer1[curIdx].sResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(buf);

    if (curbe.Degree45)
    {
        MOS_ZeroMemory(&buf->concurrent, sizeof(buf->concurrent));
    }
    buf->Curbe = curbe;

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_encBCombinedBuffer1[curIdx].sResource);

    // clean-up the thread dependency buffer in the second combined buffer
    if (m_numberEncKernelSubThread > 1)
    {
        MOS_LOCK_PARAMS lockFlags;

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        auto data           = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_encBCombinedBuffer2[curIdx].sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(&data[m_threadTaskBufferOffset], m_threadTaskBufferSize);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_encBCombinedBuffer2[curIdx].sResource);
    }

    if (m_initEncConstTable)
    {
        // Initialize the Enc Constant Table surface
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        auto data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_encConstantTableForB.sResource,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        if (m_isMaxLcu64)
        {
            MOS_SecureMemcpy(data, m_encConstantTableForB.dwSize, (const void *)m_encLcu64ConstantDataLut, sizeof(m_encLcu64ConstantDataLut));
        }
        else
        {
            MOS_SecureMemcpy(data, m_encConstantTableForB.dwSize, (const void *)m_encLcu32ConstantDataLut, sizeof(m_encLcu32ConstantDataLut));
        }

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_encConstantTableForB.sResource);
        m_initEncConstTable = false;
    }

    // binding table index
    MBENC_COMBINED_BTI params;
    if (m_isMaxLcu64)
    {
        for (uint32_t i = 0; i < MAX_MULTI_FRAME_NUMBER; i++)
        {
            params.BTI_LCU64.Combined1DSurIndexMF1[i]           = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER1;
            params.BTI_LCU64.Combined1DSurIndexMF2[i]           = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER2;
            params.BTI_LCU64.VMEInterPredictionSurfIndexMF[i]   = MBENC_B_FRAME_VME_PRED_CURR_PIC_IDX0;
            params.BTI_LCU64.SrcSurfIndexMF[i]                  = MBENC_B_FRAME_CURR_Y;
            params.BTI_LCU64.SrcReconSurfIndexMF[i]             = MBENC_B_FRAME_CURR_Y_WITH_RECON_BOUNDARY_PIX;
            params.BTI_LCU64.CURecordSurfIndexMF[i]             = MBENC_B_FRAME_ENC_CU_RECORD;
            params.BTI_LCU64.PAKObjectSurfIndexMF[i]            = MBENC_B_FRAME_PAK_OBJ;
            params.BTI_LCU64.CUPacketSurfIndexMF[i]             = MBENC_B_FRAME_PAK_CU_RECORD;
            params.BTI_LCU64.SWScoreBoardSurfIndexMF[i]         = MBENC_B_FRAME_SW_SCOREBOARD;
            params.BTI_LCU64.QPCU16SurfIndexMF[i]               = MBENC_B_FRAME_CU_QP_DATA;
            params.BTI_LCU64.LCULevelDataSurfIndexMF[i]         = MBENC_B_FRAME_LCU_LEVEL_DATA_INPUT;
            params.BTI_LCU64.TemporalMVSurfIndexMF[i]           = MBENC_B_FRAME_COLOCATED_CU_MV_DATA;
            params.BTI_LCU64.HmeDataSurfIndexMF[i]              = MBENC_B_FRAME_HME_MOTION_PREDICTOR_DATA;
            params.BTI_LCU64.VME2XInterPredictionSurfIndexMF[i] = MBENC_B_FRAME_VME_PRED_FOR_2X_DS_CURR;
        }
        params.BTI_LCU64.DebugSurfIndexMF[0]  = MBENC_B_FRAME_DEBUG_SURFACE;
        params.BTI_LCU64.DebugSurfIndexMF[1]  = MBENC_B_FRAME_DEBUG_SURFACE1;
        params.BTI_LCU64.DebugSurfIndexMF[2]  = MBENC_B_FRAME_DEBUG_SURFACE2;
        params.BTI_LCU64.DebugSurfIndexMF[3]  = MBENC_B_FRAME_DEBUG_SURFACE3;
        params.BTI_LCU64.HEVCCnstLutSurfIndex = MBENC_B_FRAME_ENC_CONST_TABLE;
        params.BTI_LCU64.LoadBalenceSurfIndex = MBENC_B_FRAME_CONCURRENT_TG_DATA;
    }
    else
    {
        for (uint32_t i = 0; i < MAX_MULTI_FRAME_NUMBER; i++)
        {
            params.BTI_LCU32.Combined1DSurIndexMF1[i]         = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER1;
            params.BTI_LCU32.Combined1DSurIndexMF2[i]         = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER2;
            params.BTI_LCU32.VMEInterPredictionSurfIndexMF[i] = MBENC_B_FRAME_VME_PRED_CURR_PIC_IDX0;
            params.BTI_LCU32.SrcSurfIndexMF[i]                = MBENC_B_FRAME_CURR_Y;
            params.BTI_LCU32.SrcReconSurfIndexMF[i]           = MBENC_B_FRAME_CURR_Y_WITH_RECON_BOUNDARY_PIX;
            params.BTI_LCU32.CURecordSurfIndexMF[i]           = MBENC_B_FRAME_ENC_CU_RECORD;
            params.BTI_LCU32.PAKObjectSurfIndexMF[i]          = MBENC_B_FRAME_PAK_OBJ;
            params.BTI_LCU32.CUPacketSurfIndexMF[i]           = MBENC_B_FRAME_PAK_CU_RECORD;
            params.BTI_LCU32.SWScoreBoardSurfIndexMF[i]       = MBENC_B_FRAME_SW_SCOREBOARD;
            params.BTI_LCU32.QPCU16SurfIndexMF[i]             = MBENC_B_FRAME_CU_QP_DATA;
            params.BTI_LCU32.LCULevelDataSurfIndexMF[i]       = MBENC_B_FRAME_LCU_LEVEL_DATA_INPUT;
            params.BTI_LCU32.TemporalMVSurfIndexMF[i]         = MBENC_B_FRAME_COLOCATED_CU_MV_DATA;
            params.BTI_LCU32.HmeDataSurfIndexMF[i]            = MBENC_B_FRAME_HME_MOTION_PREDICTOR_DATA;
        }
        params.BTI_LCU32.DebugSurfIndexMF[0]  = MBENC_B_FRAME_DEBUG_SURFACE;
        params.BTI_LCU32.DebugSurfIndexMF[1]  = MBENC_B_FRAME_DEBUG_SURFACE1;
        params.BTI_LCU32.DebugSurfIndexMF[2]  = MBENC_B_FRAME_DEBUG_SURFACE2;
        params.BTI_LCU32.DebugSurfIndexMF[3]  = MBENC_B_FRAME_DEBUG_SURFACE3;
        params.BTI_LCU32.HEVCCnstLutSurfIndex = MBENC_B_FRAME_ENC_CONST_TABLE;
        params.BTI_LCU32.LoadBalenceSurfIndex = MBENC_B_FRAME_CONCURRENT_TG_DATA;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);
    PMHW_KERNEL_STATE kernelState = m_isMaxLcu64 ? &m_mbEncKernelStates[MBENC_LCU64_KRNIDX] : &m_mbEncKernelStates[MBENC_LCU32_KRNIDX];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &params,
        kernelState->dwCurbeOffset,
        sizeof(params)));

    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC BrcInitReset Kernel
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG12::SetCurbeBrcInitReset(
    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    if (brcKrnIdx != CODECHAL_HEVC_BRC_INIT && brcKrnIdx != CODECHAL_HEVC_BRC_RESET)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Brc kernel requested is not init or reset\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Initialize the CURBE data
    BRC_INITRESET_CURBE curbe = m_brcInitResetCurbeInit;

    uint32_t profileLevelMaxFrame = GetProfileLevelMaxFrameSize();

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR ||
        m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR ||
        m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        if (m_hevcSeqParams->InitVBVBufferFullnessInBit == 0)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Initial VBV Buffer Fullness is zero\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_hevcSeqParams->VBVBufferSizeInBit == 0)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("VBV buffer size in bits is zero\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    curbe.DW0_ProfileLevelMaxFrame = profileLevelMaxFrame;
    curbe.DW1_InitBufFull          = m_hevcSeqParams->InitVBVBufferFullnessInBit;
    curbe.DW2_BufSize              = m_hevcSeqParams->VBVBufferSizeInBit;
    curbe.DW3_TargetBitRate        = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;  //DDI in Kbits
    curbe.DW4_MaximumBitRate       = m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    curbe.DW5_MinimumBitRate       = 0;
    curbe.DW6_FrameRateM           = m_hevcSeqParams->FrameRate.Numerator;
    curbe.DW7_FrameRateD           = m_hevcSeqParams->FrameRate.Denominator;
    curbe.DW8_BRCFlag              = BRCINIT_IGNORE_PICTURE_HEADER_SIZE;  // always ignore the picture header size set in BRC Update curbe;

    if (m_hevcPicParams->NumROI)
    {
        curbe.DW8_BRCFlag |= BRCINIT_DISABLE_MBBRC;  // BRC ROI need disable MBBRC logic in LcuBrc Kernel
    }
    else
    {
        curbe.DW8_BRCFlag |= (m_lcuBrcEnabled) ? 0 : BRCINIT_DISABLE_MBBRC;
    }

    curbe.DW8_BRCFlag |= (m_brcEnabled && m_numPipe > 1) ? BRCINIT_USEHUCBRC : 0;
    // For non-ICQ, ACQP Buffer always set to 1
    curbe.DW25_ACQPBuffer        = 1;
    curbe.DW25_SlidingWindowSize = m_slidingWindowSize;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        curbe.DW4_MaximumBitRate = curbe.DW3_TargetBitRate;
        curbe.DW8_BRCFlag |= BRCINIT_ISCBR;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (curbe.DW4_MaximumBitRate < curbe.DW3_TargetBitRate)
        {
            curbe.DW4_MaximumBitRate = 2 * curbe.DW3_TargetBitRate;
        }
        curbe.DW8_BRCFlag |= BRCINIT_ISVBR;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe.DW8_BRCFlag |= BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        curbe.DW3_TargetBitRate  = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;  //DDI in Kbits
        curbe.DW4_MaximumBitRate = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        curbe.DW8_BRCFlag |= BRCINIT_ISICQ;
        curbe.DW25_ACQPBuffer = m_hevcSeqParams->ICQQualityFactor;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM)
    {
        curbe.DW4_MaximumBitRate = curbe.DW3_TargetBitRate;
        curbe.DW8_BRCFlag |= BRCINIT_ISVCM;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.DW8_BRCFlag = BRCINIT_ISCQP;
    }
    else if (m_hevcSeqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (curbe.DW4_MaximumBitRate < curbe.DW3_TargetBitRate)
        {
            curbe.DW4_MaximumBitRate = curbe.DW3_TargetBitRate;  // Use max bit rate for HRD compliance
        }
        curbe.DW8_BRCFlag = curbe.DW8_BRCFlag | BRCINIT_ISQVBR | BRCINIT_ISVBR;  // We need to make sure that VBR is used for QP determination.
        // use ICQQualityFactor to determine the larger Qp for each MB
        curbe.DW25_ACQPBuffer = m_hevcSeqParams->ICQQualityFactor;
    }
    curbe.DW9_FrameWidth       = m_oriFrameWidth;
    curbe.DW10_FrameHeight     = m_oriFrameHeight;
    curbe.DW10_AVBRAccuracy    = m_usAvbrAccuracy;
    curbe.DW11_AVBRConvergence = m_usAvbrConvergence;
    curbe.DW12_NumberSlice     = m_numSlices;

    /**********************************************************************
    In case of non-HB/BPyramid Structure
    BRC_Param_A = GopP
    BRC_Param_B = GopB
    In case of HB/BPyramid GOP Structure
    BRC_Param_A, BRC_Param_B, BRC_Param_C, BRC_Param_D are
    BRC Parameters set as follows as per CModel equation
    ***********************************************************************/
    // BPyramid GOP
    if (m_HierchGopBRCEnabled)
    {
        curbe.DW8_BRCGopP   = ((m_hevcSeqParams->GopPicSize + m_hevcSeqParams->GopRefDist - 1) / m_hevcSeqParams->GopRefDist);
        curbe.DW9_BRCGopB   = curbe.DW8_BRCGopP;
        curbe.DW13_BRCGopB1 = curbe.DW8_BRCGopP * 2;
        curbe.DW14_BRCGopB2 = ((m_hevcSeqParams->GopPicSize) - (curbe.DW8_BRCGopP) - (curbe.DW13_BRCGopB1) - (curbe.DW9_BRCGopB));
        // B1 Level GOP
        if (m_hevcSeqParams->GopRefDist <= 4 || curbe.DW14_BRCGopB2 == 0)
        {
            curbe.DW14_MaxBRCLevel = 3;
        }
        // B2 Level GOP
        else
        {
            curbe.DW14_MaxBRCLevel = 4;
        }
    }
    // For Regular GOP - No BPyramid
    else
    {
        curbe.DW14_MaxBRCLevel = 1;
        curbe.DW8_BRCGopP      = (m_hevcSeqParams->GopRefDist) ? ((m_hevcSeqParams->GopPicSize - 1) / m_hevcSeqParams->GopRefDist) : 0;
        curbe.DW9_BRCGopB      = m_hevcSeqParams->GopPicSize - 1 - curbe.DW8_BRCGopP;
    }

    // Set dynamic thresholds
    double inputBitsPerFrame = (double)((double)curbe.DW4_MaximumBitRate * (double)curbe.DW7_FrameRateD);
    inputBitsPerFrame        = (double)(inputBitsPerFrame / curbe.DW6_FrameRateM);

    if (curbe.DW2_BufSize < (uint32_t)inputBitsPerFrame * 4)
    {
        curbe.DW2_BufSize = (uint32_t)inputBitsPerFrame * 4;
    }

    if (curbe.DW1_InitBufFull == 0)
    {
        curbe.DW1_InitBufFull = 7 * curbe.DW2_BufSize / 8;
    }
    if (curbe.DW1_InitBufFull < (uint32_t)(inputBitsPerFrame * 2))
    {
        curbe.DW1_InitBufFull = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (curbe.DW1_InitBufFull > curbe.DW2_BufSize)
    {
        curbe.DW1_InitBufFull = curbe.DW2_BufSize;
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        curbe.DW2_BufSize     = 2 * m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;
        curbe.DW1_InitBufFull = (uint32_t)(0.75 * curbe.DW2_BufSize);
    }

    if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
    {
        curbe.DW15_LongTermInterval = 0;  // no LTR for low delay brc
    }
    else
    {
        curbe.DW15_LongTermInterval = (m_enableBrcLTR && m_ltrInterval) ? m_ltrInterval : m_enableBrcLTR ? HEVC_BRC_LONG_TERM_REFRENCE_FLAG : 0;
    }

    double bpsRatio = ((double)inputBitsPerFrame / ((double)(curbe.DW2_BufSize) / 30));
    bpsRatio        = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    curbe.DW19_DeviationThreshold0_PBframe = (uint32_t)(-50 * pow(0.90, bpsRatio));
    curbe.DW19_DeviationThreshold1_PBframe = (uint32_t)(-50 * pow(0.66, bpsRatio));
    curbe.DW19_DeviationThreshold2_PBframe = (uint32_t)(-50 * pow(0.46, bpsRatio));
    curbe.DW19_DeviationThreshold3_PBframe = (uint32_t)(-50 * pow(0.3, bpsRatio));

    curbe.DW20_DeviationThreshold4_PBframe = (uint32_t)(50 * pow(0.3, bpsRatio));
    curbe.DW20_DeviationThreshold5_PBframe = (uint32_t)(50 * pow(0.46, bpsRatio));
    curbe.DW20_DeviationThreshold6_PBframe = (uint32_t)(50 * pow(0.7, bpsRatio));
    curbe.DW20_DeviationThreshold7_PBframe = (uint32_t)(50 * pow(0.9, bpsRatio));

    curbe.DW21_DeviationThreshold0_VBRcontrol = (uint32_t)(-50 * pow(0.9, bpsRatio));
    curbe.DW21_DeviationThreshold1_VBRcontrol = (uint32_t)(-50 * pow(0.7, bpsRatio));
    curbe.DW21_DeviationThreshold2_VBRcontrol = (uint32_t)(-50 * pow(0.5, bpsRatio));
    curbe.DW21_DeviationThreshold3_VBRcontrol = (uint32_t)(-50 * pow(0.3, bpsRatio));

    curbe.DW22_DeviationThreshold4_VBRcontrol = (uint32_t)(100 * pow(0.4, bpsRatio));
    curbe.DW22_DeviationThreshold5_VBRcontrol = (uint32_t)(100 * pow(0.5, bpsRatio));
    curbe.DW22_DeviationThreshold6_VBRcontrol = (uint32_t)(100 * pow(0.75, bpsRatio));
    curbe.DW22_DeviationThreshold7_VBRcontrol = (uint32_t)(100 * pow(0.9, bpsRatio));

    curbe.DW23_DeviationThreshold0_Iframe = (uint32_t)(-50 * pow(0.8, bpsRatio));
    curbe.DW23_DeviationThreshold1_Iframe = (uint32_t)(-50 * pow(0.6, bpsRatio));
    curbe.DW23_DeviationThreshold2_Iframe = (uint32_t)(-50 * pow(0.34, bpsRatio));
    curbe.DW23_DeviationThreshold3_Iframe = (uint32_t)(-50 * pow(0.2, bpsRatio));

    curbe.DW24_DeviationThreshold4_Iframe = (uint32_t)(50 * pow(0.2, bpsRatio));
    curbe.DW24_DeviationThreshold5_Iframe = (uint32_t)(50 * pow(0.4, bpsRatio));
    curbe.DW24_DeviationThreshold6_Iframe = (uint32_t)(50 * pow(0.66, bpsRatio));
    curbe.DW24_DeviationThreshold7_Iframe = (uint32_t)(50 * pow(0.9, bpsRatio));

    if (m_hevcSeqParams->HierarchicalFlag && !m_hevcSeqParams->LowDelayMode &&
        (m_hevcSeqParams->GopRefDist == 4 || m_hevcSeqParams->GopRefDist == 8))
    {
        curbe.DW26_RandomAccess = true;
    }
    else
    {
        curbe.DW26_RandomAccess = false;
    }

    if (m_brcInit)
    {
        m_dBrcInitCurrentTargetBufFullInBits = curbe.DW1_InitBufFull;
    }

    m_brcInitResetBufSizeInBits      = curbe.DW2_BufSize;
    m_dBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

// ------------------------------------------------------------------------------
//| Purpose:    Setup curbe for HEVC BrcUpdate Kernel
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncHevcStateG12::SetCurbeBrcUpdate(
    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (brcKrnIdx != CODECHAL_HEVC_BRC_FRAME_UPDATE && brcKrnIdx != CODECHAL_HEVC_BRC_LCU_UPDATE)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Brc kernel requested is not frame update or LCU update\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    // Initialize the CURBE data
    BRCUPDATE_CURBE curbe = m_brcUpdateCurbeInit;

    curbe.DW5_TargetSize_Flag = 0;

    if (m_dBrcInitCurrentTargetBufFullInBits > (double)m_brcInitResetBufSizeInBits)
    {
        m_dBrcInitCurrentTargetBufFullInBits -= (double)m_brcInitResetBufSizeInBits;
        curbe.DW5_TargetSize_Flag = 1;
    }

    if (m_numSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        curbe.DW6_NumSkippedFrames     = m_numSkipFrames;
        curbe.DW15_SizeOfSkippedFrames = m_sizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame * m_numSkipFrames;
    }

    curbe.DW0_TargetSize  = (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits);
    curbe.DW1_FrameNumber = m_storeData - 1;  // Check if we can remove this (set to 0)

    // BRC PAK statistic buffer from last frame, the encoded size includes header already.
    // in BRC Initreset kernel, curbe DW8_BRCFlag will always ignore picture header size, so no need to set picture header size here.
    curbe.DW2_PictureHeaderSize = 0;
    curbe.DW5_CurrFrameBrcLevel = m_currFrameBrcLevel;
    curbe.DW5_MaxNumPAKs        = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
    {
        curbe.DW6_CqpValue = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    }
    if (m_hevcPicParams->NumROI)
    {
        curbe.DW6_ROIEnable    = m_brcEnabled ? false : true;
        curbe.DW6_BRCROIEnable = m_brcEnabled ? true : false;
        curbe.DW6_RoiRatio     = CalculateROIRatio();
    }
    curbe.DW6_SlidingWindowEnable = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);

    //for low delay brc
    curbe.DW6_LowDelayEnable    = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);
    curbe.DW16_UserMaxFrameSize = GetProfileLevelMaxFrameSize();
    curbe.DW14_ParallelMode     = m_hevcSeqParams->ParallelBRC;

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe.DW3_StartGAdjFrame0 = (uint32_t)((10 * m_usAvbrConvergence) / (double)150);
        curbe.DW3_StartGAdjFrame1 = (uint32_t)((50 * m_usAvbrConvergence) / (double)150);
        curbe.DW4_StartGAdjFrame2 = (uint32_t)((100 * m_usAvbrConvergence) / (double)150);
        curbe.DW4_StartGAdjFrame3 = (uint32_t)((150 * m_usAvbrConvergence) / (double)150);

        curbe.DW11_gRateRatioThreshold0 =
            (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 40)));
        curbe.DW11_gRateRatioThreshold1 =
            (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 75)));
        curbe.DW12_gRateRatioThreshold2 = (uint32_t)((100 - (m_usAvbrAccuracy / (double)30) * (100 - 97)));
        curbe.DW12_gRateRatioThreshold3 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (103 - 100)));
        curbe.DW12_gRateRatioThreshold4 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (125 - 100)));
        curbe.DW12_gRateRatioThreshold5 = (uint32_t)((100 + (m_usAvbrAccuracy / (double)30) * (160 - 100)));
    }

    if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
    {
        curbe.DW17_LongTerm_Current = 0;  // no LTR for low delay brc
    }
    else
    {
        m_isFrameLTR                = (CodecHal_PictureIsLongTermRef(m_currReconstructedPic));
        curbe.DW17_LongTerm_Current = (m_enableBrcLTR && m_isFrameLTR) ? 1 : 0;
    }

    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SendMbEncSurfacesIKernel(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t                               startBTI = 0, mbenc_I_KRNIDX = MBENC_LCU32_KRNIDX;
    CODECHAL_SURFACE_CODEC_PARAMS          surfaceCodecParams;
    PMOS_SURFACE                           inputSurface = m_rawSurfaceToEnc;
    PMHW_KERNEL_STATE                      kernelState  = &m_mbEncKernelStates[mbenc_I_KRNIDX];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_mbEncKernelBindingTable[mbenc_I_KRNIDX];

    // Combined 1D buffer 1, which contains regular kernel curbe and concurrent map
    startBTI = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource,
        m_encBCombinedBuffer1[m_currRecycledBufIdx].dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "Hevc_CombinedBuffer1",
            m_encBCombinedBuffer1[m_currRecycledBufIdx].dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_I_MBENC)););

    // VME surfaces
    startBTI = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Programming dummy surfaces even if not used (VME requirement), currently setting to input surface
    for (int32_t surface_idx = 0; surface_idx < 8; surface_idx++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
            &surfaceCodecParams,
            inputSurface,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBTI++]));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    //Source Y and UV
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        false));

    surfaceCodecParams.bUseUVPlane = true;

    surfaceCodecParams.dwUVBindingTableOffset = bindingTable->dwBindingTableEntries[startBTI++];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            inputSurface,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "MbEnc_Input_SrcSurf")));
    // Current Y with reconstructed boundary pixels
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_currPicWithReconBoundaryPix,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Enc CU Record
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_intermediateCuRecordSurfaceLcu32,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK object command surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        m_mvOffset,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU packet for PAK surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        m_mbCodeSize - m_mvOffset,
        m_mvOffset,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    //Software scoreboard surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        m_swScoreboardState->GetCurSwScoreboardSurface(),
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        true));

    surfaceCodecParams.bUse32UINTSurfaceFormat = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Scratch surface for Internal Use Only
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_scratchSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU 16x16 QP data input surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Lcu level data input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_lcuLevelInputDataSurface[m_currRecycledBufIdx],
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Enc I Constant Table surface // CostLUT Buf
    startBTI = MBENC_I_FRAME_ENC_CONST_TABLE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encConstantTableForB.sResource,
        m_encConstantTableForB.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

#if 0
    // Concurrent Thread Group Data surface
    startBTI = MBENC_I_FRAME_CONCURRENT_TG_DATA;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &resConcurrentThreadGroupData.sResource,
        resConcurrentThreadGroupData.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));
#endif

    // Brc Combined Enc parameter surface
    startBTI = MBENC_I_FRAME_BRC_COMBINED_ENC_PARAMETER_SURFACE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcInputForEncKernelBuffer->sResource,
        HEVC_FRAMEBRC_BUF_CONST_SIZE,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Kernel debug surface
    startBTI = MBENC_I_FRAME_DEBUG_DUMP;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_debugSurface[0].sResource,
        m_debugSurface[0].dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SendMbEncSurfacesBKernel(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelStates);
    PMHW_KERNEL_STATE kernelState = m_isMaxLcu64 ? &m_mbEncKernelStates[MBENC_LCU64_KRNIDX] : &m_mbEncKernelStates[MBENC_LCU32_KRNIDX];

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mbEncKernelBindingTable);
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = m_isMaxLcu64 ? &m_mbEncKernelBindingTable[MBENC_LCU64_KRNIDX] : &m_mbEncKernelBindingTable[MBENC_LCU32_KRNIDX];

    PMOS_SURFACE                  inputSurface = m_rawSurfaceToEnc;
    uint32_t                      startBTI     = MBENC_B_FRAME_VME_PRED_CURR_PIC_IDX0;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;

    // Combined 1D buffer 1, which contains regular kernel curbe and concurrent map
    startBTI = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource,
        m_encBCombinedBuffer1[m_currRecycledBufIdx].dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "Hevc_CombinedBuffer1",
            m_encBCombinedBuffer1[m_currRecycledBufIdx].dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC)););
    // Combined 1D buffer 2, which contains non fixed sizes of buffers
    startBTI = MBENC_B_FRAME_ENCODER_COMBINED_BUFFER2;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
        m_encBCombinedBuffer2[m_currRecycledBufIdx].dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));
    surfaceCodecParams.bRawSurface = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
            CodechalDbgAttr::attrOutput,
            "Hevc_CombinedBuffer2",
            m_encBCombinedBuffer2[m_currRecycledBufIdx].dwSize,
            0,
            CODECHAL_MEDIA_STATE_HEVC_B_MBENC)););
    // VME surfaces
    startBTI = MBENC_B_FRAME_VME_PRED_CURR_PIC_IDX0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    for (int32_t surface_idx = 0; surface_idx < 4; surface_idx++)
    {
        int32_t       ll     = 0;
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            int32_t      idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
            PMOS_SURFACE refSurfacePtr;
            if (surface_idx == 0 && m_useWeightedSurfaceForL0)
            {
                refSurfacePtr = m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L0_START + surface_idx);
            }
            else
            {
                refSurfacePtr = &m_refList[idx]->sRefBuffer;
            }

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                refSurfacePtr,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBTI++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &m_refList[idx]->sRefBuffer,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data())));
        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                inputSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBTI++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        ll     = 1;
        refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
        {
            int32_t      idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
            PMOS_SURFACE refSurfacePtr;
            if (surface_idx == 0 && m_useWeightedSurfaceForL1)
            {
                refSurfacePtr = m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L1_START + surface_idx);
            }
            else
            {
                refSurfacePtr = &m_refList[idx]->sRefBuffer;
            }

            // Picture Y VME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                refSurfacePtr,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBTI++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            CODECHAL_DEBUG_TOOL(
                m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &m_refList[idx]->sRefBuffer,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data())));
        }
        else
        {
            // Providing Dummy surface as per VME requirement.
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                &surfaceCodecParams,
                inputSurface,
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                bindingTable->dwBindingTableEntries[startBTI++]));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    //Source Y and UV
    startBTI = MBENC_B_FRAME_CURR_Y;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        inputSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        false));

    surfaceCodecParams.bUseUVPlane = true;

    surfaceCodecParams.dwUVBindingTableOffset = bindingTable->dwBindingTableEntries[startBTI];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            inputSurface,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "MbEnc_Input_SrcSurf")));

    // Current Y with reconstructed boundary pixels
    startBTI = MBENC_B_FRAME_CURR_Y_WITH_RECON_BOUNDARY_PIX;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_currPicWithReconBoundaryPix,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Enc CU Record
    startBTI = MBENC_B_FRAME_ENC_CU_RECORD;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_intermediateCuRecordSurfaceLcu32,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        0,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK object command surface
    startBTI = MBENC_B_FRAME_PAK_OBJ;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        m_mvOffset,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU packet for PAK surface
    startBTI = MBENC_B_FRAME_PAK_CU_RECORD;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbCodeSurface,
        m_mbCodeSize - m_mvOffset,
        m_mvOffset,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    //Software scoreboard surface
    startBTI = MBENC_B_FRAME_SW_SCOREBOARD;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        m_swScoreboardState->GetCurSwScoreboardSurface(),
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        m_verticalLineStride,
        true));

    surfaceCodecParams.bUse32UINTSurfaceFormat = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Scratch surface for Internal Use Only
    startBTI = MBENC_B_FRAME_SCRATCH_SURFACE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_scratchSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // CU 16x16 QP data input surface
    startBTI = MBENC_B_FRAME_CU_QP_DATA;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        m_verticalLineStride,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Lcu level data input
    startBTI = MBENC_B_FRAME_LCU_LEVEL_DATA_INPUT;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_lcuLevelInputDataSurface[m_currRecycledBufIdx],
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        m_verticalLineStride,
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Enc B 32x32 Constant Table surface
    startBTI = MBENC_B_FRAME_ENC_CONST_TABLE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_encConstantTableForB.sResource,
        m_encConstantTableForB.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Colocated CU Motion Vector Data surface
    startBTI                    = MBENC_B_FRAME_COLOCATED_CU_MV_DATA;
    uint8_t mbCodeIdxForTempMVP = 0xFF;
    if (m_hevcPicParams->CollocatedRefPicIndex != 0xFF && m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
    {
        uint8_t frameIdx = m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].FrameIdx;

        mbCodeIdxForTempMVP = m_refList[frameIdx]->ucScalingIdx;
    }

    if (m_pictureCodingType == I_TYPE)
    {
        // No temoporal MVP in the I frame
        m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
    }
    else
    {
        if (mbCodeIdxForTempMVP == 0xFF && m_hevcSliceParams->slice_temporal_mvp_enable_flag)
        {
            // Temporal reference MV index is invalid and so disable the temporal MVP
            CODECHAL_ENCODE_ASSERT(false);
            m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
        }
    }

    if (mbCodeIdxForTempMVP == 0xFF)
    {
        startBTI++;
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            m_trackedBuf->GetMvTemporalBuffer(mbCodeIdxForTempMVP),
            m_sizeOfMvTemporalBuffer,
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBTI++],
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    startBTI = MBENC_B_FRAME_HME_MOTION_PREDICTOR_DATA;

    // HME motion predictor data
    if (m_hmeEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer),
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBTI++],
            m_verticalLineStride,
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        startBTI++;
    }

    // Brc Combined Enc parameter surface
    startBTI = MBENC_B_FRAME_BRC_COMBINED_ENC_PARAMETER_SURFACE;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcInputForEncKernelBuffer->sResource,
        HEVC_FRAMEBRC_BUF_CONST_SIZE,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        false));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    startBTI = MBENC_B_FRAME_VME_PRED_FOR_2X_DS_CURR;
    if (m_isMaxLcu64)
    {
        PMOS_SURFACE currScaledSurface2x = m_trackedBuf->Get2xDsSurface(CODEC_CURR_TRACKED_BUFFER);

        //VME 2X Inter prediction surface for current frame
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
            &surfaceCodecParams,
            currScaledSurface2x,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBTI++]));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                currScaledSurface2x,
                CodechalDbgAttr::attrReferenceSurfaces,
                "2xScaledSurf")));

        // RefFrame's 2x DS surface
        for (int32_t surface_idx = 0; surface_idx < 4; surface_idx++)
        {
            int32_t       ll     = 0;
            CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                int32_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

                // Picture Y VME
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx),
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBTI++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                CODECHAL_DEBUG_TOOL(
                    m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                    std::string refSurfName      = "Ref2xScaledSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx),
                        CodechalDbgAttr::attrReferenceSurfaces,
                        refSurfName.data())));
            }
            else
            {
                // Providing Dummy surface as per VME requirement.
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    currScaledSurface2x,
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBTI++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            ll     = 1;
            refPic = m_hevcSliceParams->RefPicList[ll][surface_idx];
            if (!CodecHal_PictureIsInvalid(refPic) &&
                !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
            {
                int32_t idx = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

                // Picture Y VME
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx),
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBTI++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                CODECHAL_DEBUG_TOOL(
                    m_debugInterface->m_refIndex = (uint16_t)refPic.FrameIdx;
                    std::string refSurfName      = "Ref2xScaledSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        m_trackedBuf->Get2xDsSurface(m_refList[idx]->ucScalingIdx),
                        CodechalDbgAttr::attrReferenceSurfaces,
                        refSurfName.data())));
            }
            else
            {
                // Providing Dummy surface as per VME requirement.
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParamsVME(
                    &surfaceCodecParams,
                    currScaledSurface2x,
                    m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value,
                    bindingTable->dwBindingTableEntries[startBTI++]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }

    // Encoder History Input Buffer
    startBTI = MBENC_B_FRAME_ENCODER_HISTORY_INPUT_BUFFER;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_encoderHistoryInputBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Encoder History Output Buffer
    startBTI = MBENC_B_FRAME_ENCODER_HISTORY_OUTPUT_BUFFER;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_encoderHistoryOutputBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        m_verticalLineStride,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Kernel debug surface
    startBTI = MBENC_B_FRAME_DEBUG_SURFACE;
    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_debugSurface); i++, startBTI++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_debugSurface[i].sResource,
            m_debugSurface[i].dwSize,
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBTI],
            false));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SendBrcInitResetSurfaces(
    PMOS_COMMAND_BUFFER      cmdBuffer,
    CODECHAL_HEVC_BRC_KRNIDX krnIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (krnIdx != CODECHAL_HEVC_BRC_INIT && krnIdx != CODECHAL_HEVC_BRC_RESET)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Brc kernel requested is not init or reset\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_brcKernelBindingTable[krnIdx];
    uint32_t                               startBti     = 0;
    CODECHAL_SURFACE_CODEC_PARAMS          surfaceCodecParams;
    // BRC History Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcHistoryBuffer,
        m_brcHistoryBufferSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));

    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[krnIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Distortion Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        m_brcDistortion,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetupBrcConstantTable(
    PMOS_SURFACE brcConstantData)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *outputData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &brcConstantData->OsResource, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(outputData);
    uint8_t *inputData  = (uint8_t *)g_cInit_HEVC_BRC_QP_ADJUST;
    uint32_t inputSize  = sizeof(g_cInit_HEVC_BRC_QP_ADJUST);
    uint32_t outputSize = brcConstantData->dwHeight * brcConstantData->dwPitch;

    // 576-byte of Qp adjust table
    while ((inputSize >= brcConstantData->dwWidth) && (outputSize >= brcConstantData->dwWidth))
    {
        MOS_SecureMemcpy(outputData, outputSize, inputData, brcConstantData->dwWidth);
        outputData += brcConstantData->dwPitch;
        outputSize -= brcConstantData->dwPitch;
        inputData += brcConstantData->dwWidth;
        inputSize -= brcConstantData->dwWidth;
    }
    //lambda and mode cost
    if (m_isMaxLcu64)
    {
        inputData = (uint8_t *)m_brcLcu64x64LambdaModeCostInit;
        inputSize = sizeof(m_brcLcu64x64LambdaModeCostInit);
    }
    else
    {
        inputData = (uint8_t *)m_brcLcu32x32LambdaModeCostInit;
        inputSize = sizeof(m_brcLcu32x32LambdaModeCostInit);
    }

    while ((inputSize >= brcConstantData->dwWidth) && (outputSize >= brcConstantData->dwWidth))
    {
        MOS_SecureMemcpy(outputData, outputSize, inputData, brcConstantData->dwWidth);
        outputData += brcConstantData->dwPitch;
        outputSize -= brcConstantData->dwPitch;
        inputData += brcConstantData->dwWidth;
        inputSize -= brcConstantData->dwWidth;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &brcConstantData->OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SendBrcFrameUpdateSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Fill HCP_IMG_STATE so that BRC kernel can use it to generate the write buffer for PAK
    PMOS_RESOURCE            brcHcpStateReadBuffer = &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx];
    MHW_VDBOX_HEVC_PIC_STATE mhwHevcPicState;
    mhwHevcPicState.pHevcEncSeqParams     = m_hevcSeqParams;
    mhwHevcPicState.pHevcEncPicParams     = m_hevcPicParams;
    mhwHevcPicState.bUseVDEnc             = m_vdencEnabled ? 1 : 0;
    mhwHevcPicState.brcNumPakPasses       = m_mfxInterface->GetBrcNumPakPasses();
    mhwHevcPicState.sseEnabledInVmeEncode = m_sseEnabled;
    mhwHevcPicState.rhodomainRCEnable     = m_brcEnabled && (m_numPipe > 1);
    mhwHevcPicState.bSAOEnable            = m_hevcSeqParams->SAO_enabled_flag ? (m_hevcSliceParams->slice_sao_luma_flag || m_hevcSliceParams->slice_sao_chroma_flag) : 0;
    mhwHevcPicState.bTransformSkipEnable  = m_hevcPicParams->transform_skip_enabled_flag;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcPicBrcBuffer(brcHcpStateReadBuffer, &mhwHevcPicState));

    PMOS_SURFACE brcConstantData = &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBrcConstantTable(brcConstantData));

    uint32_t                               startBti     = 0;
    PMHW_KERNEL_STATE                      kernelState  = &m_brcKernelStates[CODECHAL_HEVC_BRC_FRAME_UPDATE];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_brcKernelBindingTable[CODECHAL_HEVC_BRC_FRAME_UPDATE];
    CODECHAL_SURFACE_CODEC_PARAMS          surfaceCodecParams;

    // BRC History Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcHistoryBuffer,
        m_brcHistoryBufferSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Prev PAK statistics output buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead],
        m_hevcBrcPakStatisticsSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC HCP_PIC_STATE read
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        brcHcpStateReadBuffer,
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC HCP_PIC_STATE write
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
        m_brcBuffers.dwBrcHcpPicStateSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Combined ENC-parameter buffer
    startBti++;

    // BRC Distortion Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        m_brcDistortion,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Data Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        brcConstantData,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        0,
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Pixel MB Statistics surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_resMbStatsBuffer,
        m_hwInterface->m_avcMbStatBufferSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Mv and Distortion summation surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
        &surfaceCodecParams,
        &m_mvAndDistortionSumSurface.sResource,
        m_mvAndDistortionSumSurface.dwSize,
        0,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBti++],
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_mvAndDistortionSumSurface.sResource,
            CodechalDbgAttr::attrInput,
            "MvDistSum",
            m_mvAndDistortionSumSurface.dwSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx],
            CodechalDbgAttr::attrInput,
            "ImgStateRead",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
            &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx],
            CodechalDbgAttr::attrInput,
            "ConstData",
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        // PAK statistics buffer is only dumped for BrcUpdate kernel input
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForRead],
            CodechalDbgAttr::attrInput,
            "PakStats",
            HEVC_BRC_PAK_STATISTCS_SIZE,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        // HEVC maintains a ptr to its own distortion surface, as it may be a couple different surfaces
        if (m_brcDistortion) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_debugInterface->DumpBuffer(
                    &m_brcDistortion->OsResource,
                    CodechalDbgAttr::attrInput,
                    "BrcDist_BeforeFrameBRC",
                    m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                    m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                    CODECHAL_MEDIA_STATE_BRC_UPDATE));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(&m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrInput,
            "HistoryRead_beforeFramBRC",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        if (m_brcBuffers.pMbEncKernelStateInUse) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                CODECHAL_MEDIA_STATE_BRC_UPDATE,
                m_brcBuffers.pMbEncKernelStateInUse));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(&m_resMbStatsBuffer,
            CodechalDbgAttr::attrInput,
            "MBStatsSurf",
            m_hwInterface->m_avcMbStatBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));)
    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SendBrcLcuUpdateSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PMHW_KERNEL_STATE                      kernelState  = &m_brcKernelStates[CODECHAL_HEVC_BRC_LCU_UPDATE];
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable = &m_brcKernelBindingTable[CODECHAL_HEVC_BRC_LCU_UPDATE];
    uint32_t                               startBTI     = 0;
    CODECHAL_SURFACE_CODEC_PARAMS          surfaceCodecParams;

    if (m_brcEnabled)
    {
        // BRC History Buffer
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_brcBuffers.resBrcHistoryBuffer,
            m_brcHistoryBufferSize,
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBTI++],
            true));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // BRC Distortion Surface - Intra or Inter
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
            &surfaceCodecParams,
            m_brcDistortion,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value,
            bindingTable->dwBindingTableEntries[startBTI++],
            0,
            true));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // Pixel MB Statistics surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams1D(
            &surfaceCodecParams,
            &m_resMbStatsBuffer,
            m_hwInterface->m_avcMbStatBufferSize,
            0,
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
            bindingTable->dwBindingTableEntries[startBTI++],
            false));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        // CQP ROI
        startBTI += 3;
    }
    // MB QP surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcMbQpBuffer,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        0,
        true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // ROI surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceCodecParams2D(
        &surfaceCodecParams,
        &m_brcBuffers.sBrcRoiSurface,
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ROI_ENCODE].Value,
        bindingTable->dwBindingTableEntries[startBTI++],
        0,
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::GetCustomDispatchPattern(
    PMHW_WALKER_PARAMS            walkerParams,
    PCODECHAL_WALKER_CODEC_PARAMS walkerCodecParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(walkerParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(walkerCodecParams);

    MOS_ZeroMemory(walkerParams, sizeof(*walkerParams));

    walkerParams->WalkerMode = (MHW_WALKER_MODE)walkerCodecParams->WalkerMode;

    walkerParams->dwLocalLoopExecCount  = 0xFFFF;  //MAX VALUE
    walkerParams->dwGlobalLoopExecCount = 0xFFFF;  //MAX VALUE

    // the following code is copied from the kernel ULT
    uint32_t maxThreadWidth, maxThreadHeight;
    uint32_t threadSpaceWidth, threadSpaceHeight, concurGroupNum, threadScaleV;

    threadSpaceWidth  = walkerCodecParams->dwResolutionX;
    threadSpaceHeight = walkerCodecParams->dwResolutionY;
    maxThreadWidth    = threadSpaceWidth;
    maxThreadHeight   = threadSpaceHeight;
    concurGroupNum    = m_numberConcurrentGroup;
    threadScaleV      = m_numberEncKernelSubThread;

    if (concurGroupNum > 1)
    {
        maxThreadWidth  = threadSpaceWidth;
        maxThreadHeight = threadSpaceWidth + (threadSpaceWidth + threadSpaceHeight + concurGroupNum - 2) / concurGroupNum;
        maxThreadHeight *= threadScaleV;
        maxThreadHeight += 1;
    }
    else
    {
        threadSpaceHeight *= threadScaleV;
        maxThreadHeight *= threadScaleV;
    }

    uint32_t localLoopExecCount = m_degree45Needed ? (2 * m_numWavefrontInOneRegion + 1) : m_numWavefrontInOneRegion;

    eStatus = InitMediaObjectWalker(maxThreadWidth,
        maxThreadHeight,
        concurGroupNum - 1,
        m_swScoreboardState->GetDependencyPattern(),
        m_numberEncKernelSubThread - 1,
        localLoopExecCount,
        *walkerParams);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::GenerateLcuLevelData(MOS_SURFACE &lcuLevelInputDataSurfaceParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_tileParams);

    uint32_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    uint32_t numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;

    uint32_t shift    = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
    uint32_t residual = (1 << shift) - 1;

    uint32_t frameWidthInLcu  = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1 + residual) >> shift;
    uint32_t frameHeightInLcu = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1 + residual) >> shift;

    PLCU_LEVEL_DATA *lcuInfo = (PLCU_LEVEL_DATA *)MOS_AllocMemory(sizeof(PLCU_LEVEL_DATA) * frameWidthInLcu);
    CODECHAL_ENCODE_CHK_NULL_RETURN(lcuInfo);
    for (uint32_t i = 0; i < frameWidthInLcu; i++)
    {
        lcuInfo[i] = (PLCU_LEVEL_DATA)MOS_AllocMemory(sizeof(LCU_LEVEL_DATA) * frameHeightInLcu);
        if (lcuInfo[i] == nullptr)
        {
            for (uint32_t j = 0; j < i; j++)
            {
                MOS_FreeMemory(lcuInfo[j]);
            }
            MOS_FreeMemory(lcuInfo);
            CODECHAL_ENCODE_CHK_NULL_RETURN(nullptr);
        }
        MOS_ZeroMemory(lcuInfo[i], (sizeof(LCU_LEVEL_DATA) * frameHeightInLcu));
    }

    // Tiling case
    if (numTileColumns > 1 || numTileRows > 1)
    {
        // This assumes that the entire Slice is contained within a Tile
        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                uint32_t                             tileId      = tileRow * numTileColumns + tileCol;
                MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 currentTile = m_tileParams[tileId];

                uint32_t tileColumnWidth = (currentTile.TileWidthInMinCbMinus1 + 1 + residual) >> shift;
                uint32_t tileRowHeight   = (currentTile.TileHeightInMinCbMinus1 + 1 + residual) >> shift;

                for (uint32_t startLCU = 0, sliceStartLcu = 0, slcCount = 0; slcCount < m_numSlices; slcCount++)
                {
                    bool lastSliceInTile = false, sliceInTile = false;

                    eStatus = (MOS_STATUS)IsSliceInTile(slcCount,
                        &currentTile,
                        &sliceInTile,
                        &lastSliceInTile);
                    if (eStatus != MOS_STATUS_SUCCESS)
                    {
                        for (uint32_t i = 0; i < frameWidthInLcu; i++)
                        {
                            MOS_FreeMemory(lcuInfo[i]);
                        }
                        MOS_FreeMemory(lcuInfo);
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                    }

                    if (!sliceInTile)
                    {
                        startLCU += m_hevcSliceParams[slcCount].NumLCUsInSlice;
                        continue;
                    }

                    sliceStartLcu      = m_hevcSliceParams[slcCount].slice_segment_address;
                    uint32_t sliceLcuX = sliceStartLcu % frameWidthInLcu;
                    uint32_t sliceLcuY = sliceStartLcu / frameWidthInLcu;

                    for (uint32_t i = 0; i < m_hevcSliceParams[slcCount].NumLCUsInSlice; i++)
                    {
                        lcuInfo[sliceLcuX][sliceLcuY].SliceStartLcuIndex   = (uint16_t)startLCU;
                        lcuInfo[sliceLcuX][sliceLcuY].SliceEndLcuIndex     = (uint16_t)(startLCU + m_hevcSliceParams[slcCount].NumLCUsInSlice);  // this should be next slice start index
                        lcuInfo[sliceLcuX][sliceLcuY].SliceId              = (uint16_t)slcCount;
                        lcuInfo[sliceLcuX][sliceLcuY].TileId               = (uint16_t)tileId;
                        lcuInfo[sliceLcuX][sliceLcuY].TileStartCoordinateX = (uint16_t)currentTile.TileStartLCUX;
                        lcuInfo[sliceLcuX][sliceLcuY].TileStartCoordinateY = (uint16_t)currentTile.TileStartLCUY;
                        lcuInfo[sliceLcuX][sliceLcuY].TileEndCoordinateX   = (uint16_t)(currentTile.TileStartLCUX + tileColumnWidth);
                        lcuInfo[sliceLcuX][sliceLcuY].TileEndCoordinateY   = (uint16_t)(currentTile.TileStartLCUY + tileRowHeight);

                        sliceLcuX++;

                        if (sliceLcuX >= currentTile.TileStartLCUX + tileColumnWidth)
                        {
                            sliceLcuX = currentTile.TileStartLCUX;
                            sliceLcuY++;
                        }
                    }
                    startLCU += m_hevcSliceParams[slcCount].NumLCUsInSlice;
                }
            }
        }
    }
    else  // non-tiling case
    {
        for (uint32_t startLCU = 0, sliceStartLcu = 0, slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            sliceStartLcu      = m_hevcSliceParams[slcCount].slice_segment_address;
            uint32_t sliceLcuX = sliceStartLcu % frameWidthInLcu;
            uint32_t sliceLcuY = sliceStartLcu / frameWidthInLcu;

            for (uint32_t i = 0; i < m_hevcSliceParams[slcCount].NumLCUsInSlice; i++)
            {
                lcuInfo[sliceLcuX][sliceLcuY].SliceStartLcuIndex   = (uint16_t)startLCU;
                lcuInfo[sliceLcuX][sliceLcuY].SliceEndLcuIndex     = (uint16_t)(startLCU + m_hevcSliceParams[slcCount].NumLCUsInSlice);  // this should be next slice start index
                lcuInfo[sliceLcuX][sliceLcuY].SliceId              = (uint16_t)slcCount;
                lcuInfo[sliceLcuX][sliceLcuY].TileId               = 0;
                lcuInfo[sliceLcuX][sliceLcuY].TileStartCoordinateX = 0;
                lcuInfo[sliceLcuX][sliceLcuY].TileStartCoordinateY = 0;
                lcuInfo[sliceLcuX][sliceLcuY].TileEndCoordinateX   = (uint16_t)frameWidthInLcu;
                lcuInfo[sliceLcuX][sliceLcuY].TileEndCoordinateY   = (uint16_t)frameHeightInLcu;

                sliceLcuX++;

                if (sliceLcuX >= frameWidthInLcu)
                {
                    sliceLcuX = 0;
                    sliceLcuY++;
                }
            }
            startLCU += m_hevcSliceParams[slcCount].NumLCUsInSlice;
        }
    }

    // Write LCU Info to the surface
    if (!Mos_ResourceIsNull(&lcuLevelInputDataSurfaceParam.OsResource))
    {
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly          = 1;
        PLCU_LEVEL_DATA lcuLevelData = (PLCU_LEVEL_DATA)m_osInterface->pfnLockResource(
            m_osInterface,
            &lcuLevelInputDataSurfaceParam.OsResource,
            &lockFlags);
        if (lcuLevelData == nullptr)
        {
            for (uint32_t i = 0; i < frameWidthInLcu; i++)
            {
                MOS_FreeMemory(lcuInfo[i]);
            }
            MOS_FreeMemory(lcuInfo);
            CODECHAL_ENCODE_CHK_NULL_RETURN(nullptr);
        }

        uint8_t *dataRowStart = (uint8_t *)lcuLevelData;

        for (uint32_t sliceLcuY = 0; sliceLcuY < frameHeightInLcu; sliceLcuY++)
        {
            for (uint32_t sliceLcuX = 0; sliceLcuX < frameWidthInLcu; sliceLcuX++)
            {
                *(lcuLevelData) = lcuInfo[sliceLcuX][sliceLcuY];

                if ((sliceLcuX + 1) == frameWidthInLcu)
                {
                    dataRowStart += lcuLevelInputDataSurfaceParam.dwPitch;
                    lcuLevelData = (PLCU_LEVEL_DATA)dataRowStart;
                }
                else
                {
                    lcuLevelData++;
                }
            }
        }

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &lcuLevelInputDataSurfaceParam.OsResource);
    }
    else
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        CODECHAL_ENCODE_ASSERTMESSAGE("Null pointer exception\n");
    }

    // Freeing the temporarily allocated memory
    if (lcuInfo)
    {
        for (uint32_t i = 0; i < frameWidthInLcu; i++)
        {
            MOS_FreeMemory(lcuInfo[i]);
        }
        MOS_FreeMemory(lcuInfo);
    }
    return eStatus;
}

// Helper class to describe quadtree node
class QuadTreeNode
{
    friend class QuadTree;

public:
    QuadTreeNode(const QuadTreeNode *ctb, uint32_t x, uint32_t y, uint32_t level, uint32_t ctbLog2Size) : m_ctb(ctb), m_x(x), m_y(y), m_level(level), m_size((1 << ctbLog2Size) >> level), m_ctbLog2Size(ctbLog2Size)
    {
    }

protected:
    void Build(uint32_t picWidth, uint32_t picHeight)
    {
        if (DoesBlockCrossCodedPicture(picWidth, picHeight))
        {
            CreateCUs();
            for_each(m_childBlocks.begin(), m_childBlocks.end(), [&](QuadTreeNode &blk) { blk.Build(picWidth, picHeight); });
        }
    }

    void CreateCUs()
    {
        uint32_t size  = m_size / 2;
        uint32_t level = m_level + 1;

        m_childBlocks.emplace_back(m_ctb, m_x, m_y, level, m_ctbLog2Size);
        m_childBlocks.emplace_back(m_ctb, m_x + size, m_y, level, m_ctbLog2Size);
        m_childBlocks.emplace_back(m_ctb, m_x, m_y + size, level, m_ctbLog2Size);
        m_childBlocks.emplace_back(m_ctb, m_x + size, m_y + size, level, m_ctbLog2Size);
    }

    bool DoesBlockCrossCodedPicture(uint32_t w, uint32_t h) const
    {
        return (m_x < w && ((m_x + m_size) > w)) || (m_y < h && ((m_y + m_size) > h));
    }

public:
    const QuadTreeNode *      m_ctb         = nullptr;  // the root of CTB
    const uint32_t            m_x           = 0;
    const uint32_t            m_y           = 0;
    const uint32_t            m_level       = 0;
    const uint32_t            m_size        = 0;
    const uint32_t            m_ctbLog2Size = 0;
    std::vector<QuadTreeNode> m_childBlocks = {};
};

class QuadTree : public QuadTreeNode
{
public:
    QuadTree(uint32_t x, uint32_t y, uint32_t ctbLog2Size)
        : QuadTreeNode(this, x, y, 0, ctbLog2Size)
    {
    }

    // Build quadtree in the way none of the blocks crosses picture boundary
    void BuildQuadTree(uint32_t width, uint32_t height)
    {
        m_picWidth  = width;
        m_picHeight = height;

        Build(width, height);

        CUs.reserve(64);
        FillCuList(*this, CUs);
    }

    static void GetSplitFlags(const QuadTreeNode &blk, HcpPakObjectG12 &pakObj)
    {
        auto idx = [](uint32_t x0, uint32_t y0, uint32_t x, uint32_t y, uint32_t log2CbSize) {
            auto const nCbS = (1 << log2CbSize);
            return (x - x0) / nCbS + (y - y0) / nCbS * 2;
        };

        if (blk.m_childBlocks.empty())  // Block doesn't have splits
            return;

        switch (blk.m_level)
        {
        case 0:
            pakObj.DW1.Split_flag_level0 = 1;
            break;

        case 1:
        {
            auto const blkIdx = idx(blk.m_ctb->m_x, blk.m_ctb->m_y, blk.m_x, blk.m_y, blk.m_ctbLog2Size - 1);
            pakObj.DW1.Split_flag_level1 |= 1 << blkIdx;
        }
        break;

        case 2:
        {
            auto const blkIdx1 = idx(blk.m_ctb->m_x, blk.m_ctb->m_y, blk.m_x, blk.m_y, blk.m_ctbLog2Size - 1);
            auto const nCbS1   = (1 << (blk.m_ctbLog2Size - 1));
            auto const x1      = blk.m_ctb->m_x + nCbS1 * (blkIdx1 % 2);
            auto const y1      = blk.m_ctb->m_y + nCbS1 * (blkIdx1 / 2);
            auto const blkIdx2 = idx(x1, y1, blk.m_x, blk.m_y, blk.m_ctbLog2Size - 2);
            switch (blkIdx1)
            {
            case 0:
                pakObj.DW1.Split_flag_level2_level1part0 |= 1 << blkIdx2;
                break;
            case 1:
                pakObj.DW1.Split_flag_level2_level1part1 |= 1 << blkIdx2;
                break;
            case 2:
                pakObj.DW1.Split_flag_level2_level1part2 |= 1 << blkIdx2;
                break;
            case 3:
                pakObj.DW1.Split_flag_level2_level1part3 |= 1 << blkIdx2;
                break;
            };
        }
        break;
        }

        for_each(blk.m_childBlocks.begin(), blk.m_childBlocks.end(), [&](const QuadTreeNode &blk) { GetSplitFlags(blk, pakObj); });
    }

protected:
    // Prepare a list of CU inside a coded picure boundary
    void FillCuList(const QuadTreeNode &cu, std::vector<const QuadTreeNode *> &list)
    {
        if (cu.m_childBlocks.empty() && ((cu.m_x + cu.m_size) <= m_picWidth) && ((cu.m_y + cu.m_size) <= m_picHeight))
            list.push_back(&cu);
        else
            for_each(cu.m_childBlocks.begin(), cu.m_childBlocks.end(), [&](const QuadTreeNode &blk) { FillCuList(blk, list); });
    }

    uint32_t m_picWidth  = 0;
    uint32_t m_picHeight = 0;

public:
    std::vector<const QuadTreeNode *> CUs = {};
};

MOS_STATUS CodechalEncHevcStateG12::GenerateSkipFrameMbCodeSurface(SkipFrameInfo &skipframeInfo)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlags = {};
    lockFlags.WriteOnly       = 1;
    uint8_t *data             = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &skipframeInfo.m_resMbCodeSkipFrameSurface, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);
    MOS_ZeroMemory(data, m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE);

    auto pakObjData = (HcpPakObjectG12 *)data;
    auto cuData     = (EncodeHevcCuDataG12 *)(data + m_mvOffset);

    auto const ctbSize          = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    auto const maxNumCuInCtb    = (ctbSize / CODECHAL_HEVC_MIN_CU_SIZE) * (ctbSize / CODECHAL_HEVC_MIN_CU_SIZE);
    auto const picWidthInCtb    = MOS_ROUNDUP_DIVIDE(m_frameWidth, ctbSize);
    auto const picHeightInCtb   = MOS_ROUNDUP_DIVIDE(m_frameHeight, ctbSize);
    uint32_t   num_tile_columns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    uint32_t * tileColumnsStartPosition{new uint32_t[num_tile_columns]{}};

    for (uint32_t i = 0; i < (num_tile_columns); i++)
    {
        if (m_hevcPicParams->tile_column_width[i] == 0)
        {
            tileColumnsStartPosition[i] = picWidthInCtb;
            break;
        }

        if (i == 0)
        {
            tileColumnsStartPosition[i] = m_hevcPicParams->tile_column_width[i];
            continue;
        }

        tileColumnsStartPosition[i] = tileColumnsStartPosition[i - 1] + m_hevcPicParams->tile_column_width[i];
    }

    // Prepare CTB splits for corner cases:
    // Last column
    QuadTree lastColumnCtb((picWidthInCtb - 1) * ctbSize, 0, m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    lastColumnCtb.BuildQuadTree(m_frameWidth, m_frameHeight);

    // Last row
    QuadTree lastRowCtb(0, (picHeightInCtb - 1) * ctbSize, m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    lastRowCtb.BuildQuadTree(m_frameWidth, m_frameHeight);

    // Right bottom CTB
    QuadTree lastColRowCtb((picWidthInCtb - 1) * ctbSize, (picHeightInCtb - 1) * ctbSize, m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    lastColRowCtb.BuildQuadTree(m_frameWidth, m_frameHeight);

    uint32_t sliceFirstCtbIdx;
    uint32_t ctbXAddr;
    uint32_t ctbYAddr;
    uint32_t nCUs;
    uint32_t tileEnd;
    uint32_t tileStart;
    for (uint32_t slcIdx = 0; slcIdx < m_numSlices; ++slcIdx)
    {
        sliceFirstCtbIdx = m_hevcSliceParams[slcIdx].slice_segment_address;
        tileEnd          = 0;
        tileStart        = 0;
        ctbXAddr         = sliceFirstCtbIdx % picWidthInCtb;
        ctbYAddr         = sliceFirstCtbIdx / picWidthInCtb;
        for (uint32_t i = 0; i < num_tile_columns; i++)
        {
            //Determine what tile slice belongs to
            if (ctbXAddr < tileColumnsStartPosition[i])
            {
                tileEnd   = tileColumnsStartPosition[i];
                tileStart = (i == 0) ? 0 : tileColumnsStartPosition[i - 1];
                break;
            }
        }

        for (uint32_t ctbIdxInSlice = 0; ctbIdxInSlice < m_hevcSliceParams[slcIdx].NumLCUsInSlice; ++ctbIdxInSlice, ++pakObjData, ++ctbXAddr)
        {
            if (ctbXAddr >= tileEnd)
            {
                ctbYAddr++;
                ctbXAddr = tileStart;
            }
            pakObjData->DW0.Type                    = 0x03;
            pakObjData->DW0.Opcode                  = 0x27;
            pakObjData->DW0.SubOp                   = 0x21;
            pakObjData->DW0.DwordLength             = 0x3;
            pakObjData->DW2.Current_LCU_X_Addr      = ctbXAddr;
            pakObjData->DW2.Current_LCU_Y_Addr      = ctbYAddr;
            pakObjData->DW4.LCUForceZeroCoeff       = 1;  // Force skip CUs
            pakObjData->DW4.Disable_SAO_On_LCU_Flag = 1;

            const bool bCtbCrossRightPicBoundary       = (ctbXAddr + 1) * ctbSize > m_frameWidth;
            const bool bCtbCrossBottomPicBoundary      = (ctbYAddr + 1) * ctbSize > m_frameHeight;
            const bool bCtbCrossRightBottomPicBoundary = bCtbCrossRightPicBoundary && bCtbCrossBottomPicBoundary;
            if (bCtbCrossRightBottomPicBoundary)
            {
                QuadTree::GetSplitFlags(lastColRowCtb, *pakObjData);
                nCUs = lastColRowCtb.CUs.size();
            }
            else if (bCtbCrossRightPicBoundary)
            {
                QuadTree::GetSplitFlags(lastColumnCtb, *pakObjData);
                nCUs = lastColumnCtb.CUs.size();
            }
            else if (bCtbCrossBottomPicBoundary)
            {
                QuadTree::GetSplitFlags(lastRowCtb, *pakObjData);
                nCUs = lastRowCtb.CUs.size();
            }
            else  // default case
            {
                nCUs = 1;
                // For regular CTB, CU splits are not needed. All level values are zero
            }
            pakObjData->DW1.CU_count_minus1 = nCUs - 1;

            if (ctbIdxInSlice == (m_hevcSliceParams[slcIdx].NumLCUsInSlice - 1))
            {
                pakObjData->DW1.LastCtbOfTileFlag = pakObjData->DW1.LastCtbOfSliceFlag = 1;
                pakObjData->DW5                                                        = 0x05000000;  // Add batch buffer end flag
            }

            auto CeilLog2 = [](uint32_t x) {
                auto l = 0;
                while (x > (1U << l)) l++;
                return l;
            };

            // Fill CU records
            for (unsigned int cuIdx = 0; cuIdx < nCUs; ++cuIdx, ++cuData)
            {
                cuData->DW7_CuPredMode = 1;  // Inter

                // Note that this can work only for B slices.
                // If P slice support appears, we need to have the 2nd skipFrameMbCodeSurface
                // When panic mode is triggered backwards reference only should be used
                cuData->DW7_InterPredIdcMv0 = 0;
                cuData->DW7_InterPredIdcMv1 = 0;

                if (bCtbCrossRightBottomPicBoundary)
                {
                    cuData->DW7_CuSize = CeilLog2(lastColRowCtb.CUs[cuIdx]->m_size) - 3;
                }
                else if (bCtbCrossRightPicBoundary)
                {
                    cuData->DW7_CuSize = CeilLog2(lastColumnCtb.CUs[cuIdx]->m_size) - 3;
                }
                else if (bCtbCrossBottomPicBoundary)
                {
                    cuData->DW7_CuSize = CeilLog2(lastRowCtb.CUs[cuIdx]->m_size) - 3;
                }
                else
                {
                    cuData->DW7_CuSize = m_hevcSeqParams->log2_max_coding_block_size_minus3;
                }

                if (cuData->DW7_CuSize == 3)  // 64x64
                {
                    cuData->DW5_TuSize        = 0xff;  // 4 TUs 32x32
                    cuData->DW6_TuCountMinus1 = 3;
                }
                else if (cuData->DW7_CuSize == 2)  // 32x32
                {
                    cuData->DW5_TuSize = 3;  // 1 TU 32x32
                }
                else if (cuData->DW7_CuSize == 1)  // 16x16
                {
                    cuData->DW5_TuSize = 2;  // 1 TU 16x16
                }
                else  // 8x8
                {
                    cuData->DW5_TuSize = 1;  // 1 TU 8x8
                }
            }
            cuData += (maxNumCuInCtb - nCUs);  // Shift to CUs of next CTB


        }
    }
    m_osInterface->pfnUnlockResource(m_osInterface, &skipframeInfo.m_resMbCodeSkipFrameSurface);
    delete[] tileColumnsStartPosition;

    skipframeInfo.numSlices = m_numSlices;
    uint32_t mbCodeSize     = m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE;

    #if USE_CODECHAL_DEBUG_TOOL
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &skipframeInfo.m_resMbCodeSkipFrameSurface,
        CodechalDbgAttr::attrInput,
        "SkipFrameSurface",
        mbCodeSize,
        0,
        CODECHAL_MEDIA_STATE_BRC_UPDATE));
    #endif

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::GenerateConcurrentThreadGroupData(MOS_RESOURCE &concurrentThreadGroupData)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&concurrentThreadGroupData))
    {
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly                            = 1;
        PCONCURRENT_THREAD_GROUP_DATA concurrentTgData = (PCONCURRENT_THREAD_GROUP_DATA)m_osInterface->pfnLockResource(
            m_osInterface,
            &concurrentThreadGroupData,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(concurrentTgData);

        MOS_ZeroMemory(concurrentTgData, concurrentThreadGroupData.iSize);

        uint32_t shift    = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
        uint32_t residual = (1 << shift) - 1;

        uint32_t frameWidthInLCU  = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1 + residual) >> shift;
        uint32_t frameHeightInLCU = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1 + residual) >> shift;

        uint32_t slcCount = 0;
        // Currently only using one thread group for each slice. Extend it to multiple soon.
        for (uint32_t startLcu = 0; slcCount < m_numSlices; slcCount++, startLcu += m_hevcSliceParams[slcCount].NumLCUsInSlice)
        {
            uint32_t sliceStartLcu  = m_hevcSliceParams[slcCount].slice_segment_address;
            uint32_t sliceStartLcux = sliceStartLcu % frameWidthInLCU;
            uint32_t sliceStartLcuy = sliceStartLcu / frameWidthInLCU;

            uint32_t sliceEndLcu  = (uint16_t)(startLcu + m_hevcSliceParams[slcCount].NumLCUsInSlice);  // this should be next slice start index
            uint32_t sliceEndLcux = sliceStartLcu % frameWidthInLCU;
            uint32_t sliceEndLcuy = sliceStartLcu / frameWidthInLCU;

            concurrentTgData->CurrSliceStartLcuX = (uint16_t)sliceStartLcux;
            concurrentTgData->CurrSliceStartLcuY = (uint16_t)sliceStartLcuy;

            concurrentTgData->CurrSliceEndLcuX = (uint16_t)sliceEndLcux;
            concurrentTgData->CurrSliceEndLcuY = (uint16_t)sliceEndLcuy;

            concurrentTgData->CurrTgStartLcuX = (uint16_t)sliceStartLcux;
            concurrentTgData->CurrTgStartLcuY = (uint16_t)sliceStartLcuy;

            concurrentTgData->CurrTgEndLcuX = (uint16_t)sliceEndLcux;
            concurrentTgData->CurrTgEndLcuY = (uint16_t)sliceEndLcuy;
        }

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &concurrentThreadGroupData);
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Null pointer exception\n");
        return MOS_STATUS_NULL_POINTER;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncodeMbEncKernel(
    CODECHAL_MEDIA_STATE_TYPE encFunctionType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL);

    // Initialize DSH kernel state
    PMHW_KERNEL_STATE            kernelState;
    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    CODECHAL_WALKER_DEGREE       walkerDegree;
    MHW_WALKER_PARAMS            walkerParams;
    uint32_t                     walkerResolutionX, walkerResolutionY;
    bool                         customDispatchPattern = true;
    uint16_t                     totalThreadNumPerLcu  = 1;

    if (m_hevcPicParams->CodingType == I_TYPE)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_HEVC_I_MBENC;
    }
    else
    {
        encFunctionType = m_isMaxLcu64 ? CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC : CODECHAL_MEDIA_STATE_HEVC_B_MBENC;
    }

    if (m_isMaxLcu64)
    {
        kernelState = &m_mbEncKernelStates[MBENC_LCU64_KRNIDX];
        if (m_hevcSeqParams->TargetUsage == 1)
        {
            walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE) >> 6;
            walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE) >> 6;
        }
        else
        {
            walkerResolutionX = 2 * (MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE) >> 6);
            walkerResolutionY = 2 * (MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE) >> 6);
        }
    }
    else
    {
        kernelState       = &m_mbEncKernelStates[MBENC_LCU32_KRNIDX];
        walkerResolutionX = MOS_ALIGN_CEIL(m_frameWidth, 32) >> 5;
        walkerResolutionY = MOS_ALIGN_CEIL(m_frameHeight, 32) >> 5;
    }

    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode             = m_walkerMode;
    walkerCodecParams.dwResolutionX          = walkerResolutionX;
    walkerCodecParams.dwResolutionY          = walkerResolutionY;
    walkerCodecParams.dwNumSlices            = m_numSlices;
    walkerCodecParams.usTotalThreadNumPerLcu = totalThreadNumPerLcu;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCustomDispatchPattern(&walkerParams, &walkerCodecParams));

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

    // Set up the DSH/SSH as normal
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

    // Generate Lcu Level Data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateLcuLevelData(m_lcuLevelInputDataSurface[m_currRecycledBufIdx]));

    // Generate Concurrent Thread Group Data
    if (m_swScoreboardState->GetDependencyPattern() == dependencyWavefront26Degree ||
        m_swScoreboardState->GetDependencyPattern() == dependencyWavefront26ZDegree ||
        m_swScoreboardState->GetDependencyPattern() == dependencyWavefront26XDegree)
    {
        // Generate Concurrent Thread Group Data
        uint32_t curIdx = m_currRecycledBufIdx;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateConcurrentThreadGroupData(m_encBCombinedBuffer1[curIdx].sResource));
    }
    else
    {
        // For 45D walking patter, kernel generates the concurrent thread group by itself. No need for driver to generate.
    }

    // setup curbe
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMbEncBKernel());

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
        //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpHEVCMbEncCurbeG12(
        //m_debugInterface,
        //encFunctionType,
        //&m_encBCombinedBuffer1[m_currRecycledBufIdx].sResource));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType      = encFunctionType;
    sendKernelCmdsParams.pKernelState         = kernelState;
    // TO DO : Remove scoreboard from VFE STATE Command
    sendKernelCmdsParams.bEnableCustomScoreBoard = false;
    sendKernelCmdsParams.pCustomScoreBoard       = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // send surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfacesBKernel(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        if (m_pictureCodingType == I_TYPE) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                &m_lcuLevelInputDataSurface[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "HEVC_I_MBENC_LcuLevelData_In",
                CODECHAL_MEDIA_STATE_HEVC_I_MBENC));
        } else {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
                &m_lcuLevelInputDataSurface[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "HEVC_B_MBENC_LcuLevelData_In",
                CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
        })

    if ((encFunctionType == CODECHAL_MEDIA_STATE_HEVC_B_MBENC) || (encFunctionType == CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC))
    {
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_encConstantTableForB.sResource,
            "HEVC_B_MBENC_ConstantData_In",
            CodechalDbgAttr::attrOutput,
            m_encConstantTableForB.dwSize,
            0,
            encFunctionType)));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for MBEnc surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_debugSurface[0].sResource,
            CodechalDbgAttr::attrOutput,
            "DebugDataSurface_Out0",
            m_debugSurface[0].dwSize,
            0,
            encFunctionType));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_debugSurface[1].sResource,
            CodechalDbgAttr::attrOutput,
            "DebugDataSurface_Out1",
            m_debugSurface[1].dwSize,
            0,
            encFunctionType));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_debugSurface[2].sResource,
            CodechalDbgAttr::attrOutput,
            "DebugDataSurface_Out2",
            m_debugSurface[2].dwSize,
            0,
            encFunctionType));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_debugSurface[3].sResource,
            CodechalDbgAttr::attrOutput,
            "DebugDataSurface_Out3",
            m_debugSurface[3].dwSize,
            0,
            encFunctionType)););

#if 0  // the dump should be done in the GetStatusReport. However, if ENC causes PAK hangs-up, there is no way to get them.
    {
        CODECHAL_DEBUG_TOOL(
            CODEC_REF_LIST      currRefList;

        currRefList = *(pRefList[m_currReconstructedPic.FrameIdx]);
        currRefList.RefPic = m_currOriginalPic;

        m_debugInterface->CurrPic = m_currOriginalPic;
        m_debugInterface->dwBufferDumpFrameNum = m_storeData;
        m_debugInterface->wFrameType = m_pictureCodingType;

        //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeMbEncMbPakOutput(
        //    m_debugInterface,
        //    this,
        //    &currRefList,
        //    (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
        //    CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &currRefList.resRefMbCodeBuffer,
            CodechalDbgAttr::attrOutput,
            "MbCode",
            m_picWidthInMb * m_frameFieldHeightInMb*64,
            CodecHal_PictureIsBottomField(currRefList.RefPic) ? m_frameFieldHeightInMb * m_picWidthInMb * 64 : 0,
            (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
            CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));

        if (m_mvDataSize)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &currRefList.resRefMvDataBuffer,
                CodechalDbgAttr::attrOutput,
                "MbData",
                m_picWidthInMb * m_frameFieldHeightInMb * (32 * 4),
                CodecHal_PictureIsBottomField(currRefList.RefPic) ? MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * (32 * 4), 0x1000) : 0,
                (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
                CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
        }
        if (CodecHalIsFeiEncode(m_codecFunction))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resDistortionBuffer,
                CodechalDbgAttr::attrOutput,
                "DistortionSurf",
                m_picWidthInMb * m_frameFieldHeightInMb * 48,
                CodecHal_PictureIsBottomField(currRefList.RefPic) ? MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * 48, 0x1000) : 0,
                (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
                CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
        }

        )

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeCombineBuffer(
                    this,
                    &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource,
                    m_encBCombinedBuffer2[m_currRecycledBufIdx].dwSize,
                    (const char*)"_Hevc_CombinedBuffer2",
                    false));
        );

        // Dump SW scoreboard surface - Output of MBENC
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpHevcEncodeSwScoreboardSurface(
                m_debugInterface,
                m_swScoreboardState->GetCurSwScoreboardSurface(), false));
        );

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeCombineBuffer(
                this,
                &m_encConstantTableForB.sResource,
                m_encConstantTableForB.dwSize,
                (const char*)"_Hevc_EncConstantTable",
                true));
        );

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeCombineBuffer(
                this,
                &m_debugSurface[0].sResource,
                m_debugSurface[0].dwSize,
                (const char*)"_Hevc_DebugDump0",
                true));
        );

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeCombineBuffer(
                this,
                &m_debugSurface[1].sResource,
                m_debugSurface[1].dwSize,
                (const char*)"_Hevc_DebugDump1",
                true));
        );

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeCombineBuffer(
                this,
                &m_debugSurface[2].sResource,
                m_debugSurface[2].dwSize,
                (const char*)"_Hevc_DebugDump2",
                true));
        );

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeCombineBuffer(
                this,
                &m_debugSurface[3].sResource,
                m_debugSurface[3].dwSize,
                (const char*)"_Hevc_DebugDump3",
                true));
        );

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_currPicWithReconBoundaryPix,
                CodechalDbgAttr::attrReconstructedSurface,
                "ReconSurf")));
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncodeBrcInitResetKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_brcKernelStates);

    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = m_brcInit ? CODECHAL_HEVC_BRC_INIT : CODECHAL_HEVC_BRC_RESET;

    // Initialize DSH kernel state
    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];

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

    // Set up the DSH/SSH as normal
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

    // Setup curbe for BrcInitReset kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcInitReset(
        brcKrnIdx));

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_INIT_RESET;
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

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType      = encFunctionType;
    sendKernelCmdsParams.pKernelState         = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Send surfaces for BrcInitReset Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcInitResetSurfaces(&cmdBuffer, brcKrnIdx));

    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));

    MediaObjectInlineData mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData      = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for BrcInitReset surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncodeBrcFrameUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE);

    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = CODECHAL_HEVC_BRC_FRAME_UPDATE;

    // Initialize DSH kernel state
    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];

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

    // Set up the DSH/SSH as normal
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

    // Setup curbe for BrcFrameUpdate kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcUpdate(
        brcKrnIdx));

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_UPDATE;
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

    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams                 = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState    = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Send surfaces for BrcFrameUpdate Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcFrameUpdateSurfaces(&cmdBuffer));

    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));

    MediaObjectInlineData mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData      = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for BrcFrameUpdate surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncodeBrcLcuUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_LCU);

    CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx = CODECHAL_HEVC_BRC_LCU_UPDATE;

    // Initialize DSH kernel state
    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKrnIdx];

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

    // Set up the DSH/SSH as normal
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

    // Setup curbe for BrcFrameUpdate kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcUpdate(
        brcKrnIdx));

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_MB_BRC_UPDATE;
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

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType      = encFunctionType;
    sendKernelCmdsParams.pKernelState         = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    if (m_hevcPicParams->NumROI)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupROISurface());
    }

    // Send surfaces for BrcFrameUpdate Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcLcuUpdateSurfaces(&cmdBuffer));

    // Program Media walker
    uint32_t resolutionX, resolutionY;
    resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth);
    resolutionX = MOS_ROUNDUP_SHIFT(resolutionX, 4);
    resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight);
    resolutionY = MOS_ROUNDUP_SHIFT(resolutionY, 3);
    CODECHAL_ENCODE_ASSERTMESSAGE("LucBRC thread space = %d x %d", resolutionX, resolutionY);

    MHW_WALKER_PARAMS walkerParams;
    MOS_ZeroMemory(&walkerParams, sizeof(walkerParams));

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode              = m_walkerMode;
    walkerCodecParams.dwResolutionX           = resolutionX;
    walkerCodecParams.dwResolutionY           = resolutionY;
    walkerCodecParams.bNoDependency           = true;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId               = m_groupId;
    walkerCodecParams.wPictureCodingType      = m_pictureCodingType;
    walkerCodecParams.bUseScoreboard          = false;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    // Add dump for BrcFrameUpdate surface state heap here
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncodeKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_pakOnlyTest)
    {
        // Skip ENC when PAK only mode is enabled
        return eStatus;
    }

    if (m_pictureCodingType == P_TYPE)
    {
        m_lowDelay = true;
    }

    if (m_hevcPicParams->bUsedAsRef || m_brcEnabled)
    {
        m_currRefSync = &m_refSync[m_currMbCodeIdx];

        // Check if the signal obj has been used before
        if (!m_hevcSeqParams->ParallelBRC && (m_currRefSync->uiSemaphoreObjCount || m_currRefSync->bInUsed))
        {
            MOS_SYNC_PARAMS syncParams  = g_cInitSyncParams;
            syncParams.GpuContext       = m_renderContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;
            syncParams.uiSemaphoreCount = m_currRefSync->uiSemaphoreObjCount;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount = 0;
            m_currRefSync->bInUsed             = false;
        }
    }
    else
    {
        m_currRefSync = nullptr;
    }

    //Reset to use a different performance tag ID
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_firstTaskInPhase = true;
    m_lastTaskInPhase  = false;

    m_brcInputForEncKernelBuffer = &m_encBCombinedBuffer2[m_currRecycledBufIdx];

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    // BRC init is called once even for CQP mode when ROI is enabled, hence also checking for first frame flag
    if ((m_brcEnabled && (m_brcInit || m_brcReset)) || (m_firstFrame && m_hevcPicParams->NumROI))
    {
        m_firstTaskInPhase = m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hevcBrcG12->EncodeBrcInitResetKernel());
        m_brcInit = m_brcReset = false;
    }

    m_firstTaskInPhase = true;
    m_lastTaskInPhase  = false;

    CodechalEncodeSwScoreboard::KernelParams swScoreboardKernelParames;
    MOS_ZeroMemory(&swScoreboardKernelParames, sizeof(swScoreboardKernelParames));

    InitSwScoreBoardParams(swScoreboardKernelParames);

    if (m_useSwInitScoreboard)
    {
        SetupSwScoreBoard(&swScoreboardKernelParames);
    }
    else
    {
        // Call SW scoreboard Init kernel used by MBEnc kernel
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->Execute(&swScoreboardKernelParames));
    }

    // Dump SW scoreboard surface - Output of SW scoreboard Init Kernel and Input to MBENC
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpSurface(
        m_swScoreboardState->GetCurSwScoreboardSurface(),
        CodechalDbgAttr::attrInput,
        "InitSWScoreboard_In",
        CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT)));

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
    cscScalingKernelParams.bLastTaskInPhaseCSC =
        cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_16xMeSupported || m_hmeEnabled || m_brcEnabled);
    cscScalingKernelParams.bLastTaskInPhase16xDS    = !(m_32xMeSupported || m_hmeEnabled || m_brcEnabled);
    cscScalingKernelParams.bLastTaskInPhase32xDS    = !(m_hmeEnabled || m_brcEnabled);

    CodechalEncodeCscDsG12::HevcExtKernelParams hevcExtCscParams;
    MOS_ZeroMemory(&hevcExtCscParams, sizeof(hevcExtCscParams));

    if (m_isMaxLcu64)
    {
        hevcExtCscParams.bHevcEncHistorySum            = true;
        hevcExtCscParams.bUseLCU32                     = false;
        hevcExtCscParams.presHistoryBuffer             = &m_encBCombinedBuffer2[m_lastRecycledBufIdx].sResource;
        hevcExtCscParams.dwSizeHistoryBuffer           = m_historyOutBufferSize;
        hevcExtCscParams.dwOffsetHistoryBuffer         = m_historyOutBufferOffset;
        hevcExtCscParams.presHistorySumBuffer          = &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource;
        hevcExtCscParams.dwSizeHistorySumBuffer        = sizeof(MBENC_COMBINED_BUFFER2::ucHistoryInBuffer);
        hevcExtCscParams.dwOffsetHistorySumBuffer      = sizeof(MBENC_COMBINED_BUFFER2::ucBrcCombinedEncBuffer);
        hevcExtCscParams.presMultiThreadTaskBuffer     = &m_encBCombinedBuffer2[m_currRecycledBufIdx].sResource;
        hevcExtCscParams.dwSizeMultiThreadTaskBuffer   = m_threadTaskBufferSize;
        hevcExtCscParams.dwOffsetMultiThreadTaskBuffer = m_threadTaskBufferOffset;
        cscScalingKernelParams.hevcExtParams           = &hevcExtCscParams;
    }
    else
    {
        cscScalingKernelParams.hevcExtParams = nullptr;  // LCU32 does not require history buffers
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));

    if (m_hmeEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel());
    }
    else if (m_brcEnabled && m_hevcPicParams->CodingType == I_TYPE)
    {
        m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeIntraDistKernel());
    }

    // BRC + MbEnc in second task phase
    m_firstTaskInPhase = true;
    m_lastTaskInPhase  = false;

    // Wait for PAK if necessary
    CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForPak());

    // ROI uses the BRC LCU update kernel, even in CQP.  So we will call it
    if (m_hevcPicParams->NumROI && !m_brcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hevcBrcG12->EncodeBrcLcuUpdateKernel());
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame;

        CODECHAL_DEBUG_TOOL(
            if (!Mos_ResourceIsNull(&m_brcBuffers.sBrcMbQpBuffer.OsResource)) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_brcBuffers.sBrcMbQpBuffer.OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MbQp",
                    m_brcBuffers.sBrcMbQpBuffer.dwPitch * m_brcBuffers.sBrcMbQpBuffer.dwHeight,
                    m_brcBuffers.dwBrcMbQpBottomFieldOffset,
                    CODECHAL_MEDIA_STATE_BRC_UPDATE));
            } CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(&m_brcDistortion->OsResource,
                CodechalDbgAttr::attrInput,
                "BrcDist_AfterLcuBrc",
                m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));)
    }

    if (m_brcEnabled)
    {
        m_hevcBrcG12->m_brcNumPakPasses = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hevcBrcG12->EncodeBrcFrameUpdateKernel());

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcDistortion->OsResource,
                CodechalDbgAttr::attrInput,
                "BrcDist_AfterFrameBrc",
                m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resBrcHistoryBuffer,
                CodechalDbgAttr::attrOutput,
                "HistoryWrite",
                m_brcHistoryBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "ImgStateWrite",
                BRC_IMG_STATE_SIZE_PER_PASS_G11 * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));)

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcDistortion->OsResource,
                CodechalDbgAttr::attrInput,
                "BrcDist_AfterFrameBrcUpdate",
                m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "ImgStateWrite",
                BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resBrcHistoryBuffer,
                CodechalDbgAttr::attrOutput,
                "HistoryWrite",
                m_brcHistoryBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.sBrcIntraDistortionBuffer.OsResource,
                CodechalDbgAttr::attrOutput,
                "Idistortion",
                m_brcBuffers.sBrcIntraDistortionBuffer.dwWidth * m_brcBuffers.sBrcIntraDistortionBuffer.dwHeight,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));)

        if (m_lcuBrcEnabled || m_hevcPicParams->NumROI)
        {
            // LCU-based BRC needs to have frame-based one to be call first in order to get HCP_IMG_STATE command result
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hevcBrcG12->EncodeBrcLcuUpdateKernel());
            m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame;
        }
        else
        {
            m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame;
        }

        CODECHAL_DEBUG_TOOL(
            if (!Mos_ResourceIsNull(&m_brcBuffers.sBrcMbQpBuffer.OsResource)) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_brcBuffers.sBrcMbQpBuffer.OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MbQp",
                    m_brcBuffers.sBrcMbQpBuffer.dwPitch * m_brcBuffers.sBrcMbQpBuffer.dwHeight,
                    m_brcBuffers.dwBrcMbQpBottomFieldOffset,
                    CODECHAL_MEDIA_STATE_BRC_UPDATE));
            } CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(&m_brcDistortion->OsResource,
                CodechalDbgAttr::attrInput,
                "BrcDist_AfterLcuBrcUpdate",
                m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
                m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));)
    }

    m_useWeightedSurfaceForL0 = false;
    m_useWeightedSurfaceForL1 = false;

    //currently only support same weightoffset for all slices, and only support Luma weighted prediction
    auto slicetype = m_hevcSliceParams->slice_type;
    if (m_weightedPredictionSupported && !m_feiEnable &&
        ((slicetype == CODECHAL_HEVC_P_SLICE && m_hevcPicParams->weighted_pred_flag) ||
            (slicetype == CODECHAL_HEVC_B_SLICE && m_hevcPicParams->weighted_bipred_flag)))
    {
        uint32_t                      LumaWeightFlag[2] = {0};  //[L0, L1]
        CodechalEncodeWP::SliceParams sliceWPParams;
        MOS_FillMemory((void *)&sliceWPParams, sizeof(sliceWPParams), 0);

        //populate the slice WP parameter structure
        sliceWPParams.luma_log2_weight_denom = m_hevcSliceParams->luma_log2_weight_denom;  // luma weidht denom
        for (auto i = 0; i < 2; i++)
        {
            for (auto j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
            {
                sliceWPParams.weights[i][j][0][0] = (1 << m_hevcSliceParams->luma_log2_weight_denom) +
                                                    m_hevcSliceParams->delta_luma_weight[i][j];  //Luma weight
                sliceWPParams.weights[i][j][0][1] = m_hevcSliceParams->luma_offset[i][j];        //Luma offset

                if (m_hevcSliceParams->delta_luma_weight[i][j] || m_hevcSliceParams->luma_offset[i][j])
                {
                    LumaWeightFlag[i] |= (1 << j);
                }
            }
        }

        CodechalEncodeWP::KernelParams wpKernelParams;
        MOS_FillMemory((void *)&wpKernelParams, sizeof(wpKernelParams), 0);
        wpKernelParams.useWeightedSurfaceForL0 = &m_useWeightedSurfaceForL0;
        wpKernelParams.useWeightedSurfaceForL1 = &m_useWeightedSurfaceForL1;
        wpKernelParams.slcWPParams             = &sliceWPParams;

        // Weighted Prediction to be applied for L0
        for (auto i = 0; i < (m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            if ((LumaWeightFlag[LIST_0] & (1 << i)) && (i < CODEC_MAX_FORWARD_WP_FRAME))
            {
                CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][i];
                if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
                {
                    MOS_SURFACE refFrameInput;
                    uint8_t     frameIndex = m_picIdx[refPic.FrameIdx].ucPicIdx;
                    refFrameInput          = m_hevcPicParams->bUseRawPicForRef ? m_refList[frameIndex]->sRefRawBuffer : m_refList[frameIndex]->sRefReconBuffer;

                    //Weighted Prediction for ith forward reference frame
                    wpKernelParams.useRefPicList1 = false;
                    wpKernelParams.wpIndex        = i;
                    wpKernelParams.refFrameInput  = &refFrameInput;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_wpState->Execute(&wpKernelParams));
                }
            }
        }

        // Weighted Predition to be applied for L1
        if (slicetype == CODECHAL_HEVC_B_SLICE && m_hevcPicParams->weighted_bipred_flag)
        {
            for (auto i = 0; i < (m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1); i++)
            {
                if ((LumaWeightFlag[LIST_1] & (1 << i)) && (i < CODEC_MAX_BACKWARD_WP_FRAME))
                {
                    CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_1][i];
                    if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
                    {
                        MOS_SURFACE refFrameInput;
                        uint8_t     frameIndex = m_picIdx[refPic.FrameIdx].ucPicIdx;
                        refFrameInput          = m_hevcPicParams->bUseRawPicForRef ? m_refList[frameIndex]->sRefRawBuffer : m_refList[frameIndex]->sRefReconBuffer;

                        //Weighted Prediction for ith backward reference frame
                        wpKernelParams.useRefPicList1 = true;
                        wpKernelParams.wpIndex        = i;
                        wpKernelParams.refFrameInput  = &refFrameInput;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_wpState->Execute(&wpKernelParams));
                    }
                }
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    // Weighted prediction for L0 Reporting
    userFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = m_useWeightedSurfaceForL0;
    userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L0_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1);
    // Weighted prediction for L1 Reporting
    userFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = m_useWeightedSurfaceForL1;
    userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L1_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1);

#endif  // _DEBUG || _RELEASE_INTERNAL

    // Reset to use a different performance tag ID
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    m_lastTaskInPhase = true;

    if (m_hevcPicParams->CodingType == I_TYPE)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(CODECHAL_MEDIA_STATE_HEVC_I_MBENC));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(m_isMaxLcu64 ? CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC : CODECHAL_MEDIA_STATE_HEVC_B_MBENC));
    }

    if (m_brcEnabled && m_enableFramePanicMode && (false == m_hevcSeqParams->DisableHRDConformance) &&
        m_skipFrameInfo.numSlices != m_numSlices)  // 'numSlices != m_numSlices' check is to re-generate surface if slice layout changed from previous frame
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GenerateSkipFrameMbCodeSurface(m_skipFrameInfo));
    }

    // Notify PAK engine once ENC is done
    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        if (m_useMdf)
        {
            if (!m_computeContextEnabled)
            {
                syncParams.GpuContext = MOS_GPU_CONTEXT_RENDER3;  //MDF uses render3
            }
            else
            {
                syncParams.GpuContext = MOS_GPU_CONTEXT_CM_COMPUTE;
            }
        }
        else
        {
            syncParams.GpuContext = m_renderContext;
        }
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        uint32_t old_stream_index  = m_osInterface->streamIndex;
        m_osInterface->streamIndex = static_cast<CmQueueRT *>(m_cmQueue)->StreamIndex();
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
        m_osInterface->streamIndex = old_stream_index;
    }

    if (m_brcEnabled)
    {
        if (m_hevcSeqParams->ParallelBRC)
        {
            m_brcBuffers.uiCurrBrcPakStasIdxForRead =
                (m_brcBuffers.uiCurrBrcPakStasIdxForRead + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }
    }

    CODECHAL_DEBUG_TOOL(
        uint8_t       index;
        CODEC_PICTURE refPic;
        if (m_useWeightedSurfaceForL0) {
            refPic = m_hevcSliceParams->RefPicList[LIST_0][0];
            index  = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_In_L0")));

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L0_START + 0),
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_Out_L0")));
        } if (m_useWeightedSurfaceForL1) {
            refPic = m_hevcSliceParams->RefPicList[LIST_1][0];
            index  = m_hevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_In_L1")));

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L1_START + 0),
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_Out_L1")));
        })

    m_lastPictureCodingType = m_pictureCodingType;
    m_lastRecycledBufIdx    = m_currRecycledBufIdx;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::EncodeIntraDistKernel()
{
    CodechalKernelIntraDist::CurbeParam curbeParam;
    curbeParam.downScaledWidthInMb4x  = m_downscaledWidthInMb4x;
    curbeParam.downScaledHeightInMb4x = m_downscaledHeightInMb4x;

    CodechalKernelIntraDist::SurfaceParams surfaceParam;
    surfaceParam.input4xDsSurface =
        surfaceParam.input4xDsVmeSurface    = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    surfaceParam.intraDistSurface           = m_brcDistortion;
    surfaceParam.intraDistBottomFieldOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Execute(curbeParam, surfaceParam));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcStateG12::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    // Create weighted prediction kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_wpState = MOS_New(CodechalEncodeWPG12, this));
    m_wpState->SetKernelBase(m_kernelBase);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_wpState->InitKernelState());
    // create intra distortion kernel
    m_intraDistKernel = MOS_New(CodechalKernelIntraDist, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_intraDistKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));

    // Create SW scoreboard init kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_swScoreboardState = MOS_New(CodechalEncodeSwScoreboardG12, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->InitKernelState());
    // Create Hme kernel
    m_hmeKernel = MOS_New(CodechalKernelHmeG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetDmemHuCPakIntegrate(
    PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES || !m_brcEnabled)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    HucPakStitchDmemEncG12 *hucPakStitchDmem = (HucPakStitchDmemEncG12 *)m_osInterface->pfnLockResource(
        m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]), &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucPakStitchDmem);

    MOS_ZeroMemory(hucPakStitchDmem, sizeof(HucPakStitchDmemEncG12));

    // reset all the offsets to -1
    uint32_t TotalOffsetSize = sizeof(hucPakStitchDmem->TileSizeRecord_offset) +
                               sizeof(hucPakStitchDmem->VDENCSTAT_offset) +
                               sizeof(hucPakStitchDmem->HEVC_PAKSTAT_offset) +
                               sizeof(hucPakStitchDmem->HEVC_Streamout_offset) +
                               sizeof(hucPakStitchDmem->VP9_PAK_STAT_offset) +
                               sizeof(hucPakStitchDmem->Vp9CounterBuffer_offset);
    MOS_FillMemory(hucPakStitchDmem, TotalOffsetSize, 0xFF);

    uint16_t numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    uint16_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    CODECHAL_ENCODE_ASSERT(numTileColumns > 0 && numTileColumns % 2 == 0);                       //numTileColumns is nonzero and even number; 2 or 4
    CODECHAL_ENCODE_ASSERT(m_numPipe > 0 && m_numPipe % 2 == 0 && numTileColumns <= m_numPipe);  //ucNumPipe is nonzero and even number; 2 or 4
    uint16_t numTiles        = numTileRows * numTileColumns;
    uint16_t numTilesPerPipe = m_numTiles / m_numPipe;

    hucPakStitchDmem->PicWidthInPixel          = (uint16_t)m_frameWidth;
    hucPakStitchDmem->PicHeightInPixel         = (uint16_t)m_frameHeight;
    hucPakStitchDmem->TotalNumberOfPAKs        = m_numPipe;
    hucPakStitchDmem->Codec                    = 1;  // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
    hucPakStitchDmem->MAXPass                  = m_brcEnabled ? (m_numPassesInOnePipe + 1) : 1;
    hucPakStitchDmem->CurrentPass              = (uint8_t)currentPass + 1;  // // Current BRC pass [1..MAXPass]
    hucPakStitchDmem->MinCUSize                = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    hucPakStitchDmem->CabacZeroWordFlag        = true;                                          // to do: set to true later
    hucPakStitchDmem->bitdepth_luma            = m_hevcSeqParams->bit_depth_luma_minus8 + 8;    // default: 8
    hucPakStitchDmem->bitdepth_chroma          = m_hevcSeqParams->bit_depth_chroma_minus8 + 8;  // default: 8
    hucPakStitchDmem->ChromaFormatIdc          = m_hevcSeqParams->chroma_format_idc;
    hucPakStitchDmem->TotalSizeInCommandBuffer = m_numTiles * CODECHAL_CACHELINE_SIZE;
    // Last tile length may get modified by HuC. Obtain last Tile Record, Add an offset of 8bytes to skip address field in Tile Record
    hucPakStitchDmem->OffsetInCommandBuffer   = m_tileParams[m_numTiles - 1].TileSizeStreamoutOffset * CODECHAL_CACHELINE_SIZE + 8;
    hucPakStitchDmem->LastTileBS_StartInBytes = m_tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE;

    hucPakStitchDmem->StitchEnable        = false;
    hucPakStitchDmem->StitchCommandOffset = 0;
    hucPakStitchDmem->BBEndforStitch      = HUC_BATCH_BUFFER_END;
    hucPakStitchDmem->brcUnderFlowEnable  = false;  //temporally disable underflow bit rate control in HUC fw since it need more tuning.

    PCODEC_ENCODER_SLCDATA slcData = m_slcData;
    CODECHAL_ENCODE_CHK_NULL_RETURN(slcData);
    uint32_t totalSliceHeaderSize = 0;
    for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        totalSliceHeaderSize += (slcData->BitSize + 7) >> 3;
        slcData++;
    }
    hucPakStitchDmem->SliceHeaderSizeinBits = totalSliceHeaderSize * 8;
    hucPakStitchDmem->currFrameBRClevel     = m_currFrameBrcLevel;

    //Set the kernel output offsets
    hucPakStitchDmem->TileSizeRecord_offset[0] = m_hevcFrameStatsOffset.uiTileSizeRecord;
    hucPakStitchDmem->HEVC_PAKSTAT_offset[0]   = m_hevcFrameStatsOffset.uiHevcPakStatistics;
    hucPakStitchDmem->HEVC_Streamout_offset[0] = 0xFFFFFFFF;
    hucPakStitchDmem->VDENCSTAT_offset[0]      = 0xFFFFFFFF;

    for (auto i = 0; i < m_numPipe; i++)
    {
        hucPakStitchDmem->NumTiles[i] = numTilesPerPipe;

        // Statistics are dumped out at a tile level. Driver shares with kernel starting offset of each pipe statistic.
        // Offset is calculated by adding size of statistics/pipe to the offset in combined statistics region.
        hucPakStitchDmem->TileSizeRecord_offset[i + 1] = (i * numTilesPerPipe * m_hevcStatsSize.uiTileSizeRecord) +
                                                         m_hevcTileStatsOffset.uiTileSizeRecord;
        hucPakStitchDmem->HEVC_PAKSTAT_offset[i + 1] = (i * numTilesPerPipe * m_hevcStatsSize.uiHevcPakStatistics) +
                                                       m_hevcTileStatsOffset.uiHevcPakStatistics;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]));

    MOS_ZeroMemory(dmemParams, sizeof(MHW_VDBOX_HUC_DMEM_STATE_PARAMS));
    dmemParams->presHucDataSource = &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]);
    dmemParams->dwDataLength      = MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemEncG12), CODECHAL_CACHELINE_SIZE);
    dmemParams->dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetRegionsHuCPakIntegrate(
    PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 ||
        (currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES && m_brcEnabled) ||
        (currentPass != 0 && m_cqpEnabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());
    MOS_ZeroMemory(virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));
    // Add Virtual addr
    virtualAddrParams->regionParams[0].presRegion  = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 0 - Tile based input statistics from PAK/ VDEnc
    virtualAddrParams->regionParams[0].dwOffset    = 0;
    virtualAddrParams->regionParams[1].presRegion  = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 1 - HuC Frame statistics output
    virtualAddrParams->regionParams[1].isWritable  = true;
    virtualAddrParams->regionParams[4].presRegion  = &m_resBitstreamBuffer;  // Region 4 - Last Tile bitstream
    virtualAddrParams->regionParams[5].presRegion  = &m_resBitstreamBuffer;  // Region 5 - HuC modifies the last tile bitstream before stitch command
    virtualAddrParams->regionParams[5].isWritable  = true;
    virtualAddrParams->regionParams[6].presRegion  = &m_brcBuffers.resBrcHistoryBuffer;  // Region 6  History Buffer (Input/Output)
    virtualAddrParams->regionParams[6].isWritable  = true;
    virtualAddrParams->regionParams[7].presRegion  = &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx];  //&m_resHucPakStitchReadBatchBuffer;             // Region 7 - HCP PIC state command
    virtualAddrParams->regionParams[9].presRegion  = &m_resBrcDataBuffer;                                               // Region 9  HuC outputs BRC data
    virtualAddrParams->regionParams[9].isWritable  = true;
    virtualAddrParams->regionParams[8].presRegion  = &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass];  // Region 8 - data buffer read by HUC for stitching cmd generation
    virtualAddrParams->regionParams[10].presRegion = &m_HucStitchCmdBatchBuffer.OsResource;                         // Region 10 - SLB for stitching cmd output from Huc
    virtualAddrParams->regionParams[10].isWritable = true;
    virtualAddrParams->regionParams[15].presRegion = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;  // Region 15 [In/Out] - Tile Record Buffer
    virtualAddrParams->regionParams[15].dwOffset   = 0;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetDmemHuCPakIntegrateCqp(
    PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    int32_t currentPass = GetCurrentPass();
    if (currentPass != 0 || (!m_cqpEnabled && m_hevcSeqParams->RateControlMethod != RATECONTROL_ICQ))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    HucPakStitchDmemEncG12 *hucPakStitchDmem = (HucPakStitchDmemEncG12 *)m_osInterface->pfnLockResource(
        m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]), &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucPakStitchDmem);

    MOS_ZeroMemory(hucPakStitchDmem, sizeof(HucPakStitchDmemEncG12));

    // reset all the offsets to -1
    uint32_t TotalOffsetSize = sizeof(hucPakStitchDmem->TileSizeRecord_offset) +
                               sizeof(hucPakStitchDmem->VDENCSTAT_offset) +
                               sizeof(hucPakStitchDmem->HEVC_PAKSTAT_offset) +
                               sizeof(hucPakStitchDmem->HEVC_Streamout_offset) +
                               sizeof(hucPakStitchDmem->VP9_PAK_STAT_offset) +
                               sizeof(hucPakStitchDmem->Vp9CounterBuffer_offset);
    MOS_FillMemory(hucPakStitchDmem, TotalOffsetSize, 0xFF);

    uint16_t numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    uint16_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    CODECHAL_ENCODE_ASSERT(numTileColumns > 0 && numTileColumns % 2 == 0);                       //numTileColumns is nonzero and even number; 2 or 4
    CODECHAL_ENCODE_ASSERT(m_numPipe > 0 && m_numPipe % 2 == 0 && numTileColumns <= m_numPipe);  //ucNumPipe is nonzero and even number; 2 or 4
    uint16_t numTiles        = numTileRows * numTileColumns;
    uint16_t numTilesPerPipe = m_numTiles / m_numPipe;

    hucPakStitchDmem->PicWidthInPixel          = (uint16_t)m_frameWidth;
    hucPakStitchDmem->PicHeightInPixel         = (uint16_t)m_frameHeight;
    hucPakStitchDmem->TotalNumberOfPAKs        = m_numPipe;
    hucPakStitchDmem->Codec                    = 2;  //HEVC DP CQP
    hucPakStitchDmem->MAXPass                  = 1;
    hucPakStitchDmem->CurrentPass              = 1;
    hucPakStitchDmem->MinCUSize                = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    hucPakStitchDmem->CabacZeroWordFlag        = true;
    hucPakStitchDmem->bitdepth_luma            = m_hevcSeqParams->bit_depth_luma_minus8 + 8;    // default: 8
    hucPakStitchDmem->bitdepth_chroma          = m_hevcSeqParams->bit_depth_chroma_minus8 + 8;  // default: 8
    hucPakStitchDmem->ChromaFormatIdc          = m_hevcSeqParams->chroma_format_idc;
    hucPakStitchDmem->TotalSizeInCommandBuffer = m_numTiles * CODECHAL_CACHELINE_SIZE;
    // Last tile length may get modified by HuC. Obtain last Tile Record, Add an offset of 8bytes to skip address field in Tile Record
    hucPakStitchDmem->OffsetInCommandBuffer   = m_tileParams[m_numTiles - 1].TileSizeStreamoutOffset * CODECHAL_CACHELINE_SIZE + 8;
    hucPakStitchDmem->LastTileBS_StartInBytes = m_tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE;

    hucPakStitchDmem->StitchEnable        = false;
    hucPakStitchDmem->StitchCommandOffset = 0;
    hucPakStitchDmem->BBEndforStitch      = HUC_BATCH_BUFFER_END;

    //Set the kernel output offsets
    hucPakStitchDmem->TileSizeRecord_offset[0] = m_hevcFrameStatsOffset.uiTileSizeRecord;
    hucPakStitchDmem->HEVC_PAKSTAT_offset[0]   = 0xFFFFFFFF;
    hucPakStitchDmem->HEVC_Streamout_offset[0] = 0xFFFFFFFF;
    hucPakStitchDmem->VDENCSTAT_offset[0]      = 0xFFFFFFFF;

    for (auto i = 0; i < m_numPipe; i++)
    {
        hucPakStitchDmem->NumTiles[i] = numTilesPerPipe;

        // Statistics are dumped out at a tile level. Driver shares with kernel starting offset of each pipe statistic.
        // Offset is calculated by adding size of statistics/pipe to the offset in combined statistics region.
        hucPakStitchDmem->TileSizeRecord_offset[i + 1] = (i * numTilesPerPipe * m_hevcStatsSize.uiTileSizeRecord) +
                                                         m_hevcTileStatsOffset.uiTileSizeRecord;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]));

    MOS_ZeroMemory(dmemParams, sizeof(MHW_VDBOX_HUC_DMEM_STATE_PARAMS));
    dmemParams->presHucDataSource = &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]);
    dmemParams->dwDataLength      = MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemEncG12), CODECHAL_CACHELINE_SIZE);
    dmemParams->dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ConfigStitchDataBuffer()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 ||
        (currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES && m_brcEnabled) ||
        (currentPass != 0 && m_cqpEnabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    HucCommandData *hucStitchDataBuf = (HucCommandData *)m_osInterface->pfnLockResource(m_osInterface, &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass], &lockFlagsWriteOnly);

    MOS_ZeroMemory(hucStitchDataBuf, sizeof(HucCommandData));
    hucStitchDataBuf->TotalCommands          = 1;
    hucStitchDataBuf->InputCOM[0].SizeOfData = 0xF;

    HucInputCmdG12 hucInputCmd;
    MOS_ZeroMemory(&hucInputCmd, sizeof(HucInputCmdG12));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);
    hucInputCmd.SelectionForIndData = m_osInterface->osCpInterface->IsCpEnabled() ? 4 : 0;
    hucInputCmd.CmdMode             = HUC_CMD_LIST_MODE;
    hucInputCmd.LengthOfTable       = (uint8_t)(m_numTiles);
    hucInputCmd.CopySize            = m_hwInterface->m_tileRecordSize;
    ;

    PMOS_RESOURCE presSrc = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
        m_osInterface,
        presSrc,
        false,
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
        m_osInterface,
        &m_resBitstreamBuffer,
        true,
        true));

    uint64_t srcAddr          = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, presSrc);
    uint64_t destAddr         = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, &m_resBitstreamBuffer);
    hucInputCmd.SrcAddrBottom = (uint32_t)(srcAddr & 0x00000000FFFFFFFF);
    hucInputCmd.SrcAddrTop    = (uint32_t)((srcAddr & 0xFFFFFFFF00000000) >> 32);

    hucInputCmd.DestAddrBottom = (uint32_t)(destAddr & 0x00000000FFFFFFFF);
    hucInputCmd.DestAddrTop    = (uint32_t)((destAddr & 0xFFFFFFFF00000000) >> 32);

    MOS_SecureMemcpy(hucStitchDataBuf->InputCOM[0].data, sizeof(HucInputCmdG12), &hucInputCmd, sizeof(HucInputCmdG12));

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass]);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetRegionsHuCPakIntegrateCqp(
    PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 ||
        (m_hevcSeqParams->RateControlMethod != RATECONTROL_ICQ && m_brcEnabled) ||
        (currentPass != 0 && m_cqpEnabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MOS_ZeroMemory(virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());

    // Add Virtual addr
    virtualAddrParams->regionParams[0].presRegion = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 0 - Tile based input statistics from PAK/ VDEnc
    virtualAddrParams->regionParams[0].dwOffset   = 0;
    virtualAddrParams->regionParams[1].presRegion = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 1 - HuC Frame statistics output
    virtualAddrParams->regionParams[1].isWritable = true;
    virtualAddrParams->regionParams[4].presRegion = &m_resBitstreamBuffer;  // Region 4 - Last Tile bitstream
    virtualAddrParams->regionParams[5].presRegion = &m_resBitstreamBuffer;  // Region 5 - HuC modifies the last tile bitstream before stitch command
    virtualAddrParams->regionParams[5].isWritable = true;
    virtualAddrParams->regionParams[6].presRegion = &m_brcBuffers.resBrcHistoryBuffer;  // Region 6  History Buffer (Input/Output)
    virtualAddrParams->regionParams[6].isWritable = true;
    virtualAddrParams->regionParams[7].presRegion = &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx];  //&m_resHucPakStitchReadBatchBuffer;             // Region 7 - HCP PIC state command

    virtualAddrParams->regionParams[9].presRegion  = &m_resBrcDataBuffer;  // Region 9  HuC outputs BRC data
    virtualAddrParams->regionParams[9].isWritable  = true;
    virtualAddrParams->regionParams[8].presRegion  = &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass];  // Region 8 - data buffer read by HUC for stitching cmd generation
    virtualAddrParams->regionParams[10].presRegion = &m_HucStitchCmdBatchBuffer.OsResource;                         // Region 10 - SLB for stitching cmd output from Huc
    virtualAddrParams->regionParams[10].isWritable = true;
    virtualAddrParams->regionParams[15].presRegion = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;  // Region 15 [In/Out] - Tile Record Buffer
    virtualAddrParams->regionParams[15].dwOffset   = 0;

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodechalEncHevcStateG12::ResetImgCtrlRegInPAKStatisticsBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource      = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
    storeDataParams.dwResourceOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL);
    storeDataParams.dwValue          = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return eStatus;
}
#endif

MOS_STATUS CodechalEncHevcStateG12::ReadBrcPakStatisticsForScalability(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
    MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
    miCpyMemMemParams.presSrc     = &m_resBrcDataBuffer;
    miCpyMemMemParams.dwSrcOffset = CODECHAL_OFFSETOF(PakIntegrationBrcData, FrameByteCount);
    miCpyMemMemParams.presDst     = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
    miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));

    MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
    miCpyMemMemParams.presSrc     = &m_resBrcDataBuffer;
    miCpyMemMemParams.dwSrcOffset = CODECHAL_OFFSETOF(PakIntegrationBrcData, FrameByteCountNoHeader);
    miCpyMemMemParams.presDst     = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
    miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME_NOHEADER);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));

    MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
    miCpyMemMemParams.presSrc     = &m_resBrcDataBuffer;
    miCpyMemMemParams.dwSrcOffset = CODECHAL_OFFSETOF(PakIntegrationBrcData, HCP_ImageStatusControl);
    miCpyMemMemParams.presDst     = &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite];
    miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));

    uint32_t dwOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                        m_encodeStatusBuf.dwNumPassesOffset +  // Num passes offset
                        sizeof(uint32_t) * 2;                  // encodeStatus is offset by 2 DWs in the resource

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource      = &m_encodeStatusBuf.resStatusBuffer;
    storeDataParams.dwResourceOffset = dwOffset;
    storeDataParams.dwValue          = (uint8_t)GetCurrentPass();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::DumpHucDebugOutputBuffers()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //only dump HuC in/out buffers in brc scalability case
    bool dumpDebugBuffers = IsLastPipe() && (m_numPipe >= 2) && m_brcEnabled;
    if (m_singleTaskPhaseSupported)
    {
        dumpDebugBuffers = dumpDebugBuffers && IsLastPass();
    }

    if (dumpDebugBuffers)
    {
        CODECHAL_DEBUG_TOOL(
            int32_t currentPass = GetCurrentPass();
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
                &m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass],
                sizeof(HucPakStitchDmemEncG12),
                currentPass,
                hucRegionDumpPakIntegrate));

            // Region 7 - HEVC PIC State Command
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx],
                0,
                m_hwInterface->m_vdenc2ndLevelBatchBufferSize,
                7,
                "_PicState",
                true,
                currentPass,
                hucRegionDumpPakIntegrate));

            // Region 5 -  Last Tile PAK Bitstream Output
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_resBitstreamBuffer,
                0,
                m_encodeParams.dwBitstreamSize,
                5,
                "_Bitstream",
                false,
                currentPass,
                hucRegionDumpPakIntegrate));

            // Region 6 - BRC History buffer
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_brcBuffers.resBrcHistoryBuffer,
                0,
                m_brcHistoryBufferSize,
                6,
                "_HistoryBuffer",
                false,
                currentPass,
                hucRegionDumpPakIntegrate));
            // Region 9 - HCP BRC Data Output
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_resBrcDataBuffer,
                0,
                CODECHAL_CACHELINE_SIZE,
                9,
                "_HcpBrcData",
                false,
                currentPass,
                hucRegionDumpPakIntegrate));
            // Region 1 - Output Aggregated Frame Level Statistics
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_resHuCPakAggregatedFrameStatsBuffer.sResource,
                0,
                m_hwInterface->m_pakIntAggregatedFrameStatsSize,  // program exact out size
                1,
                "_AggregateFrameStats",
                false,
                currentPass,
                hucRegionDumpPakIntegrate));
            // Region 0 - Tile Statistics Constant Buffer
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource,
                0,
                m_hwInterface->m_pakIntTileStatsSize,
                0,
                "_TileBasedStats",
                true,
                currentPass,
                hucRegionDumpPakIntegrate));
            // Region 15 - Tile Record Buffer
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource,
                0,
                m_tileRecordBuffer[m_virtualEngineBbIndex].dwSize,
                15,
                "_TileRecord",
                false,
                currentPass,
                hucRegionDumpPakIntegrate));)
    }

    return eStatus;
}

CodechalEncHevcStateG12::CodechalEncHevcStateG12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    : CodechalEncHevcState(hwInterface, debugInterface, standardInfo)
{
    m_2xMeSupported =
        m_useCommonKernel = true;
    m_useHwScoreboard     = false;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase          = (uint8_t *)IGCODECKRN_G12;
#else
    m_kernelBase          = nullptr;
#endif
    m_kuidCommon          = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
    m_hucPakStitchEnabled = true;
    m_scalabilityState    = nullptr;

    MOS_ZeroMemory(&m_currPicWithReconBoundaryPix, sizeof(m_currPicWithReconBoundaryPix));
    MOS_ZeroMemory(&m_lcuLevelInputDataSurface, sizeof(m_lcuLevelInputDataSurface));
    MOS_ZeroMemory(&m_encoderHistoryInputBuffer, sizeof(m_encoderHistoryInputBuffer));
    MOS_ZeroMemory(&m_encoderHistoryOutputBuffer, sizeof(m_encoderHistoryOutputBuffer));
    MOS_ZeroMemory(&m_intermediateCuRecordSurfaceLcu32, sizeof(m_intermediateCuRecordSurfaceLcu32));
    MOS_ZeroMemory(&m_scratchSurface, sizeof(m_scratchSurface));
    MOS_ZeroMemory(&m_16x16QpInputData, sizeof(m_16x16QpInputData));
    MOS_ZeroMemory(m_debugSurface, sizeof(m_debugSurface));
    MOS_ZeroMemory(&m_encConstantTableForB, sizeof(m_encConstantTableForB));
    MOS_ZeroMemory(&m_mvAndDistortionSumSurface, sizeof(m_mvAndDistortionSumSurface));
    MOS_ZeroMemory(m_encBCombinedBuffer1, sizeof(m_encBCombinedBuffer1));
    MOS_ZeroMemory(m_encBCombinedBuffer2, sizeof(m_encBCombinedBuffer2));

    MOS_ZeroMemory(&m_resPakcuLevelStreamoutData, sizeof(m_resPakcuLevelStreamoutData));
    MOS_ZeroMemory(&m_resPakSliceLevelStreamoutData, sizeof(m_resPakSliceLevelStreamoutData));
    MOS_ZeroMemory(m_resTileBasedStatisticsBuffer, sizeof(m_resTileBasedStatisticsBuffer));
    MOS_ZeroMemory(&m_resHuCPakAggregatedFrameStatsBuffer, sizeof(m_resHuCPakAggregatedFrameStatsBuffer));
    MOS_ZeroMemory(m_tileRecordBuffer, sizeof(m_tileRecordBuffer));
    MOS_ZeroMemory(&m_kmdVeOveride, sizeof(m_kmdVeOveride));
    MOS_ZeroMemory(&m_resHcpScalabilitySyncBuffer, sizeof(m_resHcpScalabilitySyncBuffer));

    MOS_ZeroMemory(m_veBatchBuffer, sizeof(m_veBatchBuffer));
    MOS_ZeroMemory(&m_realCmdBuffer, sizeof(m_realCmdBuffer));
    MOS_ZeroMemory(&m_resBrcSemaphoreMem, sizeof(m_resBrcSemaphoreMem));
    MOS_ZeroMemory(&m_resBrcPakSemaphoreMem, sizeof(m_resBrcPakSemaphoreMem));
    MOS_ZeroMemory(&m_resPipeStartSemaMem, sizeof(m_resPipeStartSemaMem));
    MOS_ZeroMemory(&m_resPipeCompleteSemaMem, sizeof(m_resPipeCompleteSemaMem));
    MOS_ZeroMemory(m_resHucPakStitchDmemBuffer, sizeof(m_resHucPakStitchDmemBuffer));
    MOS_ZeroMemory(&m_resBrcDataBuffer, sizeof(m_resBrcDataBuffer));
    MOS_ZeroMemory(&m_skipFrameInfo.m_resMbCodeSkipFrameSurface, sizeof(m_skipFrameInfo.m_resMbCodeSkipFrameSurface));

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_HEVC_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize     = CODECHAL_INIT_DSH_SIZE_HEVC_ENC;

    m_kuid             = IDR_CODEC_HEVC_COMBINED_KENREL_INTEL;
    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize);
    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);

    Mos_SetVirtualEngineSupported(m_osInterface, true);
}

CodechalEncHevcStateG12::~CodechalEncHevcStateG12()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_wpState)
    {
        MOS_Delete(m_wpState);
        m_wpState = nullptr;
    }

    if (m_intraDistKernel)
    {
        MOS_Delete(m_intraDistKernel);
        m_intraDistKernel = nullptr;
    }

    if (m_swScoreboardState)
    {
        MOS_Delete(m_swScoreboardState);
        m_swScoreboardState = nullptr;
    }

    if (m_scalabilityState)
    {
        MOS_FreeMemAndSetNull(m_scalabilityState);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_statusReportDebugInterface != nullptr)
    {
        MOS_Delete(m_statusReportDebugInterface);
        m_statusReportDebugInterface = nullptr;
    }
#endif
}

MOS_STATUS CodechalEncHevcStateG12::Allocate(CodechalSetting *codecHalSettings)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (!m_statusReportDebugInterface)
    {
        m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_statusReportDebugInterface->Initialize(m_hwInterface, codecHalSettings->codecFunction));
    }
#endif

    return CodechalEncoderState::Allocate(codecHalSettings);
}

uint32_t CodechalEncHevcStateG12::CodecHalHevc_GetFileSize(char *fileName)
{
    FILE *   fp       = nullptr;
    uint32_t fileSize = 0;
    MOS_SecureFileOpen(&fp, fileName, "rb");
    if (fp == nullptr)
    {
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fclose(fp);

    return fileSize;
}

MOS_STATUS CodechalEncHevcStateG12::LoadSourceAndRef2xDSFromFile(
    PMOS_SURFACE pRef2xSurface,
    PMOS_SURFACE pSrc2xSurface,
    uint8_t      reflist,
    uint8_t      refIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_loadKernelInput == false || (pSrc2xSurface && Mos_ResourceIsNull(&pSrc2xSurface->OsResource)) ||
        (pRef2xSurface && Mos_ResourceIsNull(&pRef2xSurface->OsResource)) ||
        (pSrc2xSurface == NULL && pRef2xSurface == NULL))
    {
        return eStatus;
    }

    char pathOfRef2xDSCmd[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_SecureStringPrint(pathOfRef2xDSCmd,
        sizeof(pathOfRef2xDSCmd),
        sizeof(pathOfRef2xDSCmd),
        "%s\\Ref2xDSL%1d%1d.dat.%d",
        m_loadKernelInputDataFolder,
        reflist,
        refIdx,
        m_frameNum);
    char pathOfSrc2xDSCmd[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_SecureStringPrint(pathOfSrc2xDSCmd,
        sizeof(pathOfSrc2xDSCmd),
        sizeof(pathOfSrc2xDSCmd),
        "%s\\Src2xDS.dat.%d",
        m_loadKernelInputDataFolder,
        m_frameNum);

    uint32_t sizeRef2xDS = CodecHalHevc_GetFileSize(pathOfRef2xDSCmd);
    uint32_t sizeSrc2xDS = CodecHalHevc_GetFileSize(pathOfSrc2xDSCmd);
    if (sizeRef2xDS == 0 && sizeSrc2xDS == 0)
        return MOS_STATUS_SUCCESS;
    MOS_LOCK_PARAMS lockFlags;

    if (pRef2xSurface && sizeRef2xDS)
    {
        if (sizeRef2xDS > (pRef2xSurface->dwPitch * pRef2xSurface->dwHeight * 3 / 2))
        {
            return MOS_STATUS_INVALID_FILE_SIZE;
        }
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface, &pRef2xSurface->OsResource, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        FILE *Ref2xDS = nullptr;
        eStatus       = MOS_SecureFileOpen(&Ref2xDS, pathOfRef2xDSCmd, "rb");
        if (Ref2xDS == nullptr)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &pRef2xSurface->OsResource);
            return eStatus;
        }

        uint32_t sizeToRead = sizeRef2xDS * 2 / 3;
        if (sizeToRead != fread((void *)data, 1, sizeToRead, Ref2xDS))
        {
            fclose(Ref2xDS);
            m_osInterface->pfnUnlockResource(m_osInterface, &pRef2xSurface->OsResource);
            return MOS_STATUS_INVALID_FILE_SIZE;
        }
        fclose(Ref2xDS);
        //MOS_ZeroMemory(data + sizeToRead, sizeRef2xDS-sizeToRead);

        m_osInterface->pfnUnlockResource(m_osInterface, &pRef2xSurface->OsResource);
    }

    if (pSrc2xSurface && sizeSrc2xDS)
    {
        if (sizeSrc2xDS > (pSrc2xSurface->dwPitch * pSrc2xSurface->dwHeight * 3 / 2))
        {
            return MOS_STATUS_INVALID_FILE_SIZE;
        }

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface, &pSrc2xSurface->OsResource, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        FILE *Src2xDS = nullptr;
        eStatus       = MOS_SecureFileOpen(&Src2xDS, pathOfSrc2xDSCmd, "rb");
        if (Src2xDS == nullptr)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &pSrc2xSurface->OsResource);
            return eStatus;
        }

        uint32_t sizeToRead = sizeSrc2xDS * 2 / 3;
        if (sizeToRead != fread((void *)data, 1, sizeToRead, Src2xDS))
        {
            fclose(Src2xDS);
            m_osInterface->pfnUnlockResource(m_osInterface, &pSrc2xSurface->OsResource);
            return MOS_STATUS_INVALID_FILE_SIZE;
        }
        fclose(Src2xDS);
        //MOS_ZeroMemory(data + sizeToRead, sizeRef2xDS-sizeToRead);

        m_osInterface->pfnUnlockResource(m_osInterface, &pSrc2xSurface->OsResource);
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::LoadPakCommandAndCuRecordFromFile()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    char pathOfPakCmd[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_SecureStringPrint(pathOfPakCmd,
        sizeof(pathOfPakCmd),
        sizeof(pathOfPakCmd),
        "%s\\PAKObj.dat.%d",
        m_pakOnlyDataFolder,
        m_frameNum);

    char pathOfCuRecord[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_SecureStringPrint(pathOfCuRecord,
        sizeof(pathOfCuRecord),
        sizeof(pathOfCuRecord),
        "%s\\CURecord.dat.%d",
        m_pakOnlyDataFolder,
        m_frameNum);

    uint32_t sizePakObj = CodecHalHevc_GetFileSize(pathOfPakCmd);
    if (sizePakObj == 0 || sizePakObj > m_mvOffset)
    {
        return MOS_STATUS_INVALID_FILE_SIZE;
    }

    uint32_t sizeCuRecord = CodecHalHevc_GetFileSize(pathOfCuRecord);
    if (sizeCuRecord == 0 || sizeCuRecord > m_mbCodeSize - m_mvOffset)
    {
        return MOS_STATUS_INVALID_FILE_SIZE;
    }

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface, &m_resMbCodeSurface, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    FILE *pakObj = nullptr;
    eStatus      = MOS_SecureFileOpen(&pakObj, pathOfPakCmd, "rb");
    if (pakObj == nullptr)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);
        return eStatus;
    }

    uint8_t *pakCmd = data;
    if (sizePakObj != fread((void *)pakCmd, 1, sizePakObj, pakObj))
    {
        fclose(pakObj);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);
        return MOS_STATUS_INVALID_FILE_SIZE;
    }
    fclose(pakObj);

    uint8_t *record  = data + m_mvOffset;
    FILE *   fRecord = nullptr;
    eStatus          = MOS_SecureFileOpen(&fRecord, pathOfCuRecord, "rb");
    if (fRecord == nullptr)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);
        return eStatus;
    }

    if (sizeCuRecord != fread((void *)record, 1, sizeCuRecord, fRecord))
    {
        fclose(fRecord);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);
        return MOS_STATUS_INVALID_FILE_SIZE;
    }
    fclose(fRecord);

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);

    if (m_brcEnabled)
    {
        //Image State
        char pathOfPicState[MOS_USER_CONTROL_MAX_DATA_SIZE];
        MOS_SecureStringPrint(pathOfPicState,
            sizeof(pathOfPicState),
            sizeof(pathOfPicState),
            "%s\\BrcUpdate_ImgStateWrite.dat.%d",
            m_pakOnlyDataFolder,
            m_frameNum);

        uint32_t sizePicState = CodecHalHevc_GetFileSize(pathOfPicState);
        if (sizePicState == 0)
        {
            return MOS_STATUS_INVALID_FILE_SIZE;
        }

        data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface, &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx], &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        FILE *fPicState = nullptr;
        eStatus         = MOS_SecureFileOpen(&fPicState, pathOfPicState, "rb");
        if (fPicState == nullptr)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx]);
            return eStatus;
        }

        if (sizePicState != fread((void *)data, 1, sizePicState, fPicState))
        {
            fclose(fPicState);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx]);
            return MOS_STATUS_INVALID_FILE_SIZE;
        }
        fclose(fPicState);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_brcBuffers.resBrcImageStatesWriteBuffer[m_currRecycledBufIdx]);
    }

    return eStatus;
}

uint8_t CodechalEncHevcStateG12::PicCodingTypeToSliceType(uint16_t pictureCodingType)
{
    uint8_t sliceType = 0;

    switch (pictureCodingType)
    {
    case I_TYPE:
        sliceType = CODECHAL_ENCODE_HEVC_I_SLICE;
        break;
    case P_TYPE:
        sliceType = CODECHAL_ENCODE_HEVC_P_SLICE;
        break;
    case B_TYPE:
    case B1_TYPE:
    case B2_TYPE:
        sliceType = CODECHAL_ENCODE_HEVC_B_SLICE;
        break;
    default:
        CODECHAL_ENCODE_ASSERT(false);
    }
    return sliceType;
}

// The following code is from the kernel ULT
MOS_STATUS CodechalEncHevcStateG12::InitMediaObjectWalker(
    uint32_t           threadSpaceWidth,
    uint32_t           threadSpaceHeight,
    uint32_t           colorCountMinusOne,
    DependencyPattern  dependencyPattern,
    uint32_t           childThreadNumber,
    uint32_t           localLoopExecCount,
    MHW_WALKER_PARAMS &walkerParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    walkerParams.ColorCountMinusOne    = colorCountMinusOne;
    walkerParams.dwGlobalLoopExecCount = 0x3ff;
    walkerParams.dwLocalLoopExecCount  = 0x3ff;

    if (dependencyPattern == dependencyWavefrontHorizontal)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = 0;
        walkerParams.LocalInnerLoopUnit.y = 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;
    }
    else if (dependencyPattern == dependencyWavefrontVertical)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 0;
        walkerParams.LocalOutLoopStride.y = 1;
        walkerParams.LocalInnerLoopUnit.x = 1;
        walkerParams.LocalInnerLoopUnit.y = 0;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;
    }
    else if (dependencyPattern == dependencyWavefront45Degree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -1;
        walkerParams.LocalInnerLoopUnit.y = 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;
    }
    else if (dependencyPattern == dependencyWavefront26Degree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -2;
        walkerParams.LocalInnerLoopUnit.y = 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;
    }
    else if ((dependencyPattern == dependencyWavefront45XDegree) ||
             (dependencyPattern == dependencyWavefront45XDegreeAlt))
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -1;
        walkerParams.LocalInnerLoopUnit.y = childThreadNumber + 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = childThreadNumber;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 1;
    }
    else if ((dependencyPattern == dependencyWavefront26XDegree) ||
             (dependencyPattern == dependencyWavefront26XDegreeAlt))
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -2;
        walkerParams.LocalInnerLoopUnit.y = childThreadNumber + 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = childThreadNumber;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 1;
    }
    else if (dependencyPattern == dependencyWavefront45XVp9Degree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -1;
        walkerParams.LocalInnerLoopUnit.y = 4;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 3;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 1;
    }
    else if (dependencyPattern == dependencyWavefront26ZDegree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = 2;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = -4;
        walkerParams.GlobalInnerLoopUnit.y    = 2;

        // Local
        walkerParams.BlockResolution.x    = 2;
        walkerParams.BlockResolution.y    = 2;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 0;
        walkerParams.LocalOutLoopStride.y = 1;
        walkerParams.LocalInnerLoopUnit.x = 1;
        walkerParams.LocalInnerLoopUnit.y = 0;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;
    }
    else if (dependencyPattern == dependencyWavefront26ZigDegree)
    {
        int32_t size_x = threadSpaceWidth;   //(threadSpaceWidth + 1)>> 1;
        int32_t size_y = threadSpaceHeight;  //threadSpaceHeight << 1;

        // Global
        walkerParams.GlobalResolution.x       = size_x;
        walkerParams.GlobalResolution.y       = size_y;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = size_x;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = size_y;

        // Local
        walkerParams.BlockResolution.x    = size_x;
        walkerParams.BlockResolution.y    = size_y;
        walkerParams.LocalStart.x         = 0;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -2;
        walkerParams.LocalInnerLoopUnit.y = 4;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 3;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 1;
    }
    else if (dependencyPattern == dependencyWavefront45DDegree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = threadSpaceWidth;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -1;
        walkerParams.LocalInnerLoopUnit.y = 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;
        if (colorCountMinusOne > 0)
        {
            walkerParams.dwLocalLoopExecCount = localLoopExecCount;
        }
    }
    else if (dependencyPattern == dependencyWavefront26DDegree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;
        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = threadSpaceWidth;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -2;
        walkerParams.LocalInnerLoopUnit.y = 1;
        // Mid
        walkerParams.MiddleLoopExtraSteps = 0;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 0;

        if (colorCountMinusOne > 0)
        {
            walkerParams.dwLocalLoopExecCount = localLoopExecCount;
        }
    }
    else if (dependencyPattern == dependencyWavefront45XDDegree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;

        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = threadSpaceWidth;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -1;
        walkerParams.LocalInnerLoopUnit.y = childThreadNumber + 1;

        // Mid
        walkerParams.MiddleLoopExtraSteps = childThreadNumber;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 1;
        if (colorCountMinusOne > 0)
        {
            walkerParams.dwLocalLoopExecCount = localLoopExecCount;
        }
    }
    else if (dependencyPattern == dependencyWavefront26XDDegree)
    {
        // Global
        walkerParams.GlobalResolution.x       = threadSpaceWidth;
        walkerParams.GlobalResolution.y       = threadSpaceHeight;
        walkerParams.GlobalStart.x            = 0;
        walkerParams.GlobalStart.y            = 0;
        walkerParams.GlobalOutlerLoopStride.x = threadSpaceWidth;
        walkerParams.GlobalOutlerLoopStride.y = 0;
        walkerParams.GlobalInnerLoopUnit.x    = 0;
        walkerParams.GlobalInnerLoopUnit.y    = threadSpaceHeight;
        // Local
        walkerParams.BlockResolution.x    = threadSpaceWidth;
        walkerParams.BlockResolution.y    = threadSpaceHeight;
        walkerParams.LocalStart.x         = threadSpaceWidth;
        walkerParams.LocalStart.y         = 0;
        walkerParams.LocalOutLoopStride.x = 1;
        walkerParams.LocalOutLoopStride.y = 0;
        walkerParams.LocalInnerLoopUnit.x = -2;
        walkerParams.LocalInnerLoopUnit.y = childThreadNumber + 1;
        // Mid
        walkerParams.MiddleLoopExtraSteps = childThreadNumber;
        walkerParams.MidLoopUnitX         = 0;
        walkerParams.MidLoopUnitY         = 1;

        if (colorCountMinusOne > 0)
        {
            walkerParams.dwLocalLoopExecCount = localLoopExecCount;
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported walking pattern is observed\n");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }
    return eStatus;
}

bool CodechalEncHevcStateG12::IsDegree45Needed()
{
    if (m_numberConcurrentGroup == 1 && m_numberEncKernelSubThread == 1)
    {
        return false;
    }
    return true;
}

void CodechalEncHevcStateG12::DecideConcurrentGroupAndWaveFrontNumber()
{
    uint32_t          shift       = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
    uint32_t          widthInLcu  = MOS_ROUNDUP_SHIFT((m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1), shift);
    uint32_t          heightInLcu = MOS_ROUNDUP_SHIFT((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1), shift);
    DependencyPattern walkerDegree;

    //As per kernel ULT,for all non TU1 cases m_numberEncKernelSubThread should be set to 1
    // LCU32 has no multiple thread support,
    if (!m_isMaxLcu64 || m_hevcSeqParams->TargetUsage != 1)
    {
        m_numberEncKernelSubThread = 1;  // LCU32 has no multiple thread support
    }

    while (heightInLcu / m_numberConcurrentGroup == 0)
    {
        m_numberConcurrentGroup = m_numberConcurrentGroup >> 1;
        if (m_numberConcurrentGroup == 0)
        {
            // Try out all values and now have to use the default ones.
            // Concurrent group and wave-front split must be enabled together
            m_numberConcurrentGroup = 1;
            break;
        }
    }

    if (m_numberConcurrentGroup > 1)
    {
        m_numWavefrontInOneRegion = 0;
        while (m_numWavefrontInOneRegion == 0)
        {
            uint32_t shift = m_degree45Needed ? 0 : 1;

            m_numWavefrontInOneRegion =
                (widthInLcu + ((heightInLcu - 1) << shift) + m_numberConcurrentGroup - 1) / m_numberConcurrentGroup;

            if (m_numWavefrontInOneRegion > 0)
            {
                // this is a valid setting and number of regisions is greater than or equal to 1
                break;
            }
            m_numberConcurrentGroup = m_numberConcurrentGroup >> 1;
            if (m_numberConcurrentGroup == 0)
            {
                // Try out all values and now have to use the default ones.
                m_numberConcurrentGroup = 1;
                break;
            }
        }
    }
    else
    {
        m_numWavefrontInOneRegion = 0;
    }

    m_numberEncKernelSubThread = MOS_MIN(m_numberEncKernelSubThread, m_hevcThreadTaskDataNum);

    return;
}

void CodechalEncHevcStateG12::InitSwScoreBoardParams(CodechalEncodeSwScoreboard::KernelParams &swScoreboardKernelParames)
{
    uint32_t widthAlignedMaxLcu;
    uint32_t heightAlignedMaxLcu;
    uint32_t widthAlignedLcu32;
    uint32_t heightAlignedLcu32;

    if (m_mfeEnabled && m_colorBitMfeEnabled)
    {
        widthAlignedMaxLcu  = MOS_ALIGN_CEIL(m_mfeEncodeParams.maxWidth, MAX_LCU_SIZE);
        heightAlignedMaxLcu = MOS_ALIGN_CEIL(m_mfeEncodeParams.maxHeight, MAX_LCU_SIZE);
        widthAlignedLcu32   = MOS_ALIGN_CEIL(m_mfeEncodeParams.maxWidth, 32);
        heightAlignedLcu32  = MOS_ALIGN_CEIL(m_mfeEncodeParams.maxHeight, 32);
    }
    else
    {
        widthAlignedMaxLcu  = m_widthAlignedMaxLcu;
        heightAlignedMaxLcu = m_heightAlignedMaxLcu;
        widthAlignedLcu32   = m_widthAlignedLcu32;
        heightAlignedLcu32  = m_heightAlignedLcu32;
    }

    // SW scoreboard Kernel Call -- to be continued - DS + HME kernel call
    swScoreboardKernelParames.isHevc = false;  // can be set to false. Need to enabled only for an optimization which is not needed for now

    m_degree45Needed = true;
    if (m_hevcSeqParams->TargetUsage == 1)
    {
        m_numberConcurrentGroup = MOS_MIN(m_maxWavefrontsforTU1, m_numberConcurrentGroup);
        // m_numberConcurrentGroup should  default to 2 here for TU1. the only other value allowed from reg key will be 1
        m_degree45Needed = false;
    }

    DecideConcurrentGroupAndWaveFrontNumber();

    DependencyPattern walkPattern;
    if (m_hevcSeqParams->TargetUsage == 1)
    {
        if (m_isMaxLcu64)
        {
            walkPattern = m_numberConcurrentGroup == 1 ? dependencyWavefront26XDegreeAlt : dependencyWavefront26XDDegree;
        }
        else
        {
            walkPattern = m_numberConcurrentGroup == 1 ? dependencyWavefront26Degree : dependencyWavefront26DDegree;
        }
    }
    else if (m_hevcSeqParams->TargetUsage == 4)
    {
        walkPattern = m_numberConcurrentGroup == 1 ? dependencyWavefront45Degree : dependencyWavefront45DDegree;
    }
    else
    {
        walkPattern = dependencyWavefront45DDegree;
    }
    m_swScoreboardState->SetDependencyPattern(walkPattern);

    if (m_isMaxLcu64)
    {
        if (m_hevcSeqParams->TargetUsage == 1)
        {
            swScoreboardKernelParames.scoreboardWidth  = (widthAlignedMaxLcu >> 6);
            swScoreboardKernelParames.scoreboardHeight = (heightAlignedMaxLcu >> 6) * m_numberEncKernelSubThread;
        }
        else
        {
            swScoreboardKernelParames.scoreboardWidth  = 2 * (widthAlignedMaxLcu >> 6);
            swScoreboardKernelParames.scoreboardHeight = 2 * (heightAlignedMaxLcu >> 6);
        }
        swScoreboardKernelParames.numberOfWaveFrontSplit = m_numberConcurrentGroup;
        swScoreboardKernelParames.numberOfChildThread    = m_numberEncKernelSubThread - 1;  // child thread number is minus one of the total sub-thread for the main thread takes one.
    }
    else
    {
        swScoreboardKernelParames.scoreboardWidth        = widthAlignedLcu32 >> 5;
        swScoreboardKernelParames.scoreboardHeight       = heightAlignedLcu32 >> 5;
        swScoreboardKernelParames.numberOfWaveFrontSplit = m_numberConcurrentGroup;
        swScoreboardKernelParames.numberOfChildThread    = 0;
    }

    swScoreboardKernelParames.swScoreboardSurfaceWidth  = swScoreboardKernelParames.scoreboardWidth;
    swScoreboardKernelParames.swScoreboardSurfaceHeight = swScoreboardKernelParames.scoreboardHeight;

    m_swScoreboardState->SetCurSwScoreboardSurfaceIndex(m_currRecycledBufIdx);

    swScoreboardKernelParames.lcuInfoSurface = &m_lcuLevelInputDataSurface[m_currRecycledBufIdx];
}

MOS_STATUS CodechalEncHevcStateG12::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncHevcState::UserFeatureKeyReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID, m_numberConcurrentGroup);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_SUBTHREAD_NUM_ID_G12, m_numberEncKernelSubThread);
    CodecHalEncode_WriteKey64(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VE_DEBUG_OVERRIDE_G12, m_kmdVeOveride.Value);

    if (m_pakOnlyTest)
    {
        CodecHalEncode_WriteStringKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_PAK_ONLY_ID_G12, m_pakOnlyDataFolder, strlen(m_pakOnlyDataFolder));
    }
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_USED_VDBOX_NUM_ID, m_numPipe);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID_G12, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));
#endif

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetupSwScoreBoard(CodechalEncodeSwScoreboard::KernelParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (Mos_ResourceIsNull(&m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource))
    {
        MOS_ZeroMemory(m_swScoreboardState->GetCurSwScoreboardSurface(), sizeof(*m_swScoreboardState->GetCurSwScoreboardSurface()));

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
        MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
        allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
        allocParamsForBuffer2D.Format   = Format_R32U;
        allocParamsForBuffer2D.dwWidth  = params->swScoreboardSurfaceWidth;
        allocParamsForBuffer2D.dwHeight = params->swScoreboardSurfaceHeight;
        allocParamsForBuffer2D.pBufName = "SW Scoreboard Init buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            m_swScoreboardState->GetCurSwScoreboardSurface()));
    }

    if (m_swScoreboard == nullptr)
    {
        m_swScoreboard = (uint8_t *)MOS_AllocAndZeroMemory(params->scoreboardWidth * sizeof(uint32_t) * params->scoreboardHeight);
        InitSWScoreboard(m_swScoreboard, params->scoreboardWidth, params->scoreboardHeight, m_swScoreboardState->GetDependencyPattern(), (char)(params->numberOfChildThread));
    }

    MOS_LOCK_PARAMS lockFlags;

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    for (uint32_t h = 0; h < params->scoreboardHeight; h++)
    {
        uint32_t s = params->scoreboardWidth * sizeof(uint32_t);
        MOS_SecureMemcpy(data, s, &m_swScoreboard[h * s], s);
        data += m_swScoreboardState->GetCurSwScoreboardSurface()->dwPitch;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource);

    return eStatus;
}

void CodechalEncHevcStateG12::SetDependency(
    uint8_t &numDependencies,
    char *   scoreboardDeltaX,
    char *   scoreboardDeltaY,
    uint32_t dependencyPattern,
    char     childThreadNumber)
{
    if (dependencyPattern == dependencyWavefrontHorizontal)
    {
        numDependencies = m_numDependencyHorizontal;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependencyHorizontal, m_dxWavefrontHorizontal, m_numDependencyHorizontal);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependencyHorizontal, m_dyWavefrontHorizontal, m_numDependencyHorizontal);
    }
    else if (dependencyPattern == dependencyWavefrontVertical)
    {
        numDependencies = m_numDependencyVertical;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependencyVertical, m_dxWavefrontVertical, m_numDependencyVertical);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependencyVertical, m_dyWavefrontVertical, m_numDependencyVertical);
    }
    else if (dependencyPattern == dependencyWavefront45Degree)
    {
        numDependencies = m_numDependency45Degree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency45Degree, m_dxWavefront45Degree, m_numDependency45Degree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency45Degree, m_dyWavefront45Degree, m_numDependency45Degree);
    }
    else if (dependencyPattern == dependencyWavefront26Degree ||
             dependencyPattern == dependencyWavefront26DDegree)
    {
        numDependencies = m_numDependency26Degree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency26Degree, m_dxWavefront26Degree, m_numDependency26Degree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency26Degree, m_dyWavefront26Degree, m_numDependency26Degree);
    }
    else if (dependencyPattern == dependencyWavefront45XDegree)
    {
        numDependencies = m_numDependency45xDegree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency45xDegree, m_dxWavefront45xDegree, m_numDependency45xDegree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency45xDegree, m_dyWavefront45xDegree, m_numDependency45xDegree);
        numDependencies     = childThreadNumber + 2;
        scoreboardDeltaY[0] = childThreadNumber;
    }
    else if (dependencyPattern == dependencyWavefront26XDegree)
    {
        numDependencies = m_numDependency26xDegree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency26xDegree, m_dxWavefront26xDegree, m_numDependency26xDegree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency26xDegree, m_dyWavefront26xDegree, m_numDependency26xDegree);
        numDependencies     = childThreadNumber + 3;
        scoreboardDeltaY[0] = childThreadNumber;
    }
    else if ((dependencyPattern == dependencyWavefront45XDegreeAlt) ||
             (dependencyPattern == dependencyWavefront45XDDegree))
    {
        numDependencies = m_numDependency45xDegreeAlt;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency45xDegreeAlt, m_dxWavefront45xDegreeAlt, m_numDependency45xDegreeAlt);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency45xDegreeAlt, m_dyWavefront45xDegreeAlt, m_numDependency45xDegreeAlt);
        scoreboardDeltaY[0] = childThreadNumber;
    }
    else if ((dependencyPattern == dependencyWavefront26XDegreeAlt) ||
             (dependencyPattern == dependencyWavefront26XDDegree))
    {
        numDependencies = m_numDependency26xDegreeAlt;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency26xDegreeAlt, m_dxWavefront26xDegreeAlt, m_numDependency26xDegreeAlt);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency26xDegreeAlt, m_dyWavefront26xDegreeAlt, m_numDependency26xDegreeAlt);
        scoreboardDeltaY[0] = childThreadNumber;
    }
    else if (dependencyPattern == dependencyWavefront45XVp9Degree)
    {
        numDependencies = m_numDependency45xVp9Degree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency45xVp9Degree, m_dxWavefront45xVp9Degree, m_numDependency45xVp9Degree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency45xVp9Degree, m_dyWavefront45xVp9Degree, m_numDependency45xVp9Degree);
    }
    else if (dependencyPattern == dependencyWavefront26ZDegree)
    {
        numDependencies = m_numDependency26zDegree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency26zDegree, m_dxWavefront26zDegree, m_numDependency26zDegree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency26zDegree, m_dyWavefront26zDegree, m_numDependency26zDegree);
    }
    else if (dependencyPattern == dependencyWavefront26ZigDegree)
    {
        numDependencies = m_numDependency26ZigDegree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency26ZigDegree, m_dxWavefront26ZigDegree, m_numDependency26ZigDegree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency26ZigDegree, m_dyWavefront26ZigDegree, m_numDependency26ZigDegree);
    }
    else if (dependencyPattern == dependencyWavefront45DDegree)
    {
        numDependencies = m_numDependency45Degree;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependency45Degree, m_dxWavefront45Degree, m_numDependency45Degree);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependency45Degree, m_dyWavefront45Degree, m_numDependency45Degree);
    }
    else
    {
        numDependencies = m_numDependencyNone;
        MOS_SecureMemcpy(scoreboardDeltaX, m_numDependencyNone, m_dxWavefrontNone, m_numDependencyNone);
        MOS_SecureMemcpy(scoreboardDeltaY, m_numDependencyNone, m_dyWavefrontNone, m_numDependencyNone);
    }
}

// ========================================================================================
// FUNCTION:        InitSWScoreboard
// DESCRIPTION:        Initialize software scoreboard for a specific dependency pattern.
// INPUTS:            scoreboardWidth - Width of scoreboard in Entries
//                    scoreboardHeight - Height of scoreboard in Entries
//                    dependencyPattern - The Enumeration of the Dependency Pattern
// OUTPUTS:            scoreboard - Pointer to scoreboard in Memory
// ========================================================================================
void CodechalEncHevcStateG12::InitSWScoreboard(uint8_t *scoreboard, uint32_t scoreboardWidth, uint32_t scoreboardHeight, uint32_t dependencyPattern, char childThreadNumber)
{
    // 1. Select Dependency Pattern
    uint8_t numDependencies;
    char    scoreboardDeltaX[m_maxNumDependency];
    char    scoreboardDeltaY[m_maxNumDependency];
    memset(scoreboardDeltaX, 0, sizeof(scoreboardDeltaX));
    memset(scoreboardDeltaY, 0, sizeof(scoreboardDeltaY));

    SetDependency(numDependencies, scoreboardDeltaX, scoreboardDeltaY, dependencyPattern, childThreadNumber);

    // 2. Initialize scoreboard (CPU Based)
    int32_t   dependentLocationX = 0;
    int32_t   dependentLocationY = 0;
    uint32_t *scoreboardInDws    = (uint32_t *)scoreboard;
    int32_t   totalThreadNumber  = childThreadNumber + 1;
    for (int32_t y = 0; y < (int32_t)scoreboardHeight; y += totalThreadNumber)
    {
        for (int32_t x = 0; x < (int32_t)scoreboardWidth; x++)
        {
            scoreboardInDws[y * scoreboardWidth + x] = 0;

            // Add dependencies accordingly
            for (int32_t i = 0; i < numDependencies; i++)
            {
                dependentLocationX = x + scoreboardDeltaX[i];
                dependentLocationY = y + scoreboardDeltaY[i];
                if ((dependentLocationX < 0) || (dependentLocationY < 0) ||
                    (dependentLocationX >= (int32_t)scoreboardWidth) ||
                    (dependentLocationY >= (int32_t)scoreboardHeight))
                {
                    // Do not add dependency because thread does not exist
                }
                else
                {
                    scoreboardInDws[y * scoreboardWidth + x] |= (1 << i);
                }
            }  // End NumDep
        }      // End x

        for (int32_t n = y + 1; n < y + totalThreadNumber; n++)
        {
            for (int32_t k = 0; k < (int32_t)scoreboardWidth; k++)
            {
                scoreboardInDws[n * scoreboardWidth + k] = scoreboardInDws[y * scoreboardWidth + k];
            }
        }

    }  // End y
}

void CodechalEncHevcStateG12::CreateMhwParams()
{
    m_sliceStateParams     = MOS_New(MHW_VDBOX_HEVC_SLICE_STATE_G12);
    m_pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12);
    m_pipeBufAddrParams    = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12);
}

MOS_STATUS CodechalEncHevcStateG12::CalculatePictureStateCommandSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetHxxStateCommandSize(
            CODECHAL_ENCODE_MODE_HEVC,
            &m_defaultPictureStatesSize,
            &m_defaultPicturePatchListSize,
            &stateCmdSizeParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    *m_pipeBufAddrParams = {};
    SetHcpPipeBufAddrParams(*m_pipeBufAddrParams);
#ifdef _MMC_SUPPORTED
    m_mmcState->SetPipeBufAddr(m_pipeBufAddrParams);
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(cmdBuffer, m_pipeBufAddrParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetTileData(
    MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 *tileCodingParams,
    uint32_t                              bitstreamBufSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        return eStatus;
    }

    uint32_t colBd[100]       = {0};
    uint32_t num_tile_columns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    for (uint32_t i = 0; i < num_tile_columns; i++)
    {
        colBd[i + 1] = colBd[i] + m_hevcPicParams->tile_column_width[i];
    }

    uint32_t rowBd[100]    = {0};
    uint32_t num_tile_rows = m_hevcPicParams->num_tile_rows_minus1 + 1;
    for (uint32_t i = 0; i < num_tile_rows; i++)
    {
        rowBd[i + 1] = rowBd[i] + m_hevcPicParams->tile_row_height[i];
    }

    m_numTiles = num_tile_rows * num_tile_columns;

    uint32_t const uiNumCuRecordTab[]  = {1, 4, 16, 64};  //LCU: 8x8->1, 16x16->4, 32x32->16, 64x64->64
    uint32_t       numCuRecord         = uiNumCuRecordTab[MOS_MIN(3, m_hevcSeqParams->log2_max_coding_block_size_minus3)];
    uint32_t       bitstreamByteOffset = 0, saoRowstoreOffset = 0, cuLevelStreamoutOffset = 0, sseRowstoreOffset = 0;
    int32_t        frameWidthInMinCb  = m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1;
    int32_t        frameHeightInMinCb = m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1;
    int32_t        shift              = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
    uint32_t       NumLCUInPic        = 0;

    for (uint32_t i = 0; i < num_tile_rows; i++)
    {
        for (uint32_t j = 0; j < num_tile_columns; j++)
        {
            NumLCUInPic += m_hevcPicParams->tile_row_height[i] * m_hevcPicParams->tile_column_width[j];
        }
    }

    uint32_t numSliceInTile = 0;
    for (uint32_t uiNumLCUsInTiles = 0, i = 0; i < num_tile_rows; i++)
    {
        for (uint32_t j = 0; j < num_tile_columns; j++)
        {
            uint32_t idx          = i * num_tile_columns + j;
            uint32_t numLCUInTile = m_hevcPicParams->tile_row_height[i] * m_hevcPicParams->tile_column_width[j];

            tileCodingParams[idx].TileStartLCUX = colBd[j];
            tileCodingParams[idx].TileStartLCUY = rowBd[i];

            tileCodingParams[idx].TileColumnStoreSelect = j % 2;
            tileCodingParams[idx].TileRowStoreSelect    = i % 2;

            if (j != num_tile_columns - 1)
            {
                tileCodingParams[idx].TileWidthInMinCbMinus1 = (m_hevcPicParams->tile_column_width[j] << shift) - 1;
                tileCodingParams[idx].IsLastTileofRow        = false;
            }
            else
            {
                tileCodingParams[idx].TileWidthInMinCbMinus1 = (frameWidthInMinCb - (colBd[j] << shift)) - 1;
                tileCodingParams[idx].IsLastTileofRow        = true;
            }

            if (i != num_tile_rows - 1)
            {
                tileCodingParams[idx].IsLastTileofColumn      = false;
                tileCodingParams[idx].TileHeightInMinCbMinus1 = (m_hevcPicParams->tile_row_height[i] << shift) - 1;
            }
            else
            {
                tileCodingParams[idx].TileHeightInMinCbMinus1 = (frameHeightInMinCb - (rowBd[i] << shift)) - 1;
                tileCodingParams[idx].IsLastTileofColumn      = true;
            }

            tileCodingParams[idx].NumOfTilesInFrame       = m_numTiles;
            tileCodingParams[idx].NumOfTileColumnsInFrame = num_tile_columns;
            tileCodingParams[idx].CuRecordOffset          = MOS_ALIGN_CEIL(((numCuRecord * uiNumLCUsInTiles) * m_hcpInterface->GetHevcEncCuRecordSize()),
                                                       CODECHAL_CACHELINE_SIZE) /
                                                   CODECHAL_CACHELINE_SIZE;
            tileCodingParams[idx].NumberOfActiveBePipes = (m_numPipe > 1) ? m_numPipe : 1;

            tileCodingParams[idx].PakTileStatisticsOffset              = m_sizeOfHcpPakFrameStats * idx / CODECHAL_CACHELINE_SIZE;
            tileCodingParams[idx].TileSizeStreamoutOffset              = idx;
            tileCodingParams[idx].Vp9ProbabilityCounterStreamoutOffset = 0;
            tileCodingParams[idx].presHcpSyncBuffer                    = &m_resHcpScalabilitySyncBuffer.sResource;
            tileCodingParams[idx].CuLevelStreamoutOffset               = cuLevelStreamoutOffset;
            tileCodingParams[idx].SliceSizeStreamoutOffset             = numSliceInTile;
            tileCodingParams[idx].SseRowstoreOffset                    = sseRowstoreOffset;
            tileCodingParams[idx].BitstreamByteOffset                  = bitstreamByteOffset;
            tileCodingParams[idx].SaoRowstoreOffset                    = saoRowstoreOffset;

            cuLevelStreamoutOffset += MOS_ALIGN_CEIL((tileCodingParams[idx].TileWidthInMinCbMinus1 + 1) * (tileCodingParams[idx].TileHeightInMinCbMinus1 + 1) * 16, CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;
            sseRowstoreOffset += ((m_hevcPicParams->tile_column_width[j] + 3) * m_sizeOfSseSrcPixelRowStoreBufferPerLcu) / CODECHAL_CACHELINE_SIZE;
            saoRowstoreOffset += (MOS_ALIGN_CEIL(m_hevcPicParams->tile_column_width[j], 4) * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU) / CODECHAL_CACHELINE_SIZE;
            uint64_t totalSizeTemp        = (uint64_t)bitstreamBufSize * (uint64_t)numLCUInTile;
            uint32_t bitStreamSizePerTile = (uint32_t)(totalSizeTemp / (uint64_t)NumLCUInPic) + ((totalSizeTemp % (uint64_t)NumLCUInPic) ? 1 : 0);
            bitstreamByteOffset += MOS_ALIGN_CEIL(bitStreamSizePerTile, CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;
            uiNumLCUsInTiles += numLCUInTile;

            for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
            {
                bool lastSliceInTile = false, sliceInTile = false;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                    &tileCodingParams[idx],
                    &sliceInTile,
                    &lastSliceInTile));
                numSliceInTile += (sliceInTile ? 1 : 0);
            }
        }
        // same row store buffer for different tile rows.
        saoRowstoreOffset = 0;
        sseRowstoreOffset = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::IsSliceInTile(
    uint32_t                              sliceNumber,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 currentTile,
    bool *                                sliceInTile,
    bool *                                lastSliceInTile)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(currentTile);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceInTile);
    CODECHAL_ENCODE_CHK_NULL_RETURN(lastSliceInTile);

    uint32_t shift            = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
    uint32_t residual         = (1 << shift) - 1;
    uint32_t frameWidthInLCU  = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1 + residual) >> shift;
    uint32_t frameHeightInLCU = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1 + residual) >> shift;

    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = &m_hevcSliceParams[sliceNumber];
    uint32_t                        sliceStartLCU = hevcSlcParams->slice_segment_address;
    uint32_t                        sliceLCUx     = sliceStartLCU % frameWidthInLCU;
    uint32_t                        sliceLCUy     = sliceStartLCU / frameWidthInLCU;

    uint32_t tile_column_width = (currentTile->TileWidthInMinCbMinus1 + 1 + residual) >> shift;
    uint32_t tile_row_height   = (currentTile->TileHeightInMinCbMinus1 + 1 + residual) >> shift;
    if (sliceLCUx < currentTile->TileStartLCUX ||
        sliceLCUy < currentTile->TileStartLCUY ||
        sliceLCUx >= currentTile->TileStartLCUX + tile_column_width ||
        sliceLCUy >= currentTile->TileStartLCUY + tile_row_height)
    {
        // slice start is not in the tile boundary
        *lastSliceInTile = *sliceInTile = false;
        return eStatus;
    }

    sliceLCUx += (hevcSlcParams->NumLCUsInSlice - 1) % tile_column_width;
    sliceLCUy += (hevcSlcParams->NumLCUsInSlice - 1) / tile_column_width;

    if (sliceLCUx >= currentTile->TileStartLCUX + tile_column_width)
    {
        sliceLCUx -= tile_column_width;
        sliceLCUy++;
    }

    if (sliceLCUx < currentTile->TileStartLCUX ||
        sliceLCUy < currentTile->TileStartLCUY ||
        sliceLCUx >= currentTile->TileStartLCUX + tile_column_width ||
        sliceLCUy >= currentTile->TileStartLCUY + tile_row_height)
    {
        // last LCU of the slice is out of the tile boundary
        *lastSliceInTile = *sliceInTile = false;
        return eStatus;
    }

    *sliceInTile = true;

    sliceLCUx++;
    sliceLCUy++;

    // the end of slice is at the boundary of tile
    *lastSliceInTile = (sliceLCUx == currentTile->TileStartLCUX + tile_column_width &&
                        sliceLCUy == currentTile->TileStartLCUY + tile_row_height);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AddHcpRefIdxCmd(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    PMHW_BATCH_BUFFER           batchBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcPicParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = params->pEncodeHevcPicParams;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS   hevcSlcParams = params->pEncodeHevcSliceParams;

    if (hevcSlcParams->slice_type != CODECHAL_ENCODE_HEVC_I_SLICE)
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 refIdxParams;

        refIdxParams.CurrPic         = hevcPicParams->CurrReconstructedPic;
        refIdxParams.isEncode        = true;
        refIdxParams.ucList          = LIST_0;
        refIdxParams.ucNumRefForList = hevcSlcParams->num_ref_idx_l0_active_minus1 + 1;
        eStatus                      = MOS_SecureMemcpy(&refIdxParams.RefPicList, sizeof(refIdxParams.RefPicList), &hevcSlcParams->RefPicList, sizeof(hevcSlcParams->RefPicList));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        refIdxParams.hevcRefList  = (void **)m_refList;
        refIdxParams.poc_curr_pic = hevcPicParams->CurrPicOrderCnt;
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = hevcPicParams->RefFramePOCList[i];
        }

        refIdxParams.pRefIdxMapping     = params->pRefIdxMapping;
        refIdxParams.RefFieldPicFlag    = 0;  // there is no interlaced support in encoder
        refIdxParams.RefBottomFieldFlag = 0;  // there is no interlaced support in encoder

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(cmdBuffer, batchBuffer, &refIdxParams));

        if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
        {
            refIdxParams.ucList          = LIST_1;
            refIdxParams.ucNumRefForList = hevcSlcParams->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(cmdBuffer, batchBuffer, &refIdxParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER   cmdBuffer,
    bool                  frameTrackingRequested,
    MHW_MI_MMIOREGISTERS *mmioRegister)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl      = false;
    forceWakeupParams.bMFXPowerWellControlMask  = true;
    forceWakeupParams.bHEVCPowerWellControl     = true;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
        cmdBuffer,
        &forceWakeupParams));

    if (UseRenderCommandBuffer())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTrackingRequested, mmioRegister));
        return eStatus;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(m_miInterface, cmdBuffer, gpuContext));
#endif

    if (!IsLastPipe())
    {
        return eStatus;
    }

    PMOS_COMMAND_BUFFER commandBufferInUse;
    if (m_realCmdBuffer.pCmdBase)
    {
        commandBufferInUse = &m_realCmdBuffer;
    }
    else if (cmdBuffer && cmdBuffer->pCmdBase)
    {
        commandBufferInUse = cmdBuffer;
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // initialize command buffer attributes
    commandBufferInUse->Attributes.bTurboMode               = m_hwInterface->m_turboMode;
    commandBufferInUse->Attributes.dwNumRequestedEUSlices   = m_hwInterface->m_numRequestedEuSlices;
    commandBufferInUse->Attributes.dwNumRequestedSubSlices  = m_hwInterface->m_numRequestedSubSlices;
    commandBufferInUse->Attributes.dwNumRequestedEUs        = m_hwInterface->m_numRequestedEus;
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
    genericPrologParams.pOsInterface     = m_hwInterface->GetOsInterface();
    genericPrologParams.pvMiInterface    = m_hwInterface->GetMiInterface();
    genericPrologParams.bMmcEnabled      = CodecHalMmcState::IsMmcEnabled();
    genericPrologParams.dwStoreDataValue = m_storeData - 1;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(commandBufferInUse, &genericPrologParams));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeHevcG12, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL

//MOS_STATUS CodechalEncHevcStateG12::CodecHal_DbgDumpHEVCMbEncCurbeG12(
//    CodechalDebugInterface         *pDebugInterface,
//    CODECHAL_MEDIA_STATE_TYPE       Function,
//    PMOS_RESOURCE                   presDBuffer)
//{
//#define WRITE_CURBE_FIELD_TO_FILE(field) {\
//    MOS_SecureStringPrint(sOutBuf, sizeof(sOutBuf), sizeof(sOutBuf), "field = %d\n", pCurbeData->field);\
//    CodecHal_DbgAddStringToBufferNewLine(&FileParams, sOutBuf);}
//
//    PMOS_INTERFACE              m_osInterface = nullptr;
//    PCCHAR                      pcFunction = nullptr;
//    char                        sAttrib[125];
//    char                        sOutBuf[MAX_FIELD_LENGTH];
//    CODECHAL_DBG_FILE_PARAMS    FileParams;
//    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
//    MOS_LOCK_PARAMS             LockFlags;
//    CodechalEncHevcStateG12::MBENC_COMBINED_BUFFER1 *pEncComBuf1 = nullptr;
//
//    CODECHAL_DEBUG_FUNCTION_ENTER;
//
//    CODECHAL_DEBUG_CHK_NULL(pDebugInterface);
//    CODECHAL_DEBUG_CHK_NULL(pDebugInterface->pOsInterface);
//    CODECHAL_DEBUG_CHK_NULL(pDebugInterface->pHwInterface);
//    m_osInterface = pDebugInterface->pOsInterface;
//
//    pcFunction = CodecHal_DbgGetFunctionType(
//        pDebugInterface, Function, DBG_CMD_BUFFER_DUMP_DEFAULT);
//    CODECHAL_DEBUG_CHK_NULL(pcFunction);
//
//    MOS_SecureStringPrint(sAttrib, sizeof(sAttrib), sizeof(sAttrib), "%s%s", pcFunction, CODECHAL_DBG_STRING_CURBE);
//
//    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
//    LockFlags.ReadOnly = 1;
//
//    pEncComBuf1 = (CodechalEncHevcStateG12::MBENC_COMBINED_BUFFER1*)m_osInterface->pfnLockResource(
//        m_osInterface,
//        presDBuffer,
//        &LockFlags);
//
//    FileParams = g_cInitDbgFileParams;
//
//    if (!CodecHal_DbgAttribIsEnabled(pDebugInterface, sAttrib))
//    {
//        return eStatus;
//    }
//
//    MOS_ZeroMemory(pDebugInterface->sPath, sizeof(pDebugInterface->sPath));
//
//    CODECHAL_DEBUG_CHK_STATUS(CodecHal_DbgConstructFilenameString(
//        pDebugInterface,
//        pcFunction,
//        CODECHAL_DBG_STRING_CURBE,
//        CODECHAL_DBG_STRING_TXT));
//
//    if (CodecHal_DbgAttribIsEnabled(pDebugInterface, CODECHAL_DBG_STRING_DUMPDATAINBINARY))
//    {
//        CODECHAL_DEBUG_CHK_STATUS(CodecHal_DbgDumpBufferInHexDwords(
//            pDebugInterface,
//            (uint8_t*)&pEncComBuf1->Curbe,
//            sizeof(pEncComBuf1->Curbe)));
//    }
//    else
//    {
//        CodechalEncHevcStateG12::MBENC_CURBE* pCurbeData = &pEncComBuf1->Curbe;
//
//        FileParams.lRemaining = sizeof(char)* MAX_FIELD_LENGTH * MAX_NUM_ATTRIBUTES;
//        FileParams.psWriteToFile = (char*)MOS_AllocAndZeroMemory(FileParams.lRemaining);
//        CODECHAL_DEBUG_CHK_NULL(FileParams.psWriteToFile);
//        FileParams.dwOffset = 0;
//
//        memset(sOutBuf, 0, sizeof(sOutBuf));
//
//        MOS_SecureStringPrint(sOutBuf, sizeof(sOutBuf), sizeof(sOutBuf), "# CURBE Parameters:");
//        CodecHal_DbgAddStringToBufferNewLine(&FileParams, sOutBuf);
//
//        WRITE_CURBE_FIELD_TO_FILE(FrameWidthInSamples);
//        WRITE_CURBE_FIELD_TO_FILE(FrameHeightInSamples);
//
//        WRITE_CURBE_FIELD_TO_FILE(Log2MaxCUSize);
//        WRITE_CURBE_FIELD_TO_FILE(Log2MinCUSize);
//        WRITE_CURBE_FIELD_TO_FILE(Log2MaxTUSize);
//        WRITE_CURBE_FIELD_TO_FILE(Log2MinTUSize);
//        WRITE_CURBE_FIELD_TO_FILE(MaxIntraRdeIter);
//        WRITE_CURBE_FIELD_TO_FILE(QPType);
//        WRITE_CURBE_FIELD_TO_FILE(MaxTransformDepthInter);
//        WRITE_CURBE_FIELD_TO_FILE(MaxTransformDepthIntra);
//        WRITE_CURBE_FIELD_TO_FILE(Log2ParallelMergeLevel);
//
//        WRITE_CURBE_FIELD_TO_FILE(CornerNeighborPixel);
//        WRITE_CURBE_FIELD_TO_FILE(IntraNeighborAvailFlags);
//        WRITE_CURBE_FIELD_TO_FILE(ChromaFormatType);
//        WRITE_CURBE_FIELD_TO_FILE(SubPelMode);
//        WRITE_CURBE_FIELD_TO_FILE(InterSADMeasure);
//        WRITE_CURBE_FIELD_TO_FILE(IntraSADMeasure);
//        WRITE_CURBE_FIELD_TO_FILE(IntraPrediction);
//        WRITE_CURBE_FIELD_TO_FILE(RefIDCostMode);
//        WRITE_CURBE_FIELD_TO_FILE(TUBasedCostSetting);
//
//        WRITE_CURBE_FIELD_TO_FILE(ExplictModeEn);
//        WRITE_CURBE_FIELD_TO_FILE(AdaptiveEn);
//        WRITE_CURBE_FIELD_TO_FILE(EarlyImeSuccessEn);
//        WRITE_CURBE_FIELD_TO_FILE(IntraSpeedMode);
//        WRITE_CURBE_FIELD_TO_FILE(IMECostCentersSel);
//        WRITE_CURBE_FIELD_TO_FILE(RDEQuantRoundValue);
//        WRITE_CURBE_FIELD_TO_FILE(IMERefWindowSize);
//        WRITE_CURBE_FIELD_TO_FILE(IntraComputeType);
//        WRITE_CURBE_FIELD_TO_FILE(Depth0IntraPredition);
//        WRITE_CURBE_FIELD_TO_FILE(TUDepthControl);
//        WRITE_CURBE_FIELD_TO_FILE(IntraTuRecFeedbackDisable);
//        WRITE_CURBE_FIELD_TO_FILE(MergeListBiDisable);
//        WRITE_CURBE_FIELD_TO_FILE(EarlyImeStop);
//
//        WRITE_CURBE_FIELD_TO_FILE(FrameQP);
//        WRITE_CURBE_FIELD_TO_FILE(FrameQPSign);
//        WRITE_CURBE_FIELD_TO_FILE(ConcurrentGroupNum);
//        WRITE_CURBE_FIELD_TO_FILE(NumofUnitInWaveFront);
//
//        WRITE_CURBE_FIELD_TO_FILE(LoadBalenceEnable);
//        WRITE_CURBE_FIELD_TO_FILE(NumberofMultiFrame);
//        WRITE_CURBE_FIELD_TO_FILE(Degree45);
//        WRITE_CURBE_FIELD_TO_FILE(Break12Dependency);
//        WRITE_CURBE_FIELD_TO_FILE(ThreadNumber);
//
//        WRITE_CURBE_FIELD_TO_FILE(Pic_init_qp_B);
//        WRITE_CURBE_FIELD_TO_FILE(Pic_init_qp_P);
//        WRITE_CURBE_FIELD_TO_FILE(Pic_init_qp_I);
//
//        WRITE_CURBE_FIELD_TO_FILE(NumofRowTile);
//        WRITE_CURBE_FIELD_TO_FILE(NumofColumnTile);
//
//        WRITE_CURBE_FIELD_TO_FILE(TransquantBypassEnableFlag);
//        WRITE_CURBE_FIELD_TO_FILE(PCMEnabledFlag);
//        WRITE_CURBE_FIELD_TO_FILE(CuQpDeltaEnabledFlag);
//        WRITE_CURBE_FIELD_TO_FILE(Stepping);
//        WRITE_CURBE_FIELD_TO_FILE(WaveFrontSplitsEnable);
//        WRITE_CURBE_FIELD_TO_FILE(HMEFlag);
//        WRITE_CURBE_FIELD_TO_FILE(SuperHME);
//        WRITE_CURBE_FIELD_TO_FILE(UltraHME);
//        WRITE_CURBE_FIELD_TO_FILE(Cu64SkipCheckOnly);
//        WRITE_CURBE_FIELD_TO_FILE(EnableCu64Check);
//        WRITE_CURBE_FIELD_TO_FILE(Cu642Nx2NCheckOnly);
//        WRITE_CURBE_FIELD_TO_FILE(EnableCu64AmpCheck);
//        WRITE_CURBE_FIELD_TO_FILE(DisablePIntra);
//        WRITE_CURBE_FIELD_TO_FILE(DisableIntraTURec);
//        WRITE_CURBE_FIELD_TO_FILE(InheritIntraModeFromTU0);
//        WRITE_CURBE_FIELD_TO_FILE(CostScalingForRA);
//        WRITE_CURBE_FIELD_TO_FILE(DisableIntraNxN);
//
//        WRITE_CURBE_FIELD_TO_FILE(MaxRefIdxL0);
//        WRITE_CURBE_FIELD_TO_FILE(MaxRefIdxL1);
//        WRITE_CURBE_FIELD_TO_FILE(MaxBRefIdxL0);
//
//        WRITE_CURBE_FIELD_TO_FILE(SkipEarlyTermination);
//        WRITE_CURBE_FIELD_TO_FILE(SkipEarlyTermSize);
//        WRITE_CURBE_FIELD_TO_FILE(Dynamic64Enable);
//        WRITE_CURBE_FIELD_TO_FILE(Dynamic64Order);
//        WRITE_CURBE_FIELD_TO_FILE(Dynamic64Th);
//        WRITE_CURBE_FIELD_TO_FILE(DynamicOrderTh);
//        WRITE_CURBE_FIELD_TO_FILE(PerBFrameQPOffset);
//        WRITE_CURBE_FIELD_TO_FILE(IncreaseExitThresh);
//        WRITE_CURBE_FIELD_TO_FILE(Dynamic64Min32);
//        WRITE_CURBE_FIELD_TO_FILE(LastFrameIsIntra);
//
//        WRITE_CURBE_FIELD_TO_FILE(LenSP);
//        WRITE_CURBE_FIELD_TO_FILE(MaxNumSU);
//
//        WRITE_CURBE_FIELD_TO_FILE(CostTableIndex);
//
//        WRITE_CURBE_FIELD_TO_FILE(SliceType);
//        WRITE_CURBE_FIELD_TO_FILE(TemporalMvpEnableFlag);
//        WRITE_CURBE_FIELD_TO_FILE(CollocatedFromL0Flag);
//        WRITE_CURBE_FIELD_TO_FILE(theSameRefList);
//        WRITE_CURBE_FIELD_TO_FILE(IsLowDelay);
//        WRITE_CURBE_FIELD_TO_FILE(MaxNumMergeCand);
//        WRITE_CURBE_FIELD_TO_FILE(NumRefIdxL0);
//        WRITE_CURBE_FIELD_TO_FILE(NumRefIdxL1);
//
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_0);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_0);
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_1);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_1);
//
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_2);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_2);
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_3);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_3);
//
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_4);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_4);
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_5);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_5);
//
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_6);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_6);
//        WRITE_CURBE_FIELD_TO_FILE(FwdPocNumber_L0_mTb_7);
//        WRITE_CURBE_FIELD_TO_FILE(BwdPocNumber_L1_mTb_7);
//
//        WRITE_CURBE_FIELD_TO_FILE(LongTermReferenceFlags_L0);
//        WRITE_CURBE_FIELD_TO_FILE(LongTermReferenceFlags_L1);
//
//        WRITE_CURBE_FIELD_TO_FILE(RefFrameWinWidth);
//        WRITE_CURBE_FIELD_TO_FILE(RefFrameWinHeight);
//
//        WRITE_CURBE_FIELD_TO_FILE(RoundingInter);
//        WRITE_CURBE_FIELD_TO_FILE(RoundingIntra);
//        WRITE_CURBE_FIELD_TO_FILE(MaxThreadWidth);
//        WRITE_CURBE_FIELD_TO_FILE(MaxThreadHeight);
//
//        CODECHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(
//            pDebugInterface->sPath,
//            FileParams.psWriteToFile,
//            FileParams.dwOffset));
//    }
//
//finish:
//    if (m_osInterface && pEncComBuf1)
//    {
//        m_osInterface->pfnUnlockResource(
//            m_osInterface,
//            presDBuffer);
//    }
//
//    if (FileParams.psWriteToFile)
//    {
//        MOS_FreeMemory(FileParams.psWriteToFile);
//    }
//    return eStatus;
//}

#endif
MOS_STATUS CodechalEncHevcStateG12::VerifyCommandBufferSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode & resize CommandBuffer Size for every BRC pass
        if (!m_singleTaskPhaseSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
        }
        return eStatus;
    }

    // virtual engine
    uint32_t requestedSize =
        m_pictureStatesSize +
        m_extraPictureStatesSize +
        (m_sliceStatesSize * m_numSlices);

    requestedSize += (requestedSize * m_numPassesInOnePipe + m_hucCommandsSize);

    // Running in the multiple VDBOX mode
    int currentPipe = GetCurrentPipe();
    if (currentPipe < 0 || currentPipe >= m_numPipe)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (IsFirstPipe() && m_osInterface->bUsesPatchList)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    PMOS_COMMAND_BUFFER pCmdBuffer = m_singleTaskPhaseSupported ? &m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][0] : &m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][currentPass];

    if (Mos_ResourceIsNull(&pCmdBuffer->OsResource) ||
        m_sizeOfVeBatchBuffer < requestedSize)
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes  = requestedSize;
        allocParamsForBufferLinear.pBufName = "Batch buffer for each VDBOX";

        if (!Mos_ResourceIsNull(&pCmdBuffer->OsResource))
        {
            if (pCmdBuffer->pCmdBase)
            {
                m_osInterface->pfnUnlockResource(m_osInterface, &pCmdBuffer->OsResource);
            }
            m_osInterface->pfnFreeResource(m_osInterface, &pCmdBuffer->OsResource);
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &pCmdBuffer->OsResource));

        m_sizeOfVeBatchBuffer = requestedSize;
    }

    if (pCmdBuffer->pCmdBase == nullptr)
    {
        MOS_LOCK_PARAMS lockParams;
        MOS_ZeroMemory(&lockParams, sizeof(lockParams));
        lockParams.WriteOnly = true;
        pCmdBuffer->pCmdPtr = pCmdBuffer->pCmdBase = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &pCmdBuffer->OsResource, &lockParams);
        pCmdBuffer->iRemaining                     = m_sizeOfVeBatchBuffer;
        pCmdBuffer->iOffset                        = 0;

        if (pCmdBuffer->pCmdBase == nullptr)
        {
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode
        m_realCmdBuffer.pCmdBase = m_realCmdBuffer.pCmdPtr = nullptr;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, 0));
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_realCmdBuffer, 0));

    int currentPipe = GetCurrentPipe();
    if (currentPipe < 0 || currentPipe >= m_numPipe)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    *cmdBuffer = m_singleTaskPhaseSupported ? m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][0] : m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][currentPass];

    if (m_osInterface->osCpInterface->IsCpEnabled() && cmdBuffer->iOffset == 0)
    {
        // Insert CP Prolog
        CODECHAL_ENCODE_NORMALMESSAGE("Adding cp prolog for secure scalable encode");
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->AddProlog(m_osInterface, cmdBuffer));
    }
    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);
        return eStatus;
    }

    int currentPipe = GetCurrentPipe();
    if (currentPipe < 0 || currentPipe >= m_numPipe)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    uint8_t passIndex                                               = m_singleTaskPhaseSupported ? 0 : currentPass;
    m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][passIndex] = *cmdBuffer;
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_realCmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode
        if (!UseRenderCommandBuffer())  // Set VE Hints for video contexts only
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
        return eStatus;
    }

    bool cmdBufferReadyForSubmit = IsLastPipe();

    // In STF, Hold the command buffer submission till last pass
    if (m_singleTaskPhaseSupported)
    {
        cmdBufferReadyForSubmit = cmdBufferReadyForSubmit && IsLastPass();
    }

    if (!cmdBufferReadyForSubmit)
    {
        return eStatus;
    }

    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;

    for (uint32_t i = 0; i < m_numPipe; i++)
    {
        PMOS_COMMAND_BUFFER cmdBuffer = &m_veBatchBuffer[m_virtualEngineBbIndex][i][passIndex];

        if (cmdBuffer->pCmdBase)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
        }

        cmdBuffer->pCmdBase = 0;
        cmdBuffer->iOffset = cmdBuffer->iRemaining = 0;
    }
    m_sizeOfVeBatchBuffer = 0;

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&m_realCmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_realCmdBuffer, nullRendering));
    }

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::SetSliceStructs()
{
    MOS_STATUS eStatus   = MOS_STATUS_SUCCESS;
    eStatus              = CodechalEncodeHevcBase::SetSliceStructs();
    m_numPassesInOnePipe = m_numPasses;
    m_numPasses          = (m_numPasses + 1) * m_numPipe - 1;
    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AllocateTileStatistics()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        return eStatus;
    }

    auto num_tile_rows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    auto num_tile_columns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    auto num_tiles        = num_tile_rows * num_tile_columns;

    MOS_ZeroMemory(&m_hevcFrameStatsOffset, sizeof(HEVC_TILE_STATS_INFO));
    MOS_ZeroMemory(&m_hevcTileStatsOffset, sizeof(HEVC_TILE_STATS_INFO));
    MOS_ZeroMemory(&m_hevcStatsSize, sizeof(HEVC_TILE_STATS_INFO));

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    // Set the maximum size based on frame level statistics.
    m_hevcStatsSize.uiTileSizeRecord     = CODECHAL_CACHELINE_SIZE;
    m_hevcStatsSize.uiHevcPakStatistics  = m_sizeOfHcpPakFrameStats;
    m_hevcStatsSize.uiVdencStatistics    = 0;
    m_hevcStatsSize.uiHevcSliceStreamout = CODECHAL_CACHELINE_SIZE;

    // Maintain the offsets to use for patching addresses in to the HuC Pak Integration kernel Aggregated Frame Statistics Output Buffer
    // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
    m_hevcFrameStatsOffset.uiTileSizeRecord     = 0;  // Tile Size Record is not present in resHuCPakAggregatedFrameStatsBuffer
    m_hevcFrameStatsOffset.uiHevcPakStatistics  = 0;
    m_hevcFrameStatsOffset.uiVdencStatistics    = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.uiHevcPakStatistics + m_hevcStatsSize.uiHevcPakStatistics, CODECHAL_PAGE_SIZE);
    m_hevcFrameStatsOffset.uiHevcSliceStreamout = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.uiVdencStatistics + m_hevcStatsSize.uiVdencStatistics, CODECHAL_PAGE_SIZE);

    // Frame level statistics
    m_hwInterface->m_pakIntAggregatedFrameStatsSize = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.uiHevcSliceStreamout + (m_hevcStatsSize.uiHevcSliceStreamout * CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6), CODECHAL_PAGE_SIZE);

    // HEVC Frame Statistics Buffer - Output from HuC PAK Integration kernel
    if (Mos_ResourceIsNull(&m_resHuCPakAggregatedFrameStatsBuffer.sResource))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntAggregatedFrameStatsSize;
        allocParamsForBufferLinear.pBufName = "GEN11 HCP Aggregated Frame Statistics Streamout Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resHuCPakAggregatedFrameStatsBuffer.sResource));
        m_resHuCPakAggregatedFrameStatsBuffer.dwSize = m_hwInterface->m_pakIntAggregatedFrameStatsSize;

        uint8_t *pData = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resHuCPakAggregatedFrameStatsBuffer.sResource,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
        MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resHuCPakAggregatedFrameStatsBuffer.sResource);
    }

    // Maintain the offsets to use for patching addresses in to the Tile Based Statistics Buffer
    // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
    m_hevcTileStatsOffset.uiTileSizeRecord     = 0;  // TileReord is in a separated resource
    m_hevcTileStatsOffset.uiHevcPakStatistics  = 0;  // PakStaticstics is head of m_resTileBasedStatisticsBuffer
    m_hevcTileStatsOffset.uiVdencStatistics    = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.uiHevcPakStatistics + (m_hevcStatsSize.uiHevcPakStatistics * num_tiles), CODECHAL_PAGE_SIZE);
    m_hevcTileStatsOffset.uiHevcSliceStreamout = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.uiVdencStatistics + (m_hevcStatsSize.uiVdencStatistics * num_tiles), CODECHAL_PAGE_SIZE);
    // Combined statistics size for all tiles
    m_hwInterface->m_pakIntTileStatsSize = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.uiHevcSliceStreamout + m_hevcStatsSize.uiHevcSliceStreamout * CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6, CODECHAL_PAGE_SIZE);

    // Tile size record size for all tiles
    m_hwInterface->m_tileRecordSize = m_hevcStatsSize.uiTileSizeRecord * num_tiles;

    if (Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource) || m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].dwSize < m_hwInterface->m_pakIntTileStatsSize)
    {
        if (!Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource);
        }
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntTileStatsSize;
        allocParamsForBufferLinear.pBufName = "GEN11 HCP Tile Level Statistics Streamout Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource));
        m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].dwSize = m_hwInterface->m_pakIntTileStatsSize;

        uint8_t *pData = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource,
            &lockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource);
    }

    if (Mos_ResourceIsNull(&m_tileRecordBuffer[m_virtualEngineBbIndex].sResource) || m_tileRecordBuffer[m_virtualEngineBbIndex].dwSize < m_hwInterface->m_tileRecordSize)
    {
        if (!Mos_ResourceIsNull(&m_tileRecordBuffer[m_virtualEngineBbIndex].sResource))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource);
        }
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_tileRecordSize;
        allocParamsForBufferLinear.pBufName = "Tile Record Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource));
        m_tileRecordBuffer[m_virtualEngineBbIndex].dwSize = m_hwInterface->m_tileRecordSize;

        uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource,
            &lockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource);
    }

    return eStatus;
}

void CodechalEncHevcStateG12::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpPipeBufAddrParams(pipeBufAddrParams);

    // SAO Row Store is GEN12 specific
    pipeBufAddrParams.presSaoRowStoreBuffer = &m_SAORowStoreBuffer;

    PCODECHAL_ENCODE_BUFFER tileStatisticsBuffer = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex];
    if (!Mos_ResourceIsNull(&tileStatisticsBuffer->sResource) && (m_numPipe > 1))
    {
        pipeBufAddrParams.presLcuBaseAddressBuffer     = &tileStatisticsBuffer->sResource;
        pipeBufAddrParams.dwLcuStreamOutOffset         = m_hevcTileStatsOffset.uiHevcSliceStreamout;
        pipeBufAddrParams.presFrameStatStreamOutBuffer = &tileStatisticsBuffer->sResource;
        pipeBufAddrParams.dwFrameStatStreamOutOffset   = m_hevcTileStatsOffset.uiHevcPakStatistics;
    }
}

MOS_STATUS CodechalEncHevcStateG12::ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_sseEnabled)
    {
        return eStatus;
    }

    // encodeStatus is offset by 2 DWs in the resource
    uint32_t sseOffsetinBytes = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2 + m_encodeStatusBuf.dwSumSquareErrorOffset;
    for (auto i = 0; i < 6; i++)  // 64 bit SSE values for luma/ chroma channels need to be copied
    {
        MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
        MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
        miCpyMemMemParams.presSrc     = m_hevcPicParams->tiles_enabled_flag && (m_numPipe > 1) ? &m_resHuCPakAggregatedFrameStatsBuffer.sResource : &m_resFrameStatStreamOutBuffer;
        miCpyMemMemParams.dwSrcOffset = (HEVC_PAK_STATISTICS_SSE_OFFSET + i) * sizeof(uint32_t);  // SSE luma offset is located at DW32 in Frame statistics, followed by chroma
        miCpyMemMemParams.presDst     = &m_encodeStatusBuf.resStatusBuffer;
        miCpyMemMemParams.dwDstOffset = sseOffsetinBytes + i * sizeof(uint32_t);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));
    }
    return eStatus;
}

void CodechalEncHevcStateG12::SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjBaseAddrParams)
{
    PCODECHAL_ENCODE_BUFFER tileRecordBuffer    = &m_tileRecordBuffer[m_virtualEngineBbIndex];
    bool                    useTileRecordBuffer = !Mos_ResourceIsNull(&tileRecordBuffer->sResource);

    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode                        = CODECHAL_ENCODE_MODE_HEVC;
    indObjBaseAddrParams.presMvObjectBuffer          = IsPanicModePass() ? &m_skipFrameInfo.m_resMbCodeSkipFrameSurface : &m_resMbCodeSurface;
    indObjBaseAddrParams.dwMvObjectOffset            = m_mvOffset;
    indObjBaseAddrParams.dwMvObjectSize              = m_mbCodeSize - m_mvOffset;
    indObjBaseAddrParams.presPakBaseObjectBuffer     = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize         = m_bitstreamUpperBound;
    indObjBaseAddrParams.presPakTileSizeStasBuffer   = useTileRecordBuffer ? &tileRecordBuffer->sResource : nullptr;
    indObjBaseAddrParams.dwPakTileSizeStasBufferSize = useTileRecordBuffer ? m_hwInterface->m_tileRecordSize : 0;
    indObjBaseAddrParams.dwPakTileSizeRecordOffset   = useTileRecordBuffer ? m_hevcTileStatsOffset.uiTileSizeRecord : 0;
}

MOS_STATUS CodechalEncHevcStateG12::UpdateCmdBufAttribute(
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

MOS_STATUS CodechalEncHevcStateG12::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

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

    if (m_numPipe >= 2)
    {
        int32_t currentPass = GetCurrentPass();
        if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
        for (auto i = 0; i < m_numPipe; i++)
        {
            scalSetParms.veBatchBuffer[i] = m_veBatchBuffer[m_virtualEngineBbIndex][i][passIndex].OsResource;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_SetHintParams(this, m_scalabilityState, &scalSetParms));
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_PopulateHintParams(m_scalabilityState, cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalEncHevcStateG12::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER   cmdBuffer,
    SendKernelCmdsParams *params)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VFE_PARAMS_G12 vfeParams       = {};
    vfeParams.pKernelState             = params->pKernelState;
    vfeParams.eVfeSliceDisable         = MHW_VFE_SLICE_ALL;
    vfeParams.dwMaximumNumberofThreads = m_encodeVfeMaxThreads;
    vfeParams.bFusedEuDispatch         = false;  // legacy mode

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncHevcStateG12::DumpFrameStatsBuffer(CodechalDebugInterface *debugInterface)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(debugInterface);

    PMOS_RESOURCE resBuffer = &m_resFrameStatStreamOutBuffer;
    uint32_t      offset    = 0;
    uint32_t      num_tiles = 1;
    //In scalable mode, HEVC PAK Frame Statistics gets dumped out for each tile
    if (m_numPipe > 1)
    {
        resBuffer = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;
        offset    = m_hevcTileStatsOffset.uiHevcPakStatistics;
        num_tiles = (m_hevcPicParams->num_tile_rows_minus1 + 1) * (m_hevcPicParams->num_tile_columns_minus1 + 1);
    }
    uint32_t size = MOS_ALIGN_CEIL(m_sizeOfHcpPakFrameStats * num_tiles, CODECHAL_CACHELINE_SIZE);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        resBuffer,
        CodechalDbgAttr::attrFrameState,
        "FrameStatus",
        size,
        offset,
        CODECHAL_NUM_MEDIA_STATES));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncHevcStateG12::DumpPakOutput()
{
    std::string currPassName = "PAK_PASS" + std::to_string((int)m_currPass);

    CODECHAL_DEBUG_TOOL(
        int32_t currentPass = GetCurrentPass();
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resPakcuLevelStreamoutData.sResource,
            CodechalDbgAttr::attrCUStreamout,
            currPassName.data(),
            m_resPakcuLevelStreamoutData.dwSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource,
            CodechalDbgAttr::attrTileBasedStats,
            currPassName.data(),
            m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].dwSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcPakStatisticBuffer[m_brcBuffers.uiCurrBrcPakStasIdxForWrite],
            CodechalDbgAttr::attrBrcPakStats,
            currPassName.data(),
            m_hevcBrcPakStatisticsSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_HucStitchCmdBatchBuffer.OsResource,
            CodechalDbgAttr::attr2ndLvlBatchMfx,
            currPassName.data(),
            m_hwInterface->m_HucStitchCmdBatchBufferSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass],
            CodechalDbgAttr::attrHuCStitchDataBuf,
            currPassName.data(),
            sizeof(HucCommandData),
            0,
            CODECHAL_NUM_MEDIA_STATES));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
            &m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass],
            sizeof(HucPakStitchDmemEncG12),
            currentPass,
            hucRegionDumpPakIntegrate));)

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS CodechalEncHevcStateG12::EncodeMeKernel()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hmeKernel && m_hmeKernel->Is4xMeEnabled())
    {
        CodechalKernelHme::CurbeParam curbeParam;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeCurbeParams(curbeParam));

        CodechalKernelHme::SurfaceParams surfaceParam;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeSurfaceParams(surfaceParam));

        m_hmeKernel->setnoMEKernelForPFrame(m_lowDelay);

        if (m_hmeKernel->Is16xMeEnabled())
        {
            if (m_hmeKernel->Is32xMeEnabled())
            {
                surfaceParam.downScaledWidthInMb         = m_downscaledWidthInMb32x;
                surfaceParam.downScaledHeightInMb        = m_downscaledFrameFieldHeightInMb32x;
                surfaceParam.downScaledBottomFieldOffset = m_scaled32xBottomFieldOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel32x));
            }
            surfaceParam.downScaledWidthInMb         = m_downscaledWidthInMb16x;
            surfaceParam.downScaledHeightInMb        = m_downscaledFrameFieldHeightInMb16x;
            surfaceParam.downScaledBottomFieldOffset = m_scaled16xBottomFieldOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel16x));
        }
        surfaceParam.downScaledWidthInMb         = m_downscaledWidthInMb4x;
        surfaceParam.downScaledHeightInMb        = m_downscaledFrameFieldHeightInMb4x;
        surfaceParam.downScaledBottomFieldOffset = m_scaledBottomFieldOffset;
        surfaceParam.meBrcDistortionSurface      = m_brcBuffers.meBrcDistortionSurface;

        curbeParam.sumMVThreshold = m_sumMVThreshold;

        m_lastTaskInPhase = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel4x));
    }

    return MOS_STATUS_SUCCESS;
}

void CodechalEncHevcStateG12::ResizeBufferOffset()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    //Re-calculate aligned frame width/height + aligned Max LCU width/height when resolution reset occurs
    uint32_t frameWidth    = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    uint32_t frameHeight   = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    uint32_t widthAlignedMaxLcu  = MOS_ALIGN_CEIL(frameWidth, MAX_LCU_SIZE);
    uint32_t heightAlignedMaxLcu = MOS_ALIGN_CEIL(frameHeight, MAX_LCU_SIZE);

    uint32_t               size     = 0;
    const uint32_t         numLcu64 = widthAlignedMaxLcu * heightAlignedMaxLcu / 64 / 64;
    MBENC_COMBINED_BUFFER2 fixedBuf;

    //Re-Calculate m_encBCombinedBuffer2 Size and Offsets
    m_historyOutBufferSize = MOS_ALIGN_CEIL(32 * numLcu64, CODECHAL_CACHELINE_SIZE);
    m_threadTaskBufferSize = MOS_ALIGN_CEIL(96 * numLcu64, CODECHAL_CACHELINE_SIZE);

    size = MOS_ALIGN_CEIL(sizeof(fixedBuf), CODECHAL_CACHELINE_SIZE) + m_historyOutBufferSize + m_threadTaskBufferSize;

    m_historyOutBufferOffset = MOS_ALIGN_CEIL(sizeof(fixedBuf), CODECHAL_CACHELINE_SIZE);
    m_threadTaskBufferOffset = m_historyOutBufferOffset + m_historyOutBufferSize;
}
