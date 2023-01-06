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
#ifndef __VP_KERNEL_CONFIG_H__
#define __VP_KERNEL_CONFIG_H__

#include <map>
#include "mos_defs.h"
#include "renderhal.h"
#include "hal_kerneldll_next.h"
#include "media_class_trace.h"

#define VP_USE_MEDIA_THREADS_MAX    0
// Compositing Block size
#define VP_COMP_BLOCK_WIDTH         16
#define VP_COMP_BLOCK_HEIGHT        16

#define ADD_VP_KERNEL_PARAMS(kernelId, grfCount, btCount, samplerCount, threadCount, grfStartReg, curbeLen, blockWidth, blockHeight, blocksX, blocksY)  \
            m_kernelParams.insert(std::make_pair((VpKernelID)(kernelId),                                                                                \
            RENDERHAL_KERNEL_PARAM{ (grfCount), (btCount), (samplerCount), (threadCount), (grfStartReg), (curbeLen), (blockWidth), (blockHeight), (blocksX), (blocksY)}))

#define VP_ADV_KERNEL_BINARY_ID(kernelId)   ((IDR_VP_TOTAL_NUM_KERNELS) + (kernelId))

enum DelayLoadedKernelType
{
    KernelNone = 0,
    KernelFDFB,
    KernelSEG,
    KernelSR,
    //If more kernels need to load delayed, can add to list
};

namespace vp {
class VpKernelConfig
{
public:
    VpKernelConfig();
    virtual ~VpKernelConfig();
    MOS_STATUS GetKernelParam(VpKernelID kernelId, RENDERHAL_KERNEL_PARAM &param);

    virtual Kdll_Scalingratio GetFilterScalingRatio(float scaleX, float scaleY)
    {
        return Scalingratio_Any;
    }

    virtual bool IsFcCscCoeffPatchModeEnabled()
    {
        return true;
    }

    virtual bool IsDpFcKernelEnabled()
    {
        return false;
    }

protected:
    std::map<VpKernelID, RENDERHAL_KERNEL_PARAM> m_kernelParams;    // Kernel params.

MEDIA_CLASS_DEFINE_END(vp__VpKernelConfig)
};
}
#endif // __VP_KERNEL_CONFIG_H__
