/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_vp9_dynamic_scal_packet_xe_lpm_plus.h
//! \brief    Defines the interface to vp9 dynamic scaling (reference frame scaling) packet
//!

#ifndef __ENCODE_VP9_DYNAMIC_SCAL_PACKET_XE_LPM_PLUS_H__
#define __ENCODE_VP9_DYNAMIC_SCAL_PACKET_XE_LPM_PLUS_H__

#include "encode_vp9_vdenc_packet.h"

namespace encode
{

class Vp9DynamicScalPktXe_Lpm_Plus_Base : public Vp9VdencPkt
{
public:
    //!
    //! \brief  Vp9DysRefFramePkt constructor
    //!
    Vp9DynamicScalPktXe_Lpm_Plus_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : Vp9VdencPkt(pipeline, task, hwInterface) {}

    //!
    //! \brief  Vp9DysRefFramePkt destructor
    //!
    virtual ~Vp9DynamicScalPktXe_Lpm_Plus_Base(){};

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    //!
    //! \brief  One frame is completed
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in] rcsStatus
    //!         pointer to status buffer which for RCS
    //! \param  [in, out] statusReport
    //!         pointer of EncoderStatusReport
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

    //!
    //! \brief  Dump output resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpOutput() override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "PAK_PASS_DYS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

protected:
    //!
    //! \brief  Allocate resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Add VDENC_WALKER_STATE commands to command buffer
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWalkerStateCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Add VD_CONTROL_STATE initialization
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdControlInitialize(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add command to set hcp pipe mode select parameter
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeModeSelectCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override;

    //!
    //! \brief    Set HCP surfaces' parameters
    //! \param    [in, out] surfacesParams
    //!           Pointer to surfaces' parameters structures
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHcpSurfacesParams(
        MHW_VDBOX_SURFACE_PARAMS *surfacesParams) override;

    //!
    //! \brief    Add command to set Hcp Pipe Buffer Address values
    //! \param    [in, out] cmdBuffer
    //!           Command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeBufAddrCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override;

    //!
    //! \brief  Add command to set Vdenc Pipe Buffer Address values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencPipeBufAddrCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Add command to set vdenc pipe mode select values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencPipeModeSelectCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override { return MOS_STATUS_SUCCESS; };

    //!
    //! \brief  Patch picture level command sequence into the commandBuffer
    //! \param  [in] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PatchPictureLevelCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint8_t             packetPhase);

    //!
    //! \brief    Add hcp tile coding paramesters to command buffer
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpTileCodingCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add one tile level commands sequence into the commandBuffer
    //! \param  [in, out] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \param  [in] tileRow
    //!         tile row index
    //! \param  [in] tileCol
    //!         tile column index
    //! \param  [in] tileRowPass
    //!         number of tile row pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0);

    //!
    //! \brief  Add one tile level commands sequence into the commandBuffer (No TLBB)
    //! \param  [in, out] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \param  [in] tileRow
    //!         tile row index
    //! \param  [in] tileCol
    //!         tile column index
    //! \param  [in] tileRowPass
    //!         number of tile row pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddOneTileCommandsNoTLBB(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0);

    //!
    //! \brief  Patch slice level command sequence into the commandBuffer
    //! \param  [in] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PatchSliceLevelCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint8_t             packetPhase);

    //!
    //! \brief  Retrieves the MFX image status information
    //! \param  [in] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatus(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Dump input resources or infomation before submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpInput() override;

    bool m_waReadVDEncOverflowStatus = false;  //!< Read vdenc overflow status used flag

    //// Resource handles
    PMOS_RESOURCE m_resVdencBrcUpdateDmemBufferPtr[2] = {nullptr, nullptr};  //!< One for 1st pass of next frame, and the other for the next pass of current frame.

MEDIA_CLASS_DEFINE_END(encode__Vp9DynamicScalPktXe_Lpm_Plus_Base)
};

} // namespace encode

#endif  // !__ENCODE_VP9_DYNAMIC_SCAL_PACKET_XE_LPM_PLUS_H__
