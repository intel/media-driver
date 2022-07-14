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
//! \file        mos_decompression.h
//! \brief       This module defines MOS decompression on linux platform
//!
#ifndef __MOS_DECOMPRESSION_BASE_H__
#define __MOS_DECOMPRESSION_BASE_H__

#include "mos_os.h"
#include "mediamemdecomp.h"

class MosDecompressionBase
{
public:
    //!
    //! \brief    constructor
    //!
    MosDecompressionBase(PMOS_CONTEXT osDriverContext);

    //!
    //! \brief    destructor
    //!
    virtual ~MosDecompressionBase();

    void **GetMediaMemDecompState()
    {
        return (void **)&m_mediaMemDecompState;
    }

    //!
    //! \brief    Media memory decompression
    //! \details  Entry point to decompress media memory
    //! \param    [in] osResource
    //!           The surface will be decompressed
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS MemoryDecompress(
        PMOS_RESOURCE osResource);

    //!
    //! \brief    Media memory copy
    //! \details  Entry point to copy media memory, input can support both compressed/uncompressed
    //! \param    [in] inputResource
    //!            The surface resource will be decompressed
    //! \param    [out] outputResource
    //!            The target uncompressed surface resource will be copied to
    //! \param    [in] outputCompressed
    //!            The surface resource will compressed if true for compressilbe surface
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS MediaMemoryCopy(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        bool          outputCompressed);

    //!
    //! \brief    Media memory copy 2D
    //! \details  Entry point to decompress media memory and copy with byte in unit
    //! \param    [in] inputResource
    //!            The source surface resource
    //! \param    [out] outputResource
    //!            The target surface resource will be copied to
    //! \param    [in] copyWidth
    //!            The 2D surface Width
    //! \param    [in] copyHeight
    //!            The 2D surface height
    //! \param    [in] copyInputOffset
    //!            The offset of copied surface from
    //! \param    [in] copyOutputOffset
    //!            The offset of copied to
    //! \param    [in] bpp
    //!            The copy format bit per pixel
    //! \param    [in] outputCompressed
    //!            true means apply compression on output surface, else output uncompressed surface
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS MediaMemoryCopy2D(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        uint32_t      bpp,
        bool          outputCompressed);

protected:
    MediaMemDecompBaseState *m_mediaMemDecompState = nullptr;  //!> Internal media state for memory decompression
MEDIA_CLASS_DEFINE_END(MosDecompressionBase)
};
#endif // __MOS_DECOMPRESSION_BASE_H__
