/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_vp9_vdenc_packet_xe_lpm_plus_base.h
//! \brief    Defines the interface to adapt to vp9 vdenc encode CMD packet
//!

#ifndef __ENCODE_VP9_VDENC_PACKET_XE_LPM_PLUS_BASE_H__
#define __ENCODE_VP9_VDENC_PACKET_XE_LPM_PLUS_BASE_H__

#include "encode_vp9_vdenc_packet.h"
#include "mhw_vdbox_hcp_itf.h"

namespace encode
{

class Vp9VdencPktXe_Lpm_Plus_Base : public Vp9VdencPkt
{
public:
    //!
    //! \brief  Vp9VdencPktXe_Lpm_Plus_Base constructor
    //!
    Vp9VdencPktXe_Lpm_Plus_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : Vp9VdencPkt(pipeline, task, hwInterface) {}

    //!
    //! \brief  Vp9VdencPktXe_Lpm_Plus_Base destructor
    //!
    virtual ~Vp9VdencPktXe_Lpm_Plus_Base() {}

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init() override;

    //!
    //! \brief  Prepare the parameters for command submission
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Destroy the media packet and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy() override;

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(
        MOS_COMMAND_BUFFER *commandBuffer,
        uint8_t             packetPhase = otherPacket) override;

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

protected:

    //!
    //! \brief  Allocate resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Add picture hcp commands
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddPictureHcpCommands(
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
    //! \brief  Add picture vdenc commands
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddPictureVdencCommands(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add command to set vdenc pipe mode select values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencPipeModeSelectCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override;

    //!
    //! \brief  Set vdenc pipe mode select parameter
    //! \param  [in] pipeModeSelectParams
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencPipeModeSelectParams(
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams) override;

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
    //! \brief    Add VDENC_WALKER_STATE commands to command buffer
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWalkerStateCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) override;

    //!
    //! \brief    Add VDENC_WEIGHT_OFFSET commands to command buffer
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWeightOffsetsStateCmd(
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
        uint32_t            tileRowPass=0);

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
    //! \brief  Patch tile level command sequence into the commandBuffer
    //! \param  [in] cmdBuffer
    //!         The command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PatchTileLevelCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint8_t             packetPhase);

    //!
    //! \brief  Add conditional batch buffer end for last pass
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddCondBBEndForLastPass(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add VD_CONTROL_STATE for HCP pipe initialization
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdControlInitialize(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add VD_CONTROL_STATE memory implicit flush
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdControlMemoryImplicitFlush(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add VD_CONTROL_STATE scalable mode pipe lock
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdControlScalableModePipeLock(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add VD_CONTROL_STATE scalable mode pipe unlock
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdControlScalableModePipeUnlock(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Update parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateParameters() override;

MEDIA_CLASS_DEFINE_END(encode__Vp9VdencPktXe_Lpm_Plus_Base)
};

}  // namespace encode

#endif  // !__ENCODE_VP9_VDENC_PACKET_XE_LPM_PLUS_BASE_H__