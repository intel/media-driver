/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_libva_caps_mtl.cpp
//! \brief    This file implements the C++ class/interface for mtl media capbilities.
//!

#include "media_libva.h"
#include "media_libva_util.h"
#include "media_libva_caps_mtl.h"
#include "media_libva_caps_factory.h"
#define VA_COPY_MODE_BALANCE 3

VAStatus MediaLibvaCapsMtl::GetDisplayAttributes(
    VADisplayAttribute* attribList,
    int32_t numAttribs)
{
    DDI_CHK_NULL(attribList, "Null attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    for (auto i = 0; i < numAttribs; i++)
    {
        switch (attribList->type)
        {
        #if VA_CHECK_VERSION(1, 15, 0)
        case VADisplayPCIID:
            attribList->min_value = attribList->value = attribList->max_value = (m_mediaCtx->iDeviceId & 0xffff) | 0x80860000;
            attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
            break;
        #endif
        case VADisplayAttribCopy:
            attribList->min_value = attribList->value = attribList->max_value =
                (1 << VA_EXEC_MODE_POWER_SAVING) | (1 << VA_EXEC_MODE_PERFORMANCE) | (1 << VA_EXEC_MODE_DEFAULT) | (1 << VA_COPY_MODE_BALANCE);
            // 1000:balance mode: Vebox Copy
            // 100: perfromance : Render copy
            // 10:  POWER_SAVING: BLT copy
            // 1:   default model
            // 0:   don't support media copy.
            attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
            break;
        default:
            attribList->min_value = VA_ATTRIB_NOT_SUPPORTED;
            attribList->max_value = VA_ATTRIB_NOT_SUPPORTED;
            attribList->value = VA_ATTRIB_NOT_SUPPORTED;
            attribList->flags = VA_DISPLAY_ATTRIB_NOT_SUPPORTED;
            break;
        }
        attribList++;
    }
    return VA_STATUS_SUCCESS;
}

static bool mtlRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsMtl>((uint32_t)IGFX_METEORLAKE);

static bool arlRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsMtl>((uint32_t)IGFX_ARROWLAKE);
