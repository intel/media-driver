/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline.h
//! \brief    Defines the interface for avc vdenc encode pipeline
//!
#ifndef __ENCODE_AVC_VDENC_PIPELINE_H__
#define __ENCODE_AVC_VDENC_PIPELINE_H__

#include "encode_pipeline.h"

namespace encode {

class AvcVdencPipeline : public EncodePipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    AvcVdencPipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~AvcVdencPipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;

    virtual MOS_STATUS Execute() override;

    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

    virtual MOS_STATUS Destroy() override;

    enum PacketIds
    {
        HucBrcInit = CONSTRUCTPACKETID(PACKET_COMPONENT_ENCODE, PACKET_SUBCOMPONENT_AVC, 0),
        HucBrcUpdate,
        VdencPacket
    };

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS Uninitialize() override;
    virtual MOS_STATUS UserFeatureReport() override;
    virtual MOS_STATUS CreateBufferTracker() override;
    virtual MOS_STATUS CreateStatusReport() override;
    virtual MOS_STATUS SwitchContext(uint8_t outputChromaFormat);

    //!
    //! \brief  Activate necessary packets
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ActivateVdencVideoPackets();

    //!
    //! \brief  Reset parameters after execute active packets
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ResetParams();

    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the Sequense parameters
    //!
    //! \param    [in] seqParams
    //!           Pointer to CODEC_AVC_ENCODE_SEQUENCE_PARAMS
    //! \param    [in] matrixParams
    //!           Pointer to CODEC_AVC_IQ_MATRIX_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSeqParams(
        const CODEC_AVC_ENCODE_SEQUENCE_PARAMS *seqParams,
        const CODEC_AVC_IQ_MATRIX_PARAMS       *matrixParams);

    //! \brief    Dump the picture parameters
    //!
    //! \param    [in] picParams
    //!           Pointer to CODEC_AVC_ENCODE_PIC_PARAMS
    //! \param    [in] matrixParams
    //!           Pointer to CODEC_AVC_IQ_MATRIX_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPicParams(
        const CODEC_AVC_ENCODE_PIC_PARAMS *picParams,
        const CODEC_AVC_IQ_MATRIX_PARAMS  *matrixParams);

    //! \brief    Dump the slice parameters
    //!
    //! \param    [in] sliceParams
    //!           Pointer to CODEC_AVC_ENCODE_SLICE_PARAMS
    //! \param    [in] picParams
    //!           Pointer to CODEC_AVC_ENCODE_PIC_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSliceParams(
        const CODEC_AVC_ENCODE_SLICE_PARAMS *sliceParams,
        const CODEC_AVC_ENCODE_PIC_PARAMS   *picParams);

    //! \brief    Dump VUI parameters
    //!
    //! \param    [in] avcVuiParams
    //!           Pointer to CODECHAL_ENCODE_AVC_VUI_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpVuiParams(
        const CODECHAL_ENCODE_AVC_VUI_PARAMS *avcVuiParams);

    MOS_STATUS DumpEncodePicReorder(
        std::ostringstream       &oss,
        uint32_t                  x,
        uint32_t                  y,
        const CODEC_PIC_REORDER * picReorder);

    MOS_STATUS PopulateTargetUsage();

    MOS_STATUS PopulateQuantPrecision();
#endif

    bool m_isConstDumped = false;

    bool     m_preEncEnabled = false;
    uint32_t m_encodeMode    = 0;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencPipeline)
};

}
#endif // !__ENCODE_AVC_VDENC_PIPELINE_H__
