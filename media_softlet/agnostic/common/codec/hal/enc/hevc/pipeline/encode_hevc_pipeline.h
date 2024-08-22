/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     encode_hevc_pipeline.h
//! \brief    Defines the interface for hevc encode pipeline
//!
#ifndef __ENCODE_HEVC_PIPELINE_H__
#define __ENCODE_HEVC_PIPELINE_H__
#include "encode_pipeline.h"
#include "encode_hevc_dfs.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode {

class HevcPipeline : public EncodePipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    HevcPipeline(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~HevcPipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;

    enum PacketIds
    {
        HucBrcInit = CONSTRUCTPACKETID(PACKET_COMPONENT_ENCODE, PACKET_SUBCOMPONENT_HEVC, 0),
        HucBrcUpdate,
        hevcVdencPacket,
        hevcPakIntegrate,
        hevcVdencPicPacket,
        hevcVdencTileRowPacket,
        HucBrcTileRowUpdate,
        HucLaInit,
        HucLaUpdate,
        hevcVdencPacket422,
        EncodeCheckHucLoad
#if ((_DEBUG || _RELEASE_INTERNAL) && _MEDIA_RESERVED)
        ,
        hevcVdencMvdumpPacket
#endif
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
    //!           Pointer to CODEC_HEVC_ENCODE_SEQUENCE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSeqParams(
        const CODEC_HEVC_ENCODE_SEQUENCE_PARAMS *seqParams);

    //! \brief    Dump the picture parameters
    //!
    //! \param    [in] picParams
    //!           Pointer to CODEC_HEVC_ENCODE_PICTURE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPicParams(
        const CODEC_HEVC_ENCODE_PICTURE_PARAMS *picParams);

    //! \brief    Dump the slice parameters
    //!
    //! \param    [in] sliceParams
    //!           Pointer to CODEC_HEVC_ENCODE_SLICE_PARAMS
    //! \param    [in] picParams
    //!           Pointer to CODEC_HEVC_ENCODE_PICTURE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSliceParams(
        const CODEC_HEVC_ENCODE_SLICE_PARAMS   *sliceParams,
        const CODEC_HEVC_ENCODE_PICTURE_PARAMS *picParams);
#endif


MEDIA_CLASS_DEFINE_END(encode__HevcPipeline)
};

}
#endif // !__ENCODE_HEVC_PIPELINE_H__
