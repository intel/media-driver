/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_pipeline_g12.h
//! \brief    Defines the interface for av1 decode pipeline
//!
#ifndef __DECODE_AV1_PIPELINE_G12_H__
#define __DECODE_AV1_PIPELINE_G12_H__

#include "decode_av1_pipeline_g12_base.h"
#include "codec_def_decode_av1.h"
#include "decode_filmgrain_surf_init_g12.h"
#include "decode_filmgrain_presubpipeline_g12.h"
#include "decode_filmgrain_postsubpipeline_g12.h"
#include "decode_huc_packet_creator_g12.h"

namespace decode
{
    class FilmGrainSurfaceInit;
    class FilmGrainPreSubPipeline;
    class FilmGrainPostSubPipeline;
    class Av1DecodePktG12;
    class Av1PipelineG12 : public Av1PipelineG12_Base, public HucPacketCreatorG12
    {
    public:
        //!
        //! \brief  DecodePipeline constructor
        //! \param  [in] hwInterface
        //!         Pointer to CodechalHwInterface
        //! \param  [in] debugInterface
        //!         Pointer to CodechalDebugInterface
        //!
        Av1PipelineG12(
            CodechalHwInterface *   hwInterface,
            CodechalDebugInterface *debugInterface);

        virtual ~Av1PipelineG12() {};

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

        DeclareDecodePacketId(av1FilmGrainGrvPacketId);  //!< declare packet ID for film grain getRandomValues kernel
        DeclareDecodePacketId(av1FilmGrainRp1PacketId);  //!< declare packet ID for film grain RegressPhase1 kernel
        DeclareDecodePacketId(av1FilmGrainRp2PacketId);  //!< declare packet ID for film grain RegressPhase2 kernel
        DeclareDecodePacketId(av1FilmGrainAppPacketId);  //!< declare packet ID for film grain ApplyNoise kernel

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
        //!         Codechal settings
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

        //!
        //! \brief  Create AV1 Decode feature manager for Gen12
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CreateFeatureManager() override;

        FilmGrainSurfaceInit        *m_fgCoordValSurfInitPipeline = nullptr;
        FilmGrainPreSubPipeline     *m_fgGenNoiseSubPipeline = nullptr;    //!< Film Grain Generate Noise sub pipeline, used as pre-subpipeline before HW decoding
        FilmGrainPostSubPipeline    *m_fgAppNoiseSubPipeline = nullptr;    //!< Film Grain Apply Noise sub pipeline, used as post-subpipeline after HW decoding
        bool                         m_allowVirtualNodeReassign = false;   //!< Whether allow virtual node reassign

    private:
        Av1DecodePktG12 *m_av1DecodePkt = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__Av1PipelineG12)
    };
}
#endif // !__DECODE_AV1_PIPELINE_G12_H__
