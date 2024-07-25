/*
* Copyright (c) 2018-2024, Intel Corporation
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
#ifndef __VP_SFC_COMMON_H__
#define __VP_SFC_COMMON_H__

#include "vp_common.h"
#include "vp_pipeline_common.h"
#include "mhw_sfc_itf.h"

namespace vp
{
//!
//! \brief Constants used to derive Line Buffer sizes
//!
#define SFC_CACHELINE_SIZE_IN_BYTES                         (512 / 8)
#define SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_4_TAP_12BIT       (5 * SFC_CACHELINE_SIZE_IN_BYTES / 8)
#define SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_8_TAP_12BIT       (6 * SFC_CACHELINE_SIZE_IN_BYTES / 8)
#define SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_4_TAP_8BIT        (3 * SFC_CACHELINE_SIZE_IN_BYTES / 8)
#define SFC_AVS_LINEBUFFER_SIZE_PER_PIXEL_8_TAP_8BIT        (5 * SFC_CACHELINE_SIZE_IN_BYTES / 8)
#define SFC_IEF_LINEBUFFER_SIZE_PER_VERTICAL_PIXEL          (1 * SFC_CACHELINE_SIZE_IN_BYTES / 4)
#define SFC_SFD_LINEBUFFER_SIZE_PER_PIXEL                   (1 * SFC_CACHELINE_SIZE_IN_BYTES / 10)
#define SFC_LINEBUFEER_SIZE_LIMITED                         4000

typedef struct _SFC_COLORFILL_PARAMS
{
    bool                            bColorfillEnable;                           // Colorfill Enable
    float                           fColorFillYRPixel;                          // ColorFill Y/R pixel
    float                           fColorFillUGPixel;                          // ColorFill U/G pixel
    float                           fColorFillVBPixel;                          // ColorFill V/B pixel
    float                           fColorFillAPixel;                           // ColorFill A pixel
    float                           fAlphaPixel;                                // Alpha pixel
}SFC_COLORFILL_PARAMS, *PSFC_COLORFILL_PARAMS;

//!
//! \brief Transient Render data populated for every BLT call
//!
typedef struct _VPHAL_SFC_RENDER_DATA
{
    bool                                bColorFill;                             //!< Enable ColorFill
    bool                                bScaling;                               //!< Enable Scaling
    bool                                bIEF;                                   //!< Enable IEF filter
    bool                                bCSC;                                   //!< Enable CSC filter
    bool                                bMirrorEnable;                          //!< Enable Mirror filter

    float                               fScaleX;                                //!< X Scaling ratio
    float                               fScaleY;                                //!< Y Scaling ratio
    uint16_t                            wIEFFactor;                             //!< IEF factor
    mhw::sfc::SFC_STATE_PAR            *sfcStateParams;                         //!< Pointer to SFC state params
    PVPHAL_IEF_PARAMS                   pIefParams;                             //!< Pointer to IEF params
    PMHW_AVS_PARAMS                     pAvsParams;                             //!< Pointer to AVS params
    PSFC_COLORFILL_PARAMS               pColorFillParams;                       //!< Pointer to ColorFill params
    PVPHAL_ALPHA_PARAMS                 pAlphaParams;                           //!< Pointer to Alpha params
    VPHAL_CSPACE                        SfcInputCspace;                         //!< SFC Input Color Space
    MOS_FORMAT                          SfcInputFormat;                         //!< SFC Input Format
    VPHAL_ROTATION                      SfcRotation;                            //!< SFC Rotation Mode
    uint32_t                            mirrorType;                             //!< Mirror Type -- vert/horiz
    VPHAL_SCALING_MODE                  SfcScalingMode;                         //!< SFC Scaling Mode

    uint32_t                            SfcSrcChromaSiting;                     //!< SFC Source Surface Chroma Siting

    PVP_SURFACE                         pSfcPipeOutSurface;                     //!< SFC Pipe output surface

    bool                                bForcePolyPhaseCoefs;                   //!< SFC AVS force polyphase coef
    bool                                b1stPassOfSfc2PassScaling;              //!< 1st Pass of Sfc 2Pass Scaling

} VP_SFC_RENDER_DATA, *PVP_SFC_RENDER_DATA;

typedef struct _VPHAL_SFC_RENDER_DATA_LEGACY
{
    bool                                bColorFill;                             //!< Enable ColorFill
    bool                                bScaling;                               //!< Enable Scaling
    bool                                bIEF;                                   //!< Enable IEF filter
    bool                                bCSC;                                   //!< Enable CSC filter
    bool                                bMirrorEnable;                          //!< Enable Mirror filter

    float                               fScaleX;                                //!< X Scaling ratio
    float                               fScaleY;                                //!< Y Scaling ratio
    uint16_t                            wIEFFactor;                             //!< IEF factor
    MHW_SFC_STATE_PARAMS               *sfcStateParams;                         //!< Pointer to SFC state params
    PVPHAL_IEF_PARAMS                   pIefParams;                             //!< Pointer to IEF params
    PMHW_AVS_PARAMS                     pAvsParams;                             //!< Pointer to AVS params
    PSFC_COLORFILL_PARAMS               pColorFillParams;                       //!< Pointer to ColorFill params
    PVPHAL_ALPHA_PARAMS                 pAlphaParams;                           //!< Pointer to Alpha params
    VPHAL_CSPACE                        SfcInputCspace;                         //!< SFC Input Color Space
    MOS_FORMAT                          SfcInputFormat;                         //!< SFC Input Format
    VPHAL_ROTATION                      SfcRotation;                            //!< SFC Rotation Mode
    uint32_t                            mirrorType;                             //!< Mirror Type -- vert/horiz
    VPHAL_SCALING_MODE                  SfcScalingMode;                         //!< SFC Scaling Mode

    uint32_t                            SfcSrcChromaSiting;                     //!< SFC Source Surface Chroma Siting

    PVP_SURFACE                         pSfcPipeOutSurface;                     //!< SFC Pipe output surface

    bool                                bForcePolyPhaseCoefs;                   //!< SFC AVS force polyphase coef
    bool                                b1stPassOfSfc2PassScaling;              //!< 1st Pass of Sfc 2Pass Scaling

} VP_SFC_RENDER_DATA_LEGACY, * PVP_SFC_RENDER_DATA_LEGACY;

//!
//! \brief  Structure to hold AVS Coeff tables
//!
struct VPHAL_SFC_AVS_STATE
{
    mhw::sfc::SFC_AVS_LUMA_Coeff_Table_PAR      LumaCoeffs;
    mhw::sfc::SFC_AVS_CHROMA_Coeff_Table_PAR    ChromaCoeffs;
    mhw::sfc::SFC_AVS_STATE_PAR                 AvsStateParams;
};

//!
//! \brief  Structure to hold AVS Coeff tables
//!
struct VPHAL_SFC_AVS_STATE_LEGACY
{
    MHW_SFC_AVS_LUMA_TABLE      LumaCoeffs;
    MHW_SFC_AVS_CHROMA_TABLE    ChromaCoeffs;
    MHW_SFC_AVS_STATE           AvsStateParams;
};

}
#endif // !__VP_SFC_COMMON_H__
