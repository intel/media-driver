/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_huc_brc_update_packet.h
//! \brief    Defines the implementation of avc huc update packet
//!

#ifndef __CODECHAL_AVC_HUC_BRC_UPDATE_PACKET_H__
#define __CODECHAL_AVC_HUC_BRC_UPDATE_PACKET_H__

#include "encode_huc.h"
#if _ENCODE_RESERVED
#include "encode_avc_huc_brc_update_packet_ext.h"
#endif // _ENCODE_RESERVED

namespace encode
{

class AvcBasicFeature;
class AvcEncodeBRC;

struct VdencAvcHucBrcUpdateDmem
{
    uint8_t     BRCFunc_U8;                           // =1 for Update, other values are reserved for future use
    uint8_t     RSVD[3];
    uint32_t    UPD_TARGETSIZE_U32;                   // refer to AVC BRC for calculation
    uint32_t    UPD_FRAMENUM_U32;                     // frame number
    uint32_t    UPD_PeakTxBitsPerFrame_U32;           // current global target bits - previous global target bits (global target bits += input bits per frame)
    uint32_t    UPD_FrameBudget_U32;                  // target time counter
    uint32_t    FrameByteCount;                       // PAK output via MMIO
    uint32_t    TimingBudgetOverflow;                 // PAK output via MMIO
    uint32_t    ImgStatusCtrl;                        // PAK output via MMIO
    uint32_t    IPCMNonConformant;                    // PAK output via MMIO

    uint16_t    UPD_startGAdjFrame_U16[4];            // 10, 50, 100, 150
    uint16_t    UPD_MBBudget_U16[52];                 // MB bugdet for QP 0 - 51.
    uint16_t    UPD_SLCSZ_TARGETSLCSZ_U16;            // target slice size
    uint16_t    UPD_SLCSZ_UPD_THRDELTAI_U16[42];      // slice size threshold delta for I frame
    uint16_t    UPD_SLCSZ_UPD_THRDELTAP_U16[42];      // slice size threshold delta for P frame
    uint16_t    UPD_NumOfFramesSkipped_U16;           // Recording how many frames have been skipped.
    uint16_t    UPD_SkipFrameSize_U16;                 // Recording the skip frame size for one frame. =NumMBs * 1, assuming one bit per mb for skip frame.
    uint16_t    UPD_StaticRegionPct_U16;              // One entry, recording the percentage of static region
    uint8_t     UPD_gRateRatioThreshold_U8[7];        // 80,95,99,101,105,125,160
    uint8_t     UPD_CurrFrameType_U8;                 // Use AvcBrcFrameType enum to specify frame type
    uint8_t     UPD_startGAdjMult_U8[5];              // 1, 1, 3, 2, 1
    uint8_t     UPD_startGAdjDiv_U8[5];               // 40, 5, 5, 3, 1
    uint8_t     UPD_gRateRatioThresholdQP_U8[8];      // 253,254,255,0,1,1,2,3
    uint8_t     UPD_PAKPassNum_U8;                    // current pak pass number
    uint8_t     UPD_MaxNumPass_U8;                    // 2
    uint8_t     UPD_SceneChgWidth_U8[2];              // set both to MIN((NumP + 1) / 5, 6)
    uint8_t     UPD_SceneChgDetectEn_U8;              // Enable scene change detection
    uint8_t     UPD_SceneChgPrevIntraPctThreshold_U8; // =96. scene change previous intra percentage threshold
    uint8_t     UPD_SceneChgCurIntraPctThreshold_U8;  // =192. scene change current intra percentage threshold
    uint8_t     UPD_IPAverageCoeff_U8;                // lowdelay ? 0 : 128
    uint8_t     UPD_MinQpAdjustment_U8;               // Minimum QP increase step
    uint8_t     UPD_TimingBudgetCheck_U8;             // Flag indicating if kernel will check timing budget.
    int8_t      reserved_I8[4];                       // must be zero
    uint8_t     UPD_CQP_QpValue_U8;                   // Application specified target QP in BRC_ICQ mode
    uint8_t     UPD_CQP_FracQp_U8;                    // Application specified fine position in BRC_ICQ mode
    uint8_t     UPD_HMEDetectionEnable_U8;            // 0: default, 1: HuC BRC kernel requires information from HME detection kernel output
    uint8_t     UPD_HMECostEnable_U8;                 // 0: default, 1: driver provides HME cost table
    uint8_t     UPD_DisablePFrame8x8Transform_U8;     // 0: enable, 1: disable
    uint8_t     RSVD3;                                // must be zero
    uint8_t     UPD_ROISource_U8;                     // =0: disable, 1: ROIMap from HME Static Region or from App dirty rectangle, 2: ROIMap from App
    uint8_t     RSVD4;                                // must be zero
    uint16_t    UPD_TargetSliceSize_U16;              // default: 1498, max target slice size from app DDI
    uint16_t    UPD_MaxNumSliceAllowed_U16;           // computed by driver based on level idc
    uint16_t    UPD_SLBB_Size_U16;                    // second level batch buffer (SLBB) size in bytes, the input buffer will contain two SLBBs A and B, A followed by B, A and B have the same structure.
    uint16_t    UPD_SLBB_B_Offset_U16;                // offset in bytes from the beginning of the input buffer, it points to the start of SLBB B, set by driver for skip frame support
    uint16_t    UPD_AvcImgStateOffset_U16;            // offset in bytes from the beginning of SLBB A
    uint16_t    reserved_u16;
    uint32_t    NumOfSlice;                           // PAK output via MMIO

    // HME distortion based QP adjustment
    uint16_t    AveHmeDist_U16;                       // default: 0, in HME detection kernel output
    uint8_t     HmeDistAvailable_U8;                  // 0: disabled, 1: enabled
    uint8_t     DisableDMA;                           // default =0, use DMA data transfer; =1, use regular region read/write
    uint16_t    AdditionalFrameSize_U16;              // for slice size control improvement
    uint8_t     AddNALHeaderSizeInternally_U8;
    uint8_t     UPD_RoiQpViaForceQp_U8;               // HuC does not update StreamIn Buffer, 1: HuC updates StreamIn Buffer
    uint32_t    CABACZeroInsertionSize_U32;           // PAK output via MMIO
    uint32_t    MiniFramePaddingSize_U32;             // PAK output via MMIO
    uint16_t    UPD_WidthInMB_U16;                    // width in MB
    uint16_t    UPD_HeightInMB_U16;                   // height in MB
    int8_t      UPD_ROIQpDelta_I8[8];                 // Application specified ROI QP Adjustment for Zone0, Zone1, Zone2 and Zone3, Zone4, Zone5, Zone6 and Zone7.

    //HME--Offset values need to be a multiple of 4 in order to be aligned to the 4x4 HME block for downscaled 4X HME precision and HME--Offset range is [-128,127]
    int8_t       HME0XOffset_I8;    // default = 32, Frame level X offset from the co-located (0, 0) location for HME0.
    int8_t       HME0YOffset_I8;    // default = 24, Frame level Y offset from the co-located (0, 0) location for HME0.
    int8_t       HME1XOffset_I8;    // default = -32, Frame level X offset from the co-located (0, 0) location for HME1.
    int8_t       HME1YOffset_I8;    // default = -24, Frame level Y offset from the co-located (0, 0) location for HME1.
    uint8_t      MOTION_ADAPTIVE_G4;
    uint8_t      EnableLookAhead;
    uint8_t      UPD_LA_Data_Offset_U8;
    uint8_t      UPD_CQMEnabled_U8;  // 0 indicates CQM is disabled for current frame; otherwise CQM is enabled.
    uint32_t     UPD_LA_TargetSize_U32;     // target frame size in lookahead BRC (if EnableLookAhead == 1) or TCBRC mode. If zero, lookahead BRC or TCBRC is disabled.
    uint32_t     UPD_LA_TargetFulness_U32;  // target VBV buffer fulness in lookahead BRC mode (if EnableLookAhead == 1).
    uint8_t      UPD_Delta_U8;              // delta QP of pyramid
    uint8_t      UPD_ROM_CURRENT_U8;        // ROM average of current frame
    uint8_t      UPD_ROM_ZERO_U8;           // ROM zero percentage (255 is 100%)
    uint8_t      UPD_TCBRC_SCENARIO_U8;
    uint8_t      UPD_EnableFineGrainLA;
    int8_t       UPD_DeltaQpDcOffset;
    uint16_t     UPD_NumSlicesForRounding;
    uint32_t     UPD_UserMaxFramePB;        // In Bytes
    uint8_t      UPD_ExtCurrFrameType;      // correctly calculated FrameType for all cases (including hierarchy golden BGops)
    uint8_t      UPD_AdaptiveTUEnabled;
    uint8_t      RSVD2[2];
};

struct VdencAvcHucBrcConstantData
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
    uint8_t     VdencAvcHucBrcConstantData_0[8][42];
    uint8_t     VdencAvcHucBrcConstantData_1[42];
    uint16_t    VdencAvcHucBrcConstantData_2[42];
    uint16_t    VdencAvcHucBrcConstantData_3[42];
    uint8_t     VdencAvcHucBrcConstantData_4[42];
    uint8_t     VdencAvcHucBrcConstantData_5[42];
};

class AvcHucBrcUpdatePkt : public EncodeHucPkt
{
public:
    AvcHucBrcUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
        EncodeHucPkt(pipeline, task, hwInterface) {}

    virtual ~AvcHucBrcUpdatePkt() {}

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    virtual MOS_STATUS Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function = NONE_BRC) override;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    virtual MOS_STATUS DumpOutput() override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "BRCUPDATE_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

protected:
    virtual MOS_STATUS AllocateResources() override;

    virtual MOS_STATUS SetDmemBuffer()const;

    virtual MOS_STATUS SetConstDataHuCBrcUpdate()const;

    virtual MOS_STATUS ConstructImageStateReadBuffer(PMOS_RESOURCE imageStateBuffer);

    MHW_SETPAR_DECL_HDR(MFX_AVC_IMG_STATE);
    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpHucBrcUpdate(bool isInput);

    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer) { return MOS_STATUS_SUCCESS; }

    virtual MOS_STATUS PopulateEncParam(uint8_t meMethod, void *cmd) { return MOS_STATUS_SUCCESS; }

    virtual MOS_STATUS DumpEncodeImgStats(
        PMOS_COMMAND_BUFFER cmdbuffer);
#endif

    uint32_t GetCurrConstDataBufIdx() const;

    AvcBasicFeature *m_basicFeature = nullptr;                                          //!< Avc Basic Feature used in each frame
    AvcEncodeBRC    *m_brcFeature   = nullptr;                                          //!< Avc Encode BRC Feature used in each frame

    bool m_vdencStaticFrame = false;                                                    //!< Static Frame Indicator.

    uint32_t m_vdencBrcUpdateDmemBufferSize = sizeof(VdencAvcHucBrcUpdateDmem);         //!< Offset of BRC update DMEM buffer
    uint32_t m_vdencBrcConstDataBufferSize  = sizeof(VdencAvcHucBrcConstantData);       //!< Offset of BRC const data buffer

    PMOS_RESOURCE m_vdencBrcImageStatesReadBufferOrigin[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]               = {};  //!< Read-only VDENC+PAK IMG STATE buffer.
    PMOS_RESOURCE m_vdencBrcImageStatesReadBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]                  = {};  //!< Read-only VDENC+PAK IMG STATE buffer.
    PMOS_RESOURCE m_vdencBrcUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {};  //!< Brc Update DMEM Buffer Array.
    PMOS_RESOURCE m_vdencBrcConstDataBuffer[CODECHAL_ENCODE_VDENC_BRC_CONST_BUFFER_NUM]                    = {};  //!< BRC Const Data Buffer for each frame type.

    PMOS_RESOURCE m_resPakOutputViaMmioBuffer = {};  //!< Buffer for PAK statistics output via MMIO

MEDIA_CLASS_DEFINE_END(encode__AvcHucBrcUpdatePkt)
};

}  // namespace encode

#endif   // !__CODECHAL_AVC_HUC_BRC_UPDATE_PACKET_H__
