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
//! \file     mhw_render_itf.h
//! \brief    MHW RENDER interface common base
//! \details
//!

#ifndef __MHW_RENDER_ITF_H__
#define __MHW_RENDER_ITF_H__

#include "mhw_itf.h"
#include "mhw_render_cmdpar.h"
#include "mhw_mi_itf.h"

#define _RENDER_CMD_DEF(DEF)                \
    DEF(PIPELINE_SELECT);                   \
    DEF(STATE_BASE_ADDRESS);                \
    DEF(_3DSTATE_CHROMA_KEY);               \
    DEF(PALETTE_ENTRY);                     \
    DEF(STATE_SIP);                         \
    DEF(GPGPU_CSR_BASE_ADDRESS);            \
    DEF(_3DSTATE_BINDING_TABLE_POOL_ALLOC); \
    DEF(CFE_STATE);                         \
    DEF(COMPUTE_WALKER);                    \
    DEF(STATE_COMPUTE_MODE)

namespace mhw
{
namespace render
{
class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;
        _RENDER_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS EnableL3Caching(MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *cacheSettings) = 0;
    // legacy MHW interface will be removed for another pr
    virtual MOS_STATUS SetL3Cache(PMOS_COMMAND_BUFFER cmdBuffer, std::shared_ptr<mhw::mi::Itf> miItf) = 0;

    virtual MOS_STATUS EnablePreemption(PMOS_COMMAND_BUFFER cmdBuffer, std::shared_ptr<mhw::mi::Itf> miItf) = 0;

    virtual MOS_STATUS InitMmioRegisters() = 0;

    virtual PMHW_MI_MMIOREGISTERS GetMmioRegisters() = 0;

    virtual MOS_STATUS AllocateHeaps(MHW_STATE_HEAP_SETTINGS stateHeapSettings) = 0;

    virtual PMHW_STATE_HEAP_INTERFACE GetStateHeapInterface() = 0;

    virtual bool IsPreemptionEnabled() =0;

    virtual void GetSamplerResolutionAlignUnit(bool isAVSSampler, uint32_t &widthAlignUnit, uint32_t &heightAlignUnit) = 0;

    virtual MHW_RENDER_ENGINE_CAPS* GetHwCaps() = 0;

    virtual MHW_RENDER_ENGINE_L3_CACHE_CONFIG* GetL3CacheConfig() = 0;

    virtual MOS_STATUS SetupInlineData() = 0;

    _RENDER_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
MEDIA_CLASS_DEFINE_END(mhw__render__Itf)
};
}  // namespace render
}  // namespace mhw
#endif  // __MHW_RENDER_ITF_H__
