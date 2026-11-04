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
#include <tuple>

namespace vp
{
// A graph that does not depend on the input shape leaves the size at zero. A graph
// that is built per input shape, such as a runtime IR graph, has the shape in the key.
struct VpGraphKey
{
    VP_GRAPH_ID id          = VP_GRAPH_ID_INVALID;
    uint32_t    inputWidth  = 0;
    uint32_t    inputHeight = 0;

    bool operator<(const VpGraphKey &other) const
    {
        return std::tie(id, inputWidth, inputHeight) <
               std::tie(other.id, other.inputWidth, other.inputHeight);
    }
};
using VpGraphIDList = std::vector<VpGraphKey>;

enum class GRAPH_BIN_TYPE
{
    BLOB,  // A precompiled blob, built for fixed shapes
    IR,    // An OpenVINO IR, compiled at runtime for the input shape
};

struct GRAPH_BIN
{
    GRAPH_BIN_TYPE type;
    union
    {
        struct
        {
            uint8_t *data;
            uint32_t size;
        } blob;
        struct
        {
            const uint8_t *xml;
            uint32_t       xmlSize;
            const uint8_t *bin;
            uint32_t       binSize;
        } ir;
    };
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
        const VpGraphKey &graphKey,
        GraphHandle      *&graphHandle);

    // Registers the models of the platform into the graph pool, IR and blob alike.
    // Called once after the graph set is created.
    virtual MOS_STATUS RegisterModel() { return MOS_STATUS_SUCCESS; };

protected:
#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS LoadGraphForDebug() { return MOS_STATUS_SUCCESS; };
#endif

protected:
    GRAPH_POOL                        *m_graphPool    = nullptr;
    PVP_MHWINTERFACE                   m_hwInterface  = nullptr;
    PMOS_INTERFACE                     m_osInterface  = nullptr;
    PVpAllocator                       m_allocator    = nullptr;
    std::map<VpGraphKey, GraphHandle>  m_cachedGraphs = {};

MEDIA_CLASS_DEFINE_END(vp__VpGraphSet)
};

class InitGraphCmdPackage : public CmdPackage
{
public:
    // width and height are the input shape an IR graph is compiled for, unused for a blob.
    InitGraphCmdPackage(GRAPH_BIN &graphBin, uint32_t width, uint32_t height, L0NpuInterface *npuInterface, GraphHandle *graphHandle) : m_graphHandle(graphHandle), m_npuInterface(npuInterface), m_graphBin(graphBin), m_width(width), m_height(height) {};
    virtual MOS_STATUS                  Submit();
    virtual MOS_STATUS                  Wait() { return MOS_STATUS_SUCCESS; };
    virtual std::unique_ptr<CmdPackage> Clone() const;
    virtual bool                        Releaseable() override { return m_releaseable; };

private:
    MOS_STATUS BuildGraph();

    GraphHandle    *m_graphHandle  = nullptr;
    L0NpuInterface *m_npuInterface = nullptr;
    GRAPH_BIN       m_graphBin     = {};
    uint32_t        m_width        = 0;
    uint32_t        m_height       = 0;
    bool            m_releaseable  = false;

MEDIA_CLASS_DEFINE_END(vp__InitGraphCmdPackage)
};

}  // namespace vp

#endif  //__VP_GRAPH_SET_H__
