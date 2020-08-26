/* Copyright (c) 2020, Intel Corporation
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
#include <map>

namespace vp {
class VpRenderKernelObj;
enum KernelId
{
    Kernel_Invalidate = 0,
    Kernel_FastComposition,
    Kernel_Max
};

//!
//! \brief Vebox Kernel IDs
//!
enum VP_VEBOX_KERNELID
{
    KERNEL_RESERVED = 0,
    KERNEL_UPDATEDNSTATE,
    KERNEL_VEBOX_BASE_MAX,
};

struct RENDER_KERNEL_PARAMS
{
    std::map<SurfaceType, VP_SURFACE*> *surfacesGroup;
    std::vector<KernelId>* kernelId;
};

class VpKernelSet
{
public:
    VpKernelSet(PVP_MHWINTERFACE hwInterface);
    virtual ~VpKernelSet() {};

    virtual std::map<KernelId, VpRenderKernelObj*>* GetKernelObjs()
    {
        return &m_kernelPool;
    }

    virtual MOS_STATUS Clean()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS Destroy()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS CreateKernelObjects(RENDER_KERNEL_PARAMS& kernelParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetKernelInfo(int32_t kuid, int32_t& size, void*& kernel);

private:

    Kdll_State* GetKernelEntries()
    {
        if (m_kernel)
        {
            return m_kernel->GetKdllState();
        }
        else
        {
            return nullptr;
        }
    }

protected:

    VpRenderKernel       *m_kernel = nullptr;
    std::map<KernelId, VpRenderKernelObj*> m_kernelPool;
    PVP_MHWINTERFACE      m_hwInterface = nullptr;
};
}

#endif //__VP_KERNEL_SET_H__
