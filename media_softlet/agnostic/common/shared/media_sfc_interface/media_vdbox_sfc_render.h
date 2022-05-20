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
//! \file     media_vdbox_sfc_render.h
//! \brief    Common interface and structure used in sfc interface
//! \details  Common interface and structure used in sfc interface which are platform independent
//!
#ifndef __MEDIA_VDBOX_SFC_RENDER_H__
#define __MEDIA_VDBOX_SFC_RENDER_H__

#include "mos_os_specific.h"
#include "mhw_vebox.h"
#include "vp_common.h"
#include "vp_platform_interface.h"
#include "vp_pipeline.h"

class VPMediaMemComp;
namespace vp
{
    class VpAllocator;
    class SfcRenderBase;
    class VpCscFilter;
    class VpScalingFilter;
    class VpRotMirFilter;
};

class MediaVdboxSfcRender
{
public:
    //!
    //! \brief    MediaVdboxSfcRender constructor
    //!
    MediaVdboxSfcRender();

    //!
    //! \brief    MediaVdboxSfcRender deconstructor
    //!
    virtual ~MediaVdboxSfcRender();

    //!
    //! \brief    Destroy MediaVdboxSfcRender variables
    //!
    virtual void Destroy();

    //!
    //! \brief    MediaVdboxSfcRender constructor
    //! \details  Initialize the MediaVdboxSfcRender members.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize(VP_MHWINTERFACE &vpMhwinterface, MediaMemComp *mmc);

    //!
    //! \brief    Add sfc states to command buffer
    //! \details  Add sfc states to command buffer according to sfcParam.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AddSfcStates(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &sfcParam);

    //!
    //! \brief    Check whether VDBOX-SFC Format Supported
    //! \details  Check whether VDBOX-SFC Format Supported.
    //! \param    codecStandard
    //!           [in] Codec Standard.
    //! \param    inputFormat
    //!           [in] Format of Input Frame
    //! \param    outputFormat
    //!           [in] Format of Output Frame
    //! \return   bool
    //!           Return true if supported, otherwise failed
    //!
    bool IsVdboxSfcFormatSupported(
        CODECHAL_STANDARD           codecStandard,
        MOS_FORMAT                  inputFormat,
        MOS_FORMAT                  outputFormat,
        MOS_TILE_TYPE               tileType);

protected:
    MOS_STATUS SetCSCParams(VDBOX_SFC_PARAMS &sfcParam, VP_EXECUTE_CAPS &vpExecuteCaps);
    MOS_STATUS SetScalingParams(VDBOX_SFC_PARAMS &sfcParam, VP_EXECUTE_CAPS &vpExecuteCaps);
    MOS_STATUS SetRotMirParams(VDBOX_SFC_PARAMS &sfcParam, VP_EXECUTE_CAPS &vpExecuteCaps);
    MOS_STATUS SetHistogramParams(VDBOX_SFC_PARAMS &sfcParam);
    MOS_STATUS SetSfcMmcParams(VDBOX_SFC_PARAMS &sfcParam);

    VPHAL_SCALING_MODE GetScalingMode(CODECHAL_SCALING_MODE scalingMode);

    VP_MHWINTERFACE         m_vpMhwInterface        = {};
    PMOS_INTERFACE          m_osInterface           = nullptr;
    vp::VpAllocator         *m_allocator            = nullptr;
    MediaMemComp            *m_mmc                  = nullptr;
    vp::SfcRenderBase       *m_sfcRender            = nullptr;
    vp::VpCscFilter         *m_cscFilter            = nullptr;
    vp::VpScalingFilter     *m_scalingFilter        = nullptr;
    vp::VpRotMirFilter      *m_rotMirFilter         = nullptr;
    bool                    m_isMmcAllocated        = false;
MEDIA_CLASS_DEFINE_END(MediaVdboxSfcRender)
};

#endif // __MEDIA_VDBOX_SFC_RENDER_H__
