/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_mpeg2_basic_feature.h
//! \brief    Defines the common interface for decode mpeg2 basic feature
//!
#ifndef __DECODE_MPEG2_BASIC_FEATURE_H__
#define __DECODE_MPEG2_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_mpeg2.h"
#include "decode_mpeg2_reference_frames.h"

namespace decode {

class Mpeg2BasicFeature : public DecodeBasicFeature
{
public:
    //!
    //! \brief  Mpeg2BasicFeature constructor
    //!
    Mpeg2BasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) :
        DecodeBasicFeature(allocator, hwInterface, osInterface)
    {
        if (osInterface != nullptr)
        {
            m_osInterface = osInterface;
        }

        MOS_ZeroMemory(&m_savedMpeg2MbParam, sizeof(m_savedMpeg2MbParam));
    };

    //!
    //! \brief  Mpeg2BasicFeature deconstructor
    //!
    virtual ~Mpeg2BasicFeature();

    //!
    //! \brief  Initialize mpeg2 basic feature CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update mpeg2 decodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Re-Initialize MPEG2 incomplete frame values in the first execution call
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResetFrameValues();

    struct VldSliceRecord
    {
        uint32_t   skip;
        uint32_t   offset;
        uint32_t   length;
        uint32_t   sliceStartMbOffset;
        uint32_t   prevSliceMbEnd;
        bool       isLastSlice;
        CodecDecodeMpeg2SliceParams recordSliceParam;
    };

    struct ItMbRecord
    {
        uint16_t skippedMBs;
        uint16_t expectedMBAddr;
        CodecDecodeMpeg2MbParams recordMbParam;
    };

    // One MB size (16x16) intra MB, color = RGB[4, 4, 4]
    const uint32_t Mpeg2DummyBsBuf[12] =
    {
        0x01010000, 0x54e29752, 0x002022a5, //Dummy Slice 0, q_scale_type = 0, intra_vlc_format = 0
        0x01010000, 0x4de29752, 0x8661341a, //Dummy Slice 1, q_scale_type = 0, intra_vlc_format = 1
        0x01010000, 0x54e2a772, 0x002022a5, //Dummy Slice 2, q_scale_type = 1, intra_vlc_format = 0
        0x01010000, 0x4de2a772, 0x8661341a  //Dummy Slice 3, q_scale_type = 1, intra_vlc_format = 1
    };

    const uint32_t Mpeg2DummySliceLengths[4] = {0x8, 0x8, 0x8, 0x8};
    const uint32_t Mpeg2DummySliceOffsets[4] = {0x4, 0x10, 0x1c, 0x28};

    //!
    //! \enum     Mpeg2QmTypes
    //! \brief    MPEG2 qm types
    //!
    enum Mpeg2QmTypes
    {
        mpeg2QmIntra = 0,
        mpeg2QmNonIntra,
    };

    const uint8_t m_mpeg2QuantMatrixScan[64] =
    {
        // Inverse Zig-Zag scan pattern
        0, 1, 5, 6, 14, 15, 27, 28,
        2, 4, 7, 13, 16, 26, 29, 42,
        3, 8, 12, 17, 25, 30, 41, 43,
        9, 11, 18, 24, 31, 40, 44, 53,
        10, 19, 23, 32, 39, 45, 52, 54,
        20, 22, 33, 38, 46, 51, 55, 60,
        21, 34, 37, 47, 50, 56, 59, 61,
        35, 36, 48, 49, 57, 58, 62, 63
    };

    const uint16_t m_mpeg2DefaultIntraQuantizerMatrix[64] =
    {
        8, 16, 19, 22, 26, 27, 29, 34,
        16, 16, 22, 24, 27, 29, 34, 37,
        19, 22, 26, 27, 29, 34, 34, 38,
        22, 22, 26, 27, 29, 34, 37, 40,
        22, 26, 27, 29, 32, 35, 40, 48,
        26, 27, 29, 32, 35, 40, 48, 58,
        26, 27, 29, 34, 38, 46, 56, 69,
        27, 29, 35, 38, 46, 56, 69, 83
    };

    const uint16_t m_mpeg2DefaultNonIntraQuantizerMatrix[64] =
    {
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16
    };

    CodecDecodeMpeg2PicParams      *m_mpeg2PicParams          = nullptr;          //!< Pointer to MPEG2 picture parameter
    CodecDecodeMpeg2SliceParams    *m_mpeg2SliceParams        = nullptr;          //!< Pointer to MPEG2 slice parameter
    CodecDecodeMpeg2MbParams       *m_mpeg2MbParams           = nullptr;          //!< Pointer to MPEG2 MB parameter
    CodecMpeg2IqMatrix             *m_mpeg2IqMatrixParams     = nullptr;          //!< Pointer to MPEG2 IQ matrix parameter
    CodecMpeg2IqMatrix             *m_mpeg2IqMatrixBuffer     = nullptr;          //!< Pointer to MPEG2 IQ matrix parameter
    uint32_t                        m_numMacroblocks          = 0;                //!< Number of macro blocks

    bool                            m_deblockingEnabled       = false;            //!< Indicate Deblocking is enabled
    bool                            m_streamOutEnabled        = false;            //!< Indicates if stream out enabled
    PMOS_RESOURCE                   m_streamOutBuffer         = nullptr;          //!< Stream out buffer from HW
    PMOS_BUFFER                     m_resMpeg2DummyBistream   = nullptr;          //!< Handle of MPEG2 dummy bitstream buffer
    uint32_t                        m_copiedDataBufferSize    = 0;                //!< Size of copied bitstream buffer
    uint32_t                        m_copiedDataOffset        = 0;                //!< Offset of copied bitstream
    uint32_t                        m_nextCopiedDataOffset    = 0;                //!< Offset of next copied bitstream
    uint32_t                        m_dummySliceDataOffset    = 0;                //!< Offset of dummy slice bitstream
    bool                            m_copiedDataBufferInUse   = false;            //!< Indicate copied bistream is in use
    bool                            m_copyDummySlicePresent   = false;            //!< Indicate if dummy slice has been copied
    bool                            m_incompletePicture       = false;            //!< Indicates if current input bitstream is incomplete
    bool                            m_copiedDataNeeded        = false;            //!< Indicates if copied data is needed
    bool                            m_slicesInvalid           = false;            //!< Indicate slices are invalid
    uint32_t                        m_mpeg2ISliceConcealmentMode          = 0;    //!< Mpeg2 I slice concealment mode
    uint32_t                        m_mpeg2PbSliceConcealmentMode         = 0;    //!< Mpeg2 P/B slice concealment mode
    uint32_t                        m_mpeg2PbSlicePredBiDirMvTypeOverride = 0;    //!< Mpeg2 P/B Slice Predicted BiDir Motion Type Override
    uint32_t                        m_mpeg2PbSlicePredMvOverride          = 0;    //!< Mpeg2 P/B Slice Predicted Motion Vector Overrid
    uint16_t                        m_totalNumSlicesRecv                  = 0;    //!< Total slice number received in pervious execution call
    uint32_t                        m_totalNumMbsRecv                     = 0;    //!< Total macroblocks number received in pervious execution call
    uint16_t                        m_lastMbAddress                       = 0;    //!< Address of last macro block
    uint32_t                        m_lastSliceMbEnd                      = 0;    //!< Address of last slice MB address for current execution call.

    Mpeg2ReferenceFrames            m_refFrames;                                  //!< Reference frames
    std::vector<uint32_t>           m_refFrameIndexList;                          //!< Reference frame index list
    std::vector<VldSliceRecord>     m_sliceRecord;                                //!< Record VLD mode slice info
    std::vector<ItMbRecord>         m_mbRecord;                                   //!< Record IT mode macroblock info
    uint8_t                         m_fwdRefIdx          = 0;
    uint8_t                         m_bwdRefIdx          = 0;
    static const uint32_t           m_mpeg2NumCopiedBufs = 3;

    BufferArray                    *m_copiedDataBufArray = nullptr;               //!< Handles of copied bitstream buffer array
    PMOS_BUFFER                     m_copiedDataBuf      = nullptr;
    CodecDecodeMpeg2MbParams        m_savedMpeg2MbParam;                          //!< save last MB parameters to reconstruct MPEG2 IT Object Command for Skipped MBs.

protected:
    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override
    { 
        return MOS_STATUS_SUCCESS;
    };

    MOS_STATUS SetPictureStructs();
    MOS_STATUS SetSliceStructs();
    MOS_STATUS SetMbStructs();
    MOS_STATUS CheckCompletePicture(uint16_t startSliceIdx);
    bool DetectSliceError(CodecDecodeMpeg2SliceParams &slc, uint32_t slcIdx, bool firstValidSlice);

    PMOS_INTERFACE        m_osInterface  = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Mpeg2BasicFeature)
};

}//decode

#endif // !__DECODE_MPEG2_BASIC_FEATURE_H__
