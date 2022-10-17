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
//! \file     encode_back_annotation_packet.h
//! \brief    Defines the implementation of AV1 back annotation packet
//!

#ifndef __CODECHAL_BACK_ANNOTATION_PACKET_H__
#define __CODECHAL_BACK_ANNOTATION_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "encode_utils.h"
#include "encode_av1_basic_feature.h"

namespace encode
{
    enum AnnotationTypes
    {
        frame_header_obu = 0,
        TileGroupOBU = 1
    };

    // // 64-byte alignment
    struct VdencAv1HucBackAnnotationDmem
    {
        uint32_t     firstTileGroupByteOffset;  // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
        uint32_t     reserved_32[31];           // reserved mbz
        uint16_t     reserved_16[64];          // reserved mbz

        uint8_t      tileGroupNumber;
        uint8_t      backAnnotationType;       // 0: frame_header_obu, 1:TileGroupOBU
        uint8_t      reserved_8[62];           // reserved mbz
    };

    struct VdencAv1HucCtrlBigData
    {
        uint32_t      OBUSizeByteOffset[ENCODE_VDENC_AV1_MAX_TILE_GROUP_NUM];        // the obu_size location
        uint8_t      tileNumberPerGroup[ENCODE_VDENC_AV1_MAX_TILE_GROUP_NUM];
    };

    class Av1BackAnnotationPkt : public EncodeHucPkt
    {
    public:
        Av1BackAnnotationPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
            EncodeHucPkt(pipeline, task, hwInterface)
        {
        }

        virtual ~Av1BackAnnotationPkt() {}

        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \param  [in] params
        //!         Pointer to the input parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare() override;

        MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

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
        virtual MOS_STATUS CalculateCommandSize(
            uint32_t &commandBufferSize,
            uint32_t &requestedPatchListSize) override;

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "BACK_ANNOTATION";
        }

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

    protected:
        virtual MOS_STATUS AllocateResources() override;

        virtual MOS_STATUS SetDmemBuffer();

        virtual MOS_STATUS SetHucCtrlBuffer();

#if USE_CODECHAL_DEBUG_TOOL
        virtual MOS_STATUS DumpBackAnnotation();
        virtual MOS_STATUS DumpOutput() override;
#endif
        static constexpr uint32_t               m_vdboxHucBackAnnonationKernelDescriptor = 17;   //!< Huc AV1 back annotation kernel descriptor
        static constexpr uint32_t               m_numBytesOfOBUSize = 4;   //!< byte number for OBU size

        uint32_t m_vdencbackAnnotationDmemBufferSize = sizeof(VdencAv1HucBackAnnotationDmem);
        PMOS_RESOURCE m_vdencBackAnnotationDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {}; //!< Huc AV1 back annotation DMEM buffer

        uint32_t m_vdencAv1HucCtrlBufferSize = sizeof(VdencAv1HucCtrlBigData);
        PMOS_RESOURCE m_vdencAv1HucCtrlBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {}; //!< Huc AV1 back annotation DMEM buffer

        Av1BasicFeature    *m_basicFeature = nullptr;  //!< AV1 Basic Feature used in each frame

    MEDIA_CLASS_DEFINE_END(encode__Av1BackAnnotationPkt)
    };

}  // namespace encode
#endif
