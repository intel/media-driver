/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_filmgrain_gennoise_rp2_packet_g12.h
//! \brief    Defines the implementation of av1 decode film grain regress phase2 kernel packet
//!

#ifndef __DECODE_FILMGRAIN_GENNOISE_RP2_PACKET_G12_H__
#define __DECODE_FILMGRAIN_GENNOISE_RP2_PACKET_G12_H__

#include "media_render_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_av1_filmgrain_feature_g12.h"

namespace decode
{
//Curbe definitions
struct FilmGrainRegressPhase2Curbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t YRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t URandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t VRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t YDitheringInputSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t YDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t UDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t VDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t YCoeffSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t UCoeffSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t VCoeffSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t RegressionCoefficientShift : MOS_BITFIELD_RANGE(0, 15);
            uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW10;

    static const size_t m_size     = 11;
    static const size_t m_byteSize = 44;
};
C_ASSERT(sizeof(FilmGrainRegressPhase2Curbe) == 44);

// Binding Table Definitions
//!
//! \enum     FilmGrainRegressPhase2BindingTableOffset
//! \brief    Binding table offset for regressPhase2 kernel
//!
enum FilmGrainRegressPhase2BindingTableOffset
{
    rp2InputYRandomValue = 0,
    rp2InputURandomValue,
    rp2InputVRandomValue,
    rp2InputYDithering,
    rp2OutputYDithering,
    rp2OutputUDithering,
    rp2OutputVDithering,
    rp2InputYCoeff,
    rp2InputUCoeff,
    rp2InputVCoeff,
    rp2NumSurfaces
};

enum FilmGrainKernelStateIdx;

class FilmGrainRp2Packet : public RenderCmdPacket
{
public:
    FilmGrainRp2Packet(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);

    virtual ~FilmGrainRp2Packet() {};

    MOS_STATUS Prepare() override;
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase) override;
    MOS_STATUS Initilize();

    virtual MOS_STATUS Init() override;

    virtual std::string GetPacketName() override
    {
        return "AV1_DECODE_FilmGrainRp2";
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

    MOS_STATUS SetCurbeRegressPhase2();

    MOS_STATUS Destroy() override
    {
        return RenderCmdPacket::Destroy();
    }

protected:
    MOS_STATUS KernelStateSetup();
    MOS_STATUS SetUpSurfaceState();
    virtual MOS_STATUS SetupMediaWalker() override;

protected:
    int32_t                 m_kernelIndex       = regressPhase2;
    Kdll_FilterEntry*       m_filter            = nullptr;                                       // Kernel Filter (points to base of filter array)

    MediaFeatureManager     *m_featureManager   = nullptr;
    Av1PipelineG12_Base     *m_av1Pipeline      = nullptr;
    DecodeAllocator         *m_allocator        = nullptr;
    PMOS_INTERFACE          m_osInterface       = nullptr;
    Av1BasicFeatureG12      *m_av1BasicFeature  = nullptr;
    Av1DecodeFilmGrainG12   *m_filmGrainFeature = nullptr;

    MhwVdboxVdencInterface  *m_vdencInterface   = nullptr;
    CodechalHwInterface     *m_hwInterface      = nullptr;
    DecodeMemComp           *m_mmcState         = nullptr;
    const CodecAv1PicParams *m_picParams        = nullptr;  //!< Pointer to picture parameter

    uint32_t m_bindingTableIndex[rp2NumSurfaces] =
    {
            rp2InputYRandomValue,
            rp2InputURandomValue,
            rp2InputVRandomValue,
            rp2InputYDithering,
            rp2OutputYDithering,
            rp2OutputUDithering,
            rp2OutputVDithering,
            rp2InputYCoeff,
            rp2InputUCoeff,
            rp2InputVCoeff,
    };
MEDIA_CLASS_DEFINE_END(decode__FilmGrainRp2Packet)
};
}
#endif
