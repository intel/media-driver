/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_avc_vdenc_fast_pass.cpp
//! \brief    Defines the common interface for encode avc Xe2_HPM+ fast pass feature
//!

#include "encode_avc_vdenc_fastpass.h"
#include "encode_avc_vdenc_feature_manager.h"
#include <array>

namespace encode
{

AvcVdencFastPass::AvcVdencFastPass(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) :
    MediaFeature(constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    if (hwInterface)
    {
        m_userSettingPtr = hwInterface->GetOsInterface()->pfnGetUserSettingInstance(hwInterface->GetOsInterface());
    }
    //regkey to control fast pass encode settings
    MediaUserSetting::Value outValue;
    ReadUserSetting(m_userSettingPtr,
        outValue,
        "Enable Fast Pass Encode",
        MediaUserSetting::Group::Sequence);

    m_enabled = outValue.Get<bool>();
    if (m_enabled)
    {
        MediaUserSetting::Value outValue_ratio;
        MediaUserSetting::Value outValue_type;
#if (_DEBUG || _RELEASE_INTERNAL)

        ReadUserSetting(m_userSettingPtr,
            outValue_ratio,
            "Fast Pass Encode Downscale Ratio",
            MediaUserSetting::Group::Sequence);
        ReadUserSetting(m_userSettingPtr,
            outValue_type,
            "Fast Pass Encode Downscale Type",
            MediaUserSetting::Group::Sequence);
#endif

        if (outValue_ratio.Get<int32_t>() == 0)
        {
            m_fastPassShiftIndex = 2;
        }
        else if (outValue_ratio.Get<int32_t>() == 1)
        {
            m_fastPassShiftIndex = 1;
        }
        else
            m_fastPassShiftIndex = 2;
        m_fastPassDownScaleType = (uint8_t)outValue_type.Get<int32_t>();
    }
}

MOS_STATUS AvcVdencFastPass::Update(void *params)
{
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams = m_basicFeature->m_seqParam;
    ENCODE_CHK_NULL_RETURN(avcSeqParams);

    m_dsWidth  = MOS_ALIGN_FLOOR(avcSeqParams->FrameWidth >> m_fastPassShiftIndex, 16);
    m_dsHeight = MOS_ALIGN_FLOOR(avcSeqParams->FrameHeight >> m_fastPassShiftIndex, 16);
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        params.fastPassEn    = m_enabled;
        params.fastPassScale = m_fastPassShiftIndex == 2 ? 0 : 1;  // fastPassScale:0 indicates 4x4 ds, fastPassScale:1 indicates 2x2 ds
        params.DownScaleType = m_fastPassDownScaleType;            // DownScaleType:0 indicates bilinear, DownScaleType:1 indicates NN
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();
    
    if (m_enabled)
    {
        params.width  = MOS_ALIGN_FLOOR(m_basicFeature->m_reconSurface.dwWidth >> m_fastPassShiftIndex, 16);
        params.height = MOS_ALIGN_FLOOR(m_basicFeature->m_reconSurface.dwHeight >> m_fastPassShiftIndex, 16);    
    }

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        params.pictureHeightMinusOne = (m_dsHeight / 16) - 1;
        params.pictureWidth          = m_dsWidth / 16;    
    }

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        auto frameHeight      = m_dsHeight / 16;
        auto frameWidth       = m_dsWidth / 16;

        auto     sliceParams               = &(m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices]);
        uint32_t MbNums                    = frameHeight * frameWidth;
        auto     nextsliceMbStartYPosition = (sliceParams->first_mb_in_slice + MbNums) / frameWidth;
        params.tileSliceStartLcuMbY        = sliceParams->first_mb_in_slice / frameWidth;
        params.nextTileSliceStartLcuMbY    = nextsliceMbStartYPosition > frameHeight ? frameHeight : nextsliceMbStartYPosition;    
    }

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        uint32_t numMBs    = (m_dsWidth / 16) * (m_dsHeight / 16);
        params.frameSize   = (numMBs > 0xFFFF) ? 0xFFFF : numMBs;
        params.frameHeight = (m_dsHeight / 16) - 1;
        params.frameWidth  = (m_dsWidth / 16) - 1;    
    }

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(MFX_AVC_SLICE_STATE, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        auto     sliceParams = &(m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices]);
        uint32_t startMbNum  = sliceParams->first_mb_in_slice * (1 + m_basicFeature->m_seqParam->mb_adaptive_frame_field_flag);
        uint16_t widthInMb   = m_dsWidth / 16;
        uint16_t heightInMb  = m_dsHeight / 16;
        uint16_t MbNums      = widthInMb * heightInMb;

        params.sliceHorizontalPosition     = startMbNum % widthInMb;
        params.sliceVerticalPosition       = startMbNum / widthInMb;
        params.nextSliceHorizontalPosition = (startMbNum + MbNums) % widthInMb;
        params.nextSliceVerticalPosition   = (startMbNum + MbNums) / widthInMb;
        params.isLastSlice                 = (startMbNum + MbNums) >= (uint32_t)(widthInMb * heightInMb);    
    }

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        uint32_t dsWidth4x  = m_dsWidth / 4;
        uint32_t dsHeight4x = m_dsHeight / 4;

        params.heightStage1 = dsHeight4x;
        params.widthStage1  = dsWidth4x;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD3, AvcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams = m_basicFeature->m_seqParam;
        ENCODE_CHK_NULL_RETURN(avcSeqParams);
        PCODEC_AVC_ENCODE_PIC_PARAMS avcPicParams = m_basicFeature->m_picParam;
        ENCODE_CHK_NULL_RETURN(avcPicParams);
        PCODEC_AVC_ENCODE_SLICE_PARAMS avcSliceParams = m_basicFeature->m_sliceParams;
        ENCODE_CHK_NULL_RETURN(avcSliceParams);

        auto     pictureType   = CodecHal_Clip3(0, 2, avcPicParams->CodingType - 1);
        uint16_t picWidthInMb  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_dsWidth);
        uint16_t picHeightInMb = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_dsHeight);
        auto     fastpass_qp   = CodecHal_Clip3(10, 51, avcPicParams->QpY + avcSliceParams->slice_qp_delta);
        auto     codingType    = avcPicParams->CodingType;
        // Only do this lambda offset for lower resolution and high QP range.
        uint16_t gopP = (avcSeqParams->GopRefDist) ? ((avcSeqParams->GopPicSize - 1) / avcSeqParams->GopRefDist) : 0;
        uint16_t gopB = avcSeqParams->GopPicSize - 1 - gopP;
        uint16_t numB = ((gopP > 0) ? (gopB / gopP) : 0);
        if ((numB != 0) && (picWidthInMb * 16 < 1920) && (picHeightInMb * 16 < 1080) && (fastpass_qp >= 32))
        {
            uint8_t index0 = avcPicParams->RefPicFlag ? 0 : 1;
            static const std::array<
                std::array<
                    uint8_t,
                    3>,
                2>
                table = {{
                    {0, 1, 1},
                    {0, 1, 2},
                }};
            fastpass_qp += table[index0][pictureType];
            fastpass_qp = CodecHal_Clip3(0, 51, fastpass_qp);

            for (auto i = 0; i < 8; i++)
            {
                params.vdencCmd3Par0[i] = m_CMD3Settings.AvcVdencCMD3ConstSettings_0[i][fastpass_qp];
            }

            if (codingType == I_TYPE || codingType == P_TYPE)
                params.vdencCmd3Par19 = m_CMD3Settings.AvcVdencCMD3ConstSettings_3[pictureType][fastpass_qp];

            uint8_t isIPGOP          = avcSeqParams->GopRefDist == 1 ? 1 : 0;
            uint8_t codingTypeMinus1 = avcPicParams->CodingType - 1;
            uint8_t refPic           = avcPicParams->RefPicFlag;

            params.vdencCmd3Par31 = m_CMD3Settings.par31Table[isIPGOP][codingTypeMinus1][refPic][fastpass_qp];
            params.vdencCmd3Par32 = m_CMD3Settings.par32Table[isIPGOP][codingTypeMinus1][refPic][fastpass_qp];
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode