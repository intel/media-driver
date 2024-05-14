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
//! \file     media_vebox_copy.h
//! \brief    Common Copy interface and structure used in Vebox Engine
//! \details  Common Copy interface and structure used in Vebox Engine

#ifndef __MEDIA_VEBOX_COPY_H__
#define __MEDIA_VEBOX_COPY_H__

#include <memory>
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_specific.h"
#include "mos_util_debug.h"
#include "media_interfaces_mhw.h"
#include "mhw_vebox.h"
#include "media_copy_common.h"
class MhwCpInterface;
class MhwMiInterface;
namespace mhw { namespace vebox { class Itf; } }

#define VEBOX_COPY_CHK_STATUS(_stmt)               MOS_CHK_STATUS(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_VEBOX, _stmt)
#define VEBOX_COPY_CHK_STATUS_RETURN(_stmt)        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_VEBOX, _stmt)
#define VEBOX_COPY_CHK_NULL(_ptr)                  MOS_CHK_NULL(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_VEBOX, _ptr)
#define VEBOX_COPY_CHK_NULL_RETURN(_ptr)           MOS_CHK_NULL_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_VEBOX, _ptr)
#define VEBOX_COPY_ASSERTMESSAGE(_message, ...)    MOS_ASSERTMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_VEBOX, _message, ##__VA_ARGS__)
#define VEBOX_COPY_NORMALMESSAGE(_message, ...)    MOS_NORMALMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_VEBOX, _message, ##__VA_ARGS__)

class VeboxCopyState
{
public:
    //!
    //! \brief    Vebox Copy State constructor
    //! \details  Initialize the VeboxCopyState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    VeboxCopyState(PMOS_INTERFACE    osInterface, MhwInterfaces* mhwInterfaces);

    virtual ~VeboxCopyState();
    //!
    //! \brief    Vebox Copy State initialize
    //! \details  Initialize the Vebox Copy State, create Vebox Copy State context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    Copy main surface
    //! \details  Vebox Copy State engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_SURFACE src,
        PMOS_SURFACE dst);

    //!
    //! \brief    Copy main surface
    //! \details  Vebox Copy State engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [in] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);

    //!
    //! Is ve copy supported surface
    //! \param    [in]     surface
    //!           Pointer to surface parameters
    //! \return   true if supported, else false.
    //!
    bool IsSurfaceSupported(PMOS_RESOURCE surface);

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
    virtual MOS_STATUS SetupVeboxSurfaceState(
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams,
        PMOS_SURFACE                        inputSurface,
        PMOS_SURFACE                        outputSurface);

protected:

    //! \brief    Get resource information
    //! \details  Get resource information for the specifc surface
    //! \param    [in] pSurface
    //!           Surface pointer
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS GetResourceInfo(
        PMOS_SURFACE surface);

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
    //! Is ve copy supported format
    //! \param    [in/out]     surface mos format
    //!           Pointer to Output Surface parameters
    //! \return   true if supported, else false.
    //!
    virtual bool IsVeCopySupportedFormat(MOS_FORMAT format);

    virtual void AdjustSurfaceFormat(MOS_SURFACE &surface);

protected:
    PMOS_INTERFACE       m_osInterface   = nullptr;
    MhwInterfaces       *m_mhwInterfaces = nullptr;
    MhwMiInterface      *m_miInterface   = nullptr;
    MhwVeboxInterface   *m_veboxInterface = nullptr;
    MhwCpInterface      *m_cpInterface    = nullptr;
    MhwInterfaces::CreateParams params;
MEDIA_CLASS_DEFINE_END(VeboxCopyState)
};

#endif //__MEDIA_VEBOX_COPY_H__
