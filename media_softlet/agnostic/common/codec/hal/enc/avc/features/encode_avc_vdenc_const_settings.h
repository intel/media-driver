/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_avc_vdenc_const_settings.h
//! \brief    Defines the common interface for avc vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __ENCODE_AVC_VDENC_CONST_SETTINGS_H__
#define __ENCODE_AVC_VDENC_CONST_SETTINGS_H__

#include "codec_def_common_encode.h"
#include "codec_def_encode_avc.h"
#include "encode_const_settings.h"
#include "media_class_trace.h"
#include "media_feature_const_settings.h"
#include "mos_defs.h"
#include <stdint.h>
#if _ENCODE_RESERVED
#include "encode_avc_vdenc_const_settings_ext.h"
#endif // _ENCODE_RESERVED

namespace encode
{

struct AvcVdencCMD3ConstSettings
{
    static const uint8_t  AvcVdencCMD3ConstSettings_0[8][52];
    static const uint8_t  AvcVdencCMD3ConstSettings_1[3][12];
    static const uint8_t  AvcVdencCMD3ConstSettings_2[3][13];
    static const uint8_t  AvcVdencCMD3ConstSettings_3[3][52];
    static const uint8_t  AvcVdencCMD3ConstSettings_4[52];
    static const uint16_t AvcVdencCMD3ConstSettings_5[4][2][40];
    static const uint16_t AvcVdencCMD3ConstSettings_6[4][2][40];
    static const uint16_t par31Table[2][3][2][52];
    static const uint16_t par32Table[2][3][2][52];
};

struct AvcVdencBrcSettings
{
    uint32_t  vdencBrcPakStatsBufferSize;
    uint32_t  vdencBrcStatsBufferSize;
    uint32_t  vdencBrcDbgBufferSize;
    uint32_t  vdencBrcHistoryBufferSize;

    uint32_t  vdboxHucVdencBrcInitKernelDescriptor;        //!< Huc Vdenc Brc init kernel descriptor
    uint32_t  vdboxHucVdencBrcUpdateKernelDescriptor;      //!< Huc Vdenc Brc update kernel descriptor

    double   *BRC_DevThreshI0_FP_NEG;                      //!< Negative BRC threshold for I frame
    double   *BRC_DevThreshI0_FP_POS;                      //!< Positive BRC threshold for I frame
    double   *BRC_DevThreshPB0_FP_NEG;                     //!< Negative BRC threshold for P/B frame
    double   *BRC_DevThreshPB0_FP_POS;                     //!< Positive BRC threshold for P/B frame
    double   *BRC_DevThreshVBR0_NEG;                       //!< Negative BRC threshold for VBR mode
    double   *BRC_DevThreshVBR0_POS;                       //!< Positive BRC threshold for VBR mode

    int8_t   *BRC_LowDelay_DevThreshPB0_S8;                //!< Low Delay BRC threshold for P/B frame
    int8_t   *BRC_LowDelay_DevThreshI0_S8;                 //!< Low Delay BRC threshold for I frame
    int8_t   *BRC_LowDelay_DevThreshVBR0_S8;               //!< Low Delay BRC threshold for VBR Mode

    int8_t   *BRC_INIT_DistQPDelta_I8;                     //!< Distortion QP Delta
    uint8_t  *BRC_EstRateThreshP0_U8;                      //!< Estimate Rate Thresh of P frame
    uint8_t  *BRC_EstRateThreshI0_U8;                      //!< Estimate Rate Thresh of I frame

    int8_t   *brcInitDistQpDeltaI8;
    int8_t   *brcInitDistQpDeltaI8LowDelay;

    uint16_t *BRC_UPD_start_global_adjust_frame;           //!< Start Global Adjust Frame
    uint8_t  *BRC_UPD_global_rate_ratio_threshold;         //!< Global Rate Ratio Threshold
    uint8_t  *BRC_UPD_slwin_global_rate_ratio_threshold;   //!< Slide Window Global Rate Ratio Threshold
    uint8_t  *BRC_UPD_start_global_adjust_mult;            //!< Start Global Adjust Multiply
    uint8_t  *BRC_UPD_start_global_adjust_div;             //!< Start Global Adjust Division
    int8_t   *BRC_UPD_global_rate_ratio_threshold_qp;      //!< Global Rate Ratio QP Threshold

    int8_t   *BRC_UPD_GlobalRateQPAdjTabI_U8;              //!< I Picture Global Rate QP Adjustment Table.
    int8_t   *BRC_UPD_GlobalRateQPAdjTabP_U8;              //!< P Picture Global Rate QP Adjustment Table.
    int8_t   *BRC_UPD_SlWinGlobalRateQPAdjTabP_U8;         //!< P picture Global Rate QP Adjustment Table for Sliding Window BRC
    int8_t   *BRC_UPD_GlobalRateQPAdjTabB_U8;              //!< B Picture Global Rate QP Adjustment Table.
    uint8_t  *BRC_UPD_DistThreshldI_U8;                    //!< I Picture Distortion THreshold.
    uint8_t  *BRC_UPD_DistThreshldP_U8;                    //!< P Picture Distortion THreshold.
    uint8_t  *BRC_UPD_DistThreshldB_U8;                    //!< B Picture Distortion THreshold.
    int8_t   *CBR_UPD_DistQPAdjTabI_U8;                    //!< I Picture Distortion QP Adjustment Table under CBR Mode.
    int8_t   *CBR_UPD_DistQPAdjTabP_U8;                    //!< P Picture Distortion QP Adjustment Table under CBR Mode.
    int8_t   *CBR_UPD_DistQPAdjTabB_U8;                    //!< B Picture Distortion QP Adjustment Table under CBR Mode.
    int8_t   *VBR_UPD_DistQPAdjTabI_U8;                    //!< I Picture Distortion QP Adjustment Table under VBR Mode.
    int8_t   *VBR_UPD_DistQPAdjTabP_U8;                    //!< P Picture Distortion QP Adjustment Table under VBR Mode.
    int8_t   *VBR_UPD_DistQPAdjTabB_U8;                    //!< B Picture Distortion QP Adjustment Table under VBR Mode.
    int8_t   *CBR_UPD_FrmSzAdjTabI_S8;                     //!< I Picture Frame Size Adjustment Table under CBR Mode.
    int8_t   *CBR_UPD_FrmSzAdjTabP_S8;                     //!< P Picture Frame Size Adjustment Table under CBR Mode.
    int8_t   *CBR_UPD_FrmSzAdjTabB_S8;                     //!< B Picture Frame Size Adjustment Table under CBR Mode.
    int8_t   *VBR_UPD_FrmSzAdjTabI_S8;                     //!< I Picture Frame Size Adjustment Table under VBR Mode.
    int8_t   *VBR_UPD_FrmSzAdjTabP_S8;                     //!< P Picture Frame Size Adjustment Table under VBR Mode.
    int8_t   *VBR_UPD_FrmSzAdjTabB_S8;                     //!< B Picture Frame Size Adjustment Table under VBR Mode.
    int8_t   *QVBR_UPD_FrmSzAdjTabP_S8;                    //!< P Picture Frame Size Adjustment Table under QVBR Mode.
    int8_t   *LOW_DELAY_UPD_FrmSzAdjTabI_S8;               //!< I Picture Frame Size Adjustment Table under Low Delay Mode.
    int8_t   *LOW_DELAY_UPD_FrmSzAdjTabP_S8;               //!< P Picture Frame Size Adjustment Table under Low Delay Mode.
    int8_t   *LOW_DELAY_UPD_FrmSzAdjTabB_S8;               //!< B Picture Frame Size Adjustment Table under Low Delay Mode.
    uint8_t  *BRC_UPD_FrmSzMinTabP_U8;                     //!< I Picture Minimum Frame Size Table.
    uint8_t  *BRC_UPD_FrmSzMinTabI_U8;                     //!< P Picture Minimum Frame Size Table.
    uint8_t  *BRC_UPD_FrmSzMaxTabP_U8;                     //!< I Picture Maximum Frame Size Table.
    uint8_t  *BRC_UPD_FrmSzMaxTabI_U8;                     //!< P Picture Maximum Frame Size Table.
    uint8_t  *BRC_UPD_FrmSzSCGTabP_U8;                     //!<
    uint8_t  *BRC_UPD_FrmSzSCGTabI_U8;                     //!<

    uint8_t  *BRC_UPD_I_IntraNonPred;                      //!< Cost Table for Intra Non-Prediction
    uint8_t  *BRC_UPD_I_Intra8x8;                          //!< Cost Table for Intra 8x8
    uint8_t  *BRC_UPD_I_Intra4x4;                          //!< Cost Table for Intra 4x4
    uint8_t  *BRC_UPD_P_IntraNonPred;                      //!< Cost Table for Intra Non-Prediction
    uint8_t  *BRC_UPD_P_Intra16x16;                        //!< Cost Table for Intra 16x16
    uint8_t  *BRC_UPD_P_Intra8x8;                          //!< Cost Table for Intra 8x8
    uint8_t  *BRC_UPD_P_Intra4x4;                          //!< Cost Table for Intra 4x4
    uint8_t  *BRC_UPD_P_Inter16x8;                         //!< Cost Table for Inter 16x8
    uint8_t  *BRC_UPD_P_Inter8x8;                          //!< Cost Table for Inter 8x8
    uint8_t  *BRC_UPD_P_Inter16x16;                        //!< Cost Table for Inter 16x16
    uint8_t  *BRC_UPD_P_RefId;                             //!< Cost Table for Reference Index
};

struct AvcVdencFeatureSettings : VdencFeatureSettings
{
    uint32_t  singlePassMinFrameWidth = 0;
    uint32_t  singlePassMinFrameHeight = 0;
    uint32_t  singlePassMinFramePer100s = 0;

    uint32_t  interMbMaxSize = 0;
    uint32_t  intraMbMaxSize = 0;

    bool     *perfModeEnabled = nullptr;

    uint8_t   DefaultIntraRounding     = 0;
    uint8_t   DefaultInterRounding     = 0;
    uint8_t   StaticIntraRounding[4]   = {};
    uint8_t   StaticInterRounding[4]   = {};
    const uint8_t *AdaptiveIntraRounding[4] = {nullptr, nullptr, nullptr, nullptr};
    const uint8_t *AdaptiveInterRounding[4] = {nullptr, nullptr, nullptr, nullptr};

    uint16_t *SliceSizeThrsholdsI = nullptr;
    uint16_t *SliceSizeThrsholdsP = nullptr;

    uint32_t *TrellisQuantizationRounding = nullptr;
    bool     *TrellisQuantizationEnable   = nullptr;

    uint8_t  *columnScan4x4 = nullptr;
    uint8_t  *columnScan8x8 = nullptr;

    AvcVdencCMD3ConstSettings *vdencCMD3Table = nullptr;
    AvcVdencBrcSettings      brcSettings      = {};
};

struct AvcVdencBrcConstSettings
{
    static constexpr uint32_t  m_vdencBrcPakStatsBufferSize = 204;                   //!< Vdenc bitrate control PAK buffer size
    static constexpr uint32_t  m_vdencBrcStatsBufferSize    = 80;                    //!< Vdenc bitrate control buffer size
    static constexpr uint32_t  m_vdencBrcDbgBufferSize      = 0x1000;
    static constexpr uint32_t  m_vdencBrcHistoryBufferSize  = 0x1000;
    static constexpr uint32_t  m_numDevThreshlds            = 8;

    static const uint32_t m_vdboxHucVdencBrcInitKernelDescriptor = 4;                //!< Huc Vdenc Brc init kernel descriptor
    static const uint32_t m_vdboxHucVdencBrcUpdateKernelDescriptor = 5;              //!< Huc Vdenc Brc update kernel descriptor

    static const double   m_BRC_DevThreshI0_FP_NEG[m_numDevThreshlds / 2];   //!< Negative BRC threshold for I frame
    static const double   m_BRC_DevThreshI0_FP_POS[m_numDevThreshlds / 2];   //!< Positive BRC threshold for I frame
    static const double   m_BRC_DevThreshPB0_FP_NEG[m_numDevThreshlds / 2];  //!< Negative BRC threshold for P/B frame
    static const double   m_BRC_DevThreshPB0_FP_POS[m_numDevThreshlds / 2];  //!< Positive BRC threshold for P/B frame
    static const double   m_BRC_DevThreshVBR0_NEG[m_numDevThreshlds / 2];    //!< Negative BRC threshold for VBR mode
    static const double   m_BRC_DevThreshVBR0_POS[m_numDevThreshlds / 2];    //!< Positive BRC threshold for VBR mode

    static const int8_t   m_BRC_LowDelay_DevThreshPB0_S8[8];                         //!< Low Delay BRC threshold for P/B frame
    static const int8_t   m_BRC_LowDelay_DevThreshI0_S8[8];                          //!< Low Delay BRC threshold for I frame
    static const int8_t   m_BRC_LowDelay_DevThreshVBR0_S8[8];                        //!< Low Delay BRC threshold for VBR Mode

    static const int8_t   m_BRC_INIT_DistQPDelta_I8[4];                              //!< Distortion QP Delta
    static const uint8_t  m_BRC_EstRateThreshP0_U8[7];                               //!< Estimate Rate Thresh of P frame
    static const uint8_t  m_BRC_EstRateThreshI0_U8[7];                               //!< Estimate Rate Thresh of I frame

    static const int8_t   m_brcInitDistQpDeltaI8[4];
    static const int8_t   m_brcInitDistQpDeltaI8LowDelay[4];

    static const uint16_t m_BRC_UPD_start_global_adjust_frame[4];                    //!< Start Global Adjust Frame
    static const uint8_t  m_BRC_UPD_global_rate_ratio_threshold[7];                  //!< Global Rate Ratio Threshold
    static const uint8_t  m_BRC_UPD_slwin_global_rate_ratio_threshold[7];            //!< Slide Window Global Rate Ratio Threshold
    static const uint8_t  m_BRC_UPD_start_global_adjust_mult[5];                     //!< Start Global Adjust Multiply
    static const uint8_t  m_BRC_UPD_start_global_adjust_div[5];                      //!< Start Global Adjust Division
    static const int8_t   m_BRC_UPD_global_rate_ratio_threshold_qp[8];               //!< Global Rate Ratio QP Threshold

    static const int8_t   m_BRC_UPD_GlobalRateQPAdjTabI_U8[64];                      //!< I Picture Global Rate QP Adjustment Table.
    static const int8_t   m_BRC_UPD_GlobalRateQPAdjTabP_U8[64];                      //!< P Picture Global Rate QP Adjustment Table.
    static const int8_t   m_BRC_UPD_SlWinGlobalRateQPAdjTabP_U8[64];                 //!< P picture Global Rate QP Adjustment Table for Sliding Window BRC
    static const int8_t   m_BRC_UPD_GlobalRateQPAdjTabB_U8[64];                      //!< B Picture Global Rate QP Adjustment Table.
    static const uint8_t  m_BRC_UPD_DistThreshldI_U8[10];                            //!< I Picture Distortion THreshold.
    static const uint8_t  m_BRC_UPD_DistThreshldP_U8[10];                            //!< P Picture Distortion THreshold.
    static const uint8_t  m_BRC_UPD_DistThreshldB_U8[10];                            //!< P Picture Distortion THreshold.
    static const int8_t   m_CBR_UPD_DistQPAdjTabI_U8[81];                            //!< I Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   m_CBR_UPD_DistQPAdjTabP_U8[81];                            //!< P Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   m_CBR_UPD_DistQPAdjTabB_U8[81];                            //!< B Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   m_VBR_UPD_DistQPAdjTabI_U8[81];                            //!< I Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   m_VBR_UPD_DistQPAdjTabP_U8[81];                            //!< P Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   m_VBR_UPD_DistQPAdjTabB_U8[81];                            //!< B Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   m_CBR_UPD_FrmSzAdjTabI_S8[72];                             //!< I Picture Frame Size Adjustment Table under CBR Mode.
    static const int8_t   m_CBR_UPD_FrmSzAdjTabP_S8[72];                             //!< P Picture Frame Size Adjustment Table under CBR Mode.
    static const int8_t   m_CBR_UPD_FrmSzAdjTabB_S8[72];                             //!< B Picture Frame Size Adjustment Table under CBR Mode.
    static const int8_t   m_VBR_UPD_FrmSzAdjTabI_S8[72];                             //!< I Picture Frame Size Adjustment Table under VBR Mode.
    static const int8_t   m_VBR_UPD_FrmSzAdjTabP_S8[72];                             //!< P Picture Frame Size Adjustment Table under VBR Mode.
    static const int8_t   m_VBR_UPD_FrmSzAdjTabB_S8[72];                             //!< B Picture Frame Size Adjustment Table under VBR Mode.
    static const int8_t   m_QVBR_UPD_FrmSzAdjTabP_S8[72];                            //!< P Picture Frame Size Adjustment Table under QVBR Mode.
    static const int8_t   m_LOW_DELAY_UPD_FrmSzAdjTabI_S8[72];                       //!< I Picture Frame Size Adjustment Table under Low Delay Mode.
    static const int8_t   m_LOW_DELAY_UPD_FrmSzAdjTabP_S8[72];                       //!< P Picture Frame Size Adjustment Table under Low Delay Mode.
    static const int8_t   m_LOW_DELAY_UPD_FrmSzAdjTabB_S8[72];                       //!< B Picture Frame Size Adjustment Table under Low Delay Mode.
    static const uint8_t  m_BRC_UPD_FrmSzMinTabP_U8[9];                              //!< I Picture Minimum Frame Size Table.
    static const uint8_t  m_BRC_UPD_FrmSzMinTabI_U8[9];                              //!< P Picture Minimum Frame Size Table.
    static const uint8_t  m_BRC_UPD_FrmSzMaxTabP_U8[9];                              //!< I Picture Maximum Frame Size Table.
    static const uint8_t  m_BRC_UPD_FrmSzMaxTabI_U8[9];                              //!< P Picture Maximum Frame Size Table.
    static const uint8_t  m_BRC_UPD_FrmSzSCGTabP_U8[9];                              //!<
    static const uint8_t  m_BRC_UPD_FrmSzSCGTabI_U8[9];                              //!<

    static const uint8_t  m_BRC_UPD_I_IntraNonPred[42];                              //!< Cost Table for Intra Non-Prediction
    static const uint8_t  m_BRC_UPD_I_Intra8x8[42];                                  //!< Cost Table for Intra 8x8
    static const uint8_t  m_BRC_UPD_I_Intra4x4[42];                                  //!< Cost Table for Intra 4x4
    static const uint8_t  m_BRC_UPD_P_IntraNonPred[42];                              //!< Cost Table for Intra Non-Prediction
    static const uint8_t  m_BRC_UPD_P_Intra16x16[42];                                //!< Cost Table for Intra 16x16
    static const uint8_t  m_BRC_UPD_P_Intra8x8[42];                                  //!< Cost Table for Intra 8x8
    static const uint8_t  m_BRC_UPD_P_Intra4x4[42];                                  //!< Cost Table for Intra 4x4
    static const uint8_t  m_BRC_UPD_P_Inter16x8[42];                                 //!< Cost Table for Inter 16x8
    static const uint8_t  m_BRC_UPD_P_Inter8x8[42];                                  //!< Cost Table for Inter 8x8
    static const uint8_t  m_BRC_UPD_P_Inter16x16[42];                                //!< Cost Table for Inter 16x16
    static const uint8_t  m_BRC_UPD_P_RefId[42];                                     //!< Cost Table for Reference Index
};

struct LutModeConstSettings
{
    static constexpr uint32_t INTRA_NONPRED    = 0x00;
    static constexpr uint32_t INTRA            = 0x01;
    static constexpr uint32_t INTRA_16x16      = 0x01;
    static constexpr uint32_t INTRA_8x8        = 0x02;
    static constexpr uint32_t INTRA_4x4        = 0x03;
    static constexpr uint32_t INTER_BWD        = 0x09;
    static constexpr uint32_t REF_ID           = 0x0A;
    static constexpr uint32_t INTRA_CHROMA     = 0x0B;
    static constexpr uint32_t INTER            = 0x08;
    static constexpr uint32_t INTER_16x16      = 0x08;
    static constexpr uint32_t INTER_16x8       = 0x04;
    static constexpr uint32_t INTER_8x16       = 0x04;
    static constexpr uint32_t INTER_8x8q       = 0x05;
    static constexpr uint32_t INTER_8x4q       = 0x06;
    static constexpr uint32_t INTER_4x8q       = 0x06;
    static constexpr uint32_t INTER_4x4q       = 0x07;
    static constexpr uint32_t INTER_16x8_FIELD = 0x06;
    static constexpr uint32_t INTER_8x8_FIELD  = 0x07;
};

struct RdModeConstSettings
{
    static constexpr uint32_t INTRA_MPM    = 0;
    static constexpr uint32_t INTRA_16X16  = 1;
    static constexpr uint32_t INTRA_8X8    = 2;
    static constexpr uint32_t INTRA_4X4    = 3;
    static constexpr uint32_t INTER_16X8   = 4;
    static constexpr uint32_t INTER_8X8    = 5;
    static constexpr uint32_t INTER_8X4    = 6;
    static constexpr uint32_t INTER_16X16  = 7;
    static constexpr uint32_t INTER_BWD    = 8;
    static constexpr uint32_t REF_ID       = 9;
    static constexpr uint32_t INTRA_CHROMA = 10;
    static constexpr uint32_t SKIP_16X16   = 11;
    static constexpr uint32_t DIRECT_16X16 = 12;
};

class EncodeAvcVdencConstSettings : public VdencConstSettings
{
public:

    //!
    //! \brief  EncodeAvcVdencConstSettings constructor
    //!
    EncodeAvcVdencConstSettings() = default;

    EncodeAvcVdencConstSettings(PMOS_INTERFACE osInterface);

    //!
    //! \brief  EncodeAvcVdencConstSettings deconstructor
    //!
    virtual ~EncodeAvcVdencConstSettings() {};

    //!
    //! \brief  Prepare const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PrepareConstSettings() override;

    MOS_STATUS Update(void *params) override;

protected:

    //!
    //! \brief  Prepare common settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCommonSettings() override;

    //!
    //! \brief  Prepare BRC related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcSettings() override;

    MOS_STATUS SetVdencCmd2Settings() override
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetVdencCmd3Settings();

    virtual MOS_STATUS SetVdencAvcImgStateSettings()
    {
        return MOS_STATUS_SUCCESS;
    }

    static constexpr uint32_t  m_singlePassMinFrameWidth   = 3840;
    static constexpr uint32_t  m_singlePassMinFrameHeight  = 2160;
    static constexpr uint32_t  m_singlePassMinFramePer100s = 6000;

    static constexpr uint32_t  m_interMbMaxSize            = 4095;               //! AVC inter macroblock max size
    static constexpr uint32_t  m_intraMbMaxSize            = 2700;               //! AVC intra macroblock max size

    static const bool    m_perfModeEnabled[NUM_VDENC_TARGET_USAGE_MODES];

    static constexpr uint8_t  defIntraRounding             = 5;
    static constexpr uint8_t  defInterRounding             = 2;

    static constexpr uint8_t  interRoundingP               = 3;
    static constexpr uint8_t  interRoundingB               = 0;
    static constexpr uint8_t  interRoundingBR              = 2;

    static const uint8_t adaptiveRoundingIntra_P_G1[CODEC_AVC_NUM_QP];         //!< Per-QP rounding table for Intra MBs in P frames (GOP1)
    static const uint8_t adaptiveRoundingIntra_P[CODEC_AVC_NUM_QP];            //!< Per-QP rounding table for Intra MBs in P frames
    static const uint8_t adaptiveRoundingIntra_B[CODEC_AVC_NUM_QP];            //!< Per-QP rounding table for Intra MBs in non-ref B frames
    static const uint8_t adaptiveRoundingIntra_BR[CODEC_AVC_NUM_QP];           //!< Per-QP rounding table for Intra MBs in ref B frames
    static const uint8_t adaptiveRoundingInter_P_G1[CODEC_AVC_NUM_QP];         //!< Per-QP rounding table for Inter MBs in P frames (GOP1)
    static const uint8_t adaptiveRoundingInter_P[CODEC_AVC_NUM_QP];            //!< Per-QP rounding table for Inter MBs in P frames
    static const uint8_t adaptiveRoundingInter_B[CODEC_AVC_NUM_QP];            //!< Per-QP rounding table for Intra MBs in non-ref B frames
    static const uint8_t adaptiveRoundingInter_BR[CODEC_AVC_NUM_QP];           //!< Per-QP rounding table for Intra MBs in ref B frames

    static const uint16_t  m_SliceSizeThrsholdsI[CODEC_AVC_NUM_QP];              //!< I picture slice size conformance thresholds table.
    static const uint16_t  m_SliceSizeThrsholdsP[CODEC_AVC_NUM_QP];              //!< P picture slice size conformance thresholds table.

    static const uint32_t  m_trellisQuantizationRounding[NUM_VDENC_TARGET_USAGE_MODES];
    static const bool      m_trellisQuantizationEnable[NUM_TARGET_USAGE_MODES];

    static const uint8_t   m_columnScan4x4[16];                                  //!< AVC column scan order for 4x4 block
    static const uint8_t   m_columnScan8x8[64];                                  //!< AVC column scan order for 8x8 block

    AvcVdencBrcConstSettings      m_brcSettings;
    AvcVdencCMD3ConstSettings     m_CMD3Settings;

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS m_avcSeqParams   = nullptr;
    PCODEC_AVC_ENCODE_PIC_PARAMS      m_avcPicParams   = nullptr;
    PCODEC_AVC_ENCODE_SLICE_PARAMS    m_avcSliceParams = nullptr;

    int32_t m_qp                   = 0;

MEDIA_CLASS_DEFINE_END(encode__EncodeAvcVdencConstSettings)
};

}
#endif // !__ENCODE_AVC_VDENC_CONST_SETTINGS_H__
