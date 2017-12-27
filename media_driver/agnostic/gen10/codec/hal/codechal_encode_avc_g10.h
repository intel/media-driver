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
//! \file     codechal_encode_avc_g10.h
//! \brief    This file defines the C++ class/interface for Gen10 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODE_AVC_G10_H__
#define __CODECHAL_ENCODE_AVC_G10_H__

#include "codechal_encode_avc.h"

class CodechalEncodeAvcEncG10 : public CodechalEncodeAvcEnc
{
public:

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
        void                         *binary,
        EncOperation                 operation,
		uint32_t                     krnStateIdx,
        void                         *krnHeader,
		uint32_t                     *krnSize);

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG10(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAvcEncG10();


    MOS_STATUS ExecuteKernelFunctions();

    MOS_STATUS InitializeState();

    MOS_STATUS InitKernelStateMbEnc();

    MOS_STATUS SetCurbeAvcMbEnc(PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params);

    MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params);

    MOS_STATUS InitBrcConstantBuffer(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS params);

    MOS_STATUS InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params);

    MOS_STATUS InitKernelStateBrc();

    MOS_STATUS SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params);

    MOS_STATUS SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params);

    MOS_STATUS SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params);

    MOS_STATUS SetCurbeAvcBrcBlockCopy(PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params);

    MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params);

    MOS_STATUS SendAvcBrcMbUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params);

    MOS_STATUS InitKernelStateWP();

    MOS_STATUS SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params);

    MOS_STATUS SendAvcWPSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params);

    MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams);

    MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams     *params,
        uint32_t                         *kernelOffset);

    MOS_STATUS SetupROISurface();

protected:

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS KernelDebugDumps();

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

    class BrcInitResetCurbe;
    class BrcFrameUpdateCurbe;
    class MbBrcUpdateCurbe;
    class MbEncCurbe;
    class WPCurbe;
    class BrcBlockCopyCurbe;

protected:
    static constexpr uint32_t m_initBrcHistoryBufferSize = 864;
    static constexpr uint32_t m_mbEncrcHistoryBufferSize = 128;
    static constexpr uint32_t m_brcConstSurfaceWidth = 64;
    static constexpr uint32_t m_brcConstSurfaceHeight = 53;
    static constexpr uint32_t m_refTreshold = 400;
    static constexpr uint32_t m_brcConstSurfaceEarlySkipTableSize = 128;
    static constexpr uint32_t m_brcConstSurfaceModeMvCostSize = 1664;
    static constexpr uint32_t m_brcConstSurfaceRefCostSize = 128;
    static constexpr uint32_t m_brcConstSurfaceQpList0 = 32;
    static constexpr uint32_t m_brcConstSurfaceQpList0Reserved = 32;
    static constexpr uint32_t m_brcConstSurfaceQpList1 = 32;
    static constexpr uint32_t m_brcConstSurfaceQpList1Reserved = 160;
    static constexpr uint32_t m_brcConstSurfaceIntraCostScalingFactor = 64;
    static constexpr uint32_t m_brcConstSurfaceLambdaSize = 512;
    static constexpr uint32_t m_brcConstSurfaceFtq25Size = 64;
    static constexpr uint32_t m_mbEncTargetUsageNum = 3;
    static constexpr uint32_t m_adaptiveTxDecisionThreshold = 128;
    static constexpr uint32_t m_mbTextureThreshold = 1024;
    static constexpr uint32_t m_defaultTrellisQuantIntraRounding = 5;
    static constexpr uint32_t m_sfdCostTableBufferSize = 52;
    static constexpr uint32_t m_sfdOutputBufferSize = 128;

    static const uint8_t  m_ftq25[64];
    static const uint16_t m_lambdData[256];
    static const uint16_t m_refCostMultiRefQp[NUM_PIC_TYPES][64];
    static const int32_t  m_brcBindingTableCount[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const int32_t  m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const uint32_t m_intraModeCostForHighTextureMB[CODEC_AVC_NUM_QP];
    static const uint32_t m_multiRefDisableQPCheck[NUM_TARGET_USAGE_MODES];
    static const uint32_t m_multiPred[NUM_TARGET_USAGE_MODES];
    static const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD m_IPCMThresholdTable[5];
    static const uint8_t  m_brcConstantDataTables[576]; //!< QP adjustment table, Distortion Threshold table, MaxFrame Threshold table, Distortion QP adjustment Table
};

#endif  // __CODECHAL_ENCODE_AVC_G10_H__
