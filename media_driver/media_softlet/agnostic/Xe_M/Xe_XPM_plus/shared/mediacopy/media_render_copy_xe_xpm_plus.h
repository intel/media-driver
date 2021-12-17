/*===================== begin_copyright_notice ==================================

* Copyright (c) 2021, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file       media_render_copy_xe_xpm_plus.h
//! \brief      render copy's implement file.
//! \details    PVC render copy's implement file.
//!
#ifndef __MEDIA_RENDER_COPY_XE_XPM_PLUS_H__
#define __MEDIA_RENDER_COPY_XE_XPM_PLUS_H__

#include "media_render_copy.h"
#include "media_interfaces_mhw.h"
#include "mhw_render.h"
#include "mhw_mi.h"
#include "mhw_cp_interface.h"
#include "mos_os.h"
#include "mhw_render_xe_hp.h"

#define  RENDERCOPY_SRC_INDEX 0
#define  RENDERCOPY_DST_INDEX 3
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
} RENDERCOPY_KERNELID, *PRENDERCOPY_KERNELID;

typedef struct _RENDER_COPY_CACHE_CNTL
{
    bool                           bL3CachingEnabled;
    VPHAL_MEMORY_OBJECT_CONTROL    SourceSurfMemObjCtl;
    VPHAL_MEMORY_OBJECT_CONTROL    TargetSurfMemObjCtl;

}RENDER_COPY_CACHE_CNTL, *PRENDER_COPY_CACHE_CNTL;

typedef struct _MEDIACOPY_RENDER_DATA
{
    int32_t                             iBlocksX;
    int32_t                             iBlocksY;
    int32_t                             iBindingTable;
    int32_t                             iMediaID;;
    PRENDERHAL_MEDIA_STATE              pMediaState;

    // Kernel Information
    PRENDERHAL_KERNEL_PARAM             pKernelParam;                           //!< Kernel Parameter
    Kdll_CacheEntry                     KernelEntry;                            //!< Kernel Entry
    int32_t                             iCurbeLength;
    int32_t                             iInlineLength;

    // Perf
    VPHAL_PERFTAG                       PerfTag;
    RENDER_COPY_CACHE_CNTL              SurfMemObjCtl;

    // Debug parameters
    char*                               pKernelName;                            //!< Kernel Used for current rendering
    int32_t                             iCurbeOffset;                           //!< The offset of curbe data
} MEDIACOPY_RENDER_DATA, *PMEDIACOPY_RENDER_DATA;

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

    // DWORD 1 - GRF R1.0
    union
    {
        uint32_t       InputsurfaceGindex;      // Input buffer surface G plane index
    } DW1;

    // DWORD 2 - GRF R1.0
    union
    {
        uint32_t       InputsurfaceBindex;      // Input buffer surface B plane index
    } DW2;

    // DWORD 3 - GRF R1.0
    union
    {
        uint32_t       OutputsurfaceRindex;      // Output buffer surface R plane index
    } DW3;

    // DWORD 4 - GRF R1.0
    union
    {
        uint32_t       OutputsurfaceGindex;      // Output buffer surface G plane index
    } DW4;

    // DWORD 5 - GRF R1.1
    union
    {
        uint32_t       OutputsurfaceBindex;     // Output buffer surface B plane Index
    } DW5;

    // DWORD 6 - GRF R1.2
    union
    {
        uint32_t       Widthdword;             // Image width in pxels divided by 4
    } DW6;

    // DWORD 7 - GRF R1.3
    union
    {
        uint32_t        Height;               //  Image height in pixels
    } DW7;

    // DWORD 8 - GRF R1.4
    union
    {
        uint32_t        ShiftLeftOffsetInBytes; // Byte offset of first pixel in input buffer
    } DW8;

    // DWORD 12 - GRF R2.0
    union
    {
        uint32_t        Dst2DStartX; // X-coordinate of top-left pixel in output 2D surface
    } DW9;

    // DWORD 13 - GRF R2.1
    union
    {
        uint32_t        Dst2DStartY; //Y-coordinate of top-left pixel in output 2D surface
    } DW10;

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
        uint32_t        Dst2DStartX;         // X-coordinate of top-left pixel in output 2D surface
    } DW6;

    // DWORD 6 - GRF R1.7
    union
    {
        uint32_t        Dst2DStartY;         // Y-coordinate of top-left pixel in output 2D surface
    } DW7;

}DP_RENDERCOPY_PACKED_STATIC_DATA, *PDP_RENDERCOPY_PACKED_STATIC_DATA;

typedef class  RenderCopy_Xe_Xpm_Plus *PRenderCopy_Xe_Xpm_Plus;
class RenderCopy_Xe_Xpm_Plus: public RenderCopyState
{
public:
    RenderCopy_Xe_Xpm_Plus(PMOS_INTERFACE  osInterface, MhwInterfaces  *mhwInterfaces);

    virtual ~RenderCopy_Xe_Xpm_Plus();

    //!
    //! \brief    Copy input surface to Output surface
    //! \details  Copy 2D surface to 2D surface
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [in] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopySurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

protected:

    //!
    //! \brief    Submit command
    //! \details  Submit render command
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD( );

    MOS_STATUS SetupKernel(int32_t iKDTIndex);

//!
//! \brief    Render copy omputer walker setup
//! \details  Computer walker setup for render copy
//! \param    PMHW_WALKER_PARAMS pWalkerParams
//!           [in/out] Pointer to Walker params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
    MOS_STATUS RenderCopyComputerWalker(
     PMHW_GPGPU_WALKER_PARAMS    pWalkerParams);

    //!
    //! \brief    setup surface states
    //! \details  Setup surface states for fast 1toN

    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetupSurfaceStates();

    MOS_STATUS LoadStaticData(
     int32_t* piCurbeOffset);

    MOS_STATUS GetCurentKernelID();
    int32_t GetBytesPerPixel(MOS_FORMAT        Format);
    // Compositing Kernel buffer and size
    const void                  *pcKernelBin = nullptr;

public:

    Kdll_State                   *m_pKernelDllState = nullptr;                     //!< Kernel DLL state
    RENDERCOPY_KERNELID          m_currKernelId = KERNEL_CopyKernel_1D_to_2D_NV12;
    MOS_RESOURCE                 m_KernelResource = {0};                           //!<Graphics memory for Kernel access
    MEDIACOPY_RENDER_DATA        m_RenderData = {0};
    VPHAL_SURFACE                m_Source;
    VPHAL_SURFACE                m_Target;
    bool                         m_bNullHwRenderCopy = false;
    RENDERHAL_SURFACE            RenderHalSource = {0}; // source for mhw
    RENDERHAL_SURFACE            RenderHalTarget = {0}; // target for mhw

    uint32_t                     m_WalkerWidthBlockSize = 128;
    uint32_t                     m_WalkerHeightBlockSize = 8;
};

#endif // __MEDIA_RENDER_COPY_XE_XPM_PLUS_H__
