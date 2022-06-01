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
//! \file     decode_filmgrain_gennoise_rp1_packet_g12.h
//! \brief    Defines the implementation of av1 decode film grain regres phase1 kernel packet
//!

#ifndef __DECODE_FILMGRAIN_GENNOISE_RP1_PACKET_G12_H__
#define __DECODE_FILMGRAIN_GENNOISE_RP1_PACKET_G12_H__

#include "media_render_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_av1_filmgrain_feature_g12.h"

namespace decode
{

//Curbe definitions
struct FilmGrainRegressPhase1Curbe
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
            uint32_t YDitheringSurface : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t YCoeffSurface : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t Value;
        };
    } DW2;

    static const size_t m_size     = 3;
    static const size_t m_byteSize = 12;
};
C_ASSERT(sizeof(FilmGrainRegressPhase1Curbe) == 12);

// Binding Table Definitions
//!
//! \enum     FilmGrainRegressPhase1BindingTableOffset
//! \brief    Binding table offset for regressPhase1 kernel
//!
enum FilmGrainRegressPhase1BindingTableOffset
{
    rp1InputYRandomValue = 0,
    rp1OutputYDitheringSurface,
    rp1InputYCoeff,
    rp1NumSurfaces
};

enum FilmGrainKernelStateIdx;

class FilmGrainRp1Packet : public RenderCmdPacket
{
public:
    FilmGrainRp1Packet(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);
    
    virtual ~FilmGrainRp1Packet() {};

    MOS_STATUS Prepare() override;
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase) override;
    MOS_STATUS Initilize();
    
    virtual MOS_STATUS Init() override;

    virtual std::string GetPacketName() override
    {
        return "AV1_DECODE_FilmGrainRp1";
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

    MOS_STATUS SetCurbeRegressPhase1();

    MOS_STATUS Destroy() override
    {
        return RenderCmdPacket::Destroy();
    }

protected:
    MOS_STATUS KernelStateSetup();
    MOS_STATUS SetUpSurfaceState();
    virtual MOS_STATUS SetupMediaWalker() override;

protected:
    int32_t                 m_kernelIndex       = regressPhase1;
    Kdll_FilterEntry*       m_filter            = nullptr; // Kernel Filter (points to base of filter array)
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

    uint32_t m_bindingTableIndex[rp1NumSurfaces] =
    {
        rp1InputYRandomValue,
        rp1OutputYDitheringSurface,
        rp1InputYCoeff,
    };
MEDIA_CLASS_DEFINE_END(decode__FilmGrainRp1Packet)
};
}
#endif
