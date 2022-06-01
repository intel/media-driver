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
//! \file     decode_filmgrain_gennoise_grv_packet_g12.h
//! \brief    Defines the implementation of av1 decode film grain get random values kernel packet
//!

#ifndef __DECODE_FILMGRAIN_GENNOISE_GRV_PACKET_G12_H__
#define __DECODE_FILMGRAIN_GENNOISE_GRV_PACKET_G12_H__

#include "media_render_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_av1_filmgrain_feature_g12.h"

namespace decode
{

//Curbe definitions
struct FilmGrainGetRandomValuesCurbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t GaussianSeqSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t YRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t URandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t VRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t CoordinatesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);  //Random values for coordinates surface index
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
            uint32_t NoiseShiftAmount : MOS_BITFIELD_RANGE(0, 15);
            uint32_t Reserved : MOS_BITFIELD_RANGE(16, 31);
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
            uint32_t GrainSeed : MOS_BITFIELD_RANGE(0, 31);  //Random number generation initializer
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
            uint32_t CoordinatesWidth : MOS_BITFIELD_RANGE(0, 15);    //RoundUp(ImageHeight/64)
            uint32_t CoordinatesHeight : MOS_BITFIELD_RANGE(16, 31);  //RoundUp(ImageWidth/64)
        };
        struct
        {
            uint32_t Value;
        };
    } DW7;

    static const size_t m_size     = 8;
    static const size_t m_byteSize = 32;
};
C_ASSERT(sizeof(FilmGrainGetRandomValuesCurbe) == 32);

// Binding Table Definitions
//!
//! \enum     FilmGrainGetRandomValuesBindingTableOffset
//! \brief    Binding table offset for GetRandomValues kernel
//!
enum FilmGrainGetRandomValuesBindingTableOffset
{
    grvInputGaussianSeq = 0,
    grvOutputYRandomValue,
    grvOutputURandomValue,
    grvOutputVRandomValue,
    grvOutputCoordinates,
    grvNumSurfaces
};

enum FilmGrainKernelStateIdx;

class FilmGrainGrvPacket : public RenderCmdPacket
{
public:
    FilmGrainGrvPacket(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);
    
    virtual ~FilmGrainGrvPacket(){};

    MOS_STATUS Prepare() override;
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase) override;
    MOS_STATUS Initilize();

    virtual MOS_STATUS Init() override;

    virtual std::string GetPacketName() override
    {
        return "AV1_DECODE_FilmGrainGrv";
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
    MOS_STATUS SetCurbeGetRandomValues();

protected:
    MOS_STATUS KernelStateSetup();
    MOS_STATUS SetUpSurfaceState();
    virtual MOS_STATUS SetupMediaWalker() override;

protected:
    int32_t             m_kernelIndex   = getRandomValues;
    Kdll_FilterEntry*   m_filter        = nullptr;          // Kernel Filter (points to base of filter array)

    MediaFeatureManager    *m_featureManager   = nullptr;
    Av1PipelineG12_Base    *m_av1Pipeline      = nullptr;
    DecodeAllocator        *m_allocator        = nullptr;
    PMOS_INTERFACE          m_osInterface      = nullptr;
    Av1BasicFeatureG12     *m_av1BasicFeature  = nullptr;
    Av1DecodeFilmGrainG12  *m_filmGrainFeature = nullptr;
    MhwVdboxVdencInterface *m_vdencInterface   = nullptr;
    CodechalHwInterface    *m_hwInterface      = nullptr;
    DecodeMemComp          *m_mmcState         = nullptr;
    const CodecAv1PicParams *m_picParams = nullptr;  //!< Pointer to picture parameter

    uint32_t m_bindingTableIndex[grvNumSurfaces] = {
        grvInputGaussianSeq,
        grvOutputYRandomValue,
        grvOutputURandomValue,
        grvOutputVRandomValue,
        grvOutputCoordinates
    };
MEDIA_CLASS_DEFINE_END(decode__FilmGrainGrvPacket)
};
}

#endif
