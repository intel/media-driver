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
//! \file     media_libva_caps_xehp_sdv.cpp
//! \brief    This file implements the C++ class/interface for Xe_XPM media capbilities.
//!

#include "codec_def_encode_hevc_g12.h"
#include "media_libva.h"
#include "media_libva_util.h"
#include "media_libva_caps_xehp_sdv.h"
#include "media_libva_caps_factory.h"
#include "media_ddi_encode_const.h"

VAStatus MediaLibvaCapsXeHP::CheckEncodeResolution(
        VAProfile profile,
        uint32_t width,
        uint32_t height)
{
    VAStatus status = MediaLibvaCapsG12::CheckEncodeResolution(profile, width, height);

    if (!m_vdencActive && (profile == VAProfileHEVCMain || profile == VAProfileHEVCMain10))
    {
        if (width > m_maxHevcEncWidth
             || width < m_encMinWidth
             || height > m_hevcDPEncMaxHeight
             || height < m_encMinHeight)
        {
             return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
        }
    }
    return status;
}

VAStatus MediaLibvaCapsXeHP::GetPlatformSpecificAttrib(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t *value)
{
    VAStatus status = MediaLibvaCapsG12::GetPlatformSpecificAttrib(profile, entrypoint, type, value);

    if (entrypoint == VAEntrypointEncSlice || entrypoint == VAEntrypointFEI)
    {
        if (profile == VAProfileHEVCMain || profile == VAProfileHEVCMain10)
        {
            if ((int)type == VAConfigAttribMaxPictureHeight)
            {
                *value = m_hevcDPEncMaxHeight;
            }
        }
    }

    return status;
}

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

static bool xehpRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsXeHP>((uint32_t)IGFX_XE_HP_SDV);
