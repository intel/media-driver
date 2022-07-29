/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     mos_os_mock_adaptor_specific.h
//! \brief    Common interface and structure used in mock adaptor.

#ifndef __MOS_OS_MOCK_ADAPTOR_SPECIFIC_H__
#define __MOS_OS_MOCK_ADAPTOR_SPECIFIC_H__

#include "mos_os_mock_adaptor.h"
#include "linux_system_info.h"
#include "linux_shadow_skuwa.h"

class MosMockAdaptorSpecific: public MosMockAdaptor
{
public:
    MosMockAdaptorSpecific();

    virtual ~MosMockAdaptorSpecific();
protected:
    //!
    //! \brief    Initialize SKU/WA table for device context.
    //! \details  Initialize the SKU/WA table for device context by mocking.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitializeSkuWaTable(PMOS_CONTEXT context) override;

private:
    //!
    //! \brief    Get device infomation
    //! \details  Get device information according to device ID.
    //! \return   GfxDeviceInfo
    //!           Pointer of GfxDeviceInfo.
    //!
    GfxDeviceInfo *GetDeviceInfo(uint32_t devId);

    //!
    //! \brief    Get device initialization information
    //! \details  Get device initialization information according to platform key.
    //! \return   LinuxDeviceInit
    //!           Pointer of LinuxDeviceInit.
    //!
    LinuxDeviceInit *GetDeviceInit(uint32_t platKey);

    //!
    //! \brief    Update User FeatureKey
    //! \details  Update User FeatureKey to report platform info.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateUserFeatureKey(
        PMOS_CONTEXT osContext) override;

MEDIA_CLASS_DEFINE_END(MosMockAdaptorSpecific)
};
#endif