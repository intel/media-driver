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

======================= end_copyright_notice ==================================*/
//!
//! \file       renderhal_xe_hp.h
//! \brief      header file of Xe_HP hardware functions
//! \details    Gen12 hardware functions declare
//!
#ifndef __RENDERHAL_XE_HP_H__
#define __RENDERHAL_XE_HP_H__

#include "media_class_trace.h"
#include "renderhal_xe_hp_base.h"

class XRenderHal_Interface_Xe_Hp : public XRenderHal_Interface_Xe_Hp_Base
{
public:
    XRenderHal_Interface_Xe_Hp() : XRenderHal_Interface_Xe_Hp_Base()
    {

    }
    
    virtual ~XRenderHal_Interface_Xe_Hp() {}
MEDIA_CLASS_DEFINE_END(XRenderHal_Interface_Xe_Hp)
};

#endif // __RENDERHAL_XE_HP_H__
