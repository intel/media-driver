/*===================== begin_copyright_notice ==================================

* Copyright (c) 2022 Intel Corporation
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
//! \file       media_render_copy_xe_hpm.h
//! \brief      render copy's implement file.
//! \details    render copy's implement file.
//!
#ifndef __MEDIA_RENDER_COPY_XE_HPM_H__
#define __MEDIA_RENDER_COPY_XE_HPM_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "media_render_copy.h"
#include "mhw_render_legacy.h"
class MhwInterfaces;

typedef class  RenderCopy_Xe_Hpm  *PRenderCopy_Xe_Hpm;
class RenderCopy_Xe_Hpm: public RenderCopyState
{
public:
    RenderCopy_Xe_Hpm(PMOS_INTERFACE  osInterface, MhwInterfaces  *mhwInterfaces);

    virtual ~RenderCopy_Xe_Hpm();


     //!
    //! \brief    Copy input surface to Output surface
    //! \details  Copy 2D surface to 2D surface
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [in] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopySurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

    //!
    //! \brief    Render copy omputer walker setup
    //! \details  Computer walker setup for render copy
    //! \param    PMHW_WALKER_PARAMS pWalkerParams
    //!           [in/out] Pointer to Walker params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
   virtual MOS_STATUS RenderCopyComputerWalker(
     PMHW_GPGPU_WALKER_PARAMS    pWalkerParams);

protected:

    //!
    //! \brief    Submit command
    //! \details  Submit render command
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD( );

    MOS_STATUS SetupKernel(int32_t iKDTIndex);

MEDIA_CLASS_DEFINE_END(RenderCopy_Xe_Hpm)
};

#endif // __MEDIA_RENDER_COPY_XE_HPM_H__
