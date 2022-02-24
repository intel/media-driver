/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     vp_vebox_cmd_packet.h
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#ifndef __VP_VEBOX_CMD_PACKET_H__
#define __VP_VEBOX_CMD_PACKET_H__

#include "vp_vebox_cmd_packet_next.h"

namespace vp {

class VpVeboxCmdPacket : public VpVeboxCmdPacketNext
{
public:
    VpVeboxCmdPacket(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc) :
        CmdPacket(task),
        VpVeboxCmdPacketNext(task, hwInterface, allocator, mmc)
    {
    }

    virtual ~VpVeboxCmdPacket()
    {
    }

public:
    //!
    //! \brief    Render the Vebox Cmd buffer for SendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    [in,out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [out] VeboxDiIecpCmdParams
    //!           DiIecpCmd params struct to be set
    //! \param    [out] VeboxSurfaceStateCmdParams
    //!           VPHAL surface state cmd to be set
    //! \param    [out] MhwVeboxSurfaceStateCmdParams
    //!           MHW surface state cmd to be set
    //! \param    [out] VeboxStateCmdParams
    //!           MHW vebox state cmd to be set
    //! \param    [out] FlushDwParams
    //!           MHW MI_FLUSH_DW cmd to be set
    //! \param    [in] pGenericPrologParams
    //!           pointer to Generic prolog params struct to send to cmd buffer header
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS RenderVeboxCmd(
        MOS_COMMAND_BUFFER                      *CmdBuffer,
        MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
        VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
        MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
        MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

    //!
    //! \brief    Send Vecs Status Tag
    //! \details  Add MI Flush with write back into command buffer for GPU to write 
    //!           back GPU Tag. This should be the last command in 1st level batch.
    //!           This ensures sync tag will be written after rendering is complete.
    //! \param    [in] pMhwMiInterface
    //!           MHW MI interface
    //! \param    [in] pOsInterface
    //!           Pointer to OS Interface
    //! \param    [out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SendVecsStatusTag(
      PMHW_MI_INTERFACE                   pMhwMiInterface,
      PMOS_INTERFACE                      pOsInterface,
      PMOS_COMMAND_BUFFER                 pCmdBuffer);

MEDIA_CLASS_DEFINE_END(VpVeboxCmdPacket)
};

}
#endif // !__VP_VEBOX_CMD_PACKET_H__
