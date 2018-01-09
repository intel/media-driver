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

/*
*  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

//! 
//! \file     codechal_vdenc_vp9_g10.h
//! \brief    VP9 VDENC encoder for GEN10 platform.
//! 

#ifndef __CODECHAL_VDENC_VP9_G10_H__
#define __CODECHAL_VDENC_VP9_G10_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_vdenc_vp9_base.h"

class CodechalVdencVp9StateG10: public CodechalVdencVp9State
{
public:

    struct KernelHeader
    {
        int nKernelCount;

        // DownScaling
        CODECHAL_KERNEL_HEADER PLY_DSCALE;

        // HME 
        CODECHAL_KERNEL_HEADER VP9_ME_P;

        // 32x32 I ENC KERNEL
        CODECHAL_KERNEL_HEADER VP9_Enc_I_32x32;

        // 16x16 I ENC KERNEL
        CODECHAL_KERNEL_HEADER VP9_Enc_I_16x16;

        // P ENC KERNEL
        CODECHAL_KERNEL_HEADER VP9_Enc_P;

        // TX ENC KERNEL
        CODECHAL_KERNEL_HEADER VP9_Enc_TX;

        // DYS KERNEL
        CODECHAL_KERNEL_HEADER VP9_DYS;
    };

    // VDENC BRC related buffer size
    static constexpr uint32_t m_brcStatsBufSize  = (48 * sizeof(uint32_t));
    static constexpr uint32_t m_brcPakStatsBufSize = (64 * sizeof(uint32_t));
    static constexpr uint32_t m_brcHistoryBufSize = 1152;

    //!
    //! \brief    Constructor
    //!
    CodechalVdencVp9StateG10(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencVp9StateG10() {};

    MOS_STATUS ExecuteKernelFunctions();

    MOS_STATUS SetupSegmentationStreamIn();

    bool CheckSupportedFormat(
        PMOS_SURFACE surface);

    MOS_STATUS Initialize(CodechalSetting * settings);

    //!
    //! \brief    Gets the kernel header/size from the kernel descriptor table
    //! \details
    //! \param    void* binary
    //!           [in] Pointer to the base of the combined kernel
    //! \param    CODECHAL_ENC_OPERATION operation
    //!           [in] Type of encode operation
    //! \param    uint32_t krnStateIdx
    //!           [in] Index within the ENC operation block of the kernel to be loaded
    //! \param    void* krnHeader
    //!           [in] Pointer to the CODECHAL_KERNEL_HEADER stucture to be returned to caller
    //! \param    uint32_t* krnSize
    //!           [in] Pointer to the kernel size to be returned to caller
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
     MOS_STATUS GetKernelHeaderAndSize(
        void*        binary,
        EncOperation operation,
        uint32_t     krnStateIdx,
        void*        krnHeader,
        uint32_t*    krnSize);

    static MOS_STATUS GetCommonKernelHeaderAndSize(
        void*        binary,
        EncOperation encOperation,
        uint32_t     krnStateIdx,
        void*        krnHeader,
        uint32_t*    krnSize);

    MOS_STATUS SetBindingTable(
        EncOperation encOperation,
        PCODECHAL_ENCODE_BINDING_TABLE_GENERIC encBindingTable);

    MOS_STATUS SetKernelParams(
        EncOperation encOperation,
        MHW_KERNEL_PARAM* kernelParams);

    MOS_STATUS InitKernelStateDys();

    MOS_STATUS InitKernelStateStreamin();

    MOS_STATUS InitKernelStateMe();

    MOS_STATUS InitKernelStates();

    uint32_t GetMaxBtCount();

    MOS_STATUS InitZigZagToRasterLUT(uint32_t height, uint32_t width);
};

#endif  // __CODECHAL_VDENC_VP9_G10_H__
