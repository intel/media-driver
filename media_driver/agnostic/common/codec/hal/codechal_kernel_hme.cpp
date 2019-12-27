/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_kernel_hme.cpp
//! \brief    Defines the hme kernel base
//! \details  Hme kernel base includes all common functions and definitions for HME on all platforms
//!

#include "codechal_kernel_hme.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

CodechalKernelHme::CodechalKernelHme(
    CodechalEncoderState *encoder,
    bool                 me4xDistBufferSupported)
        : CodechalKernelBase(encoder),
          m_4xMeSupported(encoder->m_hmeSupported),
          m_16xMeSupported(encoder->m_16xMeSupported),
          m_32xMeSupported(encoder->m_32xMeSupported),
          m_noMEKernelForPFrame(encoder->m_noMeKernelForPFrame),
          m_useNonLegacyStreamIn(encoder->m_useNonLegacyStreamin),
          m_4xMeDistortionBufferSupported(me4xDistBufferSupported)
{
    memset((void*)&m_curbeParam, 0, sizeof(m_curbeParam));
    memset((void*)&m_surfaceParam, 0, sizeof(m_surfaceParam));
}

CodechalKernelHme::~CodechalKernelHme()
{
    ReleaseResources();
}

MOS_STATUS CodechalKernelHme::AllocateResources()
{
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    PMOS_SURFACE allocSurface = nullptr;
    if (m_4xMeSupported)
    {
        memset((void*)&allocParamsForBuffer2D, 0, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
        allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
        allocParamsForBuffer2D.Format   = Format_Buffer_2D;

        CODECHAL_ENCODE_CHK_NULL_RETURN(allocSurface = MOS_New(MOS_SURFACE));
        memset((void*)allocSurface, 0, sizeof(MOS_SURFACE));

        allocSurface->TileType      = MOS_TILE_LINEAR;
        allocSurface->bArraySpacing = true;
        allocSurface->Format        = Format_Buffer_2D;
        allocSurface->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64);  // MediaBlockRW requires pitch multiple of 64 bytes when linear.
        allocSurface->dwHeight      = (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        allocSurface->dwPitch       = allocSurface->dwWidth;

        allocParamsForBuffer2D.dwWidth  = allocSurface->dwWidth;
        allocParamsForBuffer2D.dwHeight = allocSurface->dwHeight;
        allocParamsForBuffer2D.pBufName = "4xME MV Data Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(&allocParamsForBuffer2D, allocSurface, SurfaceId::me4xMvDataBuffer));

        if (m_4xMeDistortionBufferSupported)
        {
            uint32_t ajustedHeight =
                m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT * SCALE_FACTOR_4x;
            uint32_t downscaledFieldHeightInMB4x =
                CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(((ajustedHeight + 1) >> 1) / 4);
            CODECHAL_ENCODE_CHK_NULL_RETURN(allocSurface = MOS_New(MOS_SURFACE));
            memset((void*)allocSurface, 0, sizeof(MOS_SURFACE));
            MOS_ZeroMemory(allocSurface, sizeof(MOS_SURFACE));
            allocSurface->TileType      = MOS_TILE_LINEAR;
            allocSurface->bArraySpacing = true;
            allocSurface->Format        = Format_Buffer_2D;
            allocSurface->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
            allocSurface->dwHeight      = 2 * MOS_ALIGN_CEIL((downscaledFieldHeightInMB4x * 4 * 10), 8);
            allocSurface->dwPitch       = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);

            allocParamsForBuffer2D.dwWidth  = allocSurface->dwWidth;
            allocParamsForBuffer2D.dwHeight = allocSurface->dwHeight;
            allocParamsForBuffer2D.pBufName = "4xME Distortion Buffer";

            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(&allocParamsForBuffer2D, allocSurface, SurfaceId::me4xDistortionBuffer));
        }
    }

    if (m_16xMeSupported)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(allocSurface = MOS_New(MOS_SURFACE));
        memset((void*)allocSurface, 0, sizeof(MOS_SURFACE));
        MOS_ZeroMemory(allocSurface, sizeof(MOS_SURFACE));

        allocSurface->TileType      = MOS_TILE_LINEAR;
        allocSurface->bArraySpacing = true;
        allocSurface->Format        = Format_Buffer_2D;
        allocSurface->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64);  // MediaBlockRW requires pitch multiple of 64 bytes when linear
        allocSurface->dwHeight      = (m_downscaledHeightInMb16x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        allocSurface->dwPitch       = allocSurface->dwWidth;

        allocParamsForBuffer2D.dwWidth  = allocSurface->dwWidth;
        allocParamsForBuffer2D.dwHeight = allocSurface->dwHeight;
        allocParamsForBuffer2D.pBufName = "16xME MV Data Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(&allocParamsForBuffer2D, allocSurface, SurfaceId::me16xMvDataBuffer));
    }

    if (m_32xMeSupported)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(allocSurface = MOS_New(MOS_SURFACE));
        memset((void*)allocSurface, 0, sizeof(MOS_SURFACE));
        MOS_ZeroMemory(allocSurface, sizeof(MOS_SURFACE));

        allocSurface->TileType      = MOS_TILE_LINEAR;
        allocSurface->bArraySpacing = true;
        allocSurface->Format        = Format_Buffer_2D;
        allocSurface->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64);  // MediaBlockRW requires pitch multiple of 64 bytes when linear
        allocSurface->dwHeight      = (m_downscaledHeightInMb32x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        allocSurface->dwPitch       = allocSurface->dwWidth;

        allocParamsForBuffer2D.dwWidth  = allocSurface->dwWidth;
        allocParamsForBuffer2D.dwHeight = allocSurface->dwHeight;
        allocParamsForBuffer2D.pBufName = "32xME MV Data Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateSurface(&allocParamsForBuffer2D, allocSurface, SurfaceId::me32xMvDataBuffer));

    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHme::ReleaseResources()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHme::AddPerfTag()
{
    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL : CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    // Each ME kernel buffer counts as a separate perf task
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    return MOS_STATUS_SUCCESS;
}

MHW_KERNEL_STATE *CodechalKernelHme::GetActiveKernelState()
{
    uint32_t kernelIndex = (m_pictureCodingType == P_TYPE) ? KernelIndex::hmeP : KernelIndex::hmeB;
    auto it = m_kernelStatePool.find(kernelIndex);
    if (it != m_kernelStatePool.end())
    {
        return it->second;
    }
    MHW_KERNEL_STATE *kernelState = nullptr;
    uint32_t kernelOffset = 0;
    if (m_pictureCodingType != P_TYPE)
    {
        kernelOffset = 1;
    }

    CreateKernelState(&kernelState, kernelIndex, ENC_ME, kernelOffset);
    return kernelState;
}

CODECHAL_MEDIA_STATE_TYPE CodechalKernelHme::GetMediaStateType()
{
    return  m_32xMeInUse ? CODECHAL_MEDIA_STATE_32X_ME : m_16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;
}

MOS_STATUS CodechalKernelHme::InitWalkerCodecParams(CODECHAL_WALKER_CODEC_PARAMS &walkerParam)
{
    uint32_t scalingFactor = m_32xMeInUse ? scalingFactor32X : m_16xMeInUse ? scalingFactor16X : scalingFactor4X;
    uint32_t xResolution   = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t yResolution   = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scalingFactor);

    walkerParam.WalkerMode              = m_walkerMode;
    walkerParam.dwResolutionX           = xResolution;
    walkerParam.dwResolutionY           = yResolution;
    walkerParam.bNoDependency           = true;
    walkerParam.bMbaff                  = m_surfaceParam.mbaffEnabled;
    walkerParam.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerParam.ucGroupId               = m_groupId;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHme::SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState)
{
    if (!(m_4xMeInUse || m_16xMeInUse || m_32xMeInUse))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_surfaceParam.vdencStreamInEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfaceParam.meVdencStreamInBuffer);
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_surfaceParam.meBrcDistortionBuffer);
    }

    PMOS_SURFACE currScaledSurface;
    uint32_t     refScaledBottomFieldOffset = 0;
    bool         currFieldPicture           = CodecHal_PictureIsField(*(m_surfaceParam.currOriginalPic)) ? true : false;
    bool         currBottomField            = CodecHal_PictureIsBottomField(*(m_surfaceParam.currOriginalPic)) ? true : false;
    uint8_t      currVDirection             = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));

    surfaceParams.bIs2DSurface          = true;
    surfaceParams.bMediaBlockRW         = true;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset  = BindingTableOffset::meOutputMvDataSurface;
    surfaceParams.bIsWritable           = true;
    surfaceParams.bRenderTarget         = true;

    if (m_32xMeInUse)
    {
        currScaledSurface       = m_encoder->m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        surfaceParams.psSurface = GetSurface(SurfaceId::me32xMvDataBuffer);
        surfaceParams.dwOffset  = m_32xMeMvBottomFieldOffset;
    }
    else if (m_16xMeInUse)
    {
        currScaledSurface       = m_encoder->m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        surfaceParams.psSurface = GetSurface(SurfaceId::me16xMvDataBuffer);
        surfaceParams.dwOffset  = m_16xMeMvBottomFieldOffset;
    }
    else
    {
        currScaledSurface       = m_encoder->m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        surfaceParams.psSurface = GetSurface(SurfaceId::me4xMvDataBuffer);
        surfaceParams.dwOffset  = m_4xMeMvBottomFieldOffset;
    }

    // Force the values
    surfaceParams.psSurface->dwWidth  = MOS_ALIGN_CEIL(m_surfaceParam.downScaledWidthInMb * 32, 64);
    surfaceParams.psSurface->dwHeight = m_surfaceParam.downScaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;
    surfaceParams.psSurface->dwPitch = surfaceParams.psSurface->dwWidth;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmd,
        &surfaceParams,
        kernelState));

    if (m_16xMeInUse && Is32xMeEnabled())
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface          = true;
        surfaceParams.bMediaBlockRW         = true;
        surfaceParams.psSurface             = GetSurface(SurfaceId::me32xMvDataBuffer);
        surfaceParams.dwOffset              = currBottomField ? m_32xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset  = BindingTableOffset::meInputMvDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));
    }
    else if (Is16xMeEnabled() && !m_32xMeInUse)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface          = true;
        surfaceParams.bMediaBlockRW         = true;
        surfaceParams.psSurface             = GetSurface(SurfaceId::me16xMvDataBuffer);
        surfaceParams.dwOffset              = currBottomField ? m_16xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset  = BindingTableOffset::meInputMvDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmd,
            &surfaceParams,
            kernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (m_4xMeInUse)
    {
        if (!m_surfaceParam.vdencStreamInEnabled)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bIs2DSurface = true;
            surfaceParams.bMediaBlockRW = true;
            surfaceParams.psSurface = m_surfaceParam.meBrcDistortionBuffer;
            surfaceParams.dwOffset = m_surfaceParam.meBrcDistortionBottomFieldOffset;
            surfaceParams.dwBindingTableOffset = BindingTableOffset::meBrcDistortion;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE].Value;
            surfaceParams.bIsWritable = true;
            surfaceParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }

        if (m_4xMeDistortionBufferSupported)
        {
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bIs2DSurface = true;
            surfaceParams.bMediaBlockRW = true;
            surfaceParams.psSurface = GetSurface(SurfaceId::me4xDistortionBuffer);
            surfaceParams.psSurface->dwHeight = m_surfaceParam.downScaledHeightInMb * 4 * 10;
            surfaceParams.dwOffset = m_meDistortionBottomFieldOffset;
            surfaceParams.dwBindingTableOffset = BindingTableOffset::meDistortionSurface;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceParams.bIsWritable = true;
            surfaceParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 0 references
    CODEC_PICTURE refPic;
    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    MOS_SURFACE refScaledSurface = *currScaledSurface;

    uint8_t fwdRefBTOffset[8];
    fwdRefBTOffset[0] = BindingTableOffset::meFwdRefIdx0;
    fwdRefBTOffset[1] = BindingTableOffset::meFwdRefIdx1;
    fwdRefBTOffset[2] = BindingTableOffset::meFwdRefIdx2;
    fwdRefBTOffset[3] = BindingTableOffset::meFwdRefIdx3;
    fwdRefBTOffset[4] = BindingTableOffset::meFwdRefIdx4;
    fwdRefBTOffset[5] = BindingTableOffset::meFwdRefIdx5;
    fwdRefBTOffset[6] = BindingTableOffset::meFwdRefIdx6;
    fwdRefBTOffset[7] = BindingTableOffset::meFwdRefIdx7;

    for (uint8_t refIdx = 0; refIdx <= m_surfaceParam.numRefIdxL0ActiveMinus1; refIdx++)
    {
        refPic = m_surfaceParam.refL0List[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_surfaceParam.picIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState          = true;
                surfaceParams.psSurface             = currScaledSurface;
                surfaceParams.dwOffset              = currBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset  = BindingTableOffset::meCurrForFwdRef;
                surfaceParams.ucVDirection          = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmd,
                    &surfaceParams,
                    kernelState));
            }

            bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint8_t refPicIdx      = m_surfaceParam.picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx      = m_surfaceParam.refList[refPicIdx]->ucScalingIdx;
            
            if (m_32xMeInUse)
            {
                MOS_SURFACE* p32xSurface = m_encoder->m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            else if (m_16xMeInUse)
            {
                MOS_SURFACE* p16xSurface = m_encoder->m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            else
            {
                MOS_SURFACE* p4xSurface = m_encoder->m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState          = true;
            surfaceParams.psSurface             = &refScaledSurface;
            surfaceParams.dwOffset              = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
            surfaceParams.dwBindingTableOffset  = fwdRefBTOffset[refIdx];
            surfaceParams.ucVDirection          = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                                                  ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    uint8_t bwdRefBTOffset[2];
    bwdRefBTOffset[0] = BindingTableOffset::meBwdRefIdx0;
    bwdRefBTOffset[1] = BindingTableOffset::meBwdRefIdx1;

    for (uint8_t refIdx = 0; refIdx <= m_surfaceParam.numRefIdxL1ActiveMinus1; refIdx++)
    {
        refPic = m_surfaceParam.refL1List[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_surfaceParam.picIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState          = true;
                surfaceParams.psSurface             = currScaledSurface;
                surfaceParams.dwOffset              = currBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset  = BindingTableOffset::meCurrForBwdRef;
                surfaceParams.ucVDirection          = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmd,
                    &surfaceParams,
                    kernelState));
            }

            bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
            uint8_t refPicIdx      = m_surfaceParam.picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx      = m_surfaceParam.refList[refPicIdx]->ucScalingIdx;
            if (m_32xMeInUse)
            {
                MOS_SURFACE* p32xSurface = m_encoder->m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            else if (m_16xMeInUse)
            {
                MOS_SURFACE* p16xSurface = m_encoder->m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            else
            {
                MOS_SURFACE* p4xSurface = m_encoder->m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? m_surfaceParam.downScaledBottomFieldOffset : 0;
            }
            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState          = true;
            surfaceParams.psSurface             = &refScaledSurface;
            surfaceParams.dwOffset              = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
            surfaceParams.dwBindingTableOffset  = bwdRefBTOffset[refIdx];
            surfaceParams.ucVDirection          = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmd,
                &surfaceParams,
                kernelState));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalKernelHme::Execute(CurbeParam &curbeParam, SurfaceParams &surfaceParam, HmeLevel hmeLevel)
{
    m_4xMeInUse = Is4xMeEnabled() ? (hmeLevel & HmeLevel::hmeLevel4x) != 0 : false;
    m_16xMeInUse = Is16xMeEnabled() ? (hmeLevel & HmeLevel::hmeLevel16x) != 0  : false;
    m_32xMeInUse = Is32xMeEnabled() ? (hmeLevel & HmeLevel::hmeLevel32x) != 0 : false;

    MOS_SecureMemcpy(&m_curbeParam, sizeof(m_curbeParam), &curbeParam, sizeof(m_curbeParam));
    MOS_SecureMemcpy(&m_surfaceParam, sizeof(m_surfaceParam), &surfaceParam, sizeof(m_surfaceParam));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Run());
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalKernelHme::DumpKernelOutput()
{
    if (!Is4xMeEnabled())
    {
        return MOS_STATUS_SUCCESS;
    }
    PMOS_SURFACE surface = nullptr;
    CODECHAL_MEDIA_STATE_TYPE mediaState = GetMediaStateType();
    surface = GetSurface(SurfaceId::me4xMvDataBuffer);
    if (surface)
    {
        m_debugInterface->DumpBuffer(&surface->OsResource,
            CodechalDbgAttr::attrOutput,
            "_MvData",
            surface->dwWidth*surface->dwPitch,
            m_4xMeMvBottomFieldOffset,
            mediaState);
    }
    surface = GetSurface(SurfaceId::me16xMvDataBuffer);
    if (surface)
    {
        m_debugInterface->DumpBuffer(&surface->OsResource,
            CodechalDbgAttr::attrOutput,
            "_MvData",
            surface->dwWidth*surface->dwPitch,
            m_16xMeMvBottomFieldOffset,
            mediaState);
    }
    surface = GetSurface(SurfaceId::me32xMvDataBuffer);
    if (surface)
    {
        m_debugInterface->DumpBuffer(&surface->OsResource,
            CodechalDbgAttr::attrOutput,
            "_MvData",
            surface->dwWidth*surface->dwPitch,
            m_32xMeMvBottomFieldOffset,
            mediaState);
    }
    surface = GetSurface(SurfaceId::me4xDistortionBuffer);
    if (surface)
    {
        m_debugInterface->DumpBuffer(&surface->OsResource,
            CodechalDbgAttr::attrOutput,
            "_Distortion",
            surface->dwWidth*surface->dwPitch,
            m_meDistortionBottomFieldOffset,
            mediaState);
    }

    if (m_surfaceParam.meBrcDistortionBuffer)
    {
        m_debugInterface->DumpBuffer(&m_surfaceParam.meBrcDistortionBuffer->OsResource,
            CodechalDbgAttr::attrOutput,
            "_BrcDistortion",
            m_surfaceParam.meBrcDistortionBuffer->dwWidth*m_surfaceParam.meBrcDistortionBuffer->dwPitch,
            m_surfaceParam.meBrcDistortionBottomFieldOffset,
            mediaState);
    }
    return MOS_STATUS_SUCCESS;
}
#endif
