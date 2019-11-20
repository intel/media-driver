/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file     vphal_render_common.c
//! \brief    The file of common utilities definitions shared by low level renderers
//! \details  Common utilities for different renderers, e.g. DNDI or Comp
//!
#include "vphal_render_common.h"
#include "vphal_render_composite.h"
#include "mos_os.h"
#include "mos_solo_generic.h"
#include "hal_oca_interface.h"

extern const MEDIA_OBJECT_KA2_INLINE_DATA g_cInit_MEDIA_OBJECT_KA2_INLINE_DATA =
{
    // DWORD 0
    {
        0,                                      // DestinationBlockHorizontalOrigin
        0                                       // DestinationBlockVerticalOrigin
    },

    // DWORD 1
    {
        0,                                      // HorizontalBlockCompositeMaskLayer0
        0                                       // VerticalBlockCompositeMaskLayer0
    },

    // DWORD 2
    {
        0,                                      // HorizontalBlockCompositeMaskLayer1
        0                                       // VerticalBlockCompositeMaskLayer1
    },

    // DWORD 3
    {
        0,                                      // HorizontalBlockCompositeMaskLayer2
        0                                       // VerticalBlockCompositeMaskLayer2
    },

    // DWORD 4
    0.0F,                                       // VideoXScalingStep

    // DWORD 5
    0.0F,                                       // VideoStepDelta

    // DWORD 6
    {
        0,                                      // VerticalBlockNumber
        0                                       // AreaOfInterest
    },

    // DWORD 7
    {
        0,                                      // GroupIDNumber
    },

    // DWORD 8
    {
        0,                                      // HorizontalBlockCompositeMaskLayer3
        0                                       // VerticalBlockCompositeMaskLayer3
    },

    // DWORD 9
    {
        0,                                      // HorizontalBlockCompositeMaskLayer4
        0                                       // VerticalBlockCompositeMaskLayer4
    },

    // DWORD 10
    {
        0,                                      // HorizontalBlockCompositeMaskLayer5
        0                                       // VerticalBlockCompositeMaskLayer5
    },

    // DWORD 11
    {
        0,                                      // HorizontalBlockCompositeMaskLayer6
        0                                       // VerticalBlockCompositeMaskLayer6
    },

    // DWORD 12
    {
        0,                                      // HorizontalBlockCompositeMaskLayer7
        0                                       // VerticalBlockCompositeMaskLayer7
    },

    // DWORD 13
    0,                                          // Reserved

    // DWORD 14
    0,                                          // Reserved

    // DWORD 15
    0                                           // Reserved
};

//!
//! \brief    Initialized RenderHal Surface Type
//! \details  Initialized RenderHal Surface Type according to input VPHAL Surface Type
//! \param    [in] vpSurfType
//!           VPHAL surface type
//! \return   RENDERHAL_SURFACE_TYPE
//!
static inline RENDERHAL_SURFACE_TYPE VpHal_RndrInitRenderHalSurfType(VPHAL_SURFACE_TYPE vpSurfType)
{
    switch (vpSurfType)
    {
        case SURF_IN_BACKGROUND:
            return RENDERHAL_SURF_IN_BACKGROUND;

        case SURF_IN_PRIMARY:
            return RENDERHAL_SURF_IN_PRIMARY;

        case SURF_IN_SUBSTREAM:
            return RENDERHAL_SURF_IN_SUBSTREAM;

        case SURF_IN_REFERENCE:
            return RENDERHAL_SURF_IN_REFERENCE;

        case SURF_OUT_RENDERTARGET:
            return RENDERHAL_SURF_OUT_RENDERTARGET;

        case SURF_NONE:
        default:
            return RENDERHAL_SURF_NONE;
    }
}

//!
//! \brief    Initialized RenderHal Scaling Mode
//! \details  Initialized RenderHal Scaling Mode according to input VPHAL Scaling Mode
//! \param    [in] vpScalingMode
//!           VPHAL Scaling Mode
//! \return   RENDERHAL_SCALING_MODE
//!
static inline RENDERHAL_SCALING_MODE VpHal_RndrInitRenderHalScalingMode(VPHAL_SCALING_MODE vpScalingMode)
{
    switch (vpScalingMode)
    {
        case VPHAL_SCALING_NEAREST:
            return RENDERHAL_SCALING_NEAREST;

        case VPHAL_SCALING_BILINEAR:
            return RENDERHAL_SCALING_BILINEAR;

        case VPHAL_SCALING_AVS:
            return RENDERHAL_SCALING_AVS;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid VPHAL_SCALING_MODE %d, force to nearest mode.", vpScalingMode);
            return RENDERHAL_SCALING_NEAREST;
    }
}

//!
//! \brief    Get VpHal Scaling Mode
//! \details  Get VpHal Scaling Mode according to RenderHal Scaling Mode
//! \param    [in] RenderHalScalingMode
//!           RENDERHAL Scaling Mode
//! \return   VPHAL_SCALING_MODE
//!
static inline VPHAL_SCALING_MODE VpHal_RndrGetVpHalScalingMode(RENDERHAL_SCALING_MODE RenderHalScalingMode)
{
    switch (RenderHalScalingMode)
    {
        case RENDERHAL_SCALING_NEAREST:
            return VPHAL_SCALING_NEAREST;

        case RENDERHAL_SCALING_BILINEAR:
            return VPHAL_SCALING_BILINEAR;

        case RENDERHAL_SCALING_AVS:
            return VPHAL_SCALING_AVS;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid RENDERHAL_SCALING_MODE %d, force to nearest mode.", RenderHalScalingMode);
            return VPHAL_SCALING_NEAREST;
    }
}

//!
//! \brief    Initialized RenderHal Sample Type
//! \details  Initialized RenderHal Sample Type according to input VPHAL Sample Type
//! \param    [in] SampleType
//!           VPHAL Sample Type
//! \return   RENDERHAL_SAMPLE_TYPE
//!
static inline RENDERHAL_SAMPLE_TYPE VpHal_RndrInitRenderHalSampleType(VPHAL_SAMPLE_TYPE SampleType)
{
    switch (SampleType)
    {
        case SAMPLE_PROGRESSIVE:
            return RENDERHAL_SAMPLE_PROGRESSIVE;

        case SAMPLE_SINGLE_TOP_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_TOP_FIELD;

        case SAMPLE_SINGLE_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_SINGLE_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;

        case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
            return RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;

        case SAMPLE_INVALID:
        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid VPHAL_SAMPLE_TYPE %d.\n", SampleType);
            return RENDERHAL_SAMPLE_INVALID;
    }
}

static inline void VpHal_RndrInitPlaneOffset(
    PMOS_PLANE_OFFSET pMosPlaneOffset,
    PVPHAL_PLANE_OFFSET pVpPlaneOffset)
{
    pMosPlaneOffset->iSurfaceOffset     = pVpPlaneOffset->iSurfaceOffset;
    pMosPlaneOffset->iXOffset           = pVpPlaneOffset->iXOffset;
    pMosPlaneOffset->iYOffset           = pVpPlaneOffset->iYOffset;
    pMosPlaneOffset->iLockSurfaceOffset = pVpPlaneOffset->iLockSurfaceOffset;

}

//!
//! \brief    Initialized MHW Rotation mode
//! \details  Initialized MHW Rotation mode according to input VPHAL Rotation Type
//! \param    [in] Rotation
//!           VPHAL Rotation mode
//! \return   MHW_ROTATION
//!
static inline MHW_ROTATION VpHal_RndrInitRotationMode(VPHAL_ROTATION Rotation)
{
    MHW_ROTATION    Mode = MHW_ROTATION_IDENTITY;

    switch (Rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            Mode = MHW_ROTATION_IDENTITY;
            break;

        case VPHAL_ROTATION_90:
            Mode = MHW_ROTATION_90;
            break;

        case VPHAL_ROTATION_180:
            Mode = MHW_ROTATION_180;
            break;

        case VPHAL_ROTATION_270:
            Mode = MHW_ROTATION_270;
            break;

        case VPHAL_MIRROR_HORIZONTAL:
            Mode = MHW_MIRROR_HORIZONTAL;
            break;

        case VPHAL_MIRROR_VERTICAL:
            Mode = MHW_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            Mode = MHW_ROTATE_90_MIRROR_VERTICAL;
            break;

        case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
            Mode = MHW_ROTATE_90_MIRROR_HORIZONTAL;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid Rotation Angle.");
            break;
    }

    return Mode;
}

//!
//! \brief    Set the numbers of Slice, Sub-slice, EUs for power mode
//! \details  Set the numbers of Slice, Sub-slice, EUs recommended for
//!           the given kernel type for power mode
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] KernelID
//!           VP render Kernel ID
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_RndrCommonSetPowerMode(
    PRENDERHAL_INTERFACE                pRenderHal,
    VpKernelID                          KernelID)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint16_t                            wNumRequestedEUSlices   = 1;    // Default to 1 slice
    uint16_t                            wNumRequestedSubSlices  = 3;    // Default to 3 subslice
    uint16_t                            wNumRequestedEUs        = 8;    // Default to 8 EUs
    RENDERHAL_POWEROPTION               PowerOption;
    bool                                bSetRequestedSlices     = false;
    const VphalSseuSetting              *pcSSEUTable            = nullptr;

    VPHAL_RENDER_CHK_NULL(pRenderHal);

    if ((pRenderHal->bRequestSingleSlice) || (pRenderHal->bEUSaturationNoSSD))
    {
        bSetRequestedSlices     = true;
        // bEUSaturationNoSSD: No slice shutdown, must request 2 slices [CM EU saturation on].
        // bRequestSingleSlice: Always single slice.
        wNumRequestedEUSlices   = (pRenderHal->bEUSaturationNoSSD) ? 2 : 1;
    }
    else
    {
        bSetRequestedSlices = false;
    }

    if (pRenderHal->sseuTable)
    {
        pcSSEUTable = (const VphalSseuSetting*)pRenderHal->sseuTable;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("SSEU Table not valid.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    VPHAL_RENDER_CHK_NULL(pcSSEUTable);
    pcSSEUTable += KernelID;
    if (!bSetRequestedSlices)                        // If num Slices is already programmed, then don't change it
    {
        if (wNumRequestedEUSlices < pcSSEUTable->numSlices)
        {
            wNumRequestedEUSlices = pcSSEUTable->numSlices;
        }
    }

    wNumRequestedSubSlices  = pcSSEUTable->numSubSlices;
    wNumRequestedEUs        = pcSSEUTable->numEUs;

#if (_DEBUG || _RELEASE_INTERNAL)
    // User feature key reads
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE_ID,
        &UserFeatureData);

    if (UserFeatureData.u32Data != 0xDEADC0DE)
    {
        wNumRequestedEUSlices  = UserFeatureData.u32Data & 0xFF;               // Bits 0-7
        wNumRequestedSubSlices = (UserFeatureData.u32Data >> 8) & 0xFF;        // Bits 8-15
        wNumRequestedEUs       = (UserFeatureData.u32Data >> 16) & 0xFFFF;     // Bits 16-31
    }
#endif

    PowerOption.nSlice          = wNumRequestedEUSlices;
    PowerOption.nSubSlice       = wNumRequestedSubSlices;
    PowerOption.nEU             = wNumRequestedEUs;
    pRenderHal->pfnSetPowerOptionMode(pRenderHal, &PowerOption);

finish:
    return eStatus;
}

//       so will enable other renderers like frc and istab support status report.
//!
//! \brief      Submit commands for rendering
//! \details    Submit commands for rendering. The KMD related fields in pGenericPrologParam might be modified by this
//!             function in order to maintain the synchronization mechanism for resource.
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in] pBatchBuffer
//!             Pointer to batch buffer
//! \param      [in] bNullRendering
//!             Indicate whether is Null rendering
//! \param      [in] pWalkerParams
//!             Pointer to walker parameters
//! \param      [in] pGpGpuWalkerParams
//!             Pointer to GPGPU walker parameters
//! \param      [in] KernelID
//!             VP Kernel ID
//! \param      [in] bLastSubmission
//!             Is last submission
//! \return     MOS_STATUS
//!
MOS_STATUS VpHal_RndrCommonSubmitCommands(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMHW_BATCH_BUFFER                   pBatchBuffer,
    bool                                bNullRendering,
    PMHW_WALKER_PARAMS                  pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS            pGpGpuWalkerParams,
    VpKernelID                          KernelID,
    bool                                bLastSubmission)
{
    PMOS_INTERFACE                      pOsInterface = nullptr;
    MOS_COMMAND_BUFFER                  CmdBuffer = {};
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            dwSyncTag = 0;
    int32_t                             i = 0, iRemaining = 0;
    PMHW_MI_INTERFACE                   pMhwMiInterface = nullptr;
    MhwRenderInterface                  *pMhwRender = nullptr;
    MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = {};
    bool                                bEnableSLM = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS     GenericPrologParams = {};
    MOS_RESOURCE                        GpuStatusBuffer = {};
    MediaPerfProfiler                   *pPerfProfiler = nullptr;
    MOS_CONTEXT                         *pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface->GetMmioRegisters());
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface->pOsContext);

    eStatus             = MOS_STATUS_UNKNOWN;
    pOsInterface        = pRenderHal->pOsInterface;
    pMhwMiInterface     = pRenderHal->pMhwMiInterface;
    pMhwRender          = pRenderHal->pMhwRenderInterface;
    iRemaining          = 0;
    FlushParam          = g_cRenderHal_InitMediaStateFlushParams;
    MOS_ZeroMemory(&CmdBuffer, sizeof(CmdBuffer));
    pPerfProfiler       = pRenderHal->pPerfProfiler;
    pOsContext          = pOsInterface->pOsContext;
    pMmioRegisters      = pMhwRender->GetMmioRegisters();

    // Allocate all available space, unused buffer will be returned later
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));

    // Set initial state
    iRemaining = CmdBuffer.iRemaining;

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonSetPowerMode(
        pRenderHal,
        KernelID));

#ifndef EMUL
    if (bLastSubmission && pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer));

        // Register the buffer
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface, &GpuStatusBuffer, true, true));

        GenericPrologParams.bEnableMediaFrameTracking = true;
        GenericPrologParams.presMediaFrameTrackingSurface = &GpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

    HalOcaInterface::On1stLevelBBStart(CmdBuffer, *pOsContext, pOsInterface->CurrentGpuContextHandle,
        *pRenderHal->pMhwMiInterface, *pMmioRegisters);

    // Add kernel info to log.
    HalOcaInterface::DumpVpKernelInfo(CmdBuffer, *pOsContext, KernelID, 0, nullptr);
    // Add vphal param to log.
    HalOcaInterface::DumpVphalParam(CmdBuffer, *pOsContext, pRenderHal->pVphalOcaDumper);

    // Initialize command buffer and insert prolog
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnInitCommandBuffer(pRenderHal, &CmdBuffer, &GenericPrologParams));

    // Write timing data for 3P budget
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendTimingData(pRenderHal, &CmdBuffer, true));
    VPHAL_RENDER_CHK_STATUS(pPerfProfiler->AddPerfCollectStartCmd((void*)pRenderHal, pOsInterface, pMhwMiInterface, &CmdBuffer));

    bEnableSLM = (pGpGpuWalkerParams && pGpGpuWalkerParams->SLMSize > 0)? true : false;
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetCacheOverrideParams(
        pRenderHal,
        &pRenderHal->L3CacheSettings,
        bEnableSLM));

    // Flush media states
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendMediaStates(
        pRenderHal,
        &CmdBuffer,
        pWalkerParams,
        pGpGpuWalkerParams));

    if (pBatchBuffer)
    {
        // Register batch buffer for rendering
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pBatchBuffer->OsResource,
            false,
            true));

        HalOcaInterface::OnSubLevelBBStart(CmdBuffer, *pOsContext, &pBatchBuffer->OsResource, 0, true, 0);

        // Send Start 2nd level batch buffer command (HW/OS dependent)
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferStartCmd(
            &CmdBuffer,
            pBatchBuffer));
    }

    // Write back GPU Status tag
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendRcsStatusTag(pRenderHal, &CmdBuffer));
    }

    VPHAL_RENDER_CHK_STATUS(pPerfProfiler->AddPerfCollectEndCmd((void*)pRenderHal, pOsInterface, pMhwMiInterface, &CmdBuffer));

    // Write timing data for 3P budget
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendTimingData(pRenderHal, &CmdBuffer, false));

    if (GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform))
    {
        MHW_PIPE_CONTROL_PARAMS PipeControlParams;

        MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
        PipeControlParams.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
        PipeControlParams.bGenericMediaStateClear       = true;
        PipeControlParams.bIndirectStatePointersDisable = true;
        PipeControlParams.bDisableCSStall               = false;
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddPipeControl(&CmdBuffer, nullptr, &PipeControlParams));

        if (MEDIA_IS_WA(pRenderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
        {
            MHW_VFE_PARAMS VfeStateParams = {};
            VfeStateParams.dwNumberofURBEntries = 1;
            VPHAL_RENDER_CHK_STATUS(pMhwRender->AddMediaVfeCmd(&CmdBuffer, &VfeStateParams));
        }
    }

    if (GFX_IS_GEN_8_OR_LATER(pRenderHal->Platform))
    {
        // Add media flush command in case HW not cleaning the media state
        if (MEDIA_IS_WA(pRenderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
        {
            FlushParam.bFlushToGo = true;
            if (pWalkerParams)
            {
                FlushParam.ui8InterfaceDescriptorOffset = pWalkerParams->InterfaceDescriptorOffset;
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("ERROR, pWalkerParams is nullptr and cannot get InterfaceDescriptorOffset.");
            }
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMediaStateFlush(&CmdBuffer, nullptr, &FlushParam));
        }
        else if (MEDIA_IS_WA(pRenderHal->pWaTable, WaAddMediaStateFlushCmd))
        {
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMediaStateFlush(&CmdBuffer, nullptr, &FlushParam));
        }
    }

    HalOcaInterface::On1stLevelBBEnd(CmdBuffer, *pOsContext);

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }
    else if (VpHal_RndrCommonIsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }
    else if (GFX_IS_GEN_8_OR_LATER(pRenderHal->Platform) &&
                Mos_Solo_IsInUse(pOsInterface) &&
                pRenderHal->pOsInterface->bNoParsingAssistanceInKmd)
    {
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);

//    VPHAL_DBG_STATE_DUMPPER_DUMP_COMMAND_BUFFER(pRenderHal, &CmdBuffer);

    // Submit command buffer
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSubmitCommandBuffer(pOsInterface, &CmdBuffer, bNullRendering));

    if (bNullRendering == false)
    {
        dwSyncTag = pRenderHal->pStateHeap->dwNextTag++;

        // Set media state and batch buffer as busy
        pRenderHal->pStateHeap->pCurMediaState->bBusy = true;
        if (pBatchBuffer)
        {
            pBatchBuffer->bBusy     = true;
            pBatchBuffer->dwSyncTag = dwSyncTag;
        }
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    // Failed -> discard all changes in Command Buffer
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Buffer overflow - display overflow size
        if (CmdBuffer.iRemaining < 0)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Command Buffer overflow by %d bytes", -CmdBuffer.iRemaining);
        }

        // Move command buffer back to beginning
        i = iRemaining - CmdBuffer.iRemaining;
        CmdBuffer.iRemaining  = iRemaining;
        CmdBuffer.iOffset    -= i;
        CmdBuffer.pCmdPtr     =
            CmdBuffer.pCmdBase + CmdBuffer.iOffset / sizeof(uint32_t);

        // Return unused command buffer space to OS
        if (pOsInterface)
        {
            pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);
        }
    }

    return eStatus;
}

//!
//! \brief      Submit commands for rendering
//! \details    Submit commands for rendering with status table update enabling
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in] pBatchBuffer
//!             Pointer to batch buffer
//! \param      [in] bNullRendering
//!             Indicate whether is Null rendering
//! \param      [in] pWalkerParams
//!             Pointer to walker parameters
//! \param      [in] pGpGpuWalkerParams
//!             Pointer to GPGPU walker parameters
//! \param      [in] pStatusTableUpdateParams
//!             Pointer to pStatusTableUpdateParams
//! \param      [in] KernelID
//!             VP Kernel ID
//! \param      [in] FcKernelCount
//!             VP FC Kernel Count
//! \param      [in] FcKernelList
//!             VP FC Kernel List
//! \param      [in] bLastSubmission
//!             Is last submission
//! \return     MOS_STATUS
//!
MOS_STATUS VpHal_RndrSubmitCommands(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMHW_BATCH_BUFFER                   pBatchBuffer,
    bool                                bNullRendering,
    PMHW_WALKER_PARAMS                  pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS            pGpGpuWalkerParams,
    PSTATUS_TABLE_UPDATE_PARAMS         pStatusTableUpdateParams,
    VpKernelID                          KernelID,
    int                                 FcKernelCount,
    int                                 *FcKernelList,
    bool                                bLastSubmission)
{
    PMOS_INTERFACE                      pOsInterface = nullptr;
    MOS_COMMAND_BUFFER                  CmdBuffer = {};
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            dwSyncTag = 0;
    int32_t                             i = 0, iRemaining = 0;
    PMHW_MI_INTERFACE                   pMhwMiInterface = nullptr;
    MhwRenderInterface                  *pMhwRender = nullptr;
    MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = {};
    bool                                bEnableSLM = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS     GenericPrologParams = {};
    MOS_RESOURCE                        GpuStatusBuffer = {};
    MediaPerfProfiler                   *pPerfProfiler = nullptr;
    MOS_CONTEXT                         *pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface->GetMmioRegisters());
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface->pOsContext);

    eStatus              = MOS_STATUS_UNKNOWN;
    pOsInterface         = pRenderHal->pOsInterface;
    pMhwMiInterface      = pRenderHal->pMhwMiInterface;
    pMhwRender           = pRenderHal->pMhwRenderInterface;
    iRemaining           = 0;
    FlushParam           = g_cRenderHal_InitMediaStateFlushParams;
    MOS_ZeroMemory(&CmdBuffer, sizeof(CmdBuffer));
    pPerfProfiler       = pRenderHal->pPerfProfiler;
    pOsContext          = pOsInterface->pOsContext;
    pMmioRegisters      = pMhwRender->GetMmioRegisters();

    // Allocate all available space, unused buffer will be returned later
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));

    // Set initial state
    iRemaining = CmdBuffer.iRemaining;

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonSetPowerMode(
        pRenderHal,
        KernelID));

#ifndef EMUL
    if (bLastSubmission && pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer));

        // Register the buffer
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface, &GpuStatusBuffer, true, true));

        GenericPrologParams.bEnableMediaFrameTracking = true;
        GenericPrologParams.presMediaFrameTrackingSurface = &GpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

    HalOcaInterface::On1stLevelBBStart(CmdBuffer, *pOsContext, pOsInterface->CurrentGpuContextHandle,
        *pRenderHal->pMhwMiInterface, *pMmioRegisters);

    // Add kernel info to log.
    HalOcaInterface::DumpVpKernelInfo(CmdBuffer, *pOsContext, KernelID, FcKernelCount, FcKernelList);
    // Add vphal param to log.
    HalOcaInterface::DumpVphalParam(CmdBuffer, *pOsContext, pRenderHal->pVphalOcaDumper);

    // Initialize command buffer and insert prolog
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnInitCommandBuffer(pRenderHal, &CmdBuffer, &GenericPrologParams));

    VPHAL_RENDER_CHK_STATUS(pPerfProfiler->AddPerfCollectStartCmd((void*)pRenderHal, pOsInterface, pMhwMiInterface, &CmdBuffer));

    // Write timing data for 3P budget
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendTimingData(pRenderHal, &CmdBuffer, true));

    bEnableSLM = (pGpGpuWalkerParams && pGpGpuWalkerParams->SLMSize > 0)? true : false;
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetCacheOverrideParams(
        pRenderHal,
        &pRenderHal->L3CacheSettings,
        bEnableSLM));

    if (pRenderHal->bCmfcCoeffUpdate)
    {
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendCscCoeffSurface(pRenderHal,
            &CmdBuffer,
            pRenderHal->pCmfcCoeffSurface,
            pRenderHal->pStateHeap->pKernelAllocation[pRenderHal->iKernelAllocationID].pKernelEntry));
    }

    // Flush media states
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendMediaStates(
        pRenderHal,
        &CmdBuffer,
        pWalkerParams,
        pGpGpuWalkerParams));

    if (pBatchBuffer)
    {
        // Register batch buffer for rendering
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pBatchBuffer->OsResource,
            false,
            true));

        HalOcaInterface::OnSubLevelBBStart(CmdBuffer, *pOsContext, &pBatchBuffer->OsResource, 0, true, 0);

        // Send Start 2nd level batch buffer command (HW/OS dependent)
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferStartCmd(
            &CmdBuffer,
            pBatchBuffer));
    }

    // Write back GPU Status tag
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendRcsStatusTag(pRenderHal, &CmdBuffer));
    }

    VPHAL_RENDER_CHK_STATUS(pPerfProfiler->AddPerfCollectEndCmd((void*)pRenderHal, pOsInterface, pMhwMiInterface, &CmdBuffer));

    // Write timing data for 3P budget
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendTimingData(pRenderHal, &CmdBuffer, false));

    if (GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform))
    {
        MHW_PIPE_CONTROL_PARAMS PipeControlParams;

        MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
        PipeControlParams.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
        PipeControlParams.bGenericMediaStateClear       = true;
        PipeControlParams.bIndirectStatePointersDisable = true;
        PipeControlParams.bDisableCSStall               = false;
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddPipeControl(&CmdBuffer, nullptr, &PipeControlParams));

        if (MEDIA_IS_WA(pRenderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
        {
            MHW_VFE_PARAMS VfeStateParams = {};
            VfeStateParams.dwNumberofURBEntries = 1;
            VPHAL_RENDER_CHK_STATUS(pMhwRender->AddMediaVfeCmd(&CmdBuffer, &VfeStateParams));
        }
    }

    if (GFX_IS_GEN_8_OR_LATER(pRenderHal->Platform))
    {
        // Add media flush command in case HW not cleaning the media state
        if (MEDIA_IS_WA(pRenderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
        {
            FlushParam.bFlushToGo = true;
            if (pWalkerParams)
            {
                FlushParam.ui8InterfaceDescriptorOffset = pWalkerParams->InterfaceDescriptorOffset;
            }
            else
            {
                VPHAL_RENDER_ASSERTMESSAGE("ERROR, pWalkerParams is nullptr and cannot get InterfaceDescriptorOffset.");
            }
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMediaStateFlush(&CmdBuffer, nullptr, &FlushParam));
        }
        else if (MEDIA_IS_WA(pRenderHal->pWaTable, WaAddMediaStateFlushCmd))
        {
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMediaStateFlush(&CmdBuffer, nullptr, &FlushParam));
        }
    }

    HalOcaInterface::On1stLevelBBEnd(CmdBuffer, *pOsContext);

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }
    else if (VpHal_RndrCommonIsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }
    else if (GFX_IS_GEN_8_OR_LATER(pRenderHal->Platform) &&
                Mos_Solo_IsInUse(pOsInterface)  &&
                pRenderHal->pOsInterface->bNoParsingAssistanceInKmd)
    {
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);

//    VPHAL_DBG_STATE_DUMPPER_DUMP_COMMAND_BUFFER(pRenderHal, &CmdBuffer);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pStatusTableUpdateParams->bTriggerGPUHang == true)
    {
        // Set the GPU HANG Trigger flag here
        pOsInterface->bTriggerVPHang              = true;
        pStatusTableUpdateParams->bTriggerGPUHang = false;
    }
#endif

    // Submit command buffer
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSubmitCommandBuffer(pOsInterface, &CmdBuffer, bNullRendering));

    if (bNullRendering == false)
    {
        dwSyncTag = pRenderHal->pStateHeap->dwNextTag++;

        // Set media state and batch buffer as busy
        pRenderHal->pStateHeap->pCurMediaState->bBusy = true;
        if (pBatchBuffer)
        {
            pBatchBuffer->bBusy     = true;
            pBatchBuffer->dwSyncTag = dwSyncTag;
        }
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:

    if (pStatusTableUpdateParams && pOsInterface)
    {
        VpHal_RndrUpdateStatusTableAfterSubmit(pOsInterface, pStatusTableUpdateParams, pOsInterface->CurrentGpuContextOrdinal, eStatus);
    }

    // Failed -> discard all changes in Command Buffer
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Buffer overflow - display overflow size
        if (CmdBuffer.iRemaining < 0)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Command Buffer overflow by %d bytes", -CmdBuffer.iRemaining);
        }

        // Move command buffer back to beginning
        i = iRemaining - CmdBuffer.iRemaining;
        CmdBuffer.iRemaining  = iRemaining;
        CmdBuffer.iOffset    -= i;
        CmdBuffer.pCmdPtr     =
            CmdBuffer.pCmdBase + CmdBuffer.iOffset / sizeof(uint32_t);

        // Return unused command buffer space to OS
        if (pOsInterface)
        {
            pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);
        }
    }

    return eStatus;
}

//!
//! \brief    Initialized RenderHal Surface according to incoming VPHAL Surface
//! \param    [in] pVpSurface
//!           Pointer to the VPHAL surface
//! \param    [out] pRenderHalSurface
//!           Pointer to the RenderHal surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_RndrCommonInitRenderHalSurface(
    PVPHAL_SURFACE          pVpSurface,
    PRENDERHAL_SURFACE      pRenderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    VPHAL_RENDER_CHK_NULL(pVpSurface);
    VPHAL_RENDER_CHK_NULL(pRenderHalSurface);
    //---------------------------------------

    MOS_ZeroMemory(pRenderHalSurface, sizeof(*pRenderHalSurface));

    pRenderHalSurface->OsSurface.OsResource         = pVpSurface->OsResource;
    pRenderHalSurface->OsSurface.dwWidth            = pVpSurface->dwWidth;
    pRenderHalSurface->OsSurface.dwHeight           = pVpSurface->dwHeight;
    pRenderHalSurface->OsSurface.dwPitch            = pVpSurface->dwPitch;
    pRenderHalSurface->OsSurface.Format             = pVpSurface->Format;
    pRenderHalSurface->OsSurface.TileType           = pVpSurface->TileType;
    pRenderHalSurface->OsSurface.TileModeGMM        = pVpSurface->TileModeGMM;
    pRenderHalSurface->OsSurface.bGMMTileEnabled    = pVpSurface->bGMMTileEnabled;
    pRenderHalSurface->OsSurface.dwOffset           = pVpSurface->dwOffset;
    pRenderHalSurface->OsSurface.bIsCompressed      = pVpSurface->bIsCompressed;
    pRenderHalSurface->OsSurface.bCompressible      = pVpSurface->bCompressible;
    pRenderHalSurface->OsSurface.CompressionMode    = pVpSurface->CompressionMode;
    pRenderHalSurface->OsSurface.dwDepth            = pVpSurface->dwDepth;
    pRenderHalSurface->OsSurface.dwQPitch           = pVpSurface->dwHeight;
    pRenderHalSurface->OsSurface.MmcState           = (MOS_MEMCOMP_STATE)pVpSurface->CompressionMode;
    pRenderHalSurface->OsSurface.CompressionFormat  = pVpSurface->CompressionFormat;

    VpHal_RndrInitPlaneOffset(
        &pRenderHalSurface->OsSurface.YPlaneOffset,
        &pVpSurface->YPlaneOffset);
    VpHal_RndrInitPlaneOffset(
        &pRenderHalSurface->OsSurface.UPlaneOffset,
        &pVpSurface->UPlaneOffset);
    VpHal_RndrInitPlaneOffset(
        &pRenderHalSurface->OsSurface.VPlaneOffset,
        &pVpSurface->VPlaneOffset);

    pRenderHalSurface->rcSrc                        = pVpSurface->rcSrc;
    pRenderHalSurface->rcDst                        = pVpSurface->rcDst;
    pRenderHalSurface->rcMaxSrc                     = pVpSurface->rcMaxSrc;
    pRenderHalSurface->SurfType                     =
                    VpHal_RndrInitRenderHalSurfType(pVpSurface->SurfType);
    pRenderHalSurface->ScalingMode                  =
                    VpHal_RndrInitRenderHalScalingMode(pVpSurface->ScalingMode);
    pRenderHalSurface->ChromaSiting                 = pVpSurface->ChromaSiting;

    if (pVpSurface->pDeinterlaceParams != nullptr)
    {
        pRenderHalSurface->bDeinterlaceEnable       = true;
    }
    else
    {
        pRenderHalSurface->bDeinterlaceEnable       = false;
    }

    pRenderHalSurface->iPaletteID                   = pVpSurface->iPalette;

    pRenderHalSurface->bQueryVariance               = pVpSurface->bQueryVariance;
    pRenderHalSurface->bInterlacedScaling           = pVpSurface->bInterlacedScaling;
    pRenderHalSurface->pDeinterlaceParams           = (void *)pVpSurface->pDeinterlaceParams;
    pRenderHalSurface->SampleType                   =
                    VpHal_RndrInitRenderHalSampleType(pVpSurface->SampleType);

    pRenderHalSurface->Rotation                     =
                    VpHal_RndrInitRotationMode(pVpSurface->Rotation);

finish:
    return eStatus;
}

//!
//! \brief    Get output RenderHal Surface parameters back to VPHAL Surface
//! \param    [in] pRenderHalSurface
//!           Pointer to the RenderHal surface
//! \param    [in,out] pVpSurface
//!           Pointer to the VPHAL surface
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_RndrCommonGetBackVpSurfaceParams(
    PRENDERHAL_SURFACE      pRenderHalSurface,
    PVPHAL_SURFACE          pVpSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    VPHAL_RENDER_CHK_NULL(pVpSurface);
    VPHAL_RENDER_CHK_NULL(pRenderHalSurface);
    //---------------------------------------

    // The following params are mostly for b32MWColorFillKern on Gen75/Gen8 only
    pVpSurface->dwHeight                            = pRenderHalSurface->OsSurface.dwHeight;
    pVpSurface->dwPitch                             = pRenderHalSurface->OsSurface.dwPitch;
    pVpSurface->Format                              = pRenderHalSurface->OsSurface.Format;
    pVpSurface->dwOffset                            = pRenderHalSurface->OsSurface.dwOffset;
    pVpSurface->YPlaneOffset.iXOffset               = pRenderHalSurface->OsSurface.YPlaneOffset.iXOffset;
    pVpSurface->YPlaneOffset.iYOffset               = pRenderHalSurface->OsSurface.YPlaneOffset.iYOffset;
    pVpSurface->UPlaneOffset.iSurfaceOffset         = pRenderHalSurface->OsSurface.UPlaneOffset.iSurfaceOffset;
    pVpSurface->VPlaneOffset.iSurfaceOffset         = pRenderHalSurface->OsSurface.VPlaneOffset.iSurfaceOffset;
    pVpSurface->rcDst                               = pRenderHalSurface->rcDst;

    pVpSurface->dwWidth                             = pRenderHalSurface->OsSurface.dwWidth;
    pVpSurface->ScalingMode                         =
                    VpHal_RndrGetVpHalScalingMode(pRenderHalSurface->ScalingMode);

finish:
    return eStatus;
}

//!
//! \brief    Set Surface for HW Access
//! \details  Common Function for setting up surface state, if render would 
//!           use CP HM, need use VpHal_CommonSetSurfaceForHwAccess instead
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to bind surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_RndrCommonSetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{

    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                         iSurfaceEntries;
    RENDERHAL_SURFACE               RenderHalSurface;
    int32_t                         i;
    MOS_STATUS                      eStatus;

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    if (pOsInterface->osCpInterface->IsHMEnabled())
    {
        VPHAL_RENDER_ASSERTMESSAGE("ERROR, need to use VpHal_CommonSetSurfaceForHwAccess if under CP HM.");
    }

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonInitRenderHalSurface(
        pSurface,
        &RenderHalSurface));

    // Setup surface states-----------------------------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        &RenderHalSurface,
        pSurfaceParams,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr));

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonGetBackVpSurfaceParams(
        &RenderHalSurface,
        pSurface));

    // Bind surface states------------------------------------------------------
    for (i = 0; i < iSurfaceEntries; i++, iBTEntry++)
    {
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            iBTEntry,
            pSurfaceEntries[i]));
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Buffer Surface for HW Access
//! \details  Common Function for setting up buffer surface state, if render would 
//!           use CP HM, need use VpHal_CommonSetBufferSurfaceForHwAccess instead
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in,out] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to Bind Surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_RndrCommonSetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{
    PMOS_INTERFACE                      pOsInterface;
    RENDERHAL_SURFACE                   RenderHalSurface;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntry;
    MOS_STATUS                          eStatus;

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    if (pOsInterface->osCpInterface->IsHMEnabled())
    {
        VPHAL_RENDER_ASSERTMESSAGE("ERROR, need to use VpHal_CommonSetBufferSurfaceForHwAccess if under CP HM.");
    }

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    // Setup Buffer surface-----------------------------------------------------
    if (pSurfaceParams == nullptr)
    {
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
        pSurfaceParams = &SurfaceParam;
    }

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonInitRenderHalSurface(
        pSurface,
        &RenderHalSurface));

    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetupBufferSurfaceState(
        pRenderHal,
        &RenderHalSurface,
        pSurfaceParams,
        &pSurfaceEntry));

    // Bind surface state-------------------------------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
        pRenderHal,
        iBindingTable,
        iBTEntry,
        pSurfaceEntry));

finish:
    return eStatus;
}

//!
//! \brief    Set Surface for HW Access for CP HM
//! \details  Common Function for setting up surface state, need to use this function
//!           if render would use CP HM 
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to bind surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_CommonSetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{

    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                         iSurfaceEntries;
    int32_t                         i;
    MOS_STATUS                      eStatus;

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonInitRenderHalSurface(
        pSurface,
        pRenderSurface));

    // Setup surface states-----------------------------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        pRenderSurface,
        pSurfaceParams,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr));

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonGetBackVpSurfaceParams(
        pRenderSurface,
        pSurface));

    // Bind surface states------------------------------------------------------
    for (i = 0; i < iSurfaceEntries; i++, iBTEntry++)
    {
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            iBTEntry,
            pSurfaceEntries[i]));
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Buffer Surface for HW Access for CP HM
//! \details  Common Function for setting up buffer surface state, need to use this function
//!           if render would use CP HM
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in,out] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to Bind Surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_CommonSetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{
    PMOS_INTERFACE                      pOsInterface;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntry;
    MOS_STATUS                          eStatus;

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    // Setup Buffer surface-----------------------------------------------------
    if (pSurfaceParams == nullptr)
    {
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
        pSurfaceParams = &SurfaceParam;
    }

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonInitRenderHalSurface(
        pSurface,
        pRenderSurface));

    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetupBufferSurfaceState(
        pRenderHal,
        pRenderSurface,
        pSurfaceParams,
        &pSurfaceEntry));

    // Bind surface state-------------------------------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
        pRenderHal,
        iBindingTable,
        iBTEntry,
        pSurfaceEntry));

finish:
    return eStatus;
}

//!
//! \brief      Is Alignment WA needed
//! \details    Decide WA is needed for VEBOX/Render engine
//!             RENDER limitation with composition BOB:
//!             Height should be a multiple of 4, otherwise disable DI in Comp
//!             VEBOX limitation with TiledY NV12 input(Gen75):
//!             Height should be a multiple of 4, otherwise bypass adv render
//!             VEBOX limitation with TiledY NV12 input(Gen8/9):
//!             3D/GMM regresses to allocate linear surface when height is not
//!             a multiple of 4, no need to bypass adv render
//! \param      [in] pSurface
//!             Input surface
//! \param      [in] GpuContext
//!             GpuContext to indicate Render/Vebox
//! \return     bool
//!             true - Solution is needed; false - Solution is not needed
//!
bool VpHal_RndrCommonIsAlignmentWANeeded(
    PVPHAL_SURFACE             pSurface,
    MOS_GPU_CONTEXT            GpuContext)
{
    bool bRetVal;

    switch (GpuContext)
    {
        case MOS_GPU_CONTEXT_RENDER:
        case MOS_GPU_CONTEXT_VEBOX:
        case MOS_GPU_CONTEXT_RENDER3:
        case MOS_GPU_CONTEXT_RENDER4:
        case MOS_GPU_CONTEXT_COMPUTE:
        case MOS_GPU_CONTEXT_COMPUTE_RA:
        case MOS_GPU_CONTEXT_RENDER_RA:
            if (!(MOS_IS_ALIGNED(MOS_MIN((uint32_t)pSurface->dwHeight, (uint32_t)pSurface->rcMaxSrc.bottom), 4)) &&
                (pSurface->Format == Format_NV12))
            {
                bRetVal = true;
            }
            else
            {
                bRetVal = false;
            }
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Incorrect GPU context: %d.", GpuContext);
            bRetVal = false;
            break;
    }

    return bRetVal;
}

//!
//! \brief      Sets AVS table
//! \details    Set 4-tap or 8-tap filtering AVS table
//! \param      [in] pAvsParams
//!             Pointer to avs parameter
//! \param      [in,out] pMhwSamplerAvsTableParam
//!             Pointer to avs table parameter
//! \return     MOS_STATUS
//!
MOS_STATUS VpHal_RenderCommonSetAVSTableParam(
    PMHW_AVS_PARAMS              pAvsParams,
    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    int32_t        iCoeffTableIdx;
    int32_t        iFilterCoeffIdx;
    int32_t*       piYCoefsX;
    int32_t*       piYCoefsY;
    int32_t*       piUVCoefsX;
    int32_t*       piUVCoefsY;

    VPHAL_RENDER_CHK_NULL(pAvsParams);
    VPHAL_RENDER_CHK_NULL(pMhwSamplerAvsTableParam);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsX);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsX);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsY);

    piYCoefsX  = pAvsParams->piYCoefsX;
    piYCoefsY  = pAvsParams->piYCoefsY;
    piUVCoefsX = pAvsParams->piUVCoefsX;
    piUVCoefsY = pAvsParams->piUVCoefsY;

    for (iCoeffTableIdx = 0; iCoeffTableIdx < MHW_NUM_HW_POLYPHASE_TABLES; iCoeffTableIdx++)
    {
        PMHW_AVS_COEFFICIENT_PARAM pCoeffTable = &pMhwSamplerAvsTableParam->paMhwAvsCoeffParam[iCoeffTableIdx];
        // 4-tap filtering for G-channel, update only center 4 coeffs.
        if (pMhwSamplerAvsTableParam->b4TapGY)
        {
            pCoeffTable->ZeroXFilterCoefficient[0] = 0;
            pCoeffTable->ZeroXFilterCoefficient[1] = 0;
            pCoeffTable->ZeroXFilterCoefficient[2] = (int8_t)*(piYCoefsX++);
            pCoeffTable->ZeroXFilterCoefficient[3] = (int8_t)*(piYCoefsX++);
            pCoeffTable->ZeroXFilterCoefficient[4] = (int8_t)*(piYCoefsX++);
            pCoeffTable->ZeroXFilterCoefficient[5] = (int8_t)*(piYCoefsX++);
            pCoeffTable->ZeroXFilterCoefficient[6] = 0;
            pCoeffTable->ZeroXFilterCoefficient[7] = 0;

            pCoeffTable->ZeroYFilterCoefficient[0] = 0;
            pCoeffTable->ZeroYFilterCoefficient[1] = 0;
            pCoeffTable->ZeroYFilterCoefficient[2] = (int8_t)*(piYCoefsY++);
            pCoeffTable->ZeroYFilterCoefficient[3] = (int8_t)*(piYCoefsY++);
            pCoeffTable->ZeroYFilterCoefficient[4] = (int8_t)*(piYCoefsY++);
            pCoeffTable->ZeroYFilterCoefficient[5] = (int8_t)*(piYCoefsY++);
            pCoeffTable->ZeroYFilterCoefficient[6] = 0;
            pCoeffTable->ZeroYFilterCoefficient[7] = 0;
        }
        else // for gen8+, using 8-tap filter
        {
            for (iFilterCoeffIdx = 0; iFilterCoeffIdx < 8; iFilterCoeffIdx++)
            {
                pCoeffTable->ZeroXFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piYCoefsX++);
                pCoeffTable->ZeroYFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piYCoefsY++);
            }
        }

        // If 8-tap adaptive filter is enabled, then UV/RB will share the same coefficients with Y/G
        if (pMhwSamplerAvsTableParam->b4TapRBUV)
        {
            // [0..3] maps to filter coefficients [2..5], in actual table [0..1] are reserved
            for (iFilterCoeffIdx = 0; iFilterCoeffIdx < 4; iFilterCoeffIdx++)
            {
                pCoeffTable->OneXFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piUVCoefsX++);
                pCoeffTable->OneYFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piUVCoefsY++);
            }
        }
    }

    if (pMhwSamplerAvsTableParam->bIsCoeffExtraEnabled)
    {
        for (iCoeffTableIdx = 0; iCoeffTableIdx < MHW_NUM_HW_POLYPHASE_EXTRA_TABLES_G9; iCoeffTableIdx++)
        {
            PMHW_AVS_COEFFICIENT_PARAM pCoeffTable = &pMhwSamplerAvsTableParam->paMhwAvsCoeffParamExtra[iCoeffTableIdx];
            // 4-tap filtering for G-channel, update only center 4 coeffs.
            if (pMhwSamplerAvsTableParam->b4TapGY)
            {
                pCoeffTable->ZeroXFilterCoefficient[0] = 0;
                pCoeffTable->ZeroXFilterCoefficient[1] = 0;
                pCoeffTable->ZeroXFilterCoefficient[2] = (int8_t)*(piYCoefsX++);
                pCoeffTable->ZeroXFilterCoefficient[3] = (int8_t)*(piYCoefsX++);
                pCoeffTable->ZeroXFilterCoefficient[4] = (int8_t)*(piYCoefsX++);
                pCoeffTable->ZeroXFilterCoefficient[5] = (int8_t)*(piYCoefsX++);
                pCoeffTable->ZeroXFilterCoefficient[6] = 0;
                pCoeffTable->ZeroXFilterCoefficient[7] = 0;

                pCoeffTable->ZeroYFilterCoefficient[0] = 0;
                pCoeffTable->ZeroYFilterCoefficient[1] = 0;
                pCoeffTable->ZeroYFilterCoefficient[2] = (int8_t)*(piYCoefsY++);
                pCoeffTable->ZeroYFilterCoefficient[3] = (int8_t)*(piYCoefsY++);
                pCoeffTable->ZeroYFilterCoefficient[4] = (int8_t)*(piYCoefsY++);
                pCoeffTable->ZeroYFilterCoefficient[5] = (int8_t)*(piYCoefsY++);
                pCoeffTable->ZeroYFilterCoefficient[6] = 0;
                pCoeffTable->ZeroYFilterCoefficient[7] = 0;
            }
            else
            {
                for (iFilterCoeffIdx = 0; iFilterCoeffIdx < 8; iFilterCoeffIdx++)
                {
                    pCoeffTable->ZeroXFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piYCoefsX++);
                    pCoeffTable->ZeroYFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piYCoefsY++);
                }
            }

            // If 8-tap adaptive filter is enabled, then UV/RB will share the same coefficients with Y/G
            if (pMhwSamplerAvsTableParam->b4TapRBUV)
            {
                for (iFilterCoeffIdx = 0; iFilterCoeffIdx < 4; iFilterCoeffIdx++)
                {
                    // [0..3] maps to filter coefficients [2..5], in actual table [0..1] are reserved
                    pCoeffTable->OneXFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piUVCoefsX++);
                    pCoeffTable->OneYFilterCoefficient[iFilterCoeffIdx] = (int8_t)*(piUVCoefsY++);
                }
            }
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Initialize AVS parameters shared by Renderers
//! \details  Initialize the members of the AVS parameter and allocate memory for its coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to MHW AVS parameter
//! \param    [in] uiYCoeffTableSize
//!           Size of the Y coefficient table
//! \param    [in] uiUVCoeffTableSize
//!           Size of the UV coefficient table
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrCommonInitAVSParams(
    PMHW_AVS_PARAMS     pAVS_Params,
    uint32_t            uiYCoeffTableSize,
    uint32_t            uiUVCoeffTableSize)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    int32_t     size;
    char*       ptr;

    VPHAL_RENDER_CHK_NULL(pAVS_Params);
    VPHAL_RENDER_CHK_NULL((void*)(uiYCoeffTableSize > 0));
    VPHAL_RENDER_CHK_NULL((void*)(uiUVCoeffTableSize > 0));

    // Init AVS parameters
    pAVS_Params->Format    = Format_None;
    pAVS_Params->fScaleX   = 0.0F;
    pAVS_Params->fScaleY   = 0.0F;
    pAVS_Params->piYCoefsX = nullptr;

    // Allocate AVS coefficients, One set each for X and Y
    size = (uiYCoeffTableSize + uiUVCoeffTableSize) * 2;

    ptr = (char*)MOS_AllocAndZeroMemory(size);
    if (ptr == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("No memory to allocate AVS coefficient tables.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pAVS_Params->piYCoefsX = (int32_t*)ptr;

    ptr += uiYCoeffTableSize;
    pAVS_Params->piUVCoefsX = (int32_t*)ptr;

    ptr += uiUVCoeffTableSize;
    pAVS_Params->piYCoefsY  = (int32_t*)ptr;

    ptr += uiYCoeffTableSize;
    pAVS_Params->piUVCoefsY = (int32_t*)ptr;

finish:
    return eStatus;
}

//!
//! \brief    Destroy AVS parameters shared by Renderers
//! \details  Free the memory of AVS parameter's coefficient tables
//! \param    [in,out] pAVS_Params
//!           Pointer to VPHAL AVS parameter
//! \return   void
//!
void VpHal_RndrCommonDestroyAVSParams(
    PMHW_AVS_PARAMS   pAVS_Params)
{
    VPHAL_RENDER_ASSERT(pAVS_Params);
    MOS_SafeFreeMemory(pAVS_Params->piYCoefsX);
    pAVS_Params->piYCoefsX = nullptr;
}

//!
//! \brief    update status report rely on command buffer sync tag
//! \param    [in] pOsInterface
//!           pointer to os interface
//! \param    [in,out] pStatusTableUpdateParams
//!           pointer to STATUS_TABLE_UPDATE_PARAMS for updating status table
//! \param    [in] eMosGpuContext
//!           current mos contexnt enum
//! \param    [in] eLastStatus
//!           indicating last submition is successful or not
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrUpdateStatusTableAfterSubmit(
    PMOS_INTERFACE                  pOsInterface,
    PSTATUS_TABLE_UPDATE_PARAMS     pStatusTableUpdateParams,
    MOS_GPU_CONTEXT                 eMosGpuContext,
    MOS_STATUS                      eLastStatus)
{
    PVPHAL_STATUS_ENTRY             pStatusEntry;
    bool                            bEmptyTable;
    MOS_STATUS                      eStatus;
    uint32_t                        dwLastTag;
    PVPHAL_STATUS_TABLE             pStatusTable;
    uint32_t                        dwStatusFeedBackID;

    VPHAL_RENDER_CHK_NULL(pStatusTableUpdateParams);
    eStatus = MOS_STATUS_SUCCESS;

    if (!pStatusTableUpdateParams->bReportStatus ||
        !pStatusTableUpdateParams->bSurfIsRenderTarget)
    {
        goto finish;
    }

    VPHAL_RENDER_CHK_NULL(pStatusTableUpdateParams->pStatusTable);
    VPHAL_RENDER_CHK_NULL(pOsInterface);

    pStatusTable       = pStatusTableUpdateParams->pStatusTable;
    dwStatusFeedBackID = pStatusTableUpdateParams->StatusFeedBackID;

    VPHAL_RENDER_ASSERT(pStatusTable->uiHead < VPHAL_STATUS_TABLE_MAX_SIZE);
    VPHAL_RENDER_ASSERT(pStatusTable->uiCurrent < VPHAL_STATUS_TABLE_MAX_SIZE);

    bEmptyTable = (pStatusTable->uiCurrent == pStatusTable->uiHead);
    if (!bEmptyTable)
    {
        uint32_t uiLast                 = (pStatusTable->uiCurrent - 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        bool bSameFrameIdWithLastRender = (pStatusTable->aTableEntries[uiLast].StatusFeedBackID == dwStatusFeedBackID);
        if (bSameFrameIdWithLastRender)
        {
            pStatusTable->uiCurrent = uiLast;
        }
    }

    pStatusEntry                    = &pStatusTable->aTableEntries[pStatusTable->uiCurrent];
    pStatusEntry->StatusFeedBackID  = dwStatusFeedBackID;
    pStatusEntry->GpuContextOrdinal = eMosGpuContext;
    dwLastTag                       = pOsInterface->pfnGetGpuStatusTag(pOsInterface, eMosGpuContext) - 1;
    pStatusEntry->dwTag             = dwLastTag;
    pStatusEntry->dwStatus          = (eLastStatus == MOS_STATUS_SUCCESS)? VPREP_NOTREADY : VPREP_ERROR;
    pStatusTable->uiCurrent         = (pStatusTable->uiCurrent + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);

    // CM may use a different streamIndex, record it here
    if (pStatusTableUpdateParams->bUpdateStreamIndex)
    {
        pStatusEntry->isStreamIndexSet = true;
        pStatusEntry->streamIndex = (uint16_t)pOsInterface->streamIndex;
    }
    else
    {
        pStatusEntry->isStreamIndexSet = false;
    }

finish:
    return eStatus;
}
