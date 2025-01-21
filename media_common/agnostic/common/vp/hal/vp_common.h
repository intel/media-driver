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
//! \file     vp_common.h
//! \brief    clarify common utilities for vphal
//! \details  clarify common utilities for vphal including:
//!           some marcro, enum, union, structure, function
//!
#ifndef __VP_COMMON_H__
#define __VP_COMMON_H__

#if EMUL || VPHAL_LIB

#include "support.h"

#endif  // EMUL || VPHAL_LIB

#include "mos_os.h"
#include "vp_common_hdr.h"
#include "media_common_defs.h"
#include "vp_common_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

//! \brief  Surface cache attributes
//!
#define VPHAL_SET_SURF_MEMOBJCTL(VpField, GmmUsageEnum)                                                                      \
    {                                                                                                                        \
        Usage      = GmmUsageEnum;                                                                                           \
        MemObjCtrl = pOsInterface->pfnCachePolicyGetMemoryObject(Usage, pOsInterface->pfnGetGmmClientContext(pOsInterface)); \
        VpField    = MemObjCtrl.DwordValue;                                                                                  \
    }

//!
//! \def WITHIN_BOUNDS(a, min, max)
//! Calcualte if \a a within the range of  [\a min, \a max].
//!
#define WITHIN_BOUNDS(a, min, max) (((a) >= (min)) && ((a) <= (max)))

//!
//! \def SAME_SIZE_RECT(rect1, rect2)
//! Compare if the size of two rectangles is the same
//!
#define SAME_SIZE_RECT(rect1, rect2)                                   \
    (((rect1).right - (rect1).left == (rect2).right - (rect2).left) && \
        ((rect1).bottom - (rect1).top == (rect2).bottom - (rect2).top))

//!
//! \def IS_YUV_FULL_RANGE(_a)
//! Check if YUV full range
//!
#define IS_YUV_FULL_RANGE(_a) (_a == CSpace_BT601_FullRange ||     \
                               _a == CSpace_BT709_FullRange ||     \
                               _a == CSpace_BT601Gray_FullRange || \
                               _a == CSpace_BT2020_FullRange)

//! Structure VPHAL_SURFACE
//! \brief DDI-VPHAL surface definition
//!
struct VP_SURFACE;
struct VPHAL_SURFACE
{
    // Color Information
    VPHAL_CSPACE  ColorSpace    = CSpace_None;  //!<Color Space
    bool          ExtendedGamut = false;        //!<Extended Gamut Flag
    int32_t       iPalette      = 0;            //!<Palette Allocation
    VPHAL_PALETTE Palette       = {};           //!<Palette data

    // Rendering parameters
    RECT                   rcSrc           = {0, 0, 0, 0};  //!< Source rectangle
    RECT                   rcDst           = {0, 0, 0, 0};  //!< Destination rectangle
    RECT                   rcMaxSrc        = {0, 0, 0, 0};  //!< Max source rectangle
    PVPHAL_BLENDING_PARAMS pBlendingParams = nullptr;       //!< Blending parameters
    PVPHAL_LUMAKEY_PARAMS  pLumaKeyParams  = nullptr;       //!< Luma keying parameters
    PVPHAL_PROCAMP_PARAMS  pProcampParams  = nullptr;
    ;                                                //!< Procamp parameters
    PVPHAL_IEF_PARAMS pIEFParams         = nullptr;  //!< IEF parameters
    bool              bCalculatingAlpha  = false;    //!< Alpha calculation parameters
    bool              bQueryVariance     = false;    //!< enable variance query
    bool              bDirectionalScalar = false;    //!< Vebox Directional Scalar
    bool              bFastColorFill     = false;    //!< enable fast color fill without copy surface
    bool              bMaxRectChanged    = false;    //!< indicate rcMaxSrc been updated
    bool              b16UsrPtr          = false;    //!< is 16 byte aligned system linear memory.
    bool              bVEBOXCroppingUsed = false;    //!< Vebox crop case need use rcSrc as vebox input.
    bool              bXORComp           = false;    //!< is mono-chroma composite mode.

    // Interlaced Scaling
    bool                bInterlacedScaling    = false;          //!< Interlaced scaling
    bool                bFieldWeaving         = false;          //!< Field Weaving
    VPHAL_ISCALING_TYPE InterlacedScalingType = ISCALING_NONE;  //!< Interlaced scaling type for new interlaced scaling mode

    // Advanced Processing
    PVPHAL_DI_PARAMS        pDeinterlaceParams = nullptr;
    PVPHAL_DENOISE_PARAMS   pDenoiseParams     = nullptr;  //!< Denoise
    PVPHAL_COLORPIPE_PARAMS pColorPipeParams   = nullptr;  //!< ColorPipe

    // Frame ID and reference samples -> for advanced processing
    int32_t        FrameID      = 0;
    uint32_t       uFwdRefCount = 0;
    uint32_t       uBwdRefCount = 0;
    PVPHAL_SURFACE pFwdRef      = nullptr;
    PVPHAL_SURFACE pBwdRef      = nullptr;

    // VPHAL_SURFACE Linked list
    PVPHAL_SURFACE pNext = nullptr;
    
    // For Multiple SwPipe Usage in SwFilterPipeFactory::Create
    // This is an intermediate surface for multiple pipe in/out
    VP_SURFACE *pPipeIntermediateSurface = nullptr;

    //--------------------------------------
    // FIELDS TO BE SETUP BY VPHAL int32_tERNALLY
    //--------------------------------------
    uint32_t                 dwWidth                   = 0;                         //!<  Surface width
    uint32_t                 dwHeight                  = 0;                         //!<  Surface height
    uint32_t                 dwPitch                   = 0;                         //!<  Surface pitch
    uint32_t                 dwYPitch                  = 0;                         //!<  Surface Y plane pitch
    uint32_t                 dwUPitch                  = 0;                         //!<  Surface U plane pitch
    uint32_t                 dwVPitch                  = 0;                         //!<  Surface V plane pitch
    MOS_TILE_TYPE            TileType                  = MOS_TILE_X;                //!<  Tile Type
    MOS_TILE_MODE_GMM        TileModeGMM               = MOS_TILE_LINEAR_GMM;       //!<  Tile Mode from GMM Definition
    bool                     bGMMTileEnabled           = false;                     //!<  GMM Tile Mode Flag
    bool                     bOverlay                  = false;                     //!<  Overlay Surface
    bool                     bFlipChain                = false;                     //!<  FlipChain Surface
    VPHAL_PLANE_OFFSET       YPlaneOffset              = {0, 0, 0, 0};              //!<  Y surface plane offset
    VPHAL_PLANE_OFFSET       UPlaneOffset              = {0, 0, 0, 0};              //!<  U surface plane offset
    VPHAL_PLANE_OFFSET       VPlaneOffset              = {0, 0, 0, 0};              //!<  V surface plane offset
    int32_t                  iLayerID                  = 0;                         //!<  Layer index (0-based index)
    VPHAL_SCALING_MODE       ScalingMode               = VPHAL_SCALING_NEAREST;     //!<  Scaling Mode
    VPHAL_SCALING_PREFERENCE ScalingPreference         = VPHAL_SCALING_PREFER_SFC;  //!<  Scaling preference
    bool                     bIEF                      = false;                     //!<  IEF flag
    uint32_t                 dwSlicePitch              = 0;                         //!<  SlicePitch of a 3D surface(GT-PIN support)

    //--------------------------------------
    // FIELDS TO BE PROVIDED BY DDI
    //--------------------------------------
    // Sample information
    MOS_FORMAT         Format     = Format_None;              //!<  Surface format
    VPHAL_SURFACE_TYPE SurfType   = SURF_NONE;                //!<  Surface type (context)
    VPHAL_SAMPLE_TYPE  SampleType = SAMPLE_PROGRESSIVE;       //!<  Interlaced/Progressive sample type
    uint32_t           dwDepth    = 0;                        //!<  Surface depth
    MOS_S3D_CHANNEL    Channel    = MOS_S3D_NONE;             //!<  Channel
    uint32_t           dwOffset   = 0;                        //!<  Surface Offset (Y/Base)
    MOS_RESOURCE       OsResource = {};                       //!<  Surface resource
    VPHAL_ROTATION     Rotation   = VPHAL_ROTATION_IDENTITY;  //!<  0: 0 degree, 1: 90 degree, 2: 180 degree, 3: 270 degreee

    // Chroma siting
    uint32_t ChromaSiting  = CHROMA_SITING_NONE;
    bool     bChromaSiting = false;  //!<  Chromasiting flag

    // Surface compression mode, enable flags
    bool bCompressible = false;  // The surface is compressible, means there are additional 128 bit for MMC no matter it is compressed or not
    // The bIsCompressed in surface allocation structure should use this flag to initialize to allocate a compressible surface
    bool                  bIsCompressed     = false;  // The surface is compressed, VEBox output can only support horizontal mode, but input can be horizontal / vertical
    MOS_RESOURCE_MMC_MODE CompressionMode   = MOS_MMC_DISABLED;
    uint32_t              CompressionFormat = 0;

    //Surface cache Usage
    uint32_t CacheSetting = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t oldCacheSetting = 0;
#endif

    bool bUseSampleUnorm    = false;  //!<  true: sample unorm is used, false: DScaler or AVS is used.
    bool bUseSamplerLumakey = false;  //!<  true: sampler lumakey is used, false: lumakey is disabled or EU computed lumakey is used.
    //------------------------------------------
    // HDR related parameters, provided by DDI
    //------------------------------------------
    PVPHAL_HDR_PARAMS pHDRParams            = nullptr;
    VPHAL_GAMMA_TYPE  GammaType             = VPHAL_GAMMA_NONE;  //!<Gamma Type
    bool              bPreAPGWorkloadEnable = false;             //!< Previous Surface Execution Path

    // 3DLUT parameters
    PVPHAL_3DLUT_PARAMS p3DLutParams = nullptr;  //!< 3DLut Mapping Params

    PVPHAL_GAMUT_PARAMS pGamutParams       = nullptr;  //!< Gamut Compression & Expansion
};

//!
//! \brief  VEBOX IECP parameters
//!
class VPHAL_VEBOX_IECP_PARAMS
{
public:
    PVPHAL_COLORPIPE_PARAMS pColorPipeParams;
    PVPHAL_PROCAMP_PARAMS   pProcAmpParams;
    MOS_FORMAT              dstFormat;
    MOS_FORMAT              srcFormat;

    // CSC params
    bool     bCSCEnable;      // Enable CSC transform
    float *  pfCscCoeff;      // [3x3] CSC Coeff matrix
    float *  pfCscInOffset;   // [3x1] CSC Input Offset matrix
    float *  pfCscOutOffset;  // [3x1] CSC Output Offset matrix
    bool     bAlphaEnable;    // Alpha Enable Param
    uint16_t wAlphaValue;     // Color Pipe Alpha Value

    // Front End CSC params
    bool   bFeCSCEnable;      // Enable Front End CSC transform
    float *pfFeCscCoeff;      // [3x3] Front End CSC Coeff matrix
    float *pfFeCscInOffset;   // [3x1] Front End CSC Input Offset matrix
    float *pfFeCscOutOffset;  // [3x1] Front End CSC Output Offset matrix

    VPHAL_VEBOX_IECP_PARAMS()
    {
        pColorPipeParams = nullptr;
        pProcAmpParams   = nullptr;
        dstFormat        = Format_Any;
        srcFormat        = Format_Any;
        bCSCEnable       = false;
        pfCscCoeff       = nullptr;
        pfCscInOffset    = nullptr;
        pfCscOutOffset   = nullptr;
        bAlphaEnable     = false;
        wAlphaValue      = 0;

        bFeCSCEnable     = false;
        pfFeCscCoeff     = nullptr;
        pfFeCscInOffset  = nullptr;
        pfFeCscOutOffset = nullptr;
    }
    virtual ~VPHAL_VEBOX_IECP_PARAMS()
    {
        pColorPipeParams = nullptr;
        pProcAmpParams   = nullptr;
    }
    virtual void Init()
    {
        pColorPipeParams = nullptr;
        pProcAmpParams   = nullptr;

        dstFormat = Format_Any;
        srcFormat = Format_Any;

        bCSCEnable     = false;
        pfCscCoeff     = nullptr;
        pfCscInOffset  = nullptr;
        pfCscOutOffset = nullptr;
        bAlphaEnable   = false;
        wAlphaValue    = 0;
    }
    virtual void *GetExtParams() { return nullptr; }
};

//!
//! \brief    Initial the Type/TileType fields in Alloc Params structure
//! \details  Initial the Type/TileType fields in Alloc Params structure
//!           - Use the last type from GMM resource
//! \param    [in, out] pAllocParams
//!           Pointer to MOS_ALLOC_GFXRES_PARAMS
//! \param    [in] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] DefaultResType
//!           Expected Resource Type
//! \param    [in] DefaultTileType
//!           Expected Surface Tile Type
//!
void VpHal_AllocParamsInitType(
    PMOS_ALLOC_GFXRES_PARAMS pAllocParams,
    PVPHAL_SURFACE           pSurface,
    MOS_GFXRES_TYPE          DefaultResType,
    MOS_TILE_TYPE            DefaultTileType);

//!
//! \brief    Get Surface Info from OsResource
//! \details  Update surface info in PVPHAL_SURFACE based on allocated OsResource
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [in] pInfo
//!           Pointer to VPHAL_GET_SURFACE_INFO
//! \param    [in,out] pSurface
//!           Pointer to VPHAL_SURFACE
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_GetSurfaceInfo(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_GET_SURFACE_INFO pInfo,
    PVPHAL_SURFACE          pSurface);

//!
//! \brief
//! \details  Get CSC matrix in a form usable by Vebox, SFC and IECP kernels
//! \param    [in] SrcCspace
//!           Source Cspace
//! \param    [in] DstCspace
//!           Destination Cspace
//! \param    [out] pfCscCoeff
//!           [3x3] Coefficients matrix
//! \param    [out] pfCscInOffset
//!           [3x1] Input Offset matrix
//! \param    [out] pfCscOutOffset
//!           [3x1] Output Offset matrix
//! \return   void
//!
void VpHal_GetCscMatrix(
    VPHAL_CSPACE SrcCspace,
    VPHAL_CSPACE DstCspace,
    float *      pfCscCoeff,
    float *      pfCscInOffset,
    float *      pfCscOutOffset);

//! \brief    Transfer float type to half precision float type
//! \details  Transfer float type to half precision float (16bit) type
//! \param    [in] fInput
//!           input FP32 number
//! \return   uint16_t
//!           half precision float value in bit
//!
uint16_t VpHal_FloatToHalfFloat(
    float fInput);

#ifdef __cplusplus
}
#endif

#endif  // __VP_COMMON_H__
