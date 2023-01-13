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
//! \file     decode_avc_basic_feature.h
//! \brief    Defines the common interface for decode avc basic feature
//!
#ifndef __DECODE_AVC_BASIC_FEATURE_H__
#define __DECODE_AVC_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_avc.h"
#include "codec_def_cenc_decode.h"
#include "decode_avc_reference_frames.h"
#include "decode_avc_mv_buffers.h"

namespace decode {

//!
//! \def DECODE_AVC_MONOPIC_CHROMA_DEFAULT
//! default chroma value for mono picture
//!
#define DECODE_AVC_MONOPIC_CHROMA_DEFAULT        0x80

class AvcBasicFeature : public DecodeBasicFeature
{
public:
    //!
    //! \brief  AvcBasicFeature constructor
    //!
    AvcBasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) :
        DecodeBasicFeature(allocator, hwInterface, osInterface)
    {
        if (osInterface != nullptr)
        {
            m_osInterface = osInterface;
        }
    };

    //!
    //! \brief  AvcBasicFeature deconstructor
    //!
    virtual ~AvcBasicFeature();

    //!
    //! \brief  Initialize avc basic feature CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update avc decodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Get os interface
    //! \return PMOS_INTERFACE
    //!
    PMOS_INTERFACE GetOsInterface()
    {
        return m_osInterface;
    }

    //!
    //! \brief  Detect conformance conflict and do error concealment
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else show assert message
    //!
    MOS_STATUS ErrorDetectAndConceal();

    struct SliceRecord
    {
        uint32_t   skip;
        uint32_t   offset;
        uint32_t   length;
        uint32_t   totalBytesConsumed;
    };
    enum AvcSliceType
    {
        avcSliceP = 0,
        avcSliceB = 1,
        avcSliceI = 2
    };
    const AvcSliceType AvcBsdSliceType[10] =
    {
        AvcSliceType::avcSliceP,
        AvcSliceType::avcSliceB,
        AvcSliceType::avcSliceI,
        AvcSliceType::avcSliceP,
        AvcSliceType::avcSliceI,
        AvcSliceType::avcSliceP,
        AvcSliceType::avcSliceB,
        AvcSliceType::avcSliceI,
        AvcSliceType::avcSliceP,
        AvcSliceType::avcSliceI
    };

    bool IsAvcPSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(AvcBsdSliceType)) ? (AvcBsdSliceType[sliceType] == avcSliceP) : false;
    }
    bool IsAvcBSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(AvcBsdSliceType)) ? (AvcBsdSliceType[sliceType] == avcSliceB) : false;
    }
    bool IsAvcISlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(AvcBsdSliceType)) ? (AvcBsdSliceType[sliceType] == avcSliceI) : false;
    }

    enum AvcQmTypes
    {
        avcQmIntra4x4 = 0,
        avcQmInter4x4 = 1,
        avcQmIntra8x8 = 2,
        avcQmInter8x8 = 3
    };


    PCODEC_AVC_PIC_PARAMS           m_avcPicParams            = nullptr;      //!< Pointer to AVC picture parameter
    PCODEC_MVC_EXT_PIC_PARAMS       m_mvcExtPicParams         = nullptr;      //!< Pointer to MVC ext picture parameter
    PCODEC_AVC_SLICE_PARAMS         m_avcSliceParams          = nullptr;      //!< Pointer to AVC slice parameter
    PCODEC_AVC_IQ_MATRIX_PARAMS     m_avcIqMatrixParams       = nullptr;      //!< Pointer to AVC IQ matrix parameter

    bool                            m_picIdRemappingInUse     = false;        //!< Indicate PicId Remapping are in use
    uint32_t                        m_fullFeildsReceived      = 0;            //!< Indicate if fields are completed
    bool                            m_fullFrameData           = false;        //!< Indicate it is a full frame
    PMOS_BUFFER                     m_resMonoPicChromaBuffer  = nullptr;      //!< Handle of MonoPicture's default Chroma data surface

    bool                            m_shortFormatInUse        = false;        //!< Indicate it is Short Format
    bool                            m_deblockingEnabled       = false;        //!< Indicate Deblocking is enabled
    bool                            m_streamOutEnabled        = false;        //!< Indicates if stream out enabled
    bool                            m_isSecondField           = false;        //!< Indicate it is second field
    PMOS_RESOURCE                   m_externalStreamOutBuffer = nullptr;      //!< Stream out buffer from HW
    uint32_t                        m_fixedFrameIdx           = 0xff;         //!< This is used for interlace case which second field reference to first field
                                                                              //!< with the same frame index, need to save this frame index into refFrameList.
    uint32_t                        m_lastValidSlice          = 0;
    uint32_t                        m_slcLength               = 0;
    uint32_t                        m_slcOffset               = 0;
    bool                            m_usingVeRing             = false;
    // CencDecode buffer
    CencDecodeShareBuf              *m_cencBuf                = nullptr;

    std::vector<SliceRecord>        m_sliceRecord;                            //!< Record slice info
    AvcReferenceFrames              m_refFrames;                              //!< Reference frames
    std::vector<uint32_t>           m_refFrameIndexList;                      //!< Reference frame index list
    RefrenceAssociatedBuffer<MOS_BUFFER, AvcMvBufferOpInf, AvcBasicFeature> m_mvBuffers; //!< Reference associated buffers

protected:
    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;
    MOS_STATUS SetPictureStructs();
    MOS_STATUS SetSliceStructs();
    virtual MOS_STATUS CheckBitDepthAndChromaSampling();

    PMOS_INTERFACE        m_osInterface  = nullptr;

MEDIA_CLASS_DEFINE_END(decode__AvcBasicFeature)
};

}//decode

#endif // !__DECODE_AVC_BASIC_FEATURE_H__
