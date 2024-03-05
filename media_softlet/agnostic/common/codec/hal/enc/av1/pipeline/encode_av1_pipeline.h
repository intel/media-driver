/*
* Copyright (c) 2019 - 2023, Intel Corporation
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
//! \file     encode_av1_pipeline.h
//! \brief    Defines the interface for av1 encode pipeline
//!
#ifndef __ENCODE_AV1_PIPELINE_H__
#define __ENCODE_AV1_PIPELINE_H__
#include "encode_pipeline.h"
#include "encode_av1_vdenc_feature_manager.h"

namespace encode {
class Av1Pipeline : public EncodePipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Av1Pipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Av1Pipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;

    virtual bool IsDualEncEnabled() {return m_dualEncEnable;}

    enum Av1PacketIds
    {
        Av1HucBrcInit = CONSTRUCTPACKETID(PACKET_COMPONENT_ENCODE, PACKET_SUBCOMPONENT_AV1, 0),
        Av1HucBrcUpdate,
        Av1VdencPacket,
        Av1PakIntegrate,
        Av1BackAnnotation,
        Av1Superres,
        EncodeCheckHucLoad,
    };

protected:
    virtual MOS_STATUS Initialize(void *settings) override;
    virtual MOS_STATUS Uninitialize() override;
    virtual MOS_STATUS UserFeatureReport() override;
    virtual MOS_STATUS CreateBufferTracker() override;
    virtual MOS_STATUS CreateStatusReport() override;
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;


#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the Sequense parameters
    //!
    //! \param    [in] seqParams
    //!           Pointer to CODEC_AV1_ENCODE_SEQUENCE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSeqParams(
        const CODEC_AV1_ENCODE_SEQUENCE_PARAMS *seqParams);

    //! \brief    Dump the picture parameters
    //!
    //! \param    [in] picParams
    //!           Pointer to CODEC_AV1_ENCODE_PICTURE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPicParams(
        const CODEC_AV1_ENCODE_PICTURE_PARAMS *picParams);

    //! \brief    Dump the tile group parameters
    //!
    //! \param    [in] tilegroupParams
    //!           Pointer to CODEC_AV1_ENCODE_TILE_GROUP_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpTileGroupParams(
        const CODEC_AV1_ENCODE_TILE_GROUP_PARAMS *tilegroupParams,
        uint32_t                                  index);
#endif

    bool m_dualEncEnable = false;

MEDIA_CLASS_DEFINE_END(encode__Av1Pipeline)
};

}
#endif // !__ENCODE_AV1_PIPELINE_H__
