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
//! \file     codechal_encode_vp8_g10.h
//! \brief    VP8 dual-pipe encoder for GEN10 platform.
//!

#ifndef __CODECHAL_ENCODE_VP8_G10_H__
#define __CODECHAL_ENCODE_VP8_G10_H__

#include "codechal_encode_vp8.h"

class CodechalEncodeVp8G10 : public CodechalEncodeVp8
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeVp8G10(
        CodechalHwInterface*    hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeVp8G10() {};

protected:
    MOS_STATUS Initialize(CodechalSetting * codecHalSettings) override;

    MOS_STATUS InitKernelStateHelper(struct CodechalEncodeVp8InitKernelStateParams* params);

    MOS_STATUS InitKernelState() override;

    //!
    //! \brief    Initialize kernel state for ME
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMe();

    //!
    //! \brief    Initialize kernel state for MBENC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Initialize kernel state for BRC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateBrc();

    //!
    //! \brief    Initialize kernel state for MPU
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMpu();

    //!
    //! \brief    Initialize kernel state for TPU
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateTpu();

    MOS_STATUS InitMpuTpuBuffer() override;

    //!
    //! \brief    Calculate Max Level Ratio For Temporal Layers
    //!
    //! \param    [in] framesPer100Sec
    //!           Pointer to uint32_t
    //! \param    [in] targetBitRate
    //!           Pointer to uint32_t
    //! \param    [in] numTemporalLayersMinus1
    //!           Temporal layers minus1 number
    //! \param    [in] tempBitRate
    //!           Pointer to uint32_t
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalMaxLevelRatioForTL(
        uint16_t *framesPer100Sec,
        uint32_t *targetBitRate,
        uint32_t numTemporalLayersMinus1,
        uint32_t *tempBitRate);

    MOS_STATUS InitBrcConstantBuffer(struct CodechalVp8InitBrcConstantBufferParams*  params) override;

    MOS_STATUS InitBrcDistortionBuffer() override;

    MOS_STATUS KeyFrameUpdateMpuTpuBuffer(struct CodechalVp8UpdateMpuTpuBufferParams* params) override;

    MOS_STATUS InitMBEncConstantBuffer(
        struct CodechalVp8InitMbencConstantBufferParams*  params) override;

    MOS_STATUS SetBrcInitResetCurbe(struct CodechalVp8BrcInitResetCurbeParams* params) override;

    MOS_STATUS SetBrcUpdateCurbe(struct CodechalVp8BrcUpdateCurbeParams*   params) override;

    MOS_STATUS SetMbEncCurbe(struct CodechalVp8MbencCurbeParams* params) override;

    MOS_STATUS SetMeCurbe(struct CodechalVp8MeCurbeParams* params) override;

    MOS_STATUS SetMpuCurbe(struct CodechalVp8MpuCurbeParams* params) override;

    MOS_STATUS SetTpuCurbe(struct CodechalVp8TpuCurbeParams* params) override;

    MOS_STATUS SendBrcInitResetSurfaces(
          PMOS_COMMAND_BUFFER                         cmdBuffer,
          struct CodechalVp8BrcInitResetSurfaceParams* params) override;

    MOS_STATUS SendMpuSurfaces(
          PMOS_COMMAND_BUFFER                 cmdBuffer,
          struct CodechalVp8MpuSurfaceParams* params) override;

    //!
    //! \brief    Get Header and Size of Kernel
    //!
    //! \param    [in] binary
    //!           Pointer to void
    //! \param    [in] operation
    //!           Type of encode operation
    //! \param    [in] krnStateIdx
    //!           Index of Kernel State
    //! \param    [in] krnHeader
    //!           Pointer to void
    //! \param    [in] krnSize
    //!           Pointer to the kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                                *binary,
        EncOperation                        operation,
        uint32_t                            krnStateIdx,
        void                                *krnHeader,
        uint32_t                            *krnSize);
};

#endif  //__CODECHAL_ENC_VP8_MODS_G10_H__
