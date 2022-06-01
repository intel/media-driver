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
//! \file     decode_hevc_packet_real_tile_xe_m_base.h
//! \brief    Defines the implementation of hevc real tile decode packet
//!

#ifndef __DECODE_HEVC_PACKET_REAL_TILE_XE_M_BASE_H__
#define __DECODE_HEVC_PACKET_REAL_TILE_XE_M_BASE_H__

#include "decode_hevc_packet_xe_m_base.h"

namespace decode
{

class HevcDecodeRealTilePktXe_M_Base : public HevcDecodePktXe_M_Base
{
public:
    HevcDecodeRealTilePktXe_M_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
        : HevcDecodePktXe_M_Base(pipeline, task, hwInterface)
    {
    }
    virtual ~HevcDecodeRealTilePktXe_M_Base();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Destroy the media packet and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

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
    MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "HEVC_DECODE_REAL_TILE_PASS" + std::to_string(static_cast<uint32_t>(m_hevcPipeline->GetCurrentPass()));
    }

protected:
    //!
    //! \brief  Initialize the second level batch buffer which holds slice level cmds
    //! \param  [in] batchBuffer
    //!         Batch buffer for slice level commands
    //! \param  [in] batchBufBase
    //!         Point to slice level command buffer
    //! \param  [in] tileColNum
    //!         Tile column number
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS InitSliceLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase, uint32_t tileColNum);

    //!
    //! \brief  Calculate Command Buffer Size
    //! \param  [out] commandBufferSize
    //!         requested size
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateCommandBufferSize(uint32_t &commandBufferSize);

    //!
    //! \brief  Calculate Patch List Size
    //! \param  [out] requestedPatchListSize
    //!         requested size
    //! \return uint32_t
    //!         Patchlist size calculated
    //!
    virtual MOS_STATUS CalculatePatchListSize(uint32_t &requestedPatchListSize);

    //!
    //! \brief  Check if prolog required
    //!
    //! \return bool
    //!         True if prolog required
    //!
    bool IsPrologRequired();

    //!
    //! \brief  Check if last tile column
    //!
    //! \return bool
    //!         True if last tile column, else false
    //!
    bool IsLastTileCol();

    HevcDecodePicPktXe_M_Base *       m_picturePkt = nullptr;
    HevcDecodeSlcPktXe_M_Base *       m_slicePkt   = nullptr;

    std::vector<MOS_COMMAND_BUFFER> m_sliceLevelCmdBuffer;
MEDIA_CLASS_DEFINE_END(decode__HevcDecodeRealTilePktXe_M_Base)
};

}  // namespace decode
#endif
