/*===================== begin_copyright_notice ==================================

* Copyright (c) 2024, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file       renderhal_xe2_hpg_next.h
//! \brief      header file of xe_hpg hardware functions
//! \details    Gen12 hardware functions declare
//!
#ifndef __RENDERHAL_XE2_HPG_NEXT_H__
#define __RENDERHAL_XE2_HPG_NEXT_H__

#include "renderhal_xe_hpg_base.h"

class XRenderHal_Interface_Xe2_Hpg_Next : public XRenderHal_Interface_Xe_Hpg_Base
{
public:
    XRenderHal_Interface_Xe2_Hpg_Next() : XRenderHal_Interface_Xe_Hpg_Base()
    {

    }

    virtual ~XRenderHal_Interface_Xe2_Hpg_Next() {}

    //!
    //! \brief    Get Render Engine MMC Enable/Disable Flag. Actually the same as Xe_HPG
    //! \param    [in] pRenderHal
    //!           Pointer to Hardware Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsRenderHalMMCEnabled(
        PRENDERHAL_INTERFACE pRenderHal);

    //! \brief    Send To 3DState Binding Table Pool Alloc
    //! \details  Send To 3DState Binding Table Pool Alloc
    //! \param    PRENDERHAL_INTERFACE pRenderHal
    //!           [in] Pointer to RenderHal Interface Structure
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    virtual MOS_STATUS SendTo3DStateBindingTablePoolAlloc(
        PRENDERHAL_INTERFACE pRenderHal,
        PMOS_COMMAND_BUFFER  pCmdBuffer);

    virtual bool IsL8FormatSupported()
    {
        return false;
    }

    //!
    //! \brief    Initialize the State Heap Settings per platform
    //! \param    PRENDERHAL_INTERFACE    pRenderHal
    //!           [out] Pointer to PRENDERHAL_INTERFACE
    //! \return   void
    //!
    virtual void InitStateHeapSettings(
        PRENDERHAL_INTERFACE pRenderHal) override;

MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe2_Hpg_Next)
};

#endif // __RENDERHAL_XE_HPG_NEXT_H__
