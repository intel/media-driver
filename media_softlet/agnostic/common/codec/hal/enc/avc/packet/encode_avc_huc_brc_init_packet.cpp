/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_huc_brc_init_packet.cpp
//! \brief    Defines the interface for avc huc brc init/reset packet
//!

#include "encode_avc_huc_brc_init_packet.h"
#include "encode_avc_vdenc_const_settings.h"
#include "encode_avc_brc.h"
#include "media_avc_feature_defs.h"

namespace encode {

MOS_STATUS AvcHucBrcInitPkt::Init()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());

    ENCODE_CHK_NULL_RETURN(m_featureManager);
    m_basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcInitPkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    ENCODE_CHK_STATUS_RETURN(DumpHucBrcInit(false));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcInitPkt::AllocateResources()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

    // initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;

    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BrcInit DmemBuffer";

    for (uint32_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        PMOS_RESOURCE allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcInitDmemBuffer[i] = allocatedbuffer;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcInitPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog = false;

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }

    auto brcFeature = dynamic_cast<AvcEncodeBRC *>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    ENCODE_CHK_STATUS_RETURN(brcFeature->SaveHucStatus2Buffer(m_resHucStatus2Buffer));

    SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);
    ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_INIT));

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpHucBrcInit(true));)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcInitPkt::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(MI_FORCE_WAKEUP, m_miItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, AvcHucBrcInitPkt)
{
    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(setting);

    params.kernelDescriptor = setting->brcSettings.vdboxHucVdencBrcInitKernelDescriptor;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, AvcHucBrcInitPkt)
{
    ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

    params.function      = BRC_INIT;
    params.hucDataSource = m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx];
    params.dataLength    = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, AvcHucBrcInitPkt)
{
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

    params.function = BRC_INIT;

    params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, m_basicFeature->m_frameNum);
    params.regionParams[0].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcInitPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();

    auto osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_RETURN(osInterface);

    uint32_t hucCommandsSize = 0;
    uint32_t hucPatchListSize = 0;
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
        m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

    commandBufferSize = hucCommandsSize;
    requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcInitPkt::SetDmemBuffer()const
{
    ENCODE_FUNC_CALL();

    // Setup BrcInit DMEM
    auto hucVdencBrcInitDmem = (VdencAvcHucBrcInitDmem*)m_allocator->LockResourceForWrite(m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx]);
    ENCODE_CHK_NULL_RETURN(hucVdencBrcInitDmem);
    MOS_ZeroMemory(hucVdencBrcInitDmem, sizeof(VdencAvcHucBrcInitDmem));

    RUN_FEATURE_INTERFACE_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, SetDmemForInit, hucVdencBrcInitDmem);

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx]));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcHucBrcInitPkt::DumpHucBrcInit(bool isInput)
{
    ENCODE_FUNC_CALL();
    int32_t currentPass = m_pipeline->GetCurrentPass();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    if (isInput)
    {
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx],
            m_vdencBrcInitDmemBufferSize,
            currentPass,
            hucRegionDumpInit));
    }

    auto vdencBRCHistoryBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, 0);
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucRegion(
        vdencBRCHistoryBuffer,
        0,
        settings->brcSettings.vdencBrcHistoryBufferSize,
        0,
        "_History",
        isInput,
        currentPass,
        hucRegionDumpInit));

    return MOS_STATUS_SUCCESS;
}

#endif

}
