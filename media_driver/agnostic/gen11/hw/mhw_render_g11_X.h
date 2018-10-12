/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     mhw_render_g11_X.h
//! \brief    Defines functions for constructing  render engine commands on Gen11-based platforms
//!

#ifndef __MHW_RENDER_G11_X_H__
#define __MHW_RENDER_G11_X_H__

#include "mhw_render_generic.h"
#include "mhw_render_hwcmd_g11_X.h"

typedef struct MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G11 : MHW_RENDER_ENGINE_L3_CACHE_SETTINGS
{
    uint32_t   dwTcCntlReg = 0;
} *PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G11;

typedef struct MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G11 : MHW_RENDER_ENGINE_L3_CACHE_CONFIG
{
    uint32_t   dwL3CacheTcCntlReg_Register = 0;
    uint32_t   dwL3CacheTcCntlReg_Setting  = 0;
} *PMHW_RENDER_ENGINE_L3_CACHE_CONFIG_G11;

class MhwRenderInterfaceG11 : public MhwRenderInterfaceGeneric<mhw_render_g11_X>
{
public:
    MhwRenderInterfaceG11(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested) :
        MhwRenderInterfaceGeneric(miInterface, osInterface, gtSystemInfo, newStateHeapManagerRequested)
    {
        MHW_FUNCTION_ENTER;

        // L3 Cache size per bank = 384 KB.
        // {SLM,    URB,     DC,      RO(I/S, C, T),   L3 Client Pool}
        // {  0,     64,      0,                  0,         320     }
        m_l3CacheCntlRegisterValueDefault = 0xA0000420;
    }

    virtual ~MhwRenderInterfaceG11() { MHW_FUNCTION_ENTER; }

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params);

    MOS_STATUS AddPipelineSelectCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        bool                            gpGpuPipe);

    MOS_STATUS AddMediaObject(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_OBJECT_PARAMS        params);

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

protected:
    MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G11   m_l3CacheConfig;

    uint32_t    m_l3CacheTcCntlRegisterOffset = 0xB0A4;
    uint32_t    m_l3CacheTcCntlRegisterValueDefault = 0x0000000D;
};



#endif
