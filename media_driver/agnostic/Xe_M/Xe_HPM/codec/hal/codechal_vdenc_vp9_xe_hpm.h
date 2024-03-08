/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/


//!
//! \file     codechal_vdenc_vp9_xe_hpm.h
//! \brief    This file defines the base C++ class/interface for Xe_XPM VP9 VDENC
//!           encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_VP9_XE_XPM_H__
#define __CODECHAL_VDENC_VP9_XE_XPM_H__

#include "codechal_vdenc_vp9_g12.h"

#define MAX_PAK_NUM 8

class CodechalVdencVp9StateXe_Xpm : public CodechalVdencVp9StateG12
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalVdencVp9StateXe_Xpm(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencVp9StateG12(hwInterface, debugInterface, standardInfo){};

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencVp9StateXe_Xpm();

    virtual MOS_STATUS SetTileCommands(
        PMOS_COMMAND_BUFFER cmdBuffer) override; // override this function to add new cmd

    MOS_STATUS SetSequenceStructs() override;
    MOS_STATUS HuCBrcInitReset() override;

    //!
    //! \struct HucPakStitchDmemEncXehp
    //! \brief  The struct of Huc Com Dmem
    //!
    struct HucPakIntDmemXehp
    {
        uint32_t tileSizeRecordOffset[MAX_PAK_NUM + 1];    // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
        uint32_t vdencStatOffset[MAX_PAK_NUM + 1];       // needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t hevcPakStatOffset[MAX_PAK_NUM + 1];     // needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t hevcStreamoutOffset[MAX_PAK_NUM + 1];   // needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t vp9PakStatOffset[MAX_PAK_NUM + 1];      // needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t vp9CounterBufferOffset[MAX_PAK_NUM + 1];  // needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
        uint32_t lastTileBSStartInBytes;     // last tile in bitstream for region 4 and region 5
        uint32_t SliceHeaderSizeinBits;      // needed for HEVC dual pipe BRC
        uint16_t totalSizeInCommandBuffer;   // Total size in bytes of valid data in the command buffer
        uint16_t offsetInCommandBuffer;      // Byte  offset of the to-be-updated Length (uint32_t ) in the command buffer, 0xffff means unavailable
        uint16_t picWidthInPixel;            // Picture width in pixel
        uint16_t picHeightInPixel;           // Picture hieght in pixel
        uint16_t totalNumberOfPaks;          // [2..4] for Gen11
        uint16_t numSlices[MAX_PAK_NUM];  // this is number of slices in each PAK
        uint16_t numTilesPerPipe[MAX_PAK_NUM];  // this is number of tiles from each PAK
        uint16_t picStateStartInBytes;       // offset for  region 7 and region 8
        uint8_t  codec;                      // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
        uint8_t  maxPass;                    // Max number of BRC pass >=1
        uint8_t  currentPass;                // Current BRC pass [1..MAXPass]
        uint8_t  minCUSize;                  // Minimum CU size (3: 8x8, 4:16x16), HEVC only.
        uint8_t  cabacZeroWordFlag;          // Cabac zero flag, HEVC only
        uint8_t  bitdepthLuma;               // luma bitdepth, HEVC only
        uint8_t  bitdepthChroma;             // chroma bitdepth, HEVC only
        uint8_t  chromaFormatIdc;            // chroma format idc, HEVC only
        uint8_t  currFrameBRClevel;          // Hevc dual pipe only
        uint8_t  brcUnderFlowEnable;         // Hevc dual pipe only
        uint8_t  StitchEnable;               // enable stitch cmd for Hevc dual pipe
        uint8_t  reserved1;
        uint16_t StitchCommandOffset;  // offset in region 10 which is the second level batch buffer
        uint16_t reserved2;
        uint32_t BBEndforStitch;
        uint8_t  RSVD[32];
    };

    static constexpr uint32_t m_pakIntDmemOffsetsSize = 216;//First six offsets are set to 0xFF as unavailable. See details in HucPakIntDmemXehp struct. 

    MOS_STATUS SetDmemHuCPakInt() override;
    MOS_STATUS HuCVp9PakInt(
        PMOS_COMMAND_BUFFER cmdBuffer) override;
    MOS_STATUS AllocateResources() override;

    MOS_STATUS SetupSegmentationStreamIn() override;
    MOS_STATUS SetCurbeMe(
        MeCurbeParams *params) override;

    /*#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS PopulateDdiParam(
        PCODEC_VP9_ENCODE_SEQUENCE_PARAMS vp9SeqParams,
        _CODEC_VP9_ENCODE_PIC_PARAMS  vp9PicParams) override;
#endif*/

    MOS_STATUS Initialize(CodechalSetting * settings) override;

    MOS_STATUS ConstructPicStateBatchBuf(
        PMOS_RESOURCE picStateBuffer) override;

    MOS_STATUS CheckHucLoadStatus();
    MOS_STATUS PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer);

    //Resources
    MOS_RESOURCE      m_hucAuthBuf                                      = {};  //!< Huc authentication buffer
    MHW_BATCH_BUFFER  m_2ndLevelBB[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};  //!< 2nd level batch buffer
    PMHW_BATCH_BUFFER m_batchBuf = nullptr;
};

#endif  // __CODECHAL_VDENC_VP9_XE_XPM_H__
