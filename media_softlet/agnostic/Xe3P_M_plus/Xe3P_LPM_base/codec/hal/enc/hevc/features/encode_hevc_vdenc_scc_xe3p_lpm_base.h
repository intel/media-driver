/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_hevc_vdenc_scc_xe3p_lpm_base.h
//! \brief    Defines for Xe3P_LPM_Base hevc screen content coding feature
//!
#ifndef __ENCODE_HEVC_VDENC_SCC_XE3P_LPM_BASE_H__
#define __ENCODE_HEVC_VDENC_SCC_XE3P_LPM_BASE_H__

#include "encode_hevc_vdenc_scc.h"

namespace encode
{
class HevcVdencSccXe3P_Lpm_Base : public HevcVdencScc
{
public:
    HevcVdencSccXe3P_Lpm_Base(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    ~HevcVdencSccXe3P_Lpm_Base() {}

    bool IsCompressFlagNeeded() override;

    //!
    //! \brief  Ensure m_slotForRecNotFiltered is up-to-date for the current frame.
    //! \details Called by HEVCHucSLBBUpdatePkt before ConstructBatchBuffer() to fix
    //!          the one-frame lag: SLBB construction runs before HCP_SURFACE_STATE
    //!          SETPAR which normally updates the slot, so HCP_PIC_STATE and VDENC_CMD2
    //!          would otherwise use the stale value from the previous frame.
    //! \return MOS_STATUS
    //!
    MOS_STATUS UpdateSlotForRecNotFiltered();

MEDIA_CLASS_DEFINE_END(encode__HevcVdencSccXe3P_Lpm_Base)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_VDENC_SCC_XE3P_LPM_BASE_H__
