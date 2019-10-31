/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_vdenc_avc_g12.h
//! \brief    This file defines the base C++ class/interface for G12 AVC VDENC
//!           encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_AVC_G12_H__
#define __CODECHAL_VDENC_AVC_G12_H__

#include "codechal_vdenc_avc.h"
#include "codechal_encode_singlepipe_virtualengine.h"

class CodechalVdencAvcStateG12 : public CodechalVdencAvcState
{
   public:

    PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state
    //!
    //! \brief    Constructor
    //!
    CodechalVdencAvcStateG12(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencAvcStateG12();

    bool CheckSupportedFormat(PMOS_SURFACE surface) override;

    MOS_STATUS InitializeState() override;

    MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS       trellisQuantParams) override;

    MOS_STATUS HuCBrcDummyStreamObject(PMOS_COMMAND_BUFFER cmdBuffer) override { return MOS_STATUS_SUCCESS; }

    MOS_STATUS SetDmemHuCBrcInitReset() override;

    MOS_STATUS SetDmemHuCBrcUpdate() override;

    MOS_STATUS LoadMvCost(uint8_t qp) override;

    MOS_STATUS LoadHmeMvCost(uint8_t qp) override;

    MOS_STATUS LoadHmeMvCostTable(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
        uint8_t                           hmeMvCostTable[8][42]) override;

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        int32_t             nullRendering) override;

    MOS_STATUS InitKernelStateSFD() override;

    MOS_STATUS Initialize(CodechalSetting * settings) override;

    MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer) override;

    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER               cmdBuffer,
        bool                              frameTracking,
        MHW_MI_MMIOREGISTERS             *mmioRegister = nullptr) override;

    MOS_STATUS InitMmcState() override;

    //!
    //! \brief    Create MHW_VDBOX_STATE_CMDSIZE_PARAMS
    //!
    //! \return   PMHW_VDBOX_STATE_CMDSIZE_PARAMS
    //!
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS CreateMhwVdboxStateCmdsizeParams() override;

    //!
    //! \brief    Create PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS.
    //!
    //! \return   PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS
    //!
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CreateMhwVdboxPipeModeSelectParams() override;

    //!
    //! \brief    Create PMHW_VDBOX_AVC_IMG_PARAMS.
    //!
    //! \return   PMHW_VDBOX_AVC_IMG_PARAMS
    //!
    PMHW_VDBOX_AVC_IMG_PARAMS CreateMhwVdboxAvcImgParams() override;

    //!
    //! \brief    Create PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS.
    //!
    //! \return   PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS
    //!
    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS CreateMhwVdboxVdencWalkerStateParams() override;

    MOS_STATUS InitKernelStateMe() override;

    MOS_STATUS ExecuteMeKernel() override;

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse) override;

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params) override;

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetGpuCtxCreatOption() override;
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
    //! \brief    Encode User Feature Key Report.
    //! \details  Report user feature values set by encode.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS UserFeatureKeyReport() override;

   protected:
    class SfdCurbe;
    bool                  m_vdencUltraModeEnable = false;   //!< Enable VDEnc ultra mode

    MOS_STATUS CalculateVdencPictureStateCommandSize() override;
    virtual void SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS& param) override;

   private:
    static const uint32_t m_mvCostSkipBiasQPel[3][8];
    static const uint32_t m_hmeCostDisplayRemote[8][CODEC_AVC_NUM_QP];
    static const uint32_t m_hmeCost[8][CODEC_AVC_NUM_QP];
    static const int8_t   m_brcInitDistQpDeltaI8[4];
    static const int8_t   m_brcInitDistQpDeltaI8LowDelay[4];
    static const uint32_t m_singlePassMinFrameWidth   = 3840;
    static const uint32_t m_singlePassMinFrameHeight  = 2160;
    static const uint32_t m_singlePassMinFramePer100s = 6000;

protected:

    void SetMfxPipeModeSelectParams(
        const CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS& genericParam,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& param) override;

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
};

#endif  // __CODECHAL_VDENC_AVC_G12_H__
