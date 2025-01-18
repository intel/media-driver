/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mhw_vebox_impl.h
//! \brief    MHW VEBOX interface common base
//! \details
//!

#ifndef __MHW_VEBOX_IMPL_H__
#define __MHW_VEBOX_IMPL_H__

#include "mhw_vebox_itf.h"
#include "mhw_impl.h"
#include "hal_oca_interface_next.h"
#include "mos_solo_generic.h"

#ifdef IGFX_VEBOX_INTERFACE_EXT_SUPPORT
#include "mhw_vebox_impl_ext.h"
#endif

namespace mhw
{
namespace vebox
{

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _VEBOX_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;

        MOS_ZeroMemory(&m_veboxSettings, sizeof(m_veboxSettings));

        MEDIA_ENGINE_INFO mediaSysInfo = {};

        m_veboxSettings = g_Vebox_Settings;
        m_vebox0InUse = false;
        m_vebox1InUse = false;
        m_veboxScalabilitySupported = false;
        m_veboxSplitRatio = 50;
        memset(&m_chromaParams, 0, sizeof(m_chromaParams));

        MOS_SecureMemcpy(m_BT2020InvPixelValue, sizeof(uint32_t) * 256, g_Vebox_BT2020_Inverse_Pixel_Value, sizeof(uint32_t) * 256);
        MOS_SecureMemcpy(m_BT2020FwdPixelValue, sizeof(uint32_t) * 256, g_Vebox_BT2020_Forward_Pixel_Value, sizeof(uint32_t) * 256);
        MOS_SecureMemcpy(m_BT2020InvGammaLUT, sizeof(uint32_t) * 256, g_Vebox_BT2020_Inverse_Gamma_LUT, sizeof(uint32_t) * 256);
        MOS_SecureMemcpy(m_BT2020FwdGammaLUT, sizeof(uint32_t) * 256, g_Vebox_BT2020_Forward_Gamma_LUT, sizeof(uint32_t) * 256);

        MOS_ZeroMemory(&m_laceColorCorrection, sizeof(m_laceColorCorrection));

        MHW_CHK_NULL_NO_STATUS_RETURN(osItf);
        MOS_STATUS eStatus = osItf->pfnGetMediaEngineInfo(osItf, mediaSysInfo);
        if (eStatus == MOS_STATUS_SUCCESS)
        {
            if (mediaSysInfo.VEBoxInfo.IsValid &&
                mediaSysInfo.VEBoxInfo.NumberOfVEBoxEnabled > 1)
            {
                m_veboxScalabilitySupported = true;
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_userSettingPtr != nullptr)
        {
            ReadUserSettingForDebug(
                m_userSettingPtr,
                m_veboxSplitRatio,
                __MEDIA_USER_FEATURE_VALUE_VEBOX_SPLIT_RATIO,
                MediaUserSetting::Group::Device);
        }
#endif
    };

    static __inline uint32_t MosGetHWTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
    {
        uint32_t tileMode = 0;

        if (gmmTileEnabled)
        {
            return tileModeGMM;
        }

        switch (tileType)
        {
        case MOS_TILE_LINEAR:
            tileMode = 0;
            break;
        case MOS_TILE_YS:
            tileMode = 1;
            break;
        case MOS_TILE_X:
            tileMode = 2;
            break;
        default:
            tileMode = 3;
            break;
        }
        return tileMode;
    }

    MOS_STATUS UpdateVeboxSync() override
    {
        PMHW_VEBOX_HEAP          pVeboxHeap;
        MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
        PMOS_INTERFACE           pOsInterface;

        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(this->m_osItf);
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        pVeboxHeap      = m_veboxHeap;
        pOsInterface    = this->m_osItf;

        // If KMD frame tracking is on, the dwSyncTag has been set to gpu status tag
        // in Mhw_VeboxInterface_AssignVeboxState(). dwNextTag is not used anymore.
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            pVeboxHeap->pStates[pVeboxHeap->uiCurState].dwSyncTag =
                pVeboxHeap->dwNextTag++;
        }
        pVeboxHeap->pStates[pVeboxHeap->uiCurState].bBusy = true;

        return eStatus;
    }

    MOS_STATUS GetVeboxHeapInfo(
        const MHW_VEBOX_HEAP** ppVeboxHeap) override
    {
        MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(ppVeboxHeap);

        *ppVeboxHeap = (const MHW_VEBOX_HEAP*)m_veboxHeap;

        return eStatus;
    }

    MOS_STATUS SetVeboxHeapStateIndex(
        uint32_t index) override
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(m_veboxHeap);

        m_veboxHeap->uiCurState = index;

        return eStatus;
    }

    uint32_t GetVeboxNumInstances() override
    {
        MHW_FUNCTION_ENTER;

        return m_veboxSettings.uiNumInstances;
    }

    MOS_STATUS DestroyHeap() override
    {
        PMOS_INTERFACE       pOsInterface;
        MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(this->m_osItf);

        pOsInterface = this->m_osItf;

        if (m_veboxHeap)
        {
#if (_DEBUG || _RELEASE_INTERNAL)
            ReportVeboxId();
#endif
            if (!Mos_ResourceIsNull(&m_veboxHeap->DriverResource))
            {
                if (m_veboxHeap->pLockedDriverResourceMem)
                {
                    pOsInterface->pfnUnlockResource(
                        pOsInterface,
                        &m_veboxHeap->DriverResource);
                }

                pOsInterface->pfnFreeResource(
                    pOsInterface,
                    &m_veboxHeap->DriverResource);
            }

            if (!Mos_ResourceIsNull(&m_veboxHeap->KernelResource))
            {
                pOsInterface->pfnFreeResource(
                    pOsInterface,
                    &m_veboxHeap->KernelResource);
            }

            MOS_FreeMemory(m_veboxHeap);
            m_veboxHeap = nullptr;
        }
        return eStatus;
    }

    MOS_STATUS CreateHeap() override
    {
        MOS_STATUS              eStatus;
        uint8_t* pMem;
        uint32_t                uiSize;
        uint32_t                uiOffset;
        MOS_ALLOC_GFXRES_PARAMS AllocParams;
        MOS_LOCK_PARAMS         LockFlags;
        MEDIA_FEATURE_TABLE* skuTable = nullptr;

        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(this->m_osItf);
        MHW_CHK_NULL_RETURN(this->m_osItf->pfnGetSkuTable);

        skuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);
        MHW_CHK_NULL_RETURN(skuTable);

        eStatus = MOS_STATUS_SUCCESS;

        uiSize = sizeof(MHW_VEBOX_HEAP);
        uiSize += m_veboxSettings.uiNumInstances *
            sizeof(MHW_VEBOX_HEAP_STATE);

        // Allocate memory for VEBOX
        pMem = (uint8_t*)MOS_AllocAndZeroMemory(uiSize);
        MHW_CHK_NULL_RETURN(pMem);

        m_veboxHeap = (MHW_VEBOX_HEAP*)pMem;

        m_veboxHeap->pStates =
            (MHW_VEBOX_HEAP_STATE*)(pMem + sizeof(MHW_VEBOX_HEAP));

        // Assign offsets and sizes
        uiOffset = 0;
        m_veboxHeap->uiDndiStateOffset = uiOffset;
        uiOffset += m_veboxSettings.uiDndiStateSize;

        m_veboxHeap->uiIecpStateOffset = uiOffset;
        uiOffset += m_veboxSettings.uiIecpStateSize;

        m_veboxHeap->uiGamutStateOffset = uiOffset;
        uiOffset += m_veboxSettings.uiGamutStateSize;

        m_veboxHeap->uiVertexTableOffset = uiOffset;
        uiOffset += m_veboxSettings.uiVertexTableSize;

        m_veboxHeap->uiCapturePipeStateOffset = uiOffset;
        uiOffset += m_veboxSettings.uiCapturePipeStateSize;

        m_veboxHeap->uiGammaCorrectionStateOffset = uiOffset;
        uiOffset += m_veboxSettings.uiGammaCorrectionStateSize;

        m_veboxHeap->uiHdrStateOffset = uiOffset;
        uiOffset += m_veboxSettings.uiHdrStateSize;
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_veboxIdReportEnabled)
        {
            m_veboxHeap->uiEngineDataOffset = uiOffset;
            uiOffset += m_veboxSettings.uiEngineDataSize;
        }
#endif
        m_veboxHeap->uiInstanceSize = uiOffset;

        // Appending VeboxHeap sync data after all vebox heap instances
        m_veboxHeap->uiOffsetSync =
            m_veboxHeap->uiInstanceSize *
            m_veboxSettings.uiNumInstances;

        // Allocate GPU memory
        uiSize = m_veboxHeap->uiInstanceSize *
            m_veboxSettings.uiNumInstances +
            m_veboxSettings.uiSyncSize;

        // for using vdbox copy, the size have to be cache line aligned
        MOS_ALIGN_CEIL(uiSize, MHW_CACHELINE_SIZE);

        m_veboxHeap->uiStateHeapSize = uiSize;

        MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

        AllocParams.Type = MOS_GFXRES_BUFFER;
        AllocParams.TileType = MOS_TILE_LINEAR;
        AllocParams.Format = Format_Buffer;
        AllocParams.dwBytes = uiSize;
        AllocParams.pBufName = "VphalVeboxHeap";
        AllocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF;

        if (MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
        {
            AllocParams.dwMemType = MOS_MEMPOOL_SYSTEMMEMORY;
        }

        MHW_CHK_STATUS_RETURN(this->m_osItf->pfnAllocateResource(
            this->m_osItf,
            &AllocParams,
            &m_veboxHeap->DriverResource));

        if (MEDIA_IS_SKU(skuTable, FtrLimitedLMemBar))
        {
            // Use device memory for vebox heap kernel resource, as no cpu access on it.
            AllocParams.dwMemType = MOS_MEMPOOL_DEVICEMEMORY;
        }
        AllocParams.Flags.bNotLockable = 1;
        MHW_CHK_STATUS_RETURN(this->m_osItf->pfnAllocateResource(
            this->m_osItf,
            &AllocParams,
            &m_veboxHeap->KernelResource));

        // Lock the driver resource
        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

        LockFlags.NoOverWrite = 1;

        m_veboxHeap->pLockedDriverResourceMem =
            (uint8_t*)this->m_osItf->pfnLockResource(
                this->m_osItf,
                &m_veboxHeap->DriverResource,
                &LockFlags);
        MHW_CHK_NULL_RETURN(m_veboxHeap->pLockedDriverResourceMem);

        // Initialize VeboxHeap controls that depend on mapping
        m_veboxHeap->pSync =
            (uint32_t*)(m_veboxHeap->pLockedDriverResourceMem +
                m_veboxHeap->uiOffsetSync);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            DestroyHeap();
        }
        return eStatus;
    }

    MOS_STATUS SetgnLumaWgts(uint32_t lumaStadTh, uint32_t TGNEThCnt, bool tGNEEnable) override
    {
        dw4X4TGNEThCnt = TGNEThCnt;
        bTGNEEnable    = tGNEEnable;
        dwLumaStadTh   = lumaStadTh;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetgnChromaWgts(uint32_t chromaStadTh) override
    {
        dwChromaStadTh = chromaStadTh;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetgnHVSParams(
        bool tGNEEnable, uint32_t lumaStadTh, uint32_t chromaStadTh, 
        uint32_t tGNEThCnt, uint32_t historyInit, bool fallBack) override
    {
        dw4X4TGNEThCnt = tGNEThCnt;
        bTGNEEnable    = tGNEEnable;
        dwLumaStadTh   = lumaStadTh;
        dwChromaStadTh = chromaStadTh;
        bHVSfallback   = fallBack;
        dwHistoryInit  = historyInit;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetgnHVSMode(bool hVSAutoBdrate, bool hVSAutoSubjective, uint32_t bSDThreshold) override
    {
        bHVSAutoBdrateEnable     = hVSAutoBdrate;
        bHVSAutoSubjectiveEnable = hVSAutoSubjective;
        dwBSDThreshold           = bSDThreshold;

        return MOS_STATUS_SUCCESS;
    }


    void RefreshVeboxSync()
    {
        MHW_VEBOX_HEAP              *pVeboxHeap;
        MHW_VEBOX_HEAP_STATE        *pCurInstance;
        uint32_t                    dwCurrentTag;
        int32_t                     i;
        int32_t                     iInstanceInUse;
        MOS_NULL_RENDERING_FLAGS    NullRenderingFlags;

        MHW_FUNCTION_ENTER;
        if (m_veboxHeap == nullptr ||
            this->m_osItf == nullptr)
        {
            MHW_ASSERTMESSAGE("RefreshVeboxSync failed due to m_veboxHeap or m_osInterface is invalid ");
            return;
        }
        iInstanceInUse = 0;

        // Vebox Heap will always be locked by driver
        pVeboxHeap = m_veboxHeap;

        // Most recent tag
        if (this->m_osItf->bEnableKmdMediaFrameTracking)
        {
            dwCurrentTag = this->m_osItf->pfnGetGpuStatusSyncTag(this->m_osItf, MOS_GPU_CONTEXT_VEBOX);
        }
        else
        {
            dwCurrentTag = pVeboxHeap->pSync[0];
        }
        pVeboxHeap->dwSyncTag = dwCurrentTag - 1;

        NullRenderingFlags = this->m_osItf->pfnGetNullHWRenderFlags(
            this->m_osItf);

        // Refresh VeboxHeap states
        pCurInstance = pVeboxHeap->pStates;
        for (i = m_veboxSettings.uiNumInstances; i > 0; i--, pCurInstance++)
        {
            if (!pCurInstance->bBusy) continue;

            // The condition below is valid when sync tag wraps from 2^32-1 to 0
            if (((int32_t)(dwCurrentTag - pCurInstance->dwSyncTag) >= 0) ||
                NullRenderingFlags.VPGobal ||
                NullRenderingFlags.VPDnDi  ||
                this->m_osItf->bNullHwIsEnabled)
            {
                pCurInstance->bBusy = false;
            }
            else
            {
                iInstanceInUse++;
            }
        }

        // Save number of instance in use
        m_veboxHeapInUse = iInstanceInUse;
    }

    MOS_STATUS SetVeboxSurfaceControlBits(
        PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
        uint32_t* pSurfCtrlBits) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxDndiState(
        PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxIecpState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetDisableHistogram(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetAlphaFromStateSelect(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxLaceColorParams(
        MHW_LACE_COLOR_CORRECTION *pLaceColorParams) override
    {
        MHW_CHK_NULL_RETURN(pLaceColorParams);
        MOS_SecureMemcpy(&m_laceColorCorrection, sizeof(MHW_LACE_COLOR_CORRECTION), pLaceColorParams, sizeof(MHW_LACE_COLOR_CORRECTION));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetVeboxChromaParams(
        MHW_VEBOX_CHROMA_PARAMS* chromaParams) override
    {
        MHW_CHK_NULL_RETURN(chromaParams);
        MOS_SecureMemcpy(&m_chromaParams, sizeof(MHW_VEBOX_CHROMA_PARAMS), chromaParams, sizeof(MHW_VEBOX_CHROMA_PARAMS));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AssignVeboxState() override
    {
        uint32_t                dwWaitMs, dwWaitTag;
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
        MHW_VEBOX_HEAP_STATE    *pVeboxCurState;
        MHW_VEBOX_HEAP          *pVeboxHeap;
        uint32_t                uiOffset;

        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_RETURN(m_veboxHeap);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        pVeboxHeap = m_veboxHeap;
        pVeboxCurState = &m_veboxHeap->pStates[pVeboxHeap->uiNextState];

        // Refresh sync tag for all vebox heap instance
        RefreshVeboxSync();

        // Check validity of  current vebox heap instance
        // The code below is unlikely to be executed - unless all Vebox states are in use
        // If this ever happens, please consider increasing the number of media states
        MHW_CHK_NULL_RETURN(pVeboxCurState);
        if (pVeboxCurState->bBusy)
        {
            // Get current vebox instance sync tag
            dwWaitTag = pVeboxCurState->dwSyncTag;

            // Wait for Batch Buffer complete event OR timeout
            for (dwWaitMs = MHW_TIMEOUT_MS_DEFAULT; dwWaitMs > 0; dwWaitMs--)
            {
                uint32_t dwCurrentTag;

                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnWaitForBBCompleteNotifyEvent(
                    this->m_osItf,
                    MOS_GPU_CONTEXT_VEBOX,
                    MHW_EVENT_TIMEOUT_MS));

                if (this->m_osItf->bEnableKmdMediaFrameTracking)
                {
                    dwCurrentTag = this->m_osItf->pfnGetGpuStatusSyncTag(this->m_osItf, MOS_GPU_CONTEXT_VEBOX);
                }
                else
                {
                    dwCurrentTag = pVeboxHeap->pSync[0];
                }
                // Mark current instance status as availabe. Wait if this sync tag came back from GPU
                if ((int32_t)(dwCurrentTag - dwWaitTag) >= 0)
                {
                    pVeboxCurState->bBusy = false;
                    break;
                }
            }

            // Timeout
            if (dwWaitMs == 0)
            {
                MHW_ASSERTMESSAGE("Timeout on waiting for free Vebox Heap.");
                eStatus = MOS_STATUS_UNKNOWN;
                return eStatus;
            }
        }

        // Prepare syncTag for GPU write back
        if (this->m_osItf->bEnableKmdMediaFrameTracking)
        {
            pVeboxCurState->dwSyncTag = this->m_osItf->pfnGetGpuStatusTag(this->m_osItf, MOS_GPU_CONTEXT_VEBOX);
        }
        else
        {
            pVeboxCurState->dwSyncTag = pVeboxHeap->dwNextTag;
        }

        // Assign current state and increase next state
        pVeboxHeap->uiCurState = pVeboxHeap->uiNextState;
        pVeboxHeap->uiNextState = (pVeboxHeap->uiNextState + 1) %
            (m_veboxSettings.uiNumInstances);

        uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_veboxIdReportEnabled)
        {
            if (pVeboxCurState->engineData && MHW_VEBOX_ENGINE_CLASS_ID ==(*pVeboxCurState->engineData).classId)
            {
                // Record old engine instance id before memory clearance when reusing heap state
                m_usedVeboxID |= 1 << (*pVeboxCurState->engineData).instanceId;
            }
            if (nullptr == pVeboxCurState->engineData)
            {
                pVeboxCurState->engineData =
                    (MHW_VEBOX_ENGINE_DATA*)(pVeboxHeap->pLockedDriverResourceMem + uiOffset +
                        pVeboxHeap->uiEngineDataOffset);
            }
        }
#endif
        //Clean the memory of current veboxheap to avoid the history states
        MOS_ZeroMemory(pVeboxHeap->pLockedDriverResourceMem + uiOffset, pVeboxHeap->uiInstanceSize);

        return eStatus;
    }

  MOS_STATUS AdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pCurrSurf,
        uint32_t *pdwSurfaceWidth,
        uint32_t *pdwSurfaceHeight,
        bool      bDIEnable)
    {
        uint16_t   wWidthAlignUnit;
        uint16_t   wHeightAlignUnit;
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(pCurrSurf);
        MHW_CHK_NULL_RETURN(pdwSurfaceWidth);
        MHW_CHK_NULL_RETURN(pdwSurfaceHeight);

        // initialize
        wHeightAlignUnit = 1;
        wWidthAlignUnit  = 1;

        switch (pCurrSurf->Format)
        {
        case Format_NV12:
            wHeightAlignUnit = bDIEnable ? 4 : 2;
            wWidthAlignUnit  = 2;
            break;

        case Format_YUYV:
        case Format_YUY2:
        case Format_UYVY:
        case Format_YVYU:
        case Format_VYUY:
        case Format_Y210:
        case Format_Y216:
            wHeightAlignUnit = bDIEnable ? 2 : 1;
            wWidthAlignUnit  = 2;
            break;

        case Format_AYUV:
        case Format_Y416:
            wHeightAlignUnit = 1;
            wWidthAlignUnit  = 2;
            break;

            // For other formats, we will not do any special alignment
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_L8:
        default:
            break;
        }

        //When Crop being used in vebox, source surface height/width is updated in VeboxAdjustBoundary(), and the rcMaxSrc is used for crop rectangle.
        //But in dynamic Crop case, if the rcMaxSrc is larger than the rcSrc, the input pdwSurfaceHeight/pdwSurfaceWidth will be the input surface size.
        //And if the target surface size is smaller than input surface, it may lead to pagefault issue . So in Vebox Crop case, we set the pdwSurfaceHeight/pdwSurfaceWidth
        //with rcSrc to ensure Vebox input size is same with target Dstrec.
        if (pCurrSurf->bVEBOXCroppingUsed)
        {
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(
                MOS_MIN(pCurrSurf->dwHeight, MOS_MAX((uint32_t)pCurrSurf->rcSrc.bottom, MHW_VEBOX_MIN_HEIGHT)),
                wHeightAlignUnit);
            *pdwSurfaceWidth = MOS_ALIGN_CEIL(
                MOS_MIN(pCurrSurf->dwWidth, MOS_MAX((uint32_t)pCurrSurf->rcSrc.right, MHW_VEBOX_MIN_WIDTH)),
                wWidthAlignUnit);
            MHW_NORMALMESSAGE("bVEBOXCroppingUsed = true, SurfInput.rcSrc.bottom: %d, par.SurfInput.rcSrc.right: %d; pdwSurfaceHeight: %d, pdwSurfaceWidth: %d;",
                (uint32_t)pCurrSurf->rcSrc.bottom,
                (uint32_t)pCurrSurf->rcSrc.right,
                *pdwSurfaceHeight,
                *pdwSurfaceWidth);
            MT_LOG5(MT_VP_MHW_VE_ADJUST_SURFPARAM, MT_NORMAL, MT_VP_RENDER_VE_CROPPING, 1, MT_RECT_BOTTOM, pCurrSurf->rcSrc.bottom, 
                MT_RECT_RIGHT, pCurrSurf->rcSrc.right, MT_SURF_HEIGHT, *pdwSurfaceHeight, MT_SURF_WIDTH, *pdwSurfaceWidth);
        }
        else
        {
            // Align width and height with max src renctange with consideration of
            // these conditions:
            // The minimum of width/height should equal to or larger than
            // MHW_VEBOX_MIN_WIDTH/HEIGHT. The maximum of width/heigh should equal
            // to or smaller than surface width/height
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(
                MOS_MIN(pCurrSurf->dwHeight, MOS_MAX((uint32_t)pCurrSurf->rcMaxSrc.bottom, MHW_VEBOX_MIN_HEIGHT)),
                wHeightAlignUnit);
            *pdwSurfaceWidth = MOS_ALIGN_CEIL(
                MOS_MIN(pCurrSurf->dwWidth, MOS_MAX((uint32_t)pCurrSurf->rcMaxSrc.right, MHW_VEBOX_MIN_WIDTH)),
                wWidthAlignUnit);
            MHW_NORMALMESSAGE("bVEBOXCroppingUsed = false, SurfInput.rcMaxSrc.bottom: %d, SurfInput.rcMaxSrc.right: %d; pdwSurfaceHeight: %d, pdwSurfaceWidth: %d;",
                (uint32_t)pCurrSurf->rcMaxSrc.bottom,
                (uint32_t)pCurrSurf->rcMaxSrc.right,
                *pdwSurfaceHeight,
                *pdwSurfaceWidth);
        }

        return eStatus;
    }

    //!
    //! \brief    Set which vebox can be used by HW
    //! \details  VPHAL set which VEBOX can be use by HW
    //! \param    [in] dwVeboxIndex;
    //!           set which Vebox can be used by HW
    //! \param    [in] dwVeboxCount;
    //!           set Vebox Count
    //! \param    [in] dwUsingSFC;
    //!           set whether using SFC
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxIndex(
        uint32_t dwVeboxIndex,
        uint32_t dwVeboxCount,
        uint32_t dwUsingSFC) override
    {
        MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

        MHW_ASSERT(dwVeboxIndex < dwVeboxCount);

        m_indexofVebox = dwVeboxIndex;
        m_numofVebox = dwVeboxCount;
        m_veboxScalabilityEnabled = (dwVeboxCount > 1) ? m_veboxScalabilitySupported : false;
        m_usingSfc = dwUsingSFC;

        return eStatus;
    }

    MOS_STATUS TraceIndirectStateInfo(MOS_COMMAND_BUFFER& cmdBuffer, MOS_CONTEXT& mosContext, bool isCmBuffer, bool useVeboxHeapKernelResource)
    {
        if (isCmBuffer)
        {
            char ocaLog[] = "Vebox indirect state use CmBuffer";
            HalOcaInterfaceNext::TraceMessage(cmdBuffer, (MOS_CONTEXT_HANDLE)&mosContext, ocaLog, sizeof(ocaLog));
        }
        else
        {
            if (useVeboxHeapKernelResource)
            {
                char ocaLog[] = "Vebox indirect state use KernelResource";
                HalOcaInterfaceNext::TraceMessage(cmdBuffer, (MOS_CONTEXT_HANDLE)&mosContext, ocaLog, sizeof(ocaLog));
            }
            else
            {
                char ocaLog[] = "Vebox indirect state use DriverResource";
                HalOcaInterfaceNext::TraceMessage(cmdBuffer, (MOS_CONTEXT_HANDLE)&mosContext, ocaLog, sizeof(ocaLog));
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    //!
//! \brief    Create Gpu Context for Vebox
//! \details  Create Gpu Context for Vebox
//! \param    [in] pOsInterface
//!           OS interface
//! \param    [in] VeboxGpuContext
//!           Vebox Gpu Context
//! \param    [in] VeboxGpuNode
//!           Vebox Gpu Node
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
    MOS_STATUS CreateGpuContext(
        PMOS_INTERFACE  pOsInterface,
        MOS_GPU_CONTEXT VeboxGpuContext,
        MOS_GPU_NODE    VeboxGpuNode) override
    {
        MEDIA_FEATURE_TABLE *skuTable;
        MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(pOsInterface);

        skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
        MHW_CHK_NULL_RETURN(skuTable);

#if (_DEBUG || _RELEASE_INTERNAL)
        if (MEDIA_IS_SKU(skuTable, FtrContextBasedScheduling) && pOsInterface->bVeboxScalabilityMode)
        {
            pOsInterface->ctxBasedScheduling = true;
        }
#endif

        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
        {
            MOS_GPUCTX_CREATOPTIONS createOption;

            // Create VEBOX/VEBOX2 Context
            MHW_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
                pOsInterface,
                VeboxGpuContext,
                VeboxGpuNode,
                &createOption));
        }
        else
        {
            MOS_GPUCTX_CREATOPTIONS_ENHANCED createOptionenhanced;
            MEDIA_SYSTEM_INFO*               pGtSystemInfo;

            pGtSystemInfo = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
            MHW_CHK_NULL_RETURN(pGtSystemInfo);

            if (pOsInterface->ctxBasedScheduling)
            {
                createOptionenhanced.LRCACount = pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled;
            }
            else
            {
                createOptionenhanced.LRCACount = 1;
                createOptionenhanced.UsingSFC  = true;
            }

            // Create VEBOX/VEBOX2 Context
            MHW_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
                pOsInterface,
                VeboxGpuContext,
                VeboxGpuNode,
                &createOptionenhanced));
        }

        return eStatus;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS ValidateVeboxScalabilityConfig()
    {
        MEDIA_SYSTEM_INFO* pGtSystemInfo = nullptr;
        MOS_FORCE_VEBOX   eForceVebox;
        bool              bScalableVEMode;
        bool              bUseVE1, bUseVE2, bUseVE3, bUseVE4;
        MOS_STATUS        eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(this->m_osItf);

        eForceVebox     = this->m_osItf->eForceVebox;
        bScalableVEMode = ((this->m_osItf->bVeboxScalabilityMode) ? true : false);
        pGtSystemInfo = this->m_osItf->pfnGetGtSystemInfo(this->m_osItf);
        MHW_CHK_NULL_RETURN(pGtSystemInfo);

        if (eForceVebox != MOS_FORCE_VEBOX_NONE &&
            eForceVebox != MOS_FORCE_VEBOX_1 &&
            eForceVebox != MOS_FORCE_VEBOX_2 &&
            eForceVebox != MOS_FORCE_VEBOX_1_2 &&
            eForceVebox != MOS_FORCE_VEBOX_1_2_3 &&
            eForceVebox != MOS_FORCE_VEBOX_1_2_3_4)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("eForceVebox value is invalid.");
            return eStatus;
        }

        if (!bScalableVEMode &&
            (eForceVebox == MOS_FORCE_VEBOX_1_2 ||
                eForceVebox == MOS_FORCE_VEBOX_1_2_3 ||
                eForceVebox == MOS_FORCE_VEBOX_1_2_3_4))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("eForceVebox value is not consistent with scalability mode.");

            return eStatus;
        }

        if (bScalableVEMode && !m_veboxScalabilitySupported)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("scalability mode is not allowed on current platform!");

            return eStatus;
        }

        bUseVE1 = bUseVE2 = bUseVE3 = bUseVE4 = false;
        if (eForceVebox == MOS_FORCE_VEBOX_NONE)
        {
            bUseVE1 = true;
        }
        else
        {
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_1, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE1);
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_2, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE2);
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_3, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE3);
            MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_4, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE4);
        }

        if (!pGtSystemInfo->VEBoxInfo.IsValid ||
            (uint32_t)(bUseVE1 + bUseVE2 + bUseVE3 + bUseVE4) > pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("the forced VEBOX is not enabled in current platform.");
        }

        return eStatus;
    }

    bool IsVeboxIdReportEnabled() override
    {
        return m_veboxIdReportEnabled;
    }

    MOS_STATUS ReportVeboxId() override
    {
        if (m_veboxIdReportEnabled)
        {
            MHW_CHK_NULL_RETURN(m_veboxHeap);
            MHW_CHK_NULL_RETURN(m_veboxHeap->pStates);
            uint32_t reportedUsedID = 0;
            for (uint32_t index = 0; index < m_veboxSettings.uiNumInstances; index++)
            {
                const MHW_VEBOX_HEAP_STATE &curInstance = m_veboxHeap->pStates[index];
                if (curInstance.engineData && MHW_VEBOX_ENGINE_CLASS_ID == (*curInstance.engineData).classId)
                {
                    m_usedVeboxID |= 1 << (*curInstance.engineData).instanceId;
                }
            }
            // It is only used in debugging scenarios for multi-Vebox platforms.
            // No need to add any locks here.
            // This approach can handle all current debugging scenarios with multiple threads/processes.
            ReadUserSettingForDebug(
                m_userSettingPtr,
                reportedUsedID,
                __MEDIA_USER_FEATURE_VALUE_USED_VEBOX_ID,
                MediaUserSetting::Group::Sequence,
                0,
                false,
                MEDIA_USER_SETTING_INTERNAL_REPORT);
            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_USED_VEBOX_ID,
                reportedUsedID | m_usedVeboxID,
                MediaUserSetting::Group::Sequence);
        }
        return MOS_STATUS_SUCCESS;
    }
#endif

    MOS_STATUS VeboxAdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pcurrSurf,
        uint32_t *                pdwSurfaceWidth,
        uint32_t *                pdwSurfaceHeight,
        bool                      bDIEnable) override
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(pdwSurfaceWidth);
        MHW_CHK_NULL_RETURN(pdwSurfaceHeight);

        MHW_CHK_STATUS_RETURN(AdjustBoundary(pcurrSurf, pdwSurfaceWidth, pdwSurfaceHeight, bDIEnable));
        // match the vebox width with sfc input width to fix corruption issue when sfc scalability enabled
        if (m_veboxScalabilityEnabled && m_usingSfc && this->m_osItf->bSimIsActive)
        {
            *pdwSurfaceWidth  = MOS_ALIGN_CEIL(*pdwSurfaceWidth, 16);
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(*pdwSurfaceHeight, 4);
        }

        return eStatus;
    }

    MOS_STATUS FindVeboxGpuNodeToUse(
        PMHW_VEBOX_GPUNODE_LIMIT pGpuNodeLimit) override
    {
        MOS_GPU_NODE VeboxGpuNode = MOS_GPU_NODE_VE;
        MOS_STATUS   eStatus      = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL_RETURN(pGpuNodeLimit);

        // KMD Virtual Engine, use virtual GPU NODE-- MOS_GPU_NODE_VE
        pGpuNodeLimit->dwGpuNodeToUse = VeboxGpuNode;

#if !EMUL
#if (_DEBUG || _RELEASE_INTERNAL)
        if (Mos_Solo_IsInUse(this->m_osItf))
        {
            MHW_CHK_STATUS_RETURN(ValidateVeboxScalabilityConfig());
        }
#endif

        Mos_Solo_CheckNodeLimitation(this->m_osItf, &pGpuNodeLimit->dwGpuNodeToUse);
#endif

        return eStatus;
    }

    MOS_STATUS SetVeboxInUse(
        bool inputVebox0,
        bool inputVebox1)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        m_vebox0InUse = inputVebox0;
        m_vebox1InUse = inputVebox1;

        return eStatus;
    }

    bool IsScalabilitySupported()
    {
        return m_veboxScalabilitySupported;
    }

    MOS_STATUS AddVeboxSurfaces(
        PMOS_COMMAND_BUFFER                pCmdBufferInUse,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams) override
    {
        return MOS_STATUS_SUCCESS;
    }

    bool IsVeboxScalabilitywith4K() override
    {
        return m_veboxScalabilitywith4K;
    }

    MOS_STATUS Add1DLutState(void *&surface, PMHW_1DLUT_PARAMS p1DLutParams) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddFP16State(PMHW_FP16_PARAMS pFP16Params) override
    {
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEBOX_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VEBOX_SURFACE_STATE);
         cmd.DW1.SurfaceIdentification = params.SurfaceIdentification;
         cmd.DW2.Width                 = params.Width;
         cmd.DW2.Height                = params.Height;

         cmd.DW3.HalfPitchForChroma     = params.HalfPitchForChroma;
         cmd.DW3.InterleaveChroma       = params.InterleaveChroma;
         cmd.DW3.SurfaceFormat          = params.SurfaceFormat;
         cmd.DW3.BayerInputAlignment    = params.BayerInputAlignment;
         cmd.DW3.BayerPatternOffset     = params.BayerPatternOffset;
         cmd.DW3.BayerPatternFormat     = params.BayerPatternFormat;
         cmd.DW3.SurfacePitch           = params.SurfacePitch;
         cmd.DW3.TileMode               = params.TileMode;

         cmd.DW4.XOffsetForU    = params.XOffsetForU;
         cmd.DW4.YOffsetForU    = params.YOffsetForU;
         cmd.DW5.XOffsetForV    = params.XOffsetForV;
         cmd.DW5.YOffsetForV    = params.YOffsetForV;
        
         // May fix this for stereo surfaces
         cmd.DW6.YOffsetForFrame = params.YOffsetForFrame;
         cmd.DW6.XOffsetForFrame = params.XOffsetForFrame;

         cmd.DW7.DerivedSurfacePitch                    = params.DerivedSurfacePitch;
         cmd.DW8.SurfacePitchForSkinScoreOutputSurfaces = params.SurfacePitchForSkinScoreOutputSurfaces;

         cmd.DW7.CompressionFormat = params.CompressionFormat;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEBOX_STATE)
    {
        _MHW_SETCMD_CALLBASE(VEBOX_STATE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEBOX_TILING_CONVERT)
    {
        _MHW_SETCMD_CALLBASE(VEBOX_TILING_CONVERT);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VEB_DI_IECP)
    {
        _MHW_SETCMD_CALLBASE(VEB_DI_IECP);

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = Itf;

    bool                      m_veboxScalabilitySupported = false;
    bool                      m_veboxScalabilityEnabled   = false;
    bool                      m_vebox0InUse               = false;
    bool                      m_vebox1InUse               = false;
    uint32_t                  m_indexofVebox              = 0;
    uint32_t                  m_numofVebox                = 1;
    uint32_t                  m_usingSfc                  = 0;
    uint32_t                  m_veboxSplitRatio           = 0;
    MHW_LACE_COLOR_CORRECTION m_laceColorCorrection       = {};
    uint32_t                  m_BT2020InvPixelValue[256]  = {};
    uint32_t                  m_BT2020FwdPixelValue[256]  = {};
    uint32_t                  m_BT2020InvGammaLUT[256]    = {};
    uint32_t                  m_BT2020FwdGammaLUT[256]    = {};
    uint32_t                  dwLumaStadTh                = 3200;
    uint32_t                  dwChromaStadTh              = 1600;
    uint32_t                  dw4X4TGNEThCnt              = 576;
    uint32_t                  dwHistoryInit               = 32;
    uint32_t                  dwBSDThreshold              = 480;
    bool                      bTGNEEnable                 = false;
    bool                      bHVSAutoBdrateEnable        = false;
    bool                      bHVSAutoSubjectiveEnable    = false;
    bool                      bHVSfallback                = false;

    MHW_VEBOX_HEAP            *m_veboxHeap                = nullptr;
    MHW_VEBOX_SETTINGS        m_veboxSettings             = {};
    bool                      m_veboxScalabilitywith4K    = false;
    MHW_VEBOX_CHROMA_PARAMS   m_chromaParams              = {};

    //VPHAL_HVSDENOISE_PARAMS m_HvsParams       = {};
    MHW_VEBOX_DNDI_PARAMS   m_veboxDNDIParams = {};
    MHW_VEBOX_IECP_PARAMS   m_veboxIecpParams = {};
    MHW_VEBOX_CHROMA_SAMPLING m_chromaSampling = {};
    MHW_VEBOX_GAMUT_PARAMS  m_veboxGamutParams = {};

    int                    m_veboxHeapInUse = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                   m_veboxIdReportEnabled = false;
    uint32_t               m_usedVeboxID = 0;
#endif
MEDIA_CLASS_DEFINE_END(mhw__vebox__Impl)
};
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_IMPL_H__
