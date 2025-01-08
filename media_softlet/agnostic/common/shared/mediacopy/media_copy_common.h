/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_copy_common.h
//! \brief    Common defination and structure used in media copy
//! \details  Common defination and structure used in media copy, header file only.

#ifndef __MEDIA_COPY_COMMON_H__
#define __MEDIA_COPY_COMMON_H__

#define RENDERCOPY_SRC_INDEX 0
#define RENDERCOPY_DST_INDEX 3
#define RENDER_COPY_THREADS_MAX  0
#define RENDER_COPY_NUM          9

#define MCPY_CHK_STATUS(_stmt)               MOS_CHK_STATUS(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _stmt)
#define MCPY_CHK_STATUS_RETURN(_stmt)        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _stmt)
#define MCPY_CHK_NULL(_ptr)                  MOS_CHK_NULL(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _ptr)
#define MCPY_CHK_NULL_RETURN(_ptr)           MOS_CHK_NULL_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _ptr)
#define MCPY_ASSERTMESSAGE(_message, ...)    MOS_ASSERTMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _message, ##__VA_ARGS__)
#define MCPY_NORMALMESSAGE(_message, ...)    MOS_NORMALMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define BLT_CHK_STATUS(_stmt)               MOS_CHK_STATUS(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _stmt)
#define BLT_CHK_STATUS_RETURN(_stmt)        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _stmt)
#define BLT_CHK_NULL(_ptr)                  MOS_CHK_NULL(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _ptr)
#define BLT_CHK_NULL_RETURN(_ptr)           MOS_CHK_NULL_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _ptr)
#define BLT_ASSERTMESSAGE(_message, ...)    MOS_ASSERTMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _message, ##__VA_ARGS__)
#define BLT_BITS_PER_BYTE                   8

#define VEBOX_COPY                          ((uint32_t)(VPHAL_MCP_VEBOX_COPY))
#define RENDER_COPY                         ((uint32_t)(VPHAL_MCP_RENDER_COPY))
#define BLT_COPY                            ((uint32_t)(VPHAL_MCP_BLT_COPY))

#define SINGLE_PLANE                        0x1
#define TWO_PLANES                          0x2
#define THREE_PLANES                        0x3

typedef enum YUVANDRGB_PLANE_INDEX_ENUM
{
    MCPY_PLANE_Y  = 0,
    MCPY_PLANE_U  = 1,
    MCPY_PLANE_V  = 2,
} YUVANDRGB_PLANE_INDEX;

//!
//! \brief  Enum for CCS read/write flag
//!
enum CCS_FLAG
{
    CCS_READ = 0,
    CCS_WRITE,
};

//!
//! \brief  Structure for BLT parameter
//!
typedef struct _BLT_STATE_PARAM
{
    bool             bCopyMainSurface;
    PMOS_RESOURCE    pSrcSurface;
    PMOS_RESOURCE    pDstSurface;
    bool             bCopyCCS;
    CCS_FLAG         ccsFlag;
    PMOS_SURFACE     pSrcCCS;
    PMOS_SURFACE     pDstCCS;
}BLT_STATE_PARAM, * PBLT_STATE_PARAM;

//!
//! \brief Kernel IDs
//!
typedef enum _RENDERCOPY_KERNELID
{
    KERNEL_CopyKernel_1D_to_2D_NV12 = 0,
    KERNEL_CopyKernel_2D_to_1D_NV12,
    KERNEL_CopyKernel_2D_to_2D_NV12,
    KERNEL_CopyKernel_1D_to_2D_Planar,
    KERNEL_CopyKernel_2D_to_1D_Planar,
    KERNEL_CopyKernel_2D_to_2D_Planar,
    KERNEL_CopyKernel_1D_to_2D_Packed,
    KERNEL_CopyKernel_2D_to_1D_Packed,
    KERNEL_CopyKernel_2D_to_2D_Packed,
    KERNEL_CopyKernel_MAX
} RENDERCOPY_KERNELID, * PRENDERCOPY_KERNELID;

typedef struct _DP_RENDERCOPY_NV12_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        uint32_t       Inputsurfaceindex;      // Input buffer surface index
    } DW0;

    // DWORD 1 - GRF R1.1
    union
    {
        uint32_t       Outputsurfaceindex;     // Output buffer surface Index
    } DW1;

    // DWORD 2 - GRF R1.2
    union
    {
        uint32_t       Widthdword;             // Image width in pxels divided by 4
    } DW2;

    // DWORD 3 - GRF R1.3
    union
    {
        uint32_t        Height;               //  Image height in pixels
    } DW3;

    // DWORD 4 - GRF R1.4
    union
    {
        uint32_t        ShiftLeftOffsetInBytes; // Byte offset of first pixel in input buffer
    } DW4;

    // DWORD 5 - GRF R1.5
    union
    {
        uint32_t        Widthstride;           // Stride of 1 line of pixels in input buffer
    } DW5;

    // DWORD 6 - GRF R1.6
    union
    {
        uint32_t        Heightstride;         // Stride of number of lines of Y pixels before start of UV pixels
    } DW6;

}DP_RENDERCOPY_NV12_STATIC_DATA, *PDP_RENDERCOPY_NV12_STATIC_DATA;

typedef struct _DP_RENDERCOPY_RGBP_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        uint32_t       InputsurfaceRindex;      // Input buffer surface R plane index
    } DW0;

    // DWORD 1 - GRF R1.1
    union
    {
        uint32_t       InputsurfaceGindex;      // Input buffer surface G plane index
    } DW1;

    // DWORD 2 - GRF R1.2
    union
    {
        uint32_t       InputsurfaceBindex;      // Input buffer surface B plane index
    } DW2;

    // DWORD 3 - GRF R1.3
    union
    {
        uint32_t       OutputsurfaceRindex;      // Output buffer surface R plane index
    } DW3;

    // DWORD 4 - GRF R1.4
    union
    {
        uint32_t       OutputsurfaceGindex;      // Output buffer surface G plane index
    } DW4;

    // DWORD 5 - GRF R1.5
    union
    {
        uint32_t       OutputsurfaceBindex;     // Output buffer surface B plane Index
    } DW5;

    // DWORD 6 - GRF R1.6
    union
    {
        uint32_t       Widthdword;             // Image width in pxels divided by 4
    } DW6;

    // DWORD 7 - GRF R1.7
    union
    {
        uint32_t        Height;               //  Image height in pixels
    } DW7;

    // DWORD 8 - GRF R2.0
    union
    {
        uint32_t        ShiftLeftOffsetInBytes; // Byte offset of first pixel in input buffer
    } DW8;

    // DWORD 9 - GRF R2.1
    union
    {
        uint32_t        WidthdwordNoPadding; // excluding output padding
    } DW9;

    // DWORD 10 - GRF R2.2
    union
    {
        uint32_t        Dst2DStartX; // X-coordinate of top-left pixel in output 2D surface
    } DW10;

    // DWORD 12 - GRF R2.3
    union
    {
        uint32_t        Dst2DStartY; //Y-coordinate of top-left pixel in output 2D surface
    } DW11;

}DP_RENDERCOPY_RGBP_STATIC_DATA, *PDP_RENDERCOPY_RGBP_STATIC_DATA;

typedef struct _DP_RENDERCOPY_PACKED_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        uint32_t       InputSurfaceIndex;      // Input buffer surface index
    } DW0;

    // DWORD 1 - GRF R1.1
    union
    {
        uint32_t       OutputSurfaceIndex;     // Output buffer surface Index
    } DW1;

    // DWORD 2 - GRF R1.2
    union
    {
        uint32_t       WidthDWord;             // Image width in pixels * bytes_per_pixel / 4
    } DW2;

    // DWORD 3 - GRF R1.3
    union
    {
        uint32_t        Height;               //  Image height in pixels
    } DW3;

    // DWORD 4 - GRF R1.4
    union
    {
        uint32_t        ShiftLeftOffsetInBytes; // Byte offset of first pixel in input buffer
    } DW4;

    // DWORD 5 - GRF R1.5
    union
    {
        uint32_t        ThreadHeight;           // Height in pixels / 32
    } DW5;

    // DWORD 6 - GRF R1.6
    union
    {
        uint32_t        WidthdwordNoPadding;         // Input image width in pixels divided by 4
    } DW6;

    // DWORD 6 - GRF R1.7
    union
    {
        uint32_t        Dst2DStartX;         // X-coordinate of top-left pixel in output 2D surface
    } DW7;

    // DWORD 6 - GRF R2.0
    union
    {
        uint32_t        Dst2DStartY;         // Y-coordinate of top-left pixel in output 2D surface
    } DW8;

}DP_RENDERCOPY_PACKED_STATIC_DATA, *PDP_RENDERCOPY_PACKED_STATIC_DATA;

#endif
