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
//! \file     encode_huc_la_init_packet.cpp
//! \brief    Defines the implementation of Huc Lookahead init packet
//!

#include "encode_huc_la_init_packet.h"
#include "encode_vdenc_lpla_analysis.h"

namespace encode
{
    MOS_STATUS HucLaInitPkt::Init()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        HUC_CHK_STATUS_RETURN(EncodeHucPkt::Init());
        ENCODE_CHK_NULL_RETURN(m_featureManager);
        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return eStatus;
    }

    MOS_STATUS HucLaInitPkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        return eStatus;
    }

    MOS_STATUS HucLaInitPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog    = false;

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, LA_INIT));

        return eStatus;
    }
    
    MOS_STATUS HucLaInitPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize  = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize      = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HucLaInitPkt)
    {
        params.kernelDescriptor = VDBOX_HUC_LA_ANALYSIS_KERNEL_DESCRIPTOR;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucLaInitPkt)
    {
        params.function = LA_INIT;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucLaInitPkt)
    {
        params.function = LA_INIT;
        return MOS_STATUS_SUCCESS;
    }

} //encode
