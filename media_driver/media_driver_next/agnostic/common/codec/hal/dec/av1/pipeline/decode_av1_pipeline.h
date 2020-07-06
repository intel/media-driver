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
//! \file     decode_av1_pipeline.h
//! \brief    Defines the interface for av1 decode pipeline
//!
#ifndef __DECODE_AV1_PIPELINE_H__
#define __DECODE_AV1_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_av1_basic_feature.h"

namespace decode {

class Av1Pipeline : public DecodePipeline
{
public:
    enum Av1DecodeMode
    {
        baseDecodeMode,         //!< Legacy decode mode with single pipe
        realTileDecodeMode,     //!< Real tile decode mode
    };

    //!
    //! \brief  Av1Pipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    Av1Pipeline(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~Av1Pipeline() {};

    Av1DecodeMode GetDecodeMode();

    bool FrameBasedDecodingInUse();

    DeclareDecodePacketId(av1DecodePacketId);
    DeclareDecodePacketId(av1PictureSubPacketId);
    DeclareDecodePacketId(av1TileSubPacketId);
    DeclareDecodePacketId(defaultCdfBufCopyPacketId);

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

#if 0
#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the quantization matrix parameters
    //!
    //! \param    [in] matrixData
    //!           Pointer to CODECHAL_HEVC_IQ_MATRIX_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    //MOS_STATUS DumpIQParams(PCODECHAL_HEVC_IQ_MATRIX_PARAMS matrixData);
#endif
#endif
protected:
    HucCopyPkt    *m_cdfCopyPkt       = nullptr;          //!< Update default cdf buffer with huc stream out packet
    Av1DecodeMode  m_decodeMode       = baseDecodeMode;   //!< Decode mode
    uint8_t        m_passNum          = 1;                //!< Decode pass number
    bool           m_isFirstTileInFrm = true;             //!< First tile in the first frame
};

}
#endif // !__DECODE_AV1_PIPELINE_H__
