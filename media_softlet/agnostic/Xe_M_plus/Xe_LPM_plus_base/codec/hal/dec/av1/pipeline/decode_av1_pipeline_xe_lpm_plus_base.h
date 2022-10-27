/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_av1_pipeline_xe_lpm_plus_base.h
//! \brief    Defines the interface for av1 decode pipeline
//!
#ifndef __DECODE_AV1_PIPELINE_XE_LPM_PLUS_BASE_H__
#define __DECODE_AV1_PIPELINE_XE_LPM_PLUS_BASE_H__

#include "decode_av1_pipeline.h"
#include "codec_def_decode_av1.h"
#include "decode_huc_packet_creator.h"

namespace decode
{
    class Av1DecodePktXe_Lpm_Plus_Base;
class Av1PipelineXe_Lpm_Plus_Base : public Av1Pipeline, public HucPacketCreator
    {
    public:
        //!
        //! \brief  DecodePipeline constructor
        //! \param  [in] hwInterface
        //!         Pointer to CodechalHwInterface
        //! \param  [in] debugInterface
        //!         Pointer to CodechalDebugInterface
        //!
        Av1PipelineXe_Lpm_Plus_Base(
            CodechalHwInterfaceNext *   hwInterface,
            CodechalDebugInterface *debugInterface);

        virtual ~Av1PipelineXe_Lpm_Plus_Base() {};

        virtual MOS_STATUS Init(void *settings) override;

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \param  [in] params
        //!         Pointer to the input parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare(void *params) override;

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
        virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;

        //!
        //! \brief  Initialize media context for decode pipeline
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS InitContext();

        //!
        //! \brief    Initialize MMC state
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!
        virtual MOS_STATUS InitMmcState();

        //!
        //! \brief  Create status report
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS CreateStatusReport() override;

        //!
        //! \brief  Create AV1 Decode feature manager for Gen12
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CreateFeatureManager() override;

#if USE_CODECHAL_DEBUG_TOOL
        //!
        //! \brief  Dump render targets
        //! \param  [in] reportData
        //!         Decode report data
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS DumpOutput(const DecodeStatusReportData& reportData) override;
#endif
    private:
        Av1DecodePktXe_Lpm_Plus_Base *m_av1DecodePkt = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__Av1PipelineXe_Lpm_Plus_Base)
    };
}
#endif // !__DECODE_AV1_PIPELINE_XE_LPM_PLUS_BASE_H__
