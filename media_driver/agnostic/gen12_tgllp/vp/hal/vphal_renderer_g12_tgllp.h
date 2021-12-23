/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file     vphal_renderer_g12_tgllp.h
//! \brief    The header file of VPHAL top level rendering component
//! \details  The top renderer is responsible for coordinating the sequence of calls to low level renderers, e.g. DNDI or Comp
//!
#ifndef __VPHAL_RENDERER_G12TGLLP_H__
#define __VPHAL_RENDERER_G12TGLLP_H__

#include "vphal_renderer_g12.h"

// Two pass down scaling for down scaling quality due to 8-Tab polyphase filter.
#define VPHAL_MAX_NUM_DS_SURFACES 2

//!
//! \brief VPHAL renderer Gen12 class
//!
class VphalRendererG12Tgllp : public VphalRendererG12
{
public:
    //!
    //! \brief    VphalRendererG12Tgllp constructor
    //! \details  Based on the HW and OS info, initialize the renderer interfaces
    //! \param    [in] pRenderHal
    //!           Pointer to RenderHal Interface Structure
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalRendererG12Tgllp(
        PRENDERHAL_INTERFACE                pRenderHal,
        MOS_STATUS                          *pStatus) :
        VphalRendererG12(pRenderHal, pStatus)
    {
        bEnableCMFC = true;
        for (uint32_t nIndex = 0; nIndex < VPHAL_MAX_NUM_DS_SURFACES; nIndex++)
        {
            m_pDSSurface[nIndex] = nullptr;
        }
    }

    //!
    //! \brief    VPHAL renderer destructor
    //! \details  Destory the resources allocated for the renderers
    //!           including VEBOX and Composite.
    //! \param    [in,out] VphalRenderer* pRenderer
    //!           VPHAL renderer pointer
    //! \return   VOID
    //!
    ~VphalRendererG12Tgllp()
    {
        for (uint32_t nIndex = 0; nIndex < VPHAL_MAX_NUM_DS_SURFACES; nIndex++)
        {
            if (m_pDSSurface[nIndex])
            {
                m_pOsInterface->pfnFreeResource(m_pOsInterface, &m_pDSSurface[nIndex]->OsResource);
                //release 3dlut params
                if (m_pDSSurface[nIndex]->p3DLutParams)
                {
                    MOS_FreeMemory(m_pDSSurface[nIndex]->p3DLutParams);
                    m_pDSSurface[nIndex]->p3DLutParams = nullptr;
                }
            }
            MOS_FreeMemAndSetNull(m_pDSSurface[nIndex]);
        }
    }

    //!
    //! \brief    Initialize the KDLL parameters
    //! \details  Initialize the KDLL parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS InitKdllParam();

    //!
    //! \brief    Main render function
    //! \details  The top level renderer function, which may contain multiple
    //!           passes of rendering
    //! \param    [in] pcRenderParams
    //!           Const pointer to VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS Render(PCVPHAL_RENDER_PARAMS   pcRenderParams);

    //!
    //! \brief    set Render Gpu Context
    //! \details  set Render Gpu Context based on lumakey and CCS status.
    //! \param    [in] RenderParams
    //!           VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetRenderGpuContext(VPHAL_RENDER_PARAMS& RenderParams);

private:
    //!
    //! \brief    Scaling function
    //! \details  The scaling function is only for scaling without other VP features.
    //!           Down scaling needs 2 pass if scaling ratio is >2 for better quality.
    //!           Pass#1 DS to 1/2 target resolution; Pass #2: DS from 1/2 target resolution to target resolution
    //! \param    [in,out] pRenderParams
    //!           Pointer to VPHAL render parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RenderScaling(
        PVPHAL_RENDER_PARAMS    pRenderParams);
    //!
    //! \brief    Allocate surface
    //! \details  Allocate surface according to the attributes of surface except the specified width/height/format.
    //! \param    [in] RenderParams
    //!           VPHAL render parameter
    //! \param    [in] pSurface
    //!           Pointer to the surface which specifies the attributes except the specified width/height/format.
    //! \param    [in] pAllocatedSurface
    //!           Pointer to the allocated surface.
    //! \param    [in] dwSurfaceWidth
    //!           The width of allocated surface.
    //! \param    [in] dwSurfaceHeight
    //!           The height of allocated surface.
    //! \param    [in] eFormat
    //!           The format of allocated surface.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateSurface(
        PCVPHAL_RENDER_PARAMS       pcRenderParams,
        PVPHAL_SURFACE              pSurface,
        PVPHAL_SURFACE              pAllocatedSurface,
        uint32_t                    dwSurfaceWidth,
        uint32_t                    dwSurfaceHeight,
        MOS_FORMAT                  eFormat);

    // Surfaces for down scaling if down scaling firstly in the use case scaling + 3dlut
    PVPHAL_SURFACE    m_pDSSurface[VPHAL_MAX_NUM_DS_SURFACES];
};

#endif // __VPHAL_RENDER_G12TGLLP_H__
