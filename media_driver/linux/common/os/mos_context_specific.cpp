/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file     mos_context_specific.cpp
//! \brief    Container for Linux/Android specific parameters shared across different GPU contexts of the same device instance 
//!

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_resource_defs.h"
#include <unistd.h>
#include <dlfcn.h>
#include "hwinfo_linux.h"
#include <stdlib.h>

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"

#include "mos_context_specific.h"
#include "mos_gpucontextmgr.h"
#include "mos_cmdbufmgr.h"

OsContextSpecific::OsContextSpecific()
{
    for (int i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        m_GpuContextHandle[i] = MOS_GPU_CONTEXT_INVALID_HANDLE;
    }

    MOS_OS_FUNCTION_ENTER;
}

OsContextSpecific::~OsContextSpecific()
{
    MOS_OS_FUNCTION_ENTER;
}

void OsContextSpecific::GetGpuPriority(int32_t* pPriority)
{
    uint64_t priority = 0;
    mos_get_context_param(m_intelContext, 0, I915_CONTEXT_PARAM_PRIORITY, &priority);
    *pPriority = (int32_t)priority;
}

void OsContextSpecific::SetGpuPriority(int32_t priority)
{
    int ret = 0;
    ret = mos_set_context_param(m_intelContext, 0, I915_CONTEXT_PARAM_PRIORITY,(uint64_t)priority);
    if (ret != 0)
    {
        MOS_OS_NORMALMESSAGE("Warning: failed to set the gpu priority, errno is %d", ret);
    }
}

MOS_STATUS OsContextSpecific::Init(PMOS_CONTEXT pOsDriverContext)
{
    uint32_t      iDeviceId = 0;
    MOS_STATUS    eStatus;
    uint32_t      i = 0;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    if (GetOsContextValid() == false)
    {
        if( nullptr == pOsDriverContext         ||
            nullptr == pOsDriverContext->bufmgr ||
            0 >= pOsDriverContext->fd )
        {
            MOS_OS_ASSERT(false);
            return MOS_STATUS_INVALID_HANDLE;
        }
        m_apoMosEnabled = pOsDriverContext->m_apoMosEnabled;
        m_bufmgr        = pOsDriverContext->bufmgr;
        m_gpuContextMgr = static_cast<GpuContextMgr *>(pOsDriverContext->m_gpuContextMgr);
        m_cmdBufMgr     = static_cast<CmdBufMgr *>(pOsDriverContext->m_cmdBufMgr);
        m_fd            = pOsDriverContext->fd;
        MOS_SecureMemcpy(&m_perfData, sizeof(PERF_DATA), pOsDriverContext->pPerfData, sizeof(PERF_DATA));
        mos_bufmgr_gem_enable_reuse(pOsDriverContext->bufmgr);
        m_pGmmClientContext = pOsDriverContext->pGmmClientContext;
        m_auxTableMgr = pOsDriverContext->m_auxTableMgr;
    
        // DDI layer can pass over the DeviceID.
        iDeviceId = pOsDriverContext->iDeviceId;
        if (0 == iDeviceId)
        {
            PLATFORM           platformInfo;
            MEDIA_FEATURE_TABLE  skuTable;
            MEDIA_WA_TABLE       waTable;
            MEDIA_SYSTEM_INFO    gtSystemInfo;
    
            MOS_ZeroMemory(&platformInfo, sizeof(platformInfo));
            MOS_ZeroMemory(&skuTable, sizeof(skuTable));
            MOS_ZeroMemory(&waTable, sizeof(waTable));
            MOS_ZeroMemory(&gtSystemInfo, sizeof(gtSystemInfo));
            eStatus = HWInfo_GetGfxInfo(pOsDriverContext->fd, pOsDriverContext->bufmgr, &platformInfo, &skuTable, &waTable, &gtSystemInfo);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization");
                return eStatus;
            }
    
            MOS_SecureMemcpy(&m_platformInfo, sizeof(PLATFORM), &platformInfo, sizeof(PLATFORM));
            MOS_SecureMemcpy(&m_gtSystemInfo, sizeof(MEDIA_SYSTEM_INFO), &gtSystemInfo, sizeof(MEDIA_SYSTEM_INFO));
    
            pOsDriverContext->iDeviceId      = platformInfo.usDeviceID;
            m_skuTable = skuTable;
            m_waTable  = waTable;
    
            pOsDriverContext->SkuTable       = skuTable;
            pOsDriverContext->WaTable        = waTable;
            pOsDriverContext->gtSystemInfo   = gtSystemInfo;
            pOsDriverContext->platform       = platformInfo;
    
            MOS_OS_NORMALMESSAGE("DeviceID was created DeviceID = %d, platform product %d", iDeviceId, platformInfo.eProductFamily);
        }
        else
        {
            // pOsDriverContext's parameters were passed by CmCreateDevice.
            // Get SkuTable/WaTable/systemInfo/platform from OSDriver directly.
            MOS_SecureMemcpy(&m_platformInfo, sizeof(PLATFORM), &(pOsDriverContext->platform), sizeof(PLATFORM));
            MOS_SecureMemcpy(&m_gtSystemInfo, sizeof(MEDIA_SYSTEM_INFO), &(pOsDriverContext->gtSystemInfo), sizeof(MEDIA_SYSTEM_INFO));
    
            m_skuTable = pOsDriverContext->SkuTable;
            m_waTable  = pOsDriverContext->WaTable;
        }

        m_use64BitRelocs = true;
        m_useSwSwizzling = pOsDriverContext->bSimIsActive || MEDIA_IS_SKU(&m_skuTable, FtrUseSwSwizzling);
        m_tileYFlag      = MEDIA_IS_SKU(&m_skuTable, FtrTileY);

        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_START,
                          &eStatus, sizeof(eStatus), nullptr, 0);
        if (!Mos_Solo_IsEnabled(nullptr) && MEDIA_IS_SKU(&m_skuTable,FtrContextBasedScheduling))
        {
            m_intelContext = mos_gem_context_create_ext(pOsDriverContext->bufmgr,0);
            if (m_intelContext)
            {
                m_intelContext->vm = mos_gem_vm_create(pOsDriverContext->bufmgr);
                if (m_intelContext->vm == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("Failed to create vm.\n");
                    return MOS_STATUS_UNKNOWN;
                }
            }
        }
        else //use legacy context create ioctl for pre-gen11 platforms
        {
           m_intelContext = mos_gem_context_create(pOsDriverContext->bufmgr);
           if (m_intelContext)
           {
               m_intelContext->vm = nullptr;
           }
        }
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_CREATE, EVENT_TYPE_END,
                          &m_intelContext, sizeof(void *),
                          &eStatus, sizeof(eStatus));

        if (m_intelContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to create drm intel context");
            return MOS_STATUS_UNKNOWN;
        }

        m_isAtomSOC = IS_ATOMSOC(iDeviceId);

    #ifndef ANDROID
    
        if ((m_gtSystemInfo.VDBoxInfo.IsValid) && (m_gtSystemInfo.VDBoxInfo.NumberOfVDBoxEnabled > 1))
        {
            m_kmdHasVCS2 = true;
        }
        else
        {
            m_kmdHasVCS2 = false;
        }

    #endif
    
        m_transcryptedKernels       = nullptr;
        m_transcryptedKernelsSize   = 0;
    
        // For Media Memory compression
        m_mediaMemDecompState       = pOsDriverContext->ppMediaMemDecompState;
        m_memoryDecompress          = pOsDriverContext->pfnMemoryDecompress;
        m_mediaMemCopy              = pOsDriverContext->pfnMediaMemoryCopy;
        m_mediaMemCopy2D            = pOsDriverContext->pfnMediaMemoryCopy2D;
        m_mosContext                = pOsDriverContext;
    
        m_noParsingAssistanceInKmd  = true;
        m_numNalUnitBytesIncluded   = MOS_NAL_UNIT_LENGTH - MOS_NAL_UNIT_STARTCODE_LENGTH;
    
        // Init reset count for the context
        uint32_t dwResetCount       = 0;
        mos_get_reset_stats(m_intelContext, &dwResetCount, nullptr, nullptr);
        m_gpuResetCount             = dwResetCount;
        m_gpuActiveBatch            = 0;
        m_gpuPendingBatch           = 0;
    
        m_usesPatchList             = true;
        m_usesGfxAddress            = false;
    
        m_inlineCodecStatusUpdate   = true;
    
    #if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
        CommandBufferDumpInit(pOsDriverContext);
    #endif
    
        SetOsContextValid(true);
    }
    return eStatus;
}

void OsContextSpecific::Destroy()
{
    MOS_OS_FUNCTION_ENTER;

    if (GetOsContextValid() == true)
    {
        // APO MOS will destory each stream's GPU context at different place
        if (!m_apoMosEnabled)
        {
            for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
            {
                if (m_GpuContextHandle[i] != MOS_GPU_CONTEXT_INVALID_HANDLE)
                {
                    if (m_gpuContextMgr == nullptr)
                    {
                        MOS_OS_ASSERTMESSAGE("GpuContextMgr is null when destroy GpuContext");
                        break;
                    }
                    auto gpuContext = m_gpuContextMgr->GetGpuContext(m_GpuContextHandle[i]);
                    if (gpuContext == nullptr)
                    {
                        MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active gpuContextHandle");
                        continue;
                    }
                    m_gpuContextMgr->DestroyGpuContext(gpuContext);
                }
            }
        }

        m_skuTable.reset();
        m_waTable.reset();
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_DESTROY, EVENT_TYPE_START,
                          &m_intelContext, sizeof(void *), nullptr, 0);
        if (m_intelContext && m_intelContext->vm)
        {
            mos_gem_vm_destroy(m_intelContext->bufmgr, m_intelContext->vm);
        }
        if (m_intelContext)
        {
            mos_gem_context_destroy(m_intelContext);
        }
        MOS_TraceEventExt(EVENT_GPU_CONTEXT_DESTROY, EVENT_TYPE_END,
                          nullptr, 0, nullptr, 0);
        SetOsContextValid(false);
    }
}

