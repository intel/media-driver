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
//! \file     cp_interfaces.cpp
//! \brief    The class implementation of CpInterfaces base class.
//!

#include "codec_def_common.h"
#include "codec_def_common_encode.h"
#include "codec_def_encode_jpeg.h"
#include "media_libva_util.h"
#include "media_libva_caps.h"
#include "cp_interfaces.h"

static bool isRegistered = CpInterfacesFactory::Register<CpInterfaces>(CP_INTERFACE);

CodechalSecureDecodeInterface* CpInterfaces::Create_SecureDecodeInterface(
    CodechalSetting *    codechalSettings,
    CodechalHwInterface *hwInterfaceInput)
{
    CodechalSecureDecodeInterface *pInterface = nullptr;

    using Create_SecureDecodeFuncType = CodechalSecureDecodeInterface *(*)(
        CodechalSetting * codecHalSettings,
        CodechalHwInterface * hwInterfaceInput);
    CPLibUtils::InvokeCpFunc<Create_SecureDecodeFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_SECUREDECODE,
        codechalSettings,
        hwInterfaceInput);

    return pInterface;
}

void CpInterfaces::Delete_SecureDecodeInterface(CodechalSecureDecodeInterface *pInterface)
{
    if (pInterface != nullptr)
    {
        using Delete_SecureDecodeFuncType = void (*)(CodechalSecureDecodeInterface *pInterface);
        CPLibUtils::InvokeCpFunc<Delete_SecureDecodeFuncType>(CPLibUtils::FUNC_DELETE_SECUREDECODE, pInterface);
    }
}

MhwCpInterface* CpInterfaces::Create_MhwCpInterface(PMOS_INTERFACE osInterface)
{
    MhwCpInterface* pInterface = nullptr;

    using Create_MhwCpFuncType = MhwCpInterface* (*)(PMOS_INTERFACE osInterface);
    CPLibUtils::InvokeCpFunc<Create_MhwCpFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_MHWCP, osInterface);

    return pInterface;
}

void CpInterfaces::Delete_MhwCpInterface(MhwCpInterface *pInterface)
{
    if (nullptr == pInterface)
    {
        return;
    }

    if (typeid(*pInterface) == typeid(MhwCpInterface))
    {
        MOS_Delete(pInterface);
    }
    else
    {
        using Delete_MhwCpFuncType = void (*)(MhwCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MhwCpFuncType>(
            CPLibUtils::FUNC_DELETE_MHWCP,
            pInterface);
    }
}

MosCpInterface* CpInterfaces::Create_MosCpInterface(void* pvOsInterface)
{
    MosCpInterface* pInterface = nullptr;

    using Create_MosCpFuncType = MosCpInterface* (*)(void* pvOsResource);
    CPLibUtils::InvokeCpFunc<Create_MosCpFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_MOSCP, pvOsInterface);

    return pInterface;
}

void CpInterfaces::Delete_MosCpInterface(MosCpInterface* pInterface)
{
    if (nullptr == pInterface)
    {
        return;
    }

    if (typeid(*pInterface) == typeid(MosCpInterface))
    {
        MOS_Delete(pInterface);
    }
    else
    {
        using Delete_MosCpFuncType = void (*)(MosCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MosCpFuncType>(
            CPLibUtils::FUNC_DELETE_MOSCP,
            pInterface);
    }
}

DdiCpInterface* CpInterfaces::Create_DdiCpInterface(MOS_CONTEXT& mosCtx)
{
    DdiCpInterface* pInterface = nullptr;

    using Create_DdiCpFuncType = DdiCpInterface* (*)(MOS_CONTEXT* pMosCtx);
    CPLibUtils::InvokeCpFunc<Create_DdiCpFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_DDICP, &mosCtx);

    return pInterface;
}

void CpInterfaces::Delete_DdiCpInterface(DdiCpInterface* pInterface)
{
    if (nullptr == pInterface)
    {
        return;
    }

    if (typeid(*pInterface) == typeid(DdiCpInterface))
    {
        MOS_Delete(pInterface);
    }
    else
    {
        using Delete_DdiCp= void (*)(DdiCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_DdiCp>(
            CPLibUtils::FUNC_DELETE_DDICP,
            pInterface);
    }
}

MediaLibvaCapsCpInterface* CpInterfaces::Create_MediaLibvaCapsCpInterface(
    DDI_MEDIA_CONTEXT *mediaCtx,
    MediaLibvaCaps *mediaCaps)
{
    MediaLibvaCapsCpInterface* pInterface = nullptr;

    using Create_MediaLibvaCapsCpFuncType = MediaLibvaCapsCpInterface* (*)(DDI_MEDIA_CONTEXT *mediaCtx, MediaLibvaCaps *mediaCaps);
    CPLibUtils::InvokeCpFunc<Create_MediaLibvaCapsCpFuncType>(
        pInterface,
        CPLibUtils::FUNC_CREATE_MEDIALIBVACAPSCP,
        mediaCtx,
        mediaCaps);

    return pInterface;
}

void CpInterfaces::Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pInterface)
{
    if (nullptr == pInterface)
    {
        return;
    }

    if (typeid(*pInterface) == typeid(MediaLibvaCapsCpInterface))
    {
        MOS_Delete(pInterface);
    }
    else
    {
        using Delete_MediaLibvaCapsCpFuncType = void (*)(MediaLibvaCapsCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MediaLibvaCapsCpFuncType>(
            CPLibUtils::FUNC_DELETE_MEDIALIBVACAPSCP,
            pInterface);
    }
}
