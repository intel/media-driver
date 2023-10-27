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
//! \file     encode_vdenc_lpla_analysis.h
//! \brief    Defines for encode lowpower lookahead(Lookahead Analysis Pass) feature
//!

#ifndef __ENCODE_VDENC_LPLA_ANALYSIS_H__
#define __ENCODE_VDENC_LPLA_ANALYSIS_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "encode_basic_feature.h"
#include "encode_pipeline.h"
#include "encode_huc_brc_update_packet.h"
#include "encode_lpla.h"

namespace encode
{
#define VDBOX_HUC_LA_ANALYSIS_KERNEL_DESCRIPTOR 16

    struct VdencStreamInState
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

        inline bool operator==(const VdencStreamInState &ps) const
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

    // cqm type
    enum
    {
        CQM_HINT_USE_FLAT_MATRIX          = 0,    //use flat matrix
        CQM_HINT_USE_WEAK_CUST_MATRIX     = 1,    //use weak customized matrix
        CQM_HINT_USE_MEDIUM_CUST_MATRIX   = 2,    //use medium customized matrix
        CQM_HINT_USE_STRONG_CUST_MATRIX   = 3,    //use strong customized matrix
        CQM_HINT_USE_EXTREME_CUST_MATRIX  = 4,    //use extreme customized matrix
        CQM_HINT_NUM_CUST_MATRIX          = 4,    //the number of customized matrix
        CQM_HINT_INVALID                  = 0xFF  //invalid hint
    };
    //!
    //! \struct    VdencHevcLaStats
    //! \brief     Vdenc HEVC lookahead info for BRC
    //!
    struct VdencHevcLaStats
    {
        uint32_t sad = 0;
        uint32_t frameByteCount = 0;
        uint32_t headerBitCount = 0;
        uint32_t intraCuCount = 0;
        uint32_t reserved[4];
    };

    //!
    //! \struct    VdencHevcLaData
    //! \brief     Encode lookahead analysis output data structure
    //!
    struct VdencHevcLaData
    {
        uint32_t reserved0[1];
        uint32_t targetFrameSize = 0;
        uint32_t targetBufferFulness = 0;
        uint32_t reserved1[2];
        union
        {
            struct
            {
                uint32_t cqmHint : 8;  //!< Custom quantization matrix hint. 0x00 - flat matrix; 0x01 - CQM; 0xFF - invalid hint; other values are reserved.
                uint32_t reserved2 : 24;
            };
            uint32_t encodeHints = 0;
        };
        uint32_t pyramidDeltaQP = 0;
        uint32_t reserved3[9];
    };

    //!
    //! \struct    VdencHevcHucLaDmem
    //! \brief     This struct is defined for Lookahead HUC kernel DMEM
    //!
    struct VdencHevcHucLaDmem
    {
        uint32_t lookAheadFunc = 0;  //0: init, 1 update
        // Init
        uint32_t lengthAhead = 0;         // in the units of frames
        uint32_t vbvBufferSize = 0;       // in the units of frames
        uint32_t vbvInitialFullness = 0;  // in the units of frames
        uint32_t cuCount = 0;             // normalized 8x8 CU count
        uint32_t statsRecords = 0;        // # of statistic records
        uint32_t averageFrameSize = 0;    // in the units of bytes, should be target_bit_rate/frame_rate
        uint16_t minGop = 0;
        uint16_t maxGop = 0;
        uint16_t BGop = 0;
        uint16_t AGop = 0;
        uint16_t AGop_Threshold = 0;
        uint16_t PGop = 0;
        uint8_t  downscaleRatio = 0;  // 0-no scale, 1-2x, 2-4x
        uint8_t  isIframeInsideBGOP = 0;
        uint8_t  adaptiveIDR = 0;
        uint8_t  GopOpt = 0;  //0 open GOP, 1 close GOP, 2 strict GOP
        uint32_t mbr_ratio = 0;
        uint8_t  la_dump_type = 0;
        uint8_t  codec_type = 0;
        uint8_t  RSVD[2];
        uint32_t enc_frame_width = 0;
        uint32_t enc_frame_height = 0;
        uint8_t  RSVD1[4];
        // for Update, valid only when lookAheadFunc = 1
        uint32_t validStatsRecords = 0;  // # of valid stats records
        uint32_t offset = 0;             // offset in unit of entries
        uint8_t  cqmQpThreshold = 0;     // QP threshold for CQM enable/disable. If estimated QP > CQM_QP_threshold, kernel set HUC_HEVC_LA_DATA.enableCQM to 1.
        uint8_t  currentPass = 0;
        uint8_t  RSVD2[54];
    };

    //!
    //! \struct    CodechalVdencHevcLaData
    //! \brief     Codechal encode lookahead analysis output data structure
    //!
    struct CodechalVdencHevcLaData
    {
        uint32_t reserved0[1];
        uint32_t targetFrameSize;
        uint32_t targetBufferFulness;
        uint32_t reserved1[2];
        union
        {
            struct
            {
                uint32_t cqmHint    : 8;  //!< Custom quantization matrix hint. 0x00 - flat matrix; 0x01 - CQM; 0xFF - invalid hint; other values are reserved.
                uint32_t reserved2  : 24;
            };
            uint32_t encodeHints;
        };
        uint32_t pyramidDeltaQP;
        uint8_t  adaptive_rounding;
        uint8_t  miniGopSize;
        uint32_t reserved3[8];
    };

    class VdencLplaAnalysis : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::huc::Itf::ParSetting
    {
    public:
        VdencLplaAnalysis(
            MediaFeatureManager *featureManager,
            EncodeAllocator *allocator,
            CodechalHwInterfaceNext *hwInterface,
            void *constSettings);

        virtual ~VdencLplaAnalysis();

        //!
        //! \brief  Init lpla analysis features related parameter
        //! \param  [in] settings
        //!         Pointer to settings
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init(void *settings) override;

        //!
        //! \brief  Update cqp basic features related parameter
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(void *params) override;

        //!
        //! \brief    Setup/configure encoder based on sequence parameter set
        //! \details  It is invoked when the encoder receives a new sequence parameter set and it would
        //!           set up and configure the encoder state that used for the sequence
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetSequenceStructs();

        MOS_STATUS UpdateLaDataIdx();

        //!
        //! \brief    Read stats from VDEnc and PAK for lookahead
        //!
        //! \param    [in] cmdBuffer
        //!            Pointer to command buffer
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS StoreLookaheadStatistics(MOS_COMMAND_BUFFER &cmdBuffer, MHW_VDBOX_NODE_IND vdboxIndex);

        //!
        //! \brief    Read stats from VDEnc for lookahead
        //!
        //! \param    [in] cmdBuffer
        //!            Pointer to command buffer
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS StoreVdencStatistics(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t index);

        MOS_STATUS SetLaUpdateDmemParameters(HUC_DMEM_STATE_PAR_ALIAS &dmemParams,
            uint8_t currRecycledBufIdx, uint16_t curPass, uint16_t numPasses);

        //!
        //! \brief  Set look ahead init dmem buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetLaInitDmemBuffer() const;

        //!
        //! \brief  Set look ahead update dmem buffer
        //! \param  [in] currRecycledBufIdx
        //!         Current recycled buffer index
        //! \param  [in] currLaDataIdx
        //!         Current lookahead data index
        //! \param  [in] numValidLaRecords
        //!         Valid Lookahead records number
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetLaUpdateDmemBuffer(uint8_t currRecycledBufIdx, uint8_t currLaDataIdx,
            uint32_t numValidLaRecords, uint16_t curPass, uint16_t numPasses);

        //!
        //! \brief  Get look ahead status report
        //! \param  [in] encodeStatusMfx
        //!         encode status
        //! \param  [in] statusReportData
        //!         status report data
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS GetLplaStatusReport(EncodeStatusMfx *encodeStatusMfx, EncodeStatusReportData *statusReportData);

        //!
        //! \brief  Calculate Look ahead records
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculateLaRecords(bool blastPass);

        //!
        //! \brief  Check if look ahead pass is required
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        bool IsLaAnalysisRequired();

        //!
        //! \brief  Check if look ahead record is empty
        //! \return bool
        //!         true if record is empty
        //!
        bool IsLaRecordsEmpty();

        //!
        //! \brief  Check if look ahead record is empty
        //! \return bool
        //!         true if record is empty
        //!
        bool IsLastPicInStream() { return m_lastPicInStream; }

        //!
        //! \brief  Check if look ahead init is required
        //! \return bool
        //!         true if look ahead init is required
        //!
        bool IsLaInitRequired() { return m_lookaheadInit; }

        inline bool IsLplaAIdrEnabled(){ return m_lookaheadAdaptiveI; }

        MOS_STATUS SetConditionalPass(bool blastPass, bool &condPass);
        MOS_STATUS EnableStreamIn(bool is1stPass, bool isLastPass, bool &streaminEnabled);
        MOS_STATUS SetVdencPipeBufAddrParams(bool enableStreamIn, MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

        MOS_STATUS ReadLPLAData(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE resource, uint32_t baseOffset);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_CMD2);

        MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);

        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

#if USE_CODECHAL_DEBUG_TOOL
        //!
        //! \brief  Dump look ahead resource
        //! \param  [in] pipeline
        //!         encode pipeline
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpLaResource(EncodePipeline *pipeline, bool isInput);
#endif

    protected:
        //! \brief  Allocate feature related resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AllocateResources() override;

        MOS_STATUS GetLaStatsStoreIdx(uint8_t &index);

        MOS_STATUS SetupForceIntraStreamIn();

        MOS_STATUS SetStreaminDataPerRegion(
            uint32_t streamInWidth, uint32_t top, uint32_t bottom, uint32_t left, uint32_t right,
            mhw::vdbox::vdenc::VDENC_STREAMIN_STATE_PAR *streaminParams, void *streaminData);

        MOS_STATUS SetStreaminDataPerLcu(mhw::vdbox::vdenc::VDENC_STREAMIN_STATE_PAR *streaminParams, void *streaminData);

        MOS_STATUS StreaminZigZagToLinearMap( uint32_t  streamInWidth,  uint32_t  x, uint32_t  y, uint32_t *offset, uint32_t *xyOffset);

        CODEC_HEVC_ENCODE_SEQUENCE_PARAMS *m_hevcSeqParams = nullptr;  //!< Pointer to sequence parameter
        CODEC_HEVC_ENCODE_PICTURE_PARAMS  *m_hevcPicParams = nullptr;  //!< Pointer to picture parameter
        CODEC_HEVC_ENCODE_SLICE_PARAMS *m_hevcSliceParams = nullptr; //!< Pointer to slice parameter
        HevcBasicFeature                  *m_hevcBasicFeature = nullptr;  //!< Hevc Basic Feature used in each frame

        EncodeBasicFeature       *m_basicFeature    = nullptr;  //!< EncodeBasicFeature
        CodechalHwInterfaceNext      *m_hwInterface     = nullptr;  //!< Codechal HW Interface
        EncodeAllocator          *m_allocator       = nullptr;  //!< Encode Allocator
        PMOS_INTERFACE            m_osInterface     = nullptr;

        std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;
        std::shared_ptr<mhw::mi::Itf>         m_miItf  = nullptr;

        bool                       m_streamInEnabled             = false;    //!< Stream in enable
        bool                       m_lookaheadAdaptiveI          = false;    //!< Adaptive I flag for lookaheads
        static constexpr uint32_t  m_numLaDataEntry              = 128;      //!< number of entries in lookahead data buffer and lookahead stats buffer
        PMOS_RESOURCE              m_vdencLaStatsBuffer          = nullptr;  //!< VDEnc statistics buffer for lookahead
        PMOS_RESOURCE              m_vdencLaDataBuffer           = nullptr;  //!< lookahead data buffer, output of lookahead analysis
        uint32_t                   m_brcLooaheadStatsBufferSize  = 0;        //!< Lookahead statistics buffer size
        uint32_t                   m_numSlices                   = 0;        //!< Number of slices
        PCODEC_ENCODER_SLCDATA     m_slcData                     = nullptr;  //!< record slice header size & position
        PCODECHAL_NAL_UNIT_PARAMS *m_nalUnitParams               = nullptr;  //!< Pointer to NAL unit parameters
        uint8_t                    m_lookaheadDepth              = 0;        //!< Number of frames to lookahead
        uint32_t                   m_vdencLaInitDmemBufferSize   = 0;        //!< Offset of Lookahead init DMEM buffer
        PMOS_RESOURCE              m_vdencLaInitDmemBuffer       = nullptr;  //!< lookahead init dmem buffer
        uint32_t                   m_averageFrameSize            = 0;        //!< Average frame size based on targed bitrate and frame rate, in unit of bits
        PMOS_RESOURCE              m_vdencLaHistoryBuffer        = nullptr;  //!< VDEnc lookahead history buffer
        static constexpr uint32_t  m_LaHistoryBufSize            = 8192;     //!< Lookahead history buffer size
        uint32_t                   m_brcLooaheadDataBufferSize   = 0;        //!< Lookahead statistics buffer size
        uint32_t                   m_vdencLaUpdateDmemBufferSize = 0;        //!< Offset of Lookahead update DMEM buffer
        uint8_t                    m_cqmQpThreshold              = 40;       //!< QP threshold for CQM enable/disable. Used by lookahead analysis kernel.
        bool                       m_lastPicInStream             = false;    //!< Flag to indicate if it is last picture in stream
        uint32_t                   m_targetBufferFulness         = 0;        //!< Target encode buffer fulness in bits, used by BRC and calculated from initial buffer fulness, target frame size (from DDI) and average frame size
        int32_t                    m_bufferFulnessError          = 0;        //!< VBV buffer fulness error between unit of bits (used by driver) and unit of frame (used by LA analsis kernel)
        bool                       m_initDeltaQP                 = true;     //!< Initial delta QP
        uint32_t                   m_prevQpModulationStrength    = 0;        //!< Previous QP Modulation strength
        uint32_t                   m_prevTargetFrameSize         = 0;        //!< Target frame size of previous frame.
        bool                       m_lookaheadReport             = false;    //!< Lookahead report valid flag
        uint32_t                   m_numValidLaRecords           = 0;        //!< Number of valid lookahead records
        uint8_t                    m_currLaDataIdx               = 0;        //!< Current lookahead data index
        mutable bool               m_lookaheadInit               = true;     //!< Lookahead init flag
        EncodeLPLA                *m_lplaHelper                  = nullptr;  //!< Lookahead helper
        uint32_t                   m_offset = 0;

        uint32_t                   m_intraInterval               = 0;  //!< Frame count since last I frame
        bool                       m_forceIntraSteamInSetupDone  = false;
        PMOS_RESOURCE              m_forceIntraStreamInBuf       = nullptr;
        PMOS_RESOURCE              m_vdencLaUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_LPLA_NUM_OF_PASSES] = {};  //!< VDEnc Lookahead Update DMEM buffer
        uint32_t                   m_statsBuffer[600][4]                                                                       = {};
        bool                       m_useDSData = false;
        bool                       m_bLastPicFlagFirstIn                                                                       = true;

    MEDIA_CLASS_DEFINE_END(encode__VdencLplaAnalysis)
    };
} // namespace encode

#endif
