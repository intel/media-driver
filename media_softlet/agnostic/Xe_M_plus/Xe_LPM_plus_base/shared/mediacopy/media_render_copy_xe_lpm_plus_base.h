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
//! \file       media_render_copy_xe_lpm_plus_base.h
//! \brief      render copy's implement file.
//! \details    render copy's implement file.
//!
#ifndef __MEDIA_RENDER_COPY_XE_LPM_PLUS_H__
#define __MEDIA_RENDER_COPY_XE_LPM_PLUS_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "media_render_copy_next.h"
class MhwInterfacesNext;

typedef class  RenderCopyXe_LPM_Plus_Base  *PRenderCopyXe_LPM_Plus_Base;
class RenderCopyXe_LPM_Plus_Base: public RenderCopyStateNext
{
public:
    RenderCopyXe_LPM_Plus_Base(PMOS_INTERFACE  osInterface, MhwInterfacesNext  *mhwInterfaces);

    virtual ~RenderCopyXe_LPM_Plus_Base();

MEDIA_CLASS_DEFINE_END(RenderCopyXe_LPM_Plus_Base)
};

#endif // __MEDIA_RENDER_COPY_XE_HPM_H__
