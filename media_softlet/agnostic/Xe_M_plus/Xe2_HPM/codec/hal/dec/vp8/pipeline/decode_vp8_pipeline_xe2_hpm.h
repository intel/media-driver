/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     decode_vp8_pipeline_xe2_hpm.h
//! \brief    Defines the interface for vp8 decode pipeline
//!
#ifndef __DECODE_VP8_PIPELINE_XE2_HPM_H__
#define __DECODE_VP8_PIPELINE_XE2_HPM_H__

#include "decode_vp8_pipeline.h"
#include "decode_vp8_packet_xe_lpm_plus_base.h"
#include "codec_def_decode_vp8.h"
#include "decode_huc_packet_creator.h"

namespace decode
{

class Vp8PipelineXe2_Hpm : public Vp8Pipeline, public HucPacketCreator
{
public:
    //!
    //! \brief  DecodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp8PipelineXe2_Hpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Vp8PipelineXe2_Hpm(){};

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
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings) override;

#ifdef _MMC_SUPPORTED
    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();
#endif

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpParams(Vp8BasicFeature &basicFeature);
#endif

private:
    Vp8DecodePktXe_Lpm_Plus_Base *vp8DecodePkt = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Vp8PipelineXe2_Hpm)
};
}  // namespace decode
#endif  // !__DECODE_VP8_PIPELINE_XE2_HPM_H__
