/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     media_mem_compression.h
//! \brief    Defines the common interface for mmc
//! \details
//!

#ifndef __MEDIA_MEM_COMPRESSION_H__
#define __MEDIA_MEM_COMPRESSION_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_utilities_common.h"
#include "media_mem_compression_next.h"
class MhwMiInterface;

class MediaMemComp : public MediaMemCompNext
{
public:
    //!
    //! \brief    Construct
    //!
    MediaMemComp(PMOS_INTERFACE osInterface, MhwMiInterface *miInterface);

protected:
    MhwMiInterface *m_mhwMiInterface = nullptr;
MEDIA_CLASS_DEFINE_END(MediaMemComp)
};

#endif //__MEDIA_MEM_COMPRESSION_H__
