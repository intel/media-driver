/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_decode_jpeg.h
//! \brief    Defines the decode interface extension for JPEG.
//! \details  Defines all types, macros, and functions required by CodecHal for JPEG decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_JPEG_H__
#define __CODECHAL_DECODER_JPEG_H__

#include "codechal_decoder.h"
#include "codechal_decode_sfc_jpeg.h"

//!
//! \def CODECHAL_DECODE_JPEG_BLOCK_SIZE
//! Jpeg block size
//!
#define CODECHAL_DECODE_JPEG_BLOCK_SIZE            8

typedef class CodechalDecodeJpeg *PCODECHAL_DECODE_JPEG_STATE;

//!
//! \class CodechalDecodeJpeg
//! \brief This class defines the member fields, functions etc used by JPEG decoder.
//!
class CodechalDecodeJpeg: public CodechalDecode
{
public:
    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeJpeg(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeJpeg(const CodechalDecodeJpeg&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeJpeg& operator=(const CodechalDecodeJpeg&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeJpeg();

    //!
    //! \brief    Allocate and Initialize JPEG decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateStandard (
        CodechalSetting *settings) override;

    //!
    //! \brief  Set states for each frame to prepare for JPEG decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetFrameStates () override;

    //!
    //! \brief    JPEG decoder state level function
    //! \details  State level function for JPEG decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DecodeStateLevel () override;

    //!
    //! \brief    JPEG decoder primitive level function
    //! \details  Primitive level function for GEN specific JPEG decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DecodePrimitiveLevel () override;

    //!
    //! \brief  Indicates whether or not the jpeg scan is incomplete
    //! \return If jpeg scan is incomplete \see m_incompleteJpegScan
    //!
    bool IsIncompleteJpegScan() override { return m_incompleteJpegScan; }

    MOS_STATUS InitMmcState() override;

#ifdef _DECODE_PROCESSING_SUPPORTED
    virtual MOS_STATUS InitSfcState();
#endif

    MOS_SURFACE               m_destSurface;    //!< Pointer to MOS_SURFACE of render surface
    CodecDecodeJpegPicParams *m_jpegPicParams = nullptr;  //!< Picture parameter for JPEG

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpIQParams(
        CodecJpegQuantMatrix *matrixData);

    MOS_STATUS DumpPicParams(
        CodecDecodeJpegPicParams *picParams);

    MOS_STATUS DumpScanParams(
        CodecDecodeJpegScanParameter *scanParams);

    MOS_STATUS DumpHuffmanTable(
        PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable);
#endif

protected:

    //!
    //! \brief    Initialize during begin frame
    //! \details  Initialize during begin frame in JPEG decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeBeginFrame();

    //!
    //! \brief    Copy data surface
    //! \details  Copy data surface in JPEG decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CopyDataSurface();
    //!
    //! \brief    Check supported format
    //! \details  Check supported format in JPEG decode driver
    //! \param    [in,out] format
    //!           MOS_Format to check
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckSupportedFormat(PMOS_FORMAT format);
    //!
    //! \brief    Check and copy incomplete bit stream
    //! \details  Check and copy incomplete bit stream in JPEG decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckAndCopyIncompleteBitStream();
    //!
    //! \brief    Allocate resources
    //! \details  Allocate resources for JPEG decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources ();
    //!
    //! \brief    Set output surface layout
    //! \details  Set output surface layout for JPEG decode driver
    //! \param    [out] outputSurfLayout
    //!           Pointer to CodecDecodeJpegImageLayout
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void SetOutputSurfaceLayout(
        CodecDecodeJpegImageLayout *outputSurfLayout);

protected:
    //!
    //! \brief  Indicates whether or not the SFC is inuse
    //!         JPEG may not use SFC even when FtrSFCPipe == True, but it can't be known when creating device.
    //! \return If SFC is inuse
    //!
    bool IsSfcInUse(CodechalSetting * codecHalSettings) override
    {
        return (codecHalSettings->sfcEnablingHinted && MEDIA_IS_SKU(m_skuTable, FtrSFCPipe));

    }

protected:
    uint32_t                            m_dataSize;          //!< Data size of the bitstream
    uint32_t                            m_dataOffset;        //!< Data offset of the bitstream
    CodecDecodeJpegScanParameter *      m_jpegScanParams = nullptr;    //!< Scan parameter for JPEG
    CodecJpegQuantMatrix *              m_jpegQMatrix = nullptr;       //!< QMatrix for JPEG
    PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE m_jpegHuffmanTable;  //!< Huffman table for JPEG

    MOS_RESOURCE m_resDataBuffer;          //!< Handle of bitstream buffer
    MOS_RESOURCE m_resCopiedDataBuffer;    //!< The internal buffer to store copied data
    uint32_t     m_copiedDataBufferSize;   //!< The max size of the internal copied buffer
    uint32_t     m_nextCopiedDataOffset;   //!< The offset of the next bitstream data used for copying
    uint32_t     m_totalDataLength;        //!< The total data length
    uint32_t     m_preNumScans;            //!< Record the previous scan number before the new scan comes
    bool         m_copiedDataBufferInUse;  //!< Flag to indicate whether the copy data buffer is used

    //! \brief Indicates if current input scan for Jpeg is incomplete
    bool                    m_incompleteJpegScan = false;

    MOS_RESOURCE m_resSyncObjectWaContextInUse;     //!< Signals on the video WA context
    MOS_RESOURCE m_resSyncObjectVideoContextInUse;  //!< Signals on the video context

#ifdef _DECODE_PROCESSING_SUPPORTED
    CodechalJpegSfcState *m_sfcState = nullptr;  //!< SFC state
#endif
};
#endif
