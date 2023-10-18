/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     ddi_encode_avc_specific.h
//! \brief    Defines class for DDI media avc encode
//!

#ifndef __DDI_ENCODE_AVC_SPECIFIC_H__
#define __DDI_ENCODE_AVC_SPECIFIC_H__

#include "ddi_encode_base_specific.h"

namespace encode
{

class AvcInBits
{
public:
    AvcInBits() = delete;

    AvcInBits(uint8_t *pInBits, uint32_t BitSize);

    virtual ~AvcInBits() {};

    void SkipBits(uint32_t n);

    uint32_t GetBit();

    uint32_t GetBits(uint32_t n);

    uint32_t GetUE();

    uint32_t GetBitOffset();

    void ResetBitOffset();
 
protected:
    uint8_t const *m_pInBits = nullptr;
    uint32_t m_BitSize = 0;
    uint32_t m_BitOffset = 0;

MEDIA_CLASS_DEFINE_END(encode__AvcInBits) 
};

class AvcOutBits
{
public:
    AvcOutBits() = delete;

    AvcOutBits(uint8_t *pOutBits, uint32_t BitSize);

    virtual ~AvcOutBits() {};

    void PutBit(uint32_t v);

    void PutBits(uint32_t v, uint32_t n);

    uint32_t GetBitOffset();

protected:
    uint8_t *m_pOutBits = nullptr;
    uint8_t *m_pOutBitsEnd = nullptr;
    uint32_t m_BitSize = 0;
    uint32_t m_ByteSize = 0;
    uint32_t m_BitOffset = 0;

MEDIA_CLASS_DEFINE_END(encode__AvcOutBits)
};

//!
//! \class  DdiEncodeAvc
//! \brief  Ddi encode AVC
//!
class DdiEncodeAvc : public encode::DdiEncodeBase
{
public:
    //!
    //! \brief    Constructor
    //!
    DdiEncodeAvc(){};

    //!
    //! \brief    Destructor
    //!
    virtual ~DdiEncodeAvc();

    virtual VAStatus ContextInitialize(CodechalSetting * codecHalSettings) override;

    virtual VAStatus RenderPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VABufferID       *buffers,
        int32_t          numBuffers) override;

    //!
    //! \brief    Parse Qp matrix
    //! \details  Parse Qp matrix which called by RenderPicture
    //!
    //! \param    [in] ptr
    //!           Pointer to buffer VAIQMatrixBufferH264
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus Qmatrix(void *ptr);

    //!
    //! \brief    Parse Sequence parameters
    //! \details  Parse Sequence parameters which called by RenderPicture
    //!
    //! \param    [in] ptr
    //!           Pointer to buffer VAEncSequenceParameterBufferH264
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    virtual VAStatus ParseSeqParams(void *ptr);

    //!
    //! \brief    Parse slice parameters
    //! \details  Parse slice parameters which called by RenderPicture
    //!
    //! \param    [in] mediaCtx
    //!           Ddi media context
    //!
    //! \param    [in] ptr
    //!           Pointer to buffer VAEncSliceParameterBufferH264
    //!
    //! \param    [in] numSlices
    //!           Number of slices
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseSlcParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr,
        uint32_t          numSlices);

    //!
    //! \brief    Find the NAL Unit Start Codes
    //! \details  Find the NAL unit start codes offset and NAL Unit start codes 
    //!           length in packed header NAL unit data buffer
    //!
    //! \param    [in] buf
    //!           Pointer to packed header NAL unit data
    //! \param    [in] size
    //!           byte size of packed header NAL unit data
    //! \param    [in] startCodesOffset
    //!           Pointer to NAL unit start codes offset from the packed header
    //!           NAL unit data buf
    //! \param    [in] startCodesLength
    //!           Pointer to NAL unit start codes length
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success, 
    //!           else VA_STATUS_ERROR_INVALID_BUFFER if start codes doesn't exit
    //!
    VAStatus FindNalUnitStartCodes(
        uint8_t * buf,
        uint32_t size,
        uint32_t * startCodesOffset,
        uint32_t * startCodesLength);

    //!
    //! \brief    Parse packedHeader paramters
    //! \details  Parse packedHeader parameters which called by RenderPicture
    //!
    //! \param    [in] ptr
    //!           Pointer to buffer VAEncPackedHeaderParameterBuffer
    //!
    //! \return   MOS_STATUS
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParsePackedHeaderParams(void *ptr);

    //!
    //! \brief    Parse packedHeader data
    //! \details  Parse packedHeader data which called by RenderPicture
    //!
    //! \param    [in] ptr
    //!           Pointer to packedHeader data
    //!
    //! \return   MOS_STATUS
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParsePackedHeaderData(void *ptr);

    //!
    //! \brief    Parse misc parameters
    //! \details  Parse misc parameters data which called by RenderPicture
    //!           including hrd, framerate, ratecontrol, private, quantization,
    //!           RIR, SkipFrame, FrameSize, QualityLevel, SliceSize, ROI,
    //!           CustomRoundingControl, SubMbPartPelIntel, ROIPrivate
    //!
    //! \param    [in] ptr
    //!           Pointer to misc paramters buffer
    //!
    //! \return   MOS_STATUS
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    virtual VAStatus ParseMiscParams(void *ptr);

protected:
    uint32_t getSliceParameterBufferSize() override;

    uint32_t getSequenceParameterBufferSize() override;

    uint32_t getPictureParameterBufferSize() override;

    uint32_t getQMatrixBufferSize() override;

    virtual VAStatus EncodeInCodecHal(uint32_t numSlices) override;

    virtual VAStatus ResetAtFrameLevel() override;

    virtual VAStatus ParsePicParams(
        DDI_MEDIA_CONTEXT *mediaCtx,
        void              *ptr) override;

    virtual void ClearPicParams() override;

    virtual MOS_STATUS CheckPackedSlcHeaderData(
        void *pInSlcHdr,
        uint32_t InBitSize,
        void **ppOutSlcHdr,
        uint32_t &OutBitSize);

    //!
    //! \brief    Convert slice struct from VA to codechal
    //! \details  Convert slice struct from VA to codechal
    //!
    //! \param    [in] vaSliceStruct
    //!           VA Slice Struct
    //!
    //! \return   uint8_t
    //!           CODECHAL_SLICE_STRUCT_ARBITRARYROWSLICE,
    //!           CODECHAL_SLICE_STRUCT_POW2ROWS,
    //!           CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE,
    //!           CODECHAL_SLICE_STRUCT_ROWSLICE,
    //!           default is CODECHAL_SLICE_STRUCT_ONESLICE
    //!
    uint8_t ConvertSliceStruct(uint32_t vaSliceStruct);

    //!
    //! \brief    Convert va slice type to codechal picture type
    //! \details  Convert va slice type to codechal picture type
    //!
    //! \param    [in] vaSliceType
    //!           VA Slice Type
    //!
    //! \return   uin8_t
    //!           I_TYPE, P_TYPE, B_TYPE
    //!
    uint8_t CodechalPicTypeFromVaSlcType(uint8_t vaSliceType);

    //!
    //! \brief    Get profile from va profile
    //! \details  Get profile from va profile
    //!
    //! \return   CODEC_AVC_PROFILE_IDC
    //!           CODEC_AVC_BASE_PROFILE, CODEC_AVC_MAIN_PROFILE
    //!           or CODEC_AVC_HIGH_PROFILE, default is
    //!           CODEC_AVC_MAIN_PROFILE
    //!
    CODEC_AVC_PROFILE_IDC GetAVCProfileFromVAProfile();

    //!
    //! \brief    Parse DirtyROI params
    //! \details  Parse DirtyROI params
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterBufferDirtyROI
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamDirtyROI(void *data);

    //!
    //! \brief    Parse frame rate settings
    //! \details  Parse VAEncMiscParameterFrameRate to
    //!           PCODEC_AVC_ENCODE_SEQUENCE_PARAMS
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterFrameRate
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamFR(void *data);

    //!
    //! \brief    Parse HRD settings
    //! \details  Parse VAEncMiscParameterHRD to
    //!           PCODEC_AVC_ENCODE_SEQUENCE_PARAMS
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterHRD
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamHRD(void *data);

    //!
    //! \brief    Parse max frame size setting
    //! \details  Parse VAEncMiscParameterMaxFrameSize to
    //!           PCODECHAL_AVC_ENCODE_SEQ_PARAMS
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterBufferMaxFrameSize
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamMaxFrameSize(void *data);

    //!
    //! \brief    Parse max frame size setting for multiple pass
    //! \details  Parse VAEncMiscParameterBufferMultiPassFrameSize
    //!           to PCODEC_AVC_ENCODE_PIC_PARAMS
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterBufferMultiPassFrameSize
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamMultiPassFrameSize(void *data);

    //!
    //! \brief    Parse sliceSizeInBytes for enabling VDENC dynamic slice
    //! \details  Parse sliceSizeInBytes for enabling VDENC dynamic slice
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterMaxSliceSize
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamMaxSliceSize(void *data);

    //!
    //! \brief    Parse enc quality parameters settings
    //! \details  Parse enc quality parameters settings
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterPrivate
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!

    VAStatus ParseMiscParamEncQuality(void *data);

    //!
    //! \brief    Parse qualtiy level settings
    //! \details  Parse quality level(Target Usage) settings passed by app
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterBufferQualityLevel
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamQualityLevel(void *data);

    //!
    //! \brief    Parse trellis related settings
    //! \details  Parse trellis related settings
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterQuantization
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamQuantization(void *data);

    //!
    //! \brief    Parse rate control related settings
    //! \details  Parse rate control related settings
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterRateControl
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamRC(void *data);

    //!
    //! \brief    Parse region of interest settings
    //! \details  Parse region of interest settings
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterBufferROI
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamROI(void *data);

    //!
    //! \brief    Parse rounding parameters for slices
    //! \details  Parse rounding parameters for slices
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterCustomRoundingControl
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamRounding(void *data);

    //!
    //! \brief    Parse rolling intra refresh setting
    //! \details  Parse rolling intra refresh setting, Row/Colum
    //!           rolling supported on Gen8
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterSkipFrame
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamSkipFrame(void *data);

    //!
    //! \brief    Parse sub macroblock partition setting
    //! \details  Parse sub marcoblock partition setting
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterSubMbPartPelH264
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParamSubMbPartPel(void *data);

    //!
    //! \brief    Parse rolling intra refresh setting
    //! \details  Parse rolling intra refresh setting, Row/Colum
    //!           rolling supported on Gen8
    //!
    //! \param    [in] data
    //!           Pointer to buffer VAEncMiscParameterRIR
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if successful, else fail reason
    //!
    VAStatus ParseMiscParameterRIR(void *data);

    //!
    //! \brief    Get Slice Reference Index
    //! \details  To change the value of  sliceParams->RefPicList[][].FrameIdx
    //!           from the frame itself  to the index in picParams->RefFrameList
    //!
    //! \param    [in] picReference
    //!           Pointer to CODEC_PICTURE
    //! \param    [in] slcReference
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void GetSlcRefIdx(CODEC_PICTURE *picReference, CODEC_PICTURE *slcReference);

    //!
    //! \brief    Setup Codec Picture for AVC
    //!
    //! \param    [in] mediaCtx
    //!           Pointer to DDI_MEDIA_CONTEXT
    //! \param    [in] rtTbl
    //!           Pointer to DDI_CODEC_RENDER_TARGET_TABLE
    //! \param    [in] vaPic
    //!           H264 VAPicture structure
    //! \param    [in] fieldPicFlag
    //!           Field picture flag
    //! \param    [in] picReference
    //!           Reference picture flag
    //! \param    [in] sliceReference
    //!           Reference slice flag
    //! \param    [out] codecHalPic
    //!           Pointer to CODEC_PICTURE
    //!
    //! \return   void
    //!
    void SetupCodecPicture(
        DDI_MEDIA_CONTEXT                   *mediaCtx,
        DDI_CODEC_RENDER_TARGET_TABLE       *rtTbl,
        CODEC_PICTURE                       *codecHalPic,
        VAPictureH264                       vaPic,
        bool                                fieldPicFlag,
        bool                                picReference,
        bool                                sliceReference);

    CODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS *m_qcParams = nullptr;       //!< Quality control parameters.
    CODECHAL_ENCODE_AVC_ROUNDING_PARAMS     *m_roundingParams = nullptr; //!< Rounding parameters.
    uint8_t                                 m_weightScale4x4[6][16];     //!< Inverse quantization weight scale 4x4.
    uint8_t                                 m_weightScale8x8[2][64];     //!< Inverse quantization weight scale 8x8.
    uint16_t                                m_previousFRper100sec = 0;   //!< For saving FR value to be used in case of dynamic BRC reset.

    //!
    //! \brief    Return the CODECHAL_FUNCTION type for give profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   Codehal function
    //!
    CODECHAL_FUNCTION GetEncodeCodecFunction(VAProfile profile, VAEntrypoint entrypoint, bool bVDEnc) override;
    //!
    //! \brief    Return internal encode mode for given profile and entrypoint
    //!
    //! \param    [in] profile
    //!           Specify the VAProfile
    //!
    //! \param    [in] entrypoint
    //!           Specify the VAEntrypoint
    //!
    //! \return   Codehal mode
    //!
    CODECHAL_MODE GetEncodeCodecMode(VAProfile profile, VAEntrypoint entrypoint) override;

private:
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

    //! \brief H.264 current picture parameter set id
    uint8_t current_pic_parameter_set_id = 0;

    //! \brief H.264 current sequence parameter set id
    uint8_t current_seq_parameter_set_id = 0;

    //! \brief H.264 Inverse Quantization Matrix Buffer.
    PCODEC_AVC_IQ_MATRIX_PARAMS iqMatrixParams = nullptr;

    //! \brief H.264 Inverse Quantization Weight Scale.
    PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS iqWeightScaleLists = nullptr;

MEDIA_CLASS_DEFINE_END(encode__DdiEncodeAvc)
};

}
#endif /* __DDI_ENCODE_AVC_SPECIFIC_H__ */
