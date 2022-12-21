/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_brc.cpp
//! \brief    Defines the common interface for vp9 brc features
//!

#include "encode_vp9_brc.h"
#include "encode_utils.h"
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "encode_vp9_segmentation.h"

namespace encode
{
constexpr int8_t Vp9EncodeBrc::m_instRateThresholdI[Vp9EncodeBrc::m_numInstRateThresholds];
constexpr int8_t Vp9EncodeBrc::m_instRateThresholdP[Vp9EncodeBrc::m_numInstRateThresholds];
constexpr double Vp9EncodeBrc::m_devThresholdFpNegI[Vp9EncodeBrc::m_numDevThresholds / 2];
constexpr double Vp9EncodeBrc::m_devThresholdFpPosI[Vp9EncodeBrc::m_numDevThresholds / 2];
constexpr double Vp9EncodeBrc::m_devThresholdFpNegPB[Vp9EncodeBrc::m_numDevThresholds / 2];
constexpr double Vp9EncodeBrc::m_devThresholdFpPosPB[Vp9EncodeBrc::m_numDevThresholds / 2];
constexpr double Vp9EncodeBrc::m_devThresholdVbrNeg[Vp9EncodeBrc::m_numDevThresholds / 2];
constexpr double Vp9EncodeBrc::m_devThresholdVbrPos[Vp9EncodeBrc::m_numDevThresholds / 2];

const uint32_t Vp9EncodeBrc::m_brcConstData[2][416] =
{
    // I Frame
    {
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x32191900, 0x00264B4B, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x0E0A0602, 0x06040212, 0x00000E0A, 0x00080402, 0x04020000, 0x000000FE, 0xFEFCFA02, 0xF8F60000,
        0xF200FEFC, 0xFEFCF8F4, 0xFCF6F2EE, 0x0A0402FE, 0x04021410, 0x00100C08, 0x0C080402, 0x02000000,
        0x0000FE04, 0xFEFC0200, 0xFA0000FE, 0x00FEFEFC, 0xFEFCFAF6, 0xF8F4F200, 0x0402FEFC, 0x0214100A,
        0x100C0804, 0x08040200, 0x0000000C, 0x00FE0402, 0xFC020000, 0x0000FEFE, 0xFEFEFCFA, 0xFCFAF600,
        0xF4F200FE, 0x00FEFCF8, 0x00000000, 0x14100C08, 0x00000000, 0x0E0A0600, 0x0000FE12, 0x08060000,
        0xFEFC0E0C, 0x02000000, 0xFA0A0604, 0x0000FEFC, 0x0A060200, 0x00FEFCF8, 0x06020000, 0xFCFAF60A,
        0x020000FE, 0xF8F40A06, 0x0000FEFC, 0xF40A0602, 0x00FEFCF8, 0x0A060200, 0x00000000, 0x0E0A0600,
        0x00000012, 0x0A060000, 0x00FE100C, 0x06000000, 0xFC100E0A, 0x000000FE, 0x0C0A0804, 0x00FEFCFA,
        0x08020000, 0xFEFCF80A, 0x02000000, 0xFCF80A08, 0x0000FEFE, 0xF80A0800, 0x00FEFCFA, 0x0A020000,
        0xFEFCF8F6, 0x02000000, 0x00000008, 0x0A060000, 0x0000120E, 0x06000000, 0xFE100C0A, 0x00000000,
        0x100E0A06, 0x0000FEFC, 0x0A080400, 0xFEFCFA0C, 0x02000000, 0xFCF80A08, 0x000000FE, 0xF80A0802,
        0x00FEFEFC, 0x0A080000, 0xFEFCFAF8, 0x02000000, 0xFCF8F60A, 0x000000FE, 0x00000802, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    // P Frame
    {
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626,
        0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904,
        0x0B0B001E, 0x0000000B, 0x13194B0D, 0x1F5E2626, 0x19321F4B, 0x1E1E1904, 0x0B0B001E, 0x0000000B,
        0x0E0A0602, 0x06040212, 0x00000E0A, 0x00080402, 0x04020000, 0x000000FE, 0xFEFCFA02, 0xF8F60000,
        0xF200FEFC, 0xFEFCF8F4, 0xFCF6F2EE, 0x0A0402FE, 0x04021410, 0x00100C08, 0x0C080402, 0x02000000,
        0x0000FE04, 0xFEFC0200, 0xFA0000FE, 0x00FEFEFC, 0xFEFCFAF6, 0xF8F4F200, 0x0402FEFC, 0x0214100A,
        0x100C0804, 0x08040200, 0x0000000C, 0x00FE0402, 0xFC020000, 0x0000FEFE, 0xFEFEFCFA, 0xFCFAF600,
        0xF4F200FE, 0x00FEFCF8, 0x00000000, 0x14100C08, 0x00000000, 0x0E0A0600, 0x0000FE12, 0x08060000,
        0xFEFC0E0C, 0x02000000, 0xFA0A0604, 0x0000FEFC, 0x0A060200, 0x00FEFCF8, 0x06020000, 0xFCFAF60A,
        0x020000FE, 0xF8F40A06, 0x0000FEFC, 0xF40A0602, 0x00FEFCF8, 0x0A060200, 0x00000000, 0x0E0A0600,
        0x00000012, 0x0A060000, 0x00FE100C, 0x06000000, 0xFC100E0A, 0x000000FE, 0x0C0A0804, 0x00FEFCFA,
        0x08020000, 0xFEFCF80A, 0x02000000, 0xFCF80A08, 0x0000FEFE, 0xF80A0800, 0x00FEFCFA, 0x0A020000,
        0xFEFCF8F6, 0x02000000, 0x00000008, 0x0A060000, 0x0000120E, 0x06000000, 0xFE100C0A, 0x00000000,
        0x100E0A06, 0x0000FEFC, 0x0A080400, 0xFEFCFA0C, 0x02000000, 0xFCF80A08, 0x000000FE, 0xF80A0802,
        0x00FEFEFC, 0x0A080000, 0xFEFCFAF8, 0x02000000, 0xFCF8F60A, 0x000000FE, 0x00000802, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    }
};

Vp9EncodeBrc::Vp9EncodeBrc(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings) : MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
                           m_hwInterface(hwInterface),
                           m_allocator(allocator)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);

    m_featureManager = featureManager;
    auto encFeatureManager = dynamic_cast<EncodeVp9VdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

    m_vdencInterfaceNew = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_vdencInterfaceNew);

    m_hcpInterfaceNew = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpInterfaceNew);
}

Vp9EncodeBrc::~Vp9EncodeBrc()
{
    FreeResources();
}

MOS_STATUS Vp9EncodeBrc::Init(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

    m_vdencEnabled = true;

    // VDENC BRC related buffer size
    m_brcHistoryBufferSize = m_brcHistoryBufSize;
    // VDENC BRC related buffer size
    m_vdencBrcStatsBufferSize    = m_brcStatsBufSize;
    m_vdencBrcPakStatsBufferSize = m_brcPakStatsBufSize;

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "VP9 Encode Multipass BRC Enable",
        MediaUserSetting::Group::Sequence);
    m_multipassBrcSupported = outValue.Get<bool>();
#endif

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;
    auto           vp9PicParams = m_basicFeature->m_vp9PicParams;
    auto           vp9SeqParams = m_basicFeature->m_vp9SeqParams;

    if (m_basicFeature->m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }

    auto featureManager = dynamic_cast<EncodeVp9VdencFeatureManager *>(m_featureManager);
    ENCODE_CHK_NULL_RETURN(featureManager);
    auto numPasses = featureManager->GetNumPass();

    auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
    auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

    if (dysRefFrameFlags != DYS_REF_NONE && dysVdencMultiPassEnabled)
    {
        if (m_vdencBrcEnabled)
        {
            m_basicFeature->m_dysBrc = true;
            // Reduce passes by 1, as m_numPasses == 1 becomes m_numPasses = 0 for Huc to run
            numPasses = (numPasses > 0) ? (numPasses - 1) : numPasses;
            featureManager->SetNumPass(numPasses);
        }
        else
        {
            m_basicFeature->m_dysCqp = true;
        }
    }

    // This is BRC SinglePass case
    // Actually, repak is disabled
    if (m_vdencBrcEnabled && (dysRefFrameFlags != DYS_REF_NONE) && !dysVdencMultiPassEnabled)
    {
        m_basicFeature->m_dysBrc = true;
        // Reduce passes by 1, as m_numPasses == 1 becomes m_numPasses = 0 for Huc to run
        numPasses = (numPasses > 0) ? (numPasses - 1) : numPasses;
        featureManager->SetNumPass(numPasses);
    }

    if (!m_vdencBrcEnabled && (dysRefFrameFlags != DYS_REF_NONE))
    {
        m_basicFeature->m_dysCqp = true;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::SetRegionsForBrcUpdate(mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const
{
    ENCODE_FUNC_CALL();

    // Initialize BRC constant data buffer
    ENCODE_CHK_STATUS_RETURN(InitConstantDataBuffer());

    // History Buffer - IN/OUT
    params.regionParams[0].presRegion = const_cast<PMOS_RESOURCE>(& m_brcBuffers.resBrcHistoryBuffer);
    params.regionParams[0].isWritable = true;
    // VDENC Stats Buffer - IN
    params.regionParams[1].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
    params.regionParams[1].dwOffset   = 0;
    // Frame (not PAK) Stats Buffer - IN
    params.regionParams[2].presRegion = const_cast<PMOS_RESOURCE>(&m_resFrameStatStreamOutBuffer);
    params.regionParams[2].dwOffset   = 0;
    // BRC Data - OUT
    params.regionParams[4].presRegion = const_cast<PMOS_RESOURCE>(&m_brcBuffers.resBrcHucDataBuffer);
    params.regionParams[4].isWritable = true;
    // Const Data - IN
    params.regionParams[5].presRegion = const_cast<PMOS_RESOURCE>(GetBrcConstantBuffer());
    // PAK MMIO - IN
    params.regionParams[7].presRegion = const_cast<PMOS_RESOURCE>(&m_brcBuffers.resBrcBitstreamSizeBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::SetDmemForUpdate(void *params, bool isFirstPass) const
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    auto dmem = (HucBrcUpdateDmem *)params;

    const CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = m_basicFeature->m_vp9SeqParams;
    const CODEC_VP9_ENCODE_PIC_PARAMS *     vp9PicParams = m_basicFeature->m_vp9PicParams;

    // BRC update setting
    if (m_curTargetFullness > vp9SeqParams->VBVBufferSizeInBit)
    {
        dmem->UPD_OVERFLOW_FLAG_U8 = 0x1;
        m_curTargetFullness -= vp9SeqParams->VBVBufferSizeInBit;
    }
    // We only set target fullness on first BRC pass
    if (isFirstPass)
    {
        dmem->UPD_TARGET_BUF_FULLNESS_U32 = (int32_t)m_curTargetFullness;
    }

    dmem->UPD_FRAMENUM_U32                = m_basicFeature->m_frameNum;
    dmem->UPD_Temporal_Level_U8           = vp9PicParams->temporal_id;
    dmem->UPD_HRD_BUFF_FULLNESS_UPPER_I32 = vp9SeqParams->UpperVBVBufferLevelThresholdInBit;
    dmem->UPD_HRD_BUFF_FULLNESS_LOWER_I32 = vp9SeqParams->LowerVBVBufferLevelThresholdInBit;

    // Frame info.
    dmem->UPD_CurWidth_U16     = (uint16_t)m_basicFeature->m_frameWidth;
    dmem->UPD_CurHeight_U16    = (uint16_t)m_basicFeature->m_frameHeight;
    dmem->UPD_CurrFrameType_U8 = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 2 : 0;

    // Offsets
    dmem->UPD_VDEncImgStateOffset = m_basicFeature->m_slbbImgStateOffset;
    dmem->UPD_SLBBSize            = m_basicFeature->m_hucSlbbSize;
    dmem->UPD_PicStateOffset      = m_basicFeature->m_hucPicStateOffset;

    // Thresholds not programmed by driver currently

    // Global adjust settings not programmed by driver currently.

    // QP's
    dmem->UPD_ACQQp_U8 = vp9PicParams->LumaACQIndex;

    // We increment by the average frame value once for each frame
    if (isFirstPass)
    {
        m_curTargetFullness += m_inputBitsPerFrame;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::SetDmemForInit(void *params) const
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    auto dmem = (HucBrcInitDmem *)params;

    const CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = m_basicFeature->m_vp9SeqParams;

    dmem->BRCFunc              = IsBrcInit() ? 0 : 2;  // 0 for init, 2 for reset
    dmem->ProfileLevelMaxFrame = m_basicFeature->GetProfileLevelMaxFrameSize();
    dmem->InitBufFullness      = vp9SeqParams->InitVBVBufferFullnessInBit;
    dmem->BufSize              = vp9SeqParams->VBVBufferSizeInBit;

    dmem->TargetBitrate = vp9SeqParams->TargetBitRate[vp9SeqParams->NumTemporalLayersMinus1] * CODECHAL_ENCODE_BRC_KBPS;
    dmem->MaxRate       = vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;

    ENCODE_CHK_STATUS_RETURN(SetBrcSettings(dmem));

    m_inputBitsPerFrame = ((vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS) * 100.) / ((vp9SeqParams->FrameRate[vp9SeqParams->NumTemporalLayersMinus1].uiNumerator * 100.) / vp9SeqParams->FrameRate[vp9SeqParams->NumTemporalLayersMinus1].uiDenominator);
    m_curTargetFullness = vp9SeqParams->TargetBitRate[vp9SeqParams->NumTemporalLayersMinus1] * CODECHAL_ENCODE_BRC_KBPS;

    // lets allow huc to calculate it
    dmem->GoldenFrameInterval = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::GetHucBrcBuffers(HucBrcBuffers *&buffers)
{
    ENCODE_FUNC_CALL();

    buffers = &m_brcBuffers;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::GetBrcHistoryBufferSize(uint32_t &size)
{
    ENCODE_FUNC_CALL();

    size = m_brcHistoryBufferSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::GetVdencBrcStatsBufferSize(uint32_t &size)
{
    ENCODE_FUNC_CALL();

    size = m_vdencBrcStatsBufferSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::GetVdencBrcPakStatsBufferSize(uint32_t &size)
{
    ENCODE_FUNC_CALL();

    size = m_vdencBrcPakStatsBufferSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::AllocateResources()
{
    ENCODE_FUNC_CALL();
    MOS_RESOURCE *allocatedBuffer = nullptr;

    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

    // Initiate allocation parameters
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // VDENC BRC statistics buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_basicFeature->m_maxTileNumber * m_vdencBrcStatsBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Statistics Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
    m_basicFeature->m_recycleBuf->RegisterResource(VdencStatsBuffer, allocParamsForBufferLinear, 1);

    // Allocate frame statistics stream out data destination buffer
    uint32_t size                       = MOS_ALIGN_CEIL(m_vdencBrcPakStatsBufferSize, CODECHAL_PAGE_SIZE);  // Align to page is HuC requirement
    allocParamsForBufferLinear.dwBytes  = size * m_basicFeature->m_maxTileNumber;
    allocParamsForBufferLinear.pBufName = "FrameStatStreamOutBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(FrameStatStreamOutBuffer, allocParamsForBufferLinear, 1);
    allocatedBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resFrameStatStreamOutBuffer = *allocatedBuffer;

    uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(&m_resFrameStatStreamOutBuffer);
    ENCODE_CHK_NULL_RETURN(data);
    MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
    m_allocator->UnLock(&m_resFrameStatStreamOutBuffer);

    // BRC history buffer
    allocParamsForBufferLinear.dwBytes  = m_vdencEnabled ? MOS_ALIGN_CEIL(m_brcHistoryBufferSize, CODECHAL_PAGE_SIZE) : m_brcHistoryBufferSize;
    allocParamsForBufferLinear.pBufName = "BRC History Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(VdencBRCHistoryBuffer, allocParamsForBufferLinear, 1);
    allocatedBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, 0);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resBrcHistoryBuffer = *allocatedBuffer;

    // BRC constant data buffer of I/P frame
    allocParamsForBufferLinear.dwBytes  = m_vdencEnabled ? MOS_ALIGN_CEIL(m_brcConstantSurfaceSize, CODECHAL_PAGE_SIZE) : CODECHAL_ENCODE_VP9_BRC_CONSTANTSURFACE_SIZE;
    allocParamsForBufferLinear.pBufName = "BRC Constant Data Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;

    for (auto i = 0; i < 2; ++i)
    {
        allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_brcBuffers.resBrcConstantDataBuffer[i] = *allocatedBuffer;
    }

    // PicState BRC read buffer
    size                                = CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS * m_brcMaxNumPasses;
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "BRC Pic State Read Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resPicStateBrcReadBuffer = *allocatedBuffer;

    data = (uint8_t *)m_allocator->LockResourceForWrite(&m_brcBuffers.resPicStateBrcReadBuffer);
    ENCODE_CHK_NULL_RETURN(data);
    MOS_ZeroMemory(data, size);
    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(&m_brcBuffers.resPicStateBrcReadBuffer));

    // PicState BRC write and HuC read buffers
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "BRC Pic State Write Buffer";
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resPicStateBrcWriteHucReadBuffer = *allocatedBuffer;

    data = (uint8_t *)m_allocator->LockResourceForWrite(&m_brcBuffers.resPicStateBrcWriteHucReadBuffer);
    ENCODE_CHK_NULL_RETURN(data);
    MOS_ZeroMemory(data, size);
    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(&m_brcBuffers.resPicStateBrcWriteHucReadBuffer));

    // PicState HuC write buffer
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "BRC Huc Pic State Write Buffer";
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resPicStateHucWriteBuffer = *allocatedBuffer;

    // SegmentState BRC read buffer
    allocParamsForBufferLinear.dwBytes  = CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE;
    allocParamsForBufferLinear.pBufName = "BRC Segment State Read Buffer";
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resSegmentStateBrcReadBuffer = *allocatedBuffer;

    // SegmentState BRC write buffer
    allocParamsForBufferLinear.dwBytes  = CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE;
    allocParamsForBufferLinear.pBufName = "BRC Segment State Write Buffer";
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resSegmentStateBrcWriteBuffer = *allocatedBuffer;

    // BRC bitstream size data buffer
    allocParamsForBufferLinear.dwBytes  = m_vdencEnabled ? MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER_SIZE, CODECHAL_PAGE_SIZE) : CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER_SIZE;
    allocParamsForBufferLinear.pBufName = "BRC Bitstream Size Data buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resBrcBitstreamSizeBuffer = *allocatedBuffer;

    // BRC HuC data buffer
    allocParamsForBufferLinear.dwBytes  = m_vdencEnabled ? MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE) : CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE;
    allocParamsForBufferLinear.pBufName = "BRC HuC Data Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resBrcHucDataBuffer = *allocatedBuffer;

    // BRC MSDK buffer
    allocParamsForBufferLinear.dwBytes  = CODECHAL_ENCODE_VP9_BRC_MSDK_PAK_BUFFER_SIZE;
    allocParamsForBufferLinear.pBufName = "BRC MSDK Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_brcBuffers.resBrcMsdkPakBuffer = *allocatedBuffer;

    // Huc debug output buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(1024 * sizeof(uint32_t), CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "HucDebugOutputBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcDebugBuffer, allocParamsForBufferLinear, 1);

    // Huc VP9 pak mmio buffer
    allocParamsForBufferLinear.dwBytes  = 4 * sizeof(uint32_t);
    allocParamsForBufferLinear.pBufName = "HucPakMmioBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(VdencBrcPakMmioBuffer, allocParamsForBufferLinear, 1);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::FreeResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hwInterface);

    return eStatus;
}

const MOS_RESOURCE* Vp9EncodeBrc::GetBrcConstantBuffer() const
{
    ENCODE_FUNC_CALL();

    uint16_t pictureCodingType = m_basicFeature->m_pictureCodingType;
    ENCODE_ASSERT(pictureCodingType == I_TYPE || pictureCodingType == P_TYPE);

    return &(m_brcBuffers.resBrcConstantDataBuffer[pictureCodingType - 1]);
}

MOS_STATUS Vp9EncodeBrc::InitConstantDataBuffer() const
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    if (m_initBrcConstantDataBuffer)
        return eStatus;

    uint16_t pictureCodingType = m_basicFeature->m_pictureCodingType;
    ENCODE_ASSERT(pictureCodingType == I_TYPE || pictureCodingType == P_TYPE);

    // I/P frame const data
    for (auto i = 0; i < 2; ++i)
    {
        const PMOS_RESOURCE brcConstDataBuffer = const_cast<PMOS_RESOURCE>(&m_brcBuffers.resBrcConstantDataBuffer[i]);
        uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(brcConstDataBuffer);
        ENCODE_CHK_NULL_RETURN(data);

        eStatus = MOS_SecureMemcpy(data,
            sizeof(m_brcConstData[i]),
            m_brcConstData[i],
            sizeof(m_brcConstData[i]));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(brcConstDataBuffer));
            ENCODE_ASSERTMESSAGE("Failed to initialize constant memory buffer.");
            return eStatus;
        }

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(brcConstDataBuffer));
    }

    const_cast<Vp9EncodeBrc *>(this)->m_initBrcConstantDataBuffer = true;

    return eStatus;
}

MOS_STATUS Vp9EncodeBrc::SetBrcSettings(void *params) const
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    auto dmem = (HucBrcInitDmem *)params;

    const CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = m_basicFeature->m_vp9SeqParams;

    FRAME_RATE targetFR = vp9SeqParams->FrameRate[vp9SeqParams->NumTemporalLayersMinus1];
    dmem->FrameRateM    = targetFR.uiNumerator;
    dmem->FrameRateD    = targetFR.uiDenominator;

    switch (vp9SeqParams->RateControlMethod)
    {
    case RATECONTROL_CBR:
        dmem->BRCFlag = CODECHAL_ENCODE_BRCINIT_ISCBR;
        dmem->MaxRate = dmem->TargetBitrate;
        break;
    case RATECONTROL_VBR:
        dmem->BRCFlag = CODECHAL_ENCODE_BRCINIT_ISVBR;
        break;
    case RATECONTROL_AVBR:
        dmem->BRCFlag = CODECHAL_ENCODE_BRCINIT_ISAVBR;
        break;
    case RATECONTROL_CQL:
        dmem->BRCFlag = CODECHAL_ENCODE_BRCINIT_ISCQL;
        dmem->LevelQP = vp9SeqParams->ICQQualityFactor;
        break;
    default:
        ENCODE_ASSERTMESSAGE("BRCInit: Invalid rate control provided (%d)", vp9SeqParams->RateControlMethod);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (dmem->MaxRate < dmem->TargetBitrate)
    {
        dmem->MaxRate = 2 * dmem->TargetBitrate;
    }

    dmem->GopP        = vp9SeqParams->GopPicSize - 1;
    dmem->FrameWidth  = (uint16_t)m_basicFeature->m_frameWidth;
    dmem->FrameHeight = (uint16_t)m_basicFeature->m_frameHeight;

    // Limit 1-255 as the QP range
    dmem->MinQP = 1;
    dmem->MaxQP = CODEC_VP9_MAX_QP;

    dmem->EnableScaling = vp9SeqParams->SeqFlags.fields.EnableDynamicScaling;

    for (auto i = 0; i < m_numInstRateThresholds; ++i)
    {
        dmem->InstRateThreshI0[i] = m_instRateThresholdI[i];
        dmem->InstRateThreshP0[i] = m_instRateThresholdP[i];
    }

    double inputBitsPerFrame = ((double)dmem->MaxRate * (double)dmem->FrameRateD) / (double)dmem->FrameRateM;
    // Make sure the buffer size can contain at least 4 frames in average
    if (dmem->BufSize < (uint32_t)(inputBitsPerFrame * 4))
    {
        dmem->BufSize = (uint32_t)(inputBitsPerFrame * 4);
    }
    // Make sure the initial buffer size is larger than 2 average frames and smaller than the max buffer size.
    if (dmem->InitBufFullness == 0)
    {
        dmem->InitBufFullness = 7 * dmem->BufSize / 8;
    }
    if (dmem->InitBufFullness < (uint32_t)(inputBitsPerFrame * 2))
    {
        dmem->InitBufFullness = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (dmem->InitBufFullness > dmem->BufSize)
    {
        dmem->InitBufFullness = dmem->BufSize;
    }

    double bpsRatio = inputBitsPerFrame / ((double)dmem->BufSize / m_devStdFps);
    bpsRatio        = MOS_CLAMP_MIN_MAX(bpsRatio, m_bpsRatioLow, m_bpsRatioHigh);
    for (auto i = 0; i < m_numDevThresholds / 2; i++)
    {
        dmem->DevThreshPB0[i]                          = (int8_t)(m_negMultPb * pow(m_devThresholdFpNegPB[i], bpsRatio));
        dmem->DevThreshPB0[i + m_numDevThresholds / 2] = (int8_t)(m_posMultPb * pow(m_devThresholdFpPosPB[i], bpsRatio));

        dmem->DevThreshI0[i]                          = (int8_t)(m_negMultPb * pow(m_devThresholdFpNegI[i], bpsRatio));
        dmem->DevThreshI0[i + m_numDevThresholds / 2] = (int8_t)(m_posMultPb * pow(m_devThresholdFpPosI[i], bpsRatio));

        dmem->DevThreshVBR0[i]                          = (int8_t)(m_negMultVbr * pow(m_devThresholdVbrNeg[i], bpsRatio));
        dmem->DevThreshVBR0[i + m_numDevThresholds / 2] = (int8_t)(m_posMultVbr * pow(m_devThresholdVbrPos[i], bpsRatio));
    }

    int32_t qpI = 0, qpP = 0;
    ENCODE_CHK_STATUS_RETURN(ComputeVDEncInitQP(qpI, qpP));

    dmem->InitQPI = (uint8_t)qpI;
    dmem->InitQPP = (uint8_t)qpP;

    dmem->Total_Level = vp9SeqParams->NumTemporalLayersMinus1 + 1;
    if (dmem->Total_Level > 1)
    {
        ENCODE_CHK_STATUS_RETURN(CalculateTemporalRatios(
            dmem->Total_Level, dmem->TargetBitrate, targetFR, dmem->MaxLevel_Ratio));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::ComputeVDEncInitQP(int32_t &initQpI, int32_t &initQpP) const
{
    ENCODE_FUNC_CALL();

    uint32_t frameSize = ((m_basicFeature->m_frameWidth * m_basicFeature->m_frameHeight * 3) >> 1);

    const float x0 = 0, y0 = 1.19f, x1 = 1.75f, y1 = 1.75f;

    int32_t qpP = (uint32_t)(1. / 1.2 * pow(10.0, (log10(frameSize * 2. / 3. * ((float)m_basicFeature->m_vp9SeqParams->FrameRate[0].uiNumerator) / ((float)m_basicFeature->m_vp9SeqParams->TargetBitRate[0] * CODECHAL_ENCODE_BRC_KBPS * m_basicFeature->m_vp9SeqParams->FrameRate[0].uiDenominator)) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);
    qpP         = (int32_t)((float)qpP * (5.0));
    qpP -= 20;
    qpP = MOS_CLAMP_MIN_MAX(qpP, 1, 200);

    int32_t  qpI     = (qpP > 4) ? (qpP - 4) : qpP;
    uint16_t numP    = m_basicFeature->m_vp9SeqParams->GopPicSize - 1;
    int16_t  qiboost = numP / 30 - 1;
    qiboost          = MOS_CLAMP_MIN_MAX(qiboost, 0, 20);

    qpI -= qiboost;
    qpI = MOS_CLAMP_MIN_MAX(qpI, 1, 200);

    qpP = qpI + 20;

    initQpI = qpI;
    initQpP = qpP;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeBrc::CalculateTemporalRatios(uint16_t numberOfLayers, uint32_t maxTemporalBitrate, FRAME_RATE maxTemporalFrameRate, uint8_t *maxLevelRatios) const
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(maxLevelRatios);

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = m_basicFeature->m_vp9SeqParams;

    if (numberOfLayers <= 1)
    {
        ENCODE_ASSERTMESSAGE("Need to have multiple temporal layers to calculate ratios");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (numberOfLayers > CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS)
    {
        ENCODE_ASSERTMESSAGE("VP9 VDEnc supports only %d temporal layers (%d provided)",
            CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS,
            numberOfLayers);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!maxTemporalBitrate || !maxTemporalFrameRate.uiDenominator)
    {
        ENCODE_ASSERTMESSAGE("Invalid bitrate or framerate provided to calculate ratios");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Calculate normalized denominator as least common multiplier of all layers denominators
    uint32_t normalizedDenominator = 1;
    normalizedDenominator          = CalculateNormalizedDenominator(vp9SeqParams->FrameRate, numberOfLayers, normalizedDenominator);

    // Calculate 0 layer framerate multiplier and apply it
    FRAME_RATE currentLayerFrameRate = vp9SeqParams->FrameRate[0];
    uint32_t   frameRateMultiplier   = normalizedDenominator / currentLayerFrameRate.uiDenominator;
    currentLayerFrameRate.uiNumerator *= frameRateMultiplier;
    currentLayerFrameRate.uiDenominator *= frameRateMultiplier;

    uint32_t currentLayerBitrate = vp9SeqParams->TargetBitRate[0] * CODECHAL_ENCODE_BRC_KBPS;

    maxLevelRatios[0] = (currentLayerBitrate << 6) / maxTemporalBitrate *
                        currentLayerFrameRate.uiDenominator / maxTemporalFrameRate.uiDenominator *
                        maxTemporalFrameRate.uiNumerator / currentLayerFrameRate.uiNumerator;

    for (auto i = 1; i < numberOfLayers; ++i)
    {
        // From ddi
        // framerate and bitrate are provided on asceding order
        // 0 indexed is base player properties (bitrate and framerate)
        // 1 indexed is first layer properties including layer below (which is base)
        // so on, every current layer properties values include current and all previous layers properties values

        // Extract actual layer bitrate
        currentLayerBitrate = vp9SeqParams->TargetBitRate[i] * CODECHAL_ENCODE_BRC_KBPS -
                              vp9SeqParams->TargetBitRate[i - 1] * CODECHAL_ENCODE_BRC_KBPS;

        // Extract actual layer framerate
        currentLayerFrameRate.uiNumerator   = vp9SeqParams->FrameRate[i].uiNumerator * (normalizedDenominator / vp9SeqParams->FrameRate[i].uiDenominator) -
                                              vp9SeqParams->FrameRate[i - 1].uiNumerator * (normalizedDenominator / vp9SeqParams->FrameRate[i - 1].uiDenominator);
        currentLayerFrameRate.uiDenominator = normalizedDenominator;

        // Based on hardware behavior to calculate ratio
        // current layer bitrate is in unit of 1 / 64
        // 64 is just a number to represent a range or temporal bitrate for different layers
        // For ex: 22,22,20 means each layer splits in the ratio of 22/64, 22/64 and 20/64 in terms of bitrate that needs to be achieved
        maxLevelRatios[i] = (currentLayerBitrate << 6) /
                            maxTemporalBitrate * currentLayerFrameRate.uiDenominator /
                            maxTemporalFrameRate.uiDenominator * maxTemporalFrameRate.uiNumerator /
                            currentLayerFrameRate.uiNumerator;
    }

    return eStatus;
}

uint32_t Vp9EncodeBrc::CalculateNormalizedDenominator(FRAME_RATE *frameRates, uint16_t numberOfLayers, uint32_t normalizedDenominator) const
{
    ENCODE_FUNC_CALL();

    // If pointer to the list of FrameRates is null, return the current Normalized Denominator.
    if (!frameRates)
    {
        return normalizedDenominator;
    }

    if (numberOfLayers == 0)
    {
        return normalizedDenominator;
    }

    normalizedDenominator = normalizedDenominator * frameRates[numberOfLayers - 1].uiDenominator / MosUtilities::MosGCD(normalizedDenominator, frameRates[numberOfLayers - 1].uiDenominator);

    return CalculateNormalizedDenominator(frameRates, numberOfLayers - 1, normalizedDenominator);
}

MOS_STATUS Vp9EncodeBrc::SetSequenceStructs()
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_VP9_ENCODE_SEQUENCE_PARAMS *vp9SeqParams = m_basicFeature->m_vp9SeqParams;
    ENCODE_CHK_NULL_RETURN(vp9SeqParams);

    m_brcEnabled = CodecHalIsRateControlBrc(vp9SeqParams->RateControlMethod, CODECHAL_VP9);
    if (m_brcEnabled)
    {
        m_brcReset        = vp9SeqParams->SeqFlags.fields.bResetBRC;
        m_vdencBrcEnabled = true;
    }

    m_basicFeature->m_tsEnabled = (vp9SeqParams->NumTemporalLayersMinus1 > 0) ? true : false;

    if (m_basicFeature->m_tsEnabled && m_brcEnabled)
    {
        // Check base layer properties
        ENCODE_ASSERT(vp9SeqParams->FrameRate[0].uiNumerator > 0 && vp9SeqParams->FrameRate[0].uiDenominator > 0);
        ENCODE_ASSERT(vp9SeqParams->TargetBitRate[0] > 0);

        for (auto i = 1; i < (vp9SeqParams->NumTemporalLayersMinus1 + 1); ++i)
        {
            // Check current layer properties
            ENCODE_ASSERT(vp9SeqParams->FrameRate[i].uiNumerator > 0 && vp9SeqParams->FrameRate[i].uiDenominator > 0);
            ENCODE_ASSERT(vp9SeqParams->TargetBitRate[i] > 0);

            // Check current layer properties are bigger than previous layer (since current includes previous layer properties)
            ENCODE_ASSERT(vp9SeqParams->FrameRate[i].uiNumerator / vp9SeqParams->FrameRate[i].uiDenominator >
                          vp9SeqParams->FrameRate[i - 1].uiNumerator / vp9SeqParams->FrameRate[i - 1].uiDenominator);
            ENCODE_ASSERT(vp9SeqParams->TargetBitRate[i] > vp9SeqParams->TargetBitRate[i - 1]);
        }
    }

    return eStatus;
}

MHW_SETPAR_DECL_SRC(HCP_VP9_PIC_STATE, Vp9EncodeBrc)
{
    ENCODE_FUNC_CALL();

    params.sseEnable = m_vdencBrcEnabled;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9EncodeBrc)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    params.presStreamOutBuffer = nullptr;

    if (!m_basicFeature->m_scalableMode)
    {
        params.presVdencStreamOutBuffer     = m_basicFeature->m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
        params.dwVdencStatsStreamOutOffset  = 0;
        params.presFrameStatStreamOutBuffer = const_cast<PMOS_RESOURCE>(&m_resFrameStatStreamOutBuffer);
        params.dwFrameStatStreamOutOffset   = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Vp9EncodeBrc)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    if (!m_basicFeature->m_scalableMode)
    {
        params.streamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
        params.streamOutOffset = 0;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
