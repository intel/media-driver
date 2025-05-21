/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_ddi_encode_av1.h
//! \brief    Defines class for DDI media av1 encode
//!

#ifndef __MEDIA_LIBVA_ENCODER_AV1_H__
#define __MEDIA_LIBVA_ENCODER_AV1_H__

#define TILE_GROUP_NUM_INCREMENT 8

#include "media_ddi_encode_base.h"
#include "codec_def_encode_av1.h"
#include "codec_def_encode_vp9.h"

//!
//! \class DdiEncodeAV1
//! \brief Ddi encode AV1
//!
class DdiEncodeAV1 : public DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeAV1(){};

    //!
    //! \brief    Destructor
    //!
    ~DdiEncodeAV1();

    //!
    //! \brief    Initialize Encode Context and CodecHal Setting for AV1
    //!
    //! \param    [out] codecHalSettings
    //!           Pointer to CodecHalSetting *
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successs, else fail reason
    //!
    VAStatus ContextInitialize(CodechalSetting *codecHalSettings) override;

    //!
    //! \brief    Parse buffer to the server.
    //!
    //! \param    [in] ctx
    //!           Pointer to VADriverContextP
    //! \param    [in] context
    //!           VA context ID
    //! \param    [in] buffers
    //!           Pointer to VABufferID
    //! \parsm    [in] numBuffers
    //!           Number of buffers
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

protected:
    //!
    //! \brief    Reset Encode Context At Frame Level
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ResetAtFrameLevel() override;

    //!
    //! \brief    Encode in CodecHal for Hevc
    //!
    //! \param    [in] numSlices
    //!           Number of slice data structures
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus EncodeInCodecHal(uint32_t numSlices) override;

    //!
    //! \brief    Parse Picture Parameter buffer to Encode Context
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to Picture Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePicParams(DDI_MEDIA_CONTEXT *mediaCtx, void *ptr) override;

    //!
    //! \brief    Parse Sequence Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Sequence Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSeqParams(void *ptr);

    //!
    //! \brief    Parse Slice Parameter buffer to Encode Context
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] ptr
    //!           Pointer to Slice Parameter buffer
    //! \param    [in] numSlices
    //!           Number of slice
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseTileGroupParams(void *ptr, uint32_t numTileGroupParams);

    //!
    //! \brief    Parse Packed Header Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Parameter buffer
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePackedHeaderParams(void *ptr);

    //!
    //! \brief    Parse Packed Header Data buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Packed Header Data buffer
    //!
    //! return    VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParsePackedHeaderData(void *ptr);

    //!
    //! \brief    Parse Misc Parameter buffer to Encode Context
    //!
    //! \param    [in] ptr
    //!           Pointer to Misc Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParams(void *ptr);

    //!
    //! \brief    Parse Segment Map Parameter buffer to Encode Context
    //!
    //! \param    [in] buf
    //!           Pointer to Segment Map Parameter buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseSegMapParams(void *ptr);

    uint32_t getSequenceParameterBufferSize() override;

    uint32_t getPictureParameterBufferSize() override;

    uint32_t getSliceParameterBufferSize() override;

private:
    //!
    //! \brief    Setup Codec Picture for AV1
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] surfaceID
    //!           VASurface index
    //! \param    [in] picReference
    //!           Reference picture flag
    //! \param    [out] codecHalPic
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void SetupCodecPicture(
        DDI_MEDIA_CONTEXT                     *mediaCtx,
        DDI_CODEC_RENDER_TARGET_TABLE         *rtTbl,
        CODEC_PICTURE                         *codecHalPic,
        VASurfaceID                           surfaceID,
        bool                                  picReference);

    //!
    //! \brief    Parse Misc Param VBV Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param VBV Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamVBV(void *data);

    //!
    //! \brief    Parse Misc Param FrameRate Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param FR Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamFR(void *data);

    //!
    //! \brief    Parse Misc Param RateControl Data buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param RC Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamRC(void *data);

    //!
    //! \brief    Parse Misc Param Enc Quality to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Private Data buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamEncQuality(void *data);

    //!
    //! \brief    Parse Misc Parameter Temporal Layer Params buffer to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Parameter Temporal Layer Params buffer
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamTemporalLayerParams(void *data);

    //!
    //! \brief    Parse Misc Param Buffer Quality Level to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Buffer Quality Level
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamQualityLevel(void *data);

    //!
    //! \brief    Parse Misc Param Max Frame Size to Encode Context
    //!
    //! \param    [in] data
    //!           Pointer to Misc Param Buffer Max Frame Size
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus ParseMiscParamMaxFrameSize(void *data);

    //!
    //! \brief    Setup Codec Picture for AV1
    //!
    //! \param    [in] picParams
    //!           Pointer to picture parameters
    //! \param    [in] platform
    //!           Pointer to platform
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus CheckCDEF(const VAEncPictureParameterBufferAV1 *picParams,
        PRODUCT_FAMILY platform);

    VAStatus CheckTile(const VAEncPictureParameterBufferAV1 *picParams);

    //!
    //! \brief    Check whether swizzle needed
    //!
    //! \param    [in] rawSurface
    //!           Pointer of Raw Surface
    //!
    //! \return   bool, true if need, otherwise false
    //!
    inline bool NeedDisplayFormatSwizzle(DDI_MEDIA_SURFACE *rawSurface)
    {
        if (Media_Format_A8R8G8B8 == rawSurface->format ||
            Media_Format_X8R8G8B8 == rawSurface->format ||
            Media_Format_B10G10R10A2 == rawSurface->format)
        {
            return true;
        }

        return false;
    }
    uint32_t savedTargetBit[ENCODE_AV1_MAX_NUM_TEMPORAL_LAYERS]  = { 0 };
    uint32_t savedFrameRate[ENCODE_AV1_MAX_NUM_TEMPORAL_LAYERS]  = { 0 };
    uint32_t savedMaxBitRate[ENCODE_AV1_MAX_NUM_TEMPORAL_LAYERS] = { 0 };
    uint32_t savedQualityFactor = 0;

    uint32_t allocatedTileNum = 0;
    bool m_isSegParamsChanged = false;
};
#endif  //__MEDIA_LIBVA_ENCODER_AV1_H__
