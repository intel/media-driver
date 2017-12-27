/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file      codechal_decode_avc.cpp
//! \brief     This modules implements Render interface layer for AVC decoding to be used on all operating systems/DDIs, across CODECHAL components.
//!

#include "codechal_decoder.h"
#include "codechal_decode_avc.h"
#include "codechal_decode_sfc_avc.h"
#include "codechal_mmc_decode_avc.h"
#include "codechal_secure_decode.h"
#include "codechal_cenc_decode.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

//Check whether interview prediction is used through POC
#define CodecHalAVC_IsInterviewPred(currPic, currPoc, avcRefListIdx) ( ((avcRefListIdx)!=(currPic).FrameIdx) &&              \
    (!CodecHal_PictureIsTopField(currPic)    && (pAvcRefList[avcRefListIdx]->iFieldOrderCnt[1] == (currPoc)[1]) ||       \
     !CodecHal_PictureIsBottomField(currPic) && (pAvcRefList[avcRefListIdx]->iFieldOrderCnt[0] == (currPoc)[0])) &&      \
    ((currPic).FrameIdx != 0x7f) )

MOS_STATUS CodechalDecodeAvc::SendSlice(
    PMHW_VDBOX_AVC_SLICE_STATE      avcSliceState,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(avcSliceState);
    CODECHAL_DECODE_CHK_NULL_RETURN(avcSliceState->pAvcPicIdx);
    CODECHAL_DECODE_CHK_NULL_RETURN(avcSliceState->pAvcPicParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(avcSliceState->pAvcSliceParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(avcSliceState->ppAvcRefList);

    PCODEC_AVC_PIC_PARAMS avcPicParams = avcSliceState->pAvcPicParams;
    PCODEC_AVC_SLICE_PARAMS slc = avcSliceState->pAvcSliceParams;

    avcSliceState->ucDisableDeblockingFilterIdc = slc->disable_deblocking_filter_idc;
    avcSliceState->ucSliceBetaOffsetDiv2 = slc->slice_beta_offset_div2;
    avcSliceState->ucSliceAlphaC0OffsetDiv2 = slc->slice_alpha_c0_offset_div2;

    if (avcSliceState->bShortFormatInUse)
    {
        // send slice address except last one
        if (!avcSliceState->bLastSlice)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdAvcSliceAddrCmd(
                cmdBuffer,
                avcSliceState));
        }
    }
    else
    {
        MHW_VDBOX_AVC_REF_IDX_PARAMS refIdxParams;
        MOS_ZeroMemory(&refIdxParams, sizeof(MHW_VDBOX_AVC_REF_IDX_PARAMS));

        if (!m_mfxInterface->IsAvcISlice(slc->slice_type))
        {
            refIdxParams.CurrPic = avcPicParams->CurrPic;
            refIdxParams.uiList = 0;
            refIdxParams.uiNumRefForList = slc->num_ref_idx_l0_active_minus1 + 1;

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                &refIdxParams.RefPicList,
                sizeof(refIdxParams.RefPicList),
                &slc->RefPicList,
                sizeof(slc->RefPicList)),
                "Failed to copy memory");

            refIdxParams.pAvcPicIdx = avcSliceState->pAvcPicIdx;
            refIdxParams.ppAvcRefList = avcSliceState->ppAvcRefList;
            refIdxParams.bIntelEntrypointInUse = avcSliceState->bIntelEntrypointInUse;
            refIdxParams.bPicIdRemappingInUse = avcSliceState->bPicIdRemappingInUse;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcRefIdx(cmdBuffer, nullptr, &refIdxParams));

            MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS weightOffsetParams;

            if (m_mfxInterface->IsAvcPSlice(slc->slice_type) &&
                avcPicParams->pic_fields.weighted_pred_flag == 1)
            {
                weightOffsetParams.uiList = 0;
                CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                    &weightOffsetParams.Weights,
                    sizeof(weightOffsetParams.Weights),
                    &slc->Weights,
                    sizeof(slc->Weights)),
                    "Failed to copy memory");

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcWeightOffset(cmdBuffer, nullptr, &weightOffsetParams));
            }

            if (m_mfxInterface->IsAvcBSlice(slc->slice_type))
            {
                refIdxParams.uiList = 1;
                refIdxParams.uiNumRefForList = slc->num_ref_idx_l1_active_minus1 + 1;
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcRefIdx(cmdBuffer, nullptr, &refIdxParams));

                if (avcPicParams->pic_fields.weighted_bipred_idc == 1)
                {
                    weightOffsetParams.uiList = 0;
                    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                        &weightOffsetParams.Weights,
                        sizeof(weightOffsetParams.Weights),
                        &slc->Weights,
                        sizeof(slc->Weights)),
                        "Failed to copy memory");

                    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcWeightOffset(cmdBuffer, nullptr, &weightOffsetParams));
                    weightOffsetParams.uiList = 1;
                    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcWeightOffset(cmdBuffer, nullptr, &weightOffsetParams));
                }
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcSlice(cmdBuffer, nullptr, avcSliceState));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdAvcBsdObjectCmd(
        cmdBuffer,
        avcSliceState));

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::FormatAvcMonoPicture()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PCODEC_AVC_PIC_PARAMS picParams = (PCODEC_AVC_PIC_PARAMS)pAvcPicParams;
    if (picParams->seq_fields.chroma_format_idc != avcChromaFormatMono)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_SURFACE dstSurface;
    MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
    dstSurface.Format = Format_NV12;
    dstSurface.OsResource = m_decodeParams.m_destSurface->OsResource;
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &dstSurface));

    uint32_t height = dstSurface.dwHeight;
    uint32_t pitch = dstSurface.dwPitch;
    uint32_t chromaHeight = height / 2;
    uint32_t frameHeight = MOS_ALIGN_CEIL(height, 16);
    uint32_t alignedFrameHeight = MOS_ALIGN_CEIL(frameHeight, MOS_YTILE_H_ALIGNMENT);
    uint32_t alignedChromaHeight = MOS_ALIGN_CEIL(chromaHeight, MOS_YTILE_H_ALIGNMENT);
    uint32_t frameSize = pitch * MOS_ALIGN_CEIL((frameHeight + chromaHeight), MOS_YTILE_H_ALIGNMENT);
    uint32_t chromaBufSize = MOS_ALIGN_CEIL(pitch * alignedChromaHeight, MHW_PAGE_SIZE);

    if (Mos_ResourceIsNull(&resMonoPictureChromaBuffer))
    {
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMonoPictureChromaBuffer,
            chromaBufSize,
            "MonoPictureChromaBuffer",
            true,
            CODECHAL_DECODE_AVC_MONOPIC_CHROMA_DEFAULT),
            "Failed to allocate MonoPicture Chroma Buffer.");
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CodechalHucStreamoutParams hucStreamOutParams;
    if (!m_hwInterface->m_noHuC)
    {
        m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContextForWa);
        m_osInterface->pfnResetOsStates(m_osInterface);


        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));

        // use huc stream out to do clear to clear copy

        MOS_ZeroMemory(&hucStreamOutParams, sizeof(hucStreamOutParams));
        hucStreamOutParams.dataBuffer = &resMonoPictureChromaBuffer;
        hucStreamOutParams.streamOutObjectBuffer = &m_decodeParams.m_destSurface->OsResource;
    }

    uint32_t uvblockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    uint32_t uvrowSize = pitch * uvblockHeight * 2;

    uint32_t dstOffset = 0, x = 0, uvsize = 0;
    if (frameHeight % MOS_YTILE_H_ALIGNMENT)
    {
        dstOffset = LinearToYTiledAddress(x, frameHeight, pitch);

        if (m_hwInterface->m_noHuC)
        {
            CodechalDataCopyParams dataCopyParams;
            MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
            dataCopyParams.srcResource = &resMonoPictureChromaBuffer;
            dataCopyParams.srcSize     = uvrowSize;
            dataCopyParams.srcOffset   = 0;
            dataCopyParams.dstResource = &m_decodeParams.m_destSurface->OsResource;
            dataCopyParams.dstSize     = frameSize;
            dataCopyParams.dstOffset   = dstOffset;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
                &cmdBuffer,                                 // pCmdBuffer
                &resMonoPictureChromaBuffer,                // presSrc
                &m_decodeParams.m_destSurface->OsResource,  // presDst
                uvrowSize,                               // u32CopyLength
                0,                                          // u32CopyInputOffset
                dstOffset));                             // u32CopyOutputOffset
        }
    }

    dstOffset = dstSurface.UPlaneOffset.iSurfaceOffset;
    uvsize    = frameSize - pitch * alignedFrameHeight;

    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource     = &resMonoPictureChromaBuffer;
        dataCopyParams.srcSize         = uvsize;
        dataCopyParams.srcOffset       = 0;
        dataCopyParams.dstResource     = &m_decodeParams.m_destSurface->OsResource;
        dataCopyParams.dstSize         = frameSize;
        dataCopyParams.dstOffset       = dstOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
            &cmdBuffer,                                 // pCmdBuffer
            &resMonoPictureChromaBuffer,                // presSrc
            &m_decodeParams.m_destSurface->OsResource,  // presDst
            uvsize,                                  // u32CopyLength
            0,                                          // u32CopyInputOffset
            dstOffset));                             // u32CopyOutputOffset

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        MOS_SYNC_PARAMS syncParams;

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

        m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext);
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::AllocateInvalidRefBuffer()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //AlloctateResource
    if (Mos_ResourceIsNull(&resInvalidRefBuffer))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &sDestSurface));

        MOS_SURFACE surface;
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateSurface(
            &surface,
            sDestSurface.dwPitch,
            sDestSurface.dwHeight,
            "InvalidRefBuffer"),
            "Failed to allocate invalid reference buffer.");
        resInvalidRefBuffer = surface.OsResource;

        // set all the pixels to 1<<(BitDepth-1) according to the AVC spec
        CodechalResLock ResourceLock(m_osInterface, &resInvalidRefBuffer);
        auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
        CODECHAL_DECODE_CHK_NULL_RETURN(data);

        uint32_t size = sDestSurface.dwPitch * sDestSurface.dwHeight * 3 / 2;
        MOS_FillMemory(data, size, CODECHAL_DECODE_AVC_INVALID_REFPIC_VALUE);  //INVALID_REFPIC_VALUE = 1<<(8-1): Intel Graphic only support 8bit AVC output now.
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::SetAndAllocateDmvBufferIndex(
    PCODEC_AVC_DMV_LIST         avcMVBufList,
    bool                        usedForRef,
    uint8_t                     frameIdx,
    uint32_t                    avcDmvBufferSize,
    uint8_t                     *dmvIdx,
    MOS_RESOURCE                *avcDmvBuffers)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(avcMVBufList);
    CODECHAL_DECODE_CHK_NULL_RETURN(dmvIdx);
    CODECHAL_DECODE_CHK_NULL_RETURN(avcDmvBuffers);

    uint8_t index = 0;
    if (usedForRef)
    {
        uint8_t i;
        for (i = 0; i < CODEC_AVC_NUM_REF_DMV_BUFFERS; i++)
        {
            if (!avcMVBufList[i].bInUse)
            {
                index = i;
                avcMVBufList[i].bInUse = true;
                avcMVBufList[i].ucFrameId = frameIdx;
                break;
            }
        }
        if (i == CODEC_AVC_NUM_REF_DMV_BUFFERS)
        {
            // Should never happen, something must be wrong
            CODECHAL_DECODE_ASSERTMESSAGE("No DMV Buffer found.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }
    else
    {
        index = CODEC_AVC_NUM_REF_DMV_BUFFERS;
    }

    if (Mos_ResourceIsNull(&avcDmvBuffers[index]))
    {
        // Allocate DMV buffer if it has not been allocated already.
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &avcDmvBuffers[index],
            avcDmvBufferSize,
            "MvBuffer"),
            "Failed to allocate MV Buffer.");
    }

    *dmvIdx = index;
    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::InitMvcDummyDmvBuffer(
    uint32_t                    *mvcWaDummyDmvBuf,
    uint32_t                    size,
    PMOS_RESOURCE               mvcDummyDmvBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
        mvcDummyDmvBuffer,
        size,
        "MvBuffer"),
        "Failed to allocate WA Mvc Dummy DMV Buffer.");

    uint8_t *dummyDmvBuffer = nullptr, *mbDmvBuffer = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(dummyDmvBuffer = (uint8_t*)MOS_AllocAndZeroMemory(size));
    mbDmvBuffer = dummyDmvBuffer;

    uint32_t i, numMBs = size / 64;
    for (i = 0; i<numMBs; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(mbDmvBuffer, 64, mvcWaDummyDmvBuf, 64));
        mbDmvBuffer += 64;
    }

    CodechalResLock ResourceLock(m_osInterface, mvcDummyDmvBuffer);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(data, size, (void*)dummyDmvBuffer, size));

    MOS_FreeMemAndSetNull(dummyDmvBuffer);
    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;
    
    CODECHAL_DECODE_CHK_NULL_RETURN(pAvcRefList);
    for (int i = 0; i < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE; i++)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(pAvcRefList[i]);
    }

    PCODEC_AVC_PIC_PARAMS picParams = pAvcPicParams;
    CODEC_PICTURE prevPic = CurrPic;
    CODEC_PICTURE currPic = picParams->CurrPic;

    uint8_t i, ii;
    if (picParams->pic_fields.field_pic_flag)
    {
        // Note: The AVC criteria of second field following first field is not compatible with MVC.
        // For MVC compatibility, a buffer of 16 frame indexes are used to store
        // the consecutive first fiels of different views in the same Acess Unit.
        ii = CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS;
        // Find if the current field is a second field (no matching first field)
        for (i = 0; i < CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS; i++)
        {
            if (u8FirstFieldIdxList[i] == currPic.FrameIdx)
            {
                break;
            }
            // At the same time, find the first empty entrance to store the frame index.
            // Used when the current field is a first field.
            ii = (u8FirstFieldIdxList[i] == CODECHAL_DECODE_AVC_INVALID_FRAME_IDX
                && ii == CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS) ? i : ii;
        }

        if ((prevPic.PicFlags != currPic.PicFlags) || (prevPic.FrameIdx != currPic.FrameIdx) || (currPic.PicFlags == PICTURE_INVALID))
        {
            if (i < CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS)
            {
                bSecondField = true;
                u8FirstFieldIdxList[i] = CODECHAL_DECODE_AVC_INVALID_FRAME_IDX;
                while (i > 0) // Clear all smaller indexes in case there are missing fields.
                {
                    u8FirstFieldIdxList[--i] = CODECHAL_DECODE_AVC_INVALID_FRAME_IDX;
                }
            }
            else
            {
                bSecondField = false;
                if (ii == CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS)
                {
                    ii = 0;
                }
                else
                {
                    u8FirstFieldIdxList[ii++] = currPic.FrameIdx;
                }
                // Clear all larger indexes in case there are missing fields.
                while (ii < CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS)
                {
                    u8FirstFieldIdxList[ii++] = CODECHAL_DECODE_AVC_INVALID_FRAME_IDX;
                }
            }
        }

        m_height =
            MOS_ALIGN_CEIL(m_height, MOS_YTILE_H_ALIGNMENT);
    }
    else
    {
        for (i = 0; i < CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS; i++)
        {
            u8FirstFieldIdxList[i] = CODECHAL_DECODE_AVC_INVALID_FRAME_IDX;
        }
        bSecondField = false;
    }

    m_statusReportFeedbackNumber = picParams->StatusReportFeedbackNumber;
    CurrPic = currPic;
    pAvcRefList[currPic.FrameIdx]->RefPic = CurrPic;
    pAvcRefList[currPic.FrameIdx]->resRefPic = sDestSurface.OsResource;

    // Override reference list with ref surface passed from DDI
    for (i = 0; i < u32RefSurfaceNum; i++)
    {
        pAvcRefList[i]->resRefPic = pRefFrameSurface[i].OsResource;
    }

    uint8_t index, duplicatedIdx;
    for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (!CodecHal_PictureIsInvalid(picParams->RefFrameList[i]))
        {
            index = picParams->RefFrameList[i].FrameIdx;
            pAvcRefList[index]->sFrameNumber = picParams->FrameNumList[i];
        }
    }

    PCODEC_AVC_DMV_LIST avcMVBufList = &avcDmvList[0];
    uint8_t dmvidx;
    if (!CodecHal_PictureIsInvalid(prevPic))
    {
        for (i = 0; i < CODEC_AVC_NUM_REF_DMV_BUFFERS; i++)
        {
            avcMVBufList[i].bInUse = false;
        }
        for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (!CodecHal_PictureIsInvalid(picParams->RefFrameList[i]))
            {
                index = picParams->RefFrameList[i].FrameIdx;
                avcMVBufList[pAvcRefList[index]->ucDMVIdx[0]].bInUse = true;
                avcMVBufList[pAvcRefList[index]->ucDMVIdx[1]].bInUse = true;
            }
        }
    }
    else if (pAvcRefList[currPic.FrameIdx]->bUsedAsRef && !bSecondField)
    {
        dmvidx = pAvcRefList[currPic.FrameIdx]->ucFrameId;
        if (avcFrameStoreID[dmvidx].reUse)
        {
            avcFrameStoreID[dmvidx].reUse = false;
        }
        else
        {
            avcFrameStoreID[dmvidx].inUse = false;
        }

        dmvidx = pAvcRefList[currPic.FrameIdx]->ucDMVIdx[0];
        if (avcDmvList[dmvidx].bReUse)
        {
            avcDmvList[dmvidx].bReUse = false;
        }
        else
        {
            avcDmvList[dmvidx].bInUse = false;
        }
        dmvidx = pAvcRefList[currPic.FrameIdx]->ucDMVIdx[1];
        if (avcDmvList[dmvidx].bReUse)
        {
            avcDmvList[dmvidx].bReUse = false;
        }
        else
        {
            avcDmvList[dmvidx].bInUse = false;
        }
    }

    if (picParams->pic_fields.reference_pic_flag)
    {
        if (!bSecondField)
        {
            pAvcRefList[currPic.FrameIdx]->bUsedAsRef = true;
        }
    }
    else
    {
        pAvcRefList[currPic.FrameIdx]->bUsedAsRef = false;
    }

    if (!bSecondField)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetAndAllocateDmvBufferIndex(
            &avcDmvList[0],
            (bool)picParams->pic_fields.reference_pic_flag,
            currPic.FrameIdx,
            u32AvcDmvBufferSize,
            &u8AvcMvBufferIndex,
            resAvcDmvBuffers));

        dmvidx = u8AvcMvBufferIndex;

        //first and second field will use the same DMV buffer in field mode
        pAvcRefList[currPic.FrameIdx]->ucDMVIdx[0] = dmvidx;
        pAvcRefList[currPic.FrameIdx]->ucDMVIdx[1] = dmvidx;
    }
    else
    {
        dmvidx = pAvcRefList[currPic.FrameIdx]->ucDMVIdx[1];
        u8AvcMvBufferIndex = dmvidx;
    }

    avcDmvList[dmvidx].ucFrameId = currPic.FrameIdx;

    for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        avcFrameStoreID[i].inUse = false;
    }

    PCODEC_PIC_ID picIdx = &avcPicIdx[0];
    bool invalidRef = false;
    for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        picIdx[i].bValid = false;
        index = picParams->RefFrameList[i].FrameIdx;

        if (!CodecHal_PictureIsInvalid(picParams->RefFrameList[i]))
        {
            if (Mos_ResourceIsNull(&(pAvcRefList[index]->resRefPic)))
            {
                // when I picture has invalid reference pictures, reallocate resource for the referecnce pictures.
                if (picParams->pic_fields.IntraPicFlag)
                {
                    eStatus = AllocateInvalidRefBuffer();
                    if (eStatus != MOS_STATUS_SUCCESS)
                    {
                        invalidRef = true;
                        CODECHAL_DECODE_ASSERTMESSAGE("Can not allocate Invalid Ref Buffer.");
                        continue;
                    }

                    pAvcRefList[index]->RefPic = picParams->RefFrameList[i];
                    pAvcRefList[index]->resRefPic = resInvalidRefBuffer;
                    pAvcRefList[index]->bUsedAsRef = true;
                    pAvcRefList[index]->ucDMVIdx[0] = pAvcRefList[currPic.FrameIdx]->ucDMVIdx[0];
                    pAvcRefList[index]->ucDMVIdx[1] = pAvcRefList[currPic.FrameIdx]->ucDMVIdx[1];
                }
                else
                {
                    invalidRef = false;
                    CODECHAL_DECODE_ASSERTMESSAGE("Found Invalid Ref.");
                    continue;
                }
            }

            duplicatedIdx = 0;
            for (ii = 0; ii < i; ii++)
            {
                if (picIdx[ii].bValid && index == picParams->RefFrameList[ii].FrameIdx)
                {
                    duplicatedIdx = 1;
                    break;
                }
            }
            if (duplicatedIdx)
            {
                continue;
            }

            pAvcRefList[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(pAvcRefList[index]->RefPic, picParams->RefFrameList[i]);
            pAvcRefList[index]->iFieldOrderCnt[0] = picParams->FieldOrderCntList[i][0];
            pAvcRefList[index]->iFieldOrderCnt[1] = picParams->FieldOrderCntList[i][1];

            picIdx[i].bValid = true;
            picIdx[i].ucPicIdx = index;

            if (!CodecHal_PictureIsInvalid(prevPic))
            {
                for (ii = 0; ii < pAvcRefList[prevPic.FrameIdx]->ucNumRef; ii++)
                {
                    if (index == pAvcRefList[prevPic.FrameIdx]->RefList[ii].FrameIdx)
                    {
                        if (pAvcRefList[index]->ucFrameId == 0x7f)
                        {
                            // Should never happen, something must be wrong
                            CODECHAL_DECODE_ASSERTMESSAGE("Invaid Ref Frame Id Found");
                            pAvcRefList[index]->ucFrameId = 0;
                        }
                        avcFrameStoreID[pAvcRefList[index]->ucFrameId].inUse = true;
                        break;
                    }
                }
                if (ii == pAvcRefList[prevPic.FrameIdx]->ucNumRef)
                {
                    pAvcRefList[index]->ucFrameId = 0x7f;
                }
            }
            else
            {
                avcDmvList[pAvcRefList[index]->ucDMVIdx[0]].ucFrameId =
                    avcDmvList[pAvcRefList[index]->ucDMVIdx[1]].ucFrameId =
                    pAvcRefList[index]->ucFrameId;
            }
            picIdx[i].ucDMVOffset[0] = pAvcRefList[index]->ucDMVIdx[0];
            picIdx[i].ucDMVOffset[1] = pAvcRefList[index]->ucDMVIdx[1];
        }
    }

    // Save the current RefList and corresponding reference frame flags
    ii = 0;
    uint16_t nonExistingFrameFlags = 0;
    uint32_t usedForReferenceFlags = 0;
    for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (picIdx[i].bValid)
        {
            pAvcRefList[currPic.FrameIdx]->RefList[ii] = picParams->RefFrameList[i];
            nonExistingFrameFlags |= ((picParams->NonExistingFrameFlags >> i) & 1) << ii;
            usedForReferenceFlags |= ((picParams->UsedForReferenceFlags >> (i * 2)) & 3) << (ii * 2);
            ii++;
        }
    }
    pAvcRefList[currPic.FrameIdx]->ucNumRef = ii;
    pAvcRefList[currPic.FrameIdx]->usNonExistingFrameFlags = nonExistingFrameFlags;
    pAvcRefList[currPic.FrameIdx]->uiUsedForReferenceFlags = usedForReferenceFlags;

    SetFrameStoreIds(currPic.FrameIdx);

    // Store CurrFieldOrderCnt
    if (CodecHal_PictureIsBottomField(currPic))
    {
        pAvcRefList[currPic.FrameIdx]->iFieldOrderCnt[1] = picParams->CurrFieldOrderCnt[1];
    }
    else
    {
        pAvcRefList[currPic.FrameIdx]->iFieldOrderCnt[0] = picParams->CurrFieldOrderCnt[0];
        if (CodecHal_PictureIsFrame(currPic))
        {
            pAvcRefList[currPic.FrameIdx]->iFieldOrderCnt[1] = picParams->CurrFieldOrderCnt[1];
        }
    }

    //MVC related inter-view reference    
    pAvcRefList[currPic.FrameIdx]->bUsedAsInterViewRef = false;
    if (pMvcExtPicParams)
    {
        if (pMvcExtPicParams->inter_view_flag)
        {
            pAvcRefList[currPic.FrameIdx]->bUsedAsInterViewRef = true;
        }
    }

    if (invalidRef)
    {
        // There was an invalid reference, do not process the picture.
        return MOS_STATUS_UNKNOWN;
    }
    else
    {
        return MOS_STATUS_SUCCESS;
    }
}

MOS_STATUS CodechalDecodeAvc::AllocateResourcesVariableSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODEC_AVC_PIC_PARAMS picParam = pAvcPicParams;
    uint32_t numMacroblocks, numSliceRecords;
    uint16_t picWidthInMB, picHeightInMB;
    bool invalidSliceNum = false;
    picWidthInMB = MOS_MAX(u16PicWidthInMbLastMaxAlloced, (picParam->pic_width_in_mbs_minus1 + 1));
    picHeightInMB = MOS_MAX(u16PicHeightInMbLastMaxAlloced, (picParam->pic_height_in_mbs_minus1 + 1));
    numMacroblocks = picWidthInMB * picHeightInMB;

    numSliceRecords = numMacroblocks;
    if (u32NumSlices > numMacroblocks)
    {
        invalidSliceNum = true;
        numSliceRecords = u32NumSlices;
    }

    if ((numSliceRecords > (uint32_t)u16PicWidthInMbLastMaxAlloced * u16PicHeightInMbLastMaxAlloced) ||
        (pVldSliceRecord == nullptr))
    {
        if (pVldSliceRecord != nullptr)
        {
            MOS_FreeMemory(pVldSliceRecord);
        }
        pVldSliceRecord =
            (PCODECHAL_VLD_SLICE_RECORD)MOS_AllocAndZeroMemory(numSliceRecords * sizeof(CODECHAL_VLD_SLICE_RECORD));
        if (pVldSliceRecord == nullptr)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate memory.");
            eStatus = MOS_STATUS_NO_SPACE;
            return eStatus;
        }
    }

    if (invalidSliceNum == true)
    {
        for (uint32_t i = 0; i < numSliceRecords; i++)
        {
            pVldSliceRecord[i].dwSkip = true;
        }
    }

    if ((picWidthInMB > u16PicWidthInMbLastMaxAlloced) ||
        Mos_ResourceIsNull(&resMfdDeblockingFilterRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&resMfdDeblockingFilterRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMfdDeblockingFilterRowStoreScratchBuffer);
        }

        // Deblocking Filter Row Store Scratch buffer
        //(Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMfdDeblockingFilterRowStoreScratchBuffer,
            picWidthInMB * 4 * CODECHAL_CACHELINE_SIZE,
            "DeblockingScratchBuffer"),
            "Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
    }

    if (m_mfxInterface->IsBsdMpcRowstoreCacheEnabled() == false)
    {
        uint16_t  tempBsdMpcRowStoreScratchBufferPicWidthInMB;
        tempBsdMpcRowStoreScratchBufferPicWidthInMB
            = MOS_MAX(u16BsdMpcRowStoreScratchBufferPicWidthInMb, (picParam->pic_width_in_mbs_minus1 + 1));

        if ((tempBsdMpcRowStoreScratchBufferPicWidthInMB > u16BsdMpcRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&resBsdMpcRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resBsdMpcRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resBsdMpcRowStoreScratchBuffer);
            }
            // BSD/MPC Row Store Scratch buffer
            // (FrameWidth in MB) * (2) * (CacheLine size per MB)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resBsdMpcRowStoreScratchBuffer,
                tempBsdMpcRowStoreScratchBufferPicWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                "MpcScratchBuffer"),
                "Failed to allocate BSD/MPC Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u16BsdMpcRowStoreScratchBufferPicWidthInMb = tempBsdMpcRowStoreScratchBufferPicWidthInMB;
    }

    if (m_mfxInterface->IsIntraRowstoreCacheEnabled() == false)
    {
        uint16_t  tempMfdIntraRowStoreScratchBufferPicWidthInMB;
        tempMfdIntraRowStoreScratchBufferPicWidthInMB = MOS_MAX(u16MfdIntraRowStoreScratchBufferPicWidthInMb, (picParam->pic_width_in_mbs_minus1 + 1));

        if ((tempMfdIntraRowStoreScratchBufferPicWidthInMB > u16MfdIntraRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&resMfdIntraRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resMfdIntraRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMfdIntraRowStoreScratchBuffer);
            }
            // Intra Row Store Scratch buffer
            // (FrameWidth in MB) * (CacheLine size per MB)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMfdIntraRowStoreScratchBuffer,
                tempMfdIntraRowStoreScratchBufferPicWidthInMB * CODECHAL_CACHELINE_SIZE,
                "IntraScratchBuffer"),
                "Failed to allocate AVC BSD Intra Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u16MfdIntraRowStoreScratchBufferPicWidthInMb = tempMfdIntraRowStoreScratchBufferPicWidthInMB;
    }

    if (m_mfxInterface->IsMprRowstoreCacheEnabled() == false)
    {
        uint16_t  tempMprRowStoreScratchBufferPicWidthInMB;
        tempMprRowStoreScratchBufferPicWidthInMB = MOS_MAX(u16MprRowStoreScratchBufferPicWidthInMb, (picParam->pic_width_in_mbs_minus1 + 1));

        if ((tempMprRowStoreScratchBufferPicWidthInMB > u16MprRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&resMprRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resMprRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMprRowStoreScratchBuffer);
            }
            // MPR Row Store Scratch buffer
            // (FrameWidth in MB) * (CacheLine size per MB) * 2
            // IVB+ platforms need to have double MPR size for MBAFF
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMprRowStoreScratchBuffer,
                tempMprRowStoreScratchBufferPicWidthInMB * CODECHAL_CACHELINE_SIZE * 2,
                "MprScratchBuffer"),
                "Failed to allocate AVC BSD MPR Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u16MprRowStoreScratchBufferPicWidthInMb = tempMprRowStoreScratchBufferPicWidthInMB;
    }

    uint32_t ctr;
    if ((picWidthInMB > u16PicWidthInMbLastMaxAlloced) ||
        (picHeightInMB > u16PicHeightInMbLastMaxAlloced) ||
        (u32AvcDmvBufferSize == 0))
    {
        for (ctr = 0; ctr < CODEC_AVC_NUM_DMV_BUFFERS; ctr++)
        {
            if (!Mos_ResourceIsNull(&resAvcDmvBuffers[ctr]))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resAvcDmvBuffers[ctr]);
            }
        }

        // AVC MV buffer, 64 bytes per MB
        u32AvcDmvBufferSize =
            64 * picWidthInMB * (uint32_t)(MOS_ALIGN_CEIL(picHeightInMB, 2));

        for (ctr = 0; ctr < 4; ctr++)
        {
            // Allocate first 3 ref DMV buffers and then grow as needed, always 
            // allocate the last DMV buffer which is used for non referenced pictures. 
            if (ctr == 3)
            {
                ctr = CODEC_AVC_NUM_DMV_BUFFERS - 1;
            }

            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resAvcDmvBuffers[ctr],
                u32AvcDmvBufferSize,
                "MvBuffer"),
                "Failed to allocate Linux WA AVC BSD MV Buffer.");
        }
    }

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AllocateResource(this));
    }

    //record the width and height used for allocation internal resources.
    u16PicWidthInMbLastMaxAlloced = picWidthInMB;
    u16PicHeightInMbLastMaxAlloced = picHeightInMB;

    return eStatus;
}


MOS_STATUS CodechalDecodeAvc::AllocateResourcesFixedSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &resSyncObjectWaContextInUse));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &resSyncObjectVideoContextInUse));

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    CodecHalAllocateDataList(
        pAvcRefList,
        CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);

    CurrPic.PicFlags = PICTURE_INVALID;
    CurrPic.FrameIdx = CODEC_AVC_NUM_UNCOMPRESSED_SURFACE;

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeAvc, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeAvc::AllocateStandard(
    PCODECHAL_SETTINGS          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    if (settings->bDownsamplingHinted)
    {
        // Create Render Context for field scaling
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_RENDER,
            MOS_GPU_NODE_3D,
            MOS_GPU_CONTEXT_CREATE_DEFAULT));
        m_renderContext = MOS_GPU_CONTEXT_RENDER;
    }

    bIntelEntrypointInUse = (settings->bIntelEntrypointInUse) ? true : false;
    m_width = settings->dwWidth;
    m_height = settings->dwHeight;
    u16PicWidthInMb = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width);
    u16PicHeightInMb = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_height);
    bShortFormatInUse = (settings->bShortFormatInUse) ? true : false;

#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(SfcState.InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif
    for (uint8_t i = 0; i < CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS; i++)
    {
        u8FirstFieldIdxList[i] = CODECHAL_DECODE_AVC_INVALID_FRAME_IDX;
    }

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        CODECHAL_DECODE_MODE_AVCVLD,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        bShortFormatInUse);

    // Slice Level Commands (cannot be placed in 2nd level batch)
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        CODECHAL_DECODE_MODE_AVCVLD,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        bShortFormatInUse);

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesFixedSizes());

    return eStatus;
}

CodechalDecodeAvc::~CodechalDecodeAvc()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CodecHalFreeDataList(pAvcRefList, CODEC_AVC_NUM_UNCOMPRESSED_SURFACE);

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &resSyncObjectWaContextInUse);

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &resSyncObjectVideoContextInUse);

    MOS_FreeMemory(pVldSliceRecord);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMfdDeblockingFilterRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resBsdMpcRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMfdIntraRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMprRowStoreScratchBuffer);

    if (!Mos_ResourceIsNull(&resMonoPictureChromaBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resMonoPictureChromaBuffer);
    }

    for (uint32_t ctr = 0; ctr < CODEC_AVC_NUM_DMV_BUFFERS; ctr++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resAvcDmvBuffers[ctr]);
    }

    if (!Mos_ResourceIsNull(&resInvalidRefBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resInvalidRefBuffer);
    }

    return;
}

MOS_STATUS CodechalDecodeAvc::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);

    if (m_cencDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->SetParamsForDecode(this, m_hwInterface, m_debugInterface, &m_decodeParams));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resDataBuffer,
                CodechalDbgAttr::attrBitstream,
                "_DEC",
                u32DataSize,
                u32DataOffset,
                CODECHAL_NUM_MEDIA_STATES));
        )
    }
    else
    {
        u32DataSize = m_decodeParams.m_dataSize;
        u32DataOffset = m_decodeParams.m_dataOffset;
        u32NumSlices = m_decodeParams.m_numSlices;
        pAvcPicParams = (PCODEC_AVC_PIC_PARAMS)m_decodeParams.m_picParams;
        pMvcExtPicParams = (PCODEC_MVC_EXT_PIC_PARAMS)m_decodeParams.m_extPicParams;
        pAvcSliceParams = (PCODEC_AVC_SLICE_PARAMS)m_decodeParams.m_sliceParams;
        pAvcIQMatrixParams = (PCODEC_AVC_IQ_MATRIX_PARAMS)m_decodeParams.m_iqMatrixBuffer;
        sDestSurface = *(m_decodeParams.m_destSurface);
        pRefFrameSurface = m_decodeParams.m_refFrameSurface;
        u32RefSurfaceNum = m_decodeParams.m_refSurfaceNum;
        CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);
        resDataBuffer = *(m_decodeParams.m_dataBuffer);
        bPicIdRemappingInUse = (m_decodeParams.m_picIdRemappingInUse) ? true : false;

        CODECHAL_DECODE_CHK_NULL_RETURN(pAvcSliceParams);
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(pAvcPicParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(pAvcIQMatrixParams);

    m_width = (pAvcPicParams->pic_width_in_mbs_minus1 + 1) * CODECHAL_MACROBLOCK_WIDTH;
    m_height = (pAvcPicParams->pic_height_in_mbs_minus1 + 1) * CODECHAL_MACROBLOCK_HEIGHT;
    bDeblockingEnabled = false;

    if (bShortFormatInUse)
    {
        // When HW parses the slice_header, disable_deblocking_filter_idc is not yet known,
        // so always enable ILDB in this case.
        bDeblockingEnabled = true;
    }
    else
    {
        for (uint32_t i = 0; i < u32NumSlices; i++)
        {
            if (pAvcSliceParams[i].disable_deblocking_filter_idc != 1)
            {
                bDeblockingEnabled = true;
                break;
            }
        }
    }

    if (m_mfxInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
        rowstoreParams.Mode = CODECHAL_DECODE_MODE_AVCVLD;
        rowstoreParams.bMbaff = pAvcPicParams->seq_fields.mb_adaptive_frame_field_flag;
        rowstoreParams.bIsFrame = pAvcPicParams->seq_fields.frame_mbs_only_flag;
        rowstoreParams.dwPicWidth = m_width;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesVariableSizes());

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetPictureStructs());

    CODECHAL_DECODE_CHK_STATUS_RETURN(FormatAvcMonoPicture());

    if (pAvcPicParams->pic_fields.IntraPicFlag)
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
#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DECODE_PROCESSING_PARAMS *decProcessingParams = m_decodeParams.m_procParams;
    if (decProcessingParams != nullptr)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_fieldScalingInterface);

        CODECHAL_DECODE_CHK_STATUS_RETURN(SfcState.CheckAndInitialize(
            decProcessingParams,
            pAvcPicParams,
            m_width,
            m_height,
            bDeblockingEnabled));

        if (!((!CodecHal_PictureIsFrame(pAvcPicParams->CurrPic) ||
            pAvcPicParams->seq_fields.mb_adaptive_frame_field_flag) &&
            m_fieldScalingInterface->IsFieldScalingSupported(decProcessingParams)) && SfcState.bSfcPipeOut == false)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            CODECHAL_DECODE_ASSERTMESSAGE("Downsampling parameters are NOT supported!");
            return eStatus;
        }
    }
#endif
    m_crrPic = pAvcPicParams->CurrPic;
    m_secondField =
        CodecHal_PictureIsBottomField(pAvcPicParams->CurrPic);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->CurrPic = m_crrPic;
        m_debugInterface->bSecondField = m_secondField;
        m_debugInterface->wFrameType = m_perfType;

        if (pAvcPicParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
                pAvcPicParams));
        }

        if (pAvcIQMatrixParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(
                pAvcIQMatrixParams));
        }

        if (pMvcExtPicParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpMvcExtPicParams(
                pMvcExtPicParams));
        }

        if (pAvcSliceParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSliceParams(
                pAvcSliceParams,
                u32NumSlices));
        }
    )

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::InitPicMhwParams(
    PIC_MHW_PARAMS              *picMhwParams)
{
     MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;
    PMOS_RESOURCE firstValidFrame = &(sDestSurface.OsResource);

    CODECHAL_DECODE_CHK_NULL_RETURN(picMhwParams);

    MOS_ZeroMemory(picMhwParams, sizeof(*picMhwParams));
    
    picMhwParams->PipeModeSelectParams.Mode = CODECHAL_DECODE_MODE_AVCVLD;
    //enable decodestreamout if either app or codechal dump need it
    picMhwParams->PipeModeSelectParams.bStreamOutEnabled     =
        m_decodeParams.m_streamOutEnabled || m_streamOutEnabled;
    picMhwParams->PipeModeSelectParams.bPostDeblockOutEnable = bDeblockingEnabled;
    picMhwParams->PipeModeSelectParams.bPreDeblockOutEnable  = !bDeblockingEnabled;
    picMhwParams->PipeModeSelectParams.bShortFormatInUse     = bShortFormatInUse;

    picMhwParams->SurfaceParams.Mode = CODECHAL_DECODE_MODE_AVCVLD;
    picMhwParams->SurfaceParams.psSurface = &sDestSurface;

    picMhwParams->PipeBufAddrParams.Mode = CODECHAL_DECODE_MODE_AVCVLD;
    if (bDeblockingEnabled)
    {
        picMhwParams->PipeBufAddrParams.psPostDeblockSurface = &sDestSurface;        
    }
    else
    {
        picMhwParams->PipeBufAddrParams.psPreDeblockSurface = &sDestSurface;        
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&picMhwParams->PipeBufAddrParams));

    picMhwParams->PipeBufAddrParams.presMfdIntraRowStoreScratchBuffer            =
        &resMfdIntraRowStoreScratchBuffer;
    picMhwParams->PipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &resMfdDeblockingFilterRowStoreScratchBuffer;

    //Do not support export decode streamout to app buffer and codechal_dump simultenously 
    //which can lead to extra memory copy
    //decode streamout to application
    if (m_decodeParams.m_streamOutEnabled)
    {
        picMhwParams->PipeBufAddrParams.presStreamOutBuffer = m_decodeParams.m_externalStreamOutBuffer;
    }
    //decode streamout to codechal_dump
    else if (m_streamOutEnabled)
    {
        picMhwParams->PipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);

        CODECHAL_DEBUG_TOOL(
            // mark the buffer as in-use
            m_streamOutCurrStatusIdx[m_streamOutCurrBufIdx] = 0;
        )
    }


    MOS_SURFACE dstSurface;
    uint8_t firstValidFrameId = CODEC_AVC_MAX_NUM_REF_FRAME;
    uint8_t activeFrameCnt = 0;
    uint8_t picIdx, frameId, i;
    MOS_ZeroMemory(presReferences, (sizeof(PMOS_RESOURCE)* CODEC_AVC_MAX_NUM_REF_FRAME));
    for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (avcPicIdx[i].bValid)
        {
            activeFrameCnt++;
            picIdx = avcPicIdx[i].ucPicIdx;
            frameId =
                (bPicIdRemappingInUse) ? i : pAvcRefList[picIdx]->ucFrameId;

            presReferences[frameId] =
                &(pAvcRefList[picIdx]->resRefPic);

            CODECHAL_DEBUG_TOOL(
                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
            dstSurface.Format = Format_NV12;
            dstSurface.OsResource = *(presReferences[frameId]);
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                m_osInterface,
                &dstSurface));

            m_debugInterface->wRefIndex = frameId;
            std::string refSurfName = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->wRefIndex));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &dstSurface,
                CodechalDbgAttr::attrReferenceSurfaces,
                refSurfName.data()));
            )

            if (frameId < firstValidFrameId)
            {
                firstValidFrameId = frameId;
                firstValidFrame = presReferences[frameId];
            }
        }
    }

    for (i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        // error concealment for the unset reference addresses
        if (!presReferences[i])
        {
            presReferences[i] = firstValidFrame;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&picMhwParams->PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));

    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(picMhwParams->PipeBufAddrParams.presReferences, sizeof(PMOS_RESOURCE)*CODEC_AVC_MAX_NUM_REF_FRAME,
        presReferences, sizeof(PMOS_RESOURCE)*CODEC_AVC_MAX_NUM_REF_FRAME));

    picMhwParams->IndObjBaseAddrParams.Mode           = CODECHAL_DECODE_MODE_AVCVLD;
    picMhwParams->IndObjBaseAddrParams.presDataBuffer = &resDataBuffer;
    picMhwParams->IndObjBaseAddrParams.dwDataSize     = u32DataSize;
    picMhwParams->IndObjBaseAddrParams.dwDataOffset   = u32DataOffset;
    
    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->SetBitstreamBuffer(&picMhwParams->IndObjBaseAddrParams));
    }

    picMhwParams->BspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &resBsdMpcRowStoreScratchBuffer;
    picMhwParams->BspBufBaseAddrParams.presMprRowStoreScratchBuffer    = &resMprRowStoreScratchBuffer;

    picMhwParams->QmParams.Standard     = CODECHAL_AVC;
    picMhwParams->QmParams.pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)pAvcIQMatrixParams;

    picMhwParams->PicIdParams.bPicIdRemappingInUse = bPicIdRemappingInUse;
    picMhwParams->PicIdParams.pAvcPicIdx           = &(avcPicIdx[0]);

    picMhwParams->ImgParams.pAvcPicParams    = pAvcPicParams;
    picMhwParams->ImgParams.pMvcExtPicParams = pMvcExtPicParams;
    picMhwParams->ImgParams.ucActiveFrameCnt = activeFrameCnt;

    picMhwParams->AvcDirectmodeParams.CurrPic                 = CurrPic;
    picMhwParams->AvcDirectmodeParams.uiUsedForReferenceFlags = pAvcPicParams->UsedForReferenceFlags;
    picMhwParams->AvcDirectmodeParams.presAvcDmvBuffers       = &(resAvcDmvBuffers[0]);
    picMhwParams->AvcDirectmodeParams.ucAvcDmvIdx             = u8AvcMvBufferIndex;
    picMhwParams->AvcDirectmodeParams.pAvcDmvList             = &(avcDmvList[0]);
    picMhwParams->AvcDirectmodeParams.pAvcPicIdx              = &(avcPicIdx[0]);
    picMhwParams->AvcDirectmodeParams.ppAvcRefList            = &(pAvcRefList[0]);
    picMhwParams->AvcDirectmodeParams.bPicIdRemappingInUse    = bPicIdRemappingInUse;
    picMhwParams->AvcDirectmodeParams.presMvcDummyDmvBuffer   = &(resMvcDummyDmvBuffer[
        (pAvcPicParams->seq_fields.direct_8x8_inference_flag) ? 1 : 0]);

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::AddPictureCmds(
    PMOS_COMMAND_BUFFER         cmdBuf,
    PIC_MHW_PARAMS              *picMhwParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuf);
    CODECHAL_DECODE_CHK_NULL_RETURN(picMhwParams);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(cmdBuf, &picMhwParams->PipeModeSelectParams));

#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(SfcState.AddSfcCommands(cmdBuf));
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(cmdBuf, &picMhwParams->SurfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(cmdBuf, &picMhwParams->PipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(cmdBuf, &picMhwParams->IndObjBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(cmdBuf, &picMhwParams->BspBufBaseAddrParams));

    if (bShortFormatInUse)
    {
        MHW_VDBOX_AVC_DPB_PARAMS dpbParams;
        MOS_ZeroMemory(&dpbParams, sizeof(dpbParams));
        dpbParams.pAvcPicParams = pAvcPicParams;
        dpbParams.pMvcExtPicParams = pMvcExtPicParams;
        dpbParams.ppAvcRefList = &(pAvcRefList[0]);
        dpbParams.pAvcPicIdx = &(avcPicIdx[0]);
        dpbParams.bPicIdRemappingInUse = bPicIdRemappingInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdAvcDpbCmd(cmdBuf, &dpbParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdAvcPicidCmd(cmdBuf, &picMhwParams->PicIdParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(cmdBuf, nullptr, &picMhwParams->ImgParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(cmdBuf, &picMhwParams->QmParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcDirectmodeCmd(cmdBuf, &picMhwParams->AvcDirectmodeParams));

    return eStatus;
}


MOS_STATUS CodechalDecodeAvc::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    PIC_MHW_PARAMS picMhwParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicMhwParams(&picMhwParams));

    if (m_cencDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->CheckStatusReportNum(this, &cmdBuffer, m_vdboxIndex));
    }

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureCmds(&cmdBuffer, &picMhwParams));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::ParseSlice(
    PMOS_COMMAND_BUFFER         cmdBuf)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODEC_AVC_SLICE_PARAMS slc = pAvcSliceParams;
    uint16_t frameInMbs = (pAvcPicParams->pic_height_in_mbs_minus1 + 1) * (pAvcPicParams->pic_width_in_mbs_minus1 + 1);

    CODECHAL_DECODE_CHK_NULL_RETURN(pVldSliceRecord);
    CODECHAL_DECODE_CHK_NULL_RETURN(slc);

    // Setup static slice state parameters
    MHW_VDBOX_AVC_SLICE_STATE avcSliceState;
    MOS_ZeroMemory(&avcSliceState, sizeof(avcSliceState));
    avcSliceState.bIntelEntrypointInUse = bIntelEntrypointInUse;
    avcSliceState.bPicIdRemappingInUse = bPicIdRemappingInUse;
    avcSliceState.bShortFormatInUse = bShortFormatInUse;
    avcSliceState.presDataBuffer = &resDataBuffer;
    avcSliceState.pAvcPicParams = pAvcPicParams;
    avcSliceState.pMvcExtPicParams = pMvcExtPicParams;
    avcSliceState.pAvcPicIdx = &(avcPicIdx[0]);
    avcSliceState.ppAvcRefList = &(pAvcRefList[0]);
    avcSliceState.bPhantomSlice = false;
    avcSliceState.dwTotalBytesConsumed = 0;

    uint32_t length = 0, slcCount = 0;
    uint32_t offset = 0;
    uint32_t lastValidSlice = 0;
    bool firstValidSlice = true;
    bool invalidSlicePresent = false;
    for (slcCount = 0; slcCount < u32NumSlices; slcCount++)
    {

        if (invalidSlicePresent == true)
        {
            break;
        }

        if (pVldSliceRecord[slcCount].dwSkip)
        {
            continue;
        }

        length = slc->slice_data_size;

        if (slcCount < u32NumSlices - 1)
        {
            // Skip remaining slices if the number of MBs already reaches the total before the last slice or slice overlap occurs.
            if ((!bShortFormatInUse) &&
                ((slc->first_mb_in_slice + slc->NumMbsForSlice >= frameInMbs) ||
                ((slc + 1)->first_mb_in_slice <= slc->first_mb_in_slice)))
            {
                uint32_t count = slcCount + 1;

                slc->first_mb_in_next_slice = 0;
                invalidSlicePresent = true;

                while (count < u32NumSlices)
                {
                    pVldSliceRecord[count++].dwSkip = true;
                }
            }
            else
            {
                slc->first_mb_in_next_slice = (slc + 1)->first_mb_in_slice;
            }
        }
        else
        {
            slc->first_mb_in_next_slice = 0;
        }


        // error handling for garbage data
        if (((uint64_t)(slc->slice_data_offset) + length) > u32DataSize)
        {
            slc++;
            pVldSliceRecord[slcCount].dwSkip = true;
            continue;
        }            

        if (!bShortFormatInUse)
        {
            offset = (slc->slice_data_bit_offset >> 3) + m_osInterface->dwNumNalUnitBytesIncluded;

            if (offset > length)
            {
                slc++;
                pVldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            // For first slice, first_mb_in_slice must be 0, otherwise it is corrupted
            // Skip slice when  first_mb_in_slice is corrupted.
            if ((0 == slcCount && slc->first_mb_in_slice) ||
                (slc->first_mb_in_slice >= frameInMbs) ||
                (pAvcPicParams->seq_fields.mb_adaptive_frame_field_flag &&
                    !pAvcPicParams->pic_fields.field_pic_flag &&
                    (slc->first_mb_in_slice >= frameInMbs / 2)))
            {
                slc++;
                pVldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            if (firstValidSlice && slc->first_mb_in_slice)
            {

                uint16_t usStartMbNum, usNextStartMbNum;

                // ensure that slc->first_mb_in_next_slice is always non-zero for this phantom slice

                usNextStartMbNum = slc->first_mb_in_next_slice;
                usStartMbNum = slc->first_mb_in_slice;
                slc->first_mb_in_slice = 0;
                slc->first_mb_in_next_slice = usStartMbNum;

                avcSliceState.pAvcSliceParams = slc;
                avcSliceState.dwOffset = 0;
                avcSliceState.dwLength = slc->slice_data_offset;
                avcSliceState.dwNextOffset = slc->slice_data_offset;
                avcSliceState.dwNextLength = slc->slice_data_size;

                CODECHAL_DECODE_CHK_STATUS_RETURN(SendSlice(&avcSliceState, cmdBuf));

                slc->first_mb_in_slice = usStartMbNum;
                slc->first_mb_in_next_slice = usNextStartMbNum;
            }
        }

        firstValidSlice = false;
        lastValidSlice = slcCount;
        length -= offset;
        pVldSliceRecord[slcCount].dwLength = length;
        pVldSliceRecord[slcCount].dwOffset = offset;
        slc++;
    }
    slc = pAvcSliceParams;

    uint32_t skippedSlc = 0;
    for (slcCount = 0; slcCount < u32NumSlices; slcCount++)
    {
        if (pVldSliceRecord[slcCount].dwSkip)
        {
            //For DECE clear bytes calculation: Total bytes in the bit-stream consumed so far
            avcSliceState.dwTotalBytesConsumed = slc->slice_data_offset + slc->slice_data_size;

            slc++;
            skippedSlc++;
            continue;
        }

        if (slcCount < lastValidSlice)
        {
            offset = (slc + 1)->slice_data_offset;
            length = (slc + 1)->slice_data_size;
        }
        avcSliceState.pAvcSliceParams = slc;
        avcSliceState.dwOffset = pVldSliceRecord[slcCount].dwOffset;
        avcSliceState.dwLength = pVldSliceRecord[slcCount].dwLength;
        avcSliceState.dwNextOffset = offset;
        avcSliceState.dwNextLength = length;
        avcSliceState.dwSliceIndex = slcCount;
        avcSliceState.bLastSlice = (slcCount == lastValidSlice);

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendSlice(&avcSliceState, cmdBuf));

        //For DECE clear bytes calculation: Total bytes in the bit-stream consumed so far
        avcSliceState.dwTotalBytesConsumed = slc->slice_data_offset + slc->slice_data_size;

        slc++;
    }

    MOS_ZeroMemory(pVldSliceRecord, (u32NumSlices * sizeof(CODECHAL_VLD_SLICE_RECORD)));

    return eStatus;
}


MOS_STATUS CodechalDecodeAvc::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(pAvcPicParams);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_cencDecoder)
    {
        // Pass the correct 2nd level batch buffer index set in CencDecode()
        uint8_t sliceBatchBufferIdx;
        sliceBatchBufferIdx = pAvcRefList[CurrPic.FrameIdx]->ucCencBufIdx[bSecondField];

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_cencDecoder->SetBatchBufferForDecode(m_debugInterface, sliceBatchBufferIdx, &cmdBuffer));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseSlice(&cmdBuffer));
    }

    // Check if destination surface needs to be synchronized
    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &sDestSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    if (!CodecHal_PictureIsField(pAvcPicParams->CurrPic) ||
        !bSecondField)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

        // Update the resource tag (s/w tag) for On-Demand Sync
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
    }

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync &&
        (!CodecHal_PictureIsField(pAvcPicParams->CurrPic) || bSecondField))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic = pAvcPicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic = pAvcPicParams->CurrPic;
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes = pAvcRefList[pAvcPicParams->CurrPic.FrameIdx]->resRefPic;

        CODECHAL_DEBUG_TOOL
        (
            if (m_streamOutEnabled)
            {
                // add current streamout buffer to the report and move onto the next one
                decodeStatusReport.m_streamOutBuf = &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
                decodeStatusReport.m_streamoutIdx = m_streamOutCurrBufIdx;
                if (++m_streamOutCurrBufIdx >= CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS)
                {
                    m_streamOutCurrBufIdx = 0;
                }
                // check next buffer in the list is free.
                if (m_streamOutCurrStatusIdx[m_streamOutCurrBufIdx] != CODECHAL_DECODE_STATUS_NUM)
                {
                    // We've run out of buffers. Temporarily lock the next one down to force a wait. Then mark it as free.
                    CodechalResLock ResourceLock(m_osInterface, &(m_streamOutBuffer[m_streamOutCurrBufIdx]));
                    ResourceLock.Lock(CodechalResLock::readOnly);

                    m_streamOutCurrStatusIdx[m_streamOutCurrBufIdx] = CODECHAL_DECODE_STATUS_NUM;
                }
            }

            decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(pAvcPicParams->CurrPic);
            decodeStatusReport.m_frameType = m_perfType;
        )

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    bool syncCompleteFrame = (pAvcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono && !m_hwInterface->m_noHuC);
    if (syncCompleteFrame)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));

    //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&sDestSurface);        
    )

#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DECODE_PROCESSING_PARAMS *decProcessingParams = m_decodeParams.m_procParams;
    if (decProcessingParams != nullptr && !SfcState.bSfcPipeOut && (bSecondField || pAvcPicParams->seq_fields.mb_adaptive_frame_field_flag))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_fieldScalingInterface->DoFieldScaling(
            decProcessingParams,
            m_renderContext,
            m_disableDecodeSyncLock,
            m_disableLockForTranscode));
    }
    else
#endif
    {
        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    pAvcRefList[pAvcPicParams->CurrPic.FrameIdx]->resRefPic =
        sDestSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!CodecHal_PictureIsField(pAvcPicParams->CurrPic) || bSecondField)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &sDestSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
#ifdef _DECODE_PROCESSING_SUPPORTED
        if (decProcessingParams && !SfcState.bSfcPipeOut)
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &decProcessingParams->pOutputSurface->OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
#endif
    }
#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        // Dump out downsampling result
        if (decProcessingParams && decProcessingParams->pOutputSurface)
        {
            MOS_SURFACE dstSurface;
            MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
            dstSurface.Format = Format_NV12;
            dstSurface.OsResource = decProcessingParams->pOutputSurface->OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                m_osInterface,
                &dstSurface));

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &dstSurface,
                CodechalDbgAttr::attrSfcOutputSurface,
                "SfcDstSurf"));
        }
    )
#endif
    return eStatus;
}

MOS_STATUS CodechalDecodeAvc::CalcDownsamplingParams(
    void                        *picParams,
    uint32_t                    *refSurfWidth,
    uint32_t                    *refSurfHeight,
    MOS_FORMAT                  *format,
    uint8_t                     *frameIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(picParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(refSurfWidth);
    CODECHAL_DECODE_CHK_NULL_RETURN(refSurfHeight);
    CODECHAL_DECODE_CHK_NULL_RETURN(format);
    CODECHAL_DECODE_CHK_NULL_RETURN(frameIdx);

    PCODEC_AVC_PIC_PARAMS avcPicParams = (PCODEC_AVC_PIC_PARAMS)picParams;

    *refSurfWidth = 0;
    *refSurfHeight = 0;
    *format = Format_NV12;
    *frameIdx = avcPicParams->CurrPic.FrameIdx;

    if (m_refSurfaces == nullptr)
    {
        *refSurfWidth = (avcPicParams->pic_width_in_mbs_minus1 + 1) * CODECHAL_MACROBLOCK_WIDTH;
        *refSurfHeight = (avcPicParams->pic_height_in_mbs_minus1 + 1) * CODECHAL_MACROBLOCK_HEIGHT;
    }

    return eStatus;
}

CodechalDecodeAvc::CodechalDecodeAvc(
    CodechalHwInterface *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecode(hwInterface, debugInterface, standardInfo)
{
    m_mmc = nullptr;

    // Parameters passed by application
    u16PicWidthInMb = 0;
    u16PicHeightInMb = 0;
    u16PicWidthInMbLastMaxAlloced = 0;
    u16PicHeightInMbLastMaxAlloced = 0;
    bIntelEntrypointInUse = false;
    bShortFormatInUse = false;
    bPicIdRemappingInUse = false;
    u32DataSize = 0;
    u32DataOffset = 0;
    u32NumSlices = 0;
    u32RefSurfaceNum = 0;
    pAvcPicParams = nullptr;
    pMvcExtPicParams = nullptr;
    pAvcSliceParams = nullptr;
    pAvcIQMatrixParams = nullptr;

    MOS_ZeroMemory(presReferences, (sizeof(PMOS_RESOURCE)* CODEC_AVC_MAX_NUM_REF_FRAME));
    MOS_ZeroMemory(&resDataBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resMonoPictureChromaBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resMfdIntraRowStoreScratchBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resMfdDeblockingFilterRowStoreScratchBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resBsdMpcRowStoreScratchBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resMprRowStoreScratchBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resAvcDmvBuffers, (sizeof(MOS_RESOURCE)* CODEC_AVC_NUM_DMV_BUFFERS));
    MOS_ZeroMemory(&resInvalidRefBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&resMvcDummyDmvBuffer, (sizeof(MOS_RESOURCE) * 2));
    pRefFrameSurface = nullptr;

    pVldSliceRecord = nullptr;

    u16BsdMpcRowStoreScratchBufferPicWidthInMb = 0;
    u16MfdIntraRowStoreScratchBufferPicWidthInMb = 0;
    u16MprRowStoreScratchBufferPicWidthInMb = 0;

    MOS_ZeroMemory(u8FirstFieldIdxList, (sizeof(uint8_t)* CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS));

    bSecondField = false;
    bDeblockingEnabled = false;

    // Decode process usage
    MOS_ZeroMemory(&CurrPic, sizeof(CODEC_PICTURE));

    MOS_ZeroMemory(&avcFrameStoreID, (sizeof(CODEC_AVC_FRAME_STORE_ID)* CODEC_AVC_MAX_NUM_REF_FRAME));

    u8AvcMvBufferIndex = 0;
    MOS_ZeroMemory(&avcDmvList, (sizeof(CODEC_AVC_DMV_LIST)* CODEC_AVC_NUM_DMV_BUFFERS));

    MOS_ZeroMemory(&avcPicIdx, (sizeof(CODEC_PIC_ID)* CODEC_AVC_MAX_NUM_REF_FRAME));
    MOS_ZeroMemory(pAvcRefList, (sizeof(PCODEC_REF_LIST)* CODEC_AVC_NUM_UNCOMPRESSED_SURFACE));

    u32AvcDmvBufferSize = 0;

    //Currently, crc calculation is only supported in AVC decoder
    m_reportFrameCrc = true;

};

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeAvc::DumpMvcExtPicParams(
    PCODEC_MVC_EXT_PIC_PARAMS mvcExtPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrMvcExtPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(mvcExtPicParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "CurrViewID: " << std::hex << +mvcExtPicParams->CurrViewID << std::endl;
    oss << "anchor_pic_flag: " << +mvcExtPicParams->anchor_pic_flag << std::endl;
    oss << "inter_view_flag: " << +mvcExtPicParams->inter_view_flag << std::endl;
    oss << "NumInterViewRefsL0: " << +mvcExtPicParams->NumInterViewRefsL0 << std::endl;
    oss << "NumInterViewRefsL1: " << +mvcExtPicParams->NumInterViewRefsL1 << std::endl;
    oss << "bPicFlags: " << +mvcExtPicParams->bPicFlags << std::endl;
    oss << "SwitchToAVC: " << +mvcExtPicParams->SwitchToAVC << std::endl;
    oss << "Reserved7Bits: " << +mvcExtPicParams->Reserved7Bits << std::endl;
    oss << "Reserved8Bits: " << +mvcExtPicParams->Reserved8Bits << std::endl;

    //Dump ViewIDList[16]
    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "ViewIDList[" << +i << "]: "
            << +mvcExtPicParams->ViewIDList[i] << std::endl;
    }

    //Dump InterViewRefList[2][16]
    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "InterViewRefList[0][" << +i << "]: "
            << +mvcExtPicParams->InterViewRefList[0][i] << std::endl;
        oss << "InterViewRefList[1][" << +i << "]: "
            << +mvcExtPicParams->InterViewRefList[1][i] << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufMvcPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeAvc::DumpPicParams(
    PCODEC_AVC_PIC_PARAMS picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "CurrPic FrameIdx: " << std::dec << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << std::hex << +picParams->CurrPic.PicFlags << std::endl;

    //Dump RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << std::dec << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << std::hex << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    oss << "pic_width_in_mbs_minus1: " << std::dec << +picParams->pic_width_in_mbs_minus1 << std::endl;
    oss << "pic_height_in_mbs_minus1: " << +picParams->pic_height_in_mbs_minus1 << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "num_ref_frames: " << +picParams->num_ref_frames << std::endl;
    oss << "CurrFieldOrderCnt: " << +picParams->CurrFieldOrderCnt[0] << std::endl;
    oss << "CurrFieldOrderCnt: " << +picParams->CurrFieldOrderCnt[1] << std::endl;

    //Dump FieldOrderCntList (16x2)
    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "FieldOrderCntList[" << +i << "]:";
        for (uint8_t j = 0; j < 16; j++)
            oss << +picParams->FieldOrderCntList[j][i] << " ";
        oss << std::endl;
    }

    //Dump seq_fields
    oss << "seq_fields value: " << +picParams->seq_fields.value << std::endl;
    oss << "chroma_format_idc: " << +picParams->seq_fields.chroma_format_idc << std::endl;
    oss << "residual_colour_transform_flag: " << std::hex << +picParams->seq_fields.residual_colour_transform_flag << std::endl;
    oss << "frame_mbs_only_flag: " << std::hex << +picParams->seq_fields.frame_mbs_only_flag << std::endl;
    oss << "mb_adaptive_frame_field_flag: " << std::hex << +picParams->seq_fields.mb_adaptive_frame_field_flag << std::endl;
    oss << "direct_8x8_inference_flag: " << std::hex << +picParams->seq_fields.direct_8x8_inference_flag << std::endl;
    oss << "log2_max_frame_num_minus4: " << std::dec << +picParams->seq_fields.log2_max_frame_num_minus4 << std::endl;
    oss << "pic_order_cnt_type: " << +picParams->seq_fields.pic_order_cnt_type << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "delta_pic_order_always_zero_flag: " << std::hex << +picParams->seq_fields.delta_pic_order_always_zero_flag << std::endl;
    oss << "num_slice_groups_minus1:" << std::dec << +picParams->num_slice_groups_minus1 << std::endl;
    oss << "slice_group_map_type:" << std::dec << +picParams->slice_group_map_type << std::endl;
    oss << "slice_group_change_rate_minus1:" << std::dec << +picParams->slice_group_change_rate_minus1 << std::endl;
    oss << "pic_init_qp_minus26:" << std::dec << +picParams->pic_init_qp_minus26 << std::endl;
    oss << "chroma_qp_index_offset:" << std::dec << +picParams->chroma_qp_index_offset << std::endl;
    oss << "second_chroma_qp_index_offset:" << std::dec << +picParams->second_chroma_qp_index_offset << std::endl;

    //Dump pic_fields
    oss << "pic_fields value: " << std::dec << +picParams->pic_fields.value << std::endl;
    oss << "entropy_coding_mode_flag: " << std::hex << +picParams->pic_fields.entropy_coding_mode_flag << std::endl;
    oss << "weighted_pred_flag: " << std::hex << +picParams->pic_fields.weighted_pred_flag << std::endl;
    oss << "weighted_bipred_idc: " << std::dec << +picParams->pic_fields.weighted_bipred_idc << std::endl;
    oss << "transform_8x8_mode_flag: " << std::hex << +picParams->pic_fields.transform_8x8_mode_flag << std::endl;
    oss << "field_pic_flag: " << std::hex << +picParams->pic_fields.field_pic_flag << std::endl;
    oss << "constrained_intra_pred_flag: " << std::hex << +picParams->pic_fields.constrained_intra_pred_flag << std::endl;
    oss << "pic_order_present_flag: " << std::hex << +picParams->pic_fields.pic_order_present_flag << std::endl;
    oss << "deblocking_filter_control_present_flag: " << std::hex << +picParams->pic_fields.deblocking_filter_control_present_flag << std::endl;
    oss << "redundant_pic_cnt_present_flag: " << std::hex << +picParams->pic_fields.redundant_pic_cnt_present_flag << std::endl;
    oss << "reference_pic_flag: " << std::hex << +picParams->pic_fields.reference_pic_flag << std::endl;
    oss << "IntraPicFlag: " << std::hex << +picParams->pic_fields.IntraPicFlag << std::endl;

    //Dump Short format specific
    oss << "num_ref_idx_l0_active_minus1: " << std::dec << +picParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1: " << std::dec << +picParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "NonExistingFrameFlags: " << std::hex << +picParams->NonExistingFrameFlags << std::endl;
    oss << "UsedForReferenceFlags: " << std::hex << +picParams->UsedForReferenceFlags << std::endl;
    oss << "frame_num: " << std::dec << +picParams->frame_num << std::endl;
    oss << "StatusReportFeedbackNumber: " << std::dec << +picParams->StatusReportFeedbackNumber << std::endl;

    //Dump FrameNumList[16]
    oss << "scaling_list_present_flag_buffer:";
    for (uint8_t i = 0; i < 16; i++)
        oss << std::hex << picParams->FrameNumList[i];
    oss << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeAvc::DumpSliceParams(
    PCODEC_AVC_SLICE_PARAMS sliceParams,
    uint32_t                numSlices)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    PCODEC_AVC_SLICE_PARAMS sliceControl = nullptr;

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint16_t j = 0; j < numSlices; j++)
    {
        sliceControl = &sliceParams[j];

        oss << "Data for Slice number = " << std::dec << +j << std::endl;
        oss << "slice_data_size: " << std::dec << +sliceControl->slice_data_size << std::endl;
        oss << "slice_data_offset: " << std::dec << +sliceControl->slice_data_offset << std::endl;
        //Dump Long format specific
        oss << "slice_data_bit_offset: " << std::dec << +sliceControl->slice_data_bit_offset << std::endl;
        oss << "first_mb_in_slice: " << std::dec << +sliceControl->first_mb_in_slice << std::endl;
        oss << "NumMbsForSlice: " << std::dec << +sliceControl->NumMbsForSlice << std::endl;
        oss << "slice_type: " << std::dec << +sliceControl->slice_type << std::endl;
        oss << "direct_spatial_mv_pred_flag: " << std::hex << +sliceControl->direct_spatial_mv_pred_flag << std::endl;
        oss << "num_ref_idx_l0_active_minus1: " << std::dec << +sliceControl->num_ref_idx_l0_active_minus1 << std::endl;
        oss << "num_ref_idx_l1_active_minus1: " << std::dec << +sliceControl->num_ref_idx_l1_active_minus1 << std::endl;
        oss << "cabac_init_idc: " << std::dec << +sliceControl->cabac_init_idc << std::endl;
        oss << "slice_qp_delta: " << std::dec << +sliceControl->slice_qp_delta << std::endl;
        oss << "disable_deblocking_filter_idc: " << std::dec << +sliceControl->disable_deblocking_filter_idc << std::endl;
        oss << "slice_alpha_c0_offset_div2: " << std::dec << +sliceControl->slice_alpha_c0_offset_div2 << std::endl;
        oss << "slice_beta_offset_div2: " << std::dec << +sliceControl->slice_beta_offset_div2 << std::endl;

        //Dump RefPicList[2][32]
        for (uint8_t i = 0; i < 32; ++i)
        {
            oss << "RefPicList[0][" << std::dec << +i << "] FrameIdx: " << std::dec << +sliceControl->RefPicList[0][i].FrameIdx << std::endl;
            oss << "RefPicList[0][" << std::dec << +i << "] PicFlags: " << std::hex << +sliceControl->RefPicList[0][i].PicFlags << std::endl;
            oss << "RefPicList[1][" << std::dec << +i << "] FrameIdx: " << std::dec << +sliceControl->RefPicList[1][i].FrameIdx << std::endl;
            oss << "RefPicList[1][" << std::dec << +i << "] PicFlags: " << std::hex << +sliceControl->RefPicList[1][i].PicFlags << std::endl;
        }

        oss << "luma_log2_weight_denom: " << std::dec << +sliceControl->luma_log2_weight_denom << std::endl;
        oss << "chroma_log2_weight_denom: " << std::dec << +sliceControl->chroma_log2_weight_denom << std::endl;
        oss << "slice_id: " << std::dec << +sliceControl->slice_id << std::endl;

        //Dump Weights[2][32][3][2]
        for (uint8_t i = 0; i < 32; ++i)
        {
            oss << "Weights[0][" << std::dec << +i << "][0][0]: " << std::hex << +sliceControl->Weights[0][i][0][0] << std::endl;
            oss << "Weights[0][" << std::dec << +i << "][0][1]: " << std::hex << +sliceControl->Weights[0][i][0][1] << std::endl;
            oss << "Weights[0][" << std::dec << +i << "][1][0]: " << std::hex << +sliceControl->Weights[0][i][1][0] << std::endl;
            oss << "Weights[0][" << std::dec << +i << "][1][1]: " << std::hex << +sliceControl->Weights[0][i][1][1] << std::endl;
            oss << "Weights[1][" << std::dec << +i << "][0][0]: " << std::hex << +sliceControl->Weights[1][i][0][0] << std::endl;
            oss << "Weights[1][" << std::dec << +i << "][0][1]: " << std::hex << +sliceControl->Weights[1][i][0][1] << std::endl;
            oss << "Weights[1][" << std::dec << +i << "][1][0]: " << std::hex << +sliceControl->Weights[1][i][1][0] << std::endl;
            oss << "Weights[1][" << std::dec << +i << "][1][1]: " << std::hex << +sliceControl->Weights[1][i][1][1] << std::endl;
            oss << "Weights[0][" << std::dec << +i << "][2][0]: " << std::hex << +sliceControl->Weights[0][i][2][0] << std::endl;
            oss << "Weights[0][" << std::dec << +i << "][2][1]: " << std::hex << +sliceControl->Weights[0][i][2][1] << std::endl;
            oss << "Weights[1][" << std::dec << +i << "][2][0]: " << std::hex << +sliceControl->Weights[1][i][2][0] << std::endl;
            oss << "Weights[1][" << std::dec << +i << "][2][1]: " << std::hex << +sliceControl->Weights[1][i][2][1] << std::endl;
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

MOS_STATUS CodechalDecodeAvc::DumpIQParams(
    PCODEC_AVC_IQ_MATRIX_PARAMS matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    uint32_t idx, idx2;
    // 4x4 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "Qmatrix_H264_ScalingLists4x4[" << std::dec << +idx2 << "]:" << std::endl;
        for (idx = 0; idx < 12; idx += 4)
        {
            oss << "ScalingList4x4[" << std::dec << +idx / 4 << "]:";
            oss << std::hex << +matrixData->ScalingList4x4[idx2][idx] << " ";
            oss << std::hex << +matrixData->ScalingList4x4[idx2][idx + 1] << " ";
            oss << std::hex << +matrixData->ScalingList4x4[idx2][idx + 2] << " ";
            oss << std::hex << +matrixData->ScalingList4x4[idx2][idx + 3] << " ";
            oss << std::endl;
        }
        oss << std::endl;
    }
    // 8x8 block
    for (idx2 = 0; idx2 < 2; idx2++)
    {
        oss << "Qmatrix_H264_ScalingLists8x8[" << std::dec << +idx2 << "]:" << std::endl;
        for (idx = 0; idx < 56; idx += 8)
        {
            oss << "ScalingList8x8[" << std::dec << +idx / 8 << "]:";
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 1] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 2] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 3] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 4] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 5] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 6] << " " ;
            oss << std::hex << +matrixData->ScalingList8x8[idx2][idx + 7] << " " ;
            oss << std::endl;
        }
        oss << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    
    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS CodechalDecodeAvc::SetFrameStoreIds(uint8_t frameIdx)
{

    CODECHAL_DECODE_CHK_NULL_RETURN(avcFrameStoreID);
    CODECHAL_DECODE_CHK_NULL_RETURN(pAvcRefList);

    uint8_t invalidFrame = 0x7f;

    for (uint8_t i = 0; i < pAvcRefList[frameIdx]->ucNumRef; i++)
    {
        uint8_t index;
        index = pAvcRefList[frameIdx]->RefList[i].FrameIdx;
        if (pAvcRefList[index]->ucFrameId == invalidFrame)
        {
            uint8_t j;
            for (j = 0; j < CODEC_AVC_MAX_NUM_REF_FRAME; j++)
            {
                if (!avcFrameStoreID[j].inUse)
                {
                    pAvcRefList[index]->ucFrameId = j;
                    avcFrameStoreID[j].inUse = true;
                    break;
                }
            }
            if (j == CODEC_AVC_MAX_NUM_REF_FRAME)
            {
                // should never happen, something must be wrong
                CODECHAL_PUBLIC_ASSERT(false);
                pAvcRefList[index]->ucFrameId = 0;
                avcFrameStoreID[0].inUse = true;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}
