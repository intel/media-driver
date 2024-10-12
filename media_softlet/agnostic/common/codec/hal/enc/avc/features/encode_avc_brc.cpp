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
//! \file     encode_avc_brc.cpp
//! \brief    Defines the common interface for avc brc features
//!

#include "encode_avc_brc.h"
#include "encode_avc_vdenc_feature_manager.h"
#include "encode_avc_vdenc_const_settings.h"
#include "encode_avc_huc_brc_init_packet.h"
#include "encode_avc_huc_brc_update_packet.h"
#include "encode_avc_basic_feature.h"

namespace encode
{

#define ENCODE_AVC_BRC_MIN_QP                      1
#define VDENC_AVC_BRC_MIN_QP                       10
#define VDENC_AVC_BPS_RATIO_LOW                    0.1
#define VDENC_AVC_BPS_RATIO_HIGH                   3.5
#define ENCODE_AVC_VDENC_MIN_ICQ_QUALITYFACTOR     11

//dynamic deviation thresholds calculation
#define VDENC_AVC_POS_MULT_I                       50
#define VDENC_AVC_NEG_MULT_I                       -50
#define VDENC_AVC_POS_MULT_PB                      50
#define VDENC_AVC_NEG_MULT_PB                      -50
#define VDENC_AVC_POS_MULT_VBR                     100
#define VDENC_AVC_NEG_MULT_VBR                     -50

#define VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS   2
#define VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS   1
#define VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS     32
#define VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS     24

#define VDENC_AVC_AVBR_TOPQPDELTATHRFORADAPT2PASS  2
#define VDENC_AVC_AVBR_BOTQPDELTATHRFORADAPT2PASS  2
#define VDENC_AVC_AVBR_TOPFRMSZTHRFORADAPT2PASS    48
#define VDENC_AVC_AVBR_BOTFRMSZTHRFORADAPT2PASS    32

#define VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS_4K 5
#define VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS_4K 5
#define VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS_4K   80
#define VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS_4K   80

// VDEnc BRC Flag in BRC Init Kernel
typedef enum _BRCFLAG
{
    BRCFLAG_ISICQ                        = 0x0000,
    BRCFLAG_ISCBR                        = 0x0010,
    BRCFLAG_ISVBR                        = 0x0020,
    BRCFLAG_ISVCM                        = 0x0040,
    BRCFLAG_ISLOWDELAY                   = 0x0080
} BRCFLAG;

AvcEncodeBRC::AvcEncodeBRC(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) :
    MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
    m_featureManager = featureManager;
    //TODO: can be optimized after move encode parameter to feature manager.
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManager*>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
    m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    m_mfxItf   = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
    m_miItf    = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
}

AvcEncodeBRC::~AvcEncodeBRC()
{
    FreeBrcResources();
}

MOS_STATUS AvcEncodeBRC::Init(void *setting)
{
    ENCODE_FUNC_CALL();

    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "AVC Encode MB BRC",
        MediaUserSetting::Group::Sequence);

    int32_t val                  = outValue.Get<int32_t>();
    if ((val == 0) || (val == 1))
    {
        m_mbBrcUserFeatureKeyControl = true;
        m_mbBrcEnabled               =  val ? true : false;
    }

    outValue = 0;

    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "VDEnc Single Pass Enable",
        MediaUserSetting::Group::Sequence);

    m_vdencSinglePassEnable = outValue.Get<int32_t>() == 1;

#if (_DEBUG || _RELEASE_INTERNAL)
    auto statusKey = ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "Lpla TargetBufferFulness Data Address",
        MediaUserSetting::Group::Sequence);
    const char             *path_buffer = outValue.ConstString().c_str();

    if (statusKey == MOS_STATUS_SUCCESS && strcmp(path_buffer, "") != 0)
        m_useBufferFulnessData = true;

    if (m_useBufferFulnessData)
    {
        std::ifstream fp(path_buffer);
        if (!fp.is_open())
        {
            m_useBufferFulnessData = false;
            ENCODE_ASSERTMESSAGE("lpla targetBufferFulness data load failed!");
        }
        int         cnt  = 0;
        std::string line = "";
        while (getline(fp, line))
        {
            m_bufferFulnessData_csv[cnt++] = atoi(line.c_str());
        }
    }
#endif

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());

    if (m_basicFeature->m_seqParam->RateControlMethod == RATECONTROL_VCM && m_basicFeature->m_pictureCodingType == B_TYPE)
    {
        ENCODE_ASSERTMESSAGE("VCM BRC mode does not support B-frames\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_basicFeature->m_seqParam->RateControlMethod == RATECONTROL_VCM &&
        (m_basicFeature->m_picParam->FieldCodingFlag || m_basicFeature->m_picParam->FieldFrameCodingFlag))
    {
        ENCODE_ASSERTMESSAGE("VCM BRC mode does not support interlaced\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Encode RateControl Method",
        m_rcMode,
        MediaUserSetting::Group::Sequence);

    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Adaptive TU Enable",
        MediaUserSetting::Group::Sequence);
    m_basicFeature->m_picParam->AdaptiveTUEnabled |= outValue.Get<uint8_t>();
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::AllocateResources()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    auto brcSettings = setting->brcSettings;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // VDENC Statistics buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(brcSettings.vdencBrcStatsBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Statistics Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(VdencStatsBuffer, allocParamsForBufferLinear, 1));

    // VDENC BRC PAK MMIO buffer
    allocParamsForBufferLinear.dwBytes = sizeof(VdencBrcPakMmio);
    allocParamsForBufferLinear.pBufName = "VDENC BRC PAK MMIO Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcPakMmioBuffer, allocParamsForBufferLinear, 1));

    // Debug buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(brcSettings.vdencBrcDbgBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Debug Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcDebugBuffer, allocParamsForBufferLinear, 1));

    // BRC history buffer
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(brcSettings.vdencBrcHistoryBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC History Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(VdencBRCHistoryBuffer, allocParamsForBufferLinear, 1));

    // VDENC uses second level batch buffer for image state cmds
    MOS_ZeroMemory(&m_batchBufferForVdencImgStat, sizeof(MHW_BATCH_BUFFER));
    m_batchBufferForVdencImgStat.bSecondLevel = true;
    ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
        m_hwInterface->GetOsInterface(),
        &m_batchBufferForVdencImgStat,
        nullptr,
        GetVdencBRCImgStateBufferSize()));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::SetDmemForInit(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    auto hucVdencBrcInitDmem =(VdencAvcHucBrcInitDmem*)params;
    static_assert(sizeof(VdencAvcHucBrcInitDmem) == 192, "VdencAvcHucBrcInitDmem size MUST be equal to 192");

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    auto brcSettings  = setting->brcSettings;
    auto avcSeqParams = m_basicFeature->m_seqParam;
    auto avcPicParams = m_basicFeature->m_picParam;

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) // Low Delay Mode
    {
        avcSeqParams->MaxBitRate = avcSeqParams->TargetBitRate;
    }

    m_dBrcInitResetInputBitsPerFrame     = avcSeqParams->MaxBitRate * 100.0 / avcSeqParams->FramesPer100Sec;
    m_dBrcInitCurrentTargetBufFullInBits = m_dBrcInitResetInputBitsPerFrame;
    m_dBrcTargetSize                     = avcSeqParams->InitVBVBufferFullnessInBit;

    hucVdencBrcInitDmem->BRCFunc_U8 = m_brcInit ? 0 : 2;  // 0 for init, 2 for reset

    hucVdencBrcInitDmem->INIT_FrameWidth_U16  = (uint16_t)m_basicFeature->m_frameWidth;
    hucVdencBrcInitDmem->INIT_FrameHeight_U16 = (uint16_t)m_basicFeature->m_frameHeight;

    hucVdencBrcInitDmem->INIT_TargetBitrate_U32 = avcSeqParams->TargetBitRate;
    hucVdencBrcInitDmem->INIT_MinRate_U32 = avcSeqParams->MinBitRate;
    hucVdencBrcInitDmem->INIT_MaxRate_U32 = avcSeqParams->MaxBitRate;
    hucVdencBrcInitDmem->INIT_BufSize_U32 = avcSeqParams->VBVBufferSizeInBit;
    hucVdencBrcInitDmem->INIT_InitBufFull_U32 = avcSeqParams->InitVBVBufferFullnessInBit;

    if (hucVdencBrcInitDmem->INIT_InitBufFull_U32 > avcSeqParams->VBVBufferSizeInBit)
        hucVdencBrcInitDmem->INIT_InitBufFull_U32 = avcSeqParams->VBVBufferSizeInBit;

    switch (avcSeqParams->RateControlMethod)
    {
    case RATECONTROL_CBR:
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISCBR;
        break;
    case RATECONTROL_VBR:
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISVBR;
        break;
    case RATECONTROL_QVBR:
        // QVBR will use VBR BRCFlag, triggered when ICQQualityFactor > 10
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISVBR;
        break;
        // Temp solution using AVBR for low delay case, before the BRC flag is added to DDI
    case RATECONTROL_AVBR:
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISLOWDELAY;
        break;
    case RATECONTROL_ICQ:
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISICQ;
        break;
    case RATECONTROL_VCM:
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISVCM;
        break;
    default:
        break;
    }

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) // Low Delay Mode
    {
        hucVdencBrcInitDmem->INIT_BRCFlag_U16 = BRCFLAG_ISLOWDELAY;
        hucVdencBrcInitDmem->INIT_LowDelayGoldenFrameBoost_U8 = 0; //get from ?
    }

    hucVdencBrcInitDmem->INIT_FrameRateM_U32 = avcSeqParams->FramesPer100Sec;
    hucVdencBrcInitDmem->INIT_FrameRateD_U32 = 100;

    hucVdencBrcInitDmem->INIT_ProfileLevelMaxFrame_U32 = m_basicFeature->GetProfileLevelMaxFrameSize();

    if (avcSeqParams->GopRefDist && (avcSeqParams->GopPicSize > 0))
    {
        uint16_t intraPeriod               = avcSeqParams->GopPicSize - 1;
        if (intraPeriod % avcSeqParams->GopRefDist != 0)
        {
            // Implement original cmodel behavior: use only complete Gop (for intraPeriod=6, GopRefDist=4 => intraPeriod must be 4)
            intraPeriod -= intraPeriod % avcSeqParams->GopRefDist;
        }

        hucVdencBrcInitDmem->INIT_GopP_U16 = intraPeriod / avcSeqParams->GopRefDist;
        hucVdencBrcInitDmem->INIT_GopB_U16 = (avcSeqParams->GopRefDist - 1) * hucVdencBrcInitDmem->INIT_GopP_U16;

        if (avcSeqParams->GopRefDist >= avcSeqParams->GopPicSize && avcSeqParams->GopPicSize > 1)
        {
            hucVdencBrcInitDmem->INIT_ExtGopP_U16 = 1;
            hucVdencBrcInitDmem->INIT_ExtGopB_U16 = avcSeqParams->GopPicSize - 2;
        }
        else if (IsBPyramidWithGoldenBGOP()) // For now AVC support only GOP4 and GOP8 golden gops
        {
            auto    dmem     = hucVdencBrcInitDmem;
            uint8_t BGopSize = (uint8_t)avcSeqParams->GopRefDist;

            dmem->INIT_ExtGopP_U16 = (uint16_t)(intraPeriod / BGopSize);
            dmem->INIT_ExtGopB_U16 = BGopSize > 1 ? (uint16_t)dmem->INIT_ExtGopP_U16 : 0;

            uint16_t gopB1Multiplicator = (BGopSize > 2 ? 1 : 0) +
                                          (BGopSize == 4 || BGopSize > 5 ? 1 : 0);
            uint16_t gopB2Multiplicator = (BGopSize > 3 ? 1 : 0);

            dmem->INIT_ExtGopB1_U16      = dmem->INIT_ExtGopP_U16 * gopB1Multiplicator;
            dmem->INIT_ExtGopB2_U16      = (intraPeriod - dmem->INIT_ExtGopP_U16 - dmem->INIT_ExtGopB_U16 - dmem->INIT_ExtGopB1_U16) * gopB2Multiplicator;
            dmem->INIT_ExtMaxBrcLevel_U8 = dmem->INIT_ExtGopB1_U16 == 0 ? 2 :
                                          (dmem->INIT_ExtGopB2_U16 == 0 ? AvcBrcFrameType::B1_FRAME : AvcBrcFrameType::B2_FRAME);
            ENCODE_CHK_COND_RETURN(BGopSize != (1 << (dmem->INIT_ExtMaxBrcLevel_U8 - 1)),
                "'BGopSize' (GopRefDist) must be equal '1 << (dmem->INIT_MaxBrcLevel_U16 - 1)'");
        }
        else
        {
            hucVdencBrcInitDmem->INIT_ExtGopP_U16 = hucVdencBrcInitDmem->INIT_GopP_U16;
            hucVdencBrcInitDmem->INIT_ExtGopB_U16 = hucVdencBrcInitDmem->INIT_GopB_U16;

            hucVdencBrcInitDmem->INIT_ExtGopB1_U16 = hucVdencBrcInitDmem->INIT_ExtGopB2_U16 = hucVdencBrcInitDmem->INIT_ExtMaxBrcLevel_U8 = 0;
        }
    }

    if (m_basicFeature->m_minMaxQpControlEnabled)
    {
        hucVdencBrcInitDmem->INIT_MinQP_U16 = m_basicFeature->m_iMinQp;
        hucVdencBrcInitDmem->INIT_MaxQP_U16 = m_basicFeature->m_iMaxQp;
    }
    else
    {
        hucVdencBrcInitDmem->INIT_MinQP_U16 = VDENC_AVC_BRC_MIN_QP;     // Setting values from arch spec
        hucVdencBrcInitDmem->INIT_MaxQP_U16 = CODECHAL_ENCODE_AVC_MAX_SLICE_QP;  // Setting values from arch spec
    }

                                                                             //dynamic deviation thresholds
    double inputBitsPerFrame = avcSeqParams->MaxBitRate * 100.0 / avcSeqParams->FramesPer100Sec;
    double bps_ratio = inputBitsPerFrame / (avcSeqParams->VBVBufferSizeInBit * 100.0 / avcSeqParams->FramesPer100Sec/*DEV_STD_FPS*/);
    if (bps_ratio < VDENC_AVC_BPS_RATIO_LOW) bps_ratio = VDENC_AVC_BPS_RATIO_LOW;
    if (bps_ratio > VDENC_AVC_BPS_RATIO_HIGH) bps_ratio = VDENC_AVC_BPS_RATIO_HIGH;

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) // Low Delay Mode
    {
        MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_DevThreshPB0_S8, 8 * sizeof(int8_t),
                         brcSettings.BRC_LowDelay_DevThreshPB0_S8, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_DevThreshI0_S8, 8 * sizeof(int8_t),
                         brcSettings.BRC_LowDelay_DevThreshI0_S8, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_DevThreshVBR0_S8, 8 * sizeof(int8_t),
                         brcSettings.BRC_LowDelay_DevThreshVBR0_S8, 8 * sizeof(int8_t));
    }
    else
    {
        //dynamic deviation thresholds
        for (int i = 0; i < AvcVdencBrcConstSettings::m_numDevThreshlds / 2; i++)
        {
            hucVdencBrcInitDmem->INIT_DevThreshPB0_S8[i] =
                (int8_t)(VDENC_AVC_NEG_MULT_PB * pow(brcSettings.BRC_DevThreshPB0_FP_NEG[i], bps_ratio));
            hucVdencBrcInitDmem->INIT_DevThreshPB0_S8[i + AvcVdencBrcConstSettings::m_numDevThreshlds / 2] =
                (int8_t)(VDENC_AVC_POS_MULT_PB * pow(brcSettings.BRC_DevThreshPB0_FP_POS[i], bps_ratio));

            hucVdencBrcInitDmem->INIT_DevThreshI0_S8[i] =
                (int8_t)(VDENC_AVC_NEG_MULT_I * pow(brcSettings.BRC_DevThreshI0_FP_NEG[i], bps_ratio));
            hucVdencBrcInitDmem->INIT_DevThreshI0_S8[i + AvcVdencBrcConstSettings::m_numDevThreshlds / 2] =
                (int8_t)(VDENC_AVC_POS_MULT_I * pow(brcSettings.BRC_DevThreshI0_FP_POS[i], bps_ratio));

            hucVdencBrcInitDmem->INIT_DevThreshVBR0_S8[i] =
                (int8_t)(VDENC_AVC_NEG_MULT_VBR * pow(brcSettings.BRC_DevThreshVBR0_NEG[i], bps_ratio));
            hucVdencBrcInitDmem->INIT_DevThreshVBR0_S8[i + AvcVdencBrcConstSettings::m_numDevThreshlds / 2] =
                (int8_t)(VDENC_AVC_POS_MULT_VBR * pow(brcSettings.BRC_DevThreshVBR0_POS[i], bps_ratio));
        }
    }

    hucVdencBrcInitDmem->INIT_InitQPIP = (uint8_t)ComputeBRCInitQP();

    // MBBRC control
    if (m_mbBrcEnabled)
    {
        hucVdencBrcInitDmem->INIT_MbQpCtrl_U8 = 1;
        MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t),
                         brcSettings.BRC_INIT_DistQPDelta_I8, 4 * sizeof(int8_t));
    }

    hucVdencBrcInitDmem->INIT_SliceSizeCtrlEn_U8 = avcSeqParams->EnableSliceLevelRateCtrl; // Enable slice size control

    hucVdencBrcInitDmem->INIT_OscillationQpDelta_U8 =
        ((avcSeqParams->RateControlMethod == RATECONTROL_VCM) || (avcSeqParams->RateControlMethod == RATECONTROL_QVBR)) ? 16 : 0;
    hucVdencBrcInitDmem->INIT_HRDConformanceCheckDisable_U8 =
        ((avcSeqParams->RateControlMethod == RATECONTROL_VCM) || (avcSeqParams->RateControlMethod == RATECONTROL_AVBR)) ? 1 : 0;

    // Adaptive 2nd re-encode pass
    if (m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb >= ((3840 * 2160) >> 8)) // >= 4K
    {
        hucVdencBrcInitDmem->INIT_TopQPDeltaThrForAdapt2Pass_U8 = VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS_4K;
        hucVdencBrcInitDmem->INIT_BotQPDeltaThrForAdapt2Pass_U8 = VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS_4K;
        hucVdencBrcInitDmem->INIT_TopFrmSzThrForAdapt2Pass_U8 = VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS_4K;
        hucVdencBrcInitDmem->INIT_BotFrmSzThrForAdapt2Pass_U8 = VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS_4K;
    }
    else
    {
        if (avcSeqParams->RateControlMethod == RATECONTROL_AVBR)
        {
            hucVdencBrcInitDmem->INIT_TopQPDeltaThrForAdapt2Pass_U8 = VDENC_AVC_AVBR_TOPQPDELTATHRFORADAPT2PASS;
            hucVdencBrcInitDmem->INIT_BotQPDeltaThrForAdapt2Pass_U8 = VDENC_AVC_AVBR_BOTQPDELTATHRFORADAPT2PASS;
            hucVdencBrcInitDmem->INIT_TopFrmSzThrForAdapt2Pass_U8 = VDENC_AVC_AVBR_TOPFRMSZTHRFORADAPT2PASS;
            hucVdencBrcInitDmem->INIT_BotFrmSzThrForAdapt2Pass_U8 = VDENC_AVC_AVBR_BOTFRMSZTHRFORADAPT2PASS;
        }
        else
        {
            hucVdencBrcInitDmem->INIT_TopQPDeltaThrForAdapt2Pass_U8 = VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS;
            hucVdencBrcInitDmem->INIT_BotQPDeltaThrForAdapt2Pass_U8 = VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS;
            hucVdencBrcInitDmem->INIT_TopFrmSzThrForAdapt2Pass_U8 = VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS;
            hucVdencBrcInitDmem->INIT_BotFrmSzThrForAdapt2Pass_U8 = VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS;
        }
    }

    hucVdencBrcInitDmem->INIT_QPSelectForFirstPass_U8 = 1;
    hucVdencBrcInitDmem->INIT_MBHeaderCompensation_U8 = 1;
    hucVdencBrcInitDmem->INIT_DeltaQP_Adaptation_U8 = 1;
    hucVdencBrcInitDmem->INIT_MaxCRFQualityFactor_U8 = CODECHAL_ENCODE_AVC_MAX_ICQ_QUALITYFACTOR + 1;

    if (RATECONTROL_QVBR == avcSeqParams->RateControlMethod || RATECONTROL_ICQ == avcSeqParams->RateControlMethod)
    {
        hucVdencBrcInitDmem->INIT_CRFQualityFactor_U8 = (uint8_t)avcSeqParams->ICQQualityFactor;
        hucVdencBrcInitDmem->INIT_ScenarioInfo_U8 = (RATECONTROL_QVBR == avcSeqParams->RateControlMethod) ? 1 : 0;
    }

    if (m_basicFeature->m_picParam->NumDirtyROI)
    {
        hucVdencBrcInitDmem->INIT_ScenarioInfo_U8 = 1; //DISPLAYREMOTING
    }

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW) // Sliding Window BRC
    {
        hucVdencBrcInitDmem->INIT_SlidingWidowRCEnable_U8 = 1;
        if (avcSeqParams->RateControlMethod == RATECONTROL_CBR && avcSeqParams->TargetBitRate != 0)
        {
            hucVdencBrcInitDmem->INIT_SlidingWindowSize_U8         = MOS_MIN((uint8_t)avcSeqParams->SlidingWindowSize, 60);
            hucVdencBrcInitDmem->INIT_SlidingWindowMaxRateRatio_U8 = (uint8_t)((uint64_t)avcSeqParams->MaxBitRatePerSlidingWindow * 100 / avcSeqParams->TargetBitRate);
        }
        else
        {
            hucVdencBrcInitDmem->INIT_SlidingWindowSize_U8         = (uint8_t)(avcSeqParams->FramesPer100Sec / 100);
            hucVdencBrcInitDmem->INIT_SlidingWindowMaxRateRatio_U8 = 120;
        }
    }

    MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_EstRateThreshP0_U8, 7 * sizeof(uint8_t),
                     brcSettings.BRC_EstRateThreshP0_U8, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_EstRateThreshI0_U8, 7 * sizeof(uint8_t),
                     brcSettings.BRC_EstRateThreshI0_U8, 7 * sizeof(uint8_t));

    // fractional QP enable for extended rho domain
    hucVdencBrcInitDmem->INIT_FracQPEnable_U8 = (uint8_t)m_vdencItf->IsRhoDomainStatsEnabled();

    // force enabling fractional QP and disabling HRD conformance for TCBRC
    if ((avcPicParams->TargetFrameSize > 0) && (m_basicFeature->m_lookaheadDepth == 0))
    {
        hucVdencBrcInitDmem->INIT_FracQPEnable_U8 = 1;
        hucVdencBrcInitDmem->INIT_HRDConformanceCheckDisable_U8 = 1;
    }

    hucVdencBrcInitDmem->INIT_SinglePassOnly = (uint8_t)m_vdencSinglePassEnable;

    if (avcSeqParams->ScenarioInfo == ESCENARIO_GAMESTREAMING)
    {
        if (avcSeqParams->RateControlMethod == RATECONTROL_VBR)
        {
            avcSeqParams->MaxBitRate = avcSeqParams->TargetBitRate;
        }

        // Disable delta QP adaption for non-VCM/ICQ/LowDelay until we have better algorithm
        if ((avcSeqParams->RateControlMethod != RATECONTROL_VCM) &&
            (avcSeqParams->RateControlMethod != RATECONTROL_ICQ) &&
            (avcSeqParams->FrameSizeTolerance != EFRAMESIZETOL_EXTREMELY_LOW))
        {
            hucVdencBrcInitDmem->INIT_DeltaQP_Adaptation_U8 = 0;
        }

        hucVdencBrcInitDmem->INIT_New_DeltaQP_Adaptation_U8 = 1;
    }

    if (((avcSeqParams->TargetUsage & 0x07) == TARGETUSAGE_BEST_SPEED) &&
        (avcSeqParams->FrameWidth >= setting->singlePassMinFrameWidth) &&
        (avcSeqParams->FrameHeight >= setting->singlePassMinFrameHeight) &&
        (avcSeqParams->FramesPer100Sec >= setting->singlePassMinFramePer100s))
    {
        hucVdencBrcInitDmem->INIT_SinglePassOnly = true;
    }

    hucVdencBrcInitDmem->INIT_LookaheadDepth_U8 = m_basicFeature->m_lookaheadDepth;

    //Override the DistQPDelta setting
    if (m_mbBrcEnabled)
    {
        if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t),
                             brcSettings.brcInitDistQpDeltaI8LowDelay, 4 * sizeof(int8_t));
        }
        else
        {
            MOS_SecureMemcpy(hucVdencBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t),
                             brcSettings.brcInitDistQpDeltaI8, 4 * sizeof(int8_t));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::SetDmemForUpdate(void *params, uint16_t currPass, bool bIsLastPass)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    auto hucVdencBrcUpdateDmem =(VdencAvcHucBrcUpdateDmem*)params;
    static_assert(sizeof(VdencAvcHucBrcUpdateDmem) == 448, "VdencAvcHucBrcUpdateDmem size MUST be equal to 448");

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    auto brcSettings = setting->brcSettings;
    auto avcSeqParams = m_basicFeature->m_seqParam;
    auto avcPicParams = m_basicFeature->m_picParam;

    hucVdencBrcUpdateDmem->BRCFunc_U8 = 1;   // Update:1

    if (!m_brcInit && (currPass == 0))
    {
        m_brcInitPreviousTargetBufFullInBits = (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits);
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame;
        m_dBrcTargetSize += m_dBrcInitResetInputBitsPerFrame;
    }

    if (m_dBrcTargetSize > avcSeqParams->VBVBufferSizeInBit)
    {
        m_dBrcTargetSize -= avcSeqParams->VBVBufferSizeInBit;
    }

    hucVdencBrcUpdateDmem->UPD_FRAMENUM_U32           = m_basicFeature->m_frameNum;
    hucVdencBrcUpdateDmem->UPD_TARGETSIZE_U32         = (uint32_t)(m_dBrcTargetSize);
    hucVdencBrcUpdateDmem->UPD_PeakTxBitsPerFrame_U32 = (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits - m_brcInitPreviousTargetBufFullInBits);

    //Dynamic slice size control
    if (avcSeqParams->EnableSliceLevelRateCtrl)
    {
        hucVdencBrcUpdateDmem->UPD_SLCSZ_TARGETSLCSZ_U16 = (uint16_t)avcPicParams->SliceSizeInBytes; // target slice size
        hucVdencBrcUpdateDmem->UPD_TargetSliceSize_U16 = (uint16_t)avcPicParams->SliceSizeInBytes; // set max slice size to be same as target slice size
        hucVdencBrcUpdateDmem->UPD_MaxNumSliceAllowed_U16 = (uint16_t)m_basicFeature->m_maxNumSlicesAllowed;

        for (uint8_t k = 0; k < 42; k++)
        {
            hucVdencBrcUpdateDmem->UPD_SLCSZ_UPD_THRDELTAI_U16[k] = MOS_MIN(avcPicParams->SliceSizeInBytes - 150, setting->SliceSizeThrsholdsI[k + 10]);
            hucVdencBrcUpdateDmem->UPD_SLCSZ_UPD_THRDELTAP_U16[k] = MOS_MIN(avcPicParams->SliceSizeInBytes - 150, setting->SliceSizeThrsholdsP[k + 10]);
        }
    }
    else
    {
        hucVdencBrcUpdateDmem->UPD_SLCSZ_TARGETSLCSZ_U16 = 0;
        hucVdencBrcUpdateDmem->UPD_TargetSliceSize_U16 = 0;
        hucVdencBrcUpdateDmem->UPD_MaxNumSliceAllowed_U16 = 0;

        for (uint8_t k = 0; k < 42; k++)
        {
            hucVdencBrcUpdateDmem->UPD_SLCSZ_UPD_THRDELTAI_U16[k] = 0;
            hucVdencBrcUpdateDmem->UPD_SLCSZ_UPD_THRDELTAP_U16[k] = 0;
        }
    }

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW) // Sliding Window BRC
    {
        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->UPD_gRateRatioThreshold_U8, 7 * sizeof(uint8_t),
                         brcSettings.BRC_UPD_slwin_global_rate_ratio_threshold, 7 * sizeof(uint8_t));
    }
    else
    {
        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->UPD_gRateRatioThreshold_U8, 7 * sizeof(uint8_t),
                         brcSettings.BRC_UPD_global_rate_ratio_threshold, 7 * sizeof(uint8_t));
    }

    SetFrameTypeForUpdate(hucVdencBrcUpdateDmem, currPass);

    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->UPD_startGAdjFrame_U16, 4 * sizeof(uint16_t),
                     brcSettings.BRC_UPD_start_global_adjust_frame, 4 * sizeof(uint16_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->UPD_startGAdjMult_U8, 5 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_start_global_adjust_mult, 5 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->UPD_startGAdjDiv_U8, 5 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_start_global_adjust_div, 5 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->UPD_gRateRatioThresholdQP_U8, 8 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_global_rate_ratio_threshold_qp, 8 * sizeof(uint8_t));

    hucVdencBrcUpdateDmem->UPD_PAKPassNum_U8 = (uint8_t)currPass;
    hucVdencBrcUpdateDmem->UPD_MaxNumPass_U8 = m_featureManager->GetNumPass();

    hucVdencBrcUpdateDmem->UPD_AdaptiveTUEnabled = avcPicParams->AdaptiveTUEnabled;

    uint32_t numP = 0;
    if (avcSeqParams->GopRefDist && (avcSeqParams->GopPicSize > 0))
    {
        numP = (avcSeqParams->GopPicSize - 1) / avcSeqParams->GopRefDist;
    }

    for (int32_t i = 0; i < 2; i++)
    {
        hucVdencBrcUpdateDmem->UPD_SceneChgWidth_U8[i] = (uint8_t)MOS_MIN((numP + 1) / 5, 6);
    }

    hucVdencBrcUpdateDmem->UPD_SceneChgDetectEn_U8 = 1;
    hucVdencBrcUpdateDmem->UPD_SceneChgPrevIntraPctThreshold_U8 = 0x60;
    hucVdencBrcUpdateDmem->UPD_SceneChgCurIntraPctThreshold_U8 = 0xc0;

    hucVdencBrcUpdateDmem->UPD_IPAverageCoeff_U8 = (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) ? 0 : 0x80;

    hucVdencBrcUpdateDmem->UPD_CQP_FracQp_U8 = 0;

    if (avcSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        hucVdencBrcUpdateDmem->UPD_CQP_QpValue_U8 = 18;   //Cmodel suggested a few values to try: 18,20,26,30
    }
    else
    {
        hucVdencBrcUpdateDmem->UPD_CQP_QpValue_U8 = 0;
    }

    hucVdencBrcUpdateDmem->UPD_HMEDetectionEnable_U8 = 0;

    if (FRAME_SKIP_NORMAL == m_basicFeature->m_skipFrameFlag)
    {
        hucVdencBrcUpdateDmem->UPD_SkipFrameSize_U16      = (uint16_t)avcPicParams->SizeSkipFrames;
        hucVdencBrcUpdateDmem->UPD_NumOfFramesSkipped_U16 = (uint16_t)avcPicParams->NumSkipFrames;
    }
    else
    {
        hucVdencBrcUpdateDmem->UPD_SkipFrameSize_U16      = 0;
        hucVdencBrcUpdateDmem->UPD_NumOfFramesSkipped_U16 = 0;
    }

    // HMECost enabled by default in CModel V11738+
    hucVdencBrcUpdateDmem->UPD_HMECostEnable_U8 = 1;

    // ROI and static region pct parameters
    // must be zero if they are not used
    hucVdencBrcUpdateDmem->UPD_RoiQpViaForceQp_U8 = 0;
    hucVdencBrcUpdateDmem->UPD_StaticRegionPct_U16 = 0;
    hucVdencBrcUpdateDmem->UPD_ROISource_U8 = 0;
    if (avcPicParams->NumROI)
    {
        ENCODE_CHK_COND_RETURN(avcPicParams->NumROIDistinctDeltaQp > sizeof(hucVdencBrcUpdateDmem->UPD_ROIQpDelta_I8) - 1, "Number of ROI is greater that dmem roi array size");

        hucVdencBrcUpdateDmem->UPD_RoiQpViaForceQp_U8 = avcPicParams->bNativeROI ? 0 : 1;
        for (uint8_t i = 0; i < avcPicParams->NumROIDistinctDeltaQp; i++)
        {
            hucVdencBrcUpdateDmem->UPD_ROIQpDelta_I8[i + 1] = avcPicParams->ROIDistinctDeltaQp[i];
        }
    }
    else if (avcPicParams->NumDirtyROI)
    {
        //hucVdencBrcUpdateDmem->UPD_StaticRegionPct_U16 = (uint16_t)m_vdencStaticRegionPct;
        if (m_mbBrcEnabled)
        {
            hucVdencBrcUpdateDmem->UPD_ROISource_U8 = 2;
        }
    }

    hucVdencBrcUpdateDmem->UPD_SLBB_Size_U16 = (uint16_t)MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_CACHELINE_SIZE);

    hucVdencBrcUpdateDmem->UPD_WidthInMB_U16  = m_basicFeature->m_picWidthInMb;
    hucVdencBrcUpdateDmem->UPD_HeightInMB_U16 = m_basicFeature->m_picHeightInMb;

    hucVdencBrcUpdateDmem->MOTION_ADAPTIVE_G4 = (avcSeqParams->ScenarioInfo == ESCENARIO_GAMESTREAMING) || ((avcPicParams->TargetFrameSize > 0) && (m_basicFeature->m_lookaheadDepth == 0));  // GS or TCBRC
    hucVdencBrcUpdateDmem->UPD_CQMEnabled_U8  = avcSeqParams->seq_scaling_matrix_present_flag || avcPicParams->pic_scaling_matrix_present_flag;

    hucVdencBrcUpdateDmem->UPD_LA_TargetSize_U32    = avcPicParams->TargetFrameSize << 3;
    hucVdencBrcUpdateDmem->UPD_TCBRC_SCENARIO_U8    = 0;  // Temporal fix because of DDI flag deprication. Use Cloud Gaming mode by default
    hucVdencBrcUpdateDmem->UPD_NumSlicesForRounding = GetAdaptiveRoundingNumSlices();
    hucVdencBrcUpdateDmem->UPD_UserMaxFramePB       = avcSeqParams->UserMaxPBFrameSize;

    if (avcSeqParams->LookaheadDepth == 0 && avcPicParams->TargetFrameSize > 0 &&
        (hucVdencBrcUpdateDmem->UPD_TCBRC_SCENARIO_U8 == 2 || avcSeqParams->UserMaxPBFrameSize <= avcPicParams->TargetFrameSize * 2))
    {
        if (avcSeqParams->UserMaxPBFrameSize == 0)
        {
            hucVdencBrcUpdateDmem->UPD_UserMaxFramePB = 2 * avcPicParams->TargetFrameSize;
        }

        uint32_t inputbitsperframe = avcPicParams->TargetFrameSize << 3;

        if (avcPicParams->CodingType == I_TYPE)
        {
            hucVdencBrcUpdateDmem->UPD_LA_TargetSize_U32 = inputbitsperframe + inputbitsperframe / 8;
        }
        else if (abs((int)(8 * hucVdencBrcUpdateDmem->UPD_UserMaxFramePB - inputbitsperframe)) <= 8)
        {
            hucVdencBrcUpdateDmem->UPD_LA_TargetSize_U32 = inputbitsperframe * 9 / 10;
        }
        else
        {
            hucVdencBrcUpdateDmem->UPD_LA_TargetSize_U32 = inputbitsperframe;
        }
    }

    if (m_basicFeature->m_lookaheadDepth > 0)
    {
        DeltaQPUpdate(avcPicParams->QpModulationStrength, bIsLastPass);
        hucVdencBrcUpdateDmem->EnableLookAhead          = 1;
#if (_DEBUG || _RELEASE_INTERNAL)
        hucVdencBrcUpdateDmem->UPD_LA_TargetFulness_U32 = m_useBufferFulnessData ? m_bufferFulnessData_csv[m_basicFeature->m_frameNum % m_bufferFulnessDataSize] : m_basicFeature->m_targetBufferFulness;
#else
        hucVdencBrcUpdateDmem->UPD_LA_TargetFulness_U32 = m_basicFeature->m_targetBufferFulness;
#endif
        hucVdencBrcUpdateDmem->UPD_Delta_U8             = m_qpModulationStrength;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::FillHucConstData(uint8_t *data, uint8_t pictureType)
{
    auto hucConstData = (VdencAvcHucBrcConstantData*)data;

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    auto brcSettings = setting->brcSettings;
    auto avcSeqParams = m_basicFeature->m_seqParam;

    MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabI_U8, 64 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_GlobalRateQPAdjTabI_U8, 64 * sizeof(uint8_t));
    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW)  // Sliding Window BRC
    {
        MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t),
                         brcSettings.BRC_UPD_SlWinGlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t),
                         brcSettings.BRC_UPD_GlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t));
    }
    MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabB_U8, 64 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_GlobalRateQPAdjTabB_U8, 64 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_DistThreshldI_U8, 10 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_DistThreshldI_U8, 10 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_DistThreshldP_U8, 10 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_DistThreshldP_U8, 10 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_DistThreshldB_U8, 10 * sizeof(uint8_t),
                     brcSettings.BRC_UPD_DistThreshldB_U8, 10 * sizeof(uint8_t));

    if (avcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabI_U8, 81 * sizeof(uint8_t),
                         brcSettings.CBR_UPD_DistQPAdjTabI_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabP_U8, 81 * sizeof(uint8_t),
                         brcSettings.CBR_UPD_DistQPAdjTabP_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabB_U8, 81 * sizeof(uint8_t),
                         brcSettings.CBR_UPD_DistQPAdjTabB_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabI_S8, 72 * sizeof(uint8_t),
                         brcSettings.CBR_UPD_FrmSzAdjTabI_S8, 72 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t),
                         brcSettings.CBR_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabB_S8, 72 * sizeof(uint8_t),
                         brcSettings.CBR_UPD_FrmSzAdjTabB_S8, 72 * sizeof(int8_t));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabI_U8, 81 * sizeof(uint8_t),
                         brcSettings.VBR_UPD_DistQPAdjTabI_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabP_U8, 81 * sizeof(uint8_t),
                         brcSettings.VBR_UPD_DistQPAdjTabP_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabB_U8, 81 * sizeof(uint8_t),
                         brcSettings.VBR_UPD_DistQPAdjTabB_U8, 81 * sizeof(int8_t));

        if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)  // Low Delay Mode
        {
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabI_S8, 72 * sizeof(uint8_t),
                             brcSettings.LOW_DELAY_UPD_FrmSzAdjTabI_S8, 72 * sizeof(int8_t));
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t),
                             brcSettings.LOW_DELAY_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabB_S8, 72 * sizeof(uint8_t),
                             brcSettings.LOW_DELAY_UPD_FrmSzAdjTabB_S8, 72 * sizeof(int8_t));
        }
        else
        {
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabI_S8, 72 * sizeof(uint8_t),
                             brcSettings.VBR_UPD_FrmSzAdjTabI_S8, 72 * sizeof(int8_t));

            if (avcSeqParams->RateControlMethod == RATECONTROL_QVBR)
            {
                MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t),
                                 brcSettings.QVBR_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
            }
            else
            {
                MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t),
                                 brcSettings.VBR_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
            }

            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabB_S8, 72 * sizeof(uint8_t),
                             brcSettings.VBR_UPD_FrmSzAdjTabB_S8, 72 * sizeof(int8_t));
        }
    }

    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMinTabP_U8, 9 * sizeof(uint8_t), brcSettings.BRC_UPD_FrmSzMinTabP_U8, 9 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMinTabI_U8, 9 * sizeof(uint8_t), brcSettings.BRC_UPD_FrmSzMinTabI_U8, 9 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMaxTabP_U8, 9 * sizeof(uint8_t), brcSettings.BRC_UPD_FrmSzMaxTabP_U8, 9 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMaxTabI_U8, 9 * sizeof(uint8_t), brcSettings.BRC_UPD_FrmSzMaxTabI_U8, 9 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_FrmSzSCGTabP_U8, 9 * sizeof(uint8_t), brcSettings.BRC_UPD_FrmSzSCGTabP_U8, 9 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_FrmSzSCGTabI_U8, 9 * sizeof(uint8_t), brcSettings.BRC_UPD_FrmSzSCGTabI_U8, 9 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_I_IntraNonPred, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_I_IntraNonPred, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_I_Intra8x8, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_I_Intra8x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_I_Intra4x4, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_I_Intra4x4, 42 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_P_IntraNonPred, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_IntraNonPred, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Intra16x16, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_Intra16x16, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Intra8x8, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_Intra8x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Intra4x4, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_Intra4x4, 42 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_P_Inter16x8, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_Inter16x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Inter8x8, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_Inter8x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Inter16x16, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_Inter16x16, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_RefId, 42 * sizeof(uint8_t), brcSettings.BRC_UPD_P_RefId, 42 * sizeof(uint8_t));

    ENCODE_CHK_STATUS_RETURN(LoadConstTable0(hucConstData->VdencAvcHucBrcConstantData_0));
    ENCODE_CHK_STATUS_RETURN(LoadConstTable3(pictureType, hucConstData->VdencAvcHucBrcConstantData_1));
    ENCODE_CHK_STATUS_RETURN(LoadConstTable5(pictureType, hucConstData->VdencAvcHucBrcConstantData_2));
    ENCODE_CHK_STATUS_RETURN(LoadConstTable6(pictureType, hucConstData->VdencAvcHucBrcConstantData_3));
    ENCODE_CHK_STATUS_RETURN(LoadConstTable7(pictureType, hucConstData->VdencAvcHucBrcConstantData_4));
    ENCODE_CHK_STATUS_RETURN(LoadConstTable8(pictureType, hucConstData->VdencAvcHucBrcConstantData_5));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::SaveBrcUpdateDmemBufferPtr(
    PMOS_RESOURCE vdencBrcUpdateDmemBuffer0,
    PMOS_RESOURCE vdencBrcUpdateDmemBuffer1)
{
    ENCODE_FUNC_CALL();

    m_vdencBrcUpdateDmemBufferPtr[0] = vdencBrcUpdateDmemBuffer0;
    m_vdencBrcUpdateDmemBufferPtr[1] = vdencBrcUpdateDmemBuffer1;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::SaveHucStatus2Buffer(PMOS_RESOURCE hucStatus2Buffer)
{
    ENCODE_FUNC_CALL();

    m_hucStatus2BufferPtr = hucStatus2Buffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::SetMfcStatusParams(EncodeStatusReadParams &params)
{
    ENCODE_FUNC_CALL();

    if (m_vdencBrcEnabled)
    {
        params.vdencBrcEnabled                = true;
        params.vdencBrcNumOfSliceOffset       = CODECHAL_OFFSETOF(VdencAvcHucBrcUpdateDmem, NumOfSlice);
        params.resVdencBrcUpdateDmemBufferPtr = m_vdencBrcUpdateDmemBufferPtr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::LoadConstTable0(uint8_t constTable[8][42])
{
    ENCODE_FUNC_CALL();

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    const auto &constTable1 = settings->vdencCMD3Table->AvcVdencCMD3ConstSettings_0;

    for (auto i = 0; i < 8; i++)
    {
        for (auto j = 0; j < 42; j++)
        {
            constTable[i][j] = constTable1[i][j + 10];
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::LoadConstTable3(uint8_t pictureType, uint8_t ConstTable3[42])
{
    ENCODE_FUNC_CALL();

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    const auto &  table      = settings->vdencCMD3Table->AvcVdencCMD3ConstSettings_3;
    const uint8_t codingType = pictureType + 1;

    for (auto i = 0; i < 42; i++)
    {
        if (codingType == I_TYPE || codingType == P_TYPE)
        {
            ConstTable3[i] = table[pictureType][i + 10];
        }
        else  // B and Ref B
        {
            ConstTable3[i] = 14;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::LoadConstTable5(uint8_t pictureType, uint16_t ConstTable5[42])
{
    ENCODE_FUNC_CALL();

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    const auto & table = settings->vdencCMD3Table->AvcVdencCMD3ConstSettings_5;
    uint8_t    isIPGOP = m_basicFeature->m_seqParam->GopRefDist == 1 ? 1 : 0;
    for (int8_t qp = 10; qp < 52; ++qp)
    {
        uint8_t idx        = uint8_t(qp > 12 ? qp - 12 : 0);
        ConstTable5[qp-10] = table[pictureType][isIPGOP][idx];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::LoadConstTable6(uint8_t pictureType, uint16_t ConstTable6[42])
{
    ENCODE_FUNC_CALL();

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    const auto & table = settings->vdencCMD3Table->AvcVdencCMD3ConstSettings_6;
    uint8_t    isIPGOP = m_basicFeature->m_seqParam->GopRefDist == 1 ? 1 : 0;
    for (int8_t qp = 10; qp < 52; ++qp)
    {
        uint8_t idx        = uint8_t(qp > 12 ? qp - 12 : 0);
        ConstTable6[qp-10] = table[pictureType][isIPGOP][idx];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::LoadConstTable7(uint8_t pictureType, uint8_t ConstTable7[42])
{
    ENCODE_FUNC_CALL();

    const uint8_t codingType = pictureType + 1;
    if (codingType == I_TYPE)
        return MOS_STATUS_SUCCESS;

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    uint8_t isIPGOP = m_basicFeature->m_seqParam->GopRefDist == 1;
    auto    index   = (codingType == P_TYPE) ? (isIPGOP ? 3 : 2) : (codingType == B_TYPE ? 0 : 1);

    const auto &table   = settings->AdaptiveInterRounding[index];
    for (int8_t qp = 10; qp < 52; ++qp)
    {
        ConstTable7[qp - 10] = table[qp];
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeBRC::LoadConstTable8(uint8_t pictureType, uint8_t ConstTable8[42])
{
    ENCODE_FUNC_CALL();

    const uint8_t codingType = pictureType + 1;
    if (codingType == I_TYPE)
        return MOS_STATUS_SUCCESS;

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    uint8_t isIPGOP = m_basicFeature->m_seqParam->GopRefDist == 1;
    auto    index   = (codingType == P_TYPE) ? (isIPGOP ? 3 : 2) : (codingType == B_TYPE ? 0 : 1);

    const auto &table = settings->AdaptiveIntraRounding[index];
    for (int8_t qp = 10; qp < 52; ++qp)
    {
        ConstTable8[qp - 10] = table[qp];
    }

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS AvcEncodeBRC::SetSequenceStructs()
{
    ENCODE_FUNC_CALL();

    auto seqParams = m_basicFeature->m_seqParam;

    m_brcInit         = m_basicFeature->m_resolutionChanged;
    m_vdencBrcEnabled = IsVdencBrcSupported(seqParams);
    m_rcMode          = m_vdencBrcEnabled ? seqParams->RateControlMethod : 0;

    if (IsRateControlBrc(seqParams->RateControlMethod) && m_vdencBrcEnabled == 0)
    {
        // Error propagation
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // control MBBRC if the user feature key does not exist
    if (m_vdencBrcEnabled && !m_mbBrcUserFeatureKeyControl)
    {
        SetMbBrc();
    }

    // BRC Init or Reset
    if (seqParams->bInitBRC)
    {
        m_brcInit = seqParams->bInitBRC;
    }
    else
    {
        m_brcReset = seqParams->bResetBRC;
    }

    if (seqParams->RateControlMethod == RATECONTROL_ICQ || seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (seqParams->ICQQualityFactor < ENCODE_AVC_VDENC_MIN_ICQ_QUALITYFACTOR ||
            seqParams->ICQQualityFactor > CODECHAL_ENCODE_AVC_MAX_ICQ_QUALITYFACTOR)
        {
            ENCODE_ASSERTMESSAGE("Invalid ICQ Quality Factor input\n");
            seqParams->ICQQualityFactor = (uint16_t)CodecHal_Clip3(ENCODE_AVC_VDENC_MIN_ICQ_QUALITYFACTOR,
                CODECHAL_ENCODE_AVC_MAX_ICQ_QUALITYFACTOR,
                seqParams->ICQQualityFactor);
        }
    }

    return MOS_STATUS_SUCCESS;
}

void AvcEncodeBRC::SetMbBrc()
{
    ENCODE_FUNC_CALL();

    auto seqParams = m_basicFeature->m_seqParam;
    if (seqParams->RateControlMethod == RATECONTROL_ICQ || seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        // If the rate control method is ICQ or QVBR then enable MBBRC by default for all TUs and ignore the app input
        m_mbBrcEnabled = true;
        ENCODE_NORMALMESSAGE("MBBRC enabled with rate control = %d", seqParams->RateControlMethod);
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        // If the rate control method is VCM then disable MBBRC by default for all TUs and ignore the app input
        m_mbBrcEnabled = false;
    }
    else
    {
        switch (seqParams->MBBRC)
        {
        case mbBrcInternal:
            m_mbBrcEnabled = true;
            break;
        case mbBrcDisabled:
            m_mbBrcEnabled = false;
            break;
        case mbBrcEnabled:
            m_mbBrcEnabled = true;
            break;
        }
    }
}

MOS_STATUS AvcEncodeBRC::FreeBrcResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

    if (m_batchBufferForVdencImgStat.iSize)
    {
        ENCODE_CHK_STATUS_RETURN(Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_batchBufferForVdencImgStat, nullptr));
    }

    return eStatus;
}

int32_t AvcEncodeBRC::ComputeBRCInitQP()
{
    ENCODE_FUNC_CALL();

    const float x0 = 0, y0 = 1.19f, x1 = 1.75f, y1 = 1.75f;
    uint32_t    frameSize;
    int32_t     QP, deltaQ;

    auto seqParams = m_basicFeature->m_seqParam;

    // InitQPIP calculation
    frameSize = ((m_basicFeature->m_frameWidth * m_basicFeature->m_frameHeight * 3) >> 1);
    QP        = (int32_t)(1. / 1.2 * pow(10.0, (log10(frameSize * 2. / 3. * ((float)seqParams->FramesPer100Sec) / ((float)(seqParams->TargetBitRate) * 100)) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);
    QP += 2;
    //add additional change based on buffer size. It is especially useful for low delay
    deltaQ = (int32_t)(9 - (seqParams->VBVBufferSizeInBit * ((float)seqParams->FramesPer100Sec) / ((float)(seqParams->TargetBitRate) * 100)));
    QP += deltaQ < 0 ? 0 : deltaQ;
    QP = CodecHal_Clip3(ENCODE_AVC_BRC_MIN_QP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP, QP);
    QP--;
    if (QP < 0)
        QP = 1;

    return QP;
}

MOS_STATUS AvcEncodeBRC::DeltaQPUpdate(uint8_t qpModulationStrength, bool bIsLastPass)
{
    ENCODE_FUNC_CALL();

    uint8_t qpStrength = (uint8_t)(qpModulationStrength + (qpModulationStrength >> 1));
    if (!m_isFirstDeltaQPCalculation)
    {
        if (qpModulationStrength == 0)
        {
            m_qpModulationStrength = 0;
        }
        else
        {
            m_qpModulationStrength = (m_qpModulationStrength + qpStrength + 1) >> 1;
        }
    }
    else
    {
        m_qpModulationStrength = qpStrength;
        if (bIsLastPass)
        {
            m_isFirstDeltaQPCalculation = false;
        }
    }

    return MOS_STATUS_SUCCESS;
}

void AvcEncodeBRC::SetFrameTypeForUpdate(VdencAvcHucBrcUpdateDmem *dmem, uint16_t currPass)
{
    dmem->UPD_CurrFrameType_U8 = (m_basicFeature->m_pictureCodingType + 1) % 3;  // I:2, P:0, B:1. Same values in AvcBrcFrameType
    if (dmem->UPD_CurrFrameType_U8 == 1 && m_basicFeature->m_picParam->RefPicFlag == 1)
    {
        dmem->UPD_CurrFrameType_U8 = 3;  // separated type for reference B for legacy BRC
    }

    if (m_basicFeature->m_pictureCodingType == I_TYPE || m_basicFeature->m_pictureCodingType == P_TYPE)
    {
        m_frameIdxInBGop = 0;
    }
    else if (currPass == 0)
    {
        ++m_frameIdxInBGop;
    }

    //
    // Calculate correct ExtCurrFrameType only for Golden GOPs
    //
    // Calculate B-frame type based on hierarchy level for B-pyramid
    if (IsBPyramidWithGoldenBGOP() && m_basicFeature->m_pictureCodingType == B_TYPE)
    {
        uint16_t curOrder = 0, curLvlInBGop = 0;
        CalculateCurLvlInBGop(m_frameIdxInBGop, 1, m_basicFeature->m_seqParam->GopRefDist, 0, curOrder, curLvlInBGop);

        dmem->UPD_ExtCurrFrameType = curLvlInBGop == 1 ? AvcBrcFrameType::B1_FRAME : (curLvlInBGop == 2 ? AvcBrcFrameType::B2_FRAME : AvcBrcFrameType::B_FRAME);
    }
}

bool AvcEncodeBRC::IsVdencBrcSupported(
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams)
{
    ENCODE_FUNC_CALL();

    bool vdencBrcSupported = false;

    // HuC based BRC should be used when 1) BRC is requested by App and 2) HuC FW is loaded and HuC is enabled for use.
    if (IsRateControlBrc(avcSeqParams->RateControlMethod))
    {
        if (!MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEnableMediaKernels))
        {
            ENCODE_ASSERTMESSAGE("Failed to load HuC firmware!");
        }

        vdencBrcSupported = MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEnableMediaKernels);
    }

    // Simple check for BRC parameters; if error, disable BRC and continue encoding
    if ((vdencBrcSupported) &&
        (avcSeqParams->RateControlMethod != RATECONTROL_ICQ) &&
        (((!avcSeqParams->InitVBVBufferFullnessInBit || !avcSeqParams->VBVBufferSizeInBit || !avcSeqParams->MaxBitRate) &&
           (avcSeqParams->RateControlMethod != RATECONTROL_AVBR)) ||
           !avcSeqParams->TargetBitRate ||
           !avcSeqParams->FramesPer100Sec))
    {
        ENCODE_ASSERTMESSAGE("Fatal error in AVC Encoding BRC parameters.");
        ENCODE_ASSERTMESSAGE("RateControlMethod = %d, InitVBVBufferFullnessInBit = %d, VBVBufferSizeInBit = %d, MaxBitRate = %d, TargetBitRate = %d, FramesPer100Sec = %d",
            avcSeqParams->RateControlMethod,
            avcSeqParams->InitVBVBufferFullnessInBit,
            avcSeqParams->VBVBufferSizeInBit,
            avcSeqParams->MaxBitRate,
            avcSeqParams->TargetBitRate,
            avcSeqParams->FramesPer100Sec);
        vdencBrcSupported = false;
    }

    return vdencBrcSupported;
}

uint32_t AvcEncodeBRC::GetVdencBRCImgStateBufferSize()
{
    return MOS_ALIGN_CEIL(MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_CACHELINE_SIZE) +
                              CODECHAL_ENCODE_AVC_MAX_SLICES_SUPPORTED * GetVdencOneSliceStateSize(),
        CODECHAL_PAGE_SIZE);
}

uint32_t AvcEncodeBRC::GetVdencOneSliceStateSize()
{
    return m_mfxItf->MHW_GETSIZE_F(MFX_AVC_SLICE_STATE)() +
           m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_SLICE_STATE)() +
           m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
}

bool AvcEncodeBRC::IsBPyramidWithGoldenBGOP()
{
    return
          // Add HierarchicalFlag to condition to not use hierarchy B-Frames when HierarchicalFlag==0
          // m_basicFeature->m_seqParam->HierarchicalFlag &&
            (m_basicFeature->m_seqParam->GopRefDist == 2 ||
             m_basicFeature->m_seqParam->GopRefDist == 4 ||
             m_basicFeature->m_seqParam->GopRefDist == 8);
}

void AvcEncodeBRC::CalculateCurLvlInBGop(uint16_t curFrameIdxInBGop, uint16_t begin, uint16_t end, uint16_t curLvl, uint16_t &curOrder, uint16_t &retLvl)
{
    curOrder += 1;
    if (curOrder == curFrameIdxInBGop)
    {
        retLvl = curLvl;
        return;
    }

    if (end - begin > 1)
    {
        uint16_t pivot = (begin + end) >> 1;
        CalculateCurLvlInBGop(curFrameIdxInBGop, begin, pivot, curLvl + 1, curOrder, retLvl);

        if (pivot + 1 != end)
            CalculateCurLvlInBGop(curFrameIdxInBGop, pivot + 1, end, curLvl + 1, curOrder, retLvl);
    }
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcEncodeBRC)
{
    params.frameStatisticsStreamOut = m_vdencBrcEnabled || m_basicFeature->m_picParam->StatusReportEnable.fields.FrameStats;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, AvcEncodeBRC)
{
    if (params.function == BRC_UPDATE)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        // Output regions
        params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, m_basicFeature->m_frameNum);
        params.regionParams[0].isWritable = true;
        params.regionParams[6].presRegion = const_cast<PMOS_RESOURCE>(&m_batchBufferForVdencImgStat.OsResource);
        params.regionParams[6].isWritable = true;

        // region 15 always in clear
        params.regionParams[15].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcDebugBuffer, 0);
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
