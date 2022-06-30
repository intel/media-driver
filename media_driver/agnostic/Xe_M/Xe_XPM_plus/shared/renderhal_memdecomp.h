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
//! \file     renderhal_memdecomp.h
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details
//!

#ifndef __RENDERHAL_MEMDECOMP_H__
#define __RENDERHAL_MEMDECOMP_H__

#include "mhw_render_legacy.h"
#include "mos_os.h"
#include "mediamemdecomp.h"
#include "renderhal_legacy.h"

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_RENDER sub-comp
//------------------------------------------------------------------------------
#define RENDERHAL_MEMORY_DECOMP_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _expr)

#define RENDERHAL_MEMORY_DECOMP_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define RENDERHAL_MEMORY_DECOMP_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define RENDERHAL_MEMORY_DECOMP_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define RENDERHAL_MEMORY_DECOMP_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER)

#define RENDERHAL_MEMORY_DECOMP_EXITMESSAGE(_message, ...)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define RENDERHAL_MEMORY_DECOMP_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define RENDERHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define RENDERHAL_MEMORY_DECOMP_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt, _message, ##__VA_ARGS__)

#define RENDERHAL_MEMORY_DECOMP_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define RENDERHAL_MEMORY_DECOMP_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define RENDERHAL_MEMORY_DECOMP_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

//!
//! \class MediaRenderDecompState
//! \brief Media render inplace memory decompression state. This class defines the member fields
//!        functions etc used by memory decompression.
//!
class MediaRenderDecompState : public MediaMemDecompBaseState
{
public:

    //!
    //! \brief    Constructor, initiallize
    //!
    MediaRenderDecompState();

    //!
    //! \brief    Copy constructor
    //!
    MediaRenderDecompState(const MediaRenderDecompState &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaRenderDecompState &operator=(const MediaRenderDecompState &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaRenderDecompState();

    //!
    //! \brief    Media memory decompression
    //! \details  Entry point to decompress media memory
    //! \param    [in] targetResource
    //!            The surface will be decompressed
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS MemoryDecompress(
        PMOS_RESOURCE targetResource);


    //!
    //! \brief    Media memory copy
    //! \details  Entry point to copy media memory, input can support both compressed/uncompressed
    //! \param    [in] inputSurface
    //!            The surface resource will be decompressed
    //! \param    [out] outputSurface
    //!            The target uncompressed surface resource will be copied to
    //! \param    [in] outputCompressed
    //!            The surface resource will compressed if true for compressilbe surface
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS MediaMemoryCopy(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        bool          outputCompressed);

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
    //! \param    [in] bpp
    //!            bit per pixel for copied surfaces
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
        uint32_t      bpp,
        bool          bOutputCompressed);

    //!
    //! \brief    Initialize memory decompress state
    //! \param    [in] osInterface
    //!           Os interface
    //! \param    [in] cpInterface
    //!           CP interface
    //! \param    [in] mhwMiInterface
    //!           Hw MI interface
    //! \param    [in] MhwRenderInterface
    //!           Hw render interface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS Initialize(
        PMOS_INTERFACE                  osInterface,
        MhwCpInterface                  *cpInterface,
        PMHW_MI_INTERFACE               mhwMiInterface,
        MhwRenderInterface              *renderInterface);

    virtual PMOS_INTERFACE GetDecompStateMosInterface()
    {
        return m_osInterface;
    }
protected:
    PMOS_INTERFACE              m_osInterface       = nullptr;
    PMHW_MI_INTERFACE           m_mhwMiInterface    = nullptr;
    MhwCpInterface              *m_cpInterface      = nullptr;
    MhwRenderInterface          *m_renderInterface  = nullptr;
};

#endif // __RENDERHAL_MEMDECOMP_H__
