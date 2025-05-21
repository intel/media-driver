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
//! \file     decode_jpeg_pipeline_m12.h
//! \brief    Defines the interface for jpeg decode pipeline
//!
#ifndef __DECODE_JPEG_PIPELINE_M12_H__
#define __DECODE_JPEG_PIPELINE_M12_H__
#include "decode_jpeg_pipeline.h"
#include "decode_jpeg_packet_m12.h"
#include "decode_jpeg_downsampling_packet.h"
#include "decode_huc_packet_creator_g12.h"

namespace decode {

class JpegPipelineM12 : public JpegPipeline, public HucPacketCreatorG12
{
public:
    //!
    //! \brief  DecodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    JpegPipelineM12(
        CodechalHwInterface    *hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~JpegPipelineM12() {};

    virtual MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) final;

    //!
    //! \brief  Finish the execution for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute() final;

    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

    uint32_t GetCompletedReport();

    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Create post sub packets
    //! \param  [in] subPipelineManager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager) override;
    //!
    //! \brief  Create pre sub packets
    //! \param  [in] subPipelineManager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager) override;
    virtual MOS_STATUS CreateFeatureManager() override;

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  Create sub packets
    //! \param  [in] codecSettings
    //!         Point to codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

    //!
    //! \brief  Initialize media context for decode pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitContext();

#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief    Dump the parameters
    //! \param  [in] basicFeature
    //!         Reference to JpegBasicFeature
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpParams(JpegBasicFeature &basicFeature);
#endif

private:
    JpegDecodePktM12 *m_jpegDecodePkt = nullptr;
    CodechalHwInterface *m_hwInterface   = nullptr;
MEDIA_CLASS_DEFINE_END(decode__JpegPipelineM12)
};

}
#endif // !__DECODE_JPEG_PIPELINE_M12_H__
