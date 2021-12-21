/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     renderhal_memdecomp_xe_xpm_plus.h
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details
//!

#ifndef __RENDERHAL_MEMDECOMP_XE_XPM_PLUS_H__
#define __RENDERHAL_MEMDECOMP_XE_XPM_PLUS_H__

#include "mhw_render.h"
#include "mos_os.h"
#include "mediamemdecomp.h"
#include "renderhal.h"

//!
//! \class MediaRenderDecompState
//! \brief Media render inplace memory decompression state. This class defines the member fields
//!        functions etc used by memory decompression.
//!
class MediaRenderDecompStateXe_Xpm_Plus : public MediaMemDecompBaseState
{
public:

    //!
    //! \brief    Constructor, initiallize
    //!
    MediaRenderDecompStateXe_Xpm_Plus() {}

    //!
    //! \brief    Copy constructor
    //!
    MediaRenderDecompStateXe_Xpm_Plus(const MediaRenderDecompStateXe_Xpm_Plus &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaRenderDecompStateXe_Xpm_Plus &operator=(const MediaRenderDecompStateXe_Xpm_Plus &) = delete;

    MOS_STATUS Initialize(
        PMOS_INTERFACE                  osInterface,
        MhwCpInterface                  *cpInterface,
        PMHW_MI_INTERFACE               mhwMiInterface,
        MhwRenderInterface              *renderInterface);

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaRenderDecompStateXe_Xpm_Plus() {}

    virtual MOS_STATUS MemoryDecompress(
        PMOS_RESOURCE targetResource) { return MOS_STATUS_SUCCESS; }

    virtual PMOS_INTERFACE GetDecompStateMosInterface() { return nullptr; }
};

#endif // __RENDERHAL_MEMDECOMP_XE_XPM_PLUS_H__
