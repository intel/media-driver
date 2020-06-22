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
//! \file     codechal_vdenc_avc_g10.h
//! \brief    This file defines the base C++ class/interface for G10 AVC VDENC 
//!           encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_AVC_G10_H__
#define __CODECHAL_VDENC_AVC_G10_H__

#include "codechal_vdenc_avc.h"

class CodechalVdencAvcStateG10 : public CodechalVdencAvcState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalVdencAvcStateG10(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencAvcStateG10();

    //!
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

    bool CheckSupportedFormat( PMOS_SURFACE surface) override;

    // Inherited virtual functions
    MOS_STATUS InitializeState() override;

    MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams) override;

    MOS_STATUS SetDmemHuCBrcInitReset() override;

    MOS_STATUS SetDmemHuCBrcUpdate() override;

    MOS_STATUS LoadMvCost( uint8_t qp) override;

    MOS_STATUS LoadHmeMvCost(uint8_t qp) override;

    MOS_STATUS LoadHmeMvCostTable(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
        uint8_t hmeMvCostTable[8][42]) override;

private:
    static const uint32_t m_mvCostSkipBiasQPel[3][8];
    static const uint32_t m_hmeCostDisplayRemote[8][CODEC_AVC_NUM_QP];
    static const uint32_t m_hmeCost[8][CODEC_AVC_NUM_QP];
    static const int8_t   m_brcInitDistQpDeltaI8[4];
    static const int8_t   m_brcInitDistQpDeltaI8LowDelay[4];
    static const uint32_t m_singlePassMinFrameWidth = 3840;
    static const uint32_t m_singlePassMinFrameHeight = 2160;
    static const uint32_t m_singlePassMinFramePer100s = 6000;

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

using PCodechalVdencAvcStateG10 = CodechalVdencAvcStateG10*;

#endif  // __CODECHAL_VDENC_AVC_G10_H__
