/*
* Copyright (c) 2022-2025, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_impl_xe3_lpm_base.h
//! \brief    MHW VDBOX VDENC interface common base for Xe3_LPM+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE3_LPM_BASE_H__
#define __MHW_VDBOX_VDENC_IMPL_XE3_LPM_BASE_H__

#include "mhw_vdbox_vdenc_impl.h"

#ifdef _MEDIA_RESERVED
#include "mhw_vdbox_vdenc_impl_xe3_lpm_base_ext.h"
#endif

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe3_lpm_base
{
template <typename cmd_t>
class BaseImpl : public vdenc::Impl<cmd_t>
{
public:
    virtual uint32_t GetCmd1CommandSize() override
    {
        return cmd_t::VDENC_CMD1_CMD::byteSize;
    }

    virtual uint32_t GetCmd2CommandSize() override
    {
        return cmd_t::VDENC_CMD2_CMD::byteSize;
    }

    MOS_STATUS SetRowstoreCachingOffsets(const RowStorePar &par) override
    {
        MHW_FUNCTION_ENTER;
        base_t::SetRowstoreCachingOffsets(par);
        
        switch (par.mode)
        {
        case RowStorePar::AV1: {
            if (this->m_rowStoreCache.vdenc.supported)
            {
                if (par.frameWidth > 2048)
                {
                    this->m_rowStoreCache.vdenc.enabled = false;
                }
            }
            break;
        }
        default: {
            break;
        }
        }
        
        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = vdenc::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe3_lpm_base__BaseImpl)
};
}  // namespace xe3_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE3_LPM_BASE_H__
