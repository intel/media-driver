/* Copyright (c) 2020, Intel Corporation
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
//! \file     vp_render_sfc_xe_xpm_base.h
//! \brief    The header file of the base class of SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#ifndef __VP_RENDER_SFC_XE_XPM_BASE_H__
#define __VP_RENDER_SFC_XE_XPM_BASE_H__

#include "mhw_sfc_g12_X.h"
#include "vp_render_sfc_m12.h"

namespace vp {

class SfcRenderXe_Xpm_Base : public SfcRenderM12
{

public:
    SfcRenderXe_Xpm_Base(VP_MHWINTERFACE &vpMhwinterface, PVpAllocator &allocator, bool disbaleSfcDithering);
    virtual ~SfcRenderXe_Xpm_Base() override;
    virtual bool IsVdboxSfcInputFormatSupported(
        CODECHAL_STANDARD           codecStandard,
        MOS_FORMAT                  inputFormat) override;
    virtual bool IsVdboxSfcOutputFormatSupported(
        CODECHAL_STANDARD           codecStandard,
        MOS_FORMAT                  outputFormat,
        MOS_TILE_TYPE               tileType) override;
    virtual MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams) override;

    //!
    //! \brief    Set csc parameters
    //! \details  Set csc parameters
    //! \param    [in] cscParams
    //!           Csc parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetCSCParams(PSFC_CSC_PARAMS cscParams) override;

protected:
    virtual MOS_STATUS SetInterlacedScalingParams(PSFC_SCALING_PARAMS scalingParams);
    virtual MOS_STATUS InitSfcStateParams() override;

    virtual MOS_STATUS AllocateResources() override;
    virtual MOS_STATUS FreeResources() override;

    virtual bool IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat) override;
    virtual bool IsCscNeeded(SFC_CSC_PARAMS &cscParams) override;
    virtual MOS_STATUS SetMmcParams(PMOS_SURFACE renderTarget, bool isFormatMmcSupported, bool isMmcEnabled) override;

    VP_SURFACE *m_tempFieldSurface = nullptr;

MEDIA_CLASS_DEFINE_END(vp__SfcRenderXe_Xpm_Base)
};

}
#endif // !__VP_RENDER_SFC_XE_XPM_BASE_H__
