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
//! \file     decode_hevc_picture_packet_m12.cpp
//! \brief    Defines the interface for hevc decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_hevc_picture_packet_xe_m_base.h"
#include "decode_hevc_picture_packet_m12.h"
#include "decode_hevc_mem_compression_m12.h"
#include "mhw_mi_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "codechal_hw_g12_X.h"
#include "decode_common_feature_defs.h"

namespace decode
{

HevcDecodePicPktM12::~HevcDecodePicPktM12()
{
}

MOS_STATUS HevcDecodePicPktM12::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcDecodePicPktXe_M_Base::Init());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
{
    // Send VD_CONTROL_STATE Pipe Initialization
    DECODE_CHK_STATUS(VdInit(cmdBuffer));

    DECODE_CHK_STATUS(AddHcpPipeModeSelectCmd(cmdBuffer));
    if (IsRealTilePhase() || IsBackEndPhase())
    {
        VdScalabPipeLock(cmdBuffer);
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr &&
        m_downSamplingFeature->IsEnabled())
    {
        if (!IsFrontEndPhase())
        {
            DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
        }
    }
#endif

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12 pipeBufAddrParams;
    DECODE_CHK_STATUS(SetHcpPipeBufAddrParams(pipeBufAddrParams));

    DECODE_CHK_STATUS(AddHcpSurfaces(cmdBuffer, pipeBufAddrParams));
    DECODE_CHK_STATUS(AddHcpPipeBufAddrCmd(cmdBuffer, pipeBufAddrParams));
    DECODE_CHK_STATUS(AddHcpIndObjBaseAddrCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddHcpQmStateCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddHcpPicStateCmd(cmdBuffer));

    if (m_hevcPicParams->tiles_enabled_flag == 1)
    {
        DECODE_CHK_STATUS(AddHcpTileStateCmd(cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.initialization = true;

    MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
    DECODE_CHK_NULL(miInterfaceG12);
    DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.scalableModePipeLock = true;

    MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
    DECODE_CHK_NULL(miInterfaceG12);
    DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

    return MOS_STATUS_SUCCESS;
}

void HevcDecodePicPktM12::SetHcpPipeModeSelectParams(
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParamsBase)
{
    DECODE_FUNC_CALL();
    HevcDecodePicPktXe_M_Base::SetHcpPipeModeSelectParams(pipeModeSelectParamsBase);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &pipeModeSelectParams =
        static_cast<MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12&>(pipeModeSelectParamsBase);

    uint32_t pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    uint32_t multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    if (m_phase != nullptr)
    {
        m_phase->GetMode(pipeWorkMode, multiEngineMode);
    }
    pipeModeSelectParams.PipeWorkMode    = static_cast<MHW_VDBOX_HCP_PIPE_WORK_MODE>(pipeWorkMode);
    pipeModeSelectParams.MultiEngineMode = static_cast<MHW_VDBOX_HCP_MULTI_ENGINE_MODE>(multiEngineMode);

    auto decodeMode = m_hevcPipeline->GetDecodeMode();
    if (decodeMode == HevcPipeline::realTileDecodeMode)
    {
        if (m_hevcPipeline->IsFirstPass())
        {
            pipeModeSelectParams.ucPhaseIndicator = MHW_VDBOX_HCP_RT_FIRST_PHASE;
        }
        else if (m_hevcPipeline->IsLastPass())
        {
            pipeModeSelectParams.ucPhaseIndicator = MHW_VDBOX_HCP_RT_LAST_PHASE;
        }
        else
        {
            pipeModeSelectParams.ucPhaseIndicator = MHW_VDBOX_HCP_RT_MIDDLE_PHASE;
        }
    }

    pipeModeSelectParams.bHEVCSeparateTileProgramming = 
        (m_hevcPipeline->GetDecodeMode() == HevcPipeline::separateTileDecodeMode);    
}

MOS_STATUS HevcDecodePicPktM12::AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
    SetHcpPipeModeSelectParams(pipeModeSelectParams);

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParamsBase)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcDecodePicPktXe_M_Base::SetHcpPipeBufAddrParams(pipeBufAddrParamsBase));

#ifdef _MMC_SUPPORTED
    HevcDecodeMemCompM12 *hevcDecodeMemComp = dynamic_cast<HevcDecodeMemCompM12 *>(m_mmcState);
    DECODE_CHK_NULL(hevcDecodeMemComp);
    DECODE_CHK_STATUS(hevcDecodeMemComp->CheckReferenceList(*m_hevcBasicFeature, pipeBufAddrParamsBase.PostDeblockSurfMmcState,
        pipeBufAddrParamsBase.PreDeblockSurfMmcState, pipeBufAddrParamsBase.presReferences));
#endif

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12 &pipeBufAddrParams =
        static_cast<MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12&>(pipeBufAddrParamsBase);

    auto decodeMode = m_hevcPipeline->GetDecodeMode();
    if (decodeMode == HevcPipeline::virtualTileDecodeMode ||
        decodeMode == HevcPipeline::realTileDecodeMode)
    {
        pipeBufAddrParams.presSliceStateStreamOutBuffer        = &(m_resSliceStateStreamOutBuffer->OsResource);
        pipeBufAddrParams.presMvUpRightColStoreBuffer          = &(m_resMvUpRightColStoreBuffer->OsResource);
        pipeBufAddrParams.presIntraPredUpRightColStoreBuffer   = &(m_resIntraPredUpRightColStoreBuffer->OsResource);
        pipeBufAddrParams.presIntraPredLeftReconColStoreBuffer = &(m_resIntraPredLeftReconColStoreBuffer->OsResource);
        pipeBufAddrParams.presCABACSyntaxStreamOutBuffer       = &(m_resCABACSyntaxStreamOutBuffer->OsResource);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::AddHcpPipeBufAddrCmd(
    MOS_COMMAND_BUFFER &cmdBuffer, MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12 &pipeBufAddrParams)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));
    return MOS_STATUS_SUCCESS;
}

void HevcDecodePicPktM12::SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParamsBase)
{
    DECODE_FUNC_CALL();
    HevcDecodePicPktXe_M_Base::SetHcpPicStateParams(picStateParamsBase);

    MHW_VDBOX_HEVC_PIC_STATE_G12 &picStateParams =
        static_cast<MHW_VDBOX_HEVC_PIC_STATE_G12&>(picStateParamsBase);
    picStateParams.pHevcExtPicParams = m_hevcRextPicParams;
    picStateParams.pHevcSccPicParams = m_hevcSccPicParams;
    picStateParams.ucRecNotFilteredID = m_hevcBasicFeature->m_refFrames.m_IBCRefIdx;
}

MOS_STATUS HevcDecodePicPktM12::AddHcpPicStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HEVC_PIC_STATE_G12 picStateParams;
    SetHcpPicStateParams(picStateParams);

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpPicStateCmd(&cmdBuffer, &picStateParams));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;

    auto decodeMode = m_hevcPipeline->GetDecodeMode();
    if (decodeMode == HevcPipeline::virtualTileDecodeMode ||
        decodeMode == HevcPipeline::realTileDecodeMode)
    {
        stateCmdSizeParams.bScalableMode = true;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *decodeDownSampling =
        dynamic_cast<DecodeDownSamplingFeature*>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    stateCmdSizeParams.bSfcInUse = (decodeDownSampling != nullptr);
#endif

    // Picture Level Commands
    DECODE_CHK_STATUS(m_hwInterface->GetHcpStateCommandSize(
            m_hevcBasicFeature->m_mode,
            &commandBufferSize,
            &requestedPatchListSize,
            &stateCmdSizeParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktM12::ValidateCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);
    DECODE_CHK_NULL(m_resCABACSyntaxStreamOutBuffer);

    CodechalHwInterfaceG12* hwInterface = dynamic_cast<CodechalHwInterfaceG12*>(m_hwInterface);
    DECODE_CHK_NULL(hwInterface);

    uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADLESSTHANIDD;
    DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
        &m_resCABACStreamOutSizeBuffer->OsResource,
        0,
        m_resCABACSyntaxStreamOutBuffer->size,
        true,
        true,
        compareOperation,
        &cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

}
