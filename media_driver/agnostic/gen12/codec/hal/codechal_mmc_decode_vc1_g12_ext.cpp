/*
* Copyright (c) 2017-2022, Intel Corporation
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
//! \file     codechal_mmc_decode_vc1_g12_ext.cpp
//! \brief    Implements VC1 Gen12 media memory compression extra interface
//!

#include "codechal_mmc_decode_vc1_g12_ext.h"
#include "mos_interface.h"

CodechalMmcDecodeVc1G12Ext::CodechalMmcDecodeVc1G12Ext(
    CodechalHwInterface* hwInterface,
    CodecHalMmcState* mmcState) :
    m_hwInterface(hwInterface),
    m_mmcState(mmcState)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_HW_ASSERT(m_hwInterface);
    CODECHAL_HW_ASSERT(m_mmcState);
}

MOS_STATUS CodechalMmcDecodeVc1G12Ext::CopyAuxSurfForSkip(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_RESOURCE       srcResource,
    PMOS_RESOURCE       destResource)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(srcResource);
    CODECHAL_DECODE_CHK_NULL_RETURN(srcResource->pGmmResInfo);
    CODECHAL_DECODE_CHK_NULL_RETURN(destResource);
    CODECHAL_DECODE_CHK_NULL_RETURN(destResource->pGmmResInfo);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface);

    GMM_RESOURCE_FLAG srcFlags  = srcResource->pGmmResInfo->GetResFlags();
    GMM_RESOURCE_FLAG destFlags = destResource->pGmmResInfo->GetResFlags();
    uint32_t          srcArrayIndex = 0;
    uint32_t          destArrayIndex = 0;
    PMOS_INTERFACE    osInterface = m_hwInterface->GetOsInterface();

    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);

    srcArrayIndex  = osInterface->pfnGetResourceArrayIndex(srcResource);
    destArrayIndex = osInterface->pfnGetResourceArrayIndex(destResource);

    if (m_mmcState->IsMmcEnabled() && srcFlags.Gpu.UnifiedAuxSurface && destFlags.Gpu.UnifiedAuxSurface)
    {
        CodechalHucStreamoutParams HucStreamOutParams;
        MOS_ZeroMemory(&HucStreamOutParams, sizeof(HucStreamOutParams));

        // params calculation for aux data
        uint64_t srcGfxAddr     = m_hwInterface->GetOsInterface()->pfnGetResourceGfxAddress(m_hwInterface->GetOsInterface(), srcResource);
        uint64_t srcAuxYOffset  = srcResource->pGmmResInfo->GetPlanarAuxOffset(srcArrayIndex, GMM_AUX_Y_CCS);
        uint32_t srcAuxSurfSize = (uint32_t)srcResource->pGmmResInfo->GetAuxQPitch();

        uint64_t destGfxAddr     = m_hwInterface->GetOsInterface()->pfnGetResourceGfxAddress(m_hwInterface->GetOsInterface(), destResource);
        uint64_t destAuxYOffset  = destResource->pGmmResInfo->GetPlanarAuxOffset(destArrayIndex, GMM_AUX_Y_CCS);
        uint32_t destAuxSurfSize = (uint32_t)destResource->pGmmResInfo->GetAuxQPitch();

        // Ind Obj Addr command
        HucStreamOutParams.dataBuffer            = srcResource;
        HucStreamOutParams.dataSize              = srcAuxSurfSize;
        HucStreamOutParams.dataOffset            = MOS_ALIGN_FLOOR(srcAuxYOffset, MHW_PAGE_SIZE);
        HucStreamOutParams.streamOutObjectBuffer = destResource;
        HucStreamOutParams.streamOutObjectSize   = destAuxSurfSize;
        HucStreamOutParams.streamOutObjectOffset = MOS_ALIGN_FLOOR(destAuxYOffset, MHW_PAGE_SIZE);

        // Stream object params
        HucStreamOutParams.indStreamInLength    = srcAuxSurfSize;
        HucStreamOutParams.inputRelativeOffset  = 0;
        HucStreamOutParams.outputRelativeOffset = 0;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->PerformHucStreamOut(
            &HucStreamOutParams,
            cmdBuffer));
    }

    return eStatus;
}
