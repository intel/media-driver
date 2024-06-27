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
//! \file     encode_av1_brc_init_packet.h
//! \brief    Defines the implementation of av1 brc init packet
//!

#ifndef __CODECHAL_AV1_BRC_INIT_PACKET_H__
#define __CODECHAL_AV1_BRC_INIT_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "encode_utils.h"
#include "encode_av1_vdenc_pipeline.h"
#include "encode_av1_basic_feature.h"

namespace encode
{
    struct VdencAv1HucBrcInitDmem
    {
        uint32_t     BRCFunc;                    // 0: Init; 2: Reset

        uint32_t     INIT_ProfileLevelMaxFrame;  // Limit on maximum frame nuamber based on selected profile and level, could be user defined 
        uint32_t     INIT_InitBufFullness;       // Initial buffer fullness
        uint32_t     INIT_BufSize;               // Buffer Size
        uint32_t     INIT_TargetBitrate;         // Average (target) bit rate
        uint32_t     INIT_MaxRate;               // Maximum bit rate
        uint32_t     INIT_MinRate;               // Minimum bit rate
        uint32_t     INIT_FrameRateM;            // FrameRateM
        uint32_t     INIT_FrameRateD;            // :ud, FrameRateD
        uint32_t     RSVD32[8];                  // mbz

        uint16_t     INIT_BRCFlag;               // BRC flag 0: ACQP, 0x10: CBR, 0x20: VBR, 0x40: AVBR, 0x80: CQL
        uint16_t     Reserved1;                  // mbz
        uint16_t     INIT_GopP;                  // number of P frames in the GOP
        uint16_t     INIT_GopB;                  // number of B frames in the GOP
        uint16_t     INIT_FrameWidth;            // frame width
        uint16_t     INIT_FrameHeight;           // frame height
        uint16_t     INIT_MinQP;                 // minimum QP
        uint16_t     INIT_MaxQP;                 // maximum QP
        uint16_t     INIT_LevelQP;               // level QP
        uint16_t     INIT_GoldenFrameInterval;   // golden frame interval
        uint16_t     INIT_EnableScaling;         // enable resolution scaling
        uint16_t     INIT_OvershootCBR_pct;      // default: 115, CBR overshoot percentage
        uint16_t     INIT_GopB1;                  // number of B1 frames in the GOP
        uint16_t     INIT_GopB2;                  // number of B2 frames in the GOP
        uint16_t     INIT_GopB3;                  // number of B3 frames in the GOP
        uint16_t     RSVD16[7];                   // mbz

        int8_t       INIT_InstRateThreshP0[4];   // instant rate threshold for P frame, 4 elements
        int8_t       Reserved2[4];               // mbz
        int8_t       INIT_InstRateThreshI0[4];   // instant rate threshold 0 for I frame, 4 elements
        int8_t       INIT_DevThreshPB0[8];       // deviation threshold for P and B frame, 8 elements
        int8_t       INIT_DevThreshVBR0[8];      // deviation threshold for VBR control, 8 elements
        int8_t       INIT_DevThreshI0[8];        // deviation threshold for I frame, 8 elements
        uint8_t      INIT_InitQPP;
        uint8_t      INIT_InitQPI;
        uint8_t      INIT_SegMapGenerating;      // Cmodel sets the flag only in BRC Init() as of now
        uint8_t      INIT_Total_Level;
        uint8_t      INIT_MaxLevel_Ratio[16];    // level ratio
        uint8_t      INIT_SLIDINGWINDOW_ENABLE;
        uint8_t      INIT_SLIDINGWINDOW_SIZE;
        uint8_t      RSVD8[22];                  // mbz
    };

    class Av1BrcInitPkt : public EncodeHucPkt
    {
    public:
        Av1BrcInitPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
            EncodeHucPkt(pipeline, task, hwInterface)
        {
        }

        virtual ~Av1BrcInitPkt() {}

        virtual MOS_STATUS Init() override;

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
            return "BRCINIT";
        }

    protected:
        virtual MOS_STATUS AllocateResources() override;

        MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpInput() override;
#endif
        static constexpr uint32_t m_vdboxHucAv1BrcInitKernelDescriptor = 18;//!< Huc Av1 Brc init kernel descriptor

        uint32_t m_vdencBrcInitDmemBufferSize = sizeof(VdencAv1HucBrcInitDmem);
        MOS_RESOURCE m_vdencBrcInitDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {}; //!< VDEnc BrcInit DMEM buffer

        Av1BasicFeature* m_basicFeature = nullptr;  //!< Av1 Basic Feature used in each frame

    MEDIA_CLASS_DEFINE_END(encode__Av1BrcInitPkt)
    };

}  // namespace encode
#endif
