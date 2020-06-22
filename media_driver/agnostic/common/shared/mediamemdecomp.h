/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mediamemdecomp.h
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details  

#ifndef __MEDIAMEMORYDECOMP_H__
#define __MEDIAMEMORYDECOMP_H__

#include "mos_os.h"


class MediaMemDecompBaseState
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaMemDecompBaseState()
    {
    }

    //!
    //! \brief    Deconstructor
    //!
    virtual ~MediaMemDecompBaseState()
    {
    }

    //!
    //! \brief    Media memory decompression
    //! \details  Entry point to decompress media memory
    //! \param    targetResource
    //!           [in] The surface will be decompressed
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MemoryDecompress(
        PMOS_RESOURCE targetResource) = 0;

    //!
    //! \brief    Media memory decompression
    //! \details  Entry point to decompress media memory and copy
    //! \param    [in] inputSurface
    //!            The source surface resource
    //! \param    [out] outputSurface
    //!            The target surface resource will be copied to
    //! \param    [in] bOutputCompressed
    //!            true means apply compression on output surface, else output uncompressed surface
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS MediaMemoryCopy(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        bool          bOutputCompressed)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Media memory copy 2D
    //! \details  Entry point to decompress media memory and copy with byte in unit
    //! \param    [in] inputSurface
    //!            The source surface resource
    //! \param    [out] outputSurface
    //!            The target surface resource will be copied to
    //! \param    [in] copyWidth
    //!            The 2D surface Width
    //! \param    [in] copyHeight
    //!            The 2D surface height
    //! \param    [in] copyInputOffset
    //!            The offset of copied surface from
    //! \param    [in] copyOutputOffset
    //!            The offset of copied to
    //! \param    [in] bOutputCompressed
    //!            true means apply compression on output surface, else output uncompressed surface
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS MediaMemoryCopy2D(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        bool          bOutputCompressed)
    {
        return MOS_STATUS_SUCCESS;
    }

};

#endif // __MEDIAMEMORYDECOMP_H__
