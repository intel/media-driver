/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_reference_frames.cpp
//! \brief    Defines reference list related logic for encode vp9
//!

#include "encode_vp9_reference_frames.h"
#include "encode_utils.h"
#include "encode_vp9_basic_feature.h"
#include "encode_vp9_vdenc_pipeline.h"

namespace encode
{
Vp9ReferenceFrames::~Vp9ReferenceFrames()
{
    ENCODE_FUNC_CALL();

    EncodeFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);
}

MOS_STATUS Vp9ReferenceFrames::Init(Vp9BasicFeature *basicFeature)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_basicFeature = basicFeature;
    ENCODE_CHK_STATUS_RETURN(EncodeAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::Update()
{
    ENCODE_FUNC_CALL();

    // initialize internal structures for current frame before set up

    // m_refFrameFlags is to indicate which frames to be used as reference
    // m_refFrameFlags & 0x01 != 0: Last ref frames used as reference
    // m_refFrameFlags & 0x02 != 0: Golden ref frames used as reference
    // m_refFrameFlags & 0x04 != 0: Alternate ref frames used as reference
    m_refFrameFlags = 0;
    m_numRefFrames  = 0;
    m_lastRefPic    = nullptr;
    m_goldenRefPic  = nullptr;
    m_altRefPic     = nullptr;

    m_dysRefFrameFlags = DYS_REF_NONE;
    m_currDysRefList   = nullptr;
    m_dysRefIndex      = 0;

    for (auto i = 0; i < CODEC_VP9_NUM_REF_FRAMES; ++i)
    {
        m_picIdx[i].bValid = false;
    }

    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    // Save current frame information into m_refList
    auto currRefIdx = picParams->CurrReconstructedPic.FrameIdx;
    m_currRefList   = m_refList[currRefIdx];

    m_currRefList->sRefReconBuffer    = m_basicFeature->m_reconSurface;
    m_currRefList->sRefRawBuffer      = m_basicFeature->m_rawSurface;
    m_currRefList->RefPic             = picParams->CurrReconstructedPic;
    m_currRefList->bUsedAsRef         = true;
    m_currRefList->resBitstreamBuffer = m_basicFeature->m_resBitstreamBuffer;
    m_currRefList->dwFrameWidth       = m_basicFeature->m_oriFrameWidth;
    m_currRefList->dwFrameHeight      = m_basicFeature->m_oriFrameHeight;
    // m_currRefList->ucScalingIdx also is one critical index,
    // It used to associate temperal mv buffer, down scaling surfaces.
    // It will be set in m_trackBuf->Acquire(...).

    // Setup reference related structures.
    if ((picParams->PicFlags.fields.frame_type != 0) && !picParams->PicFlags.fields.intra_only)
    {
        ENCODE_CHK_STATUS_RETURN(SetupRefFlags());  // setup m_refFrameFlags
        ENCODE_CHK_STATUS_RETURN(SetupRefPic());    // setup reference pictures (m_lastRefPic, m_goldenRefPic, m_altRefPic
        ENCODE_CHK_STATUS_RETURN(SetupRefIndex());  // setup m_picIdx
    }
    m_dysCurrFrameFlag = m_dysRefFrameFlags;
    ENCODE_CHK_STATUS_RETURN(SetupDysRefPic());     // setup dynamic scaling reference picture

    // Save the RefFrameList for current frame
    uint8_t ii = 0;
    for (auto i = 0; i < CODEC_VP9_NUM_REF_FRAMES; ++i)
    {
        if (m_picIdx[i].bValid)
        {
            m_currRefList->RefList[ii] = picParams->RefFrameList[i];
            ii++;
        }
    }
    m_currRefList->ucNumRef = ii;

    // In case there is overflow
    if ((picParams->LumaACQIndex + picParams->LumaDCQIndexDelta) < 0)
    {
        picParams->LumaACQIndex = MOS_ABS(picParams->LumaDCQIndexDelta) + 1;
    }
    m_currRefList->ucQPValue[0] = picParams->LumaACQIndex + picParams->LumaDCQIndexDelta;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetHcpSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *surfaceParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(surfaceParams);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    // For PAK engine, we do NOT use scaled reference images even if dynamic scaling is enabled
    for (auto i = 0; i < maxReferenceIds; ++i)
    {
        m_refSurface[i] = m_refSurfaceNonScaled[i] = m_dsRefSurface4x[i] = m_dsRefSurface8x[i] = nullptr;
    }

    if (m_basicFeature->m_pictureCodingType == I_TYPE)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);
    auto allocator = m_basicFeature->GetAllocator();
    ENCODE_CHK_NULL_RETURN(allocator);
    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    if (m_refFrameFlags & 0x01)
    {
        auto refPicIdx = picParams->RefFlags.fields.LastRefIdx;
        ENCODE_ASSERT((refPicIdx < CODEC_VP9_NUM_REF_FRAMES) &&
                      (!CodecHal_PictureIsInvalid(picParams->RefFrameList[refPicIdx])));

        auto frameIdx = picParams->RefFrameList[refPicIdx].FrameIdx;

        m_refSurfaceNonScaled[lastFrame] = &(m_refList[frameIdx]->sRefBuffer);
        m_refSurface[lastFrame] =
            (m_dysRefFrameFlags & DYS_REF_LAST) ? &(m_refList[frameIdx]->sDysSurface)
                                                : m_refSurfaceNonScaled[lastFrame];

        auto scalingIdx = m_refList[frameIdx]->ucScalingIdx;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->Resize4x8xforDS(scalingIdx));

        m_dsRefSurface4x[lastFrame] = trackedBuf->GetSurface(BufferType::ds4xSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(m_dsRefSurface4x[lastFrame]);
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_dsRefSurface4x[lastFrame]));

        m_dsRefSurface8x[lastFrame] = trackedBuf->GetSurface(BufferType::ds8xSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(m_dsRefSurface8x[lastFrame]);
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_dsRefSurface8x[lastFrame]));
    }

    if (m_refFrameFlags & 0x02)
    {
        auto refPicIdx = picParams->RefFlags.fields.GoldenRefIdx;
        ENCODE_ASSERT((refPicIdx < CODEC_VP9_NUM_REF_FRAMES) &&
                      (!CodecHal_PictureIsInvalid(picParams->RefFrameList[refPicIdx])));

        auto frameIdx = picParams->RefFrameList[refPicIdx].FrameIdx;

        m_refSurfaceNonScaled[goldenFrame] = &(m_refList[frameIdx]->sRefBuffer);
        m_refSurface[goldenFrame] =
            (m_dysRefFrameFlags & DYS_REF_GOLDEN) ? &(m_refList[frameIdx]->sDysSurface)
                                                  : m_refSurfaceNonScaled[goldenFrame];

        auto scalingIdx = m_refList[frameIdx]->ucScalingIdx;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->Resize4x8xforDS(scalingIdx));

        m_dsRefSurface4x[goldenFrame] = trackedBuf->GetSurface(BufferType::ds4xSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(m_dsRefSurface4x[goldenFrame]);
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_dsRefSurface4x[goldenFrame]));

        m_dsRefSurface8x[goldenFrame] = trackedBuf->GetSurface(BufferType::ds8xSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(m_dsRefSurface8x[goldenFrame]);
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_dsRefSurface8x[goldenFrame]));
    }

    if (m_refFrameFlags & 0x04)
    {
        auto refPicIdx = picParams->RefFlags.fields.AltRefIdx;
        ENCODE_ASSERT((refPicIdx < CODEC_VP9_NUM_REF_FRAMES) &&
                      (!CodecHal_PictureIsInvalid(picParams->RefFrameList[refPicIdx])));

        auto frameIdx = picParams->RefFrameList[refPicIdx].FrameIdx;

        m_refSurfaceNonScaled[altFrame] = &(m_refList[frameIdx]->sRefBuffer);
        m_refSurface[altFrame] =
            (m_dysRefFrameFlags & DYS_REF_ALT) ? &(m_refList[frameIdx]->sDysSurface)
                                               : m_refSurfaceNonScaled[altFrame];

        auto scalingIdx = m_refList[frameIdx]->ucScalingIdx;
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->Resize4x8xforDS(scalingIdx));

        m_dsRefSurface4x[altFrame] = trackedBuf->GetSurface(BufferType::ds4xSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(m_dsRefSurface4x[altFrame]);
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_dsRefSurface4x[altFrame]));

        m_dsRefSurface8x[altFrame] = trackedBuf->GetSurface(BufferType::ds8xSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(m_dsRefSurface8x[altFrame]);
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_dsRefSurface8x[altFrame]));
    }

    if (!m_refSurface[lastFrame])
    {
        m_refSurface[lastFrame] = (m_refSurface[goldenFrame]) ? m_refSurface[goldenFrame]
                                                              : m_refSurface[altFrame];

        m_refSurfaceNonScaled[lastFrame] = (m_refSurfaceNonScaled[goldenFrame]) ? m_refSurfaceNonScaled[goldenFrame]
                                                                                : m_refSurfaceNonScaled[altFrame];

        m_dsRefSurface4x[lastFrame] = (m_dsRefSurface4x[goldenFrame]) ? m_dsRefSurface4x[goldenFrame]
                                                                      : m_dsRefSurface4x[altFrame];

        m_dsRefSurface8x[lastFrame] = (m_dsRefSurface8x[goldenFrame]) ? m_dsRefSurface8x[goldenFrame]
                                                                      : m_dsRefSurface8x[altFrame];
    }

    if (!m_refSurface[goldenFrame])
    {
        m_refSurface[goldenFrame] = (m_refSurface[lastFrame]) ? m_refSurface[lastFrame]
                                                              : m_refSurface[altFrame];

        m_refSurfaceNonScaled[goldenFrame] = (m_refSurfaceNonScaled[lastFrame]) ? m_refSurfaceNonScaled[lastFrame]
                                                                                : m_refSurfaceNonScaled[altFrame];

        m_dsRefSurface4x[goldenFrame] = (m_dsRefSurface4x[lastFrame]) ? m_dsRefSurface4x[lastFrame]
                                                                      : m_dsRefSurface4x[altFrame];

        m_dsRefSurface8x[goldenFrame] = (m_dsRefSurface8x[lastFrame]) ? m_dsRefSurface8x[lastFrame]
                                                                      : m_dsRefSurface8x[altFrame];
    }

    if (!m_refSurface[altFrame])
    {
        m_refSurface[altFrame] = (m_refSurface[lastFrame]) ? m_refSurface[lastFrame]
                                                           : m_refSurface[goldenFrame];

        m_refSurfaceNonScaled[altFrame] = (m_refSurfaceNonScaled[lastFrame]) ? m_refSurfaceNonScaled[lastFrame]
                                                                             : m_refSurfaceNonScaled[goldenFrame];

        m_dsRefSurface4x[altFrame] = (m_dsRefSurface4x[lastFrame]) ? m_dsRefSurface4x[lastFrame]
                                                                   : m_dsRefSurface4x[goldenFrame];

        m_dsRefSurface8x[altFrame] = (m_dsRefSurface8x[lastFrame]) ? m_dsRefSurface8x[lastFrame]
                                                                   : m_dsRefSurface8x[goldenFrame];
    }

    // Program Surface params for Reference surfaces
    if ((m_dysRefFrameFlags != DYS_REF_NONE) && !m_basicFeature->m_dysVdencMultiPassEnabled)
    {
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].psSurface            = m_refSurfaceNonScaled[lastFrame];
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].bVdencDynamicScaling = true;

        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].psSurface            = m_refSurfaceNonScaled[goldenFrame];
        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].bVdencDynamicScaling = true;

        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].psSurface            = m_refSurfaceNonScaled[altFrame];
        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].bVdencDynamicScaling = true;
    }
    else
    {
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].psSurface   = m_refSurface[lastFrame];
        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].psSurface = m_refSurface[goldenFrame];
        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].psSurface = m_refSurface[altFrame];
    }

    if (m_dysCurrFrameFlag)
    {
        if (m_basicFeature->m_dysVdencMultiPassEnabled)
        {
            surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].dwReconSurfHeight =
                MOS_ALIGN_CEIL(
                    (m_refSurface[lastFrame] ? m_refSurface[lastFrame]->dwHeight : 0),
                    CODEC_VP9_MIN_BLOCK_WIDTH);

            surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].dwReconSurfHeight =
                MOS_ALIGN_CEIL(
                    (m_refSurface[goldenFrame] ? m_refSurface[goldenFrame]->dwHeight : 0),
                    CODEC_VP9_MIN_BLOCK_WIDTH);

            surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].dwReconSurfHeight =
                MOS_ALIGN_CEIL(
                    (m_refSurface[altFrame] ? m_refSurface[altFrame]->dwHeight : 0),
                    CODEC_VP9_MIN_BLOCK_WIDTH);
        }
        else
        {
            surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].dwReconSurfHeight =
                MOS_ALIGN_CEIL(
                    (m_refSurfaceNonScaled[lastFrame] ? m_refSurfaceNonScaled[lastFrame]->dwHeight : 0),
                    CODEC_VP9_MIN_BLOCK_WIDTH);

            surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].dwReconSurfHeight =
                MOS_ALIGN_CEIL((m_refSurfaceNonScaled[goldenFrame] ? m_refSurfaceNonScaled[goldenFrame]->dwHeight : 0),
                    CODEC_VP9_MIN_BLOCK_WIDTH);

            surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].dwReconSurfHeight =
                MOS_ALIGN_CEIL((m_refSurfaceNonScaled[altFrame] ? m_refSurfaceNonScaled[altFrame]->dwHeight : 0),
                    CODEC_VP9_MIN_BLOCK_WIDTH);
        }
    }
    else
    {
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].dwReconSurfHeight =
        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].dwReconSurfHeight =
        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].dwReconSurfHeight = m_basicFeature->m_rawSurfaceToPak->dwHeight;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetDysHcpSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *surfaceParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(surfaceParams);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    // For PAK engine, we do NOT use scaled reference images even if dynamic scaling is enabled
    for (auto i = 0; i < maxReferenceIds; ++i)
    {
        m_dysRefSurface[i] = nullptr;
    }

    if (m_basicFeature->m_pictureCodingType == I_TYPE)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    if (m_refFrameFlags & 0x01)
    {
        auto refPicIdx = picParams->RefFlags.fields.LastRefIdx;
        ENCODE_ASSERT((refPicIdx < CODEC_VP9_NUM_REF_FRAMES) &&
                      (!CodecHal_PictureIsInvalid(picParams->RefFrameList[refPicIdx])));

        auto frameIdx = picParams->RefFrameList[refPicIdx].FrameIdx;

        m_dysRefSurface[lastFrame] = &(m_refList[frameIdx]->sRefBuffer);
    }

    if (m_refFrameFlags & 0x02)
    {
        auto refPicIdx = picParams->RefFlags.fields.GoldenRefIdx;
        ENCODE_ASSERT((refPicIdx < CODEC_VP9_NUM_REF_FRAMES) &&
                      (!CodecHal_PictureIsInvalid(picParams->RefFrameList[refPicIdx])));

        auto frameIdx = picParams->RefFrameList[refPicIdx].FrameIdx;

        m_dysRefSurface[goldenFrame] = &(m_refList[frameIdx]->sRefBuffer);
    }

    if (m_refFrameFlags & 0x04)
    {
        auto refPicIdx = picParams->RefFlags.fields.AltRefIdx;
        ENCODE_ASSERT((refPicIdx < CODEC_VP9_NUM_REF_FRAMES) &&
                      (!CodecHal_PictureIsInvalid(picParams->RefFrameList[refPicIdx])));

        auto frameIdx = picParams->RefFrameList[refPicIdx].FrameIdx;

        m_dysRefSurface[altFrame] = &(m_refList[frameIdx]->sRefBuffer);
    }

    if (!m_dysRefSurface[lastFrame])
    {
        m_dysRefSurface[lastFrame] = (m_dysRefSurface[goldenFrame]) ? m_dysRefSurface[goldenFrame]
                                                                    : m_dysRefSurface[altFrame];
    }

    if (!m_dysRefSurface[goldenFrame])
    {
        m_dysRefSurface[goldenFrame] = (m_dysRefSurface[lastFrame]) ? m_dysRefSurface[lastFrame]
                                                                    : m_dysRefSurface[altFrame];
    }

    if (!m_dysRefSurface[altFrame])
    {
        m_dysRefSurface[altFrame] = (m_dysRefSurface[lastFrame]) ? m_dysRefSurface[lastFrame]
                                                                 : m_dysRefSurface[goldenFrame];
    }

    // Program Surface params for Last/Golden/Al Reference surfaces
    surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].psSurface   = m_dysRefSurface[lastFrame];
    surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].psSurface = m_dysRefSurface[goldenFrame];
    surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].psSurface = m_dysRefSurface[altFrame];

    surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].dwReconSurfHeight =
        MOS_ALIGN_CEIL((m_dysRefSurface[lastFrame] ? m_dysRefSurface[lastFrame]->dwHeight : 0),
            CODEC_VP9_MIN_BLOCK_WIDTH);

    surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].dwReconSurfHeight =
        MOS_ALIGN_CEIL((m_dysRefSurface[goldenFrame] ? m_dysRefSurface[goldenFrame]->dwHeight : 0),
            CODEC_VP9_MIN_BLOCK_WIDTH);

    surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].dwReconSurfHeight =
        MOS_ALIGN_CEIL((m_dysRefSurface[altFrame] ? m_dysRefSurface[altFrame]->dwHeight : 0),
            CODEC_VP9_MIN_BLOCK_WIDTH);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetDysHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS *pipeBufAddrParams)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(pipeBufAddrParams);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    if (m_basicFeature->m_pictureCodingType != I_TYPE)
    {
        for (auto i = 0; i < maxReferenceIds; ++i)
        {
            ENCODE_CHK_NULL_RETURN(m_dysRefSurface[i]);

            pipeBufAddrParams->presReferences[i] = &m_dysRefSurface[i]->OsResource;
        }

        pipeBufAddrParams->presColMvTempBuffer[0] =
            trackedBuf->GetBuffer(
                BufferType::mvTemporalBuffer,
                m_basicFeature->m_lastMvTemporalBufferIndex);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetVdencSurfaceParams(MHW_VDBOX_SURFACE_PARAMS *surfaceParams)
{
    ENCODE_FUNC_CALL();

    // Change ref surfaces to scaled for VDENC for DYS
    if ((m_dysRefFrameFlags != DYS_REF_NONE) && !m_basicFeature->m_dysVdencMultiPassEnabled)
    {
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].psSurface   = m_refSurface[lastFrame];
        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].psSurface = m_refSurface[goldenFrame];
        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].psSurface = m_refSurface[altFrame];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetVdencPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS *pipeBufAddrParams)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(pipeBufAddrParams);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    pipeBufAddrParams->bDynamicScalingEnable     = (m_dysRefFrameFlags != DYS_REF_NONE) && !m_basicFeature->m_dysVdencMultiPassEnabled;
    pipeBufAddrParams->dwNumRefIdxL0ActiveMinus1 = (m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type) ? m_numRefFrames - 1 : 0;
    pipeBufAddrParams->dwNumRefIdxL1ActiveMinus1 = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetupRefFlags()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    auto lastRefIdx   = picParams->RefFlags.fields.LastRefIdx;
    auto goldenRefIdx = picParams->RefFlags.fields.GoldenRefIdx;
    auto altRefIdx    = picParams->RefFlags.fields.AltRefIdx;

    m_refFrameFlags = picParams->RefFlags.fields.ref_frame_ctrl_l0 | picParams->RefFlags.fields.ref_frame_ctrl_l1;

    if (CodecHal_PictureIsInvalid(picParams->RefFrameList[lastRefIdx]))
    {
        m_refFrameFlags &= ~0x1;
    }
    if (CodecHal_PictureIsInvalid(picParams->RefFrameList[goldenRefIdx]))
    {
        m_refFrameFlags &= ~0x2;
    }
    if (CodecHal_PictureIsInvalid(picParams->RefFrameList[altRefIdx]))
    {
        m_refFrameFlags &= ~0x4;
    }

    // Consolidate the reference flag, because two reference frame may have the same index
    if ((m_refFrameFlags & 0x01) &&
        (picParams->RefFrameList[lastRefIdx].FrameIdx == picParams->RefFrameList[altRefIdx].FrameIdx))
    {
        m_refFrameFlags &= ~0x4;  // Skip alt frame
    }
    if ((m_refFrameFlags & 0x02) &&
        (picParams->RefFrameList[goldenRefIdx].FrameIdx == picParams->RefFrameList[altRefIdx].FrameIdx))
    {
        m_refFrameFlags &= ~0x4;  // Skip alt frame
    }

    if (m_refFrameFlags == 7 && !m_basicFeature->m_16xMeSupported)
    {
        // Can support max 2 reference frames when SHME disabled, so ignore alt frame
        m_refFrameFlags &= ~0x4;
    }

    // Max number of reference is 1 for TU7
    if (m_refFrameFlags != 1 && TargetUsage::isSpeed(m_basicFeature->m_vp9SeqParams->TargetUsage))
    {
        m_refFrameFlags = 1;
    }

    if (m_refFrameFlags == 0)
    {
        ENCODE_ASSERTMESSAGE("Ref list is empty!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetupRefPic()
{
    ENCODE_FUNC_CALL();

    auto allocator = m_basicFeature->GetAllocator();
    ENCODE_CHK_NULL_RETURN(allocator);

    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto seqParams = m_basicFeature->m_vp9SeqParams;
    ENCODE_CHK_NULL_RETURN(seqParams);

    // Last reference frame
    if (m_refFrameFlags & 0x01)
    {
        auto refIdx = picParams->RefFlags.fields.LastRefIdx;
        auto index  = picParams->RefFrameList[refIdx].FrameIdx;

        m_refList[index]->sRefBuffer =
            seqParams->SeqFlags.fields.bUseRawReconRef ? m_refList[index]->sRefRawBuffer
                                                       : m_refList[index]->sRefReconBuffer;

        m_lastRefPic = &m_refList[index]->sRefBuffer;
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_lastRefPic));
        m_lastRefPic->dwWidth  = m_refList[index]->dwFrameWidth;
        m_lastRefPic->dwHeight = m_refList[index]->dwFrameHeight;
        m_numRefFrames++;

        if (seqParams->SeqFlags.fields.EnableDynamicScaling &&
            ((m_refList[index]->dwFrameWidth != m_basicFeature->m_oriFrameWidth) ||
                (m_refList[index]->dwFrameHeight != m_basicFeature->m_oriFrameHeight)))
        {
            m_dysRefFrameFlags |= DYS_REF_LAST;
        }
    }

    // Golden reference frame
    if (m_refFrameFlags & 0x02)
    {
        auto refIdx = picParams->RefFlags.fields.GoldenRefIdx;
        auto index  = picParams->RefFrameList[refIdx].FrameIdx;

        m_refList[index]->sRefBuffer =
            seqParams->SeqFlags.fields.bUseRawReconRef ? m_refList[index]->sRefRawBuffer
                                                       : m_refList[index]->sRefReconBuffer;

        m_goldenRefPic = &m_refList[index]->sRefBuffer;
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_goldenRefPic));
        m_goldenRefPic->dwWidth  = m_refList[index]->dwFrameWidth;
        m_goldenRefPic->dwHeight = m_refList[index]->dwFrameHeight;
        m_numRefFrames++;

        if (seqParams->SeqFlags.fields.EnableDynamicScaling &&
            ((m_refList[index]->dwFrameWidth != m_basicFeature->m_oriFrameWidth) ||
                (m_refList[index]->dwFrameHeight != m_basicFeature->m_oriFrameHeight)))
        {
            m_dysRefFrameFlags |= DYS_REF_GOLDEN;
        }
    }

    // Alter reference frame
    if (m_refFrameFlags & 0x04)
    {
        auto refIdx = picParams->RefFlags.fields.AltRefIdx;
        auto index  = picParams->RefFrameList[refIdx].FrameIdx;

        m_refList[index]->sRefBuffer =
            seqParams->SeqFlags.fields.bUseRawReconRef ? m_refList[index]->sRefRawBuffer
                                                       : m_refList[index]->sRefReconBuffer;

        m_altRefPic = &m_refList[index]->sRefBuffer;
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(m_altRefPic));
        m_altRefPic->dwWidth  = m_refList[index]->dwFrameWidth;
        m_altRefPic->dwHeight = m_refList[index]->dwFrameHeight;
        m_numRefFrames++;

        if (seqParams->SeqFlags.fields.EnableDynamicScaling &&
            ((m_refList[index]->dwFrameWidth != m_basicFeature->m_oriFrameWidth) ||
                (m_refList[index]->dwFrameHeight != m_basicFeature->m_oriFrameHeight)))
        {
            m_dysRefFrameFlags |= DYS_REF_ALT;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetupDysRefPic()
{
    ENCODE_FUNC_CALL();

    if (m_dysRefFrameFlags == DYS_REF_NONE)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto allocator = m_basicFeature->GetAllocator();
    ENCODE_CHK_NULL_RETURN(allocator);

    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    // Allocate dynamic scaled surfaces if needed
    uint8_t frameIdx = 0, dysRefIdx = 0, numDysRefFrames = 0;

    if (m_dysRefFrameFlags & DYS_REF_LAST)
    {
        auto refIndex = picParams->RefFlags.fields.LastRefIdx;
        frameIdx      = picParams->RefFrameList[refIndex].FrameIdx;
        dysRefIdx     = 1;
        numDysRefFrames++;
    }
    if (m_dysRefFrameFlags & DYS_REF_GOLDEN) {
        auto refIndex = picParams->RefFlags.fields.GoldenRefIdx;
        frameIdx      = picParams->RefFrameList[refIndex].FrameIdx;
        dysRefIdx     = 2;
        numDysRefFrames++;
    }
    if (m_dysRefFrameFlags & DYS_REF_ALT)
    {
        auto refIndex = picParams->RefFlags.fields.AltRefIdx;
        frameIdx      = picParams->RefFrameList[refIndex].FrameIdx;
        dysRefIdx     = 3;
        numDysRefFrames++;
    }
    if (numDysRefFrames > 1)
    {
        // For performance reason, we only support single reference for dynamic scaling
        ENCODE_ASSERTMESSAGE("Only single reference is suppored for dynamic scaling!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PCODEC_REF_LIST *refList = &m_refList[0];
    if (Mos_ResourceIsNull(&refList[frameIdx]->sDysSurface.OsResource) ||
        (refList[frameIdx]->sDysSurface.dwWidth != m_basicFeature->m_reconSurface.dwWidth) ||
        (refList[frameIdx]->sDysSurface.dwHeight != m_basicFeature->m_reconSurface.dwHeight))
    {
        // free existing resource first if resolution changes
        if (!Mos_ResourceIsNull(&refList[frameIdx]->sDysSurface.OsResource))
        {
            ENCODE_CHK_STATUS_RETURN(allocator->DestroyResource(&refList[frameIdx]->sDysSurface.OsResource));
        }

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer;
        MOS_ZeroMemory(&allocParamsForBuffer, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer.Type            = MOS_GFXRES_2D;
        allocParamsForBuffer.TileType        = MOS_TILE_Y;
        allocParamsForBuffer.Format          = m_basicFeature->m_reconSurface.Format;
        allocParamsForBuffer.bIsCompressible = m_basicFeature->m_mmcState ? m_basicFeature->m_mmcState->IsMmcEnabled() : false;

        allocParamsForBuffer.dwWidth  = MOS_ALIGN_CEIL(m_basicFeature->m_reconSurface.dwWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
        allocParamsForBuffer.dwHeight = MOS_ALIGN_CEIL(m_basicFeature->m_reconSurface.dwHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
        allocParamsForBuffer.pBufName = "Dynamic Scaled Surface for VP9";

        auto allocatedResource = allocator->AllocateResource(allocParamsForBuffer, false);
        ENCODE_CHK_NULL_RETURN(allocatedResource);
        refList[frameIdx]->sDysSurface.OsResource = *allocatedResource;
        ENCODE_CHK_STATUS_RETURN(allocator->GetSurfaceInfo(&refList[frameIdx]->sDysSurface));
    }

    refList[frameIdx]->sDysSurface.dwWidth  = m_basicFeature->m_oriFrameWidth;
    refList[frameIdx]->sDysSurface.dwHeight = m_basicFeature->m_oriFrameHeight;

    m_currDysRefList = refList[frameIdx];
    m_dysRefIndex    = dysRefIdx;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::SetupRefIndex()
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);

    for (auto i = 0; i < CODEC_VP9_NUM_REF_FRAMES; ++i)
    {
        if (picParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            auto index         = picParams->RefFrameList[i].FrameIdx;
            bool duplicatedIdx = false;
            for (auto j = 0; j < i; ++j)
            {
                if (m_picIdx[j].bValid && index == picParams->RefFrameList[j].FrameIdx)
                {
                    // We find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                    duplicatedIdx = true;
                    break;
                }
            }
            if (duplicatedIdx)
            {
                continue;
            }

            // This reference frame is unique. Save it into the full reference list with 127 items
            m_refList[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(m_refList[index]->RefPic, picParams->RefFrameList[i]);

            m_picIdx[i].bValid   = true;
            m_picIdx[i].ucPicIdx = index;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9ReferenceFrames::DumpInput(Vp9VdencPipeline *pipeline)
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    CodechalDebugInterface *debugInterface = pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    std::stringstream pipeIdxStrStream;
    pipeIdxStrStream << "_" << (int)pipeline->GetCurrentPipe();

    std::string surfacePassName = "Pass" + std::to_string((uint32_t)pipeline->GetCurrentPass());
    surfacePassName += pipeIdxStrStream.str() + "_input";

    // Last reference
    if ((m_refFrameFlags & 0x01) && m_lastRefPic)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            m_lastRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            (surfacePassName + "_LastRefSurf").data()));
    }

    // Golden reference
    if (m_refFrameFlags & 0x02 && m_goldenRefPic)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            m_goldenRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            (surfacePassName + "_GoldenRefSurf").data()));
    }

    // Alt reference
    if (m_refFrameFlags & 0x04 && m_altRefPic)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            m_altRefPic,
            CodechalDbgAttr::attrReferenceSurfaces,
            (surfacePassName + "_AltRefSurf").data()));
    }
#endif

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp9ReferenceFrames::usePrevInFindMvRef() const
{
    auto picParams     = m_basicFeature->m_vp9PicParams;
    auto prevFrameInfo = m_basicFeature->m_prevFrameInfo;

    if (picParams->PicFlags.fields.error_resilient_mode ||
        prevFrameInfo.KeyFrame || prevFrameInfo.IntraOnly || !prevFrameInfo.ShowFrame ||
        (prevFrameInfo.FrameWidth != (picParams->SrcFrameWidthMinus1 + 1)) ||
        (prevFrameInfo.FrameHeight != (picParams->SrcFrameHeightMinus1 + 1)))
    {
        return 0;
    }

    return 1;
}

MOS_STATUS Vp9ReferenceFrames::SetDysValue(bool value)
{
    m_dysEnabled = value;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_VP9_PIC_STATE, Vp9ReferenceFrames)
{
    ENCODE_FUNC_CALL();

    auto picParams = m_basicFeature->m_vp9PicParams;

    params.refFrameSignBias02 = picParams->RefFlags.fields.LastRefSignBias          |
                                (picParams->RefFlags.fields.GoldenRefSignBias << 1) |
                                (picParams->RefFlags.fields.AltRefSignBias    << 2);

    if (picParams->PicFlags.fields.frame_type && !picParams->PicFlags.fields.intra_only)
    {
        uint32_t curFrameWidth  = picParams->SrcFrameWidthMinus1 + 1;
        uint32_t curFrameHeight = picParams->SrcFrameHeightMinus1 + 1;

        uint32_t m_vp9ScalingFactor = (1 << 14);

        bool useDysRefSurface = (m_dysRefFrameFlags != DYS_REF_NONE) && m_basicFeature->m_dysVdencMultiPassEnabled;

        PCODEC_PICTURE refFrameList = &(picParams->RefFrameList[0]);

        params.lastFrameType = !m_basicFeature->m_prevFrameInfo.KeyFrame;

        params.usePrevInFindMvReferences = usePrevInFindMvRef();

        if ((picParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x01) || (picParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x01))
        {
            uint8_t  lastRefPicIndex    = refFrameList[picParams->RefFlags.fields.LastRefIdx].FrameIdx;
            uint32_t lastRefFrameWidth  = m_refList[lastRefPicIndex]->dwFrameWidth;
            uint32_t lastRefFrameHeight = m_refList[lastRefPicIndex]->dwFrameHeight;

            if (useDysRefSurface)
            {
                lastRefFrameWidth  = curFrameWidth;
                lastRefFrameHeight = curFrameWidth;
            }

            params.horizontalScaleFactorForLast  = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            params.verticalScaleFactorForLast    = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;
            params.lastFrameWidthInPixelsMinus1  = lastRefFrameWidth - 1;
            params.lastFrameHeightInPixelsMinus1 = lastRefFrameHeight - 1;
        }

        if ((picParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x02) || (picParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x02))
        {
            uint8_t  goldenRefPicIndex    = refFrameList[picParams->RefFlags.fields.GoldenRefIdx].FrameIdx;
            uint32_t goldenRefFrameWidth  = m_refList[goldenRefPicIndex]->dwFrameWidth;
            uint32_t goldenRefFrameHeight = m_refList[goldenRefPicIndex]->dwFrameHeight;

            if (useDysRefSurface)
            {
                goldenRefFrameWidth  = curFrameWidth;
                goldenRefFrameHeight = curFrameHeight;
            }

            params.horizontalScaleFactorForGolden = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            params.verticalScaleFactorForGolden   = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            params.goldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
            params.goldenFrameHeightInPixelsMinus1 = goldenRefFrameHeight - 1;
        }

        if ((picParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x04) || (picParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x04))
        {
            uint8_t  altRefPicIndex    = refFrameList[picParams->RefFlags.fields.AltRefIdx].FrameIdx;
            uint32_t altRefFrameWidth  = m_refList[altRefPicIndex]->dwFrameWidth;
            uint32_t altRefFrameHeight = m_refList[altRefPicIndex]->dwFrameHeight;

            if (useDysRefSurface)
            {
                altRefFrameWidth  = curFrameWidth;
                altRefFrameHeight = curFrameHeight;
            }

            params.horizontalScaleFactorForAltref = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            params.verticalScaleFactorForAltref   = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            params.altrefFrameWidthInPixelsMinus1  = altRefFrameWidth - 1;
            params.altrefFrameHeightInPixelsMinus1 = altRefFrameHeight - 1;
        }
    }

    char *lfRefDelta  = &(picParams->LFRefDelta[0]);
    char *lfModeDelta = &(picParams->LFModeDelta[0]);

    params.lfRefDelta0 = m_basicFeature->Convert2SignMagnitude((lfRefDelta[0]), 7);
    params.lfRefDelta1 = m_basicFeature->Convert2SignMagnitude((lfRefDelta[1]), 7);
    params.lfRefDelta2 = m_basicFeature->Convert2SignMagnitude((lfRefDelta[2]), 7);
    params.lfRefDelta3 = m_basicFeature->Convert2SignMagnitude((lfRefDelta[3]), 7);

    params.lfModeDelta0 = m_basicFeature->Convert2SignMagnitude((lfModeDelta[0]), 7);
    params.lfModeDelta1 = m_basicFeature->Convert2SignMagnitude((lfModeDelta[1]), 7);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9ReferenceFrames)
{
    ENCODE_FUNC_CALL();

    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    if (!m_dysEnabled)
    {
        params.bDynamicScalingEnable     = (m_dysRefFrameFlags != DYS_REF_NONE) && !m_basicFeature->m_dysVdencMultiPassEnabled;
        params.dwNumRefIdxL0ActiveMinus1 = (m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type) ? m_numRefFrames - 1 : 0;
        params.dwNumRefIdxL1ActiveMinus1 = 0;

        // Add for P frame support
        if (m_basicFeature->m_pictureCodingType != I_TYPE)
        {
            for (auto i = 0; i < maxReferenceIds; ++i)
            {
                ENCODE_CHK_NULL_RETURN(m_refSurface[i]);
                ENCODE_CHK_NULL_RETURN(m_dsRefSurface4x[i]);
                ENCODE_CHK_NULL_RETURN(m_dsRefSurface8x[i]);

                if ((m_dysRefFrameFlags != DYS_REF_NONE) && !m_basicFeature->m_dysVdencMultiPassEnabled)
                {
                    params.presReferences[i]     = &m_refSurfaceNonScaled[i]->OsResource;
                    params.presReferences[i + 4] = &m_refSurfaceNonScaled[i]->OsResource;
                }
                else
                {
                    params.presReferences[i] = &m_refSurface[i]->OsResource;
                }
                params.presVdencReferences[i] = &m_refSurface[i]->OsResource;

                // 4x/8x DS surface for VDEnc
                params.presVdenc4xDsSurface[i] = &m_dsRefSurface4x[i]->OsResource;
                params.presVdenc8xDsSurface[i] = &m_dsRefSurface8x[i]->OsResource;
            }

            if ((m_dysRefFrameFlags != DYS_REF_NONE) && !m_basicFeature->m_dysVdencMultiPassEnabled)
            {
                params.psFwdRefSurface0 = m_refSurface[lastFrame];
                params.psFwdRefSurface1 = m_refSurface[goldenFrame];
                params.psFwdRefSurface2 = m_refSurface[altFrame];
            }
        }
    }
    else
    {
        if (m_basicFeature->m_pictureCodingType != I_TYPE)
        {
            for (auto i = 0; i < maxReferenceIds; ++i)
            {
                ENCODE_CHK_NULL_RETURN(m_dysRefSurface[i]);

                params.presReferences[i] = &m_dysRefSurface[i]->OsResource;
            }
        }

        m_dysEnabled = false;
    }

    if (m_basicFeature->m_pictureCodingType != I_TYPE)
    {
        params.presColMvTempBuffer[0] = trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, m_basicFeature->m_lastMvTemporalBufferIndex);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Vp9ReferenceFrames)
{
    ENCODE_FUNC_CALL();

    auto trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(trackedBuf);

    params.numActiveRefL0 = (m_basicFeature->m_vp9PicParams->PicFlags.fields.frame_type) ? m_numRefFrames : 1;
    params.numActiveRefL1 = 0;

    // Add for P frame support
    if (m_basicFeature->m_pictureCodingType != I_TYPE)
    {
        for (auto i = 0; i < maxReferenceIds; ++i)
        {
            ENCODE_CHK_NULL_RETURN(m_refSurface[i]);
            ENCODE_CHK_NULL_RETURN(m_dsRefSurface4x[i]);
            ENCODE_CHK_NULL_RETURN(m_dsRefSurface8x[i]);

            params.refs[i]         = &m_refSurface[i]->OsResource;
            params.refsDsStage2[i] = &m_dsRefSurface4x[i]->OsResource;
            params.refsDsStage1[i] = &m_dsRefSurface8x[i]->OsResource;
        }

        params.colMvTempBuffer[0] = trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, m_basicFeature->m_lastMvTemporalBufferIndex);
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
