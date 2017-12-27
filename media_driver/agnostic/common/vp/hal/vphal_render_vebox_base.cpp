/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     vphal_render_vebox_base.cpp
//! \brief    Common interface used in Vebox
//! \details  Common interface used in Vebox which are platform independent
//!

#include "vphal.h"
#include "vphal_render_vebox_base.h"
#include "vphal_debug.h"
#include "vphal_renderer.h"
#include "vphal_render_vebox_util_base.h"

#include "vphal_render_common.h"
#include "renderhal_platform_interface.h"

extern const Kdll_Layer g_cSurfaceType_Layer[];

extern const VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION g_cInit_VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION =
{
    // DWORD 0
    {
        {NOISE_BLF_RANGE_THRESHOLD_S0_DEFAULT},           // RangeThrStart0
    },

    // DWORD 1
    {
        {NOISE_BLF_RANGE_THRESHOLD_S1_DEFAULT},           // RangeThrStart1
    },

    // DWORD 2
    {
        {NOISE_BLF_RANGE_THRESHOLD_S2_DEFAULT},           // RangeThrStart2
    },

    // DWORD 3
    {
        {NOISE_BLF_RANGE_THRESHOLD_S3_DEFAULT},           // RangeThrStart3
    },

    // DWORD 4
    {
        {NOISE_BLF_RANGE_THRESHOLD_S4_DEFAULT},           // RangeThrStart4
    },

    // DWORD 5
    {
        {NOISE_BLF_RANGE_THRESHOLD_S5_DEFAULT},           // RangeThrStart5
    },

    // DWORD 6
    {
        {0},                                              // Reserved
    },

    // DWORD 7
    {
        {0},                                              // Reserved
    },

    // DWORD 8
    {
        {NOISE_BLF_RANGE_WGTS0_DEFAULT},                  // RangeWgt0
    },

    // DWORD 9
    {
        {NOISE_BLF_RANGE_WGTS1_DEFAULT},                  // RangeWgt1
    },

    // DWORD 10
    {
        {NOISE_BLF_RANGE_WGTS2_DEFAULT},                  // RangeWgt2
    },

    // DWORD 11
    {
        {NOISE_BLF_RANGE_WGTS3_DEFAULT},                  // RangeWgt3
    },

    // DWORD 12
    {
        {NOISE_BLF_RANGE_WGTS4_DEFAULT},                  // RangeWgt4
    },

    // DWORD 13
    {
        {NOISE_BLF_RANGE_WGTS5_DEFAULT},                  // RangeWgt5
    },

    // DWORD 14
    {
        {0},                                              // Reserved
    },

    // DWORD 15
    {
        {0},                                              // Reserved
    },

    // DWORD 16 - 41: DistWgt[5][5]
    {
        {NOISE_BLF_DISTANCE_WGTS00_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS02_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS00_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS10_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS12_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS10_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS20_DEFAULT, NOISE_BLF_DISTANCE_WGTS21_DEFAULT, NOISE_BLF_DISTANCE_WGTS22_DEFAULT, NOISE_BLF_DISTANCE_WGTS21_DEFAULT, NOISE_BLF_DISTANCE_WGTS20_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS10_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS12_DEFAULT, NOISE_BLF_DISTANCE_WGTS11_DEFAULT, NOISE_BLF_DISTANCE_WGTS10_DEFAULT},
        {NOISE_BLF_DISTANCE_WGTS00_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS02_DEFAULT, NOISE_BLF_DISTANCE_WGTS01_DEFAULT, NOISE_BLF_DISTANCE_WGTS00_DEFAULT},
    },

    // Padding
    {
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
        0,                                      // Padding
    }
};

//!
//! \brief    Send Vecs Status Tag
//! \details  Add MI Flush with write back into command buffer for GPU to write 
//!           back GPU Tag. This should be the last command in 1st level batch.
//!           This ensures sync tag will be written after rendering is complete.
//! \param    [in] pMhwMiInterface
//!           MHW MI interface
//! \param    [in] pOsInterface
//!           Pointer to OS Interface
//! \param    [out] pCmdBuffer
//!           Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSendVecsStatusTag(
    PMHW_MI_INTERFACE                   pMhwMiInterface,
    PMOS_INTERFACE                      pOsInterface,
    PMOS_COMMAND_BUFFER                 pCmdBuffer)
{
    MOS_RESOURCE                        GpuStatusBuffer;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    VPHAL_RENDER_CHK_NULL(pMhwMiInterface);
    VPHAL_RENDER_CHK_NULL(pOsInterface);
    VPHAL_RENDER_CHK_NULL(pCmdBuffer);
    //------------------------------------

#if EMUL
    // Dummy function for VpSolo, since no sync b/w GPU contexts
    goto finish;
#endif

    // Get GPU Status buffer
    pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer);

    // Register the buffer
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &GpuStatusBuffer,
        true,
        true));

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    FlushDwParams.pOsResource       = &GpuStatusBuffer;
    FlushDwParams.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    FlushDwParams.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiFlushDwCmd(
        pCmdBuffer,
        &FlushDwParams));

    // Increase buffer tag for next usage
    pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

finish:
    return eStatus;
}

//!
//! \brief    Initialize VEBOX state
//! \param    [in] pSettings
//!           Pointer to VPHAL settings
//! \param    [in] pKernelDllState
//!           Pointer to KDLL state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::Initialize(
    const VphalSettings         *pSettings,
    Kdll_State                  *pKernelDllState)
{
    MOS_STATUS                          eStatus;
    PRENDERHAL_INTERFACE                pRenderHal;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    MOS_NULL_RENDERING_FLAGS            NullRenderingFlags;
    PVPHAL_VEBOX_STATE                  pVeboxState = this;

    eStatus      = MOS_STATUS_SUCCESS;
    pRenderHal   = pVeboxState->m_pRenderHal;

    if (m_reporting == nullptr)
    {
        m_reporting = MOS_New(VphalFeatureReport);
    }

    if (pRenderHal == nullptr)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    if (!m_currentSurface)
    {
        m_currentSurface = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        if (!m_currentSurface)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }

    if (!m_previousSurface)
    {
        m_previousSurface = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        if (!m_previousSurface)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }
    
    for (uint32_t i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
    {
        if (!FFDNSurfaces[i])
        {
            FFDNSurfaces[i] = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
            if (!FFDNSurfaces[i])
            {
                return MOS_STATUS_NO_SPACE;
            }
        }
    }

    for (uint32_t i = 0; i < VPHAL_MAX_NUM_FFDI_SURFACES; i++)
    {
        if (!FFDISurfaces[i])
        {
            FFDISurfaces[i] = (VPHAL_SURFACE*)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
            if (!FFDISurfaces[i])
            {
                return MOS_STATUS_NO_SPACE;
            }
        }
    }

    // Initial IECP modules
    if (!m_IECP)
    {
        m_IECP = MOS_New(VPHAL_VEBOX_IECP_RENDERER);
        if (!m_IECP)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }
    m_IECP->m_veboxState = this;
    m_IECP->m_renderData = pVeboxState->GetLastExecRenderData();

    if (MEDIA_IS_SKU(m_pSkuTable, FtrSFCPipe) && !m_sfcPipeState)
    {
#if __VPHAL_SFC_SUPPORTED
        m_sfcPipeState = CreateSfcState();
#else
        m_sfcPipeState = MOS_New(VphalSfcState, m_pOsInterface, m_pRenderHal, m_pSfcInterface);
#endif
        if (!m_sfcPipeState)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }

    // Vebox Comp Bypass is on by default
    pVeboxState->dwCompBypassMode = VPHAL_COMP_BYPASS_ENABLED;

    // Read user feature key to get the Composition Bypass mode
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

    // Vebox Comp Bypass is on by default
    UserFeatureData.u32Data = VPHAL_COMP_BYPASS_ENABLED;

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_BYPASS_COMPOSITION_ID,
        &UserFeatureData));
    pVeboxState->dwCompBypassMode = UserFeatureData.u32Data;

    if (MEDIA_IS_SKU(pVeboxState->m_pSkuTable, FtrSFCPipe) &&
        m_sfcPipeState)
    {
        // Read user feature key to Disable SFC
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
            nullptr,
            __VPHAL_VEBOX_DISABLE_SFC_ID,
            &UserFeatureData));
        m_sfcPipeState->SetDisable(UserFeatureData.bData ? true : false);
    }

    pVeboxState->bEnableMMC = 0;
    pVeboxState->bDisableTemporalDenoiseFilter = 0;
    pVeboxState->bDisableTemporalDenoiseFilterUserKey = 0;

    NullRenderingFlags  = pVeboxState->m_pOsInterface->pfnGetNullHWRenderFlags(
                        pVeboxState->m_pOsInterface);

    pVeboxState->bNullHwRenderDnDi    =
                    NullRenderingFlags.VPDnDi ||
                    NullRenderingFlags.VPGobal;

    // Setup disable render flag controlled by a user feature key for validation purpose
    pVeboxState->SetRenderDisableFlag( pSettings->disableDnDi == false ? false : true );

    // Enable/Disable kernel Copy/Update for VEBox
    pVeboxState->dwKernelUpdate       = pSettings->kernelUpdate;

    // Setup Same Sample Threshold for VEBOX
    pVeboxState->iSameSampleThreshold = pSettings->sameSampleThreshold;

    // Setup interface to KDLL
    pVeboxState->m_pKernelDllState      = pKernelDllState;

    // Initialize State variables
    pVeboxState->iCurFrameID          = FIRST_FRAME;
    pVeboxState->iPrvFrameID          = FIRST_FRAME;
    pVeboxState->bFirstFrame          = true;

finish:
    return eStatus;
}

//!
//! \brief    Destroy function 
//! \details  Release all VEBOX related resources 
//!           that needs virtual function and thus cannot be done in destructors
//! \return   void
//!
void VPHAL_VEBOX_STATE::Destroy()
{
    PVPHAL_VEBOX_STATE                  pVeboxState = this;

    if (pVeboxState)
    {
        // Free VEBOX allocations
        if (MEDIA_IS_SKU(pVeboxState->m_pSkuTable, FtrVERing))
        {
            pVeboxState->FreeResources();
        }
    }
}

//!
//! \brief    Copy Dndi Surface Params
//! \details  Copies surface params to output surface
//!           based on src and temp surfaces
//! \param    [in] pSrcSurface
//!           Pointer to Source Surface
//! \param    [in] pTempSurface
//!           Pointer to Temporary Surface
//! \param    [in,out] pOutSurface
//!           Pointer to Out Surface
//! \return   void
//!
void VPHAL_VEBOX_STATE::VeboxCopySurfaceParams(
    const PVPHAL_SURFACE            pSrcSurface,
    const PVPHAL_SURFACE            pTempSurface,
    PVPHAL_SURFACE                  pOutSurface)
{
    PMOS_INTERFACE       pOsInterface;
    const PVPHAL_VEBOX_STATE        pVeboxState = this;
    const PVPHAL_VEBOX_RENDER_DATA  pRenderData = GetLastExecRenderData();

    pOsInterface = pVeboxState->m_pOsInterface;

    // Copy all parameters from SrcSurface to Output Surface
    CopySurfaceValue(pOutSurface, pSrcSurface);

    // Disable variance query if applicable
    if (pVeboxState->IsQueryVarianceEnabled())
    {
        pOutSurface->bQueryVariance = false;
    }

    // Use original input as AdvProc's output (may occur in Variance) 
    if (pSrcSurface == pTempSurface)
    {
        goto finish;
    }

    // Copy relevant surf params from Temp Surface to Output surface
    pOutSurface->OsResource         = pTempSurface->OsResource;
    pOutSurface->Format             = pTempSurface->Format;
    pOutSurface->dwHeight           = pTempSurface->dwHeight;
    pOutSurface->dwWidth            = pTempSurface->dwWidth;
    pOutSurface->dwPitch            = pTempSurface->dwPitch;
    pOutSurface->TileType           = pTempSurface->TileType;
    pOutSurface->SampleType         = pTempSurface->SampleType;
    pOutSurface->ColorSpace         = pTempSurface->ColorSpace;
    pOutSurface->dwOffset           = pTempSurface->dwOffset;
    pOutSurface->YPlaneOffset       = pTempSurface->YPlaneOffset;
    pOutSurface->UPlaneOffset       = pTempSurface->UPlaneOffset;
    pOutSurface->VPlaneOffset       = pTempSurface->VPlaneOffset;
    pOutSurface->bCompressible      = pTempSurface->bCompressible;
    pOutSurface->bIsCompressed      = pTempSurface->bIsCompressed;
    pOutSurface->CompressionMode    = pTempSurface->CompressionMode;

    // Reset deinterlace params if applicable
    if (pRenderData->bDeinterlace)
    {
        pOutSurface->pDeinterlaceParams = nullptr;
    }

    // Reset Allocations
    pOsInterface->pfnResetResourceAllocationIndex(
        pOsInterface, 
        &pOutSurface->OsResource);

finish:
    return;
}

//!
//! \brief    Setup reference surfaces
//! \details  Setup reference surfaces for app feeds reference case and
//!           no reference frame case
//! \param    [in] pSrcSurface
//!           Pointer to Source Surface
//! \return   PVPHAL_SURFACE
//!           Pointer to Reference surface or nullptr if no reference
//!
PVPHAL_SURFACE VPHAL_VEBOX_STATE::VeboxSetReference(
    PVPHAL_SURFACE              pSrcSurface)
{
    PVPHAL_SURFACE pRefSurface = nullptr;
    PVPHAL_VEBOX_STATE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    if (pRenderData->bRefValid)
    {
        // Set the Reference surface
        if (IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState))
        {
            // Vebox defines reference as previous input frame.
            // In this mode, treat current as the previous
            pRefSurface = pSrcSurface; 
        }
        else
        {
            pRefSurface = pSrcSurface->pBwdRef;
            VPHAL_RENDER_ASSERT( pRefSurface && pSrcSurface->uBwdRefCount > 0);
        }

        // Discontinuity - hence can't reuse previous surfaces
        if (!pRenderData->bSameSamples && pRenderData->bOutOfBound)
        {
            if (pRenderData->bDenoise && 
                !Mos_ResourceIsNull(&pVeboxState->m_currentSurface->OsResource))
            {
                // Save prev call's current sample as prev sample for current call
                CopySurfaceValue(pVeboxState->m_previousSurface, pVeboxState->m_currentSurface);
                pVeboxState->m_previousSurface->FrameID = pRefSurface->FrameID;
            }
            else if ((pRenderData->bEnableMMC || IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData)) &&
                     pRenderData->bDenoise &&
                     pRenderData->bDeinterlace &&
                     Mos_ResourceIsNull(&pVeboxState->m_currentSurface->OsResource))
            {
                // In WHCK test, there is a special test.Test app will set future / post reference frame for frame 0.
                // Then, frame 0 will do deinterlace using uncompressed reference frame instead of denoised compressed frames if MMC on.
                // If using reference frame to do ADI deinterlace, there is corruption with previous DI frame of the VEBOX output which output by SFC.   
                pRenderData->bRefValid = false;
            }
            else
            {
                // Save cur sample as prev for next call
                CopySurfaceValue(pVeboxState->m_previousSurface, pRefSurface);
            }
            pVeboxState->VeboxClearFmdStates();
            VPHAL_RENDER_NORMALMESSAGE("Discontinuity.");
        }
        else
        {
            if (Mos_ResourceIsNull(&pVeboxState->m_currentSurface->OsResource))
            {
                // If Current Resource is not valid, always set the
                // previous surface as Reference surface
                CopySurfaceValue(pVeboxState->m_previousSurface, pRefSurface);
            }
            else
            {
                if (pRenderData->bDenoise)
                {
                    // Save prev call's current sample as prev sample for current call
                    CopySurfaceValue(pVeboxState->m_previousSurface, pVeboxState->m_currentSurface);
                    pVeboxState->m_previousSurface->FrameID = pRefSurface->FrameID;
                }
                else
                {
                    // Use application supplied reference frame
                    CopySurfaceValue(pVeboxState->m_previousSurface, pRefSurface);
                }
            }
        }
    }

    if (!pRenderData->bRefValid)    // No Reference frame provided.
    {
        // Disable FMD.
        if (!IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
        {
            pVeboxState->VeboxClearFmdStates();
        }

        if (pRenderData->bDenoise && 
            (pSrcSurface->SampleType == SAMPLE_PROGRESSIVE))
        {

            //  The first "current" frame may not have a resource (i.e. no 
            //  memory has been allocated for this image surface.
            //  All subsequent frames should have a resource.
            if (!Mos_ResourceIsNull(&pVeboxState->m_currentSurface->OsResource))
            {
                // At this stage, the CurrentSurface contains the Denoise output
                // from the previous iteration.
                CopySurfaceValue(pVeboxState->m_previousSurface, pVeboxState->m_currentSurface);

                pRefSurface = pVeboxState->m_previousSurface;
                pRenderData->bRefValid = true; 
            }
        }
        else if (pRenderData->bDeinterlace)
        {
            // Force DI when there is no ref sample
            // Update Surfaces DI params
            pSrcSurface->pDeinterlaceParams->bSingleField = true;
            pRenderData->bSingleField = true;

            VPHAL_RENDER_NORMALMESSAGE("BOB using VEBOX h/w (no ref sample).");
        }
    }

    return pRefSurface;
}

//!
//! \brief    Set DI output sample
//! \details  Set DI sample to be used for compositing stage followed by VEBOX
//!           feature reporting
//! \param    [in] pSrcSurface
//!           Pointer to Source Surface
//! \param    [in,out] pOutputSurface
//!           Pointer to Output Surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if no error else MOS_STATUS_UNKNOWN
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSetDiOutput(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutputSurface)
{
    PVPHAL_SURFACE  pDstSurface;
    int32_t         iFrame0;
    int32_t         iFrame1;
    PVPHAL_VEBOX_STATE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    if (pRenderData->bDeinterlace)
    {
        if (pVeboxState->m_pVeboxExecState->bDIOutputPair01)
        {
            iFrame0 = 0;
            iFrame1 = 1;
        }
        else
        {
            iFrame0 = 2;
            iFrame1 = 3;
        }

		// for 30i->60fps + Comp
		if (pRenderData->b60fpsDi)
		{
			// Output 1st field of current frame according to DDI flag
			if (pRenderData->bSingleField                                               ||
				(pSrcSurface->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD)     ||
				(pSrcSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD) ||
				(pSrcSurface->SampleType == SAMPLE_SINGLE_BOTTOM_FIELD)                 ||
				(pSrcSurface->SampleType == SAMPLE_PROGRESSIVE))
			{
				pDstSurface = pVeboxState->FFDISurfaces[iFrame1];
			}
			else
			{
				// First sample output - 2nd field of the previous frame
				pDstSurface = pVeboxState->FFDISurfaces[iFrame0];
			}
		}
		// for 30i->30fps + Comp, which differentiates from 30i->60fps for the correct
		// output according to SampleType input
		// eg. TF/BF matches with {0,2,4,6...}/{0,1,3,5...} output
		else
		{
			// Output 1st field of current frame according to DDI flag
			if (pRenderData->bSingleField                                              ||
				(pSrcSurface->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD) ||
				(pSrcSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)   ||
				(pSrcSurface->SampleType == SAMPLE_SINGLE_TOP_FIELD)                   ||
				(pSrcSurface->SampleType == SAMPLE_PROGRESSIVE))
			{
				pDstSurface = pVeboxState->FFDISurfaces[iFrame1];
			}
			else
			{
				// First sample output - 2nd field of the previous frame
				pDstSurface = pVeboxState->FFDISurfaces[iFrame0];
			}
		}
     }
     else if (pRenderData->bIECP)
     {
         // IECP output
         pDstSurface = pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
     }
     else if (pRenderData->bDenoise)
     {
         // Denoise output
         pDstSurface = pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
     }
     else
     {
         VPHAL_RENDER_ASSERTMESSAGE("incorrect dndi output.");
         return MOS_STATUS_UNKNOWN;
     }

    //----------------------------
    // VEBOX feature reporting
    //----------------------------
    m_reporting->IECP    = IsIECPEnabled();
    m_reporting->Denoise = pRenderData->bDenoise;
    if (pRenderData->bDeinterlace)
    {
        m_reporting->DeinterlaceMode =
            (pRenderData->bSingleField && !pRenderData->bRefValid ) ?
            VPHAL_DI_REPORT_ADI_BOB :    // VEBOX BOB
            VPHAL_DI_REPORT_ADI;         // ADI
    }

    // Copy relevant surf params from pDst to pOutput
    VeboxCopySurfaceParams(
        pSrcSurface,
        pDstSurface,
        pOutputSurface);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Vebox Set PerfTag
//! \details  Set Vebox PerfTag: DN, DI and IECP
//! \param    [in] srcFmt
//!           Source surface format of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSetPerfTag(
    MOS_FORMAT               srcFmt)
{
    MOS_STATUS     eStatus;
    PVPHAL_PERFTAG pPerfTag;
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    eStatus  = MOS_STATUS_INVALID_PARAMETER;
    pPerfTag = &pRenderData->PerfTag;

    switch (srcFmt)
    {
        case Format_NV12:
            if (pRenderData->bDeinterlace ||
                IsQueryVarianceEnabled())
            {
                if (pRenderData->bDenoise ||
                    pRenderData->bChromaDenoise)
                {
                    if (IsIECPEnabled())
                    {
                        *pPerfTag = VPHAL_NV12_DNDI_422CP;
                    }
                    else
                    {
                        *pPerfTag = VPHAL_NV12_DNDI_PA;
                    }
                }
                else
                {
                    if (IsIECPEnabled())
                    {
                        *pPerfTag = VPHAL_PL_DI_422CP;
                    }
                    else
                    {
                        *pPerfTag = VPHAL_PL_DI_PA;
                    }
                }
            }
            else
            {
                if (pRenderData->bDenoise ||
                    pRenderData->bChromaDenoise)
                {
                    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
                    {
                        switch (pRenderData->pRenderTarget->Format)
                        {
                            case Format_NV12:
                                *pPerfTag = VPHAL_NV12_DN_420CP;
                                break;
                            CASE_PA_FORMAT:
                                *pPerfTag = VPHAL_NV12_DN_422CP;
                                break;
                            case Format_RGB32:
                                *pPerfTag = VPHAL_NV12_DN_RGB32CP;
                            case Format_A8R8G8B8:
                            case Format_A8B8G8R8:
                                *pPerfTag = VPHAL_NV12_DN_RGB32CP;
                                break;
                            case Format_P010:
                            case Format_P016:
                            case Format_Y410:
                            case Format_Y416:
                            case Format_Y210:
                            case Format_Y216:
                            case Format_AYUV:
                                *pPerfTag = VPHAL_NONE;
                                break;
                            default:
                                VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                                goto finish;
                        }
                    }
                    else if (IsIECPEnabled())
                    {
                        *pPerfTag = VPHAL_NV12_DN_420CP;
                    }
                    else
                    {
                        *pPerfTag = VPHAL_NV12_DN_NV12;
                    }
                }
                else
                {
                    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
                    {
                        switch (pRenderData->pRenderTarget->Format)
                        {
                            case Format_NV12:
                                *pPerfTag = VPHAL_NV12_420CP;
                                break;
                            CASE_PA_FORMAT:
                                *pPerfTag = VPHAL_NV12_422CP;
                                break;
                            case Format_RGB32:
                                *pPerfTag = VPHAL_NV12_RGB32CP;
                            case Format_A8R8G8B8:
                            case Format_A8B8G8R8:
                            case Format_R10G10B10A2:
                            case Format_B10G10R10A2:
                                *pPerfTag = VPHAL_NV12_RGB32CP;
                                break;
                            case Format_P010:
                            case Format_P016:
                            case Format_Y410:
                            case Format_Y416:
                            case Format_Y210:
                            case Format_Y216:
                            case Format_AYUV:
                                *pPerfTag = VPHAL_NONE;
                                break;
                            default:
                                VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                                goto finish;
                        }
                    }
                    else
                    {
                        *pPerfTag = VPHAL_NV12_420CP;
                    }
                }
            }
            break;

        CASE_PA_FORMAT:
            if (pRenderData->bDeinterlace ||
                IsQueryVarianceEnabled())
            {
                if (pRenderData->bDenoise ||
                    pRenderData->bChromaDenoise)
                {
                    if (IsIECPEnabled())
                    {
                        *pPerfTag = VPHAL_PA_DNDI_422CP;
                    }
                    else
                    {
                        *pPerfTag = VPHAL_PA_DNDI_PA;
                    }
                }
                else
                {
                    if (IsIECPEnabled())
                    {
                        *pPerfTag = VPHAL_PA_DI_422CP;
                    }
                    else
                    {
                        *pPerfTag = VPHAL_PA_DI_PA;
                    }
                }
            }
            else
            {
                if (pRenderData->bDenoise ||
                    pRenderData->bChromaDenoise)
                {
                    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
                    {
                        switch (pRenderData->pRenderTarget->Format)
                        {
                            case Format_NV12:
                                *pPerfTag = VPHAL_PA_DN_420CP;
                                break;
                            CASE_PA_FORMAT:
                                *pPerfTag = VPHAL_PA_DN_422CP;
                                break;
                            case Format_RGB32:
                                *pPerfTag = VPHAL_PA_DN_RGB32CP;
                                break;
                            case Format_P010:
                            case Format_P016:
                            case Format_Y410:
                            case Format_Y416:
                            case Format_Y210:
                            case Format_Y216:
                            case Format_AYUV:
                                *pPerfTag = VPHAL_NONE;
                                break;
                            default:
                                VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                                goto finish;
                        }
                    }
                    else if (IsIECPEnabled())
                    {
                        *pPerfTag = VPHAL_PA_DN_422CP;
                    }
                    else
                    {
                        *pPerfTag = VPHAL_PA_DN_PA;
                    }
                }
                else
                {
                    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
                    {
                        switch (pRenderData->pRenderTarget->Format)
                        {
                            case Format_NV12:
                                *pPerfTag = VPHAL_PA_420CP;
                                break;
                            CASE_PA_FORMAT:
                                *pPerfTag = VPHAL_PA_422CP;
                                break;
                            case Format_RGB32:
                                *pPerfTag = VPHAL_PA_RGB32CP;
                                break;
                            case Format_A8R8G8B8:
                            case Format_A8B8G8R8:
                            case Format_R10G10B10A2:
                            case Format_B10G10R10A2:
                                *pPerfTag = VPHAL_PA_RGB32CP;
                                break;
                            case Format_P010:
                            case Format_P016:
                            case Format_Y410:
                            case Format_Y416:
                            case Format_Y210:
                            case Format_Y216:
                            case Format_AYUV:
                                *pPerfTag = VPHAL_NONE;
                                break;
                            default:
                                VPHAL_RENDER_ASSERTMESSAGE("Output Format Not found.");
                                goto finish;
                        }
                    }
                    else
                    {
                        *pPerfTag = VPHAL_PA_422CP;
                    }
                }
            }
            break;

        case Format_AYUV:
            break;

        CASE_RGB32_FORMAT:
            // RGB Input Support for SFC
            *pPerfTag = VPHAL_NONE;
            break;
            
        case Format_P010:
            // P010 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P010;
            break;

        case Format_P016:
            // P016 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P016;
            break;        

        case Format_P210:
            // P210 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P210;
            break;

        case Format_P216:
            // P216 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P216;
            break;

        case Format_Y210:
            // Y210 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y210;
            break;

        case Format_Y216:
            // Y216 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y216;
            break;

        case Format_Y410:
            // Y410 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y410;
            break;

        case Format_Y416:
            // Y416 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y416;
            break;

        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
            *pPerfTag = VPHAL_NONE;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Format Not found.");
            goto finish;
    } // switch (srcFmt)

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Vebox Populate VEBOX parameters
//! \details  Populate the Vebox VEBOX state parameters to VEBOX RenderData
//! \param    [in] pLumaParams
//!           Pointer to Luma DN and DI parameter
//! \param    [in] pChromaParams
//!           Pointer to Chroma DN parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxPopulateDNDIParams(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
    PVPHAL_DNUV_PARAMS              pChromaParams)
{
    PMHW_VEBOX_DNDI_PARAMS          pVeboxDNDIParams;
    PVPHAL_VEBOX_RENDER_DATA        pRenderData = GetLastExecRenderData();

    // Populate the VEBOX VEBOX parameters
    pVeboxDNDIParams = &pRenderData->VeboxDNDIParams;

    // DI and Luma Denoise Params
    if (pLumaParams != nullptr)
    {
        if (pRenderData->bDenoise)
        {
            pVeboxDNDIParams->dwDenoiseASDThreshold     = pLumaParams->dwDenoiseASDThreshold;
            pVeboxDNDIParams->dwDenoiseHistoryDelta     = pLumaParams->dwDenoiseHistoryDelta;
            pVeboxDNDIParams->dwDenoiseMaximumHistory   = pLumaParams->dwDenoiseMaximumHistory;
            pVeboxDNDIParams->dwDenoiseSTADThreshold    = pLumaParams->dwDenoiseSTADThreshold;
            pVeboxDNDIParams->dwDenoiseSCMThreshold     = pLumaParams->dwDenoiseSCMThreshold;
            pVeboxDNDIParams->dwDenoiseMPThreshold      = pLumaParams->dwDenoiseMPThreshold;
            pVeboxDNDIParams->dwLTDThreshold            = pLumaParams->dwLTDThreshold;
            pVeboxDNDIParams->dwTDThreshold             = pLumaParams->dwTDThreshold;
            pVeboxDNDIParams->dwGoodNeighborThreshold   = pLumaParams->dwGoodNeighborThreshold;
            pVeboxDNDIParams->bProgressiveDN            = pLumaParams->bProgressiveDN;
        }
        pVeboxDNDIParams->dwFMDFirstFieldCurrFrame      = pLumaParams->dwFMDFirstFieldCurrFrame;
        pVeboxDNDIParams->dwFMDSecondFieldPrevFrame     = pLumaParams->dwFMDSecondFieldPrevFrame;
        pVeboxDNDIParams->bDNDITopFirst                 = pLumaParams->bDNDITopFirst;
    }

    // Only need to reverse bDNDITopFirst for no reference case, no need to reverse it for having refrenece case 
    if (!pRenderData->bRefValid)
    {
        pVeboxDNDIParams->bDNDITopFirst                 = pRenderData->bTopField;
    }

    // Chroma Denoise Params
    if (pRenderData->bChromaDenoise && pChromaParams != nullptr)
    {
        pVeboxDNDIParams->dwChromaSTADThreshold     = pChromaParams->dwSTADThresholdU; // Use U threshold for now
        pVeboxDNDIParams->dwChromaLTDThreshold      = pChromaParams->dwLTDThresholdU;  // Use U threshold for now 
        pVeboxDNDIParams->dwChromaTDThreshold       = pChromaParams->dwTDThresholdU;   // Use U threshold for now 
        pVeboxDNDIParams->bChromaDNEnable           = pRenderData->bChromaDenoise;
    }

    pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams = pVeboxDNDIParams;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Vebox Set FMD parameter
//! \details  Set up the FMD parameters for DNDI State
//! \param    [out] pLumaParams
//!           Pointer to DNDI Param for set FMD parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSetFMDParams(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM     pLumaParams)
{
    PVPHAL_VEBOX_RENDER_DATA         pRenderData = GetLastExecRenderData();
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;
    
    VPHAL_RENDER_CHK_NULL(pLumaParams);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (pRenderData->bProgressive && pRenderData->bAutoDenoise)
    {
        // out1 = Cur1st + Cur2nd
        pLumaParams->dwFMDFirstFieldCurrFrame =
            MEDIASTATE_DNDI_FIELDCOPY_NEXT;
        // out2 = Prv1st + Prv2nd
        pLumaParams->dwFMDSecondFieldPrevFrame =
            MEDIASTATE_DNDI_FIELDCOPY_PREV;
    }
    else
#endif
    {   
        pLumaParams->dwFMDFirstFieldCurrFrame =
            MEDIASTATE_DNDI_DEINTERLACE;
        pLumaParams->dwFMDSecondFieldPrevFrame =
            MEDIASTATE_DNDI_DEINTERLACE;
    }

finish:
    return eStatus;
}

//!
//! \brief    Vebox Set VEBOX parameter
//! \details  Set up the VEBOX parameter value
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSetDNDIParams(
    PVPHAL_SURFACE              pSrcSurface)
{
    MOS_STATUS                       eStatus;
    VPHAL_SAMPLER_STATE_DNDI_PARAM   lumaParams;
    VPHAL_DNUV_PARAMS                chromaParams;
    PVPHAL_SAMPLER_STATE_DNDI_PARAM  pLumaParams;
    PVPHAL_DNUV_PARAMS               pChromaParams;
    PVPHAL_VEBOX_STATE               pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA         pRenderData = GetLastExecRenderData();

    eStatus             = MOS_STATUS_SUCCESS;
    pLumaParams         = &lumaParams;     // Params for DI and LumaDN
    pChromaParams       = &chromaParams;   // Params for ChromaDN

    // Set Luma and Chroma DNDI params
    VPHAL_RENDER_CHK_STATUS(pVeboxState->SetDNDIParams(
        pSrcSurface, 
        pLumaParams, 
        pChromaParams));

    if (!pRenderData->bRefValid)
    {
        // setting LTDThreshold = TDThreshold = 0 forces SpatialDenoiseOnly
        pLumaParams->dwLTDThreshold   = 0;
        pLumaParams->dwTDThreshold    = 0;
    }

    if (pRenderData->bDenoise) 
    {
        pLumaParams->bDNEnable = true;

        if (pRenderData->bProgressive)
        {
            pLumaParams->bProgressiveDN = true;
        }
    }

    if (pRenderData->bDeinterlace || IsQueryVarianceEnabled())
    {
        pLumaParams->bDIEnable = true;
        pLumaParams->bDNDITopFirst = pRenderData->bTFF;
    }

    VeboxSetFMDParams(pLumaParams);

    VeboxPopulateDNDIParams(
        pLumaParams,
        pChromaParams);

finish:
    return eStatus;
}

//!
//! \brief    Flush command buffer for update kernels
//! \details  Flush the command buffer for Update kernels
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxFlushUpdateStateCmdBuffer()
{
    PRENDERHAL_INTERFACE                pRenderHal;
    PRENDERHAL_STATE_HEAP               pStateHeap;
    MhwRenderInterface                  *pMhwRender;
    PMOS_INTERFACE                      pOsInterface;
    MOS_COMMAND_BUFFER                  CmdBuffer;
    MOS_STATUS                          eStatus;
    int32_t                             i, iRemaining;
    PMHW_MI_INTERFACE                   pMhwMiInterface;
    MHW_MEDIA_OBJECT_PARAMS             MediaObjectParams;
    MEDIA_OBJECT_KA2_INLINE_DATA        InlineData;
    int32_t                             iInlineSize;
    MHW_PIPE_CONTROL_PARAMS             PipeCtlParams;
    MHW_ID_LOAD_PARAMS                  IdLoadParams;
    PVPHAL_VEBOX_STATE                  pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA            pRenderData = GetLastExecRenderData();

    eStatus                 = MOS_STATUS_SUCCESS;
    pRenderHal              = pVeboxState->m_pRenderHal;
    pStateHeap              = pRenderHal->pStateHeap;
    pMhwRender              = pRenderHal->pMhwRenderInterface;
    pMhwMiInterface         = pRenderHal->pMhwMiInterface;
    pOsInterface            = pVeboxState->m_pOsInterface;

    iRemaining = 0;
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));

    // Set initial state
    iRemaining = CmdBuffer.iRemaining;

    // Initialize command buffer and insert prolog
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnInitCommandBuffer(pRenderHal, &CmdBuffer, nullptr));

    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendSyncTag(pRenderHal, &CmdBuffer));

    VPHAL_RENDER_CHK_STATUS(pMhwRender->EnablePreemption(&CmdBuffer));

    // Send Media Pipeline Select command
    VPHAL_RENDER_CHK_STATUS(pMhwRender->AddPipelineSelectCmd(
        &CmdBuffer,
        false));

    VPHAL_RENDER_CHK_STATUS(pMhwRender->AddStateBaseAddrCmd(
        &CmdBuffer,
        &pRenderHal->StateBaseAddressParams));

    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendSurfaces(pRenderHal, &CmdBuffer));

    VPHAL_RENDER_CHK_STATUS(pMhwRender->AddMediaVfeCmd(
        &CmdBuffer,
        pRenderHal->pRenderHalPltInterface->GetVfeStateParameters()));

    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSendCurbeLoad(pRenderHal, &CmdBuffer));

    // Send Interface Descriptor Load
    MOS_ZeroMemory(&IdLoadParams, sizeof(IdLoadParams));
    IdLoadParams.pKernelState                     = nullptr;
    IdLoadParams.dwInterfaceDescriptorStartOffset = pStateHeap->pCurMediaState->dwOffset +  pStateHeap->dwOffsetMediaID;
    IdLoadParams.dwInterfaceDescriptorLength      = pRenderHal->StateHeapSettings.iMediaIDs * pStateHeap->dwSizeMediaID;
    VPHAL_RENDER_CHK_STATUS(pMhwRender->AddMediaIDLoadCmd(&CmdBuffer, &IdLoadParams));

    // Each media obj include header (no inline data is needed, 
    // however add 1 DW dummy inline to avoid HW Hang)
    iInlineSize = 1 * sizeof(uint32_t);
    InlineData  = g_cInit_MEDIA_OBJECT_KA2_INLINE_DATA;

    MOS_ZeroMemory(&MediaObjectParams, sizeof(MediaObjectParams));
    MediaObjectParams.dwInlineDataSize              = iInlineSize;
    MediaObjectParams.pInlineData                   = &InlineData;

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Launch Interface Descriptor for DN Update kernel
    if (pRenderData->bAutoDenoise)
    {
        MediaObjectParams.dwInterfaceDescriptorOffset   = pRenderData->iMediaID0;
        VPHAL_RENDER_CHK_STATUS(pMhwRender->AddMediaObject(
            &CmdBuffer,
            nullptr,
            &MediaObjectParams));
        pVeboxState->m_currKernelId =  kernelVeboxUpdateDnState;
    }
#endif

    VPHAL_RENDER_CHK_STATUS(VeboxFlushUpdateStateAddExtraKernels(
            CmdBuffer,
            MediaObjectParams));

    if (GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform))
    {
        // Add a pipe_control and vfe to clear the media state to fix PF issue.
        MHW_PIPE_CONTROL_PARAMS PipeControlParams;

        MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
        PipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        PipeControlParams.bGenericMediaStateClear = true;
        PipeControlParams.bIndirectStatePointersDisable = true;
        PipeControlParams.bDisableCSStall = false;
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddPipeControl(&CmdBuffer, nullptr, &PipeControlParams));

        if (MEDIA_IS_WA(pRenderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
        {
            MHW_VFE_PARAMS VfeStateParams;

            MOS_ZeroMemory(&VfeStateParams, sizeof(VfeStateParams));
            VfeStateParams.dwNumberofURBEntries = 1;
            VPHAL_RENDER_CHK_STATUS(pMhwRender->AddMediaVfeCmd(&CmdBuffer, &VfeStateParams));
        }
    }

    if (GFX_IS_GEN_8_OR_LATER(pRenderHal->Platform))
    {
        // Add media flush command in case HW not cleaning the media state
        // On CHV A Stepping, use MSFlush with Watermark or FlushToGo
        MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = g_cRenderHal_InitMediaStateFlushParams;

        if (MEDIA_IS_WA(pRenderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
        {
            FlushParam.bFlushToGo = true;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMediaStateFlush(&CmdBuffer, nullptr, &FlushParam));
        }
        else if (MEDIA_IS_WA(pRenderHal->pWaTable, WaAddMediaStateFlushCmd))
        {
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMediaStateFlush(&CmdBuffer, nullptr, &FlushParam));
        }
    }

    if (VpHal_RndrCommonIsMiBBEndNeeded(pOsInterface))
    {
        // Add Batch Buffer end command (HW/OS dependent)
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(&CmdBuffer, nullptr));
    }

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
        CmdBuffer.pCmdPtr     = CmdBuffer.pCmdBase + CmdBuffer.iOffset/sizeof(uint32_t);
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(
        pOsInterface,
        &CmdBuffer,
        0);

    // Flush the command buffer
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSubmitCommandBuffer(
        pOsInterface,
        &CmdBuffer,
        pVeboxState->bNullHwRenderDnDi));

    return eStatus;
}

//!
//! \brief    Vebox Copy Vebox state heap, intended for HM or IDM
//! \details  Copy Vebox state heap between different memory
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxCopyVeboxStates()
{
    return MOS_STATUS_SUCCESS;  // no need to copy, always use driver resource in clear memory
}

//!
//! \brief    Calculate offsets of statistics surface address based on the
//!           functions which were enabled in the previous call,
//!           and store the width and height of the per-block statistics into DNDI_STATE
//! \details
//! Layout of Statistics surface when Temporal DI enabled
//!     --------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!     |-------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=4       | ...\n
//!     |------------------------------\n
//!     | ...\n
//!     |------------------------------\n
//!     | 16 bytes for x=0, Y=height-4| ...\n
//!     |-----------------------------------------------Pitch----------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Previous)| 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Current) | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Previous)| 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Current) | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     ---------------------------------------------------------------------------------------------------------------\n
//!
//! Layout of Statistics surface when DN or Spatial DI enabled (and Temporal DI disabled)
//!     --------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
//!     |-------------------------------------------------------------\n
//!     | 16 bytes for x=0, Y=4       | ...\n
//!     |------------------------------\n
//!     | ...\n
//!     |------------------------------\n
//!     | 16 bytes for x=0, Y=height-4| ...\n
//!     |-----------------------------------------------Pitch----------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Input)   | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Input)   | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     ---------------------------------------------------------------------------------------------------------------\n
//!
//! Layout of Statistics surface when both DN and DI are disabled
//!     ------------------------------------------------Pitch----------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 0 (Input)   | 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
//!     |--------------------------------------------------------------------------------------------------------------\n
//!     | 256 DW of ACE histogram Slice 1 (Input)   | 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
//!     ---------------------------------------------------------------------------------------------------------------\n
//! \param    [out] pStatSlice0Offset
//!           Statistics surface Slice 0 base pointer
//! \param    [out] pStatSlice1Offset
//!           Statistics surface Slice 1 base pointer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxGetStatisticsSurfaceOffsets(
    int32_t*                    pStatSlice0Offset,
    int32_t*                    pStatSlice1Offset)
{
    PVPHAL_VEBOX_STATE          pVeboxState = this;
    uint32_t    uiPitch;
    int32_t     iOffset;
    MOS_STATUS  eStatus;

    eStatus     = MOS_STATUS_UNKNOWN;
    uiPitch     = 0;

    // Query platform dependent size of per frame information
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_PER_FRAME_SIZE, &uiPitch));

    // Get the base address of Frame based statistics for each slice
    if (pVeboxState->bDIEnabled) // VEBOX, VEBOX+IECP
    {
        // Frame based statistics begins after Encoder statistics
        iOffset = pVeboxState->dwVeboxPerBlockStatisticsWidth *
                  pVeboxState->dwVeboxPerBlockStatisticsHeight;

        *pStatSlice0Offset = iOffset + uiPitch;                                     // Slice 0 current frame
        *pStatSlice1Offset = iOffset + uiPitch * 3;                                 // Slice 1 current frame
    }
    else if (pVeboxState->bDNEnabled) // DN, DN_IECP, SpatialDI
    {
        // Frame based statistics begins after Encoder statistics
        iOffset = pVeboxState->dwVeboxPerBlockStatisticsWidth *
                  pVeboxState->dwVeboxPerBlockStatisticsHeight;

        *pStatSlice0Offset = iOffset;                                               // Slice 0 input frame
        *pStatSlice1Offset = iOffset + uiPitch;                                     // Slice 1 input frame
    }
    else // IECP only
    {
        *pStatSlice0Offset = 0;                                                     // Slice 0 input frame
        *pStatSlice1Offset = uiPitch;                                               // Slice 1 input frame
    }

finish:
    return eStatus;
}

//!
//! \brief    Vebox state heap update for auto mode features
//! \details  Update Vebox indirect states for auto mode features
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxUpdateVeboxStates(
    PVPHAL_SURFACE              pSrcSurface)
{
#if VEBOX_AUTO_DENOISE_SUPPORTED
    PRENDERHAL_INTERFACE        pRenderHal;
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus;
    int32_t                     iCurbeOffsetDN;
    int32_t                     iKrnAllocation;
    MHW_KERNEL_PARAM            MhwKernelParam;

    PVPHAL_VEBOX_STATE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    eStatus             = MOS_STATUS_SUCCESS;
    pRenderHal          = pVeboxState->m_pRenderHal;
    pOsInterface        = pVeboxState->m_pOsInterface;

    if (!pRenderData->bAutoDenoise)
    {
        // only when auto denoise is on do we need to update VEBOX states
        return MOS_STATUS_SUCCESS;
    }

    // Switch GPU Context to Render Engine
    pOsInterface->pfnSetGpuContext(pOsInterface, RenderGpuContext);

    // Reset allocation list and house keeping
    pOsInterface->pfnResetOsStates(pOsInterface);

    // Register the resource of GSH
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

    // Set up UpdateDNState kernel
    if (pRenderData->bAutoDenoise)
    {
        pVeboxState->SetupVeboxKernel(KERNEL_UPDATEDNSTATE);
    }

    //----------------------------------
    // Allocate and reset media state
    //----------------------------------
    pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_VEBOX);
    VPHAL_RENDER_CHK_NULL(pRenderData->pMediaState);

    //----------------------------------
    //Allocate and reset SSH instance
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    //----------------------------------
    // Assign and Reset Binding Table
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
        pRenderHal,
        &pRenderData->iBindingTable));

    //------------------------------------------
    // Setup Surface states for DN Update kernel
    //------------------------------------------
    if (pRenderData->bAutoDenoise)
    {
        VPHAL_RENDER_CHK_STATUS(SetupSurfaceStatesForDenoise());
    }

    //----------------------------------
    // Load static data (platform specific)
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pVeboxState->LoadUpdateDenoiseKernelStaticData(
        &iCurbeOffsetDN));

    //----------------------------------
    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        pVeboxState->pKernelParamTable[KERNEL_UPDATEDNSTATE].Thread_Count,
        pRenderData->iCurbeLength,
        pRenderData->iInlineLength,
        nullptr));

    //----------------------------------
    // Load DN update kernel to GSH
    //----------------------------------
    if (pRenderData->bAutoDenoise)
    {
        INIT_MHW_KERNEL_PARAM(MhwKernelParam, &pRenderData->KernelEntry[KERNEL_UPDATEDNSTATE]);
        iKrnAllocation = pRenderHal->pfnLoadKernel(
            pRenderHal,
            pRenderData->pKernelParam[KERNEL_UPDATEDNSTATE],
            &MhwKernelParam,
            nullptr);

        if (iKrnAllocation < 0) 
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        //----------------------------------
        // Allocate Media ID, link to kernel
        //----------------------------------
        pRenderData->iMediaID0 = pRenderHal->pfnAllocateMediaID(
            pRenderHal,
            iKrnAllocation,
            pRenderData->iBindingTable,
            iCurbeOffsetDN,
            pRenderData->pKernelParam[KERNEL_UPDATEDNSTATE]->CURBE_Length << 5,
            0,
            nullptr);

        if (pRenderData->iMediaID0 < 0) 
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }

    //---------------------------
    // Render Batch Buffer (Submit commands to HW)
    //---------------------------
    VPHAL_RENDER_CHK_STATUS(VeboxFlushUpdateStateCmdBuffer());

finish:
    return eStatus;
#else
    return MOS_STATUS_SUCCESS;
#endif
}

MOS_STATUS VPHAL_VEBOX_STATE::VeboxSetupIndirectStates(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pOutSurface)
{
    PMOS_INTERFACE                  pOsInterface;
    PMHW_VEBOX_INTERFACE            pVeboxInterface;
    MOS_STATUS                      eStatus;
    MHW_VEBOX_IECP_PARAMS           VeboxIecpParams;
    MHW_VEBOX_GAMUT_PARAMS          VeboxGamutParams;
    PVPHAL_VEBOX_STATE              pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA        pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pSrcSurface);

    pOsInterface = pVeboxState->m_pOsInterface;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;

    VPHAL_RENDER_CHK_NULL(pOsInterface);

    // Initialize structure before using
    MOS_ZeroMemory(&VeboxIecpParams, sizeof(MHW_VEBOX_IECP_PARAMS));
    MOS_ZeroMemory(&VeboxGamutParams, sizeof(MHW_VEBOX_GAMUT_PARAMS));
    // Gamma is default to 2.2 since SDR uses 2.2
    VeboxGamutParams.InputGammaValue    = MHW_GAMMA_2P2;
    VeboxGamutParams.OutputGammaValue   = MHW_GAMMA_2P2;

    //----------------------------------
    // Allocate and reset VEBOX state
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AssignVeboxState()); 

    // Set VEBOX State Params
    if (pRenderData->bDeinterlace       ||
        pRenderData->bDenoise           ||
        pRenderData->bChromaDenoise)
    {
        VPHAL_RENDER_CHK_STATUS(VeboxSetDNDIParams(pSrcSurface));
    }

    if (pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams)
    {
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxDndiState(
            pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams));
    }

    // Set IECP State Params
    if (pRenderData->bIECP ||
        IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) ||
        IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        m_IECP->SetParams(pSrcSurface, pOutSurface);
    }

    // Set IECP State Params
    if (pRenderData->GetVeboxStateParams()->pVphalVeboxIecpParams)
    {
        VPHAL_RENDER_CHK_STATUS(m_IECP->InitParams(
            pSrcSurface->ColorSpace,
            &VeboxIecpParams));
        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxIecpState(
            &VeboxIecpParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Doing prepare stage tasks for VeboxSendVeboxCmd
//!           Parameters might remain unchanged in case
//! \param    [out] CmdBuffer
//!           reference to Cmd buffer control struct
//! \param    [out] GenericPrologParams
//!           Generic prolog params struct to be set
//! \param    [out] GpuStatusBuffer
//!           GpuStatusBuffer resource to be set
//! \param    [out] iRemaining
//!           integer showing initial cmd buffer usage
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSendVeboxCmd_Prepare(
    MOS_COMMAND_BUFFER&                      CmdBuffer,
    RENDERHAL_GENERIC_PROLOG_PARAMS&         GenericPrologParams,
    MOS_RESOURCE&                            GpuStatusBuffer,
    int32_t&                                 iRemaining)
{
    MOS_STATUS                              eStatus      = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE                          pOsInterface = m_pOsInterface;
    PVPHAL_VEBOX_STATE                      pVeboxState  = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData  = GetLastExecRenderData();

    // Switch GPU context to VEBOX
    pOsInterface->pfnSetGpuContext(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

    // Reset allocation list and house keeping
    pOsInterface->pfnResetOsStates(pOsInterface);

    // initialize the command buffer struct
    MOS_ZeroMemory(&CmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));

    // Set initial state
    iRemaining = CmdBuffer.iRemaining;

    //---------------------------
    // Set Performance Tags
    //---------------------------
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxSetPerfTag(pVeboxState->m_currentSurface->Format));
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, pRenderData->PerfTag);

    MOS_ZeroMemory(&GenericPrologParams, sizeof(GenericPrologParams));

    // Linux will do nothing here since currently no frame tracking support

#ifndef EMUL
    if (pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &GpuStatusBuffer));

        // Register the buffer
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface, &GpuStatusBuffer, true, true));

        GenericPrologParams.bEnableMediaFrameTracking       = true;
        GenericPrologParams.presMediaFrameTrackingSurface   = &GpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

finish:
    return eStatus;
}

//!
//! \brief    Render the Vebox Cmd buffer for VeboxSendVeboxCmd
//!           Parameters might remain unchanged in case
//! \param    [in,out] CmdBuffer
//!           reference to Cmd buffer control struct
//! \param    [out] VeboxDiIecpCmdParams
//!           DiIecpCmd params struct to be set
//! \param    [out] VeboxSurfaceStateCmdParams
//!           VPHAL surface state cmd to be set
//! \param    [out] MhwVeboxSurfaceStateCmdParams
//!           MHW surface state cmd to be set
//! \param    [out] VeboxStateCmdParams
//!           MHW vebox state cmd to be set
//! \param    [out] FlushDwParams
//!           MHW MI_FLUSH_DW cmd to be set
//! \param    [in] pGenericPrologParams
//!           pointer to Generic prolog params struct to send to cmd buffer header
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxRenderVeboxCmd(
    MOS_COMMAND_BUFFER&                      CmdBuffer,
    MHW_VEBOX_DI_IECP_CMD_PARAMS&            VeboxDiIecpCmdParams,
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS&    VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS&      MhwVeboxSurfaceStateCmdParams,
    MHW_VEBOX_STATE_CMD_PARAMS&              VeboxStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS&                  FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS         pGenericPrologParams)
{
    MOS_STATUS                              eStatus;
    PRENDERHAL_INTERFACE                    pRenderHal;
    PMOS_INTERFACE                          pOsInterface;
    PMHW_MI_INTERFACE                       pMhwMiInterface;
    PMHW_VEBOX_INTERFACE                    pVeboxInterface;
    bool                                    bDiVarianceEnable;
    const MHW_VEBOX_HEAP                    *pVeboxHeap = nullptr;
    PVPHAL_VEBOX_STATE                      pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    eStatus                 = MOS_STATUS_SUCCESS;
    pRenderHal              = pVeboxState->m_pRenderHal;
    pMhwMiInterface         = pRenderHal->pMhwMiInterface;
    pOsInterface            = pVeboxState->m_pOsInterface;
    pVeboxInterface         = pVeboxState->m_pVeboxInterface;

    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->GetVeboxHeapInfo(
                                &pVeboxHeap));
    VPHAL_RENDER_CHK_NULL(pVeboxHeap);

    // Initialize command buffer and insert prolog
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnInitCommandBuffer(pRenderHal, &CmdBuffer, pGenericPrologParams));

    bDiVarianceEnable = pRenderData->bDeinterlace || IsQueryVarianceEnabled();

    pVeboxState->SetupSurfaceStates(
        bDiVarianceEnable, 
        &VeboxSurfaceStateCmdParams);

    pVeboxState->SetupVeboxState(
        bDiVarianceEnable, 
        &VeboxStateCmdParams);

    // Ensure LACE LUT table is ready to be written
    if (VeboxStateCmdParams.pLaceLookUpTables)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface,
            VeboxStateCmdParams.pLaceLookUpTables,
            MOS_GPU_CONTEXT_VEBOX,
            false);
    }

    VPHAL_RENDER_CHK_STATUS(pVeboxState->SetupDiIecpState(
        bDiVarianceEnable, 
        &VeboxDiIecpCmdParams));

    // Ensure output is ready to be written
    if (VeboxDiIecpCmdParams.pOsResCurrOutput)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            VeboxDiIecpCmdParams.pOsResCurrOutput,
            MOS_GPU_CONTEXT_VEBOX,
            true);

        // Synchronize overlay if overlay is used because output could be Render Target
        if (VeboxSurfaceStateCmdParams.pSurfOutput &&
            VeboxSurfaceStateCmdParams.pSurfOutput->bOverlay)
        {
            pOsInterface->pfnSyncOnOverlayResource(
                pOsInterface,
                VeboxDiIecpCmdParams.pOsResCurrOutput,
                MOS_GPU_CONTEXT_VEBOX);
        }
    }

    if (VeboxDiIecpCmdParams.pOsResPrevOutput)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            VeboxDiIecpCmdParams.pOsResPrevOutput,
            MOS_GPU_CONTEXT_VEBOX,
            true);
    }

    if (VeboxDiIecpCmdParams.pOsResDenoisedCurrOutput)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            VeboxDiIecpCmdParams.pOsResDenoisedCurrOutput,
            MOS_GPU_CONTEXT_VEBOX,
            true);
    }

    if (VeboxDiIecpCmdParams.pOsResStatisticsOutput)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            VeboxDiIecpCmdParams.pOsResStatisticsOutput,
            MOS_GPU_CONTEXT_VEBOX,
            true);
    }

    //---------------------------------
    // Initialize Vebox Surface State Params
    //---------------------------------
    VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceStateCmdParams(
        &VeboxSurfaceStateCmdParams, &MhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send MMC CMD
    //---------------------------------
    VPHAL_RENDER_CHK_STATUS_RETURN(VeboxRenderMMCPipeCmd(
        pVeboxInterface,
        pMhwMiInterface,
        &(MhwVeboxSurfaceStateCmdParams.SurfInput),
        &VeboxDiIecpCmdParams,
        &CmdBuffer));
    
    //---------------------------------
    // Send CMD: Vebox_State
    //--------------------------------- 
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxState(
        &CmdBuffer,
        &VeboxStateCmdParams,
        0));

    //---------------------------------
    // Send CMD: Vebox_Surface_State
    //---------------------------------
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaces(
        &CmdBuffer,
        &MhwVeboxSurfaceStateCmdParams));

    //---------------------------------
    // Send CMD: SFC pipe commands
    //---------------------------------
    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        VPHAL_RENDER_CHK_STATUS(m_sfcPipeState->SendSfcCmd(
            pRenderData,
            &CmdBuffer));
    }

    //---------------------------------
    // Send CMD: Vebox_DI_IECP
    //---------------------------------
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxDiIecp(
        &CmdBuffer,
        &VeboxDiIecpCmdParams));

    //---------------------------------
    // Write GPU Status Tag for Tag based synchronization
    //---------------------------------
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        VPHAL_RENDER_CHK_STATUS(VeboxSendVecsStatusTag(
            pMhwMiInterface,
            pOsInterface,
            &CmdBuffer));
    }

    //---------------------------------
    // Write Sync tag for Vebox Heap Synchronization
    // If KMD frame tracking is on, the synchronization of Vebox Heap will use Status tag which
    // is updated using KMD frame tracking.
    //---------------------------------
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
        FlushDwParams.pOsResource                   = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
        FlushDwParams.dwResourceOffset              = pVeboxHeap->uiOffsetSync;
        FlushDwParams.dwDataDW1                     = pVeboxHeap->dwNextTag;
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiFlushDwCmd(
            &CmdBuffer,
            &FlushDwParams));
    }

    if (pOsInterface->bNoParsingAssistanceInKmd)
    {
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(
            &CmdBuffer,
            nullptr));
    }
    else if (VpHal_RndrCommonIsMiBBEndNeeded(pOsInterface))
    {
        // Add Batch Buffer end command (HW/OS dependent)
        VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(
            &CmdBuffer,
            nullptr));
    }

    VPHAL_DBG_STATE_DUMPPER_DUMP_COMMAND_BUFFER(pRenderHal, &CmdBuffer);

finish:
    return eStatus;

}

//!
//! \brief    Vebox send Vebox ring HW commands
//! \details  Send Vebox ring Commands.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSendVeboxCmd()
{
    PRENDERHAL_INTERFACE                    pRenderHal;
    PMOS_INTERFACE                          pOsInterface;
    MOS_COMMAND_BUFFER                      CmdBuffer;
    MOS_STATUS                              eStatus;
    int32_t                                 iRemaining, i;
    MHW_VEBOX_DI_IECP_CMD_PARAMS            VeboxDiIecpCmdParams;
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    VeboxSurfaceStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      MhwVeboxSurfaceStateCmdParams;
    MHW_VEBOX_STATE_CMD_PARAMS              VeboxStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS                  FlushDwParams;
    PMHW_VEBOX_INTERFACE                    pVeboxInterface;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams;
    MOS_RESOURCE                            GpuStatusBuffer;
    PVPHAL_VEBOX_STATE                      pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    eStatus                 = MOS_STATUS_SUCCESS;
    if (pVeboxState == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("pVeboxState not available.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    pRenderHal              = pVeboxState->m_pRenderHal;
    pOsInterface            = pVeboxState->m_pOsInterface;
    iRemaining              = 0;
    pVeboxInterface         = pVeboxState->m_pVeboxInterface;

    VPHAL_RENDER_CHK_NULL(pRenderData);

    VPHAL_RENDER_CHK_STATUS(VeboxSendVeboxCmd_Prepare(
        CmdBuffer,
        GenericPrologParams,
        GpuStatusBuffer,
        iRemaining));

    VPHAL_RENDER_CHK_STATUS(VeboxRenderVeboxCmd(
        CmdBuffer,
        VeboxDiIecpCmdParams,
        VeboxSurfaceStateCmdParams,
        MhwVeboxSurfaceStateCmdParams,
        VeboxStateCmdParams,
        FlushDwParams,
        &GenericPrologParams));

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
        CmdBuffer.pCmdPtr     = CmdBuffer.pCmdBase + CmdBuffer.iOffset/sizeof(uint32_t);
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);

    VPHAL_RENDER_CHK_STATUS(VeboxSyncIndirectStateCmd());
    VPHAL_RENDER_CHK_STATUS(VeboxSendVeboxCmdSetParamBeforeSubmit());

    // Flush the command buffer
    // WARNING: This CHK_STATUS (former CHK_HR) might cause an infinite loop when fail!
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSubmitCommandBuffer(
        pOsInterface,
        &CmdBuffer,
        pVeboxState->bNullHwRenderDnDi));

    VpHal_RndrUpdateStatusTableAfterSubmit(pOsInterface, &m_StatusTableUpdateParams, MOS_GPU_CONTEXT_VEBOX, eStatus);

    if (pVeboxState->bNullHwRenderDnDi == false)
    {
        pVeboxInterface->UpdateVeboxSync( );
    }

    return eStatus;
}

//!
//! \brief    Sync for Indirect state Copy and Update Kernels
//! \details  Sync for Indirect state Copy and Update Kernels before Send Vebox ring Commands.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxSyncIndirectStateCmd()
{
#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (GetLastExecRenderData()->bAutoDenoise)
    {
        // Make sure copy kernel and update kernels are finished before submitting
        // VEBOX commands

        m_pOsInterface->pfnSyncGpuContext(
            m_pOsInterface,
            RenderGpuContext,
            MOS_GPU_CONTEXT_VEBOX);
    }
#endif

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Vebox set common rendering flag
//! \details  Common flags should be set before other flags,
//!           and it should be independent with other flags
//! \param    [in] pSrc
//!           Pointer to input surface of Vebox
//! \param    [in] pRenderTarget
//!           Pointer to Render targe surface of VPP BLT
//! \return   void
//!
void VPHAL_VEBOX_STATE::VeboxSetCommonRenderingFlags(
    PVPHAL_SURFACE              pSrc,
    PVPHAL_SURFACE              pRenderTarget)
{
    PVPHAL_SURFACE              pRef;
    PVPHAL_SURFACE              pCurSurf;
    PVPHAL_SURFACE              pPrvSurf;
    int32_t                     iSameSampleThreshold;
    PVPHAL_VEBOX_STATE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    if (IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState))
    {
        // Treat forward frame as current frame input to vebox
        // and current frame as previous frame input to vebox
        pRef     = pSrc->pFwdRef;
        pCurSurf = pRef;
        pPrvSurf = pSrc;
        VPHAL_RENDER_ASSERT(pRef && pSrc->uFwdRefCount > 0);
    }
    else
    {
        // In serial mode (mode0) or transition mode (mode0to2),
        // treat current frame as current frame input to vebox
        // and previous frame as previous frame input to vebox
        // if a previous exists.
        // It is valid case to have no previous frame reference
        pRef     = (pSrc->uBwdRefCount > 0) ? pSrc->pBwdRef : nullptr;
        pCurSurf = pSrc;
        pPrvSurf = pRef;
    }

    iSameSampleThreshold        = pVeboxState->iSameSampleThreshold;

    VpHal_GetScalingRatio(pSrc, pRenderTarget, &pRenderData->fScaleX, &pRenderData->fScaleY);

    pRenderData->bProgressive   = (pSrc->SampleType == SAMPLE_PROGRESSIVE);

    pRenderData->bDenoise       = (pSrc->pDenoiseParams                 &&
                                   pSrc->pDenoiseParams->bEnableLuma    &&
                                   pVeboxState->IsDnFormatSupported(pSrc));

    pRenderData->bChromaDenoise = (pSrc->pDenoiseParams                 &&
                                   pSrc->pDenoiseParams->bEnableChroma  &&
                                   pSrc->pDenoiseParams->bEnableLuma    &&
                                   pVeboxState->IsDnFormatSupported(pSrc));

#if VEBOX_AUTO_DENOISE_SUPPORTED
    pRenderData->bAutoDenoise   = (pRenderData->bDenoise                &&
                                   pSrc->pDenoiseParams                 &&
                                   pSrc->pDenoiseParams->bAutoDetect    &&
                                   pVeboxState->IsDnFormatSupported(pSrc));
#endif


    // Free dDenoiseParams when DN don`t support source format
    // to avoid the possible using by mistake, 8 alignement for DN in renderhal 
    if ((!pRenderData->bDenoise) && (pSrc->pDenoiseParams != nullptr))
    {
        MOS_FreeMemAndSetNull(pSrc->pDenoiseParams);
    }

    pRenderData->bDeinterlace   = (pVeboxState->IsDiFormatSupported(pSrc)            &&
                                   ((pSrc->pDeinterlaceParams                        &&
                                    pSrc->pDeinterlaceParams->DIMode == DI_MODE_ADI) ||
                                    (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData)           &&
                                     pSrc->pDeinterlaceParams                        &&
                                     pSrc->pDeinterlaceParams->DIMode == DI_MODE_BOB)));

    pRenderData->bRefValid      = (pRef                                 &&
                                   (pSrc->Format   == pRef->Format)     &&
                                   (pSrc->dwWidth  == pRef->dwWidth)    &&
                                   (pSrc->dwHeight == pRef->dwHeight)   &&
                                   (pSrc->FrameID  != pRef->FrameID));

    // Flags needs to be set if the reference sample is valid
    if (pRenderData->bRefValid)
    {
        pRenderData->bSameSamples   = 
               WITHIN_BOUNDS(
                      pCurSurf->FrameID - pVeboxState->iCurFrameID,
                      -iSameSampleThreshold,
                      iSameSampleThreshold) && 
               WITHIN_BOUNDS(
                      pPrvSurf->FrameID - pVeboxState->iPrvFrameID,
                      -iSameSampleThreshold,
                      iSameSampleThreshold);

        pRenderData->bOutOfBound    = 
               OUT_OF_BOUNDS(
                      pPrvSurf->FrameID - pVeboxState->iCurFrameID,
                      -iSameSampleThreshold,
                      iSameSampleThreshold);
    }
    // bSameSamples flag also needs to be set for no reference case
    else 
    {
         pRenderData->bSameSamples  = 
               WITHIN_BOUNDS(
                      pCurSurf->FrameID - pVeboxState->iCurFrameID,
                      -iSameSampleThreshold,
                      iSameSampleThreshold);
    }

    pVeboxState->bSameSamples = pRenderData->bSameSamples;

    // Cache Render Target pointer
    pRenderData->pRenderTarget = pRenderTarget;
}

//!
//! \brief    Vebox set Field related rendering flag
//! \details  Set Field related flags for interlaced or related features
//! \param    [in] pSrc
//!           Pointer to input surface of Vebox
//! \return   void
//!
void VPHAL_VEBOX_STATE::VeboxSetFieldRenderingFlags(
    PVPHAL_SURFACE              pSrc)
{
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    // No need to check future surface for Mode2 here
    // Because only current frame will change the field setting.
    // And whether current blt are top or bottom field doesn't matter here
    pRenderData->bTFF =
        (pSrc->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD) ||
        (pSrc->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD);

    // Seting bTopField flag
    pRenderData->bTopField =
        (pSrc->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD) ||
        (pSrc->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD);
}

//!
//! \brief    Vebox set rendering flag
//! \details  Setup Rendering Flags due to different usage case
//! \param    [in] pSrc
//!           Pointer to input surface of Vebox
//! \param    [in] pRenderTarget
//!           Pointer to Render targe surface of VPP BLT
//! \return   void
//!
void VPHAL_VEBOX_STATE::VeboxSetRenderingFlags(
    PVPHAL_SURFACE              pSrc,
    PVPHAL_SURFACE              pRenderTarget)
{
    PRENDERHAL_INTERFACE        pRenderHal;
    PMOS_INTERFACE              pOsInterface;
    PVPHAL_VEBOX_STATE           pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA     pRenderData = GetLastExecRenderData();

    pRenderHal                  = pVeboxState->m_pRenderHal;
    pOsInterface                = pVeboxState->m_pOsInterface;

    VeboxSetCommonRenderingFlags(pSrc, pRenderTarget);

    // surface height should be a multiple of 4 when DN/DI is enabled and input format is Planar 420
    if ((IS_VEBOX_SURFACE_HEIGHT_UNALIGNED(pSrc, 4)) &&
        (pSrc->Format == Format_P010                 ||
         pSrc->Format == Format_P016                 ||
         pSrc->Format == Format_NV12))
    {
        pRenderData->bDenoise     = false;
        pRenderData->bDeinterlace = false;
    }

    // surface height should be a multiple of 2 for all format 
    // when Denoise is enabled and progressiveDN is disabled
    if (IS_VEBOX_SURFACE_HEIGHT_UNALIGNED(pSrc, 2) &&
        pRenderData->bDenoise                      &&
        (!pRenderData->bProgressive))
    {
        pRenderData->bDenoise     = false;
    }

    // Flags only needs to be set if deinterlacing is needed
    if (pRenderData->bDeinterlace)
    {
        VeboxSetFieldRenderingFlags(pSrc);

        pRenderData->bSingleField   = (pRenderData->bRefValid                           &&
                                        pSrc->pDeinterlaceParams->DIMode != DI_MODE_BOB) ?
                                        pSrc->pDeinterlaceParams->bSingleField            :
                                        true;

        // Detect ADI mode (30i->30fps or 30i->60fps) according to DDI
        pRenderData->b60fpsDi       = !pSrc->pDeinterlaceParams->bSingleField;
    }

    // Need to refine later
    // Actually, behind CSC can do nothing which is related to degamma/gamma 
    pRenderData->bBeCsc             = (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData) &&
                                        pSrc->ColorSpace != pRenderTarget->ColorSpace);

    pRenderData->bProcamp           = (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData)  &&
                                        pSrc->pProcampParams                    &&
                                        pSrc->pProcampParams->bEnabled);

    pRenderData->bColorPipe         = (pSrc->pColorPipeParams &&
                                        (pSrc->pColorPipeParams->bEnableSTE     ||
                                         pSrc->pColorPipeParams->bEnableTCC));

    pRenderData->bIECP              = (pSrc->pColorPipeParams &&
                                        (pSrc->pColorPipeParams->bEnableSTE     ||
                                         pSrc->pColorPipeParams->bEnableTCC)    ||
                                        pRenderData->bBeCsc                     ||
                                        pRenderData->bProcamp);

    // Vebox can be bypassed if no feature is needed
    if (!(pRenderData->bDenoise          ||
          pRenderData->bDeinterlace      ||
          pRenderData->bIECP             ||
          IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData)))
    {
        pRenderData->bVeboxBypass = true;
    }
}

//!
//! \brief    Vebox initialize STMM History
//! \details  Initialize STMM History surface
//! Description:
//!   This function is used by VEBox for initializing
//!   the STMM surface.  The STMM / Denoise history is a custom surface used 
//!   for both input and output. Each cache line contains data for 4 4x4s. 
//!   The STMM for each 4x4 is 8 bytes, while the denoise history is 1 byte 
//!   and the chroma denoise history is 1 byte for each U and V.
//!   Byte    Data\n
//!   0       STMM for 2 luma values at luma Y=0, X=0 to 1\n
//!   1       STMM for 2 luma values at luma Y=0, X=2 to 3\n
//!   2       Luma Denoise History for 4x4 at 0,0\n
//!   3       Not Used\n
//!   4-5     STMM for luma from X=4 to 7\n
//!   6       Luma Denoise History for 4x4 at 0,4\n
//!   7       Not Used\n
//!   8-15    Repeat for 4x4s at 0,8 and 0,12\n
//!   16      STMM for 2 luma values at luma Y=1,X=0 to 1\n
//!   17      STMM for 2 luma values at luma Y=1, X=2 to 3\n
//!   18      U Chroma Denoise History\n
//!   19      Not Used\n
//!   20-31   Repeat for 3 4x4s at 1,4, 1,8 and 1,12\n
//!   32      STMM for 2 luma values at luma Y=2,X=0 to 1\n
//!   33      STMM for 2 luma values at luma Y=2, X=2 to 3\n
//!   34      V Chroma Denoise History\n
//!   35      Not Used\n
//!   36-47   Repeat for 3 4x4s at 2,4, 2,8 and 2,12\n
//!   48      STMM for 2 luma values at luma Y=3,X=0 to 1\n
//!   49      STMM for 2 luma values at luma Y=3, X=2 to 3\n
//!   50-51   Not Used\n
//!   36-47   Repeat for 3 4x4s at 3,4, 3,8 and 3,12\n
//! \param    [in] iSurfaceIndex
//!           Index of STMM surface array
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxInitSTMMHistory(
    int32_t                      iSurfaceIndex)
{
    MOS_STATUS          eStatus;
    PMOS_INTERFACE      pOsInterface;
    uint32_t            dwSize;
    int32_t             x, y;
    uint8_t*            pByte;
    MOS_LOCK_PARAMS     LockFlags;
    PVPHAL_VEBOX_STATE  pVeboxState = this;

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = pVeboxState->m_pOsInterface;

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly    = 1;
    LockFlags.TiledAsTiled = 1; // Set TiledAsTiled flag for STMM surface initialization.

    // Lock the surface for writing
    pByte = (uint8_t*)pOsInterface->pfnLockResource(
        pOsInterface,
        &(pVeboxState->STMMSurfaces[iSurfaceIndex].OsResource),
        &LockFlags);

    VPHAL_RENDER_CHK_NULL(pByte);

    dwSize = pVeboxState->STMMSurfaces[iSurfaceIndex].dwWidth >> 2;

    // Fill STMM surface with DN history init values.
    for (y = 0; y < (int32_t)pVeboxState->STMMSurfaces[iSurfaceIndex].dwHeight; y++)
    {
        for (x = 0; x < (int32_t)dwSize; x++)
        {
            MOS_FillMemory(pByte, 2, DNDI_HISTORY_INITVALUE);
            // skip denosie history init.
            pByte += 4;
        }

        pByte += pVeboxState->STMMSurfaces[iSurfaceIndex].dwPitch - pVeboxState->STMMSurfaces[iSurfaceIndex].dwWidth;
    }

    // Unlock the surface
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnUnlockResource(
            pOsInterface,
            &(pVeboxState->STMMSurfaces[iSurfaceIndex].OsResource)));

finish:
    return eStatus;
}

#if VEBOX_AUTO_DENOISE_SUPPORTED
//!
//! \brief    Vebox initialize Spatial Configuration Surface
//! \details  Initialize Spatial Configuration surface
//! Description:
//!   This function is used by VEBox for initializing
//!   the Spatial Attributes Configuration surface. 
//!   The GEN9+ DN kernel will use the init data in this surface and write back output data 
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxInitSpatialAttributesConfiguration()
{
    MOS_STATUS          eStatus;
    PMOS_INTERFACE      pOsInterface;
    VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION *pSpatialAttributesConfiguration;
    MOS_LOCK_PARAMS     LockFlags;
    PVPHAL_VEBOX_STATE  pVeboxState = this;

    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = pVeboxState->m_pOsInterface;

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    // Lock the surface for writing
    pSpatialAttributesConfiguration = (VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION*)pOsInterface->pfnLockResource(
        pOsInterface,
        &(pVeboxState->VeboxSpatialAttributesConfigurationSurface.OsResource),
        &LockFlags);

    VPHAL_RENDER_CHK_NULL(pSpatialAttributesConfiguration);
    *pSpatialAttributesConfiguration = g_cInit_VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION;

    // Unlock the surface
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnUnlockResource(
        pOsInterface,
        &(pVeboxState->VeboxSpatialAttributesConfigurationSurface.OsResource)));

finish:
    return eStatus;
}
#endif

//!
//! \brief    Update Vebox Execution State for Vebox/Render parallelism
//! \details
//!     Purpose   : Handle Vebox execution state machine transitions
//!
//!     Mode0:    Enter or stay in this state as long has (a) there are no future 
//!               frames present or (b) FRC is active. Parallel execution is
//!               handled different in FRC mode. (c) Vebox/SFC output path is
//!               applied. Parallel execution is not needed when it is Vebox/SFC 
//!               to output. Mode0 is considered the legacy serial vebox execution mode.
//!     Mode0To2: Enter this state when a future frame becomes present. In this
//!               state, perform a one time start up sequence in order to transistion
//!               to Mode2 parallel execution state. 
//!     Mode2:    Enter this state as long a future frame is present. This is the
//!               steady parallel execution state where we process 1 frame ahead.
//!               i.e. On BLT(N), we do vebox on the future frame N+1 and composite 
//!               frame N in the same BLT(). 
//!     Mode2To0: Enter this state when in Mode2 and no future frame is present. 
//!               Transition back to Mode0.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] OutputPipe
//!           The output path the driver uses to write the RenderTarget
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::UpdateVeboxExecutionState(
    PVPHAL_SURFACE          pSrcSurface,
    VPHAL_OUTPUT_PIPE_MODE  OutputPipe)
{
    MOS_STATUS  eStatus;
    bool        bSameSamples;
    bool        bOutOfBound;
    int32_t     iSameSampleThreshold;
    PVPHAL_VEBOX_STATE      pVeboxState  = this;

    VPHAL_RENDER_ASSERT(pVeboxState);
    VPHAL_RENDER_ASSERT(pSrcSurface);

    eStatus                 = MOS_STATUS_SUCCESS;
    bSameSamples            = false;
    bOutOfBound             = false;
    iSameSampleThreshold    = pVeboxState->iSameSampleThreshold;;

    if (IS_VEBOX_EXECUTION_MODE_PARALLEL_CAPABLE(pVeboxState->m_pVeboxExecState))
    {
        if ((pVeboxState->m_pVeboxExecState->bFrcActive)  ||
            (OutputPipe != VPHAL_OUTPUT_PIPE_MODE_COMP))
        {
            SET_VEBOX_EXECUTION_MODE(pVeboxState->m_pVeboxExecState, VEBOX_EXEC_MODE_0);
        }
        else if (pSrcSurface->uFwdRefCount == 0) 
        {
            // Transition Mode2 to Mode0
            if (IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState))
            {
                SET_VEBOX_EXECUTION_MODE(pVeboxState->m_pVeboxExecState, VEBOX_EXEC_MODE_2_TO_0);

                // In the following case
                //    Blt#    Bwd   Curr  Fwd       Field    bSameSample
                //    ----------------------------------------------------------
                //    Blt0    F1     F2    F3       Top       false
                //    Blt1    F2     F3    nullptr  Bot       true -> No render
                // Driver will still output last blt result through the SameSample flow.
                // Since mode 2 has already toggled the output pair,
                // we need to toggle the output pair back.
                // For other cases that introduce new render, RenderMode0() will reset the outputpair.
                pVeboxState->m_pVeboxExecState->bDIOutputPair01 = !pVeboxState->m_pVeboxExecState->bDIOutputPair01;

                if (IS_VEBOX_EXECUTION_MODE_2_TO_0(pVeboxState->m_pVeboxExecState))
                {
                    SET_VEBOX_EXECUTION_MODE(pVeboxState->m_pVeboxExecState, VEBOX_EXEC_MODE_0);
                }
            }
            else // Steady Mode0 state
            {
                VPHAL_RENDER_ASSERT(
                    IS_VEBOX_EXECUTION_MODE_0(pVeboxState->m_pVeboxExecState) || 
                    IS_VEBOX_EXECUTION_MODE_2_TO_0(pVeboxState->m_pVeboxExecState));
            }
        }
        else // Future Frame present
        {
            // Transition Mode0 to Mode2
            if (IS_VEBOX_EXECUTION_MODE_0(pVeboxState->m_pVeboxExecState))
            {
                // Raise the FFDI surface number for mode 2 use.
                pVeboxState->iNumFFDISurfaces = 4;

                // When previous blt is Mode0,
                // pVeboxState->iCurFrameID are previous blt's current frame
                // pVeboxState->iPrvFrameID are previous blt's bwd frame
                bSameSamples   = 
                    (pSrcSurface->uBwdRefCount > 0 && pSrcSurface->pBwdRef) &&
                    WITHIN_BOUNDS(
                            pSrcSurface->FrameID - pVeboxState->iCurFrameID,
                            -iSameSampleThreshold,
                            iSameSampleThreshold) && 
                    WITHIN_BOUNDS(
                            pSrcSurface->pBwdRef->FrameID - pVeboxState->iPrvFrameID,
                            -iSameSampleThreshold,
                            iSameSampleThreshold);

                bOutOfBound    = 
                    (pSrcSurface->uBwdRefCount > 0 && pSrcSurface->pBwdRef) &&
                    OUT_OF_BOUNDS(
                            pSrcSurface->pBwdRef->FrameID - pVeboxState->iCurFrameID,
                            -iSameSampleThreshold,
                            iSameSampleThreshold);

                // Only switch to mode 2 for non-SameSample, non-frame repeat,
                // and non-frame drop case.
                // The case we want to switch mode are marked OK in this table:
                // bSameSamples bOutOfBound Switch   Note
                // ----------------------------------------------------------
                //    true        true       NG     Same Sample / Frame Repeat
                //    true        false      NG     Self-reference frame Repeat
                //    false       true       NG     Frame Drop
                //    false       false      OK     Normal case
                //
                // 1. SameSample case in interlaced mode. Already has sample
                //    for output and will not do new render, and thus no way
                //    to switch to mode 2. Switch to Mode0To2 now will stuck
                //    in the state forever.
                //    One example that causing the issue:
                //    Blt#    Bwd   Curr  Fwd       Field    bSameSamples
                //    ----------------------------------------------------------
                //    Blt0    F1     F2    nullptr  Top       false
                //    Blt1    F1     F2    F3       Bot       true -> No render
                // 2. Frame Reapt case in both progressive/interlaced modes.
                //    Future frame might stay future for a while and not in use
                //    as next blt's current. 
                //    And, in mode 2 driver will output the previous blt's future frame.
                //    In repeating frame scene it is not expected to do so.
                //    Keep in mode 0 until new frames are received.
                // 3. Frame-drop case, the player or hw resource might not 
                //    rich enough yet, another frame drop might occur immediately.
                //    To help smooth playback, only switch to mode2 when the player
                //    can provide consecutive two blts without frame drop.
                if (!bSameSamples && !bOutOfBound)
                {
                    SET_VEBOX_EXECUTION_MODE(pVeboxState->m_pVeboxExecState, VEBOX_EXEC_MODE_0_TO_2);
                }
            }
            else // Steady Mode2 state
            {
                VPHAL_RENDER_ASSERT(
                    IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState) ||
                    IS_VEBOX_EXECUTION_MODE_0_TO_2(pVeboxState->m_pVeboxExecState));

                VPHAL_RENDER_ASSERT(pSrcSurface->pFwdRef);

                // When previous blt is Mode2,
                // pVeboxState->iCurFrameID are previous blt's future frame
                // pVeboxState->iPrvFrameID are previous blt's current frame
                bSameSamples   = 
                    WITHIN_BOUNDS(
                            pSrcSurface->pFwdRef->FrameID - pVeboxState->iCurFrameID,
                            -iSameSampleThreshold,
                            iSameSampleThreshold) && 
                    WITHIN_BOUNDS(
                            pSrcSurface->FrameID - pVeboxState->iPrvFrameID,
                            -iSameSampleThreshold,
                            iSameSampleThreshold);

                bOutOfBound    = 
                    OUT_OF_BOUNDS(
                            pSrcSurface->FrameID - pVeboxState->iCurFrameID,
                            -iSameSampleThreshold,
                            iSameSampleThreshold);

                // If previous blt's future frame are not coming as this blt's
                // current frame, we cannot use the pre-rendered results.
                // Two possibilities:
                // Case 1: SameSameples.
                //         Current remains current, future remains future.
                //         a. For interlaced, Keep in Mode 2, can reuse the previous output.
                //         b. For progressive, current flow is to render with new parameters.
                //            Cannot stay in mode2, to avoid output the previous blt's
                //            future frame as current.
                // Case 2: Discontinuity - hence can't reuse previous surfaces
                //         i.e !bSameSamples && OutOfBound.
                //         Nothing can be reused.
                //
                //         Both cases can switch back to mode 0To2 for render new current frame
                //         as well as new future frame.
                //         But such abnormal condition often due to player's intended bahavior.
                //         It may outputing static menu, or wrong reference frame given.
                //         The extra render of furture frame in Mode0To2 might not be 
                //         used in next blt.
                //         So switch to mode0 to save bandwidth.
                //         If the player can keep providing future frame without repeating
                //         or frame drop, we can resume to mode 2 in next blt.

                if ((!bSameSamples && bOutOfBound) ||
                    (bSameSamples && !pSrcSurface->pDeinterlaceParams))
                {
                    SET_VEBOX_EXECUTION_MODE(pVeboxState->m_pVeboxExecState, VEBOX_EXEC_MODE_0);
                }

            }
        }
    }

    return eStatus;
}

//!
//! \brief    Copy and update vebox state
//! \details  Copy and update vebox state for input frame.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxCopyAndUpdateVeboxState(
    PVPHAL_SURFACE           pSrcSurface)
{
    PVPHAL_VEBOX_STATE       pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_ASSERT(pVeboxState);
    VPHAL_RENDER_ASSERT(pRenderData);
    VPHAL_RENDER_ASSERT(pSrcSurface);

    // Setup VEBOX State
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxSetupIndirectStates(
            pSrcSurface,
            IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData) ? 
            pRenderData->pRenderTarget              : 
            pVeboxState->FFDISurfaces[0]));

    // Copy VEBOX State
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxCopyVeboxStates());

    // Update VEBOX State
    VPHAL_RENDER_CHK_STATUS(VeboxUpdateVeboxStates(pSrcSurface));

finish:
    return eStatus;
}

//!
//! \brief    Vebox render mode2
//! \details  VEBOX/IECP Rendering for future frame
//!           [Flow] 1. For future frame; send cmd.
//!                  2. setup state for next vebox operation.
//!                  3. Request "speculative" copy state, update state.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pOutputSurface
//!           Pointer to output surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxRenderMode2(
    PVPHAL_SURFACE           pSrcSurface,
    PVPHAL_SURFACE           pOutputSurface)
{
    PMOS_INTERFACE           pOsInterface;
    PVPHAL_SURFACE           pRefSurface;
    MOS_STATUS               eStatus;
    PVPHAL_VEBOX_STATE       pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    MOS_UNUSED(pOutputSurface);

    VPHAL_RENDER_ASSERT(pVeboxState);
    VPHAL_RENDER_ASSERT(pRenderData);
    VPHAL_RENDER_ASSERT(pSrcSurface);
    VPHAL_RENDER_ASSERT(pOutputSurface);
    VPHAL_RENDER_ASSERT((IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState) == true));

    // Initialize Variables
    pOsInterface    = pVeboxState->m_pOsInterface;
    eStatus         = MOS_STATUS_SUCCESS;

    // Ensure the input is ready to be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface, 
        &pSrcSurface->OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    if (pRenderData->bRefValid)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pSrcSurface->pFwdRef->OsResource,
            MOS_GPU_CONTEXT_VEBOX,
            false);
    }

    // Set up reference surfaces. Should pick up future frame
    pRefSurface = VeboxSetReference(pSrcSurface);

    // Set current DN output buffer
    pRenderData->iCurDNOut = pVeboxState->iCurDNIndex;

    // Set the FMD output frames
    if (pVeboxState->m_pVeboxExecState->bDIOutputPair01 == true)
    {
        pRenderData->iFrame0      = 0;
        pRenderData->iFrame1      = 1;
        pVeboxState->m_pVeboxExecState->bDIOutputPair01 = false;
    }
    else
    {
        pRenderData->iFrame0      = 2;
        pRenderData->iFrame1      = 3;
        pVeboxState->m_pVeboxExecState->bDIOutputPair01 = true;
    }

    // Setup Motion history for DI
    // for ffDI, ffDN and ffDNDI cases
    pRenderData->iCurHistIn  = (pVeboxState->iCurStmmIndex) & 1;
    pRenderData->iCurHistOut = (pVeboxState->iCurStmmIndex + 1) & 1;

    // Set current frame. Previous frame is set in VeboxSetReference()
    CopySurfaceValue(pVeboxState->m_currentSurface, pSrcSurface->pFwdRef);

    // Set current/previous timestamps for next call
    pVeboxState->iCurFrameID = pSrcSurface->pFwdRef->FrameID;
    pVeboxState->iPrvFrameID = pSrcSurface->FrameID;

    // Allocate Resources if needed
    VPHAL_RENDER_CHK_STATUS(pVeboxState->AllocateResources());

    // For CP HM which requires to use render engine for copy and
    // update Vebox heap, speculative copy has already been done in previous
    // blt call to increase the Vebox & Render engine parallelism.
    // For LM, CPU can lock & update the resource quickly here because
    // previous blt's Vebox workload should be already done. 
    // Thus, here we do the copy & update only for LM.
    if (!pOsInterface->osCpInterface->IsHMEnabled())
    {
        // Setup, Copy and Update VEBOX State
        VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxCopyAndUpdateVeboxState(pSrcSurface));
    }

    // Send VEBOX Command Buffer
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxSendVeboxCmd());

#if 0
    // FMD Calc extra variances has been moved to after composition
    // to prevent blocking Vebox / Composition parallelism.
    REQUEST_VEBOX_POSTPONED_FMD_CALC(pVeboxState->m_pVeboxExecState);
#endif

    //--------------------------------------------------------------------------
    // ffDN and ffDNDI cases
    //--------------------------------------------------------------------------
    if (pRenderData->bDenoise)
    {
        CopySurfaceValue(pVeboxState->m_currentSurface, pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]);
    }

    //--------------------------------------------------------------------------
    // Swap buffers for next iteration
    //--------------------------------------------------------------------------
    pVeboxState->iCurDNIndex     = (pRenderData->iCurDNOut + 1) & 1;
    pVeboxState->iCurStmmIndex   = (pVeboxState->iCurStmmIndex + 1) & 1;

finish:
    return eStatus;
}

//!
//! \brief    Vebox render mode0to2
//! \details  Purpose   : VEBOX/IECP Rendering for current and future frame
//!           [Flow] 1. For current frame; setup state, copy state, update state, send cmd.
//!                  2. For future frame;  setup state, copy state, update state, send cmd.
//!                  3. setup state for next vebox operation.
//!                  4. Request "speculative" copy state, update state.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pOutputSurface
//!           Pointer to output surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxRenderMode0To2(
    PVPHAL_SURFACE           pSrcSurface,
    PVPHAL_SURFACE           pOutputSurface)
{
    PMOS_INTERFACE           pOsInterface;
    PVPHAL_SURFACE           pRefSurface;
    MOS_STATUS               eStatus;
    PVPHAL_VEBOX_STATE       pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    MOS_UNUSED(pOutputSurface);

    VPHAL_RENDER_ASSERT(pVeboxState);
    VPHAL_RENDER_ASSERT(pRenderData);
    VPHAL_RENDER_ASSERT(pSrcSurface);
    VPHAL_RENDER_ASSERT(pOutputSurface);
    VPHAL_RENDER_ASSERT((IS_VEBOX_EXECUTION_MODE_0_TO_2(pVeboxState->m_pVeboxExecState) == true));

    // Initialize Variables
    pOsInterface    = pVeboxState->m_pOsInterface;
    eStatus         = MOS_STATUS_SUCCESS;

    // =========================================================================
    // 1st operation (frame 1 - current surface)
    // =========================================================================

    // Ensure the input is ready to be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface, 
        &pSrcSurface->OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    if (pRenderData->bRefValid)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pSrcSurface->pBwdRef->OsResource,
            MOS_GPU_CONTEXT_VEBOX,
            false);
    }

    // Set up reference surfaces
    pRefSurface = VeboxSetReference(pSrcSurface);

    // Set current DN output buffer
    pRenderData->iCurDNOut = pVeboxState->iCurDNIndex;

    // To avoid ambiguity, for Mode0To2,
    // Always render 1st frame to 01 pair,
    //        render 2nd frame to 23 pair,
    //        and use 01 pair as output in mode0To2.
    // No need to toggle the DIOutputPair01 flag here.
    pVeboxState->m_pVeboxExecState->bDIOutputPair01 = true;

    // Set the FMD output frames
    pRenderData->iFrame0 = 0;
    pRenderData->iFrame1 = 1;

    // Setup Motion history for DI
    // for ffDI, ffDN and ffDNDI cases
    pRenderData->iCurHistIn  = (pVeboxState->iCurStmmIndex) & 1;
    pRenderData->iCurHistOut = (pVeboxState->iCurStmmIndex + 1) & 1;

    // Set current src = current primary input
    CopySurfaceValue(pVeboxState->m_currentSurface, pSrcSurface);

    // Allocate Resources if needed
    VPHAL_RENDER_CHK_STATUS(pVeboxState->AllocateResources());

    // Set current/previous timestamps for next call
    pVeboxState->iCurFrameID = pSrcSurface->FrameID;

    if (pRenderData->bRefValid)
    {
        VPHAL_RENDER_CHK_NULL(pRefSurface);

        pVeboxState->iPrvFrameID = pRefSurface->FrameID;
    }
    else
    {
        pVeboxState->iPrvFrameID = -1;
    }

    // Setup, Copy and Update VEBOX State
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxCopyAndUpdateVeboxState(pSrcSurface));

    // Send VEBOX Command Buffer
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxSendVeboxCmd());

    // Next phase synchronization with Render Engine
    pOsInterface->pfnSyncGpuContext(
        pOsInterface,
        MOS_GPU_CONTEXT_VEBOX,
        RenderGpuContext);

    //--------------------------------------------------------------------------
    // ffDN and ffDNDI cases
    //--------------------------------------------------------------------------
    if (pRenderData->bDenoise)
    {
        CopySurfaceValue(pVeboxState->m_currentSurface, pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]);
    }

    //--------------------------------------------------------------------------
    // Swap buffers for next iteration
    //--------------------------------------------------------------------------
    pVeboxState->iCurDNIndex     = (pRenderData->iCurDNOut + 1) & 1;
    pVeboxState->iCurStmmIndex   = (pVeboxState->iCurStmmIndex + 1) & 1;

    // Set the first frame flag
    if (pVeboxState->bFirstFrame)
    {
        pVeboxState->bFirstFrame = false;
    }

    // End 1st operation (frame 1 - current surface)

    // Switch state
    SET_VEBOX_EXECUTION_MODE(pVeboxState->m_pVeboxExecState, VEBOX_EXEC_MODE_2);

    // In VpHal_VeboxGetStatisticsSurfaceOffsets, the offset of vebox statistics surface 
    // address is based on whether bDN/DIEnabled in the previous function call. We get
    // bDN/DIEnabled here, so that bDN/DIEnabled can be used at the beginning of the next
    // function call. 
    // When Spatial DI enabled and Temporal DI disabled, the vebox statistics surface
    // layout is same as the case when only DN enabled, so use DNEnabled to include the
    // case when Spatial DI enabled. 
    pVeboxState->bDNEnabled = pRenderData->bDenoise         ||
                             pRenderData->bChromaDenoise    ||
                             ((pRenderData->bDeinterlace    ||
                               pVeboxState->IsQueryVarianceEnabled()) && 
                              !pRenderData->bRefValid);

    pVeboxState->bDIEnabled = (pRenderData->bDeinterlace   || 
                              pVeboxState->IsQueryVarianceEnabled()) && 
                             pRenderData->bRefValid;

    // =========================================================================
    // 2nd operation (frame 2 - future surface)
    // =========================================================================

    // Set render flags for frame 2
    pVeboxState->VeboxSetRenderingFlags(
        pSrcSurface,
        pRenderData->pRenderTarget);

    if (pRenderData->bRefValid)
    {
        // Ensure the input is ready to be read
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pSrcSurface->pFwdRef->OsResource,
            MOS_GPU_CONTEXT_VEBOX,
            false);
    }

    // Set up reference surfaces.
    // It set pVeboxState->PreviousSurface w/ the correct surface.
    // pReference is not used.
    pRefSurface = VeboxSetReference(pSrcSurface);

    // Set current DN output buffer
    pRenderData->iCurDNOut = pVeboxState->iCurDNIndex;

    // Set the FMD output frames
    pRenderData->iFrame0 = 2;
    pRenderData->iFrame1 = 3;

    // Setup Motion history for DI
    // for ffDI, ffDN and ffDNDI cases
    pRenderData->iCurHistIn  = (pVeboxState->iCurStmmIndex) & 1;
    pRenderData->iCurHistOut = (pVeboxState->iCurStmmIndex + 1) & 1;

    // Set current frame. Previous frame is set in VeboxSetReference()
    CopySurfaceValue(pVeboxState->m_currentSurface, pSrcSurface->pFwdRef);

    // Set current/previous timestamps for next call
    pVeboxState->iCurFrameID = pSrcSurface->pFwdRef->FrameID;
    pVeboxState->iPrvFrameID = pSrcSurface->FrameID;

    // Setup, Copy and Update VEBOX State
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxCopyAndUpdateVeboxState(pSrcSurface));

    // Send VEBOX Command Buffer
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxSendVeboxCmd());

    //--------------------------------------------------------------------------
    // ffDN and ffDNDI cases
    //--------------------------------------------------------------------------
    if (pRenderData->bDenoise)
    {
        CopySurfaceValue(pVeboxState->m_currentSurface, pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]);
    }

    //--------------------------------------------------------------------------
    // Swap buffers for next iteration
    //--------------------------------------------------------------------------
    pVeboxState->iCurDNIndex     = (pRenderData->iCurDNOut + 1) & 1;
    pVeboxState->iCurStmmIndex   = (pVeboxState->iCurStmmIndex + 1) & 1;

    // End 2nd operation (frame 2 - future surface)

finish:
    return eStatus;
}

//!
//! \brief    Check whether DN surface limitation is satisfied
//! \param    [in] bDenoise
//!           Flag to indicate whether DN is enabled
//! \param    [in] CurrentSurface
//!           Input surface of Vebox
//! \param    [in] FFDNSurface
//!           DN surface of Vebox
//! \return   bool
//!           Return true for limitation satisfied, otherwise false
//!
bool VPHAL_VEBOX_STATE::VeboxDNSurfaceLimitationSatisfied(
    bool                    bDenoise,
    VPHAL_SURFACE           *CurrentSurface,
    VPHAL_SURFACE           *FFDNSurface)
{
    if (bDenoise == false)
    {
        return true;
    }
    else
    {
        if (CurrentSurface->bIsCompressed   == FFDNSurface->bIsCompressed   &&
            CurrentSurface->bCompressible   == FFDNSurface->bCompressible   &&
            CurrentSurface->CompressionMode == FFDNSurface->CompressionMode &&
            CurrentSurface->dwWidth         == FFDNSurface->dwWidth         &&
            CurrentSurface->dwHeight        == FFDNSurface->dwHeight        &&
            CurrentSurface->dwPitch         == FFDNSurface->dwPitch)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

//!
//! \brief    Vebox Render mode0
//! \details  VEBOX/IECP Rendering for current frame
//!           [Flow] 1. For current frame; setup state, copy state, update state, send cmd.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pOutputSurface
//!           Pointer to output surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::VeboxRenderMode0(
    PVPHAL_SURFACE           pSrcSurface,
    PVPHAL_SURFACE           pOutputSurface)
{
    PMOS_INTERFACE        pOsInterface;
    PVPHAL_SURFACE        pRefSurface;
    MOS_STATUS            eStatus;
    PVPHAL_VEBOX_STATE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_ASSERT(pVeboxState);
    VPHAL_RENDER_ASSERT(pRenderData);
    VPHAL_RENDER_ASSERT(pSrcSurface);
    VPHAL_RENDER_ASSERT(pOutputSurface);
    VPHAL_RENDER_ASSERT((IS_VEBOX_EXECUTION_MODE_0(pVeboxState->m_pVeboxExecState) == true));

    // Initialize Variables
    pOsInterface            = pVeboxState->m_pOsInterface;
    eStatus                 = MOS_STATUS_SUCCESS;

    // Ensure the input is ready to be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface, 
        &pSrcSurface->OsResource,
        MOS_GPU_CONTEXT_VEBOX,
        false);

    if (pRenderData->bRefValid)
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface, 
            &pSrcSurface->pBwdRef->OsResource,
            MOS_GPU_CONTEXT_VEBOX,
            false);
    }

    // Set up reference surfaces
    pRefSurface = VeboxSetReference(pSrcSurface);

    // Set current DN output buffer
    pRenderData->iCurDNOut = pVeboxState->iCurDNIndex;

    // Set the FMD output frames
    pRenderData->iFrame0   = 0;
    pRenderData->iFrame1   = 1;

    // Always use 01 pair in mode0. In Mode2, may fall back to Mode0 so need to reset. 
    pVeboxState->m_pVeboxExecState->bDIOutputPair01 = true;

    // Setup Motion history for DI
    // for ffDI, ffDN and ffDNDI cases
    pRenderData->iCurHistIn  = (pVeboxState->iCurStmmIndex) & 1;
    pRenderData->iCurHistOut = (pVeboxState->iCurStmmIndex + 1) & 1;

    // Set current src = current primary input
    CopySurfaceValue(pVeboxState->m_currentSurface, pSrcSurface);

    // Set current/previous timestamps for next call
    // If DN surface limitation is not satisfied, e.g. input uncompressed, DN compressed, then need to re-allocate DN surfaces.
    // When rcMaxSrc changed we should call AllocateResources to increase Statistics buffer size.
    if (pRenderData->bSameSamples                    &&
        !pVeboxState->m_currentSurface->bMaxRectChanged &&
        IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData)        &&
        VeboxDNSurfaceLimitationSatisfied(pRenderData->bDenoise, pVeboxState->m_currentSurface, pVeboxState->FFDNSurfaces[0]))
    {
        // Do nothing when VEBOX+SFC scenario's BLT2 case hits here
    }
    else
    {
        // Allocate Resources if needed
        VPHAL_RENDER_CHK_STATUS(pVeboxState->AllocateResources());

        pVeboxState->iCurFrameID = pSrcSurface->FrameID;
    }

    if (pRenderData->bRefValid)
    {
        VPHAL_RENDER_CHK_NULL(pRefSurface);

        pVeboxState->iPrvFrameID = pRefSurface->FrameID;
    }
    else
    {
        if (pRenderData->bSameSamples  &&
            IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
        {
            // Do nothing when VEBOX+SFC scenario's BLT2 case hits here
        }
        else
        {
            pVeboxState->iPrvFrameID = -1;
        }
    }

    // Setup, Copy and Update VEBOX State
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxCopyAndUpdateVeboxState(pSrcSurface));

    // Setup SFC State if SFC is needed for current rendering
    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        m_sfcPipeState->SetStereoChannel(pVeboxState->uiCurrentChannel);

        VPHAL_RENDER_CHK_STATUS(m_sfcPipeState->SetupSfcState(
            pSrcSurface,
            pOutputSurface,
            pRenderData));
    }

    // Send VEBOX Command Buffer
    VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxSendVeboxCmd());

    //--------------------------------------------------------------------------
    // ffDN and ffDNDI cases
    //--------------------------------------------------------------------------
    if (pRenderData->bDenoise)
    {
        CopySurfaceValue(pVeboxState->m_currentSurface, pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]);
    }

    if ((pRenderData->bDeinterlace ||
        !pRenderData->bRefValid)   &&
        pRenderData->bSameSamples  &&
        IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        CopySurfaceValue(pVeboxState->m_currentSurface, pVeboxState->FFDNSurfaces[(pRenderData->iCurDNOut + 1) & 1]);
    }
    else
    {
        //--------------------------------------------------------------------------
        // Swap buffers for next iteration
        //--------------------------------------------------------------------------
        pVeboxState->iCurDNIndex     = (pRenderData->iCurDNOut + 1) & 1;
        pVeboxState->iCurStmmIndex   = (pVeboxState->iCurStmmIndex + 1) & 1;
    }

    // Set the first frame flag
    if (pVeboxState->bFirstFrame)
    {
        pVeboxState->bFirstFrame = false;
    }

finish:
    return eStatus;
}

PVPHAL_SURFACE VPHAL_VEBOX_STATE::GetOutputSurfForDiSameSampleWithSFC(
    PVPHAL_SURFACE pSrcSurface)
{
    PVPHAL_VEBOX_STATE       pVeboxState    = this;
    PVPHAL_VEBOX_RENDER_DATA pRenderData    = pVeboxState->GetLastExecRenderData();
    PVPHAL_SURFACE           pOutputSurface = pSrcSurface;

    // Update rect sizes in FFDI surface if input surface rect size changes
    if (pSrcSurface->rcSrc.left      != pVeboxState->FFDISurfaces[0]->rcSrc.left     ||
        pSrcSurface->rcSrc.right     != pVeboxState->FFDISurfaces[0]->rcSrc.right    ||
        pSrcSurface->rcSrc.top       != pVeboxState->FFDISurfaces[0]->rcSrc.top      ||
        pSrcSurface->rcSrc.bottom    != pVeboxState->FFDISurfaces[0]->rcSrc.bottom   ||
        pSrcSurface->rcDst.left      != pVeboxState->FFDISurfaces[0]->rcDst.left     ||
        pSrcSurface->rcDst.right     != pVeboxState->FFDISurfaces[0]->rcDst.right    ||
        pSrcSurface->rcDst.top       != pVeboxState->FFDISurfaces[0]->rcDst.top      ||
        pSrcSurface->rcDst.bottom    != pVeboxState->FFDISurfaces[0]->rcDst.bottom   ||
        pSrcSurface->rcMaxSrc.left   != pVeboxState->FFDISurfaces[0]->rcMaxSrc.left  ||
        pSrcSurface->rcMaxSrc.right  != pVeboxState->FFDISurfaces[0]->rcMaxSrc.right ||
        pSrcSurface->rcMaxSrc.top    != pVeboxState->FFDISurfaces[0]->rcMaxSrc.top   ||
        pSrcSurface->rcMaxSrc.bottom != pVeboxState->FFDISurfaces[0]->rcMaxSrc.bottom)
    {
        pVeboxState->FFDISurfaces[0]->rcSrc    = pSrcSurface->rcSrc;
        pVeboxState->FFDISurfaces[0]->rcDst    = pSrcSurface->rcDst;
        pVeboxState->FFDISurfaces[0]->rcMaxSrc = pSrcSurface->rcMaxSrc;
    }

    if (pSrcSurface->rcSrc.left      != pVeboxState->FFDISurfaces[1]->rcSrc.left     ||
        pSrcSurface->rcSrc.right     != pVeboxState->FFDISurfaces[1]->rcSrc.right    ||
        pSrcSurface->rcSrc.top       != pVeboxState->FFDISurfaces[1]->rcSrc.top      ||
        pSrcSurface->rcSrc.bottom    != pVeboxState->FFDISurfaces[1]->rcSrc.bottom   ||
        pSrcSurface->rcDst.left      != pVeboxState->FFDISurfaces[1]->rcDst.left     ||
        pSrcSurface->rcDst.right     != pVeboxState->FFDISurfaces[1]->rcDst.right    ||
        pSrcSurface->rcDst.top       != pVeboxState->FFDISurfaces[1]->rcDst.top      ||
        pSrcSurface->rcDst.bottom    != pVeboxState->FFDISurfaces[1]->rcDst.bottom   ||
        pSrcSurface->rcMaxSrc.left   != pVeboxState->FFDISurfaces[1]->rcMaxSrc.left  ||
        pSrcSurface->rcMaxSrc.right  != pVeboxState->FFDISurfaces[1]->rcMaxSrc.right ||
        pSrcSurface->rcMaxSrc.top    != pVeboxState->FFDISurfaces[1]->rcMaxSrc.top   ||
        pSrcSurface->rcMaxSrc.bottom != pVeboxState->FFDISurfaces[1]->rcMaxSrc.bottom)
    {
        pVeboxState->FFDISurfaces[1]->rcSrc    = pSrcSurface->rcSrc;
        pVeboxState->FFDISurfaces[1]->rcDst    = pSrcSurface->rcDst;
        pVeboxState->FFDISurfaces[1]->rcMaxSrc = pSrcSurface->rcMaxSrc;
    }
            
    // Update IEF parameters in FFDI surface
    pVeboxState->FFDISurfaces[0]->pIEFParams   = pSrcSurface->pIEFParams;
    pVeboxState->FFDISurfaces[1]->pIEFParams   = pSrcSurface->pIEFParams;

    // Set BLT1's Current DI Output as BLT2's input, it is always under Mode0
    // BLT1 output 1st field of current frame for the following cases:
    // a) 30fps (bSingleField),
    // c) 60fps 2nd field
    if (pRenderData->bSingleField                                               ||
        (pSrcSurface->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD)     ||
        (pSrcSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD) ||
        (pSrcSurface->SampleType == SAMPLE_SINGLE_BOTTOM_FIELD)                 ||
        (pSrcSurface->SampleType == SAMPLE_PROGRESSIVE))
    {
        pOutputSurface = pVeboxState->FFDISurfaces[1];
    }
    else             
    {
        // First sample output - 2nd field of the previous frame             
        pOutputSurface = pVeboxState->FFDISurfaces[0];
    }

    // Force vebox to bypass data on BLT2
    pRenderData->bDeinterlace   = false;
    pRenderData->bIECP          = false;
    pRenderData->bDenoise       = false;
    pRenderData->bChromaDenoise = false;
#if VEBOX_AUTO_DENOISE_SUPPORTED
    pRenderData->bAutoDenoise   = false;
#endif
    pRenderData->bRefValid      = false;

    return pOutputSurface;
}

//!
//! \brief    Vebox Rendering
//! \details  Vebox rendering, do all the staff for Vebox process,
//!           include implementation of VEBOX/IECP features,
//!           execution of Vebox/Render parallelism,
//!           CPU/GPU (for CP HM) path for Vebox state heap update,
//!           setup input/output/statistics/STMM surface,
//! \param    [in] pcRenderParams
//!           Pointer to Render parameters
//! \param    [in,out] pRenderPassData
//!           Pointer to Render data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::Render(
    PCVPHAL_RENDER_PARAMS  pcRenderParams,
    RenderpassData         *pRenderPassData)
{
    PRENDERHAL_INTERFACE     pRenderHal;
    PMOS_INTERFACE           pOsInterface;
    MOS_STATUS               eStatus;
    PVPHAL_VEBOX_RENDER_DATA pRenderData;
    bool                     bRender;
    PVPHAL_VEBOX_STATE       pVeboxState = this;
    PVPHAL_SURFACE           pSrcSurface;
    PVPHAL_SURFACE           pOutputSurface;

    MOS_UNUSED(pcRenderParams);

    pSrcSurface     = pRenderPassData->pSrcSurface;
    pOutputSurface  = pRenderPassData->pOutSurface;

    VPHAL_RENDER_ASSERT(pVeboxState);
    VPHAL_RENDER_ASSERT(pSrcSurface);
    VPHAL_RENDER_ASSERT(pOutputSurface);

    // Initialize Variables
    pRenderHal              = pVeboxState->m_pRenderHal; 
    pOsInterface            = pVeboxState->m_pOsInterface;
    eStatus                 = MOS_STATUS_SUCCESS;
    bRender                 = false;
    pRenderData             = pVeboxState->GetLastExecRenderData();

    VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_STAGE(VPHAL_DBG_STAGE_VEBOX);

    // Check bSameSamples only when reference is avaliable, DI, Variance Query is enabled
    if (pRenderData->bRefValid     &&
        pRenderData->bSameSamples  && 
        (pRenderData->bDeinterlace || pVeboxState->IsQueryVarianceEnabled()))
    {
        // No frames to generate -> output frames already in buffer
        if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) &&
            pRenderData->bDeinterlace)                // Vebox + SFC
        {
            pSrcSurface = GetOutputSurfForDiSameSampleWithSFC(pSrcSurface);
        }
        else
        {
            // Need not submit Vebox commands, jump out accordingly
            goto dndi_sample_out;
        }
    }

    if (IS_VEBOX_EXECUTION_MODE_0_TO_2(pVeboxState->m_pVeboxExecState))
    {
        // Transition from serial to parallel mode. 2 vebox operations, current 
        // and future frames
        VPHAL_RENDER_CHK_STATUS(VeboxRenderMode0To2(
            pSrcSurface,
            pOutputSurface));
    }
    else if (IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState))
    {
        // Parallel mode. 1 vebox operation, future frame
        VPHAL_RENDER_CHK_STATUS(VeboxRenderMode2(
            pSrcSurface,
            pOutputSurface));
    }
    else if (IS_VEBOX_EXECUTION_MODE_0(pVeboxState->m_pVeboxExecState))
    {
        // Legacy serial mode. 1 vebox operation, current frame
        VPHAL_RENDER_CHK_STATUS(VeboxRenderMode0(
            pSrcSurface,
            pOutputSurface));
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid VEBox state.");
        goto finish;
    }

    VPHAL_DBG_STATE_DUMPPER_DUMP_VEBOX_STATES(pRenderHal, pVeboxState);

    if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        goto sfc_sample_out;
    }

    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        goto vebox_out_in_RT;
    }

dndi_sample_out:
    if (IS_VEBOX_EXECUTION_MODE_2(pVeboxState->m_pVeboxExecState))
    {
        // Select output surface that was rendered in the previous blt
        // It will be used in VpHal_VeboxSetDiOutput()
        // Note: After toggling it, pRenderData->iCurDNOut does not represent
        // the one that is rendered this time
        pRenderData->iCurDNOut = (pRenderData->iCurDNOut + 1) & 1;
    }

    // Select DI sample to be used for compositing stage
    VPHAL_RENDER_CHK_STATUS(VeboxSetDiOutput(pSrcSurface, pOutputSurface));

    VPHAL_RENDER_EXITMESSAGE("Exit with DI output.");
    goto finish;

sfc_sample_out:
    if (pVeboxState->m_pVeboxExecState->bDIOutputPair01)
    {
        pRenderData->iFrame0 = pRenderData->bSameSamples ? 1 : 0;
    }
    else
    {
        pRenderData->iFrame0 = pRenderData->bSameSamples ? 3 : 2;
    }

    // Feature reporting
    m_reporting->IECP    = pRenderData->bIECP;
    m_reporting->Denoise = pRenderData->bDenoise;

    if (pRenderData->bDeinterlace)
    {
        m_reporting->DeinterlaceMode =
            (pRenderData->bSingleField &&
                (!pRenderData->bRefValid  ||
                pSrcSurface->pDeinterlaceParams->DIMode == DI_MODE_BOB)) ?
            VPHAL_DI_REPORT_ADI_BOB                                   :    // VEBOX BOB
            VPHAL_DI_REPORT_ADI;                                           // ADI
    }

    // Select SFC output
    VPHAL_RENDER_EXITMESSAGE("Exit with SFC output.");
    goto finish;

vebox_out_in_RT:
    VPHAL_RENDER_EXITMESSAGE("Exit VEBOX with CSC output.");

finish:
    // In VpHal_VeboxGetStatisticsSurfaceOffsets, the offset of vebox statistics surface 
    // address is based on whether bDN/DIEnabled in the previous function call. We get
    // bDN/DIEnabled here, so that bDN/DIEnabled can be used at the beginning of the next
    // function call. 
    // When Spatial DI enabled and Temporal DI disabled, the vebox statistics surface
    // layout is same as the case when only DN enabled, so use DNEnabled to include the
    // case when Spatial DI enabled. 
    if (pRenderData->bSameSamples  &&
        IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
    {
        // Do nothing when VEBOX+SFC scenario's BLT2 case hits here
    }
    else
    {
        pVeboxState->bDNEnabled = pRenderData->bDenoise         ||
                                  pRenderData->bChromaDenoise   ||
                                  ((pRenderData->bDeinterlace   ||
                                   pVeboxState->IsQueryVarianceEnabled()) && 
                                  !pRenderData->bRefValid);

        pVeboxState->bDIEnabled = (pRenderData->bDeinterlace    || 
                                   pVeboxState->IsQueryVarianceEnabled()) && 
                                   pRenderData->bRefValid;
    }

    pOsInterface->pfnSetGpuContext(pOsInterface, RenderGpuContext);

    // Vebox feature report -- set the output pipe
    m_reporting->OutputPipeMode = pRenderData->OutputPipe;
    m_reporting->VEFeatureInUse = !pRenderData->bVeboxBypass;

    return eStatus;
}

//!
//! \brief    Set DI output frame
//! \details  Choose 2nd Field of Previous frame, 1st Field of Current frame
//!           or both frames
//! \param    [in] pRenderData
//!           Pointer to Render data
//! \param    [in] pVeboxState
//!           Pointer to Vebox State
//! \param    [in] pVeboxMode
//!           Pointer to Vebox Mode
//! \return   GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
//!           Return Previous/Current/Both frames
//!
GFX_MEDIA_VEBOX_DI_OUTPUT_MODE VPHAL_VEBOX_STATE::SetDIOutputFrame(
	PVPHAL_VEBOX_RENDER_DATA pRenderData,
	PVPHAL_VEBOX_STATE       pVeboxState,
	PMHW_VEBOX_MODE          pVeboxMode)
{
	// for 30i->30fps + SFC
	if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) && !pRenderData->b60fpsDi)
	{
		// Set BLT1's Current DI Output as BLT2's input, it is always under Mode0
		// BLT1 output 1st field of current frame for the following cases:
		if (pVeboxMode->DNDIFirstFrame                                                            ||
			(pVeboxState->m_currentSurface->SampleType == SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD) ||
			(pVeboxState->m_currentSurface->SampleType == SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD)   ||
			(pVeboxState->m_currentSurface->SampleType == SAMPLE_SINGLE_TOP_FIELD)                   ||
			(pVeboxState->m_currentSurface->SampleType == SAMPLE_PROGRESSIVE))
		{
			return MEDIA_VEBOX_DI_OUTPUT_CURRENT;
		}
		else
		{
			// First sample output - 2nd field of the previous frame             
			return MEDIA_VEBOX_DI_OUTPUT_PREVIOUS;
		}
	}
	// for 30i->60fps or other 30i->30fps cases
	else
	{
		return pVeboxMode->DNDIFirstFrame ?
			MEDIA_VEBOX_DI_OUTPUT_CURRENT :
			MEDIA_VEBOX_DI_OUTPUT_BOTH;
	}
}

//!
//! \brief    Vebox post-composition activity for parallel engine
//! \details  update Vebox state heap, and FMD extra variances
//! \param    [in] pVeboxExecState
//!           Pointer to Vebox execution state
//! \param    [in] pPriSurface
//!           Pointer to primary surface of compostion, which was processed in last Vebox call
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE::PostCompRender(
    PVPHAL_VEBOX_EXEC_STATE     pVeboxExecState,
    PVPHAL_SURFACE              pPriSurface)
{
    MOS_STATUS              eStatus;
    PMOS_INTERFACE          pOsInterface;
    PVPHAL_VEBOX_STATE      pVeboxState = this;

    VPHAL_RENDER_ASSERT(pVeboxState);

    eStatus             = MOS_STATUS_SUCCESS;
    pOsInterface        = pVeboxState->m_pOsInterface;

    // In Mode0To2 or Mode2, speculatively copy vebox state information for use 
    // in next vebox op based on last vebox op's state info.
    if (IS_VEBOX_SPECULATIVE_COPY_REQUESTED(pVeboxExecState))
    {
        VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxCopyAndUpdateVeboxState(
            pPriSurface));

        RESET_VEBOX_SPECULATIVE_COPY(pVeboxExecState);
    }

finish:
    return eStatus;
}

//!
//! \brief    Check if 2 passes CSC are needed
//! \param    [in] pSrc
//!           Pointer to input surface of Vebox
//! \param    [in] pRenderTarget
//!           Pointer to Render targe surface of VPP BLT
//! \return   bool
//!           return true if 2 Passes CSC is needed, otherwise false
//!
bool VPHAL_VEBOX_STATE::VeboxIs2PassesCSCNeeded(
    PVPHAL_SURFACE              pSrc,
    PVPHAL_SURFACE              pRenderTarget)
{
    bool bRet                   = false;
    bool b2PassesCSCNeeded      = false;
    bool bFormatSupported       = false;
    bool bPlatformSupported     = false;
    PVPHAL_VEBOX_STATE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    MOS_OS_CHK_NULL_NO_STATUS(pVeboxState);
    MOS_OS_CHK_NULL_NO_STATUS(pSrc);
    MOS_OS_CHK_NULL_NO_STATUS(pRenderTarget);
    MOS_OS_CHK_NULL_NO_STATUS(pRenderData);

    // 2 Passes CSC is used in BT2020YUV->BT601/709YUV
    // Isolate decoder require SFC output, but SFC can not support RGB input, 
    // so sRGB need two pass, that same as original logic.
    if (IS_COLOR_SPACE_BT2020_YUV(pSrc->ColorSpace))
    {
        if ((pRenderTarget->ColorSpace == CSpace_BT601)           ||
            (pRenderTarget->ColorSpace == CSpace_BT709)           ||
            (pRenderTarget->ColorSpace == CSpace_BT601_FullRange) ||
            (pRenderTarget->ColorSpace == CSpace_BT709_FullRange) ||
            (pRenderTarget->ColorSpace == CSpace_stRGB)           ||
            (pRenderTarget->ColorSpace == CSpace_sRGB))
        {
            b2PassesCSCNeeded = true;
        }
    }

    // VEBOX support input format
    bFormatSupported       = pVeboxState->IsFormatSupported(pSrc);
    // Platform support 2 passes CSC
    bPlatformSupported     = Is2PassesCscPlatformSupported();

    bRet = bFormatSupported && bPlatformSupported && b2PassesCSCNeeded;

finish:
    return bRet;
}

//!
//! \brief    copy Report data about features
//! \details  copy Report data from this render
//! \param    [out] pReporting    
//!           pointer to the Report data to copy data to
//!
void VPHAL_VEBOX_STATE::CopyFeatureReporting(VphalFeatureReport* pReporting)
{
    pReporting->IECP            = m_reporting->IECP;
    pReporting->Denoise         = m_reporting->Denoise;
    pReporting->DeinterlaceMode = m_reporting->DeinterlaceMode;
    pReporting->OutputPipeMode  = m_reporting->OutputPipeMode;
    pReporting->VPMMCInUse      = bEnableMMC;
    pReporting->VEFeatureInUse  = m_reporting->VEFeatureInUse;
}

//!
//! \brief    copy Report data about resources
//! \details  copy Report data from this render
//! \param    [out] pReporting    
//!           pointer to the Report data to copy data to
//!
void VPHAL_VEBOX_STATE::CopyResourceReporting(VphalFeatureReport* pReporting)
{
    // Report Vebox intermediate surface
    pReporting->FFDICompressible   = m_reporting->FFDICompressible;
    pReporting->FFDICompressMode   = m_reporting->FFDICompressMode;
    pReporting->FFDNCompressible   = m_reporting->FFDNCompressible;
    pReporting->FFDNCompressMode   = m_reporting->FFDNCompressMode;
    pReporting->STMMCompressible   = m_reporting->STMMCompressible;
    pReporting->STMMCompressMode   = m_reporting->STMMCompressMode;
    pReporting->ScalerCompressible = m_reporting->ScalerCompressible;
    pReporting->ScalerCompressMode = m_reporting->ScalerCompressMode;
}

//!
//! \brief    copy Report data
//! \details  copy Report data from this render
//! \param    [out] pReporting    
//!           pointer to the Report data to copy data to
//!
void VPHAL_VEBOX_STATE::CopyReporting(VphalFeatureReport* pReporting)
{
    VPHAL_RENDER_ASSERT(pReporting);

    CopyFeatureReporting(pReporting);
    CopyResourceReporting(pReporting);
}

//!
//! \brief    Perform Rendering in VEBOX
//! \details  Check whether VEBOX Rendering is enabled. When it's enabled, perform VEBOX Rendering
//!           on the input surface and get the output surface
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in] pcRenderParams
//!           Const pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to RenderpassData structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrRenderVebox(
    VphalRenderer           *pRenderer,
    PCVPHAL_RENDER_PARAMS   pcRenderParams,
    RenderpassData          *pRenderPassData)
{
    MOS_STATUS               eStatus;
    PMOS_INTERFACE           pOsInterface;
    RenderState              *pRenderState;
    VphalFeatureReport*      pReport;
    PVPHAL_SURFACE           pOutSurface = nullptr;
    RECT                     rcTemp;

    //------------------------------------------------------
    VPHAL_RENDER_ASSERT(pRenderer);
    VPHAL_RENDER_ASSERT(pcRenderParams);
    VPHAL_RENDER_CHK_NULL(pRenderer->GetOsInterface());
    //------------------------------------------------------

    eStatus                 = MOS_STATUS_SUCCESS;
    pOsInterface            = pRenderer->GetOsInterface();
    pReport                 = pRenderer->GetReport();
    pRenderState            = pRenderer->pRender[VPHAL_RENDER_ID_VEBOX + pRenderer->uiCurrentChannel];
    pOutSurface             = pRenderPassData->GetTempOutputSurface();

    pRenderPassData->bOutputGenerated  = false;

    VPHAL_RENDER_CHK_NULL(pRenderState);
    VPHAL_RENDER_ASSERT(pRenderState->GetRenderHalInterface());

    pRenderPassData->bCompNeeded  = true;

    if (!pRenderState->GetRenderDisableFlag())
    {
        MOS_ZeroMemory(pOutSurface, sizeof(VPHAL_SURFACE));

        pRenderPassData->bCompNeeded = false;


        // Check if DNDI Render can be applied
        if (!pRenderState->IsNeeded(
            pcRenderParams,
            pRenderPassData))
        {
            goto finish;
        }

        if (pRenderPassData->bCompNeeded == false)
        {
            // Render Target is the output surface
            pOutSurface = pcRenderParams->pTarget[0];
        }

        pRenderPassData->pOutSurface    = pOutSurface;

        VPHAL_RENDER_CHK_STATUS(pRenderState->Render(
                                                pcRenderParams,
                                                pRenderPassData))

        pRenderState->CopyReporting(pReport);

        if (pRenderPassData->bCompNeeded)
        {
            pRenderPassData->bOutputGenerated = true;
        }
    }

finish:
    if (pRenderPassData->bOutputGenerated)
    {   
        pRenderPassData->pOutSurface = pRenderPassData->pOutSurface;
    }
    return eStatus;
}



VPHAL_VEBOX_STATE::VPHAL_VEBOX_STATE(
    PMOS_INTERFACE                  pOsInterface,
    PMHW_VEBOX_INTERFACE            pVeboxInterface,
    PMHW_SFC_INTERFACE              pSfcInterface,
    PRENDERHAL_INTERFACE            pRenderHal,
    PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
    PVPHAL_RNDR_PERF_DATA           pPerfData,
    const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
    MOS_STATUS                      *peStatus) :
    m_pVeboxInterface(pVeboxInterface),
    m_pSfcInterface(pSfcInterface),
    m_pVeboxExecState(pVeboxExecState),
    m_currKernelId(baseKernelMaxNumID),
    RenderState(pOsInterface, pRenderHal, pPerfData, peStatus)
{
    // External components
    m_IECP                  = nullptr;
    m_sfcPipeState          = nullptr;
    m_pKernelDllState       = nullptr;
    m_pLastExecRenderData   = nullptr;
    CscOutputCspace         = CSpace_Any;
    CscInputCspace          = CSpace_Any;

    int i;
    for (i = 0; i < 9; i++)
    {
        fCscCoeff[i] = 0.0f;
    }

    for (i = 0; i < 3; i++)
    {
        fCscInOffset[i] = 0.0f;
        fCscOutOffset[i] = 0.0f;
    }

    for (i = 0; i < 2; i++)
    {
        SearchFilter[i] = {};
    }

    // Threshold for discontinuity check
    iSameSampleThreshold = 0;

    // Resources
    m_currentSurface            = nullptr;           //!< Current frame
    m_previousSurface           = nullptr;           //!< Previous frame
    RenderHalCurrentSurface     = {};                //!< Current frame for MHW
    RenderHalPreviousSurface    = {};                //!< Previous frame for MHW

    for (i = 0; i < VPHAL_MAX_NUM_FFDI_SURFACES; i++)
    {
        FFDISurfaces[i] = nullptr;
    }
    VeboxStatisticsSurface          = {};            //!< Statistics Surface for VEBOX
    RenderHalVeboxStatisticsSurface = {};            //!< Statistics Surface for VEBOX for MHW
#if VEBOX_AUTO_DENOISE_SUPPORTED
    VeboxTempSurface                = {};            //!< Temp Surface for Vebox State update kernels
    VeboxSpatialAttributesConfigurationSurface = {}; //!< Spatial Attributes Configuration Surface for DN kernel Gen9+
    RenderHalVeboxSpatialAttributesConfigurationSurface = {}; //!< Spatial Attributes Configuration Surface for DN kernel Gen9+ for MHW
    VeboxHeapResource    = {};                       //!< Vebox Heap resource for DN kernel
    tmpResource          = {};                       //!< Temp resource for DN kernel
    RenderHalVeboxHeapResource = {};                 //!< Vebox Heap resource for DN kernel for MHW
    RenderHalTmpResource       = {};                 //!< Temp resource for DN kernel for MHW
#endif

    // DNDI
    for (i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
    {
        FFDNSurfaces[i] = nullptr;
    }

    for (i = 0; i < VPHAL_NUM_STMM_SURFACES; i++)
    {
        STMMSurfaces[i] = {};
    }

    // BNE system memory pointer
    pBNEData    = nullptr;                          //!< System memory for GNE calculating
    dwBNESize   = 0;                                //!< System memory size for BNE surface

    // Statistics
    dwVeboxPerBlockStatisticsWidth  = 0;            //!< Per block statistics width
    dwVeboxPerBlockStatisticsHeight = 0;            //!< Per block statistics height

    //!< Surface memory object control
    // Get cache settings
    DnDiSurfMemObjCtl = dndiCacheCntl;

    // Batch Buffers
    iBatchBufferCount = 0;                          //!< Number of batch buffers
    for (i = 0; i < VPHAL_DNDI_BUFFERS_MAX; i++)
    {
        BatchBuffer[i] = {};
        BufferParam[i] = {};
    }

    // Denoise output control
    iCurDNIndex = 0;                                //!< Current index of Denoise Output

    // DNDI
    iNumFFDISurfaces    = 0;                        //!< Actual number of FFDISurfaces. Is <= VPHAL_NUM_FFDI_SURFACES
    iCurStmmIndex       = 0;                        //!< Current index of Motion History Buffer
    dwGlobalNoiseLevel  = 0;                        //!< Global Noise Level

    // Chroma DN
    iCurHistIndex       = 0;                        //!< Current index of Chroma Denoise History Buffer
    dwGlobalNoiseLevelU = 0;                        //!< Global Noise Level for U
    dwGlobalNoiseLevelV = 0;                        //!< Global Noise Level for V
    bFirstFrame         = false;                    //!< First frame case for Chroma DN


    // timestamps for DI output control
    iCurFrameID = 0;                                //!< Current Frame ID
    iPrvFrameID = 0;                                //!< Previous Frame ID

    // for Pre-Processing
    bSameSamples    = false;                        //!< True for second DI
    iCallID         = 0;                            //!< Current render call ID;
    bDNEnabled      = false;                        //!< DN was enabled in the previous call
    bDIEnabled      = false;                        //!< DI was enabled in the previous call

    // Platform dependent states
    pKernelParamTable   = nullptr;                  //!< Kernel Parameter table

    // HW Params
    dwKernelUpdate      = 0;                        //!< Enable/Disable kernel update

    dwCompBypassMode    = 0;                        //!< Bypass Composition Optimization read from User feature keys

    // Debug parameters
    pKernelName                          = nullptr; //!< Kernel Used for current rendering
    bNullHwRenderDnDi                    = false;   //!< Null rendering for DnDi function 

    bEnableMMC                           = false;   //!< Memory compression enbale flag - read from User feature keys
    bDisableTemporalDenoiseFilter        = false;   //!< Temporal denoise filter disable flag - read from User feature keys
    bDisableTemporalDenoiseFilterUserKey = false;   //!< Backup temporal denoise filter disable flag - read from User feature keys

    RenderGpuContext = pOsInterface ? (pOsInterface->CurrentGpuContextOrdinal) : MOS_GPU_CONTEXT_RENDER;
}

VPHAL_VEBOX_STATE::~VPHAL_VEBOX_STATE()
{
    PRENDERHAL_INTERFACE pRenderHal;
    PMHW_BATCH_BUFFER    pBuffer;
    int32_t              i;
    PVPHAL_VEBOX_STATE                  pVeboxState = this;

    VPHAL_RENDER_ASSERT(pVeboxState);

    pRenderHal   = pVeboxState->m_pRenderHal;

    VPHAL_RENDER_ASSERT(pRenderHal);

    MOS_FreeMemAndSetNull(m_currentSurface);
    MOS_FreeMemAndSetNull(m_previousSurface);

    for (uint32_t i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
    {
        MOS_FreeMemAndSetNull(FFDNSurfaces[i]);
    }

    for (uint32_t i = 0; i < VPHAL_MAX_NUM_FFDI_SURFACES; i++)
    {
        MOS_FreeMemAndSetNull(FFDISurfaces[i]);
    }

    // Destroy Batch Buffers
    for (i = 0; i < pVeboxState->iBatchBufferCount; i++)
    {
        pBuffer = &pVeboxState->BatchBuffer[i];
        pRenderHal->pfnFreeBB(pRenderHal, pBuffer);
    }

    if (m_pLastExecRenderData)
    {
        MOS_Delete(m_pLastExecRenderData);
        m_pLastExecRenderData = nullptr;
    }

    if (m_IECP)
    {
        MOS_Delete(m_IECP);
        m_IECP = nullptr;
    }

    // Destroy SFC state
    if (MEDIA_IS_SKU(m_pSkuTable, FtrSFCPipe) && m_sfcPipeState)
    {
        MOS_Delete(m_sfcPipeState);
        m_sfcPipeState = nullptr;
    }
}

VPHAL_VEBOX_RENDER_DATA::~VPHAL_VEBOX_RENDER_DATA()
{
    if (m_pVeboxStateParams)
    {
        MOS_Delete(m_pVeboxStateParams);
        m_pVeboxStateParams = nullptr;
    }

    if (m_pVeboxIecpParams)
    {
        MOS_Delete(m_pVeboxIecpParams);
        m_pVeboxIecpParams = nullptr;
    }
}

MOS_STATUS VPHAL_VEBOX_RENDER_DATA::Init()
{   
    // Vebox State Parameters
    // m_pVeboxStateParams needs to be set to nullptr in constructor

    if (!m_pVeboxStateParams)
    {
        m_pVeboxStateParams = MOS_New(VPHAL_VEBOX_STATE_PARAMS);
        if (!m_pVeboxStateParams)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }
    else
    {
        m_pVeboxStateParams->Init();
    }

    // Vebox IECP State Parameters
    // m_pVeboxIecpParams needs to be set to nullptr in constructor
    if (!m_pVeboxIecpParams)
    {
        m_pVeboxIecpParams = MOS_New(VPHAL_VEBOX_IECP_PARAMS);
        if (!m_pVeboxIecpParams)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }
    else
    {
        m_pVeboxIecpParams->Init();
    }

    bColorPipe      = false;
    bIECP           = false;
    bProcamp        = false;

    // Flags
    bRefValid       = false;
    bSameSamples    = false;
    bProgressive    = false;
    bDenoise        = false;
#if VEBOX_AUTO_DENOISE_SUPPORTED
    bAutoDenoise    = false;
#endif
    bChromaDenoise  = false;
    bOutOfBound     = false;
    bVDIWalker      = false;

    // DNDI/Vebox
    bDeinterlace    = false;
    bSingleField    = false;
    bTFF            = false;
    bTopField       = false;
    bBeCsc          = false;
    bVeboxBypass    = false;
	b60fpsDi        = false;

    // Surface Information
    iFrame0     = 0;
    iFrame1     = 0;
    iCurDNIn    = 0;
    iCurDNOut   = 0;
    iCurHistIn  = 0;
    iCurHistOut = 0;

    // Geometry
    iBlocksX        = 0;
    iBlocksY        = 0;
    iBindingTable   = 0;
    iMediaID0       = 0;
    iMediaID1       = 0;

    // Perf
    PerfTag = VPHAL_NONE;

    // States
    pMediaState     = nullptr;
    pVeboxState     = nullptr;
    pRenderTarget   = nullptr;

    SamplerStateParams  = { };

    VeboxDNDIParams     = { };

    pAlphaParams    = nullptr;

    // Batch Buffer rendering arguments
    BbArgs = { };

    // Vebox output parameters
    OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;

    // Kernel Information
    int i;
    for (i = 0; i < VPHAL_NUM_KERNEL_VEBOX; i++)
    {
        pKernelParam[i] = nullptr;
        KernelEntry[i]  = { };
    }

    pDNUVParams     = nullptr;
    iCurbeLength    = 0;
    iInlineLength   = 0;

    // Debug parameters
    pKernelName = nullptr;
    Component   = COMPONENT_UNKNOWN;

    // Memory compression flag
    bEnableMMC = false;

    fScaleX = 0.0f;                                //!< X Scaling ratio
    fScaleY = 0.0f;                                //!< Y Scaling ratio

    return MOS_STATUS_SUCCESS;
}
