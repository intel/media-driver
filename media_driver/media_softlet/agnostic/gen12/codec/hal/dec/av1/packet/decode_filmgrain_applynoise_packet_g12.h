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
//! \file     decode_filmgrain_applynoise_packet_g12.h
//! \brief    Defines the implementation of av1 decode film grain apply noise kernel packet
//!

#ifndef __DECODE_FILMGRAIN_APPLYNOISE_PACKET_G12_H__
#define __DECODE_FILMGRAIN_APPLYNOISE_PACKET_G12_H__

#include "media_render_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_av1_filmgrain_feature_g12.h"

namespace decode
{

//Curbe definitions
struct FilmGrainApplyNoiseCurbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t InputYuvSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t OutputYSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t OutputUvSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW2;

    // uint32_t 3
    union
    {
        struct
        {
            uint32_t YDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW3;

    // uint32_t 4
    union
    {
        struct
        {
            uint32_t UDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW4;

    // uint32_t 5
    union
    {
        struct
        {
            uint32_t VDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW5;

    // uint32_t 6
    union
    {
        struct
        {
            uint32_t RandomValuesForCoordinatesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW6;

    // uint32_t 7
    union
    {
        struct
        {
            uint32_t YGammaCorrectionLutSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW7;

    // uint32_t 8
    union
    {
        struct
        {
            uint32_t UGammaCorrectionLutSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW8;

    // uint32_t 9
    union
    {
        struct
        {
            uint32_t VGammaCorrectionLutSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW9;

    // uint32_t 10
    union
    {
        struct
        {
            uint32_t EnableYFilmGrain : MOS_BITFIELD_RANGE(0, 15);
            uint32_t EnableUFilmGrain : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW10;

    // uint32_t 11
    union
    {
        struct
        {
            uint32_t EnableVFilmGrain : MOS_BITFIELD_RANGE(0, 15);
            uint32_t RandomValuesForCoordinatesTableWidth : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW11;

    // uint32_t 12
    union
    {
        struct
        {
            uint32_t ImageHeight : MOS_BITFIELD_RANGE(0, 15);
            uint32_t ScalingShiftValue : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW12;

    // uint32_t 13
    union
    {
        struct
        {
            uint32_t MinimumYClippingValue : MOS_BITFIELD_RANGE(0, 15);
            uint32_t MaximumYClippingValue : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW13;

    // uint32_t 14
    union
    {
        struct
        {
            uint32_t MinimumUvClippingValue : MOS_BITFIELD_RANGE(0, 15);
            uint32_t MaximumUvClippingValue : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW14;

    // uint32_t 15
    union
    {
        struct
        {
            uint32_t CbLumaMultiplier : MOS_BITFIELD_RANGE(0, 15);
            uint32_t CbMultiplier : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW15;

    // uint32_t 16
    union
    {
        struct
        {
            uint32_t CbOffset : MOS_BITFIELD_RANGE(0, 15);
            uint32_t CrLumaMultiplier : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW16;

    // uint32_t 17
    union
    {
        struct
        {
            uint32_t CrMultiplier : MOS_BITFIELD_RANGE(0, 15);
            uint32_t CrOffset : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW17;

    static const size_t m_size     = 18;
    static const size_t m_byteSize = 72;
};
C_ASSERT(sizeof(FilmGrainApplyNoiseCurbe) == 72);

// Binding Table Definitions
//!
//! \enum     FilmGrainApplyNoiseBindingTableOffset
//! \brief    Binding table offset for ApplyNoise kernel
//!
enum FilmGrainApplyNoiseBindingTableOffset
{
    anInputYOrYuv = 0,
    anInputUv,
    anOutputY,
    anOutputUv,
    anInputYDithering,
    anInputUDithering,
    anInputVDithering,
    anInputRandomValuesCoordinates,
    anInputYGammaLut,
    anInputUGammaLut,
    anInputVGammaLut,
    anNumSurfaces
};

enum FilmGrainKernelStateIdx;

class FilmGrainAppNoisePkt : public RenderCmdPacket
{
public:
    FilmGrainAppNoisePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);
    
    virtual ~FilmGrainAppNoisePkt() {};

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Prepare() override;

    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase) override;
    MOS_STATUS Initilize();
    virtual MOS_STATUS Init() override;
    virtual std::string GetPacketName() override
    {
        return "AV1_DECODE_FilmGrainAppNoise";
    }

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    MOS_STATUS SetCurbeApplyNoise(
        FilmGrainProcParams *procParams);

    //!
    //! \brief  Destroy the media packet and release the resources specific to it
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy() override
    {
        return RenderCmdPacket::Destroy();
    }

protected:
    static const int32_t m_minLumaLegalRange   = 16;
    static const int32_t m_maxLumaLegalRange   = 235;
    static const int32_t m_minChromaLegalRange = 16;
    static const int32_t m_maxChromaLegalRange = 240;

    MOS_STATUS KernelStateSetup();
    MOS_STATUS SetUpSurfaceState();

    virtual MOS_STATUS SetupMediaWalker() override;

protected:
    int32_t                 m_kernelIndex       = applyNoise;
    Kdll_FilterEntry*       m_filter            = nullptr;                 //!< Kernel Filter (points to base of filter array)

    MediaFeatureManager     *m_featureManager   = nullptr;
    Av1PipelineG12_Base     *m_av1Pipeline      = nullptr;
    DecodeAllocator         *m_allocator        = nullptr;
    PMOS_INTERFACE          m_osInterface      = nullptr;
    Av1BasicFeatureG12      *m_av1BasicFeature  = nullptr;
    Av1DecodeFilmGrainG12   *m_filmGrainFeature = nullptr;

    MhwVdboxVdencInterface  *m_vdencInterface   = nullptr;
    CodechalHwInterface     *m_hwInterface      = nullptr;
    DecodeMemComp           *m_mmcState         = nullptr;
    const CodecAv1PicParams *m_picParams        = nullptr;                  //!< Pointer to picture parameter

    // Surfaces for ApplyNoise
    FilmGrainProcParams     *m_filmGrainProcParams = nullptr;

    uint32_t m_bindingTableIndex[anNumSurfaces] =
    {
        anInputYOrYuv,
        anInputUv,
        anOutputY,
        anOutputUv,
        anInputYDithering,
        anInputUDithering,
        anInputVDithering,
        anInputRandomValuesCoordinates,
        anInputYGammaLut,
        anInputUGammaLut,
        anInputVGammaLut,
    };
MEDIA_CLASS_DEFINE_END(decode__FilmGrainAppNoisePkt)
};
}

#endif
