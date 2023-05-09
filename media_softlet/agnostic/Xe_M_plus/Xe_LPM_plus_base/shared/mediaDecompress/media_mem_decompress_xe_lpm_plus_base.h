/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     media_mem_decompress_xe_lpm_plus_base.h
//! \brief    Common interface and structure used in media decompress
//! \details  Common interface and structure used in media decompresswhich are platform independent
//!
#ifndef __MEDIA_MEM_DECOMPRESS_XE_LPM_PLUS_BASE_H__
#define __MEDIA_MEM_DECOMPRESS_XE_LPM_PLUS_BASE_H__

#include "media_mem_decompression_next.h"
#include "media_perf_profiler.h"

class MediaMemDeCompNext_Xe_Lpm_Plus_Base: virtual public MediaMemDeCompNext
{
public:

public:
    //!
    //! \brief    Constructor, initiallize
    //!
    MediaMemDeCompNext_Xe_Lpm_Plus_Base();

    //!
    //! \brief    Deconstructor
    //!
    virtual ~MediaMemDeCompNext_Xe_Lpm_Plus_Base()
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_tempLinearSurface.OsResource);

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();

        if (!perfProfiler)
        {
            MOS_OS_ASSERTMESSAGE("Destroy MediaPerfProfiler failed!");
        }
        else
        {
            MediaPerfProfiler::Destroy(perfProfiler, (void *)this, m_osInterface);
        }
    }

    //!
    //! \brief    Creat platform specific MMD (media memory decompression) HW interface
    //! \param    [in] surface
    //!           Pointers to surface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS RenderDecompCMD(
        PMOS_SURFACE surface);

    //!
    //! \brief    Media memory decompression Enabled or not
    //! \details  Media memory decompression Enabled or not
    //!
    //! \return   true if MMC decompression enabled, else false.
    //!
    virtual MOS_STATUS IsVeboxDecompressionEnabled();

    //!
    //! \brief    Media memory double buffer decompression render
    //! \details  Entry point to decompress media memory
    //! \param    [in] surface
    //!           Input surface to be copyed and decompressed
    //! \param    [out] surface
    //!           Output surface for clear data
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS RenderDoubleBufferDecompCMD(
        PMOS_SURFACE inputSurface,
        PMOS_SURFACE outputSurface);

    MOS_STATUS LinearCopyWith64Aligned(
        PMOS_SURFACE inputSurface,
        PMOS_SURFACE outputSurface);

    MOS_STATUS ReAllocateLinearSurface(
        PMOS_SURFACE            pSurface,
        PCCHAR                  pSurfaceName,
        MOS_FORMAT              Format,
        MOS_GFXRES_TYPE         DefaultResType,
        uint32_t                dwWidth,
        uint32_t                dwHeight,
        bool* pbAllocated);

    //!
    //! Vebox Send Vebox_Tile_Convert command
    //! \param    [in,out] cmdBuffer
    //!           Pointer to PMOS_COMMAND_BUFFER command parameters
    //! \param    [in]     surface
    //!           Pointer to Input Surface parameters
    //! \param    [in]     surface
    //!           Pointer to Output Surface parameters
    //! \param    [in]    streamID
    //!            Stream ID for current surface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS VeboxSendVeboxTileConvertCMD(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_SURFACE        inputSurface,
        PMOS_SURFACE        outputSurface,
        uint32_t            streamID);

protected:
    MOS_SURFACE m_tempLinearSurface = {};
    int32_t     m_multiprocesssinglebin = 0;   //!< multi process single binary flag
MEDIA_CLASS_DEFINE_END(MediaMemDeCompNext_Xe_Lpm_Plus_Base)
};

#endif // __MEDIA_MEM_DECOMPRESS_XE_LPM_PLUS_BASE_H__

