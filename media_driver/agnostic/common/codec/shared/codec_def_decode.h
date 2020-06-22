/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codec_def_decoder.h
//! \brief    Defines the decode interface for CodecHal.
//! \details  The decode interface is further sub-divided by standard, this file is for the base interface which is shared by all decode standards.
//!

#ifndef __CODEC_DEF_DECODE_H__
#define __CODEC_DEF_DECODE_H__

#include "mos_os.h"
#include "codec_def_decode_jpeg.h"

struct CencDecodeShareBuf;

//!
//! \struct CodechalDecodeParams
//! \brief  Parameters passed in via Execute() to perform decoding.
//!
struct CodechalDecodeParams
{
    //! \brief Decode render target
    PMOS_SURFACE            m_destSurface = nullptr;
    //! \brief Reference frame surface
    PMOS_SURFACE            m_refFrameSurface = nullptr;
    //! \brief [VC1] Deblocked output of OLP
    PMOS_SURFACE            m_deblockSurface = nullptr;
    //! \brief Resource containing the bitstream (VLD mode) or residual difference (IT mode) data
    PMOS_RESOURCE           m_dataBuffer = nullptr;
    //! \brief [VC1] Resource containing the bitplane data
    PMOS_RESOURCE           m_bitplaneBuffer = nullptr;
    //! \brief [VP8 & VP9] resource containing coefficient probability data
    PMOS_RESOURCE           m_coefProbBuffer = nullptr;
    //! \brief [VP8 & VP9] resource containing the last reference surface which was not registered.
    PMOS_RESOURCE           m_presNoneRegLastRefFrame = nullptr;
    //! \brief [VP8 & VP9] resource containing the golden reference surface which was not registered.
    PMOS_RESOURCE           m_presNoneRegGoldenRefFrame = nullptr;
    //! \brief [VP8 & VP9] resource containing the alt reference surface which was not registered.
    PMOS_RESOURCE           m_presNoneRegAltRefFrame = nullptr;
    //! \brief [VC1 IT] Deblock data
    //!    For advanced profile P frames, this data should be formated as an array of 6 bytes for each MB:
    //!        Byte0: ILDBControlDataforY0
    //!        Byte1: ILDBControlDataforY1
    //!        Byte2: ILDBControlDataforY2
    //!        Byte3: ILDBControlDataforY3
    //!        Byte4: ILDBControlDataforCb
    //!        Byte5: ILDBControlDataforCr
    uint8_t                 *m_deblockData = nullptr;

    //! \brief [Codecs (HEVC & VP9)] Bitstream buffer data
    uint8_t                 *m_bitStreamBufData = nullptr;
    //! \brief Size of the data contained in m_dataBuffer
    uint32_t                m_dataSize = 0;
    //! \brief Offset of the data contained in presDataBuffer
    uint32_t                m_dataOffset = 0;
    //! \brief [VLD mode] Number of slices to be decoded
    uint32_t                m_numSlices = 0;
    //! \brief [IT mode] Number of MBs to be decoded
    uint32_t                m_numMacroblocks = 0;
    //! \brief [VC1] size of the data contained in m_bitplaneBuffer
    uint32_t                m_vc1BitplaneSize = 0;
    //! \brief [VP8 & VP9] Size of the data contained in m_coefProbBuffer
    uint32_t                m_coefProbSize = 0;
    //! \brief [VC1 IT] Size of the data contained in m_deblockData
    uint32_t                m_deblockDataSize = 0;
    //! \brief Number of reference frame surface
    uint32_t                m_refSurfaceNum = 0;

    //! \brief Indicates whether or not stream out information should be returned to the DDI
    bool                    m_streamOutEnabled = false;
    //! \brief Resource to contain the stream out output from HW
    PMOS_RESOURCE           m_externalStreamOutBuffer = nullptr;

    //! \brief [CENC Decode] Status reporting number associated with the current frame.
    uint16_t                m_cencDecodeStatusReportNum = 0;
    CencDecodeShareBuf      *m_cencBuf = nullptr;

    //! \brief Picture level parameters to be used for decoding
    void                    *m_picParams = nullptr;
    //! \brief Additional picture level parameters to be used for decoding.
    //!      In certain cases additional parameters are needed to supplement m_picParams (MVC, etc).
    void                    *m_extPicParams = nullptr;
    //! \brief Picture Level parameters for HEVC advanced feature
    void                    *m_advPicParams = nullptr;
    //! \brief [VLD mode] Slice level parameters to be used for decoding
    void                    *m_sliceParams = nullptr;
    //!< [VLD LF mode] Intel long format
    void                    *m_subsetParams = nullptr;
    //! \brief Additional slice level parameters to be used for decoding.
    //!      In certain cases additional parameters are needed to supplement m_sliceParams (MVC, etc).
    void                    *m_extSliceParams = nullptr;
    //! \brief Inverse quant matrix data
    void                    *m_iqMatrixBuffer = nullptr;
    //! \brief [IT mode] MB level parameters to be used for decoding
    void                    *m_macroblockParams = nullptr;

    //! \brief Reference count used for downsampling, If non-zero enables downsampling of the render target.
    uint32_t                m_refFrameCnt = 0;
#ifdef _DECODE_PROCESSING_SUPPORTED
    //! \brief Parameters used for processing the decode render target, if invalid, decode render target processing will not be used.
    void                    *m_procParams = nullptr;
#endif
    //! \brief [Predication] Resource for predication
    PMOS_RESOURCE           m_presPredication = nullptr;
    //! \brief [Predication] Offset for Predication resource
    uint64_t                m_predicationResOffset = 0;
    //! \brief [Predication] Predication mode
    bool                    m_predicationNotEqualZero = false;

    //! \brief [Predication] Indicates whether or not Predication is enabled
    bool                    m_predicationEnabled = false;
    //! \brief [Predication] Temp buffer for Predication
    PMOS_RESOURCE           *m_tempPredicationBuffer = nullptr;

    //! \brief [JPEG] Huffman table data
    void                    *m_huffmanTable = nullptr;
    //! \brief [JPEG] Describes the layout of the decode render target
    CodecDecodeJpegImageLayout m_outputSurfLayout = {{0}};

    //! \brief [AVC] Indicates whethe or not PicId remapping is in use
    bool                    m_picIdRemappingInUse = false;

    // MPEG2 Specific Parameters
    //! \brief [MPEG2] MPEG2 I slice concealment mode
    uint32_t                m_mpeg2ISliceConcealmentMode = 0;
    //! \brief [MPEG2] MPEG2 P/B slice concealment mode
    uint32_t                m_mpeg2PBSliceConcealmentMode = 0;
    //! \brief [MPEG2] MPEG2 P/B Slice Predicted BiDir Motion Type Override
    uint32_t                m_mpeg2PBSlicePredBiDirMVTypeOverride = 0;
    //! \brief [MPEG2] MPEG2 P/B Slice Predicted Motion Vector Override
    uint32_t                m_mpeg2PBSlicePredMVOverride = 0;

    //! \brief [VP8] Indicates whether or not the driver is expected to parse parameters from the frame header
    bool                    m_bitstreamLockingInUse = false;
    //! \brief [VP8] Indicates whether or not the bitstream buffer may be directly locked to perform header parsing.
    bool                    m_bitstreamLockable = false;

    //! \brief [SetMarker] Indicates whether or not SetMarker is enabled
    bool                    m_setMarkerEnabled = false;
    //! \brief [SetMarker] Resource for SetMarker
    PMOS_RESOURCE           m_presSetMarker = nullptr;
    //! \brief [SetMarker] Number Timestamp for SetMarker
    uint32_t                setMarkerNumTs = 0;
    //! \brief [CENC Decode] Indicates if it's a full frame
    bool                    m_bFullFrameData = false;
    //! \brief MSDK event handling
    HANDLE                  m_gpuAppTaskEvent;
    //! \brief execution call index in multiple execution call mode
    uint32_t                m_executeCallIndex = 0;
};

typedef enum _CODECHAL_DUMMY_REFERENCE_STATUS
{
    CODECHAL_DUMMY_REFERENCE_INVALID,
    CODECHAL_DUMMY_REFERENCE_DPB,
    CODECHAL_DUMMY_REFERENCE_DEST_SURFACE,
    CODECHAL_DUMMY_REFERENCE_ALLOCATED
} CODECHAL_DUMMY_REFERENCE_STATUS;

//!
//! \class CodechalDecodeRestoreData
//! \brief This class restore the data when destory, used as temporal storage and restore automatically.
//!
template   <typename T>
class CodechalDecodeRestoreData
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalDecodeRestoreData(T *mem):
        m_mem(mem)
    {
        if (m_mem != nullptr)
        {
            m_restoreValue = *mem;
        }
    }
    //!
    //! \brief    Constructor
    //!
    CodechalDecodeRestoreData(T *mem, T restoreValue):
        m_mem(mem)
    {
        if (m_mem != nullptr)
        {
            m_restoreValue = restoreValue;
        }
    }

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeRestoreData()
    {
        if (m_mem != nullptr)
        {
            *m_mem = m_restoreValue;
        }
    }

private:
    T * m_mem = nullptr;          //!< Point to the memory need to be restored when this class destroy
    T m_restoreValue = {};   //!< The value to be restored to memory when this class destroy
};
#endif  // __CODEC_DEF_DECODE_H__
