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
//! \file     mhw_vdbox_avp_generic.h
//! \brief    MHW interface for constructing AVP commands for the Vdbox engine
//! \details  Impelements shared Vdbox AVP command construction functions across all platforms as templates
//!

#ifndef _MHW_VDBOX_AVP_GENERIC_H_
#define _MHW_VDBOX_AVP_GENERIC_H_

#include "mhw_vdbox_avp_interface.h"

//!  MHW Vdbox Avp generic interface
/*!
This class defines the shared Avp command construction functions across all platforms as templates
*/
template <class TAvpCmds>
class MhwVdboxAvpInterfaceGeneric : public MhwVdboxAvpInterface
{
protected:
    static const uint32_t      m_av1ScalingFactor       = (1 << 14);    //!< AV1 Scaling factor
    static const uint32_t      m_rawUVPlaneAlignment    = 4;            //!< starting Gen9 the alignment is relaxed to 4x instead of 16x
    static const uint32_t      m_reconUVPlaneAlignment  = 8;            //!< recon UV Plane alignment
    static const uint32_t      m_uvPlaneAlignmentLegacy = 8;            //!< starting Gen9 the alignment is relaxed to 4x instead of 16x

    //!
    //! \brief    Constructor
    //!
    MhwVdboxAvpInterfaceGeneric(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxAvpInterface(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief   Destructor
    //!
    virtual ~MhwVdboxAvpInterfaceGeneric() {}

    MOS_STATUS AddAvpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);
        MHW_ASSERT(params->Mode != CODECHAL_UNSUPPORTED_MODE);

        typename TAvpCmds::AVP_SURFACE_STATE_CMD cmd;

        uint32_t uvPlaneAlignment   = m_uvPlaneAlignmentLegacy;

        cmd.DW1.SurfaceId           = params->ucSurfaceStateId;
        cmd.DW1.SurfacePitchMinus1  = params->psSurface->dwPitch - 1;

        if (params->ucSurfaceStateId == srcInputPic)
        {
            uvPlaneAlignment = params->dwUVPlaneAlignment ? params->dwUVPlaneAlignment : m_rawUVPlaneAlignment;
        }
        else
        {
            uvPlaneAlignment = params->dwUVPlaneAlignment ? params->dwUVPlaneAlignment : m_reconUVPlaneAlignment;
        }

        cmd.DW2.YOffsetForUCbInPixel =
            MOS_ALIGN_CEIL((params->psSurface->UPlaneOffset.iSurfaceOffset - params->psSurface->dwOffset) / params->psSurface->dwPitch + params->psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }
};
#endif
