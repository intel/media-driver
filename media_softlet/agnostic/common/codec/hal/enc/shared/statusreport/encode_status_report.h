/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     encode_status_report.h
//! \brief    Defines the class for encode status report
//! \details  
//!
#ifndef __ENCODE_STATUS_REPORT_H__
#define __ENCODE_STATUS_REPORT_H__

#include "media_status_report.h"
#include "encode_status_report_defs.h"
#include "encode_utils.h"
#include "encode_allocator.h"
#include "codec_def_common_encode.h"
#include "media_status_report.h"

namespace encode {

    //!
    //! \struct EncodeStatusReportData
    //! \brief  Encode status report structure
    //!
    struct EncodeStatusReportData
    {
        enum BLOCK_SIZE
        {
            BLOCK_4X4   = 0,
            BLOCK_16X16 = 1,
        };

        struct FRAME_STATS_INFO
        {
            float    PSNRLuma;
            float    PSNRCb;
            float    PSNRCr;
            uint64_t SADLuma;
            float    Qp;

            union
            {
                uint32_t NumMB;
                uint32_t NumCTU;
            };

            BLOCK_SIZE BlockSize;
            uint32_t   NumIntraBlock;
            uint32_t   NumInterBlock;
            uint32_t   NumSkippedBlock;
            uint32_t   reserved[8];
        };

        struct CTUHeader
        {
            union
            {
                struct
                {
                    uint32_t CUcountminus1 : 6;
                    uint32_t MaxDepth : 2;
                    uint32_t reserved : 24;
                };
                uint32_t DW0;
            };
            uint16_t CurrXAddr;
            uint16_t CurrYAddr;
            uint32_t reserved1;
        };

        struct Int16Pair
        {
            int16_t x;
            int16_t y;
        };

        struct CUInfo
        {
            union
            {
                struct
                {
                    uint32_t CU_Size : 2;
                    uint32_t CU_pred_mode : 1;
                    uint32_t CU_part_mode : 3;
                    uint32_t InterPred_IDC_MV0 : 2;
                    uint32_t InterPred_IDC_MV1 : 2;
                    uint32_t LumaIntraMode : 6;
                    uint32_t ChromaIntraMode : 3;
                    uint32_t reserved : 13;
                };
                uint32_t DW0;
            };

            union
            {
                struct
                {
                    uint32_t LumaIntraMode4x4_1 : 6;
                    uint32_t LumaIntraMode4x4_2 : 6;
                    uint32_t LumaIntraMode4x4_3 : 6;
                    uint32_t reserved1 : 14;
                };
                uint32_t DW1;
            };

            int8_t   QP;
            uint8_t  reserved2[3];
            uint32_t SAD;

            Int16Pair MV[2][2];

            union
            {
                struct
                {
                    uint32_t L0_MV0_RefID : 4;
                    uint32_t L0_MV1_RefID : 4;
                    uint32_t L1_MV0_RefID : 4;
                    uint32_t L1_MV1_RefID : 4;
                    uint32_t reserved3 : 16;
                };
                uint32_t DW8;
            };

            uint32_t reserved4[10];
        };

        struct CTUInfo
        {
            CTUHeader CtuHeader;
            CUInfo    CuInfo[64];
            uint32_t  reserved;
        };

        struct MBInfo
        {
            union
            {
                struct
                {
                    uint32_t MBType : 5;
                    uint32_t InterMBMode : 2;
                    uint32_t IntraMBMode : 2;
                    uint32_t IntraMBFlag : 1;
                    uint32_t SubMBShapes : 8;
                    uint32_t SubMBShapeMode : 8;
                    uint32_t ChromaIntraPredMode : 2;
                    uint32_t reserved : 4;
                };
                uint32_t DW0;
            };

            uint32_t SAD;
            int8_t   Qp;
            uint8_t  reserved1[3];

            uint16_t LumaIntraMode[4];

            uint32_t reserved2;
        };

        struct BLOCK_STATS_INFO
        {
            union
            {
                uint32_t NumMB;
                uint32_t NumCTU;
            };

            union
            {
                CTUInfo *HEVCCTUArray;
                MBInfo  *AVCMBArray;
            };

            uint32_t reserved[8];
        };

        struct BLOCK_SSIM_INFO
        {
            uint32_t NumBlockInColumns;
            uint32_t NumBlockInRows;
            uint8_t* BlockSsimArray;
            uint32_t reserved1[2];
            uint64_t reserved2[2];
        };

        struct BLOCK_QUALITY_INFO
        {
            BLOCK_SSIM_INFO BlockSsim2DS;
            BLOCK_SSIM_INFO BlockSsim4DS;
            BLOCK_SSIM_INFO BlockSsim8DS;
            BLOCK_SSIM_INFO BlockSsim16DS;
            uint32_t        reserved1[32];
            uint64_t        reserved2[12];
        };

        CODECHAL_STATUS                 codecStatus;            //!< Status for the picture associated with this status report
        uint32_t                        statusReportNumber;     //!< Status report number associated with the picture in this status report provided in CodechalEncoderState::Execute()
        CODEC_PICTURE                   currOriginalPic;        //!< Uncompressed frame information for the picture associated with this status report
        CODECHAL_ENCODE_FUNCTION_ID     func;                   //!< Encode function requested at CodechalEncoderState::Execute()
        const void                      *currRefList;           //!< Reference list for the current frame, used for dump purposes with CodecHal Debug Tool
                                                                /*! \brief Specifies the order in which the statuses are expected.
                                                                *
                                                                *   The order in which a status is returned is requested at the DDI level and the order itself is determined by StatusReportNumber.
                                                                *       FALSE indicates the statuses should be returned in reverse order.
                                                                *       TRUE indicates the statuses should be returned in sequential order.
                                                                */
        bool                            sequential;
        /*! \brief Coded bitstream size reported by HW.
        *
        *   The size reported by HW is the total bitstream size that is encoded by HW including any bitstream buffer overrun.  That is, HW continues counting the encoded bytes past the programmed upperbound based on the allocated bitstream buffer size.  The framework can compare this value to the allocated buffer size to determine if there was overflow for this frame and can act accordingly.
        */
        uint32_t                        bitstreamSize;
        /*! \brief Qp value for Y used for the first PAK pass.
        *
        *   It is not valid if CQP is set by framework.
        */
        int8_t                          qpY;
        /*! \brief Suggested Qp delta value for Y.
        *
        *   Framework can add this delta Qp with the first pass QpY to get the final Qp used for multi-pass.  It is not valid if CQP is set by framework.
        *   Note: Framework can use this reported QpY and suggestedQpYDelta to set QpY in picture parameter to minimize LCU level Qp delta.
        */
        int8_t                          suggestedQPYDelta;
        uint8_t                         numberPasses;       //!< Number of PAK passes executed.
        uint8_t                         averageQP;          //!< The average QP of all MBs or LCUs of the frame.
        HwCounter                       hwCounterValue;
        uint64_t *                      hwCtr;

        union
        {
            struct
            {
                uint32_t panicMode              : 1;    //!< Indicates that panic mode was triggered by HW for this frame.
                uint32_t sliceSizeOverflow      : 1;    //!< When SliceLevelRateCtrl is used, indicates the requested slice size was not met for one or more generated slices.
                uint32_t numSlicesNonCompliant  : 1;    //!< When SliceLevelRateCtrl is used, indicates whether or not the number of generated slices exceeds specification limits.
                uint32_t longTermReference      : 1;
                uint32_t frameSkipped           : 1;
                uint32_t sceneChangeDetected    : 1;
                uint32_t                        : 26;
            };
            uint32_t queryStatusFlags;
        };
        /*! \brief The average MAD (Mean Absolute Difference) across all macroblocks in the Y plane.
        *
        *    The MAD value is the mean of the absolute difference between the pixels in the original block and the corresponding pixels in the block being used for comparison, from motion compensation or intra spatial prediction. MAD reporting is disabled by default.
        */
        uint32_t                        mad;
        uint32_t                        loopFilterLevel;        //!< [VP9]
        int8_t                          longTermIndication;     //!< [VP9]
        uint16_t                        nextFrameWidthMinus1;   //!< [VP9]
        uint16_t                        nextFrameHeightMinus1;  //!< [VP9]
        uint8_t                         numberSlices;           //!< Number of slices generated for the frame.
        uint16_t                        psnrX100[3];            //!< PSNR for different channels
        uint32_t                        numberTilesInFrame;     //!< Number of tiles generated for the frame.
        uint8_t                         usedVdBoxNumber;        //!< Number of vdbox used.
        uint32_t                        sizeOfSliceSizesBuffer; //!< Store the size of slice size buffer
        uint16_t                        *sliceSizes;           //!< Pointer to the slice size buffer
        uint32_t                        sizeOfTileInfoBuffer;   //!< Store the size of tile info buffer
        CodechalTileInfo*               hevcTileinfo;          //!< Pointer to the tile info buffer
        uint32_t                        numTileReported;        //!< The number of tiles reported in status

        /*! \brief indicate whether it is single stream encoder or MFE.
        *
        *    For single stream encoder (regular), this value should be set to default 0. For Multi-Frame-Encoder (MFE), this value is the StreamId that is set by application.
        */
        uint32_t                        streamId;

        LookaheadReport                 *pLookaheadStatus;     //!< Pointer to the lookahead status buffer. Valid in lookahead pass only.

        FRAME_STATS_INFO *pFrmStatsInfo;
        BLOCK_STATS_INFO *pBlkStatsInfo;

        // Store Data for Av1 Back Annotation
        uint32_t                        av1FrameHdrOBUSizeByteOffset;
        uint32_t                        av1EnableFrameOBU;
        uint32_t                        frameWidth;
        uint32_t                        frameHeight;

        uint32_t                        MSE[3];

        BLOCK_QUALITY_INFO* pBlkQualityInfo;
    };

    class EncoderStatusReport : public MediaStatusReport
    {
    public:
        //!
        //! \brief  Constructor
        //! \param  [in] hwInterface
        //!         Pointer to CodechalHwInterface
        //! \param  [in] allocator
        //!         Pointer to EncodeAllocator
        //! \param  [in] enableMfx
        //!         Enable Mfx status buffer contarol if true
        //! \param  [in] enableRcs
        //!         Enable Rcs status buffer contarol if true
        //!
        EncoderStatusReport(EncodeAllocator *allocator, PMOS_INTERFACE pOsInterface, bool enableMfx, bool enableRcs, bool enableCp);
        virtual ~EncoderStatusReport();

        //!
        //! \brief  Create resources for status report and do initialization
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Create() override;
        //!
        //! \brief  Destroy resources for status report
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Destroy();
        //!
        //! \brief  Initialize the status in report for each item
        //! 
        //! \details Called per frame for normal usages.
        //!          It can be called per tilerow if enable tile replay mode.
        //!
        //! \param  [in] inputPar
        //!         Pointer to parameters pass to status report.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init(void *inputPar) override;
        //!
        //! \brief  Reset Status
        //! 
        //! \details Called per frame for normal usages.
        //!          It can be called per tilerow if enable tile replay mode.
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Reset() override;

        virtual PMOS_RESOURCE GetHwCtrBuf();

    protected:
        //!
        //! \brief  Collect the status report information into report buffer.
        //! \param  [in] report
        //!         The report buffer address provided by DDI.
        //! \param  [in] index
        //!         The index of current requesting report.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS ParseStatus(void *report, uint32_t index) override;

        virtual MOS_STATUS SetStatus(void *report, uint32_t index, bool outOfRange = false) override;

        //!
        //! \brief  Set offsets for Mfx status buffer.
        //! \return void
        //!
        void SetOffsetsForStatusBufMfx();

        //!
        //! \brief  Collect the common MFX status report data.
        //! \param  [in] statusReportData
        //!         The pointer to EncodeStatusReportData.
        //! \param  [in] index
        //!         The index of current requesting report.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS GetCommonMfxReportData(
            EncodeStatusReportData *statusReportData,
            uint32_t index);

        //!
        //! \brief  Update the status result of current report.
        //! \param  [in] statusReportData
        //!         The pointer to EncodeStatusReportData.
        //! \param  [in] encodeStatusRcs
        //!         The RCS status report buffer.
        //! \param  [in] completed
        //!         Whether the request frame compelted.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS UpdateCodecStatus(
            EncodeStatusReportData *statusReportData,
            EncodeStatusRcs *encodeStatusRcs,
            bool completed);

        //!
        //! \brief  Get the index accroding the inputed codec function.
        //! \param  [in] func
        //!         The value of Codec Function
        //! \return uint32_t
        //!         Return pair index of CodecFuncToFuncIdPairs
        //!
        inline uint32_t GetIdForCodecFuncToFuncIdPairs(uint32_t func) const
        {
            uint32_t ret = 0;
            while (func > 1)
            {
                func >>= 1;
                ret++;
            }

            return ret;
        }

    protected:
        EncodeStatusReportData m_statusReportData[m_statusNum] = {};
        PMOS_INTERFACE         m_osInterface = nullptr;
        bool                   m_enableMfx = false;
        bool                   m_enableRcs = false;
        bool                   m_enableCp  = false;

        const uint32_t         m_statusBufSizeMfx = MOS_ALIGN_CEIL(sizeof(EncodeStatusMfx), sizeof(uint64_t));
        const uint32_t         m_statusBufSizeRcs = MOS_ALIGN_CEIL(sizeof(EncodeStatusRcs), sizeof(uint64_t));

        PMOS_RESOURCE          m_statusBufMfx = nullptr;
        PMOS_RESOURCE          m_statusBufRcs = nullptr;
        uint8_t                *m_dataStatusMfx = nullptr;
        uint8_t                *m_dataStatusRcs = nullptr;
        PMOS_RESOURCE           m_hwcounterBuf  = nullptr;
        uint64_t *              m_hwcounter     = nullptr;
        uint32_t *              m_hwcounterBase = nullptr;

        EncodeAllocator        *m_allocator = nullptr;  //!< encoder allocator

        bool                   m_hwWalker = false;
        uint16_t               m_picWidthInMb = 0;
        uint16_t               m_frameFieldHeightInMb = 0;
        uint32_t               m_maxNumSlicesAllowed = 0;

        static const uint32_t  m_maxCodecFuncNum = 12;
        static const uint32_t  m_codecFuncToFuncIdPairs[m_maxCodecFuncNum];

    MEDIA_CLASS_DEFINE_END(encode__EncoderStatusReport)
    };
}

#endif // !__ENCODE_STATUS_REPORT_H__
