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
//! \file     codechal_encode_jpeg_g12.cpp
//! \brief    Defines state class for JPEG encoder.
//!

#include "codechal_encode_jpeg_g12.h"
#include "codechal_mmc_g12.h"
#include "mos_util_user_interface.h"

CodechalEncodeJpegStateG12::CodechalEncodeJpegStateG12(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo)
        : CodechalEncodeJpegState(hwInterface, debugInterface, standardInfo),
        m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    InitMmcState();
    Mos_SetVirtualEngineSupported(m_osInterface, true);
    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);
}

CodechalEncodeJpegStateG12::~CodechalEncodeJpegStateG12()
{
    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }
}

MOS_STATUS CodechalEncodeJpegStateG12::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTrackingRequested,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiForceWakeupCmd(
        cmdBuffer,
        &forceWakeupParams));

    return CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTrackingRequested, mmioRegister);
}

MOS_STATUS CodechalEncodeJpegStateG12::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodecHalMmcStateG12, m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeJpegStateG12::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bNeedSyncWithPrevious = true;
        vesetParams.bSFCInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, cmdBuffer, true));

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG12::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)

    // VE2.0 Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));

#endif // _DEBUG || _RELEASE_INTERNAL
    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    int32_t             bNullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
     m_osInterface,
     cmdBuffer,
     bNullRendering));

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG12::Initialize(CodechalSetting  *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(settings);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::Initialize(settings));

    // Picture Level Commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetMfxStateCommandsDataSize(
            CODECHAL_ENCODE_MODE_JPEG,
            &m_pictureStatesSize,
            &m_picturePatchListSize,
            0));

    // Slice Level Commands (cannot be placed in 2nd level batch)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetMfxPrimitiveCommandsDataSize(
            CODECHAL_ENCODE_MODE_JPEG,
            &m_sliceStatesSize,
            &m_slicePatchListSize,
            0));

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeJpegStateG12::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SetGpuCtxCreatOption());
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
            m_sinlgePipeVeState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

