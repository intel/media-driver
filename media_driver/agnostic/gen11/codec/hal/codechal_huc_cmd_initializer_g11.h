/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_huc_cmd_initializer_g11.h
//! \brief    Defines class for G11 HUC cmd initializer encoder.
//!

#ifndef __CODECHAL_CMD_INITIALIZER_G11_H__
#define __CODECHAL_CMD_INITIALIZER_G11_H__

#include "codechal_huc_cmd_initializer.h"

#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
#include "codechal_encode_hevc_base.h"
#endif
#include "codechal_encoder_base.h"

//!
//! \struct HucInputCmd1
//! \brief  The struct of Huc input command 1
//!
struct HucInputCmd1G11
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
    uint8_t  tiles_enabled_flag;
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
    uint8_t  TransformSkip;
    uint32_t Intra32X32ModeMask;

    // VP9
    uint16_t SrcFrameWidthMinus1;
    uint16_t SrcFrameHeightMinus1;

    uint8_t  SegmentationEnabled;
    uint8_t  PrevFrameSegEnabled;
    uint8_t  SegMapStreamInEnabled;
    uint8_t  LumaACQIndex;

    int8_t   LumaDCQIndexDelta;
    uint8_t  log2_tile_columns;
    uint8_t  log2_tile_rows;
    uint8_t  DynamicScalingEnabled;

    int16_t  SegmentQIndexDelta[8];
    
    // for hevc max/min qp
    uint8_t  BRCMaxQp;
    uint8_t  BRCMinQp;

    // for VP9 VDEnc Gen11
    uint8_t  dysVdencMultiPassEnabled;

    uint8_t  Rsvd[1];
};

//!
//! \struct HucInputCmd3G11
//! \brief  The struct of Huc input command 3
//!
struct HucInputCmd3G11
{
    uint8_t  SelectionForIndData    = 0;
    uint8_t  CmdMode                = 0;
    uint16_t LengthOfTable          = 0;

    uint32_t SrcBaseOffset          = 0;
    uint32_t DestBaseOffset         = 0;

    uint32_t Reserved[3]            = { 0 };

    uint32_t CopySize               = 0;    // use this as indicator of size for copy base addr cmd. Since encode will not implement CopySize for copy cmd

    uint32_t ReservedCounter[4]     = {0};

    uint32_t SrcAddrBottom          = 0;
    uint32_t SrcAddrTop             = 0;
    uint32_t DestAddrBottom         = 0;
    uint32_t DestAddrTop            = 0;
};

//!
//! \struct HucCopyParams
//! \brief  The struct of params used for huc copy
//!
typedef struct
{
    PMOS_RESOURCE presSrc       = nullptr;
    PMOS_RESOURCE presDst       = nullptr;
    uint32_t      size          = 0;
    uint16_t      lengthOfTable = 0;
} HucCopyParams;

//!
//! \class  CodechalCmdInitializer
//! \brief  Command Initializer class
//!
class CodechalCmdInitializerG11 : public CodechalCmdInitializer
{
public:
    //!
    //! \brief    Constructor
    //!

    CodechalCmdInitializerG11(CodechalEncoderState *encoder);

    //!
    //! \brief   Default Constructor
    //!
    CodechalCmdInitializerG11() {};

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalCmdInitializerG11() {};

    //! Inherited virtual functions
    virtual MOS_STATUS CmdInitializerAllocateResources(
        CodechalHwInterface*    m_hwInterface) override;

    virtual MOS_STATUS CmdInitializerSetDmem(bool brcEnabled) override;

    //!
    //! \brief    Free Resources
    //!
    virtual void CmdInitializerFreeResources() override;

    //!
    //! \brief    Add copy commands of command initializer HuC FW
    //!
    //! \param    [in] cmdBuffer
    //!           Command Buffer
    //! \param    [in] params
    //!           Params used to add copy commands
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCopyCmds(
        PMOS_COMMAND_BUFFER cmdBuffer,
        HucCopyParams* params);

    //!
    //! \brief    Add command of initializer HuC FW
    //!
    //! \param    [in] cmdId
    //!           id of the command
    //! \param    [in] data
    //!           content of the command
    //! \param    [in] size
    //!           size of the command
    //! \param    [in] startOffset
    //!           start offset of the command in slb
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCmdConstData(
        uint16_t cmdId,
        uint32_t* data,
        uint16_t size,
        uint32_t startOffset
    );


#ifdef _VP9_ENCODE_VDENC_SUPPORTED
    //!
    //! \brief    Set DMEM of command initializer HuC FW for VP9
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CmdInitializerVp9SetDmem() override;

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
    MOS_STATUS CmdInitializerVp9Execute(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE picStateBuffer) override;
#endif

    bool m_hevcVisualQualityImprovement = false;            //!< VQI enable flag

protected:
#if defined (_HEVC_ENCODE_VME_SUPPORTED) || defined (_HEVC_ENCODE_VDENC_SUPPORTED)
    MOS_STATUS ConstructHevcHucCmd1ConstData(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
        struct HucComData *                hucConstData) override;
        
    MOS_STATUS ConstructHevcHucCmd2ConstData(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS  picParams,
        PCODEC_HEVC_ENCODE_SLICE_PARAMS    sliceParams,
        struct HucComData *                hucConstData) override;

    uint16_t GetCmd2StartOffset(bool brcEnabled) override;
    uint16_t GetCmd1StartOffset(bool brcEnabled) override;
    virtual uint16_t GetCmd5StartOffset(bool brcEnabled);
#endif  

    MOS_RESOURCE    m_cmdInitializerCopyDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];      //!< Dmem buffer for huc cmd initialization copy
    MOS_RESOURCE    m_cmdInitializerCopyDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];      //!< Data buffer for huc cmd initialization copy
    MHW_BATCH_BUFFER    m_vdencCopyBatchBuffer = {};             //!< SLB for huc cmd initialization copy

    //!
    //! \brief    Set dmem buffer for huc cmd initialization copy
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCopyDmem();

    //!
    //! \brief    Set data buffer for huc cmd initialization copy
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCopyData(
        HucCopyParams* params);
};
using PCODECHAL_CMD_INITIALIZER_G11 = class CodechalCmdInitializerG11*;

#endif  //__CODECHAL_CMD_INITIALIZER_G11_H__
