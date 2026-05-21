/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_huc_slbb_update_pkt.cpp
//! \brief    Implements the HucSLBBUpdatePkt class for Second Level Batch Buffer updates using HuC
//!

#include "encode_huc_slbb_update_pkt.h"
#include "media_interfaces_huc_kernel_source.h"
#include "codechal_debug.h"
#if _SW_HUC
#include "encode_sw_slbb_update.h"
#endif

namespace encode
{

HucSLBBUpdatePkt::HucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
    : EncodeHucPkt(pipeline, task, hwInterface),
      EncodeHucPPGTTPkt(hwInterface)
{
    ENCODE_FUNC_CALL();
    m_itfPPGTT = std::dynamic_pointer_cast<mhw::vdbox::huc::ItfPPGTT>(m_hucItf);
}

HucSLBBUpdatePkt::~HucSLBBUpdatePkt()
{
    ENCODE_FUNC_CALL();
    
    if (m_hucKernelSource != nullptr)
    {
        m_hucKernelSource->ReportMode(m_userSettingPtr);
    }
}

MOS_STATUS HucSLBBUpdatePkt::Init()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    
    // Create HuC kernel source factory
    m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
    ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
    
    // Determine PPGTT mode
    ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->Init(m_hwInterface->GetSkuTable(), m_userSettingPtr));
    m_isPPGTT = m_hucKernelSource->IsPpgttMode();

    // Call parent class Init
    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucSLBBUpdatePkt::AllocateResources()
{
    ENCODE_FUNC_CALL();
    
    // Call parent AllocateResources
    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());
    
    // Allocate DMEM buffer for SLBB update operations
    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForDmemBuffer;
        MOS_ZeroMemory(&allocParamsForDmemBuffer, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForDmemBuffer.Type               = MOS_GFXRES_BUFFER;
        allocParamsForDmemBuffer.TileType           = MOS_TILE_LINEAR;
        allocParamsForDmemBuffer.Format             = Format_Buffer;
        allocParamsForDmemBuffer.dwBytes            = MOS_ALIGN_CEIL(m_slbbUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForDmemBuffer.pBufName           = "HuC SLBB Update DMEM Buffer";
        allocParamsForDmemBuffer.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        
        MOS_RESOURCE *allocatedBuffer = m_allocator->AllocateResource(allocParamsForDmemBuffer, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_slbbUpdateDmemBuffer[k] = *allocatedBuffer;
    }
    
    return MOS_STATUS_SUCCESS;
}

#if _SW_HUC
MOS_STATUS HucSLBBUpdatePkt::InitSwSLBB(HuCFunction function)
{
    ENCODE_FUNC_CALL();

    if (m_swSLBB == nullptr)
    {
        EncodeBasicFeature* basicFeature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeature);
        m_swSLBB = EncodeSwBrc::CreateFactory(basicFeature->m_mode, m_allocator, m_hwInterface->GetOsInterface(), function);
    }
    return MOS_STATUS_SUCCESS;
}
#endif  // !_SW_HUC

MOS_STATUS HucSLBBUpdatePkt::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
{
    ENCODE_FUNC_CALL();
    
    ENCODE_CHK_NULL_RETURN(cmdBuffer);
    ENCODE_CHK_NULL_RETURN(m_itfPPGTT);

#if _SW_HUC
    // Handle SW SLBB initialization and execution for SLBB_UPDATE only
    if (function == SLBB_UPDATE)
    {
        ENCODE_CHK_STATUS_RETURN(InitSwSLBB(function));
        // Downcast to SwSlbbUpdate to access SwSLBBEnabled() method
        // SwSLBBEnabled() is specific to SwSlbbUpdate and not in base class EncodeSwBrc
        // This allows independent enablement of SW SLBB separate from SW BRC
        auto swSlbbUpdate = std::dynamic_pointer_cast<SwSlbbUpdate>(m_swSLBB);
        if (swSlbbUpdate && swSlbbUpdate->SwSLBBEnabled())
        {
            // Add prolog commands before SW simulation (same as HW path)
            if (prologNeeded)
            {
                ENCODE_CHK_STATUS_RETURN(AddForceWakeup(*cmdBuffer));
                ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*cmdBuffer));
            }

            SETPAR(HUC_DMEM_STATE, m_hucItf);
            SETPAR(HUC_VIRTUAL_ADDR_STATE, m_hucItf);

            auto &virtualAddrParams = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();
            auto &dmemParams        = m_hucItf->MHW_GETPAR_F(HUC_DMEM_STATE)();

            CODECHAL_DEBUG_TOOL(
                ENCODE_CHK_STATUS_RETURN(DumpInput());)

            EncodeBasicFeature *basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
            ENCODE_CHK_NULL_RETURN(basicFeature);

            return m_swSLBB->SwBrcImpl(
                function,
                virtualAddrParams,
                dmemParams,
                basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));
        }
    }
#endif  // !_SW_HUC

    // Add prolog commands if needed
    if (prologNeeded)
    {
        ENCODE_CHK_STATUS_RETURN(AddForceWakeup(*cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*cmdBuffer));
    }

    // Start performance collection
    ENCODE_CHK_STATUS_RETURN(StartPerfCollect(*cmdBuffer));

    // Conditionally add HUC_IMEM_ADDR commands for PPGTT mode
    if (m_isPPGTT)
    {
        SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, m_itfPPGTT, cmdBuffer);
    }

    // Execute HUC command sequence
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_IMEM_STATE(cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_PIPE_MODE_SELECT(cmdBuffer));

    SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, cmdBuffer);
    SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, cmdBuffer);

    // Enable HuC status reporting
    m_enableHucStatusReport = true;
    ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(cmdBuffer, storeHucStatus2Needed));

    // Start HuC execution
    SETPAR_AND_ADDCMD(HUC_START, m_hucItf, cmdBuffer);

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());)

    // Perform pipeline flush
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, cmdBuffer);

    // Flush the engine to ensure memory written out
    ENCODE_CHK_NULL_RETURN(m_miItf);
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    // End performance collection and status reporting
    ENCODE_CHK_STATUS_RETURN(EndPerfCollect(*cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(StoreHuCStatusRegister(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucSLBBUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();
    
    // Call parent CalculateCommandSize
    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::CalculateCommandSize(commandBufferSize, requestedPatchListSize));

    // Add extra space for HUC_IMEM_ADDR commands in PPGTT mode
    if (m_isPPGTT)
    {
        ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
        commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
        
        // Align to page boundaries
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);
    }
    
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();
    
    params.kernelDescriptor = m_vdboxHucKernelDescriptor;
    
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();
    
    // Call codec-specific SetDmem() to populate DMEM buffer
    ENCODE_CHK_STATUS_RETURN(SetDmem());
    
    // Configure DMEM state parameters
    params.function      = SLBB_UPDATE;
    params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_slbbUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx]);
    params.dataLength    = MOS_ALIGN_CEIL(m_slbbUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
    
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();
    
    // Base implementation - derived classes should override for codec-specific virtual address setup
    // Set up SLBB buffer if available
    if (m_slbbBuffer != nullptr)
    {
        params.regionParams[0].presRegion = m_slbbBuffer;
        params.regionParams[0].isWritable = true;
    }
    
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode