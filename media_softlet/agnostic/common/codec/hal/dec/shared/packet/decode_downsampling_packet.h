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
//! \file     decode_downsampling_packet.h
//! \brief    Defines the common interface for decode down sampling sub packet
//! \details  The decode down sampling feature interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!

#ifndef __DECODE_DOWNSAMPLING_PACKET_H__
#define __DECODE_DOWNSAMPLING_PACKET_H__

#include "decode_sub_packet.h"
#include "codechal_setting.h"
#include "decode_pipeline.h"
#include "media_sfc_interface.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

class DecodeDownSamplingPkt : public DecodeSubPacket
{
public:
    //!
    //! \brief  Decode down sampling sub packet constructor
    //!
    DecodeDownSamplingPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief  Decode down sampling sub packet destructor
    //!
    virtual ~DecodeDownSamplingPkt();

    //!
    //! \brief  Initialize the down sampling sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Update the parameters for down sampling sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Execute sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer);

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //! \brief  Check if down sampling supported for current frame
    //! \return bool
    //!         true if down sampling supported for current frame
    bool IsSupported() { return m_isSupported; }

    //! \brief Set down sampling mode
    //! \param [in, out] mode
    //!        down sampling mode
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode) = 0;

    //!
    //! \brief    Sfc Command Size
    //! \details  Calculate Command size of SFC commands.
    //! \return   uint32_t
    //!           Return calculated size
    //!
    uint32_t GetSfcCmdSize() { return (m_sfcInterface ? (m_sfcInterface->GetSfcCommandSize()) : 0); };

protected:
    virtual MOS_STATUS InitSfcParams(VDBOX_SFC_PARAMS &sfcParams);

    std::shared_ptr<MediaSfcInterface>  m_sfcInterface = nullptr;
    DecodeBasicFeature        *m_basicFeature = nullptr;
    DecodeDownSamplingFeature *m_downSampling = nullptr;
    bool                       m_isSupported  = false;

    VDBOX_SFC_PARAMS           m_sfcParams;

MEDIA_CLASS_DEFINE_END(decode__DecodeDownSamplingPkt)
};

}

#endif  // !_DECODE_PROCESSING_SUPPORTED
#endif  // !__DECODE_DOWNSAMPLING_PACKET_H__
