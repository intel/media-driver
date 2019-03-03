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
//! \file     codechal_vdenc_avc_g9_bxt.h
//! \brief    This file defines the base C++ class/interface for BXT AVC VDENC 
//!           encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_AVC_G9_BXT_H__
#define __CODECHAL_VDENC_AVC_G9_BXT_H__

#include "codechal_vdenc_avc_g9.h"

class CodechalVdencAvcStateG9Bxt : public CodechalVdencAvcStateG9
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalVdencAvcStateG9Bxt(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalVdencAvcStateG9Bxt();

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] pvBinary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] dwKrnStateIdx
    //!           Kernel state index
    //! \param    [out] pvKrnHeader
    //!           Pointer to kernel header
    //! \param    [out] pdwKrnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS EncodeGetKernelHeaderAndSize(
        void                           *pvBinary,
        EncOperation                   operation,
        uint32_t                       dwKrnStateIdx,
        void                           *pvKrnHeader,
        uint32_t                       *pdwKrnSize);

    // Inherited virtual functions
    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS pParams,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS       pTrellisQuantParams);

    virtual MOS_STATUS SetDmemHuCBrcInitReset();

    virtual MOS_STATUS SetDmemHuCBrcUpdate();

    virtual MOS_STATUS LoadMvCost(
        uint8_t QP);

    virtual MOS_STATUS LoadHmeMvCost(
        uint8_t QP);

    virtual MOS_STATUS LoadHmeMvCostTable(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS   pSeqParams,
        uint8_t                               HMEMVCostTable[8][42]);

private:
    static const uint32_t MV_Cost_SkipBias_QPel[8];
    static const uint32_t HmeCost[8][CODEC_AVC_NUM_QP];

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS PopulateBrcInitParam(
        void *cmd);

    virtual MOS_STATUS PopulateBrcUpdateParam(
        void *cmd);

    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd);

    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer);
#endif
};

#endif  // __CODECHAL_VDENC_AVC_G9_BXT_H__
