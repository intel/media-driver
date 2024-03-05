/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     codechal_vdenc_avc_xe_hpm.h
//! \brief    This file defines the base C++ class/interface for Xe_HPM AVC VDENC
//!           encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_AVC_XE_HPM_H__
#define __CODECHAL_VDENC_AVC_XE_HPM_H__

#include "mhw_vdbox_xe_xpm.h"
#include "codechal_vdenc_avc_g12.h"

typedef struct _AVCVdencBRCCostantDataXe_Hpm
{
    uint8_t     UPD_GlobalRateQPAdjTabI_U8[64];
    uint8_t     UPD_GlobalRateQPAdjTabP_U8[64];
    uint8_t     UPD_GlobalRateQPAdjTabB_U8[64];
    uint8_t     UPD_DistThreshldI_U8[10];
    uint8_t     UPD_DistThreshldP_U8[10];
    uint8_t     UPD_DistThreshldB_U8[10];
    uint8_t     UPD_DistQPAdjTabI_U8[81];
    uint8_t     UPD_DistQPAdjTabP_U8[81];
    uint8_t     UPD_DistQPAdjTabB_U8[81];
    int8_t      UPD_BufRateAdjTabI_S8[72];
    int8_t      UPD_BufRateAdjTabP_S8[72];
    int8_t      UPD_BufRateAdjTabB_S8[72];
    uint8_t     UPD_FrmSzMinTabP_U8[9];
    uint8_t     UPD_FrmSzMinTabB_U8[9];
    uint8_t     UPD_FrmSzMinTabI_U8[9];
    uint8_t     UPD_FrmSzMaxTabP_U8[9];
    uint8_t     UPD_FrmSzMaxTabB_U8[9];
    uint8_t     UPD_FrmSzMaxTabI_U8[9];
    uint8_t     UPD_FrmSzSCGTabP_U8[9];
    uint8_t     UPD_FrmSzSCGTabB_U8[9];
    uint8_t     UPD_FrmSzSCGTabI_U8[9];
    // Cost Table 14*42 = 588 bytes
    uint8_t     UPD_I_IntraNonPred[42];
    uint8_t     UPD_I_Intra16x16[42];
    uint8_t     UPD_I_Intra8x8[42];
    uint8_t     UPD_I_Intra4x4[42];
    uint8_t     UPD_I_IntraChroma[42];
    uint8_t     UPD_P_IntraNonPred[42];
    uint8_t     UPD_P_Intra16x16[42];
    uint8_t     UPD_P_Intra8x8[42];
    uint8_t     UPD_P_Intra4x4[42];
    uint8_t     UPD_P_IntraChroma[42];
    uint8_t     UPD_P_Inter16x8[42];
    uint8_t     UPD_P_Inter8x8[42];
    uint8_t     UPD_P_Inter16x16[42];
    uint8_t     UPD_P_RefId[42];
    uint8_t     Reserved[630];
} AVCVdencBRCCostantDataXe_Hpm, *PAVCVdencBRCCostantDataXe_Hpm;

class CodechalVdencAvcStateXe_Hpm : public CodechalVdencAvcStateG12
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalVdencAvcStateXe_Hpm(
        CodechalHwInterface    *hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencAvcStateG12(hwInterface, debugInterface, standardInfo)
    {
        m_oneOnOneMapping = false;
        m_computeContextEnabled = true;
        m_nonNativeBrcRoiSupported = true;

        // Override pointers to slice size thresholds tables with new ones to align with HW improvements
        m_vdencSSCThrsTblI = SliceSizeThrsholdsI_Xe_Hpm;
        m_vdencSSCThrsTblP = SliceSizeThrsholdsP_Xe_Hpm;

#if USE_CODECHAL_DEBUG_TOOL
        m_populateTargetUsage = true;
#endif
    };

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencAvcStateXe_Hpm();

    virtual MOS_STATUS AllocateResources() override;
    virtual MOS_STATUS AllocateMDFResources() override;

    virtual MOS_STATUS Initialize(CodechalSetting * settings) override;

    virtual MOS_STATUS InitializeState() override;

    virtual MOS_STATUS InitMmcState() override;

    virtual MOS_STATUS DeltaQPUpdate(uint8_t QpModulationStrength) override;

    virtual void MotionEstimationDisableCheck() override;

    virtual bool CheckSupportedFormat(PMOS_SURFACE surface) override;

    virtual MOS_STATUS SetPictureStructs() override;

    virtual MOS_STATUS ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params) override;

    virtual uint32_t GetCurrConstDataBufIdx() override;

    virtual MOS_STATUS HuCBrcInitReset() override;

    virtual MOS_STATUS AddMfxAvcSlice(
        PMOS_COMMAND_BUFFER        cmdBuffer,
        PMHW_BATCH_BUFFER          batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState) override;

    virtual MOS_STATUS AddVdencSliceStateCmd(
        PMOS_COMMAND_BUFFER        cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE params) override;

    MOS_STATUS Execute(void *params) override;

protected:
    struct BrcInitDmem;
    struct BrcUpdateDmem;

    PMHW_VDBOX_AVC_IMG_PARAMS CreateMhwVdboxAvcImgParams() override;

    void SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS &param) override;

    void SetMfxPipeModeSelectParams(
        const CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS &genericParam,
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &param) override;

    MOS_STATUS SetMfxPipeBufAddrStateParams(
        CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS genericParam,
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &param) override;

    MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS       trellisQuantParams) override;

    MOS_STATUS LoadHmeMvCostTable(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
        uint8_t                           hmeMvCostTable[8][42]) override;

    uint32_t GetBRCCostantDataSize() override { return sizeof(AVCVdencBRCCostantDataXe_Hpm); }

    MOS_STATUS FillHucConstData(uint8_t *data, uint8_t picType) override;

    MOS_STATUS SetRounding(PCODECHAL_ENCODE_AVC_ROUNDING_PARAMS param, PMHW_VDBOX_AVC_SLICE_STATE sliceState) override;

    MOS_STATUS SetupWalkerContext(
        MOS_COMMAND_BUFFER* cmdBuffer,
        SendKernelCmdsParams* params) override;

    void CopyMBQPDataToStreamIn(CODECHAL_VDENC_STREAMIN_STATE* pData, uint8_t* pInputData) override;

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd) override;

    uint32_t GetPakVDEncPassDumpSize() override;
#endif

    virtual uint32_t GetVdencBRCImgStateBufferSize() override;

    virtual uint16_t GetAdaptiveRoundingNumSlices() override;

    MOS_STATUS AddVdencBrcImgBuffer(
        PMOS_RESOURCE             vdencBrcImgBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params) override;

    MOS_STATUS SetupThirdRef(PMOS_RESOURCE vdencStreamIn);

    // Switch GPU context at execute stage
    MOS_STATUS SwitchContext();

    MOS_STATUS ChangeContext();

    MOS_STATUS CheckHucLoadStatus();

    MOS_STATUS PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer);

    uint32_t m_mfxAvcImgStateSize    = 0;
    uint32_t m_vdencCmd3Size         = 0;
    uint32_t m_vdencAvcImgStateSize  = 0;
    uint32_t m_mfxAvcSlcStateSize    = 0;
    uint32_t m_vdencAvcSlcStateSize  = 0;
    uint32_t m_miBatchBufferEndSize  = 0;

    bool m_isContextSwitched         = false;  // used to change virtual node association at execute stage only once

    static const uint8_t G0_P_InterRounding[52];
    static const uint8_t G0_P_IntraRounding[52];
    static const uint8_t G3_P_InterRounding[52];
    static const uint8_t G3_P_IntraRounding[52];
    static const uint8_t G3_rB_InterRounding[52];
    static const uint8_t G3_rB_IntraRounding[52];
    static const uint8_t G3_B_InterRounding[52];
    static const uint8_t G3_B_IntraRounding[52];

    //Resources
    MOS_RESOURCE      m_hucAuthBuf                                      = {};  //!< Huc authentication buffer
    MHW_BATCH_BUFFER  m_2ndLevelBB[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};  //!< 2nd level batch buffer
    PMHW_BATCH_BUFFER m_batchBuf = nullptr;

private:
    static const uint16_t SliceSizeThrsholdsI_Xe_Hpm[52];
    static const uint16_t SliceSizeThrsholdsP_Xe_Hpm[52];
};

#endif  // __CODECHAL_VDENC_AVC_XE_HPM_H__
