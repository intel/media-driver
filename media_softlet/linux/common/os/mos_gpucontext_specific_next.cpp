/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file    mos_gpucontext_specific_next.cpp
//! \brief   Container class for the Linux specific gpu context
//!

#include <unistd.h>
#include "mos_gpucontext_specific_next.h"
#include "mos_context_specific_next.h"
#include "mos_graphicsresource_specific_next.h"
#include "mos_commandbuffer_specific_next.h"
#include "mos_util_devult_specific_next.h"
#include "mos_cmdbufmgr_next.h"
#include "mos_os_virtualengine_next.h"
#include "mos_interface.h"
#include "mos_os_cp_interface_specific.h"
#ifdef ENABLE_XE_KMD
#include "mos_gpucontext_specific_next_xe.h"
#endif

#define MI_BATCHBUFFER_END 0x05000000
static pthread_mutex_t command_dump_mutex = PTHREAD_MUTEX_INITIALIZER;

void GpuContextSpecificNext::StoreCreateOptions(PMOS_GPUCTX_CREATOPTIONS createoption)
{
    if (typeid(*createoption) == typeid(MOS_GPUCTX_CREATOPTIONS_ENHANCED))
    {
        m_bEnhancedUsed = true;
        MosUtilities::MosSecureMemcpy(&m_createOptionEnhanced, sizeof(MOS_GPUCTX_CREATOPTIONS_ENHANCED), createoption, sizeof(MOS_GPUCTX_CREATOPTIONS_ENHANCED));
    }
    else
    {
        MosUtilities::MosSecureMemcpy(&m_createOption, sizeof(MOS_GPUCTX_CREATOPTIONS), createoption, sizeof(MOS_GPUCTX_CREATOPTIONS));
    }
}

GpuContextSpecificNext::GpuContextSpecificNext(
    const MOS_GPU_NODE gpuNode,
    CmdBufMgrNext         *cmdBufMgr,
    GpuContextNext        *reusedContext)
{
    MOS_OS_FUNCTION_ENTER;

    m_nodeOrdinal          = gpuNode;
    m_cmdBufMgr            = cmdBufMgr;
    m_statusBufferResource = nullptr;
    m_maxPatchLocationsize = PATCHLOCATIONLIST_SIZE;

    if (reusedContext)
    {
        MOS_OS_NORMALMESSAGE("gpucontex reusing not enabled on Linux.");
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // get user engine instance setting from environment variable
    char *engineInstances = getenv("INTEL_ENGINE_INSTANCE");
    if (engineInstances != nullptr)
    {
        errno             = 0;
        long int instance = strtol(engineInstances, nullptr, 16);
        /* Check for various possible errors. */
        if ((errno == ERANGE && instance == LONG_MAX) || (instance < 0))
        {
            MOS_OS_NORMALMESSAGE("Invalid INTEL_ENGINE_INSTANCE setting.(%s)\n", engineInstances);
            m_engineInstanceSelect = 0x0;
        }
        else
        {
            m_engineInstanceSelect = (uint32_t)instance;
        }
    }
#endif
}

GpuContextSpecificNext::~GpuContextSpecificNext()
{
    MOS_OS_FUNCTION_ENTER;

    Clear();
}

GpuContextNext *GpuContextSpecificNext::Create(
    const MOS_GPU_NODE    gpuNode,
    CmdBufMgrNext         *cmdBufMgr,
    GpuContextNext        *reusedContext)
{
    MOS_OS_FUNCTION_ENTER;
    if (nullptr == cmdBufMgr)
    {
        return nullptr;
    }
    OsContextSpecificNext *osDeviceContext = dynamic_cast<OsContextSpecificNext*>(cmdBufMgr->m_osContext);
    if (nullptr == osDeviceContext)
    {
        return nullptr;
    }
    int type = osDeviceContext->GetDeviceType();
    if (DEVICE_TYPE_I915 == type)
    {
        return MOS_New(GpuContextSpecificNext, gpuNode, cmdBufMgr, reusedContext);
    }
#ifdef ENABLE_XE_KMD
    else if (DEVICE_TYPE_XE == type)
    {
        return MOS_New(GpuContextSpecificNextXe, gpuNode, cmdBufMgr, reusedContext);
    }
#endif
    return nullptr;
}

MOS_STATUS GpuContextSpecificNext::RecreateContext(bool bIsProtected, MOS_STREAM_HANDLE streamState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    // clear existing context
    Clear();
    m_bProtectedContext = bIsProtected;
    PMOS_GPUCTX_CREATOPTIONS createOption;
    if (m_bEnhancedUsed)
    {
        createOption = &m_createOptionEnhanced;
    }
    else
    {
        createOption = &m_createOption;
    }
    eStatus = Init(m_osContext, streamState, createOption);
    return eStatus;
}

MOS_STATUS GpuContextSpecificNext::PatchGPUContextProtection(MOS_STREAM_HANDLE streamState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(streamState);
    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osParameters);

    // clean up clear gem context and create protected gem context if CP is enabled
    if (streamState->osCpInterface &&
        streamState->osCpInterface->IsCpEnabled())    // Check if CP is enabled as protected GEM context is only needed when CP is enabled
        {
            if (streamState->ctxBasedScheduling)
            {
                if (m_bProtectedContext == false)    // Check if GEM context is already protected or not
                {
                    // Context is not protected, recreate it as protected
                    eStatus = RecreateContext(true, streamState);
                    if (eStatus == MOS_STATUS_SUCCESS)
                    {
                        //Register Protected Context
                        streamState->osCpInterface->RegisterAndCheckProtectedGemCtx(true, (void*)this, nullptr);
                    }
                }
                //If m_bProtectedContext == true then check if is stale context or not.
                //If it is stale protected context then recreate another one
                else
                {
                    bool bIsContextStale = false;
                    //Check protected context
                    streamState->osCpInterface->RegisterAndCheckProtectedGemCtx(false, (void*)this, &bIsContextStale);

                    //Recreate protected context
                    if (bIsContextStale)
                    {
                        eStatus = RecreateContext(true, streamState);
                        if (eStatus == MOS_STATUS_SUCCESS)
                        {
                            //Register Protected Context
                            streamState->osCpInterface->RegisterAndCheckProtectedGemCtx(true, (void*)this, nullptr);
                        }
                    }
                }
            }
            else
            {
                if (osParameters->m_protectedGEMContext == false)
                {
                    // for non context based scheduling protected context is always created as protected during Initialization if needed
                    // If it is not created during Initialization then do nothing and add a comment for Debug purposes
                    MOS_OS_CRITICALMESSAGE("Using Clear GEM context when protected Context is needed");
                    eStatus = MOS_STATUS_SUCCESS;
                }
            }
        }

    // clean up protected gem context and recreate clear gem context if CP is disabled
    if (streamState->osCpInterface &&
        !streamState->osCpInterface->IsCpEnabled() &&
        streamState->ctxBasedScheduling &&
        m_bProtectedContext == true)    // Check if GEM context is protected or not
        {
            // Context is protected, recreate it as clear
            eStatus = RecreateContext(false, streamState);
        }

    return eStatus;
}

MOS_STATUS GpuContextSpecificNext::Init3DCtx(PMOS_CONTEXT osParameters,
                PMOS_GPUCTX_CREATOPTIONS createOption,
                unsigned int *nengine,
                void *engine_map)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             osParameters->intel_context,
                                             I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE,
                                             m_bProtectedContext,
                                             engine_map,
                                             1,
                                             *nengine,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.\n");
        return MOS_STATUS_UNKNOWN;
    }
    m_i915Context[0]->pOsContext = osParameters;

    __u16 engine_class = I915_ENGINE_CLASS_RENDER;
    __u64 caps = 0;

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.\n");
        return MOS_STATUS_UNKNOWN;
    }

    if (mos_set_context_param_load_balance(m_i915Context[0], (struct i915_engine_class_instance *)engine_map, *nengine))
    {
        MOS_OS_ASSERTMESSAGE("Failed to set balancer extension.\n");
        return MOS_STATUS_UNKNOWN;
    }

    if (createOption->SSEUValue != 0)
    {
        struct drm_i915_gem_context_param_sseu sseu;
        MosUtilities::MosZeroMemory(&sseu, sizeof(sseu));
        sseu.flags = I915_CONTEXT_SSEU_FLAG_ENGINE_INDEX;
        sseu.engine.engine_instance = m_i915ExecFlag;

        if (mos_get_context_param_sseu(m_i915Context[0], &sseu))
        {
            MOS_OS_ASSERTMESSAGE("Failed to get sseu configuration.");
            return MOS_STATUS_UNKNOWN;
        }

        if (mos_hweight8(m_i915Context[0], sseu.subslice_mask) > createOption->packed.SubSliceCount)
        {
            sseu.subslice_mask = mos_switch_off_n_bits(m_i915Context[0], sseu.subslice_mask,
                    mos_hweight8(m_i915Context[0], sseu.subslice_mask)-createOption->packed.SubSliceCount);
        }

        if (mos_set_context_param_sseu(m_i915Context[0], sseu))
        {
            MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
            return MOS_STATUS_UNKNOWN;
        }
    }

    return eStatus;
}

MOS_STATUS GpuContextSpecificNext::InitComputeCtx(PMOS_CONTEXT osParameters,
                unsigned int *nengine,
                void *engine_map,
                MOS_GPU_NODE gpuNode,
                bool *isEngineSelectEnable)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             osParameters->intel_context,
                                             I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE,
                                             m_bProtectedContext,
                                             engine_map,
                                             1,
                                             *nengine,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.\n");
        return MOS_STATUS_UNKNOWN;
    }
    m_i915Context[0]->pOsContext = osParameters;

    __u16 engine_class = 4; //To change later when linux define the name
    __u64 caps = 0;

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.\n");
        return MOS_STATUS_UNKNOWN;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    *isEngineSelectEnable = SelectEngineInstanceByUser(engine_map, nengine, m_engineInstanceSelect, gpuNode);
#endif
    if (mos_set_context_param_load_balance(m_i915Context[0], (struct i915_engine_class_instance *)engine_map, *nengine))
    {
        MOS_OS_ASSERTMESSAGE("Failed to set balancer extension.\n");
        return MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

MOS_STATUS GpuContextSpecificNext::InitVdVeCtx(PMOS_CONTEXT osParameters,
                MOS_STREAM_HANDLE streamState,
                PMOS_GPUCTX_CREATOPTIONS createOption,
                unsigned int *nengine,
                void *engine_map,
                MOS_GPU_NODE gpuNode,
                bool *isEngineSelectEnable)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             osParameters->intel_context,
                                             I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE,
                                             m_bProtectedContext,
                                             engine_map,
                                             1,
                                             *nengine,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.\n");
        return MOS_STATUS_UNKNOWN;
    }
    struct i915_engine_class_instance *_engine_map = (struct i915_engine_class_instance *)engine_map;
    m_i915Context[0]->pOsContext = osParameters;

    __u16 engine_class = (gpuNode == MOS_GPU_NODE_VE)? I915_ENGINE_CLASS_VIDEO_ENHANCE : I915_ENGINE_CLASS_VIDEO;
    __u64 caps = 0;

    SetEngineQueryFlags(createOption, caps);

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, (void *)_engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.\n");
        return MOS_STATUS_UNKNOWN;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    *isEngineSelectEnable = SelectEngineInstanceByUser((void *)_engine_map, nengine, m_engineInstanceSelect, gpuNode);
#endif
    if (mos_set_context_param_load_balance(m_i915Context[0], _engine_map, *nengine))
    {
        MOS_OS_ASSERTMESSAGE("Failed to set balancer extension.\n");
        return MOS_STATUS_UNKNOWN;
    }

    if (*nengine >= 2 && *nengine <= MAX_ENGINE_INSTANCE_NUM)
    {
        int i;
        //master queue
        m_i915Context[1] = mos_context_create_shared(osParameters->bufmgr,
                                                            osParameters->intel_context,
                                                            I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE,
                                                            m_bProtectedContext,
                                                            (void *)_engine_map,
                                                            1,
                                                            1,
                                                            0);
        if (m_i915Context[1] == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to create master context.\n");
            return MOS_STATUS_UNKNOWN;
        }
        m_i915Context[1]->pOsContext = osParameters;

        if (mos_set_context_param_load_balance(m_i915Context[1], _engine_map, 1))
        {
            MOS_OS_ASSERTMESSAGE("Failed to set master context bond extension.\n");
            return MOS_STATUS_UNKNOWN;
        }

        //slave queue
        for (i=1; i < *nengine; i++)
        {
            m_i915Context[i+1] = mos_context_create_shared(osParameters->bufmgr,
                                                                osParameters->intel_context,
                                                                I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE,
                                                                m_bProtectedContext,
                                                                (void *)_engine_map,
                                                                1,
                                                                1,
                                                                0);
            if (m_i915Context[i+1] == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to create slave context.\n");
                return MOS_STATUS_UNKNOWN;
            }
            m_i915Context[i+1]->pOsContext = osParameters;

            if (mos_set_context_param_bond(m_i915Context[i+1], _engine_map[0], &_engine_map[i], 1) != S_SUCCESS)
            {
                int err = errno;
                if (err == ENODEV)
                {
                    mos_context_destroy(m_i915Context[1]);
                    mos_context_destroy(m_i915Context[i+1]);
                    m_i915Context[i+1] = nullptr;
                    break;
                }
                else
                {
                    MOS_OS_ASSERTMESSAGE("Failed to set slave context bond extension. errno=%d\n",err);
                    return MOS_STATUS_UNKNOWN;
                }
            }
        }
        if (i == *nengine)
        {
            streamState->bParallelSubmission = false;
        }
        else
        {
            streamState->bParallelSubmission = true;
            //create context with different width
            for(i = 1; i < *nengine; i++)
            {
                unsigned int ctxWidth = i + 1;
                m_i915Context[i] = mos_context_create_shared(osParameters->bufmgr,
                                                             osParameters->intel_context,
                                                             0, // I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE not allowed for parallel submission
                                                             m_bProtectedContext,
                                                             (void *)_engine_map,
                                                             ctxWidth,
                                                             1,
                                                             0);
                if (mos_set_context_param_parallel(m_i915Context[i], _engine_map, ctxWidth) != S_SUCCESS)
                {
                    MOS_OS_ASSERTMESSAGE("Failed to set parallel extension since discontinuous logical engine.\n");
                    mos_context_destroy(m_i915Context[i]);
                    m_i915Context[i] = nullptr;
                    break;
                }
            }
        }
    }

    return eStatus;
}

MOS_STATUS GpuContextSpecificNext::InitBltCtx(PMOS_CONTEXT osParameters,
                unsigned int *nengine,
                void *engine_map)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_i915Context[0] = mos_context_create_shared(osParameters->bufmgr,
                                             osParameters->intel_context,
                                             I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE,
                                             m_bProtectedContext,
                                             engine_map,
                                             1,
                                             *nengine,
                                             0);
    if (m_i915Context[0] == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create context.\n");
        return MOS_STATUS_UNKNOWN;
    }
    m_i915Context[0]->pOsContext = osParameters;

    __u16 engine_class = I915_ENGINE_CLASS_COPY;
    __u64 caps = 0;

    if (mos_query_engines(osParameters->bufmgr, engine_class, caps, nengine, engine_map))
    {
        MOS_OS_ASSERTMESSAGE("Failed to query engines.\n");
        return MOS_STATUS_UNKNOWN;
    }

    if (mos_set_context_param_load_balance(m_i915Context[0], (struct i915_engine_class_instance *)engine_map, *nengine))
    {
        MOS_OS_ASSERTMESSAGE("Failed to set balancer extension.\n");
        return MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}


MOS_STATUS GpuContextSpecificNext::Init(OsContextNext *osContext,
                    MOS_STREAM_HANDLE streamState,
                    PMOS_GPUCTX_CREATOPTIONS createOption)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osContext);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(createOption);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osParameters);

    m_osParameters       = osParameters;

    MOS_GPU_NODE gpuNode = MOS_GPU_NODE_3D;
    gpuNode = static_cast<MOS_GPU_NODE>(createOption->gpuNode);

    if (m_cmdBufPoolMutex == nullptr)
    {
        m_cmdBufPoolMutex = MosUtilities::MosCreateMutex();
    }

    MOS_OS_CHK_NULL_RETURN(m_cmdBufPoolMutex);

    MosUtilities::MosLockMutex(m_cmdBufPoolMutex);

    m_cmdBufPool.clear();

    MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);

    m_ocaLogSectionSupported = osContext->m_ocaLogSectionSupported;
    if (m_ocaLogSectionSupported)
    {
        // increase size for oca log section
        m_commandBufferSize = MosOcaInterfaceSpecific::IncreaseSize(COMMAND_BUFFER_SIZE);
    }
    else
    {
        m_commandBufferSize = COMMAND_BUFFER_SIZE;
    }

    m_nextFetchIndex = 0;

    m_cmdBufFlushed = true;

    m_osContext = osContext;

    MOS_OS_CHK_STATUS_RETURN(AllocateGPUStatusBuf());

    m_commandBuffer = (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));

    MOS_OS_CHK_NULL_RETURN(m_commandBuffer);

    m_IndirectHeapSize = 0;

    // each thread has its own GPU context, so do not need any lock as guarder here
    m_allocationList = (ALLOCATION_LIST *)MOS_AllocAndZeroMemory(sizeof(ALLOCATION_LIST) * ALLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_allocationList);
    m_maxNumAllocations = ALLOCATIONLIST_SIZE;

    m_patchLocationList = (PATCHLOCATIONLIST *)MOS_AllocAndZeroMemory(sizeof(PATCHLOCATIONLIST) * PATCHLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);
    m_maxPatchLocationsize = PATCHLOCATIONLIST_SIZE;

    m_attachedResources = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * ALLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_attachedResources);

    m_writeModeList = (bool *)MOS_AllocAndZeroMemory(sizeof(bool) * ALLOCATIONLIST_SIZE);
    MOS_OS_CHK_NULL_RETURN(m_writeModeList);

    m_GPUStatusTag = 1;

    StoreCreateOptions(createOption);

    for (int i=0; i<MAX_ENGINE_INSTANCE_NUM+1; i++)
    {
        m_i915Context[i] = nullptr;
    }

    if (streamState->ctxBasedScheduling)
    {
        bool         isEngineSelectEnable = false;
        unsigned int nengine              = 0;
        size_t       engine_class_size    = 0;
        void         *engine_map          = nullptr;

        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_START,
                          &gpuNode, sizeof(gpuNode), nullptr, 0);

        m_i915ExecFlag = I915_EXEC_DEFAULT;

        if (mos_query_engines_count(osParameters->bufmgr, &nengine))
        {
            MOS_OS_ASSERTMESSAGE("Failed to query engines count.\n");
            return MOS_STATUS_UNKNOWN;
        }
        engine_class_size = mos_get_engine_class_size(osParameters->bufmgr);
        if (!engine_class_size)
        {
            MOS_OS_ASSERTMESSAGE("Failed to get engine class instance size.\n");
            return MOS_STATUS_UNKNOWN;
        }
        engine_map = MOS_AllocAndZeroMemory(nengine * engine_class_size);
        MOS_OS_CHK_NULL_RETURN(engine_map);

        if (gpuNode == MOS_GPU_NODE_3D)
        {
            eStatus = Init3DCtx(osParameters, createOption, &nengine, engine_map);
        }
        else if (gpuNode == MOS_GPU_NODE_COMPUTE)
        {
            eStatus = InitComputeCtx(osParameters, &nengine, engine_map, gpuNode, &isEngineSelectEnable);
        }
        else if (gpuNode == MOS_GPU_NODE_VIDEO || gpuNode == MOS_GPU_NODE_VIDEO2
                || gpuNode == MOS_GPU_NODE_VE)
        {
            eStatus = InitVdVeCtx(osParameters, streamState, createOption, &nengine, engine_map, gpuNode, &isEngineSelectEnable);
        }
        else if (gpuNode == MOS_GPU_NODE_BLT)
        {
            eStatus = InitBltCtx(osParameters, &nengine, engine_map);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Unknown engine class.\n");
            MOS_SafeFreeMemory(engine_map);
            return MOS_STATUS_UNKNOWN;
        }

        if (eStatus == MOS_STATUS_SUCCESS)
        {
            MOS_OS_CHK_STATUS_RETURN(ReportEngineInfo(engine_map, nengine, isEngineSelectEnable));
        }
        MOS_SafeFreeMemory(engine_map);
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_END,
                          m_i915Context, sizeof(void *),
                          &nengine, sizeof(nengine));
    }

    return eStatus;
}

void GpuContextSpecificNext::Clear()
{
    MOS_OS_FUNCTION_ENTER;

    MOS_TraceEventExt(EVENT_GPU_CONTEXT_DESTROY, EVENT_TYPE_START,
                      m_i915Context, sizeof(void *), nullptr, 0);
    // hanlde the status buf bundled w/ the specified gpucontext
    if (m_statusBufferResource && m_statusBufferResource->pGfxResourceNext)
    {
        if (m_statusBufferResource->pGfxResourceNext->Unlock(m_osContext) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("failed to unlock the status buf bundled w/ the specified gpucontext");
        }
        m_statusBufferResource->pGfxResourceNext->Free(m_osContext, 0);
        MOS_Delete(m_statusBufferResource->pGfxResourceNext);
    }
    MOS_FreeMemAndSetNull(m_statusBufferResource);

    if(m_cmdBufPoolMutex)
    {
        MosUtilities::MosLockMutex(m_cmdBufPoolMutex);

        if (m_cmdBufMgr)
        {
            for (auto& curCommandBuffer : m_cmdBufPool)
            {
                auto curCommandBufferSpecific = static_cast<CommandBufferSpecificNext *>(curCommandBuffer);
                if (curCommandBufferSpecific == nullptr)
                    continue;
                curCommandBufferSpecific->waitReady(); // wait ready and return to comamnd buffer manager.
                m_cmdBufMgr->ReleaseCmdBuf(curCommandBuffer);
            }
        }

        m_cmdBufPool.clear();

        MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
        MosUtilities::MosDestroyMutex(m_cmdBufPoolMutex);
        m_cmdBufPoolMutex = nullptr;
    }

    MOS_SafeFreeMemory(m_commandBuffer);
    m_commandBuffer = nullptr;
    MOS_SafeFreeMemory(m_allocationList);
    m_allocationList = nullptr;
    MOS_SafeFreeMemory(m_patchLocationList);
    m_patchLocationList = nullptr;
    MOS_SafeFreeMemory(m_attachedResources);
    m_attachedResources = nullptr;
    MOS_SafeFreeMemory(m_writeModeList);
    m_writeModeList = nullptr;

    for (int i=0; i<MAX_ENGINE_INSTANCE_NUM; i++)
    {
        if (m_i915Context[i])
        {
            mos_context_destroy(m_i915Context[i]);
            m_i915Context[i] = nullptr;
        }
    }
    MOS_TraceEventExt(EVENT_GPU_CONTEXT_DESTROY, EVENT_TYPE_END,
                      nullptr, 0, nullptr, 0);
}

MOS_STATUS GpuContextSpecificNext::RegisterResource(
    PMOS_RESOURCE osResource,
    bool          writeFlag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osResource);

    MOS_OS_CHK_NULL_RETURN(m_attachedResources);

    PMOS_RESOURCE registeredResources = m_attachedResources;
    uint32_t      allocationIndex     = 0;

    for ( allocationIndex = 0; allocationIndex < m_resCount; allocationIndex++, registeredResources++)
    {
        if (osResource->bo == registeredResources->bo)
        {
            break;
        }
    }

    // Allocation list to be updated
    if (allocationIndex < m_maxNumAllocations)
    {
        // New buffer
        if (allocationIndex == m_resCount)
        {
            m_resCount++;
        }

        // Set allocation
        if (m_gpuContext >= MOS_GPU_CONTEXT_MAX)
        {
            MOS_OS_ASSERTMESSAGE("Gpu context exceeds max.");
            return MOS_STATUS_UNKNOWN; 
        }

        osResource->iAllocationIndex[m_gpuContext] = (allocationIndex);
        m_attachedResources[allocationIndex]           = *osResource;
        m_writeModeList[allocationIndex] |= writeFlag;
        m_allocationList[allocationIndex].hAllocation = &m_attachedResources[allocationIndex];
        m_allocationList[allocationIndex].WriteOperation |= writeFlag;
        m_numAllocations = m_resCount;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Reached max # registrations.");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::SetPatchEntry(
    MOS_STREAM_HANDLE streamState,
    PMOS_PATCH_ENTRY_PARAMS params)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(params);

    m_patchLocationList[m_currentNumPatchLocations].AllocationIndex  = params->uiAllocationIndex;
    m_patchLocationList[m_currentNumPatchLocations].AllocationOffset = params->uiResourceOffset;
    m_patchLocationList[m_currentNumPatchLocations].PatchOffset      = params->uiPatchOffset;
    m_patchLocationList[m_currentNumPatchLocations].uiWriteOperation = params->bWrite ? true: false;
    m_patchLocationList[m_currentNumPatchLocations].cmdBo            =
                params->cmdBuffer != nullptr ? params->cmdBuffer->OsResource.bo : nullptr;

    if (streamState->osCpInterface &&
        streamState->osCpInterface->IsHMEnabled())
    {
        if (MOS_STATUS_SUCCESS != streamState->osCpInterface->RegisterPatchForHM(
            (uint32_t *)(params->cmdBufBase + params->uiPatchOffset),
            params->bWrite,
            params->HwCommandType,
            params->forceDwordOffset,
            params->presResource,
            &m_patchLocationList[m_currentNumPatchLocations]))
        {
            MOS_OS_ASSERTMESSAGE("Failed to RegisterPatchForHM.");
        }
    }

    m_currentNumPatchLocations++;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::GetCommandBuffer(
    PMOS_COMMAND_BUFFER comamndBuffer,
    uint32_t            flags)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(comamndBuffer);
    MOS_OS_CHK_NULL_RETURN(m_cmdBufMgr);
    MOS_OS_CHK_NULL_RETURN(m_commandBuffer);

    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
    CommandBufferNext* cmdBuf = nullptr;

    uint32_t secondaryIdx = flags;
    bool isPrimaryCmdBuffer = (secondaryIdx == 0);
    bool hasSecondaryCmdBuffer = (!isPrimaryCmdBuffer &&
                               (m_secondaryCmdBufs.count(secondaryIdx) != 0));

    bool needToAlloc = ((isPrimaryCmdBuffer && m_cmdBufFlushed) ||
                        (!isPrimaryCmdBuffer && !hasSecondaryCmdBuffer));

    if (needToAlloc)
    {
        MosUtilities::MosLockMutex(m_cmdBufPoolMutex);
        if (m_cmdBufPool.size() < MAX_CMD_BUF_NUM)
        {
            cmdBuf = m_cmdBufMgr->PickupOneCmdBuf(m_commandBufferSize);
            if (cmdBuf == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
                MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
                return MOS_STATUS_NULL_POINTER;
            }
            if ((eStatus = cmdBuf->BindToGpuContext(this)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Invalid status of BindToGpuContext.");
                MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
                return eStatus;
            }
            m_cmdBufPool.push_back(cmdBuf);
        }
        else if (m_cmdBufPool.size() == MAX_CMD_BUF_NUM && m_nextFetchIndex < m_cmdBufPool.size())
        {
            auto cmdBufOld = m_cmdBufPool[m_nextFetchIndex];
            auto cmdBufSpecificOld = static_cast<CommandBufferSpecificNext *>(cmdBufOld);
            if (cmdBufSpecificOld == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
                MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
                return MOS_STATUS_NULL_POINTER;
            }
            cmdBufSpecificOld->waitReady();
            cmdBufSpecificOld->UnBindToGpuContext();
            m_cmdBufMgr->ReleaseCmdBuf(cmdBufOld);  // here just return old command buffer to available pool

            //pick up new comamnd buffer
            cmdBuf = m_cmdBufMgr->PickupOneCmdBuf(m_commandBufferSize);
            if (cmdBuf == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
                MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
                return MOS_STATUS_NULL_POINTER;
            }
            if ((eStatus = cmdBuf->BindToGpuContext(this)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Invalid status of BindToGpuContext.");
                MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
                return eStatus;
            }
            m_cmdBufPool[m_nextFetchIndex] = cmdBuf;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Command buffer bool size exceed max.");
            MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);
            return MOS_STATUS_UNKNOWN;
        }
        MosUtilities::MosUnlockMutex(m_cmdBufPoolMutex);

        // util now, we got new command buffer from CmdBufMgr, next step to fill in the input command buffer
        MOS_OS_CHK_STATUS_RETURN(cmdBuf->GetResource()->ConvertToMosResource(&comamndBuffer->OsResource));
        comamndBuffer->pCmdBase   = (uint32_t *)cmdBuf->GetLockAddr();
        comamndBuffer->pCmdPtr    = (uint32_t *)cmdBuf->GetLockAddr();
        comamndBuffer->iOffset    = 0;
        comamndBuffer->iRemaining = cmdBuf->GetCmdBufSize();
        comamndBuffer->iCmdIndex  = m_nextFetchIndex;
        comamndBuffer->iVdboxNodeIndex = MOS_VDBOX_NODE_INVALID;
        comamndBuffer->iVeboxNodeIndex = MOS_VEBOX_NODE_INVALID;
        comamndBuffer->Attributes.pAttriVe = nullptr;
        comamndBuffer->is1stLvlBB = true;

        // zero comamnd buffer
        MosUtilities::MosZeroMemory(comamndBuffer->pCmdBase, comamndBuffer->iRemaining);
        comamndBuffer->iSubmissionType = SUBMISSION_TYPE_SINGLE_PIPE;
        MosUtilities::MosZeroMemory(&comamndBuffer->Attributes,sizeof(comamndBuffer->Attributes));

        if (isPrimaryCmdBuffer)
        {
            // update command buffer relared filed in GPU context
            m_cmdBufFlushed = false;

            // keep a copy in GPU context
            MosUtilities::MosSecureMemcpy(m_commandBuffer, sizeof(MOS_COMMAND_BUFFER), comamndBuffer, sizeof(MOS_COMMAND_BUFFER));
        }
        else
        {
            PMOS_COMMAND_BUFFER tempCmdBuf = (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));
            MOS_OS_CHK_NULL_RETURN(tempCmdBuf);
            m_secondaryCmdBufs[secondaryIdx] = tempCmdBuf;
            MosUtilities::MosSecureMemcpy(tempCmdBuf, sizeof(MOS_COMMAND_BUFFER), comamndBuffer, sizeof(MOS_COMMAND_BUFFER));
        }

        if (m_ocaLogSectionSupported)
        {
            MOS_LINUX_BO *boTemp = ((GraphicsResourceSpecificNext *)cmdBuf->GetResource())->GetBufferObject();
            MosOcaInterfaceSpecific::InitOcaLogSection(boTemp);
        }

        // Command buffers are treated as cyclical buffers, the CB after the just submitted one
        // has the minimal fence value that we should wait
        m_nextFetchIndex++;
        if (m_nextFetchIndex >= MAX_CMD_BUF_NUM)
        {
            m_nextFetchIndex = 0;
        }
    }
    else
    {
        // current command buffer still active, directly copy to comamndBuffer
        if (isPrimaryCmdBuffer)
        {
            MosUtilities::MosSecureMemcpy(comamndBuffer, sizeof(MOS_COMMAND_BUFFER), m_commandBuffer, sizeof(MOS_COMMAND_BUFFER));
        }
        else
        {
            MosUtilities::MosSecureMemcpy(comamndBuffer, sizeof(MOS_COMMAND_BUFFER), m_secondaryCmdBufs[secondaryIdx], sizeof(MOS_COMMAND_BUFFER));
        }
    }

    if (isPrimaryCmdBuffer)
    {
        MOS_OS_CHK_STATUS_RETURN(RegisterResource(&m_commandBuffer->OsResource, false));
    }
    else
    {
        MOS_OS_CHK_STATUS_RETURN(RegisterResource(&m_secondaryCmdBufs[secondaryIdx]->OsResource, false));
    }

    return MOS_STATUS_SUCCESS;
}

void GpuContextSpecificNext::ReturnCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t            flags)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(cmdBuffer);
    MOS_OS_ASSERT(m_commandBuffer);

    bool isPrimaryCmdBuf = (flags == 0);

    if (isPrimaryCmdBuf)
    {
        m_commandBuffer->iOffset    = cmdBuffer->iOffset;
        m_commandBuffer->iRemaining = cmdBuffer->iRemaining;
        m_commandBuffer->pCmdPtr    = cmdBuffer->pCmdPtr;
        m_commandBuffer->iVdboxNodeIndex = cmdBuffer->iVdboxNodeIndex;
        m_commandBuffer->iVeboxNodeIndex = cmdBuffer->iVeboxNodeIndex;
    }
    else
    {
        uint32_t secondaryIdx = flags;
        MOS_OS_ASSERT(m_secondaryCmdBufs.count(secondaryIdx));

        MosUtilities::MosSecureMemcpy(m_secondaryCmdBufs[secondaryIdx], sizeof(MOS_COMMAND_BUFFER), cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    }
}

MOS_STATUS GpuContextSpecificNext::ResetCommandBuffer()
{
    m_cmdBufFlushed = true;
    auto it = m_secondaryCmdBufs.begin();
    while(it != m_secondaryCmdBufs.end())
    {
        MOS_FreeMemory(it->second);
        it++;
    }
    m_secondaryCmdBufs.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::SetIndirectStateSize(const uint32_t size)
{
    if (m_ocaLogSectionSupported)
    {
        if(MosOcaInterfaceSpecific::IncreaseSize(size) < m_commandBufferSize)
        {
            m_IndirectHeapSize = size;
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Indirect State Size if out of boundry!");
            return MOS_STATUS_UNKNOWN;
        }
    }
    else
    {
        if(size < m_commandBufferSize)
        {
            m_IndirectHeapSize = size;
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Indirect State Size if out of boundry!");
            return MOS_STATUS_UNKNOWN;
        }
    }
}

MOS_STATUS GpuContextSpecificNext::GetIndirectState(
    uint32_t &offset,
    uint32_t &size)
{
    MOS_OS_FUNCTION_ENTER;

    if (m_ocaLogSectionSupported)
    {
        offset = m_commandBufferSize - m_IndirectHeapSize - OCA_LOG_SECTION_SIZE_MAX;
    }
    else
    {
        offset = m_commandBufferSize - m_IndirectHeapSize;
    }
    size = m_IndirectHeapSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::GetIndirectStatePointer(
    uint8_t **indirectState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(indirectState);

    if (m_ocaLogSectionSupported)
    {
        *indirectState = (uint8_t *)m_commandBuffer->pCmdBase + m_commandBufferSize - m_IndirectHeapSize - OCA_LOG_SECTION_SIZE_MAX;
    }
    else
    {
        *indirectState = (uint8_t *)m_commandBuffer->pCmdBase + m_commandBufferSize - m_IndirectHeapSize;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::ResizeCommandBufferAndPatchList(
    uint32_t requestedCommandBufferSize,
    uint32_t requestedPatchListSize,
    uint32_t flags)
{
    MOS_OS_FUNCTION_ENTER;

    // m_commandBufferSize is used for allocate command buffer and submit command buffer, in this moment, command buffer has not allocated yet.
    // Linux KMD requires command buffer size align to 8 bytes, or it will not execute the commands.
    if (m_ocaLogSectionSupported /*&& !m_ocaSizeIncreaseDone*/)
    {
        m_commandBufferSize = MOS_ALIGN_CEIL(MosOcaInterfaceSpecific::IncreaseSize(requestedCommandBufferSize), 8);
        // m_ocaSizeIncreaseDone = true;
    }
    else
    {
        m_commandBufferSize = MOS_ALIGN_CEIL(requestedCommandBufferSize, 8);
    }

    if (requestedPatchListSize > m_maxPatchLocationsize)
    {
        PPATCHLOCATIONLIST newPatchList = (PPATCHLOCATIONLIST)MOS_ReallocMemory(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * requestedPatchListSize);
        MOS_OS_CHK_NULL_RETURN(newPatchList);

        m_patchLocationList = newPatchList;

        // now zero the extended portion
        MosUtilities::MosZeroMemory((m_patchLocationList + m_maxPatchLocationsize), sizeof(PATCHLOCATIONLIST) * (requestedPatchListSize - m_maxPatchLocationsize));
        m_maxPatchLocationsize = requestedPatchListSize;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::ResizeCommandBuffer(uint32_t requestedSize)
{
    MOS_OS_FUNCTION_ENTER;

    m_commandBufferSize = requestedSize;

    return MOS_STATUS_SUCCESS;
}

MOS_VDBOX_NODE_IND GpuContextSpecificNext::GetVdboxNodeId(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_VDBOX_NODE_IND idx = MOS_VDBOX_NODE_INVALID;

    if (cmdBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("No cmd buffer provided in GetVdboxNodeId!");
        return idx;
    }

    // If we have assigned vdbox index for the given cmdbuf, return it immediately
    if (MOS_VDBOX_NODE_INVALID != cmdBuffer->iVdboxNodeIndex) {
        idx = cmdBuffer->iVdboxNodeIndex;
        return idx;
    }

    return idx;
}

uint32_t GpuContextSpecificNext::GetVcsExecFlag(
    PMOS_COMMAND_BUFFER cmdBuffer,
    MOS_GPU_NODE gpuNode)
{
    if (cmdBuffer == 0)
    {
        MOS_OS_ASSERTMESSAGE("Input invalid(null) parameter.");
        return I915_EXEC_DEFAULT;
    }

    uint32_t vcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;

    if (MOS_VDBOX_NODE_INVALID == cmdBuffer->iVdboxNodeIndex)
    {
       // That's those case when BB did not have any VDBOX# specific commands.
       // Thus, we need to select VDBOX# here. Alternatively we can rely on KMD
       // to make balancing for us, i.e. rely on Virtual Engine support.
       cmdBuffer->iVdboxNodeIndex = GetVdboxNodeId(cmdBuffer);
       if (MOS_VDBOX_NODE_INVALID == cmdBuffer->iVdboxNodeIndex)
       {
           cmdBuffer->iVdboxNodeIndex = (gpuNode == MOS_GPU_NODE_VIDEO)?
               MOS_VDBOX_NODE_1: MOS_VDBOX_NODE_2;
       }
     }

     if (MOS_VDBOX_NODE_1 == cmdBuffer->iVdboxNodeIndex)
     {
         vcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
     }
     else if (MOS_VDBOX_NODE_2 == cmdBuffer->iVdboxNodeIndex)
     {
         vcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
     }

     return vcsExecFlag;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_LINUX_BO* GpuContextSpecificNext::GetNopCommandBuffer(
    MOS_STREAM_HANDLE streamState)
{
    int j;
    uint32_t *buf = nullptr;
    MOS_LINUX_BO* bo = nullptr;

    j = 0;

    if(streamState == nullptr || streamState->perStreamParameters == nullptr)
    {
        return nullptr;
    }

    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    struct mos_drm_bo_alloc alloc;
    alloc.name = "NOP_CMD_BO";
    alloc.size = 4096;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_VIDEOMEMORY;
    bo = mos_bo_alloc(perStreamParameters->bufmgr, &alloc);
    if(bo == nullptr)
    {
        return nullptr;
    }

    mos_bo_map(bo, 1);
    buf = (uint32_t*)bo->virt;
    if(buf == nullptr)
    {
        mos_bo_unreference(bo);
        return nullptr;
    }

    buf[j++] = 0x05000000; // MI_BATCH_BUFFER_END

    mos_bo_unmap(bo);

    return bo;
}
#endif // _DEBUG || _RELEASE_INTERNAL


MOS_STATUS GpuContextSpecificNext::MapResourcesToAuxTable(mos_linux_bo *cmd_bo)
{
    MOS_OS_CHK_NULL_RETURN(cmd_bo);

    OsContextSpecificNext *osCtx = static_cast<OsContextSpecificNext*>(m_osContext);
    MOS_OS_CHK_NULL_RETURN(osCtx);

    AuxTableMgr *auxTableMgr = osCtx->GetAuxTableMgr();
    if (auxTableMgr && m_currentNumPatchLocations > 0)
    {
        // Map compress allocations to aux table if it is not mapped.
        for (uint32_t i = 0; i < m_numAllocations; i++)
        {
            auto res = (PMOS_RESOURCE)m_allocationList[i].hAllocation;
            MOS_OS_CHK_NULL_RETURN(res);
            MOS_OS_CHK_STATUS_RETURN(auxTableMgr->MapResource(res->pGmmResInfo, res->bo));
        }
        MOS_OS_CHK_STATUS_RETURN(auxTableMgr->EmitAuxTableBOList(cmd_bo));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GpuContextSpecificNext::SubmitCommandBuffer(
    MOS_STREAM_HANDLE   streamState,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                nullRendering)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_TraceEventExt(EVENT_MOS_BATCH_SUBMIT, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_OS_CHK_NULL_RETURN(streamState);
    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(perStreamParameters);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);
    MOS_OS_CHK_NULL_RETURN(m_patchLocationList);

    MOS_GPU_NODE gpuNode  = OSKMGetGpuNode(m_gpuContext);
    uint32_t     execFlag = gpuNode;
    MOS_STATUS   eStatus  = MOS_STATUS_SUCCESS;
    int32_t      ret      = 0;
    bool         scalaEnabled = false;
    auto         it           = m_secondaryCmdBufs.begin();

    // Command buffer object DRM pointer
    m_cmdBufFlushed = true;
    auto cmd_bo     = cmdBuffer->OsResource.bo;

    // Map Resource to Aux if needed
    MapResourcesToAuxTable(cmd_bo);
    for(auto it : m_secondaryCmdBufs)
    {
        MapResourcesToAuxTable(it.second->OsResource.bo);
    }

    if (m_secondaryCmdBufs.size() >= 2)
    {
        scalaEnabled = true;
        cmdBuffer->iSubmissionType = SUBMISSION_TYPE_MULTI_PIPE_MASTER;
    }

    std::vector<PMOS_RESOURCE> mappedResList;
    std::vector<MOS_LINUX_BO *> skipSyncBoList;

    // Now, the patching will be done, based on the patch list.
    for (uint32_t patchIndex = 0; patchIndex < m_currentNumPatchLocations; patchIndex++)
    {
        auto currentPatch = &m_patchLocationList[patchIndex];
        MOS_OS_CHK_NULL_RETURN(currentPatch);

        auto tempCmdBo = currentPatch->cmdBo == nullptr ? cmd_bo : currentPatch->cmdBo;

        // Following are for Nested BB buffer, if it's nested BB, we need to ensure it's locked.
        if (tempCmdBo != cmd_bo)
        {
            bool isSecondaryCmdBuf = false;
            it = m_secondaryCmdBufs.begin();
            while(it != m_secondaryCmdBufs.end())
            {
                if (it->second->OsResource.bo == tempCmdBo)
                {
                    isSecondaryCmdBuf = true;
                    break;
                }
                it++;
            }

            for(auto allocIdx = 0; allocIdx < m_numAllocations && (!isSecondaryCmdBuf); allocIdx++)
            {
                auto tempRes = (PMOS_RESOURCE)m_allocationList[allocIdx].hAllocation;
                if (tempCmdBo == tempRes->bo)
                {
                    GraphicsResourceNext::LockParams param;
                    param.m_writeRequest = true;
                    tempRes->pGfxResourceNext->Lock(m_osContext, param);
                    mappedResList.push_back(tempRes);
                    break;
                }
            }
        }

        // This is the resource for which patching will be done
        auto resource = (PMOS_RESOURCE)m_allocationList[currentPatch->AllocationIndex].hAllocation;
        MOS_OS_CHK_NULL_RETURN(resource);

        // For now, we'll assume the system memory's DRM bo pointer
        // is NULL.  If nullptr is detected, then the resource has been
        // placed inside the command buffer's indirect state area.
        // We'll simply set alloc_bo to the command buffer's bo pointer.
        MOS_OS_ASSERT(resource->bo);

        auto alloc_bo = (resource->bo) ? resource->bo : tempCmdBo;

        MOS_OS_CHK_STATUS_RETURN(streamState->osCpInterface->PermeatePatchForHM(
            tempCmdBo->virt,
            currentPatch,
            resource));

        uint64_t boOffset = alloc_bo->offset64;
        if (!mos_bo_is_softpin(alloc_bo))
        {
            if (alloc_bo != tempCmdBo)
            {
                auto item_ctx = perStreamParameters->contextOffsetList.begin();
                for (; item_ctx != perStreamParameters->contextOffsetList.end(); item_ctx++)
                {
                    if (item_ctx->intel_context == perStreamParameters->intel_context && item_ctx->target_bo == alloc_bo)
                    {
                        boOffset = item_ctx->offset64;
                        break;
                    }
                }
            }
        }

        MOS_OS_CHK_NULL_RETURN(tempCmdBo->virt);
        if (perStreamParameters->bUse64BitRelocs)
        {
            *((uint64_t *)((uint8_t *)tempCmdBo->virt + currentPatch->PatchOffset)) =
                    boOffset + currentPatch->AllocationOffset;
        }
        else
        {
            *((uint32_t *)((uint8_t *)tempCmdBo->virt + currentPatch->PatchOffset)) =
                    boOffset + currentPatch->AllocationOffset;
        }

        if (scalaEnabled)
        {
            it = m_secondaryCmdBufs.begin();
            while(it != m_secondaryCmdBufs.end())
            {
                if (it->second->OsResource.bo == tempCmdBo &&
                    it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE &&
                    !mos_bo_is_exec_object_async(alloc_bo))
                {
                    skipSyncBoList.push_back(alloc_bo);
                    break;
                }
                it++;
            }
        }
        else if (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE &&
                 !mos_bo_is_exec_object_async(alloc_bo))
        {
            skipSyncBoList.push_back(alloc_bo);
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        {
            uint32_t evtData[] = {alloc_bo->handle, currentPatch->uiWriteOperation, currentPatch->AllocationOffset};
            MOS_TraceEventExt(EVENT_MOS_BATCH_SUBMIT, EVENT_TYPE_INFO,
                              evtData, sizeof(evtData),
                              &boOffset, sizeof(boOffset));
        }
#endif

        if(mos_bo_is_softpin(alloc_bo))
        {
            if (alloc_bo != tempCmdBo)
            {
                ret = mos_bo_add_softpin_target(tempCmdBo, alloc_bo, currentPatch->uiWriteOperation);
            }
        }
        else
        {
            // This call will patch the command buffer with the offsets of the indirect state region of the command buffer
            ret = mos_bo_emit_reloc(
                tempCmdBo,                                                         // Command buffer
                currentPatch->PatchOffset,                                         // Offset in the command buffer
                alloc_bo,                                                          // Allocation object for which the patch will be made.
                currentPatch->AllocationOffset,                                    // Offset to the indirect state
                I915_GEM_DOMAIN_RENDER,                                            // Read domain
                (currentPatch->uiWriteOperation) ? I915_GEM_DOMAIN_RENDER : 0x0,   // Write domain
                boOffset);
        }

        if (ret != 0)
        {
            MOS_OS_ASSERTMESSAGE("Error patching alloc_bo = 0x%x, cmd_bo = 0x%x.",
                (uintptr_t)alloc_bo,
                (uintptr_t)tempCmdBo);
            return MOS_STATUS_UNKNOWN;
        }
    }

    for(auto res: mappedResList)
    {
        res->pGfxResourceNext->Unlock(m_osContext);
    }
    mappedResList.clear();

    if (scalaEnabled)
    {
         it = m_secondaryCmdBufs.begin();
         while(it != m_secondaryCmdBufs.end())
         {
             //Add Batch buffer End Command
             uint32_t batchBufferEndCmd = MI_BATCHBUFFER_END;
             if (MOS_FAILED(Mos_AddCommand(
                     it->second,
                     &batchBufferEndCmd,
                     sizeof(uint32_t))))
             {
                 MOS_OS_ASSERTMESSAGE("Inserting BB_END failed!");
                 return MOS_STATUS_UNKNOWN;
             }
             it++;
         }
    }
    else
    {
        //Add Batch buffer End Command
        uint32_t batchBufferEndCmd = MI_BATCHBUFFER_END;
        if (MOS_FAILED(Mos_AddCommand(
                cmdBuffer,
                &batchBufferEndCmd,
                sizeof(uint32_t))))
        {
            MOS_OS_ASSERTMESSAGE("Inserting BB_END failed!");
            return MOS_STATUS_UNKNOWN;
        }
    }
    // dump before cmd buffer unmap
    MOS_TraceDumpExt("CmdBuffer", m_gpuContext, cmdBuffer->pCmdBase, cmdBuffer->iOffset);

    // Now, we can unmap the video command buffer, since we don't need CPU access anymore.
    MOS_OS_CHK_NULL_RETURN(cmdBuffer->OsResource.pGfxResourceNext);

    cmdBuffer->OsResource.pGfxResourceNext->Unlock(m_osContext);

    it = m_secondaryCmdBufs.begin();
    while(it != m_secondaryCmdBufs.end())
    {
        MOS_OS_CHK_NULL_RETURN(it->second->OsResource.pGfxResourceNext);
        it->second->OsResource.pGfxResourceNext->Unlock(m_osContext);

        it++;
    }

    int32_t perfData;
    if (perStreamParameters->pPerfData != nullptr)
    {
        perfData = *(int32_t *)(perStreamParameters->pPerfData);
    }
    else
    {
        perfData = 0;
    }

    drm_clip_rect_t *cliprects     = nullptr;
    int32_t          num_cliprects = 0;
    int32_t          DR4           = perStreamParameters->uEnablePerfTag ? perfData : 0;

    //Since CB2 command is not supported, remove it and set cliprects to nullprt as default.
    if ((gpuNode == MOS_GPU_NODE_VIDEO || gpuNode == MOS_GPU_NODE_VIDEO2) &&
        (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_SINGLE_PIPE_MASK))
    {
        if (perStreamParameters->bKMDHasVCS2)
        {
            if (perStreamParameters->bPerCmdBufferBalancing)
            {
                execFlag = GetVcsExecFlag(cmdBuffer, gpuNode);
            }
            else if (gpuNode == MOS_GPU_NODE_VIDEO)
            {
                execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
            }
            else if (gpuNode == MOS_GPU_NODE_VIDEO2)
            {
                execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
            }
        }
        else
        {
            execFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
 
    MOS_LINUX_BO *nop_cmd_bo = nullptr; 

    if (nullRendering == true)
    {
        nop_cmd_bo = GetNopCommandBuffer(streamState);

        if (nop_cmd_bo)
        {
            ret = mos_bo_mrb_exec(nop_cmd_bo,
                4096,
                nullptr,
                0,
                0,
                execFlag);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Mos_GetNopCommandBuffer_Linux failed!");
        }
    }

#endif  //(_DEBUG || _RELEASE_INTERNAL)

    if (gpuNode != I915_EXEC_RENDER &&
        streamState->osCpInterface->IsTearDownHappen())
    {
        // skip PAK command when CP tear down happen to avoid of GPU hang
        // conditonal batch buffer start PoC is in progress
    }
    else if (nullRendering == false)
    {
        UnlockPendingOcaBuffers(cmdBuffer, perStreamParameters);
        if (streamState->ctxBasedScheduling && m_i915Context[0] != nullptr)
        {
            if (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASK)
            {
                if (scalaEnabled && !streamState->bParallelSubmission)
                {
                    uint32_t secondaryIndex = 0;
                    it = m_secondaryCmdBufs.begin();
                    while(it != m_secondaryCmdBufs.end())
                    {
                        if (it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE)
                        {
                            if(execFlag == MOS_GPU_NODE_VE)
                            {
                                // decode excluded since init in other place
                                it->second->iSubmissionType |= (secondaryIndex << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT);
                                secondaryIndex++;
                            }
                        }
                        ret = SubmitPipeCommands(it->second,
                                                 it->second->OsResource.bo,
                                                 perStreamParameters,
                                                 skipSyncBoList,
                                                 execFlag,
                                                 DR4);
                        it++;
                    }
                }
                else if(scalaEnabled && streamState->bParallelSubmission)
                {
                    ret = ParallelSubmitCommands(m_secondaryCmdBufs,
                                         perStreamParameters,
                                         execFlag,
                                         DR4);
                }
                else
                {
                    ret = SubmitPipeCommands(cmdBuffer,
                                             cmd_bo,
                                             perStreamParameters,
                                             skipSyncBoList,
                                             execFlag,
                                             DR4);
                }
            }
            else
            {
                ret = mos_bo_context_exec2(cmd_bo,
                    m_commandBufferSize,
                    m_i915Context[0],
                    cliprects,
                    num_cliprects,
                    DR4,
                    m_i915ExecFlag,
                    nullptr);
            }
        }
        else
        {
            ret = mos_bo_context_exec2(cmd_bo,
                m_commandBufferSize,
                perStreamParameters->intel_context,
                cliprects,
                num_cliprects,
                DR4,
                execFlag,
                nullptr);
        }
        if (ret != 0)
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
    }

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Command buffer submission failed!");
    }

    MosUtilDevUltSpecific::MOS_DEVULT_FuncCall(pfnUltGetCmdBuf, cmdBuffer);

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
pthread_mutex_lock(&command_dump_mutex);
if (streamState->dumpCommandBuffer)
    {
        if (scalaEnabled)
        {
            it = m_secondaryCmdBufs.begin();
            while(it != m_secondaryCmdBufs.end())
            {
                mos_bo_map(it->second->OsResource.bo, 0);
                MosInterface::DumpCommandBuffer(streamState, it->second);
                mos_bo_unmap(it->second->OsResource.bo);
                it++;
            }
        }
        else
        {
            mos_bo_map(cmd_bo, 0);
            MosInterface::DumpCommandBuffer(streamState, cmdBuffer);
            mos_bo_unmap(cmd_bo);
        }
    }
    pthread_mutex_unlock(&command_dump_mutex);
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)
    if (nop_cmd_bo)
    {
        mos_bo_unreference(nop_cmd_bo);
    }
#endif  //(_DEBUG || _RELEASE_INTERNAL)

    //clear command buffer relocations to fix memory leak issue
    for (uint32_t patchIndex = 0; patchIndex < m_currentNumPatchLocations; patchIndex++)
    {
        auto currentPatch = &m_patchLocationList[patchIndex];
        MOS_OS_CHK_NULL_RETURN(currentPatch);

        if(currentPatch->cmdBo)
            mos_bo_clear_relocs(currentPatch->cmdBo, 0);
    }

    it = m_secondaryCmdBufs.begin();
    while(it != m_secondaryCmdBufs.end())
    {
        MOS_FreeMemory(it->second);
        it++;
    }
    m_secondaryCmdBufs.clear();

    skipSyncBoList.clear();

    // Reset resource allocation
    m_numAllocations = 0;
    MosUtilities::MosZeroMemory(m_allocationList, sizeof(ALLOCATION_LIST) * m_maxNumAllocations);
    m_currentNumPatchLocations = 0;
    MosUtilities::MosZeroMemory(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * m_maxNumAllocations);
    m_resCount = 0;

    MosUtilities::MosZeroMemory(m_writeModeList, sizeof(bool) * m_maxNumAllocations);
finish:
    MOS_TraceEventExt(EVENT_MOS_BATCH_SUBMIT, EVENT_TYPE_END, &eStatus, sizeof(eStatus), nullptr, 0);
    return eStatus;
}

void GpuContextSpecificNext::UnlockPendingOcaBuffers(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_CONTEXT mosContext)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(cmdBuffer);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(mosContext);
    MosOcaInterface *pOcaInterface         = &MosOcaInterfaceSpecific::GetInstance();
    if (nullptr == pOcaInterface || !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled())
    {
        // Will come here for UMD_OCA not being enabled case.
        return;
    }

    int count = 0;
    struct MOS_OCA_EXEC_LIST_INFO *info = nullptr;
    if ((cmdBuffer->iSubmissionType & SUBMISSION_TYPE_SINGLE_PIPE_MASK) && ((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaDumpExecListInfoEnabled())
    {
        info = mos_bo_get_softpin_targets_info(cmdBuffer->OsResource.bo, &count);
    }

    pOcaInterface->UnlockPendingOcaBuffers(mosContext, info, count);

    if(info)
    {
        free(info);
    }
}

int32_t GpuContextSpecificNext::SubmitPipeCommands(
    MOS_COMMAND_BUFFER *cmdBuffer,
    MOS_LINUX_BO *cmdBo,
    PMOS_CONTEXT osContext,
    const std::vector<MOS_LINUX_BO *> &skipSyncBoList,
    uint32_t execFlag,
    int32_t dr4)
{
    int32_t      ret        = 0;
    int          fence      = -1;
    unsigned int fence_flag = 0;

    MOS_LINUX_CONTEXT *queue = m_i915Context[0];
    bool isVeboxSubmission   = false;

    if (execFlag == MOS_GPU_NODE_VIDEO || execFlag == MOS_GPU_NODE_VIDEO2)
    {
        execFlag = I915_EXEC_DEFAULT;
    }
    if (execFlag == MOS_GPU_NODE_VE)
    {
        execFlag = I915_EXEC_DEFAULT;
        isVeboxSubmission = true;
    }

    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE)
    {
        fence = osContext->submit_fence;
        fence_flag = I915_EXEC_FENCE_SUBMIT;
        int slave_index = (cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_MASK) >> SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT;
        if(slave_index < 7)
        {
            queue = m_i915Context[2 + slave_index]; //0 is for single pipe, 1 is for master, slave starts from 2
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("slave_index value: %s is invalid!", slave_index);
            return -1;
        }

        if (isVeboxSubmission)
        {
            queue = m_i915Context[cmdBuffer->iVeboxNodeIndex + 1];
        }

        for(auto bo: skipSyncBoList)
        {
            mos_bo_set_exec_object_async(cmdBo, bo);
        }
    }

    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER)
    {
        //Only master pipe needs fence out flag
        fence_flag = I915_EXEC_FENCE_OUT;
        queue = m_i915Context[1];
    }

    ret = mos_bo_context_exec2(cmdBo,
                                  cmdBo->size,
                                  queue,
                                  nullptr,
                                  0,
                                  dr4,
                                  execFlag | fence_flag,
                                  &fence);

    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER)
    {
        osContext->submit_fence = fence;
    }

    if(cmdBuffer->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE)
    {
        close(fence);
    }

    return ret;
}

int32_t GpuContextSpecificNext::ParallelSubmitCommands(
    std::map<uint32_t, PMOS_COMMAND_BUFFER> secondaryCmdBufs,
    PMOS_CONTEXT osContext,
    uint32_t execFlag,
    int32_t dr4)
{
    int32_t      ret        = 0;
    int          fence      = -1;
    unsigned int fenceFlag  = 0;
    auto         it         = m_secondaryCmdBufs.begin();
    MOS_LINUX_BO *cmdBos[MAX_PARALLEN_CMD_BO_NUM];
    int          numBos     = 0; // exclude FE bo

    MOS_LINUX_CONTEXT *queue = m_i915Context[0];
    bool isVeboxSubmission   = false;

    if (execFlag == MOS_GPU_NODE_VIDEO || execFlag == MOS_GPU_NODE_VIDEO2)
    {
        execFlag = I915_EXEC_DEFAULT;
    }
    if (execFlag == MOS_GPU_NODE_VE)
    {
        execFlag = I915_EXEC_DEFAULT;
        isVeboxSubmission = true;
    }

    while(it != m_secondaryCmdBufs.end())
    {
        if(it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_ALONE)
        {
            fenceFlag = I915_EXEC_FENCE_OUT;
            queue = m_i915Context[0];

            ret = mos_bo_context_exec2(it->second->OsResource.bo,
                                  it->second->OsResource.bo->size,
                                  queue,
                                  nullptr,
                                  0,
                                  dr4,
                                  execFlag | fenceFlag,
                                  &fence);

            osContext->submit_fence = fence;
        }

        if((it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_MASTER)
            || (it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_SLAVE))
        {
            cmdBos[numBos++] = it->second->OsResource.bo;

            if(it->second->iSubmissionType & SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE)
            {
                queue = m_i915Context[numBos - 1];
                MOS_OS_CHK_NULL_RETURN(queue);
                if(-1 != fence)
                {
                    fenceFlag = I915_EXEC_FENCE_IN;
                }

                ret = mos_bo_context_exec3(cmdBos,
                                              numBos,
                                              queue,
                                              nullptr,
                                              0,
                                              dr4,
                                              execFlag | fenceFlag,
                                              &fence);

                for(int i = 0; i < numBos; i++)
                {
                    cmdBos[i] = nullptr;
                }
                numBos = 0;

                if(-1 != fence)
                {
                    close(fence);
                }
            }
        }

        it++;
    }

    return ret;
}

void GpuContextSpecificNext::IncrementGpuStatusTag()
{
    m_GPUStatusTag = m_GPUStatusTag % UINT_MAX + 1;
    if (m_GPUStatusTag == 0)
    {
        m_GPUStatusTag = 1;
    }
}

void GpuContextSpecificNext::UpdatePriority(int32_t priority)
{
    if(m_currCtxPriority == priority)
    {
        return;
    }

    for (int32_t i=0; i<MAX_ENGINE_INSTANCE_NUM+1; i++)
    {
        if (m_i915Context[i] != nullptr)
        {
            int32_t ret = mos_set_context_param(m_i915Context[i], 0, I915_CONTEXT_PARAM_PRIORITY,(uint64_t)priority);
            if (ret != 0)
            {
                MOS_OS_ASSERTMESSAGE("failed to set the gpu priority, errno is %d", ret);
                break;
            }
        }
    }
    m_currCtxPriority = priority;
}

void GpuContextSpecificNext::ResetGpuContextStatus()
{
    MosUtilities::MosZeroMemory(m_allocationList, sizeof(ALLOCATION_LIST) * ALLOCATIONLIST_SIZE);
    m_numAllocations = 0;
    MosUtilities::MosZeroMemory(m_patchLocationList, sizeof(PATCHLOCATIONLIST) * PATCHLOCATIONLIST_SIZE);
    m_currentNumPatchLocations = 0;

    MosUtilities::MosZeroMemory(m_attachedResources, sizeof(MOS_RESOURCE) * ALLOCATIONLIST_SIZE);
    m_resCount = 0;

    MosUtilities::MosZeroMemory(m_writeModeList, sizeof(bool) * ALLOCATIONLIST_SIZE);

    if ((m_cmdBufFlushed == true) && m_commandBuffer->OsResource.bo)
    {
        m_commandBuffer->OsResource.bo = nullptr;
    }
}

MOS_STATUS GpuContextSpecificNext::AllocateGPUStatusBuf()
{
    MOS_OS_FUNCTION_ENTER;

    m_statusBufferResource = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
    MOS_OS_CHK_NULL_RETURN(m_statusBufferResource);

    GraphicsResourceNext::CreateParams params;
    params.m_tileType  = MOS_TILE_LINEAR;
    params.m_type      = MOS_GFXRES_BUFFER;
    params.m_format    = Format_Buffer;
    params.m_width     = sizeof(MOS_GPU_STATUS_DATA);
    params.m_height    = 1;
    params.m_depth     = 1;
    params.m_arraySize = 1;
    params.m_name      = "GPU Status Buffer";

    GraphicsResourceNext *graphicsResource = GraphicsResourceNext::CreateGraphicResource(GraphicsResourceNext::osSpecificResource);
    MOS_OS_CHK_NULL_RETURN(graphicsResource);

    MOS_OS_CHK_STATUS_RETURN(graphicsResource->Allocate(m_osContext, params));

    GraphicsResourceNext::LockParams lockParams;
    lockParams.m_writeRequest = true;
    auto gpuStatusData       = (MOS_GPU_STATUS_DATA *)graphicsResource->Lock(m_osContext, lockParams);
    if (gpuStatusData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Unable to lock gpu eStatus buffer for read.");
        graphicsResource->Free(m_osContext);
        MOS_Delete(graphicsResource);
        return MOS_STATUS_UNKNOWN;
    }

    MOS_STATUS eStatus = graphicsResource->ConvertToMosResource(m_statusBufferResource);
    MOS_OS_CHK_STATUS_RETURN(eStatus);

    return MOS_STATUS_SUCCESS;
}

PMOS_RESOURCE GpuContextSpecificNext::GetOcaRTLogResource(PMOS_RESOURCE globalInst)
{
    // OcaRTLogResources are shared w/ different video processors.
    // iAllocationIndex array in MOS_RESOURCE indexed by gpu_context type. When resource being accessed
    // in GpuContextSpecificNext::RegisterResource and Mos_Specific_GetResourceAllocationIndex w/ more
    // than 2 video processors, the value may be overwritten and wrong allocation Index in array may be used.
    // To avoid this, use duplicate MOS_RESOURCE instance in GPU Context to ensure differnt iAllocationIndex
    // array of OcaRTLogResources being used for different GPU Context.
    if (!m_ocaRtLogResInited && globalInst)
    {
        m_ocaRtLogResource = *globalInst;
        m_ocaRtLogResInited = true;
    }
    return &m_ocaRtLogResource;
}

#if (_DEBUG || _RELEASE_INTERNAL)
bool GpuContextSpecificNext::SelectEngineInstanceByUser(void *engine_map,
        uint32_t *engineNum, uint32_t userEngineInstance, MOS_GPU_NODE gpuNode)
{
    uint32_t engineInstance     = 0x0;

    if (userEngineInstance && m_osParameters)
    {
        if(gpuNode == MOS_GPU_NODE_COMPUTE)
        {
            engineInstance  = (userEngineInstance >> ENGINE_INSTANCE_SELECT_COMPUTE_INSTANCE_SHIFT)
                & (ENGINE_INSTANCE_SELECT_ENABLE_MASK >> (MAX_ENGINE_INSTANCE_NUM - *engineNum));
        }
        else if(gpuNode == MOS_GPU_NODE_VE)
        {
            engineInstance  = (userEngineInstance >> ENGINE_INSTANCE_SELECT_VEBOX_INSTANCE_SHIFT)
                & (ENGINE_INSTANCE_SELECT_ENABLE_MASK >> (MAX_ENGINE_INSTANCE_NUM - *engineNum));
        }
        else if(gpuNode == MOS_GPU_NODE_VIDEO || gpuNode == MOS_GPU_NODE_VIDEO2)
        {
            engineInstance  = (userEngineInstance >> ENGINE_INSTANCE_SELECT_VDBOX_INSTANCE_SHIFT)
                & (ENGINE_INSTANCE_SELECT_ENABLE_MASK >> (MAX_ENGINE_INSTANCE_NUM - *engineNum));
        }
        else
        {
            MOS_OS_NORMALMESSAGE("Invalid gpu node in use.");
        }

        mos_select_fixed_engine(m_osParameters->bufmgr, engine_map, engineNum, engineInstance);
    }

    return engineInstance;
}
#endif
