/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

File Name: vphal_render_sfc_xe_xpm.h

Abstract:
    Handles rendering operations for VE to SFC Pipe

Notes:
    This module contains rendering definitions for Adv processing - all
    rendering parameters should be defined in this module, along with platform
    capabilities and/or restrictions

======================= end_copyright_notice ==================================*/
//!
//! \file     vphal_render_sfc_xe_xpm.h
//! \brief    The header file of the VPHAL SFC Xe_XPM rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VPHAL_RENDER_SFC_XE_XPM_H__
#define __VPHAL_RENDER_SFC_XE_XPM_H__

#include "mhw_sfc_xe_xpm.h"
#include "vphal_render_sfc_g12_base.h"

#if __VPHAL_SFC_SUPPORTED
class VphalSfcStateXe_Xpm :virtual public VphalSfcStateG12
{
public:
    VphalSfcStateXe_Xpm(
        PMOS_INTERFACE       osInterface,
        PRENDERHAL_INTERFACE renderHal,
        PMHW_SFC_INTERFACE   sfcInterface);

    virtual ~VphalSfcStateXe_Xpm()
    {

    }

    //!
    //! \brief    Initialize sfc render data
    //! \return   void
    //!
    virtual void InitRenderData()
    {
        VphalSfcState::InitRenderData();
        m_renderData.SfcStateParams = (MHW_SFC_STATE_PARAMS_XE_XPM*)MOS_AllocAndZeroMemory(sizeof(MHW_SFC_STATE_PARAMS_XE_XPM));
    }

protected:

    virtual bool IsOutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

    virtual MOS_STATUS SetSfcStateParams(
        PVPHAL_VEBOX_RENDER_DATA    pRenderData,
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pOutSurface);

    VPHAL_OUTPUT_PIPE_MODE GetOutputPipe(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget,
        PCVPHAL_RENDER_PARAMS       pcRenderParams);

    //!
    //! \brief    Set Sfc index used by HW
    //! \details  VPHAL set Sfc index used by HW
    //! \param    [in] dwSfcIndex;
    //!           set which Sfc can be used by HW
    //! \param    [in] dwSfcCount;
    //!           set Sfc Count
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSfcIndex(
        uint32_t                    dwSfcIndex,
        uint32_t                    dwSfcCount);

    //!
    //! \brief    Free resources used by SFC Pipe
    //! \details  Free the AVS and IEF line buffer surfaces for SFC
    //! \return   void
    //!
    virtual void FreeResources();

    //!
    //! \brief    Allocate Resources for SFC Pipe
    //! \details  Allocate the AVS and IEF line buffer surfaces for SFC
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Get width and height align unit of output format
    //! \param    [in] outputFormat
    //!           output format
    //! \param    [out] widthAlignUnit
    //!           width align unit
    //! \param    [out] heightAlignUnit
    //!           height align unit
    //! \return   void
    //!
    virtual void GetOutputWidthHeightAlignUnit(
        MOS_FORMAT outputFormat,
        uint16_t & widthAlignUnit,
        uint16_t & heightAlignUnit,
        bool       isInterlacedScaling);

    //!
    //! \brief    Get width and height align unit of input format
    //! \param    [in] inputFormat
    //!           input format
    //! \param    [in] outputFormat
    //!           output format
    //! \param    [out] widthAlignUnit
    //!           width align unit
    //! \param    [out] heightAlignUnit
    //!           height align unit
    //! \return   void
    //!
    virtual void GetInputWidthHeightAlignUnit(
        MOS_FORMAT              inputFormat,
        MOS_FORMAT              outputFormat,
        uint16_t                &widthAlignUnit,
        uint16_t                &heightAlignUnit,
        bool                    isInterlacedScaling = false);

    virtual bool IsOutputCapable(
        bool            isColorFill,
        PVPHAL_SURFACE  src,
        PVPHAL_SURFACE  renderTarget)
    {
        bool isOutputCapable = false;

        VPHAL_RENDER_CHK_NULL_NO_STATUS(src);
        VPHAL_RENDER_CHK_NULL_NO_STATUS(renderTarget);

        isOutputCapable = VphalSfcStateG12::IsOutputCapable(
            isColorFill,
            src,
            renderTarget);

        if (isColorFill &&
            (renderTarget->rcDst.top != 0 ||
             renderTarget->rcDst.left != 0))
        {
            isOutputCapable = false;
        }

finish:
        return isOutputCapable;
    }

    //!
    //! \brief    Check whether dithering Needed
    //! \details  Check whether dithering Needed
    //! \param    [in] formatInput
    //!           The input format
    //!           [in] formatOutput
    //!           The output format
    //! \return   bool
    //!
    bool IsDitheringNeeded(MOS_FORMAT formatInput, MOS_FORMAT formatOutput);

    VPHAL_SURFACE m_AVSLineBufferSurfaceSplit[MHW_SFC_MAX_PIPE_NUM_XE_XPM] = {}; //!< AVS Line Buffer Surface for SFC
    VPHAL_SURFACE m_IEFLineBufferSurfaceSplit[MHW_SFC_MAX_PIPE_NUM_XE_XPM] = {}; //!< IEF Line Buffer Surface for SFC

    bool m_disableSfcDithering = false;             //!< flag for dithering enable or disable, for debug purpose.
};

#endif // __VPHAL_SFC_SUPPORTED
#endif // __VPHAL_RENDER_SFC_XE_XPM_H__
