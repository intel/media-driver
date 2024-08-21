/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_av1_pipeline_xe2_hpm.h
//! \brief    Defines the interface for av1 decode pipeline
//!
#ifndef __DECODE_AV1_PIPELINE_XE2_HPM_H__
#define __DECODE_AV1_PIPELINE_XE2_HPM_H__

#include "decode_av1_pipeline_xe_lpm_plus_base.h"

namespace decode
{
class Av1PipelineXe2_Hpm : public Av1PipelineXe_Lpm_Plus_Base
    {
    public:
        //!
        //! \brief  DecodePipeline constructor
        //! \param  [in] hwInterface
        //!         Pointer to CodechalHwInterface
        //! \param  [in] debugInterface
        //!         Pointer to CodechalDebugInterface
        //!
        Av1PipelineXe2_Hpm(
            CodechalHwInterfaceNext *   hwInterface,
            CodechalDebugInterface *debugInterface);

        virtual ~Av1PipelineXe2_Hpm(){};

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \param  [in] params
        //!         Pointer to the input parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare(void *params) final;

        //!
        //! \brief  Create sub packets
        //! \param  [in] codecSettings
        //!         Point to codechal settings
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;
        virtual MOS_STATUS CreateFeatureManager() override;

    protected:
        //!
        //! \brief    Initialize MMC state
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!
        virtual MOS_STATUS InitMmcState() override;

    MEDIA_CLASS_DEFINE_END(decode__Av1PipelineXe2_Hpm)
    };
}
#endif // !__DECODE_AV1_PIPELINE_XE2_HPM_H__
