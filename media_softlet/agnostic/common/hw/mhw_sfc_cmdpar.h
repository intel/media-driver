/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     mhw_sfc_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_SFC_CMDPAR_H__
#define __MHW_SFC_CMDPAR_H__

#include "mhw_sfc.h"
#include "mhw_cmdpar.h"

namespace mhw
{
namespace sfc
{
static constexpr uint32_t MHW_SFC_MAX_WIDTH    = 16 * 1024;
static constexpr uint32_t MHW_SFC_MAX_HEIGHT   = 16 * 1024;

static constexpr uint32_t MHW_SFC_OUTPUT_MIN_WIDTH  = 32;
static constexpr uint32_t MHW_SFC_OUTPUT_MIN_HEIGHT = 32;

static constexpr uint32_t MHW_SFC_MAX_PIPE_NUM = 4;

enum MHW_SFC_INDEX
{
    MHW_SFC_INDEX_0 = 0,
    MHW_SFC_INDEX_1 = 1,
    MHW_SFC_INDEX_2 = 2,
    MHW_SFC_INDEX_3 = 3
};

enum SFC_PIPE_MODE
{
    SFC_PIPE_MODE_VDBOX = 0,
    SFC_PIPE_MODE_VEBOX = 1,
    SFC_PIPE_MODE_HCP   = 2,
    SFC_PIPE_MODE_AVP   = 5
};

enum VD_VE_ORDER_MODE
{
    LCU_16_16_HEVC          = 0,
    LCU_32_32_HEVC          = 1,
    LCU_64_64_HEVC          = 2,
    LCU_64_64_VP9           = 3,  //should use 3 for VP9
    LCU_64_64_SHIFT_AV1     = 0,
    LCU_128_128_SHIFT_AV1   = 1,
    LCU_64_64_NOSHIFT_AV1   = 2,
    LCU_128_128_NOSHIFT_AV1 = 3,
};

struct _MHW_PAR_T(SFC_LOCK)
{
    uint8_t  sfcPipeMode     = 0;               //!< SFC Pipe Mode
    uint32_t dwGaClientId    = 0;               // Ga Client Id
    bool     bOutputToMemory = false;           // Write Vebox or Vdbox o/p to memory
};

struct _MHW_PAR_T(SFC_STATE)
{
    uint8_t  sfcPipeMode                        = 0;    //!< SFC Pipe Mode: VD-to-SFC or VE-to-SFC
    uint32_t dwVDVEInputOrderingMode            = 0;
    uint32_t dwInputChromaSubSampling           = 0;    // Chroma subsampling at SFC input
    float    fAlphaPixel                        = 0.0;  // Alpha pixel
    float    fChromaSubSamplingXSiteOffset      = 0.0f; // Chroma siting X offset
    float    fChromaSubSamplingYSiteOffset      = 0.0f; // Chroma siting Y offset
    uint32_t dwChromaDownSamplingMode           = 0;    // Chroma Downsampling Mode
    uint32_t dwChromaDownSamplingVerticalCoef   = 0;    // Chomra Downsampling Vertical Coef
    uint32_t dwChromaDownSamplingHorizontalCoef = 0;    // Chomra Downsampling Horizontal Coef

    uint32_t   dwOutputFrameHeight              = 0;          // Output Frame Height
    uint32_t   dwOutputFrameWidth               = 0;          // Output Frame Width
    MOS_FORMAT OutputFrameFormat                = Format_Any; // Output Frame Format
    uint32_t   dwInputFrameHeight               = 0;          // Input Frame Height
    uint32_t   dwInputFrameWidth                = 0;          // Input Frame Width
    MOS_FORMAT InputFrameFormat                 = Format_Any; // Input Frame Format

    // Scaling parameters
    uint32_t dwAVSFilterMode                    = 0;     // Bilinear, 5x5 or 8x8
    uint32_t dwSourceRegionHeight               = 0;     // Source/Crop region height
    uint32_t dwSourceRegionWidth                = 0;     // Source/Crop region width
    uint32_t dwSourceRegionVerticalOffset       = 0;     // Source/Crop region vertical offset
    uint32_t dwSourceRegionHorizontalOffset     = 0;     // Source/Crop region horizontal offset
    uint32_t dwScaledRegionHeight               = 0;     // Scaled region height
    uint32_t dwScaledRegionWidth                = 0;     // Scaled region width
    uint32_t dwScaledRegionVerticalOffset       = 0;     // Scaled region vertical offset
    uint32_t dwScaledRegionHorizontalOffset     = 0;     // Scaled region horizontal offset
    uint32_t dwTargetRectangleStartHorizontalOffset = 0;     // Target rectangle start horizontal offset
    uint32_t dwTargetRectangleEndHorizontalOffset   = 0;     // Target rectangle end horizontal offset
    uint32_t dwTargetRectangleStartVerticalOffset   = 0;     // Target rectangle start vertical offset
    uint32_t dwTargetRectangleEndVerticalOffset     = 0;     // Target rectangle end vertical offset
    bool     bRectangleEnabled                  = false; // Target rectangle enabled
    float    fAVSXScalingRatio                  = 0.0f;  // X Scaling Ratio
    float    fAVSYScalingRatio                  = 0.0f;  // Y Scaling Ratio
    bool     bBypassXAdaptiveFilter             = false; // If true, X direction will use Default Sharpness level to blend
                                                         // b/w smooth and sharp filters rather than the calculated value
    bool bBypassYAdaptiveFilter                 = false; // If true, Y direction will use Default Sharpness level to blend
                                                         // b/w smooth and sharp filters rather than the calculated value
    bool bRGBAdaptive                           = false; // If true, Enable the RGB Adaptive filter
    // IEF params
    bool bIEFEnable                             = false;  // IEF Filter enable
    bool bSkinToneTunedIEFEnable                = false;  // Skin Tone Tuned IEF enable
    bool bAVSChromaUpsamplingEnable             = false;  // Up sample chroma prior to IEF filter
    bool b8tapChromafiltering                   = false;  // This bit enables 8 tap filtering for Chroma Channels

    // Rotation Params
    MHW_ROTATION RotationMode                   = MHW_ROTATION_IDENTITY; // Rotation mode -- 0, 90, 180 or 270
    uint32_t     dwMirrorType                   = 0;                     // Mirror Type -- vert/horiz
    bool         bMirrorEnable                  = false;                 // Mirror mode -- enable/disable

    // ColorFill params
    bool  bColorFillEnable                      = false; // ColorFill enable
    float fColorFillYRPixel                     = 0.0f;  // ColorFill Y/R pixel
    float fColorFillUGPixel                     = 0.0f;  // ColorFill U/G pixel
    float fColorFillVBPixel                     = 0.0f;  // ColorFill V/B pixel
    float fColorFillAPixel                      = 0.0f;  // ColorFill A pixel

    // CSC Params
    bool bCSCEnable                             = false;  // YUV->RGB/YUV->YUV CSC enable
    bool bRGBASwapEnable                        = false;  // R, B Channel Swap enable
    bool bInputColorSpace                       = false;  //0: YUV color space, 1:RGB color space
    bool isFullRgbG10P709                       = false;  // Whether output colorspace is COLOR_SPACE_RGB_FULL_G10_NONE_P709
    // Memory compression Enable Flag
    bool bMMCEnable                             = false;            // Flag used to decide whether sfc output should be compressed
    MOS_RESOURCE_MMC_MODE MMCMode               = MOS_MMC_DISABLED; // Memory compression mode

    // Resources used by SFC
    PMOS_RESOURCE pOsResOutputSurface           = nullptr;  // Output Frame written by SFC
    PMOS_RESOURCE pOsResAVSLineBuffer           = nullptr;  // AVS Line buffer used by SFC
    PMOS_RESOURCE pOsResIEFLineBuffer           = nullptr;  // IEF Line buffer used by SFC

    uint32_t dwOutputSurfaceOffset              = 0;  // Output Frame offset (page based offset)
    uint16_t wOutputSurfaceUXOffset             = 0;  // Output Frame offset (page internal U offset for X axis)
    uint16_t wOutputSurfaceUYOffset             = 0;  // Output Frame offset (page internal U offset for Y axis)
    uint16_t wOutputSurfaceVXOffset             = 0;  // Output Frame offset (page internal V offset for X axis)
    uint16_t wOutputSurfaceVYOffset             = 0;  // Output Frame offset (page internal V offset for Y axis)

    // HCP-SFC pipe only for scalability and more input/output color format
    uint32_t engineMode                         = 0;        //!< 0 - single, 1 - left most column, 2 - right most column, 3 - middle column
    uint32_t inputBitDepth                      = 0;        //!< 0 - 8bit, 1 - 10bit, 2 - 12bit
    uint32_t tileType                           = 0;        //!< virtual tile = 1, another tile = 0
    uint32_t srcStartX                          = 0;        //!< Source surface column horizontal start position in pixel
    uint32_t srcEndX                            = 0;        //!< Source surface column horizontal end position in pixel
    uint32_t dstStartX                          = 0;        //!< Destination surface column horizontal start position in pixel
    uint32_t dstEndX                            = 0;        //!< Destination surface column horizontal end position in pixel

    // Histogram stream out
    PMOS_SURFACE histogramSurface               = nullptr;  //!< Histogram stream out buffer
    // Row Store and Column Store Scratch buffer
    PMOS_RESOURCE resAvsLineBuffer              = nullptr;  // AVS Row Store buffer used by SFC
    PMOS_RESOURCE resIefLineBuffer              = nullptr;  // IEF Row Store buffer used by SFC
    PMOS_RESOURCE resSfdLineBuffer              = nullptr;  // SFD Row Store buffer used by SFC
    PMOS_RESOURCE resAvsLineTileBuffer          = nullptr;  // AVS Column Store buffer used by SFC
    PMOS_RESOURCE resIefLineTileBuffer          = nullptr;  // IEF Column Store buffer used by SFC
    PMOS_RESOURCE resSfdLineTileBuffer          = nullptr;  // SFD Column Store buffer used by SFC

    uint32_t ditheringEn                        = 0;        //!< 0 - disable, 1 - enable.

    // Interlaced Scaling parameters
    uint32_t      iScalingType                     = 0;
    uint32_t      inputFrameDataFormat             = 0;  // Input frame data format -- Progressive, Interleaved, Field mode
    uint32_t      outputFrameDataFormat            = 0;  // Output frame data format -- Progressive, Interleaved, Field mode
    uint32_t      topBottomField                   = 0;  // Top/Bottom field -- Top field, Bottom field
    uint32_t      topBottomFieldFirst              = 0;  // Top/Bottom field first
    uint32_t      outputSampleType                 = 0;  // Output sample type
    uint32_t      bottomFieldVerticalScalingOffset = 0;  // Bottom field vertical scaling offset
    PMOS_RESOURCE tempFieldResource = nullptr;           // Temp filed surface

    PMOS_RESOURCE sfcIndirectState  = nullptr;

    PMOS_RESOURCE pOsResAVSLineBufferSplit[MHW_SFC_MAX_PIPE_NUM] = {};  //!< AVS Line buffer used by SFC
    PMOS_RESOURCE pOsResIEFLineBufferSplit[MHW_SFC_MAX_PIPE_NUM] = {};  //!< IEF Line buffer used by SFC
    PMHW_SFC_OUT_SURFACE_PARAMS             pOutSurface = nullptr;
    uint32_t      av1TileRowNumber                               = 0;
    uint32_t      av1TileColumnNumber                            = 0;
    bool          isDemosaicEnabled                              = false;  //!< Enable Demosaic
};

struct _MHW_PAR_T(SFC_AVS_STATE)
{
    uint8_t  sfcPipeMode             = 0;        //!< SFC Pipe Mode
    uint32_t dwInputHorizontalSiting = 0;
    uint32_t dwInputVerticalSitting  = 0;
    uint32_t dwAVSFilterMode         = 0;        // Bilinear, 5x5 or 8x8
};

struct _MHW_PAR_T(SFC_IEF_STATE)
{
    uint8_t  sfcPipeMode = 0; //!< SFC Pipe Mode

    // IEF params
    bool     bSkinDetailFactor   = false;  // Skin Detail Factor
    bool     bVYSTDEnable        = false;  // Enable STD in VY subspace
    bool     bIEFEnable          = false;  // Enable IEF
    uint8_t  StrongEdgeWeight    = 0;
    uint8_t  RegularWeight       = 0;
    uint8_t  StrongEdgeThreshold = 0;
    uint32_t dwGainFactor        = 0;
    uint32_t dwR5xCoefficient    = 0;
    uint32_t dwR5cxCoefficient   = 0;
    uint32_t dwR5cCoefficient    = 0;
    uint32_t dwR3xCoefficient    = 0;
    uint32_t dwR3cCoefficient    = 0;

    // CSC params
    bool  bCSCEnable             = false;    // Enable CSC transform
    float *pfCscCoeff            = nullptr;  // [3x3] CSC Coeff matrix
    float *pfCscInOffset         = nullptr;  // [3x1] CSC Input Offset matrix
    float *pfCscOutOffset        = nullptr;  // [3x1] CSC Output Offset matrix
};

struct _MHW_PAR_T(SFC_AVS_CHROMA_Coeff_Table)
{
    uint8_t                      sfcPipeMode = 0;                           //!< SFC Pipe Mode
    SFC_AVS_CHROMA_FILTER_COEFF  ChromaTable[NUM_HW_POLYPHASE_TABLES] = {};
};

struct _MHW_PAR_T(SFC_AVS_LUMA_Coeff_Table)
{
    uint8_t                      sfcPipeMode = 0;                          //!< SFC Pipe Mode
    SFC_AVS_LUMA_FILTER_COEFF    LumaTable[NUM_HW_POLYPHASE_TABLES] = {};
};

struct _MHW_PAR_T(SFC_FRAME_START)
{
    uint8_t sfcPipeMode = 0;  //!< SFC Pipe Mode
};

}  // namespace sfc
}  // namespace mhw

#endif  // __MHW_SFC_CMDPAR_H__
