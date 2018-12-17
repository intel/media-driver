/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     media_libva_caps_cp_interface.cpp
//! \brief    The class implementation of MediaLibvaCapsCpInterface
//!

#include "media_libva_caps_cp_interface.h"
#include "cplib_utils.h"
#include <typeinfo>

static void CapsStubMessage()
{
    MOS_NORMALMESSAGE(
        MOS_COMPONENT_CP, 
        MOS_CP_SUBCOMP_CAPS, 
        CP_STUB_MESSAGE);
}

MediaLibvaCapsCpInterface* Create_MediaLibvaCapsCpInterface()
{
    MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface = nullptr;
    using Create_MediaLibvaCapsCpFuncType = MediaLibvaCapsCpInterface* (*)();
    CPLibUtils::InvokeCpFunc<Create_MediaLibvaCapsCpFuncType>(
        pMediaLibvaCapsCpInterface, 
        CPLibUtils::FUNC_CREATE_MEDIALIBVACAPSCP);

    if(nullptr == pMediaLibvaCapsCpInterface)
    {
        CapsStubMessage();
    }
    
    return nullptr == pMediaLibvaCapsCpInterface ? MOS_New(MediaLibvaCapsCpInterface) : pMediaLibvaCapsCpInterface;
}

void Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface)
{
    if(nullptr == pMediaLibvaCapsCpInterface) 
    {
        return;
    }

    if(typeid(*pMediaLibvaCapsCpInterface) == typeid(MediaLibvaCapsCpInterface))
    {
        MOS_Delete(pMediaLibvaCapsCpInterface);
    }
    else
    {
        using Delete_MediaLibvaCapsCpFuncType = void (*)(MediaLibvaCapsCpInterface*);
        CPLibUtils::InvokeCpFunc<Delete_MediaLibvaCapsCpFuncType>(
            CPLibUtils::FUNC_DELETE_MEDIALIBVACAPSCP, 
            pMediaLibvaCapsCpInterface);
    }
}

bool MediaLibvaCapsCpInterface::IsDecEncryptionSupported(DDI_MEDIA_CONTEXT* mediaCtx)
{
    CapsStubMessage();
    return false;
}

int32_t MediaLibvaCapsCpInterface::GetEncryptionTypes(
    VAProfile profile, 
    uint32_t *encryptionType, 
    uint32_t arraySize)
{
    CapsStubMessage();
    return -1;
}
