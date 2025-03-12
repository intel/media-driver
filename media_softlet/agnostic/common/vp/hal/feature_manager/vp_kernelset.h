/* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     vp_render_kernelset.h
//! \brief    The header file of the base class of kernel set
//! \details  The kernel set will include kernel generation from binary.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VP_KERNEL_SET_H__
#define __VP_KERNEL_SET_H__

#include "vp_platform_interface.h"
#include "vp_pipeline_common.h"
#include "vp_render_kernel_obj.h"
#include <map>

namespace vp {

// KernelIndex is the index in KERNEL_PARAMS_LIST
using KERNEL_OBJECTS = std::map<KernelIndex, VpRenderKernelObj*>;
using KERNEL_RENDER_DATA = std::map<KernelIndex, KERNEL_PACKET_RENDER_DATA>;
using KERNEL_PARAMS_LIST = std::vector<KERNEL_PARAMS>;

class VpKernelSet
{
public:
    VpKernelSet(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator);
    virtual ~VpKernelSet()
    {
        for (auto &it : m_cachedKernels)
        {
            MOS_Delete(it.second);
        }
    };

    virtual MOS_STATUS Clean()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS Destroy()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS CreateSingleKernelObject(
        VpRenderKernelObj *&kernel,
        VpKernelID          kernelId,
        KernelIndex         kernelIndex,
        std::string         kernelName);

    virtual MOS_STATUS CreateKernelObjects(
        KERNEL_PARAMS_LIST& kernelParams,
        VP_SURFACE_GROUP& surfacesGroup,
        KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup,
        KERNEL_CONFIGS& kernelConfigs,
        KERNEL_OBJECTS& kernelObjs,
        VP_RENDER_CACHE_CNTL& surfMemCacheCtl,
        VP_PACKET_SHARED_CONTEXT* sharedContext);

    virtual MOS_STATUS DestroyKernelObjects(KERNEL_OBJECTS& kernelObjs)
    {
        while (!kernelObjs.empty())
        {
            auto it = kernelObjs.begin();
            if (m_cachedKernels.size() == 0 || m_cachedKernels.end() == m_cachedKernels.find(it->second->GetKernelId()))
            {
                // Only destroy the kernels not exists in m_cachedKernels.
                MOS_Delete(it->second);
            }
            kernelObjs.erase(it);
        }

        return MOS_STATUS_SUCCESS;
    }
protected:

    MOS_STATUS GetKernelInfo(std::string kernalName, uint32_t kuid, uint32_t &size, void *&kernel);

    MOS_STATUS FindAndInitKernelObj(VpRenderKernelObj* kernelObj);

protected:
    KERNEL_POOL*          m_pKernelPool = nullptr;
    PVP_MHWINTERFACE      m_hwInterface = nullptr;
    PVpAllocator          m_allocator   = nullptr;
    std::map<VpKernelID, VpRenderKernelObj *> m_cachedKernels;

MEDIA_CLASS_DEFINE_END(vp__VpKernelSet)
};
}

#endif //__VP_KERNEL_SET_H__
