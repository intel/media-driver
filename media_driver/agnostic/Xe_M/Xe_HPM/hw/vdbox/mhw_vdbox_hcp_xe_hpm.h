/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_hcp_impl_legacy_xe_hpm.h
//! \brief    Defines functions for constructing Vdbox HCP commands on XE_HPM platform
//!

#ifndef __MHW_VDBOX_HCP_IMPL_LEGACY_XE_HPM_H__
#define __MHW_VDBOX_HCP_IMPL_LEGACY_XE_HPM_H__

#include "mhw_vdbox_hcp_impl_xe_hpm.h"
#include "mhw_vdbox_hcp_xe_xpm.h"

class MhwVdboxHcpInterfaceXe_Hpm : public MhwVdboxHcpInterfaceXe_Xpm
{
public:
    MhwVdboxHcpInterfaceXe_Hpm(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool            decodeInUse) : MhwVdboxHcpInterfaceXe_Xpm(osInterface, miInterface, cpInterface, decodeInUse) {}

    std::shared_ptr<void> GetNewHcpInterface() override
    {
        if (!m_hcpItfNew)
        {
            auto ptr = std::make_shared<mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Impl>(m_osInterface);
            ptr->SetCacheabilitySettings(m_cacheabilitySettings);
            m_hcpItfNew = ptr;
        }

        return m_hcpItfNew;
    }
    ~MhwVdboxHcpInterfaceXe_Hpm() {}
};

#endif  // __MHW_VDBOX_HCP_IMPL_LEGACY_XE_HPM_H__
