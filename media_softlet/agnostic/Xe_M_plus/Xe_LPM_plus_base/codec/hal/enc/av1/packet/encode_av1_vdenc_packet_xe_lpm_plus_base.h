/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe_lpm_plus_base.h
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline Xe_LPM_plus+
//!

#ifndef __CODECHAL_AV1_VDENC_PACKET_XE_LPM_PLUS_BASE_H__
#define __CODECHAL_AV1_VDENC_PACKET_XE_LPM_PLUS_BASE_H__

#include "encode_av1_vdenc_packet.h"
#include "encode_utils.h"
#include "codec_hw_xe_lpm_plus_base.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"
namespace encode
{
class Av1VdencPktXe_Lpm_Plus_Base : public Av1VdencPkt
{
public:
    Av1VdencPktXe_Lpm_Plus_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : Av1VdencPkt(pipeline, task, hwInterface)
    {
        auto hw = dynamic_cast<CodechalHwInterfaceXe_Lpm_Plus_Base*>(m_hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hw);

        m_avpItf       = std::static_pointer_cast<mhw::vdbox::avp::Itf>(hw->GetAvpInterfaceNext());
    }

    MOS_STATUS Init() override;

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Submit(
        MOS_COMMAND_BUFFER *commandBuffer,
        uint8_t             packetPhase = otherPacket) override;

protected:
    MOS_STATUS PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);

    virtual MOS_STATUS AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0);

    virtual MOS_STATUS AddCommandsExt(MOS_COMMAND_BUFFER& cmdBuffer) { return MOS_STATUS_SUCCESS; };

    MOS_STATUS Construct3rdLevelBatch();

    void UpdateParameters() override;

    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

    MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS AllocateResources() override;

    MOS_STATUS RegisterPostCdef();

    MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_TILE_CODING);

    MOS_STATUS AddAllCmds_AVP_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpStatistics();
#endif  // USE_CODECHAL_DEBUG_TOOL

    bool                   m_userFeatureUpdated_post_cdef                  = false;    //!< Inidate if mmc user feature key for post cdef is updated
    uint16_t               m_tileColStartSb[64]                            = {};       //!< tile column start SB
    uint16_t               m_tileRowStartSb[64]                            = {};       //!< tile row start SB

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPktXe_Lpm_Plus_Base)
};
}  // namespace encode

#endif
