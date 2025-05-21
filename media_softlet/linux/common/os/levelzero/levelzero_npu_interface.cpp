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
//!
//! \file     levelzero_interface.cpp
//! \brief    MOS interface implementation
//!

#include "levelzero_npu_interface.h"

L0NpuInterface::L0NpuInterface(PMOS_INTERFACE osInterface) : L0Interface(osInterface)
{
}

L0NpuInterface ::~L0NpuInterface()
{
}

MOS_STATUS L0NpuInterface::GetDriver()
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0NpuInterface::BuildGraph(L0NpuInterface *npuInterface, uint8_t *blob, uint32_t size, ze_graph_handle_t &graph)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0NpuInterface::DestroyGraph(L0NpuInterface *npuInterface, ze_graph_handle_t &graph)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0NpuInterface::AppendGraph(L0NpuInterface *npuInterface, ze_command_list_handle_t &cmdList, ze_graph_handle_t &graph, std::vector<void *> &args)
{
    return MOS_STATUS_UNIMPLEMENTED;
}