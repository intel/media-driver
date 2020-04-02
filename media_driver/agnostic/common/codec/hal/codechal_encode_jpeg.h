/*
* Copyright (c) 2012-2017, Intel Corporation
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
//! \file     codechal_encode_jpeg.h
//! \brief    Defines the encode interface extension for JPEG.
//! \details  Defines all types, macros, and functions required by CodecHal for JPEG encoding. Definitions are not externally facing.
//!

#ifndef __CODECHAL_ENCODER_JPEG_H__
#define __CODECHAL_ENCODER_JPEG_H__

#include "codechal_encoder_base.h"

//!
//! \struct CodechalEncodeJpegHuffTable
//! \brief Define the Huffman Table structure used by JPEG Encode
//!
struct CodechalEncodeJpegHuffTable
{
    uint32_t  m_tableClass; //!< table class
    uint32_t  m_tableID;    //!< table ID
    //This is the max size possible for these arrays, for DC table the actual occupied bits will be lesser
    // For AC table we need one extra byte to store 00, denoting end of huffman values
    uint8_t   m_huffSize[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL + 1]; //!< Huffman size, occupies 1 byte in HW command
    uint16_t  m_huffCode[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL + 1]; //!< Huffman code, occupies 2 bytes in HW command
};

#pragma pack(push,1)
//!
//! \struct CodechalEncodeJpegFrameHeader
//! \brief Define JPEG Frame Header structure
//!
struct CodechalEncodeJpegFrameHeader
{
    uint16_t m_sof;        //!< Start of frame marker
    uint16_t m_lf;         //!< Frame header length
    uint8_t  m_p;          //!< Precision
    uint16_t m_y;          //!< max number of lines in image
    uint16_t m_x;          //!< max number of samples per line in image
    uint8_t  m_nf;         //!< Number of image components in the frame

    struct
    {
        uint8_t  m_ci;              //!< Component identifier
        uint8_t  m_samplingFactori; //!< 4 MSBs are the horizontal and 4 LSBs are vertical sampling factors
        uint8_t  m_tqi;             //!< Quantization table selector (0-3)
    } m_codechalJpegFrameComponent[256];  //!< JPEG frame component array
};

//!
//! \struct CodechalJpegHuffmanHeader
//! \brief Define JPEG Huffman Header structure
//!
struct CodechalJpegHuffmanHeader
{
    uint16_t  m_dht;                                    //!< Define Huffman Table Marker
    uint16_t  m_lh;                                     //!< Huffman table definition length
    uint8_t   m_tableClassAndDestn;                     //!< 4 bits of Huffman table class (0 = DC, 1 = AC) and 4 bits of destination identifier
    uint8_t   m_li[JPEG_NUM_HUFF_TABLE_AC_BITS];        //!< List BITS of Huffman table
    uint8_t   m_vij[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL];    //!< Value associated with each huffman code
};

//!
//! \struct CodechalEncodeJpegQuantHeader
//! \brief Define JPEG Quant Header structure
//!
struct CodechalEncodeJpegQuantHeader
{
    uint16_t  m_dqt;                            //!< Define Quantization Marker
    uint16_t  m_lq;                             //!< Quantization table definition length
    uint8_t   m_tablePrecisionAndDestination;   //!< 4 bits of element precision and 4 bits of destination selector
    uint8_t   m_qk[JPEG_NUM_QUANTMATRIX];       //!< Quantization table elements
};

//!
//! \struct CodechalEncodeJpegRestartHeader
//! \brief Define JPEG Restart Header structure
//!
struct CodechalEncodeJpegRestartHeader
{
    uint16_t  m_dri;    //!< Define restart interval marker
    uint16_t  m_lr;     //!< Restart interval segment length
    uint16_t  m_ri;     //!< Restart interval
};

//!
//! \struct CodechalEncodeJpegScanComponent
//! \brief JPEG Scan Component structure
//!
struct CodechalEncodeJpegScanComponent
{
    uint8_t  m_csj;  //!< Scan component selector
    uint8_t  m_tdaj; //!< 4 bits of DC huffman table destination selector and 4 bits of AC huffman table selector
};

//!
//! \struct CodechalEncodeJpegScanHeader
//! \brief JPEG Scan Header structure
//!
struct CodechalEncodeJpegScanHeader
{
    uint16_t                          m_sos;                                  //!< Start of scan marker
    uint16_t                          m_ls;                                   //!< Scan header length
    uint8_t                           m_ns;                                   //!< Number of image components in scan
    CodechalEncodeJpegScanComponent   m_scanComponent[jpegNumComponent - 1];  //!< ignoring alpha component
    uint8_t                           m_ss;                                   //!< start of spectral selection
    uint8_t                           m_se;                                   //!< end of spectral selection
    uint8_t                           m_ahl;                                  //!< successive approximation bit position high and low
};
#pragma pack(pop)

//!  JPEG Encoder State class
//!
//!This class defines the JPEG encoder state, it includes
//!common member fields, functions, interfaces etc for all Gens.
//!
//!To create a JPEG encoder instance, client needs to call new(std::nothrow) CodechalEncodeJpegState(pEncoder)
//!
class CodechalEncodeJpegState : public CodechalEncoderState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeJpegState(
            CodechalHwInterface* hwInterface,
            CodechalDebugInterface* debugInterface,
            PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeJpegState() {};

    //derived from base class
    MOS_STATUS Initialize(CodechalSetting *settings) override;

    MOS_STATUS AllocateResources() override;

    void FreeResources() override;

    MOS_STATUS InitializePicture(const EncoderParams& params) override;

    virtual MOS_STATUS CheckResChangeAndCsc() override;

    MOS_STATUS ExecutePictureLevel() override;

    MOS_STATUS ExecuteSliceLevel() override;

    uint32_t CalculateCommandBufferSize() override;

    MOS_STATUS GetStatusReport(
            EncodeStatus*       encodeStatus,
            EncodeStatusReport* encodeStatusReport) override { return MOS_STATUS_SUCCESS;};

    MOS_STATUS ExecuteKernelFunctions() override { return MOS_STATUS_SUCCESS;};

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpQuantTables(
        CodecEncodeJpegQuantTable *quantTable);

    MOS_STATUS DumpPicParams(
        CodecEncodeJpegPictureParams *picParams);

    MOS_STATUS DumpScanParams(
        CodecEncodeJpegScanHeader *scanParams);

    MOS_STATUS DumpHuffmanTable(
        CodecEncodeJpegHuffmanDataArray *huffmanTable);
#endif

    // Variables
    static const uint32_t                       m_jpegEncodeSoi         = 0xFFD8;                              //!< JPEG Encode Header Markers SOI
    static const uint32_t                       m_jpegEncodeSos         = 0xFFDA;                              //!< JPEG Encode Header Markers SOS

    // Parameters passed by application
    CodecEncodeJpegPictureParams                *m_jpegPicParams        = nullptr;                             //!< Pointer to picture parameter
    CodecEncodeJpegScanHeader                   *m_jpegScanParams       = nullptr;                             //!< Pointer to scan parameter
    CodecEncodeJpegQuantTable                   *m_jpegQuantTables      = nullptr;                             //!< Pointer to quant tables
    CodecEncodeJpegHuffmanDataArray             *m_jpegHuffmanTable     = nullptr;                             //!< Pointer to Huffman table
    void                                        *m_applicationData      = nullptr;                             //!< Pointer to Application data
    CODEC_REF_LIST                              *m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG];            //!< Pointer to reference pictures, added for eStatus reporting

    // Other
    uint32_t                                    m_appDataSize         = 0;                                     //!< Pointer to Application data size
    bool                                        m_jpegQuantMatrixSent  = false;                                //!< JPEG: bool to tell if quant matrix was sent by the app or not
    bool                                        m_fullHeaderInAppData  = false;

protected:
    //!
    //! \brief    Map Huffman value index, implemented based on table K.5 in JPEG spec.
    //!
    //! \param    [in] huffValIndex
    //!           Huffman Value Index
    //!
    //! \return   The mapped index
    //!
    uint8_t MapHuffValIndex(uint8_t huffValIndex);

    //!
    //! \brief    Generate table of Huffman code sizes, implemented based on Flowchart in figure C.1 in JPEG spec
    //!
    //! \param    [in] bits
    //!           Contains the number of codes of each size
    //! \param    [out] huffSize
    //!           Huffman Size table
    //! \param    [out] lastK
    //!           Index of the last entry in the table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateSizeTable(
        uint8_t     bits[],
        uint8_t     huffSize[],
        uint8_t&    lastK);

    //!
    //! \brief    Generate table of Huffman codes, implemented based on Flowchart in figure C.2 in JPEG spec
    //!
    //! \param    [in] huffSize
    //!           Huffman Size table
    //! \param    [out] huffCode
    //!           Huffman Code table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateCodeTable(
        uint8_t     huffSize[],
        uint16_t    huffCode[]);

    //!
    //! \brief    Generate Huffman codes in symbol value order, implemented based on Flowchart in figure C.3 in JPEG spec
    //!
    //! \param    [in] huffVal
    //!           Huffman Value table
    //! \param    [in, out] huffSize
    //!           Huffman Size table
    //! \param    [in, out] huffCode
    //!           Huffman Code table
    //! \param    [in] lastK
    //!           Index of the last entry in the table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS OrderCodes(
        uint8_t     huffVal[],
        uint8_t     huffSize[],
        uint16_t    huffCode[],
        uint8_t     lastK);

    //!
    //! \brief    Convert Huffman data to table, including 3 steps: Step 1 - Generate size table, Step2 - Generate code table, Step 3 - Order codes.
    //!
    //! \param    [in] huffmanData
    //!           Huffman Data
    //! \param    [out] huffmanTable
    //!           Huffman table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConvertHuffDataToTable(
        CodecEncodeJpegHuffData             huffmanData,
        CodechalEncodeJpegHuffTable         *huffmanTable);

    //!
    //! \brief    Pack SOI
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackSOI(BSBuffer *buffer);

    //!
    //! \brief    Pack Application Data
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] appDataChunk
    //!           Application Data Chunk
    //! \param    [in] size
    //!           Application Data Size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackApplicationData(
        BSBuffer                        *buffer,
        uint8_t                         *appDataChunk,
        uint32_t                        size);

    //!
    //! \brief    Pack Frame Header
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] useSingleDefaultQuantTable
    //!           The flag of using single default Quant Table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackFrameHeader(
        BSBuffer                        *buffer,
        bool                            useSingleDefaultQuantTable);

    //!
    //! \brief    Pack Huffman Table
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] tableIndex
    //!           The Huffman Table Index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackHuffmanTable(
        BSBuffer                        *buffer,
        uint32_t                        tableIndex);

    //!
    //! \brief    Pack Quant Table
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //! \param    [in] componentType
    //!           The Component Type
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackQuantTable(
        BSBuffer                        *buffer,
        CodecJpegComponents             componentType);

    //!
    //! \brief    Pack Restart Interval
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackRestartInterval(
        BSBuffer                        *buffer);

    //!
    //! \brief    Pack Scan Header
    //!
    //! \param    [out] buffer
    //!           Bitstream buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackScanHeader(
        BSBuffer                        *buffer);
};

#endif //__CODECHAL_ENCODER_JPEG_H__
