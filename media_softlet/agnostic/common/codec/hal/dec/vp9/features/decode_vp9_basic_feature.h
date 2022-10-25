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
//! \file     decode_vp9_basic_feature.h
//! \brief    Defines the common interface for decode vp9 basic feature
//!
#ifndef __DECODE_VP9_BASIC_FEATURE_H__
#define __DECODE_VP9_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_vp9.h"
#include "codec_def_vp9_probs.h"
#include "decode_vp9_reference_frames.h"

namespace decode
{
//!
//! \struct _CODECHAL_DECODE_VP9_PROB_UPDATE
//! \brief  Define variables for VP9 decode probabilty updated
//!
typedef struct _CODECHAL_DECODE_VP9_PROB_UPDATE
{
    int32_t bSegProbCopy;      //!< seg tree and pred prob update with values from App.
    int32_t bProbSave;         //!< Save prob buffer
    int32_t bProbRestore;      //!< Restore prob buffer
    int32_t bProbReset;        //!<  1: reset; 0: not reset
    int32_t bResetFull;        //!< full reset or partial (section A) reset
    int32_t bResetKeyDefault;  //!<  reset to key or inter default
    uint8_t SegTreeProbs[7];   //!< Segment tree prob buffers
    uint8_t SegPredProbs[3];   //!< Segment predict prob buffers
} CODECHAL_DECODE_VP9_PROB_UPDATE, *PCODECHAL_DECODE_VP9_PROB_UPDATE;

class Vp9BasicFeature : public DecodeBasicFeature
{
public:
    //!
    //! \brief  Vp9BasicFeature constructor
    //!
    Vp9BasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface);

    //!
    //! \brief  Vp9BasicFeature deconstructor
    //!
    virtual ~Vp9BasicFeature();

    //!
    //! \brief  Initialize vp9 basic feature CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update vp9 decodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MOS_STATUS DetermineInternalBufferUpdate();

    MOS_STATUS InitDefaultProbBufferTable();
    
    MOS_STATUS AllocateVP9MVBuffer();

    MOS_STATUS AllocateSegmentBuffer();

    // Parameters passed from application
    uint16_t                  m_frameWidthAlignedMinBlk  = 0;        //!< Picture Width aligned to minBlock
    uint16_t                  m_frameHeightAlignedMinBlk = 0;        //!< Picture Height aligned to minBlock
    uint8_t                   m_vp9DepthIndicator        = 0;        //!< Indicate it is 8/10/12 bit VP9
    CODEC_VP9_PIC_PARAMS *    m_vp9PicParams             = nullptr;  //!< Pointer to VP9 picture parameter
    CODEC_VP9_SEGMENT_PARAMS *m_vp9SegmentParams         = nullptr;  //!< Pointer to VP9 segments parameter
    CODEC_VP9_SLICE_PARAMS *  m_vp9SliceParams           = nullptr;  //!< Pointer to Vp9 slice parameter
    Vp9ReferenceFrames        m_refFrames;                           //!< Reference frames

    uint16_t m_prevFrmWidth        = 0;      //!< Frame width of the previous frame
    uint16_t m_prevFrmHeight       = 0;      //!< Frame height of the previous frame
    uint16_t m_allocatedWidthInSb  = 0;      //!< Frame width in super block
    uint16_t m_allocatedHeightInSb = 0;      //!< Frame height in super block
    bool     m_resetSegIdBuffer    = false;  //!< if segment id buffer need to do reset
    bool     m_pendingResetPartial = false;  //!< indicating if there is pending partial reset operation on prob buffer 0.

    PMOS_BUFFER m_resVp9SegmentIdBuffer;  //!< Handle of VP9 Segment ID surface

    PMOS_BUFFER m_resVp9ProbBuffer[CODEC_VP9_NUM_CONTEXTS + 1]  = {};
    PMOS_BUFFER m_resVp9MvTemporalBuffer[CODECHAL_VP9_NUM_MV_BUFFERS] = {};  //!< Handle of VP9 MV Temporal buffer
    uint8_t     m_curMvTempBufIdx = 0;                                               //!< Current mv temporal buffer index
    uint8_t     m_colMvTempBufIdx = 0;                                               //!< Colocated mv temporal buffer index

    PMOS_RESOURCE m_presLastRefSurface   = nullptr;  //!< Pointer to last reference surface
    PMOS_RESOURCE m_presGoldenRefSurface = nullptr;  //!< Pointer to golden reference surface
    PMOS_RESOURCE m_presAltRefSurface    = nullptr;  //!< Pointer to alternate reference surface
    MOS_SURFACE   m_lastRefSurface;                  //!< MOS_SURFACE of last reference surface
    MOS_SURFACE   m_goldenRefSurface;                //!< MOS_SURFACE of golden reference surface
    MOS_SURFACE   m_altRefSurface;                   //!< MOS_SURFACE of alternate reference surface

    // Internally maintained
    uint8_t                         m_frameCtxIdx          = 0;
    bool                            m_fullProbBufferUpdate = false;  //!< indicating if prob buffer is a full buffer update
    CODECHAL_DECODE_VP9_PROB_UPDATE m_probUpdateFlags;               //!< Prob update flags

    bool    m_pendingCopySegProbs[CODEC_VP9_NUM_CONTEXTS];     //!< indicating if there is pending seg probs copy operation on each prob buffers.
    uint8_t m_segTreeProbs[7];                                 //!< saved seg tree probs for pending seg probs copy operation to use
    uint8_t m_segPredProbs[3];                                 //!< saved seg pred probs for pending seg probs copy operation to use
    bool    m_pendingResetFullTables[CODEC_VP9_NUM_CONTEXTS];  //!< indicating if there is pending full frame context table reset operation on each prob buffers except 0.
    bool    m_saveInterProbs = false;                          //!< indicating if inter probs is saved for prob buffer 0.
    uint8_t m_interProbSaved[CODECHAL_VP9_INTER_PROB_SIZE];    //!< indicating if internal prob buffer saved

    PMOS_INTERFACE m_osInterface = nullptr;
    bool           m_secureMode  = false;

    union
    {
        struct
        {
            uint8_t KeyFrame : 1;       //!< [0..1]
            uint8_t IntraOnly : 1;      //!< [0..1]
            uint8_t Display : 1;        //!< [0..1]
            uint8_t ReservedField : 5;  //!< [0]
        } fields;
        uint8_t value;
    } m_prevFrameParams;  //!< Previous frame parameters

protected:
    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;
    MOS_STATUS         SetPictureStructs();
    MOS_STATUS         SetSegmentData();

    //! \brief Hcp Interface
    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Vp9BasicFeature)
};

}  // namespace decode

#endif  // !__DECODE_VP9_BASIC_FEATURE_H__
