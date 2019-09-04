/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     vphal_render_vebox_iecp.cpp
//! \brief    VPHAL VEBOX Image Enhancement and Color Processing (IECP) interfaces
//! \details  VPHAL VEBOX Image Enhancement and Color Processing (IECP) interfaces
//!

#include "vphal_render_vebox_procamp.h"
#include "vphal_render_vebox_ste.h"
#include "vphal_render_vebox_tcc.h"
#include "vphal_render_vebox_util_base.h"

//!
//! \brief    Initial IECP filters
//! \details  Initial IECP filters.
//!           All avaliable opened IECP filters should add to m_filters array here. 
//!
VPHAL_VEBOX_IECP_RENDERER::VPHAL_VEBOX_IECP_RENDERER()
{
    int32_t i = 0;

    // add all avaliable opened IECP filters, max size is 15.
#if VPHAL_RENDER_VEBOX_TCC_ENABLE
    m_filters[i++] = MOS_New(VPHAL_VEBOX_IECP_TCC);
#endif  // VPHAL_RENDER_VEBOX_TCC_ENABLE
#if VPHAL_RENDER_VEBOX_STE_ENABLE
    m_filters[i++] = MOS_New(VPHAL_VEBOX_IECP_STE);
#endif // VPHAL_RENDER_VEBOX_STE_ENABLE
#if VPHAL_RENDER_VEBOX_PROCAMP_ENABLE
    m_filters[i++] = MOS_New(VPHAL_VEBOX_IECP_ProcAmp);
#endif // VPHAL_RENDER_VEBOX_PROCAMP_ENABLE

    m_filters[i]  = nullptr;
    m_filterCount = i;

    // Initial pointers as nullptr
    m_veboxState = nullptr;
    m_renderData = nullptr;
}

//!
//! \brief    Destroy IECP filters
//! \details  Destroy IECP filters.
//!           Delete allocated IECP filters. 
//!
VPHAL_VEBOX_IECP_RENDERER::~VPHAL_VEBOX_IECP_RENDERER()
{
    int32_t i = 0;

    for (i = 0; i < m_filterCount; i++)
    {
        if (m_filters[i])
        {
            MOS_Delete(m_filters[i]);
            m_filters[i] = nullptr;
        }
    }
}

//!
//! \brief    Vebox set alpha parameter
//! \details  Setup Alpha Params
//! \param    [in] pOutSurface
//!           Pointer to output surface of Vebox
//! \param    [in,out] pVphalVeboxIecpParams
//!           Pointer to IECP parameter
//! \return   void
//!
void VPHAL_VEBOX_IECP_RENDERER::VeboxSetAlphaParams(
    PVPHAL_SURFACE                  pOutSurface,
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams)
{
    PVPHAL_VEBOX_RENDER_DATA        pRenderData = m_renderData;

    if (pRenderData->pAlphaParams != nullptr)
    {
        switch (pRenderData->pAlphaParams->AlphaMode)
        {
        case VPHAL_ALPHA_FILL_MODE_NONE:
            if (pOutSurface->Format == Format_A8R8G8B8)
            {
                pVphalVeboxIecpParams->wAlphaValue =
                    (uint8_t)(0xff * pRenderData->pAlphaParams->fAlpha);
            }
            else
            {
                pVphalVeboxIecpParams->wAlphaValue = 0xff;
            }
            break;

            // VEBOX does not support Background Color
        case VPHAL_ALPHA_FILL_MODE_BACKGROUND:

            // VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM case is hit when the
            // input does not have alpha
            // So we set Opaque alpha channel.
        case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
        case VPHAL_ALPHA_FILL_MODE_OPAQUE:
        default:
            pVphalVeboxIecpParams->wAlphaValue = 0xff;
            break;
        }
    }
    else
    {
        pVphalVeboxIecpParams->wAlphaValue = 0xff;
    }
}

//!
//! \brief    Initial Vebox IECP state parameter
//! \param    [in] VphalColorSpace
//!           Vphal color space
//! \param    [in] pMhwVeboxIecpParams
//!           Pointer to Mhw Vebox IECP parameters
//! \return   MOS_STATUS
//!
MOS_STATUS VPHAL_VEBOX_IECP_RENDERER::InitParams(
    VPHAL_CSPACE                    VphalColorSpace,
    PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams = m_renderData->GetVeboxIECPParams();
    int32_t i = 0;

    VPHAL_RENDER_CHK_NULL(pMhwVeboxIecpParams);

    MOS_ZeroMemory(pMhwVeboxIecpParams, sizeof(*pMhwVeboxIecpParams));

    for (i = 0; i < m_filterCount; i++)
    {
        VPHAL_RENDER_CHK_NULL(m_filters[i]);
        m_filters[i]->InitParams(pVphalVeboxIecpParams, pMhwVeboxIecpParams);
    }

    pMhwVeboxIecpParams->ColorSpace     = VPHal_VpHalCspace2MhwCspace(VphalColorSpace);
    pMhwVeboxIecpParams->dstFormat      = pVphalVeboxIecpParams->dstFormat;
    pMhwVeboxIecpParams->srcFormat      = pVphalVeboxIecpParams->srcFormat;
    pMhwVeboxIecpParams->bCSCEnable     = pVphalVeboxIecpParams->bCSCEnable;
    pMhwVeboxIecpParams->pfCscCoeff     = pVphalVeboxIecpParams->pfCscCoeff;
    pMhwVeboxIecpParams->pfCscInOffset  = pVphalVeboxIecpParams->pfCscInOffset;
    pMhwVeboxIecpParams->pfCscOutOffset = pVphalVeboxIecpParams->pfCscOutOffset;
    pMhwVeboxIecpParams->bAlphaEnable   = pVphalVeboxIecpParams->bAlphaEnable;
    pMhwVeboxIecpParams->wAlphaValue    = pVphalVeboxIecpParams->wAlphaValue;

finish:
    return eStatus;
}

//!
//! \brief    Vebox set IECP parameter
//! \details  Set Vebox IECP state parameter
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pOutSurface
//!           Pointer to output surface of Vebox
//! \return   void
//!
void VPHAL_VEBOX_IECP_RENDERER::SetParams(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutSurface)
{
    PVPHAL_VEBOX_STATE              pVeboxState           = m_veboxState;
    PVPHAL_VEBOX_RENDER_DATA        pRenderData           = m_renderData;
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams = m_renderData->GetVeboxIECPParams();
    int32_t i = 0;

    for (i = 0; i < m_filterCount; i++)
    {
        if (m_filters[i])
        {
            m_filters[i]->SetParams(pSrcSurface, m_renderData);
        }
    }

    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) ||
        IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        pRenderData->GetVeboxStateParams()->pVphalVeboxIecpParams = pVphalVeboxIecpParams;
    }

    // Check if Back End CSC is enabled in VEBOX
    if (pRenderData->bBeCsc)
    {
        // Calculate matrix if not done so before. CSC is expensive!
        if ((pVeboxState->CscInputCspace  != pSrcSurface->ColorSpace) ||
            (pVeboxState->CscOutputCspace != pOutSurface->ColorSpace))
        {
            // Get the matrix to use for conversion
            pVeboxState->VeboxGetBeCSCMatrix(
                pSrcSurface,
                pOutSurface);

            // Store it for next BLT
            pVeboxState->CscInputCspace  = pSrcSurface->ColorSpace;
            pVeboxState->CscOutputCspace = pOutSurface->ColorSpace;
        }

        // Copy the values into IECP Params
        pVphalVeboxIecpParams->bCSCEnable       = true;
        pVphalVeboxIecpParams->pfCscCoeff       = pVeboxState->fCscCoeff;
        pVphalVeboxIecpParams->pfCscInOffset    = pVeboxState->fCscInOffset;
        pVphalVeboxIecpParams->pfCscOutOffset   = pVeboxState->fCscOutOffset;

        // Set Alpha State
        if ((pOutSurface->Format == Format_A8R8G8B8) ||
            (pOutSurface->Format == Format_A8B8G8R8) ||
            (pOutSurface->Format == Format_X8R8G8B8))
        {
            pVphalVeboxIecpParams->bAlphaEnable = true;
            VeboxSetAlphaParams(pOutSurface, pVphalVeboxIecpParams);
        }
        else
        {
            pVphalVeboxIecpParams->bAlphaEnable = false;
        }

        // Set the output format which can be referred by YUV_Channel_Swap bit
        pVphalVeboxIecpParams->dstFormat = pOutSurface->Format;

        // Set input foramt
        pVphalVeboxIecpParams->srcFormat = pSrcSurface->Format;

        pRenderData->GetVeboxStateParams()->pVphalVeboxIecpParams = pVphalVeboxIecpParams;
    }
}
