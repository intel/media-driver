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
//! \file     encode_vp9_huc_brc_init_packet.cpp
//! \brief    Defines the interface for huc brc init/reset packet for VP9
//!
#include "encode_vp9_huc_brc_init_packet.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_vdenc_feature_manager.h"

namespace encode
{

const uint32_t Vp9HucBrcInitPkt::m_brcInitDmem[48] =
{
    0x00000000, 0x00038400, 0x00030D40, 0x000C3500, 0x00061A80, 0x00061A80, 0x00000000, 0x0000001E,
    0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x270F0020, 0x02800000, 0x00010168,
    0x000000FF, 0x0000000E, 0x00000073, 0x00000000, 0x00000000, 0x7846321E, 0x7846321E, 0x735A321E,
    0xE5DFD8D1, 0x2F29211B, 0xE5DDD7D1, 0x5E56463F, 0xEAE3DAD4, 0x2F281F16, 0x01007488, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

MOS_STATUS Vp9HucBrcInitPkt::Init()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcInitPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    // To match watchdog timer threshold in codechal
    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));
    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog    = false;

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }

    ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog));

    RUN_FEATURE_INTERFACE_NO_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, DisableBrcInitReset);

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcInitPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();

    auto osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_RETURN(osInterface);

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    uint32_t vdencHucStatesSize = 0;
    uint32_t hucPatchListSize   = 0;

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
        m_basicFeature->m_mode, (uint32_t *)&vdencHucStatesSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

    commandBufferSize      = vdencHucStatesSize;
    requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcInitPkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL

    uint32_t brcHistoryBufferSize = 0;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, GetBrcHistoryBufferSize, brcHistoryBufferSize);

    // Region 0: BRC History Buffer (In/Out)
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistoryBuffer", false, hucRegionDumpInit, brcHistoryBufferSize));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcInitPkt::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

    // Initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

    // BRC init/reset DMEM
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucBrcInitDmem), CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BrcInit DmemBuffer";
    MOS_RESOURCE *allocatedBuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resVdencBrcInitDmemBuffer = *allocatedBuffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcInitPkt::SetDmemBuffer() const
{
    ENCODE_FUNC_CALL();

    // Setup BRC DMEM
    HucBrcInitDmem *dmem = (HucBrcInitDmem *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_resVdencBrcInitDmemBuffer));
    ENCODE_CHK_NULL_RETURN(dmem);

    MOS_SecureMemcpy(dmem, sizeof(HucBrcInitDmem), m_brcInitDmem, sizeof(m_brcInitDmem));

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, SetDmemForInit, dmem);

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_resVdencBrcInitDmemBuffer)));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9HucBrcInitPkt::DumpInput()
{
    ENCODE_FUNC_CALL();

    // Dump Huc/Brc init
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
        &m_resVdencBrcInitDmemBuffer,
        sizeof(HucBrcInitDmem),
        0,
        hucRegionDumpInit));

    uint32_t brcHistoryBufferSize = 0;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, GetBrcHistoryBufferSize, brcHistoryBufferSize);

    // Region 0: BRC History Buffer (In/Out)
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistoryBuffer", true, hucRegionDumpInit, brcHistoryBufferSize));

    return MOS_STATUS_SUCCESS;
}
#endif

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, Vp9HucBrcInitPkt)
{
    ENCODE_FUNC_CALL();

    params.kernelDescriptor = m_vdboxHucVp9VdencBrcInitKernelDescriptor;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Vp9HucBrcInitPkt)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

    params.function      = BRC_INIT;
    params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_resVdencBrcInitDmemBuffer);
    params.dataLength    = MOS_ALIGN_CEIL(sizeof(HucBrcInitDmem), CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, Vp9HucBrcInitPkt)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
    
    // Set regions virtual address parameters
    params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, m_basicFeature->m_frameNum);
    params.regionParams[0].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
