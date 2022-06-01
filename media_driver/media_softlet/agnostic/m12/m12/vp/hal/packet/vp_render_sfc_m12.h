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
//! \file     vp_render_sfc_m12.h
//! \brief    The header file of the base class of SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#ifndef __VP_RENDER_SFC_M12_H__
#define __VP_RENDER_SFC_M12_H__

#include "mhw_sfc_g12_X.h"
#include "vp_render_sfc_base_legacy.h"

namespace vp {

class SfcRenderM12 : public SfcRenderBaseLegacy
{
public:
    SfcRenderM12(VP_MHWINTERFACE &vpMhwinterface, PVpAllocator &allocator, bool disbaleSfcDithering);
    virtual     ~SfcRenderM12();

    //!
    //! \brief    Setup SFC states and parameters
    //! \details  Setup SFC states and parameters including M12 SFC State
    //! \param    [in] targetSurface
    //!           Pointer to Output Surface
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupSfcState(
        PVP_SURFACE                     targetSurface);

    virtual MOS_STATUS AddSfcLock(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_LOCK_PARAMS           pSfcLockParams);

    //!
    //! \brief    Set sfc pipe selected with vebox
    //! \details  Set sfc pipe selected with vebox
    //! \param    [in] dwSfcIndex
    //!           Sfc pipe selected with vebox
    //! \param    [in] dwSfcCount
    //!           Sfc pipe num in total
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSfcPipe(
        uint32_t dwSfcIndex,
        uint32_t dwSfcCount);

protected:
    //!
    //! \brief    Initiazlize SFC State Parameters
    //! \details  Initiazlize SFC State Parameters
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitSfcStateParams();

    virtual MOS_STATUS SetSfcStateInputOrderingModeHcp(
        PMHW_SFC_STATE_PARAMS       sfcStateParams);

    virtual MOS_STATUS SetCodecPipeMode(CODECHAL_STANDARD codecStandard);

    virtual MOS_STATUS SetupScalabilityParams();

    virtual bool IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat);
    virtual bool IsCscNeeded(SFC_CSC_PARAMS &cscParams);
MEDIA_CLASS_DEFINE_END(vp__SfcRenderM12)
};

}
#endif // !__VP_RENDER_SFC_M12_H__
