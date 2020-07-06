/*
* Copyright (c) 2019, Intel Corporation
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
//! \file      cp_streamout_interface.cpp
//! \brief     Stub file for cp streamout pkt interface
//!

#include "cp_streamout_interface.h"
#include "cp_interfaces_next.h"

static void CpStreamOutStubMessage()
{
    MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, "This function is stubbed as CP is not enabled.");
}

CpStreamOutInterface *Create_CpStreamOutInterface(
    MediaPipeline *pipeline,
    MediaTask *task,
    CodechalHwInterface *hwInterface)
{
    if (nullptr == pipeline || nullptr == task || nullptr == hwInterface)
    {
        MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, "NULL pointer parameters");
        return nullptr;
    }

    CpInterfacesNext *cp_interface = dynamic_cast<CpInterfacesNext *>(CpInterfacesFactory::Create(CP_INTERFACE));
    if (nullptr == cp_interface)
    {
        MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, "NULL pointer parameters");
        return nullptr;
    }

    CpStreamOutInterface *pInterface = nullptr;
    pInterface = cp_interface->Create_CpStreamOutInterface(pipeline, task, hwInterface);
    MOS_Delete(cp_interface);

    if (nullptr == pInterface) CpStreamOutStubMessage();

    return pInterface;
}

void Delete_CpStreamOutInterface(CpStreamOutInterface *pInterface)
{
    CpInterfacesNext *cp_interface = dynamic_cast<CpInterfacesNext *>(CpInterfacesFactory::Create(CP_INTERFACE));
    if (pInterface != nullptr && cp_interface != nullptr)
    {
        cp_interface->Delete_CpStreamOutInterface(pInterface);
    }
    MOS_Delete(cp_interface);
}
