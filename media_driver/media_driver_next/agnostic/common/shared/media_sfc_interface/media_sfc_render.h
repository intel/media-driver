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
//! \file     media_sfc_render.h
//! \brief    Common interface and structure used in sfc interface
//! \details  Common interface and structure used in sfc interface which are platform independent
//!
#ifndef __MEDIA_SFC_RENDER_H__
#define __MEDIA_SFC_RENDER_H__

#include "mos_os_specific.h"

namespace vp
{
class VpPlatformInterface;
class VpPipeline;
};
class MhwCpInterface;
class MhwMiInterface;
class MhwSfcInterface;
class MhwVeboxInterface;
struct _VPHAL_STATUS_TABLE;
class MediaVdboxSfcRender;
struct VEBOX_SFC_PARAMS;
struct VDBOX_SFC_PARAMS;
struct _RENDERHAL_INTERFACE;

class MediaSfcRender
{
public:
    //!
    //! \brief    MediaSfcRender constructor
    //! \details  Initialize the MediaSfcRender members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    MediaSfcRender(PMOS_INTERFACE osInterface);

    virtual ~MediaSfcRender();

    virtual void Destroy();

    MOS_STATUS Render(VEBOX_SFC_PARAMS &param);
    MOS_STATUS Render(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &param);
    //!
    //! \brief    BltState initialize
    //! \details  Initialize the BltState, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

protected:
    vp::VpPlatformInterface *m_vpPlatformInterface  = nullptr;
    vp::VpPipeline          *m_vpPipeline           = nullptr;
    _RENDERHAL_INTERFACE    *m_renderHal            = nullptr;
    MhwCpInterface          *m_cpInterface          = nullptr;
    MhwSfcInterface         *m_sfcInterface         = nullptr;
    MhwVeboxInterface       *m_veboxInterface       = nullptr;
    _VPHAL_STATUS_TABLE     *m_statusTable          = nullptr;
    PMOS_INTERFACE          m_osInterface           = nullptr;
    MediaVdboxSfcRender     *m_vdboxSfcRender       = nullptr;
    bool                    m_initialized           = false;
};

#endif // __MEDIA_SFC_RENDER_H__
