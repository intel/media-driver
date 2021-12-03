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
//! \file     media_user_settings_mgr_g12_plus.cpp
//! \brief    Common user feature interface on Gen12 platform
//!

#include "mos_utilities.h"
#include "media_user_settings_mgr_g12_plus.h"

static MOS_USER_FEATURE_VALUE MOSUserFeatureValueDescFields_Xe_M_base[MOS_NUM_USER_FEATURE_VALUES_XE_M_BASE] =
{
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AV1_DECODE_DRIVER_S2L_ENABLE_ID_G12,
     "AV1 Decode Driver S2L Enable Flag",
     __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
     __MEDIA_USER_FEATURE_SUBKEY_REPORT,
     "Decode",
     MOS_USER_FEATURE_TYPE_USER,
     MOS_USER_FEATURE_VALUE_TYPE_INT32,
     "1",
     "AV1 Decode driver S2L enable flag. 0: HUC FW S2L, 1: Driver S2L."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MFE_FAST_SUBMIT_ID_G12,
    "Enable MFE Fast Submit",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "Codec",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_BOOL,
    "0",
    "Enable MFE fast submission. Default 0: disabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MFE_MULTISTREAM_SCHEDULER_ID_G12,
    "Enable MFE MultiStream Scheduler",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "Codec",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_BOOL,
    "0",
    "Enable MFE multistream scheduler. Default 0: disabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_FORCE_DELTA_QP_ENABLE_ID_G12,
    "HEVC VDEnc Force Delta QP Enable",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "Encode",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_INT32,
    "1",
    "Enable Force Delta QP for HEVC VDEnc"),
    /* VP gen12 ext */
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_VEBOX_TGNE_ENABLE_VP_ID_G12,
    "Enable Vebox GNE",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "VP",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_UINT32,
    "1",
    "Enable Vebox GNE. 1: Enable, 0: Disable."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MMC_ENC_POST_CDEF_RECON_COMPRESSIBLE_ID,
    "Encode Post CDEF Recon Compressible",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "Report",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_INT32,
    "0",
    "Report Key to indicate if the surface is MMCD capable (0: no; 1: yes)."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MMC_ENC_POST_CDEF_RECON_COMPRESSMODE_ID,
    "Encode Post CDEF Recon Compress Mode",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "Report",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_INT32,
    "0",
    "Report Key to indicate the MMCD compression mode of a surface "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_LOCKABLE_RESOURCE_ID,
    "Lockable Resource",
    __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
    __MEDIA_USER_FEATURE_SUBKEY_REPORT,
    "Codec",
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_VALUE_TYPE_INT32,
    "0",
    "Enables/Disables lockable resource."),
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_TILE_INFO_ID,
        "Tile Info",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Report tile infomation including tileID and memory region size."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_RCS_ID,
        "RCS Instance",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Report RCS instance infomation."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_VCS_ID,
        "VCS Instance",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Report VCS instance infomation."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_VECS_ID,
        "VECS Instance",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Report VECS instance infomation."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENGINE_INSTANCE_CCS_ID,
        "CCS Instance",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Report CCS instance infomation."),
#endif //(_DEBUG || _RELEASE_INTERNAL)
};

MediaUserSettingsMgr_Xe_M_base::MediaUserSettingsMgr_Xe_M_base()
{
    MOS_STATUS eStatus = MosUtilities::MosDeclareUserFeatureKeysFromDescFields(
        MOSUserFeatureValueDescFields_Xe_M_base,
        MOS_NUM_USER_FEATURE_VALUES_XE_M_BASE,
        __MOS_USER_FEATURE_KEY_G12_PLUS_MAX_ID);

    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("MOS util G12 user feature key init error");
    }
}

MediaUserSettingsMgr_Xe_M_base::~MediaUserSettingsMgr_Xe_M_base()
{
    MOS_STATUS eStatus = MosUtilities::MosDestroyUserFeatureKeysFromDescFields(
        MOSUserFeatureValueDescFields_Xe_M_base,
        MOS_NUM_USER_FEATURE_VALUES_XE_M_BASE,
        __MOS_USER_FEATURE_KEY_G12_PLUS_MAX_ID);

    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("MOS util G12 user feature key destroy error");
    }
}


