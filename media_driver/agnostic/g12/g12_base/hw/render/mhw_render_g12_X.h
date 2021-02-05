/*
* Copyright (c) 2015-2020, Intel Corporation
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
//! \file     mhw_render_g12_X.h
//! \brief    Defines functions for constructing  render engine commands on Gen12-based platforms
//!

#ifndef __MHW_RENDER_G12_X_H__
#define __MHW_RENDER_G12_X_H__

#include "mhw_render_generic.h"
#include "mhw_render_hwcmd_g12_X.h"
#include "mhw_mmio_g12.h"
#include "mhw_state_heap_g12.h"

typedef struct MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12 : MHW_RENDER_ENGINE_L3_CACHE_SETTINGS
{
    uint32_t   dwTcCntlReg = 0;
    uint32_t   dwAllocReg  = 0;
    bool       bUpdateDefault = 0;
} *PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12;

typedef struct MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12 : MHW_RENDER_ENGINE_L3_CACHE_CONFIG
{
    uint32_t   dwRcsL3CacheTcCntlReg_Register = 0;
    uint32_t   dwL3CacheTcCntlReg_Setting  = 0;
    uint32_t   dwRcsL3CacheAllocReg_Register  = 0;
    uint32_t   dwL3CacheAllocReg_Setting   = 0;
    uint32_t   dwCcs0L3CacheTcCntlReg_Register = 0;
    uint32_t   dwCcs0L3CacheAllocReg_Register = 0;
} *PMHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12;

struct MHW_VFE_PARAMS_G12 : MHW_VFE_PARAMS
{
    bool  bFusedEuDispatch = 0;
    uint32_t numOfWalkers = 0;
    bool  enableSingleSliceDispatchCcsMode = 0;

    // Surface state offset of scratch space buffer.
    uint32_t scratchStateOffset = 0;
};

class MhwRenderInterfaceG12 : public MhwRenderInterfaceGeneric<mhw_render_g12_X>
{
public:
    MhwRenderInterfaceG12(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested) :
        MhwRenderInterfaceGeneric(miInterface, osInterface, gtSystemInfo, newStateHeapManagerRequested)
    {
        MHW_FUNCTION_ENTER;

        m_l3CacheCntlRegisterValueDefault = 0x80000080;

        InitMmioRegisters();
    }

    virtual ~MhwRenderInterfaceG12() { MHW_FUNCTION_ENTER; }

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
        PMHW_PALETTE_PARAMS             params)
    {
        MHW_ASSERTMESSAGE("PALETTE_ENTRY is not supported!");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddGpgpuCsrBaseAddrCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMOS_RESOURCE                   csrResource);

    MOS_STATUS EnableL3Caching(
        PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS    cacheSettings);

    MOS_STATUS SetL3Cache(
        PMOS_COMMAND_BUFFER             cmdBuffer );

    MHW_RENDER_ENGINE_L3_CACHE_CONFIG* GetL3CacheConfig() { return &m_l3CacheConfig; }

    virtual PMHW_MI_MMIOREGISTERS GetMmioRegisters()
    {
        return &m_mmioRegisters;
    }

    //!
    //! \brief    Get AVS sampler state Inc unit
    //! \details  Get AVS sampler state Inc unit
    //! \return   [out] uint32_t
    //!           AVS sampler unit.
    virtual uint32_t GetSamplerStateAVSIncUnit() { return MHW_SAMPLER_STATE_AVS_INC_G12; }

    //!
    //! \brief    Get Conv sampler state Inc unit
    //! \details  Get Conv sampler state Inc unit
    //! \return   [out] uint32_t
    //!           Conv sampler unit.
    virtual uint32_t GetSamplerStateConvIncUnit() { return MHW_SAMPLER_STATE_CONV_INC_G12; }

    //!
    //! \brief    Get the sampler height and width align unit
    //! \details  NV12 format needs the width and height to be a multiple of some unit
    //! \param    [in] bool
    //!           true if AVS sampler, false otherwise
    //! \param    [in, out] uint32_t
    //!           weight align unit
    //! \param    [in, out] uint32_t
    //!           height align unit
    virtual void GetSamplerResolutionAlignUnit(bool isAVSSampler, uint32_t &widthAlignUnit, uint32_t &heightAlignUnit)
    {
        // enable 2 plane NV12 when width is not multiple of 2 or height is
        // not multiple of 4. For AVS sampler, no limitation for 4 alignment.
        widthAlignUnit  = isAVSSampler ? MHW_AVS_SAMPLER_WIDTH_ALIGN_UNIT : MHW_SAMPLER_WIDTH_ALIGN_UNIT_G12;
        heightAlignUnit = isAVSSampler ? MHW_AVS_SAMPLER_HEIGHT_ALIGN_UNIT : MHW_SAMPLER_HEIGHT_ALIGN_UNIT_G12;
    }

protected:
    MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12   m_l3CacheConfig;

    uint32_t    m_l3CacheTcCntlRegisterValueDefault = 0;
    uint32_t    m_l3CacheAllocRegisterValueDefault = 0xD0000020;

private:
    //! \brief Mmio registers address
    MHW_MI_MMIOREGISTERS    m_mmioRegisters = {};
    void InitMmioRegisters();
};
#endif
