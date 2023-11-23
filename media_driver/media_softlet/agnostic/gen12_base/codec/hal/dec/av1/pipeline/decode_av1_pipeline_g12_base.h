/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_pipeline_g12_base.h
//! \brief    Defines the interface for av1 decode pipeline
//!
#ifndef __DECODE_AV1_PIPELINE_G12_BASE_H__
#define __DECODE_AV1_PIPELINE_G12_BASE_H__

#include "decode_pipeline.h"
#include "decode_av1_basic_feature_g12.h"

namespace decode {

class Av1PipelineG12_Base : public DecodePipeline
{
public:
    enum Av1DecodeMode
    {
        baseDecodeMode,         //!< Legacy decode mode with single pipe
        realTileDecodeMode,     //!< Real tile decode mode
    };

    //!
    //! \brief  Av1PipelineG12_Base constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Av1PipelineG12_Base(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Av1PipelineG12_Base() {};

    Av1DecodeMode GetDecodeMode();

    bool    FrameBasedDecodingInUse();

    bool    TileBasedDecodingInuse() {return m_forceTileBasedDecoding;}

    DeclareDecodePacketId(av1DecodePacketId);
    DeclareDecodePacketId(av1PictureSubPacketId);
    DeclareDecodePacketId(av1TileSubPacketId);

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
    //! \brief  Active decode packets
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ActivateDecodePackets();

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
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings) override;

    //!
    //! \brief  Create post sub packets
    //! \param  [in] subPipelineManager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager) override;

    //!
    //! \brief  Create pre sub packets
    //! \param  [in] subPipelineManager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager) override;

#if USE_CODECHAL_DEBUG_TOOL
        //! \brief    Dump the parameters
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpParams(Av1BasicFeatureG12 &basicFeature);

        //! \brief    Dump the picture parameters
        //!
        //! \param    [in] picParams
        //!           Pointer to CodecAv1PicParams
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpPicParams(CodecAv1PicParams *picParams);

        //! \brief    Dump Tile Group parameters into file
        //!
        //! \param    [in] tileParams
        //!           Pointer to CodecAv1TileParams
        //!
        //! \param    [in] tileNum
        //!           Number of tiles
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpTileParams(CodecAv1TileParams *tileParams, uint32_t tileNum);
#endif

protected:
    Av1DecodeMode  m_decodeMode       = baseDecodeMode;   //!< Decode mode
    uint16_t       m_passNum          = 1;                //!< Decode pass number
    bool           m_isFirstTileInFrm = true;             //!< First tile in the first frame
    bool           m_forceTileBasedDecoding = false;      //!< Force tile based decoding
    CodechalHwInterface *m_hwInterface            = nullptr;
MEDIA_CLASS_DEFINE_END(decode__Av1PipelineG12_Base)
};

}
#endif // !__DECODE_AV1_PIPELINE_G12_BASE_H__
