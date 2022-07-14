/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe_m_base.h
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline xe m base
//!

#ifndef __CODECHAL_AV1_VDENC_PACKET_XE_M_BASE_H__
#define __CODECHAL_AV1_VDENC_PACKET_XE_M_BASE_H__

#include "encode_av1_vdenc_packet.h"
#include "codechal_hw_g12_X.h"

//!
//! \struct AtomicScratchBuffer
//! \brief  The sturct of Atomic Scratch Buffer
//!
struct AtomicScratchBufferAv1
{
    PMOS_RESOURCE   resAtomicScratchBuffer;     //!> Handle of eStatus buffer
    uint32_t        *pData;                     //!> Pointer of the buffer of actual data
    uint16_t        encodeUpdateIndex;          //!> used for VDBOX update encode status
    uint16_t        tearDownIndex;              //!> Reserved for future extension
    uint32_t        zeroValueOffset;            //!> Store the result of the ATOMIC_CMP
    uint32_t        operand1Offset;             //!> Operand 1 of the ATOMIC_CMP
    uint32_t        operand2Offset;             //!> Operand 2 of the ATOMIC_CMP
    uint32_t        operand3Offset;             //!> Copy of the operand 1
    uint32_t        size;                       //!> Size of the buffer
    uint32_t        operandSetSize;             //!> Size of Operand set
};

namespace encode
{
    class Av1VdencPktXe_M_Base : public Av1VdencPkt
    {
    public:
        Av1VdencPktXe_M_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface) :
            Av1VdencPkt(pipeline, task, hwInterface)
        {
            auto hwInterfaceG12 = dynamic_cast<CodechalHwInterfaceG12 *>(m_hwInterface);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterfaceG12);
        }

        virtual ~Av1VdencPktXe_M_Base() {}

        MOS_STATUS Prepare() override;


        //!
        //! \brief  Add the command sequence into the commandBuffer and
        //!         and return to the caller task
        //! \param  [in] commandBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Submit(
            MOS_COMMAND_BUFFER* commandBuffer,
            uint8_t packetPhase = otherPacket) override;

    protected:
        MOS_STATUS PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);
        MOS_STATUS AddOneTileCommands(
            MOS_COMMAND_BUFFER  &cmdBuffer,
            uint32_t            tileRow,
            uint32_t            tileCol,
            uint32_t            tileRowPass = 0);

        MOS_STATUS Construct3rdLevelBatch();

        void UpdateParameters() override;

        MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);
        MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer);

        MOS_STATUS AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS AllocateResources() override;

        virtual MOS_STATUS CalculateAvpPictureStateCommandSize(uint32_t * commandsSize, uint32_t * patchListSize) override;
        virtual MOS_STATUS CalculateAvpCommandsSize() override;

        MOS_STATUS ReadAvpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer) override;
        MOS_STATUS RegisterPostCdef();
        MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface);

        MHW_SETPAR_DECL_HDR(AVP_IND_OBJ_BASE_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

        MHW_SETPAR_DECL_HDR(AVP_TILE_CODING);

        MOS_STATUS AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_AVP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_AVP_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

#if USE_CODECHAL_DEBUG_TOOL
        MOS_STATUS DumpStatistics();
#endif  // USE_CODECHAL_DEBUG_TOOL

        bool  m_userFeatureUpdated_post_cdef = false;  //!< Inidate if mmc user feature key for post cdef is updated

    private:
        uint16_t m_tileColStartSb[64];  //!< tile column start SB
        uint16_t m_tileRowStartSb[64];  //!< tile row start SB
    MEDIA_CLASS_DEFINE_END(encode__Av1VdencPktXe_M_Base)
    };

}

#endif
