/*
* Copyright (c) 2014-2019, Intel Corporation
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
//! \file     mhw_vebox.h
//! \brief    MHW interface for constructing commands for the VEBOX 
//! \details  Impelements the functionalities common across all platforms for MHW_VEBOX
//!

#ifndef __MHW_VEBOX_H__
#define __MHW_VEBOX_H__

#include "mos_os.h"
#include "mhw_utilities.h"
#include "mhw_cp_interface.h"

#include <math.h>

#define MHW_FORWARD_GAMMA_SEGMENT_COUNT       1024    //!< Forward Gamma Correction MAX Control Points

//!
//! \brief  VEBOX constants
//!
#define MHW_VEBOX_MIN_HEIGHT      16
#define MHW_VEBOX_MIN_WIDTH       64
#define MHW_VEBOX_LINEAR_PITCH    64
//!
//! \brief  ACE constants
//!
#define MHW_NUM_ACE_PWLF_COEFF    5

//!
//! \brief  Vebox DNDI constants
//!
#define DNDI_HISTORY_INITVALUE                      0xFF

//!
//! \brief  Vebox Denoise pixel range threshold numbers
//!
#define MHW_PIXRANGETHRES_NUM     6

//!
//! \brief  STE Parameters
//!
#define MHW_STE_OPTIMAL           3  // for perfect HD-HQV Score

//!
//! \brief  Default size of area for sync, debugging, performance collecting
//!
#define MHW_SYNC_SIZE   128 // range: (128 ... 4096)

//!
//! \brief  Default number of media states (Dynamic GSH mode)
//!
#define MHW_MAX_VEBOX_STATES    16

#define MHW_PI                     3.14159265358979324f         //!< Definition the const pi

//!
//! \def MHW_DEGREE_TO_RADIAN(degree)
//! Convert \a degree value to \a radian value.
//!
#define MHW_DEGREE_TO_RADIAN(degree)   (degree) * (MHW_PI) / 180
#define AVS_TBL_COEF_PREC         6           //!< Table coef precision (after decimal point
#define SAME_SAMPLE_THRESHOLD     1000        //!< When checking whether 2 timestamps are the same, leave room for some error

#define MHW_VEBOX_WATCHDOG_ENABLE_COUNTER                 0x0
#define MHW_VEBOX_WATCHDOG_DISABLE_COUNTER                0xFFFFFFFF
#define MHW_VEBOX_TIMEOUT_MS                              60

//!
//! \brief Color Spaces enum
//!
typedef enum _MHW_CSPACE
{
    MHW_CSpace_None     = -5        ,   //!< Unidentified
    MHW_CSpace_Source   = -4        ,   //!< Current source Color Space

    // Groups of Color Spaces
    MHW_CSpace_RGB      = -3        ,   //!< sRGB
    MHW_CSpace_YUV      = -2        ,   //!< YUV BT601 or BT709 - non xvYCC
    MHW_CSpace_Gray     = -1        ,   //!< Gray scale image with only Y component
    MHW_CSpace_Any      =  0        ,   //!< Any

    // Specific Color Spaces
    MHW_CSpace_sRGB                 ,   //!< RGB - sRGB       -   RGB[0,255]
    MHW_CSpace_stRGB                ,   //!< RGB - stRGB      -   RGB[16,235]
    MHW_CSpace_BT601                ,   //!< YUV BT.601 Y[16,235] UV[16,240]
    MHW_CSpace_BT601_FullRange      ,   //!< YUV BT.601 Y[0,255]  UV[-128,+127]
    MHW_CSpace_BT709                ,   //!< YUV BT.709 Y[16,235] UV[16,240]
    MHW_CSpace_BT709_FullRange      ,   //!< YUV BT.709 Y[0,255]  UV[-128,+127]
    MHW_CSpace_xvYCC601             ,   //!< xvYCC 601 Y[16,235]  UV[16,240]
    MHW_CSpace_xvYCC709             ,   //!< xvYCC 709 Y[16,235]  UV[16,240]
    MHW_CSpace_BT601Gray            ,   //!< BT.601 Y[16,235]
    MHW_CSpace_BT601Gray_FullRange  ,   //!< BT.601 Y[0,255]
    MHW_CSpace_BT2020               ,   //!< BT.2020 YUV Limited Range 10bit Y[64, 940] UV[64, 960]
    MHW_CSpace_BT2020_FullRange     ,   //!< BT.2020 YUV Full Range 10bit [0, 1023]
    MHW_CSpace_BT2020_RGB           ,   //!< BT.2020 RGB Full Range 10bit [0, 1023]
    MHW_CSpace_BT2020_stRGB         ,   //!< BT.2020 RGB Studio Range 10bit [64, 940]
    MHW_CSpace_Count                    //!< Keep this at the end
} MHW_CSPACE;
C_ASSERT(MHW_CSpace_Count == 15);

//!
//! \brief Gamut Mode enum
//!
typedef enum _MHW_GAMUT_MODE
{
    MHW_GAMUT_MODE_NONE,
    MHW_GAMUT_MODE_BASIC,
    MHW_GAMUT_MODE_ADVANCED
} MHW_GAMUT_MODE;
C_ASSERT(MHW_GAMUT_MODE_ADVANCED == 2);

//!
//! \brief Gamma Values configuration enum
//!
typedef enum _MHW_GAMMA_VALUE
{
    MHW_GAMMA_1P0 = 0,
    MHW_GAMMA_2P2,
    MHW_GAMMA_2P6
} MHW_GAMMA_VALUE;
C_ASSERT(MHW_GAMMA_2P6 == 2);

//!
//! \brief  Structure to Vebox Mode
//!
typedef struct _MHW_VEBOX_MODE
{
    uint32_t    ColorGamutExpansionEnable           : 1;
    uint32_t    ColorGamutCompressionEnable         : 1;
    uint32_t    GlobalIECPEnable                    : 1;
    uint32_t    DNEnable                            : 1;
    uint32_t    DIEnable                            : 1;
    uint32_t    DNDIFirstFrame                      : 1;
    uint32_t    DIOutputFrames                      : 2;
    uint32_t    PipeSynchronizeDisable              : 1;
    uint32_t    DemosaicEnable                      : 1;  // Gen8+
    uint32_t    VignetteEnable                      : 1;  // Gen8+
    uint32_t    AlphaPlaneEnable                    : 1;
    uint32_t    HotPixelFilteringEnable             : 1;  // Gen8+
    uint32_t    SingleSliceVeboxEnable              : 2;  // Gen9+
    uint32_t    LACECorrectionEnable                : 1;  // Gen9+
    uint32_t    DisableEncoderStatistics            : 1;  // Gen9+
    uint32_t    DisableTemporalDenoiseFilter        : 1;  // Gen9+
    uint32_t    SinglePipeIECPEnable                : 1;  // Gen9+
    uint32_t    SFCParallelWriteEnable              : 1;  // Gen9+
    uint32_t    ScalarMode                          : 1;  // Gen10+
    uint32_t    ForwardGammaCorrectionEnable        : 1;  // Gen9+
    uint32_t    Hdr1DLutEnable                      : 1;
    uint32_t    Fp16ModeEnable                      : 1;
    uint32_t                                        : 8; // Reserved
} MHW_VEBOX_MODE, *PMHW_VEBOX_MODE;

typedef enum _MHW_VEBOX_ADDRESS_SHIFT
{
    MHW_VEBOX_DI_IECP_SHIFT                    = 12,
} MHW_VEBOX_ADDRESS_SHIFT;

//!
//! \brief  Structure to Chroma Sampling to handle VEBOX_STATE_CMD Command
//!
typedef struct _MHW_VEBOX_CHROMA_SAMPLING
{
    uint32_t    ChromaUpsamplingCoSitedHorizontalOffset     : 2;
    uint32_t    ChromaUpsamplingCoSitedVerticalOffset       : 3;
    uint32_t    ChromaDownsamplingCoSitedHorizontalOffset   : 2;
    uint32_t    ChromaDownsamplingCoSitedVerticalOffset     : 3;
    uint32_t    BypassChromaUpsampling                      : 1;
    uint32_t    BypassChromaDownsampling                    : 1;
    uint32_t                                                : 20; // Reserved
} MHW_VEBOX_CHROMA_SAMPLING, *PMHW_VEBOX_CHROMA_SAMPLING;

typedef struct _MHW_VEBOX_3D_LUT
{
    uint32_t    ArbitrationPriorityControl                  : 2;
    uint32_t    Lut3dEnable                                 : 1;
    uint32_t    Lut3dSize                                   : 2;
    uint32_t                                                : 27; // Reserved
} MHW_VEBOX_3D_LUT, *PMHW_VEBOX_3D_LUT;

//!
//! \brief  Structure to handle VEBOX_STATE_CMD Command
//!
typedef struct _MHW_VEBOX_STATE_CMD_PARAMS
{
    MHW_VEBOX_MODE                      VeboxMode;
    MHW_VEBOX_CHROMA_SAMPLING           ChromaSampling;
    MHW_VEBOX_3D_LUT                    LUT3D;
    bool                                bUseVeboxHeapKernelResource;
    PMOS_RESOURCE                       pLaceLookUpTables;
    PMOS_RESOURCE                       pVeboxParamSurf;
    PMOS_RESOURCE                       pVebox3DLookUpTables;
    PMOS_RESOURCE                       pVebox1DLookUpTables;
    MOS_RESOURCE                        DummyIecpResource;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    LaceLookUpTablesSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    Vebox3DLookUpTablesSurfCtrl;
    bool                                bNoUseVeboxHeap;
} MHW_VEBOX_STATE_CMD_PARAMS, *PMHW_VEBOX_STATE_CMD_PARAMS;

//!
//! \brief  VEBOX DNDI parameters
//!
typedef struct _MHW_VEBOX_DNDI_PARAMS
{
    // DI and Luma Denoise Params
    uint32_t  dwDenoiseASDThreshold;
    uint32_t  dwDenoiseHistoryDelta;
    uint32_t  dwDenoiseMaximumHistory;
    uint32_t  dwDenoiseSTADThreshold;
    uint32_t  dwDenoiseSCMThreshold;
    uint32_t  dwDenoiseMPThreshold;
    uint32_t  dwLTDThreshold;
    uint32_t  dwTDThreshold;
    uint32_t  dwGoodNeighborThreshold;
    bool      bDNDITopFirst;
    bool      bProgressiveDN;
    uint32_t  dwFMDFirstFieldCurrFrame;
    uint32_t  dwFMDSecondFieldPrevFrame;

    // Pixel Range Threshold Array for 5x5 Spatial Filter
    uint32_t  dwPixRangeThreshold[MHW_PIXRANGETHRES_NUM];
    uint32_t  dwPixRangeWeight[MHW_PIXRANGETHRES_NUM];

    // Chroma Denoise Params
    uint32_t  dwHistoryInitUV;
    uint32_t  dwChromaSTADThreshold;
    uint32_t  dwChromaLTDThreshold;
    uint32_t  dwChromaTDThreshold;
    bool      bChromaDNEnable;

    // Hot Pixel Params
    uint32_t  dwHotPixelThreshold;
    uint32_t  dwHotPixelCount;

    // CNL New DI
    uint32_t  dwLumaTDMWeight;
    uint32_t  dwChromaTDMWeight;
    uint32_t  dwSHCMDelta;
    uint32_t  dwSHCMThreshold;
    uint32_t  dwSVCMDelta;
    uint32_t  dwSVCMThreshold;
    bool      bFasterConvergence;
    bool      bTDMLumaSmallerWindow;
    bool      bTDMChromaSmallerWindow;
    uint32_t  dwLumaTDMCoringThreshold;
    uint32_t  dwChromaTDMCoringThreshold;
    bool      bBypassDeflickerFilter;
    bool      bUseSyntheticContentMedian;
    bool      bLocalCheck;
    bool      bSyntheticContentCheck;
    bool      bSyntheticFrame;
    bool      bSCDEnable;
    uint32_t  dwDirectionCheckThreshold;
    uint32_t  dwTearingLowThreshold;
    uint32_t  dwTearingHighThreshold;
    uint32_t  dwDiffCheckSlackThreshold;
    uint32_t  dwSADWT0;
    uint32_t  dwSADWT1;
    uint32_t  dwSADWT2;
    uint32_t  dwSADWT3;
    uint32_t  dwSADWT4;
    uint32_t  dwSADWT6;
    uint32_t  dwLPFWtLUT0;
    uint32_t  dwLPFWtLUT1;
    uint32_t  dwLPFWtLUT2;
    uint32_t  dwLPFWtLUT3;
    uint32_t  dwLPFWtLUT4;
    uint32_t  dwLPFWtLUT5;
    uint32_t  dwLPFWtLUT6;
    uint32_t  dwLPFWtLUT7;
} MHW_VEBOX_DNDI_PARAMS, *PMHW_VEBOX_DNDI_PARAMS;

//!
//! Structure MHW_STE_PARAMS
//! \brief STE parameters - Skin Tone Enhancement
//!
typedef struct _MHW_STE_PARAMS
{
    uint32_t            dwSTEFactor;
    uint32_t            satP1;
    uint32_t            satS0;
    uint32_t            satS1;
} MHW_STE_PARAMS, *PMHW_STE_PARAMS;

//!
//! Structure MHW_TCC_PARAMS
//! \brief TCC parameters - Total Color Control
//!
typedef struct _MHW_TCC_PARAMS
{
    uint8_t             Red;
    uint8_t             Green;
    uint8_t             Blue;
    uint8_t             Cyan;
    uint8_t             Magenta;
    uint8_t             Yellow;
} MHW_TCC_PARAMS, *PMHW_TCC_PARAMS;

//!
//! Structure MHW_LACE_PARAMS
//! \brief LACE parameters - Local Adaptive Contrast Enhancement
//!
typedef struct _MHW_LACE_PARAMS
{
    bool                bSTD;                               // To enable Skin Tone Special handling
    uint32_t            dwStrength;                         // LACE strength
    uint16_t            wMinAceLuma;                        // Minimum Luma for which ACE has to be applied
    uint16_t            wMaxAceLuma;                        // Maximum Luma for which ACE has to be applied
} MHW_LACE_PARAMS, *PMHW_LACE_PARAMS;

//!
//! Structure MHW_COLORPIPE_PARAMS
//! \brief IECP Parameters - Color Pipe
//!
typedef struct _MHW_COLORPIPE_PARAMS
{
    uint32_t            bActive;                    //!< Active or not
    bool                bEnableACE;
    bool                bEnableSTE;
    bool                bEnableTCC;
    bool                bAceLevelChanged;
    uint32_t            dwAceLevel;
    bool                bEnableLACE;
    MHW_STE_PARAMS      SteParams;
    MHW_TCC_PARAMS      TccParams;
    MHW_LACE_PARAMS     LaceParams;
} MHW_COLORPIPE_PARAMS, *PMHW_COLORPIPE_PARAMS;

//!
//! \brief  Adaptive Contrast Enhancement (ACE) Params
//!
typedef struct _MHW_ACE_PARAMS
{
    uint32_t bActive;
    uint16_t wACEPWLF_X[MHW_NUM_ACE_PWLF_COEFF];                                 //  Piece-wise linear function x
    uint16_t wACEPWLF_Y[MHW_NUM_ACE_PWLF_COEFF];                                 //  Piece-wise linear function y
    uint16_t wACEPWLF_S[MHW_NUM_ACE_PWLF_COEFF];                                 //  Piece-wise linear function slope
    uint16_t wACEPWLF_B[MHW_NUM_ACE_PWLF_COEFF];                                 //  Piece-wise linear function bias
} MHW_ACE_PARAMS, *PMHW_ACE_PARAMS;

//!
//! Structure MHW_PROCAMP_PARAMS
//! \brief Procamp parameters
//!
typedef struct _MHW_PROCAMP_PARAMS
{
    uint32_t            bActive;                    //!< Active or not
    bool                bEnabled;
    uint32_t            brightness;
    uint32_t            contrast;
    uint32_t            sinCS;
    uint32_t            cosCS;
} MHW_PROCAMP_PARAMS, *PMHW_PROCAMP_PARAMS;

//!
//! Structure MHW_HOTPIXEL_PARAMS
//! \brief Hotpixel Parameters
//!
typedef struct _MHW_HOTPIXEL_PARAMS
{
    uint32_t bActive;
    uint32_t PixelThreshold;
    uint32_t PixelCount;
} MHW_HOTPIXEL_PARAMS, *PMHW_HOTPIXEL_PARAMS;

//!
//! Structure MHW_VIGNETTE_PARAMS
//! \brief Vignette Parameters
//!
typedef struct _MHW_VIGNETTE_PARAMS
{
    uint32_t bActive;
    uint32_t Width;
    uint32_t Height;
    uint32_t Stride;
    uint8_t *pCorrectionMap;
} MHW_VIGNETTE_PARAMS, *PMHW_VIGNETTE_PARAMS;

//!
//! Structure MHW_BLACK_LEVEL_PARAMS
//! \brief Black Level Parameters
//!
typedef struct _MHW_BLACK_LEVEL_PARAMS
{
    uint32_t bActive;
    uint32_t R;
    uint32_t G0;
    uint32_t B;
    uint32_t G1;
} MHW_BLACK_LEVEL_PARAMS, *PMHW_BLACK_LEVEL_PARAMS;

//!
//! Structure MHW_WB_MODE
//! \brief WB Parameters
//!
typedef enum _MHW_WB_MODE
{
    MHW_WB_NONE,
    MHW_WB_MANUAL,
    MHW_WB_AUTO_IMAGE,
    MHW_WB_COUNT
} MHW_WB_MODE;

//!
//! Structure MHW_WHITE_BALANCE_PARAMS
//! \brief White Balance Parameters
//!
typedef struct _MHW_WHITE_BALANCE_PARAMS
{
    uint32_t        bActive;
    MHW_WB_MODE     Mode;
    float           RedCorrection;
    float           GreenTopCorrection;
    float           BlueCorrection;
    float           GreenBottomCorrection;
} MHW_WHITE_BALANCE_PARAMS, *PMHW_WHITE_BALANCE_PARAMS;

//!
//! Structure MHW_COLOR_CORRECTION_PARAMS
//! \brief Color Correction Parameters
//!
typedef struct _MHW_COLOR_CORRECTION_PARAMS
{
    uint32_t bActive;
    float   CCM[3][3];
} MHW_COLOR_CORRECTION_PARAMS, *PMHW_COLOR_CORRECTION_PARAMS;

//!
//! Structure MHW_FORWARD_GAMMA_SEG
//! \brief Forward Gamma  SEG Parameters
//!
typedef struct _MHW_FORWARD_GAMMA_SEG
{
    uint16_t PixelValue;                                                         //!< SKL+ U16     Range=[0, 65535]
    uint16_t RedChannelCorrectedValue;                                           //!< SKL+ U16     Range=[0, 65535]
    uint16_t GreenChannelCorrectedValue;                                         //!< SKL+ U16     Range=[0, 65535]
    uint16_t BlueChannelCorrectedValue;                                          //!< SKL+ U16     Range=[0, 65535]
} MHW_FORWARD_GAMMA_SEG, *PMHW_FORWARD_GAMMA_SEG;

//!
//! Structure MHW_FWD_GAMMA_PARAMS
//! \brief Forward Gamma Parameters
//!
typedef struct _MHW_FWD_GAMMA_PARAMS
{
    uint32_t                    bActive;
    MHW_FORWARD_GAMMA_SEG       Segment[MHW_FORWARD_GAMMA_SEGMENT_COUNT];
} MHW_FWD_GAMMA_PARAMS, *PMHW_FWD_GAMMA_PARAMS;

//!
//! Structure MHW_FECSC_PARAMS
//! \brief Front End CSC Parameters
//!
typedef struct _MHW_FECSC_PARAMS
{
    uint32_t bActive;
    float   PreOffset[3];
    float   Matrix[3][3];
    float   PostOffset[3];
} MHW_FECSC_PARAMS, *PMHW_FECSC_PARAMS;

//!
//! Structure MHW_BECSC_PARAMS
//! \brief Back End CSC Parameters
//!
typedef struct _MHW_BECSC_PARAMS
{
    uint32_t bActive;
    float   PreOffset[3];
    float   Matrix[3][3];
    float   PostOffset[3];
} MHW_BECSC_PARAMS, *PMHW_BECSC_PARAMS;

//!
//! Structure MHW_LGCA_PARAMS
//! \brief  Geometric Lens Correction and Chorma Aberration  parameters
//!
typedef struct _MHW_LENS_CORRECTION_PARAMS
{
    uint32_t bActive;                    //!< Active or not
    float   a[3];                       //!< Array (red, green, blue) of values for "a"
    float   b[3];                       //!< Array (red, green, blue) of values for "b"
    float   c[3];                       //!< Array (red, green, blue) of values for "c"
    float   d[3];                       //!< Array (red, green, blue) of values for "d"
} MHW_LENS_CORRECTION_PARAMS, *PMHW_LENS_CORRECTION_PARAMS;

//!
//! Structure MHW_ICC_COLOR_CONVERSION_PARAMS
//! \brief  ICC Color Conversion
//!
typedef struct _MHW_ICC_COLOR_CONVERSION_PARAMS
{
    uint32_t bActive;                    //!< Active or not
    uint32_t LUTSize;                    //!< Size (one dimensions) of the LUT
    uint32_t LUTLength;                  //!< Length of the LUT, in unit of bit
    uint8_t *pLUT;                       //!< Pointer to the LUT value
} MHW_ICC_COLOR_CONVERSION_PARAMS, *PMHW_ICC_COLOR_CONVERSION_PARAMS;

typedef struct _MHW_DEBAYER_PARAMS
{
    uint32_t BayerInput;                  //!< 0 – MSB (default); 1 – LSB
    uint32_t LSBBayerBitDepth;            //!< 10, 12 or 14 for varies bayer input
} MHW_DEBAYER_PARAMS, *PMHW_DEBAYER_PARAMS;

//!
//! Structure MHW_CAPPIPE_PARAMS
//! \brief Capture Pipe Parameters
//!
typedef struct _MHW_CAPPIPE_PARAMS
{
    uint32_t                        bActive;
    MHW_HOTPIXEL_PARAMS             HotPixelParams;
    MHW_VIGNETTE_PARAMS             VignetteParams;
    MHW_BLACK_LEVEL_PARAMS          BlackLevelParams;
    MHW_WHITE_BALANCE_PARAMS        WhiteBalanceParams;
    MHW_COLOR_CORRECTION_PARAMS     ColorCorrectionParams;
    MHW_FWD_GAMMA_PARAMS            FwdGammaParams;
    MHW_FECSC_PARAMS                FECSCParams;
    MHW_BECSC_PARAMS                BECSCParams;
    MHW_LENS_CORRECTION_PARAMS      LensCorrectionParams;
    MHW_ICC_COLOR_CONVERSION_PARAMS ICCColorConversionParams;
    MHW_DEBAYER_PARAMS              DebayerParams;
} MHW_CAPPIPE_PARAMS, *PMHW_CAPPIPE_PARAMS;

//!
//! Structure MHW_3DLUT_PARAMS 
//! \details No pre-si version for MHW_VEBOX_IECP_PARAMS, just leave it now and handle it later
//!
typedef struct _MHW_3DLUT_PARAMS
{
    uint32_t bActive;                    //!< Active or not
    uint32_t LUTSize;                    //!< Size (one dimensions) of the LUT
    uint32_t LUTLength;                  //!< Length of the LUT, in unit of bit
    uint8_t *pLUT;                       //!< Pointer to the LUT value
} MHW_3DLUT_PARAMS, *PMHW_3DLUT_PARAMS;

//! 
//! \brief  VEBOX HDR PARAMS
//! \details For CCM settings, move 1DLut to here later
typedef struct _MHW_1DLUT_PARAMS
{
    uint32_t bActive;
    void     *p1DLUT;
    uint32_t LUTSize;
    int32_t *pCCM;
    uint32_t CCMSize;
} MHW_1DLUT_PARAMS, *PMHW_1DLUT_PARAMS;

//!
//! \brief  VEBOX IECP parameters
//!
typedef struct _MHW_VEBOX_IECP_PARAMS
{
    MHW_COLORPIPE_PARAMS            ColorPipeParams;
    MHW_ACE_PARAMS                  AceParams;
    MHW_PROCAMP_PARAMS              ProcAmpParams;
    MHW_CAPPIPE_PARAMS              CapPipeParams;
    MOS_FORMAT                      dstFormat;
    MOS_FORMAT                      srcFormat;
    MHW_CSPACE                      ColorSpace;                                 // ColorSpace of the input surface

    // CSC params
    bool                            bCSCEnable;                                 // Enable CSC transform
    float                           *pfCscCoeff;                                 // [3x3] CSC Coeff matrix
    float                           *pfCscInOffset;                              // [3x1] CSC Input Offset matrix
    float                           *pfCscOutOffset;                             // [3x1] CSC Output Offset matrix
    bool                            bAlphaEnable;                               // Alpha Enable Param
    uint16_t                        wAlphaValue;                                // Color Pipe Alpha Value

    bool                            bAce;

    MHW_3DLUT_PARAMS                s3DLutParams;
    MHW_1DLUT_PARAMS                s1DLutParams;
} MHW_VEBOX_IECP_PARAMS, *PMHW_VEBOX_IECP_PARAMS;

//!
//! \brief  VEBOX CH_DIR_FILTER_COEFFICIENT parameters
//!
typedef struct _MHW_VEBOX_CH_DIR_FILTER_COEFFICIENT
{
    uint32_t                        FilterCoefficient0;
    uint32_t                        FilterCoefficient1;
    uint32_t                        FilterCoefficient2;
    uint32_t                        FilterCoefficient3;
    uint32_t                        FilterCoefficient4;
    uint32_t                        FilterCoefficient5;
    uint32_t                        FilterCoefficient6;
    uint32_t                        FilterCoefficient7;
} MHW_VEBOX_CH_DIR_FILTER_COEFFICIENT, *PMHW_VEBOX_CH_DIR_FILTER_COEFFICIENT;

//!
//! \brief  VEBOX Scalar parameters
//!
typedef struct _MHW_VEBOX_SCALAR_PARAMS
{
    bool                                    bDisableChromaChannelDiagonalInterpolation;
    MHW_VEBOX_CH_DIR_FILTER_COEFFICIENT     UVChCoefficient0;
    MHW_VEBOX_CH_DIR_FILTER_COEFFICIENT     UVChCoefficient1;
    MHW_VEBOX_CH_DIR_FILTER_COEFFICIENT     UVChCoefficient2;
    MHW_VEBOX_CH_DIR_FILTER_COEFFICIENT     UVChCoefficient3;
    uint32_t                                dwSADWT0;
    uint32_t                                dwSADWT1;
    uint32_t                                dwSADWT2;
    uint32_t                                dwSADWT3;
    uint32_t                                dwSADWT4;
    uint32_t                                dwSADWT5;
} MHW_VEBOX_SCALAR_PARAMS, *PMHW_VEBOX_SCALAR_PARAMS;

//!
//! \brief  VEBOX Gamut parameters
//!
typedef struct _MHW_VEBOX_GAMUT_PARAMS
{
    MHW_CSPACE                          ColorSpace;                             // ColorSpace of the input surface
    MHW_CSPACE                          dstColorSpace;                          // ColorSpace of the output surface
    MOS_FORMAT                          srcFormat;                              // Input surface format
    MOS_FORMAT                          dstFormat;                              // output surface format
    // GComp
    MHW_GAMUT_MODE                      GCompMode;
    // Basic mode params
    uint32_t                            GCompBasicMode;
    int32_t                             iBasicModeScalingFactor;
    // Advanced mode params
    int32_t                             iDin;
    int32_t                             iDinDefault;
    int32_t                             iDout;
    int32_t                             iDoutDefault;

    // GExp
    MHW_GAMUT_MODE                      GExpMode;
    int32_t                             Matrix[3][3];

    // Gamma correction
    bool                                bGammaCorr;
    MHW_GAMMA_VALUE                     InputGammaValue;
    MHW_GAMMA_VALUE                     OutputGammaValue;

    // HDR
    bool                                bH2S;
    uint16_t                            uiMaxCLL;
} MHW_VEBOX_GAMUT_PARAMS, *PMHW_VEBOX_GAMUT_PARAMS;

//!
//! \brief  Structure to handle VEBOX_DI_IECP_CMD Command
//!
typedef struct _MHW_VEBOX_DI_IECP_CMD_PARAMS
{
    uint32_t                            dwEndingX;
    uint32_t                            dwStartingX;
    uint32_t                            dwCurrInputSurfOffset;
    uint32_t                            dwPrevInputSurfOffset;
    uint32_t                            dwCurrOutputSurfOffset;
    uint32_t                            dwStreamID;                         // Stream ID for input surface
    uint32_t                            dwStreamIDOutput;                   // Stream ID for output surface

    PMOS_RESOURCE                       pOsResCurrInput;
    PMOS_RESOURCE                       pOsResPrevInput;
    PMOS_RESOURCE                       pOsResStmmInput;
    PMOS_RESOURCE                       pOsResStmmOutput;
    PMOS_RESOURCE                       pOsResDenoisedCurrOutput;
    PMOS_RESOURCE                       pOsResCurrOutput;
    PMOS_RESOURCE                       pOsResPrevOutput;
    PMOS_RESOURCE                       pOsResStatisticsOutput;
    PMOS_RESOURCE                       pOsResAlphaOrVignette;
    PMOS_RESOURCE                       pOsResLaceOrAceOrRgbHistogram;
    PMOS_RESOURCE                       pOsResSkinScoreSurface;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS    CurrInputSurfCtrl;//can be removed after VPHAL moving to new cmd definition
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    PrevInputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    StmmInputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    StmmOutputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    DenoisedCurrOutputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    CurrOutputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    PrevOutputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    StatisticsOutputSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    AlphaOrVignetteSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    LaceOrAceOrRgbHistogramSurfCtrl;
    MHW_MEMORY_OBJECT_CONTROL_PARAMS    SkinScoreSurfaceSurfCtrl;

    MOS_MEMCOMP_STATE                   CurInputSurfMMCState;
} MHW_VEBOX_DI_IECP_CMD_PARAMS, *PMHW_VEBOX_DI_IECP_CMD_PARAMS;

//!
//! \brief  Structure to VEBOX SURFACE PARAMETERS for VEBOX_SURFACE_STATE_CMD Command
//!
typedef struct _MHW_VEBOX_SURFACE_PARAMS
{
    uint32_t                    bActive;
    bool                        bIsCompressed;
    MOS_FORMAT                  Format;              //!<  Surface format
    uint32_t                    dwCompressionFormat; //!<  Surface Compression Format
    uint32_t                    dwWidth;             //!<  Surface width
    uint32_t                    dwHeight;            //!<  Surface height
    uint32_t                    dwPitch;             //!<  Surface pitch
    uint32_t                    dwBitDepth;          //!<  Surface bitdepth
    uint32_t                    dwStreamID;          //!<  Surface StreamID
    uint32_t                    dwYoffset;           //!<  Surface Yoffset in Vertical
    uint32_t                    dwUYoffset;          //!<  Surface Uoffset in Vertical
    MOS_TILE_TYPE               TileType;            //!<  Tile Type
    RECT                        rcMaxSrc;            //!< Max source rectangle
    PMOS_RESOURCE               pOsResource;         //!<  Surface resource
} MHW_VEBOX_SURFACE_PARAMS, *PMHW_VEBOX_SURFACE_PARAMS;

//!
//! \brief  Structure to handle VEBOX_SURFACE_STATE_CMD Command
//!
typedef struct _MHW_VEBOX_SURFACE_STATE_CMD_PARAMS
{
    MHW_VEBOX_SURFACE_PARAMS         SurfInput;
    MHW_VEBOX_SURFACE_PARAMS         SurfOutput;
    MHW_VEBOX_SURFACE_PARAMS         SurfSTMM;
    MHW_VEBOX_SURFACE_PARAMS         SurfDNOutput;
    MHW_VEBOX_SURFACE_PARAMS         SurfSkinScoreOutput;
    bool                             bDIEnable;
    bool                             bOutputValid;
} MHW_VEBOX_SURFACE_STATE_CMD_PARAMS, *PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS;

//!
//! \brief  VEBOX input surface control bits params
//!
typedef struct _MHW_VEBOX_SURFACE_CNTL_PARAMS
{
    bool                            bIsCompressed;
    MOS_RESOURCE_MMC_MODE           CompressionMode;
} MHW_VEBOX_SURFACE_CNTL_PARAMS, *PMHW_VEBOX_SURFACE_CNTL_PARAMS;

//!
//! \brief  VEBOX Heap State Structure
//!
typedef struct _MHW_VEBOX_HEAP_STATE
{
    bool        bBusy;                                      // true if the state is in use (must sync before use)
    uint32_t    dwSyncTag;                                  // Vebox heap state sync tag
} MHW_VEBOX_HEAP_STATE, *PMHW_VEBOX_HEAP_STATE;

//!
//! \brief  VEBOX Heap Structure
//!
typedef struct _MHW_VEBOX_HEAP
{
    uint32_t                uiCurState;                                         // Current VEBOX State
    uint32_t                uiNextState;                                        // Next VEBOX State
    uint32_t                uiOffsetSync;                                       // Offset of sync data in VeboxHeap
    uint32_t                uiDndiStateOffset;                                  // DNDI or Scalar state offset
    uint32_t                uiIecpStateOffset;                                  // IECP state offset
    uint32_t                uiGamutStateOffset;                                 // Gamut state offset
    uint32_t                uiVertexTableOffset;                                // Vertex Table offset
    uint32_t                uiCapturePipeStateOffset;                           // Capture Pipe state offset
    uint32_t                uiGammaCorrectionStateOffset;                       // Gamma Correction state offset
    uint32_t                ui3DLUTStateOffset;                                 // 3D LUT state offset
    uint32_t                ui1DLUTStateOffset;                                 // Hdr State offset
    uint32_t                uiInstanceSize;                                     // Size of single instance of VEBOX states
    uint32_t                uiStateHeapSize;                                    // Total size of VEBOX States heap
    PMHW_VEBOX_HEAP_STATE   pStates;                                            // Array of VEBOX Heap States
    MOS_RESOURCE            DriverResource;                                     // Graphics memory for Driver access
    MOS_RESOURCE            KernelResource;                                     // Graphics memory for Kernel access
    uint8_t                 *pLockedDriverResourceMem;                           // Locked Driver resource memory

    // Synchronization
    volatile uint32_t       *pSync;                                              // Pointer to sync area (when locked)
    uint32_t                dwNextTag;                                          // Next sync tag value to use
    uint32_t                dwSyncTag;                                          // Last sync tag completed
} MHW_VEBOX_HEAP, *PMHW_VEBOX_HEAP;

//!
//! \brief  VEBOX settings Structure
//!
typedef struct
{
    uint32_t            uiNumInstances;                                         // Number of VEBOX instances to create
    uint32_t            uiSyncSize;                                             // Sync Data Size
    uint32_t            uiDndiStateSize;                                        // DNDI State Size
    uint32_t            uiIecpStateSize;                                        // IECP State Size
    uint32_t            uiGamutStateSize;                                       // Gamut State Size
    uint32_t            uiVertexTableSize;                                      // Vertex Table Size
    uint32_t            uiCapturePipeStateSize;                                 // Capture Pipe State Size (Gen8+)
    uint32_t            uiGammaCorrectionStateSize;                             // Gamma Correction State Size (Gen9+)
    uint32_t            ui3DLUTStateSize;                                       // 3D LUT State Size (Gen10+)
    uint32_t            ui1DLUTStateSize;                                       // VEBOX Hdr 1DLUT State Size
} MHW_VEBOX_SETTINGS, *PMHW_VEBOX_SETTINGS;
typedef const MHW_VEBOX_SETTINGS CMHW_VEBOX_SETTINGS, *PCMHW_VEBOX_SETTINGS;

//!
//! \brief  MHW VEBOX GPUNODE Structure
//!
typedef struct _MHW_VEBOX_GPUNODE_LIMIT
{
    bool    bSfcInUse;
    bool    bCpEnabled;
    uint32_t dwGpuNodeToUse;
} MHW_VEBOX_GPUNODE_LIMIT, *PMHW_VEBOX_GPUNODE_LIMIT;



class MhwVeboxInterface
{
public:
    virtual ~MhwVeboxInterface()
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief      Add VEBOX Vertex Table
    //! \details    Add Vebox vertex table based on color space
    //! \param      [in] ColorSpace
    //!             ColorSpace of the source surface
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxVertexTable(
        MHW_CSPACE                              ColorSpace) = 0;

    //!
    //! \brief    Add Vebox State
    //! \details  Add Vebox State commands
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \param    [in] pVeboxStateCmdParams
    //!           Pointer to Vebox State Params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxState(
        PMOS_COMMAND_BUFFER                     pCmdBuffer,
        PMHW_VEBOX_STATE_CMD_PARAMS             pVeboxStateCmdParams,
        bool                                    bUseCmBuffer) = 0;

    //!
    //! \brief      Send Vebox Surface State commands
    //! \details    Set surface state for input and output surfaces
    //! \param      [in] pCmdBuffer
    //!             Pointer to command buffer
    //! \param      [in] pVeboxSurfaceStateCmdParams
    //!             Pointer to surface state params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxSurfaces(
        PMOS_COMMAND_BUFFER                     pCmdBuffer,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS     pVeboxSurfaceStateCmdParams) = 0;

    //!
    //! \brief    Send Vebox Di Iecp
    //! \details  Send Vebox DI IECP commands
    //! \param    [in] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \param    [in] pVeboxDiIecpCmdParams
    //!           Pointer to DI IECP Params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxDiIecp(
        PMOS_COMMAND_BUFFER                     pCmdBuffer,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams) = 0;

    //!
    //! \brief      Add VEBOX DNDI States
    //! \details    Add vebox dndi states
    //! \param      [in] pVeboxDndiParams
    //!             Pointer to VEBOX DNDI State Params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxDndiState(
        PMHW_VEBOX_DNDI_PARAMS                  pVeboxDndiParams) = 0;

    //!
    //! \brief      Add VEBOX Gamut States
    //! \details    Add Vebox Gamut states
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \param      [in] pVeboxGamutParams
    //!             Pointer to VEBOX Gamut State Params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxGamutState(
        PMHW_VEBOX_IECP_PARAMS                  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS                 pVeboxGamutParams) = 0;

    //!
    //! \brief      Add VEBOX IECP States
    //! \details    Add Vebox IECP states STD/E, ACE, TCC, FECSC, BLC, ProcAmp, also add CapPipe state
    //! \param      [in, out] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxIecpState(
        PMHW_VEBOX_IECP_PARAMS                  pVeboxIecpParams) = 0;

    //!
    //! \brief      Add VEBOX IECP ACE State
    //! \details    Add vebox IECP  ACE State
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxIecpAceState(
        PMHW_VEBOX_IECP_PARAMS                  pVeboxIecpParams) = 0;

    //!
    //! \brief      Vebox adjust boundary
    //! \details    Adjust the width and height of the surface for Vebox
    //! \param      [in] pSurfaceParam
    //!             pointer to input Surface
    //! \param      [out] pdwSurfaceWidth
    //!             Adjusted surface width
    //! \param      [out] pdwSurfaceHeight
    //!             Adjusted surface height
    //! \param      [in] bDIEnable
    //!             identify if DI is enabled
    //! \return     MOS_STATUS
    //!
    virtual MOS_STATUS VeboxAdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS               pSurfaceParam,
        uint32_t                                *pdwSurfaceWidth,
        uint32_t                                *pdwSurfaceHeight,
        bool                                    bDIEnable) = 0;

    //! \brief      Get Full Image Histogram
    //! \details    Get VEBOX IECP ACE State inforation about Full Image Histogram
    //! \param      [in, out] pFullImageHistogram
    //!             Pointer to Full Image Histogram Param
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVeboxAce_FullImageHistogram(
        uint32_t                                *pFullImageHistogram)
    {
        MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }
    //!
    //! \brief    Add Vebox Surface Controls Bits
    //! \details  Add Vebox Surface Control Bits for input surface
    //! \param    [in] pVeboxSurfCntlParams
    //!           Pointer to VEBOX Surface control parameters
    //! \param    [out] pSurfCtrlBits
    //!           Pointer to surface control bits
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS AddVeboxSurfaceControlBits(
        PMHW_VEBOX_SURFACE_CNTL_PARAMS          pVeboxSurfCntlParams,
        uint32_t                                *pSurfCtrlBits) = 0;

    //!
    //! \brief    Decide Which GPU Node to use for Vebox
    //! \details  Client facing function to create gpu context used by Vebox
    //! \param    [out] pVEGpuNodeLimit
    //!           VEGpuNode Limitation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FindVeboxGpuNodeToUse(
        PMHW_VEBOX_GPUNODE_LIMIT                pVeboxGpuNodeLimit)
    {
        MOS_GPU_NODE VeboxGpuNode = MOS_GPU_NODE_VE;
        MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL(pVeboxGpuNodeLimit);

        pVeboxGpuNodeLimit->dwGpuNodeToUse = VeboxGpuNode;

    finish:
        return eStatus;
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
    virtual MOS_STATUS CreateGpuContext(
        PMOS_INTERFACE  pOsInterface,
        MOS_GPU_CONTEXT VeboxGpuContext,
        MOS_GPU_NODE    VeboxGpuNode)
    {
        MOS_GPUCTX_CREATOPTIONS createOption;
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

        MHW_CHK_NULL(pOsInterface);

        MHW_CHK_STATUS(pOsInterface->pfnCreateGpuContext(
            pOsInterface,
            VeboxGpuContext,
            VeboxGpuNode,
            &createOption));

    finish:
        return eStatus;
    }

protected:
    MhwVeboxInterface(PMOS_INTERFACE pOsInterface);

public:
    //!
    //! \brief    Adds a resource to the command buffer or indirect state (SSH)
    //! \details  Internal MHW function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer or state
    //! \param    [in] pOsInterface
    //!           OS interface
    //! \param    [in] pCmdBuffer
    //!           If adding a resource to the command buffer, the buffer to which the resource
    //!           is added
    //! \param    [in] pParams
    //!           Parameters necessary to add the graphics address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*pfnAddResourceToCmd)(
        PMOS_INTERFACE                          pOsInterface,
        PMOS_COMMAND_BUFFER                     pCmdBuffer,
        PMHW_RESOURCE_PARAMS                    pParams);

    //! \brief VEBOX Heap management functions
    //!
    //! \brief    Create Vebox Heap
    //! \details  Create Vebox Heap
    //!     VPHAL_VEBOX (CPU)
    //!      -------------------
    //!     | DriverResource    |
    //!      -------------------
    //!     | KernelResource    |
    //!      -------------------
    //!
    //!      GPU (Driver Resource)      GPU (Kernel Resource)        VEBOX State (in Graphics Memory)
    //!      -------------------         -------------------        ---------------------
    //!     | VEBOX State 0     |       | VEBOX State 0     |       | DNDI State         | 
    //!      -------------------         -------------------         --------------------
    //!     | VEBOX State 1     |       | VEBOX State 1     |       | IECP State         |
    //!      -------------------         -------------------         --------------------
    //!     | VEBOX State N     |       | VEBOX State N     |       | Gamut              |
    //!      -------------------         -------------------         --------------------
    //!     | VEBOX Sync Data   |       | VEBOX Sync Data   |       | Vertex State       |
    //!      -------------------                                     --------------------
    //!                                                             | CapturePipe State  |
    //!                                                              -------------------- 
    //!                                                             | Gamma Correction State |
    //!                                                              ------------------------
    //!                                                             | 3D LUT State           |
    //!                                                              ------------------------
    //! \return   MOS_STATUS
    //!
    MOS_STATUS CreateHeap();

    //!
    //! \brief    Destroy Vebox Heap
    //! \details  Destroy Vebox Heap
    //! \return   MOS_STATUS
    //!
    MOS_STATUS DestroyHeap();

    //!
    //! \brief    Assign Vebox State
    //! \details  Gets a pointer to the next available vebox heap instance
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS: if vebox heap avalible, sets pVeboxHeap->uiCurState
    //!                 pointer to avalible vebox heap instance index and clear
    //!                 bBusy flag for it
    //!           MOS_STATUS_UNKNOWN: invalid, no available vebox heap instance available + timeout
    //!
    MOS_STATUS AssignVeboxState();

    //!
    //! \brief    Get Vebox Heap Information
    //! \details  Get Vebox Heap pointer to access Vebox Heap information
    //! \param    [out] ppVeboxHeap
    //!           Pointer to const Vebox Heap Structure
    //! \return   MOS_STATUS
    //!
    MOS_STATUS GetVeboxHeapInfo(
        const MHW_VEBOX_HEAP                    **ppVeboxHeap);

    //!
    //! \brief    Update Vebox Sync tag info
    //! \details  Update Vebox Heap Sync tag info
    //! \return   MOS_STATUS
    //!
    MOS_STATUS UpdateVeboxSync();

private:
    //!
    //! \brief    Refresh Vebox Sync
    //! \details  Update Vebox Heap Sync tags and clear bBusy flag in availble
    //!           pVeboxHeap->pStates
    //! \return   void
    //!
    void RefreshVeboxSync();

    //! \brief    Vebox heap instance in use
    int                    m_veboxHeapInUse = 0;

 public:
    PMOS_INTERFACE         m_osInterface   = nullptr;
    PMHW_VEBOX_HEAP        m_veboxHeap     = nullptr;
    MHW_VEBOX_SETTINGS     m_veboxSettings;
};

#endif // __MHW_VEBOX_H__
