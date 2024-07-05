/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_vdenc_pipeline.h
//! \brief    Defines the interface for av1 vdenc encode pipeline
//!
#ifndef __ENCODE_AV1_VDENC_PIPELINE_H__
#define __ENCODE_AV1_VDENC_PIPELINE_H__

#include "encode_av1_pipeline.h"
#include "encode_scalability_defs.h"

namespace encode {
class Av1VdencPipeline : public encode::Av1Pipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //! \param  [in] standardInfo
    //!         Pointer to PCODECHAL_STANDARD_INFO
    //!
    Av1VdencPipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Av1VdencPipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;
    virtual MOS_STATUS Execute() override;
    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;
    virtual MOS_STATUS Destroy() override;

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS UserFeatureReport() override;
    virtual MOS_STATUS ActivateVdencVideoPackets();
    virtual MOS_STATUS CreateFeatureManager() override;
    virtual MOS_STATUS SwitchContext(uint8_t outputChromaFormat, uint16_t numTileRows, uint16_t numTileColumns);
    virtual MOS_STATUS InitMmcState() = 0;
    virtual MOS_STATUS FillStatusReportParameters(EncoderStatusParameters* pPar, EncoderParams* pEncPar);
    virtual MOS_STATUS HuCCheckAndInit();
    virtual MOS_STATUS ResetParams();

    bool m_preEncEnabled = false;

MEDIA_CLASS_DEFINE_END(encode__Av1VdencPipeline)
};

}
#endif // !__ENCODE_AV1_VDENC_PIPELINE_H__
