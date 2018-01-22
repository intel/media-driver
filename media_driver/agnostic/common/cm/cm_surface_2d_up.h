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
//! \file      cm_surface_2d_up.h
//! \brief     Contains Class CmSurface2DUP declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSURFACE2DUP_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSURFACE2DUP_H_

#include "cm_def.h"

namespace CMRT_UMD
{

//! \brief      CmSurface2DUP Class to manage 2D surface created upon user
//!             provided system memory.
//! \details    Each CmSurface2DUP object is associated with a SurfaceIndex
//!             object containing a unique index value the surface is mapped to
//!             when created by the CmDevice. The CmDevice keeps the mapping b/w
//!             index and CmSurface2DUP. The SurfaceIndex is passed to CM kernel
//!             function (genx_main) as argument to indicate the surface. CPU can
//!             access the system memory through the memory point. GPU can access
//!             the 2D surface through SurfaceIndex in kernel. It is application's
//!             responsibility to make sure the accesses from both sides are not
//!             overlapped.
class CmSurface2DUP
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated
    //!             with the surface.
    //! \param      [out] index
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex* &index) = 0;

    //!
    //! \brief      Selects one of the pre-defined memory object control
    //!             settings for this surface.
    //! \details    This API is platform related for SKL and plus platforms.
    //! \param      [in] memCtrl
    //!             The selected pre-defined memory object control setting.
    //! \retval     CM_SUCCESS if the memory object control is set successfully.
    //! \retval     CM_FAILURE if the memory object control is not set
    //!             correctly.
    //! \note       This function is only implemented for hardware and simulation mode, .
    //!
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl) = 0;

    //!
    //! \brief      Set surface property for interlace usage.
    //! \details    A surface can be set as top filed, bottom filed for
    //!             interlace usage.
    //! \param      [in] frameType
    //!             It specifies type for surface, it has three
    //!             types:CM_FRAME,CM_TOP_FIELD, CM_BOTTOM_FIELD.
    //! \returns    CM_SUCCESS.
    //! \note       By default, the surface property is CM_FRAME type which means
    //!             its content is progressive data, not interleave data.
    //!
    CM_RT_API virtual int32_t SetProperty(CM_FRAME_TYPE frameType) = 0;
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSURFACE2DUP_H_
