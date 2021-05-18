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
//! \file     media_libva_interface_next.cpp
//! \brief    libva interface next implementaion.
//!

#include "media_libva_interface_next.h"
#include "media_libva_context_next.h"
#include "media_libva_common.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "mos_utilities.h"

VAStatus MediaLibvaInterfaceNext::Initialize (
    VADriverContextP ctx,
    int32_t          devicefd,
    int32_t         *major_version,
    int32_t         *minor_version
)
{
    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::InitMediaContext (
        VADriverContextP ctx,
        int32_t          devicefd,
        int32_t         *major_version,
        int32_t         *minor_version
)
{
    DDI_FUNCTION_ENTER();

    if(major_version)
    {
        *major_version = VA_MAJOR_VERSION;
    }

    if(minor_version)
    {
        *minor_version = VA_MINOR_VERSION;
    }

    MediaLibvaContextNext *pMediaLibvaContextNext = nullptr;
    pMediaLibvaContextNext = MOS_New(MediaLibvaContextNext);

    if(nullptr == pMediaLibvaContextNext)
    {
        DDI_ASSERTMESSAGE("Unable to allocate media libva context next");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if(VA_STATUS_SUCCESS != pMediaLibvaContextNext->Init())
    {
        DDI_ASSERTMESSAGE("Unable to init media libva context next");
        pMediaLibvaContextNext->Free();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::LoadFunction(VADriverContextP ctx)
{
    DDI_FUNCTION_ENTER();
    
    DDI_CHK_NULL(ctx,         "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTable    *pVTable     = DDI_CODEC_GET_VTABLE(ctx);
    DDI_CHK_NULL(pVTable,     "nullptr pVTable",      VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTableVPP *pVTableVpp  = DDI_CODEC_GET_VTABLE_VPP(ctx);
    DDI_CHK_NULL(pVTableVpp,  "nullptr pVTableVpp",   VA_STATUS_ERROR_INVALID_CONTEXT);

#if VA_CHECK_VERSION(1,11,0)
    struct VADriverVTableProt *pVTableProt = DDI_CODEC_GET_VTABLE_PROT(ctx);
    DDI_CHK_NULL(pVTableProt,  "nullptr pVTableProt",   VA_STATUS_ERROR_INVALID_CONTEXT);
#endif

    return VA_STATUS_SUCCESS;
}