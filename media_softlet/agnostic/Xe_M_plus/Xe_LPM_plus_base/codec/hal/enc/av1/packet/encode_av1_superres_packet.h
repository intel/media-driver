/*
* Copyright (c) 2021 - 2023, Intel Corporation
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
//! \file     encode_av1_superres_packet.h
//! \brief    Downscaling packet for super-res, it will use VE SFC to downscale raw surface.
//!

#ifndef __ENCODE_AV1_SUPERRES_PACKET_H__
#define __ENCODE_AV1_SUPERRES_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_av1_vdenc_pipeline.h"
#include "encode_av1_basic_feature.h"
#include "encode_av1_superres.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_mi_itf.h"
#include "media_sfc_interface.h"

namespace encode
{
class Av1SuperresPkt : public CmdPacket, public mhw::vdbox::avp::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    Av1SuperresPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

    MOS_STATUS Prepare() override;

    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

protected:
    struct SURFACE
    {
        PMOS_SURFACE resource        = nullptr;
        uint32_t     unalignedWidth  = 0;
        uint32_t     unalignedHeight = 0;
    };

    enum class REF_SCALE_TYPE
    {
        NO_SCALE,
        USE_VE_SFC,
        USE_PAK,
    };

protected:
    MOS_STATUS PrepareRawSurface();

    MOS_STATUS PrepareRefSurface();

    MOS_STATUS AllocateScaledRefs(uint8_t scalingIdx);

    MOS_STATUS SubmitVeSfcDownscaling(const SURFACE &inSurf, const SURFACE &outSurf);

protected:
    Av1VdencPipeline *                      m_pipeLine   = nullptr;
    std::shared_ptr<mhw::vdbox::avp::Itf>   m_avpItf     = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf   = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf      = nullptr;
    std::shared_ptr<MediaSfcInterface>      m_sfcItf     = nullptr;
    EncodeMemComp *                         m_mmcState   = nullptr;
    EncodeAllocator *                       m_allocator  = nullptr;
    TrackedBuffer *                         m_trackedBuf = nullptr;

    std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager  = nullptr;
    Av1BasicFeature *                                 m_basicFeature    = nullptr;
    Av1SuperRes *                                     m_superResFeature = nullptr;

    uint32_t m_prevDsWidth = 0;

    SURFACE m_raw         = {};
    SURFACE m_rawDs       = {};
    SURFACE m_ref         = {};
    SURFACE m_ref4x       = {};
    SURFACE m_ref8x       = {};
    SURFACE m_refScaled   = {};
    SURFACE m_ref4xScaled = {};
    SURFACE m_ref8xScaled = {};

    bool m_is10Bit      = false;
    bool m_isIntraFrame = false;
    bool m_useSuperRes  = false;
    bool m_widthChanged = false;

    REF_SCALE_TYPE m_refDsType = REF_SCALE_TYPE::NO_SCALE;

    const uint32_t  m_fakeHeader     = 0;  // fake header for AVP_PAK_INSERT_OBJECT, since we do not need bitstream in PAK downscaling
    mutable uint8_t m_surfaceStateId = 0;

MEDIA_CLASS_DEFINE_END(encode__Av1SuperresPkt)
};
}  // namespace encode

#endif  // __ENCODE_AV1_SUPERRES_PACKET_H__
