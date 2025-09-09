/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     encode_hevc_vdenc_height_padding.h
//! \brief    Defines for Xe3_LPM+ hevc height padding feature
//!
#ifndef __ENCODE_HEVC_VDENC_HEIGHT_PADDING_H__
#define __ENCODE_HEVC_VDENC_HEIGHT_PADDING_H__

#include "encode_allocator.h"
#include "encode_hevc_basic_feature.h"
#include "mhw_vdbox_aqm_itf.h"

namespace encode
{
class HevcVdencHeightPadding : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting, public mhw::vdbox::aqm::Itf::ParSetting
{
public:
    HevcVdencHeightPadding(
        MediaFeatureManager     *featureManager,
        EncodeAllocator         *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void                    *constSettings) : MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr), m_allocator(allocator)
    {
        ENCODE_FUNC_CALL();
    }

    ~HevcVdencHeightPadding() {}

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AQM_PIC_STATE);

protected:
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS m_hevcSeqParams = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  m_hevcPicParams = nullptr;
    uint32_t                           m_alignedHeight = 0;
    EncodeAllocator                   *m_allocator     = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__HevcVdencHeightPadding)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_VDENC_HEIGHT_PADDING_H__