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
//! \file     encode_avc_vdenc_fullenc.h
//! \brief    Defines the common interface for avc vdenc fullenc encode features
//!
#ifndef __ENCODE_AVC_VDENC_FULLENC_H__
#define __ENCODE_AVC_VDENC_FULLENC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "encode_avc_basic_feature.h"
#include "encode_avc_vdenc_preenc.h"
#include "mhw_vdbox_vdenc_itf.h"
#if _MEDIA_RESERVED
#include "encode_avc_vdenc_fullenc_ext.h"
#endif // _MEDIA_RESERVED

namespace encode
{

class AvcVdencFullEnc : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    AvcVdencFullEnc(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    virtual ~AvcVdencFullEnc();

    //!
    //! \brief  Init full-enc basic features related parameter
    //!
    //! \param  [in] settings
    //!         Pointer to settings
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update full-enc features related parameter
    //!
    //! \param  [in] params
    //!         Pointer to parameters
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

protected:
    MOS_STATUS ecodeAvcFullencFuntion0();
    MOS_STATUS UpdateTrackedBufferParameters();
    MOS_STATUS UpdatePreEncSize();

    PMOS_INTERFACE   m_osInterface  = nullptr;  //!< Os Inteface
    AvcBasicFeature *m_basicFeature = nullptr;  //!< EncodeBasicFeature
    EncodeAllocator *m_allocator    = nullptr;
    AvcVdencPreEnc *m_preEncFeature = nullptr;

    PMOS_RESOURCE ecodeAvcFullencMember0 = nullptr;
    PMOS_RESOURCE ecodeAvcFullencMember1 = nullptr;

    uint8_t  ecodeAvcFullencMember2 = 0;
    uint8_t  ecodeAvcFullencMember3 = 0;
    uint8_t  ecodeAvcFullencMember4 = 0;
    uint32_t ecodeAvcFullencMember5 = 0;
    uint8_t  ecodeAvcFullencMember6 = 0;
    uint16_t ecodeAvcFullencMember7 = 0;
    uint16_t ecodeAvcFullencMember8 = 0;
    FILE *   m_pfile0               = nullptr;
    FILE *   m_pfile1               = nullptr;

    std::string ecodeAvcFullencMember9  = "";
    std::string ecodeAvcFullencMember10 = "";

    uint8_t ecodeAvcFullencMember11 = 0;

    uint32_t m_encodeMode = 0;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencFullEnc)
};
}  // namespace encode

#endif  // !__ENCODE_AVC_VDENC_FULLENC_H__
