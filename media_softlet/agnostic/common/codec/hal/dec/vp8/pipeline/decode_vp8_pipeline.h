/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_vp8_pipeline.h
//! \brief    Defines the interface for vp8 decode pipeline
//!
#ifndef __DECODE_VP8_PIPELINE_H__
#define __DECODE_VP8_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_vp8_basic_feature.h"
#include "decode_phase.h"

namespace decode {

class Vp8Pipeline : public DecodePipeline
{
public:
    enum Vp8DecodeMode
    {
        baseDecodeMode,         //!< Legacy decode mode with single pipe
    };

    //!
    //! \brief  Vp8Pipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp8Pipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Vp8Pipeline() {};
    Vp8DecodeMode GetDecodeMode();
    //!
    //! \brief  Declare Regkeys in the scope of vp9 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

    DeclareDecodePacketId(vp8PictureSubPacketId);
    DeclareDecodePacketId(vp8SliceSubPacketId);
    DeclareDecodePacketId(vp8DecodePacketId);

    Vp8BasicFeature *m_basicFeature     = nullptr;
    Vp8DecodeMode    m_decodeMode       = baseDecodeMode;   //!< Decode mode

    std::vector<DecodePhase *> m_phaseList;                //!< Phase list

protected:
    //!
    //! \brief  Initialize the decode pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings) override;

    //!
    //! \brief  Uninitialize the decode pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) override;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  Finish the execution for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute() override;

    //!
    //! \brief  create media feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager() override;

    //!
    //! \brief  Create sub packets
    //! \param  [in] codecSettings
    //!         Point to codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;


#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the picture parameters into file
    //!
    //! \param    [in] picParams
    //!           Pointer to PCODEC_VP8_PIC_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPicParams(PCODEC_VP8_PIC_PARAMS picParams);

    //! \brief    Dump the slice parameters into file
    //!
    //! \param    [in] sliceParams
    //!           Pointer to CODEC_VP8_SLICE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSliceParams(CODEC_VP8_SLICE_PARAMS *sliceParams);

    //! \brief    Dump Inverse Quantization Matrix Buffer into file
    //!
    //! \param    [in] matrixData
    //!           Pointer to CODEC_VP8_IQ_MATRIX_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpIQParams(CODEC_VP8_IQ_MATRIX_PARAMS *matrixData);

    //! \brief    Dump CoefProbBuffer into file
    //!
    //! \param    [in] m_resCoefProbBuffer
    //!           Pointer to m_resCoefProbBuffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpCoefProbBuffer(PMOS_RESOURCE m_resCoefProbBuffer);
#endif

MEDIA_CLASS_DEFINE_END(decode__Vp8Pipeline)
};

}
#endif // !__DECODE_VP8_PIPELINE_H__
