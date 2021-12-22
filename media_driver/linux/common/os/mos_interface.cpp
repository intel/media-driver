/*
* Copyright (c) 2009-2021, Intel Corporation
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
//! \file     mos_interface.cpp
//! \brief    MOS interface implementation
//!

#include "mos_interface.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_os_specific_next.h"
#include "media_libva_common.h"
#include "mos_auxtable_mgr.h"
#include "mos_os_virtualengine_singlepipe_specific_next.h"
#include "mos_os_virtualengine_scalability_specific_next.h"
#include "mos_graphicsresource_specific_next.h"
#include "mos_bufmgr_priv.h"

#if (_DEBUG || _RELEASE_INTERNAL)
#include <stdlib.h>   //for simulate random OS API failure
#include <time.h>     //for simulate random OS API failure
#endif

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#include <fstream>
#include <sstream>
#include <iomanip>
#endif

MOS_STATUS MosInterface::InitOsUtilities(DDI_DEVICE_CONTEXT ddiDeviceContext)
{
    MOS_UNUSED(ddiDeviceContext);
    MosUtilities::MosUtilitiesInit(nullptr);

    // MOS_OS_FUNCTION_ENTER need mos utilities init
    MOS_OS_FUNCTION_ENTER;

#if (_DEBUG || _RELEASE_INTERNAL)
    //Init MOS OS API fail simulate flags
    MosInitOsApiFailSimulateFlag(ddiDeviceContext);
#endif

    //Read user feature key here for Per Utility Tool Enabling
    if (!g_perfutility->bPerfUtilityKey)
    {
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MosUtilities::MosUserFeatureReadValueID(
            NULL,
            __MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE_ID,
            &UserFeatureData,
            (MOS_CONTEXT_HANDLE) nullptr);
        g_perfutility->dwPerfUtilityIsEnabled = UserFeatureData.i32Data;

        char                        sFilePath[MOS_MAX_PERF_FILENAME_LEN + 1] = "";
        MOS_USER_FEATURE_VALUE_DATA perfFilePath;
        MOS_STATUS                  eStatus_Perf = MOS_STATUS_SUCCESS;

        MosUtilities::MosZeroMemory(&perfFilePath, sizeof(perfFilePath));
        perfFilePath.StringData.pStringData = sFilePath;
        eStatus_Perf                        = MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY_ID,
            &perfFilePath,
            (MOS_CONTEXT_HANDLE) nullptr);
        if (eStatus_Perf == MOS_STATUS_SUCCESS)
        {
            g_perfutility->setupFilePath(sFilePath);
        }
        else
        {
            g_perfutility->setupFilePath();
        }

        g_perfutility->bPerfUtilityKey = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CloseOsUtilities(PMOS_CONTEXT mosCtx)
{
    MOS_OS_FUNCTION_ENTER;
    // Close MOS utlities
    MosUtilities::MosUtilitiesClose(nullptr);

#if (_DEBUG || _RELEASE_INTERNAL)
    //reset MOS init OS API simulate flags
    MosDeinitOsApiFailSimulateFlag();
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsDeviceContext(DDI_DEVICE_CONTEXT ddiDeviceContext, MOS_DEVICE_HANDLE *deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(deviceContext);
    MOS_OS_CHK_NULL_RETURN(ddiDeviceContext);

    *deviceContext = MOS_New(OsContextSpecificNext);

    MOS_OS_CHK_NULL_RETURN(*deviceContext);

    MOS_OS_CHK_STATUS_RETURN((*deviceContext)->Init((PMOS_CONTEXT)ddiDeviceContext));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsDeviceContext(MOS_DEVICE_HANDLE deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    if (deviceContext)
    {
        deviceContext->CleanUp();
        MOS_Delete(deviceContext);
        deviceContext = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsStreamState(
    MOS_STREAM_HANDLE    *streamState,
    MOS_DEVICE_HANDLE    deviceContext,
    MOS_INTERFACE_HANDLE osInterface,
    MOS_COMPONENT        component,
    EXTRA_PARAMS         extraParams)
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData     = {};
    MOS_STATUS                  eStatusUserFeature  = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(deviceContext);

    *streamState = MOS_New(MosStreamState);
    MOS_OS_CHK_NULL_RETURN(*streamState);

    (*streamState)->osDeviceContext         = deviceContext;
    (*streamState)->component               = component;
    (*streamState)->currentGpuContextHandle = 0;

    (*streamState)->simIsActive             = false;
    (*streamState)->mediaReset              = false;

    (*streamState)->usesPatchList           = true;
    (*streamState)->usesGfxAddress          = !(*streamState)->usesPatchList;

    MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SIM_ENABLE_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->simIsActive = (int32_t)userFeatureData.i32Data;

    // Null HW Driver
    // 0: Disabled
    MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->nullHwAccelerationEnable.Value = userFeatureData.u32Data;
#endif

    // SupportVirtualEngine flag is set by Hals
    (*streamState)->supportVirtualEngine    = false;
    (*streamState)->useHwSemaForResSyncInVe = false;
    (*streamState)->virtualEngineInterface  = nullptr;
    (*streamState)->veEnable                = false;
    (*streamState)->phasedSubmission        = true;

    auto skuTable = GetSkuTable(*streamState);
    MOS_OS_CHK_NULL_RETURN(skuTable);
    if (MEDIA_IS_SKU(skuTable, FtrGucSubmission))
    {
        (*streamState)->bGucSubmission = true;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // read the "Force VDBOX" user feature key
    // 0: not force
    MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->eForceVdbox = userFeatureData.u32Data;

    //Read Scalable/Legacy Decode mode on Gen11+
    //1:by default for scalable decode mode
    //0:for legacy decode mode
    MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
    eStatusUserFeature = MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_HCP_SCALABILITY_DECODE_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->hcpDecScalabilityMode = userFeatureData.u32Data ? MOS_SCALABILITY_ENABLE_MODE_DEFAULT : MOS_SCALABILITY_ENABLE_MODE_FALSE;
    if((*streamState)->hcpDecScalabilityMode
        && (eStatusUserFeature == MOS_STATUS_SUCCESS))
    {
        //user's value to enable scalability
        (*streamState)->hcpDecScalabilityMode = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE;
    }

    (*streamState)->frameSplit = false;
    MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MosUtilities::MosUserFeatureReadValueID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_LINUX_FRAME_SPLIT_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->frameSplit = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_GUC_SUBMISSION_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->bGucSubmission = (*streamState)->bGucSubmission && ((uint32_t)userFeatureData.i32Data);

    //KMD Virtual Engine DebugOverride
    // 0: not Override
    MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VE_DEBUG_OVERRIDE_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE) nullptr);
    (*streamState)->enableDbgOvrdInVirtualEngine = userFeatureData.u32Data ? true : false;
#endif

    if (component == COMPONENT_VPCommon ||
        component == COMPONENT_VPreP    ||
        component == COMPONENT_LibVA)
    {
        // UMD Vebox Virtual Engine Scalability Mode
        // 0: disable. can set to 1 only when KMD VE is enabled.
        MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
        eStatusUserFeature = MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE_ID,
            &userFeatureData,
            (MOS_CONTEXT_HANDLE) nullptr);
        (*streamState)->veboxScalabilityMode = userFeatureData.u32Data ? MOS_SCALABILITY_ENABLE_MODE_DEFAULT : MOS_SCALABILITY_ENABLE_MODE_FALSE;

#if (_DEBUG || _RELEASE_INTERNAL)
        if((*streamState)->veboxScalabilityMode
            && (eStatusUserFeature == MOS_STATUS_SUCCESS))
        {
            //user's value to enable scalability
            (*streamState)->veboxScalabilityMode = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE;
            (*streamState)->enableDbgOvrdInVirtualEngine = true;

            if ((*streamState)->eForceVebox == MOS_FORCE_VEBOX_NONE)
            {
                (*streamState)->eForceVebox = MOS_FORCE_VEBOX_1_2;
            }
        }
        else if ((!(*streamState)->veboxScalabilityMode)
            && (eStatusUserFeature == MOS_STATUS_SUCCESS))
        {
            (*streamState)->enableDbgOvrdInVirtualEngine = true;
            (*streamState)->eForceVebox        = MOS_FORCE_VEBOX_NONE;
        }

        // read the "Force VEBOX" user feature key
        // 0: not force
        MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_FORCE_VEBOX_ID,
            &userFeatureData,
            (MOS_CONTEXT_HANDLE) nullptr);
        (*streamState)->eForceVebox = (MOS_FORCE_VEBOX)userFeatureData.u32Data;
#endif
    }

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    // Report if pre-si environment is in use
    userFeatureWriteData.Value.i32Data = (*streamState)->simIsActive;
    userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE_ID;
    MosUtilities::MosUserFeatureWriteValuesID(
        nullptr,
        &userFeatureWriteData,
        1,
        (MOS_CONTEXT_HANDLE)nullptr);

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    DumpCommandBufferInit(*streamState);
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

    MOS_OS_CHK_STATUS_RETURN(MosInterface::InitStreamParameters(*streamState, extraParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsStreamState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    MOS_Delete(streamState);
    streamState = nullptr;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::InitStreamParameters(
    MOS_STREAM_HANDLE   streamState,
    EXTRA_PARAMS        extraParams)
{
    MOS_STATUS                  eStatus             = MOS_STATUS_SUCCESS;
    PMOS_CONTEXT                context             = nullptr;
    uint32_t                    resetCount          = 0;
    int32_t                     ret                 = 0;
    MOS_BUFMGR                  *bufMgr             = nullptr;
    int32_t                     fd                  = -1;
    OsContextSpecificNext       *osDeviceContext    = nullptr;
    MOS_USER_FEATURE_VALUE_DATA userFeatureData     = {};

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    MOS_OS_CHK_NULL_RETURN(extraParams);

    osDeviceContext = (OsContextSpecificNext *)streamState->osDeviceContext;
    fd              = osDeviceContext->GetFd();
    if (0 >= fd)
    {
        MOS_OS_ASSERTMESSAGE("Invalid fd");
        return MOS_STATUS_INVALID_HANDLE;
    }

    bufMgr = osDeviceContext->GetBufMgr();
    MOS_OS_CHK_NULL_RETURN(bufMgr);

    context = (PMOS_OS_CONTEXT)MOS_AllocAndZeroMemory(sizeof(MOS_OS_CONTEXT));
    MOS_OS_CHK_NULL_RETURN(context);

    context->m_apoMosEnabled    = true;
    context->m_osDeviceContext  = streamState->osDeviceContext;
    context->bSimIsActive       = streamState->simIsActive;

    streamState->perStreamParameters = (OS_PER_STREAM_PARAMETERS)context;

    context->pGmmClientContext  = streamState->osDeviceContext->GetGmmClientContext();;

    context->bufmgr             = bufMgr;
    context->m_gpuContextMgr    = osDeviceContext->GetGpuContextMgr();
    context->m_cmdBufMgr        = osDeviceContext->GetCmdBufferMgr();
    context->fd                 = fd;
    context->pPerfData          = ((PMOS_CONTEXT)extraParams)->pPerfData;

    context->m_auxTableMgr      = osDeviceContext->GetAuxTableMgr();

    mos_bufmgr_gem_enable_reuse(bufMgr);

    context->SkuTable           = *osDeviceContext->GetSkuTable();
    context->WaTable            = *osDeviceContext->GetWaTable();
    context->gtSystemInfo       = *osDeviceContext->GetGtSysInfo();
    context->platform           = *osDeviceContext->GetPlatformInfo();

    context->bUse64BitRelocs    = true;
    context->bUseSwSwizzling    = context->bSimIsActive || MEDIA_IS_SKU(&context->SkuTable, FtrUseSwSwizzling);
    context->bTileYFlag         = MEDIA_IS_SKU(&context->SkuTable, FtrTileY);

    if (MEDIA_IS_SKU(&context->SkuTable, FtrContextBasedScheduling))
    {
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_START,
                          &eStatus, sizeof(eStatus), nullptr, 0);
        context->intel_context = mos_gem_context_create_ext(context->bufmgr, 0);
        MOS_OS_CHK_NULL_RETURN(context->intel_context);
        context->intel_context->vm = mos_gem_vm_create(context->bufmgr);
        MOS_OS_CHK_NULL_RETURN(context->intel_context->vm);
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_END,
                          &context->intel_context, sizeof(void *),
                          &eStatus, sizeof(eStatus));
    }
    else  //use legacy context create ioctl for pre-gen11 platforms
    {
        MOS_OS_ASSERTMESSAGE("Do not support the legacy context creation.\n");
        MOS_FreeMemAndSetNull(context->pPerfData);
        MOS_FreeMemAndSetNull(context);
        streamState->perStreamParameters = nullptr;
        return MOS_STATUS_UNIMPLEMENTED;
    }
    context->intel_context->pOsContext = context;
    ret                                = mos_get_reset_stats(context->intel_context, &resetCount, nullptr, nullptr);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        resetCount = 0;
    }
    streamState->ctxPriority      = 0;
    streamState->gpuResetCount    = resetCount;
    streamState->gpuActiveBatch   = 0;
    streamState->gpuPendingBatch  = 0;

    context->bIsAtomSOC           = false;
    context->bFreeContext         = true;
#ifndef ANDROID
    {
        drm_i915_getparam_t gp;
        int32_t             ret   = -1;
        int32_t             value = 0;

        //KMD support VCS2?
        gp.value = &value;
        gp.param = I915_PARAM_HAS_BSD2;

        ret = drmIoctl(context->fd, DRM_IOCTL_I915_GETPARAM, &gp);
        if (ret == 0 && value != 0)
        {
            context->bKMDHasVCS2 = true;
        }
        else
        {
            context->bKMDHasVCS2 = false;
        }
    }
#endif

    // read "Linux PerformanceTag Enable" user feature key
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE_ID,
        &userFeatureData,
        (MOS_CONTEXT_HANDLE)nullptr);
    context->uEnablePerfTag = userFeatureData.u32Data;

    return MOS_STATUS_SUCCESS;
}

uint32_t MosInterface::GetInterfaceVersion(MOS_DEVICE_HANDLE deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    // No interface version to get in Linux

    return 0;
}

PLATFORM *MosInterface::GetPlatform(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetPlatformInfo();
    }

    return nullptr;
}

MEDIA_FEATURE_TABLE *MosInterface::GetSkuTable(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetSkuTable();
    }

    return nullptr;
}

MEDIA_WA_TABLE *MosInterface::GetWaTable(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetWaTable();
    }

    return nullptr;
}

MEDIA_SYSTEM_INFO *MosInterface::GetGtSystemInfo(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetGtSysInfo();
    }

    return nullptr;
}

MOS_STATUS MosInterface::GetMediaEngineInfo(MOS_STREAM_HANDLE streamState, MEDIA_ENGINE_INFO &info)
{
    MOS_OS_FUNCTION_ENTER;

    auto systemInfo = MosInterface::GetGtSystemInfo(streamState);
    MOS_OS_CHK_NULL_RETURN(systemInfo);

    MosUtilities::MosZeroMemory(&info, sizeof(info));
    info.VDBoxInfo = systemInfo->VDBoxInfo;
    info.VEBoxInfo = systemInfo->VEBoxInfo;

    return MOS_STATUS_SUCCESS;
}

ADAPTER_INFO *MosInterface::GetAdapterInfo(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    // No adapter Info in Linux

    return nullptr;
}

MOS_STATUS MosInterface::CreateGpuContext(
    MOS_STREAM_HANDLE             streamState,
    GpuContextCreateOption       &createOption,
    GPU_CONTEXT_HANDLE           &gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    auto osDeviceContext = streamState->osDeviceContext;

    auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);

    auto cmdBufMgr = osDeviceContext->GetCmdBufferMgr();
    MOS_OS_CHK_NULL_RETURN(cmdBufMgr);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osParameters);

    if (createOption.gpuNode == MOS_GPU_NODE_3D && createOption.SSEUValue != 0)
    {
        struct drm_i915_gem_context_param_sseu sseu;
        MosUtilities::MosZeroMemory(&sseu, sizeof(sseu));
        sseu.engine.engine_class    = I915_ENGINE_CLASS_RENDER;
        sseu.engine.engine_instance = 0;

        if (mos_get_context_param_sseu(osParameters->intel_context, &sseu))
        {
            MOS_OS_ASSERTMESSAGE("Failed to get sseu configuration.");
            return MOS_STATUS_UNKNOWN;
        };

        if (mos_hweight8(sseu.subslice_mask) > createOption.packed.SubSliceCount)
        {
            sseu.subslice_mask = mos_switch_off_n_bits(sseu.subslice_mask,
                mos_hweight8(sseu.subslice_mask) - createOption.packed.SubSliceCount);
        }

        if (mos_set_context_param_sseu(osParameters->intel_context, sseu))
        {
            MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
            return MOS_STATUS_UNKNOWN;
        };
    }

    MOS_GPU_NODE gpuNode = MOS_GPU_NODE_3D;
    gpuNode = static_cast<MOS_GPU_NODE>(createOption.gpuNode);

    auto gpuContext = gpuContextMgr->CreateGpuContext(gpuNode, cmdBufMgr);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    auto gpuContextSpecific = static_cast<GpuContextSpecificNext *>(gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextSpecific);

    MOS_OS_CHK_STATUS_RETURN(gpuContextSpecific->Init(gpuContextMgr->GetOsContext(), streamState, &createOption));

    gpuContextHandle = gpuContextSpecific->GetGpuContextHandle();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetAdapterBDF(PMOS_CONTEXT mosCtx, ADAPTER_BDF *adapterBDF)
{
    MOS_OS_FUNCTION_ENTER;

    drmDevicePtr device;
    
    MOS_OS_CHK_NULL_RETURN(mosCtx);
    if (drmGetDevice(mosCtx->fd, &device) == 0)
    {
        adapterBDF->Bus      = device->businfo.pci->bus;
        adapterBDF->Device   = device->businfo.pci->dev;
        adapterBDF->Function = device->businfo.pci->func;
        drmFreeDevice(&device);
    }
    else
    {
        adapterBDF->Data = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyGpuContext(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    MOS_OS_ASSERT(gpuContext != MOS_GPU_CONTEXT_INVALID_HANDLE);

    auto gpuContextMgr = streamState->osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
    auto gpuContextInstance = gpuContextMgr->GetGpuContext(gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextInstance);

    gpuContextMgr->DestroyGpuContext(gpuContextInstance);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetGpuContext(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContextMgr = streamState->osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
    auto gpuContextPtr = gpuContextMgr->GetGpuContext(gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextPtr);

    streamState->currentGpuContextHandle = gpuContext;

    return MOS_STATUS_SUCCESS;
}

void *MosInterface::GetGpuContextbyHandle(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContextHandle)
{
    if (!streamState || !streamState->osDeviceContext)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return nullptr;
    }

    auto gpuContextMgr = streamState->osDeviceContext->GetGpuContextMgr();
    if (!gpuContextMgr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return nullptr;
    }

    GpuContextNext *gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);

    if (!gpuContext)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
    }
    return (void *)gpuContext;
}

MOS_STATUS MosInterface:: SetObjectCapture(
    PMOS_RESOURCE osResource)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    MOS_OS_CHK_NULL_RETURN(osResource);
    //---------------------------------------

    mos_bo_set_object_capture(osResource->bo);

    return eStatus;
}

MOS_STATUS MosInterface::AddCommand(
    COMMAND_BUFFER_HANDLE cmdBuffer,
    const void *cmd,
    uint32_t cmdSize)
{
    MOS_OS_FUNCTION_ENTER;

    uint32_t cmdSizeDwAligned = 0;

    MOS_OS_CHK_NULL_RETURN(cmdBuffer);
    MOS_OS_CHK_NULL_RETURN(cmd);

    if (cmdSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("Incorrect command size to add to command buffer.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    cmdSizeDwAligned = MOS_ALIGN_CEIL(cmdSize, sizeof(uint32_t));

    cmdBuffer->iOffset += cmdSizeDwAligned;
    cmdBuffer->iRemaining -= cmdSizeDwAligned;

    if (cmdBuffer->iRemaining < 0)
    {
        cmdBuffer->iOffset -= cmdSizeDwAligned;
        cmdBuffer->iRemaining += cmdSizeDwAligned;
        MOS_OS_ASSERTMESSAGE("Unable to add command: remaining space = %d, command size = %d.",
            cmdBuffer->iRemaining,
            cmdSizeDwAligned);
        return MOS_STATUS_UNKNOWN;
    }

    MOS_OS_VERBOSEMESSAGE("The command was successfully added: remaining space = %d, buffer size = %d.",
        cmdBuffer->iRemaining,
        cmdBuffer->iOffset + cmdBuffer->iRemaining);

    MosUtilities::MosSecureMemcpy(cmdBuffer->pCmdPtr, cmdSize, cmd, cmdSize);
    cmdBuffer->pCmdPtr += (cmdSizeDwAligned / sizeof(uint32_t));

    return MOS_STATUS_SUCCESS;
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
MOS_STATUS MosInterface::DumpIndirectState(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    MOS_GPU_NODE          gpuNode,
    const char            *filePathPrefix)
{
    MOS_OS_CHK_NULL_RETURN(filePathPrefix);

    if (MOS_GPU_NODE_COMPUTE == gpuNode || MOS_GPU_NODE_3D == gpuNode)
    {
        uint8_t *indirectState = nullptr;
        uint32_t offset = 0;
        uint32_t size = 0;
        MosInterface::GetIndirectState(streamState, &indirectState, offset, size);

        if (indirectState)
        {
            std::stringstream ss;
            uint32_t dwordCount = size / 4;
            uint32_t *data = (uint32_t *)indirectState;

            for (uint32_t i = 0; i < dwordCount; ++i)
            {
                if (0 == i % 4)
                {
                    if (0 != i)
                    {
                        ss << std::endl;
                    }
                    ss << "#0    #0";
                }
                ss << "    " << std::hex << std::setw(8) << std::setfill('0') << data[i];
            }

            std::stringstream fileName;
            fileName << filePathPrefix << "_binding_table.txt";
            std::fstream fs;
            fs.open(fileName.str(), std::ios_base::out | std::ios_base::app);
            fs << ss.str();
            fs.close();
        }
        else
        {
            MOS_OS_NORMALMESSAGE("nullptr == indirectState");
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DumpCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    static uint32_t dwCommandBufferNumber = 0;
    MOS_STATUS      eStatus               = MOS_STATUS_UNKNOWN;
    char *          pOutputBuffer         = nullptr;
    // Each hex value should have 9 chars.
    uint32_t SIZE_OF_ONE_WORD = 9;
    uint32_t dwBytesWritten   = 0;
    uint32_t dwNumberOfDwords = 0;
    uint32_t dwSizeToAllocate = 0;
    char     sFileName[MOS_MAX_HLT_FILENAME_LEN] = {0};
    // Maximum length of engine name is 6
    char sEngName[6];
    size_t nSizeFileNamePrefix   = 0;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    // Set the name of the engine that is going to be used.
    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);
    MOS_GPU_NODE gpuNode = gpuContext->GetContextNode();
    switch (gpuNode)
    {
    case MOS_GPU_NODE_VIDEO:
    case MOS_GPU_NODE_VIDEO2:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_VIDEO_ENGINE);
        break;
    case MOS_GPU_NODE_COMPUTE:
    case MOS_GPU_NODE_BLT:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_RENDER_ENGINE);
        break;
    case MOS_GPU_NODE_VE:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_VEBOX_ENGINE);
        break;
    case MOS_GPU_NODE_3D:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_RENDER_ENGINE);
        break;
    default:
        MOS_OS_ASSERTMESSAGE("Unsupported GPU context.");
        return eStatus;
    }

    dwNumberOfDwords = cmdBuffer->iOffset / sizeof(uint32_t);

    dwSizeToAllocate =
        dwNumberOfDwords * (SIZE_OF_ONE_WORD + 1)  // Add 1 byte for the space following each Dword.
        + 3 * SIZE_OF_ONE_WORD;                    // For engine and platform names.

    // Alloc output buffer.
    pOutputBuffer = (char *)MOS_AllocAndZeroMemory(dwSizeToAllocate);
    MOS_OS_CHK_NULL_RETURN(pOutputBuffer);

    dwBytesWritten = MosUtilities::MosSecureStringPrint(
        pOutputBuffer,
        SIZE_OF_ONE_WORD * 3,
        SIZE_OF_ONE_WORD * 3,
        "Eng=%s ",
        sEngName);

    if (streamState->dumpCommandBufferToFile)
    {
        MosUtilities::MosSecureMemcpy(sFileName, MOS_MAX_HLT_FILENAME_LEN, streamState->sDirName, MOS_MAX_HLT_FILENAME_LEN);

        nSizeFileNamePrefix = strnlen(sFileName, sizeof(sFileName));
        MosUtilities::MosSecureStringPrint(
            sFileName + nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            "%c%s%c%s_%d.txt",
            MOS_DIR_SEPERATOR,
            MOS_COMMAND_BUFFER_OUT_DIR,
            MOS_DIR_SEPERATOR,
            MOS_COMMAND_BUFFER_OUT_FILE,
            dwCommandBufferNumber);

        // Write the output buffer to file.
        MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosWriteFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
    }

    if (streamState->dumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    MosUtilities::MosZeroMemory(pOutputBuffer, dwBytesWritten);
    dwBytesWritten = 0;

    // Fill in the output buffer with the command buffer dwords.
    for (uint32_t dwIndex = 0; dwIndex < dwNumberOfDwords; dwIndex++)
    {
        dwBytesWritten += MosUtilities::MosSecureStringPrint(
            pOutputBuffer + dwBytesWritten,
            SIZE_OF_ONE_WORD + 1,
            SIZE_OF_ONE_WORD + 1,
            "%.8x ",
            cmdBuffer->pCmdBase[dwIndex]);

        if (dwBytesWritten % (SIZE_OF_ONE_WORD + 1) == 0)
        {
            if (streamState->dumpCommandBufferToFile)
            {
                MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
            }
            if (streamState->dumpCommandBufferAsMessages)
            {
                MOS_OS_NORMALMESSAGE(pOutputBuffer);
            }

            MosUtilities::MosZeroMemory(pOutputBuffer, dwBytesWritten);
            dwBytesWritten = 0;
        }
    }

    if (streamState->dumpCommandBufferToFile)
    {
        MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
        MOS_OS_CHK_STATUS_RETURN(DumpIndirectState(streamState, cmdBuffer, gpuNode, sFileName));
    }

    if (streamState->dumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    dwCommandBufferNumber++;

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;

}
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

MOS_STATUS MosInterface::GetCommandBuffer(
    MOS_STREAM_HANDLE      streamState,
    COMMAND_BUFFER_HANDLE &cmdBuffer,
    uint32_t               pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->GetCommandBuffer(cmdBuffer, pipeIdx));
}

MOS_STATUS MosInterface::ReturnCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    (gpuContext->ReturnCommandBuffer(cmdBuffer, pipeIdx));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SubmitCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    bool                  nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    gpuContext->UpdatePriority(streamState->ctxPriority);

    return (gpuContext->SubmitCommandBuffer(streamState, cmdBuffer, nullRendering));
}

MOS_STATUS MosInterface::ResetCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    // Reset the explicitly provided cmd buffer, or reset GPU context states
    if (cmdBuffer)
    {
        MOS_OS_CHK_STATUS_RETURN(gpuContext->ResetCommandBuffer());
    }
    else
    {
        gpuContext->ResetGpuContextStatus();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::VerifyCommandBufferSize(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              requestedSize,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->VerifyCommandBufferSize(requestedSize));
}

MOS_STATUS MosInterface::ResizeCommandBufferAndPatchList(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              requestedSize,
    uint32_t              requestedPatchListSize,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->ResizeCommandBufferAndPatchList(requestedSize, requestedPatchListSize, pipeIdx));
}

MOS_STATUS MosInterface::SetPatchEntry(
    MOS_STREAM_HANDLE       streamState,
    PMOS_PATCH_ENTRY_PARAMS params)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(params);
    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_OS_CHK_STATUS_RETURN(gpuContext->SetPatchEntry(streamState, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetIndirectState(
    MOS_STREAM_HANDLE streamState,
    uint8_t **indirectState,
    uint32_t &offset,
    uint32_t &size)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_OS_CHK_STATUS_RETURN(gpuContext->GetIndirectState(offset, size));
    if (indirectState)
    {
        MOS_OS_CHK_STATUS_RETURN(gpuContext->GetIndirectStatePointer(indirectState));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetupIndirectState(
    MOS_STREAM_HANDLE     streamState,
    uint32_t              size)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_OS_CHK_STATUS_RETURN(gpuContext->SetIndirectStateSize(size));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetupAttributeVeBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    // no VE attribute buffer to setup

    return MOS_STATUS_SUCCESS;
}

MOS_CMD_BUF_ATTRI_VE *MosInterface::GetAttributeVeBuffer(
        COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    // no VE attribute buffer to get
    return nullptr;
}

static GMM_RESOURCE_USAGE_TYPE GmmResourceUsage[MOS_HW_RESOURCE_DEF_MAX] =
{
    //
    // CODEC USAGES
    //
    GMM_RESOURCE_USAGE_BEGIN_CODEC,
    GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC,
    GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC_PARTIALENCSURFACE,
    GMM_RESOURCE_USAGE_POST_DEBLOCKING_CODEC,
    GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE,
    GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE,
    GMM_RESOURCE_USAGE_STREAMOUT_DATA_CODEC,
    GMM_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC,
    GMM_RESOURCE_USAGE_MACROBLOCK_STATUS_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE,
    GMM_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC,
    GMM_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE,
    GMM_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC,
    GMM_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_BITPLANE_READ_CODEC,
    GMM_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SURFACE_CURR_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_REF_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_FF,
    GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST,
    GMM_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_PAK_OBJECT_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MAD_ENCODE,
    GMM_RESOURCE_USAGE_VP8_BLOCK_MODE_COST_ENCODE,
    GMM_RESOURCE_USAGE_VP8_MB_MODE_COST_ENCODE,
    GMM_RESOURCE_USAGE_VP8_MBENC_OUTPUT_ENCODE,
    GMM_RESOURCE_USAGE_VP8_HISTOGRAM_ENCODE,
    GMM_RESOURCE_USAGE_VP8_L3_LLC_ENCODE,
    GMM_RESOURCE_USAGE_MFX_STANDALONE_DEBLOCKING_CODEC,
    GMM_RESOURCE_USAGE_HCP_MD_CODEC,
    GMM_RESOURCE_USAGE_HCP_SAO_CODEC,
    GMM_RESOURCE_USAGE_HCP_MV_CODEC,
    GMM_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC,
    GMM_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC,
    GMM_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC,
    GMM_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_VDENC_STREAMIN_CODEC,
    GMM_RESOURCE_USAGE_SURFACE_MB_QP_CODEC,
    GMM_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SSE_SRC_PIXEL_ROW_STORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SLICE_STATE_STREAM_OUT_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_CABAC_SYNTAX_STREAM_OUT_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_PRED_COL_STORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MBENC_BRC_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MB_BRC_CONST_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_ROI_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_SLICE_MAP_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_VDENC_IMAGESTATE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_UNCACHED,
    GMM_RESOURCE_USAGE_SURFACE_ELLC_ONLY,
    GMM_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY,
    GMM_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3,
    GMM_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_SOFTWARE_SCOREBOARD_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MV_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_4XME_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_INTRA_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_MB_STATS_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_PAK_STATS_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_PIC_STATE_READ_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_PIC_STATE_WRITE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_COMBINED_ENC_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_CONSTANT_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_INTERMEDIATE_CU_RECORD_SURFACE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_SCRATCH_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_LCU_LEVEL_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_HISTORY_INPUT_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_HISTORY_OUTPUT_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_DEBUG_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_CONSTANT_TABLE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_CU_RECORD_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_MV_TEMPORAL_BUFFER_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_CU_PACKET_FOR_PAK_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_BCOMBINED1_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_BCOMBINED2_ENCODE,
    GMM_RESOURCE_USAGE_FRAME_STATS_STREAMOUT_DATA_CODEC,
    GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_LINE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_COLUMN_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_HCP_MD_TILE_LINE_CODEC,
    GMM_RESOURCE_USAGE_HCP_MD_TILE_COLUMN_CODEC,
    GMM_RESOURCE_USAGE_HCP_SAO_TILE_LINE_CODEC,
    GMM_RESOURCE_USAGE_HCP_SAO_TILE_COLUMN_CODEC,
    GMM_RESOURCE_USAGE_VP9_PROBABILITY_COUNTER_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_HUC_VIRTUAL_ADDR_REGION_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SIZE_STREAMOUT_CODEC,
    GMM_RESOURCE_USAGE_COMPRESSED_HEADER_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_PROBABILITY_DELTA_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_TILE_RECORD_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_TILE_SIZE_STAS_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_END_CODEC,

    //
    // CM USAGES
    //
    CM_RESOURCE_USAGE_SurfaceState,
    CM_RESOURCE_USAGE_StateHeap,
    CM_RESOURCE_USAGE_NO_L3_SurfaceState,
    CM_RESOURCE_USAGE_NO_LLC_ELLC_SurfaceState,
    CM_RESOURCE_USAGE_NO_LLC_SurfaceState,
    CM_RESOURCE_USAGE_NO_ELLC_SurfaceState,
    CM_RESOURCE_USAGE_NO_LLC_L3_SurfaceState,
    CM_RESOURCE_USAGE_NO_ELLC_L3_SurfaceState,
    CM_RESOURCE_USAGE_NO_CACHE_SurfaceState,
    CM_RESOURCE_USAGE_L1_Enabled_SurfaceState,

    //
    // MP USAGES
    //
    MP_RESOURCE_USAGE_BEGIN,
    MP_RESOURCE_USAGE_DEFAULT,
    MP_RESOURCE_USAGE_DEFAULT_FF,
    MP_RESOURCE_USAGE_DEFAULT_RCS,
    MP_RESOURCE_USAGE_SurfaceState,
    MP_RESOURCE_USAGE_SurfaceState_FF,
    MP_RESOURCE_USAGE_SurfaceState_RCS,
    MP_RESOURCE_USAGE_AGE3_SurfaceState,
    MP_RESOURCE_USAGE_EDRAM_SurfaceState,
    MP_RESOURCE_USAGE_EDRAM_AGE3_SurfaceState,
    MP_RESOURCE_USAGE_No_L3_SurfaceState,
    MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState,
    MP_RESOURCE_USAGE_No_LLC_L3_AGE_SurfaceState,
    MP_RESOURCE_USAGE_No_LLC_eLLC_L3_AGE_SurfaceState,
    MP_RESOURCE_USAGE_PartialEnc_No_LLC_L3_AGE_SurfaceState,
    MP_RESOURCE_USAGE_END,

    // MHW - SFC
    MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,                    //!< SFC output surface
    MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface_PartialEncSurface,  //!< SFC output surface for partial secure surfaces
    MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,                    //!< SFC AVS Line buffer Surface
    MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,                    //!< SFC IEF Line buffer Surface

    // PAT Media Usages
    GMM_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS,
    // DECODE
    GMM_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM,
    GMM_RESOURCE_USAGE_DECODE_INPUT_REFERENCE,
    GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ,
    GMM_RESOURCE_USAGE_DECODE_INTERNAL_WRITE,
    GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE,
    GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE,
    GMM_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE,
    GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE,
    GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE,
    // ENCODE
    GMM_RESOURCE_USAGE_ENCODE_INPUT_RAW,
    GMM_RESOURCE_USAGE_ENCODE_INPUT_RECON,
    GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ,
    GMM_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE,
    GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE,
    GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE,
    GMM_RESOURCE_USAGE_ENCODE_EXTERNAL_READ,
    GMM_RESOURCE_USAGE_ENCODE_OUTPUT_PICTURE,
    GMM_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM,
    GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_WRITE,
    GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_READ_WRITE,
    // VP
    GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_FF,
    GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_FF,
    GMM_RESOURCE_USAGE_VP_INTERNAL_READ_FF,
    GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF,
    GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
    GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF,
    GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER,
    GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_RENDER,
    GMM_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER,
    GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_RENDER,
    GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER,
    GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_RENDER,
    // CP
    GMM_RESOURCE_USAGE_CP_EXTERNAL_READ,
    GMM_RESOURCE_USAGE_CP_INTERNAL_WRITE,
};

GMM_RESOURCE_USAGE_TYPE MosInterface::GetGmmResourceUsageType(
    MOS_HW_RESOURCE_DEF resUsage)
{
    if (resUsage >= (sizeof(GmmResourceUsage) / sizeof(GmmResourceUsage[0])))
        return GMM_RESOURCE_USAGE_UNKNOWN;

    return GmmResourceUsage[resUsage];
}

MEMORY_OBJECT_CONTROL_STATE MosInterface::GetGmmCachePolicyMemoryObject(
    GMM_CLIENT_CONTEXT      *gmmClientContext,
    GMM_RESOURCE_USAGE_TYPE gmmUsage)
{
    MOS_OS_FUNCTION_ENTER;
    if (!gmmClientContext)
    {
        return {0};
    }

    if (gmmClientContext->GetCachePolicyElement(gmmUsage).Initialized)
    {
        return gmmClientContext->CachePolicyGetMemoryObject(nullptr, gmmUsage);
    }
    else if (gmmClientContext->GetCachePolicyElement(MP_RESOURCE_USAGE_DEFAULT).Initialized)
    {
        MOS_OS_NORMALMESSAGE("Cache is not initialized for GMM_RESOURCE_USAGE_TYPE %d, use MP_RESOURCE_USAGE_DEFAULT", gmmUsage);
        return gmmClientContext->CachePolicyGetMemoryObject(nullptr, MP_RESOURCE_USAGE_DEFAULT);
    }
    else
    {
        MOS_OS_NORMALMESSAGE("Cache is not initialized for GMM_RESOURCE_USAGE_TYPE %d", gmmUsage);
        return gmmClientContext->GetCachePolicyUsage()[GMM_RESOURCE_USAGE_UNKNOWN].MemoryObjectOverride;
    }
}

MEMORY_OBJECT_CONTROL_STATE MosInterface::GetCachePolicyMemoryObject(
    GMM_CLIENT_CONTEXT *gmmClientContext,
    MOS_HW_RESOURCE_DEF mosUsage)
{
    MOS_OS_FUNCTION_ENTER;

    GMM_RESOURCE_USAGE_TYPE usage = GmmResourceUsage[mosUsage];
    return MosInterface::GetGmmCachePolicyMemoryObject(gmmClientContext, usage);
}

uint8_t MosInterface::GetCachePolicyL1Config(
    MOS_STREAM_HANDLE streamState,
    MOS_HW_RESOURCE_DEF mosUsage)
{
    MOS_OS_FUNCTION_ENTER;
    return 0;
}

MOS_STATUS MosInterface::GetReservedFromResource(MOS_RESOURCE_HANDLE resource, uint32_t &val)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS MosInterface::GetReservedFromStream(MOS_STREAM_HANDLE stream, uint32_t &val)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS MosInterface::GetReservedFromDevice(MOS_DEVICE_HANDLE device, uint32_t &val)
{
    MOS_OS_CHK_NULL_RETURN(device);
    OsContextSpecificNext *osDevice = dynamic_cast<OsContextSpecificNext*>(device);
    MOS_OS_CHK_NULL_RETURN(osDevice);
    if (osDevice->GetBufMgr()->get_reserved)
    {
        val = *(osDevice->GetBufMgr()->get_reserved);
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
}

MOS_STATUS MosInterface::GetperStreamParameters(MOS_STREAM_HANDLE stream, void **perStreamParameters)
{
    MOS_OS_CHK_NULL_RETURN(stream);
    *perStreamParameters = (void*)stream->perStreamParameters;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::ConvertResourceFromDdi(
    OsSpecificRes osResource,
    MOS_RESOURCE_HANDLE &resource,
    uint32_t firstArraySlice,
    uint32_t mipSlice)
{
    MOS_OS_FUNCTION_ENTER;

    if (firstArraySlice == OS_SPECIFIC_RESOURCE_INVALID || firstArraySlice >= OS_SPECIFIC_RESOURCE_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Cannot Convert Resource From Ddi, invalid ddi resource type!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_OS_CHK_NULL_RETURN(osResource);
    MOS_OS_CHK_NULL_RETURN(resource);

    if (firstArraySlice == OS_SPECIFIC_RESOURCE_SURFACE)
    {
        DDI_MEDIA_SURFACE *mediaSurface = (DDI_MEDIA_SURFACE *)osResource;

        switch (mediaSurface->format)
        {
        case Media_Format_NV12:
            resource->Format = Format_NV12;
            break;
        case Media_Format_NV21:
            resource->Format = Format_NV21;
            break;
        case Media_Format_YUY2:
            resource->Format = Format_YUY2;
            break;
        case Media_Format_X8R8G8B8:
            resource->Format = Format_X8R8G8B8;
            break;
        case Media_Format_X8B8G8R8:
            resource->Format = Format_X8B8G8R8;
            break;
        case Media_Format_A8B8G8R8:
        case Media_Format_R8G8B8A8:
            resource->Format = Format_A8B8G8R8;
            break;
        case Media_Format_A8R8G8B8:
            resource->Format = Format_A8R8G8B8;
            break;
        case Media_Format_R5G6B5:
            resource->Format = Format_R5G6B5;
            break;
        case Media_Format_R8G8B8:
            resource->Format = Format_R8G8B8;
            break;
        case Media_Format_RGBP:
            resource->Format = Format_RGBP;
            break;
        case Media_Format_BGRP:
            resource->Format = Format_BGRP;
            break;
        case Media_Format_444P:
            resource->Format = Format_444P;
            break;
        case Media_Format_411P:
            resource->Format = Format_411P;
            break;
        case Media_Format_IMC3:
            resource->Format = Format_IMC3;
            break;
        case Media_Format_400P:
            resource->Format = Format_400P;
            break;
        case Media_Format_422H:
            resource->Format = Format_422H;
            break;
        case Media_Format_422V:
            resource->Format = Format_422V;
            break;
        case Media_Format_Buffer:
            resource->Format = Format_Any;
        case Media_Format_P010:
            resource->Format = Format_P010;
            break;
        case Media_Format_P012:
        case Media_Format_P016:
            resource->Format = Format_P016;
            break;
        case Media_Format_Y210:
            resource->Format = Format_Y210;
            break;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212:
#endif
        case Media_Format_Y216:
            resource->Format = Format_Y216;
            break;
        case Media_Format_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV:
#endif
            resource->Format = Format_AYUV;
            break;
        case Media_Format_Y410:
            resource->Format = Format_Y410;
            break;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y412:
#endif
        case Media_Format_Y416:
            resource->Format = Format_Y416;
            break;
        case Media_Format_Y8:
            resource->Format = Format_Y8;
            break;
        case Media_Format_Y16S:
            resource->Format = Format_Y16S;
            break;
        case Media_Format_Y16U:
            resource->Format = Format_Y16U;
            break;
        case Media_Format_R10G10B10A2:
        case Media_Format_R10G10B10X2:
            resource->Format = Format_R10G10B10A2;
            break;
        case Media_Format_B10G10R10A2:
        case Media_Format_B10G10R10X2:
            resource->Format = Format_B10G10R10A2;
            break;
        case Media_Format_UYVY:
            resource->Format = Format_UYVY;
            break;
        case Media_Format_VYUY:
            resource->Format = Format_VYUY;
            break;
        case Media_Format_YVYU:
            resource->Format = Format_YVYU;
            break;
        case Media_Format_A16R16G16B16:
            resource->Format = Format_A16R16G16B16;
            break;
        case Media_Format_A16B16G16R16:
            resource->Format = Format_A16B16G16R16;
            break;
        default:
            MOS_OS_ASSERTMESSAGE("MOS: unsupported media format for surface.");
            break;
        }
        resource->iWidth   = mediaSurface->iWidth;
        resource->iHeight  = mediaSurface->iHeight;
        resource->iPitch   = mediaSurface->iPitch;
        resource->iCount   = mediaSurface->iRefCount;
        resource->isTiled  = mediaSurface->isTiled;
        resource->TileType = LinuxToMosTileType(mediaSurface->TileType);
        resource->bo       = mediaSurface->bo;
        resource->name     = mediaSurface->name;

        resource->ppCurrentFrameSemaphore   = &mediaSurface->pCurrentFrameSemaphore;
        resource->ppReferenceFrameSemaphore = &mediaSurface->pReferenceFrameSemaphore;
        resource->bSemInitialized           = false;
        resource->bMapped                   = false;

        if (mediaSurface->bMapped == true)
        {
            resource->pData = mediaSurface->pData;
        }
        else
        {
            resource->pData = nullptr;
        }
        resource->pGmmResInfo  = mediaSurface->pGmmResourceInfo;
        resource->dwGfxAddress = 0;
    }
    else if (firstArraySlice == OS_SPECIFIC_RESOURCE_BUFFER)
    {
        DDI_MEDIA_BUFFER *mediaBuffer = (DDI_MEDIA_BUFFER *)osResource;
        switch (mediaBuffer->format)
        {
        case Media_Format_Buffer:
            resource->Format  = Format_Buffer;
            resource->iWidth  = mediaBuffer->iSize;
            resource->iHeight = 1;
            resource->iPitch  = mediaBuffer->iSize;
            break;
        case Media_Format_Perf_Buffer:
            resource->Format  = Format_Buffer;
            resource->iWidth  = mediaBuffer->iSize;
            resource->iHeight = 1;
            resource->iPitch  = mediaBuffer->iSize;
            break;
        case Media_Format_2DBuffer:
            resource->Format  = Format_Buffer_2D;
            resource->iWidth  = mediaBuffer->uiWidth;
            resource->iHeight = mediaBuffer->uiHeight;
            resource->iPitch  = mediaBuffer->uiPitch;
            break;
        case Media_Format_CPU:
            return MOS_STATUS_SUCCESS;
        default:
            resource->iWidth  = mediaBuffer->iSize;
            resource->iHeight = 1;
            resource->iPitch  = mediaBuffer->iSize;
            MOS_OS_ASSERTMESSAGE("MOS: unsupported media format for surface.");
            break;
        }
        resource->iCount   = mediaBuffer->iRefCount;
        resource->isTiled  = 0;
        resource->TileType = LinuxToMosTileType(mediaBuffer->TileType);
        resource->bo       = mediaBuffer->bo;
        resource->name     = mediaBuffer->name;
        resource->bMapped  = false;

        if (mediaBuffer->bMapped == true)
        {
            resource->pData = mediaBuffer->pData;
        }
        else
        {
            resource->pData = nullptr;
        }
        resource->dwGfxAddress = 0;
        resource->pGmmResInfo  = mediaBuffer->pGmmResourceInfo;
    }

    resource->bConvertedFromDDIResource     = true;
    resource->memObjCtrlState.DwordValue    = 0;
    resource->mocsMosResUsageType           = MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsSpecificResourceInfo(OsSpecificRes resource, bool isInternal)
{
    MOS_OS_FUNCTION_ENTER;

    // No OsSpecificResourceInfo in Linux

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroySpecificResourceInfo(OsSpecificRes resource)
{
    MOS_OS_FUNCTION_ENTER;

    // No OsSpecificResourceInfo in Linux

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::AllocateResource(
    MOS_STREAM_HANDLE        streamState,
    PMOS_ALLOC_GFXRES_PARAMS params,
    MOS_RESOURCE_HANDLE      &resource
#if MOS_MESSAGES_ENABLED
    ,
    const char              *functionName,
    const char              *filename,
    int32_t                 line
#endif
)
{
    MOS_STATUS estatus = MOS_STATUS_SUCCESS;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateOsApiFail(OS_FAIL_ALLOC_GFX_RES, __FUNCTION__, __FILE__, __LINE__))
    {
        return MOS_STATUS_NO_SPACE;
    }
#endif

    resource->bConvertedFromDDIResource = false;
    if (!params->bBypassMODImpl)
    {
        resource->pGfxResourceNext = GraphicsResourceNext::CreateGraphicResource(GraphicsResourceNext::osSpecificResource);
        MOS_OS_CHK_NULL_RETURN(resource->pGfxResourceNext);

        GraphicsResourceNext::CreateParams createParams(params);
        auto eStatus = resource->pGfxResourceNext->Allocate(streamState->osDeviceContext, createParams);
        MOS_OS_CHK_STATUS_MESSAGE_RETURN(eStatus, "Allocate graphic resource failed");

        eStatus = resource->pGfxResourceNext->ConvertToMosResource(resource);
        MOS_OS_CHK_STATUS_MESSAGE_RETURN(eStatus, "Convert graphic resource failed");
    }
    else
    {
        estatus = GraphicsResourceSpecificNext::AllocateExternalResource(streamState, params, resource);
        MOS_OS_CHK_STATUS_MESSAGE_RETURN(estatus, "Allocate external graphic resource failed");
    }

    MOS_OS_CHK_NULL_RETURN(resource->pGmmResInfo);
    MosUtilities::MosAtomicIncrement(&MosUtilities::m_mosMemAllocCounterGfx);

    MOS_MEMNINJA_GFX_ALLOC_MESSAGE(
        resource->pGmmResInfo,
        params->pBufName,
        streamState->component,
        (uint32_t)resource->pGmmResInfo->GetSizeSurface(),
        params->dwArraySize,
        functionName,
        filename,
        line);

    return MOS_STATUS_SUCCESS;
}

//The input bit definition in MOS_GFXRES_FREE_FLAGS
uint32_t MosInterface::ConvertHalFreeFlagsToOsFreeFlags(
    uint32_t halFreeFlag)
{
    return halFreeFlag;
}

MOS_STATUS MosInterface::FreeResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t            flag
#if MOS_MESSAGES_ENABLED
    ,
    const char          *functionName,
    const char          *filename,
    int32_t             line
#endif  // MOS_MESSAGES_ENABLED
)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    bool osContextValid = streamState->osDeviceContext->GetOsContextValid();

    bool byPassMod = !((!resource->bConvertedFromDDIResource) && (osContextValid == true) && (resource->pGfxResourceNext));

    if (!byPassMod)
    {
        if (resource && resource->pGfxResourceNext)
        {
            resource->pGfxResourceNext->Free(streamState->osDeviceContext);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Received an empty Graphics Resource, skip free");
        }
        MOS_Delete(resource->pGfxResourceNext);
        resource->pGfxResourceNext = nullptr;

        MosUtilities::MosAtomicDecrement(&MosUtilities::m_mosMemAllocCounterGfx);
        MOS_MEMNINJA_GFX_FREE_MESSAGE(resource->pGmmResInfo, functionName, filename, line);
        MosUtilities::MosZeroMemory(resource, sizeof(*resource));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS status = GraphicsResourceSpecificNext::FreeExternalResource(streamState, resource, flag);

    if (resource->pGmmResInfo != nullptr &&
        streamState->perStreamParameters != nullptr)
    {
        PMOS_CONTEXT perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
        if (perStreamParameters && perStreamParameters->pGmmClientContext)
        {
            MosUtilities::m_mosMemAllocCounterGfx--;
            MOS_MEMNINJA_GFX_FREE_MESSAGE(resource->pGmmResInfo, functionName, filename, line);

            perStreamParameters->pGmmClientContext->DestroyResInfoObject(resource->pGmmResInfo);

            resource->pGmmResInfo = nullptr;
        }
    }

    return status;
}

MOS_STATUS MosInterface::GetResourceInfo(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MosResourceInfo     &details)  //MOS_SURFACE
{
    MOS_OS_FUNCTION_ENTER;

    GMM_RESOURCE_INFO * gmmResourceInfo = nullptr;
    GMM_DISPLAY_FRAME   gmmChannel = GMM_DISPLAY_FRAME_MAX;
    GMM_REQ_OFFSET_INFO reqInfo[3] = {};
    GMM_RESOURCE_FLAG   gmmFlags = {};
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MOS_UNUSED(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);

    // Get Gmm resource info
    gmmResourceInfo = (GMM_RESOURCE_INFO*)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(gmmResourceInfo);

    gmmFlags = gmmResourceInfo->GetResFlags();

    // Get resource information
    details.dwWidth         = GFX_ULONG_CAST(gmmResourceInfo->GetBaseWidth());
    details.dwHeight        = gmmResourceInfo->GetBaseHeight();
    details.dwPitch         = GFX_ULONG_CAST(gmmResourceInfo->GetRenderPitch());
    details.dwSize          = GFX_ULONG_CAST(gmmResourceInfo->GetSizeSurface());
    details.dwDepth         = MOS_MAX(1, gmmResourceInfo->GetBaseDepth());
    details.dwLockPitch     = GFX_ULONG_CAST(gmmResourceInfo->GetRenderPitch());

    details.dwQPitch = gmmResourceInfo->GetQPitch();

    details.bCompressible   = gmmFlags.Gpu.MMC ?
        (gmmResourceInfo->GetMmcHint(0) == GMM_MMC_HINT_ON) : false;
    details.bIsCompressed   = gmmResourceInfo->IsMediaMemoryCompressed(0);
    details.CompressionMode = (MOS_RESOURCE_MMC_MODE)gmmResourceInfo->GetMmcMode(0);

    if (0 == details.dwPitch)
    {
        MOS_OS_ASSERTMESSAGE("Pitch from GmmResource is 0, unexpected.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    // check resource's tile type
    details.TileModeGMM     = (MOS_TILE_MODE_GMM)gmmResourceInfo->GetTileModeSurfaceState();
    details.bGMMTileEnabled = true;
    switch (gmmResourceInfo->GetTileType())
    {
    case GMM_TILED_Y:
          if (gmmFlags.Info.TiledYf)
          {
              details.TileType = MOS_TILE_YF;
          }
          else if (gmmFlags.Info.TiledYs)
          {
              details.TileType = MOS_TILE_YS;
          }
          else
          {
              details.TileType = MOS_TILE_Y;
          }
          break;
    case GMM_TILED_X:
          details.TileType = MOS_TILE_X;
          break;
    case GMM_NOT_TILED:
          details.TileType = MOS_TILE_LINEAR;
          break;
    default:
          details.TileType = MOS_TILE_Y;
          break;
    }
    details.Format   = resource->Format;

    // Get planes
    MosUtilities::MosZeroMemory(reqInfo, sizeof(reqInfo));
    gmmChannel = GMM_DISPLAY_BASE;
    // Get the base offset of the surface (plane Y)
    reqInfo[2].ReqRender = true;
    reqInfo[2].Plane     = GMM_PLANE_Y;
    reqInfo[2].Frame     = gmmChannel;
    reqInfo[2].CubeFace  = __GMM_NO_CUBE_MAP;
    reqInfo[2].ArrayIndex = 0;
    gmmResourceInfo->GetOffset(reqInfo[2]);
    details.RenderOffset.YUV.Y.BaseOffset = reqInfo[2].Render.Offset;
    details.RenderOffset.YUV.Y.XOffset    = reqInfo[2].Render.XOffset;
    details.RenderOffset.YUV.Y.YOffset    = reqInfo[2].Render.YOffset;
    details.LockOffset.YUV.Y              = reqInfo[2].Lock.Offset;

    // Get U/UV plane information (plane offset, X/Y offset)
    reqInfo[0].ReqRender = true;
    reqInfo[0].Plane     = GMM_PLANE_U;
    reqInfo[0].Frame     = gmmChannel;
    reqInfo[0].CubeFace  = __GMM_NO_CUBE_MAP;
    reqInfo[0].ArrayIndex = 0;
    gmmResourceInfo->GetOffset(reqInfo[0]);

    details.RenderOffset.YUV.U.BaseOffset = reqInfo[0].Render.Offset;
    details.RenderOffset.YUV.U.XOffset    = reqInfo[0].Render.XOffset;
    details.RenderOffset.YUV.U.YOffset    = reqInfo[0].Render.YOffset;
    details.LockOffset.YUV.U              = reqInfo[0].Lock.Offset;

    // Get V plane information (plane offset, X/Y offset)
    reqInfo[1].ReqRender = true;
    reqInfo[1].Plane     = GMM_PLANE_V;
    reqInfo[1].Frame     = gmmChannel;
    reqInfo[1].CubeFace  = __GMM_NO_CUBE_MAP;
    reqInfo[1].ArrayIndex = 0;
    gmmResourceInfo->GetOffset(reqInfo[1]);

    details.RenderOffset.YUV.V.BaseOffset = reqInfo[1].Render.Offset;
    details.RenderOffset.YUV.V.XOffset    = reqInfo[1].Render.XOffset;
    details.RenderOffset.YUV.V.YOffset    = reqInfo[1].Render.YOffset;
    details.LockOffset.YUV.V              = reqInfo[1].Lock.Offset;

    // Get Y plane information (plane offset, X / Y offset)
    details.dwOffset                        = details.RenderOffset.YUV.Y.BaseOffset;
    details.YPlaneOffset.iSurfaceOffset     = details.RenderOffset.YUV.Y.BaseOffset;
    details.YPlaneOffset.iXOffset           = details.RenderOffset.YUV.Y.XOffset;
    details.YPlaneOffset.iYOffset           = details.RenderOffset.YUV.Y.YOffset;
    details.YPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.Y;

    // Get U/UV plane information (plane offset, X/Y offset)
    details.UPlaneOffset.iSurfaceOffset     = details.RenderOffset.YUV.U.BaseOffset;
    details.UPlaneOffset.iXOffset           = details.RenderOffset.YUV.U.XOffset;
    details.UPlaneOffset.iYOffset           = details.RenderOffset.YUV.U.YOffset;
    details.UPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.U;

    // Get V plane information (plane offset, X/Y offset)
    details.VPlaneOffset.iSurfaceOffset     = details.RenderOffset.YUV.V.BaseOffset;
    details.VPlaneOffset.iXOffset           = details.RenderOffset.YUV.V.XOffset;
    details.VPlaneOffset.iYOffset           = details.RenderOffset.YUV.V.YOffset;
    details.VPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.V;

    details.YoffsetForUplane = (details.UPlaneOffset.iSurfaceOffset - details.dwOffset) / details.dwPitch +
                              details.UPlaneOffset.iYOffset;
    details.YoffsetForVplane = (details.VPlaneOffset.iSurfaceOffset - details.dwOffset) / details.dwPitch +
                              details.VPlaneOffset.iYOffset;

    return eStatus;
}

void *MosInterface::LockMosResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    PMOS_LOCK_PARAMS    flags)
{
    MOS_OS_FUNCTION_ENTER;

    void *pData    = nullptr;

    if (nullptr == streamState)
    {
        MOS_OS_ASSERTMESSAGE("input parameter streamState is NULL.");
        return nullptr;
    }

    if (nullptr == resource)
    {
        MOS_OS_ASSERTMESSAGE("input parameter resource is NULL.");
        return nullptr;
    }

    if ((!resource->bConvertedFromDDIResource) && (resource->pGfxResourceNext))
    {
        if (nullptr == streamState->osDeviceContext)
        {
            MOS_OS_ASSERTMESSAGE("invalid osDeviceContext, skip lock");
            return nullptr;
        }

        if (resource->pGfxResourceNext)
        {
            GraphicsResourceNext::LockParams params(flags);
            pData = resource->pGfxResourceNext->Lock(streamState->osDeviceContext, params);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Received an empty Graphics Resource, skip lock");
            return nullptr;
        }
        return pData;
    }

    pData = GraphicsResourceSpecificNext::LockExternalResource(streamState, resource, flags);
    return pData;
}

MOS_STATUS MosInterface::UnlockMosResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    if ((!resource->bConvertedFromDDIResource) && (resource->pGfxResourceNext))
    {
        if (resource->pGfxResourceNext)
        {
            eStatus = resource->pGfxResourceNext->Unlock(streamState->osDeviceContext);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Received an empty Graphics Resource, skip unlock");
        }
        return eStatus;
    }

    eStatus = GraphicsResourceSpecificNext::UnlockExternalResource(streamState, resource);

    return eStatus;
}

MOS_STATUS MosInterface::UpdateResourceUsageType(
    PMOS_RESOURCE           pOsResource,
    MOS_HW_RESOURCE_DEF     resUsageType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------
    MOS_OS_CHK_NULL_RETURN(pOsResource);
    MOS_OS_CHK_NULL_RETURN(pOsResource->pGmmResInfo);
    //---------------------------------

    pOsResource->pGmmResInfo->OverrideCachePolicyUsage(GetGmmResourceUsageType(resUsageType));

    return eStatus;
}

MOS_STATUS MosInterface::RegisterResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    bool                write)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateOsApiFail(OS_FAIL_REGISTER_GFX_RES, __FUNCTION__, __FILE__, __LINE__))
    {
        return MOS_STATUS_NO_SPACE;
    }
#endif

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->RegisterResource(resource, write));
}

uint64_t MosInterface::GetResourceGfxAddress(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);

    if (!mos_gem_bo_is_softpin(resource->bo))
    {
        mos_bo_set_softpin(resource->bo);
    }
    return resource->bo->offset64;
}

uint32_t MosInterface::GetResourceAllocationHandle(
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    if (resource && resource->bo)
    {
        return resource->bo->handle;
    }
    else
    {
        return 0;
    }
}

uint32_t MosInterface::GetResourceAllocationIndex(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

MOS_STATUS MosInterface::SkipResourceSync(
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    // No resource sync to skip

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SyncOnResource(
    MOS_STREAM_HANDLE streamState,
    MOS_RESOURCE_HANDLE resource,
    bool writeOperation,
    GPU_CONTEXT_HANDLE requsetorGpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do sync on resource

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::ResourceSyncCallback(
    OsSpecificRes       resource,
    MOS_DEVICE_HANDLE   deviceContext,
    uint32_t            index,
    SYNC_HAZARD         hazardType,
    GPU_CONTEXT_HANDLE  busyCtx,
    GPU_CONTEXT_HANDLE  requestorCtx,
    OS_HANDLE           osHandle)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do resource sync

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::LockSyncCallback(
    OsSpecificRes       resource,
    MOS_DEVICE_HANDLE   deviceContext,
    uint32_t            index,
    SYNC_HAZARD         hazardType,
    GPU_CONTEXT_HANDLE  busyCtx,
    bool                doNotWait)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do Lock sync

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::TrimResidency(
    MOS_DEVICE_HANDLE device,
    bool      periodicTrim,
    bool      restartPeriodicTrim,
    uint64_t &numBytesToTrim,
    bool      trimToMinimum,
    bool      trimOnlyMediaResources)
{
    MOS_OS_FUNCTION_ENTER;

    // No residency to trim

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS UpdateResidency(
    MOS_DEVICE_HANDLE device,
    OsSpecificRes     resInfo,
    uint32_t          index)
{
    MOS_OS_FUNCTION_ENTER;

    // No residency to update

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetMemoryCompressionMode(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MOS_MEMCOMP_STATE   resMmcMode)
{
    MOS_OS_FUNCTION_ENTER;

    PGMM_RESOURCE_INFO pGmmResourceInfo = nullptr;
    GMM_RESOURCE_MMC_INFO GmmResMmcMode = GMM_MMC_DISABLED;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    MOS_OS_CHK_NULL_RETURN(resource);

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);

    switch (resMmcMode)
    {
    case MOS_MEMCOMP_HORIZONTAL:
        GmmResMmcMode = GMM_MMC_HORIZONTAL;
        break;
    case MOS_MEMCOMP_VERTICAL:
        GmmResMmcMode = GMM_MMC_VERTICAL;
        break;
    case MOS_MEMCOMP_DISABLED:
    default:
        GmmResMmcMode = GMM_MMC_DISABLED;
        break;
    }

    pGmmResourceInfo->SetMmcMode(GmmResMmcMode, 0);

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::GetMemoryCompressionMode(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MOS_MEMCOMP_STATE  &resMmcMode)
{
    MOS_OS_FUNCTION_ENTER;

    PGMM_RESOURCE_INFO gmmResourceInfo = nullptr;
    GMM_RESOURCE_FLAG  flags;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    MOS_OS_CHK_NULL_RETURN(resource);
    MosUtilities::MosZeroMemory(&flags, sizeof(GMM_RESOURCE_FLAG));

    // Get Gmm resource info
    gmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(gmmResourceInfo);

    flags = resource->pGmmResInfo->GetResFlags();

    if (!flags.Gpu.MMC || !flags.Gpu.CCS)
    {
        resMmcMode = MOS_MEMCOMP_DISABLED;
        return MOS_STATUS_SUCCESS;
    }

    if (flags.Info.MediaCompressed || flags.Info.RenderCompressed)
    {
        resMmcMode = flags.Info.RenderCompressed ? MOS_MEMCOMP_RC : MOS_MEMCOMP_MC;
    }
    else
    {
        switch (gmmResourceInfo->GetMmcMode(0))
        {
        case GMM_MMC_HORIZONTAL:
            resMmcMode = MOS_MEMCOMP_HORIZONTAL;
            break;
        case GMM_MMC_VERTICAL:
            resMmcMode = MOS_MEMCOMP_VERTICAL;
            break;
        case GMM_MMC_DISABLED:
        default:
            resMmcMode = MOS_MEMCOMP_DISABLED;
            break;
        }
    }

    uint32_t          MmcFormat = 0;
    GMM_RESOURCE_FORMAT gmmResFmt;
    gmmResFmt = gmmResourceInfo->GetResourceFormat();
    auto skuTable = GetSkuTable(streamState);
    MOS_OS_CHK_NULL_RETURN(MosInterface::GetGmmClientContext(streamState));
    MOS_OS_CHK_NULL_RETURN(skuTable);

    if (resMmcMode == MOS_MEMCOMP_MC         &&
       (!MEDIA_IS_SKU(skuTable, FtrFlatPhysCCS)))
    {
        MmcFormat = static_cast<uint32_t>(MosInterface::GetGmmClientContext(streamState)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));
        resMmcMode = (MmcFormat != 0) ? resMmcMode : MOS_MEMCOMP_DISABLED;
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::SetMemoryCompressionHint(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    bool                hintOn)
{
    MOS_OS_FUNCTION_ENTER;

    PGMM_RESOURCE_INFO pGmmResourceInfo = nullptr;
    uint32_t uiArrayIndex = 0;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CHK_NULL_RETURN(resource);

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);

    pGmmResourceInfo->SetMmcHint(hintOn ? GMM_MMC_HINT_ON : GMM_MMC_HINT_OFF, uiArrayIndex);

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::GetMemoryCompressionFormat(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t *          resMmcFormat)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS         eStatus = MOS_STATUS_UNKNOWN;
    PGMM_RESOURCE_INFO pGmmResourceInfo;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(resMmcFormat);

    pGmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);
    MOS_OS_CHK_NULL_RETURN(MosInterface::GetGmmClientContext(streamState));
    // Get compression format from GMM RESOURCE FORMAT
    GMM_RESOURCE_FORMAT gmmResFmt;
    gmmResFmt = pGmmResourceInfo->GetResourceFormat();
    MOS_MEMCOMP_STATE MmcMode   = MOS_MEMCOMP_DISABLED;
    uint32_t          MmcFormat = 0;
    MosInterface::GetMemoryCompressionMode(streamState, resource, MmcMode);
    switch (MmcMode)
    {
    case MOS_MEMCOMP_MC:
         MmcFormat = static_cast<uint32_t>(MosInterface::GetGmmClientContext(streamState)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));
         break;
    case MOS_MEMCOMP_RC:
         MmcFormat = static_cast<uint32_t>(MosInterface::GetGmmClientContext(streamState)->GetSurfaceStateCompressionFormat(gmmResFmt));
         break;
    default:
          MmcFormat = 0;
    }

    if (MmcFormat > 0x1F)
    {
        MOS_OS_ASSERTMESSAGE("Get a incorrect Compression format(%d) from GMM", MmcFormat);
    }
    else
    {
        *resMmcFormat = MmcFormat;
        MOS_OS_VERBOSEMESSAGE("GMM compression mode %d, compression format %d", MmcMode, MmcFormat);
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::DoubleBufferCopyResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    bool                outputCompressed)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(inputResource);
    MOS_OS_CHK_NULL_RETURN(outputResource);
    MOS_OS_CHK_NULL_RETURN(streamState);

    if (inputResource && inputResource->bo && inputResource->pGmmResInfo &&
        outputResource && outputResource->bo && outputResource->pGmmResInfo)
    {
        OsContextNext *osCtx = streamState->osDeviceContext;
        MOS_OS_CHK_NULL_RETURN(osCtx);

        MosDecompression *mosDecompression = osCtx->GetMosDecompression();
        MOS_OS_CHK_NULL_RETURN(mosDecompression);

        // Double Buffer Copy can support any tile status surface with/without compression
        mosDecompression->MediaMemoryCopy(inputResource, outputResource, outputCompressed);
    }

    return status;
}

MOS_STATUS MosInterface::MediaCopyResource2D(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    uint32_t            copyWidth,
    uint32_t            copyHeight,
    uint32_t            copyInputOffset,
    uint32_t            copyOutputOffset,
    uint32_t            bpp,
    bool                outputCompressed)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(inputResource);
    MOS_OS_CHK_NULL_RETURN(outputResource);
    MOS_OS_CHK_NULL_RETURN(streamState);

    if (inputResource && inputResource->bo && inputResource->pGmmResInfo &&
        outputResource && outputResource->bo && outputResource->pGmmResInfo)
    {
        OsContextNext *osCtx = streamState->osDeviceContext;
        MOS_OS_CHK_NULL_RETURN(osCtx);

        MosDecompression *mosDecompression = osCtx->GetMosDecompression();
        MOS_OS_CHK_NULL_RETURN(mosDecompression);

        // Double Buffer Copy can support any tile status surface with/without compression
        mosDecompression->MediaMemoryCopy2D(inputResource, outputResource,
            copyWidth, copyHeight, copyInputOffset, copyOutputOffset, bpp, outputCompressed);
    }

    return status;
}

MOS_STATUS MosInterface::DecompResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(resource->bo);
    MOS_OS_CHK_NULL_RETURN(resource->pGmmResInfo);

    MOS_LINUX_BO *bo = resource->bo;
    if (resource->pGmmResInfo->IsMediaMemoryCompressed(0))
    {
        OsContextNext *osCtx = streamState->osDeviceContext;
        MOS_OS_CHK_NULL_RETURN(osCtx);

        MosDecompression *mosDecompression = osCtx->GetMosDecompression();
        MOS_OS_CHK_NULL_RETURN(mosDecompression);

        mosDecompression->MemoryDecompress(resource);
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t MosInterface::GetGpuStatusTag(
        MOS_STREAM_HANDLE  streamState,
        GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState)
    {
        auto gpuContextIns = MosInterface::GetGpuContext(streamState, gpuContext);
        if (gpuContextIns == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Get GPU Status Tag failed.");
            return 0;
        }

        return gpuContextIns->GetGpuStatusTag();
    }
    MOS_OS_ASSERTMESSAGE("Get GPU Status Tag failed.");

    return 0;
}

MOS_STATUS MosInterface::IncrementGpuStatusTag(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    auto gpuContextIns = MosInterface::GetGpuContext(streamState, gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextIns);

    gpuContextIns->IncrementGpuStatusTag();

    return MOS_STATUS_SUCCESS;
}

uint64_t MosInterface::GetGpuStatusSyncTag(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    // No Gpu Status Sync Tag in Linux

    return 0;
}

MOS_STATUS MosInterface::GetGpuStatusBufferResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE &resource,
    GPU_CONTEXT_HANDLE  gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContextIns = MosInterface::GetGpuContext(streamState, gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextIns);

    resource = gpuContextIns->GetStatusBufferResource();

    return MOS_STATUS_SUCCESS;
}

GMM_CLIENT_CONTEXT *MosInterface::GetGmmClientContext(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetGmmClientContext();
    }

    return nullptr;
}

void MosInterface::GetGpuPriority(MOS_STREAM_HANDLE streamState, int32_t* pPriority)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set the gpu priority");
        return;
    }

    PMOS_OS_CONTEXT pOsContext = (PMOS_OS_CONTEXT)streamState->perStreamParameters;
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set the gpu priority");
        return;
    }

    uint64_t priority = 0;
    mos_get_context_param(pOsContext->intel_context, 0, I915_CONTEXT_PARAM_PRIORITY, &priority);
    *pPriority = (int32_t)priority;
}

void MosInterface::SetGpuPriority(MOS_STREAM_HANDLE streamState, int32_t priority)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set the gpu priority");
        return;
    }

    if (streamState->ctxPriority == priority)
        return;

    PMOS_OS_CONTEXT pOsContext = (PMOS_OS_CONTEXT)streamState->perStreamParameters;
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set the gpu priority");
        return;
    }

    int32_t ret = mos_set_context_param(pOsContext->intel_context, 0, I915_CONTEXT_PARAM_PRIORITY,(uint64_t)priority);
    if (ret != 0)
    {
        MOS_OS_ASSERTMESSAGE("failed to set the gpu priority, error is %d", ret);
    }

    streamState->ctxPriority = priority;
}

uint64_t MosInterface::GetAuxTableBaseAddr(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

MosCpInterface *MosInterface::GetCpInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return streamState ? streamState->osCpInterface : nullptr;
}

MosOcaInterface *MosInterface::GetOcaInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MOS_VE_HANDLE MosInterface::GetVirtualEngineState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return streamState ? streamState->virtualEngineInterface : nullptr;
}

MOS_STATUS MosInterface::SetVirtualEngineState(
    MOS_STREAM_HANDLE streamState,
    MOS_VE_HANDLE veState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(veState);

    streamState->virtualEngineInterface = veState;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateVirtualEngineState(
    MOS_STREAM_HANDLE streamState,
    PMOS_VIRTUALENGINE_INIT_PARAMS veInitParms,
    MOS_VE_HANDLE& veState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    if (veInitParms->bScalabilitySupported)
    {
        streamState->virtualEngineInterface = MOS_New(MosOsVeScalabilitySpecific);
    }
    else
    {
        streamState->virtualEngineInterface = MOS_New(MosOsVeSinglePipeSpecific);
    }
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    MOS_OS_CHK_STATUS_RETURN(streamState->virtualEngineInterface->Initialize(streamState, veInitParms));

    veState = streamState->virtualEngineInterface;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyVirtualEngineState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    if (streamState->virtualEngineInterface)
    {
        streamState->virtualEngineInterface->Destroy();
        MOS_Delete(streamState->virtualEngineInterface);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetVeHintParams(
    MOS_STREAM_HANDLE             streamState,
    PMOS_VIRTUALENGINE_SET_PARAMS veParams)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    return streamState->virtualEngineInterface->SetHintParams(veParams);
}

MOS_STATUS MosInterface::GetVeHintParams(
    MOS_STREAM_HANDLE               streamState,
    bool                            scalableMode,
    PMOS_VIRTUALENGINE_HINT_PARAMS* hintParams)
{
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);
    MOS_OS_CHK_NULL_RETURN(hintParams);

    return streamState->virtualEngineInterface->GetHintParams(scalableMode, hintParams);
}

MOS_STATUS MosInterface::SetVeSubmissionType(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuf,
    MOS_SUBMISSION_TYPE   type)
{
    MOS_OS_CHK_NULL_RETURN(cmdBuf);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    return streamState->virtualEngineInterface->SetSubmissionType(cmdBuf, type);
}

#if _DEBUG || _RELEASE_INTERNAL

uint8_t MosInterface::GetVeEngineCount(
    MOS_STREAM_HANDLE streamState)
{
    return streamState && streamState->virtualEngineInterface ?
        streamState->virtualEngineInterface->GetEngineCount() : 0;
}

uint8_t MosInterface::GetEngineLogicId(
    MOS_STREAM_HANDLE streamState,
    uint32_t instanceIdx)
{
    return streamState && streamState->virtualEngineInterface ?
        streamState->virtualEngineInterface->GetEngineLogicId(instanceIdx) : 0;
}

#endif  // _DEBUG || _RELEASE_INTERNAL
MOS_STATUS MosInterface::ComposeCommandBufferHeader(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    // No Command buffer header to compose

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::MosLoadLibrary(
    MOS_STREAM_HANDLE           streamState,
    PCCHAR                      pFileName,
    PHMODULE                    ppvModule)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_UNUSED(streamState);
    MOS_OS_ASSERT(pFileName);
    return MosUtilities::MosLoadLibrary(pFileName, ppvModule);
}

MOS_STATUS MosInterface::MosFreeLibrary(HMODULE hLibModule)
{
    MOS_OS_ASSERT(hLibModule);

    uint32_t ret = MosUtilities::MosFreeLibrary(hLibModule);

    return (ret == true) ? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}

GpuContextSpecificNext *MosInterface::GetGpuContext(MOS_STREAM_HANDLE streamState, GPU_CONTEXT_HANDLE handle)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        auto osDeviceContext = streamState->osDeviceContext;

        auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
        if (gpuContextMgr)
        {
            GpuContextNext *gpuCtx = gpuContextMgr->GetGpuContext(handle);

            return static_cast<GpuContextSpecificNext *>(gpuCtx);
        }
    }

    MOS_OS_ASSERTMESSAGE("GetGpuContext failed!");
    return nullptr;
}

void MosInterface::SetPerfTag(MOS_STREAM_HANDLE streamState, uint32_t perfTag)
{
    MOS_OS_FUNCTION_ENTER;

    uint32_t     componentTag;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState->perStreamParameters);

    auto osParameters   = (PMOS_CONTEXT)streamState->perStreamParameters;
    PERF_DATA *perfData = osParameters->pPerfData;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(perfData);

    switch (streamState->component)
    {
    case COMPONENT_VPreP:
    case COMPONENT_VPCommon:
        componentTag = PERFTAG_VPREP;
        break;

    case COMPONENT_LibVA:
        componentTag = PERFTAG_LIBVA;
        break;

    case COMPONENT_CM:
        componentTag = PERFTAG_CM;
        break;

    case COMPONENT_Decode:
        componentTag = PERFTAG_DECODE;
        break;

    case COMPONENT_Encode:
        componentTag = PERFTAG_ENCODE;
        break;

    default:
        componentTag = 0xF000 & perfData->dmaBufID;
        break;
    }

    perfData->dmaBufID = componentTag | (perfTag & 0x0fff);

    return;
}

int32_t MosInterface::IsPerfTagSet(MOS_STREAM_HANDLE streamState)
{
    uint32_t                 componentTag   = 0;
    int32_t                  ret            = false;

    MOS_OS_FUNCTION_ENTER;

    if (streamState == nullptr ||
        streamState->perStreamParameters == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("streamState or perStreamParameters invalid nullptr");
        return false;
    }

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    PERF_DATA *perfData     = osParameters->pPerfData;
    if (perfData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("perfData invalid nullptr");
        return false;
    }

    componentTag = 0xF000 & perfData->dmaBufID;

    switch (componentTag)
    {
    case PERFTAG_ENCODE:
    case PERFTAG_DECODE:
        ret = true;
        break;

    default:
        ret = false;
        break;
    }

    return ret;
}

void MosInterface::IncPerfFrameID(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState->perStreamParameters);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osParameters->pPerfData);

    osParameters->pPerfData->frameID++;

    return;
}

uint32_t MosInterface::GetPerfTag(MOS_STREAM_HANDLE streamState)
{
    uint32_t perfTag;

    MOS_OS_FUNCTION_ENTER;

    if (streamState == nullptr ||
        streamState->perStreamParameters == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("streamState or perStreamParameters invalid nullptr");
        return 0;
    }

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    perfTag = *(uint32_t *)(osParameters->pPerfData);
    return perfTag;
}

void MosInterface::SetPerfHybridKernelID(
    MOS_STREAM_HANDLE streamState,
    uint32_t          kernelID)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState->perStreamParameters);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    PERF_DATA *perfData     = osParameters->pPerfData;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(perfData);

    perfData->dmaBufID = (perfData->dmaBufID & 0xF0FF) | ((kernelID << 8) & 0x0F00);

    return;
}

void MosInterface::ResetPerfBufferID(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState->perStreamParameters);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osParameters->pPerfData);

    osParameters->pPerfData->bufferID = 0;

    return;
}

void MosInterface::IncPerfBufferID(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(streamState->perStreamParameters);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osParameters->pPerfData);

    osParameters->pPerfData->bufferID++;

    return;
}

int32_t MosInterface::IsGPUHung(
    MOS_STREAM_HANDLE streamState)
{
    uint32_t     resetCount   = 0;
    uint32_t     activeBatch  = 0;
    uint32_t     pendingBatch = 0;
    int32_t      result       = false;
    int32_t      ret          = 0;
    PMOS_CONTEXT osParameters = nullptr;

    MOS_OS_FUNCTION_ENTER;

    if (!streamState || !streamState->perStreamParameters)
    {
        MOS_OS_ASSERTMESSAGE("invalid streamstate");
        return false;
    }

    osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    ret = mos_get_reset_stats(osParameters->intel_context, &resetCount, &activeBatch, &pendingBatch);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        return false;
    }

    if (resetCount      != streamState->gpuResetCount ||
        activeBatch     != streamState->gpuActiveBatch ||
        pendingBatch    != streamState->gpuPendingBatch)
    {
        streamState->gpuResetCount    = resetCount;
        streamState->gpuActiveBatch   = activeBatch;
        streamState->gpuPendingBatch  = pendingBatch;
        result                        = true;
    }
    else
    {
        result = false;
    }
    return result;
}

bool MosInterface::IsSetMarkerEnabled(
    MOS_STREAM_HANDLE streamState)
{
    return false;
}

PMOS_RESOURCE MosInterface::GetMarkerResource(
    MOS_STREAM_HANDLE   streamState)
{
    return nullptr;
}

int MosInterface::GetPlaneSurfaceOffset(const MOS_PLANE_OFFSET &planeOffset)
{
    return planeOffset.iSurfaceOffset;
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
MOS_STATUS MosInterface::DumpCommandBufferInit(
    MOS_STREAM_HANDLE streamState)
{
    char sFileName[MOS_MAX_HLT_FILENAME_LEN] = {0};
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData = {0};
    char *psFileNameAfterPrefix = nullptr;
    size_t nSizeFileNamePrefix = 0;

    MOS_OS_CHK_NULL_RETURN(streamState);

    // Check if command buffer dump was enabled in user feature.
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE_ID,
        &UserFeatureData,
        (MOS_CONTEXT_HANDLE)streamState->perStreamParameters);
    streamState->dumpCommandBuffer            = (UserFeatureData.i32Data != 0);
    streamState->dumpCommandBufferToFile      = ((UserFeatureData.i32Data & 1) != 0);
    streamState->dumpCommandBufferAsMessages  = ((UserFeatureData.i32Data & 2) != 0);

    if (streamState->dumpCommandBufferToFile)
    {
        // Create output directory.
        eStatus = MosUtilDebug::MosLogFileNamePrefix(streamState->sDirName, nullptr);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create log file prefix. Status = %d", eStatus);
            return eStatus;
        }

        memcpy(sFileName, streamState->sDirName, MOS_MAX_HLT_FILENAME_LEN);
        nSizeFileNamePrefix = strnlen(sFileName, sizeof(sFileName));
        MOS_SecureStringPrint(
            sFileName + nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            "%c%s",
            MOS_DIR_SEPERATOR,
            MOS_COMMAND_BUFFER_OUT_DIR);

        eStatus = MosUtilities::MosCreateDirectory(sFileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
            return eStatus;
        }
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)

uint32_t MosInterface::m_mosOsApiFailSimulateType         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateMode         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateFreq         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateHint         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateCounter      = 0;

void MosInterface::MosInitOsApiFailSimulateFlag(MOS_CONTEXT_HANDLE mosCtx)
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureValueData;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    //default off for simulate random fail
    m_mosOsApiFailSimulateType         = OS_API_FAIL_TYPE_NONE;
    m_mosOsApiFailSimulateMode         = OS_API_FAIL_SIMULATE_MODE_DEFAULT;
    m_mosOsApiFailSimulateFreq         = 0;
    m_mosOsApiFailSimulateHint         = 0;
    m_mosOsApiFailSimulateCounter      = 0;

    // Read Config : memory allocation failure simulate mode
    MosUtilities::MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_TYPE_ID,
        &userFeatureValueData,
        mosCtx);

    if (userFeatureValueData.u32Data & OS_API_FAIL_TYPE_MAX)
    {
        m_mosOsApiFailSimulateType = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init MosSimulateOsApiFailSimulateType as %d \n ", m_mosOsApiFailSimulateType);
    }
    else
    {
        m_mosOsApiFailSimulateType = OS_API_FAIL_TYPE_NONE;
        MOS_OS_NORMALMESSAGE("Invalid OS API Fail Simulate Type from config: %d \n ", userFeatureValueData.u32Data);
    }

    // Read Config : memory allocation failure simulate mode
    MosUtilities::MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_MODE_ID,
        &userFeatureValueData,
        mosCtx);

    if ((userFeatureValueData.u32Data == OS_API_FAIL_SIMULATE_MODE_DEFAULT) ||
        (userFeatureValueData.u32Data == OS_API_FAIL_SIMULATE_MODE_RANDOM) ||
        (userFeatureValueData.u32Data == OS_API_FAIL_SIMULATE_MODE_TRAVERSE))
    {
        m_mosOsApiFailSimulateMode = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init MosSimulateOsApiFailSimulateMode as %d \n ", m_mosOsApiFailSimulateMode);
    }
    else
    {
        m_mosOsApiFailSimulateMode = OS_API_FAIL_SIMULATE_MODE_DEFAULT;
        MOS_OS_NORMALMESSAGE("Invalid OS API Fail Simulate Mode from config: %d \n ", userFeatureValueData.u32Data);
    }

    // Read Config : memory allocation failure simulate frequence
    MosUtilities::MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_FREQ_ID,
        &userFeatureValueData,
        mosCtx);

    if ((userFeatureValueData.u32Data >= MIN_OS_API_FAIL_FREQ) &&
        (userFeatureValueData.u32Data <= MAX_OS_API_FAIL_FREQ))
    {
        m_mosOsApiFailSimulateFreq = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init m_MosSimulateRandomOsApiFailFreq as %d \n ", m_mosOsApiFailSimulateFreq);

        if (m_mosOsApiFailSimulateMode == OS_API_FAIL_SIMULATE_MODE_RANDOM)
        {
            srand((unsigned int)time(nullptr));
        }
    }
    else
    {
        m_mosOsApiFailSimulateFreq = 0;
        MOS_OS_NORMALMESSAGE("Invalid OS API Fail Simulate Freq from config: %d \n ", userFeatureValueData.u32Data);
    }

    // Read Config : memory allocation failure simulate counter
    MosUtilities::MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUtilities::MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_HINT_ID,
        &userFeatureValueData,
        mosCtx);

    if (userFeatureValueData.u32Data <= m_mosOsApiFailSimulateFreq)
    {
        m_mosOsApiFailSimulateHint = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init m_MosOsApiFailSimulateHint as %d \n ", m_mosOsApiFailSimulateHint);
    }
    else
    {
        m_mosOsApiFailSimulateHint = m_mosOsApiFailSimulateFreq;
        MOS_OS_NORMALMESSAGE("Set m_mosOsApiFailSimulateHint as %d since INVALID CONFIG %d \n ", m_mosOsApiFailSimulateHint, userFeatureValueData.u32Data);
    }
}

void MosInterface::MosDeinitOsApiFailSimulateFlag()
{
    //default off for simulate fail
    m_mosOsApiFailSimulateType    = OS_API_FAIL_TYPE_NONE;
    m_mosOsApiFailSimulateMode    = OS_API_FAIL_SIMULATE_MODE_DEFAULT;
    m_mosOsApiFailSimulateFreq    = 0;
    m_mosOsApiFailSimulateHint    = 0;
    m_mosOsApiFailSimulateCounter = 0;
}

bool MosInterface::MosSimulateOsApiFail(
    OS_API_FAIL_TYPE type,
    const char *functionName,
    const char *filename,
    int32_t     line)
{
    bool bSimulateOsApiFail = false;

    if (!MosOsApiFailSimulationEnabled(type))
    {
        return false;
    }

    if (m_mosOsApiFailSimulateMode == OS_API_FAIL_SIMULATE_MODE_RANDOM)
    {
        int32_t Rn = rand();
        m_mosOsApiFailSimulateCounter++;
        if (Rn % m_mosOsApiFailSimulateFreq == 1)
        {
            bSimulateOsApiFail = true;
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF,
                "Simulated OS API(Type %d) Fail (Rn=%d, SimulateAllocCounter=%d) for: functionName: %s, filename: %s, line: %d\n",
                m_mosOsApiFailSimulateType, Rn, m_mosOsApiFailSimulateCounter, functionName, filename, line);
        }
        else
        {
            bSimulateOsApiFail = false;
        }
    }
    else if (m_mosOsApiFailSimulateMode == OS_API_FAIL_SIMULATE_MODE_TRAVERSE)
    {
        if (m_mosOsApiFailSimulateCounter++ == m_mosOsApiFailSimulateHint)
        {
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF,
                "Simulated OS API(Type %d) Fail (hint=%d) for: functionName: %s, filename: %s, line: %d\n",
                m_mosOsApiFailSimulateType, m_mosOsApiFailSimulateHint, functionName, filename, line);
            bSimulateOsApiFail = true;
        }
        else
        {
            bSimulateOsApiFail = false;
        }
    }
    else
    {
        //MOS_OS_NORMALMESSAGE("Invalid m_mosOsApiFailSimulateMode: %d \n ", m_mosOsApiFailSimulateMode);
        bSimulateOsApiFail = false;
    }

    return bSimulateOsApiFail;
}
#endif  // #if (_DEBUG || _RELEASE_INTERNAL)

bool MosInterface::IsAsyncDevice(MOS_STREAM_HANDLE streamState)
{
    return false;
}
