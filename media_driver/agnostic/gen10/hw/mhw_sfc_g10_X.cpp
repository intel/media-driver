/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_sfc_g10_X.cpp
//! \brief    Constructs sfc commands on Gen10-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_sfc.h"
#include "mhw_sfc_g10_X.h"

MOS_STATUS MhwSfcInterfaceG10::AddSfcState(
    PMOS_COMMAND_BUFFER            pCmdBuffer,
    PMHW_SFC_STATE_PARAMS          pSfcStateParams,
    PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface)
{
    MHW_CHK_NULL_RETURN(pCmdBuffer);
    MHW_CHK_NULL_RETURN(pSfcStateParams);

    mhw_sfc_g10_X::SFC_STATE_CMD *cmdPtr;
    cmdPtr = (mhw_sfc_g10_X::SFC_STATE_CMD *)pCmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwSfcInterfaceGeneric::AddSfcState(pCmdBuffer, pSfcStateParams, pOutSurface));

    MHW_CHK_NULL_RETURN(cmdPtr);
    cmdPtr->DW3.PreAvsChromaDownsamplingEnable                              = pSfcStateParams->dwChromaDownSamplingMode;
    cmdPtr->DW3.PreAvsChromaDownsamplingCoSitingPositionVerticalDirection   = pSfcStateParams->dwChromaDownSamplingVerticalCoef;
    cmdPtr->DW3.PreAvsChromaDownsamplingCoSitingPositionHorizontalDirection = pSfcStateParams->dwChromaDownSamplingHorizontalCoef;

    return MOS_STATUS_SUCCESS;
}

MhwSfcInterfaceG10::MhwSfcInterfaceG10(PMOS_INTERFACE pOsInterface)
   : MhwSfcInterfaceGeneric(pOsInterface)
{
    // Get Memory control object directly from MOS.
    // If any override is needed, something like pfnOverrideMemoryObjectCtrl() / pfnComposeSurfaceCacheabilityControl()
    // will need to be implemented.
    // Caching policy if any of below modes are true

    if (m_osInterface->osCpInterface != nullptr)
    {
        if (m_osInterface->osCpInterface->IsHMEnabled() ||
            m_osInterface->osCpInterface->IsIDMEnabled() ||
            m_osInterface->osCpInterface->IsSMEnabled())
        {
            m_outputSurfCtrl.Value =
                m_osInterface->pfnCachePolicyGetMemoryObject(MOS_MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface_PartialEncSurface).DwordValue;
        }
        else
        {
            m_outputSurfCtrl.Value =
                m_osInterface->pfnCachePolicyGetMemoryObject(MOS_MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface).DwordValue;
        }
    }
    else
    {
        m_outputSurfCtrl.Value =
            m_osInterface->pfnCachePolicyGetMemoryObject(MOS_MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface).DwordValue;
    }

    m_avsLineBufferCtrl.Value =
        m_osInterface->pfnCachePolicyGetMemoryObject(MOS_MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface).DwordValue;
    m_iefLineBufferCtrl.Value =
        m_osInterface->pfnCachePolicyGetMemoryObject(MOS_MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface).DwordValue;
}
