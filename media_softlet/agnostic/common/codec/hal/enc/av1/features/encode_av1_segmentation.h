/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_av1_segmentation.h
//! \brief    Defines the common interface for encode av1 segmentation feature
//!
#ifndef __ENCODE_AV1_SEGMENTATION_H__
#define __ENCODE_AV1_SEGMENTATION_H__

#include "encode_basic_feature.h"
#include "encode_av1_reference_frames.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"

namespace encode
{

struct VdencStreamInState;
class Av1StreamIn;

class Av1Segmentation : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    Av1Segmentation(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        void *constSettings);

    virtual ~Av1Segmentation();

    //!
    //! \brief  Update segment related parameter
    //! \param  [in] params
    //!         Pointer to encode parameter
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_SEGMENT_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    bool HasZeroSegmentQIndex() const { return m_hasZeroSegmentQIndex; }

protected:
    //!
    //! \brief  Set segment id parameter
    //! \param  [in] segmentParams
    //!         Pointer to CodecAv1SegmentsParams
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSegmentIdParams(
        const PCODEC_AV1_ENCODE_PICTURE_PARAMS  ddiPicParams,
        const CODEC_Intel_Seg_AV1              *ddiSegParams);

    MOS_STATUS CheckQPAndLossless();

    //!
    //! \brief  Set up segment id stream in buffer
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupSegmentationMap();

    //!
    //! \brief  Check segmentation map
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckSegmentationMap() const;

    //!
    //! \brief  Fill segmentation map into provided stream in buffer
    //! \param  [in] streamInData
    //!         pointer to stream in buffer locked address
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FillSegmentationMap(VdencStreamInState* streamInData) const;

    MOS_STATUS AllocateSegmentationMapBuffer(uint8_t segmentBufid);

    CodecAv1SegmentsParams m_segmentParams = {};             //!< Segment Params
    uint8_t                m_segmentNum = 0;                 //!< Segment number

    Av1BasicFeature *m_basicFeature = nullptr;               //!< AV1 paramter
    EncodeAllocator *m_allocator = nullptr;                  //!< AV1 allocator
    uint8_t         *m_pSegmentMap = nullptr;                //!< segmentation map from APP
    bool             m_segmentMapProvided = false;           //!< Flag to indicate APP's segmentation map provided or not
    uint32_t         m_segmentMapDataSize = 0;               //!< segmentation map size from APP

    uint32_t                 m_segmentMapBlockSize = 0;      //!< segment map block size
    uint8_t                  m_targetUsage = 0;              //!< Target Usage
    static constexpr uint8_t m_imgStateImePredictors = 8;    //!< Number of predictors for IME

    Av1StreamIn* m_streamIn = nullptr;                       //!< The instance of stream in utility
    bool m_hasZeroSegmentQIndex = false;                     //!< Indicates if any of segments has zero qIndex
    
    int8_t        m_segmenBufferinUse[av1TotalRefsPerFrame]  = {};          //!< Indicates the num of m_segmentMapBuffer uesed for DPB
    int8_t        m_ucScalingIdtoSegID[CODEC_NUM_TRACKED_BUFFERS] = {};         //!< Map the ucscaling of DPB to segmentID of segmentMapBuffer,Array length follows ucscaling range
    PMOS_RESOURCE m_segmentMapBuffer[av1TotalRefsPerFrame]   = {nullptr};   //!< Save the segmentMap of DPB

MEDIA_CLASS_DEFINE_END(encode__Av1Segmentation)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_SEGMENTATION_H__
