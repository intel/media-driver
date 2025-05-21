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
//! \file     encode_vp9_vdenc_pipeline_xe2_lpm_base.h
//! \brief    Defines the interface for vp9 vdenc encode pipeline
//!
#ifndef __ENCODE_VP9_VDENC_PIPELINE_XE2_LPM_BASE_H__
#define __ENCODE_VP9_VDENC_PIPELINE_XE2_LPM_BASE_H__

#include "encode_vp9_vdenc_pipeline.h"

namespace encode {

class Vp9VdencPipelineXe2_Lpm_Base : public Vp9VdencPipeline
{
public:
    //!
    //! \brief  Vp9VdencPipeline_Xe2_Lpm_Base constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp9VdencPipelineXe2_Lpm_Base(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    //!
    //! \brief  Vp9VdencPipeline_Xe2_Lpm_Base destructor
    //!
    virtual ~Vp9VdencPipelineXe2_Lpm_Base() {}

    //!
    //! \brief  Initialize the VP9 VDENC pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Prepare internal parameters, should be invoked for each frame
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
    virtual MOS_STATUS Execute() override;

    //!
    //! \brief  Get VP9 VDENC pipeline execution status
    //! \param  [out] status
    //!         The point to encode status
    //! \param  [in] numStatus
    //!         The requested number of status reports
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

    //!
    //! \brief  Destory the VP9 VDENC pipeline and release internal resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Initial MMC state
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMmcState();

protected:
    //!
    //! \brief  Initialize the VP9 VDENC pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings) override;

    //!
    //! \brief  Uninitialize the VP9 VDENC pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  Reset parameters after execute active packets
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ResetParams();

    //!
    //! \brief  Get system supported Vdbox number
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetSystemVdboxNumber() override;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

MEDIA_CLASS_DEFINE_END(encode__Vp9VdencPipelineXe2_Lpm_Base)
};

}
#endif // !__ENCODE_VP9_VDENC_PIPELINE_XE2_LPM_BASE_H__