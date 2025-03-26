/*
* Copyright (c) 2025, Intel Corporation
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
#ifndef __LEVELZERO_NPU_INTERFACE_H__
#define __LEVELZERO_NPU_INTERFACE_H__
#include "levelzero_interface.h"

#define GRAPH_HANDLE_INITIALIZE_TIMEOUT_MS 20

class GraphHandle
{
public:
    GraphHandle() : m_timeoutMs(GRAPH_HANDLE_INITIALIZE_TIMEOUT_MS) {};
    ze_graph_handle_t &Graph()
    {
        return m_graph;
    }
    std::condition_variable &Condition()
    {
        return m_cv;
    }
    std::mutex &Mutex()
    {
        return m_mtx;
    }
    std::thread &Thread()
    {
        return m_initThread;
    }
    bool &Initialized()
    {
        return m_initialized;
    }
    bool &Failed()
    {
        return m_failed;
    }
    uint32_t GetTimeoutMs()
    {
        return m_timeoutMs;
    }

private:
    ze_graph_handle_t       m_graph       = nullptr;
    bool                    m_initialized = false;
    bool                    m_failed      = false;
    std::condition_variable m_cv          = {};
    std::mutex              m_mtx         = {};
    uint32_t                m_timeoutMs   = 0;
    std::thread             m_initThread;
};

class L0NpuInterface : virtual public L0Interface
{
public:
    L0NpuInterface(PMOS_INTERFACE osInterface);

    virtual ~L0NpuInterface();

    static MOS_STATUS BuildGraph(L0NpuInterface *npuInterface, uint8_t *blob, uint32_t size, ze_graph_handle_t &graph);
    static MOS_STATUS AppendGraph(L0NpuInterface *npuInterface, ze_command_list_handle_t &cmdList, ze_graph_handle_t &graph, std::vector<void *> &args);
    static MOS_STATUS DestroyGraph(L0NpuInterface *npuInterface, ze_graph_handle_t &graph);

    MOS_STATUS(*pfnBuildGraph) (L0NpuInterface *npuInterface, uint8_t *blob, uint32_t size, ze_graph_handle_t &graph) = BuildGraph;
    MOS_STATUS (*pfnAppendGraph) (L0NpuInterface *npuInterface, ze_command_list_handle_t &cmdList, ze_graph_handle_t &graph, std::vector<void *> &args) = AppendGraph;
    MOS_STATUS(*pfnDestroyGraph) (L0NpuInterface *npuInterface, ze_graph_handle_t &graph) = DestroyGraph;

protected:
    virtual MOS_STATUS       GetDriver() override;

    ze_graph_dditable_ext_t  *m_zeGraphDdiTable = nullptr;

MEDIA_CLASS_DEFINE_END(mhw__LevelZeroNpuInterface)
};

#endif
