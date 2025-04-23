/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_basic_feature.h
//! \brief    Defines the common interface for decode badic feature
//! \details  The decode basic feature interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#ifndef __DECODE_BASIC_FEATURE_H__
#define __DECODE_BASIC_FEATURE_H__

#include "codec_def_decode.h"
#include "decode_allocator.h"
#include "media_feature.h"
#include "codec_hw_next.h"
#include "codechal_setting.h"

namespace decode {

class DecodeBasicFeature: public MediaFeature
{
public:
    DecodeBasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface);
    virtual ~DecodeBasicFeature();

    //!
    //! \brief  Init decode basic parameter
    //! \param  [in] setting
    //!         Pointer to CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting);

    //!
    //! \brief  Update decode basic feature
    //! \param  [in] params
    //!         Pointer to DecoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params);

    //!
    //! \brief  Update decode dest surface
    //! \param  [in] destSurface
    //!         Decode render target from DDI
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateDestSurface(MOS_SURFACE &destSurface);

#ifdef _MMC_SUPPORTED
    //!
    //! \brief  Set MMC state
    //! \param  [in] isMmcEnabled
    //!         Flag to indicate if the MMC is enabled
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMmcState(bool isMmcEnabled)
    {
        m_isMmcEnabled = isMmcEnabled;
        return MOS_STATUS_SUCCESS;
    }
#endif

    //!
    //! \brief  Get MMC state
    //! \return bool
    //!         Return true if MMC enabled
    //!
    bool IsMmcEnabled()
    {
#ifdef _MMC_SUPPORTED
        return m_isMmcEnabled;
#else
        return false;
#endif
    }

    uint32_t            m_width = 0;                                    //!< Frame width in luma samples
    uint32_t            m_height = 0;                                   //!< Frame height in luma samples
    uint16_t            m_picWidthInMb = 0;                             //!< Picture Width in MB width count
    uint16_t            m_picHeightInMb = 0;                            //!< Picture Height in MB height count

    uint32_t            m_frameNum = 0;                                 //!< Frame number, inc by each codec pipeline
    bool                m_secondField = false;                          //!< Indicates if current field is second field(bottom field)
    uint16_t            m_pictureCodingType = 0;                        //!< I, P, B or mixed frame

    CODEC_PICTURE       m_curRenderPic = {0};                           //!< picture information of current render target

    CODECHAL_STANDARD   m_standard = CODECHAL_UNDEFINED;                //!< Decode standard
    CODECHAL_MODE       m_mode = CODECHAL_UNSUPPORTED_MODE;             //!< Decode mode
    CODECHAL_FUNCTION   m_codecFunction = CODECHAL_FUNCTION_INVALID;    //!< Decode function

    HCP_CHROMA_FORMAT_IDC m_chromaFormat = HCP_CHROMA_FORMAT_YUV420;    //!< Chroma format(420, 422 etc)
    uint8_t             m_bitDepth = 8;                                 //!< Bit depth
    bool                m_is10Bit = false;

    uint32_t            m_numSlices = 0;                                //!< [VLD mode] Number of slices to be decoded

    MOS_SURFACE         m_destSurface;                                  //!< Decode render target

    MOS_BUFFER          m_resDataBuffer;                                //!< Decode input bitstream
    uint32_t            m_dataOffset = 0;
    uint32_t            m_dataSize = 0;                                 //!< Size of the bitstream required on this picture
    PMOS_SURFACE        m_refFrameSurface = nullptr;                    //!< Handle of reference frame surface
    uint32_t            m_refSurfaceNum = 0;                            //!< Number of reference frame surface

    bool                m_reportFrameCrc = false;                       //!< Flag to indicate if report frame CRC

    bool                m_disableDecodeSyncLock = false;                //!< Indicates if decode sync lock is disabled

    bool                m_setMarkerEnabled = false;                     //!< [SetMarker] Indicates whether or not SetMarker is enabled
    PMOS_RESOURCE       m_presSetMarker = nullptr;                      //!< [SetMarker] Resource for SetMarker

    bool                            m_useDummyReference = false;        //!< Indicates if use dummy reference
    MOS_SURFACE                     m_dummyReference;                   //!< Dummy reference surface
    CODECHAL_DUMMY_REFERENCE_STATUS m_dummyReferenceStatus = CODECHAL_DUMMY_REFERENCE_INVALID; //!< Indicate the status of dummy reference
    constexpr static uint8_t m_invalidFrameIndex = 0xff;                //!< Invalid frame index
    constexpr static uint8_t m_maxFrameIndex = CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; //!< Max frame index

protected:
    //!
    //! \brief  Set required bitstream size by each codec
    //! \param  [in] requiredSize
    //!         required size for current frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) = 0;

    void *                 m_hwInterface = nullptr;
    DecodeAllocator *      m_allocator   = nullptr;
    PMOS_INTERFACE         m_osInterface = nullptr;

#ifdef _MMC_SUPPORTED
    bool                   m_isMmcEnabled = false;   //!< Indicate MMC enabled for current picture
#endif

MEDIA_CLASS_DEFINE_END(decode__DecodeBasicFeature)
};

}//decode

#endif // !__DECODE_BASIC_FEATURE_H__
