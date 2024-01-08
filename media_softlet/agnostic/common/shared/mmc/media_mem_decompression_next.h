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
//! \file     media_mem_decompression_next.h
//! \brief    Defines the common interface for mmc
//! \details
//!
#ifndef __MEDIA_MEM_DECOMPRESSION_NEXT_H__
#define __MEDIA_MEM_DECOMPRESSION_NEXT_H__

#include "mediamemdecomp.h"
#include "media_interfaces_mhw_next.h"

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

class MediaMemDeCompNext : public MediaMemDecompBaseState
{
public:

    //!
    //! \brief    Constructor, initiallize
    //!
    MediaMemDeCompNext();

    //!
    //! \brief    Copy constructor
    //!
    MediaMemDeCompNext(const MediaMemDeCompNext&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    MediaMemDeCompNext& operator=(const MediaMemDeCompNext&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~MediaMemDeCompNext();

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
    //! \param    [in] copyPitch
    //!            The 2D surface pitch
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
        uint32_t      copyPitch,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        uint32_t      bpp,
        bool          outputCompressed);

    //!
    //! \brief    Media memory tile convert
    //! \details  Convert media between Tile/Linear with decompression
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
    //! \param    [in] isTileToLinear
    //!            Convertion direction, true: tile->linear, false: linear->tile
    //! \param    [in] outputCompressed
    //!            true means apply compression on output surface, else output uncompressed surface
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS MediaMemoryTileConvert(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        bool          isTileToLinear,
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
        PMOS_INTERFACE                     osInterface,
        MhwInterfacesNext*                 mhwInterfaces);

    //!
    //! \brief    GetDecompState's mosinterface
    //! \details  get the mosinterface
    //! \return   mosinterface
    //!
    virtual PMOS_INTERFACE GetDecompStateMosInterface()
    {
        return m_osInterface;
    }

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
    //! Is Vebox Tile Convert/Decompression Format supported
    //! \param    [in/out]     surface
    //!           Pointer to Output Surface parameters
    //! \return   true if supported, else false.
    //!
    bool IsFormatSupported(PMOS_SURFACE surface);

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

public:
    // Add Public Numbers
    // Interface
    PMOS_INTERFACE              m_osInterface;

protected:

    // Interface
    std::shared_ptr<mhw::vebox::Itf>        m_veboxItf = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf = nullptr;
    MhwCpInterface                        * m_cpInterface;
    bool                                    m_veboxMMCResolveEnabled;
    PMOS_MUTEX                              m_renderMutex = nullptr;

    MediaUserSettingSharedPtr m_userSettingPtr = nullptr;  //!< UserSettingInstance
MEDIA_CLASS_DEFINE_END(MediaMemDeCompNext)
};

#endif //