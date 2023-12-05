/*
* Copyright (c) 2009-2023, Intel Corporation
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
#include "mos_os_cp_interface_specific.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontext_specific_next.h"
#include "media_libva_common.h"
#include "mos_auxtable_mgr.h"
#include "mos_os_virtualengine_singlepipe_specific_next.h"
#include "mos_os_virtualengine_scalability_specific_next.h"
#include "mos_graphicsresource_specific_next.h"
#include "mos_bufmgr_priv.h"
#include "drm_device.h"
#include "media_fourcc.h"
#include "mos_oca_rtlog_mgr.h"

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
    MediaUserSettingSharedPtr   userSettingPtr = MosGetUserSettingInstance((PMOS_CONTEXT)ddiDeviceContext);

    MosUtilities::MosUtilitiesInit(userSettingPtr);

    // MOS_OS_FUNCTION_ENTER need mos utilities init
    MOS_OS_FUNCTION_ENTER;

#if (_DEBUG || _RELEASE_INTERNAL)
    //Init MOS OS API fail simulate flags
    MosInitOsApiFailSimulateFlag(userSettingPtr);
#endif

    //Read user feature key here for Per Utility Tool Enabling
    if (!g_perfutility->bPerfUtilityKey)
    {
        g_perfutility->dwPerfUtilityIsEnabled = 0;
        ReadUserSetting(
            userSettingPtr,
            g_perfutility->dwPerfUtilityIsEnabled,
            __MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE,
            MediaUserSetting::Group::Device);


        MOS_STATUS                  eStatus_Perf = MOS_STATUS_SUCCESS;
        std::string                 perfOutputDir = "";

        eStatus_Perf = ReadUserSetting(
            userSettingPtr,
            perfOutputDir,
            __MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY,
            MediaUserSetting::Group::Device);
        if (eStatus_Perf == MOS_STATUS_SUCCESS)
        {
            g_perfutility->setupFilePath(perfOutputDir.c_str());
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

    MediaUserSettingSharedPtr userSettingPtr = MosInterface::MosGetUserSettingInstance(mosCtx);

    // Close MOS utlities
    MosUtilities::MosUtilitiesClose(userSettingPtr);

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
    MOS_STATUS                  eStatusUserFeature = MOS_STATUS_SUCCESS;
    uint32_t                    regValue = 0;
    MediaUserSettingSharedPtr   userSettingPtr = nullptr;

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

    userSettingPtr = MosInterface::MosGetUserSettingInstance((PMOS_CONTEXT)extraParams);

#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        userSettingPtr,
        (*streamState)->simIsActive,
        __MEDIA_USER_FEATURE_VALUE_SIM_ENABLE,
        MediaUserSetting::Group::Device);

    // Null HW Driver
    // 0: Disabled
    ReadUserSettingForDebug(
        userSettingPtr,
        (*streamState)->nullHwAccelerationEnable.Value,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE,
        MediaUserSetting::Group::Device);

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
        (*streamState)->bParallelSubmission = true;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // read the "Force VDBOX" user feature key
    // 0: not force
    ReadUserSettingForDebug(
        userSettingPtr,
        (*streamState)->eForceVdbox,
        __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX,
        MediaUserSetting::Group::Device);

    //Read Scalable/Legacy Decode mode on Gen11+
    //1:by default for scalable decode mode
    //0:for legacy decode mode
    regValue = 0;
    eStatusUserFeature = ReadUserSetting(
        userSettingPtr,
        regValue,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_HCP_SCALABILITY_DECODE,
        MediaUserSetting::Group::Device);

    (*streamState)->hcpDecScalabilityMode = regValue ? MOS_SCALABILITY_ENABLE_MODE_DEFAULT : MOS_SCALABILITY_ENABLE_MODE_FALSE;
    if((*streamState)->hcpDecScalabilityMode
        && (eStatusUserFeature == MOS_STATUS_SUCCESS))
    {
        //user's value to enable scalability
        (*streamState)->hcpDecScalabilityMode = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE;
    }

    (*streamState)->frameSplit = false;
    ReadUserSettingForDebug(
        userSettingPtr,
        (*streamState)->frameSplit,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_LINUX_FRAME_SPLIT,
        MediaUserSetting::Group::Device);

    regValue = 0;
    ReadUserSettingForDebug(
        userSettingPtr,
        regValue,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_GUC_SUBMISSION,
        MediaUserSetting::Group::Device);
    (*streamState)->bParallelSubmission = (*streamState)->bParallelSubmission && regValue;

    //KMD Virtual Engine DebugOverride
    // 0: not Override
    ReadUserSettingForDebug(
        userSettingPtr,
        (*streamState)->enableDbgOvrdInVirtualEngine,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VE_DEBUG_OVERRIDE,
        MediaUserSetting::Group::Device);
#endif

    if (component == COMPONENT_VPCommon ||
        component == COMPONENT_VPreP    ||
        component == COMPONENT_LibVA)
    {
        // UMD Vebox Virtual Engine Scalability Mode
        // 0: disable. can set to 1 only when KMD VE is enabled.
        regValue = 0;
        eStatusUserFeature = ReadUserSetting(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE,
            MediaUserSetting::Group::Device);
        (*streamState)->veboxScalabilityMode = regValue ? MOS_SCALABILITY_ENABLE_MODE_DEFAULT : MOS_SCALABILITY_ENABLE_MODE_FALSE;

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
        regValue = 0;
        ReadUserSettingForDebug(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_FORCE_VEBOX,
            MediaUserSetting::Group::Device);
        (*streamState)->eForceVebox = (MOS_FORCE_VEBOX)regValue;
#endif
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE,
        (*streamState)->simIsActive,
        MediaUserSetting::Group::Device);
#endif

    MOS_OS_CHK_STATUS_RETURN(MosInterface::InitStreamParameters(*streamState, extraParams));

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    DumpCommandBufferInit(*streamState);
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsStreamState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    if (streamState->mosDecompression)
    {
        MOS_Delete(streamState->mosDecompression);
        streamState->mosDecompression = nullptr;
    }

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

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    MOS_OS_CHK_NULL_RETURN(extraParams);

    osDeviceContext = (OsContextSpecificNext *)streamState->osDeviceContext;
    fd              = osDeviceContext->GetFd();
    if (0 > fd)
    {
        MOS_OS_ASSERTMESSAGE("Invalid fd");
        return MOS_STATUS_INVALID_HANDLE;
    }

    bufMgr = osDeviceContext->GetBufMgr();
    MOS_OS_CHK_NULL_RETURN(bufMgr);

    context = MOS_New(MOS_OS_CONTEXT);
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

    context->m_userSettingPtr   = ((PMOS_CONTEXT)extraParams)->m_userSettingPtr;
    context->m_auxTableMgr      = osDeviceContext->GetAuxTableMgr();

    mos_bufmgr_enable_reuse(bufMgr);

    context->m_skuTable            = *osDeviceContext->GetSkuTable();
    context->m_waTable             = *osDeviceContext->GetWaTable();
    context->m_gtSystemInfo        = *osDeviceContext->GetGtSysInfo();
    context->m_platform            = *osDeviceContext->GetPlatformInfo();
    context->m_protectedGEMContext = ((PMOS_CONTEXT)extraParams)->m_protectedGEMContext;

    context->bUse64BitRelocs    = true;
    context->bUseSwSwizzling    = context->bSimIsActive || MEDIA_IS_SKU(&context->m_skuTable, FtrUseSwSwizzling);
    context->bTileYFlag         = MEDIA_IS_SKU(&context->m_skuTable, FtrTileY);

    if (MEDIA_IS_SKU(&context->m_skuTable, FtrContextBasedScheduling))
    {
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_START,
                          &eStatus, sizeof(eStatus), nullptr, 0);
        context->intel_context = mos_context_create_ext(context->bufmgr, 0, context->m_protectedGEMContext);
        MOS_OS_CHK_NULL_RETURN(context->intel_context);
        context->intel_context->vm_id = mos_vm_create(context->bufmgr);
        if (context->intel_context->vm_id == INVALID_VM)
        {
            MOS_OS_ASSERTMESSAGE("Failed to create vm.\n");
            return MOS_STATUS_UNKNOWN;
        }
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_END,
                          &context->intel_context, sizeof(void *),
                          &eStatus, sizeof(eStatus));
    }
    else  //use legacy context create ioctl for pre-gen11 platforms
    {
        MOS_OS_ASSERTMESSAGE("Do not support the legacy context creation.\n");
        MOS_FreeMemAndSetNull(context->pPerfData);
        MOS_Delete(context);
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

    streamState->enableDecomp     = true;

#ifndef ANDROID
    {
        context->bKMDHasVCS2 = mos_has_bsd2(context->bufmgr);
    }
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
    // read "Linux PerformanceTag Enable" user feature key
    uint32_t regValue = 0;
    ReadUserSettingForDebug(
        context->m_userSettingPtr,
        regValue,
        __MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE,
        MediaUserSetting::Group::Device);

    context->uEnablePerfTag = regValue;
#endif

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

        if (mos_hweight8(osParameters->intel_context, sseu.subslice_mask) > createOption.packed.SubSliceCount)
        {
            sseu.subslice_mask = mos_switch_off_n_bits(osParameters->intel_context, sseu.subslice_mask,
                mos_hweight8(osParameters->intel_context, sseu.subslice_mask) - createOption.packed.SubSliceCount);
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

MOS_STATUS MosInterface:: SetObjectCapture(
    PMOS_RESOURCE osResource)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    MOS_OS_CHK_NULL_RETURN(osResource);
    MOS_OS_CHK_NULL_RETURN(osResource->bo);
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
        if((eStatus = MosUtilities::MosWriteFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten)) != MOS_STATUS_SUCCESS)
        {
            MOS_FreeMemory(pOutputBuffer);
            MOS_OS_CHK_STATUS_RETURN(eStatus);
        }
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
                if((eStatus = MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten)) != MOS_STATUS_SUCCESS)
                {
                    MOS_FreeMemory(pOutputBuffer);
                    MOS_OS_CHK_STATUS_RETURN(eStatus);
                }
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
        if((eStatus = MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten)) != MOS_STATUS_SUCCESS)
        {
            MOS_FreeMemory(pOutputBuffer);
            MOS_OS_CHK_STATUS_RETURN(eStatus);
        }
        if((eStatus = DumpIndirectState(streamState, cmdBuffer, gpuNode, sFileName)) != MOS_STATUS_SUCCESS)
        {
            MOS_FreeMemory(pOutputBuffer);
            MOS_OS_CHK_STATUS_RETURN(eStatus);
        }
    }

    if (streamState->dumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    dwCommandBufferNumber++;
    MOS_FreeMemory(pOutputBuffer);
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

static constexpr GMM_RESOURCE_USAGE_TYPE GmmResourceUsage[MOS_HW_RESOURCE_DEF_MAX] =
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

    // Camera  caputure.
    GMM_RESOURCE_USAGE_CAMERA_CAPTURE,
    // Media GMM Resource USAGES
    GMM_RESOURCE_USAGE_CCS_MEDIA_WRITABLE,

    // Media BLT copy USAGES
    GMM_RESOURCE_USAGE_BLT_SOURCE,
    GMM_RESOURCE_USAGE_BLT_DESTINATION,

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

MOS_HW_RESOURCE_DEF MosInterface::GmmToMosResourceUsageType(
    GMM_RESOURCE_USAGE_TYPE gmmResUsage)
{
    MOS_HW_RESOURCE_DEF mosResUsage = MOS_HW_RESOURCE_DEF_MAX;
    switch (gmmResUsage)
    {
    case GMM_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM;
        break;
    case GMM_RESOURCE_USAGE_DECODE_INPUT_REFERENCE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_INPUT_REFERENCE;
        break;
    case GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_READ;
        break;
    case GMM_RESOURCE_USAGE_DECODE_INTERNAL_WRITE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_WRITE;
        break;
    case GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE;
        break;
    case GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE;
        break;
    case GMM_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE;
        break;
    case GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE;
        break;
    case GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE:
        mosResUsage = MOS_HW_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE;
        break;
    default:
        mosResUsage = MOS_HW_RESOURCE_DEF_MAX;
    }
    return mosResUsage;
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
    else
    {
        return GetDefaultCachePolicyMemoryObject(gmmClientContext);
    }
}

MEMORY_OBJECT_CONTROL_STATE MosInterface::GetDefaultCachePolicyMemoryObject(
    GMM_CLIENT_CONTEXT *gmmClientContext)
{
    MOS_OS_FUNCTION_ENTER;
    if (!gmmClientContext)
    {
        return {0};
    }
    if (gmmClientContext->GetCachePolicyElement(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE).Initialized)  //For arch usage, use GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE as the default setting.
    {
        MOS_OS_NORMALMESSAGE("use GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE");
        return gmmClientContext->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE);
    }
    else if (gmmClientContext->GetCachePolicyElement(MP_RESOURCE_USAGE_DEFAULT).Initialized)
    {
        MOS_OS_NORMALMESSAGE("use MP_RESOURCE_USAGE_DEFAULT");
        return gmmClientContext->CachePolicyGetMemoryObject(nullptr, MP_RESOURCE_USAGE_DEFAULT);
    }
    else
    {
        MOS_OS_NORMALMESSAGE("use GMM_RESOURCE_USAGE_UNKNOWN");
        return gmmClientContext->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_UNKNOWN);
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
            break;
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
        case Media_Format_I420:
            resource->Format = Format_I420;
            break;
        case Media_Format_YV12:
            resource->Format = Format_YV12;
            break;
        default:
            MOS_OS_ASSERTMESSAGE("MOS: unsupported media format for surface.");
            break;
        }
        resource->iWidth   = mediaSurface->iWidth;
        resource->iHeight  = mediaSurface->iHeight;
        resource->iPitch   = mediaSurface->iPitch;
        // Use surface bo size as resource size since we need real bounds checking when fill padding for the surface.
        resource->iSize    = mediaSurface->bo->size;
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
        resource->iSize    = mediaBuffer->bo->size;
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
    MosUtilities::MosAtomicIncrement(MosUtilities::m_mosMemAllocCounterGfx);

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

        MosUtilities::MosAtomicDecrement(MosUtilities::m_mosMemAllocCounterGfx);
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
            MosUtilities::MosAtomicDecrement(MosUtilities::m_mosMemAllocCounterGfx);
            MOS_MEMNINJA_GFX_FREE_MESSAGE(resource->pGmmResInfo, functionName, filename, line);

            perStreamParameters->pGmmClientContext->DestroyResInfoObject(resource->pGmmResInfo);

            resource->pGmmResInfo = nullptr;
        }
    }

    return status;
}

MOS_STATUS MosInterface::FreeResource(
    OsDeviceContext    *osDeviceContext,
    MOS_RESOURCE_HANDLE resource,
    uint32_t            flag
#if MOS_MESSAGES_ENABLED
    ,
    const char *functionName,
    const char *filename,
    int32_t     line
#endif  // MOS_MESSAGES_ENABLED
)
{
    MOS_OS_FUNCTION_ENTER;
    return MOS_STATUS_UNIMPLEMENTED;
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

    // Update Uncompressed write request from resources
    if (gmmResourceInfo->GetMmcHint(0) == GMM_MMC_HINT_OFF)
    {
        resource->bUncompressedWriteNeeded = true;
    }

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

void *MosInterface::LockMosResource(
    OsDeviceContext    *osDeviceContext,
    MOS_RESOURCE_HANDLE resource,
    PMOS_LOCK_PARAMS    flags,
    bool                isDumpPacket)
{
    MOS_OS_FUNCTION_ENTER;
    return nullptr;
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

MOS_STATUS MosInterface::UnlockMosResource(
    OsDeviceContext    *osDeviceContext,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;
    return MOS_STATUS_UNIMPLEMENTED;
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

    if (!mos_bo_is_softpin(resource->bo))
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

MOS_STATUS MosInterface::WaitForCmdCompletion(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuCtx)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do WaitForCmdCompletion

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

    if (resMmcMode == MOS_MEMCOMP_MC)
    {
        MmcFormat = static_cast<uint32_t>(MosInterface::GetGmmClientContext(streamState)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));
        resMmcMode = (MmcFormat > 0 && MmcFormat < 0x1f) ? resMmcMode : MOS_MEMCOMP_DISABLED;
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

MOS_STATUS MosInterface::UnifiedMediaCopyResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    int                 preferMethod)
{
    auto pOsDeviceContext = streamState->osDeviceContext;
    MOS_OS_CHK_NULL_RETURN(pOsDeviceContext);
    auto pMosMediaCopy = pOsDeviceContext->GetMosMediaCopy();
    MOS_OS_CHK_NULL_RETURN(pMosMediaCopy);

    return pMosMediaCopy->MediaCopy(inputResource, outputResource, (MCPY_METHOD)preferMethod);
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

    MosDecompression *mosDecompression = nullptr;
    auto lbdMemDecomp = [&]()
    {
        if (inputResource->bo && inputResource->pGmmResInfo &&
            outputResource->bo && outputResource->pGmmResInfo)
        {
            MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMosDecompressionFromStreamState(streamState, mosDecompression));
            MOS_OS_CHK_NULL_RETURN(mosDecompression);

            // Double Buffer Copy can support any tile status surface with/without compression
            return mosDecompression->MediaMemoryCopy(inputResource, outputResource, outputCompressed);
        }
        else
        {
            return MOS_STATUS_NULL_POINTER;
        }
    };

    if (lbdMemDecomp() != MOS_STATUS_SUCCESS && !mosDecompression)
    {
        status = MosInterface::UnifiedMediaCopyResource(streamState, inputResource, outputResource, MCPY_METHOD_BALANCE);
    }

    return status;
}

MOS_STATUS MosInterface::VerifyMosSurface(
    PMOS_SURFACE mosSurface,
    bool        &bIsValid)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(mosSurface);
    MOS_OS_CHK_NULL_RETURN(mosSurface->OsResource.pGmmResInfo);

    if ((mosSurface->dwPitch * mosSurface->dwHeight > mosSurface->OsResource.pGmmResInfo->GetSizeMainSurface() && (mosSurface->Type != MOS_GFXRES_BUFFER)) ||
        (mosSurface->dwPitch > mosSurface->OsResource.pGmmResInfo->GetSizeMainSurface() && (mosSurface->Type == MOS_GFXRES_BUFFER)) ||
        mosSurface->dwHeight == 0 ||
        mosSurface->dwPitch == 0)
    {
        bIsValid = false;
        MOS_OS_ASSERTMESSAGE("Invalid arguments for mos surface copy: dwPitch %lu, dwHeight %lu, gmmMainSurfaceSize %llu, GFXRES Type %d",
            mosSurface->dwPitch,
            mosSurface->dwHeight,
            mosSurface->OsResource.pGmmResInfo->GetSizeMainSurface(),
            mosSurface->Type);
    }
    else
    {
        bIsValid = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::MediaCopyResource2D(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    uint32_t            copyWidth,
    uint32_t            copyHeight,
    uint32_t            bpp,
    bool                outputCompressed)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(inputResource);
    MOS_OS_CHK_NULL_RETURN(outputResource);
    MOS_OS_CHK_NULL_RETURN(streamState);

    MosDecompression *mosDecompression = nullptr;
    auto lbdMemDecomp = [&]()
    {
        if (inputResource->bo && inputResource->pGmmResInfo &&
            outputResource->bo && outputResource->pGmmResInfo)
        {
            MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMosDecompressionFromStreamState(streamState, mosDecompression));
            MOS_OS_CHK_NULL_RETURN(mosDecompression);

            // Double Buffer Copy can support any tile status surface with/without compression
            return mosDecompression->MediaMemoryCopy2D(inputResource, outputResource,
                copyWidth, copyHeight, 0, 0, bpp, outputCompressed);
        }
        else
        {
            return MOS_STATUS_NULL_POINTER;
        }
    };

    if (lbdMemDecomp() != MOS_STATUS_SUCCESS && !mosDecompression)
    {
        status = MosInterface::UnifiedMediaCopyResource(streamState, inputResource, outputResource, MCPY_METHOD_BALANCE);
    }

    return status;
}

MOS_STATUS MosInterface::MonoSurfaceCopy(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    uint32_t            copyWidth,
    uint32_t            copyHeight,
    uint32_t            copyInputOffset,
    uint32_t            copyOutputOffset,
    bool                outputCompressed)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(inputResource);
    MOS_OS_CHK_NULL_RETURN(outputResource);
    MOS_OS_CHK_NULL_RETURN(streamState);

    MosDecompression *mosDecompression = nullptr;
    auto lbdMemDecomp = [&]()
    {
        if (inputResource->bo && inputResource->pGmmResInfo &&
            outputResource->bo && outputResource->pGmmResInfo)
        {
            MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMosDecompressionFromStreamState(streamState, mosDecompression));
            MOS_OS_CHK_NULL_RETURN(mosDecompression);

            // Double Buffer Copy can support any tile status surface with/without compression
            return mosDecompression->MediaMemoryCopy2D(inputResource, outputResource,
                copyWidth, copyHeight, copyInputOffset, copyOutputOffset, 16, outputCompressed);
        }
        else
        {
            return MOS_STATUS_NULL_POINTER;
        }
    };

    // Use media copy in case of mem decompression failed
    if (lbdMemDecomp() != MOS_STATUS_SUCCESS && !mosDecompression)
    {
        MOS_SURFACE inputResInfo, outputResInfo;
        MOS_ZeroMemory(&inputResInfo, sizeof(MOS_SURFACE));
        MOS_ZeroMemory(&outputResInfo, sizeof(MOS_SURFACE));

        struct BackupResInfo
        {
            MOS_FORMAT format;
            uint32_t   height;
            uint32_t   width;
            uint32_t   pitch;
            uint32_t   offset;
        };

        inputResInfo.Format = Format_Invalid;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetResourceInfo(streamState, inputResource, inputResInfo));
        BackupResInfo bkIn = {inputResInfo.Format, inputResInfo.dwHeight, inputResInfo.dwWidth, inputResInfo.dwPitch};

        outputResInfo.Format = Format_Invalid;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetResourceInfo(streamState, outputResource, outputResInfo));
        BackupResInfo bkOut = {outputResInfo.Format, outputResInfo.dwHeight, outputResInfo.dwWidth, outputResInfo.dwPitch};

        uint32_t   pixelInByte    = 2;
        MOS_FORMAT overrideFormat = Format_Y16U;

        inputResource->pGmmResInfo->OverrideSurfaceFormat(MosInterface::MosFmtToGmmFmt(overrideFormat));
        inputResource->pGmmResInfo->OverrideBaseWidth(copyWidth / pixelInByte);
        inputResource->pGmmResInfo->OverridePitch(copyWidth);
        inputResource->pGmmResInfo->OverrideBaseHeight(copyHeight);
        outputResource->pGmmResInfo->OverrideSurfaceFormat(MosInterface::MosFmtToGmmFmt(overrideFormat));
        outputResource->pGmmResInfo->OverrideBaseWidth(copyWidth / pixelInByte);
        outputResource->pGmmResInfo->OverridePitch(copyWidth);
        outputResource->pGmmResInfo->OverrideBaseHeight(copyHeight);

        uint32_t inOffset           = inputResource->dwOffset;
        uint32_t outOffset          = outputResource->dwOffset;
        inputResource->dwOffset  = copyInputOffset;
        outputResource->dwOffset = copyOutputOffset;

        status = MosInterface::UnifiedMediaCopyResource(streamState, inputResource, outputResource, MCPY_METHOD_BALANCE);

        inputResource->pGmmResInfo->OverrideSurfaceFormat(MosInterface::MosFmtToGmmFmt(bkIn.format));
        inputResource->pGmmResInfo->OverrideBaseWidth(bkIn.width);
        inputResource->pGmmResInfo->OverrideBaseWidth(bkIn.pitch);
        inputResource->pGmmResInfo->OverrideBaseHeight(bkIn.height);
        outputResource->pGmmResInfo->OverrideSurfaceFormat(MosInterface::MosFmtToGmmFmt(bkOut.format));
        outputResource->pGmmResInfo->OverrideBaseWidth(bkOut.width);
        outputResource->pGmmResInfo->OverrideBaseWidth(bkOut.pitch);
        outputResource->pGmmResInfo->OverrideBaseHeight(bkOut.height);

        inputResource->dwOffset  = inOffset;
        outputResource->dwOffset = outOffset;
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
    GMM_RESOURCE_FLAG gmmFlags;
    gmmFlags = resource->pGmmResInfo->GetResFlags();
    if (((gmmFlags.Gpu.MMC ||
        gmmFlags.Gpu.CCS) &&
        gmmFlags.Gpu.UnifiedAuxSurface) ||
        resource->pGmmResInfo->IsMediaMemoryCompressed(0))
    {
        MosDecompression   *mosDecompression = nullptr;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMosDecompressionFromStreamState(streamState, mosDecompression));

        MOS_OS_CHK_NULL_RETURN(mosDecompression);
        mosDecompression->MemoryDecompress(resource);

        MOS_OS_CHK_STATUS_RETURN(MosInterface::SetMemoryCompressionHint(streamState, resource, false));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetMosDecompressionFromStreamState(
    MOS_STREAM_HANDLE   streamState,
    MosDecompression* & mosDecompression)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);
    mosDecompression = streamState->mosDecompression;
    if (!mosDecompression)
    {
        if (streamState->enableDecomp)
        {
            PMOS_CONTEXT mosContext = (PMOS_CONTEXT)streamState->perStreamParameters;
            MOS_OS_CHK_NULL_RETURN(mosContext);
            mosDecompression = streamState->mosDecompression = MOS_New(MosDecompression, mosContext);
        }
        else
        {
            OsContextNext *osCtx = streamState->osDeviceContext;
            MOS_OS_CHK_NULL_RETURN(osCtx);
            mosDecompression = osCtx->GetMosDecompression();
        }
    }
    MOS_OS_CHK_NULL_RETURN(mosDecompression);
    
    return status;
}

MOS_STATUS MosInterface::SetDecompSyncRes(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE syncResource)
{
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

unsigned int MosInterface::GetPATIndexFromGmm(
    GMM_CLIENT_CONTEXT *gmmClient,
    GMM_RESOURCE_INFO *gmmResourceInfo)
{
    if (gmmClient && gmmResourceInfo)
    {
        // GetDriverProtectionBits funtion could hide gmm details info,
        // and we should use GetDriverProtectionBits to replace CachePolicyGetPATIndex in future.
        // isCompressionEnable could be false temparaily.
        bool isCompressionEnable = false;
        return gmmClient->CachePolicyGetPATIndex(
                                            gmmResourceInfo,
                                            gmmResourceInfo->GetCachePolicyUsage(),
                                            &isCompressionEnable,
                                            gmmResourceInfo->GetResFlags().Info.Cacheable);
    }
    return PAT_INDEX_INVALID;
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
    mos_get_context_param(pOsContext->intel_context, 0, DRM_CONTEXT_PARAM_PRIORITY, &priority);
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

    int32_t ret = mos_set_context_param(pOsContext->intel_context, 0, DRM_CONTEXT_PARAM_PRIORITY,(uint64_t)priority);
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

    if (!streamState || !streamState->osDeviceContext)
    {
        return 0;
    }
    auto osDeviceContextSpecific    = static_cast<OsContextSpecificNext *>(streamState->osDeviceContext);
    auto auxTableMgr                = osDeviceContextSpecific->GetAuxTableMgr();
    if(!auxTableMgr)
    {
        return 0;
    }
    return auxTableMgr->GetAuxTableBase();
}

MosCpInterface *MosInterface::GetCpInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return streamState ? streamState->osCpInterface : nullptr;
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

MOS_STATUS MosInterface::SetGpuVirtualAddress(
    PMOS_RESOURCE          pResource,
    uint64_t               address)
{
    return MOS_STATUS_SUCCESS;
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

    case COMPONENT_MCPY:
        componentTag = PERFTAG_VPREP;
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

    MOS_LINUX_CONTEXT *intel_i915_ctx = osParameters->intel_context;
    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    if(gpuContext != nullptr && gpuContext->GetI915Context(0) != nullptr)
    {
        intel_i915_ctx = gpuContext->GetI915Context(0);
    }

    ret = mos_get_reset_stats(intel_i915_ctx, &resetCount, &activeBatch, &pendingBatch);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        return false;
    }

    //Chip reset will count this value, but maybe caused by ctx in other process.
    //In this case, reset has no impact on current process
    if (resetCount      != streamState->gpuResetCount)
    {
        streamState->gpuResetCount    = resetCount;
    }

    //Thses two values are ctx specific, it must reset in current process if either changes
    if  (activeBatch     != streamState->gpuActiveBatch ||
        pendingBatch    != streamState->gpuPendingBatch)
    {
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

void MosInterface::MosResetResource(PMOS_RESOURCE   resource)
{
    int32_t i = 0;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(resource);

    MOS_ZeroMemory(resource, sizeof(MOS_RESOURCE));
    resource->Format  = Format_None;
    for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        resource->iAllocationIndex[i] = MOS_INVALID_ALLOC_INDEX;
    }
    return;
}

bool MosInterface::MosResourceIsNull(PMOS_RESOURCE   resource)
{
    if( nullptr == resource )
    {
        MOS_OS_NORMALMESSAGE("found pOsResource nullptr\n");
        return true;
    }

    return ((resource->bo == nullptr)
#if (_DEBUG || _RELEASE_INTERNAL)
         && ((resource->pData == nullptr) )
#endif // (_DEBUG || _RELEASE_INTERNAL)
    );
}

MOS_STATUS MosInterface::GetGmmResourceInfo(PMOS_RESOURCE resource)
{
    return MOS_STATUS_SUCCESS;
}

int MosInterface::GetPlaneSurfaceOffset(const MOS_PLANE_OFFSET &planeOffset)
{
    return planeOffset.iSurfaceOffset;
}

uint32_t MosInterface::GetResourceArrayIndex(
    PMOS_RESOURCE resource)
{
    return 0;
}

MOS_STATUS MosInterface::SetupCurrentCmdListAndPoolFromOsInterface(
    PMOS_INTERFACE    pMosInterface,
    MOS_STREAM_HANDLE streamState)
{
    return MOS_STATUS_SUCCESS;
}

uint64_t MosInterface::GetResourceHandle(MOS_STREAM_HANDLE streamState, PMOS_RESOURCE osResource)
{
    if (osResource && osResource->bo)
    {
        return osResource->bo->handle;
    }
    else
    {
        return 0;
    }
}

MediaUserSettingSharedPtr MosInterface::MosGetUserSettingInstance(
    PMOS_CONTEXT osContext)
{
    if (osContext == nullptr)
    {
        MOS_OS_NORMALMESSAGE("Null usersetting PTR");
        return nullptr;
    }

    return osContext->m_userSettingPtr;
}

MediaUserSettingSharedPtr MosInterface::MosGetUserSettingInstance(
    MOS_STREAM_HANDLE streamState)
{
    PMOS_CONTEXT mosContext = nullptr;
    if (streamState == nullptr)
    {
        MOS_OS_NORMALMESSAGE("Null usersetting PTR");
        return nullptr;
    }
    mosContext = (PMOS_CONTEXT)streamState->perStreamParameters;
    return MosGetUserSettingInstance(mosContext);
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
MOS_STATUS MosInterface::DumpCommandBufferInit(
    MOS_STREAM_HANDLE streamState)
{
    char                        sFileName[MOS_MAX_HLT_FILENAME_LEN] = {0};
    MOS_STATUS                  eStatus                             = MOS_STATUS_UNKNOWN;
    uint32_t                    value                               = 0;
    size_t                      nSizeFileNamePrefix                 = 0;
    MediaUserSettingSharedPtr   userSettingPtr                      = MosInterface::MosGetUserSettingInstance(streamState);

    MOS_OS_CHK_NULL_RETURN(streamState);

    // Check if command buffer dump was enabled in user feature.
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE,
        MediaUserSetting::Group::Device);

    streamState->dumpCommandBuffer            = (value != 0);
    streamState->dumpCommandBufferToFile      = ((value & 1) != 0);
    streamState->dumpCommandBufferAsMessages  = ((value & 2) != 0);

    if (streamState->dumpCommandBufferToFile)
    {
        // Create output directory.
        eStatus = MosUtilDebug::MosLogFileNamePrefix(streamState->sDirName, userSettingPtr);
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

    return MOS_STATUS_SUCCESS;
}
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)

uint32_t MosInterface::m_mosOsApiFailSimulateType         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateMode         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateFreq         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateHint         = 0;
uint32_t MosInterface::m_mosOsApiFailSimulateCounter      = 0;

void MosInterface::MosInitOsApiFailSimulateFlag(MediaUserSettingSharedPtr userSettingPtr)
{
    MOS_STATUS                  eStatus         = MOS_STATUS_SUCCESS;
    uint32_t                    value           = 0;

    //default off for simulate random fail
    m_mosOsApiFailSimulateType         = OS_API_FAIL_TYPE_NONE;
    m_mosOsApiFailSimulateMode         = OS_API_FAIL_SIMULATE_MODE_DEFAULT;
    m_mosOsApiFailSimulateFreq         = 0;
    m_mosOsApiFailSimulateHint         = 0;
    m_mosOsApiFailSimulateCounter      = 0;

    // Read Config : memory allocation failure simulate mode
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_TYPE,
        MediaUserSetting::Group::Device);

    if (value & OS_API_FAIL_TYPE_MAX)
    {
        m_mosOsApiFailSimulateType = value;
        MOS_OS_NORMALMESSAGE("Init MosSimulateOsApiFailSimulateType as %d \n ", m_mosOsApiFailSimulateType);
    }
    else
    {
        m_mosOsApiFailSimulateType = OS_API_FAIL_TYPE_NONE;
        MOS_OS_NORMALMESSAGE("Invalid OS API Fail Simulate Type from config: %d \n ", value);
    }

    // Read Config : memory allocation failure simulate mode
    value = 0;
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_MODE,
        MediaUserSetting::Group::Device);
    if ((value == OS_API_FAIL_SIMULATE_MODE_DEFAULT) ||
        (value == OS_API_FAIL_SIMULATE_MODE_RANDOM) ||
        (value == OS_API_FAIL_SIMULATE_MODE_TRAVERSE))
    {
        m_mosOsApiFailSimulateMode = value;
        MOS_OS_NORMALMESSAGE("Init MosSimulateOsApiFailSimulateMode as %d \n ", m_mosOsApiFailSimulateMode);
    }
    else
    {
        m_mosOsApiFailSimulateMode = OS_API_FAIL_SIMULATE_MODE_DEFAULT;
        MOS_OS_NORMALMESSAGE("Invalid OS API Fail Simulate Mode from config: %d \n ", value);
    }

    // Read Config : memory allocation failure simulate frequence
    value = 0;
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_FREQ,
        MediaUserSetting::Group::Device);
    if ((value >= MIN_OS_API_FAIL_FREQ) &&
        (value <= MAX_OS_API_FAIL_FREQ))
    {
        m_mosOsApiFailSimulateFreq = value;
        MOS_OS_NORMALMESSAGE("Init m_MosSimulateRandomOsApiFailFreq as %d \n ", m_mosOsApiFailSimulateFreq);

        if (m_mosOsApiFailSimulateMode == OS_API_FAIL_SIMULATE_MODE_RANDOM)
        {
            srand((unsigned int)time(nullptr));
        }
    }
    else
    {
        m_mosOsApiFailSimulateFreq = 0;
        MOS_OS_NORMALMESSAGE("Invalid OS API Fail Simulate Freq from config: %d \n ", value);
    }

    // Read Config : memory allocation failure simulate counter
    value = 0;
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_OS_API_FAIL_SIMULATE_HINT,
        MediaUserSetting::Group::Device);
    if (value <= m_mosOsApiFailSimulateFreq)
    {
        m_mosOsApiFailSimulateHint = value;
        MOS_OS_NORMALMESSAGE("Init m_MosOsApiFailSimulateHint as %d \n ", m_mosOsApiFailSimulateHint);
    }
    else
    {
        m_mosOsApiFailSimulateHint = m_mosOsApiFailSimulateFreq;
        MOS_OS_NORMALMESSAGE("Set m_mosOsApiFailSimulateHint as %d since INVALID CONFIG %d \n ", m_mosOsApiFailSimulateHint, value);
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

MOS_FORMAT MosInterface::GmmFmtToMosFmt(
    GMM_RESOURCE_FORMAT format)
{
    static const std::map<GMM_RESOURCE_FORMAT, MOS_FORMAT> gmm2MosFmtMap = {
        {GMM_FORMAT_B8G8R8X8_UNORM_TYPE, Format_X8R8G8B8},
        {GMM_FORMAT_R8G8B8A8_UNORM_TYPE, Format_A8B8G8R8},
        {GMM_FORMAT_B8G8R8A8_UNORM_TYPE, Format_A8R8G8B8},
        {GMM_FORMAT_B5G6R5_UNORM_TYPE, Format_R5G6B5},
        {GMM_FORMAT_R8G8B8_UNORM, Format_R8G8B8},
        {GMM_FORMAT_R8G8B8_UINT_TYPE, Format_R8G8B8},
        {GMM_FORMAT_R32_UINT_TYPE, Format_R32U},
        {GMM_FORMAT_R32_FLOAT_TYPE, Format_R32F},
        {GMM_FORMAT_MEDIA_Y8_UNORM, Format_Y8},
        {GMM_FORMAT_MEDIA_Y1_UNORM, Format_Y8},
        {GMM_FORMAT_MEDIA_Y16_UNORM, Format_Y16U},
        {GMM_FORMAT_MEDIA_Y16_SNORM, Format_Y16S},
        {GMM_FORMAT_YUY2_2x1, Format_YUY2},
        {GMM_FORMAT_YUY2, Format_YUY2},
        {GMM_FORMAT_P8, Format_P8},
        {GMM_FORMAT_R16_UNORM_TYPE, Format_R16UN},
        {GMM_FORMAT_A8_UNORM_TYPE, Format_A8},
        {GMM_FORMAT_GENERIC_8BIT, Format_L8},
        {GMM_FORMAT_L16_UNORM_TYPE, Format_L16},
        {GMM_FORMAT_GENERIC_16BIT_TYPE, Format_D16},
        {GMM_FORMAT_YVYU, Format_YVYU},
        {GMM_FORMAT_UYVY, Format_UYVY},
        {GMM_FORMAT_VYUY_2x1, Format_VYUY},
        {GMM_FORMAT_VYUY, Format_VYUY},
        {GMM_FORMAT_P016_TYPE, Format_P016},
        {GMM_FORMAT_P010_TYPE, Format_P010},
        {GMM_FORMAT_NV12_TYPE, Format_NV12},
        {GMM_FORMAT_NV11_TYPE, Format_NV11},
        {GMM_FORMAT_P208_TYPE, Format_P208},
        {GMM_FORMAT_IMC1_TYPE, Format_IMC1},
        {GMM_FORMAT_IMC2_TYPE, Format_IMC2},
        {GMM_FORMAT_IMC3_TYPE, Format_IMC3},
        {GMM_FORMAT_IMC4_TYPE, Format_IMC4},
        {GMM_FORMAT_I420_TYPE, Format_I420},
        {GMM_FORMAT_IYUV_TYPE, Format_IYUV},
        {GMM_FORMAT_YV12_TYPE, Format_YV12},
        {GMM_FORMAT_YVU9_TYPE, Format_YVU9},
        {GMM_FORMAT_R8_UNORM_TYPE, Format_R8UN},
        {GMM_FORMAT_R16_UINT_TYPE, Format_R16U},
        {GMM_FORMAT_R8G8_SNORM, Format_V8U8},
        {GMM_FORMAT_R8_UINT_TYPE, Format_R8U},
        {GMM_FORMAT_R32_SINT, Format_R32S},
        {GMM_FORMAT_R8G8_UNORM_TYPE, Format_R8G8UN},
        {GMM_FORMAT_R16_SINT_TYPE, Format_R16S},
        {GMM_FORMAT_R16G16B16A16_UNORM_TYPE, Format_A16B16G16R16},
        {GMM_FORMAT_R16G16B16A16_FLOAT_TYPE, Format_A16B16G16R16F},
        {GMM_FORMAT_R10G10B10A2_UNORM_TYPE, Format_R10G10B10A2},
        {GMM_FORMAT_MFX_JPEG_YUV422H_TYPE, Format_422H},
        {GMM_FORMAT_MFX_JPEG_YUV411_TYPE, Format_411P},
        {GMM_FORMAT_MFX_JPEG_YUV422V_TYPE, Format_422V},
        {GMM_FORMAT_MFX_JPEG_YUV444_TYPE, Format_444P},
        {GMM_FORMAT_BAYER_BGGR16, Format_IRW0},
        {GMM_FORMAT_BAYER_RGGB16, Format_IRW1},
        {GMM_FORMAT_BAYER_GRBG16, Format_IRW2},
        {GMM_FORMAT_BAYER_GBRG16, Format_IRW3},
        {GMM_FORMAT_R8G8B8A8_UINT_TYPE, Format_AYUV},
        {GMM_FORMAT_AYUV, Format_AYUV},
        {GMM_FORMAT_R16G16_UNORM_TYPE, Format_R16G16UN},
        {GMM_FORMAT_R16_FLOAT, Format_R16F},
        {GMM_FORMAT_Y416_TYPE, Format_Y416},
        {GMM_FORMAT_Y410_TYPE, Format_Y410},
        {GMM_FORMAT_Y210_TYPE, Format_Y210},
        {GMM_FORMAT_Y216_TYPE, Format_Y216},
        {GMM_FORMAT_MFX_JPEG_YUV411R_TYPE, Format_411R},
        {GMM_FORMAT_RGBP_TYPE, Format_RGBP},
        {GMM_FORMAT_BGRP_TYPE, Format_RGBP},
        {GMM_FORMAT_R24_UNORM_X8_TYPELESS, Format_D24S8UN},
        {GMM_FORMAT_R32_FLOAT_X8X24_TYPELESS, Format_D32S8X24_FLOAT},
        {GMM_FORMAT_R16G16_SINT_TYPE, Format_R16G16S},
        {GMM_FORMAT_R32G32B32A32_FLOAT, Format_R32G32B32A32F}};

    auto iter = gmm2MosFmtMap.find(format);
    if (iter != gmm2MosFmtMap.end())
    {
        return iter->second;
    }
    return Format_Invalid;
}

GMM_RESOURCE_FORMAT MosInterface::MosFmtToGmmFmt(MOS_FORMAT format)
{
    static const std::map<MOS_FORMAT, GMM_RESOURCE_FORMAT> mos2GmmFmtMap = {
        {Format_Buffer,         GMM_FORMAT_GENERIC_8BIT},
        {Format_Buffer_2D,      GMM_FORMAT_GENERIC_8BIT},
        {Format_L8,             GMM_FORMAT_GENERIC_8BIT},
        {Format_L16,            GMM_FORMAT_L16_UNORM_TYPE},
        {Format_STMM,           GMM_FORMAT_MEDIA_Y8_UNORM},
        {Format_AI44,           GMM_FORMAT_GENERIC_8BIT},
        {Format_IA44,           GMM_FORMAT_GENERIC_8BIT},
        {Format_R5G6B5,         GMM_FORMAT_B5G6R5_UNORM_TYPE},
        {Format_R8G8B8,         GMM_FORMAT_R8G8B8_UNORM},
        {Format_X8R8G8B8,       GMM_FORMAT_B8G8R8X8_UNORM_TYPE},
        {Format_A8R8G8B8,       GMM_FORMAT_B8G8R8A8_UNORM_TYPE},
        {Format_X8B8G8R8,       GMM_FORMAT_R8G8B8X8_UNORM_TYPE},
        {Format_A8B8G8R8,       GMM_FORMAT_R8G8B8A8_UNORM_TYPE},
        {Format_R32F,           GMM_FORMAT_R32_FLOAT_TYPE},
        {Format_V8U8,           GMM_FORMAT_GENERIC_16BIT},  // matching size as format
        {Format_YUY2,           GMM_FORMAT_YUY2},
        {Format_UYVY,           GMM_FORMAT_UYVY},
        {Format_P8,             GMM_FORMAT_RENDER_8BIT_TYPE},  // matching size as format
        {Format_A8,             GMM_FORMAT_A8_UNORM_TYPE},
        {Format_AYUV,           GMM_FORMAT_AYUV_TYPE},
        {Format_NV12,           GMM_FORMAT_NV12_TYPE},
        {Format_NV21,           GMM_FORMAT_NV21_TYPE},
        {Format_YV12,           GMM_FORMAT_YV12_TYPE},
        {Format_R32U,           GMM_FORMAT_R32_UINT_TYPE},
        {Format_R32S,           GMM_FORMAT_R32_SINT_TYPE},
        {Format_RAW,            GMM_FORMAT_GENERIC_8BIT},
        {Format_444P,           GMM_FORMAT_MFX_JPEG_YUV444_TYPE},
        {Format_422H,           GMM_FORMAT_MFX_JPEG_YUV422H_TYPE},
        {Format_422V,           GMM_FORMAT_MFX_JPEG_YUV422V_TYPE},
        {Format_IMC3,           GMM_FORMAT_IMC3_TYPE},
        {Format_411P,           GMM_FORMAT_MFX_JPEG_YUV411_TYPE},
        {Format_411R,           GMM_FORMAT_MFX_JPEG_YUV411R_TYPE},
        {Format_RGBP,           GMM_FORMAT_RGBP_TYPE},
        {Format_BGRP,           GMM_FORMAT_BGRP_TYPE},
        {Format_R8U,            GMM_FORMAT_R8_UINT_TYPE},
        {Format_R8UN,           GMM_FORMAT_R8_UNORM},
        {Format_R16U,           GMM_FORMAT_R16_UINT_TYPE},
        {Format_R16F,           GMM_FORMAT_R16_FLOAT_TYPE},
        {Format_P010,           GMM_FORMAT_P010_TYPE},
        {Format_P016,           GMM_FORMAT_P016_TYPE},
        {Format_Y216,           GMM_FORMAT_Y216_TYPE},
        {Format_Y416,           GMM_FORMAT_Y416_TYPE},
        {Format_P208,           GMM_FORMAT_P208_TYPE},
        {Format_A16B16G16R16,   GMM_FORMAT_R16G16B16A16_UNORM_TYPE},
        {Format_Y210,           GMM_FORMAT_Y210_TYPE},
        {Format_Y410,           GMM_FORMAT_Y410_TYPE},
        {Format_R10G10B10A2,    GMM_FORMAT_R10G10B10A2_UNORM_TYPE},
        {Format_A16B16G16R16F,  GMM_FORMAT_R16G16B16A16_FLOAT},
        {Format_R32G32B32A32F,  GMM_FORMAT_R32G32B32A32_FLOAT}
    };
    
    auto iter = mos2GmmFmtMap.find(format);
    if (iter != mos2GmmFmtMap.end())
    {
        return iter->second;
    }
    return GMM_FORMAT_INVALID;

}

uint32_t MosInterface::MosFmtToOsFmt(MOS_FORMAT format)
{
    static const std::map<MOS_FORMAT, MOS_OS_FORMAT> mos2OsFmtMap = {
        {Format_A8R8G8B8,   (MOS_OS_FORMAT)DDI_FORMAT_A8R8G8B8},
        {Format_X8R8G8B8,   (MOS_OS_FORMAT)DDI_FORMAT_X8R8G8B8},
        {Format_A8B8G8R8,   (MOS_OS_FORMAT)DDI_FORMAT_A8B8G8R8},
        {Format_R32U,       (MOS_OS_FORMAT)DDI_FORMAT_R32F},
        {Format_R32F,       (MOS_OS_FORMAT)DDI_FORMAT_R32F},
        {Format_R5G6B5,     (MOS_OS_FORMAT)DDI_FORMAT_R5G6B5},
        {Format_YUY2,       (MOS_OS_FORMAT)DDI_FORMAT_YUY2},
        {Format_P8,         (MOS_OS_FORMAT)DDI_FORMAT_P8},
        {Format_A8P8,       (MOS_OS_FORMAT)DDI_FORMAT_A8P8},
        {Format_A8,         (MOS_OS_FORMAT)DDI_FORMAT_A8},
        {Format_L8,         (MOS_OS_FORMAT)DDI_FORMAT_L8},
        {Format_L16,        (MOS_OS_FORMAT)DDI_FORMAT_L16},
        {Format_A4L4,       (MOS_OS_FORMAT)DDI_FORMAT_A4L4},
        {Format_A8L8,       (MOS_OS_FORMAT)DDI_FORMAT_A8L8},
        {Format_V8U8,       (MOS_OS_FORMAT)DDI_FORMAT_V8U8},
        {Format_YVYU,       (MOS_OS_FORMAT)FOURCC_YVYU},
        {Format_UYVY,       (MOS_OS_FORMAT)FOURCC_UYVY},
        {Format_VYUY,       (MOS_OS_FORMAT)FOURCC_VYUY},
        {Format_AYUV,       (MOS_OS_FORMAT)FOURCC_AYUV},
        {Format_NV12,       (MOS_OS_FORMAT)FOURCC_NV12},
        {Format_NV21,       (MOS_OS_FORMAT)FOURCC_NV21},
        {Format_NV11,       (MOS_OS_FORMAT)FOURCC_NV11},
        {Format_P208,       (MOS_OS_FORMAT)FOURCC_P208},
        {Format_IMC1,       (MOS_OS_FORMAT)FOURCC_IMC1},
        {Format_IMC2,       (MOS_OS_FORMAT)FOURCC_IMC2},
        {Format_IMC3,       (MOS_OS_FORMAT)FOURCC_IMC3},
        {Format_IMC4,       (MOS_OS_FORMAT)FOURCC_IMC4},
        {Format_I420,       (MOS_OS_FORMAT)FOURCC_I420},
        {Format_IYUV,       (MOS_OS_FORMAT)FOURCC_IYUV},
        {Format_YV12,       (MOS_OS_FORMAT)FOURCC_YV12},
        {Format_YVU9,       (MOS_OS_FORMAT)FOURCC_YVU9},
        {Format_AI44,       (MOS_OS_FORMAT)FOURCC_AI44},
        {Format_IA44,       (MOS_OS_FORMAT)FOURCC_IA44},
        {Format_400P,       (MOS_OS_FORMAT)FOURCC_400P},
        {Format_411P,       (MOS_OS_FORMAT)FOURCC_411P},
        {Format_411R,       (MOS_OS_FORMAT)FOURCC_411R},
        {Format_422H,       (MOS_OS_FORMAT)FOURCC_422H},
        {Format_422V,       (MOS_OS_FORMAT)FOURCC_422V},
        {Format_444P,       (MOS_OS_FORMAT)FOURCC_444P},
        {Format_RGBP,       (MOS_OS_FORMAT)FOURCC_RGBP},
        {Format_BGRP,       (MOS_OS_FORMAT)FOURCC_BGRP},
        {Format_STMM,       (MOS_OS_FORMAT)DDI_FORMAT_P8},
        {Format_P010,       (MOS_OS_FORMAT)FOURCC_P010},
        {Format_P016,       (MOS_OS_FORMAT)FOURCC_P016},
        {Format_Y216,       (MOS_OS_FORMAT)FOURCC_Y216},
        {Format_Y416,       (MOS_OS_FORMAT)FOURCC_Y416},
        {Format_A16B16G16R16, (MOS_OS_FORMAT)DDI_FORMAT_A16B16G16R16},
        {Format_Y210,       (MOS_OS_FORMAT)FOURCC_Y210},
        {Format_Y410,       (MOS_OS_FORMAT)FOURCC_Y410},
        {Format_R32G32B32A32F, (MOS_OS_FORMAT)DDI_FORMAT_R32G32B32A32F}};

    auto iter = mos2OsFmtMap.find(format);
    if (iter != mos2OsFmtMap.end())
    {
        return iter->second;
    }
    return (MOS_OS_FORMAT)DDI_FORMAT_UNKNOWN;
}

MOS_FORMAT MosInterface::OsFmtToMosFmt(uint32_t format)
{
    static const std::map<MOS_OS_FORMAT, MOS_FORMAT> os2MosFmtMap = {
        {DDI_FORMAT_A8B8G8R8,       Format_A8R8G8B8},
        {DDI_FORMAT_X8B8G8R8,       Format_X8R8G8B8},
        {DDI_FORMAT_R32F,           Format_R32F},
        {DDI_FORMAT_A8R8G8B8,       Format_A8R8G8B8},
        {DDI_FORMAT_X8R8G8B8,       Format_X8R8G8B8},
        {DDI_FORMAT_R5G6B5,         Format_R5G6B5},
        {DDI_FORMAT_YUY2,           Format_YUY2},
        {DDI_FORMAT_P8,             Format_P8},
        {DDI_FORMAT_A8P8,           Format_A8P8},
        {DDI_FORMAT_A8,             Format_A8},
        {DDI_FORMAT_L8,             Format_L8},
        {DDI_FORMAT_L16,            Format_L16},
        {DDI_FORMAT_A4L4,           Format_A4L4},
        {DDI_FORMAT_A8L8,           Format_A8L8},
        {DDI_FORMAT_V8U8,           Format_V8U8},
        {DDI_FORMAT_A16B16G16R16,   Format_A16B16G16R16},
        {DDI_FORMAT_R32G32B32A32F,  Format_R32G32B32A32F},
        {FOURCC_YVYU,               Format_YVYU},
        {FOURCC_UYVY,               Format_UYVY},
        {FOURCC_VYUY,               Format_VYUY},
        {FOURCC_AYUV,               Format_AYUV},
        {FOURCC_NV12,               Format_NV12},
        {FOURCC_NV21,               Format_NV21},
        {FOURCC_NV11,               Format_NV11},
        {FOURCC_P208,               Format_P208},
        {FOURCC_IMC1,               Format_IMC1},
        {FOURCC_IMC2,               Format_IMC2},
        {FOURCC_IMC3,               Format_IMC3},
        {FOURCC_IMC4,               Format_IMC4},
        {FOURCC_I420,               Format_I420},
        {FOURCC_IYUV,               Format_IYUV},
        {FOURCC_YV12,               Format_YV12},
        {FOURCC_YVU9,               Format_YVU9},
        {FOURCC_AI44,               Format_AI44},
        {FOURCC_IA44,               Format_IA44},
        {FOURCC_400P,               Format_400P},
        {FOURCC_411P,               Format_411P},
        {FOURCC_411R,               Format_411R},
        {FOURCC_422H,               Format_422H},
        {FOURCC_422V,               Format_422V},
        {FOURCC_444P,               Format_444P},
        {FOURCC_RGBP,               Format_RGBP},
        {FOURCC_BGRP,               Format_BGRP},
        {FOURCC_P010,               Format_P010},
        {FOURCC_P016,               Format_P016},
        {FOURCC_Y216,               Format_Y216},
        {FOURCC_Y416,               Format_Y416},
        {FOURCC_Y210,               Format_Y210},
        {FOURCC_Y410,               Format_Y410}
    };

    auto iter = os2MosFmtMap.find(format);
    if (iter != os2MosFmtMap.end())
    {
        return iter->second;
    }
    return Format_Invalid;
}

bool MosInterface::IsCompressibelSurfaceSupported(MEDIA_FEATURE_TABLE *skuTable)
{
    if(skuTable)
    {
        return MEDIA_IS_SKU(skuTable, FtrCompressibleSurfaceDefault);
    }
    return true;
}

bool MosInterface::IsMismatchOrderProgrammingSupported()
{
    return false;
}

MOS_STATUS MosInterface::WaitForBBCompleteNotifyEvent(
    MOS_STREAM_HANDLE       streamState,
    GPU_CONTEXT_HANDLE      gpuContextHandle,
    uint32_t                uiTimeOut)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::RegisterBBCompleteNotifyEvent(
    MOS_STREAM_HANDLE   streamState,
    GPU_CONTEXT_HANDLE  gpuContextHandle)
{
    return MOS_STATUS_SUCCESS;
}

void MosInterface::GetRtLogResourceInfo(
    PMOS_INTERFACE osInterface,
    PMOS_RESOURCE &osResource,
    uint32_t &size)
{
    osResource = nullptr;
    size = 0;
    if (osInterface->osStreamState && osInterface->osStreamState->osDeviceContext)
    {
        MosOcaRTLogMgr *ocaRTLogMgr = MosOcaRTLogMgr::GetInstance();
        MOS_OS_CHK_NULL_NO_STATUS_RETURN(ocaRTLogMgr);
        GpuContextSpecificNext *gpuContext = dynamic_cast<GpuContextSpecificNext*>(osInterface->osStreamState->osDeviceContext->GetGpuContextMgr()->GetGpuContext(osInterface->osStreamState->currentGpuContextHandle));
        if (gpuContext != nullptr)
        {
            osResource = gpuContext->GetOcaRTLogResource(osInterface->osStreamState->osDeviceContext->GetOcaRTLogResource());
            size       = ocaRTLogMgr->GetRtlogHeapSize();
        }
    }
}

bool MosInterface::IsPooledResource(MOS_STREAM_HANDLE streamState, PMOS_RESOURCE osResource)
{
    return false;
}

MOS_TILE_TYPE MosInterface::MapTileType(GMM_RESOURCE_FLAG flags, GMM_TILE_TYPE type)
{
    return MOS_TILE_INVALID;
}

MOS_STATUS MosInterface::SetMultiEngineEnabled(
    PMOS_INTERFACE pOsInterface,
    MOS_COMPONENT  component,
    bool           enabled)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetMultiEngineStatus(
    PMOS_INTERFACE pOsInterface,
    PLATFORM      *platform,
    MOS_COMPONENT  component,
    bool          &isMultiDevices,
    bool          &isMultiEngine)
{
    return MOS_STATUS_SUCCESS;
}
