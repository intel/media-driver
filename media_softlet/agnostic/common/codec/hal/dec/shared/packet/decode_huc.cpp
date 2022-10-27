/*
* Copyright (c) 2021, Intel Corporation
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

#include "decode_huc.h"
#include "codechal_debug.h"
#include "decode_pipeline.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"

namespace decode
{

DecodeHucBasic::DecodeHucBasic(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext*hwInterface)
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
        m_miItf          = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
        m_vdencItf       = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
        m_hucItf         = std::static_pointer_cast<mhw::vdbox::huc::Itf>(m_hwInterface->GetHucInterfaceNext());
       
    }
}

DecodeHucBasic::~DecodeHucBasic()
{
}

MOS_STATUS DecodeHucBasic::Init()
{
    DECODE_CHK_NULL(m_pipeline);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_hucItf);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_vdencItf);
    m_basicFeature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    DECODE_CHK_STATUS(CmdPacket::Init());
    DECODE_CHK_STATUS(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::AllocateResources()
{
    DECODE_CHK_NULL(m_allocator);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::Completed(void* mfxStatus, void* rcsStatus, void* statusReport)
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

MOS_STATUS DecodeHucBasic::Destroy()
{
    return MOS_STATUS_SUCCESS;
}

void DecodeHucBasic::SetHucStatusMask(uint32_t hucStatusMask, uint32_t hucStatus2Mask)
{
    m_hucStatusMask  = hucStatusMask;
    m_hucStatus2Mask = hucStatus2Mask;
}

MOS_STATUS DecodeHucBasic::StoreHucStatusRegister(MOS_COMMAND_BUFFER& cmdBuffer)
{
    if(m_hucStatusMask == m_hucStatusInvalidMask)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_RESOURCE* osResource;
    uint32_t     offset;
    {
        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusMask, osResource, offset));

        // Write HUC_STATUS mask
        auto &par            = m_miItf->GETPAR_MI_STORE_DATA_IMM();
        par                  = {};
        par.pOsResource      = osResource;
        par.dwResourceOffset = offset;
        par.dwValue          = m_hucStatusMask;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_DATA_IMM(&cmdBuffer));
    }
    {
        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusReg, osResource, offset));

        // Store HUC_STATUS register
        auto &par           = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
        par                 = {};
        par.presStoreBuffer = osResource;
        par.dwOffset        = offset;
        par.dwRegister      = m_hucItf->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucStatusRegOffset;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::StoreHucStatus2Register(MOS_COMMAND_BUFFER& cmdBuffer)
{
    if(m_hucStatus2Mask == m_hucStatusInvalidMask)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_RESOURCE* osResource;
    uint32_t     offset;
    {
        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Mask, osResource, offset));

        // Write HUC_STATUS2 mask
        auto &par = m_miItf->GETPAR_MI_STORE_DATA_IMM();
        par                              = {};
        par.pOsResource                  = osResource;
        par.dwResourceOffset             = offset;
        par.dwValue                      = m_hucStatus2Mask;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_DATA_IMM(&cmdBuffer));  
    }
    {
        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Reg, osResource, offset));
        // Store HUC_STATUS2 register
        auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
        par                            = {};
        par.presStoreBuffer            = osResource;
        par.dwOffset                   = offset;
        par.dwRegister                 = m_hucItf->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucStatus2RegOffset;
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{

    MediaPacket::StartStatusReport(srType, cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{

    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);

    DECODE_CHK_STATUS(MediaPacket::EndStatusReport(srType, cmdBuffer));

    return MOS_STATUS_SUCCESS;

}

MOS_STATUS DecodeHucBasic::AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer, bool mfxWakeup, bool hcpWakeup)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FORCE_WAKEUP();
    par       = {};
    par.bMFXPowerWellControl                    = mfxWakeup;
    par.bMFXPowerWellControlMask                = true;
    par.bHEVCPowerWellControl                   = hcpWakeup;
    par.bHEVCPowerWellControlMask               = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FORCE_WAKEUP(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::SendPrologCmds(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DecodeSubPacket* subPacket = m_pipeline->GetSubPacket(DecodePacketId(m_pipeline, markerSubPacketId));
    DecodeMarkerPkt *makerPacket = dynamic_cast<DecodeMarkerPkt*>(subPacket);
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
    genericPrologParams.pvMiInterface = nullptr;
#ifdef _MMC_SUPPORTED
    genericPrologParams.bMmcEnabled = isMmcEnabled;
#endif
    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

    subPacket = m_pipeline->GetSubPacket(DecodePacketId(m_pipeline, predicationSubPacketId));
    DecodePredicationPkt *predicationPacket = dynamic_cast<DecodePredicationPkt*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeHucBasic::MemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
    par       = {};
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));
    return MOS_STATUS_SUCCESS;
}

}
