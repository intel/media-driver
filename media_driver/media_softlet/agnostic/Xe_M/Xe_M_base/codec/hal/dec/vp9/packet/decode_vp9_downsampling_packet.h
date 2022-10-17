/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp9_downsampling_packet.h
//! \brief    Defines the common interface for Hevc deocde down sampling sub packet
//! \details  The Vp9 decode down sampling sub packet interface is unified interface
//!           for down sampling function, used by main packet on demand.
//!

#ifndef __DECODE_VP9_DOWNSAMPLING_PACKET_H__
#define __DECODE_VP9_DOWNSAMPLING_PACKET_H__

#include "decode_downsampling_packet.h"
#include "decode_vp9_pipeline.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{

class Vp9DownSamplingPkt : public DecodeDownSamplingPkt
{
public:
    //!
    //! \brief  Decode down sampling sub packet constructor
    //!
    Vp9DownSamplingPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface);

    virtual ~Vp9DownSamplingPkt() {}

    //!
    //! \brief  Initialize the Vp9 down sampling sub packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

protected:
    virtual MOS_STATUS InitSfcParams(VDBOX_SFC_PARAMS &sfcParams) override;
    virtual MOS_STATUS InitSfcScalabParams(SCALABILITY_PARAMS &scalabilityParams);
    virtual MOS_STATUS SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode) override;

     //!
    //! \brief  Initialize SFC scalability source params for current pipe
    //! \param  [in] vp9Pipeline
    //!         Vp9 pipeline
    //! \param  [in] vp9BasicFeature
    //!         Vp9 basic feature
    //! \param  [out] scalabilityParams
    //!         Scalability params
    //! \param  [out] tileColIndex
    //!         Tile coloumn index
    //! \param  [out] tileColCount
    //!         Tile coloumn count
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitSfcScalabSrcParams(
        Vp9Pipeline &vp9Pipeline, Vp9BasicFeature &vp9BasicFeature,
        SCALABILITY_PARAMS &scalabilityParams, uint32_t &tileColIndex, uint32_t &tileColCount);
    //!
    //! \brief  Initialize SFC scalability destination params for current pipe
    //! \param  [in] vp9Pipeline
    //!         Vp9 pipeline
    //! \param  [in] vp9BasicFeature
    //!         Vp9 basic feature
    //! \param  [out] scalabilityParams
    //!         Scalability parames
    //! \param  [in] tileColIndex
    //!         Tile coloumn index
    //! \param  [in] tileColCount
    //!         Tile coloumn count
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitSfcScalabDstParams(
        Vp9Pipeline &vp9Pipeline, Vp9BasicFeature &vp9BasicFeature,
        SCALABILITY_PARAMS &scalabilityParams, const uint32_t &tileColIndex, const uint32_t &tileColCount);

    static constexpr uint32_t m_ildbXOffset      = 8;
    static constexpr uint32_t m_tileOffsetXBasic = 11;
    static constexpr uint32_t m_tileOffsetX444   = 3;
    static constexpr uint32_t m_betaPrecision    = 5;
    static constexpr uint32_t m_oneBySfFractionPrecision = 19;

    uint32_t       m_firstValidTileIndex = 0;
    uint32_t       m_lastValidTileIndex  = 0;
    uint32_t       m_dstXLandingCount    = 0;
    uint32_t       m_lastDstEndX         = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t m_dbgOvrdWidthInMinCb = 0;
#endif

MEDIA_CLASS_DEFINE_END(decode__Vp9DownSamplingPkt)
};

}

#endif
#endif
