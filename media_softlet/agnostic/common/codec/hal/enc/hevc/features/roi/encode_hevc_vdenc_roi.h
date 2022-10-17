/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_roi.h
//! \brief    Defines of the ROI feature of HEVC VDENC
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_H__
#define __CODECHAL_HEVC_VDENC_ROI_H__

#include "media_feature.h"
#include "encode_hevc_vdenc_roi_overlap.h"
#include "encode_hevc_vdenc_roi_strategy.h"
#include "encode_hevc_brc.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_huc_itf.h"

namespace encode
{

//!
//! \class    HevcVdencRoi
//!
//! \brief    HevcVdencRoi is unified interface of ROI and Dirty ROI.
//!
//! \detail   This class is Facade for Native ROI, ForceQP ROI, HuC based
//!           ForceQP ROI and Dirty ROI. At the same time, Navtive ROI,
//!           ForceQP ROI and Huc based ForceQP, we can only use one of them in
//!           one frame, which we can treat them as traditional ROI, different
//!           from dirty ROI. Dirty ROI can exist with them at the same time.
//!           All of them as a strategy inherit from RoiStrategy, which are
//!           responsable for dealing with the ROI related operations.
//!

class HevcVdencRoi : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::huc::Itf::ParSetting
{
public:
    HevcVdencRoi(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings);

    virtual ~HevcVdencRoi() {}

    //!
    //! \brief  Init encode parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    //!
    //! \brief    Set VDENC_PIPE_BUF_ADDR parameters
    //!
    //! \param    [in, out] PipeBufAddrParams
    //!           Pipe buf addr parameters
    //!
    //! \return   void
    //!
    MOS_STATUS SetVdencPipeBufAddrParams(
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

    //!
    //! \brief    Setup HuC BRC init/reset parameters
    //!
    //! \param    [in,out] hucVdencBrcInitDmem
    //!           pointer of PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemHuCBrcInitReset(
        VdencHevcHucBrcInitDmem *hucVdencBrcInitDmem);

    //!
    //! \brief    Sort and set distinct delta QPs
    //!
    //! \param    [in] numRoi
    //!           Number of ROI
    //! \param    [in] roiRegions
    //!           ROI regions
    //! \param    [in] numDistinctDeltaQp
    //!           number of distinct delta QPs
    //! \param    [in] roiDistinctDeltaQp
    //!           data of distinct delta QPs
    //! \return   bool
    //!           true if native ROI, otherwise false
    //!
    bool ProcessRoiDeltaQp(
        uint8_t    numRoi,
        CODEC_ROI  *roiRegions,
        uint8_t    numDistinctDeltaQp,
        int8_t     *roiDistinctDeltaQp);

    bool IsArbRoi() const { return m_isArbRoi; }

protected:
    using SeqParams = CODEC_HEVC_ENCODE_SEQUENCE_PARAMS;
    using PicParams = CODEC_HEVC_ENCODE_PICTURE_PARAMS;
    using SlcParams = CODEC_HEVC_ENCODE_SLICE_PARAMS;

    //!
    //! \brief    Setup ROI
    //!
    //! \param    [in] hevcSeqParams
    //!           pointer of sequence parameters
    //! \param    [in] hevcPicParams
    //!           pointer of picture parameters
    //! \param    [in] hevcSlcParams
    //!           pointer of slice parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteRoi(
        SeqParams *hevcSeqParams,
        PicParams *hevcPicParams,
        SlcParams *hevcSlcParams);

    //!
    //! \brief    Setup Force Delta QP ROI
    //!
    //! \param    [in] hevcSeqParams
    //!           pointer of sequence parameters
    //! \param    [in] hevcPicParams
    //!           pointer of picture parameters
    //! \param    [in] hevcSlcParams
    //!           pointer of slice parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteRoiExt(
        SeqParams *hevcSeqParams,
        PicParams *hevcPicParams,
        SlcParams *hevcSlcParams);

    //!
    //! \brief    Setup Dirty ROI
    //!
    //! \param    [in] hevcSeqParams
    //!           pointer of sequence parameters
    //! \param    [in] hevcPicParams
    //!           pointer of picture parameters
    //! \param    [in] hevcSlcParams
    //!           pointer of slice parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteDirtyRoi(
        SeqParams *hevcSeqParams,
        PicParams *hevcPicParams,
        SlcParams *hevcSlcParams);



    //!
    //! \brief    Write the Streamin data according to the overlap settings.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WriteStreaminData();

    //!
    //! \brief    Get the LCU number
    //! \return   uint32_t
    //!           LCU number
    //!
    uint32_t GetLCUNumber()
    {
        uint32_t streamInWidth  =
            (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32);
        uint32_t streamInHeight =
            (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) + 8;

        return (streamInWidth * streamInHeight);
    }

    //!
    //! \brief    ClearStreaminBuffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ClearStreaminBuffer(uint32_t lucNumber);

    //!
    //! \brief    Get strategy for setting command parameters
    //!
    //! \detail   When set command parameters, sometimes we need to know ROI or
    //!           dirty ROI to do this. Here, if ROI enabled, we always use ROI
    //!           to set these parameters, otherwise, we use dirty ROI.
    //!
    //! \return   RoiStrategy *
    //!           ponter of RoiStrategy
    //!
    RoiStrategy *GetStrategyForParamsSetting() const
    {
        return (m_roiEnabled ?
            m_strategyFactory.GetRoi() :
            m_strategyFactory.GetDirtyRoi());
    }

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    static constexpr uint8_t m_maxNumRoi       = 16;  //!< VDEnc maximum number of ROI supported
    static constexpr uint8_t m_maxNumNativeRoi = 3;   //!< Number of native ROI supported by VDEnc HW

    bool m_roiEnabled        = false;    //!< ROI enabled
    bool m_dirtyRoiEnabled   = false;    //!< dirty ROI enabled
    bool m_mbQpDataEnabled   = false;    //!< mb qp map enabled
    bool m_isNativeRoi       = false;    //!< Whether is Native ROI
    bool m_isArbRoi          = false;    //!< Whether is Adaptive Region Boost ROI
    bool m_roiMode           = false;    //!< 0 Force qp mode, 1 force delta qp mode
    bool m_isArbRoiSupported = true;     //!< Whether is Adaptive Region Boost ROI Supported

    PMOS_RESOURCE      m_streamIn = nullptr; //!< Stream in buffer
    uint8_t *          m_streamInTemp = nullptr;
    uint32_t           m_streamInSize = 0;
    RoiStrategyFactory m_strategyFactory;    //!< Factory of strategy
    RoiOverlap         m_roiOverlap;         //!< ROI and dirty ROI overlap

    EncodeAllocator *m_allocator = nullptr;
    EncodeBasicFeature *m_basicFeature = nullptr;
    CodechalHwInterfaceNext *m_hwInterface = nullptr;
    PMOS_INTERFACE m_osInterface = nullptr;

    bool IFrameIsSet = false;
    bool PBFrameIsSet = false;

MEDIA_CLASS_DEFINE_END(encode__HevcVdencRoi)
};

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_H__