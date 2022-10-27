/*===================== begin_copyright_notice ==================================
# Copyright (c) 2022, Intel Corporation
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
//! \file      codechal_hw_next_xe_xpm_plus.h
//! \brief     This modules implements HW interface layer to be used on XeHP platforms on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_NEXT_XE_XPM_PLUS_H__
#define __CODECHAL_HW_NEXT_XE_XPM_PLUS_H__

#include "codec_hw_next.h"
#include "mhw_render_legacy.h"

//!  Codechal hw interface xe xpm plus
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen12 platforms
*/
class CodechalHwInterfaceNextXe_Xpm_Plus : public CodechalHwInterfaceNext
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceNextXe_Xpm_Plus(
        PMOS_INTERFACE     osInterface,
        CODECHAL_FUNCTION  codecFunction,
        MhwInterfacesNext *mhwInterfacesNext,
        bool               disableScalability);

    //!
    //! \brief    Copy constructor
    //!
    CodechalHwInterfaceNextXe_Xpm_Plus(const CodechalHwInterfaceNextXe_Xpm_Plus &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalHwInterfaceNextXe_Xpm_Plus &operator=(const CodechalHwInterfaceNextXe_Xpm_Plus &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceNextXe_Xpm_Plus();

    virtual bool UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard)
    {
        // Render Engine is not not supported
        return false;
    }

private:
    //!
    //! \brief    Called by constructor
    //!
    void PrepareCmdSize(CODECHAL_FUNCTION codecFunction);

    std::shared_ptr<MhwMiInterface> m_miInterface     = nullptr;
    MhwRenderInterface             *m_renderInterface = nullptr;

MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceNextXe_Xpm_Plus)
};

#endif  // __CODECHAL_HW_NEXT_XE_XPM_PLUS_H__