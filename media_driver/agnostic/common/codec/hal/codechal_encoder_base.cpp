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
//! \file     codechal_encoder_base.cpp
//! \brief    Implements the encode interface for CodecHal.
//! \details  The encode interface is further sub-divided by standard, this file is for the base interface which is shared by all encode standards.
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_tracked_buffer_hevc.h"
#include "mos_solo_generic.h"
#include "hal_oca_interface.h"

void CodechalEncoderState::PrepareNodes(
    MOS_GPU_NODE& videoGpuNode,
    bool&         setVideoNode)
{
    if (MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface))
        return;

    if (m_vdboxOneDefaultUsed)
    {
        setVideoNode = true;
        videoGpuNode = MOS_GPU_NODE_VIDEO;
    }
    else if (m_needCheckCpEnabled)
    {
        if (m_osInterface->osCpInterface->IsCpEnabled() ||
            m_vdencEnabled)
        {
            setVideoNode = true;
            videoGpuNode = MOS_GPU_NODE_VIDEO;
        }
    }
}

MOS_STATUS CodechalEncoderState::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

    return eStatus;
}

MOS_STATUS CodechalEncoderState::CreateGpuContexts()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (CodecHalUsesVideoEngine(m_codecFunction))
    {
        MOS_GPU_NODE videoGpuNode = MOS_GPU_NODE_VIDEO;
        bool setVideoNode = false;

        // Create Video Context
        if (MEDIA_IS_SKU(m_skuTable, FtrVcs2) || 
            (MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) && m_numVdbox > 1))   // Eventually move this functionality to Mhw
        {
            setVideoNode = false;

            PrepareNodes(videoGpuNode, setVideoNode);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateVideoNodeAssociation(
                m_osInterface,
                setVideoNode,
                &videoGpuNode));
            m_videoNodeAssociationCreated = true;
        }
        m_videoGpuNode = videoGpuNode;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetGpuCtxCreatOption());
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        MOS_GPU_CONTEXT gpuContext = (videoGpuNode == MOS_GPU_NODE_VIDEO2) && !MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VDBOX2_VIDEO3 : MOS_GPU_CONTEXT_VIDEO3;

        eStatus = (MOS_STATUS)m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            gpuContext,
            videoGpuNode,
            m_gpuCtxCreatOpt);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            // Failed to create new context. Try to reuse the existing one on the same VDBox.
            if (videoGpuNode == MOS_GPU_NODE_VIDEO2)
            {
                // check other GPU contexts on VDBox2
                gpuContext = MOS_GPU_CONTEXT_VDBOX2_VIDEO;
                if (m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext) != MOS_STATUS_SUCCESS)
                {
                    gpuContext = MOS_GPU_CONTEXT_VDBOX2_VIDEO2;
                    eStatus = (MOS_STATUS)m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext);
                }
            }
            else // videoGpuNode == MOS_GPU_NODE_VIDEO
            {
                // check other GPU contexts on VDBox1
                gpuContext = MOS_GPU_CONTEXT_VIDEO;
                if (m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext) != MOS_STATUS_SUCCESS)
                {
                    gpuContext = MOS_GPU_CONTEXT_VIDEO2;
                    eStatus = (MOS_STATUS)m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext);
                }
            }

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                // No valid GPU context on current VDBox, so destroy the video node association.
                if (MEDIA_IS_SKU(m_skuTable, FtrVcs2))
                {
                    m_osInterface->pfnDestroyVideoNodeAssociation(m_osInterface, videoGpuNode);
                    m_videoNodeAssociationCreated = false;
                }

                if (videoGpuNode == MOS_GPU_NODE_VIDEO2)
                {
                    // If no valid GPU context on VDBox2, check GPU contexts on VDBox1
                    gpuContext = MOS_GPU_CONTEXT_VIDEO3;
                    if (m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext) != MOS_STATUS_SUCCESS)
                    {
                        gpuContext = MOS_GPU_CONTEXT_VIDEO;
                        if (m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext) != MOS_STATUS_SUCCESS)
                        {
                            // If this context is also invalid, return an error as no context for the video engine
                            // is available, so PAK cannot occur
                            gpuContext = MOS_GPU_CONTEXT_VIDEO2;
                            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext));
                        }
                    }

                    // When using existing VDBOX1, UMD needs to notify KMD to increase the VDBOX1 counter
                    setVideoNode = true;
                    videoGpuNode = MOS_GPU_NODE_VIDEO;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateVideoNodeAssociation(
                        m_osInterface,
                        setVideoNode,
                        &videoGpuNode));
                    m_videoNodeAssociationCreated = true;
                }
                else // videoGpuNode == MOS_GPU_NODE_VIDEO
                {
                    // We won't check GPU contexts on VDBox2 if there is no valid GPU context on VDBox1
                    // since VDBox2 is not full featured.
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            // save the updated VDBox ordinal
            m_videoGpuNode = videoGpuNode;
        }

        if (m_videoNodeAssociationCreated)
        {
            CODECHAL_UPDATE_VDBOX_USER_FEATURE(videoGpuNode);
        }

        m_videoContext = gpuContext;

        m_osInterface->pfnSetEncodePakContext(m_osInterface, m_videoContext);
    }

    if (CodecHalUsesRenderEngine(m_codecFunction, m_standard))
    {
        MOS_GPU_CONTEXT gpuContext = MOS_GPU_CONTEXT_RENDER2;
        MOS_GPU_NODE renderGpuNode = MOS_GPU_NODE_3D;

        if (!MEDIA_IS_SKU(m_skuTable, FtrCCSNode))
        {
            m_computeContextEnabled = false;
        }

        if (m_computeContextEnabled)
        {
            gpuContext = MOS_GPU_CONTEXT_COMPUTE;
            renderGpuNode = MOS_GPU_NODE_COMPUTE;
        }
        else
        {
            gpuContext = MOS_GPU_CONTEXT_RENDER2;
            renderGpuNode = MOS_GPU_NODE_3D;
        }
        MOS_GPUCTX_CREATOPTIONS createOption;

        if (m_hwInterface->m_slicePowerGate)
        {
            createOption.packed.SubSliceCount = (m_gtSystemInfo->SubSliceCount / m_gtSystemInfo->SliceCount);
            // If there are multiply sub slices, disable half of sub slices.
            if (createOption.packed.SubSliceCount > 1)
                createOption.packed.SubSliceCount >>= 1;
            createOption.packed.SliceCount = (uint8_t)m_gtSystemInfo->SliceCount;
            createOption.packed.MaxEUcountPerSubSlice = (uint8_t)(m_gtSystemInfo->EUCount / m_gtSystemInfo->SubSliceCount);
            createOption.packed.MinEUcountPerSubSlice = (uint8_t)(m_gtSystemInfo->EUCount / m_gtSystemInfo->SubSliceCount);
        }

        eStatus = (MOS_STATUS)m_osInterface->pfnCreateGpuContext(m_osInterface, gpuContext, renderGpuNode, &createOption);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            // If this context is also invalid, return an error as no context for the 3D engine
            // is available, so ENC cannot occur
            gpuContext = MOS_GPU_CONTEXT_RENDER;
            CODECHAL_ENCODE_ASSERTMESSAGE("create gpu context failure for Render Engine!");
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnIsGpuContextValid(m_osInterface, gpuContext));
        }

        m_renderContext = gpuContext;
        m_osInterface->pfnSetEncodeEncContext(m_osInterface, m_renderContext);
    }

    // Set Vdbox index in use
    m_vdboxIndex = (m_videoGpuNode == MOS_GPU_NODE_VIDEO2)? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;

    return eStatus;
}

MOS_STATUS CodechalEncoderState::DestroyMeResources(
    HmeParams* param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if (param->ps16xMeMvDataBuffer != nullptr)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps16xMeMvDataBuffer->OsResource);
    }

    if (param->ps32xMeMvDataBuffer != nullptr)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps32xMeMvDataBuffer->OsResource);
    }

    if (param->ps4xMeDistortionBuffer != nullptr)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps4xMeDistortionBuffer->OsResource);
    }

    if (param->ps4xMeMvDataBuffer != nullptr)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps4xMeMvDataBuffer->OsResource);
    }

    if (param->presMvAndDistortionSumSurface != nullptr)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            param->presMvAndDistortionSumSurface);
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::CleanUpResource(
    PMOS_RESOURCE            resource,
    PMOS_ALLOC_GFXRES_PARAMS allocParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(resource);
    CODECHAL_ENCODE_CHK_NULL_RETURN(allocParams);

    MOS_LOCK_PARAMS lockFlag;
    MOS_ZeroMemory(&lockFlag, sizeof(lockFlag));
    lockFlag.WriteOnly = true;
    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, resource, &lockFlag);
    if(data == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    if(allocParams->Format == Format_Buffer)
    {
        MOS_ZeroMemory(data, allocParams->dwBytes);
    }
    else if(allocParams->Format == Format_Buffer_2D)
    {
        MOS_ZeroMemory(data, allocParams->dwHeight * allocParams->dwWidth);
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, resource);

    return eStatus;
}

MOS_STATUS CodechalEncoderState::AllocateResources4xMe(
    HmeParams* param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if(!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    MOS_ZeroMemory(param->ps4xMeMvDataBuffer, sizeof(MOS_SURFACE));
    param->ps4xMeMvDataBuffer->TileType        = MOS_TILE_LINEAR;
    param->ps4xMeMvDataBuffer->bArraySpacing   = true;
    param->ps4xMeMvDataBuffer->Format          = Format_Buffer_2D;
    param->ps4xMeMvDataBuffer->dwWidth         = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear.
    param->ps4xMeMvDataBuffer->dwHeight        = (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
    param->ps4xMeMvDataBuffer->dwPitch         = param->ps4xMeMvDataBuffer->dwWidth;

    allocParamsForBuffer2D.dwWidth  = param->ps4xMeMvDataBuffer->dwWidth;
    allocParamsForBuffer2D.dwHeight = param->ps4xMeMvDataBuffer->dwHeight;
    allocParamsForBuffer2D.pBufName = "4xME MV Data Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBuffer2D,
        &param->ps4xMeMvDataBuffer->OsResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate 4xME MV Data Buffer.");
        return eStatus;
    }

    CleanUpResource(&param->ps4xMeMvDataBuffer->OsResource, &allocParamsForBuffer2D);

    if (param->b4xMeDistortionBufferSupported)
    {
        uint32_t adjustedHeight                   =
                        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT * SCALE_FACTOR_4x;
        uint32_t downscaledFieldHeightInMb4x     =
                        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(((adjustedHeight + 1) >> 1)/4);

        MOS_ZeroMemory(param->ps4xMeDistortionBuffer, sizeof(MOS_SURFACE));
        param->ps4xMeDistortionBuffer->TileType        = MOS_TILE_LINEAR;
        param->ps4xMeDistortionBuffer->bArraySpacing   = true;
        param->ps4xMeDistortionBuffer->Format          = Format_Buffer_2D;
        param->ps4xMeDistortionBuffer->dwWidth         = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
        param->ps4xMeDistortionBuffer->dwHeight        = 2 * MOS_ALIGN_CEIL((downscaledFieldHeightInMb4x * 4 * 10), 8);
        param->ps4xMeDistortionBuffer->dwPitch         = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);

        allocParamsForBuffer2D.dwWidth  = param->ps4xMeDistortionBuffer->dwWidth;
        allocParamsForBuffer2D.dwHeight = param->ps4xMeDistortionBuffer->dwHeight;
        allocParamsForBuffer2D.pBufName = "4xME Distortion Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &param->ps4xMeDistortionBuffer->OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate 4xME Distortion Buffer.");
            return eStatus;
        }
        CleanUpResource(&param->ps4xMeDistortionBuffer->OsResource, &allocParamsForBuffer2D);
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::AllocateResources16xMe(
    HmeParams* param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if (!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS    allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    if (m_16xMeSupported)
    {
        MOS_ZeroMemory(param->ps16xMeMvDataBuffer, sizeof(MOS_SURFACE));
        param->ps16xMeMvDataBuffer->TileType      = MOS_TILE_LINEAR;
        param->ps16xMeMvDataBuffer->bArraySpacing = true;
        param->ps16xMeMvDataBuffer->Format        = Format_Buffer_2D;
        param->ps16xMeMvDataBuffer->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear
        param->ps16xMeMvDataBuffer->dwHeight      = (m_downscaledHeightInMb16x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        param->ps16xMeMvDataBuffer->dwPitch       = param->ps16xMeMvDataBuffer->dwWidth;

        allocParamsForBuffer2D.dwWidth  = param->ps16xMeMvDataBuffer->dwWidth;
        allocParamsForBuffer2D.dwHeight = param->ps16xMeMvDataBuffer->dwHeight;
        allocParamsForBuffer2D.pBufName = "16xME MV Data Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &param->ps16xMeMvDataBuffer->OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate 16xME MV Data Buffer.");
            return eStatus;
        }
        CleanUpResource(&param->ps16xMeMvDataBuffer->OsResource, &allocParamsForBuffer2D);
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::AllocateResources32xMe(
    HmeParams* param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);
    if (!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    if (m_32xMeSupported)
    {
        MOS_ZeroMemory(param->ps32xMeMvDataBuffer, sizeof(MOS_SURFACE));
        param->ps32xMeMvDataBuffer->TileType      = MOS_TILE_LINEAR;
        param->ps32xMeMvDataBuffer->bArraySpacing = true;
        param->ps32xMeMvDataBuffer->Format        = Format_Buffer_2D;
        param->ps32xMeMvDataBuffer->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear
        param->ps32xMeMvDataBuffer->dwHeight      = (m_downscaledHeightInMb32x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        param->ps32xMeMvDataBuffer->dwPitch       = param->ps32xMeMvDataBuffer->dwWidth;

        allocParamsForBuffer2D.dwWidth  = param->ps32xMeMvDataBuffer->dwWidth;
        allocParamsForBuffer2D.dwHeight = param->ps32xMeMvDataBuffer->dwHeight;
        allocParamsForBuffer2D.pBufName = "32xME MV Data Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &param->ps32xMeMvDataBuffer->OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate 32xME MV Data Buffer\n", __FUNCTION__);
            return eStatus;
        }
        CleanUpResource(&param->ps32xMeMvDataBuffer->OsResource, &allocParamsForBuffer2D);
    }

    return eStatus;
}

// Encode Public Interface Functions
MOS_STATUS CodechalEncoderState::Allocate(CodechalSetting * codecHalSettings)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_cscDsState)
    {
        // call before m_hwInterface->Initialize() to reserve ISH space for CscDs kernel
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->Initialize());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Codechal::Allocate(codecHalSettings));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Initialize(codecHalSettings));

    // Create MMC state
    if (m_mmcState == nullptr)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState = MOS_New(CodecHalMmcState, m_hwInterface));
    }

    // create resource allocator
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator = MOS_New(CodechalEncodeAllocator, this));

    // create tracked buffer state
    if (m_standard == CODECHAL_HEVC)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBuf = MOS_New(CodechalEncodeTrackedBufferHevc, this));
    }
    else
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_trackedBuf = MOS_New(CodechalEncodeTrackedBuffer, this));
    }

    MotionEstimationDisableCheck();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CreateGpuContexts());

    if (CodecHalUsesRenderEngine(codecHalSettings->codecFunction, codecHalSettings->standard))
    {
        m_renderContextUsesNullHw = m_useNullHw[m_renderContext];
    }

    if (CodecHalUsesVideoEngine(codecHalSettings->codecFunction))
    {
        m_videoContextUsesNullHw = m_useNullHw[m_videoContext];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            m_videoContext));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            m_renderContext));
    }

    if (!m_perfProfiler)
    {
        m_perfProfiler = MediaPerfProfiler::Instance();
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_perfProfiler);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->Initialize((void*)this, m_osInterface));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::Execute(void *params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Codechal::Execute(params));

    EncoderParams *encodeParams = (EncoderParams *)params;
    // MSDK event handling
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_Solo_SetGpuAppTaskEvent(m_osInterface,encodeParams->gpuAppTaskEvent));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->SetWatchdogTimerThreshold(m_frameWidth, m_frameHeight));

    if (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ExecutePreEnc(encodeParams));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ExecuteEnc(encodeParams));
    }

    return MOS_STATUS_SUCCESS;
}

// Encoder Public Interface Functions
MOS_STATUS CodechalEncoderState::Initialize(
    CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(settings);

    m_storeData      = 1;
    m_firstFrame     = true;
    m_firstTwoFrames = true;
    m_standard       = settings->standard;
    m_mode           = settings->mode;
    m_codecFunction  = settings->codecFunction;

    if (CodecHalUsesVideoEngine(m_codecFunction))
    {
        m_pakEnabled = true;
    }

    if (CodecHalUsesRenderEngine(m_codecFunction, m_standard))
    {
        m_encEnabled = true;
    }

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    if (m_encEnabled)
    {
        m_brcPakStatisticsSize = CODECHAL_ENCODE_BRC_PAK_STATISTICS_SIZE;

        m_hwScoreboardType = 1;

        m_encodeVfeMaxThreads = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_VFE_MAX_THREADS_ID,
            &userFeatureData);
        m_encodeVfeMaxThreads = (uint32_t)userFeatureData.u32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

        m_encodeVfeMaxThreadsScaling = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_VFE_MAX_THREADS_SCALING_ID,
            &userFeatureData);
        m_encodeVfeMaxThreadsScaling = (uint32_t)userFeatureData.i32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_ID,
                &userFeatureData);

            m_hwWalker = (userFeatureData.i32Data) ? true : false;

            if (m_hwWalker)
            {
                m_walkerMode = (MHW_WALKER_MODE)0;
#if (_DEBUG || _RELEASE_INTERNAL)
                MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                MOS_UserFeature_ReadValue_ID(
                    nullptr,
                    __MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_MODE_ID,
                    &userFeatureData);
                m_walkerMode = (MHW_WALKER_MODE)userFeatureData.u32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

                if (MEDIA_IS_SKU(m_skuTable, FtrSliceShutdownOverride))
                {
                    //Default Slice State
                    m_sliceShutdownDefaultState = (uint32_t)0;
#if (_DEBUG || _RELEASE_INTERNAL)
                    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                    MOS_UserFeature_ReadValue_ID(
                        nullptr,
                        __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_DEFAULT_STATE_ID,
                        &userFeatureData);
                    m_sliceShutdownDefaultState = (uint32_t)userFeatureData.u32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

                    //Requested Slice State
                    m_sliceShutdownRequestState = (uint32_t)0;
#if (_DEBUG || _RELEASE_INTERNAL)
                    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                    MOS_UserFeature_ReadValue_ID(
                        nullptr,
                        __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_REQUEST_STATE_ID,
                        &userFeatureData);
                    m_sliceShutdownRequestState = (uint32_t)userFeatureData.u32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

                    //Slice Shutdown Resolution Threshold
                    m_ssdResolutionThreshold = (uint32_t)0;
#if (_DEBUG || _RELEASE_INTERNAL)
                    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                    MOS_UserFeature_ReadValue_ID(
                        nullptr,
                        __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_RESOLUTION_THRESHOLD_ID,
                        &userFeatureData);
                    m_ssdResolutionThreshold = (uint32_t)userFeatureData.i32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

                    //Slice Shutdown Target Usage Threshold
                    m_ssdTargetUsageThreshold = (uint32_t)0;
#if (_DEBUG || _RELEASE_INTERNAL)
                    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                    MOS_UserFeature_ReadValue_ID(
                        nullptr,
                        __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_TARGET_USAGE_THRESHOLD_ID,
                        &userFeatureData);
                    m_ssdTargetUsageThreshold = (uint32_t)userFeatureData.i32Data;
#endif // _DEBUG || _RELEASE_INTERNAL

#if (_DEBUG || _RELEASE_INTERNAL)
                    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                    MOS_UserFeature_ReadValue_ID(
                        nullptr,
                        __MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_ID,
                        &userFeatureData);

                    if (userFeatureData.i32Data)
                    {
                        char path_buffer[256];
                        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
                        MOS_ZeroMemory(path_buffer, 256);
                        userFeatureData.StringData.pStringData = path_buffer;

                        statusKey = MOS_UserFeature_ReadValue_ID(
                            nullptr,
                            __MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_PATH_ID,
                            &userFeatureData);

                        if (statusKey == MOS_STATUS_SUCCESS && userFeatureData.StringData.uSize > 0)
                        {
                            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnLoadLibrary(m_osInterface, path_buffer, &m_swBrcMode));
                        }
                    }
                    // SW BRC DLL Reporting
                    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_IN_USE_ID, (m_swBrcMode == nullptr) ? false : true);
#endif // _DEBUG || _RELEASE_INTERNAL

                    if (!m_sliceShutdownDefaultState &&
                        !m_sliceShutdownRequestState &&
                        !m_ssdTargetUsageThreshold   &&
                        !m_ssdResolutionThreshold)
                    {
                        // slice shutdown used for power efficiency
                        // use it in case of ult and if hw has more than 2 slices
                        if (MEDIA_IS_SKU(m_skuTable, FtrULT))
                        {
                            if ((GFX_IS_GEN_10_OR_LATER(m_platform) && m_gtSystemInfo->SliceCount >= 2) ||
                                MEDIA_IS_SKU(m_skuTable, FtrGT3))
                            {
                                m_sliceShutdownDefaultState   = CODECHAL_SLICE_SHUTDOWN_ONE_SLICE;
                                m_sliceShutdownRequestState   = CODECHAL_SLICE_SHUTDOWN_TWO_SLICES;
                                m_ssdResolutionThreshold      = m_hwInterface->m_ssdResolutionThreshold;
                                m_ssdTargetUsageThreshold     = m_hwInterface->m_ssdTargetUsageThreshold;
                            }
                        }
                        else if (MEDIA_IS_SKU(m_skuTable, FtrGT4))
                        {
                            m_sliceShutdownDefaultState   = CODECHAL_SLICE_SHUTDOWN_ONE_SLICE;
                            m_sliceShutdownRequestState   = CODECHAL_SLICE_SHUTDOWN_TWO_SLICES;
                            m_ssdResolutionThreshold      = m_hwInterface->m_ssdResolutionThreshold;
                            m_ssdTargetUsageThreshold     = m_hwInterface->m_ssdTargetUsageThreshold;
                        }
                    }
                }
            }
        }

        if (MEDIA_IS_SKU(m_skuTable, FtrSliceShutdown))
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_ENABLE_ID,
                &userFeatureData);
            m_sliceShutdownEnable = (userFeatureData.i32Data) ? true : false;
        }

        m_targetUsageOverride = (uint8_t)0;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_TARGET_USAGE_OVERRIDE_ID,
            &userFeatureData);
        m_targetUsageOverride = (uint8_t)userFeatureData.u32Data;
#endif // _DEBUG || _RELEASE_INTERNAL
    }

    if (m_pakEnabled)
    {
        //RCPanic settings
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_RC_PANIC_ENABLE_ID,
            &userFeatureData);
        m_panicEnable = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = 1;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_SUPPRESS_RECON_PIC_ENABLE_ID,
            &userFeatureData);
        m_suppressReconPicSupported = (userFeatureData.u32Data) ? true : false;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_COMPUTE_CONTEXT_ID,
        &userFeatureData);
    m_computeContextEnabled = (userFeatureData.u32Data) ? true : false;
#endif

#if USE_CODECHAL_DEBUG_TOOL
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_FAKE_HEADER_SIZE_ID,
        &userFeatureData);
    m_enableFakeHrdSize = (uint32_t)userFeatureData.u32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_FAKE_IFRAME_HEADER_SIZE_ID,
        &userFeatureData);
    m_fakeIFrameHrdSize = (uint32_t)userFeatureData.u32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_FAKE_PBFRAME_HEADER_SIZE_ID,
        &userFeatureData);
    m_fakePBFrameHrdSize = (uint32_t)userFeatureData.u32Data;
#endif

    m_oriFrameWidth   = settings->width;
    m_oriFrameHeight  = settings->height;
    m_picWidthInMb    = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
    m_picHeightInMb   = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
    m_frameWidth      = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight     = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;
    m_createWidth     = m_frameWidth;
    m_createHeight    = m_frameHeight;

    // HME Scaling WxH
    m_downscaledWidthInMb4x               =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x              =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x                   =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x                  =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x             =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x                   =
        m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x                  =
        m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    // UltraHME Scaling WxH
    m_downscaledWidthInMb32x              =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_32x);
    m_downscaledHeightInMb32x             =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_32x);
    m_downscaledWidth32x                   =
        m_downscaledWidthInMb32x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight32x                  =
        m_downscaledHeightInMb32x * CODECHAL_MACROBLOCK_HEIGHT;

    m_minScaledDimension      = CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE;
    m_minScaledDimensionInMb  = (CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE + 15) >> 4;

    m_currOriginalPic.PicFlags              = PICTURE_INVALID;
    m_currOriginalPic.FrameIdx = 0;
    m_currOriginalPic.PicEntry = 0;

    m_hwInterface->GetCpInterface()->RegisterParams(settings->GetCpParams());

    // flag to enable kmd for the frame tracking (so encoder driver doesn't need to send a separate command buffer
    // for frame tracking purpose). Currently this feature is disabled for HEVC.
    // For HEVC, this feature will be enabled later.

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_FRAME_TRACKING_ID,
        &userFeatureData);
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        m_frameTrackingEnabled = userFeatureData.i32Data ? true : false;
    }
    else
    {
        m_frameTrackingEnabled = m_osInterface->bEnableKmdMediaFrameTracking ? true: false;
    }

    if (m_standard == CODECHAL_AVC)
    {
        if (CodecHalUsesVideoEngine(m_codecFunction))
        {
            m_inlineEncodeStatusUpdate = m_osInterface->bInlineCodecStatusUpdate ? true: false;
        }
    }

    // Disable SHME and UHME if HME is disabled
    if(!m_hmeSupported)
    {
        m_16xMeSupported = false;
        m_32xMeSupported = false;
    }
    // Disable UHME if SHME is disabled
    else if(!m_16xMeSupported)
    {
        m_32xMeSupported = false;
    }

    // Set Vdbox index in use
    m_vdboxIndex = (m_videoGpuNode == MOS_GPU_NODE_VIDEO2)? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        Destroy();
    }

    if (!m_feiEnable)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateMDFResources());
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::AllocateMDFResources()
{
    uint32_t devOp;

    if (CodecHalIsFeiEncode(m_codecFunction) && m_codecFunction != CODECHAL_FUNCTION_FEI_PAK)
    {
        devOp = CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE;

        if (m_cmDev == nullptr)
        {
            m_osInterface->pfnNotifyStreamIndexSharing(m_osInterface);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CreateCmDevice(m_osInterface->pOsContext, m_cmDev, devOp));
        }
        //just WA for issues in MDF null support
        if (!m_cmQueue)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateQueue(m_cmQueue));
        }
        if (!m_cmTask)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->CreateTask(m_cmTask));
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::DestroyMDFResources()
{
    uint32_t i;

    if (m_cmDev && m_cmTask)
    {
        m_cmDev->DestroyTask(m_cmTask);
        m_cmTask = nullptr;
    }
    if (m_cmDev)
    {
        DestroyCmDevice(m_cmDev);
        m_cmDev = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::SetMfeSharedState(MfeSharedState *pMfeSharedState)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pMfeSharedState);

    m_mfeEncodeSharedState = pMfeSharedState;

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CodechalEncoderState::AddKernelMdf(
    CmDevice *     device,
    CmQueue *      queue,
    CmKernel *     kernel,
    CmTask *       task,
    CmThreadSpace *threadspace,
    CmEvent *&     event,
    bool           isEnqueue)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(device);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernel);
    CODECHAL_ENCODE_CHK_NULL_RETURN(queue);
    CODECHAL_ENCODE_CHK_NULL_RETURN(task);
    CODECHAL_ENCODE_CHK_NULL_RETURN(threadspace);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernel->AssociateThreadSpace(threadspace));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(task->AddKernel(kernel));
    if (isEnqueue)
    {
        queue->Enqueue(task, event);
        task->Reset();
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(task->AddSync());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::CreateMDFKernelResource(
    CodechalEncodeMdfKernelResource *resource,
    uint8_t                          kernelNum,
    uint8_t                          bufNum,
    uint8_t                          surfNum,
    uint8_t                          vmeSurfNum,
    uint16_t                         curbeSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(resource);
    if (kernelNum > 0)
    {
        resource->ppKernel  = (CmKernel **)MOS_AllocAndZeroMemory(sizeof(CmKernel *) * kernelNum);
        resource->KernelNum = kernelNum;
    }
    if (bufNum > 0)
    {
        resource->ppCmBuf = (CmBuffer **)MOS_AllocAndZeroMemory(sizeof(CmBuffer *) * bufNum);
        resource->BufNum  = bufNum;
    }
    if (surfNum > 0)
    {
        resource->ppCmSurf = (CmSurface2D **)MOS_AllocAndZeroMemory(sizeof(CmSurface2D *) * surfNum);
        resource->SurfNum  = surfNum;
    }
    if (vmeSurfNum > 0)
    {
        resource->ppCmVmeSurf = (SurfaceIndex **)MOS_AllocAndZeroMemory(sizeof(SurfaceIndex *) * vmeSurfNum);
        resource->VmeSurfNum  = vmeSurfNum;
    }
    if (curbeSize > 0)
    {
        resource->pCurbe     = (uint8_t *)MOS_AllocAndZeroMemory(curbeSize);
        resource->wCurbeSize = curbeSize;
    }

    resource->e = nullptr;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::DestroyMDFKernelResource(
    CodechalEncodeMdfKernelResource *resource)
{
    int i;
    CODECHAL_ENCODE_CHK_NULL_RETURN(resource);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(FreeMDFKernelSurfaces(resource));

    if (resource->ppKernel && resource->KernelNum)
    {
        for (i = 0; i < resource->KernelNum; i++)
        {
            if (resource->ppKernel != nullptr)
            {
                m_cmDev->DestroyKernel(resource->ppKernel[i]);
                resource->ppKernel[i] = nullptr;
            }
        }
        MOS_FreeMemory(resource->ppKernel);
        resource->ppKernel = nullptr;
    }
    if (resource->pTS)
    {
        m_cmDev->DestroyThreadSpace(resource->pTS);
        resource->pTS = nullptr;
    }
    if (resource->ppCmBuf && resource->BufNum)
    {
        MOS_FreeMemory(resource->ppCmBuf);
        resource->ppCmBuf = nullptr;
        resource->BufNum  = 0;
    }
    if (resource->ppCmSurf && resource->SurfNum)
    {
        MOS_FreeMemory(resource->ppCmSurf);
        resource->ppCmSurf = nullptr;
        resource->SurfNum  = 0;
    }
    if (resource->ppCmVmeSurf && resource->VmeSurfNum)
    {
        MOS_FreeMemory(resource->ppCmVmeSurf);
        resource->ppCmVmeSurf = nullptr;
        resource->VmeSurfNum  = 0;
    }
    if (resource->ppKernel && resource->KernelNum)
    {
        MOS_FreeMemory(resource->ppKernel);
        resource->ppKernel  = nullptr;
        resource->KernelNum = 0;
    }
    if (resource->pCurbe && resource->wCurbeSize)
    {
        MOS_FreeMemory(resource->pCurbe);
        resource->pCurbe     = nullptr;
        resource->wCurbeSize = 0;
    }
    if (resource->pCommonISA)
    {
        MOS_FreeMemory(resource->pCommonISA);
        resource->pCommonISA = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS  CodechalEncoderState::FreeMDFKernelSurfaces(
    CodechalEncodeMdfKernelResource*    resource)
{
    int i;

    for (i = 0; i < resource->VmeSurfNum; i++)
    {
        if (resource->ppCmVmeSurf[i] != nullptr && resource->ppCmVmeSurf[i] != (SurfaceIndex *)CM_NULL_SURFACE)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroyVmeSurfaceG7_5(resource->ppCmVmeSurf[i]));
            resource->ppCmVmeSurf[i] = nullptr;
        }
    }
    for (i = 0; i < resource->BufNum; i++)
    {
        if (resource->ppCmBuf[i] != nullptr)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(resource->ppCmBuf[i]));
            resource->ppCmBuf[i] = nullptr;
        }
    }
    for (i = 0; i < resource->SurfNum; i++)
    {
        if (resource->ppCmSurf[i] != nullptr)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmDev->DestroySurface(resource->ppCmSurf[i]));
            resource->ppCmSurf[i] = nullptr;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::InitCommon()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    EncoderParams* encodeParams = &m_encodeParams;
    m_newSeq                = encodeParams->bNewSeq ? true: false;          // used by all except JPEG
    m_mbDataBufferSize      = encodeParams->dwMbDataBufferSize;             // used by all except JPEG
    m_newVuiData            = encodeParams->bNewVuiData ? true: false;      // used by AVC and MPEG2
    m_picQuant              = encodeParams->bPicQuant ? true: false;        // used by AVC and MPEG2
    m_newQmatrixData        = encodeParams->bNewQmatrixData ? true: false;  // used by AVC and MPEG2
    m_numSlices             = encodeParams->dwNumSlices;                    // used by all except VP9
    m_slcData               =
        (PCODEC_ENCODER_SLCDATA)(encodeParams->pSlcHeaderData);             // used by AVC, MPEG2, and HEVC

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeParams->presBitstreamBuffer);
    m_rawSurface           = *(encodeParams->psRawSurface);           // used by all
    m_resBitstreamBuffer    = *(encodeParams->presBitstreamBuffer);   // used by all

    CODECHAL_ENCODE_CHK_COND_RETURN(
        Mos_ResourceIsNull(&m_rawSurface.OsResource),
        "Raw surface is nullptr!");

    m_rawSurfaceToEnc     =
    m_rawSurfaceToPak     = &m_rawSurface;

    if(encodeParams->psReconSurface)
    {
        m_reconSurface     = *(encodeParams->psReconSurface);         // used by all except JPEG
    }

    if(encodeParams->pBSBuffer)
    {
        m_bsBuffer          = *(encodeParams->pBSBuffer);              // used by all except VP9
    }

    return eStatus;
}

void CodechalEncoderState::ResizeOnResChange()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // if resolution changed, free existing tracked buffer resources
    m_trackedBuf->Resize();
}

MOS_STATUS CodechalEncoderState::CheckResChangeAndCsc()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_resolutionChanged)
    {
        ResizeOnResChange();
    }

    if (m_cscDsState)
    {
        // check recon surface's alignment meet HW requirement
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_cscDsState->CheckReconSurfaceAlignment(&m_reconSurface));

        if (!m_cscDsState->IsEnabled() ||
            CodecHal_PictureIsField(m_currOriginalPic) ||
            CodecHal_PictureIsInterlacedFrame(m_currOriginalPic))
        {
            // CSC disabled for interlaced frame
            m_cscDsState->ResetCscFlag();

            // check raw surface's alignment meet HW requirement
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->CheckRawSurfaceAlignment(m_rawSurfaceToEnc));
        }
        else
        {
            // check if we need to do CSC or copy non-aligned surface
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->CheckCondition());
        }
    }

    return MOS_STATUS_SUCCESS;
}

// Function to allocate all resources common to all encoders
MOS_STATUS CodechalEncoderState::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t numMbs = m_picWidthInMb * ((m_picHeightInMb+1)>>1)<<1;

    // initiate allocation paramters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    MOS_ALLOC_GFXRES_PARAMS allocParams2D;
    MOS_ZeroMemory(&allocParams2D, sizeof(allocParams2D));
    allocParams2D.Type              = MOS_GFXRES_2D;
    allocParams2D.TileType          = MOS_TILE_LINEAR;
    allocParams2D.Format            = Format_Buffer_2D;

    MOS_LOCK_PARAMS lockFlagsNoOverWrite;;
    MOS_ZeroMemory(&lockFlagsNoOverWrite, sizeof(MOS_LOCK_PARAMS));
    lockFlagsNoOverWrite.WriteOnly = 1;
    lockFlagsNoOverWrite.NoOverWrite = 1;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    // create VME and MFX sync objects
    if ((m_codecFunction == CODECHAL_FUNCTION_ENC_PAK) ||
        (m_codecFunction == (CODECHAL_FUNCTION_ENC | CODECHAL_FUNCTION_ENC_PAK)) ||
        (m_codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK) ||
        (m_codecFunction == (CODECHAL_FUNCTION_FEI_ENC | CODECHAL_FUNCTION_FEI_ENC_PAK)))
    {
        // Create OS synchronization object to sync between MFX => VME
        // if 3 is not good enough, need to increase MBCode buffer number
        m_semaphoreMaxCount = MOS_MAX_SEMAPHORE_COUNT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_resSyncObjectRenderContextInUse));

        // Create OS synchronization object to sync between VME => MFX
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_resSyncObjectVideoContextInUse));
    }

    // Create VME and VDENC/PAK sync objects
    if (m_codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK)
    {
        m_semaphoreMaxCount = MOS_MAX_SEMAPHORE_COUNT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_resSyncObjectRenderContextInUse));
    }

    //For HEVC, moved to standard specific as LCU size is not available here
    if  (m_hwInterface->GetMfxInterface()->IsRowStoreCachingSupported() &&
         ((m_mode == CODECHAL_ENCODE_MODE_AVC)                              ||
          (m_mode == CODECHAL_ENCODE_MODE_VP9 && m_vdencEnabled)))
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams = {};
        rowstoreParams.Mode         = m_mode;
        rowstoreParams.dwPicWidth   = m_frameWidth;
        rowstoreParams.bMbaff       = false;
        m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams);
    }

    if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_skipFrameBasedHWCounterRead == false)
    {
        // eStatus query reporting
        m_encodeStatusBuf.dwReportSize           = MOS_ALIGN_CEIL(sizeof(EncodeStatus), MHW_CACHELINE_SIZE);
        uint32_t size                            = sizeof(HwCounter) * CODECHAL_ENCODE_STATUS_NUM + sizeof(HwCounter);
        allocParamsForBufferLinear.dwBytes       = size;
        allocParamsForBufferLinear.pBufName      = "HWCounterQueryBuffer";
        allocParamsForBufferLinear.bIsPersistent = true;                    // keeping status buffer persistent since its used in all command buffers

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resHwCount);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Encode eStatus Buffer.");
            return eStatus;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_osInterface->pfnSkipResourceSync(
                &m_resHwCount));

        uint8_t *dataHwCount = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &(m_resHwCount),
            &lockFlagsNoOverWrite);

        if (!dataHwCount)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Local Resource for MbEnc Adv Count Query Buffer.");
            return eStatus;
        }

        MOS_ZeroMemory(dataHwCount, size);
        m_dataHwCount = (uint32_t*)dataHwCount;
    }

    // eStatus query reporting
    // HW requires the MI_CONDITIONAL_BATCH_BUFFER_END compare address aligned with cache line since TGL,
    // this change will guarantee the multi pak pass BRC works correctly
    m_encodeStatusBuf.dwReportSize = MOS_ALIGN_CEIL(sizeof(EncodeStatus), MHW_CACHELINE_SIZE);
    uint32_t size = m_encodeStatusBuf.dwReportSize * CODECHAL_ENCODE_STATUS_NUM + sizeof(uint32_t) * 2;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "StatusQueryBuffer";
    allocParamsForBufferLinear.bIsPersistent = true;                    // keeping status buffer persistent since its used in all command buffers

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_encodeStatusBuf.resStatusBuffer),
        "Failed to allocate Encode eStatus Buffer.");

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnSkipResourceSync(
        &m_encodeStatusBuf.resStatusBuffer));

    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &(m_encodeStatusBuf.resStatusBuffer),
        &lockFlagsNoOverWrite);

    MOS_ZeroMemory(data, size);
    m_encodeStatusBuf.pData           = (uint32_t*)data;
    m_encodeStatusBuf.pEncodeStatus   = (uint8_t*)(data + sizeof(uint32_t) * 2);
    m_encodeStatusBuf.dwSize          = size;

    // Addresses writen to by HW commands (MI_STORE_DATA_IMM, MI_FLUSH_DW, PIPE_CONTROL) must be QW aligned since these
    // commands are capable of writing QWs so the least significant 3 bits of the address field are not used for the
    // actual address
    m_encodeStatusBuf.dwStoreDataOffset       = 0;
    m_encodeStatusBuf.dwBSByteCountOffset     = CODECHAL_OFFSETOF(EncodeStatus, dwMFCBitstreamByteCountPerFrame);
    m_encodeStatusBuf.dwBSSEBitCountOffset    = CODECHAL_OFFSETOF(EncodeStatus, dwMFCBitstreamSyntaxElementOnlyBitCount);
    m_encodeStatusBuf.dwImageStatusMaskOffset = CODECHAL_OFFSETOF(EncodeStatus, dwImageStatusMask);
    m_encodeStatusBuf.dwImageStatusCtrlOffset = CODECHAL_OFFSETOF(EncodeStatus, ImageStatusCtrl);
    m_encodeStatusBuf.dwNumSlicesOffset       = CODECHAL_OFFSETOF(EncodeStatus, NumSlices);
    m_encodeStatusBuf.dwErrorFlagOffset       = CODECHAL_OFFSETOF(EncodeStatus, dwErrorFlags);
    m_encodeStatusBuf.dwBRCQPReportOffset     = CODECHAL_OFFSETOF(EncodeStatus, BrcQPReport);
    m_encodeStatusBuf.dwNumPassesOffset       = CODECHAL_OFFSETOF(EncodeStatus, dwNumberPasses);
    m_encodeStatusBuf.dwQpStatusCountOffset   = CODECHAL_OFFSETOF(EncodeStatus, QpStatusCount);
    m_encodeStatusBuf.dwImageStatusCtrlOfLastBRCPassOffset = CODECHAL_OFFSETOF(EncodeStatus, ImageStatusCtrlOfLastBRCPass);
    m_encodeStatusBuf.dwSceneChangedOffset    = CODECHAL_OFFSETOF(EncodeStatus, dwSceneChangedFlag);
    m_encodeStatusBuf.dwSumSquareErrorOffset  = CODECHAL_OFFSETOF(EncodeStatus, sumSquareError[0]);
    m_encodeStatusBuf.dwSliceReportOffset     = CODECHAL_OFFSETOF(EncodeStatus, sliceReport);
    m_encodeStatusBuf.dwHuCStatusMaskOffset   = CODECHAL_OFFSETOF(EncodeStatus, HuCStatusRegMask);
    m_encodeStatusBuf.dwHuCStatusRegOffset    = CODECHAL_OFFSETOF(EncodeStatus, HuCStatusReg);

    m_encodeStatusBuf.wCurrIndex  = 0;
    m_encodeStatusBuf.wFirstIndex = 0;

    if (m_encEnabled)
    {
        m_encodeStatusBufRcs.dwReportSize = MOS_ALIGN_CEIL(sizeof(EncodeStatus), sizeof(uint64_t));
        size = m_encodeStatusBufRcs.dwReportSize * CODECHAL_ENCODE_STATUS_NUM + sizeof(uint32_t) * 2;
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "StatusQueryBufferRcs";
        allocParamsForBufferLinear.bIsPersistent = true;                    // keeping status buffer persistent since its used in all command buffers
        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_encodeStatusBufRcs.resStatusBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Encode eStatus Buffer.");
            return eStatus;
        }

        data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &(m_encodeStatusBufRcs.resStatusBuffer),
            &lockFlagsNoOverWrite);

        if (data == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to lock Encode eStatus Buffer RCS.");
            return eStatus;
        }

        MOS_ZeroMemory(data, size);
        m_encodeStatusBufRcs.pData                   = (uint32_t*)data;
        m_encodeStatusBufRcs.pEncodeStatus           = (uint8_t*)(data + sizeof(uint32_t) * 2);
        m_encodeStatusBufRcs.dwSize                  = size;
        m_encodeStatusBufRcs.dwStoreDataOffset       = 0;
        m_encodeStatusBufRcs.wCurrIndex              = 0;
        m_encodeStatusBufRcs.wFirstIndex             = 0;
    }

    if (m_pakEnabled)
    {
        m_stateHeapInterface->pfnSetCmdBufStatusPtr(m_stateHeapInterface, m_encodeStatusBuf.pData);
    }
    else
    {
        m_stateHeapInterface->pfnSetCmdBufStatusPtr(m_stateHeapInterface, m_encodeStatusBufRcs.pData);
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    if(m_inlineEncodeStatusUpdate)
    {
        m_atomicScratchBuf.dwSize = MOS_ALIGN_CEIL(sizeof(AtomicScratchBuffer), sizeof(uint64_t));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        size  = MHW_CACHELINE_SIZE * 4 * 2; //  each set of scratch is 4 cacheline size, and allocate 2 set.
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "atomic sratch buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &(m_atomicScratchBuf.resAtomicScratchBuffer));

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Finger Print Source Buffer.");
            return eStatus;
        }

        data = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                &(m_atomicScratchBuf.resAtomicScratchBuffer),
                &lockFlagsWriteOnly);

        if (data == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to lock Finger Print Source Buffer.");
            return eStatus;
        }

        MOS_ZeroMemory(data, size);
        m_atomicScratchBuf.pData                 = (uint32_t*)data;
        m_atomicScratchBuf.dwSize                = size;
        m_atomicScratchBuf.dwZeroValueOffset    = 0;
        m_atomicScratchBuf.dwOperand1Offset     = MHW_CACHELINE_SIZE;
        m_atomicScratchBuf.dwOperand2Offset     = MHW_CACHELINE_SIZE * 2;
        m_atomicScratchBuf.dwOperand3Offset     = MHW_CACHELINE_SIZE * 3;
        m_atomicScratchBuf.wEncodeUpdateIndex   = 0;
        m_atomicScratchBuf.wTearDownIndex       = 1;
        m_atomicScratchBuf.dwOperandSetSize     = MHW_CACHELINE_SIZE * 4;
    }

    if (m_pakEnabled)
    {
        if(m_hwInterface->GetMfxInterface()->IsDeblockingFilterRowstoreCacheEnabled() == false)
        {
            // Deblocking Filter Row Store Scratch buffer
            allocParamsForBufferLinear.dwBytes  = m_picWidthInMb * 4 * CODECHAL_CACHELINE_SIZE; // 4 cachelines per MB
            allocParamsForBufferLinear.pBufName = "Deblocking Filter Row Store Scratch Buffer";

            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resDeblockingFilterRowStoreScratchBuffer),
                "Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        }

        if(m_hwInterface->GetMfxInterface()->IsBsdMpcRowstoreCacheEnabled() == false)
        {
            // MPC Row Store Scratch buffer
            allocParamsForBufferLinear.dwBytes  = m_picWidthInMb * 2 * 64; // 2 cachelines per MB
            allocParamsForBufferLinear.pBufName = "MPC Row Store Scratch Buffer";

            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resMPCRowStoreScratchBuffer),
                "Failed to allocate MPC Row Store Scratch Buffer.");
        }

        if (!m_vdencEnabled && m_standard != CODECHAL_HEVC)    // StreamOut is needed for HEVC VDEnc
        {
            // streamout data buffer
            allocParamsForBufferLinear.dwBytes  = numMbs * MFX_PAK_STREAMOUT_DATA_BYTE_SIZE * sizeof(uint32_t);
            allocParamsForBufferLinear.pBufName = "Pak StreamOut Buffer";

            for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                    m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_resStreamOutBuffer[i]),
                    "Failed to allocate Pak Stream Out Buffer.");
            }
        }
    }

    if (m_encEnabled || m_vdencEnabled)
    {
        // Scaled surfaces are required to run both HME and IFrameDist
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateScalingResources());
    }

    if(m_encEnabled && (!m_vdencEnabled))
    {
        for (auto i = 0; i < CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS; i++)
        {
            allocParamsForBufferLinear.dwBytes  = CODECHAL_MAD_BUFFER_SIZE;
            allocParamsForBufferLinear.pBufName = "MAD Data Buffer";

            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resMadDataBuffer[i]),
                "Failed to allocate MAD Data Buffer.");
        }
    }

    if (m_vdencEnabled)
    {
        // VDENC BRC PAK MMIO buffer
        allocParamsForBufferLinear.dwBytes  = sizeof(VdencBrcPakMmio);
        allocParamsForBufferLinear.pBufName = "VDENC BRC PAK MMIO Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
            m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resPakMmioBuffer),
            "%s: Failed to allocate VDENC BRC PAK MMIO Buffer\n", __FUNCTION__);

        // VDEnc StreamIn data buffers, shared between driver/ME kernel/VDEnc
        if ((m_mode == CODECHAL_ENCODE_MODE_HEVC) || (m_mode == CODECHAL_ENCODE_MODE_VP9))
        {
            allocParamsForBufferLinear.dwBytes = (MOS_ALIGN_CEIL(m_frameWidth, 64)/32) * (MOS_ALIGN_CEIL(m_frameHeight, 64)/32) * CODECHAL_CACHELINE_SIZE;
        }
        else
        {
            allocParamsForBufferLinear.dwBytes = m_picWidthInMb * m_picHeightInMb * CODECHAL_CACHELINE_SIZE;
        }
        allocParamsForBufferLinear.pBufName = "VDEnc StreamIn Data Buffer";

        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resVdencStreamInBuffer[i]),
                "Failed to allocate VDEnc StreamIn Data Buffer.");

            data = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_resVdencStreamInBuffer[i],
                &lockFlags);

            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(
                data,
                allocParamsForBufferLinear.dwBytes);

            m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencStreamInBuffer[i]);
        }
    }

    if (m_vdencEnabled)
    {
        // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
        allocParamsForBufferLinear.dwBytes = sizeof(uint64_t);
        allocParamsForBufferLinear.pBufName = "HUC STATUS 2 Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
            m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resHucStatus2Buffer),
            "%s: Failed to allocate HUC STATUS 2 Buffer\n", __FUNCTION__);
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::AllocateScalingResources()
{
    uint32_t                    numMBs, size;
    MOS_ALLOC_GFXRES_PARAMS     allocParamsForBuffer2D;
    MOS_ALLOC_GFXRES_PARAMS     allocParamsForBufferLinear;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    //Allocate the Batch Buffer for scaling Kernel.
    numMBs = m_picWidthInMb * ((m_picHeightInMb + 1) >> 1) << 1;
    size = m_hwInterface->GetMediaObjectBufferSize(
        numMBs,
        64);

    for (int i = 0; i < CODECHAL_ENCODE_VME_BBUF_NUM; i++)
    {
        MOS_ZeroMemory(&m_scalingBBUF[i].BatchBuffer, sizeof(m_scalingBBUF[0].BatchBuffer));

        /* For CM based Downscale kernel, unlike the old asm based downscale kernel,
        HW walker can be used as no inline data is required by the kernel. */
        if (!m_useCmScalingKernel && !m_useMwWlkrForAsmScalingKernel)
        {
            m_scalingBBUF[i].BatchBuffer.bSecondLevel = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_osInterface,
                &m_scalingBBUF[i].BatchBuffer,
                NULL,
                size));

            m_scalingBBUF[i].dwSize         = size;
            m_scalingBBUF[i].dwNumMbsInBBuf = 0;
            m_scalingBBufIdx              = CODECHAL_ENCODE_VME_BBUF_NUM - 1;
        }
    }

    //MB stats buffer is supported by AVC kernels on g9+.
    if(m_mbStatsSupported)
    {
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        // Starting from g9 HVS kernel, MBEnc Curbe is decoupled from BRC kernel and a new MBEnc BRC surface is added.
        // new HVS-based BRC kernel requires size of MBStat surface be 1024-aligned
        m_hwInterface->m_avcMbStatBufferSize = MOS_ALIGN_CEIL(m_picWidthInMb * 16 * sizeof(uint32_t)* (4 * m_downscaledHeightInMb4x), 1024);

        allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_avcMbStatBufferSize;
        allocParamsForBufferLinear.pBufName = "MB Statistics Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resMbStatsBuffer), "Failed to allocate  MB Statistics Buffer.");

        m_mbStatsBottomFieldOffset = m_picWidthInMb * 16 * sizeof(uint32_t) * (2 * m_downscaledHeightInMb4x);
    }
    else if(m_flatnessCheckSupported)
    {
        MOS_ZeroMemory(&m_flatnessCheckSurface, sizeof(MOS_SURFACE));
        m_flatnessCheckSurface.TileType         = MOS_TILE_LINEAR;
        m_flatnessCheckSurface.bArraySpacing    = true;
        m_flatnessCheckSurface.Format           = Format_Buffer_2D;

        MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
        allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
        allocParamsForBuffer2D.Format   = Format_Buffer_2D;
        // Data size for 1MB is 1DWORDs (4Bytes)
        allocParamsForBuffer2D.dwWidth  = MOS_ALIGN_CEIL(m_picWidthInMb * 4, 64);
        // Because FlatnessCheckSurface was referenced and filled during 4xDownScaling operation,
        // the height should be fit to MediaWalker height setting for 4xDS Kernel.
        allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(4 * m_downscaledHeightInMb4x, 64);
        allocParamsForBuffer2D.pBufName = "Flatness Check Surface";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &m_flatnessCheckSurface.OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate FlatnessCheck Surface.");
            return eStatus;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            &m_flatnessCheckSurface));

        m_flatnessCheckBottomFieldOffset = m_flatnessCheckSurface.dwPitch * m_flatnessCheckSurface.dwHeight >> 1;
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::ExecuteMeKernel(
    MeCurbeParams *meParams,
    MeSurfaceParams *meSurfaceParams,
    HmeLevel hmeLevel)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(meParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(meSurfaceParams);

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL : CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    // Each ME kernel buffer counts as a separate perf task
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = (hmeLevel == HME_LEVEL_32x) ? CODECHAL_MEDIA_STATE_32X_ME :
        (hmeLevel == HME_LEVEL_16x) ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    bool vdencMeInUse = false;
    if (m_vdencEnabled && (encFunctionType == CODECHAL_MEDIA_STATE_4X_ME))
    {
        vdencMeInUse = true;
        // Non legacy stream in is for hevc vp9 streamin kernel
        encFunctionType = m_useNonLegacyStreamin ? CODECHAL_MEDIA_STATE_4X_ME : CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    uint32_t krnStateIdx = vdencMeInUse ?
        CODECHAL_ENCODE_ME_IDX_VDENC :
        ((m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_ME_IDX_P : CODECHAL_ENCODE_ME_IDX_B);

    PMHW_KERNEL_STATE kernelState = &m_meKernelStates[krnStateIdx];

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
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
    meParams->hmeLvl = hmeLevel;
    meParams->pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoderGenState->SetCurbeMe(meParams));

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
        kernelState));
    )
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Setup Additional ME surface params (Most of them set up in codec specific function, so don't zero out here)
    meSurfaceParams->dwDownscaledWidthInMb = (hmeLevel == HME_LEVEL_32x) ? m_downscaledWidthInMb32x :
        (hmeLevel == HME_LEVEL_16x) ? m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    meSurfaceParams->dwDownscaledHeightInMb = (hmeLevel == HME_LEVEL_32x) ? m_downscaledFrameFieldHeightInMb32x :
        (hmeLevel == HME_LEVEL_16x) ? m_downscaledFrameFieldHeightInMb16x : m_downscaledFrameFieldHeightInMb4x;
    meSurfaceParams->b32xMeInUse = (hmeLevel == HME_LEVEL_32x) ? true : false;
    meSurfaceParams->b16xMeInUse = (hmeLevel == HME_LEVEL_16x) ? true : false;
    meSurfaceParams->pKernelState = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoderGenState->SendMeSurfaces(&cmdBuffer, meSurfaceParams));

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
    uint8_t* data = NULL;
    uint32_t size = 0;
    bool driverMeDumpEnabled = m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrOutput, encFunctionType);

    if (driverMeDumpEnabled)
    {
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        switch (hmeLevel)
        {
        case HME_LEVEL_32x:
            data = (uint8_t*)m_osInterface->pfnLockResource(
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
            data = (uint8_t*)m_osInterface->pfnLockResource(
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
                data = (uint8_t*)m_osInterface->pfnLockResource(
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
            data = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface, &meSurfaceParams->psMeDistortionBuffer->OsResource, &lockFlags);
            CODECHAL_ENCODE_CHK_NULL_RETURN(data);
            size = meSurfaceParams->psMeDistortionBuffer->dwHeight * meSurfaceParams->psMeDistortionBuffer->dwPitch;
            MOS_ZeroMemory(data, size);
            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &meSurfaceParams->psMeDistortionBuffer->OsResource);
        }
    }
    );

    uint32_t scalingFactor = (hmeLevel == HME_LEVEL_32x) ? SCALE_FACTOR_32x :
        (hmeLevel == HME_LEVEL_16x) ? SCALE_FACTOR_16x : SCALE_FACTOR_4x;

    uint32_t resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scalingFactor);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;
    walkerCodecParams.bMbaff = meSurfaceParams->bMbaff;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId = m_groupId;

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
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

bool CodechalEncoderState::CheckSupportedFormat(
    PMOS_SURFACE surface)
{
    bool isColorFormatSupported = true;

    if (!surface)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (NULL) Pointer.");
        return isColorFormatSupported;
    }

    // if input is not Tile-Y, or color format not NV12, invoke Ds+Copy kernel
    if (!IS_Y_MAJOR_TILE_FORMAT(surface->TileType) ||
        surface->Format != Format_NV12)
    {
        isColorFormatSupported = false;
    }

    return isColorFormatSupported;
}

void CodechalEncoderState::FreeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // destroy sync objects
    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectRenderContextInUse);
    }
    if (!Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObjectVideoContextInUse);
    }

    // Release eStatus buffer
    if (!Mos_ResourceIsNull(&m_encodeStatusBuf.resStatusBuffer))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &(m_encodeStatusBuf.resStatusBuffer));

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encodeStatusBuf.resStatusBuffer);
    }

    // Release HW Counter buffer
    if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_skipFrameBasedHWCounterRead == false)
    {
        if (!Mos_ResourceIsNull(&m_resHwCount))
        {
            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &(m_resHwCount));

            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resHwCount);
        }
    }

    if (!Mos_ResourceIsNull(&m_encodeStatusBufRcs.resStatusBuffer))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &(m_encodeStatusBufRcs.resStatusBuffer));

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_encodeStatusBufRcs.resStatusBuffer);
    }

    if (m_pakEnabled)
    {
        if (!Mos_ResourceIsNull(&m_resDeblockingFilterRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resDeblockingFilterRowStoreScratchBuffer);
        }
        if (!Mos_ResourceIsNull(&m_resMPCRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resMPCRowStoreScratchBuffer);
        }

        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            if (!Mos_ResourceIsNull(&m_resStreamOutBuffer[i]))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resStreamOutBuffer[i]);
            }

            if (!Mos_ResourceIsNull(&m_sliceMapSurface[i].OsResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_sliceMapSurface[i].OsResource);
            }
        }
    }

    // release CSC Downscaling kernel resources
    if (m_cscDsState)
    {
        MOS_Delete(m_cscDsState);
        m_cscDsState = nullptr;
    }

    if (m_encoderGenState)
    {
        MOS_Delete(m_encoderGenState);
        m_encoderGenState = nullptr;
    }

    if(m_inlineEncodeStatusUpdate)
    {
        if (!Mos_ResourceIsNull(&m_atomicScratchBuf.resAtomicScratchBuffer))
        {
            m_osInterface->pfnUnlockResource(
            m_osInterface,
            &(m_atomicScratchBuf.resAtomicScratchBuffer));

            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_atomicScratchBuf.resAtomicScratchBuffer);
        }
    }

    if (m_encEnabled)
    {
        for (auto i = 0; i < CODECHAL_ENCODE_VME_BBUF_NUM; i++)
        {
            if (!Mos_ResourceIsNull(&m_scalingBBUF[i].BatchBuffer.OsResource))
            {
                Mhw_FreeBb(m_osInterface, &m_scalingBBUF[i].BatchBuffer, nullptr);
            }
        }

        if(!Mos_ResourceIsNull(&m_flatnessCheckSurface.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_flatnessCheckSurface.OsResource);
        }

        if(!Mos_ResourceIsNull(&m_resMbStatsBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resMbStatsBuffer);
        }

        for (auto i = 0; i < CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS; i++)
        {
            if (!Mos_ResourceIsNull(&m_resMadDataBuffer[i]))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resMadDataBuffer[i]);
            }
        }
    }

    if (m_vdencEnabled)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resPakMmioBuffer);

        m_osInterface->pfnFreeResource(m_osInterface, &m_resHucFwBuffer);

        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resVdencStreamInBuffer[i]);
        }
    }

    if (m_vdencEnabled)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resHucStatus2Buffer);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencCmdInitializerDmemBuffer);
    for (auto i = 0; i < 2; i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencCmdInitializerDataBuffer[i]);
    }

    return;
}

void CodechalEncoderState::Destroy()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_videoNodeAssociationCreated  &&
        MEDIA_IS_SKU(m_skuTable, FtrVcs2)            &&
        (m_videoGpuNode < MOS_GPU_NODE_MAX))
    {
        // Destroy encode video node associations
        m_osInterface->pfnDestroyVideoNodeAssociation(m_osInterface, m_videoGpuNode);
    }

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
        m_mmcState = nullptr;
    }

    MOS_Delete(m_allocator);
    m_allocator = nullptr;

    MOS_Delete(m_trackedBuf);
    m_trackedBuf = nullptr;

    // Release encoder resources
    FreeResources();
    return;
}

uint32_t CodechalEncoderState::CalculateCommandBufferSize()
{
    uint32_t commandBufferSize =
        m_pictureStatesSize        +
        m_extraPictureStatesSize   +
        (m_sliceStatesSize * m_numSlices);

    if (m_singleTaskPhaseSupported)
    {
        commandBufferSize *= (m_numPasses + 1);
    }

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, 0x1000);

    return commandBufferSize;
}

MOS_STATUS CodechalEncoderState::VerifySpaceAvailable()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t requestedSize = 0;
    if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext)
    {
        requestedSize = m_vmeStatesSize;

        eStatus = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
            m_osInterface,
            requestedSize,
            0);

        return eStatus;
    }

    uint32_t requestedPatchListSize = 0;
    MOS_STATUS statusPatchList = MOS_STATUS_SUCCESS, statusCmdBuf;

    bool m_usePatchList = m_osInterface->bUsesPatchList || MEDIA_IS_SKU(m_skuTable, FtrMediaPatchless);
    if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_videoContext)
    {
        if (m_usePatchList)
        {
            requestedPatchListSize =
            m_picturePatchListSize +
            m_extraPicturePatchListSize +
            (m_slicePatchListSize * m_numSlices);

            if (m_singleTaskPhaseSupported)
            {
                requestedPatchListSize *= (m_numPasses + 1);
            }
        }

        requestedSize = CalculateCommandBufferSize();

        // Try a maximum of 3 attempts to request the required sizes from OS
        // OS could reset the sizes if necessary, therefore, requires to re-verify
        for (auto i = 0; i < 3; i++)
        {
            //Experiment shows resizing CmdBuf size and PatchList size in two calls one after the other would cause previously
            //successfully requested size to fallback to wrong value, hence never satisfying the requirement. So we call pfnResize()
            //only once depending on whether CmdBuf size not enough, or PatchList size not enough, or both.
            if (m_usePatchList)
            {
                statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                    m_osInterface,
                    requestedPatchListSize);
            }

            statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSize,
                0);

            if (statusPatchList != MOS_STATUS_SUCCESS && statusCmdBuf != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(requestedSize + COMMAND_BUFFER_RESERVED_SPACE, requestedPatchListSize));
            }
            else if (statusPatchList != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(0, requestedPatchListSize));
            }
            else if (statusCmdBuf != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(requestedSize + COMMAND_BUFFER_RESERVED_SPACE, 0));
            }
            else
            {
                m_singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
                return eStatus;
            }
        }
    }

    if (m_usePatchList)
    {
        statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
            m_osInterface,
            requestedPatchListSize);
    }

    statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
        m_osInterface,
        requestedSize,
        0);

    if ((statusCmdBuf == MOS_STATUS_SUCCESS) && (statusPatchList == MOS_STATUS_SUCCESS))
    {
        m_singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
        return eStatus;
    }

    if (m_singleTaskPhaseSupported)
    {
        uint32_t requestedSizeOriginal = 0, requestedPatchListSizeOriginal = 0;
        for (auto i = 0; i < 3; i++)
        {
            //Experiment shows resizing CmdBuf size and PatchList size in two calls one after the other would cause previously
            //successfully requested size to fallback to wrong value, hence never satisfying the requirement. So we call pfnResize()
            //only once depending on whether CmdBuf size not enough, or PatchList size not enough, or both.
            if (m_usePatchList)
            {
                statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                    m_osInterface,
                    requestedPatchListSizeOriginal);
            }

            statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
                m_osInterface,
                requestedSizeOriginal,
                0);

            if (statusPatchList != MOS_STATUS_SUCCESS && statusCmdBuf != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(requestedSizeOriginal + COMMAND_BUFFER_RESERVED_SPACE, requestedPatchListSizeOriginal));
            }
            else if (statusPatchList != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(0, requestedPatchListSizeOriginal));
            }
            else if (statusCmdBuf != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(requestedSizeOriginal + COMMAND_BUFFER_RESERVED_SPACE, 0));
            }
            else
            {
                m_singleTaskPhaseSupportedInPak = false;
                return eStatus;
            }
        }
        if (m_usePatchList)
        {
            statusPatchList = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                m_osInterface,
                requestedPatchListSizeOriginal);
        }

        statusCmdBuf = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
            m_osInterface,
            requestedSizeOriginal,
            0);

        if (statusPatchList == MOS_STATUS_SUCCESS && statusCmdBuf == MOS_STATUS_SUCCESS)
        {
            m_singleTaskPhaseSupportedInPak = false;
        }
        else
        {
            eStatus = MOS_STATUS_NO_SPACE;
        }
    }
    else
    {
        eStatus = MOS_STATUS_NO_SPACE;
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    SendKernelCmdsParams *params)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VFE_PARAMS vfeParams = {};
    vfeParams.pKernelState                      = params->pKernelState;
    vfeParams.eVfeSliceDisable                  = MHW_VFE_SLICE_ALL;
    vfeParams.Scoreboard.ScoreboardEnable       = m_useHwScoreboard;
    vfeParams.Scoreboard.ScoreboardType         = m_hwScoreboardType;
    vfeParams.dwMaximumNumberofThreads          = m_encodeVfeMaxThreads;

    if (!m_useHwScoreboard)
    {
        vfeParams.Scoreboard.ScoreboardMask = 0;
    }
    else if (params->bEnableCustomScoreBoard == true)
    {
        MOS_SecureMemcpy(&vfeParams.Scoreboard, sizeof(vfeParams.Scoreboard),
            params->pCustomScoreBoard, sizeof(*params->pCustomScoreBoard));
    }
    else if (params->bEnable45ZWalkingPattern == true)
    {
        vfeParams.Scoreboard.ScoreboardMask = 0x0F;
        vfeParams.Scoreboard.ScoreboardType = 1;

        // Scoreboard 0
        vfeParams.Scoreboard.ScoreboardDelta[0].x = 0;
        vfeParams.Scoreboard.ScoreboardDelta[0].y = 0xF;
        // Scoreboard 1
        vfeParams.Scoreboard.ScoreboardDelta[1].x = 0;
        vfeParams.Scoreboard.ScoreboardDelta[1].y = 0xE;
        // Scoreboard 2
        vfeParams.Scoreboard.ScoreboardDelta[2].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[2].y = 3;
        // Scoreboard 3
        vfeParams.Scoreboard.ScoreboardDelta[3].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[3].y = 1;
    }
    else
    {
        vfeParams.Scoreboard.ScoreboardMask       = 0xFF;

        // Scoreboard 0
        vfeParams.Scoreboard.ScoreboardDelta[0].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[0].y = 0;

        // Scoreboard 1
        vfeParams.Scoreboard.ScoreboardDelta[1].x = 0;
        vfeParams.Scoreboard.ScoreboardDelta[1].y = 0xF;

        // Scoreboard 2
        vfeParams.Scoreboard.ScoreboardDelta[2].x = 1;
        vfeParams.Scoreboard.ScoreboardDelta[2].y = 0xF;
        // Scoreboard 3
        vfeParams.Scoreboard.ScoreboardDelta[3].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[3].y = 0xF;
        // Scoreboard 4
        vfeParams.Scoreboard.ScoreboardDelta[4].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[4].y = 1;
        // Scoreboard 5
        vfeParams.Scoreboard.ScoreboardDelta[5].x = 0;
        vfeParams.Scoreboard.ScoreboardDelta[5].y = 0xE;
        // Scoreboard 6
        vfeParams.Scoreboard.ScoreboardDelta[6].x = 1;
        vfeParams.Scoreboard.ScoreboardDelta[6].y = 0xE;
        // Scoreboard 7
        vfeParams.Scoreboard.ScoreboardDelta[7].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[7].y = 0xE;
    }

    if (MEDIA_IS_WA(m_waTable, WaUseStallingScoreBoard))
        vfeParams.Scoreboard.ScoreboardType = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::SendGenericKernelCmds(
    PMOS_COMMAND_BUFFER cmdBuffer,
    SendKernelCmdsParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetDefaultSSEuSetting(params->EncFunctionType, m_setRequestedEUSlices, m_setRequestedSubSlices, m_setRequestedEUs));

    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        bool requestFrameTracking = false;

        if (CodecHalUsesOnlyRenderEngine(m_codecFunction) && m_lastEncPhase)
        {
            // frame tracking tag is only added in the last command buffer header
            requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        }
        // Send command buffer header at the beginning (OS dependent)
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(cmdBuffer, requestFrameTracking));

        m_firstTaskInPhase = false;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(cmdBuffer, params->EncFunctionType));

    if (m_renderEngineInterface->GetL3CacheConfig()->bL3CachingEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->SetL3Cache(cmdBuffer));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->EnablePreemption(cmdBuffer));

    // Add Pipeline select command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddPipelineSelectCmd(cmdBuffer, false));

    // Add State Base Addr command
    MHW_STATE_BASE_ADDR_PARAMS stateBaseAddrParams;
    MOS_ZeroMemory(&stateBaseAddrParams, sizeof(stateBaseAddrParams));
    // This bit will not be used in Driver ID but it will be used to determine if Render Target Flag has to be Clear or Set
    // Read this bit in pfnAddStateBaseAddrCmd and propagate it using ResourceParams via bRenderTarget
    stateBaseAddrParams.bDynamicStateRenderTarget = params->bDshInUse;

    MOS_RESOURCE* dsh = params->pKernelState->m_dshRegion.GetResource();
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh);
    MOS_RESOURCE* ish = params->pKernelState->m_ishRegion.GetResource();
    CODECHAL_ENCODE_CHK_NULL_RETURN(ish);
    stateBaseAddrParams.presDynamicState = dsh;
    stateBaseAddrParams.dwDynamicStateSize = params->pKernelState->m_dshRegion.GetHeapSize();
    stateBaseAddrParams.presInstructionBuffer = ish;
    stateBaseAddrParams.dwInstructionBufferSize = params->pKernelState->m_ishRegion.GetHeapSize();

    if (m_standard == CODECHAL_HEVC)
    {
        stateBaseAddrParams.mocs4InstructionCache = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddStateBaseAddrCmd(cmdBuffer, &stateBaseAddrParams));

    // Add Media VFE command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddMediaVfeCmd(cmdBuffer, params));

    // Add Media Curbe Load command
    if (params->pKernelState->KernelParams.iCurbeLength)
    {
        MHW_CURBE_LOAD_PARAMS curbeLoadParams;
        MOS_ZeroMemory(&curbeLoadParams, sizeof(curbeLoadParams));
        curbeLoadParams.pKernelState = params->pKernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaCurbeLoadCmd(cmdBuffer, &curbeLoadParams));

        HalOcaInterface::OnIndirectState(
            *cmdBuffer,
            *m_osInterface->pOsContext,
            dsh,
            params->pKernelState->m_dshRegion.GetOffset() + params->pKernelState->dwCurbeOffset,
            false,
            params->pKernelState->KernelParams.iCurbeLength);
    }

    MHW_ID_LOAD_PARAMS idLoadParams;
    MOS_ZeroMemory(&idLoadParams, sizeof(idLoadParams));
    idLoadParams.pKernelState = params->pKernelState;
    idLoadParams.dwNumKernelsLoaded = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaIDLoadCmd(cmdBuffer, &idLoadParams));

    uint32_t InterfaceDescriptorTotalLength     = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    uint32_t InterfaceDescriptorDataStartOffset = MOS_ALIGN_CEIL(
        params->pKernelState->m_dshRegion.GetOffset() + params->pKernelState->dwIdOffset,
        m_stateHeapInterface->pStateHeapInterface->GetIdAlignment());

    HalOcaInterface::OnIndirectState(
        *cmdBuffer,
        *m_osInterface->pOsContext,
        dsh,
        InterfaceDescriptorDataStartOffset,
        false,
        InterfaceDescriptorTotalLength);

    return eStatus;
}

// Refer to layout of EncodeBRCPAKStatistics_g7
MOS_STATUS CodechalEncoderState::ReadBrcPakStatistics(
    PMOS_COMMAND_BUFFER cmdBuffer,
    EncodeReadBrcPakStatsParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcPakStatisticBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presStatusBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    MmioRegistersMfx* mmioRegisters = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset        = 0;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset        = sizeof(uint32_t);
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountSliceRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource         = params->presBrcPakStatisticBuffer;
    storeDataParams.dwResourceOffset    = sizeof(uint32_t) * 2;
    storeDataParams.dwValue             = params->ucPass + 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    storeDataParams.pOsResource         = params->presStatusBuffer;
    storeDataParams.dwResourceOffset    = params->dwStatusBufNumPassesOffset;
    storeDataParams.dwValue             = params->ucPass + 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset        = sizeof(uint32_t) * (4 + params->ucPass);
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Retrieves the MFC image eStatus information
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::ReadImageStatus(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    MmioRegistersMfx* mmioRegisters = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

    EncodeStatusBuffer*  encodeStatusBuf    = &m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset        = baseOffset + encodeStatusBuf->dwImageStatusMaskOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusMaskRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset        = baseOffset + encodeStatusBuf->dwImageStatusCtrlOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // VDEnc dynamic slice overflow semaphore, DW0 is SW programmed mask(MFX_IMAGE_MASK does not support), DW1 is MFX_IMAGE_STATUS_CONTROL
    if (m_vdencBrcEnabled)
    {
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;

        // Added for VDEnc slice overflow bit in MFC_IMAGE_STATUS_CONTROL
        // The bit is connected on the non-AVC encoder side of MMIO register.
        // Need a dummy MFX_PIPE_MODE_SELECT to decoder and read this register.
        if (m_waReadVDEncOverflowStatus)
        {
            pipeModeSelectParams = {};
            pipeModeSelectParams.Mode               = CODECHAL_DECODE_MODE_AVCVLD;
            m_hwInterface->GetMfxInterface()->SetDecodeInUse(true);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));
        }

        // Store MFC_IMAGE_STATUS_CONTROL MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame.
        for (int i = 0; i < 2; i++)
        {
            if (m_resVdencBrcUpdateDmemBufferPtr[i])
            {
                miStoreRegMemParams.presStoreBuffer    = m_resVdencBrcUpdateDmemBufferPtr[i];
                miStoreRegMemParams.dwOffset           = 7 * sizeof(uint32_t); // offset of SliceSizeViolation in HUC_BRC_UPDATE_DMEM
                miStoreRegMemParams.dwRegister         = mmioRegisters->mfcImageStatusCtrlRegOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
            }
        }

        // Restore MFX_PIPE_MODE_SELECT to encode mode
        if (m_waReadVDEncOverflowStatus)
        {
            pipeModeSelectParams = {};
            pipeModeSelectParams.Mode               = m_mode;
            pipeModeSelectParams.bVdencEnabled      = true;
            m_hwInterface->GetMfxInterface()->SetDecodeInUse(false);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMfxInterface()->AddMfxPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));
        }
    }

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Retrieves the MFC registers and stores them in the eStatus report
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::ReadMfcStatus(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    MmioRegistersMfx* mmioRegisters = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

    EncodeStatusBuffer* encodeStatusBuf    = &m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset        = baseOffset + encodeStatusBuf->dwBSByteCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset        = baseOffset + encodeStatusBuf->dwBSSEBitCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset        = baseOffset + encodeStatusBuf->dwQpStatusCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcQPStatusCountOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    if (mmioRegisters->mfcAvcNumSlicesRegOffset > 0)
    {
        //read MFC_AVC_NUM_SLICES register to status report
        miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
        miStoreRegMemParams.dwOffset        = baseOffset + encodeStatusBuf->dwNumSlicesOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->mfcAvcNumSlicesRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
    }

    if (m_vdencBrcEnabled)
    {
        // Store PAK FrameSize MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame.
        for (int i = 0; i < 2; i++)
        {
            if (m_resVdencBrcUpdateDmemBufferPtr[i])
            {
                miStoreRegMemParams.presStoreBuffer = m_resVdencBrcUpdateDmemBufferPtr[i];
                miStoreRegMemParams.dwOffset        = 5 * sizeof(uint32_t);
                miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

                if (m_vdencBrcNumOfSliceOffset)
                {
                    miStoreRegMemParams.presStoreBuffer = m_resVdencBrcUpdateDmemBufferPtr[i];
                    miStoreRegMemParams.dwOffset        = m_vdencBrcNumOfSliceOffset;
                    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcAvcNumSlicesRegOffset;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
                }
            }
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadImageStatus(cmdBuffer));

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Retrieves the MFC registers and stores them in the eStatus report
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::SetStatusReportParams(
    PCODEC_REF_LIST currRefList)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    EncodeStatusBuffer*  encodeStatusBuf = nullptr;
    if ((m_codecFunction == CODECHAL_FUNCTION_ENC)         ||
        (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC) ||
        (m_codecFunction == CODECHAL_FUNCTION_FEI_ENC)     ||
        (m_codecFunction == CODECHAL_FUNCTION_HYBRIDPAK))
    {
        encodeStatusBuf = &m_encodeStatusBufRcs;
    }
    else
    {
        encodeStatusBuf = &m_encodeStatusBuf;
    }

    EncodeStatus* encodeStatus =
        (EncodeStatus*)(encodeStatusBuf->pEncodeStatus +
        encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize);

    EncodeStatusReport* encodeStatusReport = &encodeStatus->encodeStatusReport;

    encodeStatus->dwStoredData             = m_storeData;
    encodeStatusReport->StatusReportNumber = m_statusReportFeedbackNumber;
    encodeStatusReport->CurrOriginalPic    = m_currOriginalPic;
    encodeStatus->wPictureCodingType       = m_pictureCodingType;
    switch (m_codecFunction)
    {
    case CODECHAL_FUNCTION_ENC:
        encodeStatusReport->Func       = CODECHAL_ENCODE_ENC_ID;
        break;
    case CODECHAL_FUNCTION_PAK:
        encodeStatusReport->Func       = CODECHAL_ENCODE_PAK_ID;
        break;
    case CODECHAL_FUNCTION_ENC_PAK:
    case CODECHAL_FUNCTION_ENC_VDENC_PAK:
        encodeStatusReport->Func       = CODECHAL_ENCODE_ENC_PAK_ID;
        break;
    case CODECHAL_FUNCTION_FEI_PRE_ENC:
        encodeStatusReport->Func       = CODECHAL_ENCODE_FEI_PRE_ENC_ID;
        break;
    case CODECHAL_FUNCTION_FEI_ENC:
        encodeStatusReport->Func       = CODECHAL_ENCODE_FEI_ENC_ID;
        break;
    case CODECHAL_FUNCTION_FEI_PAK:
        encodeStatusReport->Func       = CODECHAL_ENCODE_FEI_PAK_ID;
        break;
    case CODECHAL_FUNCTION_FEI_ENC_PAK:
        encodeStatusReport->Func       = CODECHAL_ENCODE_FEI_ENC_PAK_ID;
        break;
    case CODECHAL_FUNCTION_HYBRIDPAK:
        encodeStatusReport->Func       = CODECHAL_ENCODE_ENC_ID; /* Only the render engine(EU) is used, MFX is not used */
        break;
    default:
        break;
    }
    encodeStatusReport->pCurrRefList       = m_currRefList;
    encodeStatusReport->NumberTilesInFrame = m_numberTilesInFrame;
    encodeStatusReport->UsedVdBoxNumber    = m_numUsedVdbox;

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Set each of status report buffer to completed status (only render context)
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::InitStatusReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    EncodeStatusBuffer* encodeStatusBuf = &m_encodeStatusBuf;
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusBuf);

    EncodeStatus* encodeStatus = (EncodeStatus*)(encodeStatusBuf->pEncodeStatus + encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);

    for (auto i = 0; i < CODECHAL_NUM_MEDIA_STATES; i += 1)
    {
        encodeStatus->qwStoredDataEnc[i].dwStoredData = CODECHAL_STATUS_QUERY_END_FLAG;
    }

    if (m_encEnabled)
    {
        EncodeStatusBuffer* encodeStatusBufRcs = &m_encodeStatusBufRcs;
        CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusBufRcs);

        encodeStatus = (EncodeStatus*)(encodeStatusBufRcs->pEncodeStatus + encodeStatusBufRcs->wCurrIndex * encodeStatusBufRcs->dwReportSize);
        CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);

        for (auto i = 0; i < CODECHAL_NUM_MEDIA_STATES; i += 1)
        {
            encodeStatus->qwStoredDataEnc[i].dwStoredData = CODECHAL_STATUS_QUERY_END_FLAG;
        }
    }

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Indicates to the driver that the batch buffer has started processing
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::StartStatusReport(
    PMOS_COMMAND_BUFFER cmdBuffer,
    CODECHAL_MEDIA_STATE_TYPE encFunctionType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    EncodeStatusBuffer* encodeStatusBuf    = &m_encodeStatusBuf;
    EncodeStatusBuffer* encodeStatusBufRcs = &m_encodeStatusBufRcs;

    if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext)
    {
        uint32_t offset =
            (encodeStatusBufRcs->wCurrIndex * m_encodeStatusBufRcs.dwReportSize) +
            encodeStatusBufRcs->dwStoreDataOffset + 8                                    +   // VME stored data offset is 2nd
            sizeof(uint32_t) * 2 * encFunctionType                                           +   // Each VME stored data is 1 QW
            sizeof(uint32_t) * 2;                                                                // encodeStatus is offset by 2 DWs in the resource

        MHW_PIPE_CONTROL_PARAMS pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.presDest                  = &encodeStatusBufRcs->resStatusBuffer;
        pipeControlParams.dwPostSyncOp              = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
        pipeControlParams.dwResourceOffset          = offset;
        pipeControlParams.dwDataDW1                 = CODECHAL_STATUS_QUERY_START_FLAG;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(
            cmdBuffer,
            nullptr,
            &pipeControlParams));
    }

    if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_videoContext)
    {
        uint32_t offset =
            (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            encodeStatusBuf->dwStoreDataOffset +    // MFX stored data offset is 1st, so no additional offset is needed
            sizeof(uint32_t) * 2;                   // encodeStatus is offset by 2 DWs in the resource

        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        storeDataParams.pOsResource      = &encodeStatusBuf->resStatusBuffer;
        storeDataParams.dwResourceOffset = offset;
        storeDataParams.dwValue          = CODECHAL_STATUS_QUERY_START_FLAG;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_skipFrameBasedHWCounterRead == false )
        {
            uint32_t writeOffset = sizeof(HwCounter) * CODECHAL_ENCODE_STATUS_NUM;

            CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetCpInterface());

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->ReadEncodeCounterFromHW(
                m_osInterface,
                cmdBuffer,
                &m_resHwCount,
                encodeStatusBuf->wCurrIndex));
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void*)this, m_osInterface, m_miInterface, cmdBuffer));
    
    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Indicates to the driver that the batch buffer has completed processing
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::EndStatusReport(
    PMOS_COMMAND_BUFFER cmdBuffer,
    CODECHAL_MEDIA_STATE_TYPE encFunctionType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag if applicable
    if (m_frameTrackingEnabled && m_osInterface->bTagResourceSync)
    {
        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        bool writeResourceSyncTag = false;

        if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext)
        {
            syncParams.GpuContext = m_renderContext;

            // Enc only and VDEnc SHME requires render engine GPU tag
            if (CodecHalUsesOnlyRenderEngine(m_codecFunction) ||
                (m_vdencEnabled && m_16xMeSupported))
            {
                writeResourceSyncTag = m_lastEncPhase && m_lastTaskInPhase;
            }
        }
        else if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_videoContext)
        {
            syncParams.GpuContext = m_videoContext;
            writeResourceSyncTag = m_lastTaskInPhase;
        }

        if (writeResourceSyncTag)
        {
            if (!m_firstField || CodecHal_PictureIsFrame(m_currOriginalPic))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(cmdBuffer, &syncParams));
            }
        }
    }

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    uint32_t offset = 0;
    if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext)
    {
        // Flush the write cache for ENC output
        MHW_PIPE_CONTROL_PARAMS pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.dwFlushMode  = MHW_FLUSH_WRITE_CACHE;
        pipeControlParams.bGenericMediaStateClear = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(cmdBuffer, nullptr, &pipeControlParams));

        if (MEDIA_IS_WA(m_waTable, WaSendDummyVFEafterPipelineSelect))
        {
            MHW_VFE_PARAMS vfeStateParams = {};
            vfeStateParams.dwNumberofURBEntries = 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeStateParams));
        }

        offset =
                (m_encodeStatusBufRcs.wCurrIndex * m_encodeStatusBufRcs.dwReportSize) +
                m_encodeStatusBufRcs.dwStoreDataOffset + 8 +   // VME stored data offset is 2nd
                sizeof(uint32_t) * 2 * encFunctionType     +   // Each VME stored data is 1 QW
                sizeof(uint32_t) * 2;                          // encodeStatus is offset by 2 DWs in the resource
        storeDataParams.pOsResource  = &m_encodeStatusBufRcs.resStatusBuffer;
    }
    else if (m_osInterface->pfnGetGpuContext(m_osInterface) == m_videoContext)
    {
        offset =
            m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize +
            m_encodeStatusBuf.dwStoreDataOffset +   // MFX stored data offset is 1st, so no additional offset is needed
            sizeof(uint32_t) * 2;                   // encodeStatus is offset by 2 DWs in the resource
        storeDataParams.pOsResource  = &m_encodeStatusBuf.resStatusBuffer;
    }

    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = CODECHAL_STATUS_QUERY_END_FLAG;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    if (encFunctionType == CODECHAL_NUM_MEDIA_STATES && m_inlineEncodeStatusUpdate)
    {
        if (m_currPass < m_numPasses)
        {
            if(m_vdencBrcEnabled)
            {
                //delay to check at the beginning of next pass util huc status updated;
            }
            else
            {
                // inc m_storeData conditionaly
                UpdateEncodeStatus(cmdBuffer, false);
            }
        }
        else
        {
            // inc m_storeData forcely
            UpdateEncodeStatus(cmdBuffer, true);
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void*)this, m_osInterface, m_miInterface, cmdBuffer));
    
    return eStatus;
}

//!
//! \brief    Update m_storeData in offset 0 of statusReport.
//! \details  Add conditonal encode status report to avoid of extra small batch buffer
//!           to avoid of extra context switch interrupt. if ImageStatusRegister show
//!           encoding completion, update the m_storeData, otherwise continue.
//!                   n0                n1                n2                n3
//!           +-------+--------+--------+--------+--------+--------+--------+--------+
//!           |   0       0    |    0            |  val/0   1/0    |    0       1    |
//!           +-------+--------+--------+--------+--------+--------+--------+--------+
//!              low     high      low     high      low     high     low     high
//!
//!           if(m_forceOperation==true)
//!              step-1:    m_storeData = m_storeData + 1                              // ADD directly
//!           else
//!              step-1:    n2_lo = ImageStatusCtrl & dwImageStatusMask                // AND
//!              step-2:    n2_lo = (n2_lo == 0) ? 0 : n2_lo                           // uint32_t CMP
//!              step-3:    n2_lo:n2_hi = (n2_lo:n2_hi == 0:1) ? 0:0 : n2_lo:n2_hi     // uint64_t CMP
//!              step-4:    n2_hi = n2_hi ^ n3_hi                                      // XOR
//!              step-5:    m_storeData = m_storeData + n2_hi                          // ADD conditionaly
//!
//! \param    [in] cmdBuffer
//!           Command buffer
//! \param    [in] forceOperation
//!           whether add m_storeData directly
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalEncoderState::UpdateEncodeStatus(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                forceOperation)
{
    MmioRegistersMfx                                *mmioRegisters;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

    // Get the right offset of EncodeStatusUpdate Operand scratch buffer
    uint32_t baseOffset        = m_atomicScratchBuf.dwOperandSetSize * m_atomicScratchBuf.wEncodeUpdateIndex;
    uint32_t zeroValueOffset   = baseOffset;
    uint32_t operand1Offset    = baseOffset + m_atomicScratchBuf.dwOperand1Offset;
    uint32_t operand2Offset    = baseOffset + m_atomicScratchBuf.dwOperand2Offset;
    uint32_t operand3Offset    = baseOffset + m_atomicScratchBuf.dwOperand3Offset;

    // Forcely ADD m_storeData, always happened in last pass.
    if(forceOperation)
    {
        // Make Flush DW call to make sure all previous work is done
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams , sizeof(flushDwParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // n2_hi = 0x1
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        storeDataParams.dwResourceOffset =  operand2Offset + sizeof(uint32_t) ;
        storeDataParams.dwValue          = 0x1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // VCS_GPR0_Lo = n2_hi = 0x1
        MHW_MI_STORE_REGISTER_MEM_PARAMS registerMemParams;
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset = operand2Offset + sizeof(uint32_t) ;
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister0LoOffset; // VCS_GPR0_Lo
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // m_storeData = m_storeData + VCS_GPR0_Lo = m_storeData + 1
        MHW_MI_ATOMIC_PARAMS atomicParams;
        MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
        atomicParams.pOsResource =&m_encodeStatusBuf.resStatusBuffer;
        atomicParams.dwResourceOffset =  0;
        atomicParams.dwDataSize = sizeof(uint32_t);
        atomicParams.Operation = MHW_MI_ATOMIC_ADD;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
            cmdBuffer,
            &atomicParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        // Make Flush DW call to make sure all previous work is done
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams , sizeof(flushDwParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // n1_lo = 0x00
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        storeDataParams.dwResourceOffset = operand1Offset;
        storeDataParams.dwValue          = 0x00;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // n2_lo = dwImageStatusMask
        MHW_MI_COPY_MEM_MEM_PARAMS copyMemMemParams;
        MOS_ZeroMemory(&copyMemMemParams , sizeof(copyMemMemParams));
        if(!m_vdencBrcEnabled)
        {
            copyMemMemParams.presSrc     = &m_encodeStatusBuf.resStatusBuffer;
            copyMemMemParams.dwSrcOffset =    (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                        m_encodeStatusBuf.dwImageStatusMaskOffset                               +
                        (sizeof(uint32_t) * 2);
        }
        else
        {
            copyMemMemParams.presSrc     = &m_resPakMmioBuffer;
            copyMemMemParams.dwSrcOffset = (sizeof(uint32_t) * 1);
        }
        copyMemMemParams.presDst     = &m_atomicScratchBuf.resAtomicScratchBuffer;
        copyMemMemParams.dwDstOffset = operand2Offset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
            cmdBuffer,
            &copyMemMemParams));

        // VCS_GPR0_Lo = ImageStatusCtrl
        MHW_MI_STORE_REGISTER_MEM_PARAMS registerMemParams;
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        if(!m_vdencBrcEnabled)
        {
            registerMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            registerMemParams.dwOffset =  (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                        m_encodeStatusBuf.dwImageStatusMaskOffset                               +
                        (sizeof(uint32_t) * 2) + sizeof(uint32_t);
        }
        else
        {
            registerMemParams.presStoreBuffer = &m_resPakMmioBuffer;
            registerMemParams.dwOffset =  (sizeof(uint32_t) * 0);
        }
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister0LoOffset; // VCS_GPR0_Lo
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Reset GPR4_Lo
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset        = zeroValueOffset; //Offset 0, has value of 0.
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset; // VCS_GPR4
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // step-1: n2_lo = n2_lo & VCS_GPR0_Lo = dwImageStatusMask & ImageStatusCtrl
        MHW_MI_ATOMIC_PARAMS atomicParams;
        MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
        atomicParams.pOsResource =  &m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize = sizeof(uint32_t);
        atomicParams.Operation = MHW_MI_ATOMIC_AND;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
            cmdBuffer,
            &atomicParams));

        // n3_lo = 0
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        storeDataParams.dwResourceOffset = operand3Offset;
        storeDataParams.dwValue          = 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // GPR0_lo = n1_lo = 0
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset        = operand1Offset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset; // VCS_GPR0
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Reset GPR4_Lo
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset        = zeroValueOffset; //Offset 0, has value of 0.
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset; // VCS_GPR4
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // step-2: n2_lo == n1_lo ? 0 : n2_lo
        // compare n1 vs n2. i.e. GRP0 vs. memory of operand2
        MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
        atomicParams.pOsResource = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize = sizeof(uint32_t);
        atomicParams.Operation = MHW_MI_ATOMIC_CMP;
        atomicParams.bReturnData = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
            cmdBuffer,
            &atomicParams));

        // n2_hi = 1
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource =&(m_atomicScratchBuf.resAtomicScratchBuffer);
        storeDataParams.dwResourceOffset = operand2Offset + sizeof(uint32_t);
        storeDataParams.dwValue          = 1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // n3_hi = 1
        MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
        storeDataParams.pOsResource =&(m_atomicScratchBuf.resAtomicScratchBuffer);
        storeDataParams.dwResourceOffset = operand3Offset + sizeof(uint32_t);
        storeDataParams.dwValue          = 1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
            cmdBuffer,
            &storeDataParams));

        // VCS_GPR0_Lo = n3_lo = 0
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset   = operand3Offset;
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister0LoOffset; // VCS_GPR0_Lo
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // GPR0_Hi = n2_hi = 1
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset = operand2Offset + sizeof(uint32_t) ; // update 1
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister0HiOffset; // VCS_GPR0_Hi
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Reset GPR4_Lo and GPR4_Hi
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer =&(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset = zeroValueOffset ;
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister4LoOffset; // VCS_GPR4_Hi
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset  =  zeroValueOffset ;
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister4HiOffset; // VCS_GPR4_Hi
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // steop-3: n2 = (n2 == 0:1) ? 0:0 : n2      // uint64_t CMP
        // If n2==0 (Lo) and 1 (Hi), covert n2 to 0 (Lo)and 0 (Hi), else no change.
        // n2 == 0:1 means encoding completsion. the n2 memory will be updated with 0:0, otherwise, no change.
        MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
        atomicParams.pOsResource = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize = sizeof(uint64_t);
        atomicParams.Operation = MHW_MI_ATOMIC_CMP;
        atomicParams.bReturnData = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
            cmdBuffer,
            &atomicParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // VCS_GPR0_Lo = n3_hi = 1
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer =  &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset = operand3Offset + sizeof(uint32_t);
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister0LoOffset; // VCS_GPR0_Lo
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));

        // step-4: n2_hi = n2_hi ^ VCS_GPR0_Lo = n2_hi ^ n3_hi
        MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
        atomicParams.pOsResource =&(m_atomicScratchBuf.resAtomicScratchBuffer);
        atomicParams.dwResourceOffset = operand2Offset + sizeof(uint32_t);
        atomicParams.dwDataSize = sizeof(uint32_t);
        atomicParams.Operation = MHW_MI_ATOMIC_XOR;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
            cmdBuffer,
            &atomicParams));

        // VCS_GPR0_Lo = n2_hi
        MOS_ZeroMemory(&registerMemParams, sizeof(registerMemParams));
        registerMemParams.presStoreBuffer = &(m_atomicScratchBuf.resAtomicScratchBuffer);
        registerMemParams.dwOffset = operand2Offset + sizeof(uint32_t) ;
        registerMemParams.dwRegister = mmioRegisters->generalPurposeRegister0LoOffset; // VCS_GPR0_Lo
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &registerMemParams));

        // step-5: m_storeData = m_storeData + VCS_GPR0_Lo = m_storeData + n2_hi
        // if not completed n2_hi should be 0, then m_storeData = m_storeData + 0
        // if completed, n2_hi should be 1, then m_storeData = m_storeData + 1
        MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
        atomicParams.pOsResource =&m_encodeStatusBuf.resStatusBuffer;
        atomicParams.dwResourceOffset =  0;
        atomicParams.dwDataSize = sizeof(uint32_t);
        atomicParams.Operation = MHW_MI_ATOMIC_ADD;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
            cmdBuffer,
            &atomicParams));

        // Make Flush DW call to make sure all previous work is done
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));
    }

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Sets up the eStatus reporting values for the next frame
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::ResetStatusReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeStatusBuf.pEncodeStatus);

    EncodeStatusBuffer* encodeStatusBuf    = &m_encodeStatusBuf;
    EncodeStatusBuffer* encodeStatusBufRcs = &m_encodeStatusBufRcs;

    EncodeStatus* encodeStatus =
        (EncodeStatus*)(encodeStatusBuf->pEncodeStatus +
        encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize);

    if (!m_frameTrackingEnabled && !m_inlineEncodeStatusUpdate)
    {
        bool renderEngineInUse = m_osInterface->pfnGetGpuContext(m_osInterface) == m_renderContext;
        bool nullRendering = false;

        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        if (renderEngineInUse)
        {
            syncParams.GpuContext = m_renderContext;
            nullRendering = m_renderContextUsesNullHw;
        }
        else
        {
            syncParams.GpuContext = m_videoContext;
            nullRendering = m_videoContextUsesNullHw;
        }

        m_osInterface->pfnResetOsStates(m_osInterface);
        MOS_COMMAND_BUFFER cmdBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

        cmdBuffer.Attributes.bTurboMode             = m_hwInterface->m_turboMode;
        cmdBuffer.Attributes.dwNumRequestedEUSlices = m_hwInterface->m_numRequestedEuSlices;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(UpdateCmdBufAttribute(&cmdBuffer, renderEngineInUse));

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface = m_osInterface;
        genericPrologParams.pvMiInterface = m_miInterface;
        genericPrologParams.bMmcEnabled = CodecHalMmcState::IsMmcEnabled();
        genericPrologParams.presStoreData = (renderEngineInUse) ?
            &encodeStatusBufRcs->resStatusBuffer : &encodeStatusBuf->resStatusBuffer;
        genericPrologParams.dwStoreDataValue = m_storeData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(&cmdBuffer, &genericPrologParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync)
        {
            if (!m_firstField || CodecHal_PictureIsFrame(m_currOriginalPic))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
            }
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_RESET_STATUS")));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, nullRendering));
    }

    if (m_videoContextUsesNullHw ||
        m_renderContextUsesNullHw)
    {
        if (CodecHalUsesOnlyRenderEngine(m_codecFunction))
        {
            *(encodeStatusBufRcs->pData) = m_storeData;
        }
        else
        {
            *(encodeStatusBuf->pData) = m_storeData;
        }
    }

    encodeStatus->dwHeaderBytesInserted = m_headerBytesInserted;
    m_headerBytesInserted = 0;

    if (!m_disableStatusReport)
    {
        m_storeData++;
        encodeStatusBuf->wCurrIndex    = (encodeStatusBuf->wCurrIndex + 1) % CODECHAL_ENCODE_STATUS_NUM;
        encodeStatusBufRcs->wCurrIndex = (encodeStatusBufRcs->wCurrIndex + 1) % CODECHAL_ENCODE_STATUS_NUM;
    }

    // clean up the Status for next frame
    encodeStatus =
        (EncodeStatus*)(encodeStatusBuf->pEncodeStatus +
        encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize);
    MOS_ZeroMemory((uint8_t*)encodeStatus, sizeof(EncodeStatus));

    if (m_encEnabled)
    {
        EncodeStatus* pEncodeStatusRcs =
            (EncodeStatus*)(encodeStatusBufRcs->pEncodeStatus +
                encodeStatusBufRcs->wCurrIndex * encodeStatusBufRcs->dwReportSize);
        MOS_ZeroMemory((uint8_t*)pEncodeStatusRcs, sizeof(EncodeStatus));
    }

    return eStatus;
}

MOS_STATUS CodechalEncoderState::ReadCounterValue(uint16_t index, EncodeStatusReport* encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport);
    uint64_t *address2Counter = nullptr;

    // Report out counter read from HW
    if (m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface))
    {
        address2Counter = (uint64_t *)(((char *)(m_dataHwCount)) + (index * sizeof(HwCounter)));

        if (MEDIA_IS_WA(m_waTable, WaReadCtrNounceRegister))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_osInterface->osCpInterface->ReadCtrNounceRegister(
                    true,
                    (uint32_t *)&m_regHwCount[index]));
            address2Counter = (uint64_t *)&m_regHwCount[index];
            CODECHAL_ENCODE_NORMALMESSAGE("MMIO returns end ctr is %llx", *address2Counter);
            CODECHAL_ENCODE_NORMALMESSAGE("bitstream size = %d.", encodeStatusReport->bitstreamSize);

            // Here gets the end counter of current bit stream, which should minus counter increment.
            *address2Counter = *address2Counter - (((encodeStatusReport->bitstreamSize + 63) >> 6) << 2);
        }
    }

    // KBL- cann't read counter from HW
    else
    {
        uint32_t ctr[4] = { 0 };
        eStatus = m_hwInterface->GetCpInterface()->GetCounterValue(ctr);
        if (MOS_STATUS_SUCCESS == eStatus)
        {
            address2Counter = (uint64_t *)ctr;
        }
        else
        {
            return eStatus;
        }
    }
    encodeStatusReport->HWCounterValue.Count = *address2Counter;
    //Report back in Big endian
    encodeStatusReport->HWCounterValue.Count = SwapEndianness(encodeStatusReport->HWCounterValue.Count);
    //IV value computation
    encodeStatusReport->HWCounterValue.IV = *(++address2Counter); 
    encodeStatusReport->HWCounterValue.IV = SwapEndianness(encodeStatusReport->HWCounterValue.IV);
    CODECHAL_ENCODE_NORMALMESSAGE(
        "encodeStatusReport->HWCounterValue.Count = 0x%llx, encodeStatusReport->HWCounterValue.IV = 0x%llx",
        encodeStatusReport->HWCounterValue.Count,
        encodeStatusReport->HWCounterValue.IV);
    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Gets available eStatus report data
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::GetStatusReport(
    void *status,
    uint16_t numStatus)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(status);
    EncodeStatusReport *codecStatus = (EncodeStatusReport *)status;

    EncodeStatusBuffer* encodeStatusBuf = nullptr;
    if (m_pakEnabled)
    {
        encodeStatusBuf = &m_encodeStatusBuf;
    }
    else
    {
        encodeStatusBuf = &m_encodeStatusBufRcs;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusBuf->pEncodeStatus);

    uint16_t numReportsAvailable    =
        (encodeStatusBuf->wCurrIndex - encodeStatusBuf->wFirstIndex) &
        (CODECHAL_ENCODE_STATUS_NUM - 1); // max is (CODECHAL_ENCODE_STATUS_NUM - 1)

    uint32_t globalHWStoredData = 0;
    if (m_pakEnabled)
    {
        globalHWStoredData = *(m_encodeStatusBuf.pData);      // HW stored Data
    }
    else
    {
        globalHWStoredData = *(m_encodeStatusBufRcs.pData);   // HW stored Data
    }
    uint32_t globalCount = m_storeData - globalHWStoredData;

    uint16_t reportsGenerated = 0;
    if (m_videoContextUsesNullHw ||
        m_renderContextUsesNullHw)
    {
        for (auto i = 0; i < numReportsAvailable; i++)
        {
            codecStatus[i].CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
            // Set fake bitstream size to avoid DDI report error
            codecStatus[i].bitstreamSize = 1024;
            reportsGenerated++;
        }

        encodeStatusBuf->wFirstIndex =
            (encodeStatusBuf->wFirstIndex + reportsGenerated) % CODECHAL_ENCODE_STATUS_NUM;

        return eStatus;
    }

    CODECHAL_ENCODE_VERBOSEMESSAGE("    numStatus = %d, dwNumReportsAvailable = %d.", numStatus, numReportsAvailable);
    CODECHAL_ENCODE_VERBOSEMESSAGE("    hwstoreData = %d, globalCount = %d", globalHWStoredData, globalCount);

    if (numReportsAvailable < numStatus && numStatus < CODECHAL_ENCODE_STATUS_NUM)
    {
        for (auto i = numReportsAvailable; i < numStatus; i++)
        {
            codecStatus[i].CodecStatus = CODECHAL_STATUS_UNAVAILABLE;
        }
        numStatus = numReportsAvailable;
    }

    if (numReportsAvailable == 0)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("No reports available, wCurrIndex = %d, wFirstIndex = %d", encodeStatusBuf->wCurrIndex, encodeStatusBuf->wFirstIndex);
        return MOS_STATUS_SUCCESS;
    }

    uint16_t index = 0;

    for (auto i = 0; i < numStatus; i++)
    {
        if(codecStatus->bSequential)
        {
            index = (encodeStatusBuf->wFirstIndex + i) & (CODECHAL_ENCODE_STATUS_NUM - 1);
        }
        else
        {
            index = (encodeStatusBuf->wFirstIndex + numStatus - i - 1) & (CODECHAL_ENCODE_STATUS_NUM - 1);
        }

        EncodeStatus* encodeStatus =
            (EncodeStatus*)(encodeStatusBuf->pEncodeStatus +
            index * encodeStatusBuf->dwReportSize);
        EncodeStatusReport* encodeStatusReport = &encodeStatus->encodeStatusReport;
        PCODEC_REF_LIST refList = encodeStatusReport->pCurrRefList;
        PMHW_VDBOX_IMAGE_STATUS_CONTROL imgStatusCtrl = &encodeStatus->ImageStatusCtrl;
        PMHW_VDBOX_PAK_NUM_OF_SLICES numSlices = &encodeStatus->NumSlices;
        uint32_t localCount = encodeStatus->dwStoredData - globalHWStoredData;

        if (localCount == 0 || localCount > globalCount)
        {
            CODECHAL_DEBUG_TOOL(
                m_statusReportDebugInterface->m_bufferDumpFrameNum = encodeStatus->dwStoredData;
            )
            
            // Current command is executed
            if (m_osInterface->pfnIsGPUHung(m_osInterface))
            {
                encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
                *(encodeStatusBuf->pData) += 1;
            }
            else if (encodeStatusReport->Func != CODECHAL_ENCODE_ENC_ID &&
                encodeStatusReport->Func != CODECHAL_ENCODE_FEI_ENC_ID &&
                encodeStatus->dwStoredDataMfx != CODECHAL_STATUS_QUERY_END_FLAG)
            {
                if(encodeStatusReport->Func == CODECHAL_ENCODE_FEI_PRE_ENC_ID)
                {
                    CODECHAL_DEBUG_TOOL(
                        m_statusReportDebugInterface->m_scaledBottomFieldOffset = m_scaledBottomFieldOffset;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpYUVSurface(
                            m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER),
                            CodechalDbgAttr::attrReconstructedSurface,
                            "4xScaledSurf"));

                        /*CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncode1Dbuffer(
                            m_debugInterface,
                            pEncoder));*/

                        // dump EncodeFeiPreproc
                        FeiPreEncParams PreEncParams;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpBuffer(
                            CodecHal_PictureIsBottomField(m_currOriginalPic) ? &PreEncParams.resStatsBotFieldBuffer
                                                                             : &PreEncParams.resStatsBotFieldBuffer,
                            CodechalDbgAttr::attrOutput,
                            "MbStats",
                            m_picWidthInMb * m_frameFieldHeightInMb * 64,
                            CodecHal_PictureIsBottomField(m_currOriginalPic) ? m_mbvProcStatsBottomFieldOffset : 0,
                            CODECHAL_MEDIA_STATE_PREPROC));)
                    encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
                }
                else
                {
                    CODECHAL_ENCODE_NORMALMESSAGE("Media reset may have occured.");
                    encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
                }
            }
            else if (m_hwWalker && encodeStatusReport->Func == CODECHAL_ENCODE_ENC_ID)
            {
                // iterate over all media states and check that all of them completed
                for (auto j = 0; j < CODECHAL_NUM_MEDIA_STATES; j += 1)
                {
                    if (encodeStatus->qwStoredDataEnc[j].dwStoredData != CODECHAL_STATUS_QUERY_END_FLAG)
                    {
                        // some media state failed to complete
                        CODECHAL_ENCODE_ASSERTMESSAGE("Error: Unable to finish encoding");
                        encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
                        break;
                    }
                }

                encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;

                if (m_codecFunction == CODECHAL_FUNCTION_HYBRIDPAK && m_mode == CODECHAL_ENCODE_MODE_VP9 &&
                    encodeStatusReport->CodecStatus != CODECHAL_STATUS_ERROR)
                {
                     unsigned int size = ((m_frameWidth + 63) >> 6) * ((m_frameHeight + 63) >> 6) + 1;
                     encodeStatusReport->bitstreamSize = CODECHAL_VP9_MB_CODE_SIZE * sizeof(uint32_t) * size;
                }
            }
            // The huffman tables sent by application were incorrect (used only for JPEG encoding)
            else if(m_standard == CODECHAL_JPEG && imgStatusCtrl->MissingHuffmanCode == 1)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Error: JPEG standard encoding: missing huffman code");
                encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
            }
            else
            {
                if (m_codecGetStatusReportDefined)
                {
                    // Call corresponding CODEC's status report function if existing
                    eStatus = GetStatusReport(encodeStatus, encodeStatusReport);
                    if (MOS_STATUS_SUCCESS != eStatus)
                    {
                        return eStatus;
                    }

                    if (m_osInterface->osCpInterface->IsCpEnabled() && m_skipFrameBasedHWCounterRead == false)
                    {
                        eStatus = ReadCounterValue(index, encodeStatusReport);
                        if (MOS_STATUS_SUCCESS != eStatus)
                        {
                            return eStatus;
                        }
                    }
                }
                else
                {
                    encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
                    encodeStatusReport->bitstreamSize =
                        encodeStatus->dwMFCBitstreamByteCountPerFrame + encodeStatus->dwHeaderBytesInserted;

                    // dwHeaderBytesInserted is for WAAVCSWHeaderInsertion
                    // and is 0 otherwise
                    encodeStatusReport->QpY = encodeStatus->BrcQPReport.DW0.QPPrimeY;
                    encodeStatusReport->SuggestedQpYDelta =
                        encodeStatus->ImageStatusCtrl.CumulativeSliceDeltaQP;
                    encodeStatusReport->NumberPasses = (uint8_t)(encodeStatus->ImageStatusCtrl.TotalNumPass + 1);
                    encodeStatusReport->SceneChangeDetected =
                        (encodeStatus->dwSceneChangedFlag & CODECHAL_ENCODE_SCENE_CHANGE_DETECTED_MASK) ? 1 : 0;

                    CODECHAL_ENCODE_CHK_NULL_RETURN(m_skuTable);

                    if (m_osInterface->osCpInterface->IsCpEnabled() && m_skipFrameBasedHWCounterRead == false)
                    {
                        eStatus = ReadCounterValue(index, encodeStatusReport);
                        if (MOS_STATUS_SUCCESS != eStatus)
                        {
                            return eStatus;
                        }
                    }

                    if (m_picWidthInMb != 0 && m_frameFieldHeightInMb != 0)
                    {
                        encodeStatusReport->AverageQp = (unsigned char)(((uint32_t)encodeStatus->QpStatusCount.cumulativeQP)
                            / (m_picWidthInMb * m_frameFieldHeightInMb));
                    }
                    encodeStatusReport->PanicMode = encodeStatus->ImageStatusCtrl.Panic;

                    // If Num slices is greater than spec limit set NumSlicesNonCompliant to 1 and report error
                    if (numSlices->NumberOfSlices > m_maxNumSlicesAllowed)
                    {
                        encodeStatusReport->NumSlicesNonCompliant = 1;
                    }
                    encodeStatusReport->NumberSlices = numSlices->NumberOfSlices;
                }

                if (encodeStatusReport->bitstreamSize > m_bitstreamUpperBound)
                {
                    encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
                    encodeStatusReport->bitstreamSize = 0;
                    CODECHAL_ENCODE_ASSERTMESSAGE("Bit-stream size exceeds upper bound!");
                    return MOS_STATUS_NOT_ENOUGH_BUFFER;
                }

                if(refList && refList->bMADEnabled)
                {
                    // set lock flag to READ_ONLY
                    MOS_LOCK_PARAMS lockFlags;
                    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
                    lockFlags.ReadOnly = 1;

                    uint8_t* data = (uint8_t* )m_osInterface->pfnLockResource(
                        m_osInterface,
                        &m_resMadDataBuffer[refList->ucMADBufferIdx],
                        &lockFlags);

                    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

                    eStatus = MOS_SecureMemcpy(
                         &encodeStatusReport->MAD,
                        CODECHAL_MAD_BUFFER_SIZE,
                         data,
                        CODECHAL_MAD_BUFFER_SIZE);
                    if(eStatus != MOS_STATUS_SUCCESS)
                    {
                        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                        return eStatus;
                    }

                    m_osInterface->pfnUnlockResource(
                        m_osInterface,
                        &m_resMadDataBuffer[refList->ucMADBufferIdx]);

                    // The driver needs to divide the output distortion by 4 before sending to the app
                    encodeStatusReport->MAD /= 4;
                }
                else
                {
                    encodeStatusReport->MAD  = 0;
                }

                CODECHAL_DEBUG_TOOL(
                    CODEC_REF_LIST currRefList = *refList;
                    currRefList.RefPic         = encodeStatusReport->CurrOriginalPic;

                    m_statusReportDebugInterface->m_currPic            = encodeStatusReport->CurrOriginalPic;
                    m_statusReportDebugInterface->m_bufferDumpFrameNum = encodeStatus->dwStoredData;
                    m_statusReportDebugInterface->m_frameType          = encodeStatus->wPictureCodingType;

                    if (!m_vdencEnabled) {
                        if (currRefList.bMADEnabled)
                        {
                            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                                m_statusReportDebugInterface->DumpBuffer(
                                &m_resMadDataBuffer[currRefList.ucMADBufferIdx],
                                CodechalDbgAttr::attrInput,
                                "MADWrite",
                                CODECHAL_MAD_BUFFER_SIZE,
                                0,
                                CODECHAL_MEDIA_STATE_ENC_NORMAL));
                        }

                        DumpMbEncPakOutput(refList, m_statusReportDebugInterface);
                    }

                    if (CodecHalUsesVideoEngine(m_codecFunction)) {
                        /*  Only where the MFX engine is used the bitstream surface will be available */
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpBuffer(
                            &currRefList.resBitstreamBuffer,
                            CodechalDbgAttr::attrBitstream,
                            "_PAK",
                            encodeStatusReport->bitstreamSize,
                            0,
                            CODECHAL_NUM_MEDIA_STATES));

                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpData(
                            encodeStatusReport,
                            sizeof(EncodeStatusReport),
                            CodechalDbgAttr::attrStatusReport,
                            "EncodeStatusReport_Buffer"));

                        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpFrameStatsBuffer(m_statusReportDebugInterface));

                        if (m_vdencEnabled)
                        {
                            /*CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeVdencOutputs(
                                m_debugInterface, pEncoder));

                            if (m_cmdGenHucUsed)
                            {
                                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeHucCmdGen(
                                    m_debugInterface, pEncoder));
                            }*/
                        }
                    }

                    if (currRefList.b32xScalingUsed) {
                        m_statusReportDebugInterface->m_scaledBottomFieldOffset = m_scaled32xBottomFieldOffset;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpYUVSurface(
                            m_trackedBuf->Get32xDsSurface(currRefList.ucScalingIdx),
                            CodechalDbgAttr::attrReconstructedSurface,
                            "32xScaledSurf"))
                    }

                    if (currRefList.b2xScalingUsed)  // Currently only used for Gen10 Hevc Encode
                    {
                        m_statusReportDebugInterface->m_scaledBottomFieldOffset = 0;  // No bottom field offset for Hevc
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpYUVSurface(
                            m_trackedBuf->Get2xDsSurface(currRefList.ucScalingIdx),
                            CodechalDbgAttr::attrReconstructedSurface,
                            "2xScaledSurf"))
                    }

                    if (currRefList.b16xScalingUsed) {
                        m_statusReportDebugInterface->m_scaledBottomFieldOffset = m_scaled16xBottomFieldOffset;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpYUVSurface(
                            m_trackedBuf->Get16xDsSurface(currRefList.ucScalingIdx),
                            CodechalDbgAttr::attrReconstructedSurface,
                            "16xScaledSurf"))
                    }

                    if (currRefList.b4xScalingUsed) {
                        m_statusReportDebugInterface->m_scaledBottomFieldOffset = m_scaledBottomFieldOffset;
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpYUVSurface(
                            m_trackedBuf->Get4xDsSurface(currRefList.ucScalingIdx),
                            CodechalDbgAttr::attrReconstructedSurface,
                            "4xScaledSurf"))
                    }

                    if (!(m_codecFunction == CODECHAL_FUNCTION_ENC || m_codecFunction == CODECHAL_FUNCTION_FEI_ENC)) {
                        if (m_codecFunction == CODECHAL_FUNCTION_HYBRIDPAK)
                        {
                            m_statusReportDebugInterface->m_hybridPakP1 = false;
                        }

                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_statusReportDebugInterface->DumpYUVSurface(
                            &currRefList.sRefReconBuffer,
                            CodechalDbgAttr::attrReconstructedSurface,
                            "ReconSurf"))
                    })
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("Incrementing reports generated to %d.", (reportsGenerated + 1));
            reportsGenerated++;
        }
        else
        {
            //update GPU status, and skip the hang frame
            if(m_osInterface->pfnIsGPUHung(m_osInterface))
            {
                *(encodeStatusBuf->pData) += 1;
                reportsGenerated++;
            }

            CODECHAL_ENCODE_VERBOSEMESSAGE("Status buffer %d is INCOMPLETE.", i);
            encodeStatusReport->CodecStatus = CODECHAL_STATUS_INCOMPLETE;
        }
        codecStatus[i] = *encodeStatusReport;
    }

    encodeStatusBuf->wFirstIndex =
        (encodeStatusBuf->wFirstIndex + reportsGenerated) % CODECHAL_ENCODE_STATUS_NUM;
    CODECHAL_ENCODE_VERBOSEMESSAGE("wFirstIndex now becomes %d.", encodeStatusBuf->wFirstIndex);

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Reports user feature keys used for encoding
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncoderState::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;

    // Encode HW Walker Reporting
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = m_hwWalker;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

    if (m_hwWalker)
    {
        // Encode HW Walker m_mode Reporting
#if (_DEBUG || _RELEASE_INTERNAL)
        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.i32Data = m_walkerMode;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
#endif // _DEBUG || _RELEASE_INTERNAL
    }

    if (MEDIA_IS_SKU(m_skuTable, FtrSliceShutdown))
    {
        // SliceShutdownEnable Reporting
        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.i32Data = m_sliceShutdownEnable;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_ENABLE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // report encode CSC method
    if (m_cscDsState)
    {
        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.i32Data = m_cscDsState->CscMethod();
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_CSC_METHOD_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.u32Data = (uint32_t)m_rawSurface.TileType;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_RAW_TILE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

        userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        userFeatureWriteData.Value.u32Data = (uint32_t)m_rawSurface.Format;
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_RAW_FORMAT_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    }

    // Encode compute context Reporting
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = m_computeContextEnabled;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_COMPUTE_CONTEXT_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
#endif

    return eStatus;
}

MOS_STATUS CodechalEncoderState::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    int32_t         nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface->pOsContext);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
    return eStatus;
}

void CodechalEncoderState::MotionEstimationDisableCheck()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_downscaledWidth4x < m_minScaledDimension || m_downscaledWidthInMb4x < m_minScaledDimensionInMb ||
        m_downscaledHeight4x < m_minScaledDimension || m_downscaledHeightInMb4x < m_minScaledDimensionInMb)
    {
        m_32xMeSupported = false;
        m_16xMeSupported = false;
        if (m_downscaledWidth4x < m_minScaledDimension || m_downscaledWidthInMb4x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth4x     = m_minScaledDimension;
            m_downscaledWidthInMb4x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth4x);
        }
        if (m_downscaledHeight4x < m_minScaledDimension || m_downscaledHeightInMb4x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight4x     = m_minScaledDimension;
            m_downscaledHeightInMb4x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight4x);
        }
    }
    else if (m_downscaledWidth16x < m_minScaledDimension  || m_downscaledWidthInMb16x < m_minScaledDimensionInMb ||
             m_downscaledHeight16x < m_minScaledDimension || m_downscaledHeightInMb16x < m_minScaledDimensionInMb)
    {
        m_32xMeSupported = false;
        if (m_downscaledWidth16x < m_minScaledDimension || m_downscaledWidthInMb16x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth16x     = m_minScaledDimension;
            m_downscaledWidthInMb16x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth16x);
        }
        if (m_downscaledHeight16x < m_minScaledDimension || m_downscaledHeightInMb16x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight16x     = m_minScaledDimension;
            m_downscaledHeightInMb16x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight16x);
        }
    }
    else
    {
        if (m_downscaledWidth32x < m_minScaledDimension || m_downscaledWidthInMb32x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth32x     = m_minScaledDimension;
            m_downscaledWidthInMb32x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth32x);
        }
        if (m_downscaledHeight32x < m_minScaledDimension || m_downscaledHeightInMb32x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight32x     = m_minScaledDimension;
            m_downscaledHeightInMb32x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight32x);
        }
    }
}

MOS_STATUS CodechalEncoderState::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool frameTrackingRequested,
    MHW_MI_MMIOREGISTERS* mmioRegister)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    // initialize command buffer attributes
    cmdBuffer->Attributes.bTurboMode               = m_hwInterface->m_turboMode;
    cmdBuffer->Attributes.bMediaPreemptionEnabled  = MOS_RCS_ENGINE_USED(gpuContext) ?
        m_renderEngineInterface->IsPreemptionEnabled() : 0;
    cmdBuffer->Attributes.dwNumRequestedEUSlices   = m_hwInterface->m_numRequestedEuSlices;
    cmdBuffer->Attributes.dwNumRequestedSubSlices  = m_hwInterface->m_numRequestedSubSlices;
    cmdBuffer->Attributes.dwNumRequestedEUs        = m_hwInterface->m_numRequestedEus;
    cmdBuffer->Attributes.bValidPowerGatingRequest = true;

    if (frameTrackingRequested && m_frameTrackingEnabled)
    {
        cmdBuffer->Attributes.bEnableMediaFrameTracking        = true;
        cmdBuffer->Attributes.resMediaFrameTrackingSurface     =
            m_encodeStatusBuf.resStatusBuffer;
        cmdBuffer->Attributes.dwMediaFrameTrackingTag          = m_storeData;
        // Set media frame tracking address offset(the offset from the encoder status buffer page)
        cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset   = 0;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(m_miInterface, cmdBuffer, gpuContext));
#endif

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface            = m_osInterface;
    genericPrologParams.pvMiInterface     = m_miInterface;
    genericPrologParams.bMmcEnabled             = CodecHalMmcState::IsMmcEnabled();
    genericPrologParams.dwStoreDataValue = m_storeData - 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(cmdBuffer, &genericPrologParams, mmioRegister));

    return eStatus;
}

MOS_STATUS CodechalEncoderState::UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS CodechalEncoderState::ExecuteEnc(
    EncoderParams* encodeParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetCpInterface());

    if (m_mfeEnabled == false || encodeParams->ExecCodecFunction == CODECHAL_FUNCTION_ENC
        || encodeParams->ExecCodecFunction == CODECHAL_FUNCTION_FEI_ENC)
    {
        // No need to wait if the driver is executing on a simulator
        EncodeStatusBuffer* pencodeStatusBuf = CodecHalUsesOnlyRenderEngine(m_codecFunction) ? &m_encodeStatusBufRcs : &m_encodeStatusBuf;
        if (!m_osInterface->bSimIsActive &&
            m_recycledBufStatusNum[m_currRecycledBufIdx] >
            *(pencodeStatusBuf->pData))
        {
            uint32_t waitMs;

            // Wait for Batch Buffer complete event OR timeout
            for (waitMs = MHW_TIMEOUT_MS_DEFAULT; waitMs > 0; waitMs -= MHW_EVENT_TIMEOUT_MS)
            {
                if (m_recycledBufStatusNum[m_currRecycledBufIdx] <= *(pencodeStatusBuf->pData))
                {
                    break;
                }

                MOS_Sleep(MHW_EVENT_TIMEOUT_MS);
            }

            CODECHAL_ENCODE_VERBOSEMESSAGE("Waited for %d ms", (MHW_TIMEOUT_MS_DEFAULT - waitMs));

            if (m_recycledBufStatusNum[m_currRecycledBufIdx] >
                *(pencodeStatusBuf->pData))
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("No recycled buffers available, wait timed out at %d ms!", MHW_TIMEOUT_MS_DEFAULT);
                CODECHAL_ENCODE_ASSERTMESSAGE("m_storeData = %d, m_recycledBufStatusNum[%d] = %d, data = %d", m_storeData, m_currRecycledBufIdx, m_recycledBufStatusNum[m_currRecycledBufIdx], *(pencodeStatusBuf->pData));
                return MOS_STATUS_CLIENT_AR_NO_SPACE;
            }
        }

        m_recycledBufStatusNum[m_currRecycledBufIdx] = m_storeData;

        // These parameters are updated at the DDI level
        if (encodeParams->bMbDisableSkipMapEnabled)
        {
            CodecHalGetResourceInfo(m_osInterface, encodeParams->psMbDisableSkipMapSurface);
        }

        CODECHAL_ENCODE_CHK_NULL_RETURN(encodeParams->psRawSurface);
        CodecHalGetResourceInfo(m_osInterface, encodeParams->psRawSurface);
        if (encodeParams->bMbQpDataEnabled)
        {
            CodecHalGetResourceInfo(m_osInterface, encodeParams->psMbQpDataSurface);
        }

        if (m_standard != CODECHAL_JPEG)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(encodeParams->psReconSurface);
            CodecHalGetResourceInfo(m_osInterface, encodeParams->psReconSurface);
        }

        m_encodeParams = *encodeParams;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->UpdateParams(true));

        if (CodecHalUsesVideoEngine(m_codecFunction))
        {
            // Get resource details of the bitstream resource
            MOS_SURFACE details;
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            CODECHAL_ENCODE_CHK_NULL_RETURN(encodeParams->presBitstreamBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, encodeParams->presBitstreamBuffer, &details));

            m_encodeParams.dwBitstreamSize = details.dwHeight * details.dwWidth;
        }

        m_osInterface->pfnIncPerfFrameID(m_osInterface);

        // init function common to all codecs, before encode each frame
        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitCommon());

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(InitializePicture(m_encodeParams),
            "Encoding initialization failed.");

        if (m_newSeq)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CheckResChangeAndCsc());
        }

        if (FRAME_SKIP_NORMAL == m_skipFrameFlag)
        {
            if (m_standard == CODECHAL_MPEG2)
            {
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(EncodeCopySkipFrame(), "Skip-frame failed.\n");
                m_skipFrameFlag = FRAME_NO_SKIP;
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(ResetStatusReport(), "Flushing encode status buffer for skipped frame failed.\n");
                m_firstFrame = false;
                return eStatus;
            }
        }

        MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
        syncParams.bReadOnly = true;

        // Synchronize MB QP data surface resource if any.
        if (encodeParams->bMbQpDataEnabled)
        {
            syncParams.presSyncResource = &encodeParams->psMbQpDataSurface->OsResource;
            syncParams.GpuContext       = m_renderContext;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        }

        // Check if source surface needs to be synchronized and should wait for decode or VPP or any other context
        syncParams.presSyncResource = &m_rawSurface.OsResource;

        if (CodecHalUsesRenderEngine(m_codecFunction, m_standard) &&
            m_firstField)
        {
            syncParams.GpuContext = m_renderContext;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            if (CodecHalUsesVideoEngine(m_codecFunction))
            {
                // Perform Sync on PAK context if it is not ENC only case.
                // This is done to set the read mask for PAK context for on demand sync
                syncParams.GpuContext = m_videoContext;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
            }
            // Update the resource tag (s/w tag) for On-Demand Sync
            // set the tag on render context for ENC case only, else set it on video context for ENC+PAK case
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }
        else if (CodecHalUsesVideoEngine(m_codecFunction))
        {
            // Perform resource sync for encode uses only video engine
            syncParams.GpuContext = m_videoContext;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        CODECHAL_ENCODE_CHK_NULL_RETURN(m_currRefList);

        if (CODECHAL_JPEG != m_standard &&  m_firstField)
        {
            for (int i = 0; i < m_currRefList->ucNumRef; i++)
            {
                CODECHAL_ENCODE_CHK_COND_RETURN(
                    m_currReconstructedPic.FrameIdx == m_currRefList->RefList[i].FrameIdx,
                    "the same frame (FrameIdx = %d) cannot be used as both Recon surface and ref frame",
                    m_currReconstructedPic.FrameIdx);
            }

            // clear flags
            m_currRefList->b2xScalingUsed =
            m_currRefList->b4xScalingUsed =
            m_currRefList->b16xScalingUsed =
            m_currRefList->b32xScalingUsed = false;

            // allocate tracked buffer for current frame
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateForCurrFrame());
            m_currRefList->ucScalingIdx = m_trackedBuf->GetCurrIndex();

            if (m_trackedBuf->IsMbCodeAllocationNeeded())
            {
                // MbCode/MvData buffer can be tracked using the same index as DS surface
                m_currRefList->ucMbCodeIdx = m_currMbCodeIdx = m_trackedBuf->GetCurrIndexMbCode();

                m_resMbCodeSurface = m_currRefList->resRefMbCodeBuffer = *m_trackedBuf->GetCurrMbCodeBuffer();
                if (m_trackedBuf->GetCurrMvDataBuffer())
                {
                    m_resMvDataSurface = m_currRefList->resRefMvDataBuffer = *m_trackedBuf->GetCurrMvDataBuffer();
                }
            }
            else
            {
                CODECHAL_ENCODE_NORMALMESSAGE("App provides MbCode and MvData buffer!");
            }

            m_trackedBuf->SetAllocationFlag(false);
        }

        if (CodecHalUsesRenderEngine(m_codecFunction, m_standard))
        {
            // set render engine context
            m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
            m_osInterface->pfnResetOsStates(m_osInterface);

            // set all status reports to completed state
            InitStatusReport();

            // on-demand sync for tracked buffer
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.bReadOnly = false;
            if (m_trackedBuf->GetWait() && !Mos_ResourceIsNull(&m_resMbCodeSurface))
            {
                syncParams.presSyncResource = &m_resMbCodeSurface;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
                m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
            }

            // Call ENC Kernels
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(ExecuteKernelFunctions(),
                "ENC failed.");
        }
    }

    if (m_mfeEnabled == false || encodeParams->ExecCodecFunction == CODECHAL_FUNCTION_PAK
        || encodeParams->ExecCodecFunction == CODECHAL_FUNCTION_FEI_PAK)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_resBitstreamBuffer, &m_reconSurface));

        if (CodecHalUsesVideoEngine(m_codecFunction))
        {
            // Set to video context
            m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext);
            m_osInterface->pfnResetOsStates(m_osInterface);
            m_currPass = 0;

            for (m_currPass = 0; m_currPass <= m_numPasses; m_currPass++)
            {
                m_firstTaskInPhase = (m_currPass == 0);
                m_lastTaskInPhase = (m_currPass == m_numPasses);

                if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

                // Setup picture level PAK commands
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(ExecutePictureLevel(),
                    "Picture level encoding failed.");

                // Setup slice level PAK commands
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(ExecuteSliceLevel(),
                    "Slice level encoding failed.");

                m_lastTaskInPhase = false;
            }
        }

        m_prevRawSurface = *m_rawSurfaceToPak;

        // User Feature Key Reporting - only happens after first frame
        if (m_firstFrame == true)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(UserFeatureKeyReport(),
                "Reporting user feature keys failed.");
        }

        m_currRecycledBufIdx =
            (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

        if (m_currRecycledBufIdx == 0)
        {
            MOS_ZeroMemory(m_recycledBufStatusNum, sizeof(m_recycledBufStatusNum));
        }

        m_currLaDataIdx = (m_currLaDataIdx + 1) % m_numLaDataEntry;

        // Flush encode eStatus buffer
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(ResetStatusReport(),
            "Flushing encode eStatus buffer failed.");

        if (m_firstFrame == false && m_firstTwoFrames == true)
        {
            m_firstTwoFrames = false;
        }
        m_firstFrame = false;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_resBitstreamBuffer, &m_reconSurface));
    }
    return eStatus;
}

uint8_t CodechalEncoderState::GetNumBrcPakPasses(uint16_t usBRCPrecision)
{
    uint8_t numBRCPAKPasses = CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES;

    switch (usBRCPrecision)
    {
    case 0:
    case 2:     numBRCPAKPasses = CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES;
        break;

    case 1:     numBRCPAKPasses = CODECHAL_ENCODE_BRC_MINIMUM_NUM_PASSES;
        break;

    case 3:     numBRCPAKPasses = CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES;
        break;

    default:    CODECHAL_ENCODE_ASSERT("Invalid BRC Precision value in Pic Params.");
        numBRCPAKPasses = CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES;
        break;
    }

    return numBRCPAKPasses;
}

CodechalEncoderGenState::CodechalEncoderGenState(CodechalEncoderState* encoder)
{
    CODECHAL_ENCODE_ASSERT(encoder);
    m_encoder = encoder;
    m_hwInterface = encoder->GetHwInterface();
    m_osInterface = encoder->GetOsInterface();
    m_miInterface = encoder->m_miInterface;
    m_renderEngineInterface = encoder->m_renderEngineInterface;
    m_stateHeapInterface = encoder->m_stateHeapInterface;
}

CodechalEncoderState::CodechalEncoderState(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo):
    Codechal(hwInterface, debugInterface)
{
    // Add Null checks here for all interfaces.
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
    m_mfxInterface = m_hwInterface->GetMfxInterface();
    m_hcpInterface = m_hwInterface->GetHcpInterface();
    m_hucInterface = m_hwInterface->GetHucInterface();
    m_vdencInterface = m_hwInterface->GetVdencInterface();
    m_miInterface = hwInterface->GetMiInterface();
    m_renderEngineInterface = hwInterface->GetRenderInterface();
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_renderEngineInterface);
    m_stateHeapInterface = m_renderEngineInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_ASSERT(m_renderEngineInterface->GetHwCaps());

    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);
    m_skuTable     = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable      = m_osInterface->pfnGetWaTable(m_osInterface);
    m_gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    m_videoGpuNode = MOS_GPU_NODE_MAX;
    m_renderContext = MOS_GPU_CONTEXT_INVALID_HANDLE;
    m_videoContext  = MOS_GPU_CONTEXT_INVALID_HANDLE;

    m_vdencEnabled = CodecHalUsesVdencEngine(standardInfo->CodecFunction);
    m_codecFunction = standardInfo->CodecFunction;

    m_vdencMeKernelState = MHW_KERNEL_STATE();
    m_vdencStreaminKernelState = MHW_KERNEL_STATE();
    m_vdencMeKernelStateRAB = MHW_KERNEL_STATE();
    m_vdencStreaminKernelStateRAB = MHW_KERNEL_STATE();

    for (auto i = 0; i < CODEC_NUM_FIELDS_PER_FRAME; i++)
    {
        m_scaling2xKernelStates[i] = MHW_KERNEL_STATE();
        m_scaling4xKernelStates[i] = MHW_KERNEL_STATE();
    }
    for (auto i = 0; i < CODECHAL_ENCODE_ME_IDX_NUM; i++)
    {
        m_meKernelStates[i] = MHW_KERNEL_STATE();
    }

    MOS_ZeroMemory(&m_encodeParams, sizeof(m_encodeParams));
    MOS_ZeroMemory(&m_resHwCount, sizeof(m_resHwCount));
    MOS_ZeroMemory(&m_rawSurface, sizeof(m_rawSurface));                // Pointer to MOS_SURFACE of raw surface
    MOS_ZeroMemory(&m_reconSurface, sizeof(m_reconSurface));              // Pointer to MOS_SURFACE of reconstructed surface
    MOS_ZeroMemory(&m_resBitstreamBuffer, sizeof(m_resBitstreamBuffer));         // Pointer to MOS_SURFACE of bitstream surface
    MOS_ZeroMemory(&m_resMbCodeSurface, sizeof(m_resMbCodeSurface));           // Pointer to MOS_SURFACE of MbCode surface
    MOS_ZeroMemory(&m_resMvDataSurface, sizeof(m_resMvDataSurface));           // Pointer to MOS_SURFACE of MvData surface

    MOS_ZeroMemory(&m_resSyncObjectRenderContextInUse, sizeof(m_resSyncObjectRenderContextInUse));
    MOS_ZeroMemory(&m_resSyncObjectVideoContextInUse, sizeof(m_resSyncObjectVideoContextInUse));
    MOS_ZeroMemory(&m_encodeStatusBuf, sizeof(m_encodeStatusBuf));                    // Stores all the status_query related data for PAK engine
    MOS_ZeroMemory(&m_encodeStatusBufRcs, sizeof(m_encodeStatusBufRcs));                 // Stores all the status_query related data for render ring (RCS)
    MOS_ZeroMemory(&m_imgStatusControlBuffer, sizeof(m_imgStatusControlBuffer));         // Stores image eStatus control data
    MOS_ZeroMemory(&m_atomicScratchBuf, sizeof(m_atomicScratchBuf));             // Stores atomic operands and result
    MOS_ZeroMemory(&m_bsBuffer, sizeof(m_bsBuffer));

    MOS_ZeroMemory(&m_resVdencCmdInitializerDmemBuffer, sizeof(m_resVdencCmdInitializerDmemBuffer));
    MOS_ZeroMemory(&m_resVdencCmdInitializerDataBuffer, sizeof(m_resVdencCmdInitializerDataBuffer));

    MOS_ZeroMemory(&m_resDistortionBuffer, sizeof(m_resDistortionBuffer));        // MBEnc Distortion Buffer
    for (auto i = 0; i < CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS; i++)
    {
        MOS_ZeroMemory(&m_resMadDataBuffer[i], sizeof(m_resMadDataBuffer[i])); // Buffers to store Mean of Absolute Differences
    }
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        MOS_ZeroMemory(&m_sliceMapSurface[i], sizeof(m_sliceMapSurface[i]));
    }

    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        MOS_ZeroMemory(&m_resVdencStreamInBuffer[i], sizeof(m_resVdencStreamInBuffer[i]));
    }
    MOS_ZeroMemory(&m_resPakMmioBuffer, sizeof(m_resPakMmioBuffer));
    MOS_ZeroMemory(&m_resHucStatus2Buffer, sizeof(m_resHucStatus2Buffer));
    MOS_ZeroMemory(&m_resHucFwBuffer, sizeof(m_resHucFwBuffer));

    MOS_ZeroMemory(&m_resDeblockingFilterRowStoreScratchBuffer, sizeof(m_resDeblockingFilterRowStoreScratchBuffer));               // Handle of deblock row store surface
    MOS_ZeroMemory(&m_resMPCRowStoreScratchBuffer, sizeof(m_resMPCRowStoreScratchBuffer));                            // Handle of mpc row store surface
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        MOS_ZeroMemory(&m_resStreamOutBuffer[i], sizeof(m_resStreamOutBuffer[i]));    // Handle of streamout data surface
    }

    MOS_ZeroMemory(&m_scaling4xBindingTable, sizeof(m_scaling4xBindingTable));
    MOS_ZeroMemory(&m_scaling2xBindingTable, sizeof(m_scaling2xBindingTable));
    for (auto i = 0; i < CODECHAL_ENCODE_VME_BBUF_NUM; i++)
    {
        MOS_ZeroMemory(&m_scalingBBUF[i], sizeof(m_scalingBBUF[i]));          // This Batch Buffer is used for scaling kernel.
    }
    MOS_ZeroMemory(&m_flatnessCheckSurface, sizeof(m_flatnessCheckSurface));
    MOS_ZeroMemory(&m_resMbStatisticsSurface, sizeof(m_resMbStatisticsSurface));
    MOS_ZeroMemory(&m_resMbStatsBuffer, sizeof(m_resMbStatsBuffer));

    MOS_ZeroMemory(&m_meBindingTable, sizeof(m_meBindingTable));

    MOS_ZeroMemory(&m_vdencMeKernelBindingTable, sizeof(m_vdencMeKernelBindingTable));

    MOS_ZeroMemory(&m_vdencStreaminKernelBindingTable, sizeof(m_vdencStreaminKernelBindingTable));
}

CodechalEncoderState::~CodechalEncoderState()
{
    if (m_gpuCtxCreatOpt)
    {
        MOS_Delete(m_gpuCtxCreatOpt);
        m_gpuCtxCreatOpt = nullptr;
    }

    DestroyMDFResources();

    if (m_perfProfiler)
    {
        MediaPerfProfiler::Destroy(m_perfProfiler, (void*)this, m_osInterface);
        m_perfProfiler = nullptr;
    }
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncoderState::DumpMbEncPakOutput(PCODEC_REF_LIST currRefList, CodechalDebugInterface* debugInterface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(currRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(debugInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            &currRefList->resRefMbCodeBuffer,
            CodechalDbgAttr::attrOutput,
            "MbCode",
            m_picWidthInMb * m_frameFieldHeightInMb * 64,
            CodecHal_PictureIsBottomField(currRefList->RefPic) ? m_frameFieldHeightInMb * m_picWidthInMb * 64 : 0,
            (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
            CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));

    if (m_mvDataSize)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            &currRefList->resRefMvDataBuffer,
            CodechalDbgAttr::attrOutput,
            "MbData",
            m_picWidthInMb * m_frameFieldHeightInMb * (32 * 4),
            CodecHal_PictureIsBottomField(currRefList->RefPic) ? MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * (32 * 4), 0x1000) : 0,
            (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
            CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
    }
    if (CodecHalIsFeiEncode(m_codecFunction))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            &m_resDistortionBuffer,
            CodechalDbgAttr::attrOutput,
            "DistortionSurf",
            m_picWidthInMb * m_frameFieldHeightInMb * 48,
            CodecHal_PictureIsBottomField(currRefList->RefPic) ? MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * 48, 0x1000) : 0,
            (m_codecFunction != CODECHAL_FUNCTION_HYBRIDPAK) ?
            CODECHAL_MEDIA_STATE_ENC_NORMAL : CODECHAL_MEDIA_STATE_HYBRID_PAK_P2));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncoderState::AddBufferWithIMMValue(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    PMOS_RESOURCE               presStoreBuffer,
    uint32_t                    offset,
    uint32_t                    value,
    bool                        bAdd)
{
    MHW_MI_STORE_REGISTER_MEM_PARAMS    StoreRegParams;
    MHW_MI_STORE_DATA_PARAMS            StoreDataParams;
    MHW_MI_LOAD_REGISTER_REG_PARAMS     LoadRegRegParams;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS     LoadRegisterImmParams;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MHW_MI_MATH_PARAMS                  MiMathParams;
    MHW_MI_ALU_PARAMS                   MiAluParams[4];
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())                                                                         \
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    auto pMmioRegistersMfx = m_mfxInterface->GetMmioRegisters(m_vdboxIndex);
    auto pMmioRegistersHcp = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &FlushDwParams));

    MOS_ZeroMemory(&LoadRegRegParams, sizeof(LoadRegRegParams));

    MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
    MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));

    miLoadRegMemParams.presStoreBuffer = presStoreBuffer;
    miLoadRegMemParams.dwOffset = offset;
    miLoadRegMemParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister0LoOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

    MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
    LoadRegisterImmParams.dwData = 0;
    LoadRegisterImmParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister0HiOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
        cmdBuffer,
        &LoadRegisterImmParams));

    MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
    LoadRegisterImmParams.dwData = value;
    LoadRegisterImmParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister4LoOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
        cmdBuffer,
        &LoadRegisterImmParams));
    MOS_ZeroMemory(&LoadRegisterImmParams, sizeof(LoadRegisterImmParams));
    LoadRegisterImmParams.dwData = 0;
    LoadRegisterImmParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister4HiOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
        cmdBuffer,
        &LoadRegisterImmParams));

    MOS_ZeroMemory(&MiMathParams, sizeof(MiMathParams));
    MOS_ZeroMemory(&MiAluParams, sizeof(MiAluParams));
    // load     srcA, reg0
    MiAluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
    MiAluParams[0].Operand1 = MHW_MI_ALU_SRCA;
    MiAluParams[0].Operand2 = MHW_MI_ALU_GPREG0;
    // load     srcB, reg4
    MiAluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
    MiAluParams[1].Operand1 = MHW_MI_ALU_SRCB;
    MiAluParams[1].Operand2 = MHW_MI_ALU_GPREG4;

    if (bAdd)
    {
        // add      srcA, srcB
        MiAluParams[2].AluOpcode = MHW_MI_ALU_ADD;
    }
    else
    {
        // sub      srcA, srcB
        MiAluParams[2].AluOpcode = MHW_MI_ALU_SUB;
    }

    // store      reg0, ACCU
    MiAluParams[3].AluOpcode = MHW_MI_ALU_STORE;
    MiAluParams[3].Operand1 = MHW_MI_ALU_GPREG0;
    MiAluParams[3].Operand2 = MHW_MI_ALU_ACCU;

    MiMathParams.pAluPayload = MiAluParams;
    MiMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiMathCmd(
        cmdBuffer,
        &MiMathParams));

    // update the value
    MOS_ZeroMemory(&StoreRegParams, sizeof(StoreRegParams));
    StoreRegParams.presStoreBuffer = presStoreBuffer;
    StoreRegParams.dwOffset = offset;
    StoreRegParams.dwRegister = pMmioRegistersMfx->generalPurposeRegister0LoOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &StoreRegParams));

    return eStatus;
}
#endif
