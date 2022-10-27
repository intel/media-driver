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
//! \file     encode_vp9_pak_integrate_packet_xe_lpm_plus.h
//! \brief    Defines the implementation of vp9 pak integrate packet
//!

#ifndef __CODECHAL_VP9_PAK_INTEGRATE_PACKET_XE_LPM_PLUS_H__
#define __CODECHAL_VP9_PAK_INTEGRATE_PACKET_XE_LPM_PLUS_H__

#include "encode_vp9_pak_integrate_packet.h"

namespace encode
{
#define MAX_PAK_NUM 8

//!
//! \struct HucPakIntDmemXe_Lpm_Plus
//! \brief  The struct of Huc Com Dmem
//!
struct HucPakIntDmemXe_Lpm_Plus
{
    uint32_t tileSizeRecordOffset[MAX_PAK_NUM + 1];    // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
    uint32_t vdencStatOffset[MAX_PAK_NUM + 1];         // Needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t hevcPakStatOffset[MAX_PAK_NUM + 1];       // Needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t hevcStreamoutOffset[MAX_PAK_NUM + 1];     // Needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t vp9PakStatOffset[MAX_PAK_NUM + 1];        // Needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t vp9CounterBufferOffset[MAX_PAK_NUM + 1];  // Needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t lastTileBSStartInBytes;                   // Last tile in bitstream for region 4 and region 5
    uint32_t SliceHeaderSizeinBits;                    // Needed for HEVC dual pipe BRC
    uint16_t totalSizeInCommandBuffer;                 // Total size in bytes of valid data in the command buffer
    uint16_t offsetInCommandBuffer;                    // Byte  offset of the to-be-updated Length (uint32_t ) in the command buffer, 0xffff means unavailable
    uint16_t picWidthInPixel;                          // Picture width in pixel
    uint16_t picHeightInPixel;                         // Picture hieght in pixel
    uint16_t totalNumberOfPaks;                        // [2..4] for Gen11
    uint16_t numSlices[MAX_PAK_NUM];                   // This is number of slices in each PAK
    uint16_t numTilesPerPipe[MAX_PAK_NUM];             // This is number of tiles from each PAK
    uint16_t picStateStartInBytes;                     // Offset for  region 7 and region 8
    uint8_t  codec;                                    // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
    uint8_t  maxPass;                                  // Max number of BRC pass >=1
    uint8_t  currentPass;                              // Current BRC pass [1..MAXPass]
    uint8_t  minCUSize;                                // Minimum CU size (3: 8x8, 4:16x16), HEVC only.
    uint8_t  cabacZeroWordFlag;                        // Cabac zero flag, HEVC only
    uint8_t  bitdepthLuma;                             // Luma bitdepth, HEVC only
    uint8_t  bitdepthChroma;                           // Chroma bitdepth, HEVC only
    uint8_t  chromaFormatIdc;                          // Chroma format idc, HEVC only
    uint8_t  currFrameBRClevel;                        // Hevc dual pipe only
    uint8_t  brcUnderFlowEnable;                       // Hevc dual pipe only
    uint8_t  StitchEnable;                             // Enable stitch cmd for Hevc dual pipe
    uint8_t  reserved1;
    uint16_t StitchCommandOffset;                      // Offset in region 10 which is the second level batch buffer
    uint16_t reserved2;
    uint32_t BBEndforStitch;
    uint8_t  RSVD[32];
};

class Vp9PakIntegratePktXe_Lpm_Plus : public Vp9PakIntegratePkt
{
public:
    Vp9PakIntegratePktXe_Lpm_Plus(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : Vp9PakIntegratePkt(pipeline, task, hwInterface)
    {
        m_hucPakIntDmemBufferSize = sizeof(HucPakIntDmemXe_Lpm_Plus);
    }

    virtual ~Vp9PakIntegratePktXe_Lpm_Plus() {}

    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);

protected:

    static constexpr uint32_t m_pakIntDmemOffsetsSize = 216;  // First six offsets are set to 0xFF as unavailable. See details in HucPakIntDmemXehp struct.

    //!
    //! \brief  Set huc dmem buffer
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemBuffer() const override;

MEDIA_CLASS_DEFINE_END(encode__Vp9PakIntegratePktXe_Lpm_Plus)
};
}  // namespace encode

#endif