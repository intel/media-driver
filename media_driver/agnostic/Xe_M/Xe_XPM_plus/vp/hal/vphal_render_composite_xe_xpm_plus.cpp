/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021-2023, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     vphal_render_composite_xe_xpm_plus.cpp
//! \brief    Composite related VPHAL functions
//! \details  Unified VP HAL Composite module including render initialization,
//!           resource allocation/free and rendering
//!
#include "vphal_render_composite_xe_xpm_plus.h"
#include "vp_hal_ddi_utils.h"

extern const Kdll_Layer g_cSurfaceType_Layer[];
extern const MEDIA_WALKER_KA2_STATIC_DATA g_cInit_MEDIA_WALKER_KA2_STATIC_DATA;

CompositeStateXe_Xpm_Plus::CompositeStateXe_Xpm_Plus (
    PMOS_INTERFACE                      pOsInterface,
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_RNDR_PERF_DATA               pPerfData,
    const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
    MOS_STATUS                          *peStatus) :
    CompositeState(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus),
    CompositeStateG12(pOsInterface, pRenderHal, pPerfData, compositeCacheCntl, peStatus)
{
    if (pRenderHal == nullptr)
    {
        *peStatus = MOS_STATUS_NULL_POINTER;
         return;
    }
    m_bFtrComputeWalker = true;
    m_need3DSampler = true;
    m_bFtrCSCCoeffPatchMode = false;
}

void CompositeStateXe_Xpm_Plus::SetFilterScalingRatio(
     Kdll_Scalingratio*     ScalingRatio)
{
    VPHAL_RENDER_ASSERT(ScalingRatio);

    if((m_fScaleX > (1.0f+ (1.0f/6.0f))) && (m_fScaleY > (1.0f + (1.0f / 6.0f))))
    {
        *ScalingRatio = Scalingratio_over1;
    }
    else if((m_fScaleX > 0.5f) && (m_fScaleY > 0.5f))
    {
        *ScalingRatio = Scalingratio_b1p2to1;
    }
    else if((m_fScaleX > 0.25f) && (m_fScaleY > 0.25f))
    {
        *ScalingRatio = Scalingratio_b1p4to1p2;
    }
    // disable 1/8 sclaing ratio due to kernel complex, and use Any Scaling ratio replace.
    /*
    else if ((m_fScaleX > 0.125f) && (m_fScaleY > 0.125f))
    {
        *ScalingRatio = Scalingratio_b1p8to1p4;
    }*/
    else
    {
        *ScalingRatio = Scalingratio_Any;
    }
}

void CompositeStateXe_Xpm_Plus::CaculateBlockSize(
    uint32_t*       uiBlockSize)
{
    VPHAL_RENDER_ASSERT(uiBlockSize);

    if ((m_fScaleX > (1.0f + (1.0f / 6.0f))) && (m_fScaleY > (1.0f + (1.0f / 6.0f))))
    {
        *uiBlockSize = 16;
    }
    else if((m_fScaleX > 0.5f) && (m_fScaleY > 0.5f))
    {
        *uiBlockSize = 8;
    }
    else
    {
        *uiBlockSize = 4;
    }
}

MOS_STATUS CompositeStateXe_Xpm_Plus::RenderInit(
    PVPHAL_COMPOSITE_PARAMS         pCompParams,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData)
{
    PRENDERHAL_INTERFACE    pRenderHal;
    RECT                    AlignedRect;
    uint32_t                uiMediaWalkerBlockSize;
    PRECT                   pDst;
    PVPHAL_SURFACE          pSource;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pCompParams);
    VPHAL_RENDER_CHK_NULL(pRenderingData);

    pRenderHal   = m_pRenderHal;

    //============================
    // Set rendering data
    //============================
    MOS_ZeroMemory(pRenderingData, sizeof(VPHAL_RENDERING_DATA_COMPOSITE));

    pSource = pCompParams->pSource[0];
    VPHAL_RENDER_CHK_NULL(pSource);

    // Set output area
    if (pCompParams->uTargetCount == 2)
    {
        // Output rectangle based on non-rotated target in case of dual output
        pRenderingData->BbArgs.rcOutput = pCompParams->Target[1].rcDst;
        pRenderingData->pTarget[1]      = &pCompParams->Target[1];
    }
    else
    {
        pRenderingData->BbArgs.rcOutput = pCompParams->Target[0].rcDst;
    }

    pDst = &(pRenderingData->BbArgs.rcOutput);

    // Set sources
    pRenderingData->iLayers                = 0;
    pRenderingData->pTarget[0]             = &pCompParams->Target[0];
    pRenderingData->pColorFill             = pCompParams->pColorFillParams;
    pRenderingData->pCompAlpha             = pCompParams->pCompAlpha;

    // Set constriction parameters
    pRenderingData->pConstriction = pCompParams->pConstriction;
    if (pCompParams->pConstriction)
    {
        pRenderingData->ConstrictionOriginX = pDst->left;
        pRenderingData->ConstrictionOriginY = pDst->top;
        pRenderingData->fConstrictionStepX  = (pDst->right - pDst->left) * 1.0f /
                                               pCompParams->pConstriction->right;
        pRenderingData->fConstrictionStepY  = (pDst->bottom - pDst->top) * 1.0f /
                                               pCompParams->pConstriction->bottom;
    }
    else
    {
        pRenderingData->ConstrictionOriginX = 0;
        pRenderingData->ConstrictionOriginY = 0;
        pRenderingData->fConstrictionStepX  = 1.0f;
        pRenderingData->fConstrictionStepY  = 1.0f;
    }

        // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (pSource->Rotation == VPHAL_ROTATION_IDENTITY    ||
        pSource->Rotation == VPHAL_ROTATION_180         ||
        pSource->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
        pSource->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        m_fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                       (float)(pSource->rcSrc.right  - pSource->rcSrc.left);
        m_fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                       (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        m_fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                       (float)(pSource->rcSrc.bottom  - pSource->rcSrc.top);
        m_fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                       (float)(pSource->rcSrc.right - pSource->rcSrc.left);
    }

    if (pRenderingData->pConstriction)
    {
        m_fScaleX /= pRenderingData->fConstrictionStepX;
        m_fScaleX /= pRenderingData->fConstrictionStepY;
    }

    uiMediaWalkerBlockSize = 16;
    CaculateBlockSize(&uiMediaWalkerBlockSize);
    // Calculate aligned output area in order to determine the total # blocks to process
    // in case of non-16x16 aligned target
    AlignedRect = *pDst;
    AlignedRect.right += uiMediaWalkerBlockSize - 1;
    AlignedRect.bottom += uiMediaWalkerBlockSize - 1;
    AlignedRect.left -= AlignedRect.left % uiMediaWalkerBlockSize;
    AlignedRect.top -= AlignedRect.top % uiMediaWalkerBlockSize;
    AlignedRect.right -= AlignedRect.right % uiMediaWalkerBlockSize;
    AlignedRect.bottom -= AlignedRect.bottom % uiMediaWalkerBlockSize;

    // Set number of blocks
    pRenderingData->iBlocksX =
     (AlignedRect.right - AlignedRect.left) / uiMediaWalkerBlockSize;
    pRenderingData->iBlocksY =
     (AlignedRect.bottom - AlignedRect.top) / uiMediaWalkerBlockSize;

    // Set AVS and 8x8 table from renderer
    pRenderingData->pAvsParams             = &m_AvsParameters;

    // Init extension data to nullptr
    pRenderingData->pExtensionData         = nullptr;

    // Initialize rendering states
    pRenderingData->Static                 = {0};
    pRenderingData->Inline                 = {0};
    pRenderingData->WalkerStatic           = {0};
    pRenderingData->DPFCStatic             = {0};
    // By default, alpha is calculated in PartBlend kernel
    pRenderingData->bAlphaCalculateEnable = false;

    // Reset Sampler Params
    MOS_ZeroMemory(
        pRenderingData->SamplerStateParams,
        sizeof(pRenderingData->SamplerStateParams));

finish:
    return eStatus;
}

bool CompositeStateXe_Xpm_Plus::RenderBufferComputeWalker(
    PMHW_BATCH_BUFFER               pBatchBuffer,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    PMHW_GPGPU_WALKER_PARAMS        pWalkerParams)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    MEDIA_DP_FC_STATIC_DATA             *pDPWalkerStatic;
    PVPHAL_BB_COMP_ARGS                 pBbArgs;
    bool                                bResult;
    int32_t                             iLayers;
    uint32_t                            uiMediaWalkerBlockSize;
    uint32_t*                           pdwDestXYTopLeft;
    uint32_t*                           pdwDestXYBottomRight;
    RECT                                AlignedRect;

    MOS_UNUSED(pBatchBuffer);

    bResult          = false;
    pRenderHal       = m_pRenderHal;
    pBbArgs          = &pRenderingData->BbArgs;
    pDPWalkerStatic  = &pRenderingData->DPFCStatic;

    VPHAL_RENDER_ASSERT(m_bFtrMediaWalker && !pBatchBuffer);

    pdwDestXYTopLeft     = (uint32_t*)(&pDPWalkerStatic->DW13);
    pdwDestXYBottomRight = (uint32_t*)(&pDPWalkerStatic->DW14);

    // GRF7.0-7, GRF8.0-7
    for (iLayers = 0;
         iLayers < pBbArgs->iLayers;
         iLayers++, pdwDestXYBottomRight++, pdwDestXYTopLeft++)
    {
        *pdwDestXYTopLeft     = (pBbArgs->rcDst[iLayers].top    << 16 ) |
                                 pBbArgs->rcDst[iLayers].left;
        *pdwDestXYBottomRight = ((pBbArgs->rcDst[iLayers].bottom - 1) << 16 ) |
                                 (pBbArgs->rcDst[iLayers].right - 1);

        VPHAL_RENDER_NORMALMESSAGE("Scaling Info: layer %d, DestXTopLeft %d, DestYTopLeft %d, DestXBottomRight %d, DestYBottomRight %d",
            iLayers, pBbArgs->rcDst[iLayers].left, pBbArgs->rcDst[iLayers].top, pBbArgs->rcDst[iLayers].right - 1, pBbArgs->rcDst[iLayers].bottom - 1);
    }

    if (pRenderingData->pTarget[1] == nullptr)
    {
        AlignedRect   = pRenderingData->pTarget[0]->rcDst;
    }
    else
    {
        AlignedRect   = pRenderingData->pTarget[1]->rcDst;
    }

    // Get media walker kernel block size
    uiMediaWalkerBlockSize = pRenderHal->pHwSizes->dwSizeMediaWalkerBlock;
    CaculateBlockSize(&uiMediaWalkerBlockSize);
    // Calculate aligned output area in order to determine the total # blocks
    // to process in case of non-16x16 aligned target.
    AlignedRect.right  += uiMediaWalkerBlockSize  - 1;
    AlignedRect.bottom += uiMediaWalkerBlockSize - 1;
    AlignedRect.left   -= AlignedRect.left   % uiMediaWalkerBlockSize;
    AlignedRect.top    -= AlignedRect.top    % uiMediaWalkerBlockSize;
    AlignedRect.right  -= AlignedRect.right  % uiMediaWalkerBlockSize;
    AlignedRect.bottom -= AlignedRect.bottom % uiMediaWalkerBlockSize;

    // Set walker cmd params - Rasterscan
    pWalkerParams->InterfaceDescriptorOffset    = pRenderingData->iMediaID;

    pWalkerParams->GroupStartingX = (AlignedRect.left / uiMediaWalkerBlockSize);
    pWalkerParams->GroupStartingY = (AlignedRect.top / uiMediaWalkerBlockSize);
    pWalkerParams->GroupWidth     = pRenderingData->iBlocksX;
    pWalkerParams->GroupHeight    = pRenderingData->iBlocksY;

    pWalkerParams->ThreadWidth  = VPHAL_COMP_COMPUTE_WALKER_THREAD_SPACE_WIDTH;
    pWalkerParams->ThreadHeight = VPHAL_COMP_COMPUTE_WALKER_THREAD_SPACE_HEIGHT;
    pWalkerParams->ThreadDepth  = VPHAL_COMP_COMPUTE_WALKER_THREAD_SPACE_DEPTH;
    pWalkerParams->IndirectDataStartAddress = pRenderingData->iCurbeOffset;
    // Indirect Data Length is a multiple of 64 bytes (size of L3 cacheline). Bits [5:0] are zero.
    pWalkerParams->IndirectDataLength       = MOS_ALIGN_CEIL(pRenderingData->iCurbeLength, 1 << MHW_COMPUTE_INDIRECT_SHIFT);
    pWalkerParams->BindingTableID = pRenderingData->iBindingTable;

    bResult = true;

    return bResult;
}

bool CompositeStateXe_Xpm_Plus::SubmitStates(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData)
{
    // States and objects
    PRENDERHAL_INTERFACE                pRenderHal;
    Kdll_State                          *pKernelDllState;   // Kernel DLL state
    Kdll_CacheEntry                     *pKernelEntry;      // Media kernel entry
    float                               pfCscMatrix[12];    // CSC matrix in floating point format
    int32_t                             piCscMatrix[12];    // CSC matrix in fixed point format

    PRENDERHAL_MEDIA_STATE              pMediaState;    // Media states
    MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic;       // Static parameters
    MEDIA_DP_FC_STATIC_DATA             *pDPStatic;     // DP parameters
    PVPHAL_SURFACE                      pSurface;       // Surface parameters
    PVPHAL_SURFACE                      pTarget;        // Render Target parameters

    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParams;

    // Media kernel parameters
    int32_t                             iFilterSize, i, j;
    int32_t                             iThreadCount;
    Kdll_FilterEntry                    *pFilter;
    Kdll_CSC_Params                     *pCscParams;
    Kdll_CSC_Matrix                     *pMatrix;
    Kdll_Procamp                        *pProcamp;

    int32_t                             iKrnAllocation;
    int32_t                             iCurbeOffset;
    int32_t                             iCurbeLength;
    int32_t                             iInlineLength;
    MHW_KERNEL_PARAM                    MhwKernelParam;

    // CSC parameters for ColorFill and Palettes
    VPHAL_CSPACE                        src_cspace, dst_cspace;
    uint8_t                             ColorFill_A;
    float                               fStepX;
    bool                                bResult = false;
    MOS_STATUS                          eStatus;
    int32_t                             iNumEntries;
    void*                               pPaletteData = nullptr;

    VPHAL_RENDER_ASSERT(m_pKernelDllState);
    VPHAL_RENDER_CHK_NULL(m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pRenderingData);
    VPHAL_RENDER_CHK_NULL(pRenderingData->pKernelEntry);

    ColorFill_A     = 0;
    pKernelDllState = m_pKernelDllState;
    pRenderHal      = m_pRenderHal;
    pKernelEntry    = pRenderingData->pKernelEntry;

    // Get Pointer to rendering data
    pStatic = (MEDIA_OBJECT_KA2_STATIC_DATA*)& pRenderingData->WalkerStatic;

    pDPStatic = &pRenderingData->DPFCStatic;

    // Get Pointer to Render Target Surface
    pTarget        = pRenderingData->pTarget[0];

    // Get Kernel Filter description
    pFilter        = pKernelEntry->pFilter;
    iFilterSize    = pKernelEntry->iFilterSize;

    // Get Kernel CSC information
    pCscParams     = pKernelEntry->pCscParams;

    pMatrix        = nullptr;
    for (i = 0; i < DL_CSC_MAX; i++)
    {
        if (pCscParams->Matrix[i].iCoeffID == CoeffID_0)
        {
            pMatrix = &pCscParams->Matrix[i];
            break;
        }
    }

    // Load CSC matrix
    if (pMatrix && pMatrix->bInUse && !m_bFtrCSCCoeffPatchMode)
    {
        // Procamp is present
        if (pMatrix->iProcampID != DL_PROCAMP_DISABLED &&
            pMatrix->iProcampID < VPHAL_MAX_PROCAMP)
        {
            // Get Procamp parameter - update matrix only if Procamp is changed
            pProcamp = &pRenderingData->pProcamp[pMatrix->iProcampID];
            if (pMatrix->iProcampVersion != pProcamp->iProcampVersion)
            {
                KernelDll_UpdateCscCoefficients(pKernelDllState, pMatrix);
            }
        }

        // CSC coeff from static parameter only applies to primary layer
        if (pMatrix->iCoeffID == CoeffID_0)
        {
            int16_t* pCoeff = pMatrix->Coeff;

            pDPStatic->DW0.CscConstantC0  = *(pCoeff++);
            pDPStatic->DW0.CscConstantC1  = *(pCoeff++);

            pDPStatic->DW1.CscConstantC2  = *(pCoeff++);
            pDPStatic->DW1.CscConstantC3  = *(pCoeff++);
            pDPStatic->DW2.CscConstantC4  = *(pCoeff++);
            pDPStatic->DW2.CscConstantC5  = *(pCoeff++);
            pDPStatic->DW3.CscConstantC6  = *(pCoeff++);
            pDPStatic->DW3.CscConstantC7  = *(pCoeff++);
            pDPStatic->DW4.CscConstantC8  = *(pCoeff++);
            pDPStatic->DW4.CscConstantC9  = *(pCoeff++);
            pDPStatic->DW5.CscConstantC10 = *(pCoeff++);
            pDPStatic->DW5.CscConstantC11 = *pCoeff;
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("CSC matrix coefficient id is non-zero.");
            goto finish;
        }
    }

    iInlineLength = CalculateInlineDataSize(pRenderingData, pStatic);

    // Set Background color (use cspace of first layer)
    if (pRenderingData->pColorFill)
    {
        VPHAL_COLOR_SAMPLE_8 Src;

        Src.dwValue = pRenderingData->pColorFill->Color;

        // get src and dst colorspaces
        src_cspace = pRenderingData->pColorFill->CSpace;

        // if iscale enabled, set colorspace to render target color space
        if ( pFilter->sampler == Sample_iScaling || pFilter->sampler == Sample_iScaling_034x || pFilter->sampler == Sample_iScaling_AVS )
        {
            dst_cspace = CSpace_None;
            // find the filter of render target and set dst_cspace to render target color space
            for (i = 0; i < iFilterSize; i++)
            {
                if ((pFilter + i)->layer == Layer_RenderTarget)
                {
                    dst_cspace = (pFilter + i)->cspace;
                }
            }

            if (dst_cspace == CSpace_None) // if color space is invalid return false
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to assign dst color spcae for iScale case.");
                goto finish;
            }
        }
        else // use selected cspace by kdll
        {
            if (GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform))
            {
                dst_cspace = pKernelDllState->colorfill_cspace;
            }
            else
            {
                dst_cspace = pFilter->cspace;
            }
        }

        // Convert BG color only if not done so before. CSC is expensive!
        if ((m_csSrc.dwValue != Src.dwValue) ||
            (m_CSpaceSrc     != src_cspace)  ||
            (m_CSpaceDst     != dst_cspace))
        {
            VpUtils::GetCscMatrixForRender8Bit(&m_csDst, &Src, src_cspace, dst_cspace);

            // store the values for next iteration
            m_csSrc     = Src;
            m_CSpaceSrc = src_cspace;
            m_CSpaceDst = dst_cspace;
        }

        // Set BG color
        if (KernelDll_IsCspace(dst_cspace, CSpace_RGB))
        {
            ColorFill_A = m_csDst.A;
            pStatic->DW13.ColorFill_R = m_csDst.R;
            pStatic->DW13.ColorFill_G = m_csDst.G;
            pStatic->DW13.ColorFill_B = m_csDst.B;
        }
        else
        {
            ColorFill_A = m_csDst.a;
            pStatic->DW13.ColorFill_Y = m_csDst.Y;
            pStatic->DW13.ColorFill_U = m_csDst.U;
            pStatic->DW13.ColorFill_V = m_csDst.V;
        }
    }

    // Load Palettes (layer cspace determines the output cspace)
    // REMARK - Last filter entry is for Render Target
    pSurface    = nullptr;     // initialize it as it may not be set such as for colorfill only case
    for (i = 0; i < iFilterSize - 1; i++, pFilter++)
    {
        // Get current layer ID
        pSurface = pRenderingData->pLayers[i];
        if (nullptr == pSurface)
        {
            continue;
        }
        // Check for palette
        if (pSurface->Palette.iNumEntries <= 0)
        {
            continue;
        }

        // Get palette CSC mode based on filter description
        src_cspace = pSurface->Palette.ColorSpace;
        dst_cspace = pFilter->cspace;

        MOS_ZeroMemory(pfCscMatrix, sizeof(pfCscMatrix));
        KernelDll_GetCSCMatrix(src_cspace, dst_cspace, pfCscMatrix);
        // convert float to fixed point format
        for (j = 0; j < 12; j++)
        {
            // multiply by 2^20 and round up
            piCscMatrix[j] = (int32_t)((pfCscMatrix[j] * 1048576.0f) + 0.5f);
        }

        eStatus = pRenderHal->pfnGetPaletteEntry(pRenderHal,
                                                 pSurface->iPalette,
                                                 pSurface->Palette.iNumEntries,
                                                 &iNumEntries,
                                                 &pPaletteData);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to Get Palette Entry.");
            goto finish;
        }

        eStatus = LoadPaletteData(&pSurface->Palette,
                                   src_cspace,
                                   dst_cspace,
                                   piCscMatrix,
                                   iNumEntries,
                                   pPaletteData);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to Load Palette.");
            eStatus = pRenderHal->pfnFreePaletteID(
                            pRenderHal,
                            &pSurface->iPalette);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to Free Palette ID.");
            }
            goto finish;
        }
    }

    // Set primary video scaling factor
    fStepX = pRenderingData->Inline.DW04.VideoXScalingStep;
    if (fStepX <= 0.0f)
    {
        fStepX = pRenderingData->Inline.DW04.VideoXScalingStep = 1.0f;
    }

    // Set 1st layer step X to the Batch Buffer selection logic
    pRenderingData->BbArgs.fStepX = fStepX;

    // Normalize scaling factors for all layers
    // Ratio of Horizontal Scaling Step to Video X Scaling Step
    // Since NLAS is ZBBed, CM FC kernels simplified scaling factor calculation, no need to normalize here
    if (!pRenderingData->bCmFcEnable)
    {
        pDPStatic->DW9.HorizontalScalingStepRatioLayer0 /= fStepX;
    }

    pMediaState = pRenderingData->pMediaState;

    // Load media kernel for compositing
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, pKernelEntry);
    iKrnAllocation = pRenderHal->pfnLoadKernel(
                                pRenderHal,
                                &m_KernelParams,
                                &MhwKernelParam,
                                pKernelEntry);

    // Check if kernel is successfully loaded in GSH
    if (iKrnAllocation < 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to load kernel in GSH.");
        goto finish;
    }

    if (pSurface == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("pSurface is nullptr.");
        goto finish;
    }

    if (pSurface->Format == Format_A8R8G8B8)
    {
       pDPStatic->DW15.waFlag = 1;
    }

    iCurbeLength = sizeof(MEDIA_DP_FC_STATIC_DATA);
    iCurbeOffset = pRenderHal->pfnLoadCurbeData(
        pRenderHal,
        pMediaState,
        pDPStatic,
        iCurbeLength);
    if (iCurbeOffset < 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to setup CURBE data.");
        goto finish;
    }

    // Allocate Media ID, link to kernel
    pRenderingData->iMediaID = pRenderHal->pfnAllocateMediaID(
        pRenderHal,
        iKrnAllocation,
        pRenderingData->iBindingTable,
        iCurbeOffset,
        iCurbeLength,
        0,
        nullptr);
    if (pRenderingData->iMediaID < 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to setup Media Interface Descriptor.");
        goto finish;
    }

    pRenderingData->iCurbeOffset = iCurbeOffset;
    pRenderingData->iCurbeLength = iCurbeLength;

    // Set Sampler states for this Media ID
    eStatus = pRenderHal->pfnSetSamplerStates(
        pRenderHal,
        pRenderingData->iMediaID,
        pRenderingData->SamplerStateParams,
        MHW_RENDER_ENGINE_SAMPLERS_MAX);

    if (MOS_FAILED(eStatus))
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to setup sampler states.");
        goto finish;
    }

    iThreadCount = GetThreadCountForVfeState(pRenderingData, pTarget);

    //----------------------------------
    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    //----------------------------------
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        iThreadCount,
        iCurbeLength,
        iInlineLength,
        nullptr));

    bResult = true;

finish:
    return bResult;
}