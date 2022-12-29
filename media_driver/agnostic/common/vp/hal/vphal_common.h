/*
* Copyright (c) 2009-2022, Intel Corporation
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
//! \file     vphal_common.h
//! \brief    clarify common utilities for vphal
//! \details  clarify common utilities for vphal including:
//!           some marcro, enum, union, structure, function
//!
#ifndef __VPHAL_COMMON_H__
#define __VPHAL_COMMON_H__

#if EMUL || VPHAL_LIB

#include "support.h"

#endif  // EMUL || VPHAL_LIB

#include "mos_os.h"
#include "vp_common_hdr.h"
#include "media_common_defs.h"
#include "vp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \def DBG_TEXT(txt)
//! Allow certain \a txt fields to be present only on DEBUG builds.
//!
#if _DEBUG
#define DBG_TEXT(txt) txt
#else
#define DBG_TEXT(txt) ""
#endif

#ifndef ABS
//!
//! \def ABS( x )
//! Calcualte absolute value of \a x.
//!
#define ABS(a)      (((a) < 0) ? (-(a)) : (a))
#endif

//!
//! \def VPHAL_ABS(x)
//! Calcualte the Abslute value of \a x.
//!
#define VPHAL_ABS(x)               (((x) > 0) ? (x) : -(x))

#define VPHAL_PI                   3.14159265358979324f //!< Definition the const pi

//!
//! \def RECT1_EQUALS_RECT2(rect1, rect2)
//! Compare if two rectangles has the same coordinate
//!
#define RECT1_EQUALS_RECT2(rect1, rect2)                                        \
    (((rect1).left  == (rect2).left)  && ((rect1).top    == (rect2).top) &&     \
     ((rect1).right == (rect2).right) && ((rect1).bottom == (rect2).bottom))

//!
//! \def RECT1_OUTSIDE_RECT2(rect1, rect2)
//! Compare if the rectangle \a rect1 is outside the rectangle \a rect2 at least partly in coordinate
//!
#define RECT1_OUTSIDE_RECT2(rect1, rect2)                                       \
    (((rect1).left  >= (rect2).right) || ((rect1).top    >= (rect2).bottom) ||  \
     ((rect1).right <= (rect2).left)  || ((rect1).bottom <= (rect2).top))

//!
// \def DEGREE_TO_RADIAN(degree)
// Convert a degree value \a degree to radian.
//!
#define DEGREE_TO_RADIAN(degree)   (degree) * (VPHAL_PI) / 180
#define AVS_TBL_COEF_PREC         6           //!< Table coef precision (after decimal point
#define SAME_SAMPLE_THRESHOLD     1000        //!< When checking whether 2 timestamps are the same, leave room for some error

// Compositing Block size
#define VPHAL_COMP_BLOCK_WIDTH  16
#define VPHAL_COMP_BLOCK_HEIGHT 16

// NLAS Default Values
#define NLAS_VERTICALCROP_MIN         0.0F
#define NLAS_VERTICALCROP_MAX         1.0F
#define NLAS_VERTICALCROP_DEFAULT     0.0F
#define NLAS_VERTICALCROP_STEP        0.001F
#define NLAS_HLINEARREGION_MIN        0.0F
#define NLAS_HLINEARREGION_MAX        1.0F
#define NLAS_HLINEARREGION_DEFAULT    1.0F
#define NLAS_HLINEARREGION_STEP       0.001F
#define NLAS_NONLINEARCROP_MIN        0.0F
#define NLAS_NONLINEARCROP_MAX        1.0F
#define NLAS_NONLINEARCROP_DEFAULT    0.0F
#define NLAS_NONLINEARCROP_STEP       0.001F

#define VPHAL_MAX_CHANNELS              2
#define VPHAL_MAX_FUTURE_FRAMES         18       //!< maximum future frames supported in VPHAL

typedef enum _VPHAL_DP_ROTATION_MODE
{
    VPHAL_DP_ROTATION_NV12_AVG            = 0,   //!< nv12 -> yuy2 by chroma average
    VPHAL_DP_ROTATION_NV12_NV12              ,   //!< nv12 -> nv12
    VPHAL_DP_ROTATION_NV12_REP               ,   //!< nv12 -> yuy2 by chroma repeat
    VPHAL_DP_ROTATION_NV12_YUY2_NOT_SET          //!< nv12 -> yuy2 by chroma average or repeat, decided by scaling mode
} VPHAL_DP_ROTATION_MODE;

//!
//! \def IS_RGB_LIMITED_RANGE(_a)
//! Check if RGB limited range
//!
#define IS_RGB_LIMITED_RANGE(_a)               (_a == CSpace_stRGB       || \
                                                _a == CSpace_BT2020_stRGB)

//!
//! \def IS_RGB_FULL_RANGE(_a)
//! Check if RGB full range
//!
#define IS_RGB_FULL_RANGE(_a)                  (_a == CSpace_sRGB       || \
                                                _a == CSpace_BT2020_sRGB)

//!
//! \def IS_YUV_LIMITED_RANGE(_a)
//! Check if YUV limited range
//!
#define IS_YUV_LIMITED_RANGE(_a)               (_a == CSpace_BT601       || \
                                                _a == CSpace_BT709       || \
                                                _a == CSpace_BT601Gray   || \
                                                _a == CSpace_BT2020)

//!
//! \def SET_VPHAL_OUTPUT_PIPE(_a, _Pipe)
//! Set the output pipe
//!
#define SET_VPHAL_OUTPUT_PIPE(_a, _Pipe)                           \
    {                                                              \
        (_a->OutputPipe = _Pipe);                                  \
        VPHAL_RENDER_NORMALMESSAGE("VPHAL_OUTPUT_PIPE %d", _Pipe); \
    }

//!
//! \def IS_VPHAL_OUTPUT_PIPE_INVALID(_a)
//! Sheck if the output pipe is invalid
//!
#define IS_VPHAL_OUTPUT_PIPE_INVALID(_a)              (_a->OutputPipe == VPHAL_OUTPUT_PIPE_MODE_INVALID)

//!
//! \def IS_VPHAL_OUTPUT_PIPE_COMP(_a)
//! Check if the output pipe is Composition
//!
#define IS_VPHAL_OUTPUT_PIPE_COMP(_a)                 (_a->OutputPipe == VPHAL_OUTPUT_PIPE_MODE_COMP)

//!
//! \def IS_VPHAL_OUTPUT_PIPE_SFC(_a)
//! Check if the output pipe is SFC
//!
#define IS_VPHAL_OUTPUT_PIPE_SFC(_a)                  (_a->OutputPipe == VPHAL_OUTPUT_PIPE_MODE_SFC)

//!
//! \def IS_VPHAL_OUTPUT_PIPE_VEBOX(_a)
//! Check if the output pipe is Vebox
//!
#define IS_VPHAL_OUTPUT_PIPE_VEBOX(_a)                (_a->OutputPipe == VPHAL_OUTPUT_PIPE_MODE_VEBOX)

//!
//! \def SET_VPHAL_COMPONENT(_a, _Component)
//! Set the Component
//!
#define SET_VPHAL_COMPONENT(_a, _Component)           (_a->Component  =  _Component)                     // Set the Component

//!
//! \def SET_VPHAL_MMC_STATE(_a, _bEnableMMC)
//! Set the Component
//!
#define SET_VPHAL_MMC_STATE(_a, _bEnableMMC)          (_a->bEnableMMC =  _bEnableMMC)                    // Set the Component

//!
//! \brief Vphal 3DLUT Channel Mapping enum
//!
typedef enum _VPHAL_3DLUT_CHANNEL_MAPPING
{
    CHANNEL_MAPPING_RGB_RGB          = 0,
    CHANNEL_MAPPING_YUV_RGB          = 1 << 0,
    CHANNEL_MAPPING_VUY_RGB          = 1 << 1,
} VPHAL_3DLUT_CHANNEL_MAPPING;

//!
//! Structure VPHAL_CONSTRICTION_PARAMS
//! \brief Constriction parameters
//!
typedef struct _VPHAL_CONSTRICTION_PARAMS
{
    RECT                rcConstriction;
} VPHAL_CONSTRICTION_PARAMS, *PVPHAL_CONSTRICTION_PARAMS;

//!
//! \brief    sinc
//! \details  Calculate sinc(x)
//! \param    [in] x
//!           float
//! \return   float
//!           sinc(x)
//!
float VpHal_Sinc(float x);

//!
//! \brief    Lanczos
//! \details  Calculate lanczos(x)
//!           Basic formula is:  lanczos(x)= VpHal_Sinc(x) * VpHal_Sinc(x / fLanczosT)
//! \param    [in] x
//!           float
//! \param    [in] dwNumEntries
//!           dword
//! \param    [in] fLanczosT
//! 
//! \return   float
//!           lanczos(x)
//!
float VpHal_Lanczos(
    float        x,
    uint32_t    dwNumEntries,
    float        fLanczosT);

//!
//! \brief    Allocates the Surface
//! \details  Allocates the Surface
//!           - if the surface is not already allocated OR
//!           - resource dimenisions OR format changed
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [in,out] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] pSurfaceName
//!           Pointer to surface name
//! \param    [in] Format
//!           Expected MOS_FORMAT
//! \param    [in] DefaultResType
//!           Expected Resource Type
//! \param    [in] DefaultTileType
//!           Expected Surface Tile Type
//! \param    [in] dwWidth
//!           Expected Surface Width
//! \param    [in] dwHeight
//!           Expected Surface Height
//! \param    [in] bCompressible
//!           Surface being compressible or not
//! \param    [in] CompressionMode
//!           Compression Mode
//! \param    [out] pbAllocated
//!           true if allocated, false for not
//! \param    [in] resUsageType
//!           resource usage type for caching
//! \param    [in] tileModeByForce
//!           Forced tile mode
//! \param    [in] memType
//!           vidoe memory location
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_ReAllocateSurface(
    PMOS_INTERFACE          pOsInterface,                                       // [in]    Pointer to OS Interface
    PVPHAL_SURFACE          pSurface,                                           // [in/out]Pointer to surface
    PCCHAR                  pSurfaceName,                                       // [in]    Pointer to surface name
    MOS_FORMAT              Format,                                             // [in]    Surface Format
    MOS_GFXRES_TYPE         DefaultResType,                                     // [in]    Default Resource Type to use if resource has not be allocated yet
    MOS_TILE_TYPE           DefaultTileType,                                    // [in]    Default Resource Tile Type to use if resource has not be allocated yet
    uint32_t                dwWidth,                                            // [in]    Resource Width
    uint32_t                dwHeight,                                           // [in]    Resource Height
    bool                    bCompressible,                                      // [in]    Flag indaicated reource is compressible or not
    MOS_RESOURCE_MMC_MODE   CompressionMode,                                    // [in]    Compression mode
    bool*                   pbAllocated,                                        // [out]   Flag indicating new allocation
    MOS_HW_RESOURCE_DEF     resUsageType = MOS_HW_RESOURCE_DEF_MAX,             // [in]    resource usage type
    MOS_TILE_MODE_GMM       tileModeByForce = MOS_TILE_UNSET_GMM,               // [in]    Flag to indicate if GMM flag tile64 need set
    Mos_MemPool             memType = MOS_MEMPOOL_VIDEOMEMORY,                  // [in]    Flag to indicate the memType
    bool                    isNotLockable = false);                             // [in]    Flag to indicate whether resource being not lockable

//!
//! \brief    Reads the Surface contents and copy to the Dst Buffer
//! \details  Reads the Surface contents and copy to the Dst Buffer
//!           - 1 lock surface
//!           - 2 copy surface data to pDst
//!           - 3 unlock surface
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [in] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] uBpp
//!           bit per pixel of surface contents
//! \param    [out] pDst
//!           output buffer to store Surface contents
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_ReadSurface (
    PMOS_INTERFACE              pOsInterface,
    PVPHAL_SURFACE              pSurface,
    uint32_t                    uBpp,
    uint8_t*                    pDst);

//!
//! \brief    Copy Data from input Buffer to the Surface contents
//! \details  Copy Data from input Buffer to the Surface contents
//!           - 1 lock surface
//!           - 2 copy data from pSrc to Surface
//!           - 3 unlock surface
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [out] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] uBpp
//!           bit per pixel of input buffer
//! \param    [in] pSrc
//!           Input buffer to store Surface contents
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_WriteSurface (
    PMOS_INTERFACE              pOsInterface,
    PVPHAL_SURFACE              pSurface,
    uint32_t                    uBpp,
    const uint8_t*              pSrc);

//!
//! \brief    Decide whether Chroma up sampling is needed
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \param    [in] pTarget
//!           Pointer to Target Surface
//! \return   bool
//!           Return true if Chroma up sampling is needed, otherwise false
//!
bool VpHal_IsChromaUpSamplingNeeded(
    PVPHAL_SURFACE          pSource,
    PVPHAL_SURFACE          pTarget);

//!
//! \brief    Decide whether Chroma down sampling is needed
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \param    [in] pTarget
//!           Pointer to Target Surface
//! \return   bool
//!           Return true if Chroma down sampling is needed, otherwise false
//!
bool VpHal_IsChromaDownSamplingNeeded(
    PVPHAL_SURFACE          pSource,
    PVPHAL_SURFACE          pTarget);

//!
//! \brief      Get the scale ratio
//! \details    Get the scale ratio from input surface to output surface
//! \param      [in] pSource
//!             Pointer to input Surface
//! \param      [in] pTarget
//!             Pointer to output Surface
//! \param      [out] pfScaleX
//!             Pointer to scaling ratio x
//! \param      [out] pfScaleY
//!             Pointer to scaling ratio y
//! \return     void
//!
void VpHal_GetScalingRatio(
    PVPHAL_SURFACE              pSource,
    PVPHAL_SURFACE              pTarget,
    float*                      pfScaleX,
    float*                      pfScaleY);

//! \brief    Transfer float type to half precision float type
//! \details  Transfer float type to half precision float (16bit) type
//! \param    [in] fInput
//!           input FP32 number
//! \return   uint16_t
//!           half precision float value in bit
//!
uint16_t VpHal_FloatToHalfFloatA(
    float fInputA);

bool IsSyncFreeNeededForMMCSurface(PVPHAL_SURFACE pSurface, PMOS_INTERFACE pOsInterface);

#ifdef __cplusplus
}
#endif

#endif  // __VPHAL_COMMON_H__
