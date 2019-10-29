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
//! \file     codechal_encode_avc_g11.h
//! \brief    This file defines the C++ class/interface for Gen11 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODE_AVC_G11_H__
#define __CODECHAL_ENCODE_AVC_G11_H__

#include "codechal_encode_avc.h"
#include "codechal_kernel_intra_dist.h"
#include "codechal_encode_sw_scoreboard_g11.h"
#include "codechal_encode_singlepipe_virtualengine.h"

class CodechalEncodeAvcEncG11 : public CodechalEncodeAvcEnc
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG11(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeAvcEncG11(const CodechalEncodeAvcEncG11&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeAvcEncG11& operator=(const CodechalEncodeAvcEncG11&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAvcEncG11();


    //!
    //! \brief    Encode frame in slice level.
    //! \details  Call related encode functions to encode
    //!           one frame in slice level.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ExecuteSliceLevel() override;

    //!
    //! \brief    Encoder related function
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteKernelFunctions( ) override;

    virtual MOS_STATUS SceneChangeReport(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   params) override;

    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] binary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] krnHeader
    //!           Pointer to kernel header
    //! \param    [out] krnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *binary,
        EncOperation                   operation,
        uint32_t                       krnStateIdx,
        void                           *krnHeader,
        uint32_t                       *krnSize);

    //!
    //! \brief    Get Trellis Quantization mode/value enable or not.
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS.
    //! \param    [out] trellisQuantParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_PARAMS, mode & value setup.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams) override;

    // state related funcs
    //!
    //! \brief    Initialize encode state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializeState() override;

    //!
    //! \brief    Init MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMbEnc() override;

    //!
    //! \brief    Init BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateBrc() override;

    //!
    //! \brief    Initialize brc constant buffer
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params) override;

    //!
    //! \brief    Initialize MB brc constant buffer
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMbBrcConstantDataBuffer(
        PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params) override;

    //!
    //! \brief    Get MBEnc kernel state index
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_ID_OFFSET_PARAM
    //!
    //! \param    [out] kernelOffset
    //!           The required kernel offset.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams*   params,
        uint32_t* kernelOffset) override;
    //!
    //! \brief    Set MbEnc kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params) override;

    //!
    //! \brief    Set up BrcInit curbe
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params) override;
    //!
    //! \brief    Set up frameBrcUpdate kernel curbe
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params) override;
    //!
    //! \brief    Set up MbBrcUpdate curbe
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params) override;
    //!
    //! \brief    Set up BrcBlockCopy kernel curbe
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcBlockCopy(
        PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params) override;
    //!
    //! \brief    Encode User Feature Key Report.
    //! \details  Report user feature values set by encode.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS UserFeatureKeyReport() override;
    //!
    //! \brief    Set up surfaces for MbEnc kernel
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params) override;

    //!
    //! \brief    Set up surfaces for BrcFrameUpdate kernel
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params) override;
    //!
    //! \brief    Set up surfaces for BrcMbUpdate kernel
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcBrcMbUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params) override;
    //!
    //! \brief    Set up ROI surfaces
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupROISurface() override;

    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr) override;

    //!
    //! \brief    Run MbEnc Kernel.
    //!
    //! \param    [in] mbEncIFrameDistInUse
    //!           MbEncIFrameDist in use or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MbEncKernel(
        bool mbEncIFrameDistInUse) override;

    //!
    //! \brief    Resize buffers due to resoluton change.
    //! \details  Resize buffers due to resoluton change.
    //!
    //! \return   void
    //!
    virtual void ResizeOnResChange() override;

    virtual MOS_STATUS InitKernelStateMe() override;

    virtual MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse) override;

    //!
    //! \brief    Set And Populate VE Hint parameters
    //! \details  Set Virtual Engine hint parameter and populate it to primary cmd buffer attributes
    //! \param    [in] cmdBuffer
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetAndPopulateVEHintParams(
        PMOS_COMMAND_BUFFER  cmdBuffer);

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetGpuCtxCreatOption() override;

    PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS PopulateBrcInitParam(
        void *cmd) override;

    virtual MOS_STATUS PopulateBrcUpdateParam(
        void *cmd) override;

    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd) override;

    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer) override;
#endif

private:
    static const uint32_t m_IntraModeCostForHighTextureMB[CODEC_AVC_NUM_QP];
    static const uint8_t m_QPAdjustmentDistThresholdMaxFrameThresholdIPB[576];
    static const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD m_IPCMThresholdTable[5];
    static const int32_t m_brcBTCounts[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const int32_t m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const uint16_t m_LambdaData[256];
    static const uint8_t m_FTQ25[64];
    static const uint16_t m_refCostMultiRefQp[NUM_PIC_TYPES][64];
    static const uint32_t m_multiPred[NUM_TARGET_USAGE_MODES];
    static const uint32_t m_MRDisableQPCheck[NUM_TARGET_USAGE_MODES];

    CodechalKernelIntraDist    *m_intraDistKernel = nullptr;
    CodechalEncodeSwScoreboard *m_swScoreboardState = nullptr;    //!< pointer to SW scoreboard init state.
};

#endif  // __CODECHAL_ENCODE_AVC_G11_H__
