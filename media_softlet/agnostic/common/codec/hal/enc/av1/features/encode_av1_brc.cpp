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
//! \file     encode_av1_brc.cpp
//! \brief    Defines the common interface for av1 brc features
//!
#include "encode_av1_basic_feature.h"
#include "encode_av1_brc.h"
#include "encode_av1_vdenc_feature_manager.h"
#include "encode_av1_vdenc_const_settings.h"
#include "encode_av1_brc_init_packet.h"
#include "encode_av1_brc_update_packet.h"
namespace encode
{
    Av1Brc::Av1Brc(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
        m_hwInterface(hwInterface),
        m_allocator(allocator)
    {
        m_featureManager = featureManager;

        ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);
        m_basicFeature = dynamic_cast<Av1BasicFeature*>(featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    }

    Av1Brc::~Av1Brc()
    {
        FreeBrcResources();
    }

    MOS_STATUS Av1Brc::Init(void *setting)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Brc::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;

        // If new seq, need to set sequence structs
        // If RATECONTROL_CQL, need to update quality and bitrate .etc
        if (encodeParams->bNewSeq || m_basicFeature->m_av1SeqParams->RateControlMethod == RATECONTROL_CQL)
        {
            ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
        }

        const auto& seqParams = *m_basicFeature->m_av1SeqParams;

#if (_DEBUG || _RELEASE_INTERNAL)
        ReportUserSetting(
            m_userSettingPtr,
            "Encode RateControl Method",
            m_rcMode,
            MediaUserSetting::Group::Sequence);
        
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Adaptive TU Enable",
            MediaUserSetting::Group::Sequence);
        m_basicFeature->m_av1PicParams->AdaptiveTUEnabled |= outValue.Get<uint8_t>(); 
#endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Brc::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // BRC history buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_brcHistoryBufSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC History Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(VdencBRCHistoryBuffer, allocParamsForBufferLinear, 1);

        // VDENC BRC PAK MMIO buffer
        allocParamsForBufferLinear.dwBytes = sizeof(Av1BrcPakMmio);
        allocParamsForBufferLinear.pBufName = "VDENC BRC PAK MMIO Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcPakMmioBuffer, allocParamsForBufferLinear, 1);

        // BRC HuC Data Buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(BRC_DATA_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "BRC HuC Data Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        MOS_RESOURCE* brcDataBuffer = m_allocator->AllocateResource(
            allocParamsForBufferLinear,
            true);
        ENCODE_CHK_NULL_RETURN(brcDataBuffer);
        m_resBrcDataBuffer = *brcDataBuffer;

        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            // VDENC uses second level batch buffer
            MOS_ZeroMemory(&m_vdenc2ndLevelBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));
            m_vdenc2ndLevelBatchBuffer[j].bSecondLevel = true;
            ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_hwInterface->GetOsInterface(),
                &m_vdenc2ndLevelBatchBuffer[j],
                nullptr,
                m_hwInterface->m_vdenc2ndLevelBatchBufferSize));

            // VDENC uses second level batch buffer
            MOS_ZeroMemory(&m_pakInsertOutputBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));
            m_pakInsertOutputBatchBuffer[j].bSecondLevel = true;
            ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_hwInterface->GetOsInterface(),
                &m_pakInsertOutputBatchBuffer[j],
                nullptr,
                CODECHAL_PAGE_SIZE));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Brc::GetBrcDataBuffer(MOS_RESOURCE *&buffer)
    {
        ENCODE_FUNC_CALL();
        buffer = &m_resBrcDataBuffer;
        return MOS_STATUS_SUCCESS;
    }

    inline uint8_t MapTCBRCScenarioInfo(ENCODE_SCENARIO& scenarioInfo)
    {
        uint8_t TCBRCScenarioInfo = 0;
        switch (scenarioInfo)
        {
        case ESCENARIO_REMOTEGAMING:
            TCBRCScenarioInfo = 0;
            break;
        case ESCENARIO_VIDEOCONFERENCE:
            TCBRCScenarioInfo = 1;
            break;
        default:
            ENCODE_ASSERTMESSAGE("TCBRC mode not supported!");
            break;
        }
        return TCBRCScenarioInfo;
    }

    MOS_STATUS Av1Brc::SetDmemForUpdate(VdencAv1HucBrcUpdateDmem *dmem) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(dmem);

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);
        auto seqParams = m_basicFeature->m_av1SeqParams;
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
        auto picParams = m_basicFeature->m_av1PicParams;
        auto       setting = static_cast<Av1VdencFeatureSettings*>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

        // BRC update setting
        if (m_curTargetFullness > seqParams->VBVBufferSizeInBit && dmem->UPD_PAKPassNum == 0)
        {
            dmem->UPD_OverflowFlag = 0x1;
            m_curTargetFullness -= seqParams->VBVBufferSizeInBit;
        }

        dmem->UPD_TARGET_BUF_FULLNESS = (uint32_t)m_curTargetFullness;
        dmem->UPD_CDF_BufferSize = MOS_ALIGN_CEIL(m_basicFeature->m_cdfMaxNumBytes, CODECHAL_CACHELINE_SIZE);
        dmem->UPD_FRAMENUM = m_basicFeature->m_frameNum;    // frame number
        dmem->UPD_HRD_BUFF_FULLNESS = m_delay;
        dmem->UPD_HRD_BUFF_FULLNESS_LOWER = seqParams->LowerVBVBufferLevelThresholdInBit;
        dmem->UPD_HRD_BUFF_FULLNESS_UPPER = seqParams->UpperVBVBufferLevelThresholdInBit;

        auto CalculatedMaxFrame = m_basicFeature->m_frameHeight * m_basicFeature->m_frameWidth;
        dmem->UPD_UserMaxFrame   = seqParams->UserMaxIFrameSize  > 0 ? MOS_MIN(seqParams->UserMaxIFrameSize, CalculatedMaxFrame) : CalculatedMaxFrame;
        dmem->UPD_UserMaxFramePB = seqParams->UserMaxPBFrameSize > 0 ? MOS_MIN(seqParams->UserMaxPBFrameSize, CalculatedMaxFrame) : CalculatedMaxFrame;

        dmem->UPD_CurWidth  = (uint16_t)m_basicFeature->m_oriFrameWidth;
        dmem->UPD_CurHeight = (uint16_t)m_basicFeature->m_oriFrameHeight;
        dmem->UPD_Asyn = 0;
        dmem->UPD_EnableAdaptiveRounding = (m_basicFeature->m_roundingMethod == RoundingMethod::adaptiveRounding);
        dmem->UPD_AdaptiveTUEnabled = picParams->AdaptiveTUEnabled;

        if (seqParams->GopRefDist == 16 && m_rcMode == RATECONTROL_CQL)
            dmem->UPD_MaxBRCLevel = 4;
        else if (seqParams->GopRefDist == 8)
            dmem->UPD_MaxBRCLevel = 3;
        else if (seqParams->GopRefDist == 4)
            dmem->UPD_MaxBRCLevel = 2;
        else if (seqParams->GopRefDist == 2)
            dmem->UPD_MaxBRCLevel = 1;
        else
            dmem->UPD_MaxBRCLevel = 0;

        bool bAllowedPyramid = seqParams->GopRefDist != 3;

        if (m_basicFeature->m_pictureCodingType == I_TYPE)
        {
            dmem->UPD_CurrFrameType = AV1_BRC_FRAME_TYPE_I;
        }
        else if (seqParams->SeqFlags.fields.HierarchicalFlag && bAllowedPyramid)
        {
            if (picParams->HierarchLevelPlus1 > 0)
            {
                std::map<int, AV1_BRC_FRAME_TYPE> hierchLevelPlus1_to_brclevel{
                    {1, AV1_BRC_FRAME_TYPE_P_OR_LB},
                    {2, AV1_BRC_FRAME_TYPE_B},
                    {3, AV1_BRC_FRAME_TYPE_B1},
                    {4, AV1_BRC_FRAME_TYPE_B2},
                    {5, AV1_BRC_FRAME_TYPE_B3}};
                dmem->UPD_CurrFrameType = hierchLevelPlus1_to_brclevel.count(picParams->HierarchLevelPlus1) ? hierchLevelPlus1_to_brclevel[picParams->HierarchLevelPlus1] : AV1_BRC_FRAME_TYPE_INVALID;
                //Invalid HierarchLevelPlus1 or LBD frames at level 3 eror check.
                if ((dmem->UPD_CurrFrameType == AV1_BRC_FRAME_TYPE_INVALID) ||
                    (m_basicFeature->m_ref.IsLowDelay() && dmem->UPD_CurrFrameType == AV1_BRC_FRAME_TYPE_B2))
                {
                    ENCODE_VERBOSEMESSAGE("AV1_BRC_FRAME_TYPE_INVALID or LBD picture doesn't support Level 4\n");
                    dmem->UPD_CurrFrameType = AV1_BRC_FRAME_TYPE_B2;
                }
            }
            else
            {
                dmem->UPD_CurrFrameType = AV1_BRC_FRAME_TYPE_P_OR_LB;  //No Hierarchical info, treated as flat case
            }
        }
        else
        {
            dmem->UPD_CurrFrameType = m_basicFeature->m_ref.IsLowDelay() ? AV1_BRC_FRAME_TYPE_P_OR_LB : AV1_BRC_FRAME_TYPE_B;
        }

#define MEMCPY_CONST(D, S) MOS_SecureMemcpy(dmem->D, sizeof(dmem->D), brcSettings.S.data, brcSettings.S.size);
        MEMCPY_CONST(UPD_startGAdjFrame, startGlobalAdjustFrame);
        MEMCPY_CONST(UPD_QPThreshold, QPThresholds);
        MEMCPY_CONST(UPD_gRateRatioThreshold, globalRateRatioThreshold);
        MEMCPY_CONST(UPD_gRateRatioThresholdQP, globalRateRatioThresholdQP);
        MEMCPY_CONST(UPD_startGAdjMult, startGlobalAdjustMult);
        MEMCPY_CONST(UPD_startGAdjDiv, startGlobalAdjustDiv);
        MEMCPY_CONST(UPD_DistThreshldI, distortionThresh);
        MEMCPY_CONST(UPD_DistThreshldP, distortionThresh);
        MEMCPY_CONST(UPD_DistThreshldB, distortionThreshB);
        MEMCPY_CONST(UPD_MaxFrameThreshI, maxFrameMultI);
        MEMCPY_CONST(UPD_MaxFrameThreshP, maxFrameMultP);
        MEMCPY_CONST(UPD_MaxFrameThreshB, maxFrameMultP);
#undef MEMCPY_CONST

        dmem->UPD_Temporal_Level = m_basicFeature->m_av1PicParams->temporal_id;
        dmem->UPD_Lowdelay = seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW ? 1 : 0;
        // To handle ultra-low-delay situation inside driver
        dmem->UPD_Lowdelay = (dmem->UPD_Lowdelay || seqParams->InitVBVBufferFullnessInBit <= m_inputbitsperframe * 2) ? 1 : 0;

        dmem->UPD_TR_TargetSize = picParams->TargetFrameSize << 3;

        if (picParams->TargetFrameSize > 0)
        {
            dmem->UPD_TCBRC_SCENARIO = MapTCBRCScenarioInfo(seqParams->ScenarioInfo);
        }

        dmem->UPD_LA_TargetFULNESS = 0;
        dmem->UPD_Delta = 0;
        dmem->UPD_LALength = 0;

        //SLBB related fields.
        dmem->UPD_SLBBSize = m_slbData.slbSize;
        dmem->UPD_AVPPiCStateCmdNum = m_slbData.avpPicStateCmdNum;
        dmem->UPD_AVPSegmentStateOffset = m_slbData.avpSegmentStateOffset;
        dmem->UPD_AVPInloopFilterStateOffset = m_slbData.avpInloopFilterStateOffset;
        dmem->UPD_VDEncCmd1Offset = m_slbData.vdencCmd1Offset;
        dmem->UPD_VDEncCmd2Offset = m_slbData.vdencCmd2Offset;
        dmem->UPD_AVPPicStateOffset = m_slbData.avpPicStateOffset;
        dmem->UPD_VDEncTileSliceStateOffset = m_slbData.vdencTileSliceStateOffset;
        dmem->UPD_TileNum = m_slbData.tileNum;

        // BA start
        dmem->UPD_LoopFilterParamsBitOffset      = (uint16_t)m_basicFeature->m_av1PicParams->LoopFilterParamsBitOffset;
        dmem->UPD_QIndexBitOffset                = (uint16_t)m_basicFeature->m_av1PicParams->QIndexBitOffset;
        dmem->UPD_SegmentationBitOffset          = (uint16_t)m_basicFeature->m_av1PicParams->SegmentationBitOffset; // after SegOn flag bit.
        dmem->UPD_FrameHdrOBUSizeInBits          = (uint16_t)m_basicFeature->m_av1PicParams->FrameHdrOBUSizeInBits; // excluding frame_header_obu's trailing_bits() and frame_obu's byte_alignment()
        dmem->UPD_FrameHdrOBUSizeInBytes         = (uint16_t)((m_basicFeature->m_av1PicParams->FrameHdrOBUSizeInBits + 7) >> 3);
        dmem->UPD_FrameHdrOBUSizeByteOffset      = (uint16_t)(m_basicFeature->m_av1PicParams->FrameHdrOBUSizeByteOffset - m_basicFeature->GetAppHdrSizeInBytes(true));
        dmem->UPD_FrameType                      = 0;
        dmem->UPD_ErrorResilientMode             = m_basicFeature->m_av1PicParams->PicFlags.fields.error_resilient_mode;
        dmem->UPD_IntraOnly                      = 0;
        dmem->UPD_PrimaryRefFrame                = 0;
        dmem->UPD_SegOn                          = m_basicFeature->m_av1PicParams->stAV1Segments.SegmentFlags.fields.segmentation_enabled;
        dmem->UPD_SegMapUpdate                   = 0;
        dmem->UPD_SegTemporalUpdate              = 0;
        dmem->UPD_SegUpdateData                  = 0;
        dmem->UPD_IsFrameOBU                     = m_basicFeature->m_av1PicParams->PicFlags.fields.EnableFrameOBU;
        dmem->UPD_EnableCDEFUpdate               = m_basicFeature->m_av1SeqParams->CodingToolFlags.fields.enable_cdef;
        dmem->UPD_CDEFParamsBitOffset            = (uint16_t)m_basicFeature->m_av1PicParams->CDEFParamsBitOffset;
        dmem->UPD_CDEFParamsSizeInBits           = (uint16_t)m_basicFeature->m_av1PicParams->CDEFParamsSizeInBits;
        dmem->UPD_EnableLFUpdate                 = 1;
        dmem->UPD_DisableCdfUpdate               = (m_basicFeature->m_av1PicParams->primary_ref_frame != av1PrimaryRefNone);
        dmem->UPD_EnableDMAForCdf                = 1;
        dmem->UPD_AdditionalHrdSizeByteCount     = 0 - m_basicFeature->m_av1PicParams->FrameSizeReducedInBytes;
        dmem->UPD_PaletteOn                      = m_basicFeature->m_av1PicParams->PicFlags.fields.PaletteModeEnable;

        if (dmem->UPD_PAKPassNum == 1)
            m_curTargetFullness += m_inputbitsperframe;

        if (picParams->PicFlags.fields.allow_intrabc && AV1_KEY_OR_INRA_FRAME(picParams->PicFlags.fields.frame_type))
        {
            dmem->UPD_EnableCDEFUpdate = 0;
            dmem->UPD_EnableLFUpdate   = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Brc::SetConstForUpdate(VdencAv1HucBrcConstantData *params) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto constData = (VdencAv1HucBrcConstantData *)params;

        auto setting = static_cast<Av1VdencFeatureSettings *>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

#define MEMCPY_CONST(D, S) MOS_SecureMemcpy(constData->D, sizeof(constData->D), brcSettings.S.data, brcSettings.S.size);
        MEMCPY_CONST(CONST_QPAdjTabI, av1DeltaQpI);
        MEMCPY_CONST(CONST_QPAdjTabP, av1DeltaQpP);
        MEMCPY_CONST(CONST_QPAdjTabB, av1DeltaQpP);
        MEMCPY_CONST(CONST_DistQPAdjTabI, av1DistortionsDeltaQpI);
        MEMCPY_CONST(CONST_DistQPAdjTabP, av1DistortionsDeltaQpP);
        MEMCPY_CONST(CONST_DistQPAdjTabB, av1DistortionsDeltaQpP);
        MEMCPY_CONST(CONST_LoopFilterLevelTabLuma, loopFilterLevelTabLuma);
        MEMCPY_CONST(CONST_LoopFilterLevelTabChroma, loopFilterLevelTabChroma);

        // ModeCosts depends on frame type
        if (m_basicFeature->m_pictureCodingType == I_TYPE)
        {
            MEMCPY_CONST(CONST_ModeCosts, hucModeCostsIFrame);
        }
        else
        {
            MEMCPY_CONST(CONST_ModeCosts, hucModeCostsPFrame);
        }
#undef MEMCPY_CONST
        return MOS_STATUS_SUCCESS;
    }

    inline uint16_t SwitchFlag(uint8_t rc)
    {
        uint16_t brcFlag = 0;
        switch (rc)
        {
        case RATECONTROL_CBR:
            brcFlag = 0x10; // BRC flag 0: ACQP, 0x10: CBR, 0x20: VBR, 0x40: AVBR, 0x80: CQL
            break;
        case RATECONTROL_VBR:
            brcFlag = 0x20;
            break;
        case RATECONTROL_CQP:
            brcFlag = 0;
            break;
        case RATECONTROL_CQL:
            brcFlag = 0x80;
            break;
        default:
            ENCODE_ASSERTMESSAGE("BRC mode not supported!");
            break;
        }
        return brcFlag;
    }

    static uint32_t CalculateNormalizedDenominator(FRAMERATE* frameRates, uint16_t numberOfLayers, uint32_t normalizedDenominator)
    {
        ENCODE_FUNC_CALL();

        // If pointer to the list of FrameRates is null, return the current Normalized Denominator.
        if (!frameRates)
        {
            return normalizedDenominator;
        }

        if (numberOfLayers == 0)
        {
            return normalizedDenominator;
        }

        normalizedDenominator = normalizedDenominator * frameRates[numberOfLayers - 1].Denominator / MosUtilities::MosGCD(normalizedDenominator, frameRates[numberOfLayers - 1].Denominator);

        return CalculateNormalizedDenominator(frameRates, numberOfLayers - 1, normalizedDenominator);
    }

    inline uint32_t GetProfileLevelMaxFrameSize(uint32_t width, uint32_t height, uint32_t userMaxFrameSize)
    {
        ENCODE_FUNC_CALL();
        uint32_t profileLevelMaxFrame = 0;

        // Might need calculate max frame size base on profile and level, confirm with arch.

        profileLevelMaxFrame = width * height;
        if (userMaxFrameSize > 0)
        {
            profileLevelMaxFrame = (uint32_t)MOS_MIN(userMaxFrameSize, profileLevelMaxFrame);
        }

        return profileLevelMaxFrame;
    }

    inline int32_t ComputeVDEncInitQPI(uint32_t width, uint32_t height, FRAMERATE frameRate, uint32_t targetBitRate, uint16_t gopPicSize, bool is10Bit, uint16_t n_p)
    {
        ENCODE_FUNC_CALL();

        uint32_t frameSize = ((width * height * 3) >> 1);
        if (is10Bit)
        {
            frameSize = (frameSize * 10) >> 3;
        }

        const float x0 = 0, y0 = 1.19f, x1 = 1.75f, y1 = 1.75f;

        int32_t qpP = (int32_t)(1. / 1.2 * pow(10.0, (log10(frameSize * 2. / 3. * ((float)frameRate.Numerator) / ((float)targetBitRate * BRC_KBPS * frameRate.Denominator)) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);
        qpP = (int32_t)((float)qpP * (5.0));
        qpP -= 20;
        qpP = MOS_CLAMP_MIN_MAX(qpP, 1, 200);

        int32_t  qpI = (qpP > 4) ? (qpP - 4) : qpP;
        int16_t  qiboost = n_p / 30 - 1;
        qiboost = MOS_CLAMP_MIN_MAX(qiboost, 10, 20);

        qpI -= qiboost;
        qpI = MOS_CLAMP_MIN_MAX(qpI, 1, 200);

        return qpI;
    }

    static std::tuple<uint32_t, uint32_t> GetFrameRate(CODEC_AV1_ENCODE_SEQUENCE_PARAMS const &seqParams)
    {
        if ((seqParams.FrameRate->Numerator != 0) && (seqParams.FrameRate->Denominator != 0))
        {
            return std::make_tuple(seqParams.FrameRate->Numerator, seqParams.FrameRate->Denominator);
        }
        else
        {
            return std::make_tuple(30, 1);
        }
    }

    MOS_STATUS Av1Brc::SetDmemForInit(VdencAv1HucBrcInitDmem *dmem) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(dmem);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        auto setting = static_cast<Av1VdencFeatureSettings*>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

        auto seqParams = m_basicFeature->m_av1SeqParams;
        ENCODE_CHK_NULL_RETURN(seqParams);

        dmem->BRCFunc = IsBRCResetRequired() ? 2 : 0;
        dmem->INIT_ProfileLevelMaxFrame = GetProfileLevelMaxFrameSize(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, seqParams->UserMaxIFrameSize);
        dmem->INIT_TargetBitrate = seqParams->TargetBitRate[seqParams->NumTemporalLayersMinus1] * BRC_KBPS;
        dmem->INIT_MaxRate = seqParams->MaxBitRate * BRC_KBPS;
        dmem->INIT_MinRate = seqParams->MinBitRate * BRC_KBPS;

        std::tie(dmem->INIT_FrameRateM, dmem->INIT_FrameRateD) = GetFrameRate(*seqParams);

        dmem->INIT_InitBufFullness = MOS_MIN(seqParams->InitVBVBufferFullnessInBit, seqParams->VBVBufferSizeInBit);
        dmem->INIT_BufSize = m_vbvSize;
        dmem->INIT_BRCFlag = SwitchFlag(seqParams->RateControlMethod);
        m_curTargetFullness = seqParams->InitVBVBufferFullnessInBit;

        auto BGOPSize = seqParams->GopRefDist;
        int32_t intraPeriod = seqParams->GopPicSize - 1;

        if (BGOPSize > 1)
        {
            if (intraPeriod > 0)
            {
                intraPeriod = ((intraPeriod + (BGOPSize - 1)) / BGOPSize) * BGOPSize;
            }
            if (intraPeriod != -1)
            {
                dmem->INIT_GopP  = intraPeriod / BGOPSize;
                dmem->INIT_GopB  = intraPeriod / BGOPSize;
                dmem->INIT_GopB1 = (dmem->INIT_GopP + dmem->INIT_GopB == intraPeriod)? 0 : dmem->INIT_GopP * 2;
                dmem->INIT_GopB2 = intraPeriod - dmem->INIT_GopP - dmem->INIT_GopB - dmem->INIT_GopB1;
                if (m_rcMode == RATECONTROL_CQL && seqParams->GopRefDist == 16)
                {
                    dmem->INIT_GopB2 = (dmem->INIT_GopP + dmem->INIT_GopB + dmem->INIT_GopB1 == intraPeriod) ? 0 : dmem->INIT_GopB1 * 2;
                    dmem->INIT_GopB3 = intraPeriod - dmem->INIT_GopP - dmem->INIT_GopB - dmem->INIT_GopB1 - dmem->INIT_GopB2;
                }
            }
            else
            {
                dmem->INIT_GopP = 9999;
                dmem->INIT_GopB = 9999;
            }
        }
        else
        {
            dmem->INIT_GopP = seqParams->GopPicSize - 1;
            dmem->INIT_GopB = 0;
        }

        dmem->INIT_FrameWidth = (uint16_t)m_basicFeature->m_oriFrameWidth;
        dmem->INIT_FrameHeight = (uint16_t)m_basicFeature->m_oriFrameHeight;
        dmem->INIT_MinQP = m_basicFeature->m_av1PicParams->MinBaseQIndex;
        dmem->INIT_MaxQP = m_basicFeature->m_av1PicParams->MaxBaseQIndex == 0 ? 255 : m_basicFeature->m_av1PicParams->MaxBaseQIndex;
        dmem->INIT_LevelQP = seqParams->ICQQualityFactor;
        dmem->INIT_GoldenFrameInterval = 14;
        dmem->INIT_EnableScaling = false;
        dmem->INIT_OvershootCBR_pct = 0; //sliding window related.

#define MEMCPY_CONST(D, S) MOS_SecureMemcpy(dmem->D, sizeof(dmem->D), brcSettings.S.data, brcSettings.S.size);
        MEMCPY_CONST(INIT_InstRateThreshI0, instRateThresholdI);
        MEMCPY_CONST(INIT_InstRateThreshP0, instRateThresholdP);
#undef MEMCPY_CONST

        if (dmem->INIT_FrameRateM == 0)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        double inputBitsPerFrame = ((double)dmem->INIT_MaxRate * (double)dmem->INIT_FrameRateD) / (double)dmem->INIT_FrameRateM;
        double bpsRatio = inputBitsPerFrame / ((double)dmem->INIT_BufSize / brcSettings.devStdFPS);
        bpsRatio = MOS_CLAMP_MIN_MAX(bpsRatio, brcSettings.bpsRatioLow, brcSettings.bpsRatioHigh);
        for (uint32_t i = 0; i < brcSettings.numDevThreshlds / 2; i++)
        {
            dmem->INIT_DevThreshPB0[i] = (int8_t)(brcSettings.negMultPB * pow(((double *)(brcSettings.devThresholdFpNegPB.data))[i], bpsRatio));
            dmem->INIT_DevThreshPB0[i + brcSettings.numDevThreshlds / 2] = (int8_t)(brcSettings.postMultPB * pow(((double *)(brcSettings.devThresholdFpPosPB.data))[i], bpsRatio));

            dmem->INIT_DevThreshI0[i] = (int8_t)(brcSettings.negMultPB * pow(((double *)(brcSettings.devThresholdFpNegI.data))[i], bpsRatio));
            dmem->INIT_DevThreshI0[i + brcSettings.numDevThreshlds / 2] = (int8_t)(brcSettings.postMultPB * pow(((double *)(brcSettings.devThresholdFpPosI.data))[i], bpsRatio));

            dmem->INIT_DevThreshVBR0[i] = (int8_t)(brcSettings.negMultVBR * pow(((double *)(brcSettings.devThresholdVbrNeg.data))[i], bpsRatio));
            dmem->INIT_DevThreshVBR0[i + brcSettings.numDevThreshlds / 2] = (int8_t)(brcSettings.posMultVBR * pow(((double *)(brcSettings.devThresholdVbrPos.data))[i], bpsRatio));
        }

        int32_t qpI = 0, qpP = 0;
        qpI = ComputeVDEncInitQPI(
                m_basicFeature->m_oriFrameWidth,
                m_basicFeature->m_oriFrameHeight,
                m_basicFeature->m_av1SeqParams->FrameRate[0],
                m_basicFeature->m_av1SeqParams->TargetBitRate[0],
                m_basicFeature->m_av1SeqParams->GopPicSize,
                m_basicFeature->m_is10Bit,
                dmem->INIT_GopP);

        qpP = qpI + 20;

        dmem->INIT_InitQPI = (uint8_t)qpI;
        dmem->INIT_InitQPP = (uint8_t)qpP;

        dmem->INIT_SegMapGenerating = false;
        dmem->INIT_Total_Level = seqParams->NumTemporalLayersMinus1 + 1;
        if (dmem->INIT_Total_Level > 1)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        dmem->INIT_SLIDINGWINDOW_ENABLE = seqParams->SlidingWindowSize != 0;
        dmem->INIT_SLIDINGWINDOW_SIZE = (uint8_t)seqParams->SlidingWindowSize;
        if (dmem->INIT_SLIDINGWINDOW_ENABLE && seqParams->TargetBitRate[0] > 0)
        {
            dmem->INIT_OvershootCBR_pct = (uint16_t)(seqParams->MaxBitRatePerSlidingWindow * 100 / seqParams->TargetBitRate[0]);
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1Brc::SetSequenceStructs()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        
        if (m_basicFeature->m_av1SeqParams->RateControlMethod == RATECONTROL_CQL)
        {
            const uint8_t ICQFactorLookup[52] = { 
                0,  1,  1,  1,  1,  2,  3,  4,  6,  7,  9,  11, 13, 16, 18, 23,
                26, 30, 34, 39, 46, 52, 59, 68, 77, 87, 98, 104,112,120,127,134,
                142,149,157,164,171,178,186,193,200,207,213,219,225,230,235,240,
                245,249,252,255 
            };
            
            uint32_t TargetBitRate                                     = m_basicFeature->m_frameWidth * m_basicFeature->m_frameHeight * 8 / 1000;
            m_basicFeature->m_av1SeqParams->ICQQualityFactor           = ICQFactorLookup[m_basicFeature->m_av1SeqParams->ICQQualityFactor];
            m_basicFeature->m_av1SeqParams->TargetBitRate[0]           = TargetBitRate;
            m_basicFeature->m_av1SeqParams->MaxBitRate                 = (TargetBitRate << 4) / 10;
            m_basicFeature->m_av1SeqParams->MinBitRate                 = 0;
            m_basicFeature->m_av1SeqParams->InitVBVBufferFullnessInBit = 8000 * (TargetBitRate << 3) / 10;
            m_basicFeature->m_av1SeqParams->VBVBufferSizeInBit         = 8000 * (TargetBitRate << 1);
        }
        
        m_brcEnabled = IsRateControlBrc(m_basicFeature->m_av1SeqParams->RateControlMethod);

        m_brcInit = m_brcEnabled && m_basicFeature->m_resolutionChanged;

        m_rcMode = m_brcEnabled? m_basicFeature->m_av1SeqParams->RateControlMethod : 0;

        if (m_rcMode == RATECONTROL_CQL || m_rcMode == RATECONTROL_QVBR)
        {
            if (m_basicFeature->m_av1SeqParams->ICQQualityFactor > ENCODE_AV1_MAX_ICQ_QUALITYFACTOR)
            {
                ENCODE_ASSERTMESSAGE("Invalid ICQ Quality Factor input (%d)\n", m_basicFeature->m_av1SeqParams->ICQQualityFactor);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }

        m_brcReset = m_basicFeature->m_av1SeqParams->SeqFlags.fields.ResetBRC;

        if (m_brcReset &&
            (!m_brcEnabled ||
             m_rcMode == RATECONTROL_CQL))
        {
            ENCODE_ASSERTMESSAGE("BRC Reset cannot be trigerred in CQP/ICQ modes - invalid BRC parameters.");
            m_brcReset = false;
        }

        if (IsBRCInitRequired())
        {
            auto seqParams = m_basicFeature->m_av1SeqParams;
            ENCODE_CHK_NULL_RETURN(seqParams);

            m_delay                   = seqParams->InitVBVBufferFullnessInBit;
            m_vbvSize                 = seqParams->VBVBufferSizeInBit;

            uint32_t nom = 0;
            uint32_t denom = 1;
            std::tie(nom, denom) = GetFrameRate(*seqParams);

            if (denom == 0)
            {
                denom = 1;
            }

            m_frameRate = (int32_t)((double)nom * 100. / (double)denom);

            //average frame size in bits based on bit rate and frame rate.
            const uint32_t maxBitrate      = seqParams->MaxBitRate * BRC_KBPS;
            m_inputbitsperframe = maxBitrate * 100. / m_frameRate;

            //make sure the buffer size can contain at least 4 frames in average
            if (m_vbvSize < (int32_t)(m_inputbitsperframe * 4))
            {
                m_vbvSize = (int32_t)(m_inputbitsperframe * 4);
            }
            //make sure the initial buffer size is larger than 2 average frames and smaller than the max buffer size.
            if (m_delay == 0)
                m_delay = 7 * m_vbvSize / 8;
            if (m_delay < (int32_t)(m_inputbitsperframe * 2))
                m_delay = (int32_t)(m_inputbitsperframe * 2);
            if (m_delay > m_vbvSize)
                m_delay = m_vbvSize;
        }

        return eStatus;
    }

    MOS_STATUS Av1Brc::FreeBrcResources()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_vdenc2ndLevelBatchBuffer[j], nullptr);
            ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
            eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_pakInsertOutputBatchBuffer[j], nullptr);
            ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        }

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Av1Brc)
    {
        ENCODE_CHK_NULL_RETURN(params.hucDataSource);

        switch (params.function)
        {
        case BRC_INIT: {
            auto dmem = (VdencAv1HucBrcInitDmem *)m_allocator->LockResourceForWrite(params.hucDataSource);

            ENCODE_CHK_NULL_RETURN(dmem);
            MOS_ZeroMemory(dmem, sizeof(VdencAv1HucBrcInitDmem));

            SetDmemForInit(dmem);

            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(params.hucDataSource));
            break;
        }
        case BRC_UPDATE: {
            auto dmem = (VdencAv1HucBrcUpdateDmem *)m_allocator->LockResourceForWrite(params.hucDataSource);

            ENCODE_CHK_NULL_RETURN(dmem);
            MOS_ZeroMemory(dmem, sizeof(VdencAv1HucBrcUpdateDmem));

            dmem->UPD_MaxNumPAKs = params.passNum;
            dmem->UPD_PAKPassNum = params.currentPass;
            dmem->UPD_SegMapGenerating = 0;

            SetDmemForUpdate(dmem);

            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(params.hucDataSource));

            break;
        }
        case PAK_INTEGRATE: {
            // nothing need to be done within brc feature for pak int
            break;
        }
        default:
            ENCODE_ASSERTMESSAGE("AV1 BRC feature supports only PAK_INTEGRATE, BRC_INIT and BRC_UPDATE HUC functions");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, Av1Brc)
    {
        if (params.function == BRC_UPDATE)
        {
            const PMOS_RESOURCE brcConstDataBuffer = params.regionParams[5].presRegion;

            auto hucConstData = (VdencAv1HucBrcConstantData *)m_allocator->LockResourceForWrite(brcConstDataBuffer);
            ENCODE_CHK_NULL_RETURN(hucConstData);

            SetConstForUpdate(hucConstData);

            m_allocator->UnLock(brcConstDataBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1Brc)
    {
        if (m_brcEnabled)
        {
            params.frameStatisticsStreamOut = true;
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
