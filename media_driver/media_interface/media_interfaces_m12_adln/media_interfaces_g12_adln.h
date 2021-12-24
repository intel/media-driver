/*
* Copyright (c) 2011-2022, Intel Corporation
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
//! \file     media_interfaces_gen12_adln.h
//! \brief    All interfaces used for Gen12 ADL-N that require factory creation
//!

#ifndef __MEDIA_INTERFACES_G12_adln_H__
#define __MEDIA_INTERFACES_G12_adln_H__


#include "media_interfaces_g12_tgllp.h"
#include "renderhal_g12_base.h"


class CMHalInterfacesG12Adln : public CMHalDevice
{
protected:
    using CMHal = CM_HAL_G12_X;
    MOS_STATUS Initialize(
        CM_HAL_STATE *pCmState);
};

#endif // __MEDIA_INTERFACES_G12_adln_H__

