/* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_graph_manager.h
//! \brief    The header file of the base class of graph set
//! \details  The graph set will include graph generation from binary.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VP_GRAPH_MANAGER_H__
#define __VP_GRAPH_MANAGER_H__

#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "vp_graphset.h"
#include "vp_ai_kernel_pipe.h"
#include "levelzero_npu_interface.h"
#include "npu_cmd_packet.h"

namespace vp
{

#define GRAPH_PACKAGE_INITIALIZE_TIMEOUT_MS 100  // first frame need initialize graph timing

class GraphPackage
{
public:
    GraphPackage() : m_npuWLContext(GRAPH_PACKAGE_INITIALIZE_TIMEOUT_MS) {};
    NpuWLContext &GetNpuWLContext()
    {
        return m_npuWLContext;
    }
    std::map<SurfaceType, PVP_SURFACE> &IntermiedateSurfaces()
    {
        return m_intermediateSurfaces;
    }

private:
    // GraphPackage cannot be copied for it contains a non-copiable member m_npuWLContext
    std::map<SurfaceType, PVP_SURFACE> m_intermediateSurfaces = {};
    NpuWLContext                       m_npuWLContext;
};

class VpGraphManager
{
public:
    VpGraphManager(VpGraphSet *graphset, PMOS_INTERFACE osInterface, VpAllocator *allocator);
    ~VpGraphManager() { Destroy(); };

    MOS_STATUS GetGraphPackage(std::vector<AI_SINGLE_NPU_GRAPH_SETTING> &graphSettings, SwFilterPipe &executingPipe, GraphPackage *&graphPackage);

private:
    MOS_STATUS Destroy();

private:
    std::map<VpGraphIDList, GraphPackage> m_graphPackages = {};
    VpGraphSet                           *m_graphset      = nullptr;
    PMOS_INTERFACE                        m_osInterface   = nullptr;
    L0NpuInterface                       *m_npuInterface  = nullptr;
    VpAllocator                          *m_allocator     = nullptr;

    MEDIA_CLASS_DEFINE_END(vp__VpGraphManager)
};

class InitGraphListCmdPackage : public CmdPackage
{
public:
    InitGraphListCmdPackage(L0NpuInterface *npuInterface, GraphPackage *graphPackage) : m_npuInterface(npuInterface), m_graphPackage(graphPackage) {};
    virtual MOS_STATUS                  Submit();
    virtual MOS_STATUS                  Wait() { return MOS_STATUS_SUCCESS; };
    virtual std::unique_ptr<CmdPackage> Clone() const;
    virtual bool                        Releaseable() override { return m_releaseable; };
    virtual MOS_STATUS                  AddGraphHandle(GraphHandle *grapHandle)
    {
        m_graphHandles.push_back(grapHandle);
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS AddSingleLayerGraphArgs(std::vector<void *> &singleLayerGraphArgs)
    {
        m_multiLayerGraphArgs.push_back(singleLayerGraphArgs);
        return MOS_STATUS_SUCCESS;
    }

private:
    L0NpuInterface                  *m_npuInterface        = nullptr;
    GraphPackage                    *m_graphPackage        = nullptr;
    std::vector<GraphHandle *>       m_graphHandles        = {};
    std::vector<std::vector<void *>> m_multiLayerGraphArgs = {};
    bool                             m_releaseable         = false;
};

}  // namespace vp

#endif  //__VP_GRAPH_SET_H__
