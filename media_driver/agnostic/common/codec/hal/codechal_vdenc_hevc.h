/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_vdenc_hevc.h
//! \brief    Defines base class for HEVC VDEnc encoder.
//!

#ifndef __CODECHAL_VDENC_HEVC_H__
#define __CODECHAL_VDENC_HEVC_H__

#include "codechal_encode_hevc_base.h"
#include "codechal_mmc_encode_hevc.h"
#include "codechal_huc_cmd_initializer.h"

//!
//! \struct    CodechalVdencHevcPakInfo
//! \brief     Codechal Vdenc HEVC Pak info
//!
struct CodechalVdencHevcPakInfo
{
    uint32_t  FrameByteCount;
    uint8_t   PAKPassNum;
};

//!
//! \struct    CodechalVdencHevcLaStats
//! \brief     Codechal Vdenc HEVC lookahead info for BRC
//!
struct CodechalVdencHevcLaStats
{
    uint32_t  sad;
    uint32_t  frameByteCount;
    uint32_t  headerBitCount;
    uint32_t  intraCuCount;
    uint32_t  reserved[4];
};

using pCodechalVdencHevcPakInfo = CodechalVdencHevcPakInfo*;

//!
//! \struct    DeltaQpForROI
//! \brief     This struct is defined for BRC Update HUC kernel
//!            region 10 - Delta Qp for ROI Buffer
//!
struct DeltaQpForROI
{
    int8_t    iDeltaQp;
};

using PDeltaQpForROI = DeltaQpForROI*;

//!
//! \struct    CodechalVdencHevcLaDmem
//! \brief     This struct is defined for Lookahead HUC kernel DMEM
//!
struct CodechalVdencHevcLaDmem
{
    uint32_t lookAheadFunc;       // 0: init, 1 update
    // for Init, valid only when lookAheadFunc = 0
    uint32_t lengthAhead;         // in the units of frames
    uint32_t vbvBufferSize;       // in the units of frames
    uint32_t vbvInitialFullness;  // in the units of frames
    uint32_t mbCount;             // normalized 16x16 block count
    uint32_t statsRecords;        // # of statistic records
    // for Update, valid only when lookAheadFunc = 1
    uint32_t validStatsRecords;   // # of valid stats records
    uint32_t offset;              // offset in unit of entries

    uint8_t RSVD[32];
};

using PCodechalVdencHevcLaDmem = CodechalVdencHevcLaDmem *;

//!
//! \class    CodechalVdencHevcState
//! \brief    HEVC VDEnc encoder base class 
//! \details    This class defines the base class for HEVC VDEnc encoder, it includes 
//!        common member fields, functions, interfaces etc shared by all GENs.
//!        Gen specific definitions, features should be put into their corresponding classes.
//!
class CodechalVdencHevcState : public CodechalEncodeHevcBase
{
public:
    //!
    //! \struct    HevcVdencBrcBuffers
    //! \brief     Hevc Vdenc brc buffers
    //!
    struct HevcVdencBrcBuffers
    {
        MOS_RESOURCE                        resBrcPakStatisticBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
        uint32_t                            uiCurrBrcPakStasIdxForRead;
        uint32_t                            uiCurrBrcPakStasIdxForWrite;
    };

    static constexpr uint8_t                m_numMaxVdencL0Ref = 3;                   //!< Max number of reference frame list0
    static constexpr uint8_t                m_numMaxVdencL1Ref = 3;                   //!< Max number of reference frame list1
    static constexpr uint32_t               m_brcPakStatsBufSize = 512;               //!< Pak statistic buffer size
    static constexpr uint32_t               m_brcStatsBufSize = 1216;                 //!< BRC Statistic buf size: 48DWs (3CLs) of HMDC Frame Stats + 256 DWs (16CLs) of Histogram Stats = 1216 bytes
    static constexpr uint32_t               m_brcHistoryBufSize = 6080;              //!< BRC history buffer size
    static constexpr uint32_t               m_brcDebugBufSize = 0x1000;               //!< BRC debug buffer size
    static constexpr uint32_t               m_LaHistoryBufSize = 8192;                //!< Lookahead history buffer size
    static constexpr uint32_t               m_weightHistSize = 1024;                  //!< Weight Histogram (part of VDEnc Statistic): 256 DWs (16CLs) of Histogram Stats = 1024
    static constexpr uint32_t               m_roiStreamInBufferSize = 65536 * CODECHAL_CACHELINE_SIZE; //!< ROI Streamin buffer size (part of BRC Update)
    static constexpr uint32_t               m_deltaQpBufferSize = 65536;              //!< DeltaQp buffer size (part of BRC Update)
    static constexpr uint32_t               m_brcLooaheadStatsBufferSize = m_numLaDataEntry * sizeof(CodechalVdencHevcLaStats); //!< Lookahead statistics buffer size
    static constexpr uint32_t               m_vdboxHucHevcBrcInitKernelDescriptor = 8;//!< Huc HEVC Brc init kernel descriptor
    static constexpr uint32_t               m_vdboxHucHevcBrcUpdateKernelDescriptor = 9;//!< Huc HEVC Brc update kernel descriptor
    static constexpr uint32_t               m_vdboxHucHevcBrcLowdelayKernelDescriptor = 10;//!< Huc HEVC Brc low delay kernel descriptor
    static constexpr uint32_t               m_vdboxHucHevcLaAnalysisKernelDescriptor = 16;//!< Huc lookahead analysis kernel descriptor

    //!< \cond SKIP_DOXYGEN
    // HuC tables
    // Note: These const values are same for all Gen now.
    // In case they become diff later, then need to move declaration to each Gen's derived class
    static const uint8_t                    m_estRateThreshP0[7];
    static const uint8_t                    m_estRateThreshB0[7];
    static const uint8_t                    m_estRateThreshI0[7];
    static const int8_t                     m_instRateThreshP0[4];
    static const int8_t                     m_instRateThreshB0[4];
    static const int8_t                     m_instRateThreshI0[4];
    static const uint16_t                   m_startGAdjFrame[4];
    static const uint8_t                    m_startGAdjMult[5];
    static const uint8_t                    m_startGAdjDiv[5];
    static const uint8_t                    m_rateRatioThreshold[7];
    static const uint8_t                    m_rateRatioThresholdQP[8];
    static const uint32_t                   m_hucModeCostsIFrame[364];
    static const uint32_t                   m_hucModeCostsPbFrame[364];
    static const uint16_t                   m_sadQpLambdaI[52];
    static const uint16_t                   m_sadQpLambdaI_VQI[52];
    static const uint16_t                   m_sadQpLambdaP[52];
    static const uint16_t                   m_rdQpLambdaI[52];
    static const uint16_t                   m_rdQpLambdaP[52];
    static const uint8_t                    m_penaltyForIntraNonDC32x32PredMode[52];
    static const uint8_t                    m_penaltyForIntraNonDC32x32PredMode_VQI[52];
    //! \endcond

    bool                                    m_hevcVdencAcqpEnabled = false;                    //!< ACQP enable flag
    bool                                    m_hevcRdoqAdaptationEnabled = false;               //!< RDOQ adaptation enable flag
    bool                                    m_hevcVdencRoundingEnabled = false;                //!< Rounding enable flag
    bool                                    m_vdencPakObjCmdStreamOutEnabled = false;          //!< Pakobj stream out enable flag
    bool                                    m_vdencHucUsed = false;                            //!< HUC usage flag
    bool                                    m_hevcVdencWeightedPredEnabled = false;            //!< Weighted pred enable flag
    bool                                    m_vdencHuCConditional2ndPass = false;              //!< HuC conditional 2nd pass enable flag
    bool                                    m_vdencNativeROIEnabled = false;                   //!< Native ROI enable flag
    bool                                    m_pakOnlyPass = false;                             //!< flag to signal VDEnc+PAK vs. PAK only
    bool                                    m_hevcVisualQualityImprovement = false;            //!< VQI enable flag
    bool                                    m_enableMotionAdaptive = false;                    //!< Motion adaptive enable flag

    //Resources for VDEnc
    MOS_RESOURCE                            m_sliceCountBuffer;                                //!< Slice count buffer
    MOS_RESOURCE                            m_vdencModeTimerBuffer;                            //!< VDEnc mode timer buffer
    MOS_RESOURCE                            m_resSliceReport[CODECHAL_ENCODE_STATUS_NUM];      //!< Slice size report buffer to be saved across passes
    uint8_t                                 m_maxNumROI = CODECHAL_ENCODE_HEVC_MAX_NUM_ROI;    //!< VDEnc maximum number of ROI supported
    uint8_t                                 m_maxNumNativeROI = ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10;  //!< Number of native ROI supported by VDEnc HW
    uint8_t                                 m_imgStateImePredictors = 8;                       //!< Number of predictors for IME

    // BRC
    HevcVdencBrcBuffers                     m_vdencBrcBuffers;                                 //!< VDEnc Brc buffers
    MOS_RESOURCE                            m_dataFromPicsBuffer;                              //!< Data Buffer of Current and Reference Pictures for Weighted Prediction
    MOS_RESOURCE                            m_vdencDeltaQpBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];                              //!< VDEnc delta QP buffer
    MOS_RESOURCE                            m_vdencOutputROIStreaminBuffer;                    //!< VDEnc Output ROI Streamin Buffer
    MOS_RESOURCE                            m_vdencBrcUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES_FOR_TILE_REPLAY];  //!< VDEnc BrcUpdate DMEM buffer
    MOS_RESOURCE                            m_vdencBrcInitDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];                          //!< VDEnc BrcInit DMEM buffer
    MOS_RESOURCE                            m_vdencBrcConstDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];                         //!< VDEnc brc constant data buffer
    MOS_RESOURCE                            m_vdencBrcHistoryBuffer;                           //!< VDEnc brc history buffer
    MOS_RESOURCE                            m_vdencReadBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];  //!< VDEnc read batch buffer
    MOS_RESOURCE                            m_vdencGroup3BatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];  //!< VDEnc read batch buffer for Group3
    MOS_RESOURCE                            m_vdencBrcDbgBuffer;                               //!< VDEnc brc debug buffer
    uint32_t                                m_deltaQpRoiBufferSize = 0;                            //!< VDEnc DeltaQp for ROI buffer size
    uint32_t                                m_brcRoiBufferSize = 0;                                //!< BRC ROI input buffer size
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS       m_virtualAddrParams = {};                              //!< BRC virtual address parameter

    // Batch Buffer for VDEnc
    MHW_BATCH_BUFFER                        m_vdenc2ndLevelBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];  //!< VDEnc 2nd level batch buffer
    uint32_t                                m_vdenc2ndLevelBatchBufferSize[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {0};
    uint32_t                                m_vdencBatchBufferPerSliceVarSize[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = { 0 };    //!< VDEnc batch buffer slice size array
    uint32_t                                m_1stPakInsertObjectCmdSize = 0;                   //!< Size of 1st PAK_INSERT_OBJ cmd
    uint32_t                                m_hcpWeightOffsetStateCmdSize = 0;                 //!< Size of HCP_WEIGHT_OFFSET_STATE cmd
    uint32_t                                m_hcpSliceStateCmdSize = 0;                        //!< Size of HCP_SLICE_STATE cmd
    uint32_t                                m_vdencWeightOffsetStateCmdSize = 0;               //!< Size of VDENC_WEIGHT_OFFSET_STATE cmd
    uint32_t                                m_miBatchBufferEndCmdSize = 0;                     //!< Size of MI_BATCH_BUFFER_END cmd
    uint32_t                                m_picStateCmdStartInBytes = 0;                     //!< Offset of PIC_STATE cmd in batch buffer
    uint32_t                                m_cmd2StartInBytes = 0;
    uint32_t                                m_vdencBrcInitDmemBufferSize = 0;                  //!< Offset of BRC init DMEM buffer
    uint32_t                                m_vdencBrcUpdateDmemBufferSize = 0;                //!< Offset of BRC update DMEM buffer
    uint32_t                                m_vdencBrcConstDataBufferSize = 0;                 //!< Offset of BRC const data buffer
    unsigned char                           m_virtualEngineBbIndex = 0;                        //!< Virtual engine batch buffer index

    // Tile related
    uint32_t                                m_maxTileNumber = 1;                               //!< max tile number, equal to 1 for Gen10

    PCODECHAL_CMD_INITIALIZER               m_hucCmdInitializer = nullptr;

    MOS_RESOURCE                            m_resDelayMinus = {0};
    uint32_t                                m_numDelay = 0;

    // Lookahead
    MOS_RESOURCE                            m_vdencLaStatsBuffer;                              //!< VDEnc statistics buffer for lookahead
    MOS_RESOURCE                            m_vdencLaInitDmemBuffer = {};                           //!< VDEnc Lookahead Init DMEM buffer
    MOS_RESOURCE                            m_vdencLaUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];                  //!< VDEnc Lookahead Update DMEM buffer
    MOS_RESOURCE                            m_vdencLaHistoryBuffer = {};                            //!< VDEnc lookahead history buffer
    bool                                    m_lookaheadPass = false;                           //!< Indicate if current pass is lookahead pass or encode pass
    bool                                    m_lookaheadInit = true;                            //!< Lookahead init flag
    bool                                    m_lookaheadUpdate = false;                         //!< Lookahead update flag
    uint32_t                                m_vdencLaInitDmemBufferSize = 0;                   //!< Offset of Lookahead init DMEM buffer
    uint32_t                                m_vdencLaUpdateDmemBufferSize = 0;                 //!< Offset of Lookahead update DMEM buffer
    uint32_t                                m_numValidLaRecords = 0;                           //!< Number of valid lookahead records

protected:
    //!
    //! \brief    Constructor
    //!
    CodechalVdencHevcState(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

public:
    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencHevcState() {};

    //!
    //! \brief    Setup ROI stream-in resource for BRC and ACQP mode
    //!
    //! \param    [in,out] streamIn
    //!           Pointer to ROI stream-in resource
    //! \param    [in,out] deltaQpBuffer
    //!           Pointer to ROI DeltaQp buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupBRCROIStreamIn(PMOS_RESOURCE streamIn, PMOS_RESOURCE deltaQpBuffer);

    //!
    //! \brief    Setup ROI stream-in resource for CQP mode
    //!
    //! \param    [in,out] streamIn
    //!           Pointer to ROI stream-in resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupROIStreamIn(PMOS_RESOURCE streamIn);

    //!
    //! \brief    Setup dirty rectangle stream-in resource
    //!
    //! \param    [in,out] streamIn
    //!           Pointer to dirty rectangle stream-in resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupDirtyRectStreamIn(PMOS_RESOURCE streamIn);

    //!
    //! \brief    Prepare VDEnc stream-in data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareVDEncStreamInData();

    //!
    //! \brief    Setup stream-in data per region 
    //!
    //! \param    [in] streamInWidth, top, bottom, left, right
    //!                streamInWidth, region corner locations, 
    //! \param    [in] streaminParams
    //!                 pointer to MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS
    //! \param    [out] streaminData
    //!                 pointer to streaminData
    //!
    //! \return   void
    //!
    virtual void SetStreaminDataPerRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData);

    //!
    //! \brief    Setup stream-in data per region 
    //!
    //! \param    [in] streamInWidth, top, bottom, left, right
    //!                streamInWidth, region corner locations
    //! \param    [in] regionId
    //!                region ID
    //! \param    [out] deltaQpMap
    //!                pointer to DeltaQpForROI
    //!
    //! \return   void
    //!
    virtual void SetBrcRoiDeltaQpMap(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        uint8_t regionId,
        PDeltaQpForROI deltaQpMap);

    //!
    //! \brief    Setup stream-in surface for a dirty rectangle
    //!
    //! \param    [in] streamInWidth, top, bottom, left, right, maxcu
    //!           StreamInWidth, dirtyRect corner locations, maxCuSize
    //! \param    [out] streaminData
    //!           Pointer to streaminData
    //!
    //! \return   void
    //!
    virtual void StreaminSetDirtyRectRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        uint8_t  maxcu,
        void* streaminData);

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
    virtual void StreaminZigZagToLinearMap(
        uint32_t streamInWidth,
        uint32_t x,
        uint32_t y,
        uint32_t* offset,
        uint32_t* xyOffset);

    //!
    //! \brief    Setup stream-in for border of non-64 aligned region
    //!
    //! \param    [in] streamInWidth, top, bottom, left, right
    //!           StreamInWidth, dirtyRect corner locations
    //! \param    [out] streaminData
    //!           Pointer to streaminData
    //!
    //! \return   void
    //!
    virtual void StreaminSetBorderNon64AlignStaticRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        void* streaminData);

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
    virtual void SetStreaminDataPerLcu(
        PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
        void* streaminData)
    {
        return;
    }

    //!
    //! \brief    Add commands in input SLB buffer for HuC Initializer CQP
    //!
    //! \param    [in,out] batchBuffer
    //!           Pointer to SLB buffer which commands are added
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ConstructBatchBufferHuCCQP(PMOS_RESOURCE batchBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Add commands in input SLB buffer for HuC BRC (including ACQP)
    //!
    //! \param    [in,out] batchBuffer
    //!           Pointer to SLB buffer which commands are added
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Setup DMEM for HuC BRC init/reset
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemHuCBrcInitReset()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Setup constant data for HuC BRC update
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetConstDataHuCBrcUpdate()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Setup DMEM for HuC BRC update
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemHuCBrcUpdate()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Setup Virtual Address Regions for HuC BRC update
    //!
    //! \param    [in] virtualAddrParams
    //!           Huc Virtual Address parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetRegionsHuCBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams);

    // HCP/PAK functions

    //!
    //! \brief    Set VDENC_PIPE_MODE_SELECT parameters
    //!
    //! \param    [in, out] pipeModeSelectParams
    //!           Pipe mode select parameters
    //!
    //! \return   void
    //!
    virtual void SetVdencPipeModeSelectParams(
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams);

    //!
    //! \brief    Set VDENC_SRC_SURFACE_STATE, VDENC_REF_SURFACE_STATE and 
    //!           VDENC_DS_REF_SURFACE_STATE parameters
    //! 
    //! \param    [in, out] srcSurfaceParams
    //!           Source picture surface parameters
    //! \param    [in, out] reconSurfaceParams
    //!           Recon picture surface parameters
    //! \param    [in, out] ds8xSurfaceParams
    //!           8x Down scaled reference surface parameters
    //! \param    [in, out] ds4xSurfaceParams
    //!           4x Down scaled reference surface parameters
    //!
    //! \return   void
    //!
    virtual void SetVdencSurfaceStateParams(
        MHW_VDBOX_SURFACE_PARAMS& srcSurfaceParams,
        MHW_VDBOX_SURFACE_PARAMS& reconSurfaceParams,
        MHW_VDBOX_SURFACE_PARAMS& ds8xSurfaceParams,
        MHW_VDBOX_SURFACE_PARAMS& ds4xSurfaceParams);

    //!
    //! \brief    Set VDENC_PIPE_BUF_ADDR parameters
    //!
    //! \param    [in, out] PipeBufAddrParams
    //!           Pipe buf addr parameters
    //!
    //! \return   void
    //!
    virtual void SetVdencPipeBufAddrParams(
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& PipeBufAddrParams);

    //!
    //! \brief    Add HCP_WEIGHT_OFFSET_STATE command to command buffer 
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in] hevcSlcParams
    //!           Pointer to HEVC slice parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpWeightOffsetStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams);

    //!
    //! \brief    Add VDENC_WEIGHT_OFFSET_STATE command to command buffer 
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in] hevcSlcParams
    //!           Pointer to HEVC slice parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWeightOffsetStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams);

    //!
    //! \brief    Add VDENC_WALKER_STATE commands to command buffer 
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \param    [in] params
    //!           Pointer to MHW_VDBOX_HEVC_SLICE_STATE parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params);

    //!
    //! \brief    Read stats for BRC from PAK
    //!
    //! \param    [in] cmdBuffer
    //!            Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadBrcPakStats(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read stats from VDEnc and PAK for lookahead
    //!
    //! \param    [in] cmdBuffer
    //!            Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StoreLookaheadStatistics(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read stats from VDEnc for lookahead
    //!
    //! \param    [in] cmdBuffer
    //!            Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StoreVdencStatistics(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read slice size info from PAK
    //!
    //! \param    [in] cmdBuffer
    //!            Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadSliceSize(PMOS_COMMAND_BUFFER cmdBuffer);

    //! \brief    Copies a page aligned chunk of memory using HuC
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] Source/ Dest surfaces/ offsets page aligned
    //!            Pointer to surfaces
    //! \param    [in] copySize
    //!            Size in bytes of data to be copied
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CopyDataBlock(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE sourceSurface,
        uint32_t sourceOffset,
        PMOS_RESOURCE destSurface,
        uint32_t destOffset,
        uint32_t copySize);
    //!
    //! \brief    Get maximal number of slices allowed for specific LevelId
    //! 
    //! \param    [in] levelIdc
    //!           Level Id
    //!
    //! \return   Maximal number of slices allowed
    //!
    uint32_t GetMaxAllowedSlices(uint8_t levelIdc);

    //!
    //! \brief    Specify if VDEnc+PAK or Pak only pass is used
    //! 
    //! \return   void
    //!
    void SetPakPassType();

    //!
    //! \brief    Invoke HuC BRC init/reset
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HuCBrcInitReset();

    //!
    //! \brief    Invoke HuC BRC update
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HuCBrcUpdate();

    //!
    //! \brief    Use dummy stream object for HuC BRC FW.
    //!
    //! \param    [in] cmdBuffer
    //!           Command Buffer.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HuCBrcDummyStreamObject(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Computer QP initialization value used for BRC
    //!
    //! \param    [out] initQPIP
    //!           QP initialization value for I/P frame 
    //! \param    [out] initQPB
    //!           QP initialization value for B frame
    //!
    //! \return   void
    //!
    void ComputeVDEncInitQP(int32_t& initQPIP, int32_t& initQPB);

    //!
    //! \brief    Add store HUC_STATUS2 register command in the command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the command buffer 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreHuCStatus2Register(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Put slice level commands in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] params
    //!           Pointer to slice state parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendHwSliceEncodeCommand(PMOS_COMMAND_BUFFER cmdBuffer, PMHW_VDBOX_HEVC_SLICE_STATE params);

    //!
    //! \brief    Sort and set distinct delta QPs
    //!
    //! \return   void
    //!
    virtual void ProcessRoiDeltaQp();

    // Inherited virtual function
    MOS_STATUS Initialize(CodechalSetting * settings)  override;
    MOS_STATUS AllocatePakResources() override;
    MOS_STATUS FreePakResources() override;
    MOS_STATUS AllocateEncResources() override;
    MOS_STATUS FreeEncResources() override;
    MOS_STATUS AllocateBrcResources() override;
    MOS_STATUS FreeBrcResources() override;
    MOS_STATUS InitializePicture(const EncoderParams& params) override;
    MOS_STATUS SetSequenceStructs() override;
    MOS_STATUS SetPictureStructs() override;
    MOS_STATUS CalcScaledDimensions() override;
    MOS_STATUS ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams) override;
    MOS_STATUS ExecutePictureLevel() override;
    MOS_STATUS ExecuteSliceLevel() override;
    MOS_STATUS ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer) override;
    MOS_STATUS UserFeatureKeyReport() override;
    MOS_STATUS GetStatusReport(
        EncodeStatus *encodeStatus,
        EncodeStatusReport *encodeStatusReport) override;
    void SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams) override;
    MOS_STATUS AddHcpPakInsertSliceHeader(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE params) override;

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpHucBrcInit();
    virtual MOS_STATUS DumpHucBrcUpdate(bool isInput);
    virtual MOS_STATUS DumpVdencOutputs();
    virtual MOS_STATUS DumpSeqParFile() override;
    MOS_STATUS PopulateDdiParam(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS    hevcSlcParams) override;

    //!
    //! \brief  Modify the frame size with fake header size
    //!
    //! \param  [in] cmdBuffer
    //!         command buffer
    //! \param  [in] fakeHeaderSizeInByte
    //!         fake header size in bytes
    //! \param  [in] resBrcUpdateCurbe
    //!         Curebe/Dmem for brcupdate kernel
    //! \param  [in] targetSizePos
    //!         offset of targetSize in resBrcUpdateCurbe
    //! \param  [in] resPakStat
    //!         Pak stastics
    //! \param  [in] slcHrdSizePos
    //!         offset of slcHrdSizePos in resPakStat
    //!
    //! \return MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ModifyEncodedFrameSizeWithFakeHeaderSize(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        uint32_t                            fakeHeaderSizeInByte,
        PMOS_RESOURCE                       resBrcUpdateCurbe,
        uint32_t                            targetSizePos,
        PMOS_RESOURCE                       resPakStat,
        uint32_t                            slcHrdSizePos);
#endif

};

//! typedef of class CodechalVdencHevcState*
using PCODECHAL_VDENC_HEVC_STATE = class CodechalVdencHevcState*;

#endif  // __CODECHAL_VDENC_HEVC_H__
