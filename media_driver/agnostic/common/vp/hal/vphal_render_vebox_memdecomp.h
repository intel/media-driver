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
//! \file     vphal_render_vebox_memdecomp.h
//! \brief    Defines data structures and interfaces for media memory decompression.
//! \details  
//!

#ifndef __VPHAL_RENDER_VEBOX_MEMDECOMP_H__
#define __VPHAL_RENDER_VEBOX_MEMDECOMP_H__

#include "mhw_render.h"
#include "mos_os.h"
#include "vphal_debug.h"
#include "mediamemdecomp.h"
#include "vphal_common.h"
#include "mhw_vebox.h"
#include "renderhal.h"

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_RENDER sub-comp
//------------------------------------------------------------------------------
#define VPHAL_MEMORY_DECOMP_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _expr)

#define VPHAL_MEMORY_DECOMP_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_MEMORY_DECOMP_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_MEMORY_DECOMP_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_MEMORY_DECOMP_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER)

#define VPHAL_MEMORY_DECOMP_EXITMESSAGE(_message, ...)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_MEMORY_DECOMP_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_MEMORY_DECOMP_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_MEMORY_DECOMP_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt, _message, ##__VA_ARGS__)

#define VPHAL_MEMORY_DECOMP_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_MEMORY_DECOMP_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_MEMORY_DECOMP_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

//!
//! \class MediaVeboxDecompState
//! \brief Media vebox inplace memory decompression state. This class defines the member fields
//!        functions etc used by memory decompression.
//!
class MediaVeboxDecompState : public MediaMemDecompBaseState
{
public:

    //!
    //! \brief    Constructor, initiallize
    //!
    MediaVeboxDecompState();

    //!
    //! \brief    Copy constructor
    //!
    MediaVeboxDecompState(const MediaVeboxDecompState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaVeboxDecompState& operator=(const MediaVeboxDecompState&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaVeboxDecompState();

    //!
    //! \brief    Media memory decompression render
    //! \details  Entry point to decompress media memory
    //! \param    [in] surface
    //!           Input surface will be decompressed
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS RenderDecompCMD(
        PMOS_SURFACE surface) = 0;

    //!
    //! \brief    Media memory double buffer decompression render
    //! \details  Entry point to decompress media memory
    //! \param    [in] surface
    //!           Input surface to be copied and decompressed
    //! \param    [out] surface
    //!           Output surface for clear data
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS RenderDoubleBufferDecompCMD(
        PMOS_SURFACE inputSurface,
        PMOS_SURFACE outputSurface) = 0;

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
    //! \brief    Media memory decompression Enabled or not
    //! \details  Media memory decompression Enabled or not
    //!
    //! \return   true if MMC decompression enabled, else false.
    //!
    virtual MOS_STATUS IsVeboxDecompressionEnabled() = 0;

    //!
    //! \brief    Media memory decompression
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
    //! \brief    Initialize memory decompress state
    //! \param    [in] osInterface
    //!           Os interface
    //! \param    [in] cpInterface
    //!           CP interface
    //! \param    [in] mhwMiInterface
    //!           Hw MI interface
    //! \param    [in] veboxInterface
    //!           Vebox interface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS Initialize(
        PMOS_INTERFACE                  osInterface,
        MhwCpInterface                 *cpInterface,
        PMHW_MI_INTERFACE               mhwMiInterface,
        PMHW_VEBOX_INTERFACE            veboxInterface);

protected:

    //!
    //! \brief    Get resource information
    //! \details  Get resource information for the specifc surface
    //! \param    [in] pSurface
    //!           Surface pointer
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS GetResourceInfo(
        PMOS_SURFACE surface);

    //!
    //! \brief    Setup Vebox_Surface_State Command parameter
    //! \param    [in/out] mhwVeboxSurfaceStateCmdParams
    //!            Pointer to VEBOX_SURFACE_STATE command parameters
    //! \param    [in] surface
    //!           Input surface pointer
    //! \param    [in] surface
    //!           output surface pointer
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS SetupVeboxSurfaceState(
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams,
        PMOS_SURFACE                        inputSurface, 
        PMOS_SURFACE                        outputSurface);

    //!
    //! \brief    Get resource information
    //! \details  Get resource information for the specifc surface
    //! \param    [in] cmdBuffer
    //!           CmdBuffer pointer
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS InitCommandBuffer(
        PMOS_COMMAND_BUFFER              cmdBuffer);

    //!
    //! Vebox Send Vebox_Tile_Convert command
    //! \param    [in/out] cmdBuffer
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
        uint32_t            streamID) = 0;

    //!
    //! Is Vebox Decompression Format supported
    //! \param    [in/out]     surface
    //!           Pointer to Output Surface parameters
    //! \return   true if supported, else false.
    //!
    bool IsDecompressionFormatSupported(PMOS_SURFACE surface);

    enum MEDIA_TILE_TYPE
    {
        MEMORY_MEDIACOMPRESSION_ENABLE = 0,
        MEMORY_RENDERCOMPRESSION_ENABLE,
    };

    enum MEDIA_SURFACE_TILE_RESOURCE_MODE
    {
        TRMODE_NONE = 0,
        TRMODE_TILEYF,
        TRMODE_TILEYS,
        TRMODE_UNKNOWN,
    };

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual void CreateSurfaceDumper();
#endif

public:
    // Add Public Numbers

    // Interface
    PMOS_INTERFACE              m_osInterface;

protected:

    // Interface
    PMHW_VEBOX_INTERFACE        m_veboxInterface;
    PMHW_MI_INTERFACE           m_mhwMiInterface;
    MhwCpInterface             *m_cpInterface;
    bool                        m_veboxMMCResolveEnabled;
#if (_DEBUG || _RELEASE_INTERNAL)
    VphalSurfaceDumper         *m_surfaceDumper = nullptr;
    uint32_t                    m_surfaceDumpCounter = 0;
#endif
};

#endif // __VPHAL_RENDER_VEBOX_MEMDECOMP_EXT_H__
