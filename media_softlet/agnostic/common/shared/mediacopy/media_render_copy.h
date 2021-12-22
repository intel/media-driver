/*
* Copyright (c) 2020, Intel Corporation
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
//! \file       render_copy.h
//! \brief      header file of render copy base functions
//! \details    define render copy basic API
//!
#ifndef __MEDIA_RENDER_COPY_H__
#define __MEDIA_RENDER_COPY_H__

#include "media_copy.h"
#include "vphal_render_common.h"

const VphalSseuSetting VpDefaultSSEUTable[baseKernelMaxNumID] =
{
    // Slice    Sub-Slice       EU      Rsvd(freq)
    {2,         3,              8,         0},
};

class RenderCopyState
{
public:
    RenderCopyState(PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces);

    virtual ~RenderCopyState();

    //!
    //! \brief    RenderCopyState initialize
    //! \details  Initialize the RenderCopyState, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

protected:

    //!
    //! \brief    Submit command
    //! \details  Submit render command
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD( );

public:
    PMOS_INTERFACE               m_osInterface     = nullptr;
    MhwInterfaces               *m_mhwInterfaces   = nullptr;
    MhwRenderInterface          *m_renderInterface = nullptr;
    RENDERHAL_INTERFACE         *m_renderHal       = nullptr;
    MhwCpInterface              *m_cpInterface     = nullptr;
    void                        *m_pKernelBin      = nullptr;
    Kdll_State                  *m_pKernelDllState = nullptr;//!< Kernel DLL state
};

#endif // __MEDIA_RENDER_COPY_H__
