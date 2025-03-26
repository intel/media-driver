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

#include "levelzero_interface.h"

MOS_STATUS L0Interface::InitZeFunctions()
{
    return MOS_STATUS_UNIMPLEMENTED;
}

L0Interface::L0Interface(PMOS_INTERFACE osInterface) : m_ordinal(0), m_osInterface(osInterface)
{
}

L0Interface::~L0Interface()
{
}

MOS_STATUS L0Interface::Init(L0Interface *l0Interface)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::Execute(L0Interface *l0Interface, std::vector<ze_command_list_handle_t> &cmdLists, ze_fence_handle_t &fence)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::ExecuteSingle(L0Interface *l0Interface, ze_command_list_handle_t &cmdList, ze_fence_handle_t &fence)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::CreateCmdList(L0Interface *l0Interface, ze_command_list_handle_t &cmdList)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::DestroyFence(L0Interface* l0Interface, ze_fence_handle_t& fence)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::DestroyCmdList(L0Interface *l0Interface, ze_command_list_handle_t &cmdList)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::CreateFence(L0Interface *l0Interface, ze_fence_handle_t &fence)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::WaitFence(L0Interface *l0Interface, ze_fence_handle_t &fence)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::AllocateHostMem(L0Interface *l0Interface, size_t size, void *&ptr)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::FreeHostMem(L0Interface *l0Interface, void *&ptr)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::Destroy()
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS L0Interface::ConvertZeResult(ze_result_t result)
{
    return MOS_STATUS_UNIMPLEMENTED;
}