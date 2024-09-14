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
//! \file     encode_hevc_vdenc_fastpass.cpp
//! \brief    Defines the Xe2_HPM+ Base interface for hevc encode fastpass features
//!

#include "encode_vdenc_hevc_fastpass.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode
{

HevcVdencFastPass::HevcVdencFastPass(
    MediaFeatureManager     *featureManager,
    EncodeAllocator         *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void* constSettings):
    MediaFeature(constSettings,hwInterface ? hwInterface->GetOsInterface() : nullptr)
{
    ENCODE_FUNC_CALL();
    auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_hevcFeature = dynamic_cast<HevcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hevcFeature);

    //regkey to control fast pass encode settings
    MediaUserSetting::Value outValue;
    ReadUserSetting(m_userSettingPtr,
        outValue,
        "Enable Fast Pass Encode",
        MediaUserSetting::Group::Sequence);

    m_enableFastPass = outValue.Get<bool>();
    if (m_enableFastPass)
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

MOS_STATUS HevcVdencFastPass::Update(void *params)
{
    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_NULL_RETURN(m_hevcFeature);
    m_hevcSeqParams = m_hevcFeature->m_hevcSeqParams;
    ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);

    // hevc needs 8-pixels aligned
    m_dsWidth  = MOS_ALIGN_FLOOR((m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3) >> m_fastPassShiftIndex, 8);
    m_dsHeight = MOS_ALIGN_FLOOR((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3) >> m_fastPassShiftIndex, 8);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.fastPassEn    = m_enableFastPass;
    params.fastPassScale = m_fastPassShiftIndex == 2 ? 0 : 1;  // fastPassScale:0 indicates 4x4 ds, fastPassScale:1 indicates 2x2 ds
    params.DownScaleType = m_fastPassDownScaleType;            // DownScaleType:0 indicates bilinear, DownScaleType:1 indicates NN
    params.chromaType     = 1;                                 // Fast pass uses 420 type
    params.bitDepthMinus8 = 0;                                 // Fast pass uses bitdepth 8
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.width  = m_dsWidth;
    params.height = m_dsHeight;
    
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t widthInPix  = m_dsWidth;
    uint32_t heightInPix = m_dsHeight;
    //no tiling
    params.tileWidth = widthInPix;
    params.tileHeight = heightInPix;

    if (m_hevcSeqParams->palette_mode_enabled_flag && m_hevcSeqParams->bit_depth_luma_minus8 == 2)
    {
        uint32_t SliceQP = m_hevcFeature->m_hevcPicParams->QpY + m_hevcFeature->m_hevcSliceParams->slice_qp_delta;
        uint32_t tableIdx;
        if (SliceQP <= 12)
        {
            tableIdx = 0;
        }
        else if (SliceQP > 12 && SliceQP <= 17)
        {
            tableIdx = 1;
        }
        else if (SliceQP > 17 && SliceQP <= 22)
        {
            tableIdx = 2;
        }
        else if (SliceQP > 22 && SliceQP <= 27)
        {
            tableIdx = 3;
        }
        else if (SliceQP > 27 && SliceQP <= 32)
        {
            tableIdx = 4;
        }
        else if (SliceQP > 32 && SliceQP <= 37)
        {
            tableIdx = 5;
        }
        else if (SliceQP > 37 && SliceQP <= 42)
        {
            tableIdx = 6;
        }
        else if (SliceQP > 42 && SliceQP <= 47)
        {
            tableIdx = 7;
        }
        else if (SliceQP > 47 && SliceQP <= 49)
        {
            tableIdx = 8;
        }
        else
        {
            tableIdx = 9;
        }
    
        static const uint32_t table[3][10][11] =
            {
                {
                    {16},
                    {16},
                    {16},
                    {16},
                    {32},
                    {48},
                    {64},
                    {96},
                    {128},
                    {256},
                },
                {
                    {16},
                    {16},
                    {16},
                    {16},
                    {32},
                    {48},
                    {64},
                    {96},
                    {128},
                    {256},
                },
                {
                    {256},
                    {256},
                    {256},
                    {256},
                    {256},
                    {768},
                    {768},
                    {768},
                    {768},
                    {768},
                },
            };

        params.VdencHEVCVP9TileSlicePar5  = 4;
        params.VdencHEVCVP9TileSlicePar14 = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][0];
        params.VdencHEVCVP9TileSlicePar6  >>= 2;
        params.VdencHEVCVP9TileSlicePar7  >>= 2;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto                            t_sliceParams = m_hevcFeature->m_hevcSliceParams;
    CODEC_HEVC_ENCODE_SLICE_PARAMS *sliceParams   = (CODEC_HEVC_ENCODE_SLICE_PARAMS *)&t_sliceParams[m_hevcFeature->m_curNumSlices];
    
    uint32_t ctbSize     = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t widthInCtb  = (m_dsWidth / ctbSize) + ((m_dsWidth % ctbSize) ? 1 : 0);  // round up
    uint32_t heightInCtb = (m_dsHeight / ctbSize) + ((m_dsHeight % ctbSize) ? 1 : 0);  // round up
    //no tiling
    params.tileSliceStartLcuMbY     = sliceParams->slice_segment_address / widthInCtb;
    params.nextTileSliceStartLcuMbX = (sliceParams->slice_segment_address + widthInCtb * heightInCtb) / heightInCtb;
    params.nextTileSliceStartLcuMbY = (sliceParams->slice_segment_address + widthInCtb * heightInCtb) / widthInCtb;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.width  = m_dsWidth;
    params.height = m_dsHeight;
    
    //Fast pass uses 420 type
    if (m_hevcSeqParams->chroma_format_idc != 1)
    {
        params.format = Format_NV12;
        params.vOffset = params.uOffset;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t dsWidth4x  = m_dsWidth / SCALE_FACTOR_4x;
    uint32_t dsHeight4x = m_dsHeight / SCALE_FACTOR_4x;

    params.heightStage1 = dsHeight4x >> 1;
    params.widthStage1  = dsWidth4x >> 1;
    params.heightStage2 = dsHeight4x;
    params.widthStage2  = dsWidth4x;

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.framewidthinmincbminus1  = (m_dsWidth >> 3) - 1;
    params.frameheightinmincbminus1 = (m_dsHeight >> 3) - 1;
    
    //Fast pass  uses 420 type, bitdepth 8
    if (m_hevcSeqParams->chroma_format_idc != 1)
    {
        params.chromaSubsampling = 1;
        params.lcuMaxBitSizeAllowedMsb2its >>= 1;
        params.lcuMaxBitsizeAllowed >>= 1;
    }
    if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
    {
        params.bitDepthChromaMinus8 = 0;
        params.bitDepthLumaMinus8   = 0;
    }

    //RDOQ TU7 is related to framesize, it needs to be downscaled.
    if (m_hevcFeature->m_targetUsage == 7)
    {
        uint32_t frameSize_ds       = m_dsWidth * m_dsHeight;        
        params.rdoqintratuthreshold = MOS_MIN(((frameSize_ds * 30) / 100) >> 8, 0xffff);
    }
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    using namespace mhw::vdbox;

    //Fast pass  uses 420 type, bitdepth 8
    if (m_hevcSeqParams->chroma_format_idc != 1)
    {
        params.yOffsetForVCr = (uint16_t)params.yOffsetForUCbInPixel;
    }
    params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HEVC_VP9_RDOQ_STATE, HevcVdencFastPass)
{
    ENCODE_FUNC_CALL();

    if (!m_enableFastPass)
    {
        return MOS_STATUS_SUCCESS;
    }

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = m_hevcFeature->m_hevcPicParams;
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    //Fast pass  uses bitdepth 8
    uint8_t bitDepthLumaMinus8   = 0;
    uint8_t bitDepthChromaMinus8 = 0;
    uint8_t codingType           = hevcPicParams->CodingType;
    auto    settings             = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    uint32_t sliceTypeIdx = (codingType == I_TYPE) ? 0 : 1;

    //Intra lambda
    MOS_ZeroMemory(params.lambdaTab, sizeof(params.lambdaTab));
    if (bitDepthLumaMinus8 == 0)
    {
        std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][0][0].begin(),
            settings->rdoqLamdas8bits[sliceTypeIdx][0][0].end(),
            std::begin(params.lambdaTab[0][0]));

        std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][0][1].begin(),
            settings->rdoqLamdas8bits[sliceTypeIdx][0][1].end(),
            std::begin(params.lambdaTab[0][1]));

        std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][1][0].begin(),
            settings->rdoqLamdas8bits[sliceTypeIdx][1][0].end(),
            std::begin(params.lambdaTab[1][0]));

        std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][1][1].begin(),
            settings->rdoqLamdas8bits[sliceTypeIdx][1][1].end(),
            std::begin(params.lambdaTab[1][1]));
    }

    if (m_hevcFeature->m_hevcRDOQPerfDisabled)
    {
        params.disableHtqPerformanceFix0 = true;
        params.disableHtqPerformanceFix1 = true;
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
