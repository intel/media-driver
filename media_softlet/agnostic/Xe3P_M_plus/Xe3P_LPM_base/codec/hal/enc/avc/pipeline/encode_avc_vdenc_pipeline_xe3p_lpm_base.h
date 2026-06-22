/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline_xe3p_lpm_base.h
//! \brief    Defines the interface for avc vdenc encode pipeline Xe3P_LPM_Base
//!
#ifndef __ENCODE_AVC_VDENC_PIPELINE_XE3P_LPM_BASE_H__
#define __ENCODE_AVC_VDENC_PIPELINE_XE3P_LPM_BASE_H__

#include "encode_avc_vdenc_pipeline.h"
#if (_DEBUG || _RELEASE_INTERNAL)
#include "bypass_hw_legacy.h"
#endif

namespace encode
{

class AvcVdencPipelineXe3P_Lpm_Base : public AvcVdencPipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    AvcVdencPipelineXe3P_Lpm_Base(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface *debugInterface) : AvcVdencPipeline(hwInterface, debugInterface) {}

    virtual ~AvcVdencPipelineXe3P_Lpm_Base() {}

    virtual MOS_STATUS Init(void *settings) override;

    virtual MOS_STATUS InitMmcState();

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS Uninitialize() override;
    virtual MOS_STATUS ActivateVdencVideoPackets() override;
    virtual MOS_STATUS SwitchContext(uint8_t outputChromaFormat) override;

#if (_DEBUG || _RELEASE_INTERNAL)
public:
    BypassHwLegacy *GetBypassHW() const { return m_bypassHW; }
    MOS_GPU_NODE    GetGpuNode()  const { return m_gpuNode; }

protected:
    BypassHwLegacy *m_bypassHW = nullptr;
    MOS_GPU_NODE    m_gpuNode  = MOS_GPU_NODE_VIDEO;
#endif

MEDIA_CLASS_DEFINE_END(encode__AvcVdencPipelineXe3P_Lpm_Base)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_PIPELINE_XE3P_LPM_BASE_H__
