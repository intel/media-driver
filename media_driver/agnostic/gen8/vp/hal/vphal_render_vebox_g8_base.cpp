/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file 
//! \brief 
//!
//!
//! \file     vphal_render_vebox_g8_base.cpp
//! \brief    Interface and structure specific for BDW (GEN8) Vebox
//! \details  Interface and structure specific for BDW (GEN8) Vebox
//!
#include "vphal.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_g8_base.h"
#include "vphal_render_vebox_util_base.h"
#include "vpkrnheader.h"

const char g_KernelDNDI_Str_g8[KERNEL_VEBOX_BASE_MAX][MAX_PATH] =
{
    DBG_TEXT("Reserved"),
    DBG_TEXT("UpdateDNState"),
};

//!
//! \brief Kernel Params
//!
const RENDERHAL_KERNEL_PARAM g_Vebox_KernelParam_g8[KERNEL_VEBOX_BASE_MAX] =
{
///* GRF_Count
//    |  BT_Count
//    |  |    Sampler_Count
//    |  |    |  Thread_Count
//    |  |    |  |                             GRF_Start_Register
//    |  |    |  |                             |   CURBE_Length
//    |  |    |  |                             |   |   block_width
//    |  |    |  |                             |   |   |    block_height
//    |  |    |  |                             |   |   |    |   blocks_x
//    |  |    |  |                             |   |   |    |   |   blocks_y
//    |  |    |  |                             |   |   |    |   |   |*/
    { 0, 0,   0, VPHAL_USE_MEDIA_THREADS_MAX,  0,  0,  0,   0,  0,  0 },    // Reserved
    { 4, 34,  0, VPHAL_USE_MEDIA_THREADS_MAX,  0,  1,  64,  8,  1,  1 },    // UPDATEDNSTATE
};

const uint32_t   dwDenoiseASDThreshold[NOISEFACTOR_MAX + 1] = {
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
    38,  38,  38,  38,  38,  38,  38,  38,  39,  39,  39,  39,  39,  39,  39,  39,
    40 };

const uint32_t   dwDenoiseHistoryDelta[NOISEFACTOR_MAX + 1] = {
    8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
    8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
    8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    8 };

const uint32_t   dwDenoiseMaximumHistory[NOISEFACTOR_MAX + 1] = {
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208 };

const uint32_t   dwDenoiseSTADThreshold[NOISEFACTOR_MAX + 1] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    140, 140, 140, 140, 141, 141, 141, 141, 142, 142, 142, 142, 143, 143, 143, 143,
    144 };

const uint32_t   dwDenoiseSCMThreshold[NOISEFACTOR_MAX + 1] = {
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
    32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,
    38,  38,  38,  38,  38,  38,  38,  38,  39,  39,  39,  39,  39,  39,  39,  39,
    40 };

const uint32_t   dwDenoiseMPThreshold[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    2 };

const uint32_t   dwLTDThreshold[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    8 };

const uint32_t   dwTDThreshold[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
    14 };

const uint32_t   dwGoodNeighborThreshold[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   1,   1,   1,   1,   2,   2,   2,   2,   3,   3,   3,   3,
    4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,
    8,   8,   8,   8,   9,   9,   9,   9,  10,  10,  10,  10,  11,  11,  11,  11,
    12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,  15,  15,  15,
    16 };

const uint32_t   dwHistoryDeltaUV[NOISEFACTOR_MAX + 1] = {
    8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
    8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
    8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    8 };

const uint32_t   dwHistoryMaxUV[NOISEFACTOR_MAX + 1] = {
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208 };

const uint32_t   dwLTDThresholdUV[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
    8 };

const uint32_t   dwTDThresholdUV[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
    14 };

const uint32_t   dwSTADThresholdUV[NOISEFACTOR_MAX + 1] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    140, 140, 140, 140, 141, 141, 141, 141, 142, 142, 142, 142, 143, 143, 143, 143,
    144 };

//!
//! \brief    Setup Vebox_State Command parameter
//! \param    [in] bDiVarianceEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxStateCmdParams
//!           Pointer to VEBOX_STATE command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::SetupVeboxState(
    bool                                    bDiVarianceEnable,
    PMHW_VEBOX_STATE_CMD_PARAMS             pVeboxStateCmdParams)
{
    PMHW_VEBOX_MODE         pVeboxMode;
    MOS_STATUS              eStatus;
    PVPHAL_VEBOX_STATE_G8_BASE              pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    pVeboxMode = &pVeboxStateCmdParams->VeboxMode;
    eStatus    = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(pVeboxStateCmdParams, sizeof(*pVeboxStateCmdParams));

    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        // On Gen8, GlobalIECP must be enabled when the output pipe is Vebox
        pVeboxMode->GlobalIECPEnable = true;
    }
    else
    {
        pVeboxMode->GlobalIECPEnable = IsIECPEnabled();
    }

    pVeboxMode->DIEnable         = bDiVarianceEnable;

    pVeboxMode->DNEnable         = pRenderData->bDenoise;
    pVeboxMode->DNDIFirstFrame   = !pRenderData->bRefValid;

    pVeboxMode->DIOutputFrames   = SetDIOutputFrame(pRenderData,pVeboxState,pVeboxMode);

    if (MEDIA_IS_SKU(pVeboxState->m_pRenderHal->pSkuTable, FtrSingleVeboxSlice))
    {
        pVeboxMode->SingleSliceVeboxEnable = false;
    }
    else
    {
        {
            pVeboxMode->SingleSliceVeboxEnable = (MEDIA_IS_SKU(pVeboxState->m_pRenderHal->pSkuTable, FtrGT3) &&
                MEDIA_IS_SKU(pVeboxState->m_pRenderHal->pSkuTable, FtrEDram)) ? false : true;
        }
    }

    return eStatus;
}

#if VEBOX_AUTO_DENOISE_SUPPORTED
//!
//! \brief    Load update kernel curbe data
//! \details  Loads the static data of update kernel to curbe
//! \param    [out] iCurbeOffsetOutDN
//!           Pointer to DN kernel curbe offset
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::LoadUpdateDenoiseKernelStaticData(
    int32_t*                            iCurbeOffsetOutDN)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    VEBOX_STATE_UPDATE_STATIC_DATA_G8   DNStaticData;        // DN Update kernelStatic parameters
    PMHW_VEBOX_INTERFACE                pVeboxInterface;
    PVPHAL_DENOISE_PARAMS               pDenoiseParams;      // Denoise
    int32_t                             iOffset0, iOffset1;
    MOS_STATUS                          eStatus;
    PVPHAL_VEBOX_STATE_G8_BASE          pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA            pRenderData = GetLastExecRenderData();

    pRenderHal      = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    eStatus         = MOS_STATUS_SUCCESS;

    // init the static data
    MOS_ZeroMemory(&DNStaticData , sizeof(VEBOX_STATE_UPDATE_STATIC_DATA_G8));

    pDenoiseParams = m_currentSurface->pDenoiseParams;
    VPHAL_RENDER_ASSERT(pDenoiseParams);

    // Get offset for slice0 and slice1
    VPHAL_RENDER_CHK_STATUS(VeboxGetStatisticsSurfaceOffsets(
        &iOffset0,
        &iOffset1));

    // Load DN update kernel CURBE data
    if (pRenderData->bAutoDenoise)
    {
        // set the curbe data for DN update kernel
        DNStaticData.DW00.OffsetToSlice0         = iOffset0;
        DNStaticData.DW01.OffsetToSlice1         = iOffset1;
        DNStaticData.DW02.FirstFrameFlag         = pVeboxState->bFirstFrame;
        DNStaticData.DW02.NoiseLevel             = pDenoiseParams->NoiseLevel;
        DNStaticData.DW03.VeboxStatisticsSurface = BI_DN_STATISTICS_SURFACE;
        DNStaticData.DW04.VeboxDndiStateSurface  = BI_DN_VEBOX_STATE_SURFACE;
        DNStaticData.DW05.VeboxTempSurface       = BI_DN_TEMP_SURFACE;

        *iCurbeOffsetOutDN = pRenderHal->pfnLoadCurbeData(
            pRenderHal,
            pRenderData->pMediaState,
            &DNStaticData,
            sizeof(DNStaticData));

        if (*iCurbeOffsetOutDN < 0)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        pRenderData->iCurbeLength += sizeof(DNStaticData);
    }

finish:
    return eStatus;
}
#endif

//!
//! \brief    Vebox get the back-end colorspace conversion matrix
//! \details  When the i/o is A8R8G8B8 or X8R8G8B8, the transfer matrix
//!           needs to be updated accordingly
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pOutSurface
//!           Pointer to output surface of Vebox
//! \return   void
//!
void VPHAL_VEBOX_STATE_G8_BASE::VeboxGetBeCSCMatrix(
    PVPHAL_SURFACE                   pSrcSurface,
    PVPHAL_SURFACE                   pOutSurface)
{
    PVPHAL_VEBOX_STATE_G8_BASE           pVeboxState = this;
    float       fTemp[3];
    uint32_t    uiSize;

    // Get the matrix to use for conversion
    VpHal_GetCscMatrix(
        pSrcSurface->ColorSpace,
        pOutSurface->ColorSpace,
        pVeboxState->fCscCoeff,
        pVeboxState->fCscInOffset,
        pVeboxState->fCscOutOffset);

    // Vebox only supports A8B8G8R8 output, swap the 1st and 3rd rows of the
    // transfer matrix for A8R8G8B8 and X8R8G8B8
    // This only happens when Vebox output is used
    if ((pOutSurface->Format == Format_A8R8G8B8) ||
        (pOutSurface->Format == Format_X8R8G8B8))
    {
        // Swap row 0 and row 2 of Coeff matrix and OutOffset matrix
        uiSize = sizeof(float) * 3;

        // Copy row 0 to Temp
        MOS_SecureMemcpy(
            fTemp,
            uiSize,
            &pVeboxState->fCscCoeff[0],
            uiSize);

        // Copy row 2 to row 0
        MOS_SecureMemcpy(
            &pVeboxState->fCscCoeff[0],
            uiSize,
            &pVeboxState->fCscCoeff[6],
            uiSize);

        // Copy Temp to row 2
        MOS_SecureMemcpy(
            &pVeboxState->fCscCoeff[6],
            uiSize,
            fTemp,
            uiSize);

        // Swap row 0 and row 2 of InOffset matrix
        fTemp[0]                      = pVeboxState->fCscOutOffset[0];
        pVeboxState->fCscOutOffset[0] = pVeboxState->fCscOutOffset[2];
        pVeboxState->fCscOutOffset[2] = fTemp[0];
    }
}

//!
//! \brief    Setup kernels for Vebox auto mode features
//! \details  Setup kernels that co-operate with Vebox auto mode features
//! \param    [in] iKDTIndex
//!           Index to Kernel Parameter Array (defined platform specific)
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::SetupVeboxKernel(
    int32_t                     iKDTIndex)
{
    Kdll_CacheEntry             *pCacheEntryTable;                              // Kernel Cache Entry table
    Kdll_FilterEntry            *pFilter;                                       // Kernel Filter (points to base of filter array)
    int32_t                     iKUID;                                          // Kernel Unique ID (VEBOX uses combined kernels)
    int32_t                     iInlineLength;                                  // Inline data length
    MOS_STATUS                  eStatus;                                        // Return code
    PVPHAL_VEBOX_STATE_G8_BASE  pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    // Initialize Variables
    eStatus             = MOS_STATUS_SUCCESS;
    pFilter             = &pVeboxState->SearchFilter[0];
    pCacheEntryTable    = pVeboxState->m_pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Initialize States
    MOS_ZeroMemory(pFilter, sizeof(pVeboxState->SearchFilter));
    MOS_ZeroMemory(&pRenderData->KernelEntry[iKDTIndex], sizeof(Kdll_CacheEntry));

#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (iKDTIndex == KERNEL_UPDATEDNSTATE)
    {
        iKUID                = IDR_VP_UpdateDNState;
        iInlineLength        = 0; // No inline data
        pRenderData->PerfTag = VPHAL_VEBOX_UPDATE_DN_STATE;
    }
    else // Incorrect index to kernel parameters array
#endif
    {
        VPHAL_RENDER_ASSERTMESSAGE("Incorrect index to kernel parameters array.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Store pointer to Kernel Parameter
    pRenderData->pKernelParam[iKDTIndex] =
        &pVeboxState->pKernelParamTable[iKDTIndex];

    // Set Parameters for Kernel Entry
    pRenderData->KernelEntry[iKDTIndex].iKUID          = iKUID;
    pRenderData->KernelEntry[iKDTIndex].iKCID          = -1;
    pRenderData->KernelEntry[iKDTIndex].iFilterSize    = 2;
    pRenderData->KernelEntry[iKDTIndex].pFilter        = pFilter;
    pRenderData->KernelEntry[iKDTIndex].iSize          = pCacheEntryTable[iKUID].iSize;
    pRenderData->KernelEntry[iKDTIndex].pBinary        = pCacheEntryTable[iKUID].pBinary;

    // set the Inline Data length
    pRenderData->iInlineLength              = iInlineLength;

    VPHAL_RENDER_VERBOSEMESSAGE("Vebox Kernels: %s.", g_KernelDNDI_Str_g8[iKDTIndex]);

finish:
    return eStatus;
}

//!
//! \brief    Check for DN only case
//! \details  Check for DN only case
//! \return   bool
//!           Return true if DN only case, otherwise not
//!
bool VPHAL_VEBOX_STATE_G8_BASE::IsDNOnly()
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    return pRenderData->bDenoise &&
        (!pRenderData->bDeinterlace) &&
        (!IsQueryVarianceEnabled()) &&
        (!IsIECPEnabled());
}


MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::GetFFDISurfParams(
    MOS_FORMAT          &Format,
    MOS_TILE_TYPE       &TileType,
    VPHAL_CSPACE        &ColorSpace,
    VPHAL_SAMPLE_TYPE   &SampleType)
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    if ((pRenderData->bDeinterlace &&
        !pRenderData->bProgressive))
    {
        Format   = Format_YUY2;
        TileType = MOS_TILE_Y;
    }
    else
    {
        // Do not change the format for progressive case
        Format = m_currentSurface->Format;
    }

    ColorSpace = m_currentSurface->ColorSpace;

    // When IECP is enabled and Bob or interlaced scaling is selected for interlaced input,
    // output surface's SampleType should be same to input's. Bob is being
    // done in Composition part
    if (pRenderData->bIECP &&
        ((m_currentSurface->pDeinterlaceParams                         &&
         m_currentSurface->pDeinterlaceParams->DIMode == DI_MODE_BOB) ||
         m_currentSurface->bInterlacedScaling))
    {
        SampleType = m_currentSurface->SampleType;
    }
    else
    {
        SampleType = SAMPLE_PROGRESSIVE;
    }

    return MOS_STATUS_SUCCESS;
}

bool VPHAL_VEBOX_STATE_G8_BASE::IsFFDISurfNeeded()
{
    PVPHAL_VEBOX_RENDER_DATA pRenderData = GetLastExecRenderData();

    if (pRenderData->bDeinterlace || pRenderData->bIECP)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPHAL_VEBOX_STATE_G8_BASE::IsFFDNSurfNeeded()
{
    return GetLastExecRenderData()->bDenoise ? true : false;
}

bool VPHAL_VEBOX_STATE_G8_BASE::IsSTMMSurfNeeded()
{

    return (GetLastExecRenderData()->bDenoise || GetLastExecRenderData()->bDeinterlace);
}

//!
//! \brief    Vebox allocate resources
//! \details  Allocate resources that will be used in Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::AllocateResources()
{
    MOS_STATUS                  eStatus;
    PMOS_INTERFACE              pOsInterface;
    PRENDERHAL_INTERFACE        pRenderHal;
    uint32_t                    dwWidth;
    uint32_t                    dwHeight;
    uint32_t                    dwSize;
    int32_t                     i;
    bool                        bAllocated;
    bool                        bDIEnable;
    MHW_VEBOX_SURFACE_PARAMS    MhwVeboxSurfaceParam;
    PMHW_VEBOX_INTERFACE        pVeboxInterface;
    PVPHAL_VEBOX_STATE_G8_BASE  pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    bAllocated   = false;
    pOsInterface = pVeboxState->m_pOsInterface;
    pRenderHal   = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;

    // Allocate FFDI/IECP surfaces----------------------------------------------
    if (IsFFDISurfNeeded())
    {
        MOS_FORMAT          format;
        MOS_TILE_TYPE       TileType;
        VPHAL_CSPACE        ColorSpace;
        VPHAL_SAMPLE_TYPE   SampleType;

        VPHAL_RENDER_CHK_STATUS(GetFFDISurfParams(format, TileType, ColorSpace, SampleType));

        for (i = 0; i < pVeboxState->iNumFFDISurfaces; i++)
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                    pOsInterface,
                    pVeboxState->FFDISurfaces[i],
                    "VeboxFFDISurface_g8",
                    format,
                    MOS_GFXRES_2D,
                    TileType,
                    pVeboxState->m_currentSurface->dwWidth,
                    pVeboxState->m_currentSurface->dwHeight,
                    false,
                    MOS_MMC_DISABLED,
                    &bAllocated));

            pVeboxState->FFDISurfaces[i]->SampleType = SampleType;

            // Copy rect sizes so that if input surface state needs to adjust,
            // output surface can be adjustted also.
            pVeboxState->FFDISurfaces[i]->rcSrc    = pVeboxState->m_currentSurface->rcSrc;
            pVeboxState->FFDISurfaces[i]->rcDst    = pVeboxState->m_currentSurface->rcDst;
            // Copy max src rect
            pVeboxState->FFDISurfaces[i]->rcMaxSrc = pVeboxState->m_currentSurface->rcMaxSrc;

            // Set Colorspace of FFDI
            pVeboxState->FFDISurfaces[i]->ColorSpace = ColorSpace;
            // Copy ScalingMode, it's used in setting SFC state
            pVeboxState->FFDISurfaces[i]->ScalingMode = pVeboxState->m_currentSurface->ScalingMode;
        }
    }
    else
    {
        // Free FFDI surfaces
        for (i = 0; i < pVeboxState->iNumFFDISurfaces; i++)
        {
            if (pVeboxState->FFDISurfaces[i])
            {
                pOsInterface->pfnFreeResource(
                    pOsInterface,
                    &pVeboxState->FFDISurfaces[i]->OsResource);
            }
        }
    }

    // Allocate FFDN surfaces---------------------------------------------------
    if (IsFFDNSurfNeeded())
    {
        for (i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                    pOsInterface,
                    pVeboxState->FFDNSurfaces[i],
                    "VeboxFFDNSurface_g8",
                    pVeboxState->m_currentSurface->Format,
                    MOS_GFXRES_2D,
                    pVeboxState->m_currentSurface->TileType,
                    pVeboxState->m_currentSurface->dwWidth,
                    pVeboxState->m_currentSurface->dwHeight,
                    false,
                    MOS_MMC_DISABLED,
                    &bAllocated));

            // if allocated, pVeboxState->PreviousSurface is not valid for DN reference.
            if (bAllocated)
            {
                // If DI is enabled, try to use app's reference if provided
                if (pVeboxState->m_currentSurface->pBwdRef && pRenderData->bDeinterlace)
                {
                    CopySurfaceValue(pVeboxState->m_previousSurface, pVeboxState->m_currentSurface->pBwdRef);
                }
                else
                {
                    pRenderData->bRefValid = false;
                }
            }

            // DN's output format should be same to input
            pVeboxState->FFDNSurfaces[i]->SampleType =
                pVeboxState->m_currentSurface->SampleType;

            // Copy rect sizes so that if input surface state needs to adjust,
            // output surface can be adjustted also.
            pVeboxState->FFDNSurfaces[i]->rcSrc    = pVeboxState->m_currentSurface->rcSrc;
            pVeboxState->FFDNSurfaces[i]->rcDst    = pVeboxState->m_currentSurface->rcDst;
            // Copy max src rect
            pVeboxState->FFDNSurfaces[i]->rcMaxSrc = pVeboxState->m_currentSurface->rcMaxSrc;

            // Set Colorspace of FFDN
            pVeboxState->FFDNSurfaces[i]->ColorSpace = pVeboxState->m_currentSurface->ColorSpace;

            // Copy FrameID and parameters, as DN output will be used as next blt's current
            pVeboxState->FFDNSurfaces[i]->FrameID            = pVeboxState->m_currentSurface->FrameID;
            pVeboxState->FFDNSurfaces[i]->pDenoiseParams     = pVeboxState->m_currentSurface->pDenoiseParams;
            // Copy ScalingMode, it's used in setting SFC state
            pVeboxState->FFDNSurfaces[i]->ScalingMode        = pVeboxState->m_currentSurface->ScalingMode;
        }
    }
    else
    {
        // Free FFDN surfaces
        for (i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
        {
            if (pVeboxState->FFDNSurfaces[i])
            {
                pOsInterface->pfnFreeResource(
                    pOsInterface,
                    &pVeboxState->FFDNSurfaces[i]->OsResource);
            }
        }
    }

    // Adjust the rcMaxSrc of pRenderTarget when Vebox output is enabled
    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        pRenderData->pRenderTarget->rcMaxSrc = pVeboxState->m_currentSurface->rcMaxSrc;
    }

    if (IsSTMMSurfNeeded())
    {
        // Allocate STMM (Spatial-Temporal Motion Measure) Surfaces------------------
        for (i = 0; i < VPHAL_NUM_STMM_SURFACES; i++)
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pVeboxState->STMMSurfaces[i],
                "VeboxSTMMSurface_g8",
                Format_STMM,
                MOS_GFXRES_2D,
                MOS_TILE_Y,
                pVeboxState->m_currentSurface->dwWidth,
                pVeboxState->m_currentSurface->dwHeight,
                false,
                MOS_MMC_DISABLED,
                &bAllocated));

            if (bAllocated)
            {
                VPHAL_RENDER_CHK_STATUS(VeboxInitSTMMHistory(i));
            }
        }
    }
    else
    {
        // Free DI history buffers (STMM = Spatial-temporal motion measure)
        for (i = 0; i < VPHAL_NUM_STMM_SURFACES; i++)
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->STMMSurfaces[i].OsResource);
        }
    }

    // Allocate Statistics State Surface----------------------------------------
    // Width to be a aligned on 64 bytes and height is 1/4 the height
    // Per frame information written twice per frame for 2 slices
    // Surface to be a rectangle aligned with dwWidth to get proper dwSize
    bDIEnable  = pRenderData->bDeinterlace || IsQueryVarianceEnabled();

    VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                            pVeboxState->m_currentSurface, &MhwVeboxSurfaceParam));
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->VeboxAdjustBoundary(
        &MhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        bDIEnable));

    dwWidth     = MOS_ALIGN_CEIL(dwWidth, 64);
    dwHeight    = MOS_ROUNDUP_DIVIDE(dwHeight, 4) +
                  MOS_ROUNDUP_DIVIDE(VPHAL_VEBOX_STATISTICS_SIZE_G8 * sizeof(uint32_t), dwWidth);
    dwSize      = dwWidth * dwHeight;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pVeboxState->VeboxStatisticsSurface,
                "VeboxStatisticsSurface_g8",
                Format_Buffer,
                MOS_GFXRES_BUFFER,
                MOS_TILE_LINEAR,
                dwSize,
                1,
                false,
                MOS_MMC_DISABLED,
                &bAllocated));

    if (bAllocated)
    {
        // initialize Statistics Surface
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnFillResource(
                    pOsInterface,
                    &(pVeboxState->VeboxStatisticsSurface.OsResource),
                    dwSize,
                    0));

        pVeboxState->dwVeboxPerBlockStatisticsWidth  = dwWidth;
        pVeboxState->dwVeboxPerBlockStatisticsHeight = dwHeight -
            MOS_ROUNDUP_DIVIDE(VPHAL_VEBOX_STATISTICS_SIZE_G8 * sizeof(uint32_t), dwWidth);
    }

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Allocate Temp Surface for Vebox Update kernels----------------------------------------
    // the surface size is one Page
    dwSize      = MHW_PAGE_SIZE;

    VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
                pOsInterface,
                &pVeboxState->VeboxTempSurface,
                "VeboxTempSurface_g8",
                Format_Buffer,
                MOS_GFXRES_BUFFER,
                MOS_TILE_LINEAR,
                dwSize,
                1,
                false,
                MOS_MMC_DISABLED,
                &bAllocated));

    if (bAllocated)
    {
        // initialize Statistics Surface
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnFillResource(
                    pOsInterface,
                    &(pVeboxState->VeboxTempSurface.OsResource),
                    dwSize,
                    0));
    }
#endif

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        pVeboxState->FreeResources();
    }

    return eStatus;
}

//!
//! \brief    Vebox free resources
//! \details  Free resources that are used in Vebox
//! \return   void
//!
void VPHAL_VEBOX_STATE_G8_BASE::FreeResources()
{
    PVPHAL_VEBOX_STATE_G8_BASE   pVeboxState = this;
    int32_t i;
    PMOS_INTERFACE       pOsInterface = pVeboxState->m_pOsInterface;

    // Free FFDI surfaces
    for (i = 0; i < pVeboxState->iNumFFDISurfaces; i++)
    {
        if (pVeboxState->FFDISurfaces[i])
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->FFDISurfaces[i]->OsResource);
        }
    }

    // Free FFDN surfaces
    for (i = 0; i < VPHAL_NUM_FFDN_SURFACES; i++)
    {
        if (pVeboxState->FFDNSurfaces[i])
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->FFDNSurfaces[i]->OsResource);
        }
    }

    // Free DI history buffers (STMM = Spatial-temporal motion measure)
    for (i = 0; i < VPHAL_NUM_STMM_SURFACES; i++)
    {
        pOsInterface->pfnFreeResource(
            pOsInterface,
            &pVeboxState->STMMSurfaces[i].OsResource);
    }

    // Free Statistics data surface for VEBOX
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pVeboxState->VeboxStatisticsSurface.OsResource);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    // Free Temp Surface for VEBOX
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &pVeboxState->VeboxTempSurface.OsResource);
#endif

}

//!
//! \brief    Setup Vebox_DI_IECP Command params for VEBOX final output surface on G75
//! \details  Setup Vebox_DI_IECP Command params for VEBOX final output surface on G75
//! \param    [in] bDiScdEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxDiIecpCmdParams
//!           Pointer to VEBOX_DI_IECP command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::SetupDiIecpStateForOutputSurf(
    bool                                    bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams)
{
    PMOS_INTERFACE                pOsInterface;
    PRENDERHAL_INTERFACE          pRenderHal;
    PMHW_VEBOX_INTERFACE          pVeboxInterface;
    PVPHAL_VEBOX_STATE_G8_BASE   pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();
    MOS_STATUS                    eStatus     = MOS_STATUS_SUCCESS;

    pOsInterface    = pVeboxState->m_pOsInterface;
    pRenderHal      = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;

    // VEBOX final output surface
    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
                pOsInterface,
                &pRenderData->pRenderTarget->OsResource,
                true,
                true));

        pVeboxDiIecpCmdParams->pOsResCurrOutput   =
            &pRenderData->pRenderTarget->OsResource;
        pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset =
            pRenderData->pRenderTarget->dwOffset;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;
    }
    else if (bDiScdEnable)
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDISurfaces[pRenderData->iFrame1]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResCurrOutput   =
            &pVeboxState->FFDISurfaces[pRenderData->iFrame1]->OsResource;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;

        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDISurfaces[pRenderData->iFrame0]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResPrevOutput   =
            &pVeboxState->FFDISurfaces[pRenderData->iFrame0]->OsResource;
        pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;
    }
    else if (IsIECPEnabled()) // IECP output surface without DI
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDISurfaces[pRenderData->iCurDNOut]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResCurrOutput   =
            &pVeboxState->FFDISurfaces[pRenderData->iCurDNOut]->OsResource;
        pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.CurrentOutputSurfMemObjCtl;
    }

finish:
    return eStatus;
}

//!
//! \brief    Setup Vebox_DI_IECP Command params for Gen75
//! \details  Setup Vebox_DI_IECP Command params for Gen75
//! \param    [in] bDiScdEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxDiIecpCmdParams
//!           Pointer to VEBOX_DI_IECP command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::SetupDiIecpState(
    bool                                    bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams)
{
    PMOS_INTERFACE          pOsInterface;
    PRENDERHAL_INTERFACE    pRenderHal;
    uint32_t                dwWidth;
    uint32_t                dwHeight;
    bool                    bDIEnable;
    MOS_STATUS              eStatus;
    MHW_VEBOX_SURFACE_PARAMS      MhwVeboxSurfaceParam;
    PMHW_VEBOX_INTERFACE          pVeboxInterface;
    PVPHAL_VEBOX_STATE_G8_BASE   pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();

    pOsInterface    = pVeboxState->m_pOsInterface;
    pRenderHal      = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    MOS_ZeroMemory(pVeboxDiIecpCmdParams, sizeof(*pVeboxDiIecpCmdParams));

    // Align dwEndingX with surface state
    bDIEnable  = pRenderData->bDeinterlace || IsQueryVarianceEnabled();
    VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(
                            pVeboxState->m_currentSurface, &MhwVeboxSurfaceParam));
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->VeboxAdjustBoundary(
        &MhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        bDIEnable));

    pVeboxDiIecpCmdParams->dwStartingX = 0;
    pVeboxDiIecpCmdParams->dwEndingX   = dwWidth - 1;

    // Input Surface
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pVeboxState->m_currentSurface->OsResource,
        false,
        true));

    pVeboxDiIecpCmdParams->pOsResCurrInput          =
        &pVeboxState->m_currentSurface->OsResource;
    pVeboxDiIecpCmdParams->dwCurrInputSurfOffset    =
        pVeboxState->m_currentSurface->dwOffset;
    pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value  =
        pVeboxState->DnDiSurfMemObjCtl.CurrentInputSurfMemObjCtl;

    // Reference surface
    if (pRenderData->bRefValid)
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->m_previousSurface->OsResource,
            false,
            true));

        pVeboxDiIecpCmdParams->pOsResPrevInput          =
            &pVeboxState->m_previousSurface->OsResource;
        pVeboxDiIecpCmdParams->dwPrevInputSurfOffset    =
            pVeboxState->m_previousSurface->dwOffset;
        pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value  =
            pVeboxState->DnDiSurfMemObjCtl.PreviousInputSurfMemObjCtl;
    }

    // VEBOX final output surface
    VPHAL_RENDER_CHK_STATUS(SetupDiIecpStateForOutputSurf(bDiScdEnable, pVeboxDiIecpCmdParams));

    // DN intermediate output surface
    if (IsFFDNSurfNeeded())
    {
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]->OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput   =
            &pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut]->OsResource;
        pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.DnOutSurfMemObjCtl;
    }

    // STMM surface
    if (bDiScdEnable || IsSTMMSurfNeeded())
    {
        // STMM in
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistIn].OsResource,
            false,
            true));

        pVeboxDiIecpCmdParams->pOsResStmmInput   =
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistIn].OsResource;
        pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.STMMInputSurfMemObjCtl;

        // STMM out
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
            pOsInterface,
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistOut].OsResource,
            true,
            true));

        pVeboxDiIecpCmdParams->pOsResStmmOutput   =
            &pVeboxState->STMMSurfaces[pRenderData->iCurHistOut].OsResource;
        pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value =
            pVeboxState->DnDiSurfMemObjCtl.STMMOutputSurfMemObjCtl;
    }

    // Statistics data: GNE, FMD, ACE...
    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pVeboxState->VeboxStatisticsSurface.OsResource,
        true,
        true));

    pVeboxDiIecpCmdParams->pOsResStatisticsOutput   =
        &pVeboxState->VeboxStatisticsSurface.OsResource;
    pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value =
        pVeboxState->DnDiSurfMemObjCtl.StatisticsOutputSurfMemObjCtl;

finish:
    return eStatus;
}

//!
//! \brief    Vebox query statistics surface layout
//! \details  Get Specific Layout Info like GNE Offset, size of per frame info inside
//!           Vebox Statistics Surface for BDW.
//! \param    [in] QueryType
//!           Query type
//! \param    [out] pQuery
//!           return layout type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::VeboxQueryStatLayout(
    VEBOX_STAT_QUERY_TYPE       QueryType,
    uint32_t*                   pQuery)
{
    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_ASSERT(pQuery);

    switch (QueryType)
    {
        case VEBOX_STAT_QUERY_GNE_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G8;
            break;

        case VEBOX_STAT_QUERY_PER_FRAME_SIZE:
            *pQuery = VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G8;
            break;

        case VEBOX_STAT_QUERY_FMD_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G8;
            break;

        case VEBOX_STAT_QUERY_STD_OFFEST:
            *pQuery = VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G8;
            break;

        default:
            VPHAL_RENDER_ASSERTMESSAGE("Vebox Statistics Layout Query, type ('%d') is not implemented.", QueryType);
            eStatus = MOS_STATUS_UNKNOWN;
            break;
    }

    return eStatus;
}

//!
//! \brief    Vebox get Luma default value
//! \details  Initialize luma denoise paramters w/ default values.
//! \param    [out] pLumaParams
//!           Pointer to Luma DN parameter
//! \return   void
//!
void VPHAL_VEBOX_STATE_G8_BASE::GetLumaDefaultValue(
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams)
{
    VPHAL_RENDER_ASSERT(pLumaParams);

    pLumaParams->dwDenoiseASDThreshold      = NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT;
    pLumaParams->dwDenoiseHistoryDelta      = NOISE_HISTORY_DELTA_DEFAULT;
    pLumaParams->dwDenoiseMaximumHistory    = NOISE_HISTORY_MAX_DEFAULT;
    pLumaParams->dwDenoiseSTADThreshold     = NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT;
    pLumaParams->dwDenoiseSCMThreshold      = NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT;
    pLumaParams->dwDenoiseMPThreshold       = NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT;
    pLumaParams->dwLTDThreshold             = NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT;
    pLumaParams->dwTDThreshold              = NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT;
}

//!
//! \brief    Vebox set DN parameter
//! \details  Set denoise paramters for luma and chroma.
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [in] pLumaParams
//!           Pointer to Luma DN parameter
//! \param    [in] pChromaParams
//!           Pointer to Chroma DN parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::SetDNDIParams(
    PVPHAL_SURFACE                  pSrcSurface,
    PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
    PVPHAL_DNUV_PARAMS              pChromaParams)
{
    MOS_STATUS                       eStatus;
    PVPHAL_DENOISE_PARAMS            pDNParams;
    uint32_t                         dwDenoiseFactor;
    PVPHAL_VEBOX_RENDER_DATA         pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_ASSERT(pSrcSurface);
    VPHAL_RENDER_ASSERT(pLumaParams);
    VPHAL_RENDER_ASSERT(pChromaParams);
    VPHAL_RENDER_ASSERT(pRenderData);

    eStatus             = MOS_STATUS_SUCCESS;
    pDNParams           = pSrcSurface->pDenoiseParams;

    // Set Luma DN params
    if (pRenderData->bDenoise)
    {
        // Setup Denoise Params
        GetLumaDefaultValue(pLumaParams);

        // Denoise Slider case (no auto DN detect)
        if (!pDNParams->bAutoDetect)
        {
            dwDenoiseFactor = (uint32_t)pDNParams->fDenoiseFactor;

            if (dwDenoiseFactor > NOISEFACTOR_MAX)
            {
                dwDenoiseFactor = NOISEFACTOR_MAX;
            }

            pLumaParams->dwGoodNeighborThreshold = dwGoodNeighborThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseASDThreshold   = dwDenoiseASDThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseHistoryDelta   = dwDenoiseHistoryDelta[dwDenoiseFactor];
            pLumaParams->dwDenoiseMaximumHistory = dwDenoiseMaximumHistory[dwDenoiseFactor];
            pLumaParams->dwDenoiseSTADThreshold  = dwDenoiseSTADThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseSCMThreshold   = dwDenoiseSCMThreshold[dwDenoiseFactor];
            pLumaParams->dwDenoiseMPThreshold    = dwDenoiseMPThreshold[dwDenoiseFactor];
            pLumaParams->dwLTDThreshold          = dwLTDThreshold[dwDenoiseFactor];
            pLumaParams->dwTDThreshold           = dwTDThreshold[dwDenoiseFactor];
        }
    }

    // Set Chroma DN params
    if (pRenderData->bChromaDenoise)
    {
        // Setup Denoise Params
        pChromaParams->dwHistoryDeltaUV = NOISE_HISTORY_DELTA_DEFAULT;
        pChromaParams->dwHistoryMaxUV   = NOISE_HISTORY_MAX_DEFAULT;

        // Denoise Slider case (no auto DN detect)
        if (!pDNParams->bAutoDetect)
        {
            dwDenoiseFactor = (uint32_t)pDNParams->fDenoiseFactor;

            if (dwDenoiseFactor > NOISEFACTOR_MAX)
            {
                dwDenoiseFactor = NOISEFACTOR_MAX;
            }

            pChromaParams->dwSTADThresholdU = dwSTADThresholdUV[dwDenoiseFactor];
            pChromaParams->dwSTADThresholdV = dwSTADThresholdUV[dwDenoiseFactor];
            pChromaParams->dwLTDThresholdU  = dwLTDThresholdUV[dwDenoiseFactor];
            pChromaParams->dwLTDThresholdV  = dwLTDThresholdUV[dwDenoiseFactor];
            pChromaParams->dwTDThresholdU   = dwTDThresholdUV[dwDenoiseFactor];
            pChromaParams->dwTDThresholdV   = dwTDThresholdUV[dwDenoiseFactor];
        }
    }

    return eStatus;
}

//!
//! \brief    Setup surface states for Vebox
//! \details  Setup surface states for use in the current Vebox Operation
//! \param    [in] DiVarianceEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxSurfaceStateCmdParams
//!           Pointer to VEBOX_SURFACE_STATE command parameters
//! \return   void
//!
void VPHAL_VEBOX_STATE_G8_BASE::SetupSurfaceStates(
    bool                                    bDiVarianceEnable,
    PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams)
{
    PVPHAL_VEBOX_STATE_G8_BASE              pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA                pRenderData = GetLastExecRenderData();

    MOS_ZeroMemory(pVeboxSurfaceStateCmdParams,
        sizeof(VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS));

    pVeboxSurfaceStateCmdParams->pSurfInput = pVeboxState->m_currentSurface;

    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))                     // Vebox output pipe
    {
        pVeboxSurfaceStateCmdParams->pSurfOutput =
            pRenderData->pRenderTarget;
    }
    else if (bDiVarianceEnable)                                      // DNDI, DI, DI + IECP
    {
        pVeboxSurfaceStateCmdParams->pSurfOutput =
            pVeboxState->FFDISurfaces[pRenderData->iFrame0];
    }
    else if (IsIECPEnabled())                                    // DN + IECP or IECP only
    {
        pVeboxSurfaceStateCmdParams->pSurfOutput =
            pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
    }
    else if (pRenderData->bDenoise)                                 // DN only
    {
        pVeboxSurfaceStateCmdParams->pSurfOutput =
            pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
        pVeboxSurfaceStateCmdParams->pSurfDNOutput =
            pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unable to determine Vebox Output Surface.");
    }

    pVeboxSurfaceStateCmdParams->pSurfDNOutput =
        pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
    pVeboxSurfaceStateCmdParams->bDIEnable = bDiVarianceEnable;
}

bool VPHAL_VEBOX_STATE_G8_BASE::UseKernelResource()
{
    return false; // can always use driver resource in clear memory
}

//!
//! \brief    Get the output pipe on BDW
//! \details  There are 2 output pipes on BDW. Check which output pipe can be applied
//! \param    [in] pcRenderParams
//!           Pointer to VpHal render parameters
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \param    [out] pbCompNeeded
//!           Return whether composition is needed after Vebox
//! \return   VPHAL_OUTPUT_PIPE_MODE
//!           Return the output pipe mode
//!
VPHAL_OUTPUT_PIPE_MODE VPHAL_VEBOX_STATE_G8_BASE::GetOutputPipe(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    PVPHAL_SURFACE              pSrcSurface,
    bool*                       pbCompNeeded)
{
    VPHAL_OUTPUT_PIPE_MODE      OutputPipe;
    bool                        bCompBypassFeasible;
    bool                        bOutputPipeVeboxFeasible;
    PVPHAL_SURFACE              pTarget;
    PVPHAL_VEBOX_STATE_G8_BASE  pVeboxState = this;

    bCompBypassFeasible = IS_COMP_BYPASS_FEASIBLE(*pbCompNeeded, pcRenderParams, pSrcSurface);
    if (!bCompBypassFeasible)
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
        goto finish;
    }

    bOutputPipeVeboxFeasible = IS_OUTPUT_PIPE_VEBOX_FEASIBLE(pVeboxState, pcRenderParams, pSrcSurface);
    if (bOutputPipeVeboxFeasible)
    {
        OutputPipe = VPHAL_OUTPUT_PIPE_MODE_VEBOX;
        goto finish;
    }

    pTarget    = pcRenderParams->pTarget[0];
    // g75 doesn't support sfc, so set output as comp here.
    OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;

    // Explore the potential to still output by VEBOX and perform quick color fill in composition
    if (bCompBypassFeasible &&
        pcRenderParams->pColorFillParams &&
        pSrcSurface->rcDst.left  == pTarget->rcDst.left &&
        pSrcSurface->rcDst.top   == pTarget->rcDst.top &&
        pSrcSurface->rcDst.right == pTarget->rcDst.right &&
        pSrcSurface->rcDst.bottom < pTarget->rcDst.bottom)
    {
        int32_t lTargetBottom;
        lTargetBottom         = pTarget->rcDst.bottom;
        pTarget->rcDst.bottom = pSrcSurface->rcDst.bottom;

        // Check if Vebox can be the output pipe again
        bOutputPipeVeboxFeasible = IS_OUTPUT_PIPE_VEBOX_FEASIBLE(pVeboxState, pcRenderParams, pSrcSurface);
        if (bOutputPipeVeboxFeasible)
        {
            OutputPipe              = VPHAL_OUTPUT_PIPE_MODE_VEBOX;
            pTarget->bFastColorFill = true;
        }
        pTarget->rcDst.bottom = lTargetBottom;
    }

finish:
    *pbCompNeeded = (OutputPipe == VPHAL_OUTPUT_PIPE_MODE_COMP) ? true : false;
    return OutputPipe;
}

//!
//! \brief    Vebox is needed on BDW
//! \details  Check if Vebox Render operation can be applied
//! \param    [in] pcRenderParams
//!           Pointer to VpHal render parameters
//! \param    [in,out] pRenderPassData
//!           Pointer to Render data
//! \return   bool
//!           Return true if Vebox is needed, otherwise false
//!
bool VPHAL_VEBOX_STATE_G8_BASE::IsNeeded(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    RenderpassData              *pRenderPassData)
{
    PVPHAL_VEBOX_RENDER_DATA    pRenderData;
    PRENDERHAL_INTERFACE        pRenderHal;
    PVPHAL_SURFACE              pRenderTarget;
    bool                        bVeboxNeeded;
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus;
    PVPHAL_VEBOX_STATE_G8_BASE  pVeboxState = this;
    PVPHAL_SURFACE              pSrcSurface;

    bVeboxNeeded  = false;
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);

    pRenderHal    = pVeboxState->m_pRenderHal;
    pOsInterface  = pVeboxState->m_pOsInterface;

    pRenderTarget = pcRenderParams->pTarget[0];
    pRenderData   = pVeboxState->GetLastExecRenderData();
    pSrcSurface   = pRenderPassData->pSrcSurface;

    VPHAL_RENDER_CHK_NULL(pSrcSurface);

    // Check whether VEBOX is available
    // VTd doesn't support VEBOX
    if (!MEDIA_IS_SKU(pVeboxState->m_pSkuTable, FtrVERing))
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    // Check if the Surface size is greater than 64x16 which is the minimum Width and Height VEBOX can handle
    if (pSrcSurface->dwWidth < MHW_VEBOX_MIN_WIDTH || pSrcSurface->dwHeight < MHW_VEBOX_MIN_HEIGHT)
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    pRenderData->Init();

    // Determine the output pipe before setting the rendering flags for Vebox
    SET_VPHAL_OUTPUT_PIPE(
        pRenderData,
        GetOutputPipe(
            pcRenderParams,
            pSrcSurface,
            &pRenderPassData->bCompNeeded));

    // Update execution state based on current and past events such as the
    // # of future and past frames available.
    pVeboxState->UpdateVeboxExecutionState(
        pSrcSurface,
        pRenderData->OutputPipe);

    // Check if Vebox can be used to process the surface
    if (pVeboxState->IsFormatSupported(pSrcSurface))
    {
        // Setup Rendering Flags for Vebox
        VeboxSetRenderingFlags(
            pSrcSurface,
            pRenderTarget);

        // Vebox is needed if Vebox isn't bypassed
        bVeboxNeeded = !pRenderData->bVeboxBypass;
    }

    // Save Alpha passed by App to be used in Vebox
    if (bVeboxNeeded && IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))
    {
        pRenderData->pAlphaParams = pcRenderParams->pCompAlpha;
    }

finish:
    return bVeboxNeeded;
}

#if VEBOX_AUTO_DENOISE_SUPPORTED
//!
//! \brief    Setup surface states for Denoise
//! \details  Setup Surface State for Vebox States Auto DN kernel
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_G8_BASE::SetupSurfaceStatesForDenoise()
{
    PRENDERHAL_INTERFACE            pRenderHal;
    PMOS_INTERFACE                  pOsInterface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams;
    MOS_STATUS                      eStatus;
    bool                            bUseKernelResource;
    const MHW_VEBOX_HEAP            *pVeboxHeap = nullptr;
    PVPHAL_VEBOX_STATE_G8_BASE     pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA        pRenderData = GetLastExecRenderData();

    eStatus            = MOS_STATUS_SUCCESS;
    pRenderHal         = pVeboxState->m_pRenderHal;
    pOsInterface       = pVeboxState->m_pOsInterface;
    VPHAL_RENDER_CHK_STATUS(pVeboxState->m_pVeboxInterface->GetVeboxHeapInfo(
                                &pVeboxHeap));
    VPHAL_RENDER_CHK_NULL(pVeboxHeap);

    bUseKernelResource = UseKernelResource();

    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    MOS_ZeroMemory(&pVeboxState->VeboxHeapResource, sizeof(VPHAL_SURFACE));
    MOS_ZeroMemory(&pVeboxState->tmpResource, sizeof(VPHAL_SURFACE));

    // Treat the 1D buffer as 2D surface
    // VEBox State Surface
    pVeboxState->VeboxHeapResource.Format   = Format_L8;
    pVeboxState->VeboxHeapResource.dwWidth  = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->VeboxHeapResource.dwPitch  = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->VeboxHeapResource.dwHeight =
        MOS_ROUNDUP_DIVIDE(pVeboxHeap->uiInstanceSize, SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH);
    pVeboxState->VeboxHeapResource.dwOffset =
        pVeboxHeap->uiInstanceSize *
        pVeboxHeap->uiCurState;
    pVeboxState->VeboxHeapResource.TileType = MOS_TILE_LINEAR;
    pVeboxState->VeboxHeapResource.OsResource = bUseKernelResource ?
                                    pVeboxHeap->KernelResource :
                                    pVeboxHeap->DriverResource;

    // Temp Surface: for Noise Level History
    pVeboxState->tmpResource.Format = Format_L8;
    pVeboxState->tmpResource.dwWidth = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->tmpResource.dwPitch = SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH; // Hard code for secure Block Copy kernel
    pVeboxState->tmpResource.dwHeight =
        MOS_ROUNDUP_DIVIDE(MHW_PAGE_SIZE, SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH);
    pVeboxState->tmpResource.dwOffset = 0;
    pVeboxState->tmpResource.TileType = MOS_TILE_LINEAR;
    pVeboxState->tmpResource.OsResource = pVeboxState->VeboxTempSurface.OsResource;

    // Statistics Surface-----------------------------------------------------------
    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
                pRenderHal,
                &pVeboxState->VeboxStatisticsSurface,
                &pVeboxState->RenderHalVeboxStatisticsSurface,
                nullptr,
                pRenderData->iBindingTable,
                BI_DN_STATISTICS_SURFACE,
                false));

    // VEBox State Surface-----------------------------------------------------------
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.isOutput     = true;
    SurfaceParams.bWidthInDword_Y   = true;
    SurfaceParams.bWidthInDword_UV  = true;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParams.bWidth16Align     = false;

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                &pVeboxState->VeboxHeapResource,
                &pVeboxState->RenderHalVeboxHeapResource,
                &SurfaceParams,
                pRenderData->iBindingTable,
                BI_DN_VEBOX_STATE_SURFACE,
                true));

    // VEBox Temp Surface-----------------------------------------------------------
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.isOutput     = true;
    SurfaceParams.bWidthInDword_Y   = true;
    SurfaceParams.bWidthInDword_UV  = true;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    SurfaceParams.bWidth16Align     = false;

    VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                pRenderHal,
                &pVeboxState->tmpResource,
                &pVeboxState->RenderHalTmpResource,
                &SurfaceParams,
                pRenderData->iBindingTable,
                BI_DN_TEMP_SURFACE,
                true));

finish:
    return eStatus;
}
#endif

//!
//! \brief    Vebox format support check
//! \details  Checks to see if Vebox operation is supported with source surface format
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   bool
//!           return true if input surface format is supported, otherwise false
//!
bool VPHAL_VEBOX_STATE_G8_BASE::IsFormatSupported(
    PVPHAL_SURFACE              pSrcSurface)
{
    bool    bRet;

    bRet = false;

    // Check if Sample Format is supported
    if (pSrcSurface->Format != Format_NV12 &&
        pSrcSurface->Format != Format_AYUV &&
        pSrcSurface->Format != Format_Y416 &&
        !IS_PA_FORMAT(pSrcSurface->Format))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX.", pSrcSurface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}

//!
//! \brief    Vebox format support check
//! \details  Checks to see if RT format is supported when Vebox output pipe is selected
//! \param    [in] pSrcSurface
//!           Pointer to Render source surface of VPP BLT
//! \param    [in] pRTSurface
//!           Pointer to Render target surface of VPP BLT
//! \return   bool
//!           return true if render target surface format is supported, otherwise false
//!
bool VPHAL_VEBOX_STATE_G8_BASE::IsRTFormatSupported(
    PVPHAL_SURFACE              pSrcSurface,
    PVPHAL_SURFACE              pRTSurface)
{
    bool                        bRet;

    bRet = false;

    // Check if RT Format is supported by Vebox
    if (IS_PA_FORMAT(pRTSurface->Format)  ||
        pRTSurface->Format == Format_NV12)
    {
        // Supported Vebox Render Target format. Vebox Pipe Output can be selected.
        bRet = true;
    }

    if ((pSrcSurface->ColorSpace == CSpace_BT2020) &&
        ((pSrcSurface->Format == Format_P010)      ||
        (pSrcSurface->Format == Format_P016))      &&
        IS_RGB32_FORMAT(pRTSurface->Format))
    {
        bRet = true;
    }

    return bRet;
}

//!
//! \brief    Vebox format support check for DN
//! \details  Check if the input surface format is supported for DN
//! \param    [in] pSrcSurface
//!           Pointer to input surface of Vebox
//! \return   bool
//!           return true if input surface format is supported, otherwise false
//!
bool VPHAL_VEBOX_STATE_G8_BASE::IsDnFormatSupported(
    PVPHAL_SURFACE              pSrcSurface)
{
    bool    bRet;

    bRet = false;
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrcSurface);

    if ((pSrcSurface->Format != Format_YUYV)     &&
        (pSrcSurface->Format != Format_VYUY)     &&
        (pSrcSurface->Format != Format_YVYU)     &&
        (pSrcSurface->Format != Format_UYVY)     &&
        (pSrcSurface->Format != Format_YUY2)     &&
        (pSrcSurface->Format != Format_Y8)       &&
        (pSrcSurface->Format != Format_NV12))
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for VEBOX DN.", pSrcSurface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}

//!
//! \brief    Check if surface format is supported by DI
//! \details  Check if surface format is supported by DI
//! \param    [in] pSrc
//!           Pointer to input surface of Vebox
//! \return   bool
//!           Return true if surface format is supported, otherwise return false
//!
bool VPHAL_VEBOX_STATE_G8_BASE::IsDiFormatSupported(
    PVPHAL_SURFACE              pSrc)
{
    bool bRet = false;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrc);

    if (pSrc->Format != Format_AYUV &&
        pSrc->Format != Format_Y410 &&
        pSrc->Format != Format_Y416 &&
        pSrc->Format != Format_P010 &&
        pSrc->Format != Format_P016 &&
        pSrc->Format != Format_A8B8G8R8 &&
        pSrc->Format != Format_A8R8G8B8 &&
        pSrc->Format != Format_B10G10R10A2 &&
        pSrc->Format != Format_R10G10B10A2 &&
        pSrc->Format != Format_A16B16G16R16 &&
        pSrc->Format != Format_A16R16G16B16)
    {
        bRet = true;
    }
    else
    {
        bRet =  false;
    }

finish:
    return bRet;
}

VPHAL_VEBOX_STATE_G8_BASE::VPHAL_VEBOX_STATE_G8_BASE(
    PMOS_INTERFACE                  pOsInterface,
    PMHW_VEBOX_INTERFACE            pVeboxInterface,
    PRENDERHAL_INTERFACE            pRenderHal,
    PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
    PVPHAL_RNDR_PERF_DATA           pPerfData,
    const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
    MOS_STATUS                      *peStatus) :
    VPHAL_VEBOX_STATE(pOsInterface, pVeboxInterface, nullptr, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus)
{
     // States
    pKernelParamTable       = (PRENDERHAL_KERNEL_PARAM)g_Vebox_KernelParam_g8;
    iNumFFDISurfaces        = 2;  // PE on: 4 used. PE off: 2 used
}
