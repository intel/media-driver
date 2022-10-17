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
//! \file     decode_jpeg_pipeline.h
//! \brief    Defines the interface for jpeg decode pipeline
//!
#ifndef __DECODE_JPEG_PIPELINE_H__
#define __DECODE_JPEG_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_jpeg_basic_feature.h"

namespace decode {

class JpegPipeline : public DecodePipeline
{
public:

    //!
    //! \brief  JpegPipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    JpegPipeline(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface* debugInterface);

    virtual ~JpegPipeline() {};

    //!
    //! \brief  Declare Regkeys in the scope of jpeg decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

    DeclareDecodePacketId(jpegDecodePacketId);
    DeclareDecodePacketId(jpegPictureSubPacketId);

protected:
    //!
    //! \brief  Initialize the decode pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings) override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) override;

    //!
    //! \brief  Uninitialize the decode pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  Active decode packets
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ActivateDecodePackets();

    //!
    //! \brief  create media feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager() override;

    //!
    //! \brief  Create sub packets
    //! \param  [in] codecSettings
    //!         Point to codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;

    virtual MOS_STATUS CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager) override;

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        CodecDecodeJpegPicParams *picParams);

    MOS_STATUS DumpIQParams(
        CodecJpegQuantMatrix *matrixData);

    MOS_STATUS DumpScanParams(
        CodecDecodeJpegScanParameter *scanParams);

   MOS_STATUS DumpHuffmanTable(
        PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable);
#endif

protected:

    JpegBasicFeature *m_basicFeature = nullptr;  //!< Jpeg Basic Feature

MEDIA_CLASS_DEFINE_END(decode__JpegPipeline)
};

}
#endif // !__DECODE_JPEG_PIPELINE_H__
