/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vphal_render_vebox_memdecomp_g12.h
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details  
//!
#ifndef __VPHAL_RENDER_VEBOX_MEMDECOMP_G12_H__
#define __VPHAL_RENDER_VEBOX_MEMDECOMP_G12_H__

#include "vphal_render_vebox_memdecomp.h"

class MediaVeboxDecompStateG12 : public MediaVeboxDecompState
{
public:
    //!
    //! \brief    Constructor, initiallize
    //!
    MediaVeboxDecompStateG12();

    //!
    //! \brief    Deconstructor
    //!
    virtual ~MediaVeboxDecompStateG12()
    {

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
};
#endif //__VPHAL_RENDER_VEBOX_MEMDECOMP_G12_H__
