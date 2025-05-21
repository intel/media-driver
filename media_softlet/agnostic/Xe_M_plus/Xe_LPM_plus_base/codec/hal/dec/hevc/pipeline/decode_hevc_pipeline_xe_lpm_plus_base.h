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
//! \file     decode_hevc_pipeline_xe_lpm_plus_base.h
//! \brief    Defines the interface for hevc decode pipeline
//!
#ifndef __DECODE_HEVC_PIPELINE_XE_LPM_PLUS_BASE_H__
#define __DECODE_HEVC_PIPELINE_XE_LPM_PLUS_BASE_H__

#include "decode_hevc_pipeline.h"
#include "codec_def_decode_hevc.h"
#include "decode_hevc_packet_long_xe_lpm_plus_base.h"
#include "decode_huc_packet_creator.h"

namespace decode {

class HevcPipelineXe_Lpm_Plus_Base : public HevcPipeline, public HucPacketCreator
{
public:
    //!
    //! \brief  DecodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    HevcPipelineXe_Lpm_Plus_Base(CodechalHwInterfaceNext *hwInterface, CodechalDebugInterface *debugInterface);

    virtual ~HevcPipelineXe_Lpm_Plus_Base() {};

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

    virtual MOS_STATUS Destroy() override;

    uint32_t GetCompletedReport();

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
    //! \brief  Initialize scalability option
    //! \param  [in] basicFeature
    //!         Hevc decode basic feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitScalabOption(HevcBasicFeature &basicFeature);

    //!
    //! \brief  Allocate resource for Hevc decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources(HevcBasicFeature &basicFeature);

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief    Dump the parameters
    //! \param  [in] basicFeature
    //!         Reference to HevcBasicFeature
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpParams(HevcBasicFeature &basicFeature);
#endif

    //!
    //! \brief  Get the number of Vdbox
    //! \return uint8_t
    //!         Return the number of Vdbox
    //!
    virtual uint8_t GetSystemVdboxNumber() override;

private:
    HevcDecodeLongPktXe_Lpm_Plus_Base *m_hevcDecodePktLong = nullptr;

MEDIA_CLASS_DEFINE_END(decode__HevcPipelineXe_Lpm_Plus_Base)
};

}
#endif // !__DECODE_HEVC_PIPELINE_XE_LPM_PLUS_BASE_H__
