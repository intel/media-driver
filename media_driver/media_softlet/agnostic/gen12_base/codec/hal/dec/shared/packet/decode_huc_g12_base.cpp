/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_huc.cpp
//! \brief    Defines the common interface for decode huc implementation
//! \details  The decode huc interface is further sub-divided by different huc usage,
//!           this file is for the base interface which is shared by all.
//!

#include "decode_huc_g12_base.h"
#include "codechal_debug.h"
#include "decode_pipeline.h"
#include "decode_predication_packet_g12.h"
#include "decode_marker_packet_g12.h"

namespace decode
{

DecodeHucBasic_G12_Base::DecodeHucBasic_G12_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
    : CmdPacket(task)
{
    m_pipeline = dynamic_cast<DecodePipeline *>(pipeline);
    if (m_pipeline != nullptr)
    {
        m_featureManager = m_pipeline->GetFeatureManager();
        m_allocator      = m_pipeline->GetDecodeAllocator();
        m_decodecp       = m_pipeline->GetDecodeCp();
    }

    if (hwInterface != nullptr)
    {
        m_hwInterface    = hwInterface;
        m_osInterface    = hwInterface->GetOsInterface();
        m_hucInterface   = hwInterface->GetHucInterface();
        m_miInterface    = hwInterface->GetMiInterface();
        m_vdencInterface = hwInterface->GetVdencInterface();
    }
}

DecodeHucBasic_G12_Base::~DecodeHucBasic_G12_Base()
{
}

MOS_STATUS DecodeHucBasic_G12_Base::Init()
{
    DECODE_CHK_NULL(m_pipeline);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_hucInterface);
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_vdencInterface);

    m_basicFeature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    DECODE_CHK_STATUS(CmdPacket::Init());
    DECODE_CHK_STATUS(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::AllocateResources()
{
    DECODE_CHK_NULL(m_allocator);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::Completed(void* mfxStatus, void* rcsStatus, void* statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);
    DECODE_CHK_NULL(statusReport);

    DecodeStatusMfx* decodeStatusMfx = (DecodeStatusMfx*)mfxStatus;
    DecodeStatusReportData* statusReportData = (DecodeStatusReportData*)statusReport;

    // Print HuC_Status and HuC_Status2 registers
    DECODE_VERBOSEMESSAGE("Index = %d", statusReportData->currDecodedPic.FrameIdx);
    DECODE_VERBOSEMESSAGE("HUC_STATUS register = 0x%x",
        decodeStatusMfx->m_hucErrorStatus >> 32);
    DECODE_VERBOSEMESSAGE("HUC_STATUS2 register = 0x%x",
        decodeStatusMfx->m_hucErrorStatus2 >> 32);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::Destroy()
{
    return MOS_STATUS_SUCCESS;
}

void DecodeHucBasic_G12_Base::SetHucStatusMask(uint32_t hucStatusMask, uint32_t hucStatus2Mask)
{
    m_hucStatusMask  = hucStatusMask;
    m_hucStatus2Mask = hucStatus2Mask;
}

MOS_STATUS DecodeHucBasic_G12_Base::StoreHucStatusRegister(MOS_COMMAND_BUFFER& cmdBuffer)
{
    if(m_hucStatusMask == m_hucStatusInvalidMask)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_RESOURCE* osResource = nullptr;
    uint32_t     offset = 0;

    DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusMask, osResource, offset));

    // Write HUC_STATUS mask
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = osResource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue = m_hucStatusMask;
    DECODE_CHK_STATUS(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusReg, osResource, offset));

    // Store HUC_STATUS register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = osResource;
    storeRegParams.dwOffset = offset;
    storeRegParams.dwRegister = m_hucInterface->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucStatusRegOffset;
    DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::StoreHucStatus2Register(MOS_COMMAND_BUFFER& cmdBuffer)
{
    if(m_hucStatus2Mask == m_hucStatusInvalidMask)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_RESOURCE* osResource = nullptr;
    uint32_t     offset = 0;

    DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Mask, osResource, offset));

    // Write HUC_STATUS2 mask
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = osResource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue = m_hucStatus2Mask;
    DECODE_CHK_STATUS(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Reg, osResource, offset));

    // Store HUC_STATUS2 register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = osResource;
    storeRegParams.dwOffset = offset;
    storeRegParams.dwRegister = m_hucInterface->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucStatus2RegOffset;
    DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{

    MediaPacket::StartStatusReport(srType, cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{

    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);

    DECODE_CHK_STATUS(MediaPacket::EndStatusReport(srType, cmdBuffer));

    return MOS_STATUS_SUCCESS;

}

MOS_STATUS DecodeHucBasic_G12_Base::AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer, bool mfxWakeup, bool hcpWakeup)
{
    DECODE_FUNC_CALL();

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl      = mfxWakeup;
    forceWakeupParams.bMFXPowerWellControlMask  = true;
    forceWakeupParams.bHEVCPowerWellControl     = hcpWakeup;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    DECODE_CHK_STATUS(m_miInterface->AddMiForceWakeupCmd(&cmdBuffer, &forceWakeupParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::SendPrologCmds(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DecodeSubPacket* subPacket = m_pipeline->GetSubPacket(DecodePacketId(m_pipeline, markerSubPacketId));
    DecodeMarkerPktG12 *makerPacket = dynamic_cast<DecodeMarkerPktG12*>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    DecodeMemComp *mmcState = m_pipeline->GetMmcState();
    bool isMmcEnabled = (mmcState != nullptr && mmcState->IsMmcEnabled());
    if (isMmcEnabled)
    {
        DECODE_CHK_STATUS(mmcState->SendPrologCmd(&cmdBuffer, false));
    }
#endif

    MHW_GENERIC_PROLOG_PARAMS  genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = m_miInterface;
#ifdef _MMC_SUPPORTED
    genericPrologParams.bMmcEnabled = isMmcEnabled;
#endif
    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmd(&cmdBuffer, &genericPrologParams));

    subPacket = m_pipeline->GetSubPacket(DecodePacketId(m_pipeline, predicationSubPacketId));
    DecodePredicationPktG12 *predicationPacket = dynamic_cast<DecodePredicationPktG12*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic_G12_Base::MemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    return MOS_STATUS_SUCCESS;
}

}
