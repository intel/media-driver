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
//! \file        mos_utilities_usersetting.cpp
//! \brief       This module implements the MOS wrapper functions for Linux/Android
//!

#include "mos_utilities.h"
#if _MEDIA_RESERVED
#include "codechal_user_settings_mgr_ext.h"
#include "vphal_user_settings_mgr_ext.h"
#endif // _MEDIA_RESERVED

#if _MEDIA_RESERVED
MediaUserSettingsMgr *MosUtilities::m_codecUserFeatureExt = nullptr;
MediaUserSettingsMgr *MosUtilities::m_vpUserFeatureExt    = nullptr;
#endif

MOS_STATUS MosUtilities::MosDeclareUserFeature()
{
    MOS_STATUS eStatus = MosDeclareUserFeatureKeysForAllDescFields();
#if _MEDIA_RESERVED
        m_codecUserFeatureExt = new CodechalUserSettingsMgr();
        m_vpUserFeatureExt    = new VphalUserSettingsMgr();
#endif
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeature()
{
    MOS_STATUS eStatus = MosDestroyUserFeatureKeysForAllDescFields();
#if _MEDIA_RESERVED
    if (m_codecUserFeatureExt)
    {
        delete m_codecUserFeatureExt;
        m_codecUserFeatureExt = nullptr;
    }
    if (m_vpUserFeatureExt)
    {
        delete m_vpUserFeatureExt;
        m_vpUserFeatureExt = nullptr;
    }
#endif // _MEDIA_RESERVED

    return eStatus;
}

