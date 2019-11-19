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
//! \file     codechal_encode_wp_mdf_g12.cpp
//! \brief    This file implements the wp init feature for all codecs on Gen12 platform
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_wp_mdf_g12.h"
#include "codeckrnheader.h"
#include "Gen12LP_WeightedPrediction_genx.h"

MOS_STATUS CodechalEncodeWPMdfG12::InitKernelStateIsa(void *kernelIsa, uint32_t kernelIsaSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (!m_cmProgram)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->LoadProgram(kernelIsa,
            kernelIsaSize,
            m_cmProgram,
            "-nojitter"));
    }
    for (uint8_t i = 0; i < CODEC_NUM_WP_FRAME; i++)
    {
        if (m_cmKrn[i] == nullptr)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateKernel(m_cmProgram,
                "Scale_frame",
                m_cmKrn[i]));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeWPMdfG12::SetCurbe(CurbeData& curbe)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_ZeroMemory(&curbe, sizeof(CurbeData));

    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    //C Model hard code log2WeightDenom = 6. No need to send WD paramters to WP Kernel.
    curbe.DW0.defaultWeight = m_curbeParams.slcParams->weights[m_curbeParams.refPicListIdx][m_curbeParams.wpIdx][0][0];
    curbe.DW0.defaultOffset = m_curbeParams.slcParams->weights[m_curbeParams.refPicListIdx][m_curbeParams.wpIdx][0][1];

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    WP init kernel function
//!
//! \param    [in] params
//!           Pointer to KernelParams
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalEncodeWPMdfG12::Execute(KernelParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    if (params->slcWPParams && params->slcWPParams->luma_log2_weight_denom != 6)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_ASSERTMESSAGE("Weighted Prediction Kernel does not support Log2LumaWeightDenom != 6!");
        return eStatus;
    }

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_WP_KERNEL);

    // Allocate output surface
    if (params->useRefPicList1)
    {
        *(params->useWeightedSurfaceForL1) = true;
        m_surfaceParams.wpOutListIdx = CODEC_WP_OUTPUT_L1_START + params->wpIndex;
    }
    else
    {
        *(params->useWeightedSurfaceForL0) = true;
        m_surfaceParams.wpOutListIdx = CODEC_WP_OUTPUT_L0_START + params->wpIndex;
    }
    if (m_surfaceParams.wpOutListIdx >= CODEC_NUM_WP_FRAME)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_ASSERTMESSAGE("index exceeds maximum value of array weightedPredOutputPicList.");
        return eStatus;
    }
    uint8_t wpKrnIdx = m_surfaceParams.wpOutListIdx;
    CmKernel* cmKrn = m_cmKrn[wpKrnIdx];

    // Setup Curbe
    m_curbeParams.refPicListIdx = (params->useRefPicList1) ? LIST_1 : LIST_0;
    m_curbeParams.wpIdx = params->wpIndex;
    m_curbeParams.slcParams = params->slcWPParams;

    //Set Surface States
    m_surfaceParams.refFrameInput = params->refFrameInput;
    m_surfaceParams.refIsBottomField = params->refIsBottomField;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupSurfaces(wpKrnIdx));

    uint32_t ResolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth);
    uint32_t ResolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);

    uint32_t threadCount = ResolutionX * ResolutionY;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->SetThreadCount(threadCount));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->CreateThreadSpace(
        ResolutionX,
        ResolutionY,
        m_threadSpace));

    if (m_groupIdSelectSupported)
    {
        m_threadSpace->SetMediaWalkerGroupSelect((CM_MW_GROUP_SELECT)m_groupId);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(cmKrn->AssociateThreadSpace(m_threadSpace));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupKernelArgs(wpKrnIdx));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->AddKernel(cmKrn));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CmEvent * event = CM_NO_EVENT;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmQueue->EnqueueFast(m_encoder->m_cmTask, event));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmTask->Reset());
        m_lastTaskInPhase = false;
    }
    else
    {
        m_encoder->m_cmTask->AddSync();
    }

    if (params->useRefPicList1 == false)
    {
        // Dump all the surfaces for debugging
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_surfaceParams.refFrameInput,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "WP_input_Surface_L0")));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx],
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_output_Surface_L0")));
    }
    else
    {
        // Dump all the surfaces for debugging
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_surfaceParams.refFrameInput,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "WP_input_Surface_L1")));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx],
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_output_Surface_L1")));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeWPMdfG12::SetupKernelArgs(uint8_t wpKrnIdx)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int idx = 0;
    CurbeData curbe;
    SurfaceIndex * pSurfIndex = nullptr;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbe(curbe));
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_wpInputSurface[wpKrnIdx]);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_wpOutputSurface[wpKrnIdx]);

    // SetKernelArg will copy curbe data
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn[wpKrnIdx]->SetKernelArg(idx++, sizeof(curbe), &curbe));
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpMDFCurbe(
            CODECHAL_MEDIA_STATE_ENC_WP,
            (uint8_t *)&curbe,
            sizeof(curbe)));)

    m_wpInputSurface[wpKrnIdx]->GetIndex(pSurfIndex);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn[wpKrnIdx]->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex));

    m_wpOutputSurface[wpKrnIdx]->GetIndex(pSurfIndex);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cmKrn[wpKrnIdx]->SetKernelArg(idx++, sizeof(SurfaceIndex), pSurfIndex));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeWPMdfG12::SetupSurfaces(uint8_t wpKrnIdx)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->UpdateSurface2D(
        &m_surfaceParams.refFrameInput->OsResource,
        m_wpInputSurface[wpKrnIdx]));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->UpdateSurface2D(
        &m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx].OsResource,
        m_wpOutputSurface[wpKrnIdx]));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeWPMdfG12::ReleaseResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    for (uint8_t i = 0; i < CODEC_NUM_WP_FRAME; i++)
    {
        if (m_wpInputSurface[i])
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroySurface(m_wpInputSurface[i]));
            m_wpInputSurface[i] = nullptr;
        }

        if (m_wpOutputSurface[i])
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->m_cmDev->DestroySurface(m_wpOutputSurface[i]));
            m_wpOutputSurface[i] = nullptr;
        }
    }

    return MOS_STATUS_SUCCESS;
}
