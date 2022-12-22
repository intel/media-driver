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
//! \file     encode_jpeg_packet.h
//! \brief    Defines the interface to adapt to JPEG pipeline
//!

#ifndef __ENCODE_JPEG_PACKET_H__
#define __ENCODE_JPEG_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_jpeg_pipeline.h"
#include "encode_jpeg_basic_feature.h"
#include "encode_jpeg_packer_feature.h"
#include "encode_status_report.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_mi_itf.h"

namespace encode
{
struct EncodeJpegHuffTable
{
    uint32_t m_tableClass;  //!< table class
    uint32_t m_tableID;     //!< table ID
    //This is the max size possible for these arrays, for DC table the actual occupied bits will be lesser
    // For AC table we need one extra byte to store 00, denoting end of huffman values
    uint8_t  m_huffSize[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL + 1];  //!< Huffman size, occupies 1 byte in HW command
    uint16_t m_huffCode[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL + 1];  //!< Huffman code, occupies 2 bytes in HW command
};

struct EncodeJpegHuffTableParams
{
    uint32_t    HuffTableID;
    uint8_t     pDCCodeLength[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]; // 12 values of 1 byte each
    uint16_t    pDCCodeValues[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]; // 12 values of 2 bytes each
    uint8_t     pACCodeLength[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]; // 162 values of 1 byte each
    uint16_t    pACCodeValues[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]; // 162 values of 2 bytes each
};

class JpegPkt : public CmdPacket, public MediaStatusReportObserver, public mhw::vdbox::mfx::Itf::ParSetting, public mhw::mi::Itf::ParSetting
{
public:

    JpegPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

    virtual ~JpegPkt() {}

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Destroy the media packet and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    //!
    //! \brief  One frame is completed
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in] rcsStatus
    //!         pointer to status buffer which for RCS
    //! \param  [in, out] statusReport
    //!         pointer of EncoderStatusReport
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //!
    //! \brief    Calculate Mfx Commands Size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS CalculateMfxCommandsSize();

    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize);

    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize);

    MmioRegistersMfx *SelectVdboxAndGetMmioRegister(
        MHW_VDBOX_NODE_IND  index,
        PMOS_COMMAND_BUFFER pCmdBuffer);

    //!
    //! \brief    Read Image status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for Image status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatus(
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER           cmdBuffer);

protected:

    //!
    //! \brief    Add command to read Mfc status
    //!
    //! \param    [in, out] cmdBuffer
    //!           Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadMfcStatus(MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Add picture-level MFX commands to command buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Reference to the command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddPictureMfxCommands(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

    MOS_STATUS SendPrologCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    
    MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

    MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer);

    MOS_STATUS PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);

    void SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType);

    //!
    //! \brief  Calculate Command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual uint32_t CalculateCommandBufferSize();

    //!
    //! \brief  Calculate Patch List Size
    //!
    //! \return uint32_t
    //!         Patchlist size calculated
    //!
    virtual uint32_t CalculatePatchListSize();

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
        uint8_t  bits[],
        uint8_t  huffSize[],
        uint8_t &lastK);

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
        uint8_t  huffSize[],
        uint16_t huffCode[]);

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
        uint8_t  huffVal[],
        uint8_t  huffSize[],
        uint16_t huffCode[],
        uint8_t  lastK);

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
        CodecEncodeJpegHuffData huffmanData,
        EncodeJpegHuffTable *huffmanTable);

    //!
    //! \brief    Add SOI
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddSOI(PMOS_COMMAND_BUFFER cmdBuffer) const;

    //!
    //! \brief    Add Application Data
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddApplicationData(PMOS_COMMAND_BUFFER cmdBuffer) const;

    //!
    //! \brief    Add Quant Tables
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //! \param    [in] useSingleDefaultQuantTable
    //!           if use single default quant talbe
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddQuantTable(PMOS_COMMAND_BUFFER cmdBuffer, bool useSingleDefaultQuantTable) const;

    //!
    //! \brief    Add Header for frame
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //! \param    [in] useSingleDefaultQuantTable
    //!           if use single default quant talbe
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddFrameHeader(PMOS_COMMAND_BUFFER cmdBuffer, bool useSingleDefaultQuantTable) const;

    //!
    //! \brief    Add Huffman Table
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //! \param    [in] tblInd
    //!           index of Huffman table
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHuffmanTable(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t tblInd) const;

    //!
    //! \brief    Add Restart interval
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddRestartInterval(PMOS_COMMAND_BUFFER cmdBuffer) const;

    //!
    //! \brief    Add Scan header
    //!
    //! \param    [out] cmdBuffer
    //!           Command Buffer for submit
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddScanHeader(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS InitMissedQuantTables();

    MOS_STATUS InitQuantMatrix();

    //!
    //! \brief  set MFC_JPEG_HUFF_TABLE - Convert encoded huffman table to actual table for HW
    //!         We need a different params struct for JPEG Encode Huffman table because JPEG decode huffman table has Bits and codes,
    //!         whereas JPEG encode huffman table has huffman code lengths and values
    //!
    //! \return MOS_STATUS
    //!
    MOS_STATUS InitHuffTable();

    MOS_STATUS AddAllCmds_MFX_FQM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFC_JPEG_HUFF_TABLE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFX_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MHW_SETPAR_DECL_HDR(MI_STORE_REGISTER_MEM);

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the output resources in status report callback function
    //!
    //! \param    [in] encodeStatusMfx
    //!           Pointer to encoder status for vdbox
    //! \param    [in] statusReportData
    //!           Pointer to encoder status report data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpResources(
        EncodeStatusMfx *       encodeStatusMfx,
        EncodeStatusReportData *statusReportData);

    MOS_STATUS DumpHuffmanTable(
        CodecEncodeJpegHuffmanDataArray *huffmanTable);

    MOS_STATUS DumpPicParams(
        CodecEncodeJpegPictureParams *picParams);

    MOS_STATUS DumpScanParams(
        CodecEncodeJpegScanHeader *scanParams);

    MOS_STATUS DumpQuantTables(
        CodecEncodeJpegQuantTable *quantTable);

#endif

    JpegPipeline             *m_pipeline       = nullptr;

    // Interfaces
    CodechalHwInterfaceNext     *m_hwInterface    = nullptr;
    JpegBasicFeature        *m_basicFeature   = nullptr;
    JpegPackerFeature       *m_jpgPkrFeature  = nullptr;
    MediaFeatureManager     *m_featureManager = nullptr;
    EncodeMemComp           *m_mmcState       = nullptr;
    EncodeCp                *m_encodecp       = nullptr;
    CodechalDebugInterface  *m_debugInterface = nullptr;

    std::shared_ptr<mhw::vdbox::mfx::Itf>   m_mfxItf   = nullptr;

    // Parameters passed from application
    CodecEncodeJpegPictureParams        *m_jpegPicParams    = nullptr;  //!< Pointer to picture parameter
    CodecEncodeJpegScanHeader           *m_jpegScanParams   = nullptr;  //!< Pointer to slice parameter
    CodecEncodeJpegQuantTable           *m_jpegQuantTables  = nullptr;
    CodecEncodeJpegHuffmanDataArray     *m_jpegHuffmanTable = nullptr;  //!< Pointer to IQWidght ScaleLists
    void                                *m_applicationData  = nullptr;  //!< Pointer to Application data

    // Patch List
    bool m_usePatchList                = 0;  //!< Use Ptach List or not

    // CMD buffer sizes
    uint32_t m_pictureStatesSize       = 0;  //!< Picture states size
    uint32_t m_picturePatchListSize    = 0;  //!< Picture patch list size
    uint32_t m_sliceStatesSize         = 0;  //!< Slice states size
    uint32_t m_slicePatchListSize      = 0;  //!< Slice patch list size

    uint32_t m_numHuffBuffers          = 0;
    uint32_t m_numQuantTables          = 0;

    bool     m_repeatHuffTable         = false;

    PMOS_RESOURCE m_pResource                      = nullptr;
    uint32_t      m_dwOffset                       = 0;
    uint32_t      m_dwValue                        = 0;

    CodecJpegQuantMatrix      m_jpegQuantMatrix                                = {};
    EncodeJpegHuffTableParams m_huffTableParams[JPEG_MAX_NUM_HUFF_TABLE_INDEX] = {};

    MHW_VDBOX_NODE_IND m_vdboxIndex    = MHW_VDBOX_NODE_1;  //!< Index of VDBOX

MEDIA_CLASS_DEFINE_END(encode__JpegPkt)
};

}  // namespace encode

#endif  // !__ENCODE_JPEG_PACKET_H__
