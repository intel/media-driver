/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     decode_avc_pipeline.h
//! \brief    Defines the interface for avc decode pipeline
//!
#ifndef __DECODE_AVC_PIPELINE_H__
#define __DECODE_AVC_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_avc_basic_feature.h"

namespace decode {

class AvcPipeline : public DecodePipeline
{
public:
    enum AvcDecodeMode
    {
        baseDecodeMode,         //!< Legacy decode mode with single pipe
    };

    //!
    //! \brief  AvcPipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    AvcPipeline(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface* debugInterface);

    //!
    //! \brief  Return if short format decode in use
    //! \return bool
    //!         True if short format in use, else false
    //!
    bool IsShortFormat();

    bool m_intelEntrypointInUse = false; //!< Indicate it is Intel-specific Format

    virtual ~AvcPipeline() {};

    AvcDecodeMode GetDecodeMode();
    //!
    //! \brief  Declare Regkeys in the scope of avc decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;
    
    //!
    //! \brief    Set decode short/long format during runtime.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDecodeFormat(bool isShortFormat) override;

    DeclareDecodePacketId(avcDecodePacketId);
    DeclareDecodePacketId(avcPictureSubPacketId);
    DeclareDecodePacketId(avcSliceSubPacketId);
    DeclareDecodePacketId(avcFormatMonoPicPktId);

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
    //! \brief  Copy decode dest surface to downsampling input surface if
    //!         a explicit copy needed for non downsampling platforms
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HandleRefOnlySurfaces();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        PCODEC_AVC_PIC_PARAMS picParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_AVC_SLICE_PARAMS sliceParams,
        uint32_t                numSlices,
        bool                    shortFormatInUse);

    MOS_STATUS DumpIQParams(
        PCODEC_AVC_IQ_MATRIX_PARAMS iqParams);
#endif

protected:

    bool            m_shortFormatInUse  = false;             //!< Indicate it is Short Format
    AvcDecodeMode   m_decodeMode        = baseDecodeMode;    //!< Decode mode
    HucCopyPktItf   *m_formatMonoPicPkt  = nullptr;          //!< Format Avc Mono Chroma with HuC Copy
    AvcBasicFeature *m_basicFeature     = nullptr;           //!< Avc Basic Feature
    bool            m_allowVirtualNodeReassign = false;      //!< Whether allow virtual node reassign
    MEDIA_CLASS_DEFINE_END(decode__AvcPipeline)
};

}
#endif // !__DECODE_AVC_PIPELINE_H__
