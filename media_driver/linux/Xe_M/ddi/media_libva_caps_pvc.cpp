/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     media_libva_caps_pvc.cpp
//! \brief    This file implements the C++ class/interface for pvc media capbilities.
//!

#include "media_libva.h"
#include "media_libva_util.h"
#include "media_libva_caps_pvc.h"
#include "media_libva_caps_factory.h"
#include "mos_bufmgr_priv.h"

extern template class MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>;

VAStatus MediaLibvaCapsPVC::LoadAv1DecProfileEntrypoints()
{
    VAStatus status = VA_STATUS_SUCCESS;

#if _AV1_DECODE_SUPPORTED
    if (m_mediaCtx->pDrmBufMgr->has_full_vd)
    {
        status = MediaLibvaCapsG12::LoadAv1DecProfileEntrypoints();
    }
#endif
    return status;
}

VAStatus MediaLibvaCapsPVC::LoadHevcEncLpProfileEntrypoints()
{
    DDI_CHK_NULL(m_mediaCtx, "nullptr m_mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_mediaCtx->pDrmBufMgr, "nullptr pDrmBufMgr", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_CONDITION(m_mediaCtx->pDrmBufMgr->has_full_vd == false,
       "Encoding is not supported",
        VA_STATUS_SUCCESS);

    VAStatus status = VA_STATUS_SUCCESS;
    const uint8_t rcModeSize = (sizeof(m_encRcMode))/(sizeof(m_encRcMode[0]));

    AttribMap *attributeList = nullptr;

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain444)
            || MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10bit444))
    {
        status = CreateEncAttributes(VAProfileHEVCMain, VAEntrypointEncSliceLP, &attributeList);
        DDI_CHK_RET(status, "Failed to initialize Caps!");
        (*attributeList)[VAConfigAttribMaxPictureWidth] = CODEC_8K_MAX_PIC_WIDTH;
        (*attributeList)[VAConfigAttribMaxPictureHeight] = CODEC_8K_MAX_PIC_HEIGHT;
        (*attributeList)[VAConfigAttribEncTileSupport] = 1;
        (*attributeList)[VAConfigAttribEncSliceStructure] = VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS |
                                                        VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS;
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
            }
        }
        AddProfileEntry(VAProfileHEVCMain, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
            }
        }
        AddProfileEntry(VAProfileHEVCMain10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain444))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
            }
        }
        AddProfileEntry(VAProfileHEVCMain444, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEncodeHEVCVdencMain10bit444))
    {
        uint32_t configStartIdx = m_encConfigs.size();
        AddEncConfig(VA_RC_CQP);
        if (MEDIA_IS_SKU(&(m_mediaCtx->SkuTable), FtrEnableMediaKernels))
        {
            for (int32_t j = 3; j < rcModeSize; j++)
            {
                AddEncConfig(m_encRcMode[j]);
            }
        }
        AddProfileEntry(VAProfileHEVCMain444_10, VAEntrypointEncSliceLP, attributeList,
                configStartIdx, m_encConfigs.size() - configStartIdx);
    }

    return status;
}

VAStatus MediaLibvaCapsPVC::CheckEncodeResolution(
        VAProfile profile,
        uint32_t width,
        uint32_t height)
{
    switch (profile)
    {
        case VAProfileHEVCMain:
        case VAProfileHEVCMain10:
        case VAProfileHEVCMain444:
        case VAProfileHEVCMain444_10:
            if (width > CODEC_8K_MAX_PIC_WIDTH
                    || width < m_hevcVDEncMinWidth
                    || height > CODEC_8K_MAX_PIC_HEIGHT
                    || height < m_hevcVDEncMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
        default:
            if (width > m_encMax4kWidth
                    || width < m_encMinWidth
                    || height > m_encMax4kHeight
                    || height < m_encMinHeight)
            {
                return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
            }
            break;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsPVC::AddEncSurfaceAttributes(
        VAProfile profile,
        VAEntrypoint entrypoint,
        VASurfaceAttrib *attribList,
        uint32_t &numAttribs)
{
    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(entrypoint == VAEntrypointEncSliceLP)
    {
        if (profile == VAProfileHEVCMain10)
        {
            attribList[numAttribs].type = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type = VAGenericValueTypeInteger;
            attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('P', '0', '1', '0');
            numAttribs++;
        }
        else if (profile == VAProfileHEVCMain444)
        {
            attribList[numAttribs].type = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type = VAGenericValueTypeInteger;
            attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_AYUV;
            numAttribs++;
        }
        else if (profile == VAProfileHEVCMain444_10)
        {
            attribList[numAttribs].type = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type = VAGenericValueTypeInteger;
            attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC_Y410;
            numAttribs++;
        }
        else
        {
            attribList[numAttribs].type = VASurfaceAttribPixelFormat;
            attribList[numAttribs].value.type = VAGenericValueTypeInteger;
            attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE;
            attribList[numAttribs].value.value.i = VA_FOURCC('N', 'V', '1', '2');
            numAttribs++;
        }
        attribList[numAttribs].type = VASurfaceAttribMaxWidth;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = CODEC_MAX_PIC_WIDTH;

        if(IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_8K_MAX_PIC_WIDTH;
        }
        if(IsAvcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_4K_MAX_PIC_WIDTH;
        }
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMaxHeight;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = CODEC_MAX_PIC_HEIGHT;
        if(IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_8K_MAX_PIC_HEIGHT;
        }
        if(IsAvcProfile(profile))
        {
            attribList[numAttribs].value.value.i = CODEC_4K_MAX_PIC_HEIGHT;
        }
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMinWidth;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = m_encMinWidth;
        if(IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = m_hevcVDEncMinWidth;
        }
        numAttribs++;

        attribList[numAttribs].type = VASurfaceAttribMinHeight;
        attribList[numAttribs].value.type = VAGenericValueTypeInteger;
        attribList[numAttribs].flags = VA_SURFACE_ATTRIB_GETTABLE;
        attribList[numAttribs].value.value.i = m_encMinHeight;
        if(IsHevcProfile(profile))
        {
            attribList[numAttribs].value.value.i = m_hevcVDEncMinHeight;
        }
        numAttribs++;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsPVC::GetPlatformSpecificAttrib(VAProfile profile,
        VAEntrypoint entrypoint,
        VAConfigAttribType type,
        uint32_t *value)
{
    DDI_CHK_NULL(value, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAStatus status = MediaLibvaCapsG12::GetPlatformSpecificAttrib(profile, entrypoint, type, value);

    if ((int)type == VAConfigAttribDecProcessing)
    {
        //PVC doesn't have SFC
        *value = VA_DEC_PROCESSING_NONE;
    }

    return status;
}

VAStatus MediaLibvaCapsPVC::GetDisplayAttributes(
            VADisplayAttribute *attribList,
            int32_t numAttribs)
{
    DDI_CHK_NULL(attribList, "Null attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    for(auto i = 0; i < numAttribs; i ++)
    {
        switch(attribList->type)
        {
#if VA_CHECK_VERSION(1, 15, 0)
            case VADisplayPCIID:
                attribList->min_value = attribList->value = attribList->max_value = (m_mediaCtx->iDeviceId & 0xffff) | 0x80860000;
                attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
                break;
#endif
            case VADisplayAttribCopy:
                attribList->min_value = attribList->value = attribList->max_value =
                    (1 << VA_EXEC_MODE_POWER_SAVING) | (1 << VA_EXEC_MODE_PERFORMANCE);
                // 100: perfromance model: Render copy
                // 10:  POWER_SAVING: BLT
                // 1:   default model: 1: vebox
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
        attribList ++;
    }
    return VA_STATUS_SUCCESS;
}

static bool pvcRegistered = MediaLibvaCapsFactory<MediaLibvaCaps, DDI_MEDIA_CONTEXT>::
    RegisterCaps<MediaLibvaCapsPVC>((uint32_t)IGFX_PVC);
