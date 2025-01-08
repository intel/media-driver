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
//! \file     media_ddi_prot.cpp
//! \brief    The class implementation of DdiMediaProtected base for all protected session
//!

#include "codec_def_common_encode.h"
#include "codec_def_encode_jpeg.h"
#include "media_libva_util.h"
#include "media_libva_caps.h"
#include "media_libva.h"
#include "media_libva_caps_cp_interface.h"
#include "media_ddi_prot.h"

static bool isDefaultRegistered = DdiProtectedFactory::Register<DdiMediaProtected>(DDI_PROTECTED_DEFAULT);
static bool isProtectedContentRegistered = DdiProtectedFactory::Register<DdiMediaProtected>(DDI_PROTECTED_CONTENT);

std::map<uint32_t, DdiMediaProtected*> DdiMediaProtected::_impl;

DdiMediaProtected* DdiMediaProtected::GetInstance(uint32_t id)
{
    if (_impl[id] == nullptr)
    {
        _impl[id] = DdiProtectedFactory::Create(id);
        if (_impl[id] == nullptr)
        {
            DDI_ASSERTMESSAGE("DdiProtectedFactory::Create fail with id=%d", id);
            return nullptr;
        }
    }

    return _impl[id];
}

void DdiMediaProtected::FreeInstances()
{
    std::map<uint32_t, DdiMediaProtected*>::iterator it;
    for (it = _impl.begin(); it != _impl.end(); it++)
    {
        MOS_Delete(it->second);
    }
}

bool DdiMediaProtected::CheckEntrypointSupported(VAEntrypoint entrypoint)
{
    return false;
}

bool DdiMediaProtected::CheckAttribList(
    VAProfile profile,
    VAEntrypoint entrypoint,
    VAConfigAttrib* attrib,
    uint32_t numAttribs)
{
    return false;
}

VAStatus DdiMediaProtected::DdiMedia_CreateProtectedSession(
    VADriverContextP        ctx,
    VAConfigID              config_id,
    VAProtectedSessionID    *protected_session)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(protected_session, "nullptr protected_session",    VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaDrvCtx->m_caps, "nullptr mediaDrvCtx->m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    MediaLibvaCapsCpInterface *cpCaps = mediaDrvCtx->m_caps->GetCpCaps();
    DDI_CHK_NULL(cpCaps, "nullptr cpCaps", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (cpCaps->IsCpConfigId(config_id))
    {
        DdiMediaProtected *prot = DdiMediaProtected::GetInstance(DDI_PROTECTED_CONTENT);
        DDI_CHK_NULL(prot, "nullptr prot", VA_STATUS_ERROR_ALLOCATION_FAILED);
        vaStatus = prot->CreateProtectedSession(ctx, config_id - DDI_CP_GEN_CONFIG_ATTRIBUTES_BASE, protected_session);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Invalid config_id");
        vaStatus = VA_STATUS_ERROR_INVALID_CONFIG;
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;

}

VAStatus DdiMediaProtected::DdiMedia_DestroyProtectedSession(
    VADriverContextP        ctx,
    VAProtectedSessionID    protected_session)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    uint32_t            ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void *ctxPtr = DdiMedia_GetContextFromProtectedSessionID(ctx, protected_session, &ctxType);
    DDI_CHK_NULL(ctxPtr, "nullptr prot", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (ctxType == DDI_MEDIA_CONTEXT_TYPE_PROTECTED_CONTENT)
    {
        DdiMediaProtected *prot = DdiMediaProtected::GetInstance(DDI_PROTECTED_CONTENT);
        DDI_CHK_NULL(prot, "nullptr prot", VA_STATUS_ERROR_ALLOCATION_FAILED);
        vaStatus = prot->DestroyProtectedSession(ctx, protected_session);
    }
    else
    {
        vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiMediaProtected::DdiMedia_AttachProtectedSession(
    VADriverContextP        ctx,
    VAContextID             context,
    VAProtectedSessionID    protected_session)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromProtectedSessionID(ctx, protected_session, &ctxType);
    DDI_CHK_NULL(ctxPtr,      "nullptr ctxPtr",     VA_STATUS_ERROR_INVALID_CONTEXT);

    if (ctxType == DDI_MEDIA_CONTEXT_TYPE_PROTECTED_CONTENT)
    {
        DdiMediaProtected *prot = DdiMediaProtected::GetInstance(DDI_PROTECTED_CONTENT);
        DDI_CHK_NULL(prot, "nullptr prot", VA_STATUS_ERROR_ALLOCATION_FAILED);
        vaStatus = prot->AttachProtectedSession(ctx, context, protected_session);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Invalid protected_session");
        vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiMediaProtected::DdiMedia_DetachProtectedSession(
    VADriverContextP        ctx,
    VAContextID             context)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    if (context != 0)
    {
        uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
        void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
        DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);
        if (ctxType != DDI_MEDIA_CONTEXT_TYPE_DECODER &&
            ctxType != DDI_MEDIA_CONTEXT_TYPE_ENCODER &&
            ctxType != DDI_MEDIA_CONTEXT_TYPE_VP)
        {
            DDI_ASSERTMESSAGE("wrong context type %d", ctxType);
            return VA_STATUS_ERROR_INVALID_CONTEXT;
        }

        DdiMediaProtected *prot = DdiMediaProtected::GetInstance(DDI_PROTECTED_CONTENT);
        DDI_CHK_NULL(prot, "nullptr prot", VA_STATUS_ERROR_ALLOCATION_FAILED);
        vaStatus = prot->DetachProtectedSession(ctx, context);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiMediaProtected::DdiMedia_ProtectedSessionExecute(
    VADriverContextP        ctx,
    VAProtectedSessionID    protected_session,
    VABufferID              data)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromProtectedSessionID(ctx, protected_session, &ctxType);
    DDI_CHK_NULL(ctxPtr,      "nullptr ctxPtr",     VA_STATUS_ERROR_INVALID_CONTEXT);

    if (ctxType == DDI_MEDIA_CONTEXT_TYPE_PROTECTED_CONTENT)
    {
        DdiMediaProtected *prot = DdiMediaProtected::GetInstance(DDI_PROTECTED_CONTENT);
        DDI_CHK_NULL(prot, "nullptr prot", VA_STATUS_ERROR_ALLOCATION_FAILED);
        vaStatus = prot->ProtectedSessionExecute(ctx, protected_session, data);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Invalid protected_session");
        vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    return vaStatus;
}

VAStatus DdiMediaProtected::DdiMedia_ProtectedSessionCreateBuffer(
    VADriverContextP        ctx,
    VAContextID             context,
    VABufferType            type,
    uint32_t                size,
    uint32_t                num_elements,
    void                    *data,
    VABufferID              *bufId)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(bufId, "nullptr bufId", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromProtectedSessionID(ctx, context, &ctxType);
    DDI_CHK_NULL(ctxPtr,      "nullptr ctxPtr",     VA_STATUS_ERROR_INVALID_CONTEXT);

    if (ctxType == DDI_MEDIA_CONTEXT_TYPE_PROTECTED_CONTENT)
    {
        DdiMediaProtected *prot = DdiMediaProtected::GetInstance(DDI_PROTECTED_CONTENT);
        DDI_CHK_NULL(prot, "nullptr prot", VA_STATUS_ERROR_ALLOCATION_FAILED);
        vaStatus = prot->ProtectedSessionCreateBuffer(ctx, context, type, size, num_elements, data, bufId);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Invalid protected_session");
        vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    return vaStatus;
}

VAStatus DdiMediaProtected::CreateProtectedSession(
    VADriverContextP        ctx,
    VAConfigID              config_id,
    VAProtectedSessionID    *protected_session)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaProtected::DestroyProtectedSession(
    VADriverContextP        ctx,
    VAProtectedSessionID    protected_session)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaProtected::AttachProtectedSession(
    VADriverContextP        ctx,
    VAContextID             context,
    VAProtectedSessionID    protected_session)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaProtected::DetachProtectedSession(
    VADriverContextP        ctx,
    VAContextID             context)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaProtected::ProtectedSessionExecute(
    VADriverContextP        ctx,
    VAProtectedSessionID    protected_session,
    VABufferID              data)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMediaProtected::ProtectedSessionCreateBuffer(
    VADriverContextP        ctx,
    VAProtectedSessionID    protected_session,
    VABufferType            type,
    uint32_t                size,
    uint32_t                num_elements,
    void                    *data,
    VABufferID              *bufId)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

uint64_t DdiMediaProtected::GetProtectedSurfaceTag(PDDI_MEDIA_CONTEXT media_ctx)
{
    return 0;
}

void DdiMedia_FreeProtectedSessionHeap(
    VADriverContextP ctx,
    PDDI_MEDIA_HEAP contextHeap,
    int32_t vaContextOffset,
    int32_t ctxNums)
{
    if (nullptr == ctx || nullptr == contextHeap)
    {
        DDI_ASSERTMESSAGE("DDI: Invalid input arguments");
        return;
    }

    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT mediaContextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)contextHeap->pHeapBase;
    if (nullptr == mediaContextHeapBase)
        return;

    for (int32_t elementId = 0; ctxNums > 0  && elementId < contextHeap->uiAllocatedHeapElements; ++elementId)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT mediaContextHeapElmt = &mediaContextHeapBase[elementId];
        if (nullptr != mediaContextHeapElmt)
        {
            if (nullptr == mediaContextHeapElmt->pVaContext)
                continue;
            VAContextID vaCtxID = (VAContextID)(mediaContextHeapElmt->uiVaContextID + vaContextOffset);
            DdiMediaProtected::DdiMedia_DestroyProtectedSession(ctx, vaCtxID);
        }
        else
        {
            DDI_ASSERTMESSAGE("DDI: Invalid mediaContextHeapElmt");
        }
        ctxNums--;
    }
}

static void* DdiMedia_GetVaContextFromHeap(
    PDDI_MEDIA_HEAP mediaHeap,
    uint32_t index,
    PMEDIA_MUTEX_T mutex)
{
    if (nullptr == mutex)
    {
        DDI_ASSERTMESSAGE("DDI: Invalid input arguments");
        return nullptr;
    }

    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  vaCtxHeapElmt = nullptr;
    void                              *context = nullptr;

    DdiMediaUtil_LockMutex(mutex);
    if(nullptr == mediaHeap || index >= mediaHeap->uiAllocatedHeapElements)
    {
        DdiMediaUtil_UnLockMutex(mutex);
        return nullptr;
    }
    vaCtxHeapElmt  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaHeap->pHeapBase;
    vaCtxHeapElmt += index;
    context        = vaCtxHeapElmt->pVaContext;
    DdiMediaUtil_UnLockMutex(mutex);

    return context;
}

void* DdiMedia_GetContextFromProtectedSessionID(
    VADriverContextP ctx,
    VAProtectedSessionID vaID,
    uint32_t *ctxType)
{
    PDDI_MEDIA_CONTEXT       mediaCtx = nullptr;
    uint32_t                 heap_index = 0, prot_index = 0;
    void                     *pSession = nullptr;

    DDI_CHK_NULL(ctx, "nullptr ctx", nullptr);
    DDI_CHK_NULL(ctxType, "nullptr ctxType", nullptr);

    mediaCtx  = DdiMedia_GetMediaContext(ctx);
    prot_index    = vaID & DDI_MEDIA_MASK_VACONTEXTID;
    heap_index    = vaID & DDI_MEDIA_MASK_VAPROTECTEDSESSION_ID;

    if ((vaID&DDI_MEDIA_MASK_VACONTEXT_TYPE) != DDI_MEDIA_VACONTEXTID_OFFSET_PROT)
    {
        DDI_ASSERTMESSAGE("Invalid protected session: 0x%x", vaID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
        return nullptr;
    }

    // 0        ~ 0x8000000: LP context
    // 0x8000000~0x10000000: CP context
    if (prot_index < DDI_MEDIA_VACONTEXTID_OFFSET_PROT_CP)
    {
        DDI_VERBOSEMESSAGE("LP protected session detected: 0x%x", vaID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_PROTECTED_LINK;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pProtCtxHeap, heap_index, &mediaCtx->ProtMutex);
    }

    DDI_VERBOSEMESSAGE("CP protected session detected: 0x%x", vaID);
    *ctxType = DDI_MEDIA_CONTEXT_TYPE_PROTECTED_CONTENT;
    return DdiMedia_GetVaContextFromHeap(mediaCtx->pProtCtxHeap, heap_index, &mediaCtx->ProtMutex);
}
