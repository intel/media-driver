/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     mhw_vdbox_mfx_impl_xe3p_lpm_base.h
//! \brief    MHW VDBOX MFX interface common base for XE3P_LPM_Base
//! \details
//!

#ifndef __MHW_VDBOX_MFX_IMPL_XE3P_LPM_BASE_H__
#define __MHW_VDBOX_MFX_IMPL_XE3P_LPM_BASE_H__

#include "mhw_vdbox_mfx_impl.h"

#define AVC_VLF_ROWSTORE_BASEADDRESS_MBAFF 1280

namespace mhw
{
namespace vdbox
{
namespace mfx
{
namespace xe3p_lpm_base
{
#define mpeg2WeightScaleSize 16

template <typename cmd_t>
class BaseImpl : public mfx::Impl<cmd_t>
{
public:
    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isShortFormat) { return MOS_STATUS_UNIMPLEMENTED; }

    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isModeSpecific) { return MOS_STATUS_UNIMPLEMENTED; }

    MOS_STATUS GetRowstoreCachingAddrs(PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(rowstoreParams);
        mfx::Impl<cmd_t>::GetRowstoreCachingAddrs(rowstoreParams);

        // VLF rowstore cache update for AVC Mbaff on Xe3
        bool avc          = rowstoreParams->Mode == CODECHAL_DECODE_MODE_AVCVLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC;
        bool vp8          = rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP8VLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP8;
        bool widthLE4K    = rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K;
        bool mbaffOrField = rowstoreParams->bMbaff || !rowstoreParams->bIsFrame;

        Itf::m_deblockingFilterRowstoreCache.enabled = Itf::m_deblockingFilterRowstoreCache.supported && widthLE4K && (avc || vp8);
        if (Itf::m_deblockingFilterRowstoreCache.enabled)
        {
            Itf::m_deblockingFilterRowstoreCache.dwAddress = avc ? (mbaffOrField ? AVC_VLF_ROWSTORE_BASEADDRESS_MBAFF
                                                                            : AVC_VLF_ROWSTORE_BASEADDRESS)
                                                                            : VP8_VLF_ROWSTORE_BASEADDRESS;
        }
        else
        {
            Itf::m_deblockingFilterRowstoreCache.dwAddress = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = mfx::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__mfx__xe3p_lpm_base__BaseImpl)
};
}  // namespace xe3p_lpm_base
}  // namespace mfx
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_MFX_IMPL_XE3P_LPM_BASE_H__
