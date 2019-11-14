/*
* Copyright (c) 2016-2019, Intel Corporation
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
//! \file     vphal_render_composite.cpp
//! \brief    Composite related VPHAL functions
//! \details  Unified VP HAL Composite module including render initialization,
//!           resource allocation/free and rendering
//!
#include "vphal_render_composite.h"
#include "vphal_renderer.h"         // for VpHal_RenderAllocateBB
#include "vphal_render_common.h"    // for VPHAL_RENDER_CACHE_CNTL
#include "vphal_render_ief.h"

// Compositing surface binding table index
#define VPHAL_COMP_BTINDEX_LAYER0          0
#define VPHAL_COMP_BTINDEX_LAYER0_FIELD0   0
#define VPHAL_COMP_BTINDEX_LAYER1          3
#define VPHAL_COMP_BTINDEX_LAYER2          6
#define VPHAL_COMP_BTINDEX_LAYER3          9
#define VPHAL_COMP_BTINDEX_LAYER4         12
#define VPHAL_COMP_BTINDEX_LAYER5         15
#define VPHAL_COMP_BTINDEX_LAYER6         18
#define VPHAL_COMP_BTINDEX_LAYER7         21
#define VPHAL_COMP_BTINDEX_RENDERTARGET   24
#define VPHAL_COMP_BTINDEX_RT_SECOND      27    // Pre-SKL
#define VPHAL_COMP_BTINDEX_L0_FIELD1_DUAL 48    // Pre-SKL

#define VPHAL_HORIZONTAL_16X16BLOCK_MASK   0
#define VPHAL_VERTICAL_16X16BLOCK_MASK     1

// CMFC macro
#define VPHAL_COMP_CMFC_COEFF_WIDTH        24
#define VPHAL_COMP_CMFC_COEFF_HEIGHT       8
#define VPHAL_COMP_BTINDEX_CSC_COEFF       34

//!
//! \brief  Sampler State Indices
//!
#define VPHAL_SAMPLER_8x8_AVS_Y         4
#define VPHAL_SAMPLER_8x8_AVS_U         8
#define VPHAL_SAMPLER_8x8_AVS_V         12

static const MEDIA_OBJECT_KA2_STATIC_DATA g_cInit_MEDIA_OBJECT_KA2_STATIC_DATA =
{
    // DWORD 0
    {
        0,                                      // CscConstantC0
        0                                       // CscConstantC1
    },

    // DWORD 1
    {
        0,                                      // CscConstantC2
        0                                       // CscConstantC3
    },

    // DWORD 2
    {
        0,                                      // CscConstantC4
        0                                       // CscConstantC5
    },

    // DWORD 3
    {
        0,                                      // CscConstantC6
        0                                       // CscConstantC7
    },

    // DWORD 4
    {
        0,                                      // CscConstantC8
        0                                       // CscConstantC9
    },

    // DWORD 5
    {
        0,                                      // CscConstantC10
        0                                       // CscConstantC11
    },

    // DWORD 6
    {
        0,                                      // ConstantBlendingAlphaLayer1
        0,                                      // ConstantBlendingAlphaLayer2
        0,                                      // ConstantBlendingAlphaLayer3
        0                                       // ConstantBlendingAlphaLayer4
    },

    // DWORD 7
    {
        0,                                      // ConstantBlendingAlphaLayer5
        0,                                      // ConstantBlendingAlphaLayer6
        0,                                      // ConstantBlendingAlphaLayer7
        7                                       // PointerToInlineParameters
    },

    // DWORD 8
    {
        0,                                      // DestinationRectangleWidth
        0                                       // DestinationRectangleHeight
    },

    // DWORD 9
    {
        0,                                      // RotationMirrorMode
        0,                                      // RotationMirrorAllLayer
        0,                                      // DualOutputMode
        0,                                      // ChannelSwap
    },

    // DWORD 10
    0,

    // DWORD 11
    0,

    // DWORD 12
    {
        0,                                      // ColorProcessingEnable
        0,                                      // MessageFormat
        0                                       // ColorProcessingStatePointer
    },

    // DWORD 13
    {
        0,                                      // ColorFill_R
        0,                                      // ColorFill_G
        0,                                      // ColorFill_B
        0                                       // ColorFill_A
    },

    // DWORD 14
    {
        0,                                      // LumakeyLowThreshold
        0,                                      // LumakeyHighThreshold
        0,                                      // NLASEnable
    },

    // DWORD 15
    {
        0,                                      // DestinationPackedYOffset
        0,                                      // DestinationPackedUOffset
        0,                                      // DestinationPackedVOffset
        0                                       // DestinationRGBFormat
    },

    // DWORD 16
    0,                                          // HorizontalScalingStepRatioLayer0

    // DWORD 17
    0,                                          // HorizontalScalingStepRatioLayer1

    // DWORD 18
    0,                                          // HorizontalScalingStepRatioLayer2

    // DWORD 19
    0,                                          // HorizontalScalingStepRatioLayer3

    // DWORD 20
    0,                                          // HorizontalScalingStepRatioLayer4

    // DWORD 21
    0,                                          // HorizontalScalingStepRatioLayer5

    // DWORD 22
    0,                                          // HorizontalScalingStepRatioLayer6

    // DWORD 23
    0,                                          // HorizontalScalingStepRatioLayer7

    // DWORD 24
    0,                                          // VerticalScalingStepRatioLayer0

    // DWORD 25
    0,                                          // VerticalScalingStepRatioLayer1

    // DWORD 26
    0,                                          // VerticalScalingStepRatioLayer2

    // DWORD 27
    0,                                          // VerticalScalingStepRatioLayer3

    // DWORD 28
    0,                                          // VerticalScalingStepRatioLayer4

    // DWORD 29
    0,                                          // VerticalScalingStepRatioLayer5

    // DWORD 30
    0,                                          // VerticalScalingStepRatioLayer6

    // DWORD 31
    0,                                          // VerticalScalingStepRatioLayer7

    // DWORD 32
    0,                                          // VerticalFrameOriginLayer0

    // DWORD 33
    0,                                          // VerticalFrameOriginLayer1

    // DWORD 34
    0,                                          // VerticalFrameOriginLayer2

    // DWORD 35
    0,                                          // VerticalFrameOriginLayer3

    // DWORD 36
    0,                                          // VerticalFrameOriginLayer4

    // DWORD 37
    0,                                          // VerticalFrameOriginLayer5

    // DWORD 38
    0,                                          // VerticalFrameOriginLayer6

    // DWORD 39
    0,                                          // VerticalFrameOriginLayer7

    // DWORD 40
    0,                                          // HorizontalFrameOriginLayer0

    // DWORD 41
    0,                                          // HorizontalFrameOriginLayer1

    // DWORD 42
    0,                                          // HorizontalFrameOriginLayer2

    // DWORD 43
    0,                                          // HorizontalFrameOriginLayer3

    // DWORD 44
    0,                                          // HorizontalFrameOriginLayer4

    // DWORD 45
    0,                                          // HorizontalFrameOriginLayer5

    // DWORD 46
    0,                                          // HorizontalFrameOriginLayer6

    // DWORD 47
    0                                           // HorizontalFrameOriginLayer7
};

static const MEDIA_WALKER_KA2_STATIC_DATA g_cInit_MEDIA_WALKER_KA2_STATIC_DATA =
{
    // DWORD 0
    {
        0,                                      // CscConstantC0
        0                                       // CscConstantC1
    },

    // DWORD 1
    {
        0,                                      // CscConstantC2
        0                                       // CscConstantC3
    },

    // DWORD 2
    {
        0,                                      // CscConstantC4
        0                                       // CscConstantC5
    },

    // DWORD 3
    {
        0,                                      // CscConstantC6
        0                                       // CscConstantC7
    },

    // DWORD 4
    {
        0,                                      // CscConstantC8
        0                                       // CscConstantC9
    },

    // DWORD 5
    {
        0,                                      // CscConstantC10
        0                                       // CscConstantC11
    },

    // DWORD 6
    {
        0,                                      // ConstantBlendingAlphaLayer1
        0,                                      // ConstantBlendingAlphaLayer2
        0,                                      // ConstantBlendingAlphaLayer3
        0                                       // ConstantBlendingAlphaLayer4
    },

    // DWORD 7
    {
        0,                                      // ConstantBlendingAlphaLayer5
        0,                                      // ConstantBlendingAlphaLayer6
        0,                                      // ConstantBlendingAlphaLayer7
        7                                       // PointerToInlineParameters
    },

    // DWORD 8
    {
        0,                                      // DestinationRectangleWidth
        0                                       // DestinationRectangleHeight
    },

    // DWORD 9
    {
        0,                                      // RotationMirrorMode
        0,                                      // RotationMirrorAllLayer
        0,                                      // DualOutputMode
        0,                                      // ChannelSwap
    },

    // DWORD 10
    0,

    // DWORD 11
    0,

    // DWORD 12
    {
        0,                                      // ColorProcessingEnable
        0,                                      // MessageFormat
        0                                       // ColorProcessingStatePointer
    },

    // DWORD 13
    {
        0,                                      // ColorFill_R
        0,                                      // ColorFill_G
        0,                                      // ColorFill_B
        0                                       // ColorFill_A
    },

    // DWORD 14
    {
        0,                                      // LumakeyLowThreshold
        0,                                      // LumakeyHighThreshold
        0,                                  // NLASEnable
    },

    // DWORD 15
    {
        0,                                      // DestinationPackedYOffset
        0,                                      // DestinationPackedUOffset
        0,                                      // DestinationPackedVOffset
        0                                       // DestinationRGBFormat
    },

    // DWORD 16
    0,                                          // HorizontalScalingStepRatioLayer0

    // DWORD 17
    0,                                          // HorizontalScalingStepRatioLayer1

    // DWORD 18
    0,                                          // HorizontalScalingStepRatioLayer2

    // DWORD 19
    0,                                          // HorizontalScalingStepRatioLayer3

    // DWORD 20
    0,                                          // HorizontalScalingStepRatioLayer4

    // DWORD 21
    0,                                          // HorizontalScalingStepRatioLayer5

    // DWORD 22
    0,                                          // HorizontalScalingStepRatioLayer6

    // DWORD 23
    0,                                          // HorizontalScalingStepRatioLayer7

    // DWORD 24
    0,                                          // VerticalScalingStepRatioLayer0

    // DWORD 25
    0,                                          // VerticalScalingStepRatioLayer1

    // DWORD 26
    0,                                          // VerticalScalingStepRatioLayer2

    // DWORD 27
    0,                                          // VerticalScalingStepRatioLayer3

    // DWORD 28
    0,                                          // VerticalScalingStepRatioLayer4

    // DWORD 29
    0,                                          // VerticalScalingStepRatioLayer5

    // DWORD 30
    0,                                          // VerticalScalingStepRatioLayer6

    // DWORD 31
    0,                                          // VerticalScalingStepRatioLayer7

    // DWORD 32
    0,                                          // VerticalFrameOriginLayer0

    // DWORD 33
    0,                                          // VerticalFrameOriginLayer1

    // DWORD 34
    0,                                          // VerticalFrameOriginLayer2

    // DWORD 35
    0,                                          // VerticalFrameOriginLayer3

    // DWORD 36
    0,                                          // VerticalFrameOriginLayer4

    // DWORD 37
    0,                                          // VerticalFrameOriginLayer5

    // DWORD 38
    0,                                          // VerticalFrameOriginLayer6

    // DWORD 39
    0,                                          // VerticalFrameOriginLayer7

    // DWORD 40
    0,                                          // HorizontalFrameOriginLayer0

    // DWORD 41
    0,                                          // HorizontalFrameOriginLayer1

    // DWORD 42
    0,                                          // HorizontalFrameOriginLayer2

    // DWORD 43
    0,                                          // HorizontalFrameOriginLayer3

    // DWORD 44
    0,                                          // HorizontalFrameOriginLayer4

    // DWORD 45
    0,                                          // HorizontalFrameOriginLayer5

    // DWORD 46
    0,                                          // HorizontalFrameOriginLayer6

    // DWORD 47
    0,                                          // HorizontalFrameOriginLayer7

    // DWORD 48

    {
       0,                                       //  DestXTopLeftLayer0
       0                                        //  DestYTopLeftLayer0
    },

   // DWORD 49
   {
       0,                                      //  DestXTopLeftLayer1
       0                                       //  DestYTopLeftLayer1
   },

   // DWORD 50
   {
       0,                                      //  DestXTopLeftLayer2
       0                                       //  DestYTopLeftLayer2
   },

   // DWORD 51
   {
       0,                                      //  DestXTopLeftLayer3
       0                                       //  DestYTopLeftLayer3
   },

   // DWORD 52
   {
       0,                                      //  DestXTopLeftLayer4
       0                                       //  DestYTopLeftLayer4
   },

   // DWORD 53
   {
       0,                                      //  DestXTopLeftLayer5
       0                                       //  DestYTopLeftLayer5
   },

   // DWORD 54
   {
       0,                                      //  DestXTopLeftLayer6
       0                                       //  DestYTopLeftLayer6
   },

   // DWORD 55
   {
       0,                                      //  DestXTopLeftLayer7
       0                                       //  DestYTopLeftLayer7
   },

   // DWORD 56
   {
       0,                                      // DestXBottomRightLayer0
       0                                       // DestYBottomRightLayer0
   },

   // DWORD 57
   {
       0,                                      // DestXBottomRightLayer1
       0                                       // DestYBottomRightLayer1
   },

   // DWORD 58
   {
       0,                                      // DestXBottomRightLayer2
       0                                       // DestYBottomRightLayer2
   },

   // DWORD 59
   {
       0,                                      // DestXBottomRightLayer3
       0                                       // DestYBottomRightLayer3
   },

   // DWORD 60
   {
       0,                                      // DestXBottomRightLayer4
       0                                       // DestYBottomRightLayer4
   },

   // DWORD 61
   {
       0,                                      // DestXBottomRightLayer5
       0                                       // DestYBottomRightLayer5
   },

   // DWORD 62
   {
       0,                                     // DestXBottomRightLayer6
       0                                      // DestYBottomRightLayer6
   },

   // DWORD 63
   {
       0,                                     // DestXBottomRightLayer7
       0                                      // DestYBottomRightLayer7
   },

   // DWORD 64
   0,                                         // MainVideoXScalingStepLeft

   // DWORD 65
   0,                                         // VideoStepDeltaForNonLinearRegion

   // DWORD 66
   {
       0,                                     // StartofLinearScalingInPixelPositionC0
       0                                      // StartofRHSNonLinearScalingInPixelPositionC1
   },

   // DWORD 67
   0,                                         // MainVideoXScalingStepCenter

   // DWORD 68
   0,                                         // MainVideoXScalingStepRight

   // DWORD 69
   {
       0,                                     // DestHorizontalBlockOrigin
       0                                      // DestVerticalBlockOrigin
   },

   // DWORD 70 - DWORD 71
   {0,0}
};

static const MEDIA_OBJECT_NLAS_INLINE_DATA g_cInit_MEDIA_OBJECT_NLAS_INLINE_DATA =
{
    0,                                          // HorizontalFrameOriginLayer0
    0,                                          // HorizontalFrameOriginLayer1
    0,                                          // HorizontalFrameOriginLayer2
    0,                                          // HorizontalFrameOriginLayer3
    0,                                          // HorizontalFrameOriginLayer4
    0,                                          // HorizontalFrameOriginLayer5
    0,                                          // HorizontalFrameOriginLayer6
    0                                           // HorizontalFrameOriginLayer7
};

const Kdll_Layer g_cSurfaceType_Layer[] =
{
    Layer_None        ,    //!< SURF_NONE
    Layer_Background  ,    //!< SURF_IN_BACKGROUND
    Layer_MainVideo   ,    //!< SURF_IN_PRIMARY
    Layer_SubVideo    ,    //!< SURF_IN_SECONDARY
    Layer_SubPicture1 ,    //!< SURF_IN_SUBSTREAM
    Layer_Graphics    ,    //!< SURF_IN_GRAPHICS
    Layer_Invalid     ,    //!< SURF_IN_REFERENCE
    Layer_RenderTarget     //!< SURF_OUT_RENDERTARGET
};

const int32_t g_cBindingTableIndex[] =
{
    VPHAL_COMP_BTINDEX_RENDERTARGET,
    VPHAL_COMP_BTINDEX_LAYER0,
    VPHAL_COMP_BTINDEX_LAYER1,
    VPHAL_COMP_BTINDEX_LAYER2,
    VPHAL_COMP_BTINDEX_LAYER3,
    VPHAL_COMP_BTINDEX_LAYER4,
    VPHAL_COMP_BTINDEX_LAYER5,
    VPHAL_COMP_BTINDEX_LAYER6,
    VPHAL_COMP_BTINDEX_LAYER7
};

const RENDERHAL_KERNEL_PARAM g_cInitKernelParamsComposite =
{
    7,                              //!< Number of registers (7 => 128 registers)
    40,                             //!< Number of BT entries
    3,                              //!< Number of samplers (3 => 9-12 samplers)
    VPHAL_USE_MEDIA_THREADS_MAX,    //!< Number of threads
    0,                              //!< Start register
    6,                              //!< Constant URB length (in 256-bits) (6 => 48 dwords)
    VPHAL_COMP_BLOCK_WIDTH,         //!< Block width
    VPHAL_COMP_BLOCK_HEIGHT,        //!< Block height
    1,                              //!< Blocks in x
    1                               //!< Blocks in y
};

//!
//! \brief    Reverse bits in a word
//! \details  Convert a post-rotated 16x16 block mask to a pre-rotated one
//! \param    [in] x
//!           16x16 block mask
//! \return   uint16_t
//!           Return bit-reversed word
//!
static uint16_t ReverseWord(uint16_t x)
{
    x = (((x & 0xaaaa) >> 1) | ((x & 0x5555) << 1));
    x = (((x & 0xcccc) >> 2) | ((x & 0x3333) << 2));
    x = (((x & 0xf0f0) >> 4) | ((x & 0x0f0f) << 4));
    return ((x >> 8) | (x << 8));
}

//!
//! \brief    Judge whether Bob Di should be enabled
//! \details  Judge whether Bob Di should be enabled according to the parameter
//!           of pDeinterlaceParams and the height of the input surface
//! \param    [in] pSrc
//!           Pointer to Source Surface
//! \return   bool
//!           Return true if Bob DI should be enabled, otherwise false
//!
bool CompositeState::IsBobDiEnabled(PVPHAL_SURFACE pSrc)
{
    bool  bRet = false;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(m_pOsInterface);

    // Kernel don't support inderlaced Y410/Y210 as input format
    bRet = (pSrc->pDeinterlaceParams     &&
           (pSrc->Format != Format_Y410  &&
            pSrc->Format != Format_Y210  &&
            pSrc->Format != Format_Y216  &&
            pSrc->Format != Format_Y416) &&
            !VpHal_RndrCommonIsAlignmentWANeeded(pSrc, m_pOsInterface->CurrentGpuContextOrdinal));

finish:
    return bRet;
}

//!
//! \brief    Judge whether 8-tap adaptive filter for all channels should be enabled
//! \details  Judge whether 8-tap adaptive filter for all channels should be enabled according to the input parameter
//! \param    [in] pSrc
//!           Pointer to Source Surface
//! \param    [in] fScaleX
//!           width scaling ratio
//! \param    [in] fScaleY
//!           height scaling ratio
//! \return   bool
//!           Return true 8-tap adaptive filter for all channels should be enabled, otherwise false
//!
bool CompositeState::Is8TapAdaptiveEnabled(
    PVPHAL_SURFACE          pSrc,
    float                   fScaleX,
    float                   fScaleY)
{
    return (m_b8TapAdaptiveEnable       &&
            (fScaleX > 1.0F || fScaleY > 1.0F)    &&
            (IS_RGB32_FORMAT(pSrc->Format)        ||
             pSrc->Format == Format_A16R16G16B16  ||
             pSrc->Format == Format_AYUV          ||
             pSrc->Format == Format_Y410          ||
             pSrc->Format == Format_Y416));
}

//!
//! \brief    Set 16x16 block inline mask based on the rotation
//! \details  Set 16x16 block inline mask with a pre-rotated 16x16 MB based on a
//!           post-rotated 16x16 block mask
//! \param    [in] rotation
//!           Rotation Degrees
//! \param    [out] pInlineDword
//!           Pointer to HW Interface
//! \param    [in] wMask
//!           Inline Mask
//! \param    [in] maskDirection
//!           Mask Direction
//! \return   void
//!
static void SetInline16x16Mask(
    VPHAL_ROTATION                      rotation,
    PVPHAL_16X16BLOCK_COMPOSITE_MASK    pInlineDword,
    uint16_t                            wMask,
    uint32_t                            maskDirection)
{
    if (VPHAL_VERTICAL_16X16BLOCK_MASK == maskDirection)
    {
        switch (rotation)
        {
            case VPHAL_ROTATION_IDENTITY:
            case VPHAL_MIRROR_HORIZONTAL:
                pInlineDword->VerticalBlockCompositeMask = wMask;
                break;
            case VPHAL_ROTATION_90:
            case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
                // swap vertical/horizontal
                pInlineDword->HorizontalBlockCompositeMask = wMask;
                break;
            case VPHAL_ROTATION_180:
            case VPHAL_MIRROR_VERTICAL:
                // reverse bits
                pInlineDword->VerticalBlockCompositeMask = ReverseWord(wMask);
                break;
            case VPHAL_ROTATION_270:
            case VPHAL_ROTATE_90_MIRROR_VERTICAL:
                // reverse bits and swap vertical/horizontal
                pInlineDword->HorizontalBlockCompositeMask = ReverseWord(wMask);
                break;
            default:
                VPHAL_RENDER_ASSERTMESSAGE("Invalid Rotation Angle.");
                break;
        }
    }
    else    // must be VPHAL_HORIZONTAL_16X16BLOCK_MASK
    {
        switch (rotation)
        {
            case VPHAL_ROTATION_IDENTITY:
            case VPHAL_MIRROR_VERTICAL:
                pInlineDword->HorizontalBlockCompositeMask = wMask;
                break;
            case VPHAL_ROTATION_90:
            case VPHAL_ROTATE_90_MIRROR_VERTICAL:
                // reverse bits and swap vertical/horizontal
                pInlineDword->VerticalBlockCompositeMask = ReverseWord(wMask);
                break;
            case VPHAL_ROTATION_180:
            case VPHAL_MIRROR_HORIZONTAL:
                // reverse bits
                pInlineDword->HorizontalBlockCompositeMask = ReverseWord(wMask);
                break;
            case VPHAL_ROTATION_270:
            case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
                // swap vertical/horizontal
                pInlineDword->VerticalBlockCompositeMask = wMask;
                break;
            default:
                VPHAL_RENDER_ASSERTMESSAGE("Invalid Rotation Angle.");
                break;
        }
    }
}

//!
//! \brief    Load Palette Data
//! \details  Load Palette Data according to color space and CSC matrix.
//! \param    [in] pInPalette
//!           Pointer to Input Palette structure
//! \param    [in] srcCspace
//!           Source color space
//! \param    [in] dstCspace
//!           Destination color space
//! \param    [in] piCscMatrix
//!           Pointer to CSC matrix to use in fixed point format
//! \param    [in] iNumEntries
//!           Number of Palette entries to be filled
//! \param    [in,out] pPaletteData
//!           Pointer to Output Palette Address
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS, otherwise MOS_STATUS_UNIMPLEMENTED if Destination Colorspace not supported,
//!            or MOS_STATUS_INVALID_PARAMETER/MOS_STATUS_NULL_POINTER
//!
static MOS_STATUS LoadPaletteData(
    PVPHAL_PALETTE          pInPalette,
    VPHAL_CSPACE            srcCspace,
    VPHAL_CSPACE            dstCspace,
    int32_t*                piCscMatrix,
    int32_t                 iNumEntries,
    void*                   pPaletteData)
{
    PVPHAL_COLOR_SAMPLE_8   pSrcColor, pDstColor;
    bool                    bHasAlpha;
    int32_t                 R, G, B;
    int32_t                 Y, U, V;
    int32_t                 i;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(pInPalette);
    VPHAL_RENDER_CHK_NULL(pInPalette->pPalette8);
    VPHAL_RENDER_CHK_NULL(piCscMatrix);
    VPHAL_RENDER_CHK_NULL(pPaletteData);

    if (pInPalette->iNumEntries < 1)
    {
        VPHAL_RENDER_ASSERTMESSAGE("invalid parameters.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    bHasAlpha = pInPalette->bHasAlpha;

    // Obtain pointer to in/out color entries
    pSrcColor   = pInPalette->pPalette8;
    pDstColor   = (PVPHAL_COLOR_SAMPLE_8)pPaletteData;

    // Load Palette by performing the required conversions
    if (srcCspace == dstCspace)
    {
        // No conversion needed
        if ((dstCspace == CSpace_sRGB) || (dstCspace == CSpace_stRGB))
        {
            for (i = 0; i < iNumEntries; i++, pSrcColor++, pDstColor++)
            {
                pDstColor->A = (bHasAlpha) ? pSrcColor->A : 255;
                pDstColor->R = pSrcColor->R;
                pDstColor->G = pSrcColor->G;
                pDstColor->B = pSrcColor->B;
            }
        }
        else
        {
            for (i = 0; i < iNumEntries; i++, pSrcColor++, pDstColor++)
            {
                pDstColor->a = (bHasAlpha) ? pSrcColor->Alpha : 255;
                pDstColor->V = pSrcColor->Cr;
                pDstColor->Y = pSrcColor->YY;
                pDstColor->U = pSrcColor->Cb;
            }
        }
    }
    else
    {
        // Conversion needed
        switch (dstCspace)
        {
            case CSpace_sRGB:
            case CSpace_stRGB:
                for (i = 0; i < iNumEntries; i++, pSrcColor++, pDstColor++)
                {
                    // YUV to RGB conversions
                    Y = pSrcColor->YY;
                    U = pSrcColor->Cb;
                    V = pSrcColor->Cr;

                    R = (Y * piCscMatrix[0]  + U * piCscMatrix[1]  +
                         V * piCscMatrix[2]  +     piCscMatrix[3]  + 0x00080000) >> 20;
                    G = (Y * piCscMatrix[4]  + U * piCscMatrix[5]  +
                         V * piCscMatrix[6]  +     piCscMatrix[7]  + 0x00080000) >> 20;
                    B = (Y * piCscMatrix[8]  + U * piCscMatrix[9]  +
                         V * piCscMatrix[10] +     piCscMatrix[11] + 0x00080000) >> 20;

                    pDstColor->A = (bHasAlpha) ? pSrcColor->Alpha : 255;
                    if (dstCspace == CSpace_sRGB)
                    {
                        pDstColor->R = MOS_MIN(MOS_MAX(0,R),255);
                        pDstColor->G = MOS_MIN(MOS_MAX(0,G),255);
                        pDstColor->B = MOS_MIN(MOS_MAX(0,B),255);
                    }
                    else
                    {
                        pDstColor->R = MOS_MIN(MOS_MAX(16,R),235);
                        pDstColor->G = MOS_MIN(MOS_MAX(16,G),235);
                        pDstColor->B = MOS_MIN(MOS_MAX(16,B),235);
                    }
                }
                break;

            case CSpace_BT601:
            case CSpace_BT709:
            case CSpace_xvYCC601:
            case CSpace_xvYCC709:
            case CSpace_BT601_FullRange:
            case CSpace_BT709_FullRange:
                for (i = 0; i < iNumEntries; i++, pSrcColor++, pDstColor++)
                {
                    R = pSrcColor->R;
                    G = pSrcColor->G;
                    B = pSrcColor->B;

                    Y = (piCscMatrix[0]  * R + piCscMatrix[1]  * G +
                         piCscMatrix[2]  * B + piCscMatrix[3]      + 0x00080000) >> 20;
                    U = (piCscMatrix[4]  * R + piCscMatrix[5]  * G +
                         piCscMatrix[6]  * B + piCscMatrix[7]      + 0x00080000) >> 20;
                    V = (piCscMatrix[8]  * R + piCscMatrix[9]  * G +
                         piCscMatrix[10] * B + piCscMatrix[11]     + 0x00080000) >> 20;

                    pDstColor->a = (bHasAlpha) ? pSrcColor->Alpha : 255;
                    if ((dstCspace == CSpace_BT601) ||
                        (dstCspace == CSpace_BT709))
                    {
                        pDstColor->V = MOS_MIN(MOS_MAX(16,V),240);
                        pDstColor->Y = MOS_MIN(MOS_MAX(16,Y),235);
                        pDstColor->U = MOS_MIN(MOS_MAX(16,U),240);
                    }
                    else
                    {
                        pDstColor->V = MOS_MIN(MOS_MAX(0,V),255);
                        pDstColor->Y = MOS_MIN(MOS_MAX(0,Y),255);
                        pDstColor->U = MOS_MIN(MOS_MAX(0,U),255);
                    }
                }
                break;

            default:
                VPHAL_RENDER_ASSERTMESSAGE("Destination Colorspace not supported.");
                eStatus = MOS_STATUS_UNIMPLEMENTED;
                break;
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Recalculate Sampler Avs 8x8 Horizontal/Vertical scaling table
//! \param    [in] SrcFormat
//!           Source Format
//! \param    [in] fScale
//!           Horizontal or Vertical Scale Factor
//! \param    [in] bVertical
//!           true if Vertical Scaling, else Horizontal Scaling
//! \param    [in] dwChromaSiting
//!           Chroma Siting
//! \param    [in] bBalancedFilter
//!           true if Gen9+, balanced filter
//! \param    [in] b8TapAdaptiveEnable
//!           true if 8Tap Adaptive Enable
//! \param    [in,out] pAvsParams
//!           Pointer to AVS Params
//! \return   MOS_STATUS
//!
static MOS_STATUS SamplerAvsCalcScalingTable(
    MOS_FORMAT                      SrcFormat,
    float                           fScale,
    bool                            bVertical,
    uint32_t                        dwChromaSiting,
    bool                            bBalancedFilter,
    bool                            b8TapAdaptiveEnable,
    PMHW_AVS_PARAMS                 pAvsParams)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MHW_PLANE                       Plane;
    int32_t                         iUvPhaseOffset;
    uint32_t                        dwHwPhrase;
    uint32_t                        YCoefTableSize;
    uint32_t                        UVCoefTableSize;
    float                           fScaleParam;
    int32_t*                        piYCoefsParam;
    int32_t*                        piUVCoefsParam;
    float                           fHPStrength;

    VPHAL_RENDER_CHK_NULL(pAvsParams);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsX);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsX);

    if (bBalancedFilter)
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;
        dwHwPhrase          = NUM_HW_POLYPHASE_TABLES_G9;
    }
    else
    {
        YCoefTableSize      = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G8;
        UVCoefTableSize     = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G8;
        dwHwPhrase          = MHW_NUM_HW_POLYPHASE_TABLES;
    }

    fHPStrength = 0.0F;
    piYCoefsParam   = bVertical ? pAvsParams->piYCoefsY : pAvsParams->piYCoefsX;
    piUVCoefsParam  = bVertical ? pAvsParams->piUVCoefsY : pAvsParams->piUVCoefsX;
    fScaleParam     = bVertical ? pAvsParams->fScaleY : pAvsParams->fScaleX;

    // Recalculate Horizontal or Vertical scaling table
    if (SrcFormat != pAvsParams->Format || fScale != fScaleParam)
    {
        MOS_ZeroMemory(piYCoefsParam, YCoefTableSize);
        MOS_ZeroMemory(piUVCoefsParam, UVCoefTableSize);

        // 4-tap filtering for RGB format G-channel if 8tap adaptive filter is not enabled.
        Plane = ((IS_RGB32_FORMAT(SrcFormat) || (SrcFormat == Format_Y410) || (SrcFormat == Format_AYUV) || (SrcFormat == Format_Y416)) && !b8TapAdaptiveEnable) ? MHW_U_PLANE : MHW_Y_PLANE;
        if (bVertical)
        {
            pAvsParams->fScaleY = fScale;
        }
        else
        {
            pAvsParams->fScaleX = fScale;
        }

        // For 1x scaling in horizontal direction, use special coefficients for filtering
        // we don't do this when bForcePolyPhaseCoefs flag is set
        if (fScale == 1.0F && !pAvsParams->bForcePolyPhaseCoefs)
        {
            VPHAL_RENDER_CHK_STATUS(Mhw_SetNearestModeTable(
                piYCoefsParam,
                Plane,
                bBalancedFilter));
            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                VPHAL_RENDER_CHK_STATUS(Mhw_SetNearestModeTable(
                    piUVCoefsParam,
                    MHW_U_PLANE,
                    bBalancedFilter));
            }
        }
        else
        {
            // Clamp the Scaling Factor if > 1.0x
            fScale = MOS_MIN(1.0F, fScale);

            VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesY(
                piYCoefsParam,
                fScale,
                Plane,
                SrcFormat,
                fHPStrength,
                true,
                dwHwPhrase));

            // If the 8-tap adaptive is enabled for all channel, then UV/RB use the same coefficient as Y/G
            // So, coefficient for UV/RB channels caculation can be passed
            if (!b8TapAdaptiveEnable)
            {
                if (!bBalancedFilter)
                {
                    VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesY(
                        piUVCoefsParam,
                        fScale,
                        MHW_U_PLANE,
                        SrcFormat,
                        fHPStrength,
                        true,
                        dwHwPhrase));
                }
                else
                {
                    // If Chroma Siting info is present
                    if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_TOP : MHW_CHROMA_SITING_HORZ_LEFT))
                    {
                        // No Chroma Siting
                        VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesUV(
                            piUVCoefsParam,
                            2.0F,
                            fScale));
                    }
                    else
                    {
                        // Chroma siting offset needs to be added
                        if (dwChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_CENTER : MHW_CHROMA_SITING_HORZ_CENTER))
                        {
                            iUvPhaseOffset = MOS_UF_ROUND(0.5F * 16.0F);   // U0.4
                        }
                        else //if (ChromaSiting & (bVertical ? MHW_CHROMA_SITING_VERT_BOTTOM : MHW_CHROMA_SITING_HORZ_RIGHT))
                        {
                            iUvPhaseOffset = MOS_UF_ROUND(1.0F * 16.0F);   // U0.4
                        }

                        VPHAL_RENDER_CHK_STATUS(Mhw_CalcPolyphaseTablesUVOffset(
                            piUVCoefsParam,
                            3.0F,
                            fScale,
                            iUvPhaseOffset));
                    }
                }
            }
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Sampler Avs 8x8 Table
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSamplerStateParams
//!           Pointer to Sampler State Params
//! \param    [in,out] pAvsParams
//!           Pointer to AVS Params
//! \param    [in] SrcFormat
//!           Source Format
//! \param    [in] fScaleX
//!           Horizontal Scale Factor
//! \param    [in] fScaleY
//!           Vertical Scale Factor
//! \param    [in] dwChromaSiting
//!           Chroma Siting
//! \return   MOS_STATUS
//!
MOS_STATUS CompositeState::SetSamplerAvsTableParam(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
    PMHW_AVS_PARAMS                 pAvsParams,
    MOS_FORMAT                      SrcFormat,
    float                           fScaleX,
    float                           fScaleY,
    uint32_t                        dwChromaSiting)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    bool                         bBalancedFilter;
    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam;
    bool                         bIsUpScaleAndYuvFormat;

    VPHAL_RENDER_CHK_NULL(pSamplerStateParams);
    VPHAL_RENDER_CHK_NULL(pAvsParams);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piYCoefsX);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsY);
    VPHAL_RENDER_CHK_NULL(pAvsParams->piUVCoefsX);

    pMhwSamplerAvsTableParam = pSamplerStateParams->Avs.pMhwSamplerAvsTableParam;

    pMhwSamplerAvsTableParam->bIsCoeffExtraEnabled = m_bAvsTableCoeffExtraEnabled;
    pMhwSamplerAvsTableParam->b8TapAdaptiveEnable  = pSamplerStateParams->Avs.b8TapAdaptiveEnable;
    bBalancedFilter                                = m_bAvsTableBalancedFilter;

    pMhwSamplerAvsTableParam->byteTransitionArea8Pixels = MEDIASTATE_AVS_TRANSITION_AREA_8_PIXELS;
    pMhwSamplerAvsTableParam->byteTransitionArea4Pixels = MEDIASTATE_AVS_TRANSITION_AREA_4_PIXELS;
    pMhwSamplerAvsTableParam->byteMaxDerivative8Pixels  = MEDIASTATE_AVS_MAX_DERIVATIVE_8_PIXELS;
    pMhwSamplerAvsTableParam->byteMaxDerivative4Pixels  = MEDIASTATE_AVS_MAX_DERIVATIVE_4_PIXELS;
    pMhwSamplerAvsTableParam->byteDefaultSharpnessLevel = MEDIASTATE_AVS_SHARPNESS_LEVEL_SHARP;

    bIsUpScaleAndYuvFormat = ((fScaleX > 1.0F || fScaleY > 1.0F) && IS_YUV_FORMAT(SrcFormat));
    if (SrcFormat == Format_Y410 ||
        SrcFormat == Format_AYUV ||
        SrcFormat == Format_Y416)
    {
        bIsUpScaleAndYuvFormat = false;
    }

    if (pMhwSamplerAvsTableParam->b8TapAdaptiveEnable)
    {
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering  = false;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering  = false;
        pMhwSamplerAvsTableParam->bAdaptiveFilterAllChannels = true;
        pMhwSamplerAvsTableParam->bEnableRGBAdaptive         = IS_RGB_FORMAT(SrcFormat);
    }
    else if (bIsUpScaleAndYuvFormat)
    {
        // enable adaptive filter if it's being upscaled in either direction. we check it before clamping the SF.
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering = false;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering = false;
    }
    else
    {
        pMhwSamplerAvsTableParam->bBypassXAdaptiveFiltering = true;
        pMhwSamplerAvsTableParam->bBypassYAdaptiveFiltering = true;
    }

    // No changes to AVS parameters -> skip
    if (SrcFormat == pAvsParams->Format &&
        fScaleX == pAvsParams->fScaleX &&
        fScaleY == pAvsParams->fScaleY)
    {
        goto finish;
    }

    // not change AVS coefficients if upscaling, to avoid recalculation
    if (fScaleX > 1.0F && pAvsParams->fScaleX > 1.0F)
    {
        pAvsParams->fScaleX = fScaleX;
    }

    // not change AVS coefficients if upscaling, to avoid recalculation
    if (fScaleY > 1.0F && pAvsParams->fScaleY > 1.0F)
    {
        pAvsParams->fScaleY = fScaleY;
    }

    AvsCoeffsCacheTag tag;
    tag.m_format              = SrcFormat;
    tag.m_8TapAdaptiveEnable  = pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false;
    tag.m_balancedFilter      = bBalancedFilter;
    tag.m_forcePolyPhaseCoefs = pAvsParams->bForcePolyPhaseCoefs ? true : false;
    tag.m_chromaSiting        = dwChromaSiting;
    tag.m_scaleX              = fScaleX;
    tag.m_scaleY              = fScaleY;

    const MHW_AVS_PARAMS *cachedAvsParams;
    cachedAvsParams = m_AvsCoeffsCache.Find(tag);

    if (cachedAvsParams)
    {
        m_AvsCoeffsCache.Clone(*cachedAvsParams, *pAvsParams);
    }
    else
    {
        // Recalculate Horizontal scaling table
        VPHAL_RENDER_CHK_STATUS(SamplerAvsCalcScalingTable(
            SrcFormat,
            fScaleX,
            false,
            dwChromaSiting,
            bBalancedFilter,
            pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
            pAvsParams));

        // Recalculate Vertical scaling table
        VPHAL_RENDER_CHK_STATUS(SamplerAvsCalcScalingTable(
            SrcFormat,
            fScaleY,
            true,
            dwChromaSiting,
            bBalancedFilter,
            pMhwSamplerAvsTableParam->b8TapAdaptiveEnable ? true : false,
            pAvsParams));

        // Save format used to calculate AVS parameters
        pAvsParams->Format = SrcFormat;

        m_AvsCoeffsCache.Insert(tag, *pAvsParams);
    }

    pMhwSamplerAvsTableParam->b4TapGY   = ((IS_RGB32_FORMAT(SrcFormat) || SrcFormat == Format_Y410 || SrcFormat == Format_AYUV || SrcFormat == Format_Y416) && !pMhwSamplerAvsTableParam->b8TapAdaptiveEnable);
    pMhwSamplerAvsTableParam->b4TapRBUV = (!pMhwSamplerAvsTableParam->b8TapAdaptiveEnable);

    VPHAL_RENDER_CHK_STATUS(VpHal_RenderCommonSetAVSTableParam(pAvsParams, pMhwSamplerAvsTableParam));

finish:
    return eStatus;
}

//!
//! \brief    Prepare phases for composite and determine intermediate colorspace
//! \param    [in] pcRenderParams
//!           Pointer to Render parameters
//! \param    [in] ppSources
//!           Pointer to the address of Source Surfaces
//! \param    [in] iSources
//!           Count of Source Surfaces
//! \return   VPHAL_CSPACE
//!           Return intermediate colorspace
//!
VPHAL_CSPACE CompositeState::PrepareCSC(
    PCVPHAL_RENDER_PARAMS   pcRenderParams,
    PVPHAL_SURFACE          *ppSources,
    int32_t                 iSources)
{
    PVPHAL_SURFACE                  pTarget;
    PVPHAL_SURFACE                  pSrc;
    int32_t                         i, j;
    int32_t                         csc_count = 0;
    int32_t                         csc_min = iSources + 1;
    int32_t                         cspace_in_use[CSpace_Count];
    bool                            bYUVTarget;
    VPHAL_CSPACE                    cs;
    VPHAL_CSPACE                    Temp_ColorSpace = CSpace_Any;
    VPHAL_CSPACE                    Main_ColorSpace = CSpace_None;

    // Check if target is YUV
    pTarget    = pcRenderParams->pTarget[0];
    bYUVTarget = IS_RGB_FORMAT(pTarget->Format) ? false : true;

    // Gets primary video cspace
    // Implements xvYCC passthrough mode
    // Set Color Spaces in use
    MOS_ZeroMemory(cspace_in_use, sizeof(cspace_in_use));
    for (i = 0; i < iSources; i++)
    {
        // Get current source
        pSrc = ppSources[i];

        // Save Main Video color space
        if (pSrc->SurfType == SURF_IN_PRIMARY &&
            Main_ColorSpace == CSpace_None)
        {
            Main_ColorSpace = pSrc->ColorSpace;
        }

        // Set xvYCC pass through mode
        if (bYUVTarget &&
            (pSrc->ColorSpace == CSpace_xvYCC709 ||
             pSrc->ColorSpace == CSpace_xvYCC601))
        {
            Temp_ColorSpace = pSrc->ColorSpace;
            goto finish;
        }

        // Don't take PAL formats into consideration
        if ((!IS_PAL_FORMAT(pSrc->Format)) &&
             pSrc->ColorSpace > CSpace_Any &&
             pSrc->ColorSpace < CSpace_Count)
        {
            cs = KernelDll_TranslateCspace(pSrc->ColorSpace);
            if (cs >= CSpace_Any)
            {
                cspace_in_use[cs]++;
            }
        }
    }

    // For every CS in use, iterate through source CS and keep a
    // count of number of CSC operation needed. Determine the Temporary
    // color space as the one requiring min. # of CSC ops.
    for (j = (CSpace_Any + 1); j < CSpace_Count; j++)
    {
        // Skip color spaces not in use
        if (!cspace_in_use[j])
        {
            continue;
        }

        // Count # of CS conversions
        cs = (VPHAL_CSPACE) j;
        csc_count = 0;
        for (i = 0; i < iSources; i++)
        {
            // Get current source
            pSrc = ppSources[i];

            // Ignore palletized layers
            if (IS_PAL_FORMAT(pSrc->Format) ||
                pSrc->ColorSpace == CSpace_Any)
            {
                continue;
            }

            // Check if CSC/PA is required
            if (KernelDll_TranslateCspace(pSrc->ColorSpace) != cs ||
                (pSrc->pProcampParams != nullptr &&
                 pSrc->pProcampParams->bEnabled))
            {
                csc_count++;
            }
        }

        // Save best choice as requiring minimum number of CSC operations
        // Use main cspace as default if same CSC count
        if ((csc_count <  csc_min) ||
            (csc_count == csc_min && cs == Main_ColorSpace) )
        {
            Temp_ColorSpace = cs;
            csc_min = csc_count;
        }
    }

    // If all layers are palletized, use the CS from first layer (as good as any other)
    if (Temp_ColorSpace == CSpace_Any && iSources > 0)
    {
        Temp_ColorSpace = ppSources[0]->ColorSpace;
    }

finish:

    VPHAL_RENDER_NORMALMESSAGE("Main_ColorSpace %d, Temp_ColorSpace %d, csc_count %d.",
        Main_ColorSpace,
        Temp_ColorSpace,
        csc_count);

    return Temp_ColorSpace;
}

//!
//! \brief    Prepare phases for composite and allocate intermediate buffer for rendering
//! \param    [in] pcRenderParams
//!           Pointer to Render parameters
//! \param    [in] ppSources
//!           Pointer to the address of Source Surfaces
//! \param    [in] iSources
//!           Count of Source Surfaces
//! \return   bool
//!           Return true if multiple phases, otherwise false
//!
bool CompositeState::PreparePhases(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    PVPHAL_SURFACE              *ppSources,
    int32_t                     iSources)
{
    PMOS_INTERFACE          pOsInterface;
    VPHAL_COMPOSITE_PARAMS  Composite;
    MOS_RESOURCE            OsResource;
    uint32_t                dwTempWidth;    // Temporary surface width
    uint32_t                dwTempHeight;   // Temporary surface height
    PVPHAL_SURFACE          pTarget;
    PVPHAL_SURFACE          pIntermediate;
    int32_t                 i;
    bool                    bMultiplePhases;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    VPHAL_GET_SURFACE_INFO  Info;

    pTarget = pcRenderParams->pTarget[0];

    // Constriction support
    dwTempWidth = dwTempHeight = 0;
    if (pcRenderParams->pConstriction)
    {
        // Force multiple phases
        bMultiplePhases = true;

        // Temporary surface size = constriction rectangle
        dwTempWidth  = pcRenderParams->pConstriction->right;
        dwTempHeight = pcRenderParams->pConstriction->bottom;
    }
    else
    {
        // Reset multiple phase support
        bMultiplePhases = false;

        // Temporary surface has the same size as render target
        dwTempWidth  = pTarget->dwWidth;
        dwTempHeight = pTarget->dwHeight;

        // Check if multiple phases by building filter for first phase
        ResetCompParams(&Composite);
        for (i = 0; i < iSources; i++)
        {
            if (!AddCompLayer(&Composite, ppSources[i]))
            {
                bMultiplePhases = true;
                break;
            }
        }

        // Add render target
        if (!AddCompTarget(&Composite, pTarget))
        {
            bMultiplePhases = true;
        }
    }

    // Reallocate Intermediate surface
    if (bMultiplePhases)
    {
        pOsInterface  = m_pOsInterface;
        pIntermediate = &m_Intermediate;

        // Allocate/Reallocate temporary output
        if (dwTempWidth  > pIntermediate->dwWidth ||
            dwTempHeight > pIntermediate->dwHeight)
        {
            // Get max values
            dwTempWidth  = MOS_MAX(dwTempWidth , pIntermediate->dwWidth);
            dwTempHeight = MOS_MAX(dwTempHeight, pIntermediate->dwHeight);

            // Allocate buffer in fixed increments
            dwTempWidth  = MOS_ALIGN_CEIL(dwTempWidth , VPHAL_BUFFER_SIZE_INCREMENT);
            dwTempHeight = MOS_ALIGN_CEIL(dwTempHeight, VPHAL_BUFFER_SIZE_INCREMENT);

            MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            MOS_ZeroMemory(&OsResource, sizeof(MOS_RESOURCE));

            AllocParams.Type     = MOS_GFXRES_2D;
            AllocParams.TileType = MOS_TILE_Y;
            AllocParams.dwWidth  = dwTempWidth;
            AllocParams.dwHeight = dwTempHeight;
            AllocParams.Format   = Format_A8R8G8B8;

            pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParams,
                &OsResource);

            // Get Allocation index of source for rendering
            pOsInterface->pfnRegisterResource(
                pOsInterface,
                &OsResource,
                false,
                true);

            if (!Mos_ResourceIsNull(&OsResource))
            {
                // Deallocate old resource
                pOsInterface->pfnFreeResource(pOsInterface,
                                              &pIntermediate->OsResource);

                // Set new resource
                pIntermediate->OsResource = OsResource;

                // Get resource info (width, height, pitch, tiling, etc)
                MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

                VpHal_GetSurfaceInfo(
                    pOsInterface,
                    &Info,
                    pIntermediate);
            }
        }

        // Set output parameters
        pIntermediate->SurfType          = SURF_IN_PRIMARY;
        pIntermediate->SampleType        = SAMPLE_PROGRESSIVE;
        pIntermediate->ColorSpace        = pTarget->ColorSpace;
        pIntermediate->ExtendedGamut     = pTarget->ExtendedGamut;
        pIntermediate->rcSrc             = pTarget->rcSrc;
        pIntermediate->rcDst             = pTarget->rcDst;
        pIntermediate->ScalingMode       = VPHAL_SCALING_BILINEAR;
        pIntermediate->bIEF              = false;

        pIntermediate = &m_Intermediate2;

        // Allocate/Reallocate temporary output
        if (dwTempWidth  > pIntermediate->dwWidth ||
            dwTempHeight > pIntermediate->dwHeight)
        {
            // Get max values
            dwTempWidth  = MOS_MAX(dwTempWidth , pIntermediate->dwWidth);
            dwTempHeight = MOS_MAX(dwTempHeight, pIntermediate->dwHeight);

            // Allocate buffer in fixed increments
            dwTempWidth  = MOS_ALIGN_CEIL(dwTempWidth , VPHAL_BUFFER_SIZE_INCREMENT);
            dwTempHeight = MOS_ALIGN_CEIL(dwTempHeight, VPHAL_BUFFER_SIZE_INCREMENT);

            MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

            AllocParams.Type     = MOS_GFXRES_2D;
            AllocParams.TileType = MOS_TILE_Y;
            AllocParams.dwWidth  = dwTempWidth;
            AllocParams.dwHeight = dwTempHeight;
            AllocParams.Format   = Format_A8R8G8B8;

            pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParams,
                &OsResource);

            if (!Mos_ResourceIsNull(&OsResource))
            {
                // Deallocate old resource
                pOsInterface->pfnFreeResource(pOsInterface,
                                              &pIntermediate->OsResource);

                // Set new resource
                pIntermediate->OsResource = OsResource;

                // Get resource info (width, height, pitch, tiling, etc)
                MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

                VpHal_GetSurfaceInfo(
                    pOsInterface,
                    &Info,
                    pIntermediate);
            }
        }

        // Set output parameters
        pIntermediate->SurfType          = SURF_IN_PRIMARY;
        pIntermediate->SampleType        = SAMPLE_PROGRESSIVE;
        pIntermediate->ColorSpace        = pTarget->ColorSpace;
        pIntermediate->ExtendedGamut     = pTarget->ExtendedGamut;
        pIntermediate->rcSrc             = pTarget->rcSrc;
        pIntermediate->rcDst             = pTarget->rcDst;
        pIntermediate->ScalingMode       = VPHAL_SCALING_BILINEAR;
        pIntermediate->bIEF              = false;
    }

    return bMultiplePhases;
}

//!
//! \brief    Reset composite rendering parameters for the current phase
//! \param    [in,out] pComposite
//!           Pointer to Composite parameters
//! \return   void
//!
void CompositeState::ResetCompParams(
    PVPHAL_COMPOSITE_PARAMS     pComposite)
{
    MOS_ZeroMemory(pComposite, sizeof(*pComposite));

    pComposite->nLayers   = VPHAL_COMP_MAX_LAYERS;
    pComposite->nPalettes = VPHAL_COMP_MAX_PALETTES;
    pComposite->nProcamp  = VPHAL_COMP_MAX_PROCAMP;
    pComposite->nLumaKeys = VPHAL_COMP_MAX_LUMA_KEY;
    pComposite->nAVS      = VPHAL_COMP_MAX_AVS;
    pComposite->nSampler  = VPHAL_COMP_MAX_SAMPLER;

    // reset render target count to 1
    pComposite->uTargetCount = 1;

    pComposite->bAlphaCalculateEnable = false;
}

//!
//! \brief    Adds a source layer for composite
//! \param    [in,out] pComposite
//!           Pointer to Composite parameters
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \return   bool
//!           Return TURE if source may be processed in the same phase, otherwise false
//!
bool CompositeState::AddCompLayer(
    PVPHAL_COMPOSITE_PARAMS     pComposite,
    PVPHAL_SURFACE              pSource)
{
    bool                bResult;
    PVPHAL_SURFACE      pPrevSource;
    bool                bSinglePhaseRotate;
    VPHAL_SCALING_MODE  scalingMode;

    bResult            = false;
    pPrevSource        = nullptr;
    bSinglePhaseRotate = false;

    if (pComposite == nullptr || pSource == nullptr)
    {
        goto finish;
    }

    scalingMode           = pSource->ScalingMode;

    // On Gen9+, Rotation is done in sampler. Multiple phases are not required.
    if (!m_bSamplerSupportRotation)
    {
        if (pComposite->uSourceCount == 0)
        {
            // Set Layer 0 rotation info
            pComposite->Rotation = pSource->Rotation;
            bSinglePhaseRotate = true;
        }
        else if (pComposite->uSourceCount == 1)
        {
            // Single phase if: L0 (angle) + L1 (no rotation) OR L1 angle == L0 angle
            bSinglePhaseRotate = (pSource->Rotation == VPHAL_ROTATION_IDENTITY ||
                                  pSource->Rotation == pComposite->Rotation) ? true : false;
        }
        else
        {
            // Get pointer to previous source
            pPrevSource = pComposite->pSource[pComposite->uSourceCount - 1];

            // Single phase if:L2 angle == L1 angle
            bSinglePhaseRotate = (pSource->Rotation == pPrevSource->Rotation) ? true : false;
        }
    }
    else
    {
        bSinglePhaseRotate = true;
    }

    // Number of layers
    pComposite->nLayers--;

    // Number of palettes
    if (pSource->Palette.PaletteType != VPHAL_PALETTE_NONE)
    {
        pComposite->nPalettes--;
    }

    // Number of procamp parameters
    if (pSource->pProcampParams)
    {
        pComposite->nProcamp--;
    }

    // Number of luma keys
    if (pSource->pLumaKeyParams)
    {
        pComposite->nLumaKeys--;
        if (pComposite->nLumaKeys < 0 || pComposite->uSourceCount > 1)
        {
            bResult = false;
            goto finish;
        }
        if (pComposite->uSourceCount == 1)
        {
            // This layer requires 3d sampler to perform luma key.
            // So set previous layer's scaling mode to AVS and reset the nSampler.
            // Disable AVS scaling mode in cases AVS is not available
            if (pComposite->pSource[0]->ScalingMode != VPHAL_SCALING_AVS && !m_need3DSampler)
            {
                pComposite->pSource[0]->ScalingMode = VPHAL_SCALING_AVS;
                pComposite->nAVS--;
            }
            pComposite->nSampler = VPHAL_COMP_MAX_SAMPLER;
        }
    }

    // Number of AVS, but lumaKey and BOB DI needs 3D sampler instead of AVS sampler.
    if (pSource->ScalingMode == VPHAL_SCALING_AVS  && !pSource->pLumaKeyParams && !IsBobDiEnabled(pSource))
    {
        pComposite->nAVS--;
    }
    // Number of Sampler filter mode, we had better only support Nearest or Bilinear filter in one phase
    // If two filters are used together, the later filter overwrite the first and cause output quality issue.
    else if ((pSource->rcDst.right - pSource->rcDst.left) == (pSource->rcSrc.right - pSource->rcSrc.left) &&
             (pSource->rcDst.bottom - pSource->rcDst.top) == (pSource->rcSrc.bottom - pSource->rcSrc.top) &&
             !IS_PL3_FORMAT(pSource->Format))
    {
        // Use sampler luma key feature only if this is not the bottom most layer
        if (pSource->pLumaKeyParams && pComposite->uSourceCount)
        {
            scalingMode           = VPHAL_SCALING_NEAREST;
            pComposite->nSampler &= VPHAL_COMP_SAMPLER_LUMAKEY;
        }
        else if (pComposite->nSampler & VPHAL_COMP_SAMPLER_NEAREST)
        {
            scalingMode           = VPHAL_SCALING_NEAREST;
            pComposite->nSampler &= VPHAL_COMP_SAMPLER_NEAREST;
        }
        else
        {
            // switch to AVS if AVS sampler is not used, decrease the count of comp phase
            scalingMode = VPHAL_SCALING_AVS;
            pComposite->nAVS--;
        }
    }
    else if (!IS_PL3_FORMAT(pSource->Format))
    {
        // Use sampler luma key feature only if this is not the bottom most layer
        if (pSource->pLumaKeyParams && pComposite->uSourceCount)
        {
            scalingMode           = VPHAL_SCALING_BILINEAR;
            pComposite->nSampler &= VPHAL_COMP_SAMPLER_LUMAKEY;
        }
        else if (pComposite->nSampler & VPHAL_COMP_SAMPLER_BILINEAR)
        {
            scalingMode           = VPHAL_SCALING_BILINEAR;
            pComposite->nSampler &= VPHAL_COMP_SAMPLER_BILINEAR;
        }
        else
        {
            // switch to AVS if AVS sampler is not used, decrease the count of comp phase
            scalingMode = VPHAL_SCALING_AVS;
            pComposite->nAVS--;
        }
    }

    // Fails if any of the limits are reached
    // Output structure has reason why failed :-)
    // multi-passes if rotation is not the same as Layer 0 rotation
    // single pass if Primary layer needs rotation and remaining layer does not need rotation
    if (pComposite->nLayers   < 0 ||
        pComposite->nPalettes < 0 ||
        pComposite->nProcamp  < 0 ||
        pComposite->nLumaKeys < 0 ||
        pComposite->nAVS      < 0 ||
        pComposite->nSampler == 0 ||
        bSinglePhaseRotate   == false)
    {
        //Multipass
        goto finish;
    }

    // Append source to compositing operation
    pSource->ScalingMode = scalingMode;
    pComposite->pSource[pComposite->uSourceCount] = pSource;
    pComposite->uSourceCount++;
    bResult = true;

    VPHAL_RENDER_NORMALMESSAGE("ScalingMode %d, nSampler %d", pSource->ScalingMode, pComposite->nSampler);

finish:
    return bResult;
}

//!
//! \brief    Adds render target layer for composite
//! \param    [in,out] pComposite
//!           Pointer to Composite parameters
//! \param    [in] pTarget
//!           Pointer to target surface
//! \return   bool
//!           Return TURE if target may be processed in the same phase, otherwise false
//!
bool CompositeState::AddCompTarget(
    PVPHAL_COMPOSITE_PARAMS     pComposite,
    PVPHAL_SURFACE              pTarget)
{
    bool    bResult;

    // Set render target
    pComposite->Target[0] = *pTarget;

    // Number of procamp parameters
    if (pTarget->pProcampParams)
    {
        pComposite->nProcamp--;
    }

    // Fails if max procamp already reached
    // Procamp needs to be performed in a different phase
    if (pComposite->nProcamp  < 0)
    {
        bResult = false;
        pComposite->Target[0].pProcampParams = nullptr;
    }
    else
    {
        bResult = true;
    }

    return bResult;
}

//!
//! \brief    Composite multiple phase rendering
//! \details  Composite render with multiple phases. In some cases we cannot process composition just in one phase
//!           for example, if the input streams count is 9 (1 primary + 8 substreams), we need to postpone the
//!           9th stream to next second phase due to the input count limitation of current composition kernel.
//! \param    [in] pcRenderParams
//!           Pointer to VPHAL_RENDER_PARAMS
//! \param    [in] ppSources
//!           Pointer to PVPHAL_SURFACE, array of input surfaces
//! \param    [in] iSources
//!           constant int iSource indicating the size of ppSources
//! \param    [in] pOutput
//!           Pointer to VPHAL_SURFACE, output surface for the overall composition process
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeState::RenderMultiPhase(
    PCVPHAL_RENDER_PARAMS   pcRenderParams,
    PVPHAL_SURFACE          *ppSources,
    const int32_t           iSources,
    PVPHAL_SURFACE          pOutput)
{
    MOS_STATUS           eStatus;
    int32_t              phase;                         // Current phase
    int32_t              i;                             // Auxiliary integer
    uint32_t             index;                         // Current source index
    PVPHAL_SURFACE       pSrc                = nullptr; // Current source surface
    PRENDERHAL_INTERFACE pRenderHal          = m_pRenderHal;
    PMOS_INTERFACE       pOsInterface        = m_pOsInterface;
    bool                 bLastPhase          = false;
    bool                 bExtraRotationPhase = false;

    // Perf related
    bool                 bPrimary, bRotation;
    VPHAL_PERFTAG        PerfTag;

    for (index = 0, phase = 0; (!bLastPhase); phase++)
    {
        VPHAL_COMPOSITE_PARAMS  CompositeParams;
        // Prepare compositing structure
        ResetCompParams(&CompositeParams);

//        VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_PHASE(phase);

        //Set the alpha for composited surface
        CompositeParams.pCompAlpha = pcRenderParams->pCompAlpha;

        // First phase - set colorfill + render all blocks
        if (phase == 0)
        {
            CompositeParams.pColorFillParams = pcRenderParams->pColorFillParams;
            CompositeParams.bSkipBlocks      = false;
        }
        else
        // Other phases - skip blocks by default (modified if RT has Procamp)
        {
            CompositeParams.bSkipBlocks      = true;
        }

        // Constricted output - affects coordinate calculations
        CompositeParams.pConstriction = pcRenderParams->pConstriction;

        // Validate samples and number of samples for each layer
        bLastPhase = true;
        bPrimary   = false;
        bRotation  = false;
        for (i = 0; index < (uint32_t)iSources || bExtraRotationPhase; i++)
        {
            // Set previous output as the first layer
            if (i == 0 && phase != 0)
            {
                // Optimization for 2 layers composition (with sub-layer rotation) usage case , in the second phase, the first layer should be layer0
                if (!m_bApplyTwoLayersCompOptimize || (iSources != 2) || (ppSources[1]->Rotation == VPHAL_ROTATION_IDENTITY))
                {
                    pSrc = pOutput;
                }
                else
                {
                    CompositeParams.pColorFillParams = pcRenderParams->pColorFillParams;
                    CompositeParams.bSkipBlocks      = false;
                    pSrc = ppSources[0];
                }
            }
            else if (i == 1 && bExtraRotationPhase)
            {
                // bExtraRotationPhase == true means that Intermediate2 was used as a
                // temp output resource in the previous phase and now is to be
                // used as an input
                pSrc = &m_Intermediate2;
                pSrc->SurfType = SURF_IN_SUBSTREAM; // set the surface type to substream
                bExtraRotationPhase = false;        // reset rotation phase
            }
            else
            {
                pSrc = ppSources[index];
                index++;
            }

            // Set if primary is present
            if (pSrc->SurfType == SURF_IN_PRIMARY)
            {
                bPrimary = true;
            }

            // Add layer to the compositing - breaks at end of phase
            pSrc->iLayerID = i;

            if (!AddCompLayer(&CompositeParams, pSrc))
            {
                bLastPhase = false;
                index--;
                break;
            }

            if (pSrc->Rotation != VPHAL_ROTATION_IDENTITY)
            {
                bRotation = true;
            }
        }

        // Setup render target
        bLastPhase &= AddCompTarget(&CompositeParams, pOutput);

        // Last phase
        if (bLastPhase && pcRenderParams->pConstriction == nullptr)
        {
            // Set the actual render target(s), process all blocks
            i = 0;
            do
            {
                CompositeParams.Target[i] = *pcRenderParams->pTarget[i];
                i++;
                CompositeParams.uTargetCount  = i;
            } while (i < (int32_t)pcRenderParams->uDstCount);

            CompositeParams.bSkipBlocks = false;

            // Force the output rectangles to be the final render target rectangles.
            // pTarget could be used as an input as well because it could be used
            // as a temp output in the previous render phase.
            CompositeParams.Target[0].rcSrc = pcRenderParams->pTarget[0]->rcSrc;
            CompositeParams.Target[0].rcDst = pcRenderParams->pTarget[0]->rcDst;
        }

        // Force output as "render target" surface type
        CompositeParams.Target[0].SurfType = SURF_OUT_RENDERTARGET;

        // Reset states before rendering (clear allocations, get GSH allocation index
        //                                + any additional housekeeping)
        pOsInterface->pfnResetOsStates(pOsInterface);
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

        // Set Slice Shutdown Mode
        if (m_bSingleSlice)
        {
            pRenderHal->pfnSetSliceShutdownMode(pRenderHal, true);
        }

        // Set performance tag for current phase
        // Set rotation perftag if there is a layer that needs to be rotated in
        // the current phase, regardless of primary or non-primary.
        if (bRotation)
        {
            PerfTag = (VPHAL_PERFTAG)((int)VPHAL_ROT + i - 1);
        }
        else if (bPrimary)
        {
            PerfTag = (VPHAL_PERFTAG)((int)VPHAL_PRI + i - 1);
        }
        else
        {
            PerfTag = (VPHAL_PERFTAG)((int)VPHAL_NONE + i);
        }
        pOsInterface->pfnSetPerfTag(pOsInterface, PerfTag);

        // Flag to indicate to do the alpha calculate
        CompositeParams.bAlphaCalculateEnable = pcRenderParams->bCalculatingAlpha;

        // Perform compositing operation
        // Optimization for 2 layers composition (with sub-layer rotation) usage case , skip the first phase composition (layer0 -> RT)
        if (!m_bApplyTwoLayersCompOptimize || bLastPhase || (iSources != 2) || (ppSources[1]->Rotation == VPHAL_ROTATION_IDENTITY))
        {
            m_bLastPhase = bLastPhase && (pcRenderParams->pConstriction == nullptr);
            VPHAL_RENDER_CHK_STATUS(RenderPhase(&CompositeParams));
        }

        // do an extra rotation if needed
        bExtraRotationPhase = false;

        // If rotation is done in sampler. Multiple phases are not required.
        if (!m_bSamplerSupportRotation)
        {
            if ((!bLastPhase) && pSrc && (pSrc->Rotation != VPHAL_ROTATION_IDENTITY))
            {
                bExtraRotationPhase = true;

                // Prepare compositing structure
                ResetCompParams(&CompositeParams);

                // Optimization for 2 layers composition (with sub-layer rotation) usage case, This is the first phase
                if ((iSources != 2) || (ppSources[1]->Rotation == VPHAL_ROTATION_IDENTITY))
                {
                    CompositeParams.bSkipBlocks   = true;
                }
                else
                {
                    // set colorfill + render all blocks
                    CompositeParams.pColorFillParams = pcRenderParams->pColorFillParams;
                    CompositeParams.bSkipBlocks      = false;
                }

                // Set the alpha for composited surface
                CompositeParams.pCompAlpha = pcRenderParams->pCompAlpha;

                // When building filter description, if the first layer is translucent,
                // colorfill will always be used. But in this rotation phase, if the
                // src rectangle can cover dest, there is no need to do colorfill.
                // Force skip it here to avoid performance drop.
                CompositeParams.bForceSkipColorFill = true;

                CompositeParams.pConstriction   = nullptr;
                // process the next sample, cannot group more samples here because
                // the temporary output will be the input of next phase and we need
                // new kernel to deal with the transparent area of the bigger rectangle
                // of the temporary output.
                pSrc = ppSources[index];
                index++;
                pSrc->iLayerID = 0;
                AddCompLayer(&CompositeParams, pSrc);

                // using pTarget as a temp resource
                if (pSrc->pBlendingParams)
                {
                    if (m_Intermediate2.pBlendingParams == nullptr)
                    {
                        m_Intermediate2.pBlendingParams = (PVPHAL_BLENDING_PARAMS)
                            MOS_AllocAndZeroMemory(sizeof(VPHAL_BLENDING_PARAMS));
                    }
                    if (m_Intermediate2.pBlendingParams)
                    {
                        m_Intermediate2.pBlendingParams->BlendType = pSrc->pBlendingParams->BlendType;
                        m_Intermediate2.pBlendingParams->fAlpha = pSrc->pBlendingParams->fAlpha;
                    }
                }
                else
                {
                    // clear the blending params if it was set from the previous phase
                    if (m_Intermediate2.pBlendingParams)
                    {
                        MOS_FreeMemory(m_Intermediate2.pBlendingParams);
                        m_Intermediate2.pBlendingParams = nullptr;
                    }
                }

                // update the output rectangles with the input rectangles
                m_Intermediate2.rcDst = m_Intermediate2.rcSrc = pSrc->rcDst;

                AddCompTarget(&CompositeParams, &m_Intermediate2);
                // Force output as "render target" surface type
                CompositeParams.Target[0].SurfType = SURF_OUT_RENDERTARGET;

                // Reset states before rendering (clear allocations, get GSH allocation index
                //                                + any additional housekeeping)
                pOsInterface->pfnResetOsStates(pOsInterface);
                VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

                // Set performance tag for current rotation phase
                PerfTag = (VPHAL_PERFTAG)((int)VPHAL_ROT);
                pOsInterface->pfnSetPerfTag(pOsInterface, PerfTag);

                // Perform compositing operation
                VPHAL_RENDER_CHK_STATUS(RenderPhase(&CompositeParams));
            }
        }
    }
    eStatus = MOS_STATUS_SUCCESS;
finish:
    return eStatus;
}

//!
//! \brief    Composite Rendering
//! \details  VPHal Composite Render entry, this render handles Procamp/CSC/ColorFill/
//!           Scaling/BOBDI
//! \param    [in,out] pcRenderParams
//!           Pointer to Render parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeState::Render(
    PCVPHAL_RENDER_PARAMS  pcRenderParams,
    RenderpassData         *pRenderPassData)
{
    MOS_STATUS              eStatus;

    PRENDERHAL_INTERFACE    pRenderHal = nullptr;
    PMOS_INTERFACE          pOsInterface;

    // Sorted sources for compositing
    int32_t                 iProcamp;                    // Procamp Index
    int32_t                 iSources;                    // Number of sources
    PVPHAL_SURFACE          pSources[VPHAL_MAX_SOURCES]; // Array of sources

    Kdll_Procamp            Procamp;
    Kdll_Procamp            *pProcamp;
    PVPHAL_PROCAMP_PARAMS   pProcampParams;

    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings = nullptr;

    VPHAL_CSPACE            ColorSpace;     // Temporary colorspace
    PVPHAL_SURFACE          pTarget;        // Render target
    PVPHAL_SURFACE          pOutput;        // Compositing output
    int32_t                 index;          // Current source index
    bool                    bMultiplePhases;
    PVPHAL_RNDR_PERF_DATA   pPerfData;

    eStatus = MOS_STATUS_UNKNOWN;

    VPHAL_RENDER_FUNCTION_ENTER;

    VPHAL_RENDER_CHK_NULL(pcRenderParams);
    VPHAL_RENDER_CHK_NULL(m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(m_pRenderHal);
    VPHAL_RENDER_ASSERT(m_pKernelDllState);

    // Increment Call ID (regardless of failure)
    m_iCallID++;

    pOsInterface = m_pOsInterface;
    pRenderHal   = m_pRenderHal;
    pPerfData    = GetPerfData();

    // Reset compositing sources
    iSources  = 0;
    iProcamp  = 0;
    pTarget   = pcRenderParams->pTarget[0];
    MOS_ZeroMemory(pSources, sizeof(pSources));

    // Reset reporting
    m_reporting->InitReportValue();

    // Reset states before rendering (clear allocations, get GSH allocation index
    //                                + any additional housekeeping)
    pOsInterface->pfnResetOsStates(pOsInterface);
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));
    pOsInterface->pfnResetPerfBufferID(pOsInterface);   // reset once per frame

    // Configure cache settings for this render operation
    pCacheSettings      = &pRenderHal->L3CacheSettings;
    MOS_ZeroMemory(pCacheSettings, sizeof(*pCacheSettings));
    pCacheSettings->bOverride                  = true;
    pCacheSettings->bL3CachingEnabled          = m_SurfMemObjCtl.bL3CachingEnabled;
    if (pPerfData->L3SQCReg1Override.bEnabled)
    {
        pCacheSettings->bSqcReg1Override       = true;
        pCacheSettings->dwSqcReg1              = pPerfData->L3SQCReg1Override.uiVal;
    }

    if (pPerfData->L3CntlReg2Override.bEnabled)
    {
        pCacheSettings->bCntlReg2Override      = true;
        pCacheSettings->dwCntlReg2             = pPerfData->L3CntlReg2Override.uiVal;
    }

    if (pPerfData->L3CntlReg3Override.bEnabled)
    {
        pCacheSettings->bCntlReg3Override      = true;
        pCacheSettings->dwCntlReg3             = pPerfData->L3CntlReg3Override.uiVal;
    }

    if (pPerfData->L3LRA1RegOverride.bEnabled)
    {
        pCacheSettings->bLra1RegOverride       = true;
        pCacheSettings->dwLra1Reg              = pPerfData->L3LRA1RegOverride.uiVal;
    }

    if (pPerfData->L3CntlRegOverride.bEnabled)
    {
        pCacheSettings->bCntlRegOverride       = true;
        pCacheSettings->dwCntlReg              = pPerfData->L3CntlRegOverride.uiVal;
    }

    index = 0;
    while (iSources < (int)pcRenderParams->uSrcCount)
    {
        VPHAL_GET_SURFACE_INFO Info;
        PVPHAL_SURFACE         pSrc;    // Current source surface

        pSrc = pcRenderParams->pSrc[index++];
        if (pSrc == nullptr)
        {
            continue;
        }

        VPHAL_RENDER_ASSERT(!Mos_ResourceIsNull(&pSrc->OsResource));

        // Get resource information
        MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

        VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
            pOsInterface,
            &Info,
            pSrc));

        // Ensure the input is ready to be read
        pOsInterface->pfnSyncOnResource(
            pOsInterface,
            &pSrc->OsResource,
            pOsInterface->CurrentGpuContextOrdinal,
            false);

        // Field Weaving needs ref sample
        if (pSrc->bFieldWeaving && pSrc->pBwdRef)
        {
            MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

            VPHAL_RENDER_CHK_STATUS(VpHal_GetSurfaceInfo(
                pOsInterface,
                &Info,
                pSrc->pBwdRef));

            // Ensure the input is ready to be read
            pOsInterface->pfnSyncOnResource(
                pOsInterface,
                &pSrc->pBwdRef->OsResource,
                pOsInterface->CurrentGpuContextOrdinal,
                false);
        }

        // Procamp
        pProcampParams = pSrc->pProcampParams;
        if (pProcampParams && pProcampParams->bEnabled)
        {
            pProcamp = &m_Procamp[iProcamp];
            Procamp.iProcampVersion = pProcamp->iProcampVersion;
            Procamp.bEnabled    =  true;
            Procamp.fBrightness =  pProcampParams->fBrightness;
            Procamp.fContrast   =  pProcampParams->fContrast  ;
            Procamp.fHue        =  pProcampParams->fHue       ;
            Procamp.fSaturation =  pProcampParams->fSaturation;

            // Update procamp version and values only if changed
            if (memcmp(pProcamp, &Procamp, sizeof(Procamp)))
            {
                Procamp.iProcampVersion = m_iProcampVersion++;
                *pProcamp = Procamp;
            }

            iProcamp++;
        }

        // Set sources for rendering
        pSources[iSources] = pSrc;
        iSources++;
    }

    // Sync on Render Target(s)
    index = 0;
    do
    {
        pOsInterface->pfnSyncOnResource(
            pOsInterface,
            &pcRenderParams->pTarget[index]->OsResource,
            pOsInterface->CurrentGpuContextOrdinal,
            true);
        index++;
    } while (index < (int32_t)pcRenderParams->uDstCount);

    // Sync Render Target with Overlay Context
    if (pTarget->bOverlay)
    {
        pOsInterface->pfnSyncOnOverlayResource(
            pOsInterface,
            &pTarget->OsResource,
            pOsInterface->CurrentGpuContextOrdinal);
    }

    // Check max number sources
    if (iSources > VPHAL_MAX_SOURCES)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid number of samples.");
        goto finish;
    }

    // Determine cspace for compositing
    ColorSpace = PrepareCSC(pcRenderParams,
                            pSources,
                            iSources);

    bMultiplePhases = PreparePhases(pcRenderParams,
                                    pSources,
                                    iSources);
    if (!bMultiplePhases)
    {
        pOutput = pTarget;
    }
    else
    {
        pOutput = &m_Intermediate;
        pOutput->ColorSpace = ColorSpace;
        m_Intermediate2.ColorSpace = ColorSpace;

        // Set AYUV or ARGB output depending on intermediate cspace
        if (KernelDll_IsCspace(ColorSpace, CSpace_RGB))
        {
            pOutput->Format = Format_A8R8G8B8;
            m_Intermediate2.Format = Format_A8R8G8B8;
        }
        else
        {
            pOutput->Format = Format_AYUV;
            m_Intermediate2.Format = Format_AYUV;
        }
    }

    VPHAL_RENDER_CHK_STATUS(RenderMultiPhase(pcRenderParams, pSources, iSources, pOutput));

    // Last constriction phase
    if (pcRenderParams->pConstriction)
    {
        VPHAL_COMPOSITE_PARAMS  CompositeParams;

        // Prepare compositing structure
        ResetCompParams(&CompositeParams);

        // set additional render target
        if (pcRenderParams->uDstCount == 2)
        {
            CompositeParams.uTargetCount    = pcRenderParams->uDstCount;
            CompositeParams.Target[1]       = *pcRenderParams->pTarget[1];
        }

        pTarget->SurfType = SURF_OUT_RENDERTARGET;

        // Prepare temporary output for final upscaling
        pOutput->ScalingMode = VPHAL_SCALING_BILINEAR;
        pOutput->rcSrc       = *(pcRenderParams->pConstriction);
        pOutput->rcDst       = pTarget->rcDst;

        AddCompLayer(&CompositeParams, pOutput);

        AddCompTarget(&CompositeParams, pTarget);

        // Reset states before rendering (clear allocations, get GSH allocation index
        //                                + any additional housekeeping)
        pOsInterface->pfnResetOsStates(pOsInterface);
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

        // Raise the flag to indicate the last comp render phase
        m_bLastPhase = true;

        // Perform compositing operation
        VPHAL_RENDER_CHK_STATUS(RenderPhase(&CompositeParams));
    }

finish:
    if (pCacheSettings)
    {
        MOS_ZeroMemory(pCacheSettings, sizeof(*pCacheSettings));
    }
    // Reset Slice Shutdown Mode
    if (pRenderHal)
    {
        pRenderHal->pfnSetSliceShutdownMode(pRenderHal, false);
    }

    VPHAL_RENDER_EXITMESSAGE("eStatus %d", eStatus);

    return eStatus;
}

//!
//! \brief    Set Composite Scaling mode
//! \param    [in,out] pSource
//!           Pointer to Source Surface
//! \param    [in] uSourceCount
//!           Count of Source Surfaces
//! \return   void
//!
void CompositeState::SetScalingMode(
    PVPHAL_SURFACE          pSource,
    uint32_t                uSourceCount)
{
    float   fScaleX, fScaleY;

    VPHAL_RENDER_ASSERT(pSource);

    // Default mode
    pSource->bIEF = false;

    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (pSource->Rotation == VPHAL_ROTATION_IDENTITY    ||
        pSource->Rotation == VPHAL_ROTATION_180         ||
        pSource->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
        pSource->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                       (float)(pSource->rcSrc.right  - pSource->rcSrc.left);
        fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                       (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                       (float)(pSource->rcSrc.bottom  - pSource->rcSrc.top);
        fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                       (float)(pSource->rcSrc.right - pSource->rcSrc.left);
    }

    // Enable AVS/IEF for primary video only
    // AVS is supported only for y-scaling ratios > 0.0625
    // Starting from IVB, AVS is supported only for x-scaling ratios > 0.0625
    if (pSource->ScalingMode == VPHAL_SCALING_AVS   &&
        fScaleX > 0.0625f                           &&
        fScaleY > 0.0625f)
    {
        // Interlaced content - Disable AVS for BOB implementation
        if (IsBobDiEnabled(pSource))
        {
            pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
        // Enable IEF
        else if (pSource->pIEFParams             &&
                 pSource->pIEFParams->bEnabled   &&
                 pSource->pIEFParams->fIEFFactor > 0.0f)
        {
            pSource->bIEF = true;
        }
        // If IEF is disabled and Scaling Ratios are 1x, use 3D Nearest
        // for better performance (applicable for Primary-only case)
        // Don't fall back to bilinear scaling if Chroma up sampling is needed
        else if (fScaleX == 1.0F   &&
                 fScaleY == 1.0F   &&
                 uSourceCount == 1 &&
                 !m_bChromaUpSampling)
        {
            pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
    }
    else
    {
        pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
    }

    // Fix Interlace Scaling Hang issue, switch avs to bilinear scaling because kernels
    // are using 3d samplers for unaligned cases
    if (pSource->bInterlacedScaling                                                 &&
        (!MOS_IS_ALIGNED(MOS_MIN(pSource->dwWidth, (uint32_t)pSource->rcSrc.right), 4) ||
         !MOS_IS_ALIGNED(pSource->dwHeight, 4)))
    {
        pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
    }

    // Fix GPU Hang on EHL, since EHL has no AVS sampler
    if (MEDIA_IS_SKU(m_pSkuTable, FtrDisableVEBoxFeatures))
    {
        pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
    }

    // WA for multilayer P010 AVS+3D one single pass corruption hw issue
    if (uSourceCount > 1 &&
        pSource->Format == Format_P010)
    {
        pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
    }

}

//!
//! \brief    Initialize Composite Rendering data
//! \details  Initialize Composite Rendering data, set output area, number of blocks,
//!           Sources, constriction parameters, rendering states, etc.
//! \param    [in] pCompParams
//!           Pointer to Composite parameters
//! \param    [out] pRenderingData
//!           Pointer to Composite Rendering data
//! \return   MOS_STATUS
//!
MOS_STATUS CompositeState::RenderInit(
    PVPHAL_COMPOSITE_PARAMS         pCompParams,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData)
{
    PRENDERHAL_INTERFACE    pRenderHal;
    RECT                    AlignedRect;
    uint32_t                uiMediaWalkerBlockSize;
    PRECT                   pDst;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pCompParams);
    VPHAL_RENDER_CHK_NULL(pRenderingData);

    pRenderHal   = m_pRenderHal;

    //============================
    // Set rendering data
    //============================
    MOS_ZeroMemory(pRenderingData, sizeof(VPHAL_RENDERING_DATA_COMPOSITE));

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

    if (m_bFtrMediaWalker)
    {
        uiMediaWalkerBlockSize = pRenderHal->pHwSizes->dwSizeMediaWalkerBlock;
    }
    else
    {
        uiMediaWalkerBlockSize = VPHAL_COMP_BLOCK_WIDTH;
    }

    // Calculate aligned output area in order to determine the total # blocks to process
    // in case of non-16x16 aligned target
    AlignedRect         = *pDst;
    AlignedRect.right  += uiMediaWalkerBlockSize  - 1;
    AlignedRect.bottom += uiMediaWalkerBlockSize - 1;
    AlignedRect.left   -= AlignedRect.left   % uiMediaWalkerBlockSize;
    AlignedRect.top    -= AlignedRect.top    % uiMediaWalkerBlockSize;
    AlignedRect.right  -= AlignedRect.right  % uiMediaWalkerBlockSize;
    AlignedRect.bottom -= AlignedRect.bottom % uiMediaWalkerBlockSize;

    // Set number of blocks
    pRenderingData->iBlocksX =
        (AlignedRect.right  - AlignedRect.left) / uiMediaWalkerBlockSize;
    pRenderingData->iBlocksY =
        (AlignedRect.bottom - AlignedRect.top ) / uiMediaWalkerBlockSize;

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

    // Set AVS and 8x8 table from renderer
    pRenderingData->pAvsParams             = &m_AvsParameters;

    // Init extension data to nullptr
    pRenderingData->pExtensionData         = nullptr;

    // Initialize rendering states
    pRenderingData->Static                 = g_cInit_MEDIA_OBJECT_KA2_STATIC_DATA;
    pRenderingData->Inline                 = g_cInit_MEDIA_OBJECT_KA2_INLINE_DATA;
    pRenderingData->WalkerStatic           = g_cInit_MEDIA_WALKER_KA2_STATIC_DATA;

    // By default, alpha is calculated in PartBlend kernel
    pRenderingData->bAlphaCalculateEnable = false;

    // Reset Sampler Params
    MOS_ZeroMemory(
        pRenderingData->SamplerStateParams,
        sizeof(pRenderingData->SamplerStateParams));

finish:
    return eStatus;
}

//!
//! \brief    Clean Composite Rendering data
//! \param    [in] pRenderingData
//!           Pointer to Composite Rendering data
//! \return   MOS_STATUS
//!
void CompositeState::CleanRenderingData(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData)
{
    MOS_UNUSED(pRenderingData);
}

//!
//! \brief    Get Binding Table Index associated with a given source for composite
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \param    [out] pBindingIndex
//!           Pointer to Binding table index
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
//!
static MOS_STATUS GetBindingIndex(
    PVPHAL_SURFACE          pSource,
    int32_t*                pBindingIndex)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (pSource != nullptr && pBindingIndex != nullptr)
    {
        if (pSource->SurfType == SURF_OUT_RENDERTARGET)
        {
            *pBindingIndex = g_cBindingTableIndex[0];
        }
        else if (pSource->iLayerID >= 0 &&
                 pSource->iLayerID < VPHAL_COMP_MAX_LAYERS)
        {
            *pBindingIndex = g_cBindingTableIndex[pSource->iLayerID + 1];
        }
        else
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
    }
    else
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

//!
//! \brief    Get Sampler Index associated with a surface state for composite
//! \param    [in] pSurface
//!           point to input Surface
//! \param    [in] pEntry
//!           Pointer to Surface state
//! \param    [out] pSamplerIndex
//!           Pointer to Sampler Index
//! \param    [out] pSamplerType
//!           Pointer to Sampler Type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise MOS_STATUS_UNKNOWN
//!
MOS_STATUS CompositeState::GetSamplerIndex(
    PVPHAL_SURFACE                      pSurface,
    PRENDERHAL_SURFACE_STATE_ENTRY      pEntry,
    int32_t*                            pSamplerIndex,
    PMHW_SAMPLER_TYPE                   pSamplerType)
{
    MOS_UNUSED(pSurface);

    if (pSamplerIndex == nullptr || pSamplerType == nullptr || pEntry == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE(" Null pointer.");
        return MOS_STATUS_NULL_POINTER;
    }

    // AVS
    if (pEntry->bAVS)
    {
        *pSamplerType = MHW_SAMPLER_TYPE_AVS;

        if (pEntry->YUVPlane == MHW_U_PLANE)
        {
            *pSamplerIndex = VPHAL_SAMPLER_8x8_AVS_U;
        }
        else if (pEntry->YUVPlane == MHW_V_PLANE)
        {
            *pSamplerIndex = VPHAL_SAMPLER_8x8_AVS_V;
        }
        else
        {
            *pSamplerIndex = VPHAL_SAMPLER_8x8_AVS_Y;
        }
    }
    // Non-AVS
    else
    {
        *pSamplerType  = MHW_SAMPLER_TYPE_3D;

        if (pEntry->YUVPlane == MHW_U_PLANE)
        {
            *pSamplerIndex = VPHAL_SAMPLER_U;
        }
        else if (pEntry->YUVPlane == MHW_V_PLANE)
        {
            *pSamplerIndex = VPHAL_SAMPLER_V;
        }
        else
        {
            *pSamplerIndex = VPHAL_SAMPLER_Y;
        }
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set Surface Parameters
//! \details  Set Surface Parameters, set flags for RT, set surface type based on scaling
//!           mode, set interlacing flags, etc.
//! \param    [in,out] pSource
//!           Pointer to Source Surface
//! \param    [out] pSurfaceParams
//!           Pointer to Surface Parameters
//! \return   void
//!
void CompositeState::SetSurfaceParams(
    PVPHAL_SURFACE                  pSource,
    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams)
{
    // Render target or private surface
    if (pSource->SurfType == SURF_OUT_RENDERTARGET)
    {
        // Disable AVS, IEF
        pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
        pSource->bIEF        = false;

        // Set flags for RT
        pSurfaceParams->bRenderTarget    = true;
        pSurfaceParams->bWidthInDword_Y  = true;
        pSurfaceParams->bWidthInDword_UV = true;
        pSurfaceParams->Boundary         = RENDERHAL_SS_BOUNDARY_DSTRECT;
    }
    // other surfaces
    else
    {
        pSurfaceParams->bRenderTarget    = false;
        pSurfaceParams->bWidthInDword_Y  = false;
        pSurfaceParams->bWidthInDword_UV = false;
        pSurfaceParams->Boundary         = RENDERHAL_SS_BOUNDARY_SRCRECT;
    }

    // Set surface type based on scaling mode
    if (pSource->ScalingMode == VPHAL_SCALING_AVS)
    {
        pSurfaceParams->Type = m_pRenderHal->SurfaceTypeAdvanced;
        pSurfaceParams->bAVS = true;
    }
    else
    {
        pSurfaceParams->Type = m_pRenderHal->SurfaceTypeDefault;
        pSurfaceParams->bAVS = false;
    }

    // Set interlacing flags
    switch (pSource->SampleType)
    {
        case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
        case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
            pSurfaceParams->bVertStride     = true;
            pSurfaceParams->bVertStrideOffs = 0;
            break;
        case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
        case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
            pSurfaceParams->bVertStride     = true;
            pSurfaceParams->bVertStrideOffs = 1;
            break;
        default:
            pSurfaceParams->bVertStride     = false;
            pSurfaceParams->bVertStrideOffs = 0;
            break;
    }

    if (pSource->iLayerID && IsNV12SamplerLumakeyNeeded(pSource, m_pRenderHal))
    {
        pSurfaceParams->b2PlaneNV12NeededByKernel = true;
    }

     VPHAL_RENDER_NORMALMESSAGE("SurfaceTYpe %d, bAVS %d, b2PlaneNV12NeededByKernel %d",
        pSurfaceParams->Type,
        pSurfaceParams->bAVS,
        pSurfaceParams->b2PlaneNV12NeededByKernel);
}

//!
//! \brief    calculate the Horiz Gap and Vert Gap with different sample location
//! \param    [in] pTarget
//!           Pointer to Source Surface
//! \Param    [in] pHorzGap
//!           Pointer to Horzontal Gap
//! \Param    [in] pVertGap
//!           Pointer to Vertital Gap
//!
static void GetOffsetChromasiting(
    PVPHAL_SURFACE                      pSource,
    float*                              pHorizGap,
    float*                              pVertGap
    )
{
    float  HorizGap = 0.0f;
    float  VertGap  = 0.0f;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSource);

    // If there is no DDI setting, we use the Horizontal Left Vertical Center as default for PL2 surface.
    if (pSource->ChromaSiting == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (IS_PL2_FORMAT(pSource->Format) || IS_PL2_FORMAT_UnAligned(pSource->Format))
        {
            VertGap = (float)(0.5f / pSource->dwHeight);
        }
    }
    else
    {
        // PL2, 6 positions are available
        if (IS_PL2_FORMAT(pSource->Format) || IS_PL2_FORMAT_UnAligned(pSource->Format))
        {
            // Horizontal Left
            if (pSource->ChromaSiting & CHROMA_SITING_HORZ_LEFT)
            {
                if (pSource->ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    VertGap = (float)(0.5f / pSource->dwHeight);
                }
                else if (pSource->ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    VertGap = (float)(1.0f / pSource->dwHeight);
                }
            }
            // Horizontal Center
            else if (pSource->ChromaSiting & CHROMA_SITING_HORZ_CENTER)
            {
                HorizGap = (float)(0.5f / pSource->dwWidth);
                if (pSource->ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    VertGap = (float)(0.5f / pSource->dwHeight);
                }
                else if (pSource->ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    VertGap = (float)(1.0f / pSource->dwHeight);
                }
            }
        }
        else if (IS_PA_FORMAT(pSource->Format))
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (pSource->ChromaSiting & (CHROMA_SITING_HORZ_CENTER))
            {
                HorizGap = (float)(0.5f / pSource->dwWidth);
            }
        }
    }
    *pVertGap = VertGap;
    *pHorizGap = HorizGap;
finish:
    return;
}

//!
//! \brief    Set Sampler AVS parameters
//! \param    [in] pRenderingData
//!           pointer to render data
//! \param    [in] pSource
//!           pointer to source surface
//! \param    [in] pSurfaceEntry
//!           pointer to source state entry
//! \param    [out] pSamplerStateParams
//!           pointer to Sampler state params
//! \param    [in] fScaleX
//!           width scaling ratio
//! \param    [in] fScaleY
//!           height scaling ratio
//! \return   MOS_STATUS
//!
MOS_STATUS CompositeState::SetSamplerAvsParams(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    PVPHAL_SURFACE                  pSource,
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry,
    PMHW_SAMPLER_STATE_PARAM        pSamplerStateParams,
    float                           fScaleX,
    float                           fScaleY)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    pSamplerStateParams->Avs.bEnableAVS     = true;
    // Default values from g_cInit_SAMPLER_STATE_8x8_GX
    pSamplerStateParams->Avs.WeakEdgeThr    = DETAIL_WEAK_EDGE_THRESHOLD;
    pSamplerStateParams->Avs.StrongEdgeThr  = DETAIL_STRONG_EDGE_THRESHOLD;
    pSamplerStateParams->Avs.StrongEdgeWght = DETAIL_STRONG_EDGE_WEIGHT;
    pSamplerStateParams->Avs.RegularWght    = DETAIL_REGULAR_EDGE_WEIGHT;
    pSamplerStateParams->Avs.NonEdgeWght    = DETAIL_NON_EDGE_WEIGHT;

    pSamplerStateParams->Avs.pMhwSamplerAvsTableParam = &m_mhwSamplerAvsTableParam;

    // When primary surface needs chroma upsampling,
    // force to use polyphase coefficients for 1x scaling for better quality
    pRenderingData->pAvsParams->bForcePolyPhaseCoefs =
        m_bChromaUpSampling;

    // Setup IEF parameters for generic or luma planes (no need to setup chroma planes)
    if (pSource->pIEFParams &&
        pSource->bIEF &&
        pSurfaceEntry->YUVPlane != MHW_U_PLANE &&
        pSurfaceEntry->YUVPlane != MHW_V_PLANE &&
        (!m_bFallbackIefPatch))                                 // if m_bFallbackIefPatch is on, fallback IEF patch from AVS to SFC
    {
        Ief ief(pSource);

        eStatus = ief.SetHwState(pSamplerStateParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("set Sampler IEF parameter failed.");
        }
    }

    eStatus = SetSamplerAvsTableParam(
        m_pRenderHal,
        pSamplerStateParams,
        pRenderingData->pAvsParams,
        pSource->Format,
        fScaleX,
        fScaleY,
        MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP);

    return eStatus;
}

//!
//! \brief    Calculate crop factor
//! \param    [in] iLayer
//!           layer index
//! \param    [in] pRenderingData
//!           pointer to render data
//! \param    [out] pfCropX
//!           crop factor
//! \param    [out] pfCropY
//!           crop factor
//! \return   MOS_STATUS
//!
MOS_STATUS CompositeState::CalculateCropParams(
    int32_t                         iLayer,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    float*                          pfCropX,
    float*                          pfCropY)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_UNUSED(iLayer);
    MOS_UNUSED(pRenderingData);

    VPHAL_RENDER_CHK_NULL_RETURN(pfCropX);
    VPHAL_RENDER_CHK_NULL_RETURN(pfCropY);

    *pfCropX = 0.0;
    *pfCropY = 0.0;

    return eStatus;
}

//!
//! \brief    Set Composite Layer
//! \details  Set Composite Layer, including setup surface state and binding table, setup
//!           lumakey parameters, setup samplers, setup alpha blending parameters, adjust
//!           geometry for BOB DI, normalize source co-ordinates, set curbe and inline
//!           data, and etc
//! \param    [in] pRenderingData
//!           Pointer to Composite Rendering data
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \param    [in] iLayerIdInCompParams
//!           Index of pSource in pCompParams
//! \param    [in,out] pCompParams
//!           Pointer to Composite parameters
//! \return   int32_t
//!           Return 1 if set layer successful, otherwise -1
//!
int32_t CompositeState::SetLayer(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    PVPHAL_SURFACE                  pSource,
    int                             iLayerIdInCompParams,
    PVPHAL_COMPOSITE_PARAMS         pCompParams)
{
    // Result
    MOS_STATUS                          eStatus;
    PRENDERHAL_SURFACE                  pRenderHalSurfaceSrc = nullptr;         // Pointer to Source RenderHal Surface related to VPHAL Surface
    PRENDERHAL_SURFACE                  pRenderHalSurfaceSrcField = nullptr;    // Pointer to Source RenderHal Surface (FieldWeaving) related to VPHAL Surface

    // States
    PRENDERHAL_INTERFACE                pRenderHal;
    int32_t                             iLayer; // The index of pSource in pRenderingData->pLayers.
    MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic;
    MEDIA_OBJECT_KA2_INLINE_DATA        *pInline;

    // Surface states
    int32_t                             iSurfaceEntries, i, iSurfaceEntries2;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntries2[MHW_MAX_SURFACE_PLANES];
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntry;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParams, SurfaceParams2;
    int32_t                             iBTentry;

    // Sampler states
    int32_t                             iSamplerID;     // Sampler ID
    MHW_SAMPLER_TYPE                    SamplerType;    // Sampler Type
    PMHW_SAMPLER_STATE_PARAM            pSamplerStateParams;
    VPHAL_SCALING_MODE                  ScalingMode;

    // Constant alpha
    uint16_t                            wAlpha;         // Constant Alpha (8.8)

    // Geometry related parameters
    uint32_t    dwSurfStateWd, dwSurfStateHt;       // Surface Width Height as programmed in SS
    uint32_t    dwDestRectWidth, dwDestRectHeight;  // Target rectangle Width Height
    float       fOffsetY, fOffsetX;                 // x,y Sr offset
    float       fShiftX , fShiftY;                  // x,y Dst shift + Sampler BOB adj
    float       fDiScaleY;                          // BOB scaling factor for Y
    float       fScaleX, fScaleY;                   // x,y scaling factor
    float       fStepX , fStepY;                    // x,y scaling steps
    float       fOriginX, fOriginY;                 // x,y layer origin
    PRECT       pTargetRect;                        // Clipping rectangle (RT)
    RECT        DestRect;                           // Clipped dest rectangle
    int32_t     iResult = 0;                        // Default result = 0 (don't render the current plane)
    float       fHorizgap, fVertgap;                // horizontal gap and vertical gap: based on Sampler need
    uint32_t    dwLow, dwHigh;
    bool        bForceNearestForUV = false;

    // cropping
    float       fCropX, fCropY;

    // Temp variable for iScaling/Field Weaving
    PVPHAL_SURFACE               pTempSurf;

    if (nullptr == pRenderingData || nullptr == pSource || nullptr == pCompParams ||
        iLayerIdInCompParams < 0 || iLayerIdInCompParams >= (int)pCompParams->uSourceCount)
    {
        VPHAL_RENDER_ASSERTMESSAGE("invalid input parameters");
        iResult = -1;
        goto finish;
    }

    pRenderHalSurfaceSrc        = &pCompParams->RenderHalSurfaceSrc[iLayerIdInCompParams];
    pRenderHalSurfaceSrcField   = &pCompParams->RenderHalSurfaceSrcField[iLayerIdInCompParams];

    ScalingMode = pSource->ScalingMode;

    // init surface parameters
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    MOS_ZeroMemory(&SurfaceParams2, sizeof(SurfaceParams2));
    pTempSurf   = nullptr;

    // Init x,y Sr offset/Dst shift.
    fOffsetX = 0;
    fOffsetY = 0;
    fShiftX  = 0;
    fShiftY  = 0;

    // Initialize States
    pRenderHal   = m_pRenderHal;
    iLayer       = pRenderingData->iLayers;
    if(m_bFtrMediaWalker)
    {
        pStatic      = (MEDIA_OBJECT_KA2_STATIC_DATA*)&pRenderingData->WalkerStatic;
    }
    else
    {
        pStatic      = &pRenderingData->Static;
    }
    pInline      = &pRenderingData->Inline;

    // Set the default alpha value to 0 for Android platform
    // According to the Partial Blending algorithm, if the default colorfill_a is set to 255, the output alpha value will be 0xFE or 0xFF.
    // It means the padding area of top layer always be opaqued. To reduce the code change risk, modify it only for Android platform.
    // Need follow-up on formal solution.
#if ANDROID
    wAlpha = 0;
#else
    wAlpha = 255;
#endif

    // set destination width and height
    if (pRenderingData->pTarget[1] == nullptr)
    {
        dwDestRectWidth  = pRenderingData->pTarget[0]->dwWidth;
        dwDestRectHeight = pRenderingData->pTarget[0]->dwHeight;
    }
    else
    {
        // destination width and height base on non-rotated
        dwDestRectWidth  = pRenderingData->pTarget[1]->dwWidth;
        dwDestRectHeight = pRenderingData->pTarget[1]->dwHeight;
    }

    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (pSource->Rotation == VPHAL_ROTATION_IDENTITY    ||
        pSource->Rotation == VPHAL_ROTATION_180         ||
        pSource->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
        pSource->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                       (float)(pSource->rcSrc.right  - pSource->rcSrc.left);
        fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                       (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        fScaleX      = (float)(pSource->rcDst.right  - pSource->rcDst.left) /
                       (float)(pSource->rcSrc.bottom  - pSource->rcSrc.top);
        fScaleY      = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                       (float)(pSource->rcSrc.right - pSource->rcSrc.left);
    }
    fDiScaleY    = 1.0f;

    CalculateCropParams(iLayer, pRenderingData, &fCropX, &fCropY);

    pTargetRect = &pRenderingData->BbArgs.rcOutput;

    if (pRenderingData->pConstriction)
    {
        fScaleX /= pRenderingData->fConstrictionStepX;
        fScaleY /= pRenderingData->fConstrictionStepY;
    }

    //-------------------------------------------
    // Set layer ID - necessary to bind surface states
    //-------------------------------------------
    pSource->iLayerID = iLayer;

    pSource->bUseSampleUnorm = IsUsingSampleUnorm(pCompParams, pSource);
    // Check whether sampler lumakey is needed. It will be used in SetSurfaceParams
    // when IsNV12SamplerLumakeyNeeded being called.
    pSource->bUseSamplerLumakey = IsSamplerLumakeySupported(pSource);
    //-------------------------------------------
    // Setup surface states
    //-------------------------------------------
    SetSurfaceParams(pSource, &SurfaceParams);

    if (m_bChromaUpSampling || m_bChromaDownSampling)
    {
        if (!MEDIA_IS_WA(m_pWaTable, WaEnableDscale)                      ||
            (MEDIA_IS_WA(m_pWaTable, WaEnableDscale)                      &&
             pSource->ScalingMode == VPHAL_SCALING_BILINEAR  &&
             fScaleX >= (float)(1.0/3.0)                     &&
             fScaleY >= (float)(1.0/3.0)))
        {
            SurfaceParams.bChromasiting      = true;
            pSource->bChromaSiting           = true;
        }
        else
        {
            SurfaceParams.bChromasiting = false;
            pSource->bChromaSiting      = false;
        }
    }
    else
    {
            SurfaceParams.bChromasiting      = false;
            pSource->bChromaSiting           = false;
    }

    if (pSource->bInterlacedScaling)
    {
        // Top Input Field
        SurfaceParams.bVertStrideOffs   = false;
        SurfaceParams.bVertStride       = true;

        // Bottom Input Field
        SurfaceParams2 = SurfaceParams;
        SurfaceParams2.bVertStrideOffs  = true;
    }
    else if (pSource->bFieldWeaving)
    {
        // Top Input Field
        SurfaceParams.bVertStrideOffs   = false;
        SurfaceParams.bVertStride       = false;

        // Bottom Input Field
        SurfaceParams2 = SurfaceParams;
    }

    // Allocate palette ID for surface (do not load palette)
    if (IS_PAL_FORMAT(pSource->Format))
    {
        eStatus = pRenderHal->pfnAllocatePaletteID(pRenderHal, &pSource->iPalette);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            iResult = -1;
            goto finish;
        }
    }

    SurfaceParams.MemObjCtl = (pSource->SurfType == SURF_IN_PRIMARY) ?
                                m_SurfMemObjCtl.PrimaryInputSurfMemObjCtl :
                                m_SurfMemObjCtl.InputSurfMemObjCtl ;

    eStatus = VpHal_RndrCommonInitRenderHalSurface(pSource, pRenderHalSurfaceSrc);
    if (MOS_FAILED(eStatus))
    {
        iResult = -1;
        goto finish;
    }
    // Setup surface states
    eStatus = pRenderHal->pfnSetupSurfaceState(
            pRenderHal,
            pRenderHalSurfaceSrc,
            &SurfaceParams,
            &iSurfaceEntries,
            pSurfaceEntries,
            nullptr);
    if (MOS_FAILED(eStatus))
    {
        iResult = -1;
        goto finish;
    }

    eStatus = VpHal_RndrCommonGetBackVpSurfaceParams(
        pRenderHalSurfaceSrc,
        pSource);
    if (MOS_FAILED(eStatus))
    {
        iResult = -1;
        goto finish;
    }

    if (ScalingMode != pSource->ScalingMode)
    {
        // The AVS may be modified to Bilinear in RenderHal_SetupSurfaceState->RenderHal_GetSurfaceStateEntries if AVS
        // is not supported by the format of source.
        // Both bUseSampleUnorm and bUseSamplerLumakey need be updated.
        pSource->bUseSampleUnorm = IsUsingSampleUnorm(pCompParams, pSource);
        pSource->bUseSamplerLumakey = IsSamplerLumakeySupported(pSource);
    }

    //--------------------------------------------------------
    // iScaling & Field Weaving needs 2 sets of input surfaces
    //--------------------------------------------------------
    iSurfaceEntries2 = 0;
    if (pSource->bInterlacedScaling || pSource->bFieldWeaving)
    {
        SurfaceParams2.MemObjCtl = (pSource->SurfType == SURF_IN_PRIMARY) ?
                                    m_SurfMemObjCtl.PrimaryInputSurfMemObjCtl :
                                    m_SurfMemObjCtl.InputSurfMemObjCtl ;

        // For Interlaced scaling 2nd field is part of the same frame
        // For Field weaving 2nd field is passed in as a ref. surface
        pTempSurf = pSource->bFieldWeaving ? pSource->pBwdRef : pSource;
        eStatus = VpHal_RndrCommonInitRenderHalSurface(pTempSurf, pRenderHalSurfaceSrcField);
        if (MOS_FAILED(eStatus))
        {
            iResult = -1;
            goto finish;
        }
        eStatus = pRenderHal->pfnSetupSurfaceState(
            pRenderHal,
            pRenderHalSurfaceSrcField,
            &SurfaceParams2,
            &iSurfaceEntries2,
            pSurfaceEntries2,
            nullptr);
        if (MOS_FAILED(eStatus))
        {
            iResult = -1;
            goto finish;
        }
        eStatus = VpHal_RndrCommonGetBackVpSurfaceParams(
            pRenderHalSurfaceSrcField,
            pTempSurf);
        if (MOS_FAILED(eStatus))
        {
            iResult = -1;
            goto finish;
        }
    }

    if (iSurfaceEntries <= 0         ||
        (pSource->bInterlacedScaling &&
         pSource->bFieldWeaving      &&
         iSurfaceEntries2            != iSurfaceEntries))
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to setup surface state.");
        iResult = -1;
        goto finish;
    }

    //-------------------------------------------
    // Bind surface states
    //-------------------------------------------
    eStatus = GetBindingIndex(pSource, &iBTentry);
    if (MOS_FAILED(eStatus))
    {
        goto finish;
    }

    for (i = 0; i < iSurfaceEntries; i++, iBTentry++)
    {
        eStatus = pRenderHal->pfnBindSurfaceState(pRenderHal,
                                        pRenderingData->iBindingTable,
                                        iBTentry,
                                        pSurfaceEntries[i]);
        if (MOS_FAILED(eStatus))
        {
            iResult = -1;
            goto finish;
        }

        // Interlaced scaling & Field Weaving case
        if (pSource->bInterlacedScaling || pSource->bFieldWeaving)
        {
            eStatus = pRenderHal->pfnBindSurfaceState(
                pRenderHal,
                pRenderingData->iBindingTable,
                VPHAL_COMP_BTINDEX_L0_FIELD1_DUAL + i,
                pSurfaceEntries2[i]);

            if (MOS_FAILED(eStatus))
            {
                iResult = -1;
                goto finish;
            }
        }
    }

    //-----------------------------------
    // Setup Luma keying parameters
    //-----------------------------------
    if (pSource->pLumaKeyParams != nullptr)
    {
        VPHAL_RENDER_NORMALMESSAGE("LumaLow %d, LumaHigh %d",
            pSource->pLumaKeyParams->LumaLow,
            pSource->pLumaKeyParams->LumaHigh);

        pStatic->DW14.LumakeyLowThreshold  = pSource->pLumaKeyParams->LumaLow;
        pStatic->DW14.LumakeyHighThreshold = pSource->pLumaKeyParams->LumaHigh;
    }

    //-----------------------------------
    // Setup Samplers
    //-----------------------------------
    for (i = 0; i < iSurfaceEntries; i++, iBTentry++)
    {
        if (pSurfaceEntries[i] == nullptr)
        {
            continue;
        }

        // Obtain Sampler ID and Type
        eStatus = GetSamplerIndex(pSource,
                                  pSurfaceEntries[i],
                                  &iSamplerID,
                                  &SamplerType);

        // Point to the current SamplerStateParam
        pSamplerStateParams = &pRenderingData->SamplerStateParams[iSamplerID];

        // No need to setup a sampler
        if (MOS_FAILED(eStatus))
        {
            continue;
        }

        pSamplerStateParams->SamplerType = SamplerType;

        if (SamplerType == MHW_SAMPLER_TYPE_3D)
        {
            fOffsetX = m_fSamplerLinearBiasX;
            fOffsetY = m_fSamplerLinearBiasY;

            // Use 3D Nearest Mode only for 1x Scaling in both directions and only if the input is Progressive or interlaced scaling is used
            // In case of two or more layers, set Sampler State to Bilinear if any layer requires Bilinear
            // When primary surface needs chroma upsampling,
            // force to use 3D Bilinear Mode for 1x scaling for better quality
            if (fScaleX == 1.0F &&
                fScaleY == 1.0F &&
                !m_bChromaUpSampling &&
                !m_bChromaDownSampling &&
                (pSource->SampleType == SAMPLE_PROGRESSIVE || pSource->bInterlacedScaling || pSource->bFieldWeaving) &&
                (!pSamplerStateParams->bInUse ||
                (pSamplerStateParams->bInUse && pSamplerStateParams->Unorm.SamplerFilterMode == MHW_SAMPLER_FILTER_NEAREST)))
            {
                fShiftX  = 0.0f;
                fShiftY  = 0.0f;
                pSamplerStateParams->Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_NEAREST;
            }
            else
            {
                //For Y210/Y216 with AVS(Y)+3D(U/V) sampler, the shift is not needed.
                if ((pSource->Format == Format_Y210 ||
                    pSource->Format == Format_Y216)
                    && pSurfaceEntries[0]->bAVS)
                {
                    fShiftX = 0.0f;
                    fShiftY = 0.0f;
                }
                else
                {
                    fShiftX = VPHAL_HW_LINEAR_SHIFT;  // Bilinear scaling shift
                    fShiftY = VPHAL_HW_LINEAR_SHIFT;
                }

                pSamplerStateParams->Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_BILINEAR;
            }
            pSamplerStateParams->Unorm.AddressU = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            pSamplerStateParams->Unorm.AddressV = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
            pSamplerStateParams->Unorm.AddressW = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;

            if (IsSamplerIDForY(iSamplerID) && pSource->bUseSamplerLumakey)
            {
                //From Gen10,HW support 1 plane LumaKey process on NV12 format, Gen9 only support 2 plane LumaKey process
                //if go to 1 plane, MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8 format will be used, LumaKey value need to be set on Y channel, the corresponding bit range is 15:8
                //if go to 2 plane, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM     format will be used, LumaKey value need to be set on R channel, the corresponding bit range is 23:16
                if (IsNV12SamplerLumakeyNeeded(pSource, pRenderHal) || (pSurfaceEntries[i]->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM))
                {
                    dwLow  = pSource->pLumaKeyParams->LumaLow << 16;
                    dwHigh = (pSource->pLumaKeyParams->LumaHigh << 16) | 0xFF00FFFF;
                    bForceNearestForUV = true;
                }
                else
                {
                    dwLow  = pSource->pLumaKeyParams->LumaLow << 8;
                    dwHigh = (pSource->pLumaKeyParams->LumaHigh << 8) | 0xFFFF00FF;
                }

                pSamplerStateParams->Unorm.bChromaKeyEnable = true;
                pSamplerStateParams->Unorm.ChromaKeyMode    = MHW_CHROMAKEY_MODE_KILL_ON_ANY_MATCH;
                pSamplerStateParams->Unorm.ChromaKeyIndex   = pRenderHal->pfnAllocateChromaKey(pRenderHal, dwLow, dwHigh);
            }

            if ((!IsSamplerIDForY(iSamplerID)) && bForceNearestForUV)
            {
                pSamplerStateParams->Unorm.SamplerFilterMode = MHW_SAMPLER_FILTER_NEAREST;
            }
        }
        else if (SamplerType == MHW_SAMPLER_TYPE_AVS)
        {
            // Disable sampler bias
            fOffsetX = 0.0f;
            fOffsetY = 0.0f;

            // Disable linear shift
            fShiftX = 0.0f;
            fShiftY = 0.0f;

            pSamplerStateParams->Avs.b8TapAdaptiveEnable = Is8TapAdaptiveEnabled(pSource, fScaleX, fScaleY);

            // Set HDC Direct Write Flag
            pSamplerStateParams->Avs.bHdcDwEnable = pRenderingData->bHdcDwEnable;
        }

        pSamplerStateParams->bInUse        = true;

        // Set AVS Scaling Table
        if (pSurfaceEntries[i] && pSurfaceEntries[i]->bAVS)
        {
            VPHAL_RENDER_ASSERT(SamplerType == MHW_SAMPLER_TYPE_AVS);

            eStatus = SetSamplerAvsParams(
                pRenderingData,
                pSource,
                pSurfaceEntries[i],
                pSamplerStateParams,
                fScaleX,
                fScaleY);

            if (MOS_FAILED(eStatus))
            {
                iResult = -1;
                goto finish;
            }
        }
    }

    //-----------------------------------
    // Alpha blending optimization.
    // If Constant blending and one of the following is true, disable blending.
    // If Src+Constant blending and one of the following is true, fall back to Src blending.
    // Condition; alpha <= 0. Layer is 100% transparent.
    // Condition; alpha >= 1. Layer is 100% opaque.
    //-----------------------------------
    if (pSource->pBlendingParams &&
        ((pSource->pBlendingParams->BlendType == BLEND_CONSTANT) ||
         (pSource->pBlendingParams->BlendType == BLEND_CONSTANT_SOURCE) ||
         (pSource->pBlendingParams->BlendType == BLEND_CONSTANT_PARTIAL)))
    {
        float fAlpha = pSource->pBlendingParams->fAlpha;

        VPHAL_RENDER_NORMALMESSAGE("BlendType %d, fAlpha %d",
            pSource->pBlendingParams->BlendType,
            pSource->pBlendingParams->fAlpha);

        // Don't render layer with alpha <= 0.0f
        if (fAlpha <= 0.0f)
        {
            // layer is not visible - disable layer
            pSource->iLayerID = -1;
            goto finish;
        }
        else
        {
            wAlpha  = (uint16_t) (255.0f * fAlpha);
        }

        if (fAlpha >= 1.0f || wAlpha >= 255)
        {
            if (pSource->pBlendingParams->BlendType == BLEND_CONSTANT)
            {
                pSource->pBlendingParams->BlendType = BLEND_NONE;
            }
            else // for BlendType == BLEND_CONSTANT_SOURCE
            {
                pSource->pBlendingParams->BlendType = BLEND_SOURCE;
            }

            pSource->pBlendingParams->fAlpha    = 1.0f;
            wAlpha = 255;
        }
    }

    //-----------------------------------
    // Geometry adjustments for BOB DI
    //-----------------------------------
    // Use width and height that were used to setup surface state for plane 0
    pSurfaceEntry = pSurfaceEntries[0];
    dwSurfStateHt = pSurfaceEntry->dwHeight;
    dwSurfStateWd = pSurfaceEntry->dwWidth;

    // if 1:1 scaling and interlaced scaling or field weaving
    // do not adjust offsets since it uses Nearest sampling
    if (fScaleX == 1.0F &&
        fScaleY == 1.0F &&
        (pSource->bInterlacedScaling || pSource->bFieldWeaving))
    {
        fDiScaleY = 0.5f;
    }
    else
    {
        switch (pSource->SampleType)
        {
            case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:
            case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:
                fDiScaleY = 0.5f;
                // don't break
            case SAMPLE_SINGLE_TOP_FIELD:
                fOffsetY += 0.25f;
                break;

            case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:
            case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:
                fDiScaleY = 0.5f;
                // don't break
            case SAMPLE_SINGLE_BOTTOM_FIELD:
                fOffsetY -= 0.25f;
                break;

            case SAMPLE_PROGRESSIVE:
            default:
                fDiScaleY = 1.0f;
                break;
        }
    }

    // Normalize source co-ordinates using the width and height programmed
    // in surface state. step X, Y pre-rotated
    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (pSource->Rotation == VPHAL_ROTATION_IDENTITY    ||
        pSource->Rotation == VPHAL_ROTATION_180         ||
        pSource->Rotation == VPHAL_MIRROR_HORIZONTAL    ||
        pSource->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fStepX = ((pSource->rcSrc.right - pSource->rcSrc.left - fCropX) * 1.0f) /
                  ((pSource->rcDst.right - pSource->rcDst.left) > 0 ?
                   (pSource->rcDst.right - pSource->rcDst.left) : 1);
        fStepY = ((pSource->rcSrc.bottom - pSource->rcSrc.top - fCropY) * fDiScaleY) /
                  ((pSource->rcDst.bottom - pSource->rcDst.top) > 0 ?
                   (pSource->rcDst.bottom - pSource->rcDst.top) : 1);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        fStepX = ((pSource->rcSrc.right - pSource->rcSrc.left - fCropX) * 1.0f) /
                  ((pSource->rcDst.bottom - pSource->rcDst.top) > 0 ?
                   (pSource->rcDst.bottom - pSource->rcDst.top) : 1);
        fStepY = ((pSource->rcSrc.bottom - pSource->rcSrc.top - fCropY) * fDiScaleY) /
                  ((pSource->rcDst.right - pSource->rcDst.left) > 0 ?
                   (pSource->rcDst.right - pSource->rcDst.left) : 1);
    }

    // Source sampling coordinates based on rcSrc
    fOffsetX += (pSource->rcSrc.left + fCropX / 2);
    fOffsetY += (pSource->rcSrc.top + fCropY / 2) * fDiScaleY;

    DestRect = pSource->rcDst;
    if (pRenderingData->pTarget[1] != nullptr)
    {
        // Calculate non-rotated rectangle based on rotated rcDst in source surface
        switch (pSource->Rotation)
        {
            case VPHAL_ROTATION_90:
                DestRect.left     = pSource->rcDst.top;
                DestRect.top      = dwDestRectHeight - pSource->rcDst.right;
                DestRect.right    = pSource->rcDst.bottom;
                DestRect.bottom   = dwDestRectHeight - pSource->rcDst.left;
                break;
            case VPHAL_ROTATION_180:
                DestRect.left     = dwDestRectWidth - pSource->rcDst.right;
                DestRect.top      = dwDestRectHeight - pSource->rcDst.bottom;
                DestRect.right    = dwDestRectWidth - pSource->rcDst.left;
                DestRect.bottom   = dwDestRectHeight - pSource->rcDst.top;
                break;
            case VPHAL_ROTATION_270:
                DestRect.left     = dwDestRectWidth - pSource->rcDst.bottom;
                DestRect.top      = pSource->rcDst.left;
                DestRect.right    = dwDestRectWidth - pSource->rcDst.top;
                DestRect.bottom   = pSource->rcDst.right;
                break;
            case VPHAL_MIRROR_HORIZONTAL:
                DestRect.left     = dwDestRectWidth - pSource->rcDst.right;
                DestRect.top      = pSource->rcDst.top;
                DestRect.right    = dwDestRectWidth - pSource->rcDst.left;
                DestRect.bottom   = pSource->rcDst.bottom;
                break;
            case VPHAL_MIRROR_VERTICAL:
                DestRect.left     = pSource->rcDst.left;
                DestRect.top      = dwDestRectHeight - pSource->rcDst.bottom;
                DestRect.right    = pSource->rcDst.right;
                DestRect.bottom   = dwDestRectHeight - pSource->rcDst.top;
                break;
            case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
                DestRect.left     = pSource->rcDst.top;
                DestRect.top      = pSource->rcDst.left;
                DestRect.right    = pSource->rcDst.bottom;
                DestRect.bottom   = pSource->rcDst.right;
                break;
            case VPHAL_ROTATE_90_MIRROR_VERTICAL:
                DestRect.left     = dwDestRectWidth - pSource->rcDst.bottom;
                DestRect.top      = dwDestRectHeight - pSource->rcDst.right;
                DestRect.right    = dwDestRectWidth - pSource->rcDst.top;
                DestRect.bottom   = dwDestRectHeight - pSource->rcDst.left;
                break;
            case VPHAL_ROTATION_IDENTITY:
            default:
                break;
        } // switch

        fShiftX  -= DestRect.left - pRenderingData->ConstrictionOriginX;
        fShiftY  -= DestRect.top  - pRenderingData->ConstrictionOriginY;
    }
    else
    {
        switch (pSource->Rotation)
        {
            case VPHAL_ROTATION_IDENTITY:
                // Coordinate adjustment for render target coordinates (0,0)
                fShiftX  -= pSource->rcDst.left - pRenderingData->ConstrictionOriginX;
                fShiftY  -= pSource->rcDst.top  - pRenderingData->ConstrictionOriginY;
                break;
            case VPHAL_ROTATION_90:
                // Coordinate adjustment for 90 degree rotation
                fShiftX  -= (float)pSource->rcDst.top  - (float)pRenderingData->ConstrictionOriginY;
                fShiftY  -= (float)dwDestRectWidth -
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleX -
                            (float)pSource->rcDst.left - (float)pRenderingData->ConstrictionOriginX;
                break;
            case VPHAL_ROTATION_180:
                // Coordinate adjustment for 180 degree rotation
                fShiftX  -= (float)dwDestRectWidth -
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleX -
                            (float)pSource->rcDst.left - (float)pRenderingData->ConstrictionOriginX;
                fShiftY  -= (float)dwDestRectHeight -
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleY -
                            (float)pSource->rcDst.top  - (float)pRenderingData->ConstrictionOriginY;
                break;
            case VPHAL_ROTATION_270:
                // Coordinate adjustment for 270 degree rotation
                fShiftX  -= (float)dwDestRectHeight -
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleY -
                            (float)pSource->rcDst.top - (float)pRenderingData->ConstrictionOriginY;
                fShiftY  -= (float)pSource->rcDst.left  - (float)pRenderingData->ConstrictionOriginX;
                break;
            case VPHAL_MIRROR_HORIZONTAL:
                // Coordinate adjustment for horizontal mirroring
                fShiftX  -= (float)dwDestRectWidth -
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleX -
                            (float)pSource->rcDst.left - (float)pRenderingData->ConstrictionOriginX;
                fShiftY  -= pSource->rcDst.top  - pRenderingData->ConstrictionOriginY;
                break;
            case VPHAL_MIRROR_VERTICAL:
                // Coordinate adjustment for vertical mirroring
                fShiftX  -= pSource->rcDst.left - pRenderingData->ConstrictionOriginX;
                fShiftY  -= (float)dwDestRectHeight -
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleY -
                            (float)pSource->rcDst.top  - (float)pRenderingData->ConstrictionOriginY;
                break;
            case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:
                // Coordinate adjustment for rotating 90 and horizontal mirroring
                fShiftX  -= (float)pSource->rcDst.top  - (float)pRenderingData->ConstrictionOriginY;
                fShiftY  -= (float)pSource->rcDst.left  - (float)pRenderingData->ConstrictionOriginX;
                break;
            case VPHAL_ROTATE_90_MIRROR_VERTICAL:
            default:
                // Coordinate adjustment for rotating 90 and vertical mirroring
                fShiftX  -= (float)dwDestRectHeight -
                            (float)(pSource->rcSrc.right - pSource->rcSrc.left) * fScaleY -
                            (float)pSource->rcDst.top - (float)pRenderingData->ConstrictionOriginY;
                fShiftY  -= (float)dwDestRectWidth -
                            (float)(pSource->rcSrc.bottom - pSource->rcSrc.top) * fScaleX -
                            (float)pSource->rcDst.left - (float)pRenderingData->ConstrictionOriginX;
                break;
        } // switch
    }

    // Frame origins for the current layer
    fOriginX = (fOffsetX + fShiftX * fStepX) / dwSurfStateWd;
    fOriginY = (fOffsetY + fShiftY * fStepY) / dwSurfStateHt;

    // Adjust step
    if (pRenderingData->pConstriction)
    {
        fStepX *= pRenderingData->fConstrictionStepX;
        fStepY *= pRenderingData->fConstrictionStepY;
    }

    // Normalized block step for the current layer (block increment)
    fStepX /= dwSurfStateWd;
    fStepY /= dwSurfStateHt;

    // Clip source rectangle
    DestRect.left   = MOS_MIN(MOS_MAX(pTargetRect->left, DestRect.left  ),
                            pTargetRect->right);
    DestRect.right  = MOS_MIN(MOS_MAX(pTargetRect->left, DestRect.right ),
                            pTargetRect->right);
    DestRect.top    = MOS_MIN(MOS_MAX(pTargetRect->top , DestRect.top   ),
                            pTargetRect->bottom);
    DestRect.bottom = MOS_MIN(MOS_MAX(pTargetRect->top , DestRect.bottom),
                            pTargetRect->bottom);

    if (pRenderingData->pConstriction)
    {
        DestRect.left =
                (int)((DestRect.left   - pRenderingData->ConstrictionOriginX) /
                                         pRenderingData->fConstrictionStepX);
        DestRect.right =
                (int)((DestRect.right  - pRenderingData->ConstrictionOriginX) /
                                         pRenderingData->fConstrictionStepX);
        DestRect.top =
                (int)((DestRect.top    - pRenderingData->ConstrictionOriginY) /
                                         pRenderingData->fConstrictionStepY);
        DestRect.bottom =
                (int)((DestRect.bottom - pRenderingData->ConstrictionOriginY) /
                                         pRenderingData->fConstrictionStepY);
    }

    // Layer is outside the render target area
    if (DestRect.left == DestRect.right ||
        DestRect.top  == DestRect.bottom)
    {
        // layer is not visible - disable layer
        pSource->iLayerID = -1;
        goto finish;
    }

    // Set CURBE and INLINE data
    pStatic->DW08.DestinationRectangleWidth  = dwDestRectWidth;
    pStatic->DW08.DestinationRectangleHeight = dwDestRectHeight;

    switch (iLayer)
    {
        case 0:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer0 = pSource->Rotation;
            }
            else
            {
                pStatic->DW09.RotationMirrorMode    = pSource->Rotation;
            }
            pStatic->DW13.ColorFill_A                       = wAlpha;
            pStatic->DW16.HorizontalScalingStepRatioLayer0  = fStepX;
            pStatic->DW24.VerticalScalingStepRatioLayer0    = fStepY;
            pStatic->DW40.HorizontalFrameOriginLayer0       = fOriginX;
            pStatic->DW32.VerticalFrameOriginLayer0         = fOriginY;
            pInline->DW04.VideoXScalingStep                 = fStepX;

            // ChromasitingUOffset and ChromasitingVOffset are only for 3D Sampler use case
            if (m_need3DSampler)
            {
                fHorizgap = 0;
                fVertgap = 0;
                GetOffsetChromasiting(pSource,
                                      &fHorizgap,
                                      &fVertgap);
                if (IS_PL2_FORMAT(pSource->Format))
                {

                    pStatic->DW11.ChromasitingUOffset = (float)((0.5f / (pSource->dwWidth)) - fHorizgap);
                    pStatic->DW12.ChromasitingVOffset = (float)((1.0f / (pSource->dwHeight)) - fVertgap);
                }
                else if (pSource->Format == Format_YUY2)
                {
                    pStatic->DW11.ChromasitingUOffset = (float)((1.0f / (pSource->dwWidth)) - fHorizgap);
                    pStatic->DW12.ChromasitingVOffset = (float)((0.5f / (pSource->dwHeight)) - fVertgap);
                }
            }
            break;
        case 1:
            // if L0 and L1 have the same rotation, set for all layers.
            // L1 onwards must have the same rotation in a single rendering phase.
            // pStatic->DW09.RotationMirrorAllLayer is initialized to 0 by default
            // through RenderData init outside this function.
            if (!m_bSamplerSupportRotation)
            {
                if (pRenderingData->pLayers[0]->Rotation == pSource->Rotation)
                {
                    pStatic->DW09.RotationMirrorAllLayer    = 1;
                }
            }
            else // Gen9+ uses HW based Rotation
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer1 = pSource->Rotation;
            }
            pStatic->DW06.ConstantBlendingAlphaLayer1       = wAlpha;
            pStatic->DW17.HorizontalScalingStepRatioLayer1  = fStepX;
            pStatic->DW25.VerticalScalingStepRatioLayer1    = fStepY;
            pStatic->DW41.HorizontalFrameOriginLayer1       = fOriginX;
            pStatic->DW33.VerticalFrameOriginLayer1         = fOriginY;
            break;
        case 2:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer2 = pSource->Rotation;
            }
            pStatic->DW06.ConstantBlendingAlphaLayer2       = wAlpha;
            pStatic->DW18.HorizontalScalingStepRatioLayer2  = fStepX;
            pStatic->DW26.VerticalScalingStepRatioLayer2    = fStepY;
            pStatic->DW42.HorizontalFrameOriginLayer2       = fOriginX;
            pStatic->DW34.VerticalFrameOriginLayer2         = fOriginY;
            break;
        case 3:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer3 = pSource->Rotation;
            }
            pStatic->DW06.ConstantBlendingAlphaLayer3       = wAlpha;
            pStatic->DW19.HorizontalScalingStepRatioLayer3  = fStepX;
            pStatic->DW27.VerticalScalingStepRatioLayer3    = fStepY;
            pStatic->DW43.HorizontalFrameOriginLayer3       = fOriginX;
            pStatic->DW35.VerticalFrameOriginLayer3         = fOriginY;
            break;
        case 4:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer4 = pSource->Rotation;
            }
            pStatic->DW06.ConstantBlendingAlphaLayer4       = wAlpha;
            pStatic->DW20.HorizontalScalingStepRatioLayer4  = fStepX;
            pStatic->DW28.VerticalScalingStepRatioLayer4    = fStepY;
            pStatic->DW44.HorizontalFrameOriginLayer4       = fOriginX;
            pStatic->DW36.VerticalFrameOriginLayer4         = fOriginY;
            break;
        case 5:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer5 = pSource->Rotation;
            }
            pStatic->DW07.ConstantBlendingAlphaLayer5       = wAlpha;
            pStatic->DW21.HorizontalScalingStepRatioLayer5  = fStepX;
            pStatic->DW29.VerticalScalingStepRatioLayer5    = fStepY;
            pStatic->DW45.HorizontalFrameOriginLayer5       = fOriginX;
            pStatic->DW37.VerticalFrameOriginLayer5         = fOriginY;
            break;
        case 6:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer6 = pSource->Rotation;
            }
            pStatic->DW07.ConstantBlendingAlphaLayer6       = wAlpha;
            pStatic->DW22.HorizontalScalingStepRatioLayer6  = fStepX;
            pStatic->DW30.VerticalScalingStepRatioLayer6    = fStepY;
            pStatic->DW46.HorizontalFrameOriginLayer6       = fOriginX;
            pStatic->DW38.VerticalFrameOriginLayer6         = fOriginY;
            break;
        case 7:
            // Gen9+ uses HW based Rotation
            if (m_bSamplerSupportRotation)
            {
                pStatic->DW10.ObjKa2Gen9.RotationAngleofLayer7 = pSource->Rotation;
            }
            pStatic->DW07.ConstantBlendingAlphaLayer7       = wAlpha;
            pStatic->DW23.HorizontalScalingStepRatioLayer7  = fStepX;
            pStatic->DW31.VerticalScalingStepRatioLayer7    = fStepY;
            pStatic->DW47.HorizontalFrameOriginLayer7       = fOriginX;
            pStatic->DW39.VerticalFrameOriginLayer7         = fOriginY;
            break;
        default:
            VPHAL_RENDER_ASSERTMESSAGE("Invalid layer.");
            iResult = -1;
            goto finish;
    }

    Set3DSamplerStatus(pSource, (uint8_t)iLayer, pStatic);

    // Save rendering parameters, increment number of layers
    pRenderingData->pLayers[iLayer] = pSource;
    pRenderingData->iLayers++;

    pRenderingData->BbArgs.rcDst[iLayer] = DestRect;
    pRenderingData->BbArgs.Rotation[iLayer] = pSource->Rotation;
    pRenderingData->BbArgs.iLayers++;

    VPHAL_RENDER_NORMALMESSAGE("Layer %d, SamplerType:%d, Scaling Model %d,  SamplerIndex %d",
                               iLayer, SamplerType, pSource->ScalingMode, iSamplerID);
    iResult = 1;

finish:
    return iResult;
}

//!
//! \brief    Set Composite Render Target Layer
//! \details  Set Composite Render Target Layer, setup surface state and binding table
//! \param    [in] pRenderingData
//!           Pointer to Composite Rendering data
//! \param    [in] pCompParams
//!           Pointer to Composite parameters
//! \return   int32_t
//!           Return number of Surface State entries if successful, otherwise -1
//!
int32_t CompositeState::SetLayerRT(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    PVPHAL_COMPOSITE_PARAMS         pCompParams)
{
    MOS_STATUS                          eStatus;
    PRENDERHAL_INTERFACE                pRenderHal;
    PRENDERHAL_SURFACE                  pRenderHalSurface;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParams;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                             iSurfaceEntries, i;
    int32_t                             iBTentry = 0;
    uint32_t                            uTargetIndex;
    RENDERHAL_OFFSET_OVERRIDE           PlaneOffsetOverride;
    PRENDERHAL_OFFSET_OVERRIDE          pPlaneOffsetOverride;

    iSurfaceEntries = -1;

    VPHAL_RENDER_CHK_NULL(m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pRenderingData);
    VPHAL_RENDER_CHK_NULL(pCompParams);
    VPHAL_RENDER_ASSERT(pRenderingData->pTarget[0]->SurfType == SURF_OUT_RENDERTARGET);

    pRenderHal   = m_pRenderHal;

    // init surface parameters
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    SurfaceParams.MemObjCtl = m_SurfMemObjCtl.TargetSurfMemObjCtl;

    // Used for 32x32 Media walker kernel + Color fill kernel
    if (m_bFtrMediaWalker)
    {
        if (pRenderingData->pColorFill != nullptr &&
            pRenderingData->iLayers == 0 &&
            pRenderHal->pHwSizes->dwSizeMediaWalkerBlock == 32)
        {
            SurfaceParams.b32MWColorFillKern = true;
        }
    }

    uTargetIndex = 0;
    do
    {
        SetSurfaceCompressionParams(pRenderingData->pTarget[uTargetIndex], true);
        // Get surface state allocation parameters for RT (scaling mode, stride)
        SetSurfaceParams(
            pRenderingData->pTarget[uTargetIndex],
            &SurfaceParams);
        pRenderHalSurface = &pCompParams->RenderHalSurfaceTarget[uTargetIndex];
        VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonInitRenderHalSurface(pRenderingData->pTarget[uTargetIndex],
                                                                 pRenderHalSurface));
        pPlaneOffsetOverride = GetPlaneOffsetOverrideParam(
            pRenderHalSurface,
            &SurfaceParams,
            &PlaneOffsetOverride);

        // Setup surface state
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetupSurfaceState(
                                               pRenderHal,
                                               pRenderHalSurface,
                                               &SurfaceParams,
                                               &iSurfaceEntries,
                                               pSurfaceEntries,
                                               pPlaneOffsetOverride));
        VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonGetBackVpSurfaceParams(
            pRenderHalSurface,
            pRenderingData->pTarget[uTargetIndex]));

        // Setup Binding table entries
        if (pRenderingData->pTarget[1] == nullptr)
        {
            GetBindingIndex(pRenderingData->pTarget[0], &iBTentry);
        }
        else
        {
            // pTarget[0] will be secondary render target in dual output mode
            if (uTargetIndex == 0)
            {
                iBTentry = VPHAL_COMP_BTINDEX_RT_SECOND;
            }
            else
            {
                // pTarget[1] will be primary render target
                iBTentry = VPHAL_COMP_BTINDEX_RENDERTARGET;
                // set dual output mode
                if(m_bFtrMediaWalker)
                {
                    ((MEDIA_OBJECT_KA2_STATIC_DATA*)
                        &pRenderingData->WalkerStatic)->DW09.DualOutputMode = 1;
                }
                else
                {
                    pRenderingData->Static.DW09.DualOutputMode = 1;
                }
            }
        }
        for (i = 0; i < iSurfaceEntries; i++, iBTentry++)
        {
            VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
                                            pRenderHal,
                                            pRenderingData->iBindingTable,
                                            iBTentry,
                                            pSurfaceEntries[i]));
        }

        if (pRenderingData->iLayers == 0 &&
            pRenderingData->pColorFill == nullptr )
        {
            VPHAL_RENDER_ASSERTMESSAGE("Only Render Target is present, colorfill must be enabled.");
            goto finish;
        }

        uTargetIndex++;
    } while (uTargetIndex < VPHAL_MAX_TARGETS && pRenderingData->pTarget[uTargetIndex]);

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        iSurfaceEntries = -1;
    }
    return iSurfaceEntries;
}

//!
//! \brief    Get Output Surface Chroma sitting position for kernel
//! \param    [in] pTarget
//!           Pointer to Target Surface
//! \return   uint32_t
//!           Return chroma sitting position
//!
uint32_t CompositeState::GetOutputChromaSitting(
    PVPHAL_SURFACE                      pTarget)
{
    uint32_t dwChromaSitingLocation = CHROMA_SUBSAMPLING_TOP_LEFT;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pTarget);

    // If there is no DDI setting, we use the Horizontal Left Vertical Center as default for PL2 surface.
    if (pTarget->ChromaSiting == CHROMA_SITING_NONE)
    {
        // PL2 default to Horizontal Left, Vertical Center
        if (IS_PL2_FORMAT(pTarget->Format) || IS_PL2_FORMAT_UnAligned(pTarget->Format))
        {
            dwChromaSitingLocation = CHROMA_SUBSAMPLING_CENTER_LEFT;
        }
    }
    else
    {
        // PL2, 6 positions are avalibale
        if (IS_PL2_FORMAT(pTarget->Format) || IS_PL2_FORMAT_UnAligned(pTarget->Format))
        {
            // Horizontal Left
            if (pTarget->ChromaSiting & CHROMA_SITING_HORZ_LEFT)
            {
                if (pTarget->ChromaSiting & CHROMA_SITING_VERT_TOP)
                {
                    dwChromaSitingLocation = CHROMA_SUBSAMPLING_TOP_LEFT;
                }
                else if (pTarget->ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    dwChromaSitingLocation = CHROMA_SUBSAMPLING_CENTER_LEFT;
                }
                else if (pTarget->ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    dwChromaSitingLocation = CHROMA_SUBSAMPLING_BOTTOM_LEFT;
                }
            }
            // Horizontal Center
            else if (pTarget->ChromaSiting & CHROMA_SITING_HORZ_CENTER)
            {
                if (pTarget->ChromaSiting & CHROMA_SITING_VERT_TOP)
                {
                    dwChromaSitingLocation = CHROMA_SUBSAMPLING_TOP_CENTER;
                }
                else if (pTarget->ChromaSiting & CHROMA_SITING_VERT_CENTER)
                {
                    dwChromaSitingLocation = CHROMA_SUBSAMPLING_CENTER_CENTER;
                }
                else if (pTarget->ChromaSiting & CHROMA_SITING_VERT_BOTTOM)
                {
                    dwChromaSitingLocation = CHROMA_SUBSAMPLING_BOTTOM_CENTER;
                }
            }
        }
        else if (IS_PA_FORMAT(pTarget->Format))
        {
            // For PA surface, only (H Left, V Top) and (H Center, V top) are needed.
            if (pTarget->ChromaSiting & (CHROMA_SITING_HORZ_CENTER))
            {
                dwChromaSitingLocation = CHROMA_SUBSAMPLING_TOP_CENTER;
            }
        }
    }
finish:
    return dwChromaSitingLocation;
}

//!
//! \brief    Check whether parameters for composition valid or not.
//! \param    [in] CompositeParams
//!           Parameters for composition
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeState::IsCompositeParamsValid(
    const VPHAL_COMPOSITE_PARAMS& CompositeParams)
{
    if (CompositeParams.uSourceCount > VPHAL_COMP_MAX_LAYERS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid number of sources.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Calculate and set inline data size
//! \param    [in] pRenderingData
//!           pointer to render data
//! \param    [out] pStatic
//!           pointer to static data
//! \return   void
//!
int32_t CompositeState::CalculateInlineDataSize(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    MEDIA_OBJECT_KA2_STATIC_DATA    *pStatic)
{
    // Set inline pointer
    pStatic->DW07.PointerToInlineParameters = 7;

    // Set Inline Data Size
    switch (pRenderingData->iLayers)
    {
        case 0:
            // Case 0 is trued only for colorfill only cases.
            // Colorfill uses inverted layer 0 block mask to determine colorfill region.
        case 1:
        case 2:
        case 3:
            pRenderingData->iCmdInlineSize =  8 * sizeof(uint32_t);
            break;
        case 4:
            pRenderingData->iCmdInlineSize =  9 * sizeof(uint32_t);
            break;
        case 5:
            pRenderingData->iCmdInlineSize = 10 * sizeof(uint32_t);
            break;
        case 6:
            pRenderingData->iCmdInlineSize = 11 * sizeof(uint32_t);
            break;
        case 7:
            pRenderingData->iCmdInlineSize = 12 * sizeof(uint32_t);
            break;
        case 8:
            pRenderingData->iCmdInlineSize = 13 * sizeof(uint32_t);
            break;
        default:
            VPHAL_RENDER_ASSERTMESSAGE("%s, Invalid Number of Layers.");
            break;
    }
    return pRenderingData->iCmdInlineSize;
}

//!
//! \brief    Submit Composite states
//! \details  Submit Composite states, including load CSC matrix, set Inline data,
//!           set background color, load Palettes, set output format, load kernel, load
//!           curbe data, set sampler state, set VFE State params, and etc
//! \param    [in] pRenderingData
//!           Pointer to Composite state
//! \return   bool
//!           Return TURE if successful, otherwise false
//!
bool CompositeState::SubmitStates(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData)
{
    // States and objects
    PRENDERHAL_INTERFACE                pRenderHal;
    Kdll_State                          *pKernelDllState;   // Kernel DLL state
    Kdll_CacheEntry                     *pKernelEntry;      // Media kernel entry
    float                               pfCscMatrix[12];    // CSC matrix in floating point format
    int32_t                             piCscMatrix[12];    // CSC matrix in fixed point format

    PRENDERHAL_MEDIA_STATE              pMediaState;    // Media states
    MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic;        // Static parameters
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
    if(m_bFtrMediaWalker)
    {
        pStatic    = (MEDIA_OBJECT_KA2_STATIC_DATA*)&pRenderingData->WalkerStatic;
    }
    else
    {
        pStatic    = &pRenderingData->Static;
    }

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
            pMatrix->iProcampID < m_iMaxProcampEntries)
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

            pStatic->DW00.CscConstantC0  = *(pCoeff++);
            pStatic->DW00.CscConstantC1  = *(pCoeff++);
            pStatic->DW01.CscConstantC2  = *(pCoeff++);
            pStatic->DW01.CscConstantC3  = *(pCoeff++);
            pStatic->DW02.CscConstantC4  = *(pCoeff++);
            pStatic->DW02.CscConstantC5  = *(pCoeff++);
            pStatic->DW03.CscConstantC6  = *(pCoeff++);
            pStatic->DW03.CscConstantC7  = *(pCoeff++);
            pStatic->DW04.CscConstantC8  = *(pCoeff++);
            pStatic->DW04.CscConstantC9  = *(pCoeff++);
            pStatic->DW05.CscConstantC10 = *(pCoeff++);
            pStatic->DW05.CscConstantC11 = *pCoeff;
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("CSC matrix coefficient id is non-zero.");
            goto finish;
        }
    }

    if (pRenderingData->bCmFcEnable && m_bFtrCSCCoeffPatchMode)
    {
        MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

        SurfaceParams.Type          = pRenderHal->SurfaceTypeDefault;
        SurfaceParams.bRenderTarget = false;
        SurfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        SurfaceParams.bWidth16Align = false;
        SurfaceParams.MemObjCtl     = m_SurfMemObjCtl.InputSurfMemObjCtl;

        if (!Mos_ResourceIsNull(&m_CmfcCoeff.OsResource))
        {
            VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
                m_pRenderHal,
                &m_CmfcCoeff,
                &m_RenderHalCmfcCoeff,
                &SurfaceParams,
                pRenderingData->iBindingTable,
                VPHAL_COMP_BTINDEX_CSC_COEFF,
                false));
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("Null resource found");
            eStatus = MOS_STATUS_NULL_POINTER;
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

            if (dst_cspace == CSpace_None) // if color space is invlaid return false
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
            VpHal_CSC_8(&m_csDst, &Src, src_cspace, dst_cspace);

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

/*
|    |---------------------------------------------------------------------|
|    |                      Alpha fill mode table                          |
|    |---------------------------------------------------------------------|
|    |                      ALPHA_FILL_MODE_NONE                           |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |      Save_ARGB        |
|    |      No Alpha        |        Has Alpha     |Save_RGB(ALpha frm app)|
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
|    |                    ALPHA_FILL_MODE_OPAQUE                           |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |    Save_RGB(0xff)     |
|    |      No Alpha        |        Has Alpha     |    Save_RGB(0xff)     |
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
|    |                   ALPHA_FILL_MODE_BACKGROUND                        |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |  Save_RGB(BG Alpha)   |
|    |      No Alpha        |        Has Alpha     |  Save_RGB(BG Alpha)   |
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
|    |                  ALPHA_FILL_MODE_SOURCE_STREAM                      |
|    |---------------------------------------------------------------------|
|    |        Input         |         Output       |     Kernel used       |
|    |      Has Alpha       |        Has Alpha     |      Save_ARGB        |
|    |      No Alpha        |        Has Alpha     |    Save_RGB(0xff)     |
|    |      Has Alpha       |        No Alpha      |    Save_RGB(0xff)     |
|    |      No Alpha        |        No Alpha      |    Save_RGB(0xff)     |
|    |---------------------------------------------------------------------|
*/

    // Set output format
    if (IS_PA_FORMAT(pTarget->Format)  &&
        pTarget->Format != Format_Y410 &&
        pTarget->Format != Format_Y416)
    {
        VpHal_RndrSetYUVComponents(
            pTarget->Format,
            &(pStatic->DW15.DestinationPackedYOffset),
            &(pStatic->DW15.DestinationPackedUOffset),
            &(pStatic->DW15.DestinationPackedVOffset));
    }
    else if (pFilter->bFillOutputAlphaWithConstant && pRenderingData->pCompAlpha != nullptr)
    {
        switch (pRenderingData->pCompAlpha->AlphaMode)
        {
            case VPHAL_ALPHA_FILL_MODE_NONE:
                if (pFilter->format == Format_A8R8G8B8    ||
                    pFilter->format == Format_A8B8G8R8    ||
                    pFilter->format == Format_R10G10B10A2 ||
                    pFilter->format == Format_B10G10R10A2 ||
                    pFilter->format == Format_AYUV        ||
                    pFilter->format == Format_Y410        ||
                    pFilter->format == Format_Y416)
                {
                    pStatic->DW15.DestinationRGBFormat = (uint8_t)(0xff * pRenderingData->pCompAlpha->fAlpha);
                }
                else
                {
                    pStatic->DW15.DestinationRGBFormat = 0xff;
                }
                // For color fill only case, pass through alpha value
                if (pRenderingData->pColorFill && pRenderingData->iLayers == 0)
                {
                    pStatic->DW15.DestinationRGBFormat = ColorFill_A;
                }
                break;

            case VPHAL_ALPHA_FILL_MODE_BACKGROUND:
                pStatic->DW15.DestinationRGBFormat = ColorFill_A;
                break;

            // VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM case is hit when the input does not have alpha
            // So we set Opaque alpha channel.
            case VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM:
            case VPHAL_ALPHA_FILL_MODE_OPAQUE:
            default:
                pStatic->DW15.DestinationRGBFormat = 0xff;
                break;
        }
    }
    else
    {
        pStatic->DW15.DestinationRGBFormat = 0xff;
    }

    // Set flag to swap R and B in Save_RGB/ARGB if target format is Format_A8B8G8R8/Format_X8B8G8R8/Format_B10G10R10A2.
    // No need for RGBP/BGRP, since they are 3 plane format, kenel change the RB channel by different plane order
    pStatic->DW09.ChannelSwap = ((pTarget->Format == Format_A8B8G8R8) ||
                                 (pTarget->Format == Format_X8B8G8R8) ||
                                 (pTarget->Format == Format_B10G10R10A2)) ? 1 : 0;

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
        pStatic->DW16.HorizontalScalingStepRatioLayer0 /= fStepX;
        pStatic->DW17.HorizontalScalingStepRatioLayer1 /= fStepX;
        pStatic->DW18.HorizontalScalingStepRatioLayer2 /= fStepX;
        pStatic->DW19.HorizontalScalingStepRatioLayer3 /= fStepX;
        pStatic->DW20.HorizontalScalingStepRatioLayer4 /= fStepX;
        pStatic->DW21.HorizontalScalingStepRatioLayer5 /= fStepX;
        pStatic->DW22.HorizontalScalingStepRatioLayer6 /= fStepX;
        pStatic->DW23.HorizontalScalingStepRatioLayer7 /= fStepX;
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

    SubmitStatesFillGenSpecificStaticData(pRenderingData,
                                   pTarget,
                                   pStatic);

    if (m_bFtrMediaWalker)
    {
        iCurbeLength = sizeof(MEDIA_WALKER_KA2_STATIC_DATA);
    }
    else
    {
        // Set Static parameters
        iCurbeLength = pStatic->DW14.NLASEnable ?
            sizeof(MEDIA_OBJECT_KA2_STATIC_DATA) -
            sizeof(MEDIA_OBJECT_NLAS_INLINE_DATA) :
            sizeof(MEDIA_OBJECT_KA2_STATIC_DATA);
    }

    iCurbeOffset = pRenderHal->pfnLoadCurbeData(
        pRenderHal,
        pMediaState,
        pStatic,
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

//!
//! \brief    Search for the best match BB according to the Composition BB arguments
//! \param    [in] pBatchBufferTable
//!           Pointer to the BB table to be searched
//! \param    [in] pInputBbParams
//!           Pointer to the BB params required for the best match
//! \param    [in] iBbSize
//!           the BB size required for the best match
//! \param    [out] ppBatchBuffer
//!           Pointer to the addr of the best matched BB, pointer to nullptr if there's
//!           no available matched BB
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeState::GetBestMatchBB(
    PVPHAL_BATCH_BUFFER_TABLE     pBatchBufferTable,
    PVPHAL_BATCH_BUFFER_PARAMS    pInputBbParams,
    int32_t                       iBbSize,
    PMHW_BATCH_BUFFER             *ppBatchBuffer)
{
    PMHW_BATCH_BUFFER             pBbEntry;          // 2nd level BBs array entry
    PMHW_BATCH_BUFFER             pBestMatch;        // Best match for BB allocation
    PVPHAL_BATCH_BUFFER_PARAMS    pSearchBbParams;   // Search BB parameters
    PVPHAL_BB_COMP_ARGS           pCompBbArgs;       // 2nd level buffer rendering arguments
    PVPHAL_BB_COMP_ARGS           pSearchBbArgs;     // Search BB comp parameters
    int32_t                       i;
    int32_t                       iCallID;
    int32_t                       iBbCount;
    MOS_STATUS                    eStatus;

    pBestMatch  = nullptr;
    pCompBbArgs = &pInputBbParams->BbArgs.CompositeBB;
    iCallID     = pInputBbParams->iCallID;
    eStatus     = MOS_STATUS_UNKNOWN;

    iBbCount = *pBatchBufferTable->piBatchBufferCount;
    pBbEntry = pBatchBufferTable->pBatchBufferHeader;

    for (i = iBbCount; i > 0; i--, pBbEntry++)
    {
        // Must contain valid Compositing BB Argument set, must have adequate size,
        // cannot reuse buffers from same call ID
        pSearchBbParams = (PVPHAL_BATCH_BUFFER_PARAMS)pBbEntry->pPrivateData;

        if (!pSearchBbParams                                      ||
            pBbEntry->iSize           < iBbSize                   ||
            pSearchBbParams->iCallID == iCallID                   ||
            pSearchBbParams->iType   != VPHAL_BB_TYPE_COMPOSITING ||
            pSearchBbParams->iSize   != sizeof(VPHAL_BB_COMP_ARGS))
        {
            continue;
        }

        // Must match Media ID, StepX, full blocks, different Call ID
        pSearchBbArgs = &(pSearchBbParams->BbArgs.CompositeBB);

        if (pSearchBbArgs->iMediaID    != pCompBbArgs->iMediaID ||  // != Media ID
            pSearchBbArgs->fStepX      != pCompBbArgs->fStepX   ||  // != Step X
            pSearchBbArgs->bSkipBlocks != pCompBbArgs->bSkipBlocks) // != Skip Blocks
        {
            continue;
        }

        // Target rectangle must match
        if (memcmp(&pSearchBbArgs->rcOutput, &pCompBbArgs->rcOutput, sizeof(RECT)))
        {
            continue;
        }

        // BB must contain same or more layers than input BB
        if (pSearchBbArgs->iLayers < pCompBbArgs->iLayers)
        {
            continue;
        }

        // Compare each layer, ignore layers that are not present in the input
        if (memcmp(&pSearchBbArgs->rcDst, &pCompBbArgs->rcDst, pCompBbArgs->iLayers * sizeof(RECT)))
        {
            continue;
        }

        // Compare each layer rotation, ignore layers that are not present in the input
        if (memcmp(&pSearchBbArgs->Rotation, &pCompBbArgs->Rotation, pCompBbArgs->iLayers * sizeof(VPHAL_ROTATION)))
        {
            continue;
        }

        // for AVS/Bi-Linear Scaling, NLAS enable or not
        if (pSearchBbArgs->bEnableNLAS != pCompBbArgs->bEnableNLAS)
        {
            continue;
        }

        // NLAS parameters must match when it's enabled
        if (pCompBbArgs->bEnableNLAS &&
            memcmp(&pSearchBbArgs->NLASParams, &pCompBbArgs->NLASParams, sizeof(VPHAL_NLAS_PARAMS)))
        {
            continue;
        }

        // Match -> reuse the BB regardless of the running state
        pBestMatch = pBbEntry;
        ((PVPHAL_BATCH_BUFFER_PARAMS)pBestMatch->pPrivateData)->bMatch = true;

        break;
    }

    *ppBatchBuffer = pBestMatch;
    eStatus        = MOS_STATUS_SUCCESS;
    return eStatus;
}

//!
//! \brief    Calculate Media Object size
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \return   int32_t
//!           Return the size of Media Object
//!
int32_t CompositeState::CalculateMediaObjectSize(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData)
{
    MOS_UNUSED(pRenderingData);
    int32_t size = 0;

    size += m_pRenderHal->pMhwRenderInterface->GetMediaObjectCmdSize();
    size += sizeof(MEDIA_OBJECT_KA2_INLINE_DATA);

    return size;
}

//!
//! \brief    Allocate Composite BatchBuffer
//! \details  Allocate Composite BatchBuffer, search from existing BBs for a match. If
//!           none, allocate new BB
//! \param    [in] PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData
//!           Pointer to Rendering Data
//! \param    [out] PMHW_BATCH_BUFFER * ppBatchBuffer
//!           Pointer to the addr of the available BB. Pointer to nullptr if there's no
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeState::AllocateBuffer(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
    PMHW_BATCH_BUFFER                   *ppBatchBuffer)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    VPHAL_BATCH_BUFFER_TABLE            BatchBufferTable;
    VPHAL_BATCH_BUFFER_PARAMS           InputBbParams;
    int32_t                             iBbSize;
    int32_t                             iMobjSize;
    MOS_STATUS                          eStatus;

    eStatus      = MOS_STATUS_SUCCESS;
    pRenderHal   = m_pRenderHal;

    iMobjSize = CalculateMediaObjectSize(pRenderingData);
    iBbSize   = iMobjSize * pRenderingData->iBlocksX * pRenderingData->iBlocksY;

    iBbSize = iBbSize + pRenderHal->pMhwMiInterface->GetMiBatchBufferEndCmdSize();;

    InputBbParams.iSize              = sizeof(VPHAL_BB_COMP_ARGS);
    InputBbParams.iType              = VPHAL_BB_TYPE_COMPOSITING;
    InputBbParams.iCallID            = m_iCallID;
    InputBbParams.BbArgs.CompositeBB = pRenderingData->BbArgs;

    BatchBufferTable.pBatchBufferHeader = m_BatchBuffer;
    BatchBufferTable.pBbParamsHeader    = m_BufferParam;
    BatchBufferTable.iBbCountMax        = VPHAL_COMP_BUFFERS_MAX;
    BatchBufferTable.piBatchBufferCount = &m_iBatchBufferCount;

    VPHAL_RENDER_CHK_STATUS(VpHal_RenderAllocateBB(
                  &BatchBufferTable,
                  &InputBbParams,
                  iBbSize,
                  pRenderHal,
                  ppBatchBuffer));

    // Some app had memory overrun when generating the AI44/IA44 sample contents.
    // As result, the batch buffer was trashed and causes hardware hang (TDR).
    // Adding this solution to always regenerate the media objects for AI44
    // and IA44.
    if (pRenderingData->iLayers == 1                         &&
        (pRenderingData->pLayers[0]->Format == Format_AI44   ||
         pRenderingData->pLayers[0]->Format == Format_IA44))
    {
        ((PVPHAL_BATCH_BUFFER_PARAMS)(*ppBatchBuffer)->pPrivateData)->bMatch = false;
        (*ppBatchBuffer)->iCurrent = 0;
    }

finish:
    return eStatus;
}

//!
//! \brief    Render Composite BatchBuffer
//! \details  Render Composite BatchBuffer, setup Media Object header and inline data
//! \param    [in] pBatchBuffer
//!           Pointer to BatchBuffer
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \return   bool
//!           Return true if successful, otherwise false
//!
bool CompositeState::RenderBuffer(
    PMHW_BATCH_BUFFER               pBatchBuffer,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    PMHW_MI_INTERFACE                   pMhwMiInterface;
    MOS_STATUS                          eStatus;
    PVPHAL_BB_COMP_ARGS                 pBbArgs;
    MEDIA_OBJECT_KA2_STATIC_DATA        *pStatic;
    MEDIA_OBJECT_KA2_INLINE_DATA        *pInline;
    MEDIA_OBJECT_NLAS_INLINE_DATA       *pInlineNLAS;
    VPHAL_COMPOSITE_MO_INLINE_DATA      MOInlineData;
    MHW_MEDIA_OBJECT_PARAMS             MediaObjectParams;
    uint16_t                            wMask;
    uint16_t                            wCombinedMask;
    PRECT                               rcDst;
    int32_t                             x, y, dx, dy;
    int32_t                             xl, xr, yt, yb;
    bool                                bResult;
    float                               fSrcX[8];
    uint32_t                            applyRotation;
    uint32_t                            targetIndex;

    bResult             = false;
    pRenderHal          = m_pRenderHal;
    pMhwMiInterface     = pRenderHal->pMhwMiInterface;
    MOS_ZeroMemory(fSrcX, sizeof(float) * 8);

    if (pRenderHal->pfnLockBB(pRenderHal, pBatchBuffer) != MOS_STATUS_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to lock batch buffer.");
        goto finish;
    }

    pBbArgs   = &pRenderingData->BbArgs;
    rcDst     = pBbArgs->rcDst;

    MOS_ZeroMemory(&MediaObjectParams, sizeof(MediaObjectParams));
    MediaObjectParams.dwInterfaceDescriptorOffset   = pRenderingData->iMediaID;
    MediaObjectParams.dwInlineDataSize              =
        pRenderingData->iCmdInlineSize + pRenderingData->iNLASInlineSize;

    MOInlineData.NLASInline     = g_cInit_MEDIA_OBJECT_NLAS_INLINE_DATA;
    MOInlineData.KA2Inline      = pRenderingData->Inline;
    pInline                     = &MOInlineData.KA2Inline;
    pInlineNLAS                 = &MOInlineData.NLASInline;
    pStatic                     = &pRenderingData->Static;

    // Traverse blocks in the render target area. If destination is not 16x16
    // pixel aligned, the top-most row and left-most column will launch MO cmds
    // starting from non-16x16 aligned dest coords. But the rest of the MO cmds
    // are aligned to 16x16 pixel boundary. Worst-case we would process 15 pixel
    // rows top and columns left twice.
    // In case of dual render targets, all horizontal and vertical
    // settings should be set according to non-rotated output.
    if (pRenderingData->pTarget[1] == nullptr ||
        m_bKernelSupportDualOutput)
    {
        applyRotation   = 0xFFFFFFFF;
        targetIndex     = 0;
    }
    else
    {
        applyRotation   = 0;
        targetIndex     = 1;
    }

    y  = pBbArgs->rcOutput.top;
    for (dy = 0; dy < pRenderingData->iBlocksY; dy++)
    {
        pInline->DW00.DestinationBlockVerticalOrigin = y;

        wCombinedMask = (pBbArgs->bSkipBlocks) ? 0x0000 : 0xffff;
        switch (pRenderingData->iLayers)
        {
            case 8:
                yt = rcDst[7].top    - y;
                yb = rcDst[7].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;

                // Gen9+ Possible HW Rotation, kernels not available yet.
                // DW09 bits 2:0 indicate RotationMirrorMode,
                // bit 3 indicates if RotationMirrorMode applies to all layers,
                // =1 means to apply for all layers, =0 means only for Layer0
                // In case of RatationMirrorAllLayer(bit3) = 0, all layers from
                // layer 1 onwards must be no rotation in a single rendering phase.
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW12,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 7:
                yt = rcDst[6].top    - y;
                yb = rcDst[6].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW11,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 6:
                yt = rcDst[5].top    - y;
                yb = rcDst[5].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW10,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 5:
                yt = rcDst[4].top    - y;
                yb = rcDst[4].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW09,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 4:
                yt = rcDst[3].top    - y;
                yb = rcDst[3].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW08,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 3:
                yt = rcDst[2].top    - y;
                yb = rcDst[2].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW03,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 2:
                yt = rcDst[1].top    - y;
                yb = rcDst[1].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask(
                    (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                    pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW02,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
            case 1:
                yt = rcDst[0].top    - y;
                yb = rcDst[0].bottom - y;
                yt = MOS_MIN(MOS_MAX(0, yt), VPHAL_COMP_BLOCK_HEIGHT);
                yb = MOS_MIN(MOS_MAX(0, yb), VPHAL_COMP_BLOCK_HEIGHT);
                wMask = (0xffff << yt) & ((0x0001 << yb) - 1);
                wCombinedMask |= wMask;
                SetInline16x16Mask((VPHAL_ROTATION)
                    (pStatic->DW09.RotationMirrorMode & applyRotation),
                    (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW01,
                    wMask,
                    VPHAL_VERTICAL_16X16BLOCK_MASK);
                break;
            case 0:
                // This case is true only for colorfill only cases. Force block mask to zero.
                pInline->DW01.VerticalBlockCompositeMaskLayer0 = 0;
                break;
        }

        // Skip row if no blocks are flagged for rendering
        if (!wCombinedMask)
        {
            y += VPHAL_COMP_BLOCK_HEIGHT;
            y -= y % VPHAL_COMP_BLOCK_HEIGHT;
            continue;
        }

        x = pBbArgs->rcOutput.left;

        // get the horizontal origin - the second term is necessary to ensure
        // accurate computation of the starting value of fSrcX when the output
        // rectangle does not start at 0 (for example, split-screen demo mode)
        switch (pRenderingData->iLayers)
        {
            case 8:
                fSrcX[7] = pStatic->DW47.HorizontalFrameOriginLayer7 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 7:
                fSrcX[6] = pStatic->DW46.HorizontalFrameOriginLayer6 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 6:
                fSrcX[5] = pStatic->DW45.HorizontalFrameOriginLayer5 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 5:
                fSrcX[4] = pStatic->DW44.HorizontalFrameOriginLayer4 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 4:
                fSrcX[3] = pStatic->DW43.HorizontalFrameOriginLayer3 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 3:
                fSrcX[2] = pStatic->DW42.HorizontalFrameOriginLayer2 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 2:
                fSrcX[1] = pStatic->DW41.HorizontalFrameOriginLayer1 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
            case 1:
                fSrcX[0] = pStatic->DW40.HorizontalFrameOriginLayer0 +
                    ((float)(x) / (float)(pRenderingData->pTarget[targetIndex]->dwWidth));
                break;
            case 0:
            default:
                fSrcX[0] = fSrcX[1] = fSrcX[2] = fSrcX[3] = 0;
                fSrcX[4] = fSrcX[5] = fSrcX[6] = fSrcX[7] = 0;
                break;
        }

        for (dx = 0; dx < pRenderingData->iBlocksX; dx++)
        {
            pInline->DW00.DestinationBlockHorizontalOrigin = x;

            wCombinedMask = (pBbArgs->bSkipBlocks) ? 0x0000 : 0xffff;
            switch (pRenderingData->iLayers)
            {
                case 8:
                    xl = rcDst[7].left  - x;
                    xr = rcDst[7].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW12,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 7:
                    xl = rcDst[6].left  - x;
                    xr = rcDst[6].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW11,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 6:
                    xl = rcDst[5].left  - x;
                    xr = rcDst[5].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW10,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 5:
                    xl = rcDst[4].left  - x;
                    xr = rcDst[4].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW09,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 4:
                    xl = rcDst[3].left  - x;
                    xr = rcDst[3].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW08,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 3:
                    xl = rcDst[2].left  - x;
                    xr = rcDst[2].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW03,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 2:
                    xl = rcDst[1].left  - x;
                    xr = rcDst[1].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask(
                        (VPHAL_ROTATION)((pStatic->DW09.RotationMirrorMode *
                        pStatic->DW09.RotationMirrorAllLayer) & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW02,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                case 1:
                    xl = rcDst[0].left  - x;
                    xr = rcDst[0].right - x;
                    xl = MOS_MIN(MOS_MAX(0, xl), VPHAL_COMP_BLOCK_WIDTH);
                    xr = MOS_MIN(MOS_MAX(0, xr), VPHAL_COMP_BLOCK_WIDTH);
                    wMask = (0xffff << xl) & ((0x0001 << xr) - 1);
                    wCombinedMask |= wMask;
                    SetInline16x16Mask((VPHAL_ROTATION)
                        (pStatic->DW09.RotationMirrorMode & applyRotation),
                        (PVPHAL_16X16BLOCK_COMPOSITE_MASK)&pInline->DW01,
                        wMask,
                        VPHAL_HORIZONTAL_16X16BLOCK_MASK);
                    break;
                case 0:
                    // This case is true only for colorfill only cases. Force block mask to zero.
                    pInline->DW01.HorizontalBlockCompositeMaskLayer0 = 0;
                    break;
            }

            ModifyInlineData(pBbArgs, pRenderingData, pStatic, pInline, pInlineNLAS, x, fSrcX);

            if (wCombinedMask)
            {
                if (pBbArgs->bEnableNLAS)
                {
                    MediaObjectParams.pInlineData = &MOInlineData.NLASInline;
                }
                else
                {
                    MediaObjectParams.pInlineData = &MOInlineData.KA2Inline;
                }
                VPHAL_RENDER_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddMediaObject(
                    nullptr,
                    pBatchBuffer,
                    &MediaObjectParams));
            }

            x += VPHAL_COMP_BLOCK_WIDTH;
            x -= x % VPHAL_COMP_BLOCK_WIDTH;
        }

        y += VPHAL_COMP_BLOCK_HEIGHT;
        y -= y % VPHAL_COMP_BLOCK_HEIGHT;
    }

    VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiBatchBufferEnd(nullptr, pBatchBuffer));

    if (pRenderHal->pfnUnlockBB(pRenderHal, pBatchBuffer) != MOS_STATUS_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to unlock batch buffer.");
        bResult = false;
        goto finish;
    }

    bResult = true;

finish:
    if (pBatchBuffer && pBatchBuffer->bLocked)
    {
        // Only happens in Error cases
        VPHAL_RENDER_ASSERT(0);
        eStatus = pRenderHal->pfnUnlockBB(pRenderHal, pBatchBuffer);
        VPHAL_RENDER_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        bResult = false;
    }
    return bResult;
}

//!
//! \brief    Judge whether  media walker pattern  will be vertical or not
//! \details  if input layer is one , and input is linear format and rotation 90
//!           or 270 is needed then the media walker pattern should be vertical
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \return   bool
//!           Return true if vertical media pattern used, otherwise false
//!
bool CompositeState::MediaWalkerVertical(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData)
{
    PVPHAL_SURFACE  pSource;
    bool            bVertical     = false;

    pSource     = pRenderingData->pLayers[0];

    if (pRenderingData->iLayers == 1 &&
        pSource->TileType == MOS_TILE_LINEAR &&
       (pSource->Rotation == VPHAL_ROTATION_90 || pSource->Rotation == VPHAL_ROTATION_270))
    {
        bVertical = true;
    }

    return bVertical;
}

//!
//! \brief    Modify MediaWalker Static Data
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \return   void
//!
void CompositeState::ModifyMediaWalkerStaticData(
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData)
{
    MOS_UNUSED(pRenderingData);
}

//!
//! \brief    Render Composite BatchBuffer
//! \details  Render Composite BatchBuffer, fill Walker static data fields and set walker
//!           cmd params
//! \param    [in] pBatchBuffer
//!           Pointer to BatchBuffer
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \param    [in] pWalkerParams
//!           Pointer to Walker parameters
//! \return   bool
//!           Return true if successful, otherwise false
//!
bool CompositeState::RenderBufferMediaWalker(
    PMHW_BATCH_BUFFER               pBatchBuffer,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    PMHW_WALKER_PARAMS              pWalkerParams)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    MEDIA_WALKER_KA2_STATIC_DATA        *pWalkerStatic;
    PVPHAL_BB_COMP_ARGS                 pBbArgs;
    bool                                bResult;
    int32_t                             iLayers;
    uint32_t                            uiMediaWalkerBlockSize;
    uint32_t*                           pdwDestXYTopLeft;
    uint32_t*                           pdwDestXYBottomRight;
    RECT                                AlignedRect;
    bool                                bVerticalPattern;

    MOS_UNUSED(pBatchBuffer);

    bResult          = false;
    pRenderHal       = m_pRenderHal;
    bVerticalPattern = false;
    pBbArgs          = &pRenderingData->BbArgs;
    pWalkerStatic    = &pRenderingData->WalkerStatic;

    VPHAL_RENDER_ASSERT(m_bFtrMediaWalker && !pBatchBuffer);

    pdwDestXYTopLeft     = (uint32_t*)(&pWalkerStatic->DW48);
    pdwDestXYBottomRight = (uint32_t*)(&pWalkerStatic->DW56);

    // GRF7.0-7, GRF8.0-7
    for (iLayers = 0;
         iLayers < pBbArgs->iLayers;
         iLayers++, pdwDestXYBottomRight++, pdwDestXYTopLeft++)
    {
        *pdwDestXYTopLeft     = (pBbArgs->rcDst[iLayers].top    << 16 ) |
                                 pBbArgs->rcDst[iLayers].left;
        *pdwDestXYBottomRight = ((pBbArgs->rcDst[iLayers].bottom - 1) << 16 ) |
                                 (pBbArgs->rcDst[iLayers].right - 1);
    }

    // GRF 9.0-4
    pWalkerStatic->DW64.MainVideoXScalingStepLeft                   =
        (float)pRenderingData->Inline.DW04.VideoXScalingStep;
    pWalkerStatic->DW65.VideoStepDeltaForNonLinearRegion            = 0;
    pWalkerStatic->DW66.StartofLinearScalingInPixelPositionC0       = 0;
    pWalkerStatic->DW66.StartofRHSNonLinearScalingInPixelPositionC1 = 0;
    pWalkerStatic->DW67.MainVideoXScalingStepCenter                 = 0;
    pWalkerStatic->DW68.MainVideoXScalingStepRight                  = 0;

    if (pRenderingData->pTarget[1] == nullptr)
    {
        pWalkerStatic->DW69.DestHorizontalBlockOrigin                  =
             (uint16_t)pRenderingData->pTarget[0]->rcDst.left;
        pWalkerStatic->DW69.DestVerticalBlockOrigin                    =
             (uint16_t)pRenderingData->pTarget[0]->rcDst.top;

        AlignedRect   = pRenderingData->pTarget[0]->rcDst;
    }
    else
    {
        // Horizontal and Vertical base on non-rotated in case of dual output
        pWalkerStatic->DW69.DestHorizontalBlockOrigin                   =
            (uint16_t)pRenderingData->pTarget[1]->rcDst.left;
        pWalkerStatic->DW69.DestVerticalBlockOrigin                     =
             (uint16_t)pRenderingData->pTarget[1]->rcDst.top;

         AlignedRect   = pRenderingData->pTarget[1]->rcDst;
    }

    ModifyMediaWalkerStaticData(pRenderingData);

    // Get media walker kernel block size
    uiMediaWalkerBlockSize = pRenderHal->pHwSizes->dwSizeMediaWalkerBlock;
    bVerticalPattern       = MediaWalkerVertical(pRenderingData);

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

    pWalkerParams->dwGlobalLoopExecCount                = 1;

    if (uiMediaWalkerBlockSize == 32)
    {
        pWalkerParams->ColorCountMinusOne = 3;
    }
    else
    {
        pWalkerParams->ColorCountMinusOne = 0;
    }

    if (AlignedRect.left !=0 || AlignedRect.top !=0)
    {
        // if the rect starts from any other macro  block other than the first
        // then the global resolution should be the whole frame and the global
        // start should be the rect start.
        pWalkerParams->GlobalResolution.x           =
            (AlignedRect.right / uiMediaWalkerBlockSize);
        pWalkerParams->GlobalResolution.y           =
            (AlignedRect.bottom / uiMediaWalkerBlockSize);
    }
    else
    {
        pWalkerParams->GlobalResolution.x           = pRenderingData->iBlocksX;
        pWalkerParams->GlobalResolution.y           = pRenderingData->iBlocksY;
    }

    pWalkerParams->GlobalStart.x                =
        (AlignedRect.left / uiMediaWalkerBlockSize);
    pWalkerParams->GlobalStart.y                =
        (AlignedRect.top / uiMediaWalkerBlockSize);

    pWalkerParams->GlobalOutlerLoopStride.x     = pRenderingData->iBlocksX;
    pWalkerParams->GlobalOutlerLoopStride.y     = 0;

    pWalkerParams->GlobalInnerLoopUnit.x        = 0;
    pWalkerParams->GlobalInnerLoopUnit.y        = pRenderingData->iBlocksY;

    pWalkerParams->BlockResolution.x            = pRenderingData->iBlocksX;
    pWalkerParams->BlockResolution.y            = pRenderingData->iBlocksY;

    pWalkerParams->LocalStart.x                 = 0;
    pWalkerParams->LocalStart.y                 = 0;

    if(bVerticalPattern)
    {
        pWalkerParams->LocalOutLoopStride.x         = 1;
        pWalkerParams->LocalOutLoopStride.y         = 0;

        pWalkerParams->LocalInnerLoopUnit.x         = 0;
        pWalkerParams->LocalInnerLoopUnit.y         = 1;

        pWalkerParams->dwLocalLoopExecCount         = pRenderingData->iBlocksX - 1;
        pWalkerParams->LocalEnd.x                   = 0;
        pWalkerParams->LocalEnd.y                   = pRenderingData->iBlocksY - 1;
    }
    else
    {
        pWalkerParams->LocalOutLoopStride.x         = 0;
        pWalkerParams->LocalOutLoopStride.y         = 1;

        pWalkerParams->LocalInnerLoopUnit.x         = 1;
        pWalkerParams->LocalInnerLoopUnit.y         = 0;

        pWalkerParams->dwLocalLoopExecCount         = pRenderingData->iBlocksY - 1;
        pWalkerParams->LocalEnd.x                   = pRenderingData->iBlocksX - 1;
        pWalkerParams->LocalEnd.y                   = 0;
    }

    bResult = true;

    return bResult;
}

//!
//! \brief    Render GpGpu Walker Buffer
//! \details  Render GpGpu Walker Buffer, fill Walker static data fields and set walker
//!           cmd params
//! \param    [in] pBatchBuffer
//!           Pointer to BatchBuffer
//! \param    [in] pRenderingData
//!           Pointer to Rendering Data
//! \param    [in] pWalkerParams
//!           Pointer to Walker parameters
//! \return   bool
//!           Return true if successful, otherwise false
//!
bool CompositeState::RenderBufferComputeWalker(
    PMHW_BATCH_BUFFER               pBatchBuffer,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    PMHW_GPGPU_WALKER_PARAMS        pWalkerParams)
{
    PRENDERHAL_INTERFACE                pRenderHal;
    MEDIA_WALKER_KA2_STATIC_DATA        *pWalkerStatic;
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
    pWalkerStatic    = &pRenderingData->WalkerStatic;

    VPHAL_RENDER_ASSERT(m_bFtrMediaWalker && !pBatchBuffer);

    pdwDestXYTopLeft     = (uint32_t*)(&pWalkerStatic->DW48);
    pdwDestXYBottomRight = (uint32_t*)(&pWalkerStatic->DW56);

    // GRF7.0-7, GRF8.0-7
    for (iLayers = 0;
         iLayers < pBbArgs->iLayers;
         iLayers++, pdwDestXYBottomRight++, pdwDestXYTopLeft++)
    {
        *pdwDestXYTopLeft     = (pBbArgs->rcDst[iLayers].top    << 16 ) |
                                 pBbArgs->rcDst[iLayers].left;
        *pdwDestXYBottomRight = ((pBbArgs->rcDst[iLayers].bottom - 1) << 16 ) |
                                 (pBbArgs->rcDst[iLayers].right - 1);
    }

    // GRF 9.0-4
    pWalkerStatic->DW64.MainVideoXScalingStepLeft                   =
        (float)pRenderingData->Inline.DW04.VideoXScalingStep;
    pWalkerStatic->DW65.VideoStepDeltaForNonLinearRegion            = 0;
    pWalkerStatic->DW66.StartofLinearScalingInPixelPositionC0       = 0;
    pWalkerStatic->DW66.StartofRHSNonLinearScalingInPixelPositionC1 = 0;
    pWalkerStatic->DW67.MainVideoXScalingStepCenter                 = 0;
    pWalkerStatic->DW68.MainVideoXScalingStepRight                  = 0;

    if (pRenderingData->pTarget[1] == nullptr)
    {
        pWalkerStatic->DW69.DestHorizontalBlockOrigin                  =
             (uint16_t)pRenderingData->pTarget[0]->rcDst.left;
        pWalkerStatic->DW69.DestVerticalBlockOrigin                    =
             (uint16_t)pRenderingData->pTarget[0]->rcDst.top;

        AlignedRect   = pRenderingData->pTarget[0]->rcDst;
    }
    else
    {
        // Horizontal and Vertical base on non-rotated in case of dual output
        pWalkerStatic->DW69.DestHorizontalBlockOrigin                   =
            (uint16_t)pRenderingData->pTarget[1]->rcDst.left;
        pWalkerStatic->DW69.DestVerticalBlockOrigin                     =
             (uint16_t)pRenderingData->pTarget[1]->rcDst.top;

         AlignedRect   = pRenderingData->pTarget[1]->rcDst;
    }

    ModifyMediaWalkerStaticData(pRenderingData);

    // Get media walker kernel block size
    uiMediaWalkerBlockSize = pRenderHal->pHwSizes->dwSizeMediaWalkerBlock;

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

//!
//! \brief    Calculate Composite parameter and render data
//! \param    [in] pCompParams
//!           Pointer to Composite parameters. For both input and output.
//! \param    [in] pSource
//!           Pointer to surface. For both input and output.
//! \param    [in] pRenderingData
//!           Pointer to Composite RenderData. For both input and output.
//! \param    [out] pbColorfill
//!           Pointer to color fill flag.
//! \return   void
//!
void CompositeState::CalculateRenderData(
    PVPHAL_COMPOSITE_PARAMS         pCompParams,
    PVPHAL_SURFACE                  pSource,
    PVPHAL_RENDERING_DATA_COMPOSITE pRenderingData,
    bool*                           pbColorfill)
{
    // Check if Colorfill is required
    if ((pCompParams->pColorFillParams != nullptr) &&
        (!RECT1_CONTAINS_RECT2(pSource->rcDst, pCompParams->Target[0].rcDst)))
    {
        VPHAL_RENDER_NORMALMESSAGE("bColorfill enabled");
        *pbColorfill = true;
    }

    // Set HDC Direct Write Flag
    if (pCompParams->uSourceCount == 1                                          &&  // Single Layer
        pSource->ScalingMode == VPHAL_SCALING_AVS                               &&  // AVS
        !pSource->bInterlacedScaling                                            &&  // No interlace scaling
        IS_PA_FORMAT(pSource->Format)                                           &&  // Input Format is Packed
        (IS_PA_FORMAT(pCompParams->Target[0].Format) ||
         pCompParams->Target[0].Format == Format_NV12)                          &&  // Output format is Packed or 4:2:0
        pSource->Rotation == VPHAL_ROTATION_IDENTITY                            &&  // No Rotation
        !(*pbColorfill)                                                         &&  // No Colorfill
        pSource->pLumaKeyParams == nullptr                                      &&  // No Lumakey
        pSource->pProcampParams == nullptr                                      &&  // No Procamp
        pSource->pBlendingParams == nullptr                                     &&  // No Blending
        m_bKernelSupportHdcDW)                                                      // if HDC direct write is supported
    {
        VPHAL_RENDER_NORMALMESSAGE("bHdcDwEnable enabled");
        pRenderingData->bHdcDwEnable = true;
    }
}

//!
//! \brief    Perform multiple layer composite operation in one phase
//! \details  Perform multiple layer composite operation in one phase(scaling, blending,
//!           lumakey, CSC)
//! \param    [in,out] pCompParams
//!           Pointer to Composite parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS CompositeState::RenderPhase(
    PVPHAL_COMPOSITE_PARAMS pCompParams)
{
    VPHAL_RENDERING_DATA_COMPOSITE  RenderingData           = {};
    PMOS_INTERFACE                  pOsInterface            = nullptr;
    PRENDERHAL_INTERFACE            pRenderHal              = nullptr;
    PMHW_BATCH_BUFFER               pBatchBuffer            = nullptr;
    Kdll_State                      *pKernelDllState        = nullptr;
    Kdll_CacheEntry                 *pKernelEntry           = nullptr;
    Kdll_CSC_Params                 *pCscParams             = nullptr;;
    Kdll_CSC_Matrix                 *pMatrix                = nullptr;;
    Kdll_Procamp                    *pProcamp               = nullptr;;
    int32_t                         iFilterSize             = 0;
    Kdll_FilterEntry                *pFilter                = nullptr;;
    uint32_t                        dwKernelHash            = 0;
    MOS_STATUS                      eStatus                 = MOS_STATUS_UNKNOWN;
    PVPHAL_SURFACE                  pSource                 = nullptr;
    PVPHAL_SURFACE                  *pSourceArray           = nullptr;
    PRENDERHAL_MEDIA_STATE          pMediaState             = nullptr;
    int32_t                         iBindingTableID         = 0;
    int32_t                         iLayer                  = 0;
    int32_t                         iRes                    = 0;
    MHW_WALKER_PARAMS               WalkerParams            = {};
    PMHW_WALKER_PARAMS              pWalkerParams           = nullptr;
    MHW_GPGPU_WALKER_PARAMS         ComputeWalkerParams     = {};
    PMHW_GPGPU_WALKER_PARAMS        pComputeWalkerParams    = nullptr;
    bool                            bKernelEntryUpdate      = false;
    bool                            bColorfill              = false;

    VPHAL_RENDER_ASSERT(pCompParams);
    VPHAL_RENDER_ASSERT(m_pOsInterface);
    VPHAL_RENDER_ASSERT(m_pOsInterface->osCpInterface);
    VPHAL_RENDER_ASSERT(m_pRenderHal);
    VPHAL_RENDER_ASSERT(m_pKernelDllState);

    pOsInterface    = m_pOsInterface;
    pRenderHal      = m_pRenderHal;
    pKernelDllState = m_pKernelDllState;

    //VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_STAGE(VPHAL_DBG_STAGE_COMP);

    // Check whether composition parameters are valid.
    VPHAL_RENDER_CHK_STATUS(IsCompositeParamsValid(*pCompParams));

    //============================
    // Allocate states for rendering
    //============================
    // Prepare for rendering
    VPHAL_RENDER_CHK_STATUS(RenderInit(
        pCompParams,
        &RenderingData));

    // Allocate and reset media state
    RenderingData.pMediaState = pMediaState =
                            pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_COMP);
    VPHAL_RENDER_CHK_NULL(pMediaState);

    // Allocate and reset SSH instance
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

    // Allocate and reset BT
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
               pRenderHal,
               &iBindingTableID));

    RenderingData.iBindingTable = iBindingTableID;

    //===============================
    // Setup params for each layer
    //===============================
    pSourceArray = pCompParams->pSource;
    for (iLayer = 0; iLayer < (int32_t)pCompParams->uSourceCount; iLayer++, pSourceArray++)
    {
        // Get next source
        pSource = *pSourceArray;

        // Check Scaling mode for 3D Sampler use case
        if (m_need3DSampler && pSource->ScalingMode == VPHAL_SCALING_AVS)
        {
            VPHAL_RENDER_NORMALMESSAGE("Modify ScalingMode to BILINREA from AVS due to 3D Sampler enabled");
            pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
        }

        // Set scaling mode for the current layer
        if (pCompParams->pConstriction)
        {
            pSource->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
        else
        {
            if (pSource->ScalingMode == VPHAL_SCALING_AVS &&
                pSource->SurfType == SURF_IN_PRIMARY)
            {
                m_bChromaUpSampling = VpHal_IsChromaUpSamplingNeeded(
                                                    pSource,
                                                    &pCompParams->Target[0]);
            }
            else if (m_need3DSampler                                       &&
                     pSource->ScalingMode != VPHAL_SCALING_AVS             &&
                     pSource->SurfType == SURF_IN_PRIMARY                  &&
                     ((IS_PL2_FORMAT(pSource->Format) && iLayer == 0)      || // when 3D sampler been used, PL2 chromasitting kernel does not support sub-layer chromasitting
                     pSource->Format == Format_YUY2))
            {
                m_bChromaUpSampling   = VpHal_IsChromaUpSamplingNeeded(
                                                    pSource,
                                                    &pCompParams->Target[0]);
                m_bChromaDownSampling = VpHal_IsChromaDownSamplingNeeded(
                                                    pSource,
                                                    &pCompParams->Target[0]);
            }
            else
            {
                m_bChromaUpSampling   = false;
                m_bChromaDownSampling = false;
            }

            SetScalingMode(
                pSource,
                pCompParams->uSourceCount);
        }

        // Get Allocation index of source for rendering
        if (pOsInterface->pfnRegisterResource(
                pOsInterface,
                &pSource->OsResource,
                false,
                true) != MOS_STATUS_SUCCESS)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // The parameter YOffset of surface state should be
        // a multiple of 4 when the input is accessed in field mode.For interlaced NV12
        // input, if its height is not a multiple of 4, the YOffset of UV plane will not
        // be a multiple of 4.So under this condition, we treat it as progressive input.
        if (VpHal_RndrCommonIsAlignmentWANeeded(
                pSource,
                pOsInterface->CurrentGpuContextOrdinal))
        {
            pSource->SampleType         = SAMPLE_PROGRESSIVE;
            pSource->bInterlacedScaling = false;
        }

        // If there is no scaling used for interlace surface on 10bit PA formats, force to progressive due to no supoort for kernel.
        if (pSource->bInterlacedScaling &&
            (pSource->rcSrc.right - pSource->rcSrc.left) == (pSource->rcDst.right - pSource->rcDst.left) &&
            (pSource->rcSrc.bottom - pSource->rcSrc.top) == (pSource->rcDst.bottom - pSource->rcDst.top) &&
            (pSource->Format == Format_Y210 || pSource->Format == Format_Y410))
        {
            pSource->SampleType = SAMPLE_PROGRESSIVE;
            pSource->bInterlacedScaling = false;
        }

        // Get Allocation index of reference for rendering
        if (pSource->bFieldWeaving && pSource->pBwdRef)
        {
            if (pOsInterface->pfnRegisterResource(
                    pOsInterface,
                    &pSource->pBwdRef->OsResource,
                    false,
                    true) != MOS_STATUS_SUCCESS)
            {
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }
        }

        CalculateRenderData(pCompParams, pSource, &RenderingData, &bColorfill);

        // Setup rendering parameters for current layer
        iRes = SetLayer(
                    &RenderingData,
                    pSource,
                    iLayer,
                    pCompParams);
        if (iRes < 0)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to set layer parameters.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // Report mode
        if (pSource->SurfType == SURF_IN_PRIMARY)
        {
            SetReporting(pSource);
        }
    }

    // Get allocation index for render target Setup Surface States for Render Target
    for (iLayer = 0; iLayer < (int32_t)pCompParams->uTargetCount; iLayer++)
    {
        if (pOsInterface->pfnRegisterResource(
                pOsInterface,
                &pCompParams->Target[iLayer].OsResource,
                true,
                true) != MOS_STATUS_SUCCESS)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }

    // Setup rendering parameters for RT layer(s)
    iRes = SetLayerRT(
               &RenderingData,
               pCompParams);
    if (iRes < 0)
    {
         VPHAL_RENDER_ASSERTMESSAGE("Failed to set Render Target.");
         eStatus = MOS_STATUS_UNKNOWN;
         goto finish;
    }

    //============================
    // Create search filter for Dynamic Linking
    //============================
    pFilter = m_SearchFilter;
    MOS_ZeroMemory(pFilter, sizeof(m_SearchFilter));
    pCompParams->bComputeWlaker = pRenderHal->bComputeContextInUse;

    if (!BuildFilter(
             pCompParams,
             pFilter,
             &iFilterSize))
    {
        VPHAL_RENDER_ASSERTMESSAGE("Failed to create filter description.");
        eStatus = MOS_STATUS_UNIMPLEMENTED;
        goto finish;
    }

    //Log for debug
    for (int32_t i = 0; i < iFilterSize; i++)
    {
        Kdll_FilterEntry *pTempFilter = (pFilter + i);
        
        if (pTempFilter == nullptr)
            continue;

        VPHAL_RENDER_NORMALMESSAGE("Kernel Search Filter %d: layer %d, format %d, cspace %d, \
                                   bEnableDscale %d, bIsDitherNeeded %d, chromasiting %d, colorfill %d, dualout %d, \
                                   lumakey %d, procamp %d, RenderMethod %d, sampler %d, samplerlumakey %d ", 
                                   i, pTempFilter->layer, pTempFilter->format, pTempFilter->cspace, 
                                   pTempFilter->bEnableDscale, pTempFilter->bIsDitherNeeded, 
                                   pTempFilter->chromasiting, pTempFilter->colorfill,  pTempFilter->dualout, 
                                   pTempFilter->lumakey, pTempFilter->procamp, pTempFilter->RenderMethod, pTempFilter->sampler, pTempFilter->samplerlumakey);
    }

    //============================
    // KERNEL SEARCH
    //============================
    dwKernelHash = KernelDll_SimpleHash(pFilter, iFilterSize * sizeof(Kdll_FilterEntry));
    pKernelEntry = KernelDll_GetCombinedKernel(pKernelDllState, pFilter, iFilterSize, dwKernelHash);

    if (pKernelEntry)
    {
        pCscParams = pKernelEntry->pCscParams;
        pMatrix    = &pCscParams->Matrix[pCscParams->MatrixID[0]];
        pKernelDllState->colorfill_cspace = pKernelEntry->colorfill_cspace;

        if ((pMatrix->iProcampID != DL_PROCAMP_DISABLED) &&
            (pMatrix->iProcampID < m_iMaxProcampEntries))
        {
            pProcamp           = &(m_Procamp[pMatrix->iProcampID]);
            bKernelEntryUpdate = (pProcamp->iProcampVersion != pMatrix->iProcampVersion) ? true : false;
        }
    }

    if (!pKernelEntry || bKernelEntryUpdate)
    {
        Kdll_SearchState *pSearchState = &m_KernelSearch;

        // Remove kernel entry from kernel caches
        if (bKernelEntryUpdate)
        {
            KernelDll_ReleaseHashEntry(&(pKernelDllState->KernelHashTable), pKernelEntry->wHashEntry);
            KernelDll_ReleaseCacheEntry(&(pKernelDllState->KernelCache), pKernelEntry);
        }

        // Setup kernel search
        pKernelDllState->pfnStartKernelSearch(
            pKernelDllState,
            pSearchState,
            pFilter,
            iFilterSize,
            1);

        // Search kernel
        if (!pKernelDllState->pfnSearchKernel(pKernelDllState, pSearchState))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to find a kernel.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // Build kernel
        if (!pKernelDllState->pfnBuildKernel(pKernelDllState, pSearchState))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to build kernel.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // Load resulting kernel into kernel cache
        pKernelEntry = KernelDll_AddKernel(
                           pKernelDllState,
                           pSearchState,
                           pFilter,
                           iFilterSize,
                           dwKernelHash);

        if (!pKernelEntry)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to store kernel in local cache.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }
    else
    {
        VPHAL_RENDER_NORMALMESSAGE("Use previous kernel list.");
    }

    RenderingData.bCmFcEnable  = pKernelDllState->bEnableCMFC ? true : false;

    RenderingData.bAlphaCalculateEnable = pCompParams->bAlphaCalculateEnable;

    RenderingData.pKernelEntry = pKernelEntry;
    RenderingData.pProcamp     = m_Procamp;

    //============================
    // Return RT Primaries to the app
    //============================
    if (pFilter[iFilterSize - 1].layer == Layer_RenderTarget)
    {
        pSource = &pCompParams->Target[0];
        switch (pFilter[iFilterSize - 1].cspace)
        {
            case CSpace_xvYCC709:
                pSource->ExtendedGamut = true;
                pSource->ColorSpace    = CSpace_BT709;
                break;
            case CSpace_BT709:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT709;
                break;
            case CSpace_BT709_FullRange:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT709_FullRange;
                break;
            case CSpace_xvYCC601:
                pSource->ExtendedGamut = true;
                pSource->ColorSpace    = CSpace_BT601;
                break;
            case CSpace_BT601:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT601;
                break;
            case CSpace_BT601_FullRange:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT601_FullRange;
                break;
            case CSpace_BT2020:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT2020;
            case CSpace_BT2020_FullRange:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT2020_FullRange;
            case CSpace_BT2020_RGB:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT2020_RGB;
            case CSpace_BT2020_stRGB:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_BT2020_stRGB;
            default:
                pSource->ExtendedGamut = false;
                pSource->ColorSpace    = CSpace_sRGB;
                break;
        }
    }

    if (m_bFtrMediaWalker && (!m_bFtrComputeWalker))
    {
        pBatchBuffer  = nullptr;
        pWalkerParams = &WalkerParams;

        MOS_ZeroMemory(&WalkerParams, sizeof(WalkerParams));

        // calculates media object walker static data fields
        if (!RenderBufferMediaWalker(
                 pBatchBuffer,
                 &RenderingData,
                 &WalkerParams))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to render media walker batch.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // Send Media states for compositing
        if (!SubmitStates(&RenderingData))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to submit compositing states.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // The VfeScoreboard is set after Vphal_CompSubmitStates calls pRenderHal->pfnSetVfeStateParams()
        pWalkerParams->UseScoreboard  = pRenderHal->VfeScoreboard.ScoreboardEnable;
        pWalkerParams->ScoreboardMask = pRenderHal->VfeScoreboard.ScoreboardMask;
    }
    else if (m_bFtrComputeWalker)
    {
        pBatchBuffer         = nullptr;
        pWalkerParams        = nullptr;
        pComputeWalkerParams = &ComputeWalkerParams;

        MOS_ZeroMemory(&ComputeWalkerParams, sizeof(ComputeWalkerParams));

        // calculates media object walker static data fields
        if (!RenderBufferComputeWalker(
                 pBatchBuffer,
                 &RenderingData,
                 &ComputeWalkerParams))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to render media walker batch.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // Send Media states for compositing
        if (!SubmitStates(&RenderingData))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to submit compositing states.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }
    else
    {
        // Send Media states for compositing
        if (!SubmitStates(&RenderingData))
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to submit compositing states.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }

        // Get a valid batch buffer (find a match if possible)
        VPHAL_RENDER_CHK_STATUS(AllocateBuffer(&RenderingData, &pBatchBuffer));

        // No match was found - render a new batch buffer
        if (!((PVPHAL_BATCH_BUFFER_PARAMS)pBatchBuffer->pPrivateData)->bMatch)
        {
            if (!RenderBuffer(
                     pBatchBuffer,
                     &RenderingData))
            {
                VPHAL_RENDER_ASSERTMESSAGE("Failed to render batch buffers.");
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }
        }

        // Set CallID to avoid BB reuse in the same call
        ((PVPHAL_BATCH_BUFFER_PARAMS)pBatchBuffer->pPrivateData)->iCallID = m_iCallID;
    }

    // Enable extra PIPE_CONTROL in command buffer for CMFC Coeff Surface update
    if (RenderingData.bCmFcEnable)
    {
        pRenderHal->bCmfcCoeffUpdate  = true;
        pRenderHal->pCmfcCoeffSurface = &m_CmfcCoeff.OsResource;
    }
    else
    {
        pRenderHal->bCmfcCoeffUpdate  = false;
        pRenderHal->pCmfcCoeffSurface = nullptr;
    }

    VPHAL_DBG_STATE_DUMPPER_DUMP_GSH(pRenderHal);
    VPHAL_DBG_STATE_DUMPPER_DUMP_SSH(pRenderHal);
    VPHAL_DBG_STATE_DUMPPER_DUMP_BATCH_BUFFER(pRenderHal, pBatchBuffer);

    VPHAL_RENDER_CHK_STATUS(VpHal_RndrSubmitCommands(
        pRenderHal,
        pBatchBuffer,
        m_bNullHwRenderComp,
        pWalkerParams,
        pComputeWalkerParams,
        &m_StatusTableUpdateParams,
        kernelCombinedFc,
        m_KernelSearch.KernelCount,
        m_KernelSearch.KernelID,
        m_bLastPhase));

finish:
    // clean rendering data
    CleanRenderingData(&RenderingData);
    pRenderHal->bCmfcCoeffUpdate  = false;
    pRenderHal->pCmfcCoeffSurface = nullptr;
    return eStatus;
}

//!
//! \brief    Build filter description for dynamic linking
//! \details  Build filter description(render method, current layer, layer format, layer
//!           rotation, layer colorspace, sampling mode, scaling mode, luma key, blending,
//!           colorfill, procamp, CSC) for dynamic linking
//!           parameters
//! \param    [in] pCompParams
//!           Pointer to Composite parameters
//! \param    [out] pFilter
//!           Pointer to first filter entry
//! \param    [out] piFilterSize
//!           Pointer to filter size
//! \return   bool
//!           Return true if successful, otherwise false
//!
bool CompositeState::BuildFilter(
    PVPHAL_COMPOSITE_PARAMS         pCompParams,
    PKdll_FilterEntry               pFilter,
    int32_t*                        piFilterSize)
{
    PVPHAL_SURFACE              pSrc;
    VPHAL_CSPACE                cspace_main;
    int32_t                     iMaxFilterSize;
    bool                        bColorFill, bLumaKey;
    int32_t                     i;
    PRECT                       pTargetRect;
    RENDERHAL_SURFACE           RenderHalSurface;
    bool                        bNeed;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_ASSERT(pCompParams);
    VPHAL_RENDER_ASSERT(pFilter);
    VPHAL_RENDER_ASSERT(piFilterSize);

    cspace_main    = CSpace_sRGB;                   // Default colorspace
    *piFilterSize  = 0;
    iMaxFilterSize = DL_MAX_SEARCH_FILTER_SIZE - 1; // Save one entry for Render Target
    pTargetRect    = &(pCompParams->Target[0].rcDst);

    // Initialize ColorFill flag
    bColorFill = (pCompParams->pColorFillParams != nullptr);

    for (i = 0; (i < (int)pCompParams->uSourceCount) && (iMaxFilterSize > 0); i++)
    {
        pSrc = pCompParams->pSource[i];

        //--------------------------------
        // Skip non-visible layers
        //--------------------------------
        if (pSrc->iLayerID < 0)
        {
            continue;
        }

        //--------------------------------
        // Composition path does not support conversion from BT2020 RGB to BT2020 YUV, BT2020->BT601/BT709, BT601/BT709 -> BT2020
        //--------------------------------
        if (IS_COLOR_SPACE_BT2020_RGB(pSrc->ColorSpace)   &&
            IS_COLOR_SPACE_BT2020_YUV(pCompParams->Target[0].ColorSpace))  //BT2020 RGB->BT2020 YUV
        {
            eStatus = MOS_STATUS_UNIMPLEMENTED;
        }
        else if (IS_COLOR_SPACE_BT2020(pSrc->ColorSpace) &&
                 !IS_COLOR_SPACE_BT2020(pCompParams->Target[0].ColorSpace)) //BT2020->BT601/BT709
        {
            eStatus = MOS_STATUS_UNIMPLEMENTED;
        }
        else if (!IS_COLOR_SPACE_BT2020(pSrc->ColorSpace) &&
                 IS_COLOR_SPACE_BT2020(pCompParams->Target[0].ColorSpace))  //BT601/BT709 -> BT2020
        {
            eStatus = MOS_STATUS_UNIMPLEMENTED;
        }

        if (eStatus == MOS_STATUS_UNIMPLEMENTED)
        {
            goto finish;
        }

        //--------------------------------
        // Set render method
        //--------------------------------
        pFilter->RenderMethod = m_bFtrMediaWalker ? RenderMethod_MediaObjectWalker : RenderMethod_MediaObject;

        //--------------------------------
        // Set CSC coefficient setting method for CoeffID_0
        //--------------------------------
        pFilter->SetCSCCoeffMode = m_bFtrCSCCoeffPatchMode ? SetCSCCoeffMethod_Patch : SetCSCCoeffMethod_Curbe;

        //--------------------------------
        // Set current layer
        //--------------------------------
        pFilter->layer = g_cSurfaceType_Layer[pSrc->SurfType];

        //--------------------------------
        // Set layer format
        //--------------------------------
        pFilter->format = pSrc->Format;

        // On G8, NV12 format needs the width and Height to be a multiple of 4 for both
        // 3D sampler and 8x8 sampler; G75 needs the width of NV12 input surface to be
        // a multiple of 4 for 3D sampler; G9 does not has such restriction; to simplify the
        // implementation, we enable 2 plane NV12 for all of the platform when the width
        // or Height is not a multiple of 4. Here to set the filter format in order to select
        // the PL2 kernel when building the combined kernel.
        VPHAL_RENDER_CHK_STATUS(VpHal_RndrCommonInitRenderHalSurface(pSrc, &RenderHalSurface));
        bNeed = m_pRenderHal->pfnIs2PlaneNV12Needed(
                m_pRenderHal,
                &RenderHalSurface,
                RENDERHAL_SS_BOUNDARY_SRCRECT) ? true : false;
        bNeed |= IsNV12SamplerLumakeyNeeded(pSrc, m_pRenderHal) && i;
        if (bNeed)
        {
            if (pFilter->format == Format_NV12)
            {
                pFilter->format = Format_NV12_UnAligned;
            }
            else if (pFilter->format == Format_P208)
            {
                pFilter->format = Format_P208_UnAligned;
            }
            else if (pFilter->format == Format_NV11)
            {
                pFilter->format = Format_NV11_UnAligned;
            }
            else if (pFilter->format == Format_PL2)
            {
                pFilter->format = Format_PL2_UnAligned;
            }
        }

        // Y_Uoffset(Height*2 + Height/2) of RENDERHAL_PLANES_YV12 define Bitfield_Range(0, 13) on gen9+.
        // The max value is 16383. So use PL3 kernel to avoid out of range when Y_Uoffset is larger than 16383.
        // Use PL3 plane to avoid YV12 blending issue with DI enabled and U channel shift issue with not 4-aligned height
        if ((pFilter->format   == Format_YV12)           &&
            (pSrc->ScalingMode != VPHAL_SCALING_AVS)     &&
            (pSrc->bIEF        != true)                  &&
            (pSrc->SurfType    != SURF_OUT_RENDERTARGET) &&
            m_pRenderHal->bEnableYV12SinglePass          &&
            !pSrc->pDeinterlaceParams                    &&
            !pSrc->bInterlacedScaling                    &&
            MOS_IS_ALIGNED(pSrc->dwHeight, 4)            &&
            ((pSrc->dwHeight * 2 + pSrc->dwHeight / 2) < RENDERHAL_MAX_YV12_PLANE_Y_U_OFFSET_G9))
        {
            pFilter->format = Format_YV12_Planar;
        }

        if (pFilter->format == Format_A8R8G8B8 ||
            pFilter->format == Format_X8R8G8B8 ||
            pFilter->format == Format_A8B8G8R8 ||
            pFilter->format == Format_X8B8G8R8 ||
            pFilter->format == Format_R5G6B5)
        {
            pFilter->format = Format_RGB;
        }

        //--------------------------------
        // Set layer rotation
        //--------------------------------
        pFilter->rotation = pSrc->Rotation;

        //--------------------------------
        // Set layer color space
        //--------------------------------
        // Source is palletized, leave CSC to software (driver)
        if (IS_PAL_FORMAT(pFilter->format))
        {
            pFilter->cspace = CSpace_Any;
        }
        // Source is YUV or RGB, set primaries
        else
        {
            pFilter->cspace = pSrc->ColorSpace;
        }

        // Save color space of main video
        if (pSrc->SurfType == SURF_IN_PRIMARY)
        {
            cspace_main = pFilter->cspace;
        }

        //--------------------------------
        // Set sampling mode
        //--------------------------------
        bLumaKey = (pSrc->pLumaKeyParams != nullptr);

        // Progressive main video (except RGB format) or for RGB10, use AVS
        if (pSrc->bUseSampleUnorm)
        {
            pFilter->sampler = (pSrc->bInterlacedScaling || pSrc->bFieldWeaving) ? Sample_iScaling : Sample_Scaling;
        }
        else
        {
            pFilter->sampler = (pSrc->ScalingMode == VPHAL_SCALING_AVS && !IsBobDiEnabled(pSrc)) ?
                (pSrc->bInterlacedScaling ? Sample_iScaling_AVS : Sample_Scaling_AVS) :
                (pSrc->bInterlacedScaling || pSrc->bFieldWeaving) ? Sample_iScaling_034x : Sample_Scaling_034x;
        }

        // When input format is Format_R10G10B10A2/Format_B10G10R10A2/Y410(kernel regards Y410 as Format_R10G10B10A2)
        // Dscale kernel should be used
        if (pSrc->Format == Format_R10G10B10A2 ||
            pSrc->Format == Format_B10G10R10A2 ||
            pSrc->Format == Format_Y410        ||
            pSrc->Format == Format_Y416)
        {
            pFilter->bEnableDscale = true;
        }
        else
        {
            pFilter->bEnableDscale = false;
        }

        if (m_bFtrComputeWalker)
        {
            pFilter->bWaEnableDscale = true;
        }
        else
        {
            pFilter->bWaEnableDscale = MEDIA_IS_WA(m_pWaTable, WaEnableDscale);
        }

        //--------------------------------
        // Set Luma key
        //--------------------------------
        if (bLumaKey)
        {
            pFilter->lumakey = LumaKey_True;
            pFilter->samplerlumakey = pSrc->bUseSamplerLumakey ? LumaKey_True : LumaKey_False;
        }
        else
        {
            pFilter->lumakey = LumaKey_False;
            pFilter->samplerlumakey = LumaKey_False;
        }

        //--------------------------------
        // Select function
        //--------------------------------
        if (pSrc->pBlendingParams != nullptr)
        {
            switch (pSrc->pBlendingParams->BlendType)
            {
                case BLEND_SOURCE:
                    if (IS_ALPHA4_FORMAT(pSrc->Format))
                    {
                        pFilter->process = Process_SBlend_4bits;
                    }
                    else
                    {
                        pFilter->process = Process_SBlend;
                    }
                    break;

                case BLEND_PARTIAL:
                    pFilter->process = Process_PBlend;
                    break;

                case BLEND_CONSTANT:
                    pFilter->process = Process_CBlend;
                    break;

                case BLEND_CONSTANT_SOURCE:
                    pFilter->process = Process_CSBlend;
                    break;

                case BLEND_CONSTANT_PARTIAL:
                    pFilter->process = Process_CPBlend;
                    break;

                case BLEND_NONE:
                default:
                    pFilter->process = Process_Composite;
                    break;
            }
        }
        else
        {
            pFilter->process = Process_Composite;
        }

        if (pFilter->samplerlumakey && pFilter->process != Process_Composite)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Invalid kll processing for sampler lumakey! Sampler lumakey can only work with composition.");
            pFilter->samplerlumakey = LumaKey_False;
        }

        //--------------------------------
        // Set color fill
        //--------------------------------
        if (*piFilterSize == 0 &&
            (bLumaKey ||
             (bColorFill && (!RECT1_CONTAINS_RECT2(pSrc->rcDst, pCompParams->Target[0].rcDst))) ||
             (!pCompParams->bForceSkipColorFill &&
             ((pFilter->process == Process_PBlend) ||
              (pFilter->process == Process_CBlend) ||
              (pFilter->process == Process_SBlend) ||
              (pFilter->process == Process_CSBlend)))))
        {
            pFilter->colorfill = ColorFill_True;
        }
        else
        {
            pFilter->colorfill = ColorFill_False;
        }

        //--------------------------------
        // Set Procamp parameters
        //--------------------------------
        if (pSrc->pProcampParams && pSrc->pProcampParams->bEnabled)
        {
            pFilter->procamp = 0;
        }
        else
        {
            pFilter->procamp = DL_PROCAMP_DISABLED;
        }

        //--------------------------------
        // Set chromasiting parameters
        //--------------------------------
        pFilter->chromasiting = DL_CHROMASITING_DISABLE;
        if (pSrc->bChromaSiting)
        {
            pFilter->chromasiting = 0 ;
        }

        //--------------------------------
        // reset CSC
        //--------------------------------
        pFilter->matrix = DL_CSC_DISABLED;

        // Update filter
        pFilter++;
        (*piFilterSize)++;
        iMaxFilterSize--;
    }

    //-----------------------------------------
    // Set Render Target parameters
    //-----------------------------------------
    if (pCompParams->uTargetCount == 2)
    {
        pFilter->dualout = true;
    }
    pSrc = &pCompParams->Target[0];
    pFilter->RenderMethod    = m_bFtrMediaWalker ? RenderMethod_MediaObjectWalker : RenderMethod_MediaObject;
    pFilter->SetCSCCoeffMode = m_bFtrCSCCoeffPatchMode ? SetCSCCoeffMethod_Patch : SetCSCCoeffMethod_Curbe;
    pFilter->layer    = Layer_RenderTarget;
    pFilter->format   = pSrc->Format;
    pFilter->tiletype = pSrc->TileType;
    pFilter->sampler  = Sample_None;
    pFilter->process  = Process_None;
    pFilter->procamp  = DL_PROCAMP_DISABLED;
    pFilter->matrix   = DL_CSC_DISABLED;
    pFilter->bFillOutputAlphaWithConstant = true;

    if(pCompParams->pSource[0] != nullptr &&
       pCompParams->pSource[0]->Format == Format_R5G6B5 &&
       pCompParams->Target[0].Format == Format_R5G6B5)
    {
        pFilter->bIsDitherNeeded = false;
    }else
    {
        pFilter->bIsDitherNeeded = true;
    }

    if (pFilter->format == Format_A8R8G8B8    ||
        pFilter->format == Format_A8B8G8R8    ||
        pFilter->format == Format_R10G10B10A2 ||
        pFilter->format == Format_B10G10R10A2 ||
        pFilter->format == Format_AYUV)
    {
        if (pCompParams->pCompAlpha != nullptr && pCompParams->pSource[0] != nullptr &&
            (pCompParams->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_NONE ||
             pCompParams->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM))
        {
            // When layer 0 does not have alpha channel, Save_RGB will be linked instead of
            // Save_ARGB, to avoid output alpha value corruption.
            switch (pCompParams->pSource[0]->Format)
            {
                case Format_AYUV:
                case Format_AUYV:
                case Format_AI44:
                case Format_IA44:
                case Format_A8R8G8B8:
                case Format_A8B8G8R8:
                case Format_R10G10B10A2:
                case Format_B10G10R10A2:
                case Format_A8P8:
                case Format_A8:
                    pFilter->bFillOutputAlphaWithConstant = false;
                    break;

                default:
                    break;
            }
        }
    }

    // If Rotation is done in sampler. Multiple phases are not required.
    if ((!m_bSamplerSupportRotation) &&
        (pCompParams->uSourceCount > 0))
    {
        // either single layer L0 or all layer with the same rotation degree.
        pFilter->rotation = pCompParams->pSource[0]->Rotation;
    }

    //-------------------------------------------------------
    // Set color fill for RT. Valid for colorfill only cases
    //-------------------------------------------------------
    // If filter size is zero i.e. number of layers is zero, set colorfill to true.
    if (*piFilterSize == 0)
    {
        if(bColorFill)
        {
            pFilter->colorfill = ColorFill_True;
        }
        else
        {
            eStatus = MOS_STATUS_UNIMPLEMENTED;
            goto finish;
        }
    }
    else
    {
        pFilter->colorfill = ColorFill_False;
    }

    // Get App supplied RT format
    pFilter->cspace = pSrc->ColorSpace;

    // Update filter
    (*piFilterSize)++;

finish:
    return ((eStatus == MOS_STATUS_SUCCESS) ? true : false);
}

//!
//! \brief    Initialize Colorfill parameters
//! \details  Initialize Colorfill parameters
//! \return   void
//!
void CompositeState::InitColorFillParams()
{
    m_csSrc.dwValue   = 0;
    m_csDst.dwValue   = 0;
    m_CSpaceSrc       = CSpace_None;
    m_CSpaceDst       = CSpace_None;
}

//!
//! \brief    Check if sample unorm being used for source surface.
//! \param    [in] pCompParams
//!           Pointer to Composite parameters
//! \param    pSrc
//!           [in] Pointer to Source Surface
//! \return   bool
//!           Return TRUE if use sample unorm, otherwise FALSE
//!
bool CompositeState::IsUsingSampleUnorm(
    PVPHAL_COMPOSITE_PARAMS         pCompParams,
    PVPHAL_SURFACE                  pSrc)
{
    float                       fStepX = 0, fStepY = 0;
    float                       fAdjustX = 0, fAdjustY = 0;
    bool                        bRet;
    PRECT                       pTargetRect = {0};

    if (nullptr == pCompParams || nullptr == pSrc)
    {
        VPHAL_RENDER_ASSERTMESSAGE("nullptr for input parameters");
        bRet = false;
        goto finish;
    }

    // Force using sampler16 when compute walker in use
    if (m_bFtrComputeWalker)
    {
        bRet = false;
        goto finish;
    }

    pTargetRect    = &(pCompParams->Target[0].rcDst);
    if (pCompParams->pConstriction)
    {
        fAdjustX = (pTargetRect->right  - pTargetRect->left) * 1.0f /
            pCompParams->pConstriction->right;
        fAdjustY = (pTargetRect->bottom - pTargetRect->top ) * 1.0f /
            pCompParams->pConstriction->bottom;
    }
    else
    {
        fAdjustX = fAdjustY = 1.0f;
    }
    // Calculate scaling factor for X and Y (include BOB DI)
    if (pSrc->Rotation == VPHAL_ROTATION_IDENTITY ||
        pSrc->Rotation == VPHAL_ROTATION_180      ||
        pSrc->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        pSrc->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fStepX = (pSrc->rcSrc.right  - pSrc->rcSrc.left) * fAdjustX /
                    ((pSrc->rcDst.right  - pSrc->rcDst.left) > 0 ?
                    (pSrc->rcDst.right  - pSrc->rcDst.left) : 1);
        fStepY = (pSrc->rcSrc.bottom - pSrc->rcSrc.top ) * fAdjustY /
                    ((pSrc->rcDst.bottom - pSrc->rcDst.top ) > 0 ?
                    (pSrc->rcDst.bottom - pSrc->rcDst.top ) : 1);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        fStepX = (pSrc->rcSrc.right  - pSrc->rcSrc.left) * fAdjustX /
                    ((pSrc->rcDst.bottom - pSrc->rcDst.top ) > 0 ?
                    (pSrc->rcDst.bottom - pSrc->rcDst.top ) : 1);
        fStepY = (pSrc->rcSrc.bottom - pSrc->rcSrc.top ) * fAdjustY /
                    ((pSrc->rcDst.right  - pSrc->rcDst.left) > 0 ?
                    (pSrc->rcDst.right  - pSrc->rcDst.left) : 1);
    }
    if (IsBobDiEnabled(pSrc) &&
        pSrc->ScalingMode != VPHAL_SCALING_AVS)
    {
        fStepY *= 0.5f;
    }

    // Progressive main video (except RGB format) or for RGB10, use AVS
    if ((pSrc->ScalingMode == VPHAL_SCALING_AVS) &&
        !IsBobDiEnabled(pSrc))
    {
        // GEN8 cannot support YV12 input format for iAVS scaling
        if (pSrc->bInterlacedScaling && !m_bYV12iAvsScaling && pSrc->Format == Format_YV12)
        {
            bRet = true;
            goto finish;
        }
        else
        {
            bRet = false;  // AVS
            goto finish;
        }
    }
    else
    {
        if (pSrc->Format == Format_R10G10B10A2   ||
            pSrc->Format == Format_B10G10R10A2   ||
            pSrc->Format == Format_Y410          ||
            pSrc->Format == Format_Y416)
        {
            bRet = false;  // DScaler
            goto finish;
        }
        else if (fStepX >= 3.0f || fStepY >= 3.0f)
        {
            return !MEDIA_IS_WA(m_pWaTable, WaEnableDscale);
        }
        else
        {
            bRet = true;
            goto finish;
        }
    }

finish:
    return bRet;
}

//!
//! \brief    Check if sampler lumakey being supported or not for source surface.
//! \param    pSrc
//!           [in] Pointer to Source Surface
//! \return   bool
//!           Return TRUE if support, otherwise FALSE
//!
bool CompositeState::IsSamplerLumakeySupported(PVPHAL_SURFACE pSrc)
{
    // The kernel is different b/w sampler luma key and EU computed luma key.
    // Sampler based: IDR_VP_Prepare_LumaKey_SampleUnorm
    // EU computed:   IDR_VP_Compute_Lumakey
    // When sampler lumakey being enabled by set pUnormSampler->DW1.ChromakeyEnable to 1, a lumakey mask will be generated
    // during sampler scaling.
    // IDR_VP_Prepare_LumaKey_SampleUnorm is used to prepare lumakey related parameters for composition according to the
    // lumakey mask, and the result will be put in the common registers. To avoid the result being overwriten by other
    // operation, IDR_VP_Prepare_LumaKey_SampleUnorm need to be called rigth before IDR_VP_Call_Composite.
    // Following are the conditions to enable sampler lumakey.
    // 1 Flag m_bEnableSamplerLumakey is set to true.
    // 2 Lumakey is needed for current layer.
    // 3 Enable sampler lumakey only for composition.
    // 4 Disable sampler lumakey if there's no AVS as back up for layer 0
    // 5 Enable sampler lumakey only on YUY2 and NV12 surfaces due to hw limitation.
    // 6 Enable sampler lumakey feature only if this lumakey layer is not the bottom layer.
    // 7 Enable sampler lumakey only when sample unorm being used.
    // 8 Disable sampler lumakey for mirror case, since mirror is done after sampler scaling, which cause the lumakey mask
    //   not matching the layer any more.
    return m_bEnableSamplerLumakey                                                              &&
            pSrc->pLumaKeyParams != NULL                                                        &&
            (pSrc->pBlendingParams == NULL || pSrc->pBlendingParams->BlendType == BLEND_NONE)   &&
            !m_need3DSampler                                                                    &&
            (pSrc->Format == Format_YUY2 || pSrc->Format == Format_NV12)                        &&
            pSrc->iLayerID                                                                      &&
            pSrc->bUseSampleUnorm                                                               &&
            pSrc->Rotation < VPHAL_MIRROR_HORIZONTAL;
}

//!
//! \brief    Initialize Composite state
//! \details  Initialize Composite state, including setup KernelDLL/Procamp/Colorfill
//! \param    [in] pSettings
//!           Pointer to VPHAL Settings
//! \param    [in] pKernelDllState
//!           Pointer to KernelDLL State
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS CompositeState::Initialize(
    const VphalSettings             *pSettings,
    Kdll_State                      *pKernelDllState)
{
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    MOS_NULL_RENDERING_FLAGS        NullRenderingFlags;
    bool                            bAllocated;
    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_UNUSED(pSettings);
    VPHAL_RENDER_CHK_NULL(pKernelDllState);

    if (m_reporting == nullptr)
    {
        m_reporting = MOS_New(VphalFeatureReport);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // Read user feature key to enable 8-tap adaptive filter;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_COMP_8TAP_ADAPTIVE_ENABLE_ID,
        &UserFeatureData));
    m_b8TapAdaptiveEnable = UserFeatureData.bData ? true : false;
#endif

    NullRenderingFlags = m_pOsInterface->pfnGetNullHWRenderFlags(
                        m_pOsInterface);

    m_bNullHwRenderComp =
                    NullRenderingFlags.VPComp ||
                    NullRenderingFlags.VPGobal;

    // Setup kernelDLLL
    m_pKernelDllState = pKernelDllState;

    if (m_pKernelDllState->bEnableCMFC)
    {
        // Allocate auto CSC Coeff Surface
        VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
            m_pOsInterface,
            &m_CmfcCoeff,
            "CSCCoeffSurface",
            Format_L8,
            MOS_GFXRES_2D,
            MOS_TILE_LINEAR,
            VPHAL_COMP_CMFC_COEFF_WIDTH,
            VPHAL_COMP_CMFC_COEFF_HEIGHT,
            false,
            MOS_MMC_DISABLED,
            &bAllocated));
    }

    // Setup Procamp Parameters
    KernelDll_SetupProcampParameters(pKernelDllState,
                                     m_Procamp,
                                     m_iMaxProcampEntries);

    // Init Color fill params
    InitColorFillParams();

finish:
    return eStatus;
}

//!
//! \brief    Composite Destructor
//!
CompositeState::~CompositeState()
{
}

//!
//! \brief    Composite Destroy function
//! \details  Destroy resource allocated by Composite
//!
void CompositeState::Destroy()
{
    PRENDERHAL_INTERFACE                pRenderHal;
    PMOS_INTERFACE                      pOsInterface;
    PMHW_BATCH_BUFFER                   pBuffer;
    int32_t                             i;

    VPHAL_RENDER_ASSERT(m_pRenderHal);
    VPHAL_RENDER_ASSERT(m_pOsInterface);

    pRenderHal   = m_pRenderHal;
    pOsInterface = m_pOsInterface;

    // Destroy Batch Buffers
    for (i = 0; i < m_iBatchBufferCount; i++)
    {
        pBuffer = &m_BatchBuffer[i];
        pRenderHal->pfnFreeBB(pRenderHal, pBuffer);
    }

    // Free intermediate compositing buffer
    pOsInterface->pfnFreeResource(
        pOsInterface,
        &m_Intermediate.OsResource);

    if (m_Intermediate2.pBlendingParams)
    {
        MOS_FreeMemory(m_Intermediate2.pBlendingParams);
        m_Intermediate2.pBlendingParams = nullptr;
    }

    pOsInterface->pfnFreeResource(
        pOsInterface,
        &m_Intermediate2.OsResource);

    pOsInterface->pfnFreeResource(
        pOsInterface,
        &m_CmfcCoeff.OsResource);

    // Destroy sampler 8x8 state table parameters
    VpHal_RndrCommonDestroyAVSParams(&m_AvsParameters);
}

//! \brief    Initialize interface for Composite
//! \param    [in] pOsInterface
//!           Pointer to MOS interface structure
//! \param    [in] pRenderHal
//!           Pointer to RenderHal interface structure
//! \param    [in] pPerfData
//!           Pointer to performance data structure
//! \param    [in] compositeCacheCntl
//!           Composite Cache Control Data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
CompositeState::CompositeState(
    PMOS_INTERFACE                      pOsInterface,
    PRENDERHAL_INTERFACE                pRenderHal,
    PVPHAL_RNDR_PERF_DATA               pPerfData,
    const VPHAL_COMPOSITE_CACHE_CNTL    &compositeCacheCntl,
    MOS_STATUS                          *peStatus)
    : RenderState(pOsInterface, pRenderHal, pPerfData, peStatus),
    m_iMaxProcampEntries(0),
    m_iProcampVersion(0),
    m_bNullHwRenderComp(false),
    m_b8TapAdaptiveEnable(false),
    m_pKernelDllState(nullptr),
    m_ThreadCountPrimary(0),
    m_iBatchBufferCount(0),
    m_iCallID(0),
    m_bLastPhase(false),
    m_fSamplerLinearBiasX(0),
    m_fSamplerLinearBiasY(0),
    m_bFtrMediaWalker(false),
    m_bFtrComputeWalker(false),
    m_bFtrCSCCoeffPatchMode(false),
    m_bSamplerSupportRotation(false),
    m_bChromaUpSampling(false),
    m_bChromaDownSampling(false),
    m_bFallbackIefPatch(false),
    m_bKernelSupportDualOutput(false),
    m_bKernelSupportHdcDW(false),
    m_bApplyTwoLayersCompOptimize(false),
    m_need3DSampler(false),
    m_bEnableSamplerLumakey(false),
    m_bYV12iAvsScaling(false),
    m_bAvsTableCoeffExtraEnabled(false),
    m_bAvsTableBalancedFilter(false)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;

    MOS_ZeroMemory(&m_Procamp, sizeof(m_Procamp));
    MOS_ZeroMemory(&m_csSrc, sizeof(m_csSrc));
    MOS_ZeroMemory(&m_csDst, sizeof(m_csDst));
    MOS_ZeroMemory(&m_CSpaceSrc, sizeof(m_CSpaceSrc));
    MOS_ZeroMemory(&m_CSpaceDst, sizeof(m_CSpaceDst));
    MOS_ZeroMemory(&m_SurfMemObjCtl, sizeof(m_SurfMemObjCtl));
    MOS_ZeroMemory(&m_SearchFilter, sizeof(m_SearchFilter));
    MOS_ZeroMemory(&m_KernelSearch, sizeof(m_KernelSearch));
    MOS_ZeroMemory(&m_KernelParams, sizeof(m_KernelParams));
    MOS_ZeroMemory(&m_Intermediate, sizeof(m_Intermediate));
    MOS_ZeroMemory(&m_Intermediate2, sizeof(m_Intermediate2));
    MOS_ZeroMemory(&m_CmfcCoeff, sizeof(m_CmfcCoeff));
    MOS_ZeroMemory(&m_RenderHalCmfcCoeff, sizeof(m_RenderHalCmfcCoeff));
    MOS_ZeroMemory(&m_AvsParameters, sizeof(m_AvsParameters));
    MOS_ZeroMemory(&m_mhwSamplerAvsTableParam, sizeof(m_mhwSamplerAvsTableParam));
    MOS_ZeroMemory(&m_BatchBuffer, sizeof(m_BatchBuffer));
    MOS_ZeroMemory(&m_BufferParam, sizeof(m_BufferParam));

    // Set Max number of procamp entries
    m_iMaxProcampEntries        = VPHAL_MAX_PROCAMP;

    // Set Bilinear Sampler Bias
    m_fSamplerLinearBiasX       = VPHAL_SAMPLER_BIAS_GEN575;
    m_fSamplerLinearBiasY       = VPHAL_SAMPLER_BIAS_GEN575;

    // Batch buffers
    m_iBatchBufferCount         = 0;

    // Procamp
    // Set Max number of procamp entries
    m_iMaxProcampEntries        = VPHAL_MAX_PROCAMP;
    m_iProcampVersion           = 1;

    //CSCCoeffPatchMode
    m_bFtrCSCCoeffPatchMode     = true;

    // Cache settings
    m_SurfMemObjCtl             = compositeCacheCntl;

    // Composite Kernel
    m_KernelParams              = g_cInitKernelParamsComposite;
    m_ThreadCountPrimary        = VPHAL_USE_MEDIA_THREADS_MAX;
    VPHAL_RENDER_CHK_NULL(pRenderHal);
    m_bFtrMediaWalker           = pRenderHal->pfnGetMediaWalkerStatus(pRenderHal) ? true : false;

    MOS_ZeroMemory(&m_mhwSamplerAvsTableParam, sizeof(m_mhwSamplerAvsTableParam));

    VPHAL_RENDER_CHK_NULL(pOsInterface);
    // Reset Intermediate output surface (multiple phase)
    pOsInterface->pfnResetResourceAllocationIndex(pOsInterface, &m_Intermediate.OsResource);

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE_ID,
        &UserFeatureData));
    m_bFtrCSCCoeffPatchMode = UserFeatureData.bData ? false : true;

finish:
    // copy status to output argument to pass status to caller
    if (peStatus)
    {
        *peStatus = eStatus;
    }
}

//!
//! \brief    Judge if Composite render is needed
//! \details  Check Render parameter/data if Composite render needed
//! \param    [in] pcRenderParams
//!           Pointer to Render parameters
//! \param    [in,out] pRenderPassData
//!           Pointer to Render data
//! \return   bool
//!           true if meeded. Else false
//!
// use set render flags
bool CompositeState::IsNeeded(
    PCVPHAL_RENDER_PARAMS  pcRenderParams,
    RenderpassData         *pRenderPassData)
{
    VPHAL_RENDER_ASSERT(pRenderPassData);

    MOS_UNUSED(pcRenderParams);

    return pRenderPassData->bCompNeeded;
}

//!
//! \brief    Judge if Composite render support multiple stream rendering
//! \details  Judge if Composite render support multiple stream rendering
//! \return   bool
//!           true if supported. Else false
//!
bool CompositeState::IsMultipleStreamSupported()
{
    return true;
}

//!
//! \brief    set Report data
//! \details  set Report data for this render
//! \param    [in] pSource 
//!           pointer to the surface
//!
void CompositeState::SetReporting(PVPHAL_SURFACE pSource)
{
    m_reporting->IEF                   = pSource->bIEF;
    m_reporting->ScalingMode           = pSource->ScalingMode;
    m_reporting->DeinterlaceMode       =
                (IsBobDiEnabled(pSource)) ? VPHAL_DI_REPORT_BOB :
                                                VPHAL_DI_REPORT_PROGRESSIVE;
}

//!
//! \brief    copy Report data
//! \details  copy Report data from this render
//! \param    [out] pReporting 
//!           pointer to the Report data to copy data to
//!
void CompositeState::CopyReporting(VphalFeatureReport* pReporting)
{
    VPHAL_RENDER_ASSERT(pReporting);

    pReporting->IEF         = m_reporting->IEF;
    pReporting->ScalingMode = m_reporting->ScalingMode;

    if (m_reporting->DeinterlaceMode != VPHAL_DI_REPORT_PROGRESSIVE)
    {
        pReporting->DeinterlaceMode = m_reporting->DeinterlaceMode;
    }
}

int32_t CompositeState::GetThreadCountForVfeState(
    PVPHAL_RENDERING_DATA_COMPOSITE     pRenderingData,
    PVPHAL_SURFACE                      pTarget)
{
    int iThreadCount;

    // For optimal performance, we use a different ThreadCount if we are doing
    // Composition for Primary Layer only, and the RenderTarget is Overlay or
    // FlipChain.
    iThreadCount = VPHAL_USE_MEDIA_THREADS_MAX;
    if (pRenderingData->iLayers == 1 &&
        (pTarget->bOverlay || pTarget->bFlipChain))
    {
        for (int i = 0; i < VPHAL_COMP_MAX_LAYERS; i++)
        {
            VPHAL_SURFACE *pSurface = pRenderingData->pLayers[i];
            if (pSurface != nullptr)
            {
                if (pSurface->SurfType == SURF_IN_PRIMARY)
                {
                    iThreadCount = m_ThreadCountPrimary;
                }
                break;
            }
        }
    }

    if (m_pPerfData->CompMaxThreads.bEnabled)
    {
        iThreadCount =
            m_pPerfData->CompMaxThreads.uiVal;
    }

    return iThreadCount;
}

bool CompositeState::IsSamplerIDForY(
    int32_t                            SamplerID)
{
    return (SamplerID == VPHAL_SAMPLER_Y) ? true : false;
}
