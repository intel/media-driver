/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_hevc_brc.cpp
//! \brief    Defines the common interface for hevc brc features
//!

#include "encode_hevc_basic_feature.h"
#include "encode_hevc_brc.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_vdenc_const_settings.h"
#include "encode_huc_brc_init_packet.h"
#include "encode_huc_brc_update_packet.h"
namespace encode
{
    HEVCEncodeBRC::HEVCEncodeBRC(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
        m_hwInterface(hwInterface),
        m_allocator(allocator)
    {
        m_featureManager = featureManager;
        // can be optimized after move encode parameter to feature manager.
        auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager*>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        m_basicFeature = dynamic_cast<HevcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
    }

    HEVCEncodeBRC::~HEVCEncodeBRC()
    {
        FreeBrcResources();
    }

    MOS_STATUS HEVCEncodeBRC::Init(void *setting)
    {
        ENCODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "FAST PAK ENABLE",
            MediaUserSetting::Group::Sequence);
        m_fastPakEnable = outValue.Get<bool>();

        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "HEVC VDEnc ACQP Enable",
            MediaUserSetting::Group::Sequence);
        m_hevcVDEncAcqpEnabled = outValue.Get<bool>();
#endif

        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;

        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
        ENCODE_CHK_STATUS_RETURN(UpdateBrcResources(encodeParams));

#if (_DEBUG || _RELEASE_INTERNAL)
        ReportUserSettingForDebug(
            m_userSettingPtr,
            "Encode RateControl Method",
            m_rcMode,
            MediaUserSetting::Group::Sequence);
        ReportUserSettingForDebug(
            m_userSettingPtr,
            "HEVC VDEnc ACQP Enable",
            m_hevcVDEncAcqpEnabled,
            MediaUserSetting::Group::Sequence);

        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcPicParams);
        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Adaptive TU Enable",
            MediaUserSetting::Group::Sequence);
        m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled |= outValue.Get<uint8_t>();
#endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(CodechalVdencHevcPakInfo), CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC PakInfo";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(PakInfo, allocParamsForBufferLinear, 6);

        // Allocate Frame Statistics Streamout Data Destination Buffer. DW98-100 in HCP PipeBufAddr command
        HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcBasicFeature);

        // BRC history buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_brcHistoryBufSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC History Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(VdencBRCHistoryBuffer, allocParamsForBufferLinear, 1);

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
        }

        // Lcu Base Address buffer
        // HEVC Encoder Mode: Slice size is written to this buffer when slice size conformance is enabled.
        // 1 CL (= 16 DWs = 64 bytes) per slice * Maximum number of slices in a frame.
        // Align to page for HUC requirement
        const uint32_t picWidthInMinLCU = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, CODECHAL_HEVC_MIN_LCU_SIZE);        //assume smallest LCU to get max width
        const uint32_t picHeightInMinLCU = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, CODECHAL_HEVC_MIN_LCU_SIZE);      //assume smallest LCU to get max height
        uint32_t maxLcu = picWidthInMinLCU * picHeightInMinLCU;
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(maxLcu * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "LcuBaseAddressBuffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(LcuBaseAddressBuffer, allocParamsForBufferLinear, 1);

        // VDENC BRC PAK MMIO buffer
        allocParamsForBufferLinear.dwBytes = sizeof(VdencBrcPakMmio);
        allocParamsForBufferLinear.pBufName = "VDENC BRC PAK MMIO Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcPakMmioBuffer, allocParamsForBufferLinear, 1);

        // Debug buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_brcDebugBufSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC Debug Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcDebugBuffer, allocParamsForBufferLinear, 1);

        allocParamsForBufferLinear.dwBytes  = HEVC_BRC_PAK_STATISTCS_SIZE;
        allocParamsForBufferLinear.pBufName = "BRC PAK Statistics Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_recycleBuf->RegisterResource(BrcPakStatisticBuffer, allocParamsForBufferLinear, CODECHAL_ENCODE_RECYCLED_BUFFER_NUM);

        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            MOS_RESOURCE *allocatedresource = m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBuffer,i);
            ENCODE_CHK_NULL_RETURN(allocatedresource);
            m_vdencBrcBuffers.resBrcPakStatisticBuffer[i] = allocatedresource;
        }

        m_rdLambdaArray     = MOS_NewArray(uint16_t, HUC_QP_RANGE);
        ENCODE_CHK_NULL_RETURN(m_rdLambdaArray);
        m_sadLambdaArray    = MOS_NewArray(uint16_t, HUC_QP_RANGE);
        ENCODE_CHK_NULL_RETURN(m_sadLambdaArray);

        return eStatus;
    }

    MOS_STATUS HEVCEncodeBRC::UpdateBrcResources(void *params)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
            static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        auto num_tile_rows    = hevcPicParams->num_tile_rows_minus1 + 1;
        auto num_tile_columns = hevcPicParams->num_tile_columns_minus1 + 1;
        auto num_tiles        = num_tile_rows * num_tile_columns;

        if (Mos_ResourceIsNull(&m_resBrcDataBuffer))
        {
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(num_tiles * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "BRC Data Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_resBrcDataBuffer = *resource;
        }
        // BRC Data Buffer

        return eStatus;
    }

    MOS_STATUS HEVCEncodeBRC::GetBrcDataBuffer(MOS_RESOURCE *&buffer)
    {
        ENCODE_FUNC_CALL();
        buffer = &m_resBrcDataBuffer;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetDmemForUpdate(void* params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucVdencBrcUpdateDmem =
            (VdencHevcHucBrcUpdateDmem *)params;

        auto       setting = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjFrame_U16, 4 * sizeof(uint16_t), brcSettings.startGAdjFrame.data, 4 * sizeof(uint16_t));
        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->gRateRatioThreshold_U8, 7 * sizeof(uint8_t), brcSettings.rateRatioThreshold.data, 7 * sizeof(uint8_t));
        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjMult_U8, 5 * sizeof(uint8_t), brcSettings.startGAdjMult.data, 5 * sizeof(uint8_t));
        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjDiv_U8, 5 * sizeof(uint8_t), brcSettings.startGAdjDiv.data, 5 * sizeof(uint8_t));
        MOS_SecureMemcpy(hucVdencBrcUpdateDmem->gRateRatioThresholdQP_U8, 8 * sizeof(uint8_t), brcSettings.rateRatioThresholdQP.data, 8 * sizeof(uint8_t));

        if ((IsACQPEnabled() && m_basicFeature->m_hevcSeqParams->QpAdjustment) || (IsBRCEnabled() && (m_basicFeature->m_hevcSeqParams->MBBRC != 2)))
        {
            hucVdencBrcUpdateDmem->DeltaQPForSadZone0_S8 = brcSettings.deltaQPForSadZone0_S8;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone1_S8 = brcSettings.deltaQPForSadZone1_S8;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone2_S8 = brcSettings.deltaQPForSadZone2_S8;
            hucVdencBrcUpdateDmem->DeltaQPForSadZone3_S8 = brcSettings.deltaQPForSadZone3_S8;
            hucVdencBrcUpdateDmem->DeltaQPForMvZero_S8   = brcSettings.deltaQPForMvZero_S8;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone0_S8  = brcSettings.deltaQPForMvZone0_S8;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone1_S8 = brcSettings.deltaQPForMvZone1_S8;
            hucVdencBrcUpdateDmem->DeltaQPForMvZone2_S8 = brcSettings.deltaQPForMvZone2_S8;
        }

        if (m_fastPakEnable)
        {
            hucVdencBrcUpdateDmem->ReEncodePositiveQPDeltaThr_S8 = brcSettings.reEncodePositiveQPDeltaThr_S8;
            hucVdencBrcUpdateDmem->ReEncodeNegativeQPDeltaThr_S8 = brcSettings.reEncodeNegativeQPDeltaThr_S8;
        }
        else
        {
            hucVdencBrcUpdateDmem->ReEncodePositiveQPDeltaThr_S8 = 0;
            hucVdencBrcUpdateDmem->ReEncodeNegativeQPDeltaThr_S8 = 0;
        }
        hucVdencBrcUpdateDmem->SceneChgPrevIntraPctThreshold_U8 = brcSettings.sceneChgPrevIntraPctThreshold_U8;
        hucVdencBrcUpdateDmem->SceneChgCurIntraPctThreshold_U8 = brcSettings.sceneChgCurIntraPctThreshold_U8;

        hucVdencBrcUpdateDmem->UPD_Randomaccess = m_basicFeature->m_hevcSeqParams->LowDelayMode == 1 ? 0 : 1;

        hucVdencBrcUpdateDmem->UPD_AdaptiveTUEnabled = m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetHevcDepthBasedLambda(
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS  hevcSeqParams,
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS   hevcPicParams,
        uint8_t                             qp,
        uint16_t&                           SADQPLambda,
        uint16_t&                           RDQPLambda)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        double   QPScale  = (hevcPicParams->CodingType == I_TYPE) ? 0.60 : 0.65;
        uint32_t bGopSize = hevcSeqParams->GopRefDist;
        int32_t  depth    = hevcPicParams->HierarchLevelPlus1 ? hevcPicParams->HierarchLevelPlus1 - 1 : 0;

        std::vector<double> qpFactors;
        uint32_t            lambdaType = 1;

        if (hevcSeqParams->LowDelayMode)
        {
            lambdaType = 1;
            bGopSize     = 4; // LDB set bGopSize as 4
            qpFactors  = {0.578, 0.3524, 0.3524};
        }
        else
        {
            lambdaType = 2;
            qpFactors  = {0.442, 0.3536, 0.3536, 0.68}; // seems not used;
        }

        if (lambdaType != 1)
        {
            if (hevcPicParams->CodingType == B_TYPE && lambdaType != 0 && ((bGopSize == 4) || (bGopSize == 8)))
            {
                const double LambdaScaleForRA[2][4] =
                {
                    //Base, Depth0, Depth1, Depth2
                    {1.8f, 0.9f, 1.7f, 1.0f},  // GOP 4
                    {2.0f, 0.9f, 1.4f, 0.8f}   // GOP 8
                };

                double raLambdaScale = 1.0;
                if (bGopSize == 4)
                {
                    if (depth == 2)
                    {
                        raLambdaScale = 1.0 * LambdaScaleForRA[0][3];
                    }
                    else if (depth == 1)  // depth is 1  //modification for LTR
                    {
                        raLambdaScale = 0.52 * LambdaScaleForRA[0][2];
                    }
                    else if (depth == 0)  // depth is 0  //modification for LTR
                    {
                        raLambdaScale = 0.65 * LambdaScaleForRA[0][1];
                    }

                    QPScale *= raLambdaScale * LambdaScaleForRA[0][0];
                }
                else if (bGopSize == 8)
                {
                    if (depth == 3)
                    {
                        raLambdaScale = 1.0 * LambdaScaleForRA[1][3];
                    }
                    else if (depth == 2 || depth == 1)
                    {
                        raLambdaScale = 0.52 * LambdaScaleForRA[1][2];
                    }
                    else if (depth == 0)
                    {
                        raLambdaScale = 0.65 * LambdaScaleForRA[1][1];
                    }

                    QPScale *= raLambdaScale * LambdaScaleForRA[1][0];
                }
            }
        }
        else if (bGopSize > 0)
        {
            auto Clip3 = [&](double min, double max, double x) {
                return ((x < min) ? min : ((x > max) ? max : x));
            };

            if (hevcPicParams->CodingType == I_TYPE)
            {
                QPScale = 0.57 * (1.0 - Clip3(0, 0.5, 0.05 * (bGopSize - 1)));
            }
            else
            {
                int size = qpFactors.size();
                QPScale = qpFactors[CodecHal_Clip3(0, size - 1, depth)];
                if (depth > 0)
                {
                    QPScale *= Clip3(2.0, 4.0, ((qp - 12) / 6.0));
                }
            }
        }

        double realLambda = QPScale * pow(2.0, MOS_MAX(0, qp - 12) / 3.0);
        RDQPLambda  = (uint16_t)(MOS_MIN(65535, realLambda * 4 + 0.5));  //U14.2

        realLambda        = sqrt(realLambda);
        SADQPLambda = (uint16_t)(MOS_MIN(65535, realLambda * 4 + 0.5)); //U8.2

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetConstLambdaForUpdate(void *params, bool lambdaType)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucConstData =
            (VdencHevcHucBrcConstantData *)params;

        auto       setting = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

        if (lambdaType)
        {
            ENCODE_CHK_NULL_RETURN(m_basicFeature);
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcPicParams);

            for (uint8_t qp = 0; qp < HUC_QP_RANGE; qp++)
            {
                ENCODE_CHK_STATUS_RETURN(SetHevcDepthBasedLambda(m_basicFeature->m_hevcSeqParams, m_basicFeature->m_hevcPicParams,
                    qp, m_sadLambdaArray[qp], m_rdLambdaArray[qp]));
            }

            if (m_basicFeature->m_hevcPicParams->CodingType == I_TYPE)
            {
                MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_2, brcSettings.HevcVdencBrcSettings_0.size, m_rdLambdaArray, brcSettings.HevcVdencBrcSettings_0.size);
                MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_0, brcSettings.HevcVdencBrcSettings_2.size, m_sadLambdaArray, brcSettings.HevcVdencBrcSettings_2.size);
            }
            else
            {
                MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_3, brcSettings.HevcVdencBrcSettings_1.size, m_rdLambdaArray, brcSettings.HevcVdencBrcSettings_1.size);
                MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_1, brcSettings.HevcVdencBrcSettings_3.size, m_sadLambdaArray, brcSettings.HevcVdencBrcSettings_3.size);
            }
        }
        else
        {
            MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_2, brcSettings.HevcVdencBrcSettings_0.size, brcSettings.HevcVdencBrcSettings_0.data, brcSettings.HevcVdencBrcSettings_0.size);
            MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_3, brcSettings.HevcVdencBrcSettings_1.size, brcSettings.HevcVdencBrcSettings_1.data, brcSettings.HevcVdencBrcSettings_1.size);

            MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_0, brcSettings.HevcVdencBrcSettings_2.size, brcSettings.HevcVdencBrcSettings_2.data, brcSettings.HevcVdencBrcSettings_2.size);
            MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_1, brcSettings.HevcVdencBrcSettings_3.size, brcSettings.HevcVdencBrcSettings_3.data, brcSettings.HevcVdencBrcSettings_3.size);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetConstForUpdate(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucConstData =
            (VdencHevcHucBrcConstantData *)params;

        auto       setting = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

        MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_4, brcSettings.hucConstantData.size, brcSettings.hucConstantData.data, brcSettings.hucConstantData.size);

        if (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            const int numEstrateThreshlds = 7;

            for (int i = 0; i < numEstrateThreshlds + 1; i++)
            {
                for (uint32_t j = 0; j < brcSettings.numDevThreshlds + 1; j++)
                {
                    hucConstData->VdencHevcHucBrcConstantData_5[(numEstrateThreshlds + 1)*j + i] = *brcSettings.HevcVdencBrcSettings_4[j][i];
                    hucConstData->VdencHevcHucBrcConstantData_6[(numEstrateThreshlds + 1)*j + i] = *brcSettings.HevcVdencBrcSettings_5[j][i];
                    hucConstData->VdencHevcHucBrcConstantData_7[(numEstrateThreshlds + 1)*j + i] = *brcSettings.HevcVdencBrcSettings_6[j][i];
                }
            }
        }

        // ModeCosts depends on frame type
        if (m_basicFeature->m_pictureCodingType == I_TYPE)
        {
            MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_8, brcSettings.HevcVdencBrcSettings_7.size, brcSettings.HevcVdencBrcSettings_7.data, brcSettings.HevcVdencBrcSettings_7.size);
        }
        else
        {
            MOS_SecureMemcpy(hucConstData->VdencHevcHucBrcConstantData_8, brcSettings.HevcVdencBrcSettings_8.size, brcSettings.HevcVdencBrcSettings_8.data, brcSettings.HevcVdencBrcSettings_8.size);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetDmemForInit(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucVdencBrcInitDmem =
            (VdencHevcHucBrcInitDmem *)params;

        auto setting = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
        ENCODE_CHK_NULL_RETURN(setting);

        auto brcSettings = setting->brcSettings;

        hucVdencBrcInitDmem->TargetBitrate_U32 = m_basicFeature->m_hevcSeqParams->TargetBitRate * m_brc_kbps;
        hucVdencBrcInitDmem->MaxRate_U32       = m_basicFeature->m_hevcSeqParams->MaxBitRate * m_brc_kbps;

        if (hucVdencBrcInitDmem->LowDelayMode_U8 = (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW))
        {
            MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshPB0_S8, 8 * sizeof(int8_t), brcSettings.lowdelayDevThreshPB.data, 8 * sizeof(int8_t));
            MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshVBR0_S8, 8 * sizeof(int8_t), brcSettings.lowdelayDevThreshVBR.data, 8 * sizeof(int8_t));
            MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshI0_S8, 8 * sizeof(int8_t), brcSettings.lowdelayDevThreshI.data, 8 * sizeof(int8_t));
        }
        else
        {
            int8_t DevThreshPB0_S8[8] = {};
            int8_t DevThreshVBR0_S8[8] = {};
            int8_t DevThreshI0_S8[8] = {};

            uint64_t inputbitsperframe = uint64_t(hucVdencBrcInitDmem->MaxRate_U32*100. / (hucVdencBrcInitDmem->FrameRateM_U32 * 100.0 / hucVdencBrcInitDmem->FrameRateD_U32));
            if (m_brcEnabled && (m_rcMode != RATECONTROL_ICQ) && !hucVdencBrcInitDmem->BufSize_U32)
            {
                ENCODE_ASSERTMESSAGE("VBV BufSize should not be 0 for BRC case\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            uint64_t vbvsz = hucVdencBrcInitDmem->BufSize_U32;
            double bps_ratio = inputbitsperframe / (vbvsz / brcSettings.devStdFPS);
            if (bps_ratio < brcSettings.bpsRatioLow) bps_ratio = brcSettings.bpsRatioLow;
            if (bps_ratio > brcSettings.bpsRatioHigh) bps_ratio = brcSettings.bpsRatioHigh;

            auto devThreshIFPNEG = (double *)brcSettings.devThreshIFPNEG.data;
            auto devThreshPBFPNEG = (double *)brcSettings.devThreshPBFPNEG.data;
            auto devThreshIFPPOS = (double *)brcSettings.devThreshIFPPOS.data;
            auto devThreshPBFPPOS = (double *)brcSettings.devThreshPBFPPOS.data;
            auto devThreshVBRPOS = (double *)brcSettings.devThreshVBRPOS.data;
            auto devThreshVBRNEG = (double *)brcSettings.devThreshVBRNEG.data;

            for (uint32_t i = 0; i < brcSettings.numDevThreshlds / 2; i++) {
                DevThreshPB0_S8[i] = (signed char)(brcSettings.negMultPB*pow(devThreshPBFPNEG[i], bps_ratio));
                DevThreshPB0_S8[i + brcSettings.numDevThreshlds / 2] = (signed char)(brcSettings.postMultPB*pow(devThreshPBFPPOS[i], bps_ratio));

                DevThreshI0_S8[i] = (signed char)(brcSettings.negMultPB*pow(devThreshIFPNEG[i], bps_ratio));
                DevThreshI0_S8[i + brcSettings.numDevThreshlds / 2] = (signed char)(brcSettings.postMultPB*pow(devThreshIFPPOS[i], bps_ratio));

                DevThreshVBR0_S8[i] = (signed char)(brcSettings.negMultPB*pow(devThreshVBRNEG[i], bps_ratio));
                DevThreshVBR0_S8[i + brcSettings.numDevThreshlds / 2] = (signed char)(brcSettings.posMultVBR*pow(devThreshVBRPOS[i], bps_ratio));
            }

            MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshPB0_S8, 8 * sizeof(int8_t), (void*)DevThreshPB0_S8, 8 * sizeof(int8_t));
            MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshVBR0_S8, 8 * sizeof(int8_t), (void*)DevThreshVBR0_S8, 8 * sizeof(int8_t));
            MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshI0_S8, 8 * sizeof(int8_t), (void*)DevThreshI0_S8, 8 * sizeof(int8_t));
        }

        MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshP0_S8, 4 * sizeof(int8_t), brcSettings.instRateThreshP0.data, 4 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshB0_S8, 4 * sizeof(int8_t), brcSettings.instRateThreshB0.data, 4 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshI0_S8, 4 * sizeof(int8_t), brcSettings.instRateThreshI0.data, 4 * sizeof(int8_t));

        hucVdencBrcInitDmem->TopFrmSzThrForAdapt2Pass_U8 = brcSettings.topFrmSzThrForAdapt2Pass_U8;
        hucVdencBrcInitDmem->BotFrmSzThrForAdapt2Pass_U8 = brcSettings.botFrmSzThrForAdapt2Pass_U8;

        hucVdencBrcInitDmem->TopQPDeltaThrForAdapt2Pass_U8 = brcSettings.topQPDeltaThrForAdapt2Pass_U8;
        hucVdencBrcInitDmem->BotQPDeltaThrForAdapt2Pass_U8 = brcSettings.botQPDeltaThrForAdapt2Pass_U8;

        MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshP0_U8, 7 * sizeof(uint8_t), brcSettings.estRateThreshP0.data, 7 * sizeof(uint8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshB0_U8, 7 * sizeof(uint8_t), brcSettings.estRateThreshB0.data, 7 * sizeof(uint8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshI0_U8, 7 * sizeof(uint8_t), brcSettings.estRateThreshI0.data, 7 * sizeof(uint8_t));

        if (m_brcEnabled)
        {
            ENCODE_CHK_STATUS_RETURN(SetBrcSettings(hucVdencBrcInitDmem));
        }
        else if (m_hevcVDEncAcqpEnabled)
        {
            ENCODE_CHK_STATUS_RETURN(SetAcqpSettings(hucVdencBrcInitDmem));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetVdencBatchBufferState(
        const uint32_t    currRecycledBufIdx,
        const uint32_t    slcIdx,
        PMHW_BATCH_BUFFER &vdencBatchBuffer,
        bool &            vdencHucUsed)
    {
        ENCODE_FUNC_CALL();

        vdencHucUsed     = m_vdencHucUsed;
        vdencBatchBuffer = &m_vdenc2ndLevelBatchBuffer[currRecycledBufIdx];

        vdencBatchBuffer->dwOffset =
            m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;

        auto basicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeature);

        for (uint32_t j = 0; j < slcIdx; j++)
        {
            vdencBatchBuffer->dwOffset +=
                (m_hwInterface->m_vdencBatchBufferPerSliceConstSize + basicFeature->m_vdencBatchBufferPerSliceVarSize[j]);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetSequenceStructs()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        m_brcInit = m_basicFeature->m_resolutionChanged;

        m_brcEnabled = IsRateControlBrc(m_basicFeature->m_hevcSeqParams->RateControlMethod);

        m_rcMode = m_brcEnabled ? m_basicFeature->m_hevcSeqParams->RateControlMethod : 0;

        SetLcuBrc();

        if (m_rcMode == RATECONTROL_ICQ || m_rcMode == RATECONTROL_QVBR)
        {
            if (m_basicFeature->m_hevcSeqParams->ICQQualityFactor < CODECHAL_ENCODE_HEVC_MIN_ICQ_QUALITYFACTOR ||
                m_basicFeature->m_hevcSeqParams->ICQQualityFactor > CODECHAL_ENCODE_HEVC_MAX_ICQ_QUALITYFACTOR)
            {
                ENCODE_ASSERTMESSAGE("Invalid ICQ Quality Factor input (%d)\n", m_basicFeature->m_hevcSeqParams->ICQQualityFactor);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }

        m_brcReset = m_basicFeature->m_hevcSeqParams->bResetBRC;

        if (m_brcReset &&
            (!m_brcEnabled ||
             m_rcMode == RATECONTROL_ICQ))
        {
            ENCODE_ASSERTMESSAGE("BRC Reset cannot be trigerred in CQP/ICQ modes - invalid BRC parameters.");
            m_brcReset = false;
        }

        // ACQP is also considered as BRC (special version of ICQ)
        if (m_brcEnabled)
        {
            m_vdencBrcEnabled = true;
            m_hevcVDEncAcqpEnabled = false;  // when BRC is enabled, ACQP has to be turned off
        }

        m_vdencHucUsed = m_hevcVDEncAcqpEnabled || m_vdencBrcEnabled;

        // Check VBVBufferSize
        if (m_brcEnabled)
        {
            if (m_basicFeature->m_hevcSeqParams->VBVBufferSizeInBit < m_basicFeature->m_hevcSeqParams->InitVBVBufferFullnessInBit)
            {
                ENCODE_NORMALMESSAGE(
                    "VBVBufferSizeInBit is less than InitVBVBufferFullnessInBit, \
                    min(VBVBufferSizeInBit, InitVBVBufferFullnessInBit) will set to \
                    hucVdencBrcInitDmem->InitBufFull_U32 and hucVdencBrcUpdateDmem->TARGETSIZE_U32(except Low Delay BRC).\n");
            }
        }

        return eStatus;
    }

    MOS_STATUS HEVCEncodeBRC::FreeBrcResources()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_vdenc2ndLevelBatchBuffer[j], nullptr);
            ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        }

        if (m_rdLambdaArray)
        {
            MOS_DeleteArray(m_rdLambdaArray);
        }
        if (m_sadLambdaArray)
        {
            MOS_DeleteArray(m_sadLambdaArray);
        }

        return eStatus;
    }

    MOS_STATUS HEVCEncodeBRC::SetBrcSettings(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucVdencBrcInitDmem =
            (VdencHevcHucBrcInitDmem *)params;

        switch (m_rcMode)
        {
        case RATECONTROL_ICQ:
            hucVdencBrcInitDmem->BRCFlag = 0;
            hucVdencBrcInitDmem->ACQP_U32 = m_basicFeature->m_hevcSeqParams->ICQQualityFactor;
            break;
        case RATECONTROL_CBR:
            hucVdencBrcInitDmem->BRCFlag = 1;
            break;
        case RATECONTROL_VBR:
            hucVdencBrcInitDmem->BRCFlag = 2;
            hucVdencBrcInitDmem->ACQP_U32 = 0;
            break;
        case RATECONTROL_VCM:
            hucVdencBrcInitDmem->BRCFlag = 3;
            break;
        case RATECONTROL_QVBR:
            hucVdencBrcInitDmem->BRCFlag = 2;
            hucVdencBrcInitDmem->ACQP_U32 = m_basicFeature->m_hevcSeqParams->ICQQualityFactor;
            break;
        default:
            break;
        }

        // Low Delay BRC
        if (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            hucVdencBrcInitDmem->BRCFlag = 4;
        }

        switch (m_basicFeature->m_hevcSeqParams->MBBRC)
        {
        case mbBrcInternal:
        case mbBrcEnabled:
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 3;
            break;
        case mbBrcDisabled:
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 0;
            break;
        default:
            break;
        }

        // initQPIP, initQPB values will be used for BRC in the future
        int32_t initQPIP = 0, initQPB = 0;
        ComputeVDEncInitQP(initQPIP, initQPB);
        hucVdencBrcInitDmem->InitQPIP_U8 = (uint8_t)initQPIP;
        hucVdencBrcInitDmem->InitQPB_U8 = (uint8_t)initQPB;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetAcqpSettings(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucVdencBrcInitDmem =
            (VdencHevcHucBrcInitDmem *)params;

        hucVdencBrcInitDmem->BRCFlag = 0;

        // 0=No CUQP; 1=CUQP for I-frame; 2=CUQP for P/B-frame
        // bit operation, bit 1 for I-frame, bit 2 for P/B frame
        // In VDENC mode, the field "Cu_Qp_Delta_Enabled_Flag" should always be set to 1.
        if (m_basicFeature->m_hevcSeqParams->QpAdjustment)
        {
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 3;  // wPictureCodingType I:0, P:1, B:2
        }
        else
        {
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 0;  // wPictureCodingType I:0, P:1, B:2
        }

        hucVdencBrcInitDmem->InitQPIP_U8 = m_basicFeature->m_hevcPicParams->QpY + m_basicFeature->m_hevcSliceParams->slice_qp_delta;
        hucVdencBrcInitDmem->InitQPB_U8  = m_basicFeature->m_hevcPicParams->QpY + m_basicFeature->m_hevcSliceParams->slice_qp_delta;

        return MOS_STATUS_SUCCESS;
    }

    void HEVCEncodeBRC::ComputeVDEncInitQP(int32_t& initQPIP, int32_t& initQPB)
    {
        ENCODE_FUNC_CALL();

        const float x0 = 0, y0 = 1.19f, x1 = 1.75f, y1 = 1.75f;
        uint32_t frameSize = ((m_basicFeature->m_frameWidth * m_basicFeature->m_frameHeight * 3) >> 1);

        initQPIP = (int)(1. / 1.2 * pow(10.0, (log10(frameSize * 2. / 3. * ((float)m_basicFeature->m_hevcSeqParams->FrameRate.Numerator / ((float)m_basicFeature->m_hevcSeqParams->FrameRate.Denominator * (float)m_basicFeature->m_hevcSeqParams->TargetBitRate * m_brc_kbps))) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);

        initQPIP += 2;

        int32_t gopP    = (m_basicFeature->m_hevcSeqParams->GopRefDist) ? ((m_basicFeature->m_hevcSeqParams->GopPicSize - 1) / m_basicFeature->m_hevcSeqParams->GopRefDist) : 0;
        int32_t gopB    = m_basicFeature->m_hevcSeqParams->GopPicSize - 1 - gopP;
        int32_t gopB1 = 0;
        int32_t gopB2 = 0;
        int32_t gopSize = 1 + gopP + gopB + gopB1 + gopB2;

        if (gopSize == 1)
        {
            initQPIP += 12;
        }
        else if (gopSize < 15)
        {
            initQPIP += ((14 - gopSize) >> 1);
        }

        initQPIP = CodecHal_Clip3((int32_t)m_basicFeature->m_hevcPicParams->BRCMinQp, (int32_t)m_basicFeature->m_hevcPicParams->BRCMaxQp, initQPIP);
        initQPIP--;

        if (initQPIP < 0)
        {
            initQPIP = 1;
        }

        initQPB = ((initQPIP + initQPIP) * 563 >> 10) + 1;
        initQPB = CodecHal_Clip3((int32_t)m_basicFeature->m_hevcPicParams->BRCMinQp, (int32_t)m_basicFeature->m_hevcPicParams->BRCMaxQp, initQPB);

        if (gopSize > 300)  //if intra frame is not inserted frequently
        {
            initQPIP -= 8;
            initQPB -= 8;
        }
        else
        {
            initQPIP -= 2;
            initQPB -= 2;
        }

        initQPIP = CodecHal_Clip3((int32_t)m_basicFeature->m_hevcPicParams->BRCMinQp, (int32_t)m_basicFeature->m_hevcPicParams->BRCMaxQp, initQPIP);
        initQPB  = CodecHal_Clip3((int32_t)m_basicFeature->m_hevcPicParams->BRCMinQp, (int32_t)m_basicFeature->m_hevcPicParams->BRCMaxQp, initQPB);
    }

    void HEVCEncodeBRC::SetLcuBrc()
    {
        ENCODE_FUNC_CALL();

        if (m_brcEnabled)
        {
            switch (m_basicFeature->m_hevcSeqParams->MBBRC)
            {
            case mbBrcInternal:
                m_lcuBrcEnabled = (m_basicFeature->m_hevcSeqParams->TargetUsage == 1);
                break;
            case mbBrcDisabled:
                m_lcuBrcEnabled = false;
                break;
            case mbBrcEnabled:
                m_lcuBrcEnabled = true;
                break;
            }

            if (m_rcMode == RATECONTROL_ICQ ||
                m_rcMode == RATECONTROL_QVBR ||
                m_basicFeature->m_hevcPicParams->NumROI)
            {
                // ICQ or ROI must result in LCU-based BRC to be enabled.
                m_lcuBrcEnabled = true;
            }
        }

        if (m_rcMode == RATECONTROL_VCM && m_lcuBrcEnabled)
        {
            m_lcuBrcEnabled = false;  // when VCM is enabled, only frame-based BRC
        }
    }

    MOS_STATUS HEVCEncodeBRC::SetReadBrcPakStatsParams(
        uint8_t ucPass,
        uint32_t offset,
        PMOS_RESOURCE osResource,
        EncodeReadBrcPakStatsParams &readBrcPakStatsParams)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        readBrcPakStatsParams.pHwInterface               = m_hwInterface;
        readBrcPakStatsParams.presBrcPakStatisticBuffer  = m_vdencBrcBuffers.resBrcPakStatisticBuffer[m_vdencBrcBuffers.currBrcPakStasIdxForWrite];
        readBrcPakStatsParams.presStatusBuffer           = osResource;
        readBrcPakStatsParams.dwStatusBufNumPassesOffset = offset;
        readBrcPakStatsParams.ucPass                     = ucPass;

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HEVCEncodeBRC)
    {
        params.frameStatisticsStreamOut |= m_hevcVDEncAcqpEnabled || m_vdencBrcEnabled;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, HEVCEncodeBRC)
    {
        auto basicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeature);

        bool useDefaultQpDeltas = (m_hevcVDEncAcqpEnabled && basicFeature->m_hevcSeqParams->QpAdjustment) ||
                                  (m_brcEnabled && basicFeature->m_hevcSeqParams->MBBRC != 2 /*mbBrcDisabled*/);

        if (useDefaultQpDeltas)
        {
#if !(_MEDIA_RESERVED)
            params.extSettings.emplace_back(
                [this, basicFeature](uint32_t *data) {
                    data[13] |= 0xf0120000;
                    if (basicFeature->m_hevcPicParams->CodingType == I_TYPE)
                    {
                        data[14] |= 0x000021db;
                        data[16] |= 0x00010000;
                    }
                    else
                    {
                        data[14] |= 0x000021ed;
                        data[16] |= 0xd0010000;
                        data[18] |= 0x0060010f;
                        data[19] |= 0x000000c0;
                    }
                    return MOS_STATUS_SUCCESS;
                });
#else
            params.vdencCmd2Par44[0] = 2;
            params.vdencCmd2Par44[1] = 1;
            params.vdencCmd2Par44[2] = 0;
            params.vdencCmd2Par44[3] = -1;
            if (basicFeature->m_hevcPicParams->CodingType == I_TYPE)
            {
                params.vdencCmd2Par45[0] = -5;
                params.vdencCmd2Par45[1] = -3;
                params.vdencCmd2Par45[2] = 1;
                params.vdencCmd2Par45[3] = 2;
                params.vdencCmd2Par47    = 1;
                params.vdencCmd2Par48    = 0;
                params.vdencCmd2Par50    = 0;
                params.vdencCmd2Par52[0] = 0;
                params.vdencCmd2Par52[1] = 0;
                params.vdencCmd2Par52[2] = 0;
                params.vdencCmd2Par53[0] = 0;
                params.vdencCmd2Par53[1] = 0;
            }
            else
            {
                params.vdencCmd2Par45[0] = -3;
                params.vdencCmd2Par45[1] = -2;
                params.vdencCmd2Par45[2] = 1;
                params.vdencCmd2Par45[3] = 2;
                params.vdencCmd2Par47    = 1;
                params.vdencCmd2Par48    = 0;
                params.vdencCmd2Par50    = -3;
                params.vdencCmd2Par52[0] = -1;
                params.vdencCmd2Par52[1] = 0;
                params.vdencCmd2Par52[2] = 1;
                params.vdencCmd2Par53[0] = 96;
                params.vdencCmd2Par53[1] = 192;
            }
#endif  // !(_MEDIA_RESERVED)
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HEVCEncodeBRC)
    {
        ENCODE_FUNC_CALL();

        params.bAdvancedRateControlEnable = m_brcEnabled;
        params.bBRCEnabled = m_hevcVDEncAcqpEnabled || m_vdencBrcEnabled;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HEVCEncodeBRC)
    {
        ENCODE_FUNC_CALL();

        if (params.function == BRC_UPDATE)
        {
            params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, 0);  // Region 0 - History Buffer (Input/Output)
            params.regionParams[0].isWritable = true;
        
            params.regionParams[5].presRegion = const_cast<MOS_RESOURCE*>(&m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource);         // Region 5 - Output SLB Buffer (Output)
            params.regionParams[5].isWritable = true;

            // region 15 always in clear
            params.regionParams[15].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcDebugBuffer, 0);   // Region 15 - Debug Buffer (Output)
            params.regionParams[15].isWritable = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HEVCEncodeBRC::SetCurrRecycledBufIdx(const uint8_t index)
    {
        ENCODE_FUNC_CALL();

        m_currRecycledBufIdx = index;
        return MOS_STATUS_SUCCESS;
    }

    }  // namespace encode
