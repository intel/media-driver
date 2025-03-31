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
//! \file     vp_graphset.h
//! \brief    The header file of the base class of graph set
//! \details  The graph set will include graph generation from binary.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VP_GRAPH_SET_H__
#define __VP_GRAPH_SET_H__

#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "mos_hybrid_cmd_manager.h"
#include "levelzero_npu_interface.h"

namespace vp
{
using VpGraphIDList = std::vector<VP_GRAPH_ID>;
struct GRAPH_BIN
{
    uint8_t *blob = nullptr;
    uint32_t size = 0;
};
using GRAPH_POOL = std::map<VP_GRAPH_ID, GRAPH_BIN>;

class VpGraphSet
{
public:
    VpGraphSet(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator);
    virtual ~VpGraphSet()
    {
        Clean();
    };

    virtual MOS_STATUS Clean();

    virtual MOS_STATUS Destroy()
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS GetGraphObject(
        VP_GRAPH_ID   graphId,
        GraphHandle *&graphHandle);

protected:
#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS LoadGraphForDebug() { return MOS_STATUS_SUCCESS; };
#endif

protected:
    GRAPH_POOL                        *m_graphPool    = nullptr;
    PVP_MHWINTERFACE                   m_hwInterface  = nullptr;
    PMOS_INTERFACE                     m_osInterface  = nullptr;
    PVpAllocator                       m_allocator    = nullptr;
    std::map<VP_GRAPH_ID, GraphHandle> m_cachedGraphs = {};

MEDIA_CLASS_DEFINE_END(vp__VpGraphSet)
};

class InitGraphCmdPackage : public CmdPackage
{
public:
    InitGraphCmdPackage(GRAPH_BIN &graphBin, L0NpuInterface *npuInterface, GraphHandle *graphHandle) : m_graphHandle(graphHandle), m_npuInterface(npuInterface), m_graphBin(graphBin) {};
    virtual MOS_STATUS                  Submit();
    virtual MOS_STATUS                  Wait() { return MOS_STATUS_SUCCESS; };
    virtual std::unique_ptr<CmdPackage> Clone() const;
    virtual bool                        Releaseable() override { return m_releaseable; };

private:
    GraphHandle    *m_graphHandle  = nullptr;
    L0NpuInterface *m_npuInterface = nullptr;
    GRAPH_BIN       m_graphBin     = {};
    bool            m_releaseable  = false;
};

}  // namespace vp

#endif  //__VP_GRAPH_SET_H__
