/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_av1_scc.h
//! \brief    SCC feature
//!

#ifndef __ENCODE_AV1_SCC_H__
#define __ENCODE_AV1_SCC_H__

#include "encode_av1_basic_feature.h"

namespace encode
{

enum
{
    IBC_DISABLED = 0,
    LBC_ONLY     = 1,
    IBC_ENABLED  = 3
};

enum
{
    vdencCmd2Par3Value0 = 0,
    vdencCmd2Par3Value1 = 1,
    vdencCmd2Par3Value2 = 2,
    vdencCmd2Par3Value3 = 3
};

enum
{
    vdencCmd2Par136Value0 = 0,
    vdencCmd2Par136Value1 = 2
};

enum
{
    vdencCmd2Par135Value0,
    vdencCmd2Par135Value1,
    vdencCmd2Par135Value2,
    vdencCmd2Par135Value3
};

class Av1Segmentation;

class Av1Scc : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    Av1Scc(EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings,
        MediaFeatureManager *featureManager);

    MOS_STATUS Update(void *params) override;

    virtual ~Av1Scc();

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(VDENC_CMD1);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

    MOS_STATUS UpdateIBCStatusForCurrentTile();

protected:

    MOS_STATUS MakeCdfTrackedBufferLockable();
    MOS_STATUS ResetMvProbsToDefault();

    bool m_enablePalette            = false;
    bool m_enableIBC                = false;
    bool m_IBCPossible              = false;
    bool m_resetMvProbs             = false;
    bool m_IBCEnabledForCurrentTile = false;

    Av1BasicFeature *m_basicFeature = nullptr;  //!< AV1 Basic Feature
    Av1Segmentation *m_segmentation = nullptr;  //!< AV1 Segmentation Feature
    EncodeAllocator *m_allocator    = nullptr;
    MOS_INTERFACE   *m_osInterface  = nullptr;
    const uint16_t   AV1table[10][3] =
    {
        {16, 2, 4},
        {16, 2, 4},
        {16, 2, 4},
        {16, 2, 4},
        {16, 2, 4},
        {16, 2, 5},
        {16, 2, 5},
        {16, 2, 5},
        {16, 2, 5},
        {16, 2, 5},
    };

    PMOS_SURFACE m_intrabcReconSurface = nullptr;  //!< IntraBC reconstructed surface

MEDIA_CLASS_DEFINE_END(encode__Av1Scc)
};
}  // namespace encode

#endif  // __ENCODE_AV1_SCC_H__
