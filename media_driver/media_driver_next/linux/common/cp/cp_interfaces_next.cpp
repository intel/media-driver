/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     cp_interfaces_next.cpp
//! \brief    The class implementation of CpInterfacesNext class.
//!

#include "cp_interfaces_next.h"

static bool isRegistered = CpInterfacesFactory::Register<CpInterfacesNext>(CP_INTERFACE, true);

CpStreamOutInterface *CpInterfacesNext::Create_CpStreamOutInterface(
    MediaPipeline *pipeline,
    MediaTask *task,
    CodechalHwInterface *hwInterface)
{
    CpStreamOutInterface *pInterface = nullptr;

    using Create_CpStreamOutFuncType = CpStreamOutInterface *(*)(
        MediaPipeline *pipeline,
        MediaTask *task,
        CodechalHwInterface *hwInterface);
    CPLibUtils::InvokeCpFunc<Create_CpStreamOutFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_CPSTREAMOUT,
        pipeline,
        task,
        hwInterface);

    return pInterface;
}

void CpInterfacesNext::Delete_CpStreamOutInterface(CpStreamOutInterface *pInterface)
{
    if (pInterface != nullptr)
    {
        using Delete_CpStreamOutFuncType = void (*)(CpStreamOutInterface* pCpStreamOutInterface);
        CPLibUtils::InvokeCpFunc<Delete_CpStreamOutFuncType>(CPLibUtils::FUNC_DELETE_CPSTREAMOUT, pInterface);
    }
}

DecodeCpInterface *CpInterfacesNext::Create_DecodeCpInterface(
    CodechalSetting *    codechalSettings,
    CodechalHwInterface *hwInterfaceInput)
{
    DecodeCpInterface *pInterface = nullptr;

    using Create_DecodeCpFuncType = DecodeCpInterface *(*)(
        CodechalSetting * codecHalSettings,
        CodechalHwInterface * hwInterfaceInput);

    CPLibUtils::InvokeCpFunc<Create_DecodeCpFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_DECODECP,
        codechalSettings,
        hwInterfaceInput);

    return pInterface;
}

void CpInterfacesNext::Delete_DecodeCpInterface(DecodeCpInterface *pInterface)
{
    if (pInterface != nullptr)
    {
        using Delete_DecodeCpFuncType = void (*)(DecodeCpInterface* pDecodeCpInterface);
        CPLibUtils::InvokeCpFunc<Delete_DecodeCpFuncType>(CPLibUtils::FUNC_DELETE_DECODECP, pInterface);
    }
}
