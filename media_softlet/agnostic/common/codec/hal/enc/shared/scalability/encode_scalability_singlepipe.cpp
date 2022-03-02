/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_scalability_singlepipe.cpp
//! \brief    Defines the common interface for media scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "codechal_hw.h"
#include "encode_scalability_defs.h"
#include "encode_scalability_singlepipe.h"

#include "media_context.h"
#include "media_status_report.h"
#include "mhw_utilities.h"
#include "encode_status_report_defs.h"
#include "hal_oca_interface.h"

namespace encode
{

EncodeScalabilitySinglePipe::EncodeScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType) :
    MediaScalabilitySinglePipe(hwInterface, mediaContext, componentType)
{
    if (hwInterface == nullptr)
    {
        return;
    }
    m_hwInterface = (CodechalHwInterface *)hwInterface;
    m_osInterface = m_hwInterface->GetOsInterface();
}

MOS_STATUS EncodeScalabilitySinglePipe::Initialize(const MediaScalabilityOption &option)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);

    m_scalabilityOption = MOS_New(EncodeScalabilityOption, (const EncodeScalabilityOption &)option);
    SCALABILITY_CHK_NULL_RETURN(m_scalabilityOption);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    auto statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_FRAME_TRACKING_ID,
        &userFeatureData,
        m_osInterface->pOsContext);
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        m_frameTrackingEnabled = userFeatureData.i32Data ? true : false;
    }
    else
    {
        m_frameTrackingEnabled = m_osInterface->bEnableKmdMediaFrameTracking ? true : false;
    }

    // !Don't check the return status here, because this function will return fail if there's no regist key in register.
    // But it's normal that regist key not in register.
    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);
    m_miInterface = m_hwInterface->GetMiInterface();
    SCALABILITY_CHK_NULL_RETURN(m_miInterface);

    SCALABILITY_CHK_STATUS_RETURN(MediaScalabilitySinglePipe::Initialize(option));

    PMOS_GPUCTX_CREATOPTIONS_ENHANCED gpuCtxCreateOption = 
                    dynamic_cast<PMOS_GPUCTX_CREATOPTIONS_ENHANCED>(m_gpuCtxCreateOption);
    EncodeScalabilityOption *scalabilityOption  = 
                    dynamic_cast<EncodeScalabilityOption *>(m_scalabilityOption);
    if(gpuCtxCreateOption != nullptr &&
       scalabilityOption != nullptr &&
       scalabilityOption->IsVdencEnabled())
    {
        gpuCtxCreateOption->Flags |=  (1 << 2);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeScalabilitySinglePipe::VerifyCmdBuffer(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    SCALABILITY_FUNCTION_ENTER;

    return MediaScalabilitySinglePipe::VerifyCmdBuffer(requestedSize, requestedPatchListSize, singleTaskPhaseSupportedInPak);
}

MOS_STATUS EncodeScalabilitySinglePipe::VerifySpaceAvailable(uint32_t requestedSize, uint32_t requestedPatchListSize, bool &singleTaskPhaseSupportedInPak)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;;

    bool bothPatchListAndCmdBufChkSuccess = false;
    uint8_t looptimes = m_singleTaskPhaseSupported ? 2 : 1;

    for(auto i = 0 ; i < looptimes ; i++)
    {
        SCALABILITY_CHK_STATUS_RETURN(MediaScalability::VerifySpaceAvailable(
            requestedSize, requestedPatchListSize, bothPatchListAndCmdBufChkSuccess));
        
        if (bothPatchListAndCmdBufChkSuccess == true)
        {
            singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
            return eStatus;
        }
        
        MOS_STATUS statusPatchList = MOS_STATUS_SUCCESS;
        MOS_STATUS statusCmdBuf    = MOS_STATUS_SUCCESS;
        
        if (requestedPatchListSize > 0)
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
            singleTaskPhaseSupportedInPak = m_singleTaskPhaseSupported;
            return eStatus;
        }
    }

    eStatus = MOS_STATUS_NO_SPACE;
    SCALABILITY_ASSERTMESSAGE("Resize Command buffer failed with no space!");
    return eStatus;

}

MOS_STATUS EncodeScalabilitySinglePipe::UpdateState(void *statePars)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_STATUS_RETURN(MediaScalabilitySinglePipe::UpdateState(statePars));

    MOS_STATUS   eStatus         = MOS_STATUS_SUCCESS;

    StateParams *encodeStatePars = (StateParams *)statePars;
    m_singleTaskPhaseSupported   = encodeStatePars->singleTaskPhaseSupported;
    m_statusReport               = encodeStatePars->statusReport;
    m_currentPass                = encodeStatePars->currentPass;
    m_currentRow                 = encodeStatePars->currentRow;
    m_currentSubPass             = encodeStatePars->currentSubPass;
    SCALABILITY_CHK_NULL_RETURN(m_statusReport);

    return eStatus;
}

MOS_STATUS EncodeScalabilitySinglePipe::ResizeCommandBufferAndPatchList(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    SCALABILITY_FUNCTION_ENTER;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);

    return m_hwInterface->ResizeCommandBufferAndPatchList(requestedCommandBufferSize, requestedPatchListSize);
}

MOS_STATUS EncodeScalabilitySinglePipe::SendAttrWithFrameTracking(
    MOS_COMMAND_BUFFER &cmdBuffer,
    bool                frameTrackingRequested)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    SCALABILITY_FUNCTION_ENTER;

    bool renderEngineUsed = m_mediaContext->IsRenderEngineUsed();

    // initialize command buffer attributes
    cmdBuffer.Attributes.bTurboMode               = m_hwInterface->m_turboMode;
    cmdBuffer.Attributes.bMediaPreemptionEnabled  = renderEngineUsed ? m_hwInterface->GetRenderInterface()->IsPreemptionEnabled() : 0;
    cmdBuffer.Attributes.dwNumRequestedEUSlices   = m_hwInterface->m_numRequestedEuSlices;
    cmdBuffer.Attributes.dwNumRequestedSubSlices  = m_hwInterface->m_numRequestedSubSlices;
    cmdBuffer.Attributes.dwNumRequestedEUs        = m_hwInterface->m_numRequestedEus;
    cmdBuffer.Attributes.bValidPowerGatingRequest = true;

    PMOS_RESOURCE resource = nullptr;
    uint32_t      offset   = 0;

    if (frameTrackingRequested && m_frameTrackingEnabled)
    {
        m_statusReport->GetAddress(encode::statusReportGlobalCount, resource, offset);
        cmdBuffer.Attributes.bEnableMediaFrameTracking    = true;
        cmdBuffer.Attributes.resMediaFrameTrackingSurface = resource;
        cmdBuffer.Attributes.dwMediaFrameTrackingTag      = m_statusReport->GetSubmittedCount() + 1;
        // Set media frame tracking address offset(the offset from the encoder status buffer page)
        cmdBuffer.Attributes.dwMediaFrameTrackingAddrOffset = 0;
    }

    return eStatus;
}

MOS_STATUS EncodeScalabilitySinglePipe::Oca1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_MMIOREGISTERS mmioRegister;
    SCALABILITY_CHK_NULL_RETURN(m_hwInterface);
    MhwVdboxMfxInterface *mfxInterface = m_hwInterface->GetMfxInterface();
    SCALABILITY_CHK_NULL_RETURN(mfxInterface);
    bool validMmio = mfxInterface->ConvertToMiRegister(MHW_VDBOX_NODE_1, mmioRegister);
    if (validMmio)
    {
        SCALABILITY_CHK_NULL_RETURN(m_osInterface);
        SCALABILITY_CHK_NULL_RETURN(m_osInterface->pOsContext);
        MhwMiInterface *miInterface = m_hwInterface->GetMiInterface();
        SCALABILITY_CHK_NULL_RETURN(miInterface);

        HalOcaInterface::On1stLevelBBStart(
            cmdBuffer,
            *m_osInterface->pOsContext,
            m_osInterface->CurrentGpuContextHandle,
            *miInterface,
            mmioRegister);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeScalabilitySinglePipe::Oca1stLevelBBEnd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    SCALABILITY_CHK_NULL_RETURN(m_osInterface);
    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface);

    return MOS_STATUS_SUCCESS;
}

}


