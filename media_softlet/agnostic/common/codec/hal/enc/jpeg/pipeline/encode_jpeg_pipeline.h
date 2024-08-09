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
//! \file     encode_jpeg_pipeline.h
//! \brief    Defines the interface for jpeg encode pipeline
//!
#ifndef __ENCODE_JPEG_PIPELINE_H__
#define __ENCODE_JPEG_PIPELINE_H__

#include "encode_pipeline.h"

namespace encode {

class JpegPipeline : public EncodePipeline
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
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~JpegPipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;

    virtual MOS_STATUS Execute() override;

    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS Init(void *settings) override;

    virtual MOS_STATUS InitMmcState();

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS Uninitialize() override;
    virtual MOS_STATUS UserFeatureReport() override;
    virtual MOS_STATUS CreateBufferTracker() override;
    virtual MOS_STATUS CreateStatusReport() override;
    virtual MOS_STATUS CreateFeatureManager() override;
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

    //!
    //! \brief  Activate necessary packets
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ActivateVideoPackets();

    //!
    //! \brief  Reset parameters after execute active packets
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ResetParams();

    enum PacketIds
    {
        baseJpegPacket  = CONSTRUCTPACKETID(PACKET_COMPONENT_ENCODE, PACKET_SUBCOMPONENT_JPEG, 0)
    };

MEDIA_CLASS_DEFINE_END(encode__JpegPipeline)
};

}
#endif // !__ENCODE_JPEG_PIPELINE_H__
