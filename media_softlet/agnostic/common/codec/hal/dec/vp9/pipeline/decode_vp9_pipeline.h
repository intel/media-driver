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
//! \file     decode_vp9_pipeline.h
//! \brief    Defines the interface for vp9 decode pipeline
//!
#ifndef __DECODE_VP9_PIPELINE_H__
#define __DECODE_VP9_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_vp9_basic_feature.h"
#include "decode_scalability_option.h"
#include "decode_phase.h"

namespace decode {

class Vp9Pipeline : public DecodePipeline
{
public:
    enum Vp9DecodeMode
    {
        baseDecodeMode,         //!< Legacy decode mode with single pipe
        virtualTileDecodeMode,  //!< virtual tile decode mode
    };

    //!
    //! \brief  Vp9Pipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Vp9Pipeline(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Vp9Pipeline() {};

    Vp9DecodeMode GetDecodeMode();
    //!
    //! \brief  Declare Regkeys in the scope of vp9 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

    DeclareDecodePacketId(vp9SinglePacketId);
    DeclareDecodePacketId(vp9FrontEndPacketId);
    DeclareDecodePacketId(vp9BackEndPacketId);
    DeclareDecodePacketId(vp9PictureSubPacketId);
    DeclareDecodePacketId(vp9SliceSubPacketId);
    DeclareDecodePacketId(vp9TileSubPacketId);

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

    //!
    //! \brief  Initialize context option
    //! \param  [in] basicFeature
    //!         VP9 decode basic feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitContexOption(Vp9BasicFeature &basicFeature);

    //!
    //! \brief  Initialize VP9 decode mode
    //! \param  [in] scalabMode
    //!         Decode scalability mode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitDecodeMode(ScalabilityMode scalabMode);

    //!
    //! \brief   Add one phase with pass number and pipe number
    //! \param  [in] pass
    //!         Pass number for phase
    //! \param  [in] pipe
    //!         Pipe number for phase
    //! \param  [in] activePipeNum
    //!         Acutive pipe number for current pass
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template<typename T>
    MOS_STATUS CreatePhase(uint8_t pass = 0, uint8_t pipe = 0, uint8_t activePipeNum = 1);

    //!
    //! \brief  Create VP9 decode phase list for current frame
    //! \param  [in] scalabMode
    //!         Decode scalability mode
    //! \param  [in] numPipe
    //!         Number of pipe for currently scalability mode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreatePhaseList(const ScalabilityMode scalabMode, const uint8_t numPipe);

    //!
    //! \brief  Destroy Vp9 decode phase list
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestoryPhaseList();

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the picture parameters
    //!
    //! \param    [in] picParams
    //!           Pointer to CodecAv1PicParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPicParams(CODEC_VP9_PIC_PARAMS *picParams);

    //! \brief    Dump segment parameters into file
    //!
    //! \param    [in] segmentParams
    //!           Pointer to PCODEC_VP9_SEGMENT_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSegmentParams(CODEC_VP9_SEGMENT_PARAMS *segmentParams);

    //! \brief    Dump slice parameters into file
    //!
    //! \param    [in] slcParams
    //!           Pointer to PCODEC_VP9_SLICE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSliceParams(CODEC_VP9_SLICE_PARAMS *slcParams);
#endif

protected:
    Vp9BasicFeature *m_basicFeature    = nullptr;
    Vp9DecodeMode    m_decodeMode      = baseDecodeMode;   //!< Decode mode

    DecodeScalabilityOption    m_scalabOption;             //!< VP9 decode scalability option

    std::vector<DecodePhase *> m_phaseList;                //!< Phase list

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t m_vtFrameCount = 0; //!< frame count for virtual tile decoding
#endif

MEDIA_CLASS_DEFINE_END(decode__Vp9Pipeline)
};

}
#endif // !__DECODE_VP9_PIPELINE_H__
