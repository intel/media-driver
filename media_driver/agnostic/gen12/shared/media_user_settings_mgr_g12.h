/*
* Copyright (c) 2011-2019, Intel Corporation
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
//! \file     media_user_settings_mgr_g12.h
//! \brief    Common user feature interface on Gen12 platform
//!
#ifndef __MEDIA_USER_SETTINGS_MGR_G12_H__
#define __MEDIA_USER_SETTINGS_MGR_G12_H__

#include "media_user_settings_mgr.h"

class MediaUserSettingsMgr_g12 : public MediaUserSettingsMgr
{
public:
    MediaUserSettingsMgr_g12();
    virtual ~MediaUserSettingsMgr_g12();
};

//!
//! \brief User Feature Value IDs
//!
typedef enum _MOS_USER_FEATURE_VALUE_ID_G12
{
    __MOS_USER_FEATURE_KEY_INVALID_ID_G12 = __MOS_USER_FEATURE_KEY_MAX_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_SUBTHREAD_NUM_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_LOAD_KERNEL_INPUT_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VME_BRC_LTR_ENABLE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_FORCE_SCALABILITY_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_TILEREPLAY_ENABLE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_RGB_ENCODING_ENABLE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_CAPTURE_MODE_ENABLE_ID_G12,
    /* codec gen12 based */
    __MEDIA_USER_FEATURE_VALUE_VDENC_ULTRA_MODE_ENABLE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VME_DISABLE_PANIC_MODE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VME_NUM_MEDIA_HWWALKER_INUSE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VME_BREAK12_ID_G12,
    /* codec g12hp based */
    __MEDIA_USER_FEATURE_VALUE_HEVC_VME_ENABLE_RENDER_CONTEXT_ID_G12HP,
    /* VP gen12 based */
    __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_CENTERING_DISABLE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE_ID_G12,
    __MOS_USER_FEATURE_KEY_G12_MAX_ID,
} MOS_USER_FEATURE_VALUE_ID_G12;

#define MOS_NUM_USER_FEATURE_VALUES_G12 (__MOS_USER_FEATURE_KEY_G12_MAX_ID - __MOS_USER_FEATURE_KEY_INVALID_ID_G12 - 1)

#endif // __MEDIA_USER_SETTINGS_MGR_G12_H__
