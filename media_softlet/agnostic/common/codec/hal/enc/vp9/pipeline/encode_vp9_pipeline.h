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
//! \file     encode_vp9_pipeline.h
//! \brief    Defines the interface for vp9 encode pipeline
//!
#ifndef __ENCODE_VP9_PIPELINE_H__
#define __ENCODE_VP9_PIPELINE_H__
#include "encode_pipeline.h"
#include "encode_vp9_basic_feature.h"
#include "media_vp9_feature_defs.h"

namespace encode {

class Vp9Pipeline : public EncodePipeline
{
public:
    //!
    //! \brief  Vp9Pipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp9Pipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    //!
    //! \brief  Vp9Pipeline destructor 
    //!
    virtual ~Vp9Pipeline() {}

    //!
    //! \brief  Prepare internal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) override;

protected:
    //!
    //! \brief  Initialize the VP9 encode pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings) override;

    //!
    //! \brief  Uninitialize the VP9 encode pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  Report User Settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReportUserSettingValue(const std::string &valueName,
        const MediaUserSetting::Value &                  value,
        const MediaUserSetting::Group &                  group);

    //!
    //! \brief  Declare User Settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DeclareUserSettingKeyValue(const std::string &valueName,
        const MediaUserSetting::Group &                      group,
        const MediaUserSetting::Value &                      defaultValue,
        bool                                                 isReportKey);

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  Create buffer tracker, the derived class can overload it if
    //!         requires different buffer count
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateBufferTracker() override;

    //!
    //! \brief  Create status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateStatusReport() override;

    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the segment parameters
    //!
    //! \brief    [in] segmentParams
    //!           Pointer to CODEC_VP9_ENCODE_SEGMENT_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS DumpSegmentParams(
        const CODEC_VP9_ENCODE_SEGMENT_PARAMS *segmentParams);

    //! \brief    Dump the sequence parameters
    //!
    //! \brief    [in] seqParams
    //!           Pointer to CODEC_VP9_ENCODE_SEQUENCE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS DumpSeqParams(
        const CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams);

    //! \brief    Dump the picture parameters
    //!
    //! \brief    [in] picParams
    //!           Pointer to CODEC_VP9_ENCODE_PIC_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS DumpPicParams(
        const CODEC_VP9_ENCODE_PIC_PARAMS *picParams);
#endif

    enum PacketIds
    {
        HucBrcInit = CONSTRUCTPACKETID(PACKET_COMPONENT_ENCODE, PACKET_SUBCOMPONENT_VP9, 0),
        HucBrcUpdate,
        Vp9VdencPacket,
        Vp9PakIntegrate,
        Vp9HucProb,
        Vp9HucSuperFrame,
        Vp9DynamicScal
    };

MEDIA_CLASS_DEFINE_END(encode__Vp9Pipeline)
};

}
#endif // !__ENCODE_VP9_PIPELINE_H__
