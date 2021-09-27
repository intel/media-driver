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
//! \file      renderhal_platform_interface_next.h 
//! \brief     abstract the platfrom specific APIs into one class 
//!
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__
#define __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__

#include "mos_os.h"
#include "renderhal.h"
#include "renderhal_platform_interface.h"
#include "mhw_render_itf.h"
#include "mhw_render_cmdpar.h"
#include "media_packet.h"
#include "vp_utils.h"
#include "media_feature_manager.h"

class XRenderHal_Platform_Interface_Next : public XRenderHal_Platform_Interface, public mhw::render::Itf::ParSetting
{
public:
    XRenderHal_Platform_Interface_Next() {}
    virtual ~XRenderHal_Platform_Interface_Next() {}

    MOS_STATUS AddPipelineSelectCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        bool                        gpGpuPipe);

    MOS_STATUS SendStateBaseAddress(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS AddSipStateCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS AddCfeStateCmd(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_VFE_PARAMS             params);

    MOS_STATUS SendChromaKey(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS SendPalette(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS SetL3Cache(
        PRENDERHAL_INTERFACE        pRenderHal,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    PMHW_MI_MMIOREGISTERS GetMmioRegisters(
        PRENDERHAL_INTERFACE        pRenderHal);

    MOS_STATUS EnablePreemption(
        PRENDERHAL_INTERFACE            pRenderHal,
        PMOS_COMMAND_BUFFER             pCmdBuffer);

protected:

    PRENDERHAL_INTERFACE              m_renderHal = nullptr;
    MediaFeatureManager               *m_featureManager = nullptr;
    PMHW_GPGPU_WALKER_PARAMS          m_gpgpuWalkerParams = nullptr;
    PMHW_ID_ENTRY_PARAMS              m_interfaceDescriptorParams = nullptr;
    std::shared_ptr<mhw::render::Itf> m_renderItf = nullptr;

};

#endif // __RENDERHAL_PLATFORM_INTERFACE_NEXT_H__
