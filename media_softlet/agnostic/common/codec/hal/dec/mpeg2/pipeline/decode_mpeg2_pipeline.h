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
//! \file     decode_mpeg2_pipeline.h
//! \brief    Defines the interface for mpeg2 decode pipeline
//!
#ifndef __DECODE_MPEG2_PIPELINE_H__
#define __DECODE_MPEG2_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_mpeg2_basic_feature.h"

namespace decode {

class Mpeg2Pipeline : public DecodePipeline
{
public:

    //!
    //! \brief  Mpeg2Pipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Mpeg2Pipeline(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface* debugInterface);

    virtual ~Mpeg2Pipeline() {};
    //!
    //! \brief  Declare Regkeys in the scope of mpeg2 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;
    DeclareDecodePacketId(mpeg2DecodePacketId);
    DeclareDecodePacketId(mpeg2PictureSubPacketId);
    DeclareDecodePacketId(mpeg2SliceSubPacketId);
    DeclareDecodePacketId(mpeg2MbSubPacketId);
    DeclareDecodePacketId(mpeg2BsCopyPktId);

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

    //!
    //! \brief    Copy bitstream to local buffer
    //! \details  Copy bitstream to local buffer in MPEG2 decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CopyBitstreamBuffer();

    //!
    //! \brief    Copy dummy slice to local buffer
    //! \details  Copy bitstream to local buffer in MPEG2 decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CopyDummyBitstream();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        CodecDecodeMpeg2PicParams *picParams);

    MOS_STATUS DumpSliceParams(
        CodecDecodeMpeg2SliceParams *sliceParams,
        uint32_t                     numSlices);

    MOS_STATUS DumpMbParams(
        CodecDecodeMpeg2MbParams *mbParams,
        uint32_t                  numMbs);

    MOS_STATUS DumpIQParams(
        CodecMpeg2IqMatrix *matrixData);
#endif

protected:

    Mpeg2BasicFeature *m_basicFeature = nullptr;  //!< Mpeg2 Basic Feature
    HucCopyPktItf     *m_mpeg2BsCopyPkt = nullptr;  //!< Mpeg2 bitstream with HuC Copy

MEDIA_CLASS_DEFINE_END(decode__Mpeg2Pipeline)
};

}
#endif // !__DECODE_MPEG2_PIPELINE_H__
