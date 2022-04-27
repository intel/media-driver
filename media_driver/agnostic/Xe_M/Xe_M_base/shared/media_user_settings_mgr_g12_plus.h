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
//! \file     media_user_settings_mgr_g12_plus.h
//! \brief    Common user feature interface on Gen12 platform
//!
#ifndef __media_user_settings_mgr_g12_plus_H__
#define __media_user_settings_mgr_g12_plus_H__

#include "media_user_settings_mgr_g12.h"

class MediaUserSettingsMgr_Xe_M_base : public MediaUserSettingsMgr_g12
{
public:
    MediaUserSettingsMgr_Xe_M_base();
    virtual ~MediaUserSettingsMgr_Xe_M_base();
};

//!
//! \brief User Feature Value IDs
//!
typedef enum _MOS_USER_FEATURE_VALUE_ID_G12_PLUS
{
    __MOS_USER_FEATURE_KEY_INVALID_ID_G12_PLUS = __MOS_USER_FEATURE_KEY_G12_MAX_ID,
    __MEDIA_USER_FEATURE_VALUE_AV1_DECODE_DRIVER_S2L_ENABLE_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_MFE_FAST_SUBMIT_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_MFE_MULTISTREAM_SCHEDULER_ID_G12,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_FORCE_DELTA_QP_ENABLE_ID_G12,
    /* VP gen12 ext */
    __MEDIA_USER_FEATURE_VALUE_MMC_ENC_POST_CDEF_RECON_COMPRESSIBLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMC_ENC_POST_CDEF_RECON_COMPRESSMODE_ID,
    __MEDIA_USER_FEATURE_VALUE_LOCKABLE_RESOURCE_ID,
#if (_DEBUG || _RELEASE_INTERNAL)
    __MEDIA_USER_FEATURE_VALUE_TILE_INFO_ID,
    __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_RCS_ID,
    __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_VCS_ID,
    __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_VECS_ID,
    __MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_CCS_ID,
#endif //(_DEBUG || _RELEASE_INTERNAL)
    __MOS_USER_FEATURE_KEY_G12_PLUS_MAX_ID,
} MOS_USER_FEATURE_VALUE_ID_G12_PLUS;

#define MOS_NUM_USER_FEATURE_VALUES_XE_M_BASE (__MOS_USER_FEATURE_KEY_G12_PLUS_MAX_ID - __MOS_USER_FEATURE_KEY_INVALID_ID_G12_PLUS - 1) 

#endif // __media_user_settings_mgr_g12_plus_H__
