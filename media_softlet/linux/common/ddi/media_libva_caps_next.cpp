/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_libva_caps_next.cpp
//! \brief    This file implements the base C++ class/interface for media capbilities.
//!

#include "media_libva_caps_next.h"

MediaLibvaCapsNext::MediaLibvaCapsNext(DDI_MEDIA_CONTEXT *mediaCtx)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx,    "Media context is null", );
    m_mediaCtx  = mediaCtx;

    m_capsTable = MOS_New(MediaCapsTableSpecific, mediaCtx->m_hwInfo->GetDeviceInfo());
    DDI_CHK_NULL(m_capsTable, "Caps table is null", );
}

MediaLibvaCapsNext::~MediaLibvaCapsNext()
{
    DDI_FUNC_ENTER;
    MOS_Delete(m_capsTable);
    m_capsTable = nullptr;
}

MediaLibvaCapsNext* MediaLibvaCapsNext::CreateCaps(DDI_MEDIA_CONTEXT *mediaCtx)
{
    DDI_FUNC_ENTER;

    if(mediaCtx != nullptr)
    {
        MediaLibvaCapsNext *caps = MOS_New(MediaLibvaCapsNext, mediaCtx);
        return caps;
    }
    else
    {
        return nullptr;
    }
}

VAStatus MediaLibvaCapsNext::Init()
{
    DDI_FUNC_ENTER;

    return m_capsTable->Init(m_mediaCtx);
}

ConfigList* MediaLibvaCapsNext::GetConfigList()
{
    return m_capsTable->GetConfigList();
}

VAStatus MediaLibvaCapsNext::GetAttribValue(
    VAProfile           profile,
    VAEntrypoint        entrypoint,
    VAConfigAttribType  type,
    uint32_t            *value)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable, "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(value,       "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    AttribList  *attribList = nullptr;
    attribList = m_capsTable->QuerySupportedAttrib(profile, entrypoint);
    DDI_CHK_NULL(attribList, "AttribList in null, not supported attribute",  VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int i = 0; i < attribList->size(); i++)
    {
        if(attribList->at(i).type == type)
        {
            *value = attribList->at(i).value; 
            return VA_STATUS_SUCCESS;
        }
    }
    return VA_STATUS_ERROR_INVALID_CONFIG;
}

uint32_t MediaLibvaCapsNext::GetImageFormatsMaxNum()
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(m_capsTable, "Caps table is null", 0);

    return m_capsTable->GetImageFormatsMaxNum();
}

VAStatus MediaLibvaCapsNext::QueryConfigProfiles(
    VAProfile *profileList,
    int32_t   *profilesNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable, "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(profileList, "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(profilesNum, "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    return m_capsTable->QueryConfigProfiles(profileList, profilesNum);
}

VAStatus MediaLibvaCapsNext::QueryConfigAttributes(
    VAConfigID     configId,
    VAProfile      *profile,
    VAEntrypoint   *entrypoint,
    VAConfigAttrib *attribList,
    int32_t        *numAttribs)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable,  "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(profile,      "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint,   "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attribList,   "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numAttribs,   "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus     status = VA_STATUS_SUCCESS;
    ConfigLinux  *configItem = nullptr;
    configItem = m_capsTable->QueryConfigItemFromIndex(configId);
    DDI_CHK_NULL(configItem,   "QueryConfigItemFromIndex failed",  VA_STATUS_ERROR_INVALID_PARAMETER);

    *profile        = configItem->profile;
    *entrypoint     = configItem->entrypoint;
    *numAttribs     = configItem->numAttribs;
    for (int i = 0; i < configItem->numAttribs; ++i)
    {
        if (configItem->attribList[i].value != VA_ATTRIB_NOT_SUPPORTED)
        {
            attribList[i] = configItem->attribList[i];
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsNext::CheckAttribList(
    VAProfile       profile,
    VAEntrypoint    entrypoint,
    VAConfigAttrib  *attrib,
    int32_t         numAttribs)
{
    DDI_FUNC_ENTER;

    VAStatus     status = VA_STATUS_SUCCESS;
    AttribList  *supportedAttribList = nullptr;
    supportedAttribList =  m_capsTable->QuerySupportedAttrib(profile, entrypoint);
    DDI_CHK_NULL(supportedAttribList, "AttribList in null, not supported attribute", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(int32_t j = 0; j < numAttribs; j ++)
    {
        bool isValidAttrib = false;

        // temp solution for MV tools, after tool change, it should be removed
        if(attrib[j].type == VAConfigAttribEncDynamicScaling ||
           attrib[j].type == VAConfigAttribEncRateControlExt ||
           attrib[j].type == VAConfigAttribEncTileSupport)
        {
            if(attrib[j].value == VA_ATTRIB_NOT_SUPPORTED)
            {
                isValidAttrib = true;
                continue;
            }
        }

        bool findSameType = false;
        for(int i = 0; i < supportedAttribList->size(); i++)
        {
            // Same attribute type
            findSameType = true;
            if(supportedAttribList->at(i).type == attrib[j].type)
            {
                if(attrib[j].value == CONFIG_ATTRIB_NONE)
                {
                    isValidAttrib = true;
                    continue;
                }

                if(attrib[j].type == VAConfigAttribRTFormat         ||
                   attrib[j].type == VAConfigAttribDecSliceMode     ||
                   attrib[j].type == VAConfigAttribDecJPEG          ||
                   attrib[j].type == VAConfigAttribRateControl      ||
                   attrib[j].type == VAConfigAttribEncPackedHeaders ||
                   attrib[j].type == VAConfigAttribEncIntraRefresh  ||
                   attrib[j].type == VAConfigAttribFEIFunctionType  ||
                   attrib[j].type == VAConfigAttribEncryption)
                {
                    if((supportedAttribList->at(i).value & attrib[j].value) == attrib[j].value)
                    {
                        isValidAttrib = true;
                        continue;
                    }
                    else if(attrib[j].type == VAConfigAttribRTFormat)
                    {
                        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
                    }
                }
                else if(supportedAttribList->at(i).value == attrib[j].value)
                {
                    isValidAttrib = true;
                    continue;
                }
                else if(attrib[j].type == VAConfigAttribEncSliceStructure)
                {
                    if((supportedAttribList->at(i).value & attrib[j].value) == attrib[j].value)
                    {
                        isValidAttrib = true;
                        continue;
                    }

                    if(supportedAttribList->at(i).value & VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS)
                    {
                        if( (attrib[j].value & VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS)        ||
                            (attrib[j].value & VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS)  ||
                            (attrib[j].value & VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS) ||
                            (attrib[j].value & VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS))
                        {
                            isValidAttrib = true;
                            continue;
                        }
                    }
                    else if (supportedAttribList->at(i).value &
                            (VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE))
                    {
                        if((attrib[j].value & VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS) ||
                           (attrib[j].value & VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS)     ||
                           (attrib[j].value & VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS))
                        {
                            isValidAttrib = true;
                            continue;
                        }
                    }
                }
                else if((attrib[j].type == VAConfigAttribMaxPictureWidth)  ||
                        (attrib[j].type == VAConfigAttribMaxPictureHeight) ||
                        (attrib[j].type == VAConfigAttribEncROI)           ||
                        (attrib[j].type == VAConfigAttribEncDirtyRect))
                {
                    if(attrib[j].value <= supportedAttribList->at(i).value)
                    {
                        isValidAttrib = true;
                        continue;
                    }
                }
                else if(attrib[j].type == VAConfigAttribEncMaxRefFrames)
                {
                    if( ((attrib[j].value & 0xffff) <= (supportedAttribList->at(i).value & 0xffff))  &&
                        (attrib[j].value <= supportedAttribList->at(i).value))  //high16 bit  can compare with this way
                    {
                        isValidAttrib = true;
                        continue;
                    }
                }
                else if(attrib[j].type == VAConfigAttribEncJPEG)
                {
                    VAConfigAttribValEncJPEG jpegValue, jpegSetValue;
                    jpegValue.value    = attrib[j].value;
                    jpegSetValue.value = supportedAttribList->at(i).value;

                    if( (jpegValue.bits.max_num_quantization_tables <= jpegSetValue.bits.max_num_quantization_tables)  &&
                        (jpegValue.bits.max_num_huffman_tables      <= jpegSetValue.bits.max_num_huffman_tables)       &&
                        (jpegValue.bits.max_num_scans               <= jpegSetValue.bits.max_num_scans)                &&
                        (jpegValue.bits.max_num_components          <= jpegSetValue.bits.max_num_components))
                    {
                        isValidAttrib = true;
                        continue;
                    }
                }
            }
        }

        // should be removed after msdk remove VAConfigAttribSpatialResidual attributes for VPP
        if(!findSameType)
        {
            if((profile        == VAProfileNone)                  &&
               (entrypoint     == VAEntrypointVideoProc)          &&
               (attrib[j].type == VAConfigAttribSpatialClipping))
            {
                isValidAttrib = true;
                continue;
            }
            else if((profile == VAProfileNone)               &&
                    (attrib[j].type == VAConfigAttribStats))
            {
                isValidAttrib = true;
                continue;
            }
        }

        if(!isValidAttrib)
        {
            return VA_STATUS_ERROR_INVALID_VALUE;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsNext::CreateConfig(
    VAProfile       profile,
    VAEntrypoint    entrypoint,
    VAConfigAttrib  *attribList,
    int32_t         numAttribs,
    VAConfigID      *configId)
{
    VAStatus  status = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable,  "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(configId,     "nullptr configId",   VA_STATUS_ERROR_INVALID_PARAMETER);

    status = m_capsTable->CreateConfig(profile, entrypoint, attribList, numAttribs, configId);
    if(status != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Query Supported Attrib Failed");
        return status;
    }

    return CheckAttribList(profile, entrypoint, attribList, numAttribs);
}

VAStatus MediaLibvaCapsNext::DestroyConfig(VAConfigID configId)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable,  "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);

    return m_capsTable->DestroyConfig(configId);
}

VAStatus MediaLibvaCapsNext::GetConfigAttributes(
    VAProfile       profile,
    VAEntrypoint    entrypoint,
    VAConfigAttrib  *attribList,
    int32_t         numAttribs)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable,  "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attribList,   "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    AttribList  *supportedAttribList = nullptr;
    supportedAttribList = m_capsTable->QuerySupportedAttrib(profile, entrypoint);
    DDI_CHK_NULL(supportedAttribList, "AttribList in null, not supported attribute", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int32_t j = 0; j < numAttribs; j++)
    {
        //For unknown attribute, set to VA_ATTRIB_NOT_SUPPORTED
        attribList[j].value = VA_ATTRIB_NOT_SUPPORTED;

        for(int32_t i = 0; i < supportedAttribList->size(); i++)
        {
            if(attribList[j].type == supportedAttribList->at(i).type)
            {
                attribList[j].value = supportedAttribList->at(i).value;
                break;
            }
            else
            {
               GetGeneralConfigAttrib(&attribList[j]);
            }
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsNext::GetGeneralConfigAttrib(VAConfigAttrib *attrib)
{
    static const std::map<VAConfigAttribType, uint32_t> generalAttribMap = {
#if VA_CHECK_VERSION(1, 10, 0)
    {VAConfigAttribContextPriority, CONTEXT_PRIORITY_MAX},
#endif
    };

    DDI_CHK_NULL(attrib, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (generalAttribMap.find(attrib->type) != generalAttribMap.end())
    {
        attrib->value = CONTEXT_PRIORITY_MAX;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsNext::QueryConfigEntrypoints(
    VAProfile     profile,
    VAEntrypoint  *entrypointList,
    int32_t       *entrypointNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable,    "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypointList, "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypointNum,  "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    EntrypointMap *entryMap = nullptr;
    VAStatus      status = VA_STATUS_SUCCESS;

    entryMap = m_capsTable->QueryConfigEntrypointsMap(profile);
    DDI_CHK_NULL(entryMap,       "QueryConfigEntrypointsMap failed", VA_STATUS_ERROR_UNSUPPORTED_PROFILE);

    int i = 0;
    for (auto it = entryMap->begin(); it!=entryMap->end(); ++it)
    {
        entrypointList[i] = (VAEntrypoint)(it->first);
        i++;
    }

    *entrypointNum = entryMap->size();
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsNext::QuerySurfaceAttributes(
    VAConfigID       configId,
    VASurfaceAttrib  *attribList,
    uint32_t         *numAttribs)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable, "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numAttribs,  "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    if (attribList == nullptr)
    {
        *numAttribs = DDI_CODEC_GEN_MAX_SURFACE_ATTRIBUTES;
        return VA_STATUS_SUCCESS;
    }

    ProfileSurfaceAttribInfo *surfaceAttribInfo = nullptr;
    VAStatus                 status = VA_STATUS_SUCCESS;

    surfaceAttribInfo = m_capsTable->QuerySurfaceAttributesFromConfigId(configId);
    DDI_CHK_NULL(surfaceAttribInfo, "QuerySurfaceAttributesFromConfigId failed", VA_STATUS_ERROR_INVALID_PARAMETER);

    VASurfaceAttrib *attribs = (VASurfaceAttrib *)MOS_AllocAndZeroMemory(DDI_CODEC_GEN_MAX_SURFACE_ATTRIBUTES * sizeof(*attribs));
    if (attribs == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    int i = 0;
    for (uint32_t j = 0; j < surfaceAttribInfo->size(); j++)
    {
        attribs[i].type =          surfaceAttribInfo->at(j).type1;
        attribs[i].flags =         surfaceAttribInfo->at(j).flags;
        attribs[i].value.type =    surfaceAttribInfo->at(j).value.type;
        if(attribs[i].value.type == VAGenericValueTypeInteger)
        {
            attribs[i].value.value.i = surfaceAttribInfo->at(j).value.value.i;
        }
        else if(attribs[i].value.type == VAGenericValueTypePointer)
        {
            attribs[i].value.value.p = surfaceAttribInfo->at(j).value.value.p;
        }
        else
        {
            DDI_ASSERTMESSAGE("Invalid VAGenericValueType");
            MOS_FreeMemory(attribs);
            return VA_STATUS_ERROR_UNIMPLEMENTED;
        }
        ++i;
    }

    if (i > *numAttribs)
    {
        *numAttribs = i;
        MOS_FreeMemory(attribs);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    *numAttribs = i;
    MOS_SecureMemcpy(attribList, i * sizeof(*attribs), attribs, i * sizeof(*attribs));
    MOS_FreeMemory(attribs);

    return status;
}

VAStatus MediaLibvaCapsNext::QueryImageFormats(
    VAImageFormat *formatList,
    int32_t       *numFormats)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable, "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(formatList,  "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(numFormats,  "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    int   num    = 0;
    auto  imgTbl = m_capsTable->GetImgTable();
    MOS_ZeroMemory(formatList, sizeof(VAImageFormat) * imgTbl->size());

    for(auto imgTblIter : *imgTbl)
    {
        formatList[num].fourcc           = imgTblIter.first;
        auto imageFormatInfo             = imgTblIter.second;
        DDI_CHK_NULL(imageFormatInfo,  "Null pointer",  VA_STATUS_ERROR_INVALID_PARAMETER);

        formatList[num].byte_order       = imageFormatInfo->byte_order;
        formatList[num].bits_per_pixel   = imageFormatInfo->bits_per_pixel;
        formatList[num].depth            = imageFormatInfo->depth;
        formatList[num].red_mask         = imageFormatInfo->red_mask;
        formatList[num].green_mask       = imageFormatInfo->green_mask;
        formatList[num].blue_mask        = imageFormatInfo->blue_mask;
        formatList[num].alpha_mask       = imageFormatInfo->alpha_mask;

        num++;
    }

    *numFormats = num;
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaCapsNext::PopulateColorMaskInfo(VAImageFormat *vaImgFmt)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(m_capsTable, "Caps table is null", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(vaImgFmt,    "Null pointer",       VA_STATUS_ERROR_INVALID_PARAMETER);

    auto imgTbl = m_capsTable->GetImgTable();

    if(imgTbl->find(vaImgFmt->fourcc) == imgTbl->end())
    {
        return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
    }

    auto imageFormatInfo = imgTbl->at(vaImgFmt->fourcc);
    DDI_CHK_NULL(imageFormatInfo,  "Null pointer",  VA_STATUS_ERROR_INVALID_PARAMETER);

    vaImgFmt->red_mask   = imageFormatInfo->red_mask;
    vaImgFmt->green_mask = imageFormatInfo->green_mask;
    vaImgFmt->blue_mask  = imageFormatInfo->blue_mask;
    vaImgFmt->alpha_mask = imageFormatInfo->alpha_mask;

    return VA_STATUS_SUCCESS;
}


VAStatus MediaLibvaCapsNext::QueryDisplayAttributes(
    VADisplayAttribute *attribList,
    int32_t            *attributesNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(attribList, "Null attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attributesNum, "Null attribs", VA_STATUS_ERROR_INVALID_PARAMETER);
    VADisplayAttribute * attrib = attribList;
    *attributesNum = 0;

    attrib->type = VADisplayAttribCopy;
    (*attributesNum) ++;

#if VA_CHECK_VERSION(1, 15, 0)
    attrib ++;

    attrib->type = VADisplayPCIID;
    (*attributesNum) ++;
#endif

    return GetDisplayAttributes(attribList, *attributesNum);
}

VAStatus MediaLibvaCapsNext::GetDisplayAttributes(
    VADisplayAttribute *attribList,
    int32_t             attributesNum)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(attribList, "Null attribList", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(auto i = 0; i < attributesNum; i ++)
    {
        switch(attribList->type)
        {
            case VADisplayAttribCopy:
                attribList->min_value = attribList->value = attribList->max_value = 0;
                attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
                break;
#if VA_CHECK_VERSION(1, 15, 0)
            case VADisplayPCIID:
                attribList->min_value = attribList->value = attribList->max_value = (m_mediaCtx->iDeviceId & 0xffff) | 0x80860000;
                attribList->flags = VA_DISPLAY_ATTRIB_GETTABLE;
                break;
#endif
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
