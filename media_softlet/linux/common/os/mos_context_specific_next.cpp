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
//! \file     mos_context_specific_next.cpp
//! \brief    Container for Linux/Android specific parameters shared across different GPU contexts of the same device instance 
//!

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_resource_defs.h"
#include <unistd.h>
#include <dlfcn.h>
#include "hwinfo_linux.h"
#include "mos_interface.h"
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontextmgr_next.h"
#include "mos_cmdbufmgr_next.h"
#include "mos_oca_rtlog_mgr.h"
#include "mos_oca_interface_specific.h"
#define BATCH_BUFFER_SIZE 0x80000

OsContextSpecificNext::OsContextSpecificNext()
{
    MOS_OS_FUNCTION_ENTER;
}

OsContextSpecificNext::~OsContextSpecificNext()
{
    MOS_OS_FUNCTION_ENTER;
}

MOS_STATUS OsContextSpecificNext::Init(DDI_DEVICE_CONTEXT ddiDriverContext)
{
    uint32_t      iDeviceId = 0;
    MOS_STATUS    eStatus   = MOS_STATUS_SUCCESS;
    uint32_t      value     = 0;
    uint32_t      mode      = 0;
    MediaUserSettingSharedPtr   userSettingPtr = nullptr;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    PMOS_CONTEXT osDriverContext = (PMOS_CONTEXT)ddiDriverContext;

    if (GetOsContextValid() == false)
    {
        uint32_t commandBufferSize = 0;
        m_skuTable.reset();
        m_waTable.reset();
        MosUtilities::MosZeroMemory(&m_platformInfo, sizeof(m_platformInfo));
        MosUtilities::MosZeroMemory(&m_gtSystemInfo, sizeof(m_gtSystemInfo));

        if( nullptr == osDriverContext  ||
            0 > osDriverContext->fd )
        {
            MOS_OS_ASSERT(false);
            return MOS_STATUS_INVALID_HANDLE;
        }
        m_fd = osDriverContext->fd;

        userSettingPtr = MosInterface::MosGetUserSettingInstance(osDriverContext);

        m_bufmgr = mos_bufmgr_gem_init(m_fd, BATCH_BUFFER_SIZE, &m_deviceType);
        if (nullptr == m_bufmgr)
        {
            MOS_OS_ASSERTMESSAGE("Not able to allocate buffer manager, fd=0x%d", m_fd);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        mos_bufmgr_enable_reuse(m_bufmgr);

        osDriverContext->bufmgr                 = m_bufmgr;

        //Latency reducation:replace HWGetDeviceID to get device using ioctl from drm.
        iDeviceId   = mos_bufmgr_get_devid(m_bufmgr);
        m_isAtomSOC = IS_ATOMSOC(iDeviceId);

        eStatus = NullHwInit((MOS_CONTEXT_HANDLE)osDriverContext);
        if (!GetNullHwIsEnabled())
        {
            eStatus = HWInfo_GetGfxInfo(m_fd, m_bufmgr, &m_platformInfo, &m_skuTable, &m_waTable, &m_gtSystemInfo, userSettingPtr);
        }
        else
        {
            m_platformInfo  = osDriverContext->m_platform;
            m_skuTable      = osDriverContext->m_skuTable;
            m_waTable       = osDriverContext->m_waTable;
            m_gtSystemInfo  = osDriverContext->m_gtSystemInfo;
            iDeviceId       = osDriverContext->iDeviceId;
        }

        // replace platform/sku/wa/gtsysinfo for os context
        if (Mos_Solo_IsEnabled(nullptr))
        {
            m_skuTable.reset();
            m_waTable.reset();
            Mos_Solo_SetPlatform(&m_platformInfo, (PMOS_CONTEXT)osDriverContext);
            Mos_Solo_SetSkuwaGtInfo((PMOS_CONTEXT)osDriverContext, m_platformInfo, m_skuTable, m_waTable, m_gtSystemInfo, iDeviceId);
        }

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization");
            return eStatus;
        }

        if (m_platformInfo.eProductFamily == IGFX_METEORLAKE ||
            m_platformInfo.eProductFamily == IGFX_ARROWLAKE ||
            m_platformInfo.eProductFamily == IGFX_LUNARLAKE)
        {
            ReadUserSetting(
                userSettingPtr,
                value,
                "INTEL MEDIA ALLOC MODE",
                MediaUserSetting::Group::Device);

            if (value)
            {
                mode = (value & 0x000000ff);
            }

            // Realloc cache only if it's not mode 0
            if (mode)
            {
                mos_bufmgr_realloc_cache(m_bufmgr, mode);
            }
        }

        ReadUserSetting(
            userSettingPtr,
            value,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_SOFTPIN,
            MediaUserSetting::Group::Device);

        if (value)
        {
            bool softpin_va1Malign = false;
            if (MEDIA_IS_SKU(&m_skuTable, Ftr1MGranularAuxTable))
            {
                softpin_va1Malign = true;
            }

            mos_bufmgr_enable_softpin(m_bufmgr, softpin_va1Malign);

            ReadUserSetting(
                userSettingPtr,
                value,
                __MEDIA_USER_FEATURE_VALUE_ENABLE_VM_BIND,
                MediaUserSetting::Group::Device);

            if (value)
            {
                 mos_bufmgr_enable_vmbind(m_bufmgr);
                 MOS_OS_NORMALMESSAGE("mos_bufmg_enable_vmbind");
            }
        }

        uint64_t isRecoverableContextEnabled = 0;
        MOS_LINUX_CONTEXT *intel_context = mos_context_create_ext(m_bufmgr, 0, false);
        int ret = mos_get_context_param(intel_context, 0, DRM_CONTEXT_PARAM_RECOVERABLE, &isRecoverableContextEnabled);
        if (ret == -EINVAL)
        {
            isRecoverableContextEnabled = 1;
        }
        if (intel_context)
        {
            mos_context_destroy(intel_context);
        }
        // set recoverablecontext disabled if want disable object capture
        if (MEDIA_IS_WA(&m_waTable, WaDisableSetObjectCapture) && isRecoverableContextEnabled)
        {
            mos_bufmgr_disable_object_capture(m_bufmgr);
        }

        if (MEDIA_IS_SKU(&m_skuTable, FtrEnableMediaKernels) == 0)
        {
            MEDIA_WR_WA(&m_waTable, WaHucStreamoutOnlyDisable, 0);
        }

        MosUtilities::MosTraceSetupInfo(
            (VA_MAJOR_VERSION << 16) | VA_MINOR_VERSION,
            m_platformInfo.eProductFamily,
            m_platformInfo.eRenderCoreFamily,
            (m_platformInfo.usRevId << 16) | m_platformInfo.usDeviceID);

        GMM_SKU_FEATURE_TABLE   gmmSkuTable   = {};
        GMM_WA_TABLE            gmmWaTable    = {};
        GMM_GT_SYSTEM_INFO      gmmGtInfo     = {};
        GMM_ADAPTER_BDF         gmmAdapterBDF = {};
        eStatus = HWInfo_GetGmmInfo(m_bufmgr, &gmmSkuTable, &gmmWaTable, &gmmGtInfo);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            MOS_OS_ASSERTMESSAGE("Fatal error - unsuccesfull Gmm Sku/Wa/GtSystemInfo initialization");
            return eStatus;
        }

        eStatus = MosInterface::GetAdapterBDF(osDriverContext, &gmmAdapterBDF);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            MOS_OS_ASSERTMESSAGE("Fatal error - unsuccesfull Gmm Adapter BDF initialization");
            return eStatus;
        }

        // Initialize Gmm context
        GMM_INIT_IN_ARGS  gmmInitAgrs = {};
        GMM_INIT_OUT_ARGS gmmOutArgs  = {};
        gmmInitAgrs.Platform          = m_platformInfo;
        gmmInitAgrs.pSkuTable         = &gmmSkuTable;
        gmmInitAgrs.pWaTable          = &gmmWaTable;
        gmmInitAgrs.pGtSysInfo        = &gmmGtInfo;
        gmmInitAgrs.FileDescriptor    = gmmAdapterBDF.Data;
        gmmInitAgrs.ClientType        = (GMM_CLIENT)GMM_LIBVA_LINUX;

        // replace sku/wa/gtsysinfo for gmm client context
        if (Mos_Solo_IsEnabled(nullptr))
        {
            gmmInitAgrs.pSkuTable         = &m_skuTable;
            gmmInitAgrs.pWaTable          = &m_waTable;
            gmmInitAgrs.pGtSysInfo        = &m_gtSystemInfo;
        }

        GMM_STATUS status = InitializeGmm(&gmmInitAgrs, &gmmOutArgs);
        if (status != GMM_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Fatal error - InitializeGmm fail.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        m_gmmClientContext = gmmOutArgs.pGmmClientContext;

        m_auxTableMgr = AuxTableMgr::CreateAuxTableMgr(m_bufmgr, &m_skuTable, m_gmmClientContext);

#if (_DEBUG || _RELEASE_INTERNAL)
        ReadUserSettingForDebug(
            userSettingPtr,
            osDriverContext->bSimIsActive,
            __MEDIA_USER_FEATURE_VALUE_SIM_ENABLE,
            MediaUserSetting::Group::Device);
#endif

        m_useSwSwizzling = osDriverContext->bSimIsActive || MEDIA_IS_SKU(&m_skuTable, FtrUseSwSwizzling);

        m_tileYFlag      = MEDIA_IS_SKU(&m_skuTable, FtrTileY);

        m_use64BitRelocs = true;

        if (!GetNullHwIsEnabled())
        {
            osDriverContext->iDeviceId              = iDeviceId;
            osDriverContext->m_skuTable             = m_skuTable;
            osDriverContext->m_waTable              = m_waTable;
            osDriverContext->m_gtSystemInfo         = m_gtSystemInfo;
            osDriverContext->m_platform             = m_platformInfo;
        }
        osDriverContext->pGmmClientContext      = m_gmmClientContext;
        osDriverContext->m_auxTableMgr          = m_auxTableMgr;
        osDriverContext->bUseSwSwizzling        = m_useSwSwizzling;
        osDriverContext->bTileYFlag             = m_tileYFlag;
        osDriverContext->bIsAtomSOC             = m_isAtomSOC;
        osDriverContext->m_osDeviceContext      = this;

        m_usesPatchList             = true;
        m_usesGfxAddress            = false;

        SetOsContextValid(true);
        // Prepare the command buffer manager
        if (m_ocaLogSectionSupported)
        {
            // increase size for oca log section
            commandBufferSize = MosOcaInterfaceSpecific::IncreaseSize(COMMAND_BUFFER_SIZE);
        }
        else
        {
            commandBufferSize = COMMAND_BUFFER_SIZE;
        }
        m_cmdBufMgr = CmdBufMgrNext::GetObject();
        MOS_OS_CHK_NULL_RETURN(m_cmdBufMgr);
        MOS_OS_CHK_STATUS_RETURN(m_cmdBufMgr->Initialize(this, commandBufferSize));

        // Prepare the gpu Context manager
        m_gpuContextMgr = GpuContextMgrNext::GetObject(this);
        MOS_OS_CHK_NULL_RETURN(m_gpuContextMgr);

        m_perfData = (PERF_DATA*)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
        MOS_OS_CHK_NULL_RETURN(m_perfData);
        osDriverContext->pPerfData = m_perfData;

        //It must be done with m_gpuContextMgr ready. Insides it will create gpu context.
#ifdef _MMC_SUPPORTED
        m_mosDecompression = MOS_New(MosDecompression, osDriverContext);
        MOS_OS_CHK_NULL_RETURN(m_mosDecompression);
        osDriverContext->ppMediaMemDecompState = m_mosDecompression->GetMediaMemDecompState();
        MOS_OS_CHK_NULL_RETURN(osDriverContext->ppMediaMemDecompState);
        if (*osDriverContext->ppMediaMemDecompState == nullptr)
        {
            MOS_OS_NORMALMESSAGE("Decomp state creation failed");
        }
#endif
        m_mosMediaCopy = MOS_New(MosMediaCopy, osDriverContext);
        if (nullptr == m_mosMediaCopy)
        {
            MOS_OS_NORMALMESSAGE("m_mosMediaCopy creation failed");
        }
        else
        {
            osDriverContext->ppMediaCopyState = (void **)m_mosMediaCopy->GetMediaCopyState();
            if ((nullptr == osDriverContext->ppMediaCopyState) || (nullptr == *osDriverContext->ppMediaCopyState))
            {
                MOS_OS_NORMALMESSAGE("Media Copy state creation failed");
            }
        }
    }
    MosOcaRTLogMgr::RegisterContext(this, osDriverContext);
    return eStatus;
}

void OsContextSpecificNext::Destroy()
{
    MOS_OS_FUNCTION_ENTER;

    if (GetOsContextValid() == true)
    {
        if (m_auxTableMgr != nullptr)
        {
            MOS_Delete(m_auxTableMgr);
            m_auxTableMgr = nullptr;
        }

        m_skuTable.reset();
        m_waTable.reset();

        mos_bufmgr_destroy(m_bufmgr);

        // Delete Gmm context
        GMM_INIT_OUT_ARGS gmmOutArgs = {};
        gmmOutArgs.pGmmClientContext = m_gmmClientContext;
        GmmAdapterDestroy(&gmmOutArgs);
        m_gmmClientContext = nullptr;

        SetOsContextValid(false);

        if (m_perfData != nullptr)
        {
           MOS_FreeMemory(m_perfData);
           m_perfData = nullptr;
        }

        if (m_mosMediaCopy != nullptr)
        {
           MOS_Delete(m_mosMediaCopy);
        }
    }

}

