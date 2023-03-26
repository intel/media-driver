/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_decode_mpeg2.h
//! \brief    Defines the decode interface extension for MPEG2.
//! \details  Defines all types, macros, and functions required by CodecHal for MPEG2 decoding. Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_MPEG2_H__
#define __CODECHAL_DECODER_MPEG2_H__

#include "codechal_decoder.h"

//!
//! \def CODECHAL_DECODE_MPEG2_MAXIMUM_BATCH_BUFFERS
//! maximum number of batch buffer
//!
#define CODECHAL_DECODE_MPEG2_MAXIMUM_BATCH_BUFFERS     120

//!
//! \def CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP
//! number of batch buffers per group
//!
#define CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP   3

//!
//! \def CODECHAL_DECODE_MPEG2_COPIED_SURFACES
//! number of surfaces to save the bitstream in multiple execution call case
//!
#define CODECHAL_DECODE_MPEG2_COPIED_SURFACES           3

//!
//! \def CODECHAL_DECODE_MPEG2_BYTES_PER_MB
//! bitstream size per macroblock
//!
#define CODECHAL_DECODE_MPEG2_BYTES_PER_MB              512

//!
//! \def CODECHAL_DECODE_MPEG2_MB_MOTION_FORWARD
//! definition for forward motion of macroblock
//!
#define CODECHAL_DECODE_MPEG2_MB_MOTION_FORWARD         2   //!< Bit 1

//!
//! \def CODECHAL_DECODE_MPEG2_MB_MOTION_BACKWARD
//! definition for backward motion of macroblock
//!
#define CODECHAL_DECODE_MPEG2_MB_MOTION_BACKWARD        4   //!< Bit 2

//!
//! \enum CODECHAL_MPEG2_IMT_TYPE
//! \brief Mpeg2 image type
//!
typedef enum _CODECHAL_MPEG2_IMT_TYPE
{
    CODECHAL_MPEG2_IMT_NONE = 0,         //!< triple GFXBlocks
    CODECHAL_MPEG2_IMT_FRAME_FRAME,      //!< triple
    CODECHAL_MPEG2_IMT_FIELD_FIELD,      //!< triple
    CODECHAL_MPEG2_IMT_FIELD_DUAL_PRIME, //!< triple
    CODECHAL_MPEG2_IMT_FRAME_FIELD,      //!< hex
    CODECHAL_MPEG2_IMT_FRAME_DUAL_PRIME, //!< hex
    CODECHAL_MPEG2_IMT_16X8              //!< hex
} CODECHAL_MPEG2_IMT_TYPE;

typedef class CodechalDecodeMpeg2 *PCODECHAL_DECODE_MPEG2_STATE;

//!
//! \class CodechalDecodeMpeg2
//! \brief This class defines the member fields, functions etc used by MPEG2 decoder.
//!
class CodechalDecodeMpeg2 : public CodechalDecode
{
public:

    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeMpeg2(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeMpeg2(const CodechalDecodeMpeg2&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeMpeg2& operator=(const CodechalDecodeMpeg2&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeMpeg2 ();

    //!
    //! \brief    Allocate and initialize MPEG2 decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        CodechalSetting *          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for MPEG2 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates () override;

    //!
    //! \brief    MPEG2 decoder state level function
    //! \details  State level function for MPEG2 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel () override;

    //!
    //! \brief    MPEG2 decoder primitive level function
    //! \details  Primitive level function for GEN specific MPEG2 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel () override;

    MOS_STATUS  InitMmcState() override;

    //!
    //! \brief    Allocate resources
    //! \details  Allocate resources in MPEG2 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateResources();

    //!
    //! \brief    MPEG2 decoder slice level function
    //! \details  Primitive level function in VLD mode for GEN specific MPEG2 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  SliceLevel();

    //!
    //! \brief    MPEG2 decoder macro block level function
    //! \details  Primitive level function in IT mode for GEN specific MPEG2 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MacroblockLevel();

    //!
    //! \brief    Detect slice error
    //! \details  Detect slice error in MPEG2 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    bool                DetectSliceError(
        uint16_t                        slcNum,
        uint32_t                        prevSliceMbEnd,
        bool                            firstValidSlice);

    //!
    //! \brief    Insert dummy slices
    //! \details  Insert dummy slices in MPEG2 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          InsertDummySlices(
        PMHW_BATCH_BUFFER               batchBuffer,
        uint16_t                        startMB,
        uint16_t                        endMB);

    //!
    //! \brief    Pack motion vectors
    //! \details  Pack motion vectors in MPEG2 decode driver
    //! \return   None
    //!
    void                PackMotionVectors(
        CODEC_PICTURE_FLAG              pic_flag,
        PMHW_VDBOX_MPEG2_MB_STATE       mpeg2MbState);

    //!
    //! \brief    Insert skipped macro blocks
    //! \details  Insert skipped macro blocks in MPEG2 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          InsertSkippedMacroblocks(
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_VDBOX_MPEG2_MB_STATE       params,
        uint16_t                        nextMBStart,
        uint16_t                        skippedMBs);

    //!
    //! \brief    Initialize MPEG2 incomplete frame values
    //! \details  Initialize MPEG2 incomplete frame values in MPEG2 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeBeginFrame();

    //!
    //! \brief    Copy bitstream to local buffer
    //! \details  Copy bitstream to local buffer in MPEG2 decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          CopyDataSurface(
        uint32_t                        dataSize,
        MOS_RESOURCE                    sourceSurface,
        PMOS_RESOURCE                   copiedSurface,
        uint32_t                        *currOffset);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        CodecDecodeMpeg2PicParams *picParams);

    MOS_STATUS DumpSliceParams(
        CodecDecodeMpeg2SliceParams *sliceParams,
        uint32_t                     numSlices);

    MOS_STATUS DumpIQParams(
        CodecMpeg2IqMatrix *matrixData);

    MOS_STATUS DumpMbParams(
        CodecDecodeMpeg2MbParams *mbParams);
#endif
    // Parameters passed by application
    uint16_t                     m_picWidthInMb   = 0;                               //!< Picture Width in MB width count
    uint16_t                     m_picHeightInMb  = 0;                               //!< Picture Height in MB height count
    uint32_t                     m_dataSize       = 0;                               //!< Size of bitstream
    uint32_t                     m_dataOffset     = 0;                               //!< Offset of bitstream
    uint32_t                     m_numSlices      = 0;                               //!< Number of slices
    uint32_t                     m_numMacroblocks = 0;                               //!< Number of macro blocks
    CodecDecodeMpeg2PicParams *  m_picParams      = nullptr;                         //!< Pointer to MPEG2 picture parameter
    CodecDecodeMpeg2SliceParams *m_sliceParams    = nullptr;                         //!< Pointer to MPEG2 slice parameter
    CodecDecodeMpeg2MbParams *   m_mbParams       = nullptr;                         //!< Pointer to MPEG2 macro block parameter
    CodecMpeg2IqMatrix *         m_iqMatrixBuffer = nullptr;                         //!< Pointer to MPEG2 IQ matrix parameter
    MOS_SURFACE                  m_destSurface;                                      //!< Handle of render surface
    PMOS_RESOURCE                m_presReferences[CODEC_MAX_NUM_REF_FRAME_NON_AVC];  //!< Pointer to Handle of Reference Frames
    MOS_RESOURCE                 m_resDataBuffer;                                    //!< Handle of residual difference surface
    bool                         m_deblockingEnabled = false;                        //!< Indicate Deblocking is enabled

    MOS_RESOURCE               m_resMfdDeblockingFilterRowStoreScratchBuffer;                          //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    MOS_RESOURCE               m_resBsdMpcRowStoreScratchBuffer;                                       //!< Handle of MPR Row Store Scratch data surface
    PCODECHAL_VLD_SLICE_RECORD m_vldSliceRecord = nullptr;                                             //!< Pointer to internal record of slices
    PCODEC_REF_LIST            m_mpeg2RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2];                //!< Pointer to MPEG2 Ref List
    MHW_BATCH_BUFFER           m_mediaObjectBatchBuffer[CODECHAL_DECODE_MPEG2_MAXIMUM_BATCH_BUFFERS];  //!< Handles of second level batch buffer
    uint16_t                   m_bbInUse         = 0;                                                  //!< Current index of second level batch buffer in the allocated array
    uint16_t                   m_bbAllocated     = 0;                                                  //!< Total number of second level batch buffers allocated
    uint16_t                   m_bbInUsePerFrame = 0;                                                  //!< Current index of second level batch buffers used for a frame

    // MPEG2 WAs
    bool         m_slicesInvalid = false;                                       //!< Indicate slices are invalid
    MOS_RESOURCE m_resMpeg2DummyBistream;                                       //!< Handle of MPEG2 dummy bitstream buffer
    MOS_RESOURCE m_resCopiedDataBuffer[CODECHAL_DECODE_MPEG2_COPIED_SURFACES];  //!< Handles of copied bitstream buffer
    uint32_t     m_copiedDataBufferSize                = 0;                     //!< Size of copied bitstream buffer
    uint32_t     m_currCopiedData                      = 0;                     //!< Index of current copied buffer
    bool         m_copiedDataBufferInUse               = false;                 //!< Indicate copied bistream is inuse
    bool         m_dummySliceDataPresent               = false;                 //!< Indicate dummy slice is present
    uint32_t     m_copiedDataOffset                    = 0;                     //!< Offset of copied bitstream
    uint32_t     m_nextCopiedDataOffset                = 0;                     //!< Offset of next copied bitstream
    uint32_t     m_dummySliceDataOffset                = 0;                     //!< Offset of dummy slice bitstream
    uint16_t     m_lastMbAddress                       = 0;                     //!< Address of last macro block
    uint32_t     m_mpeg2ISliceConcealmentMode          = 0;                     //!< Mpeg2 I slice concealment mode
    uint32_t     m_mpeg2PbSliceConcealmentMode         = 0;                     //!< Mpeg2 P/B slice concealment mode
    uint32_t     m_mpeg2PbSlicePredBiDirMvTypeOverride = 0;                     //!< Mpeg2 P/B Slice Predicted BiDir Motion Type Override
    uint32_t     m_mpeg2PbSlicePredMvOverride          = 0;                     //!< Mpeg2 P/B Slice Predicted Motion Vector Override

    MOS_RESOURCE m_resSyncObjectWaContextInUse;     //!< signals on the video WA context
    MOS_RESOURCE m_resSyncObjectVideoContextInUse;  //!< signals on the video context

    CodecDecodeMpeg2MbParams m_savedMpeg2MbParam;  //!< save last MB parameters to be able to reconstruct MPEG2 IT Object Command for Skipped MBs.
};

#endif  // __CODECHAL_DECODER_MPEG2_H__
