/*
* Copyright (c) 2013-2018, Intel Corporation
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
//! \file      codechal_secure_decode_interface.cpp
//! \brief     Stub file for CodecHal Secure Decode 
//!

#include "codechal_secure_decode_interface.h"
#include "cplib_utils.h"

static void SecureDecodeStubMessage()
{
    MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, CP_STUB_MESSAGE);
}

CodechalSecureDecodeInterface *Create_SecureDecodeInterface(
    CodechalSetting *    codechalSettings,
    CodechalHwInterface *hwInterfaceInput)
{
    if(nullptr == codechalSettings || nullptr == hwInterfaceInput)
    {
        MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, "NULL pointer parameters");
        return nullptr;
    }

    CodechalSecureDecodeInterface *pCodechalSecureDecodeInterface = nullptr;
    using Create_SecureDecodeFuncType                     = CodechalSecureDecodeInterface *(*)(
        CodechalSetting * codecHalSettings, 
        CodechalHwInterface * hwInterfaceInput);

    CPLibUtils::InvokeCpFunc<Create_SecureDecodeFuncType>(
        pCodechalSecureDecodeInterface, 
        CPLibUtils::FUNC_CREATE_SECUREDECODE, 
        codechalSettings, 
        hwInterfaceInput);

    if(nullptr == pCodechalSecureDecodeInterface) SecureDecodeStubMessage();

    return pCodechalSecureDecodeInterface;
}

void Delete_SecureDecodeInterface(CodechalSecureDecodeInterface *pCodechalSecureDecodeInterface)
{
    if(pCodechalSecureDecodeInterface != nullptr)
    {
        using Delete_SecureDecodeFuncType = void (*)(CodechalSecureDecodeInterface* pCodechalSecureDecodeInterface);
        CPLibUtils::InvokeCpFunc<Delete_SecureDecodeFuncType>(CPLibUtils::FUNC_DELETE_SECUREDECODE, pCodechalSecureDecodeInterface);
    }
}