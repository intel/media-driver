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
//! \file     vphal_render_vebox_iecp.h
//! \brief    VPHAL VEBOX Image Enhancement and Color Processing (IECP) interfaces
//! \details  VPHAL VEBOX Image Enhancement and Color Processing (IECP) interfaces
//!
#ifndef __VPHAL_RENDER_VEBOX_IECP_H__
#define __VPHAL_RENDER_VEBOX_IECP_H__

#include "mhw_vebox.h"

#define VPHAL_MAX_IECP_FILTERS          16                                      //!< Max number of IECP filters

typedef struct VPHAL_VEBOX_RENDER_DATA       *PVPHAL_VEBOX_RENDER_DATA;
typedef class  VPHAL_VEBOX_STATE             *PVPHAL_VEBOX_STATE;

//!
//! \brief  VEBOX IECP parameters
//!
typedef class VPHAL_VEBOX_IECP_PARAMS *PVPHAL_VEBOX_IECP_PARAMS;
typedef class VPHAL_VEBOX_IECP_PARAMS_EXT *PVPHAL_VEBOX_IECP_PARAMS_EXT;
class VPHAL_VEBOX_IECP_PARAMS
{
public:
    PVPHAL_COLORPIPE_PARAMS         pColorPipeParams;
    PVPHAL_PROCAMP_PARAMS           pProcAmpParams;
    MOS_FORMAT                      dstFormat;
    MOS_FORMAT                      srcFormat;

    // CSC params
    bool                            bCSCEnable;                                 // Enable CSC transform
    float*                          pfCscCoeff;                                 // [3x3] CSC Coeff matrix
    float*                          pfCscInOffset;                              // [3x1] CSC Input Offset matrix
    float*                          pfCscOutOffset;                             // [3x1] CSC Output Offset matrix
    bool                            bAlphaEnable;                               // Alpha Enable Param
    uint16_t                        wAlphaValue;                                // Color Pipe Alpha Value

    VPHAL_VEBOX_IECP_PARAMS()
    {
        pColorPipeParams    = nullptr;
        pProcAmpParams      = nullptr;
    }
    virtual ~VPHAL_VEBOX_IECP_PARAMS()
    {
        pColorPipeParams    = nullptr;
        pProcAmpParams      = nullptr;
    }
    virtual void Init()
    {
        pColorPipeParams    = nullptr;
        pProcAmpParams      = nullptr;

        dstFormat           = Format_Any;
        srcFormat           = Format_Any;

        bCSCEnable          = false;
        pfCscCoeff          = nullptr;
        pfCscInOffset       = nullptr;
        pfCscOutOffset      = nullptr;
        bAlphaEnable        = false;
        wAlphaValue         = 0;
    }
    virtual PVPHAL_VEBOX_IECP_PARAMS_EXT   GetExtParams() { return nullptr; }
};

class VPHAL_VEBOX_IECP_FILTER
{
public:
    virtual ~VPHAL_VEBOX_IECP_FILTER() {}

    //!
    //! \brief    Vebox set IECP parameter
    //! \details  Set Vebox IECP state parameter
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox Render Data
    //! \return   void
    //!
    virtual void SetParams(
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_VEBOX_RENDER_DATA    pRenderData) {}

    //!
    //! \brief    Init Vebox IECP parameter
    //! \param    [in] pVphalVeboxIecpParams
    //!           Pointer to input Vphal Iecp parameters
    //! \param    [in,out] pMhwVeboxIecpParams
    //!           Pointer to Mhw Iecp parameters
    //! \return   void
    //!
    virtual void InitParams(
        PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams,
        PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams) {}
};
typedef VPHAL_VEBOX_IECP_FILTER * PVPHAL_VEBOX_IECP_FILTER;

typedef class VPHAL_VEBOX_IECP_RENDERER_EXT *PVPHAL_VEBOX_IECP_RENDERER_EXT;
class VPHAL_VEBOX_IECP_RENDERER
{
public:
    VPHAL_VEBOX_IECP_RENDERER();

    virtual ~VPHAL_VEBOX_IECP_RENDERER();

    PVPHAL_VEBOX_STATE              m_veboxState;
    PVPHAL_VEBOX_RENDER_DATA        m_renderData;

    virtual PVPHAL_VEBOX_IECP_RENDERER_EXT   GetExtParams() { return nullptr; }

    //!
    //! \brief    Vebox set alpha parameter
    //! \details  Setup Alpha Params
    //! \param    [in] pOutSurface
    //!           Pointer to output surface of Vebox
    //! \param    [in,out] pVphalVeboxIecpParams
    //!           Pointer to IECP parameter
    //! \return   void
    //!
    void VeboxSetAlphaParams(
        PVPHAL_SURFACE                  pOutSurface,
        PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams);

    //!
    //! \brief    Initial Vebox IECP state parameter
    //! \param    [in] VphalColorSpace
    //!           Vphal color space
    //! \param    [in] pMhwVeboxIecpParams
    //!           Pointer to Mhw Vebox IECP parameters
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS InitParams(
        VPHAL_CSPACE                    VphalColorSpace,
        PMHW_VEBOX_IECP_PARAMS          pMhwVeboxIecpParams);

    //!
    //! \brief    Vebox set IECP parameter
    //! \details  Set Vebox IECP state parameter
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutSurface
    //!           Pointer to output surface of Vebox
    //! \return   void
    //!
    virtual void SetParams(
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pOutSurface);

private:
    // Array of VEBOX IECP Filters
    VPHAL_VEBOX_IECP_FILTER *m_filters[VPHAL_MAX_IECP_FILTERS];
    int32_t                  m_filterCount;
};
typedef VPHAL_VEBOX_IECP_RENDERER * PVPHAL_VEBOX_IECP_RENDERER;

#endif  //__VPHAL_RENDER_VEBOX_IECP_H__