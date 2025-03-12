/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_feature_manager_xe3_lpm_base.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __VP_FEATURE_MANAGER_XE3_LPM_BASE_H__
#define __VP_FEATURE_MANAGER_XE3_LPM_BASE_H__

#include "vp_feature_manager.h"

namespace vp
{
// Base class for Xe3_LPM+
class VPFeatureManagerXe3_Lpm_Base : virtual public VPFeatureManager
{
public:
    //!
    //! \brief  VPFeatureManager deconstructor
    //!
    virtual ~VPFeatureManagerXe3_Lpm_Base() {}

    //!
    //! \brief  Check the conflict between features
    //! \param  [in] params
    //!         encode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckFeatures(void *params, bool &bapgFuncSupported);

protected:
    //!
    //! \brief  VPFeatureManager constructor
    //! \param  [in] hwInterface
    //!         Pointer to VP_MHWINTERFACE
    //!
    VPFeatureManagerXe3_Lpm_Base(PVP_MHWINTERFACE hwInterface);

    //!
    //! \brief  Check whether scalability needed.
    //! \param  [out] isScalabilityNeeded
    //!         true if scalability is needed, otherwise, scalability is not needed.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsScalabilityNeeded(bool &isScalabilityNeeded, uint32_t srcWidth, uint32_t srcHeight);

    virtual bool IsOutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

    virtual bool IsRGBOutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

    virtual bool IsNV12P010OutputFormatSupported(
        PVPHAL_SURFACE              outSurface);

MEDIA_CLASS_DEFINE_END(vp__VPFeatureManagerXe3_Lpm_Base)
};

}
#endif // !__VP_FEATURE_MANAGER_XE3_LPM_BASE_H__
