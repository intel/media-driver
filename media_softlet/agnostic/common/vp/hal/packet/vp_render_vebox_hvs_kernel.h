/*
* Copyright (c) 2022, Intel Corporation
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
#ifndef __VP_RENDER_VEBOX_HVS_KERNEL_H__
#define __VP_RENDER_VEBOX_HVS_KERNEL_H__

#include "vp_render_cmd_packet.h"
namespace vp
{
// Defined in "VEBOX HVS Kernel"
struct VEBOX_HVS_STATIC_DATA
{
    union
    {
        struct
        {
            uint32_t hvsSurface;  
        };
        uint32_t Value;
    } DW00;

    union
    {
        struct
        {
            uint16_t hvsMode;
            uint16_t hvsFormat;
        };
        uint32_t Value;
    } DW01;

    union
    {
        struct
        {
            uint16_t hvsWidth;
            uint16_t hvsHeight;
        };
        uint32_t Value;
    } DW02;

    union
    {
        struct
        {
            uint32_t noiseLevel;
        };
        uint32_t Value;
    } DW03;

    union
    {
        struct
        {
            uint32_t noiseLevel_u;
        };
        uint32_t Value;
    } DW04;

    union
    {
        struct
        {
            uint32_t noiseLevel_v;
        };
        uint32_t Value;
    } DW05;

        union
    {
        struct
        {
            uint32_t sgneNoiseLevel;
        };
        uint32_t Value;
    } DW06;

    union
    {
        struct
        {
            uint32_t sgneNoiseLevel_u;
        };
        uint32_t Value;
    } DW07;

    union
    {
        struct
        {
            uint32_t sgneNoiseLevel_v;
        };
        uint32_t Value;
    } DW08;

    union
    {
        struct
        {
            uint32_t sgneCount;
        };
        uint32_t Value;
    } DW09;

    union
    {
        struct
        {
            uint32_t sgneCount_u;
        };
        uint32_t Value;
    } DW10;

    union
    {
        struct
        {
            uint32_t sgneCount_v;
        };
        uint32_t Value;
    } DW11;

    union
    {
        struct
        {
            uint32_t preNslvTemporal;
        };
        uint32_t Value;
    } DW12;

    union
    {
        struct
        {
            uint32_t preNslvTemporal_u;
        };
        uint32_t Value;
    } DW13;

    union
    {
        struct
        {
            uint32_t preNslvTemporal_v;
        };
        uint32_t Value;
    } DW14;

    union
    {
        struct
        {
            uint16_t hvsQP;
            uint16_t firstFrame;
        };
        uint32_t Value;
    }
    DW15;

    union
    {
        struct
        {
            uint16_t tgneFirstFrame;
            uint16_t fallBack;
        };
        uint32_t Value;
    } DW16;

    union
    {
        struct
        {
            uint16_t enableChroma;
            uint16_t enableTemporalGNE;
        };
        uint32_t Value;
    } DW17;

};
C_ASSERT(SIZE32(VEBOX_HVS_STATIC_DATA) == 18);

class VpRenderHVSKernel : public VpRenderKernelObj
{
public:
    VpRenderHVSKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator);
    virtual ~VpRenderHVSKernel();

    virtual MOS_STATUS Init(VpRenderKernel &kernel) override;
    virtual MOS_STATUS GetCurbeState(void *&curbe, uint32_t &curbeLength) override;

    virtual MOS_STATUS FreeCurbe(void*& curbe) override
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t   GetInlineDataSize() override
    {
        return 0;
    }

    virtual bool IsKernelCached() override
    {
        return true;
    }

    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData) override;

protected:
    virtual MOS_STATUS SetupSurfaceState() override;
    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1 = false) override;
    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext) override;
    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;

    //kernel Arguments
    KERNEL_ARGS          m_kernelArgs  = {};
    KERNEL_WALKER_PARAMS m_walkerParam = {};

    VEBOX_HVS_STATIC_DATA m_curbe = {};

    MEDIA_CLASS_DEFINE_END(vp__VpRenderHVSKernel)
};

}  // namespace vp

#endif //__VP_RENDER_VEBOX_HVS_KERNEL_H__