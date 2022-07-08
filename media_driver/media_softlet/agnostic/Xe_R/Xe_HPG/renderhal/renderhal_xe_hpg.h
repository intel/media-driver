/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file       renderhal_xe_hpg.h
//! \brief      header file of DG2 hardware functions
//! \details    Gen12 hardware functions declare
//!
#ifndef __RENDERHAL_XE_HPG_H__
#define __RENDERHAL_XE_HPG_H__

#include "renderhal_xe_hp_base.h"
#include "media_class_trace.h"
#include "mhw_render.h"
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "renderhal.h"

class XRenderHal_Interface_Xe_Hpg : public XRenderHal_Interface_Xe_Hp_Base
{
public:
    XRenderHal_Interface_Xe_Hpg() : XRenderHal_Interface_Xe_Hp_Base()
    {

    }

    virtual ~XRenderHal_Interface_Xe_Hpg() {}

    //!
    //! \brief    Get Render Engine MMC Enable/Disable Flag
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsRenderHalMMCEnabled(
        PRENDERHAL_INTERFACE pRenderHal);

    //! \brief    Send Compute Walker
    //! \details  Send Compute Walker
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to Hardware Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
    //!           [in]    Pointer to GPGPU walker parameters
    //! \return   MOS_STATUS
    virtual MOS_STATUS SendComputeWalker(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams);

    //!
    //! \brief    Initialize the State Heap Settings per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    void InitStateHeapSettings(
        PRENDERHAL_INTERFACE    pRenderHal);
MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe_Hpg)
};

#endif // __RENDERHAL_XE_HPG_H__
