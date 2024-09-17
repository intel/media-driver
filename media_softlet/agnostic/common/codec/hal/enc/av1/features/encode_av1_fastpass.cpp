/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     encode_av1_fastpass.cpp
//! \brief    Fast Pass feature
//!

#include "encode_av1_fastpass.h"

namespace encode
{
Av1FastPass::Av1FastPass(MediaFeatureManager *featureManager,
    EncodeAllocator                          *allocator,
    CodechalHwInterfaceNext                  *hwInterface,
    void                                     *constSettings):
    MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

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

            m_fastPassDownScaleRatio = (outValue_ratio.Get<int32_t>() == 1) ? 1 : 2;
            m_fastPassDownScaleType  = (uint8_t)outValue_type.Get<int32_t>();
        }
    }

    Av1FastPass::~Av1FastPass()
    {
        ENCODE_FUNC_CALL();
    }

    MOS_STATUS Av1FastPass::Update(void *params)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams = m_basicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(av1PicParams);

        if (AV1_FAST_PASS_4X_DS(m_fastPassDownScaleRatio))
        {
            // 4x down scale and av1 needs 8-pixels aligned
            m_dsWidth  = MOS_ALIGN_FLOOR((av1PicParams->frame_width_minus1 + 1) >> 2, 8);
            m_dsHeight = MOS_ALIGN_FLOOR((av1PicParams->frame_height_minus1 + 1) >> 2, 8);
        }else
        {
            // 2x down scale and av1 needs 8-pixels aligned
            m_dsWidth  = MOS_ALIGN_FLOOR((av1PicParams->frame_width_minus1 + 1) >> 1, 8);
            m_dsHeight = MOS_ALIGN_FLOOR((av1PicParams->frame_height_minus1 + 1) >> 1, 8);        
        }
     
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1FastPass)
    {
        if (m_enabled)
        {
            params.fastPassEn    = m_enabled;
            params.fastPassScale = m_fastPassDownScaleRatio;  // fastPassScale:0 indicates 4x4 ds, 1 indicates 2x2 ds
            params.DownScaleType = m_fastPassDownScaleType;   // DownScaleType:0 indicates bilinear, 1 indicates NN

            params.chromaType     = AVP_CHROMA_FORMAT_YUV420;
            params.bitDepthMinus8 = 0;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {
            params.width  = m_dsWidth;
            params.height = m_dsHeight;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams = m_basicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(av1PicParams);
        bool m_enablePalette = av1PicParams->PicFlags.fields.PaletteModeEnable;

        if (m_enabled)
        {
            params.tileWidth   = m_dsWidth;
            params.tileHeight  = m_dsHeight;

            if (m_enablePalette && m_basicFeature->m_is10Bit)
            {
                params.VdencHEVCVP9TileSlicePar5 -= 2;
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {
            //one tile supported only
            params.tileSliceStartLcuMbX = 0;
            params.tileSliceStartLcuMbY = 0;
            params.nextTileSliceStartLcuMbX = MOS_ROUNDUP_DIVIDE(m_dsWidth, av1SuperBlockWidth);
            params.nextTileSliceStartLcuMbY = MOS_ROUNDUP_DIVIDE(m_dsHeight, av1SuperBlockHeight);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {
            params.width  = m_dsWidth;
            params.height = m_dsHeight;
            params.format = Format_NV12;

            if (m_basicFeature->m_chromaFormat == AVP_CHROMA_FORMAT_YUV444
                || m_basicFeature->m_chromaFormat == AVP_CHROMA_FORMAT_YUV422)
            {
                params.vOffset = params.uOffset;
            }
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {
            uint32_t dsWidth4x  = m_dsWidth / SCALE_FACTOR_4x;
            uint32_t dsHeight4x = m_dsHeight / SCALE_FACTOR_4x;

            params.heightStage1 = dsHeight4x >> 1;
            params.widthStage1  = dsWidth4x >> 1;
            params.heightStage2 = dsHeight4x;
            params.widthStage2  = dsWidth4x;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_TILE_CODING, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {     
            //one tile supported only
            params.tileColPositionInSb = 0;
            params.tileRowPositionInSb = 0;
            params.tileWidthInSbMinus1  = MOS_ROUNDUP_DIVIDE(m_dsWidth, av1SuperBlockWidth) - 1;
            params.tileHeightInSbMinus1 = MOS_ROUNDUP_DIVIDE(m_dsHeight, av1SuperBlockHeight) - 1;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {
            params.frameWidthMinus1  = m_dsWidth - 1;
            params.frameHeightMinus1 = m_dsHeight - 1;
            for (auto i = 0; i <= av1NumInterRefFrames; i++)
            {
                params.refFrameRes[i] = CAT2SHORTS(m_dsWidth - 1, m_dsHeight - 1);
            }
            params.chromaFormat = AVP_CHROMA_FORMAT_YUV420;
            params.bitDepthIdc  = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1FastPass)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled)
        {
            if (m_basicFeature->m_chromaFormat == AVP_CHROMA_FORMAT_YUV444
                || m_basicFeature->m_chromaFormat == AVP_CHROMA_FORMAT_YUV422)
            {
                params.vOffset = params.uOffset;
            }
            params.srcFormat          = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
            params.bitDepthLumaMinus8 = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace encode
