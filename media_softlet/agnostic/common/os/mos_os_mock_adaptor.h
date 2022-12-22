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
//! \file     mos_os_mock_adaptor.h
//! \brief    Common interface and structure used in mock adaptor.
//!

#ifndef __MOS_OS_MOCK_ADAPTOR_H__
#define __MOS_OS_MOCK_ADAPTOR_H__

#include "mos_os.h"
#include "mos_os_specific.h"

#define MOS_MOCKADAPTOR_DEFAULT_PLATFORM                "tgllp"
#define MOS_MOCKADAPTOR_DEFAULT_STEPPING                "a0"

class MosMockAdaptor
{
public:
    MosMockAdaptor();

    virtual ~MosMockAdaptor();

    //!
    //! \brief    Interface for initializing Mock Adaptor.
    //! \details  Interface for initializing Mock Adaptor.
    //! \param    [in/out] osContext
    //!           Pointer to OS context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS Init(
        MOS_CONTEXT_HANDLE osDriverContext,
        OsContextNext      *osDeviceContext);

    //!
    //! \brief    Destroy MockAdaptor.
    //! \details  Destroy MockAdaptor.
    //! \return   void
    //!
    static MOS_STATUS Destroy();

protected:
    //!
    //! \brief    Initialize MockAdaptor and replace platform information for device context.
    //! \details  Initialize according to regkey and replace platform infomation for device context.
    //! \param    [in] osContext
    //!           Pointer to Os Context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS Initialize(
        PMOS_CONTEXT osContext);

    //!
    //! \brief    Read corresponding regkey for mockadaptor
    //! \details  Read corresponding regkey for mockadaptor.
    //! \param    [in] osContext
    //!           Pointer to Os Context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS RegkeyRead(PMOS_CONTEXT osContext);

    //!
    //! \brief    Initialize PlatForm.
    //! \details  Initialize PlatForm according to the regkey.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS InitializePlatForm();

    //!
    //! \brief    Initialize SKU/WA table for device context.
    //! \details  Initialize the SKU/WA table for device context by mocking.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitializeSkuWaTable(PMOS_CONTEXT context)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Replace platform infomation for device context.
    //! \details  Replace platform infomation for device context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS ReplacePlatformInfo(PMOS_CONTEXT context)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Initialize device context.
    //! \details  Initialize the device context according the regkey.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitContext(
        PMOS_CONTEXT osContext);

    //!
    //! \brief    Update User FeatureKey
    //! \details  Update User FeatureKey to report platform info.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateUserFeatureKey(
        PMOS_CONTEXT osContext)
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    PLATFORM                *m_pPlatform     = nullptr;
    MEDIA_WA_TABLE          *m_pWaTable      = nullptr;
    MEDIA_FEATURE_TABLE     *m_pSkuTable     = nullptr;
    MEDIA_SYSTEM_INFO       *m_pGtSystemInfo = nullptr;


    static bool  m_enabled;
    static PRODUCT_FAMILY m_productFamily;
    static std::string m_stepping;
    static uint16_t m_deviceId;
    static MosMockAdaptor *m_mocAdaptor;

MEDIA_CLASS_DEFINE_END(MosMockAdaptor)
};

#endif // __MOS_OS_MOCK_ADAPTOR_H__