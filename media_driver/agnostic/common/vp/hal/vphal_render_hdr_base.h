/*
* Copyright (c) 2010-2019, Intel Corporation
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
//! \file      vphal_render_hdr_base.h
//! \brief     Unified VP HAL HDR definitions
//!
//!
//! \file     vphal_render_hdr_base.h
//! \brief    Common interface and structure used in HDR
//! \details  Common interface and structure used in HDR which are platform independent
//!
#ifndef __VPHAL_RENDER_HDR_BASE_H__
#define __VPHAL_RENDER_HDR_BASE_H__

#include "mos_os.h"
#include "vphal.h"
#include "vphal_render_renderstate.h"
#include "vpkrnheader.h"

#if __cplusplus
extern "C"
{
#endif // __cplusplus

#define VPHAL_MAX_HDR_INPUT_LAYER  8
#define VPHAL_MAX_HDR_OUTPUT_LAYER 1

#define VPHAL_HDR_SAMPLER_STATE_NUM    16
#define VPHAL_HDR_SAMPLER8X8_TABLE_NUM 2

#define VPHAL_HDR_EOTF_1DLUT_POINT_NUMBER 256
#define VPHAL_HDR_OETF_1DLUT_POINT_NUMBER 256
#define VPHAL_HDR_OETF_1DLUT_WIDTH        16
#define VPHAL_HDR_OETF_1DLUT_HEIGHT       16

#define HDR_STAGES_CONFIG_TABLE_SIZE      32

#define IS_HDR_SURFACE(surface)    \
    ((surface)->pHDRParams &&      \
    (((surface)->pHDRParams->EOTF == VPHAL_HDR_EOTF_SMPTE_ST2084) || IS_RGB64_FLOAT_FORMAT((surface)->Format)))

#define IS_SDR_SURFACE(surface)             \
    ((surface)->pHDRParams == nullptr ||       \
    (((surface)->pHDRParams->EOTF == VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR) && !IS_RGB64_FLOAT_FORMAT((surface)->Format)))

//!
//! \brief    Hdr kernel ID
//!
#define KERNEL_HDR_MANDATORY    0
#define KERNEL_HDR_PREPROCESS   4
#define KERNEL_HDR_MAX          5

typedef union _HDRCaseID
{
    uint32_t    index;
    struct {
        uint32_t    InputXDR : 1;  // SDR or HDR
        uint32_t    InputGamut : 1;  // 709 or 2020 primaries
        uint32_t    OutputXDR : 1;
        uint32_t    OutputGamut : 1;
        uint32_t    OutputLinear : 1;  // fp16 output
        uint32_t    Reserved : 27;
    };
} HDRCaseID;

typedef union _HDRStageConfigEntry
{
    uint16_t    value;
    struct {
        uint16_t    CCM : 3;
        uint16_t    PWLF : 3;
        uint16_t    CCMExt1 : 3;
        uint16_t    GamutClamp1 : 1;
        uint16_t    CCMExt2 : 3;
        uint16_t    GamutClamp2 : 1;
        uint16_t    Reserved : 1;
        uint16_t    Invalid : 1;
    };
} HDRStageConfigEntry;

//!
//! \brief Hdr stages enable flag
//!
typedef union _HDRStageEnables
{
    uint16_t    value;
    struct {
        uint16_t    PriorCSCEnable : 1;
        uint16_t    EOTFEnable : 1;
        uint16_t    CCMEnable : 1;
        uint16_t    PWLFEnable : 1;
        uint16_t    CCMExt1Enable : 1;
        uint16_t    GamutClamp1Enable : 1;
        uint16_t    CCMExt2Enable : 1;
        uint16_t    GamutClamp2Enable : 1;
        uint16_t    OETFEnable : 1;
        uint16_t    PostCSCEnable : 1;
        uint16_t    Reserved : 6;
    };
} HDRStageEnables, *PHDRStageEnables;

//!
//! \brief HDR Render data populated for every BLT call
//!
typedef struct _VPHAL_HDR_RENDER_DATA
{
    VPHAL_HDR_PARAMS                HDRParams;                           //!< HDR parameters
    PVPHAL_IEF_PARAMS               pIEFParams;                          //!< IEF parameters
    float                           fPrimaryLayerScaleX;                 //!< Primary layer scaling ratio
    float                           fPrimaryLayerScaleY;                 //!< Primary layer scaling ratio
    MOS_FORMAT                      PrimaryLayerFormat;                  //!< Primary Layer Format
    VPHAL_PERFTAG                   PerfTag;                             //!< Performance tag
    PRENDERHAL_MEDIA_STATE          pMediaState;                         //!< Pointer to media state
    MHW_SAMPLER_STATE_PARAM         SamplerStateParams[VPHAL_HDR_SAMPLER_STATE_NUM];       //!< Sampler State
    PMHW_AVS_PARAMS                 pAVSParameters[VPHAL_HDR_SAMPLER8X8_TABLE_NUM];        //!< AVS parameters
    MHW_VFE_SCOREBOARD              ScoreboardParams;                    //!< Scoreboard Parameters
    int32_t                         iBlocksX;                            //!< Block numbers in X direction
    int32_t                         iBlocksY;                            //!< Block numbers in Y direction
    int32_t                         iBindingTable;                       //!< Current binding table index
    int32_t                         iMediaID;                            //!< Current media ID
    PCRENDERHAL_KERNEL_PARAM        pKernelParam[KERNEL_HDR_MAX];        //!< Kernel Parameters
    Kdll_CacheEntry                 KernelEntry[KERNEL_HDR_MAX];         //!< Kernel Entry
    int32_t                         iCurbeLength;                        //!< The length of curbe data
    int32_t                         iCurbeOffset;                        //!< The offset of curbe data
} VPHAL_HDR_RENDER_DATA, *PVPHAL_HDR_RENDER_DATA;

//!
//! \brief VPHAL HDR State
//!
typedef struct _VPHAL_HDR_STATE *PVPHAL_HDR_STATE;
typedef struct _VPHAL_HDR_STATE
{
    PMOS_INTERFACE                  pOsInterface;                   //!< Os Interface
    PRENDERHAL_INTERFACE            pRenderHal;                     //!< Render Interface
    MEDIA_FEATURE_TABLE             *pSkuTable;                     //!< SKU table
    Kdll_KernelCache                *pKernelCache;                  //!< Kernel cache
    PVPHAL_SURFACE                  pSrcSurf[VPHAL_MAX_HDR_INPUT_LAYER];                        //!< Source surface
    PVPHAL_SURFACE                  pTargetSurf[VPHAL_MAX_HDR_OUTPUT_LAYER];                    //!< Output surface
    RENDERHAL_SURFACE               RenderHalSrcSurf[VPHAL_MAX_HDR_INPUT_LAYER];                //!< Source surface
    RENDERHAL_SURFACE               RenderHalTargetSurf[VPHAL_MAX_HDR_OUTPUT_LAYER];            //!< Output surface
    MHW_AVS_PARAMS                  AVSParameters[VPHAL_HDR_SAMPLER8X8_TABLE_NUM];              //!< AVS for configure Sampler
    MHW_SAMPLER_AVS_TABLE_PARAM     mhwSamplerAvsTableParam[VPHAL_HDR_SAMPLER8X8_TABLE_NUM];    //!< params for AVS scaling 8x8 table

    PVPHAL_RNDR_PERF_DATA           pPerfData;                      //!< For performance profiling
    VPHAL_HDR_CACHE_CNTL            SurfMemObjCtl;                  //!< Cache attributes
    PRENDERHAL_KERNEL_PARAM         pKernelParamTable;              //!< Poniter to kernel Parameter table
    VphalFeatureReport              Reporting;                      //!< HDR Mode Reporting
    bool                            bDisableRender;                 //!< For validation purpose
    bool                            bBypassHdrKernelPath;           //!< For bypassing Hdr kernel path
    bool                            bNullHwRenderHdr;               //!< Null rendering flag for HDR function
    bool                            bSplitFrame;                    //!< Split Frame flag for iTouch 5ms requirement
    uint32_t                        uiSplitFramePortions = 1;       //!< Split Frame flag

    uint32_t                        uSourceCount;                                               //!< Number of sources
    uint32_t                        uTargetCount;                                               //!< Number of targets
    uint32_t                        uSourceBindingTableIndex[VPHAL_MAX_HDR_INPUT_LAYER];        //!< Binding Table Index
    uint32_t                        uTargetBindingTableIndex[VPHAL_MAX_HDR_OUTPUT_LAYER];       //!< Binding Table Index
    uint32_t                        dwSurfaceWidth;                                             //!< Record the input surface width of last HDR render
    uint32_t                        dwSurfaceHeight;                                            //!< Record the input surface height of last HDR render
    PVPHAL_COLORFILL_PARAMS         pColorFillParams;                                           //!< ColorFill - BG only

    VPHAL_HDR_LUT_MODE              LUTMode[VPHAL_MAX_HDR_INPUT_LAYER];         //!< LUT Mode
    VPHAL_HDR_LUT_MODE              GlobalLutMode;                              //!< Global LUT mode control for debugging purpose

    uint32_t                        dwOetfSurfaceWidth;             //!< Gamma 1D LUT surface
    uint32_t                        dwOetfSurfaceHeight;            //!< Gamma 1D LUT surface
    uint32_t                        dwUpdateMask;                   //!< Coefficients Update Mask

    VPHAL_SURFACE                   OETF1DLUTSurface[VPHAL_MAX_HDR_INPUT_LAYER];        //!< OETF 1D LUT surface
    VPHAL_SURFACE                   CoeffSurface;                                       //!< CSC CCM Coeff surface

    RENDERHAL_SURFACE               RenderHalOETF1DLUTSurface[VPHAL_MAX_HDR_INPUT_LAYER];       //!< OETF 1D LUT surface
    RENDERHAL_SURFACE               RenderHalCoeffSurface;                                      //!< CSC CCM Coeff surface

    uint16_t                        OetfTraditionalGamma[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER]; //!< EOTF 1D LUT traditional gamma
    uint16_t                        OetfSmpteSt2084[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER];      //!< EOTF 1D LUT SMPTE ST2084
    uint16_t                        OetfsRgb[VPHAL_HDR_OETF_1DLUT_POINT_NUMBER];             //!< EOTF 1D LUT sRGB

    uint8_t*                        pInput3DLUT;                                             //!< Input 3DLUT address for GPU generate 3DLUT

    const uint16_t                  *pHDRStageConfigTable;

    HDRStageEnables                 StageEnableFlags[VPHAL_MAX_HDR_INPUT_LAYER];

    VPHAL_GAMMA_TYPE                EOTFGamma[VPHAL_MAX_HDR_INPUT_LAYER]; //!< EOTF
    VPHAL_GAMMA_TYPE                OETFGamma[VPHAL_MAX_HDR_INPUT_LAYER]; //!< OETF
    VPHAL_HDR_MODE                  HdrMode[VPHAL_MAX_HDR_INPUT_LAYER];   //!< Hdr Mode
    VPHAL_HDR_CCM_TYPE              CCM[VPHAL_MAX_HDR_INPUT_LAYER];       //!< CCM Mode
    VPHAL_HDR_CCM_TYPE              CCMExt1[VPHAL_MAX_HDR_INPUT_LAYER];   //!< CCM Ext1 Mode
    VPHAL_HDR_CCM_TYPE              CCMExt2[VPHAL_MAX_HDR_INPUT_LAYER];   //!< CCM Ext2 Mode
    VPHAL_HDR_CSC_TYPE              PriorCSC[VPHAL_MAX_HDR_INPUT_LAYER];  //!< Prior CSC Mode
    VPHAL_HDR_CSC_TYPE              PostCSC[VPHAL_MAX_HDR_INPUT_LAYER];   //!< Post CSC Mode

    VPHAL_HDR_PARAMS                HDRLastFrameSourceParams[VPHAL_MAX_HDR_INPUT_LAYER];
    VPHAL_HDR_PARAMS                HDRLastFrameTargetParams;

    STATUS_TABLE_UPDATE_PARAMS      StatusTableUpdateParams;                   //!< Status table, Video Pre-Processing Only

    bool                            bFtrComputeWalker;

    MOS_STATUS(*pfnInitialize) (
        PVPHAL_HDR_STATE            pHdrState,
        const VphalSettings         *pSettings,
        Kdll_State                  *pKernelDllState);

    MOS_STATUS(*pfnDestroy) (
        PVPHAL_HDR_STATE            pHdrState);

    MOS_STATUS(*pfnRender) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_RENDER_PARAMS        pRenderParams);

    MOS_STATUS(*pfnFreeResources) (
        PVPHAL_HDR_STATE            pHdrState);

    MOS_STATUS(*pfnAllocateResources) (
        PVPHAL_HDR_STATE            pHdrState);

    MOS_STATUS(*pfnIsInputFormatSupported) (
        PVPHAL_SURFACE              pSrcSurface,
        bool*                       pBeSupported);

    MOS_STATUS(*pfnIsOutputFormatSupported) (
        PVPHAL_SURFACE              pTargetSurface,
        bool*                       pBeSupported);

    MOS_STATUS(*pfnSetupSurfaceStates) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData);

    MOS_STATUS(*pfnSetupPreSurfaceStates) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData);

    MOS_STATUS(*pfnGetKernelParam) (
        uint32_t                    HdrKernelID,
        int32_t*                    pKUIDOut,
        int32_t*                    pKDTIndexOut);

    MOS_STATUS(*pfnLoadStaticData) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData,
        int32_t*                    piCurbeOffsetOut);

    MOS_STATUS(*pfnLoadPreStaticData) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData,
        int32_t*                    piCurbeOffsetOut);

    MOS_STATUS(*pfnIsNeeded)(
        VphalRenderer               *pRenderer,
        bool*                       pBeNeeded);

    MOS_STATUS(*pfnInitAVSParams) (PMHW_AVS_PARAMS pAVS_Params);

    MOS_STATUS(*pfnSetSamplerAvsTable) (
        PRENDERHAL_INTERFACE            pRenderHal,
        PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
        PMHW_AVS_PARAMS                 pAvsParams,
        MOS_FORMAT                      SrcFormat,
        float                           fScaleX,
        float                           fScaleY,
        uint32_t                        dwChromaSiting);

    MOS_STATUS(*pfnSetSamplerStates) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData);

    MOS_STATUS(*pfnSetIefStates) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData,
        PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams);

    MOS_STATUS(*pfnInitOETF1DLUT) (
        PVPHAL_HDR_STATE pHdrState,
        int32_t          iIndex,
        PVPHAL_SURFACE   pOETF1DLUTSurface);

    MOS_STATUS(*pfnInitCoeff) (
        PVPHAL_HDR_STATE pHdrState,
        PVPHAL_SURFACE   pCoeffSurface);

    MOS_STATUS(*pfnInitPWLFCoeff) (
        PVPHAL_HDR_STATE pHdrState,
        PVPHAL_SURFACE   pPWLFCoeffSurface);

    void(*pfnDestoryAVSParams) (PMHW_AVS_PARAMS pAVS_Params);

    MOS_STATUS(*pfnGetSplitFramePortion) (
        PVPHAL_HDR_STATE pHdrState);

    MOS_STATUS(*pfnSetupPreprocessSurfaceStates) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData);

    MOS_STATUS(*pfnLoadPreProcessStaticData) (
        PVPHAL_HDR_STATE            pHdrState,
        PVPHAL_HDR_RENDER_DATA      pRenderData,
        int32_t*                    piCurbeOffsetOut);

} VPHAL_HDR_STATE, *PVPHAL_HDR_STATE;

//!
//! \brief    Set up HDR Render Data
//! \details  Set up HDR render data, including kernel information, input surface's block size
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [out] Pointer to HDR render data
//! \param    int32_t iKUID
//!           [in] Kernel unique ID
//! \param    int32_t iKDTIndex
//            [in] KDT index.
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrSetupRenderData(
    PVPHAL_HDR_STATE        pHdrState,
    PVPHAL_HDR_RENDER_DATA  pRenderData,
    int32_t                 iKUID,
    int32_t                 iKDTIndex);

//!
//! \brief    Hdr init renderer interface
//! \details  Initializes the Hdr interface
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to Hdr state
//! \param    PVPHAL_HW_INTERFACE pHwInterface
//!           [in] Pointer to RenderHal interface
//! \return   void
//!
MOS_STATUS VpHal_HdrInitInterface(
    PVPHAL_HDR_STATE          pHdrState,
    PRENDERHAL_INTERFACE      pRenderHal);

//!
//! \brief    Update per layer pipeline states and return update mask for each layer
//! \details  Update per layer pipeline states and return update mask for each layer
//! \param    PVPHAL_HDR_STATE pHdrStatee
//!           [in] Pointer to HDR state
//! \param    uint32_t* pdwUpdateMask
//!           [out] Pointer to update mask
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrUpdatePerLayerPipelineStates(
    PVPHAL_HDR_STATE pHdrState,
    uint32_t*        pdwUpdateMask);

//! \brief    Setup media walker command for HDR
//! \details  Setup media walker command for HDR
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Pointer to HDR state
//! \param    PVPHAL_HDR_RENDER_DATA pRenderData
//!           [in] Pointer to render data
//! \param    PMHW_WALKER_PARAMS pWalkerParams
//!           [out] Pointer to media walker parameters
//! \param    int32_t iKDTIndex
//            [in] KDT index.
//! \param    uint32_t uiPortionIndex
//            [in] Frame split portion index.
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrSetupWalkerObject(
    PVPHAL_HDR_STATE          pHdrState,
    PVPHAL_HDR_RENDER_DATA    pRenderData,
    PMHW_WALKER_PARAMS        pWalkerParams,
    int32_t                   iKDTIndex,
    uint32_t                  uiPortionIndex);

//!
//! \brief    Render GpGpu Walker Buffer
//! \details  Render GpGpu Walker Buffer, fill Walker static data fields and set walker
//!           cmd params
//! \param    [in] pHdrState
//!           Pointer to HdrState
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \param    [in] pWalkerParams
//!           Pointer to Walker parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise false
//!
MOS_STATUS Vphal_HdrSetupComputeWalker(
    PVPHAL_HDR_STATE                pHdrState,
    PVPHAL_HDR_RENDER_DATA          pRenderData,
    PMHW_GPGPU_WALKER_PARAMS        pWalkerParams);

//! \brief    Perform Rendering HDR step
//! \details  Check whether HDR is needed. When it's needed, perform HDR
//!           operation
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrRenderHDR(
    VphalRenderer           *pRenderer,
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData);

//!
//! \brief    Check if HDR path is needed
//! \details  Check if HDR path is needed
//! \param    [in] pRenderer
//!           VPHAL renderer pointer
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in] pRenderPassData
//!           Pointer to VPHAL render pass data
//! \return   bool
//!
bool VpHal_RndrIsHdrPathNeeded(
    VphalRenderer           *pRenderer,
    PVPHAL_RENDER_PARAMS    pRenderParams,
    RenderpassData          *pRenderPassData);

//!
//! \brief    Checks to see if HDR is needed and supported
//! \details  Checks to see if HDR is needed and supported
//! \param    pRenderer
//            [in] Pointer to VphalRenderer
//! \param    pBeNeeded
//!           [out] 1 Needed 0 not Needed
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrIsNeeded(
    VphalRenderer         *pRenderer,
    bool*                  pBeNeeded);

//!
//! \brief    HDR preprocess
//! \details  Launch HDR pre process kernel to render hdr coefficients surface
//! \param    PVPHAL_HDR_STATE pHdrState
//!           [in] Poniter to HDR state
//! \param    PVPHAL_RENDER_PARAMS pRenderParams
//!           [in,out] Pointer to Render parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_HdrPreprocess(
    PVPHAL_HDR_STATE        pHdrState,
    PVPHAL_RENDER_PARAMS    pRenderParams);

//!
//! \brief    Calculate Yuv To Rgb Matrix
//! \details  Calculate Yuv To Rgb Matrix
//! \param    VPHAL_CSPACE src
//!           [in] Source color space
//! \param    VPHAL_CSPACE dst
//!           [in] Dest color space
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrCalcYuvToRgbMatrix(
    VPHAL_CSPACE    src,
    VPHAL_CSPACE    dst,
    float*          pTransferMatrix,
    float*          pOutMatrix);

//!
//! \brief    Calculate Rgb To Yuv Matrix
//! \details  Calculate Rgb To Yuv Matrix
//! \param    VPHAL_CSPACE src
//!           [in] Source color space
//! \param    VPHAL_CSPACE dst
//!           [in] Dest color space
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrCalcRgbToYuvMatrix(
    VPHAL_CSPACE    src,
    VPHAL_CSPACE    dst,
    float*          pTransferMatrix,
    float*          pOutMatrix);

//!
//! \brief    Calculate CCM Matrix
//! \details  Calculate CCM Matrix
//! \param    float* pTransferMatrix
//!           [in] Pointer to input transfer matrix
//! \param    float* pOutMatrix
//!           [out] Pointer to output transfer matrix for curbe
//! \return   MOS_STATUS
//!
MOS_STATUS VpHal_HdrCalcCCMMatrix(
    float*          pTransferMatrix,
    float*          pOutMatrix);

//!
//! \brief    Calculate CCM Matrix with Monitor Gamut
//! \details  Calculate CCM Matrix with Monitor Gamut
//! \param    VPHAL_HDR_CCM_TYPE CCMType
//!           [in] CCM type
//! \param    PVPHAL_HDR_PARAMS pTarget
//!           [in] Pointer to output surface
//! \param    float TempMatrix
//!           [in] Array of temp matrix
//! \return   MOS_STATUS
//!
void VpHal_CalculateCCMWithMonitorGamut(
    VPHAL_HDR_CCM_TYPE  CCMType,
    PVPHAL_HDR_PARAMS   pTarget,
    float TempMatrix[12]);
#if __cplusplus
}
#endif // __cplusplus
#endif // __VPHAL_RENDER_HDR_BASE_H__
