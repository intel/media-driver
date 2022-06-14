/*
* Copyright (c) 2020-2021, Intel Corporation
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
#ifndef __MEDIA_SFC_RENDER_LEGACY_H__
#define __MEDIA_SFC_RENDER_LEGACY_H__

#include "mos_os_specific.h"
#include "mhw_vebox_itf.h"
#include "mhw_sfc_itf.h"
#include "mhw_mi_itf.h"
#include "media_sfc_render.h"

namespace vp
{
class VpPlatformInterface;
class VpPipeline;
struct FeatureParamScaling;
};

class MediaSfcRenderLegacy : public MediaSfcRender
{
public:
    //!
    //! \brief    MediaSfcRender constructor
    //! \details  Initialize the MediaSfcRender members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \param    mode
    //!           [in] 1: VEBOX-SFC only, 2: VDBOX-SFC only, 3: Both VEBOX-SFC and VDBOX-SFC.
    //!
    MediaSfcRenderLegacy(PMOS_INTERFACE osInterface, MEDIA_SFC_INTERFACE_MODE mode, MediaMemComp *mmc);

    virtual ~MediaSfcRenderLegacy();

    virtual void Destroy() override;

    //!
    //! \brief    Check whether the Parameter for VDBOX-SFC supported
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::vdboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VDBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    virtual MOS_STATUS IsParameterSupported(VDBOX_SFC_PARAMS &param) override;

    //!
    //! \brief    Check whether the Parameter for VEBOX-SFC supported
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::veboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VEBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    virtual MOS_STATUS IsParameterSupported(VEBOX_SFC_PARAMS &param) override;

    //!
    //! \brief    MediaSfcInterface initialize
    //! \details  Initialize the MediaSfcInterface.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize() override;

protected:
    MhwSfcInterface   *m_sfcInterface   = nullptr;
    MhwVeboxInterface *m_veboxInterface = nullptr;

    MEDIA_CLASS_DEFINE_END(MediaSfcRenderLegacy)
};

#endif // __MEDIA_SFC_RENDER_H__
