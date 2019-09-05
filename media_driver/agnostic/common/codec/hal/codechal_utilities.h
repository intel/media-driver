/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_utilities.h
//! \brief    Defines the common functions for codec.
//! \details  This modules implements utilities which are shared by encoder and decoder 
//!

#ifndef __CODECHAL_UTILITIES_H__
#define __CODECHAL_UTILITIES_H__

#include "codechal_hw.h"
#include "codec_def_common.h"

class CodechalHwInterface;
typedef struct _MHW_WALKER_PARAMS MHW_WALKER_PARAMS, *PMHW_WALKER_PARAMS;
typedef struct _MHW_STATE_HEAP_INTERFACE MHW_STATE_HEAP_INTERFACE, *PMHW_STATE_HEAP_INTERFACE;
typedef struct MHW_KERNEL_STATE *PMHW_KERNEL_STATE;
typedef struct _MHW_RCS_SURFACE_PARAMS MHW_RCS_SURFACE_PARAMS, *PMHW_RCS_SURFACE_PARAMS;
typedef struct _CODECHAL_SURFACE_CODEC_PARAMS CODECHAL_SURFACE_CODEC_PARAMS, *PCODECHAL_SURFACE_CODEC_PARAMS;

typedef enum _CODECHAL_WALKER_DEGREE
{
    CODECHAL_NO_DEGREE,
    CODECHAL_45_DEGREE,
    CODECHAL_26_DEGREE,
    CODECHAL_46_DEGREE,    // VP8 HybridPak2Pattern
    CODECHAL_26Z_DEGREE,   // HEVC
    CODECHAL_45Z_DEGREE,   // VP9 MB ENC I 16x16, P
    CODECHAL_26X_DEGREE,   // HEVC
    CODECHAL_26ZX_DEGREE,  // HEVC
    CODECHAL_45D_DEGREE,   // HEVC 45 diamond walk pattern
    CODECHAL_45XD_DEGREE,  // HEVC 45X diamond walk pattern
    CODECHAL_45X_DEGREE_ALT //HEVC 45X ALT walk pattern
} CODECHAL_WALKER_DEGREE;

typedef struct _CODECHAL_WALKER_CODEC_PARAMS
{
    uint32_t                WalkerMode;
    bool                    bUseScoreboard;
    uint32_t                dwResolutionX;
    uint32_t                dwResolutionY;
    bool                    bNoDependency;
    CODECHAL_WALKER_DEGREE  WalkerDegree;
    bool                    bUseVerticalRasterScan;
    uint32_t                wPictureCodingType;
    bool                    bMbEncIFrameDistInUse;
    bool                    bMbaff;
    uint32_t                bDirectSpatialMVPredFlag;
    bool                    bColorbitSupported;
    uint32_t                dwNumSlices;
    uint16_t                usSliceHeight;
    bool                    bGroupIdSelectSupported;
    uint8_t                 ucGroupId;
    uint32_t                ScoreboardMask;
    uint16_t                usTotalThreadNumPerLcu; //Used by Gen10 HEVC
} CODECHAL_WALKER_CODEC_PARAMS, *PCODECHAL_WALKER_CODEC_PARAMS;

typedef struct _CODECHAL_SURFACE_CODEC_PARAMS
{
    uint32_t                    Mode;
    bool                        bIs2DSurface;
    bool                        bUseUVPlane;
    bool                        bUseAdvState;
    bool                        bMediaBlockRW;
    bool                        bUse16UnormSurfaceFormat;   // Force surface format to R16_UNORM
    bool                        bUse32UnormSurfaceFormat;   // Force surface format to R32_UNORM
    bool                        bUseARGB8Format;            // Force surface format to ARGB8 for Ds+Copy kernel
    bool                        bUse32UINTSurfaceFormat;
    bool                        bRenderTarget;
    bool                        bIsWritable;
    bool                        bUseHalfHeight;
    PMOS_SURFACE                psSurface;              // 2D surface parameters
    PMOS_RESOURCE               presBuffer;             // Buffer parameters
    uint32_t                    dwSize;                 // Buffer size
    uint32_t                    dwOffset;               // Buffer offset
    uint32_t                    dwBindingTableOffset;   // Binding table offset for given surface
    uint32_t                    dwUVBindingTableOffset; // Binding table offset for the UV plane
    uint32_t                    dwVerticalLineStride;
    uint32_t                    dwVerticalLineStrideOffset;
    uint8_t                     ucVDirection;
    uint32_t                    dwCacheabilityControl;
    bool                        bForceChromaFormat;
    uint32_t                    ChromaType;
    bool                        bRawSurface;
    uint8_t                     ucSurfaceStateId;

    uint32_t                    dwWidthInUse;
    uint32_t                    dwHeightInUse;

} CODECHAL_SURFACE_CODEC_PARAMS, *PCODECHAL_SURFACE_CODEC_PARAMS;

typedef struct _CODECHAL_KERNEL_HEADER
{
    union
    {
        struct
        {
            uint32_t                                       : 6;
            uint32_t       KernelStartPointer              : 26;   // GTT 31:6
        };
        struct
        {
            uint32_t       Value;
        };
    };
} CODECHAL_KERNEL_HEADER, *PCODECHAL_KERNEL_HEADER;

//!
//! \brief    Initialize media object walker parameters 
//!
//! \param    [in] hwInterface 
//!           pointer to HW interface
//! \param    [in,out] walkerParams 
//!           Pointer to MHW_WALKER_PARAM, which will be filled with media object walker parameters 
//! \param    [in] walkerCodecParams
//!           Pointer to walkerCodecParams. Specify codec parameters
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalInitMediaObjectWalkerParams(
        CodechalHwInterface *hwInterface,
        PMHW_WALKER_PARAMS walkerParams,
        PCODECHAL_WALKER_CODEC_PARAMS walkerCodecParams);

//!
//! \brief    Gets a kernel information for a specific KUID from the combined kernel
//! \details  Gets a kernel information for a specific KUID from the combined kernel
//!
//! \param    [in] kernelBase
//!           The combined kernel
//! \param    [in] kernelUID
//!           The unique kernel identifier in the combined kernel
//! \param    [in,out] kernelBinary
//!           The binary of the kernel specified by dwKUID
//! \param    [in,out] size
//!           The size of the kernel specified by dwKUID
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalGetKernelBinaryAndSize(
        uint8_t*   kernelBase,
        uint32_t   kernelUID,
        uint8_t**  kernelBinary,
        uint32_t*  size);

//!
//! \brief    Get the surface width in bytes
//! \details  Get the suface width in bytes
//!
//! \param    [in] surface
//!           Surface pointer
//! \param    [out] widthInBytes
//!           Output the surface width
//!
//! \return   void
//!
void CodecHal_GetSurfaceWidthInBytes(
        PMOS_SURFACE                surface,
        uint32_t                   *widthInBytes);

//!
//! \brief    Set RCS surface state 
//! \details  Set RCS surface state 
//!
//! \param    [in] hwInterface 
//!           CodechalHwInterface pointer
//! \param    [in,out] cmdBuffer 
//!           MOS_COMMAND_BUFFER pointer 
//! \param    [in] surfaceCodecParams 
//!           CODECHAL_SURFACE_CODEC_PARAMS pointer. Return
//! \param    [in] kernelState 
//!           MHW_KERNEL_STATE pointer 
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalSetRcsSurfaceState(
        CodechalHwInterface             *hwInterface,
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PCODECHAL_SURFACE_CODEC_PARAMS  surfaceCodecParams,
        PMHW_KERNEL_STATE               kernelState);

//!
//! \brief    Get surface information 
//!
//! \param    [in] osInterface 
//!           OS interface handle 
//! \param    [in,out] surface 
//!           Pointer to MOS_SURFACE. Specify the OsResource and return the surface info.
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalGetResourceInfo(
    PMOS_INTERFACE osInterface,
    PMOS_SURFACE surface);

//!
//! \brief    Allocate data list with specific type and length 
//!
//! \param    [in,out] dataList
//!           Pointer to a type * pointer. Specify the address of the memory handles 
//! \param    [in] length
//!           Specify the number of data members 
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <class type>
MOS_STATUS CodecHalAllocateDataList(type **dataList, uint32_t length)
{
    type *ptr;
    ptr = (type *)MOS_AllocAndZeroMemory(sizeof(type) * length);
    if (ptr == nullptr)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("No memory to allocate CodecHal data list.");
        return MOS_STATUS_NO_SPACE;
    }
    for (uint32_t i = 0; i < length; i++)
    {
        dataList[i] = &(ptr[i]);
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Free data list
//!
//! \param    [in,out] dataList
//!           Pointer to a type * pointer. Specify the address of the memory handles 
//! \param    [in] length
//!           Specify the number of data members 
//!
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <class type>
MOS_STATUS CodecHalFreeDataList(type **dataList, uint32_t length)
{
    type* ptr;
    ptr = dataList[0];
    if (ptr)
    {
        MOS_FreeMemory(ptr);
    }
    for (uint32_t i = 0; i < length; i++)
    {
        dataList[i] = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}
#endif
