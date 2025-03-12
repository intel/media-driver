/*
* Copyright (c) 2024, Intel Corporation
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
#ifndef __VP_RENDER_OCL_FC_KERNEL_H__
#define __VP_RENDER_OCL_FC_KERNEL_H__

#include "vp_platform_interface.h"
#include "vp_render_kernel_obj.h"

namespace vp
{

class VpRenderOclFcKernel : public VpRenderKernelObj
{
public:
    VpRenderOclFcKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator);
    virtual ~VpRenderOclFcKernel();

    virtual MOS_STATUS Init(VpRenderKernel &kernel);
    virtual MOS_STATUS GetCurbeState(void *&curbe, uint32_t &curbeLength);
    virtual uint32_t   GetInlineDataSize() override
    {
        return 0;
    }
    virtual MOS_STATUS SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP &samplerStateGroup);
    virtual MOS_STATUS GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData);

    virtual MOS_STATUS SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs) override;
    virtual MOS_STATUS SetPerfTag() override;

    MOS_STATUS FreeCurbe(void *&curbe)
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    virtual MOS_STATUS SetupSurfaceState() override;
    virtual MOS_STATUS SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1 = false);
    virtual MOS_STATUS SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext);

    PRENDERHAL_INTERFACE m_renderHal      = nullptr;

    //kernel Arguments
    KERNEL_INDEX_ARG_MAP         m_kernelArgs          = {};
    KERNEL_BTIS                  m_kernelBtis          = {};
    KRN_EXECUTE_ENV              m_kernelEnv           = {};
    KERNEL_WALKER_PARAMS         m_walkerParam         = {};
    void                        *m_curbe               = nullptr;
    uint32_t                     m_kernelIndex         = 0;
    uint32_t                     m_curbeSize           = 0;
    int32_t                      m_linearSamplerIndex  = -1;
    int32_t                      m_nearestSamplerIndex = -1;
    std ::vector<uint8_t>        m_inlineData          = {};
    OCL_FC_KERNEL_CONFIG         m_kernelConfig        = {};

MEDIA_CLASS_DEFINE_END(vp__VpRenderOclFcKernel)
};
}  // namespace vp
#endif  //__VP_RENDER_OCL_FC_KERNEL_H__
