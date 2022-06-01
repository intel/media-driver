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
//! \file     decode_sub_pipeline.h
//! \brief    Defines the common interface for decode sub pipeline
//! \details  The decode sub pipeline interface is further sub-divided by decode standard,
//!           this file is for the base interface which is shared by all decoders.
//!
#ifndef __DECODE_SUB_PIPELINE_H__
#define __DECODE_SUB_PIPELINE_H__

#include "decode_scalability_defs.h"
#include "media_packet.h"
#include "media_task.h"
#include "media_context.h"
#include "codechal_setting.h"

namespace decode {

class DecodePipeline;
struct DecodePipelineParams;

class DecodeSubPipeline
{
public:
    using PacketListType       = std::map<uint32_t, MediaPacket *>;
    using ActivePacketListType = std::vector<PacketProperty>;

    //!
    //! \brief  Decode sub pipeline constructor
    //!
    DecodeSubPipeline(DecodePipeline* pipeline, MediaTask* task, uint8_t numVdbox);

    //!
    //! \brief  Decode sub pipeline destructor
    //!
    virtual ~DecodeSubPipeline();

    //!
    //! \brief  Initialize the sub pipeline
    //! \param  [in] settings
    //!         Reference to the Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(CodechalSetting& settings) = 0;

    //!
    //! \brief  Prepare interal parameters
    //! \param  [in] params
    //!         Reference to decode pipeline parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(DecodePipelineParams& params) = 0;

    //!
    //! \brief  Get packet list
    //! \return PacketListType
    //!         Return the packet list
    //!
    PacketListType & GetPacketList();

    //!
    //! \brief  Get active packets list
    //! \return ActivePacketListType
    //!         Return the active packets list
    //!
    ActivePacketListType & GetActivePackets();

    //!
    //! \brief  Get media function for context switch
    //! \return MediaFunction
    //!         Return the media function
    //!
    virtual MediaFunction GetMediaFunction() = 0;

    //!
    //! \brief  Get scalability parameters
    //! \return DecodeScalabilityPars&
    //!         Return the scalability parameters
    //!
    DecodeScalabilityPars& GetScalabilityPars();

protected:
    //!
    //! \brief  Register one packet into packet list
    //! \param  [in] packetId
    //!         Packet Id
    //! \param  [in] packet
    //!         The packet corresponding to packetId
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegisterPacket(uint32_t packetId, MediaPacket& packet);

    //!
    //! \brief  Activate one packet and add it to active packet list
    //! \param  [in] packetId
    //!         Packet Id
    //! \param  [in] immediateSubmit
    //!         Indicate if this packet to activate is needed to submit immediately after been added to task
    //! \param  [in] pass
    //!         pass belongs to the Packet
    //! \param  [in] pipe
    //!         pipe belongs to the Packet
    //! \param  [in] pipe numbers
    //!         pipe numbers the Packet needs to use
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ActivatePacket(uint32_t packetId, bool immediateSubmit,
                              uint8_t pass, uint8_t pipe, uint8_t pipeNum = 1);

    //!
    //! \brief  Reset sub pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Reset();

    //!
    //! \brief  Initialize scalability parameters
    //!
    virtual void InitScalabilityPars(PMOS_INTERFACE osInterface) = 0;

protected:
    DecodePipeline*         m_pipeline = nullptr;    //!< Decode pipeline
    MediaTask*              m_task = nullptr;        //!< Decode task
    uint8_t                 m_numVdbox = 1;          //!< Number of Vdbox

    PacketListType          m_packetList;            //!< Packets list
    ActivePacketListType    m_activePacketList;      //!< Active packets property list

    DecodeScalabilityPars   m_decodeScalabilityPars; //!< Decode scalability parameters

MEDIA_CLASS_DEFINE_END(decode__DecodeSubPipeline)
};

}//decode

#endif // !__DECODE_PIPELINE_H__
