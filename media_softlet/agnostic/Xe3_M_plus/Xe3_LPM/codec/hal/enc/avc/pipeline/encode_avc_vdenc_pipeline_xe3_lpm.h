/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline_xe3_lpm.h
//! \brief    Defines the interface for avc vdenc encode pipeline Xe3_LPM
//!
#ifndef __ENCODE_AVC_VDENC_PIPELINE_XE3_LPM_H__
#define __ENCODE_AVC_VDENC_PIPELINE_XE3_LPM_H__

#include "encode_avc_vdenc_pipeline.h"

namespace encode {

class AvcVdencPipelineXe3_Lpm : public AvcVdencPipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    AvcVdencPipelineXe3_Lpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface) : AvcVdencPipeline(hwInterface, debugInterface) {}

    virtual ~AvcVdencPipelineXe3_Lpm() {}

    virtual MOS_STATUS Init(void *settings) override;

    MOS_STATUS InitMmcState();

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS CreateFeatureManager() override;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencPipelineXe3_Lpm)
};

}
#endif // !__ENCODE_AVC_VDENC_PIPELINE_XE3_LPM_H__
