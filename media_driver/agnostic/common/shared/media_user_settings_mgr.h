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
//! \file     media_user_settings_mgr.h
//! \brief    Common MOS util user feature key service across different platform
//!

#ifndef __MEDIA_USER_SETTINGS_MGR_H__
#define __MEDIA_USER_SETTINGS_MGR_H__

#include "mos_util_user_interface.h"

class MediaUserSettingsMgr
{
public:
    virtual MOS_STATUS Initialize() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Init Function for MOS utilities user interface
    //! \details  Initial MOS utilities user interface related structures, and only execute once for multiple entries
    //! \param    [in] platform
    //!           Pointer to current platform info. Will use default platform when it's nullptr
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS MediaUserSettingsInit(PRODUCT_FAMILY productFamily = IGFX_MAX_PRODUCT);

    //!
    //! \brief    Close Function for MOS util user interface
    //! \details  close/remove MOS utilities user interface related structures, and only execute once for multiple entries
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS MediaUserSettingClose();

    static bool      IsDefaultValueChanged()
    {
        return MosUtilUserInterface::IsDefaultValueChanged();
    }

    MediaUserSettingsMgr();
    virtual ~MediaUserSettingsMgr();

private:
    static MOS_STATUS Destroy();

private:
    static MosMutex m_userFeatureMutexLock;  // brief mutex for mos util user interface multi-threading protection
    static uint32_t m_userFeatureRefCount;   // If there are more than one VPHAL state created, need reference count to keep only do initialization once
    static MediaUserSettingsMgr *m_inst;
};

#endif  //__MEDIA_USER_SETTINGS_MGR_H__
