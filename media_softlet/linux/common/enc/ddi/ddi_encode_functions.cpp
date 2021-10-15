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
//! \file     ddi_encode_functions.cpp
//! \brief    ddi encode functions implementaion.
//!
#include "ddi_encode_functions.h"
#include "media_libva_util.h"
#include "media_libva_common_next.h"

VAStatus DdiEncodeFunctions::CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib   *attribList,
    int32_t           numAttribs,
    VAConfigID       *configId)
{
    VAStatus status = VA_STATUS_SUCCESS;
    DDI_CHK_NULL(configId,   "nullptr configId",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attribList, "nullptr attribList", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);

    status = mediaCtx->m_capsNext->CreateConfig(profile, entrypoint, attribList, numAttribs, configId);
    DDI_CHK_RET(status, "Create common config failed");

    uint32_t rcMode = 0;      
    uint32_t feiFunction = 0;

    for(int i = 0; i < numAttribs; i++)
    {
        if(attribList[i].type == VAConfigAttribFEIFunctionType)
        {
            feiFunction = attribList[i].value;
        }

        if(attribList[i].type == VAConfigAttribRateControl)
        {
            rcMode = attribList[i].value;
        }
    }

    auto configList = mediaCtx->m_capsNext->GetConfigList();
    DDI_CHK_NULL(configList, "Get configList failed", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(int i = 0; i < configList->size(); i++)
    {
        if((configList->at(i).profile == profile)        &&
           (configList->at(i).entrypoint == entrypoint))
        {
            if((rcMode      == configList->at(i).componentData.data.rcMode)       &&
               (feiFunction == configList->at(i).componentData.data.feiFunction))
            {
                *configId = ADD_CONFIG_ID_OFFSET(i);
                return VA_STATUS_SUCCESS;
            }
        }
    }

    *configId = 0xFFFFFFFF;
    return VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;
}

VAStatus DdiEncodeFunctions::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTargets,
    int32_t           renderTargetsNum,
    VAContextID       *context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    return VA_STATUS_SUCCESS;
}