/*
* Copyright (c) 2020, Intel Corporation
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
//! \file       render_copy_next.h
//! \brief      header file of render copy base functions
//! \details    define render copy basic API
//!
#ifndef __MEDIA_RENDER_COPY_NEXT_H__
#define __MEDIA_RENDER_COPY_NEXT_H__

#include "media_copy.h"
#include "media_copy_common.h"
#include "renderhal.h"
#include "hal_kerneldll_next.h"

// Kernel Params ---------------------------------------------------------------
const RENDERHAL_KERNEL_PARAM g_rendercopy_KernelParam[RENDER_COPY_NUM] =
{
    ///*  GRF_Count
    //    |  BT_Count
    //    |  |    Sampler_Count
    //    |  |    |  Thread_Count
    //    |  |    |  |                             GRF_Start_Register
    //    |  |    |  |                             |   CURBE_Length
    //    |  |    |  |                             |   |   block_width
    //    |  |    |  |                             |   |   |    block_height
    //    |  |    |  |                             |   |   |    |   blocks_x
    //    |  |    |  |                             |   |   |    |   |   blocks_y
    //    |  |    |  |                             |   |   |    |   |   |*/
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_1D_to_2D_NV12
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_2D_to_1D_NV12
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_2D_to_2D_NV12
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_1D_to_2D_Planar
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_2D_to_1D_Planar
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_2D_to_2D_Planar
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_1D_to_2D_Packed
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_2D_to_1D_Packed
    {4, 34, 0, RENDER_COPY_THREADS_MAX, 0, 0, 64, 8, 1, 1},    // CopyKernel_2D_to_2D_Packed
};

typedef struct _RENDER_COPY_CACHE_CNTL
{
    bool                           bL3CachingEnabled;
    uint32_t                       SourceSurfMemObjCtl;
    uint32_t                       TargetSurfMemObjCtl;

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

class MhwInterfacesNext;
class RenderCopyStateNext
{
public:
    RenderCopyStateNext(PMOS_INTERFACE  osInterface, MhwInterfacesNext *mhwInterfaces);

    virtual ~RenderCopyStateNext();

    //!
    //! \brief    RenderCopyStateNext initialize
    //! \details  Initialize the RenderCopyStateNext, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    GetBytesPerPixel
    //! \details  Get Bytes Per each pixel for different format
    //! \param    Formate
    //!           [in] Surface's Format 
    //! \return   int32_t
    //!           Return the numb for Byte
    //!
    int32_t GetBytesPerPixelPerPlane(MOS_FORMAT   Format);

    //!
    //! \brief    Get kernel ID 
    //! \details  choose the kernel ID for each loop.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS GetCurentKernelID();

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

    MOS_STATUS LoadStaticData(int32_t  *piCurbeOffset);

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

public:
    PMOS_INTERFACE               m_osInterface     = nullptr;
    MhwInterfacesNext           *m_mhwInterfaces   = nullptr;
    RENDERHAL_INTERFACE         *m_renderHal       = nullptr;
    MhwCpInterface              *m_cpInterface     = nullptr;
    void                        *m_pKernelBin      = nullptr;
    Kdll_State                  *m_pKernelDllState = nullptr;//!< Kernel DLL state 

    RENDERCOPY_KERNELID          m_currKernelId = KERNEL_CopyKernel_1D_to_2D_NV12;
    MEDIACOPY_RENDER_DATA        m_RenderData = {};
    MOS_SURFACE                  m_Source = {};
    MOS_SURFACE                  m_Target = {};
    bool                         m_bNullHwRenderCopy = false;

    uint32_t                     m_WalkerWidthBlockSize = 128;
    uint32_t                     m_WalkerHeightBlockSize = 8;
    const void*                  m_KernelBin = nullptr;
    uint32_t                     m_KernelBinSize = 0;
    MEDIA_CLASS_DEFINE_END(RenderCopyStateNext)
};

#endif // __MEDIA_RENDER_COPY_NEXT_H__
