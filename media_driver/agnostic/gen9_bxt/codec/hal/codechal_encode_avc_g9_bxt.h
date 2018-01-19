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
//! \file     codechal_encode_avc_g9_bxt.h
//! \brief    AVC dual-pipe encoder for GEN9 BXT.
//!

#ifndef __CODECHAL_ENCODE_AVC_G9_BXT_H__
#define __CODECHAL_ENCODE_AVC_G9_BXT_H__

#include "codechal_encode_avc.h"
#include "codechal_encode_avc_g9.h"

#define MBENC_CURBE_SIZE_IN_DWORD_G9_BXT    104

class CodechalEncodeAvcEncG9Bxt : public CodechalEncodeAvcEncG9
{
public:

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
    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *pvBinary,
        EncOperation                   operation,
        uint32_t                       dwKrnStateIdx,
        void                           *pvKrnHeader,
        uint32_t                       *pdwKrnSize);

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG9Bxt(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAvcEncG9Bxt() {};

    //!
    //! \brief    Set MbEnc kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS pParams);

    //!
    //! \brief    Get Trellis Quantization mode/value enable or not.
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS.
    //! \param    [out] pTrellisQuantParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_PARAMS, mode & value setup.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    pParams,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          pTrellisQuantParams);

    //!
    //! \brief    Initialize brc constant buffer
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        pParams);

    // state related funcs
    //!
    //! \brief    Initialize encode state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializeState();

    //!
    //! \brief    Init MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Init BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateBrc();

    virtual MOS_STATUS InitKernelStateMfeMbEnc();

    virtual MOS_STATUS InitMfe();

    virtual MOS_STATUS SetCurbeAvcMfeMbEnc(
        PCODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS pParams);

    virtual MOS_STATUS UpdateMfeMbEncBindingTable(
        uint32_t submitIndex);

    //!
    //! \brief    initial multi ref Qp BRC constant Buffer.
    //!
    //! \param    [in] pParams
    //!           BRC init constant buffer params.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS InitBrcConstantBufferMultiRefQP(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS pParams);

    void UpdateSSDSliceCount();

private:
    static const uint32_t TrellisQuantizationEnable[NUM_TARGET_USAGE_MODES];
    static const uint32_t TrellisQuantizationRounding[NUM_TARGET_USAGE_MODES];
    static const uint32_t EnableAdaptiveTrellisQuantization[NUM_TARGET_USAGE_MODES];
    static const uint32_t MBEnc_CURBE_normal_I_frame[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const uint32_t MBEnc_CURBE_normal_I_field[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const uint32_t MBEnc_CURBE_normal_P_frame[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const uint32_t MBEnc_CURBE_normal_P_field[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const uint32_t MBEnc_CURBE_normal_B_frame[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const uint32_t MBEnc_CURBE_normal_B_field[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const uint32_t MBEnc_CURBE_I_frame_DIST[MBENC_CURBE_SIZE_IN_DWORD_G9_BXT];
    static const int32_t BRC_BTCOUNTS[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const int32_t BRC_CURBE_SIZE[CODECHAL_ENCODE_BRC_IDX_NUM];

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer);
#endif
};

#endif  // __CODECHAL_ENCODE_AVC_G9_BXT_H__
