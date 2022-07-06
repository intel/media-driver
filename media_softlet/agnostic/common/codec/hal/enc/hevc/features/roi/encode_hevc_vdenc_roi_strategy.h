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
//! \file     encode_hevc_vdenc_roi_strategy.h
//! \brief    Defines of the ROI strategy
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_STRATEGY_H__
#define __CODECHAL_HEVC_VDENC_ROI_STRATEGY_H__

#include "encode_recycle_resource.h"
#include "encode_hevc_basic_feature.h"
#include "encode_hevc_brc.h"
#include "encode_hevc_vdenc_roi_overlap.h"
#include "encode_hevc_vdenc_const_settings.h"
#include "mhw_vdbox_huc_itf.h"

namespace encode
{
struct HevcVdencStreamInState
{
    // DWORD 0
    union
    {
        struct
        {
            uint32_t RoiCtrl : MOS_BITFIELD_RANGE(0, 7);
            uint32_t MaxTuSize : MOS_BITFIELD_RANGE(8, 9);
            uint32_t MaxCuSize : MOS_BITFIELD_RANGE(10, 11);
            uint32_t NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
            uint32_t Reserved_0 : MOS_BITFIELD_RANGE(16, 20);
            uint32_t ForceQPDelta : MOS_BITFIELD_BIT(21);
            uint32_t PaletteDisable : MOS_BITFIELD_BIT(22);
            uint32_t Reserved_1 : MOS_BITFIELD_BIT(23);
            uint32_t PuTypeCtrl : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW0;

    // DWORD 1-4
    union
    {
        struct
        {
            uint32_t ForceMvX : MOS_BITFIELD_RANGE(0, 15);
            uint32_t ForceMvY : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW1[4];

    // DWORD 5
    union
    {
        struct
        {
            uint32_t Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            uint32_t ForceRefIdx : MOS_BITFIELD_RANGE(0, 15);  //4-bits per 16x16 block
            uint32_t NumMergeCandidateCu8x8 : MOS_BITFIELD_RANGE(16, 19);
            uint32_t NumMergeCandidateCu16x16 : MOS_BITFIELD_RANGE(20, 23);
            uint32_t NumMergeCandidateCu32x32 : MOS_BITFIELD_RANGE(24, 27);
            uint32_t NumMergeCandidateCu64x64 : MOS_BITFIELD_RANGE(28, 31);
        };
        uint32_t Value;
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            uint32_t SegID : MOS_BITFIELD_RANGE(0, 15);  //4-bits per 16x16 block
            uint32_t QpEnable : MOS_BITFIELD_RANGE(16, 19);
            uint32_t SegIDEnable : MOS_BITFIELD_RANGE(20, 20);
            uint32_t Reserved : MOS_BITFIELD_RANGE(21, 22);
            uint32_t ForceRefIdEnable : MOS_BITFIELD_RANGE(23, 23);
            uint32_t ImePredictorSelect : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW7;

    // DWORD 8-11
    union
    {
        struct
        {
            uint32_t ImePredictorMvX : MOS_BITFIELD_RANGE(0, 15);
            uint32_t ImePredictorMvY : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW8[4];

    // DWORD 12
    union
    {
        struct
        {
            uint32_t ImePredictorRefIdx : MOS_BITFIELD_RANGE(0, 15);  //4-bits per 16x16 block
            uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            uint32_t PanicModeLCUThreshold : MOS_BITFIELD_RANGE(0, 15);
            uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
        };
        uint32_t Value;
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            uint32_t ForceQp_0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t ForceQp_1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t ForceQp_2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t ForceQp_3 : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            uint32_t Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        uint32_t Value;
    } DW15;

    inline bool operator==(const HevcVdencStreamInState &ps) const
    {
        if ((this->DW0.Value == ps.DW0.Value) &&
            (this->DW1[0].Value == ps.DW1[0].Value) &&
            this->DW1[1].Value == ps.DW1[1].Value &&
            this->DW1[2].Value == ps.DW1[2].Value &&
            this->DW1[3].Value == ps.DW1[3].Value &&
            this->DW5.Value == ps.DW5.Value &&
            this->DW6.Value == ps.DW6.Value &&
            this->DW7.Value == ps.DW7.Value &&
            this->DW8[0].Value == ps.DW8[0].Value &&
            this->DW8[1].Value == ps.DW8[1].Value &&
            this->DW8[2].Value == ps.DW8[2].Value &&
            this->DW8[3].Value == ps.DW8[3].Value &&
            this->DW12.Value == ps.DW12.Value &&
            this->DW13.Value == ps.DW13.Value &&
            this->DW14.Value == ps.DW14.Value &&
            this->DW15.Value == ps.DW15.Value)
            return true;
        return false;
    }
};
//!
//! \struct    DeltaQpForRoi
//! \brief     This struct is defined for BRC Update HUC kernel
//!            region 10 - Delta Qp for ROI Buffer
//!
struct DeltaQpForRoi
{
    int8_t iDeltaQp;
};

using SeqParams      = CODEC_HEVC_ENCODE_SEQUENCE_PARAMS;
using PicParams      = CODEC_HEVC_ENCODE_PICTURE_PARAMS;
using SlcParams      = CODEC_HEVC_ENCODE_SLICE_PARAMS;
using StreamInParams = mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_STREAMIN_STATE);
using UintVector     = std::vector<uint32_t>;

//!
//! \class    RoiStrategy
//!
//! \brief    Base class of Native ROI. ForceQP ROI, HuC based ForceQP ROI and
//!           Dirty ROI.
//!
//! \detail   This class provided unified interface of all ROI, and implemented
//!           some common functions which will be used by the sub classes.
//!
class RoiStrategy : public mhw::vdbox::huc::Itf::ParSetting
{
public:
    RoiStrategy(EncodeAllocator *allocator,
        MediaFeatureManager *featureManager,
        PMOS_INTERFACE osInterface) :
        m_allocator(allocator),
        m_featureManager(featureManager),
        m_osInterface(osInterface)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        m_recycle = m_basicFeature->m_recycleBuf;
    }

    virtual ~RoiStrategy() {}

    //!
    //! \brief    Prepare parameters
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
    virtual MOS_STATUS PrepareParams(
        SeqParams *hevcSeqParams,
        PicParams *hevcPicParams,
        SlcParams *hevcSlcParams);

    //!
    //! \brief    Setup the ROI regione
    //!
    //! \param    [in] overlap
    //!           Overlap between ROI and dirty ROI
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupRoi(RoiOverlap &overlap);

    //!
    //! \brief    Write the Streamin data according to marker.
    //! \param    [in] lcuIndex
    //!           Index of LCU
    //! \param    [in] marker
    //!           overlap marker
    //! \param    [in] roiRegionIndex
    //!           Index of ROI region
    //! \param    [out] streamInBuffer
    //!           Streamin buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS WriteStreaminData(uint32_t lcuIndex,
        RoiOverlap::OverlapMarker marker,
        uint32_t roiRegionIndex,
        uint8_t *streamInBuffer);

    //!
    //! \brief    Set VDENC_PIPE_BUF_ADDR parameters
    //!
    //! \param    [in] streamIn
    //!           Stream in buffer
    //! \param    [out] PipeBufAddrParams
    //!           Pipe buf addr parameters
    //!
    //! \return   void
    //!
    virtual void SetVdencPipeBufAddrParams(PMOS_RESOURCE streamin,
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        pipeBufAddrParams.presVdencStreamInBuffer = streamin;
        return;
    }

    //!
    //! \brief    Get stream in buffer
    //!
    //! \return   PMOS_RESOURCE
    //!           Stream in buffer
    //!
    virtual PMOS_RESOURCE GetStreamInBuf() const { return nullptr; }

    //!
    //! \brief    Setup HuC BRC init/reset parameters
    //!
    //! \param    [out] hucVdencBrcInitDmem
    //!           pointer of PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemHuCBrcInitReset(
        VdencHevcHucBrcInitDmem *hucVdencBrcInitDmem)
    {
        return MOS_STATUS_SUCCESS;
    }

    void SetFeatureSetting(HevcVdencFeatureSettings *settings) { m_FeatureSettings = settings; }

protected:
    //!
    //! \brief    Calculate X/Y offsets for zigzag scan within 64 LCU
    //!
    //! \param    [in] streamInWidth
    //!           StreamInWidth, location of top left corner
    //! \param    [in] x
    //!           Position X
    //! \param    [in] y
    //!           Position Y
    //! \param    [out] offset
    //!           Offsets into the stream-in surface
    //! \param    [out] xyOffset
    //!           XY Offsets into the stream-in surface
    //!
    //! \return   void
    //!
    void StreaminZigZagToLinearMap(
        uint32_t  streamInWidth,
        uint32_t  x,
        uint32_t  y,
        uint32_t *offset,
        uint32_t *xyOffset);

    //!
    //! \brief    Calculate X/Y position for linear scan in current frame
    //!
    //! \param    [in] streamInWidth
    //!           StreamInWidth, location of top left corner
    //! \param    [in] lcuIndex
    //!           Index for 32x32 cu in current frame
    //! \param    [out] x
    //!           Position X 
    //! \param    [out] y
    //!           Position Y
    //!
    //! \return   void
    //!
    void ZigZagToRaster(
        uint32_t  streamInWidth,
        uint32_t  lcuIndex,
        uint32_t  &x,
        uint32_t  &y);

    //!
    //! \brief    Get LCUs' index In ROI region
    //!
    //! \param    [in] streamInWidth
    //!           StreamInWidth, location of top left corner
    //! \param    [in] top
    //!           top of the ROI region
    //! \param    [in] bottom
    //!           bottom of the ROI region
    //! \param    [in] left
    //!           left of the ROI region
    //! \param    [in] bottom
    //!           right of the ROI region
    //! \param    [out] lcuVector
    //!           vector of LCUs' index
    //!
    //! \return   void
    //!
    void GetLCUsInRoiRegion(
        uint32_t    streamInWidth,
        uint32_t    top,
        uint32_t    bottom,
        uint32_t    left,
        uint32_t    right,
        UintVector &lcuVector);

    //!
    //! \brief    Get LCUs' index In ROI region in Tile
    //!
    //! \param    [in] streamInWidth
    //!           StreamInWidth, location of top left corner
    //! \param    [in] top
    //!           top of the ROI region
    //! \param    [in] bottom
    //!           bottom of the ROI region
    //! \param    [in] left
    //!           left of the ROI region
    //! \param    [in] bottom
    //!           right of the ROI region
    //! \param    [out] lcuVector
    //!           vector of LCUs' index
    //!
    //! \return   void
    //!
    void GetLCUsInRoiRegionForTile(
        uint32_t    streamInWidth,
        uint32_t    top,
        uint32_t    bottom,
        uint32_t    left,
        uint32_t    right,
        UintVector &lcuVector);

    //!
    //! \brief    Setup stream-in data per region
    //!
    //! \param    [in] lcuVector
    //!                vector of LCUs' index
    //! \param    [in] streaminParams
    //!                 pointer to MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS
    //! \param    [out] streaminData
    //!                 pointer to streaminData
    //!
    //! \return   void
    //!
    void SetStreaminDataPerRegion(
        const UintVector &lcuVector,
        StreamInParams *streaminParams,
        void *streaminData);

    //!
    //! \brief    Write out stream-in data for each LCU
    //!
    //! \param    [in] streaminParams
    //!           Params to write into stream in surface
    //! \param    [out] streaminData
    //!           Pointer to streaminData
    //!
    //! \return   void
    //!
    void SetStreaminDataPerLcu(
        StreamInParams *streaminParams,
        void *streaminData);

    //!
    //! \brief    Set streamin parameter according to the TU
    //!
    //! \param    [in] cu64Align
    //!           Whether CU is 64 aligned
    //! \param    [out] streaminDataParams
    //!           Streamin data parameters
    //!
    //! \return   void
    //!
    virtual void SetStreaminParamByTU(
        bool cu64Align,
        StreamInParams &streaminDataParams);

    //!
    //! \brief    Set the ROI ctrol mode(Native/ForceQP)
    //!
    //! \param    [in] roiCtrl
    //!           ROI control
    //! \param    [in] forceQp
    //!           force QP value
    //! \param    [out] streaminDataParams
    //!           Streamin data parameters
    //!
    //! \return   void
    //!
    virtual void SetRoiCtrlMode(
        uint32_t lcuIndex,
        uint32_t regionIndex,
        StreamInParams &streaminParams) {}

    //!
    //! \brief    Set ROI Control/Force QP Data per LCU
    //!
    //! \param    [in] streaminDataParams
    //!           Streamin data parameters
    //! \param    [out] data
    //!           Streamin data
    //!
    //! \return   void
    //!
    virtual void SetQpRoiCtrlPerLcu(
        StreamInParams *streaminParams,
        HevcVdencStreamInState *data) {}

    static constexpr uint8_t m_maxNumRoi             = 16;  //!< VDEnc maximum number of ROI supported
    static constexpr uint8_t m_maxNumNativeRoi       = 3;   //!< Number of native ROI supported by VDEnc HW
    static constexpr uint8_t m_imgStateImePredictors = 8;   //!< Number of predictors for IME

    uint8_t    m_numRoi      = 0;           //!< Number of ROI
    CODEC_ROI  *m_roiRegions = nullptr;     //!< ROI regions

    uint8_t m_targetUsage = 0;              //!< Target Usage

    int8_t m_qpY          = 0;
    int8_t m_sliceQpDelta = 0;

    uint8_t m_numDistinctDeltaQp = 0;
    int8_t *m_roiDistinctDeltaQp = nullptr;

    bool     m_isTileModeEnabled  = false;
    uint32_t m_minCodingBlockSize = 0;

    EncodeAllocator *m_allocator    = nullptr;
    RecycleResource *m_recycle      = nullptr;
    HevcBasicFeature *m_basicFeature = nullptr;
    MediaFeatureManager *m_featureManager = nullptr;
    HevcVdencFeatureSettings *m_FeatureSettings = nullptr;
    PMOS_INTERFACE m_osInterface = nullptr;

MEDIA_CLASS_DEFINE_END(encode__RoiStrategy)
};

//!
//! \class    RoiStrategyFactory
//!
//! \brief    This is help class to create ROI and Dirty ROI
//!
//! \detail   At the same time, Navtive ROI,
//!           ForceQP ROI and Huc based ForceQP, we can only use one of them in
//!           one frame, which we can treat them as traditional ROI. Different
//!           with traditional ROI, dirty ROI can exist with them simultaneously.
//!           We should create traditinal ROI and dirty ROI seperately if they 
//!           exist in the same time.
//!
class RoiStrategyFactory
{
public:
    //!
    //! \brief    Create ROIs, include Native ROI, ForceQP ROI, Huc based ROI and
    //!           dirty ROI according to the input parameters
    //!
    //! \param    [in] allocator
    //!           Encode allocator
    //! \param    [in] recycle
    //!           Recycle buffer
    //! \param    [in] params
    //!           Encode parameters
    //! \param    [in] isDirtyRoi
    //!           Whether is dirty ROI or not.
    //! \param    [in] isNativeRoi
    //!           Whether is native ROI or not.
    //!
    //! \return   RoiStrategy *
    //!           Pointer of RoiStrategy
    //!
    RoiStrategy *CreateStrategy(
        EncodeAllocator     *allocator,
        MediaFeatureManager *featureManager,
        PMOS_INTERFACE       m_osInterface,
        bool                 isArb,
        bool                 isDirtyRoi,
        bool                 isNativeRoi,
        bool                 isQPMap = false);

    //!
    //! \brief    Create ROIs, include force delta qp ROI,
    //!
    //!
    //! \param    [in] allocator
    //!           Encode allocator
    //! \param    [in] featureManager
    //!           Media feature manager.
    //!
    //! \return   RoiStrategy *
    //!           Pointer of RoiStrategy
    //!
    RoiStrategy *CreateStrategyForceDeltaQP(
        EncodeAllocator *allocator,
        MediaFeatureManager *featureManager,
        PMOS_INTERFACE m_osInterface);

    ~RoiStrategyFactory();

    //!
    //! \brief    Get dirty ROI for current frame
    //!
    //! \return   RoiStrategy *
    //!           Pointer of RoiStrategy
    //!
    RoiStrategy *GetDirtyRoi() const { return m_dirtyRoi; }
    //!
    //! \brief    Get traditional ROI for current frame
    //!
    //! \return   RoiStrategy *
    //!           Pointer of RoiStrategy
    //!
    RoiStrategy *GetRoi() const { return m_currentRoi; }

private:
    RoiStrategy *m_nativeRoi  = nullptr;
    RoiStrategy *m_arbRoi  = nullptr;
    RoiStrategy *m_forceQpRoi = nullptr;
    RoiStrategy *m_hucForceQpRoi = nullptr;
    RoiStrategy *m_currentRoi = nullptr;
    RoiStrategy *m_dirtyRoi   = nullptr;
    RoiStrategy *m_QPMapROI  = nullptr;
    RoiStrategy *m_deltaQpRoi = nullptr;

MEDIA_CLASS_DEFINE_END(encode__RoiStrategyFactory)
};

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_STRATEGY_H__
