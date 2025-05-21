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
#ifndef __MEDIA_SFC_RENDER_H__
#define __MEDIA_SFC_RENDER_H__

#include "mos_os_specific.h"
#include "mhw_vebox_itf.h"
#include "mhw_sfc_itf.h"
#include "mhw_mi_itf.h"

namespace vp
{
class VpPlatformInterface;
class VpPipeline;
struct FeatureParamScaling;
};
struct _VP_MHWINTERFACE;
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
    //! \param    mode
    //!           [in] 1: VEBOX-SFC only, 2: VDBOX-SFC only, 3: Both VEBOX-SFC and VDBOX-SFC.
    //!
    MediaSfcRender(PMOS_INTERFACE osInterface, MEDIA_SFC_INTERFACE_MODE mode, MediaMemComp *mmc);

    virtual ~MediaSfcRender();

    virtual void Destroy();

    //!
    //! \brief    Check whether the Parameter for VDBOX-SFC supported
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::vdboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VDBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    virtual MOS_STATUS IsParameterSupported(VDBOX_SFC_PARAMS &param);

    //!
    //! \brief    Check whether the Parameter for VEBOX-SFC supported
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::veboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VEBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    virtual MOS_STATUS IsParameterSupported(VEBOX_SFC_PARAMS &param);

    //!
    //! \brief    Render Vdbox-SFC States
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::vdboxSfcEnabled being 1.
    //! \param    cmdBuffer
    //!           [in/out] Command Buffer to be Filled.
    //! \param    param
    //!           [in] Pointer to VDBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    MOS_STATUS Render(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &param);

    //!
    //! \brief    Render Vebox-SFC States
    //! \details  Only valid when MEDIA_SFC_INTERFACE_MODE::veboxSfcEnabled being 1.
    //! \param    param
    //!           [in] Pointer to VEBOX_SFC_PARAMS.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if supported, otherwise failed
    //!
    MOS_STATUS Render(VEBOX_SFC_PARAMS &param);

    //!
    //! \brief    Sfc Command Size
    //! \details  Calculate Command size of SFC commands.
    //! \return   uint32_t
    //!           Return calculated size
    //!
    uint32_t GetSfcCommandSize()
    {
        return m_sfcItf->MHW_GETSIZE_F(SFC_LOCK)() +
               m_sfcItf->MHW_GETSIZE_F(SFC_STATE)() +
               m_sfcItf->MHW_GETSIZE_F(SFC_AVS_STATE)() +
               m_sfcItf->MHW_GETSIZE_F(SFC_AVS_LUMA_Coeff_Table)() +
               m_sfcItf->MHW_GETSIZE_F(SFC_AVS_CHROMA_Coeff_Table)() +
               m_sfcItf->MHW_GETSIZE_F(SFC_IEF_STATE)() +
               m_sfcItf->MHW_GETSIZE_F(SFC_FRAME_START)();
    }

    //!
    //! \brief    MediaSfcInterface initialize
    //! \details  Initialize the MediaSfcInterface.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    bool IsInitialized() { return m_initialized; }

protected:
    MOS_STATUS InitScalingParams(vp::FeatureParamScaling &scalingParams, VDBOX_SFC_PARAMS &sfcParam);
    MOS_STATUS InitScalingParams(vp::FeatureParamScaling &scalingParams, VEBOX_SFC_PARAMS &sfcParam);

    _VP_MHWINTERFACE        *m_vpMhwinterface       = nullptr;
    vp::VpPlatformInterface *m_vpPlatformInterface  = nullptr;
    vp::VpPipeline          *m_vpPipeline           = nullptr;
    _RENDERHAL_INTERFACE    *m_renderHal            = nullptr;
    MhwCpInterface          *m_cpInterface          = nullptr;
    _VPHAL_STATUS_TABLE     *m_statusTable          = nullptr;
    PMOS_INTERFACE          m_osInterface           = nullptr;
    MediaVdboxSfcRender     *m_vdboxSfcRender       = nullptr;
    bool                    m_initialized           = false;
    MEDIA_SFC_INTERFACE_MODE m_mode                 = {};
    MediaMemComp            *m_mmc                  = nullptr;
    std::shared_ptr<mhw::vebox::Itf> m_veboxItf     = nullptr;
    std::shared_ptr<mhw::sfc::Itf>   m_sfcItf       = nullptr;
    std::shared_ptr<mhw::mi::Itf>    m_miItf        = nullptr;
MEDIA_CLASS_DEFINE_END(MediaSfcRender)
};

#endif // __MEDIA_SFC_RENDER_H__
