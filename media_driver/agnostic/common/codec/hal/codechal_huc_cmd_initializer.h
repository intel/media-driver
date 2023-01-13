/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_huc_cmd_initializer.h
//! \brief    Defines base class for HUC cmd initializer encoder.
//!

#ifndef __CODECHAL_CMD_INITIALIZER_H__
#define __CODECHAL_CMD_INITIALIZER_H__

#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
#include "codechal_encode_hevc_base.h"
#endif
#include "codechal_encoder_base.h"

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
class CodechalVdencVp9State;
#endif

// Command initializer command
typedef enum _CODECHAL_CMD_INITIALIZER_CMDTYPE
{
    CODECHAL_CMD1 = 1,
    CODECHAL_CMD2 = 2,
    CODECHAL_CMD3 = 3,
    CODECHAL_CMD5 = 5,
} CODECHAL_CMD_INITIALIZER_CMDTYPE;

//!
//! \struct HucComDmem
//! \brief  The struct of Huc Com Dmem
//!
struct HucComDmem
{
    uint32_t    OutputSize;               //!< Total size in byte of the Output SLB
    uint32_t    TotalOutputCommands;      //!< Total Commands in the output SLB
    uint8_t     TargetUsage;              //!< TU number
    uint8_t     Codec;                    //!< 0-HEVC VDEnc; 1-VP9 VDEnc; 2-AVC VDEnc
    uint8_t     FrameType;                //!< 0-I Frame; 1-P Frame; 2-B Frame
    uint8_t     Reserved[37];
    struct
    {
        uint16_t    StartInBytes;        //!< Command starts offset in bytes in Output SLB
        uint8_t     ID;                  //!< Command ID
        uint8_t     Type;                //!< Command Type
        uint32_t    BBEnd;
    } OutputCOM[50];
};

//!
//! \struct HucComData
//! \brief  The struct of Huc commands data
//!
struct HucComData
{
    uint32_t        TotalCommands;       //!< Total Commands in the Data buffer
    struct
    {
        uint16_t    ID;              //!< Command ID, defined and order must be same as that in DMEM
        uint16_t    SizeOfData;      //!< data size in uint32_t
        uint32_t    data[40];
    } InputCOM[50];
};

//!
//! \struct HucInputCmd1
//! \brief  The struct of Huc input command 1
//!
struct HucInputCmd1
{
    // Shared
    uint32_t FrameWidthInMinCbMinus1;

    uint32_t FrameHeightInMinCbMinus1;

    uint32_t log2_min_coding_block_size_minus3;

    uint8_t  VdencStreamInEnabled;
    uint8_t  PakOnlyMultipassEnable;
    uint16_t num_ref_idx_l0_active_minus1;

    uint16_t SADQPLambda;
    uint16_t RDQPLambda;

    // HEVC
    uint16_t num_ref_idx_l1_active_minus1;
    uint8_t  RSVD0;
    uint8_t  ROIStreamInEnabled;

    int8_t   ROIDeltaQp[8]; // [-3..3] or [-51..51]

    uint8_t  FwdPocNumForRefId0inL0;
    uint8_t  FwdPocNumForRefId0inL1;
    uint8_t  FwdPocNumForRefId1inL0;
    uint8_t  FwdPocNumForRefId1inL1;

    uint8_t  FwdPocNumForRefId2inL0;
    uint8_t  FwdPocNumForRefId2inL1;
    uint8_t  FwdPocNumForRefId3inL0;
    uint8_t  FwdPocNumForRefId3inL1;

    uint8_t  EnableRollingIntraRefresh;
    int8_t   QpDeltaForInsertedIntra;
    uint16_t IntraInsertionSize;

    uint32_t  IntraInsertionReferenceLocation[3];

    uint16_t IntraInsertionLocation;
    int8_t   QpY;
    uint8_t  RoundingEnabled;

    uint8_t  UseDefaultQpDeltas;
    uint8_t  PanicEnabled;
    uint8_t  TemporalMvpEnableFlag;
    uint8_t  RSVD[1];

    // VP9
    uint16_t SrcFrameWidthMinus1;
    uint16_t SrcFrameHeightMinus1;

    uint8_t  SegmentationEnabled;
    uint8_t  PrevFrameSegEnabled;
    uint8_t  SegMapStreamInEnabled;
    uint8_t  LumaACQIndex;

    int8_t   LumaDCQIndexDelta;
    uint8_t  RESERVED[3];

    int16_t  SegmentQIndexDelta[8];
};

//!
//! \struct HucInputCmd2
//! \brief  The struct of Huc input command 2
//!
struct HucInputCmd2
{
    uint32_t SADQPLambda;
    uint8_t RoiEnabled;
    uint8_t RSVD[3];
};

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
//!
//! \struct Vp9CmdInitializerParams
//! \brief  VP9 Params struct for huc initializer
//!
struct Vp9CmdInitializerParams
{
    uint8_t                             vdencMvCosts[12] = { 0 };
    uint8_t                             vdencRdMvCosts[12] = { 0 };
    uint8_t                             vdencHmeMvCosts[8] = { 0 };
    uint8_t                             vdencModeCosts[CODEC_VDENC_NUM_MODE_COST] = { 0 };

    uint16_t                            pictureCodingType   = 0;

    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS   seqParams           = nullptr;
    PCODEC_VP9_ENCODE_PIC_PARAMS        picParams           = nullptr;
    bool                                segmentationEnabled = false;
    bool                                segmentMapProvided  = false;
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS    segmentParams       = nullptr;
    bool                                prevFrameSegEnabled = false;
    uint8_t                             numRefFrames        = 0;
    bool                                me16Enabled         = false;
    uint8_t                             dysRefFrameFlags    = 0;
    bool                                dysVdencMultiPassEnabled    = false;
    int                                 currentPass                 = 0;
    bool                                singleTaskPhaseSupported    = false;
    bool                                lastTaskInPhase             = false;
    bool                                firstTaskInPhase            = false;
    uint32_t                            mode                        = 0;
    bool                                videoContextUsesNullHw      = false;
    CodechalDebugInterface*             debugInterface              = nullptr;
    bool                                dynamicScalingEnabled       = false;
    bool                                bPrevFrameKey               = false;
    // Common
    uint16_t                            sadQpLambda                     = 0;
    uint16_t                            rdQpLambda                      = 0;
    bool                                vdencPakOnlyMultipassEnabled    = false;

};
#endif

//!
//! \class  CodechalCmdInitializer
//! \brief  Command Initializer class
//!
class CodechalCmdInitializer
{
public:
    static constexpr uint32_t m_hucCmdInitializerKernelDescriptor = 14; //!< VDBox Huc cmd initializer kernel descriptoer

    bool                                        m_pakOnlyPass = false;
    bool                                        m_acqpEnabled = false;
    bool                                        m_brcEnabled = false;
    bool                                        m_streamInEnabled = false;
    bool                                        m_roundingEnabled = false;
    bool                                        m_panicEnabled = false;
    bool                                        m_roiStreamInEnabled = false;
    bool                                        m_brcAdaptiveRegionBoostEnabled = false;
    int32_t                                     m_currentPass = 0;
    int32_t                                     m_cmdCount = 0 ;

    CodechalEncoderState                        *m_encoder = nullptr;                //!< Pointer to ENCODER base class
    MOS_INTERFACE                               *m_osInterface = nullptr;            //!< OS interface
    MhwMiInterface                              *m_miInterface = nullptr;            //!< Common Mi Interface
    MhwVdboxVdencInterface                      *m_vdencInterface = nullptr;
    void*                                       m_picParams = nullptr;               //!< Pointer to picture parameter
    void*                                       m_seqParams = nullptr;               //!< Pointer to sequence parameter
    void*                                       m_sliceParams = nullptr;             //!< Pointer to slice parameter

#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    //VP9 related changes
    Vp9CmdInitializerParams                     m_vp9Params;
#endif
    CodechalHwInterface*                        m_hwInterface = nullptr;
    MOS_RESOURCE                                m_cmdInitializerDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][3];
    MOS_RESOURCE                                m_cmdInitializerDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][3];
    MOS_RESOURCE                                m_cmdInitializerDysScalingDmemBuffer = {0};
    MOS_RESOURCE                                m_cmdInitializerDysScalingDataBuffer = {0};

    static constexpr uint32_t CODECHAL_CMD1_SIZE = 120;
    static constexpr uint32_t CODECHAL_CMD2_SIZE = 148;

    bool m_hevcVisualQualityImprovementEnableFlag = false;  //!< VQI enable flag

public:
    //!
    //! \brief    Constructor
    //!

    CodechalCmdInitializer(CodechalEncoderState *encoder);

    //!
    //! \brief   Default Constructor
    //!
    CodechalCmdInitializer() {};
    //!

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalCmdInitializer() {};

    //!
    //! \brief    Free Resources
    //!
    virtual void CmdInitializerFreeResources();

    //!
    //! \brief    Allocate resources for VP9
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CmdInitializerAllocateResources(CodechalHwInterface*    m_hwInterface);

#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
    //!
    //! \brief    Set all the data of the InputCom of command initializer HuC FW
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CmdInitializerSetConstData(
        PMOS_INTERFACE                              osInterface,
        MhwMiInterface                              *miInterface,
        MhwVdboxVdencInterface                      *vdencInterface,
        void*                                       seqParams,
        void*                                       picParams,
        void*                                       sliceParams,
        bool                                        pakOnlyPass,
        bool                                        acqpEnabled,
        bool                                        brcEnabled,
        bool                                        streaminEnabled,
        bool                                        roiStreamInEnabled,
        bool                                        brcAdaptiveRegionBoostEnable,
        bool                                        roundingEnabled,
        bool                                        panicEnabled,
        int32_t                                     currentPass
        );

    //!
    //! \brief    Set DMEM of command initializer HuC FW
    //!
    //! \param    [in] brcEnabled
    //!           Indicate if brc is enabled
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CmdInitializerSetDmem(bool brcEnabled);

    //!
    //! \brief    Executes command initializer HuC FW
    //!
    //! \param    [in] brcEnabled
    //!           Indicate if brc is enabled
    //! \param    [in] secondlevelBB
    //!           Second level batch buffer
    //! \param    [in] cmdBuffer
    //!           cmdBuffer provided by outside
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CmdInitializerExecute(
            bool brcEnabled, 
            PMOS_RESOURCE secondlevelBB, 
            MOS_COMMAND_BUFFER* cmdBuffer = nullptr);
#endif
    //!
    //! \brief    Set Add Commands to BatchBuffer
    //!
    //! \param    [in] commandtype
    //!           Command type, type 1 or type 2 command
    //! \param    [in] cmdBuffer
    //!           Command buffer
    //! \param    [in] addToBatchBufferHuCBRC
    //!           Flag to mention if the scenario is BRC or CQP
    //! \param    [in] isLowDelayB
    //!           Flag to indicate if it is LDB or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetAddCommands(uint32_t commandtype, PMOS_COMMAND_BUFFER cmdBuffer, bool addToBatchBufferHuCBRC, uint32_t roundInterValue, uint32_t roundIntraValue, bool isLowDelayB = true, int8_t * pRefIdxMapping = nullptr, int8_t recNotFilteredID = 0)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        return eStatus;
    }
#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    //!
    //! \brief    Set all const VP9 data of the InputCom of command initializer HuC FW
    //!
    //! \param    [in] state
    //!           Pointer to CodechalVdencVp9State
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CommandInitializerSetVp9Params(CodechalVdencVp9State *state);

    //!
    //! \brief    Set DMEM of command initializer HuC FW for VP9
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CmdInitializerVp9SetDmem();

    //!
    //! \brief    Executes VP9 command initializer HuC FW
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer
    //! \param    [in] picStateBuffer
    //!           Picture state buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CmdInitializerVp9Execute(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE picStateBuffer);

    //!
    //! \brief    Set Add Commands to BatchBuffer for VP9
    //!
    //! \param    [in] commandtype
    //!           Command type, type 1 or type 2 command
    //!           [in] cmdBuffer
    //!           Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCommandsVp9(uint32_t commandtype, PMOS_COMMAND_BUFFER cmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }
#endif
#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief    Dump HuC Cmd Initializer
    //!
    //! \param    [in] secondlevelBB
    //!           Kernel output commands is stored in secondlevelBB
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpHucCmdInit(PMOS_RESOURCE secondlevelBB);
#endif

protected:
#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
    virtual MOS_STATUS ConstructHevcHucCmd1ConstData(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
        struct HucComData *                hucConstData);

    virtual MOS_STATUS ConstructHevcHucCmd2ConstData(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
        struct HucComData *                hucConstData);

    virtual uint16_t GetCmd1StartOffset(bool brcEnabled);
    virtual uint16_t GetCmd2StartOffset(bool brcEnabled);
#endif
};
using PCODECHAL_CMD_INITIALIZER = class CodechalCmdInitializer*;

#endif  //__CODECHAL_CMD_INITIALIZER_H__
