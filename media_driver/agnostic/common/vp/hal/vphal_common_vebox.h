/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     vphal_common_vebox.h
//! \brief    vphal VEBOX interface clarification
//! \details  vphal VEBOX interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_COMMON_VEBOX_H__
#define __VPHAL_COMMON_VEBOX_H__

// ProcAmp Default Values
#define PROCAMP_BRIGHTNESS_MIN     -100.0F
#define PROCAMP_BRIGHTNESS_MAX      100.0F
#define PROCAMP_BRIGHTNESS_DEFAULT    0.0F
#define PROCAMP_BRIGHTNESS_STEP       0.1F
#define PROCAMP_CONTRAST_MIN          0.0F
#define PROCAMP_CONTRAST_MAX         10.0F
#define PROCAMP_CONTRAST_DEFAULT      1.0F
#define PROCAMP_CONTRAST_STEP         0.01F
#define PROCAMP_HUE_MIN            -180.0F
#define PROCAMP_HUE_MAX             180.0F
#define PROCAMP_HUE_DEFAULT           0.0F
#define PROCAMP_HUE_STEP              0.1F
#define PROCAMP_SATURATION_MIN        0.0F
#define PROCAMP_SATURATION_MAX       10.0F
#define PROCAMP_SATURATION_DEFAULT    1.0F
#define PROCAMP_SATURATION_STEP       0.01F

// Denoise Default Values
#define NOISEREDUCTION_MIN            0.0F
#define NOISEREDUCTION_MAX           64.0F
#define NOISEREDUCTION_DEFAULT        0.0F
#define NOISEREDUCTION_STEP           1.0F

// Sharpness Values
#define EDGEENHANCEMENT_MIN           0.0F
#define EDGEENHANCEMENT_MAX          64.0F
#define EDGEENHANCEMENT_DEFAULT      44.0F
#define EDGEENHANCEMENT_STEP          1.0F
#define IEF_STRONG_EDGE_WEIGHT          7
#define IEF_REGULAR_WEIGHT              2
#define IEF_STRONG_EDGE_THRESHOLD       8

// Skin Tone Detection/Enhancement  values
#define STE_MIN                       0.0F
#define STE_MAX                       9.0F
#define STE_DEFAULT                   3.0F
#define STE_STEP                      1.0F

// Total Color Correction values
#define TCC_MIN                       0.0F
#define TCC_MAX                     255.0F
#define TCC_DEFAULT                 160.0F
#define TCC_STEP                      1.0F

// Adaptive Contrast Enhancement values
#define ACE_LEVEL_DEFAULT               5
#define ACE_STRENGTH_DEFAULT            1

//!
//! \brief Noise Level Enumeration
//!
typedef enum _VPHAL_NOISELEVEL
{
    NOISELEVEL_DEFAULT,
    NOISELEVEL_VC1_HD
} VPHAL_NOISELEVEL;
C_ASSERT(NOISELEVEL_VC1_HD == 1); //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! Structure VPHAL_PROCAMP_PARAMS
//! \brief Procamp parameters
//!
typedef struct _VPHAL_PROCAMP_PARAMS
{
    bool                bEnabled;
    float               fBrightness;
    float               fContrast;
    float               fHue;
    float               fSaturation;
} VPHAL_PROCAMP_PARAMS, *PVPHAL_PROCAMP_PARAMS;

//!
//! Structure VPHAL_DENOISE_PARAMS
//! \brief Denoise parameters
//!
typedef struct _VPHAL_DENOISE_PARAMS
{
    bool                bEnableChroma;
    bool                bEnableLuma;
    bool                bAutoDetect;
    float               fDenoiseFactor;
    VPHAL_NOISELEVEL    NoiseLevel;
} VPHAL_DENOISE_PARAMS, *PVPHAL_DENOISE_PARAMS;

//!
//! Structure VPHAL_STE_PARAMS
//! \brief STE parameters - Skin Tone Enhancement
//!
typedef struct _VPHAL_STE_PARAMS
{
    uint32_t            dwSTEFactor;
} VPHAL_STE_PARAMS, *PVPHAL_STE_PARAMS;

//!
//! Structure VPHAL_TCC_PARAMS
//! \brief TCC parameters - Total Color Control
//!
typedef struct _VPHAL_TCC_PARAMS
{
    uint8_t             Red;
    uint8_t             Green;
    uint8_t             Blue;
    uint8_t             Cyan;
    uint8_t             Magenta;
    uint8_t             Yellow;
} VPHAL_TCC_PARAMS, *PVPHAL_TCC_PARAMS;

//!
//! Structure VPHAL_COLORPIPE_PARAMS
//! \brief IECP Parameters - Color Pipe
//!
typedef struct _VPHAL_COLORPIPE_PARAMS
{
    bool                bEnableACE;
    bool                bEnableSTE;
    bool                bEnableTCC;
    bool                bAceLevelChanged;
    uint32_t            dwAceLevel;
    uint32_t            dwAceStrength;
    VPHAL_STE_PARAMS    SteParams;
    VPHAL_TCC_PARAMS    TccParams;
} VPHAL_COLORPIPE_PARAMS, *PVPHAL_COLORPIPE_PARAMS;

//!
//! Structure VPHAL_GAMUT_PARAMS
//! \brief IECP Gamut Mapping Parameters
//!
typedef struct _VPHAL_GAMUT_PARAMS
{
    VPHAL_GAMUT_MODE    GCompMode;
    VPHAL_GAMUT_MODE    GExpMode;
    VPHAL_GAMMA_VALUE   GammaValue;
    uint32_t            dwAttenuation;       //!< U2.10 [0, 1024] 0 = No down scaling, 1024 = Full down scaling
    float               displayRGBW_x[4];
    float               displayRGBW_y[4];
} VPHAL_GAMUT_PARAMS, *PVPHAL_GAMUT_PARAMS;

//!
//! Structure VPHAL_DI_PARAMS
//! \brief Deinterlacing parameters
//!
typedef struct _VPHAL_DI_PARAMS
{
    VPHAL_DI_MODE       DIMode;            //!< DeInterlacing mode
    bool                bEnableFMD;        //!< FMD
    bool                bSingleField;      //!< Used in frame Recon - if 30fps (one call per sample pair)
} VPHAL_DI_PARAMS, *PVPHAL_DI_PARAMS;

#endif  // __VPHAL_COMMON_VEBOX_H__
