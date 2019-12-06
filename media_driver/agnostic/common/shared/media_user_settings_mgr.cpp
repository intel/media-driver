/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     media_user_settings_mgr.cpp
//! \brief    Common MOS util user feature key service across different platform
//!

#include "mos_os.h"
#include "media_user_settings_mgr.h"
#include "media_interfaces_mosutil.h"
#include "mos_utilities_common.h"

uint32_t            MediaUserSettingsMgr::m_userFeatureRefCount = 0;
MosMutex            MediaUserSettingsMgr::m_userFeatureMutexLock;
MediaUserSettingsMgr *MediaUserSettingsMgr::m_inst = nullptr;

MediaUserSettingsMgr::MediaUserSettingsMgr()
{
}

MediaUserSettingsMgr::~MediaUserSettingsMgr()
{
}

MOS_STATUS MediaUserSettingsMgr::Destroy()
{
    if (m_userFeatureRefCount > 0)
    {
        --m_userFeatureRefCount;
        if (m_userFeatureRefCount == 0 && m_inst)
        {
            MOS_Delete(m_inst);
            m_inst = nullptr;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaUserSettingsMgr::MediaUserSettingsInit(PRODUCT_FAMILY productFamily)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    // lock mutex to avoid multi init and close in multi-threading env
    m_userFeatureMutexLock.Lock();
    if (m_userFeatureRefCount == 0 && m_inst == nullptr)
    {
        m_inst = (MediaUserSettingsMgr *)MosUtilDevice::CreateFactory(productFamily);
        m_userFeatureRefCount++;
    }
    m_userFeatureMutexLock.Unlock();

    return eStatus;
}

MOS_STATUS MediaUserSettingsMgr::MediaUserSettingClose()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    m_userFeatureMutexLock.Lock();
    eStatus = Destroy();
    m_userFeatureMutexLock.Unlock();

    return eStatus;
}
