/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_pipeline.h
//! \brief    Defines the interface for hevc vdenc encode pipeline
//!
#ifndef __ENCODE_HEVC_VDENC_PIPELINE_H__
#define __ENCODE_HEVC_VDENC_PIPELINE_H__

#include "encode_hevc_pipeline.h"
#include "encode_hevc_cqp.h"
#include "encode_scalability_defs.h"
namespace encode {
//!
//! \struct   CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER
//! \brief    Codechal encode HEVC PAK States buffer
//!
struct CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER
{
    uint32_t HCP_BITSTREAM_BYTECOUNT_FRAME;
    uint32_t HCP_BITSTREAM_BYTECOUNT_FRAME_NOHEADER;
    uint32_t HCP_IMAGE_STATUS_CONTROL;
    uint32_t Reserved0;
    uint32_t HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS;
    uint32_t Reserved1[3];
};

class HevcVdencPipeline : public encode::HevcPipeline
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
    HevcVdencPipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~HevcVdencPipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS Uninitialize() override;
    virtual MOS_STATUS UserFeatureReport() override;
    virtual MOS_STATUS ActivateVdencVideoPackets();
    virtual MOS_STATUS ActivateVdencTileReplayVideoPackets();
    virtual MOS_STATUS CreateFeatureManager() override;
    virtual MOS_STATUS SwitchContext(uint8_t outputChromaFormat, uint16_t numTileRows, uint16_t numTileColumns, bool enableTileReplay);
    virtual MOS_STATUS HuCCheckAndInit();

MEDIA_CLASS_DEFINE_END(encode__HevcVdencPipeline)
};

}
#endif // !__ENCODE_HEVC_VDENC_PIPELINE_H__
