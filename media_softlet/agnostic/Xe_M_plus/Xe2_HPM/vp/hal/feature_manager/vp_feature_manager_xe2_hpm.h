/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_feature_manager_xe2_hpm.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __VP_FEATURE_MANAGER_XE2_HPM_H__
#define __VP_FEATURE_MANAGER_XE2_HPM_H__

#include "vp_feature_manager_xe_lpm_plus_base.h"

namespace vp
{
class VPFeatureManagerXe2_Hpm : public VPFeatureManagerXe_Lpm_Plus_Base
{
public:
    //!
    //! \brief  VPFeatureManager constructor
    //! \param  [in] hwInterface
    //!         Pointer to VP_MHWINTERFACE
    //!
     VPFeatureManagerXe2_Hpm(PVP_MHWINTERFACE hwInterface);

    //!
    //! \brief  VPFeatureManager deconstructor
    //!
    virtual ~VPFeatureManagerXe2_Hpm() {}

MEDIA_CLASS_DEFINE_END(vp__VPFeatureManagerXe2_Hpm)
};

}
#endif // !__VP_FEATURE_MANAGER_XE2_HPM_H__
