/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_libva_context_next.cpp
//! \brief    libva context next implementaion.
//!

#include "media_libva_context_next.h"
#include "mos_utilities.h"
#include "media_factory.h"
#include "media_libva_util.h"
#include "media_libva_register.h"

VAStatus MediaLibvaContextNext::Init()
{
    return InitCompList();
}

VAStatus MediaLibvaContextNext::InitCompList()
{
    DDI_FUNCTION_ENTER();

    VAStatus status = VA_STATUS_SUCCESS;
    compList[CompCommon] = MOS_New(DdiMediaFunctions);

    if(nullptr == compList[CompCommon])
    {
        status = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DDI_ASSERTMESSAGE("MediaLibvaContextNext::Init: Unable to create m_compList CompCommon.");
        return status;
    }

    for(int i = CompCommon + 1; i < CompCount; i++)
    {
        if (FunctionsFactory::IsRegistered((CompType)i))
        {
            compList[i] = FunctionsFactory::Create((CompType)i);
            if (nullptr == compList[i])
            {
                status = VA_STATUS_ERROR_ALLOCATION_FAILED;
                DDI_ASSERTMESSAGE("MediaLibvaContextNext::Init: Unable to create m_compList %d.", i);
                return status;
            }
        }
        else
        {
            compList[i] = compList[CompCommon];
        }
    }

    return status;
}

void MediaLibvaContextNext::Free()
{
    FreeCompList();
}

void MediaLibvaContextNext::FreeCompList()
{
    DDI_FUNCTION_ENTER();
    
    MOS_Delete(compList[CompCommon]);
    compList[CompCommon] = nullptr;

    for(int i = CompCommon + 1; i < CompCount; i++)
    {
        if(nullptr != compList[i])
        {
            if(FunctionsFactory::IsRegistered((CompType)i))
            {
                MOS_Delete(compList[i]);
            }
            compList[i] = nullptr;
        }
    }
}

MediaLibvaContextNext::~MediaLibvaContextNext()
{
    Free();
}