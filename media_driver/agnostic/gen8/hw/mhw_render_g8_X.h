/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_render_g8_X.h
//! \brief    Defines functions for constructing  render engine commands on Gen8-based platforms
//!

#ifndef __MHW_RENDER_G8_X_H__
#define __MHW_RENDER_G8_X_H__

#include "mhw_render_generic.h"
#include "mhw_render_hwcmd_g8_X.h"

struct MhwRenderInterfaceG8 : public MhwRenderInterfaceGeneric<mhw_render_g8_X>
{
    MhwRenderInterfaceG8(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested) :
        MhwRenderInterfaceGeneric(miInterface, osInterface, gtSystemInfo, newStateHeapManagerRequested)
    {
        MHW_FUNCTION_ENTER;

        if (gtSystemInfo == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid input pointer provided");
            return;
        }

        if (gtSystemInfo->EUCount <= 24)
        {
            // 64 is the maximum number of URBs for not GT3
            m_hwCaps.dwMaxURBEntries = 64;
        }

        // SLM     URB     DC      RO     Rest
        // 0      256      0       0      512 (KB chunks based on GT2)
        m_l3CacheCntlRegisterValueDefault = 0x80000040;
    }

    virtual ~MhwRenderInterfaceG8() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params);

    MOS_STATUS AddMediaObject(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_OBJECT_PARAMS        params);

    MOS_STATUS AddMediaObjectWalkerCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_WALKER_PARAMS              params);

    MOS_STATUS AddPaletteLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_PALETTE_PARAMS             params);

    MOS_STATUS AddGpgpuCsrBaseAddrCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMOS_RESOURCE                   csrResource);

    MOS_STATUS EnableL3Caching(
        PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS    cacheSettings);

    MOS_STATUS SetL3Cache(
        PMOS_COMMAND_BUFFER             cmdBuffer );

    MHW_RENDER_ENGINE_L3_CACHE_CONFIG* GetL3CacheConfig() { return &m_l3CacheConfig; }
};

#endif
