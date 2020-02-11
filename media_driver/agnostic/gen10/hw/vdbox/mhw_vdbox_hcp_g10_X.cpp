/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mhw_vdbox_hcp_g10_X.cpp
//! \brief    Constructs VdBox HCP commands on Gen10-based platforms

#include "mhw_vdbox_hcp_g10_X.h"
#include "mhw_mi_hwcmd_g10_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g10_X.h"
#include "mhw_mmio_g10.h"

static uint16_t RDOQLamdas8bits[2][2][2][52] = //[Intra Slice/Inter Slice][Intra/Inter][Luma/Chroma][QP]
{
    {
        {
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
        },
        {
            {   //Inter Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        },
    },
    {
        {
            {   //Intra Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        },
        {
            {   //Inter Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        },
    }
};

static uint16_t RDOQLamdas10bits[2][2][2][64] = //[Intra Slice/Inter Slice][Intra/Inter][Luma/Chroma][QP]
{
    {
        {
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
                0x0003, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000a, 0x000d,
                0x0011, 0x0015, 0x001a, 0x0021, 0x002a, 0x0034, 0x0042, 0x0053,
                0x0069, 0x0084, 0x00a6, 0x00d2, 0x0108, 0x014d, 0x01a3, 0x0210,
                0x029a, 0x0347, 0x0421, 0x0533, 0x068d, 0x0841, 0x0a66, 0x0d1a,
                0x1082, 0x14cd, 0x1a35, 0x2105, 0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
                0x0003, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000a, 0x000d,
                0x0011, 0x0015, 0x001a, 0x0021, 0x002a, 0x0034, 0x0042, 0x0053,
                0x0069, 0x0084, 0x00a6, 0x00d2, 0x0108, 0x014d, 0x01a3, 0x0210,
                0x029a, 0x0347, 0x0421, 0x0533, 0x068d, 0x0841, 0x0a66, 0x0d1a,
                0x1082, 0x14cd, 0x1a35, 0x2105, 0x299a, 0x346a, 0x4209, 0x5333
            },
        },
        {
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        },
    },
    {
        {
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        },
        {
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        },
    }
};

void MhwVdboxHcpInterfaceG10::InitMmioRegisters()
{
    MmioRegistersHcp *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

    mmioRegisters->watchdogCountCtrlOffset                           = WATCHDOG_COUNT_CTRL_OFFSET_INIT_G10;
    mmioRegisters->watchdogCountThresholdOffset                      = WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT_G10;
    mmioRegisters->hcpDebugFEStreamOutSizeRegOffset                  = HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncImageStatusMaskRegOffset                    = HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncImageStatusCtrlRegOffset                    = HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset            = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset           = HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset    = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncQpStatusCountRegOffset                      = HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncSliceCountRegOffset                         = HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpEncVdencModeTimerRegOffset                     = HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset         = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpVp9EncBitstreamBytecountFrameNoHeaderRegOffset = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpVp9EncImageStatusMaskRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G10;
    mmioRegisters->csEngineIdOffset                                  = CS_ENGINE_ID_OFFSET_INIT_G10;
    mmioRegisters->hcpDecStatusRegOffset                             = HCP_DEC_STATUS_REG_OFFSET_INIT_G10;
    mmioRegisters->hcpCabacStatusRegOffset                           = HCP_CABAC_STATUS_REG_OFFSET_INIT_G10;

}

void MhwVdboxHcpInterfaceG10::InitRowstoreUserFeatureSettings()
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data = 0;

    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
        &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
    m_rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

    if (m_rowstoreCachingSupported)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVCDATROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_hevcDatRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVCDFROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_hevcDfRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVCSAOROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_hevcSaoRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP9_HVDROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_vp9HvdRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP9_DFROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_vp9DfRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;
    }
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetRowstoreCachingAddrs(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(rowstoreParams);

    if (m_hevcDatRowStoreCache.bSupported && (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD))
    {
        m_hevcDatRowStoreCache.bEnabled = true;
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_8K)
        {
            m_hevcDatRowStoreCache.dwAddress = HEVCDATROWSTORE_BASEADDRESS;
        }
        else
        {
            m_hevcDatRowStoreCache.dwAddress = 0;
            m_hevcDatRowStoreCache.bEnabled = false;
        }
    }

    if (m_hevcDfRowStoreCache.bSupported && (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD))
    {
        m_hevcDfRowStoreCache.bEnabled = true;
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K)
        {
            m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
        }
        else if ((rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_2K) && (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K))
        {
            if (rowstoreParams->ucBitDepthMinus8 == 0)
            {
                m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_4K;
            }
            else
            {
                m_hevcDfRowStoreCache.dwAddress = 0;
                m_hevcDfRowStoreCache.bEnabled = false;
            }
        }
        else
        {
            m_hevcDfRowStoreCache.dwAddress = 0;
            m_hevcDfRowStoreCache.bEnabled = false;
        }
    }

    if (m_hevcSaoRowStoreCache.bSupported && (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD))
    {
        m_hevcSaoRowStoreCache.bEnabled = true;
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K)
        {
            if (rowstoreParams->ucBitDepthMinus8 == 0)
            {
                m_hevcSaoRowStoreCache.dwAddress = HEVCSAOROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
            }
            else
            {
                m_hevcSaoRowStoreCache.dwAddress = 0;
                m_hevcSaoRowStoreCache.bEnabled = false;
            }
        }
        else
        {
            m_hevcSaoRowStoreCache.dwAddress = 0;
            m_hevcSaoRowStoreCache.bEnabled = false;
        }
    }

    if (m_vp9HvdRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP9VLD)
    {
        m_vp9HvdRowStoreCache.bEnabled = true;
        if ((rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_8K && rowstoreParams->ucBitDepthMinus8 == 0) ||
            (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K && rowstoreParams->ucBitDepthMinus8 == 2))
        {
            m_vp9HvdRowStoreCache.dwAddress = VP9HVDROWSTORE_BASEADDRESS;
        }
        else
        {
            m_vp9HvdRowStoreCache.dwAddress = 0;
            m_vp9HvdRowStoreCache.bEnabled = false;
        }
    }

    if (m_vp9DfRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP9VLD)
    {
        m_vp9DfRowStoreCache.bEnabled = true;
        if ((rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K && rowstoreParams->ucBitDepthMinus8 == 0) ||
            (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1K && rowstoreParams->ucBitDepthMinus8 == 2))
        {
            m_vp9DfRowStoreCache.dwAddress = VP9DFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
        }
        else
        {
            m_vp9DfRowStoreCache.dwAddress = 0;
            m_vp9DfRowStoreCache.bEnabled = false;
        }
    }

    if (m_hevcDatRowStoreCache.bSupported &&
        rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC)
    {
        m_hevcDatRowStoreCache.bEnabled = true;
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_8K)
        {
            m_hevcDatRowStoreCache.dwAddress = HEVCDATROWSTORE_BASEADDRESS;
        }
        else
        {
            m_hevcDatRowStoreCache.dwAddress = 0;
            m_hevcDatRowStoreCache.bEnabled = false;
        }
    }

    if (m_hevcDfRowStoreCache.bSupported &&
        rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC)
    {
        m_hevcDfRowStoreCache.bEnabled = true;
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K)
        {
            m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
        }
        else if ((rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_2K) && (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K))
        {
            if (rowstoreParams->ucBitDepthMinus8 == 0)
            {
                m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_4K;
            }
            else
            {
                m_hevcDfRowStoreCache.dwAddress = 0;
                m_hevcDfRowStoreCache.bEnabled = false;
            }
        }
        else
        {
            m_hevcDfRowStoreCache.dwAddress = 0;
            m_hevcDfRowStoreCache.bEnabled = false;
        }
    }

    if (m_hevcSaoRowStoreCache.bSupported &&
        rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC  && rowstoreParams->ucBitDepthMinus8 == 0)
    {
        m_hevcSaoRowStoreCache.bEnabled = true;
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K)
        {
            m_hevcSaoRowStoreCache.dwAddress = HEVCSAOROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
        }
        else
        {
            m_hevcSaoRowStoreCache.dwAddress = 0;
            m_hevcSaoRowStoreCache.bEnabled = false;
        }
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetHcpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;
    uint32_t            standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_HEVC)
    {
        maxSize =
            mhw_vdbox_vdenc_g10_X::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g10_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw_mi_g10_X::MI_LOAD_REGISTER_REG_CMD::byteSize * 8;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD);

        if (mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            /* HCP_QM_STATE_CMD may be issued up to 20 times: 3x Colour Component plus 2x intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components.
            HCP_FQP_STATE_CMD may be issued up to 8 times: 4 scaling list per intra and inter. */
            maxSize +=
                mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD::byteSize + // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
                20 * mhw_vdbox_hcp_g10_X::HCP_QM_STATE_CMD::byteSize +
                8 * mhw_vdbox_hcp_g10_X::HCP_FQM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HEVC_VP9_RDOQ_STATE_CMD::byteSize + // RDOQ
                2 * mhw_mi_g10_X::MI_STORE_DATA_IMM_CMD::byteSize + // Slice level commands
                2 * mhw_mi_g10_X::MI_FLUSH_DW_CMD::byteSize + // need for Status report, Mfc Status and
                10 * mhw_mi_g10_X::MI_STORE_REGISTER_MEM_CMD::byteSize + // 8 for BRCStatistics and 2 for RC6 WAs
                mhw_mi_g10_X::MI_LOAD_REGISTER_MEM_CMD::byteSize + // 1 for RC6
                2 * mhw_vdbox_hcp_g10_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize + // Two PAK insert object commands are for headers before the slice header and the header for the end of stream
                8 * mhw_mi_g10_X::MI_COPY_MEM_MEM_CMD::byteSize; // Need to copy SSE statistics/ Slice Size overflow into memory

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                8 * PATCH_LIST_COMMAND(HCP_FQM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) + // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
                2 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) + // Slice level commands
                2 * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) + // need for Status report, Mfc Status and
                11 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) +// 8 for BRCStatistics and 3 for RC6 WAs
                8 * PATCH_LIST_COMMAND(MI_COPY_MEM_MEM_CMD); // Need to copy SSE statistics/ Slice Size overflow into memory
        }
        else
        {
            maxSize +=
                20 * mhw_vdbox_hcp_g10_X::HCP_QM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_TILE_STATE_CMD::byteSize;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_TILE_STATE_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)     // VP9 Clear Decode
    {
        maxSize =
            mhw_vdbox_vdenc_g10_X::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g10_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD::byteSize * 4 +
            mhw_vdbox_hcp_g10_X::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
            mhw_vdbox_hcp_g10_X::HCP_VP9_PIC_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g10_X::HCP_BSD_OBJECT_CMD::byteSize +
            mhw_mi_g10_X::MI_LOAD_REGISTER_REG_CMD::byteSize * 8;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) * 4 +
            PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
            PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);

        if (mode == CODECHAL_ENCODE_MODE_VP9)
        {
            maxSize +=
                mhw_mi_g10_X::MI_FLUSH_DW_CMD::byteSize * 2 +
                mhw_mi_g10_X::MI_STORE_DATA_IMM_CMD::byteSize * 2 +
                mhw_mi_g10_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 3 +
                mhw_mi_g10_X::MI_COPY_MEM_MEM_CMD::byteSize * 4 +
                mhw_mi_g10_X::MI_BATCH_BUFFER_START_CMD::byteSize * 3;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2 +
                PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) * 2 +
                PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) * 3 +
                PATCH_LIST_COMMAND(MI_COPY_MEM_MEM_CMD) * 4 +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) * 3;
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetHcpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t            standard = CodecHal_GetStandardFromMode(mode);
    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;

    if (standard == CODECHAL_HEVC)
    {
        if (mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            maxSize =
                2 * mhw_vdbox_hcp_g10_X::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g10_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
                2 * mhw_mi_g10_X::MI_BATCH_BUFFER_START_CMD::byteSize;

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PAK_INSERT_OBJECT_CMD) +
                2 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD); // One is for the PAK command and another one is for the BB when BRC and single task mode are on
        }
        else
        {
            maxSize =
                2 * mhw_vdbox_hcp_g10_X::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g10_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)      // VP9 Clear decode does not require primitive level commands. VP9 DRM does.
    {
        if (modeSpecific)                  // VP9 DRM
        {
            maxSize +=
                mhw_vdbox_hcp_g10_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw_vdbox_hcp_g10_X::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g10_X::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetHevcBufferSize(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpBufSizeParam);

    uint32_t mvtSize = 0;
    uint32_t mvtbSize = 0;
    uint32_t bufferSize = 0;
    uint8_t  maxBitDepth = hcpBufSizeParam->ucMaxBitDepth;
    uint32_t picWidth = hcpBufSizeParam->dwPicWidth;
    uint32_t picHeight = hcpBufSizeParam->dwPicHeight;

    uint32_t picHeightLCU = picHeight >> 4;//assume smallest LCU to get max height
    uint32_t picWidthLCU = picWidth >> 4;
    uint8_t  shiftParam = (maxBitDepth == 10) ? 2 : 3;

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
        bufferSize = ((picWidth + 31) & 0xFFFFFFE0) >> shiftParam;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        bufferSize = ((picHeight + picHeightLCU * 6 + 31) & 0xFFFFFFE0) >> shiftParam;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
        bufferSize = (((((picWidth + 15) >> 4) * 188 + picWidthLCU * 9 + 1023) >> 9) + 1) & (-2);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        bufferSize = (((((picWidth + 15) >> 4) * 172 + picWidthLCU * 9 + 1023) >> 9) + 1) & (-2);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        bufferSize = (((((picHeight + 15) >> 4) * 176 + picHeightLCU * 89 + 1023) >> 9) + 1) & (-2);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE:
        bufferSize = (((((picWidth >> 1) + picWidthLCU * 3) + 15) & 0xFFFFFFF0) >> shiftParam);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE:
        bufferSize = (((((picWidth >> 1) + picWidthLCU * 6) + 15) & 0xFFFFFFF0) >> shiftParam);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL:
        bufferSize = (((((picHeight >> 1) + picHeightLCU * 6) + 15) & 0xFFFFFFF0) >> shiftParam);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
        mvtSize = ((((picWidth + 63) >> 6)*(((picHeight + 15) >> 4)) + 1)&(-2));
        mvtbSize = ((((picWidth + 31) >> 5)*(((picHeight + 31) >> 5)) + 1)&(-2));
        bufferSize = MOS_MAX(mvtSize, mvtbSize);
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }
    hcpBufSizeParam->dwBufferSize = bufferSize * MHW_CACHELINE_SIZE;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetVp9BufferSize(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpBufSizeParam);

    uint32_t bufferSize = 0;
    uint32_t dblkRsbSizeMultiplier = 0;
    uint32_t dblkCsbSizeMultiplier = 0;

    uint8_t maxBitDepth = hcpBufSizeParam->ucMaxBitDepth;
    uint32_t widthInSb = hcpBufSizeParam->dwPicWidth;
    uint32_t heightInSb = hcpBufSizeParam->dwPicHeight;
    HCP_CHROMA_FORMAT_IDC chromaFormat = (HCP_CHROMA_FORMAT_IDC)hcpBufSizeParam->ucChromaFormat;
    uint32_t sizeScale = (maxBitDepth > 8) ? 2 : 1;

    if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
    {
        dblkRsbSizeMultiplier = sizeScale * 18;
        dblkCsbSizeMultiplier = sizeScale * 17;
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("Format not supported.");
        return eStatus;
    }

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
        bufferSize = widthInSb * dblkRsbSizeMultiplier;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        bufferSize = heightInSb * dblkCsbSizeMultiplier;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        bufferSize = widthInSb * 5;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        bufferSize = heightInSb * 5;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL:
        bufferSize = widthInSb * heightInSb * 9;
        break;
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID:
        bufferSize = widthInSb * heightInSb;
        break;
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE:
        bufferSize = widthInSb;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    hcpBufSizeParam->dwBufferSize = bufferSize * MHW_CACHELINE_SIZE;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::IsHevcBufferReallocNeeded(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(reallocParam);

    bool realloc = false;
    uint32_t picWidth = reallocParam->dwPicWidth;
    uint32_t picHeight = reallocParam->dwPicHeight;
    uint32_t picWidthAlloced = reallocParam->dwPicWidthAlloced;
    uint32_t picHeightAlloced = reallocParam->dwPicHeightAlloced;

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE:
        realloc = (picWidth > picWidthAlloced) ? true : false;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL:
        realloc = (picHeight > picHeightAlloced) ? true : false;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
        realloc = (picWidth > picWidthAlloced || picHeight > picHeightAlloced) ? true : false;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }
    reallocParam->bNeedBiggerSize = realloc;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::IsVp9BufferReallocNeeded(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(reallocParam);

    bool       realloc = false;
    uint32_t widthInSb = reallocParam->dwPicWidth;
    uint32_t heightInSb = reallocParam->dwPicHeight;
    uint32_t picWidthInSbAlloced = reallocParam->dwPicWidthAlloced;
    uint32_t picHeightInSbAlloced = reallocParam->dwPicHeightAlloced;

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE:
        realloc = (widthInSb > picWidthInSbAlloced) ? true : false;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        realloc = (heightInSb > picHeightInSbAlloced) ? true : false;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID:
        realloc = (heightInSb > picHeightInSbAlloced || widthInSb > picWidthInSbAlloced) ? true : false;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    reallocParam->bNeedBiggerSize = realloc;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g10_X::HCP_PIPE_MODE_SELECT_CMD   cmd;
    PMHW_BATCH_BUFFER                               batchBuffer = nullptr;

    if (params->bBatchBufferInUse)
    {
        MHW_MI_CHK_NULL(params->pBatchBuffer);
        batchBuffer            = params->pBatchBuffer;
    }

    MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForMfxPipeModeSelect((uint32_t *)&cmd));

    cmd.DW1.AdvancedRateControlEnable    = params->bAdvancedRateControlEnable;
    cmd.DW1.SaoFirstPass                 = params->bSaoFirstPass;
    cmd.DW1.CodecStandardSelect          = CodecHal_GetStandardFromMode(params->Mode) - CODECHAL_HCP_BASE;
    cmd.DW1.PakPipelineStreamoutEnable   = params->bStreamOutEnabled;
    cmd.DW1.DeblockerStreamoutEnable     = params->bDeblockerStreamOutEnable;
    cmd.DW1.VdencMode                    = params->bVdencEnabled;
    cmd.DW1.RdoqEnabledFlag              = params->bRdoqEnable;
    // This bit is valid if global bit PAK Pipeline Streamout Enable is set to 1.
    // In VDENC mode, this field should always be 1.
    cmd.DW1.PakFrameLevelStreamoutEnable = params->bStreamOutEnabled;

    if (m_decodeInUse)
    {
        cmd.DW1.CodecSelect = cmd.CODEC_SELECT_DECODE;
    }
    else
    {
        cmd.DW1.CodecSelect = cmd.CODEC_SELECT_ENCODE;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g10_X>::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_PLANAR_420_8;
    }
    else
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_P010;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpEncodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g10_X::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g10_X>::AddHcpEncodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_PLANAR4208;
    }
    else if ((params->ucSurfaceStateId == CODECHAL_HCP_DECODED_SURFACE_ID) &&
        (params->Mode == CODECHAL_ENCODE_MODE_HEVC))
    {
        cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010VARIANT;
    }
    else
    {
        cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params)
{
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS                              resourceParams;
    MOS_SURFACE                                      details;
    mhw_vdbox_hcp_g10_X::HCP_PIPE_BUF_ADDR_STATE_CMD cmd;

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));

    // 1. MHW_VDBOX_HCP_GENERAL_STATE_SHIFT(6) may not work with DecodedPicture
    // since it needs to be 4k aligned
    resourceParams.dwLsbNum = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    //Decoded Picture
    cmd.DecodedPictureMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;

    cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable =
        (params->PreDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
    cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode =
        (params->PreDeblockSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

    cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psPreDeblockSurface->TileType);

    // For HEVC 8bit/10bit mixed case, register App's RenderTarget for specific use case
    if (params->presP010RTSurface != nullptr)
    {
        resourceParams.presResource = &(params->presP010RTSurface->OsResource);
        resourceParams.dwOffset = params->presP010RTSurface->dwOffset;
        resourceParams.pdwCmd = cmd.DecodedPicture.DW0_1.Value;
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;
        
        if (m_osInterface->bAllowExtraPatchToSameLoc)
        {
            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        else //if not allowed to patch another OsResource to same location in cmd, just register resource here
        {
            MHW_MI_CHK_STATUS(m_osInterface->pfnRegisterResource(
                m_osInterface,
                resourceParams.presResource,
                resourceParams.bIsWritable,
                resourceParams.bIsWritable));
        }
    }

    resourceParams.presResource = &(params->psPreDeblockSurface->OsResource);
    resourceParams.dwOffset = params->psPreDeblockSurface->dwOffset;
    resourceParams.pdwCmd = cmd.DecodedPicture.DW0_1.Value;
    resourceParams.dwLocationInCmd = 1;
    resourceParams.bIsWritable = true;

    MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->PreDeblockSurfMmcState));
    // Deblocking Filter Line Buffer
    if (m_hevcDfRowStoreCache.bEnabled)
    {
        cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockingFilterLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = m_hevcDfRowStoreCache.dwAddress;
    }
    else if (m_vp9DfRowStoreCache.bEnabled)
    {
        cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockingFilterLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = m_vp9DfRowStoreCache.dwAddress;
    }
    else if (params->presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
    {
        cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presMfdDeblockingFilterRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.DeblockingFilterLineBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocking Filter Tile Line Buffer
    if (params->presDeblockingFilterTileRowStoreScratchBuffer != nullptr)
    {
        cmd.DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presDeblockingFilterTileRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.DeblockingFilterTileLineBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocking Filter Tile Column Buffer
    if (params->presDeblockingFilterColumnRowStoreScratchBuffer != nullptr)
    {
        cmd.DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presDeblockingFilterColumnRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.DeblockingFilterTileColumnBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Metadata Line Buffer
    if (m_hevcDatRowStoreCache.bEnabled)
    {
        cmd.MetadataLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.MetadataLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.MetadataLineBuffer.DW0_1.Graphicsaddress476 = m_hevcDatRowStoreCache.dwAddress;
    }
    else if (m_vp9DatRowStoreCache.bEnabled)
    {
        cmd.MetadataLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.MetadataLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.MetadataLineBuffer.DW0_1.Graphicsaddress476 = m_vp9DatRowStoreCache.dwAddress;
    }
    else if (params->presMetadataLineBuffer != nullptr)
    {
        cmd.MetadataLineBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

        resourceParams.presResource = params->presMetadataLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.MetadataLineBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 13;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Metadata Tile Line Buffer
    if (params->presMetadataTileLineBuffer != nullptr)
    {
        cmd.MetadataTileLineBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

        resourceParams.presResource = params->presMetadataTileLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.MetadataTileLineBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Metadata Tile Column Buffer
    if (params->presMetadataTileColumnBuffer != nullptr)
    {
        cmd.MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

        resourceParams.presResource = params->presMetadataTileColumnBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.MetadataTileColumnBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 19;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SAO Line Buffer
    if (m_hevcSaoRowStoreCache.bEnabled)
    {
        cmd.SaoLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SaoLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.SaoLineBuffer.DW0_1.Graphicsaddress476 = m_hevcSaoRowStoreCache.dwAddress;
    }
    else if (params->presSaoLineBuffer != nullptr)
    {
        cmd.SaoLineBufferMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

        resourceParams.presResource = params->presSaoLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.SaoLineBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 22;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SAO Tile Line Buffer
    if (params->presSaoTileLineBuffer != nullptr)
    {
        cmd.SaoTileLineBufferMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

        resourceParams.presResource = params->presSaoTileLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.SaoTileLineBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 25;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SAO Tile Column Buffer
    if (params->presSaoTileColumnBuffer != nullptr)
    {
        cmd.SaoTileColumnBufferMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

        resourceParams.presResource = params->presSaoTileColumnBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.SaoTileColumnBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 28;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Current Motion Vector Temporal Buffer
    if (params->presCurMvTempBuffer != nullptr)
    {
        cmd.CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MV_CODEC].Value;

        resourceParams.presResource = params->presCurMvTempBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.CurrentMotionVectorTemporalBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 31;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Only one control DW53 for all references
    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

    bool              firstRefPic = true;
    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;

    // NOTE: for both HEVC and VP9, set all the 8 ref pic addresses in HCP_PIPE_BUF_ADDR_STATE command to valid addresses for error concealment purpose
    for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        // Reference Picture Buffer
        if (params->presReferences[i] != nullptr)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->presReferences[i], &details));

            if (firstRefPic)
            {
                MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, params->presReferences[i], &mmcMode));

                cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                firstRefPic = false;
            }

            resourceParams.presResource = params->presReferences[i];
            resourceParams.pdwCmd = cmd.ReferencePictureBaseAddressRefaddr07[i].DW0_1.Value;
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = (i * 2) + 37; // * 2 to account for QW rather than DW
            resourceParams.bIsWritable = false;

            resourceParams.dwSharedMocsOffset = 53 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW53

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable =
        (mmcMode != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode =
        (mmcMode == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    // Original Uncompressed Picture Source, Encoder only
    if (params->psRawSurface != nullptr)
    {
        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;

        //SAS: 10-b input source surfaces are not compressed on CNL, but P010V (used for reconstructed reference surfaces) can be compressed
        if (params->bRawIs10Bit)
        {
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode = MHW_MEDIA_MEMCOMP_MODE_VERTICAL;
        }
        else
        {
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &params->psRawSurface->OsResource, &mmcMode));

            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = (mmcMode != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode = (mmcMode == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;
        }

        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = cmd.OriginalUncompressedPictureSource.DW0_1.Value;
        resourceParams.dwLocationInCmd = 54;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presStreamOutBuffer != nullptr)
    {
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable =
            (params->StreamOutBufMmcState != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode =
            (params->StreamOutBufMmcState == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        resourceParams.presResource = params->presStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.StreamoutDataDestination.DW0_1.Value;
        resourceParams.dwLocationInCmd = 57;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Decoded Picture Status / Error Buffer Base Address
    // HEVC Encoder Mode: This specifies 64 byte aligned buffer address for writing Slice size, when slice size conformance is enabled.
    if (params->presLcuBaseAddressBuffer != nullptr)
    {
        cmd.DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC].Value;

        resourceParams.presResource = params->presLcuBaseAddressBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress.DW0_1.Value;
        resourceParams.dwLocationInCmd = 60;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // LCU ILDB StreamOut Buffer
    if (params->presLcuILDBStreamOutBuffer != nullptr)
    {
        cmd.LcuIldbStreamoutBufferMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC].Value;

        resourceParams.presResource = params->presLcuILDBStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.LcuIldbStreamoutBuffer.DW0_1.Value;
        resourceParams.dwLocationInCmd = 63;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    cmd.CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MV_CODEC].Value;

    for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        // Collocated Motion vector Temporal Buffer
        if (params->presColMvTempBuffer[i] != nullptr)
        {
            resourceParams.presResource = params->presColMvTempBuffer[i];
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.CollocatedMotionVectorTemporalBuffer07[i].DW0_1.Value;
            resourceParams.dwLocationInCmd = (i * 2) + 66;
            resourceParams.bIsWritable = true;

            resourceParams.dwSharedMocsOffset = 82 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW82

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    // VP9 Probability Buffer
    if (params->presVp9ProbBuffer != nullptr)
    {
        cmd.Vp9ProbabilityBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presVp9ProbBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.Vp9ProbabilityBufferReadWrite.DW0_1.Value;
        resourceParams.dwLocationInCmd = 83;
        resourceParams.bIsWritable = true;

        resourceParams.dwSharedMocsOffset = 85 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW88

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    // VP9 Segment Id Buffer
    if (params->presVp9SegmentIdBuffer != nullptr)
    {
        cmd.Vp9SegmentIdBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presVp9SegmentIdBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.DW86_87.Value;
        resourceParams.dwLocationInCmd = 86;
        resourceParams.bIsWritable = true;

        resourceParams.dwSharedMocsOffset = 88 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW88

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    // HVD Line Row Store Buffer
    if (m_vp9HvdRowStoreCache.bEnabled)
    {
        cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Graphicsaddress476 = m_vp9HvdRowStoreCache.dwAddress;
    }
    else if (params->presHvdLineRowStoreBuffer != nullptr)
    {
        cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presHvdLineRowStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Value;
        resourceParams.dwLocationInCmd = 89;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // HVC Tile Row Store Buffer
    if (params->presHvdTileRowStoreBuffer != nullptr)
    {
        cmd.Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presHvdTileRowStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.Vp9HvdTileRowstoreBufferReadWrite.DW0_1.Value;
        resourceParams.dwLocationInCmd = 92;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // HEVC SAO streamout
    if (params->presSaoStreamOutBuffer != nullptr)
    {
        cmd.SaoStreamoutDataDestinationBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

        resourceParams.presResource = params->presSaoStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.SaoStreamoutDataDestinationBufferBaseAddress.DW0_1.Value;
        resourceParams.dwLocationInCmd = 95;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Frame Statistics Streamout Data Destination Buffer
    if (params->presFrameStatStreamOutBuffer != nullptr)
    {
        cmd.FrameStatisticsStreamoutDataDestinationBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

        resourceParams.presResource = params->presFrameStatStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.FrameStatisticsStreamoutDataDestinationBufferBaseAddress.DW0_1.Value;
        resourceParams.dwLocationInCmd = 98;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SSE Source Pixel Row Store Buffer
    if (params->presSseSrcPixelRowStoreBuffer != nullptr)
    {
        cmd.SseSourcePixelRowstoreBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SSE_SRC_PIXEL_ROW_STORE_BUFFER_CODEC].Value;
        resourceParams.presResource = params->presSseSrcPixelRowStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = cmd.SseSourcePixelRowstoreBufferBaseAddress.DW0_1.Value;
        resourceParams.dwLocationInCmd = 101;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpIndObjBaseAddrCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    mhw_vdbox_hcp_g10_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

    // mode specific settings
    if (CodecHalIsDecodeModeVLD(params->Mode))
    {
        MHW_MI_CHK_NULL(params->presDataBuffer);

        cmd.HcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Value;

        resourceParams.presResource = params->presDataBuffer;
        resourceParams.dwOffset = params->dwDataOffset;
        resourceParams.pdwCmd = cmd.HcpIndirectBitstreamObjectBaseAddress.DW0_1.Value;
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwSize = params->dwDataSize;
        resourceParams.bIsWritable = false;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
    }

    // following is for encoder
    if (!m_decodeInUse)
    {
        if (params->presMvObjectBuffer)
        {
            cmd.HcpIndirectCuObjectObjectMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC].Value;

            resourceParams.presResource = params->presMvObjectBuffer;
            resourceParams.dwOffset = params->dwMvObjectOffset;
            resourceParams.pdwCmd = cmd.DW6_7.Value;
            resourceParams.dwLocationInCmd = 6;
            resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwMvObjectSize, 0x1000);
            resourceParams.bIsWritable = false;

            // no upper bound for indirect CU object
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presPakBaseObjectBuffer)
        {
            cmd.HcpPakBseObjectAddressMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

            resourceParams.presResource = params->presPakBaseObjectBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.DW9_10.Value;
            resourceParams.dwLocationInCmd = 9;
            resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwPakBaseObjectSize, 0x1000);
            resourceParams.bIsWritable = true;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presCompressedHeaderBuffer)
        {
            cmd.HcpVp9PakCompressedHeaderSyntaxStreaminMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_COMPRESSED_HEADER_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presCompressedHeaderBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.DW14_15.Value;
            resourceParams.dwLocationInCmd = 14;
            resourceParams.dwSize = params->dwCompressedHeaderSize;
            resourceParams.bIsWritable = false;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presProbabilityCounterBuffer)
        {
            cmd.HcpVp9PakProbabilityCounterStreamoutMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_PROBABILITY_COUNTER_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presProbabilityCounterBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.DW17_18.Value;
            resourceParams.dwLocationInCmd = 17;
            resourceParams.dwSize = params->dwProbabilityCounterSize;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presProbabilityDeltaBuffer)
        {
            cmd.HcpVp9PakProbabilityDeltasStreaminMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PROBABILITY_DELTA_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presProbabilityDeltaBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.DW20_21.Value;
            resourceParams.dwLocationInCmd = 20;
            resourceParams.dwSize = params->dwProbabilityDeltaSize;
            resourceParams.bIsWritable = false;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presTileRecordBuffer)
        {
            cmd.HcpVp9PakTileRecordStreamoutMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SIZE_STREAMOUT_CODEC].Value;

            resourceParams.presResource = params->presTileRecordBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.DW23_24.Value;
            resourceParams.dwLocationInCmd = 23;
            resourceParams.dwSize = params->dwTileRecordSize;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

        }
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpDecodePicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcPicParams);

    mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g10_X>::AddHcpDecodePicStateCmd(cmdBuffer, params));

    auto hevcPicParams = params->pHevcPicParams;

    cmd->DW5.BitDepthChromaMinus8 = hevcPicParams->bit_depth_chroma_minus8;
    cmd->DW5.BitDepthLumaMinus8 = hevcPicParams->bit_depth_luma_minus8;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpEncodePicStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE       params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
    MHW_MI_CHK_NULL(params->pHevcEncPicParams);

    PMHW_BATCH_BUFFER                       batchBuffer = nullptr;
    mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD  cmd;

    auto hevcSeqParams = params->pHevcEncSeqParams;
    auto hevcPicParams = params->pHevcEncPicParams;

    if (params->bBatchBufferInUse)
    {
        MHW_MI_CHK_NULL(params->pBatchBuffer);
        batchBuffer            = params->pBatchBuffer;
    }

    cmd.DW1.Framewidthinmincbminus1      = hevcSeqParams->wFrameWidthInMinCbMinus1;
    cmd.DW1.PakTransformSkipEnable       = cmd.DW4.TransformSkipEnabledFlag = hevcPicParams->transform_skip_enabled_flag;
    cmd.DW1.Frameheightinmincbminus1     = hevcSeqParams->wFrameHeightInMinCbMinus1;

    cmd.DW2.Mincusize                    = hevcSeqParams->log2_min_coding_block_size_minus3;
    cmd.DW2.CtbsizeLcusize               = hevcSeqParams->log2_max_coding_block_size_minus3;
    cmd.DW2.Maxtusize                    = hevcSeqParams->log2_max_transform_block_size_minus2;
    cmd.DW2.Mintusize                    = hevcSeqParams->log2_min_transform_block_size_minus2;
    cmd.DW2.Minpcmsize                   = 0; // Not supported in CNL
    cmd.DW2.Maxpcmsize                   = 0; // Not supported in CNL

    cmd.DW3.Colpicisi                    = 0;
    cmd.DW3.Curpicisi                    = 0;

    cmd.DW4.SampleAdaptiveOffsetEnabledFlag                 = params->bSAOEnable;
    cmd.DW4.PcmEnabledFlag                                  = 0; // Not supported in CNL
    cmd.DW4.CuQpDeltaEnabledFlag                            = hevcPicParams->cu_qp_delta_enabled_flag; // In VDENC mode, this field should always be set to 1.
    cmd.DW4.DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth          = hevcPicParams->diff_cu_qp_delta_depth;
    cmd.DW4.PcmLoopFilterDisableFlag                        = hevcSeqParams->pcm_loop_filter_disable_flag;
    cmd.DW4.ConstrainedIntraPredFlag                        = 0;
    cmd.DW4.Log2ParallelMergeLevelMinus2                    = 0;
    cmd.DW4.SignDataHidingFlag                              = 0; // currently not supported in encoder
    cmd.DW4.LoopFilterAcrossTilesEnabledFlag                = 0;
    cmd.DW4.EntropyCodingSyncEnabledFlag                    = 0; // not supported as per Dimas notes. PAK restriction
    cmd.DW4.TilesEnabledFlag                                = 0; // not supported in encoder
    cmd.DW4.WeightedPredFlag                                = hevcPicParams->weighted_pred_flag;
    cmd.DW4.WeightedBipredFlag                              = hevcPicParams->weighted_bipred_flag;
    cmd.DW4.Fieldpic                                        = 0;
    cmd.DW4.Bottomfield                                     = 0;
    cmd.DW4.AmpEnabledFlag                                  = hevcSeqParams->amp_enabled_flag;
    cmd.DW4.TransquantBypassEnableFlag                      = hevcPicParams->transquant_bypass_enabled_flag;
    cmd.DW4.StrongIntraSmoothingEnableFlag                  = hevcSeqParams->strong_intra_smoothing_enable_flag;
    cmd.DW4.CuPacketStructure                               = 0; // output from HW VME, 1/2 CL per CU

    cmd.DW5.PicCbQpOffset                                               = hevcPicParams->pps_cb_qp_offset & 0x1f;
    cmd.DW5.PicCrQpOffset                                               = hevcPicParams->pps_cr_qp_offset & 0x1f;
    cmd.DW5.MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra     = hevcSeqParams->max_transform_hierarchy_depth_intra;
    cmd.DW5.MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter     = hevcSeqParams->max_transform_hierarchy_depth_inter;
    cmd.DW5.PcmSampleBitDepthChromaMinus1                               = hevcSeqParams->pcm_sample_bit_depth_chroma_minus1;
    cmd.DW5.PcmSampleBitDepthLumaMinus1                                 = hevcSeqParams->pcm_sample_bit_depth_luma_minus1;
    cmd.DW5.BitDepthChromaMinus8                                        = hevcSeqParams->bit_depth_chroma_minus8;
    cmd.DW5.BitDepthLumaMinus8                                          = hevcSeqParams->bit_depth_luma_minus8;

    cmd.DW6.LcuMaxBitsizeAllowed                            = hevcPicParams->LcuMaxBitsizeAllowed;
    cmd.DW6.Nonfirstpassflag                                = 0; // needs to be updated for HEVC VDEnc
    cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask           = 0;
    cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask    = 0;
    cmd.DW6.FrameszunderstatusenFramebitrateminreportmask   = 0;
    cmd.DW6.LoadSlicePointerFlag                            = 0; // must be set to 0 for encoder

    cmd.DW19.RdoqEnable                                     = params->bHevcRdoqEnabled;
    cmd.DW19.SseEnable                                      = true;
    // only for VDEnc
    cmd.DW19.RhodomainRateControlEnable                     = params->bUseVDEnc;   // DW19[6]
    // RhoDomainFrameLevelQP: This QP is used for RhoDomain Frame level statistics.
    cmd.DW19.Rhodomainframelevelqp                          = params->bUseVDEnc? hevcPicParams->QpY : 0;  // DW19[13:8]
    cmd.DW19.FractionalQpAdjustmentEnable                   = params->bUseVDEnc;   // DW19[17]

    cmd.DW19.FirstSliceSegmentInPicFlag                     = 1;
    cmd.DW19.Nalunittypeflag                                = 1;

    // For HEVC VDEnc Dynamic Slice Control
    if (hevcSeqParams->SliceSizeControl == 1)
    {
        cmd.DW19.PakDynamicSliceModeEnable      = 1;
        cmd.DW19.SlicePicParameterSetId         = hevcPicParams->slice_pic_parameter_set_id;
        cmd.DW19.Nalunittypeflag                = (hevcPicParams->nal_unit_type >= HEVC_NAL_UT_BLA_W_LP) &&
                                                    (hevcPicParams->nal_unit_type <= HEVC_NAL_UT_RSV_IRAP_VCL23);
        cmd.DW19.FirstSliceSegmentInPicFlag     = 1;
        cmd.DW19.NoOutputOfPriorPicsFlag        = hevcPicParams->no_output_of_prior_pics_flag;

        cmd.DW21.SliceSizeThresholdInBytes      = hevcPicParams->MaxSliceSizeInBytes;  // HuC FW is expected to update this
        cmd.DW22.TargetSliceSizeInBytes         = hevcPicParams->MaxSliceSizeInBytes;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpFqmStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_QM_PARAMS             params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g10_X::HCP_FQM_STATE_CMD cmd;

    if (params->Standard == CODECHAL_HEVC)
    {
        MHW_MI_CHK_NULL(params->pHevcIqMatrix);

        auto     iqMatrix = params->pHevcIqMatrix;
        uint16_t *fqMatrix = (uint16_t*)cmd.Quantizermatrix;

        /* 4x4 */
        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.Quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            cmd.DW1.IntraInter = intraInter;
            cmd.DW1.Sizeid = 0;
            cmd.DW1.ColorComponent = 0;

            for (uint8_t i = 0; i < 16; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List4x4[3 * intraInter][i]);
            }

            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));
        }

        /* 8x8, 16x16 and 32x32 */
        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.Quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            cmd.DW1.IntraInter = intraInter;
            cmd.DW1.Sizeid = 1;
            cmd.DW1.ColorComponent = 0;

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List8x8[3 * intraInter][i]);
            }

            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));
        }

        /* 16x16 DC */
        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.Quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            cmd.DW1.IntraInter = intraInter;
            cmd.DW1.Sizeid = 2;
            cmd.DW1.ColorComponent = 0;
            cmd.DW1.FqmDcValue1Dc = GetReciprocalScalingValue(iqMatrix->ListDC16x16[3 * intraInter]);

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List16x16[3 * intraInter][i]);
            }

            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));
        }

        /* 32x32 DC */
        for (uint8_t i = 0; i < 32; i++)
        {
            cmd.Quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            cmd.DW1.IntraInter = intraInter;
            cmd.DW1.Sizeid = 3;
            cmd.DW1.ColorComponent = 0;
            cmd.DW1.FqmDcValue1Dc = GetReciprocalScalingValue(iqMatrix->ListDC32x32[intraInter]);

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List32x32[intraInter][i]);
            }

            MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));
        }
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpDecodeSliceStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcSliceParams);

    auto hevcPicParams = hevcSliceState->pHevcPicParams;
    auto hevcSliceParams = hevcSliceState->pHevcSliceParams;

    mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g10_X>::AddHcpDecodeSliceStateCmd(cmdBuffer, hevcSliceState));

    int32_t sliceQP = hevcSliceParams->slice_qp_delta + hevcPicParams->init_qp_minus26 + 26;
    cmd->DW3.SliceqpSignFlag = (sliceQP >= 0) ? 0 : 1;
    cmd->DW3.Sliceqp = ABS(sliceQP);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpEncodeSliceStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcSliceParams);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcSeqParams);

    mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD cmd;

    auto hevcSliceParams                    = hevcSliceState->pEncodeHevcSliceParams;
    auto hevcPicParams                      = hevcSliceState->pEncodeHevcPicParams;
    auto hevcSeqParams                      = hevcSliceState->pEncodeHevcSeqParams;

    uint32_t ctbSize                      = 1 << (hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t widthInPix                   = (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) *
                                          (hevcSeqParams->wFrameWidthInMinCbMinus1 + 1);
    uint32_t widthInCtb                   = (widthInPix / ctbSize) +
                                          ((widthInPix % ctbSize) ? 1 : 0);  // round up

    uint32_t ctbAddr                      = hevcSliceParams->slice_segment_address;
    cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder   = ctbAddr % widthInCtb;
    cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder   = ctbAddr / widthInCtb;

    if (hevcSliceState->bLastSlice)
    {
        cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
        cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
    }
    else
    {
        ctbAddr                                               = hevcSliceParams->slice_segment_address + hevcSliceParams->NumLCUsInSlice;
        cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder   = ctbAddr % widthInCtb;
        cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder   = ctbAddr / widthInCtb;
    }

    cmd.DW3.SliceType                   = hevcSliceParams->slice_type;
    cmd.DW3.Lastsliceofpic              = hevcSliceState->bLastSlice;
    cmd.DW3.SliceqpSignFlag             = ((hevcSliceParams->slice_qp_delta + hevcPicParams->QpY) >= 0)
                                               ? 0 : 1; //8 bit will have 0 as sign bit adn 10 bit might have 1 as sign bit depending on Qp
    cmd.DW3.DependentSliceFlag          = 0; // Not supported on encoder
    cmd.DW3.SliceTemporalMvpEnableFlag
                                        = hevcSliceParams->slice_temporal_mvp_enable_flag;
    cmd.DW3.Sliceqp                     = hevcSliceParams->slice_qp_delta + hevcPicParams->QpY;
    cmd.DW3.SliceCbQpOffset             = hevcSliceParams->slice_cb_qp_offset;
    cmd.DW3.SliceCbQpOffset             = hevcSliceParams->slice_cr_qp_offset;
    cmd.DW3.Intrareffetchdisable        = hevcSliceState->bIntraRefFetchDisable;

    cmd.DW4.SliceHeaderDisableDeblockingFilterFlag               = hevcSliceParams->slice_deblocking_filter_disable_flag;
    cmd.DW4.SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder
                                        = hevcSliceParams->tc_offset_div2;
    cmd.DW4.SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder
                                        = hevcSliceParams->beta_offset_div2;
    cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag
                                        = (hevcSliceState->bVdencInUse) ? hevcPicParams->loop_filter_across_slices_flag : 0;
    cmd.DW4.SliceSaoChromaFlag          = hevcSliceState->bSaoChromaFlag;
    cmd.DW4.SliceSaoLumaFlag            = hevcSliceState->bSaoLumaFlag;
    cmd.DW4.MvdL1ZeroFlag               = 0; // Decoder only - set to 0 for encoder
    cmd.DW4.Islowdelay                  = hevcSliceState->bIsLowDelay;
    cmd.DW4.CollocatedFromL0Flag        = hevcSliceParams->collocated_from_l0_flag;
    cmd.DW4.Chromalog2Weightdenom       = hevcSliceParams->luma_log2_weight_denom + hevcSliceParams->delta_chroma_log2_weight_denom;
    cmd.DW4.LumaLog2WeightDenom         = hevcSliceParams->luma_log2_weight_denom;
    cmd.DW4.CabacInitFlag               = hevcSliceParams->cabac_init_flag;
    cmd.DW4.Maxmergeidx                 = hevcSliceParams->MaxNumMergeCand - 1;

    if (cmd.DW3.SliceTemporalMvpEnableFlag)
    {
        if (cmd.DW3.SliceType == cmd.SLICE_TYPE_I_SLICE)
        {
            cmd.DW4.Collocatedrefidx = 0;
        }
        else
        {
            // need to check with Ce for DDI issues
            uint8_t collocatedFromL0Flag      = cmd.DW4.CollocatedFromL0Flag;

            uint8_t collocatedRefIndex        = hevcPicParams->CollocatedRefPicIndex;
            MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

            uint8_t collocatedFrameIdx        = hevcSliceState->pRefIdxMapping[collocatedRefIndex];
            MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

            cmd.DW4.Collocatedrefidx = collocatedFrameIdx;
        }
    }
    else
    {
        cmd.DW4.Collocatedrefidx     = 0;
    }

    cmd.DW5.Sliceheaderlength        = 0; // Decoder only, setting to 0 for Encoder

    // Currently setting to defaults used in prototype
    cmd.DW6.Roundinter               = 4;
    cmd.DW6.Roundintra               = 10;

    cmd.DW7.Cabaczerowordinsertionenable        = 1;
    cmd.DW7.Emulationbytesliceinsertenable      = 1;
    cmd.DW7.TailInsertionEnable                 = (hevcPicParams->bLastPicInSeq || hevcPicParams->bLastPicInStream) && hevcSliceState->bLastSlice;
    cmd.DW7.SlicedataEnable                     = 1;
    cmd.DW7.HeaderInsertionEnable               = 1;

    cmd.DW8.IndirectPakBseDataStartOffsetWrite  = hevcSliceState->dwHeaderBytesInserted;

    // Transform skip related parameters
    if (hevcPicParams->transform_skip_enabled_flag)
    {
        cmd.DW9.TransformskipLambda                     = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_lambda;
        cmd.DW10.TransformskipNumzerocoeffsFactor0      = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numzerocoeffs_Factor0;
        cmd.DW10.TransformskipNumnonzerocoeffsFactor0   = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numnonzerocoeffs_Factor0;
        cmd.DW10.TransformskipNumzerocoeffsFactor1      = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numzerocoeffs_Factor1;
        cmd.DW10.TransformskipNumnonzerocoeffsFactor1   = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numnonzerocoeffs_Factor1;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, hevcSliceState->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpPakInsertObject(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_PAK_INSERT_PARAMS     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g10_X::HCP_PAK_INSERT_OBJECT_CMD  cmd;

    uint32_t dwordsUsed = cmd.dwSize;

    if (params->bLastPicInSeq && params->bLastPicInStream)
    {
        uint32_t dwPadding[3];

        dwordsUsed += sizeof(dwPadding) / sizeof(dwPadding[0]);

        cmd.DW0.DwordLength                                         = OP_LENGTH(dwordsUsed);
        cmd.DW1.Headerlengthexcludefrmsize                          = 0;
        cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag          = 1;
        cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag    = 1;
        cmd.DW1.EmulationflagEmulationbytebitsinsertenable          = 0;
        cmd.DW1.SkipemulbytecntSkipEmulationByteCount               = 0;
        cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50         = 16;
        cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10           = 0;
        cmd.DW1.IndirectPayloadEnable                               = 0;

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,  params->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

        dwPadding[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
        dwPadding[1] = (1L | (1L << 24));
        dwPadding[2] = (HEVC_NAL_UT_EOB << 1) | (1L << 8);
        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,  params->pBatchBufferForPakSlices, &dwPadding[0], sizeof(dwPadding)));
    }
    else
    if (params->bLastPicInSeq || params->bLastPicInStream)
    {
        uint32_t dwLastPicInSeqData[2], dwLastPicInStreamData[2];

        dwordsUsed += params->bLastPicInSeq * 2 + params->bLastPicInStream * 2;

        cmd.DW0.DwordLength                                         = OP_LENGTH(dwordsUsed);
        cmd.DW1.Headerlengthexcludefrmsize                          = 0;
        cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag          = 1;
        cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag    = 1;
        cmd.DW1.EmulationflagEmulationbytebitsinsertenable          = 0;
        cmd.DW1.SkipemulbytecntSkipEmulationByteCount               = 0;
        cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50         = 8;
        cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10           = 0;
        cmd.DW1.IndirectPayloadEnable                               = 0;

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,  params->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

        if (params->bLastPicInSeq)
        {
            dwLastPicInSeqData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
            dwLastPicInSeqData[1] = 1;  // nuh_temporal_id_plus1
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,   params->pBatchBufferForPakSlices, &dwLastPicInSeqData[0], sizeof(dwLastPicInSeqData)));
        }

        if (params->bLastPicInStream)
        {
            dwLastPicInStreamData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOB << 1) << 24));
            dwLastPicInStreamData[1] = 1; // nuh_temporal_id_plus1
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,   params->pBatchBufferForPakSlices, &dwLastPicInStreamData[0], sizeof(dwLastPicInStreamData)));
        }
    }
    else
    {
        uint32_t byteSize = (params->dwBitSize + 7) >> 3;
        uint32_t dataBitsInLastDw = params->dwBitSize % 32;
        if (dataBitsInLastDw == 0)
        {
            dataBitsInLastDw = 32;
        }

        dwordsUsed                                                += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t);
        cmd.DW0.DwordLength                                         = OP_LENGTH(dwordsUsed);
        cmd.DW1.Headerlengthexcludefrmsize                          = 0;
        cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag          = params->bEndOfSlice;
        cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag    = params->bLastHeader;
        cmd.DW1.EmulationflagEmulationbytebitsinsertenable          = params->bEmulationByteBitsInsert;
        cmd.DW1.SkipemulbytecntSkipEmulationByteCount               = params->uiSkipEmulationCheckCount;
        cmd.DW1.SliceHeaderIndicator                                = params->bResetBitstreamStartingPos;
        cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50         = dataBitsInLastDw;
        cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10           = 0;
        cmd.DW1.IndirectPayloadEnable                               = 0;

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,  params->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

        if (byteSize)
        {
            MHW_MI_CHK_NULL(params->pBsBuffer);
            MHW_MI_CHK_NULL(params->pBsBuffer->pBase);
            uint8_t *data = (uint8_t*)(params->pBsBuffer->pBase + params->dwOffset);
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer,  params->pBatchBufferForPakSlices, data, byteSize));
        }
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpVp9PicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_PIC_STATE         params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);

    mhw_vdbox_hcp_g10_X::HCP_VP9_PIC_STATE_CMD cmd;
    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList = params->ppVp9RefList;

    cmd.DW0.DwordLength                  = mhw_vdbox_hcp_g10_X::GetOpLength(12); //VP9_PIC_STATE command is common for both Decoder and Encoder. Decoder uses only 12 DWORDS of the generated 33 DWORDS

    uint32_t curFrameWidth               = vp9PicParams->FrameWidthMinus1 + 1;
    uint32_t curFrameHeight              = vp9PicParams->FrameHeightMinus1 + 1;

    cmd.DW1.FrameWidthInPixelsMinus1     = MOS_ALIGN_CEIL(curFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    cmd.DW1.FrameHeightInPixelsMinus1    = MOS_ALIGN_CEIL(curFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    cmd.DW2.FrameType                    = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag       = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag                = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.RefreshFrameContext          = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode           = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode    = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                  = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel               = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled          = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap        = cmd.DW2.SegmentationEnabled && vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.LosslessMode                 = vp9PicParams->PicFlags.fields.LosslessFlag;
    cmd.DW2.SegmentIdStreamoutEnable     = cmd.DW2.SegmentationUpdateMap;

    cmd.DW3.Log2TileRow             = vp9PicParams->log2_tile_rows;        // No need to minus 1 here.
    cmd.DW3.Log2TileColumn          = vp9PicParams->log2_tile_columns;     // No need to minus 1 here.
    if (vp9PicParams->subsampling_x == 1 && vp9PicParams->subsampling_y == 1)
    {
        //4:2:0
        cmd.DW3.ChromaSamplingFormat = 0;
    }
    else if (vp9PicParams->subsampling_x == 1 && vp9PicParams->subsampling_y == 0)
    {
        //4:2:2
        cmd.DW3.ChromaSamplingFormat = 1;
    }
    else if (vp9PicParams->subsampling_x == 0 && vp9PicParams->subsampling_y == 0)
    {
        //4:4:4
        cmd.DW3.ChromaSamplingFormat = 2;
    }
    cmd.DW3.Bitdepthminus8 = vp9PicParams->BitDepthMinus8;
    cmd.DW3.ProfileLevel   = vp9PicParams->profile;

    cmd.DW10.UncompressedHeaderLengthInBytes70  = vp9PicParams->UncompressedHeaderLengthInBytes;
    cmd.DW10.FirstPartitionSizeInBytes150       = vp9PicParams->FirstPartitionSize;

    if (vp9PicParams->PicFlags.fields.frame_type && !vp9PicParams->PicFlags.fields.intra_only)
    {
        PCODEC_PICTURE refFrameList     = &(vp9PicParams->RefFrameList[0]);

        uint8_t lastRefPicIndex         = refFrameList[vp9PicParams->PicFlags.fields.LastRefIdx].FrameIdx;
        uint32_t lastRefFrameWidth      = vp9RefList[lastRefPicIndex]->dwFrameWidth;
        uint32_t lastRefFrameHeight     = vp9RefList[lastRefPicIndex]->dwFrameHeight;

        uint8_t goldenRefPicIndex       = refFrameList[vp9PicParams->PicFlags.fields.GoldenRefIdx].FrameIdx;
        uint32_t goldenRefFrameWidth    = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
        uint32_t goldenRefFrameHeight   = vp9RefList[goldenRefPicIndex]->dwFrameHeight;

        uint8_t altRefPicIndex          = refFrameList[vp9PicParams->PicFlags.fields.AltRefIdx].FrameIdx;
        uint32_t altRefFrameWidth       = vp9RefList[altRefPicIndex]->dwFrameWidth;
        uint32_t altRefFrameHeight      = vp9RefList[altRefPicIndex]->dwFrameHeight;

        bool isScaling = (curFrameWidth == params->dwPrevFrmWidth) && (curFrameHeight == params->dwPrevFrmHeight) ? false : true;

        cmd.DW2.AllowHiPrecisionMv              = vp9PicParams->PicFlags.fields.allow_high_precision_mv;
        cmd.DW2.McompFilterType                 = vp9PicParams->PicFlags.fields.mcomp_filter_type;
        cmd.DW2.SegmentationTemporalUpdate      = cmd.DW2.SegmentationUpdateMap && vp9PicParams->PicFlags.fields.segmentation_temporal_update;

        cmd.DW2.RefFrameSignBias02              = vp9PicParams->PicFlags.fields.LastRefSignBias | 
                                                  (vp9PicParams->PicFlags.fields.GoldenRefSignBias << 1) | 
                                                  (vp9PicParams->PicFlags.fields.AltRefSignBias << 2);

        cmd.DW2.LastFrameType                   = !params->PrevFrameParams.fields.KeyFrame;

        cmd.DW2.UsePrevInFindMvReferences       = vp9PicParams->PicFlags.fields.error_resilient_mode    ||
                                                    params->PrevFrameParams.fields.KeyFrame             ||
                                                    params->PrevFrameParams.fields.IntraOnly            ||
                                                    !params->PrevFrameParams.fields.Display             ||
                                                    isScaling ? false : true;

        cmd.DW2.SegmentIdStreaminEnable         = vp9PicParams->PicFlags.fields.error_resilient_mode    ||
                                                    !cmd.DW2.SegmentationEnabled ||
                                                    isScaling ? false : true;

        cmd.DW4.HorizontalScaleFactorForLast    = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW4.VerticalScaleFactorForLast      = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW5.HorizontalScaleFactorForGolden  = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW5.VerticalScaleFactorForGolden    = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW6.HorizontalScaleFactorForAltref  = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW6.VerticalScaleFactorForAltref    = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW7.LastFrameWidthInPixelsMinus1    = lastRefFrameWidth - 1;
        cmd.DW7.LastFrameHieghtInPixelsMinus1   = lastRefFrameHeight - 1;

        cmd.DW8.GoldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
        cmd.DW8.GoldenFrameHieghtInPixelsMinus1 = goldenRefFrameHeight - 1;

        cmd.DW9.AltrefFrameWidthInPixelsMinus1  = altRefFrameWidth - 1;
        cmd.DW9.AltrefFrameHieghtInPixelsMinus1 = altRefFrameHeight - 1;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpVp9PicStateEncCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_VDBOX_VP9_ENCODE_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);
    MHW_MI_CHK_NULL(params->ppVp9RefList);

    mhw_vdbox_hcp_g10_X::HCP_VP9_PIC_STATE_CMD cmd;

    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList  = params->ppVp9RefList;

    cmd.DW1.FrameWidthInPixelsMinus1        = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameWidthMinus1 , CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    cmd.DW1.FrameHeightInPixelsMinus1       = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameHeightMinus1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    cmd.DW2.FrameType                       = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag          = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag                   = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.AllowHiPrecisionMv              = vp9PicParams->PicFlags.fields.allow_high_precision_mv;
    cmd.DW2.McompFilterType                 = vp9PicParams->PicFlags.fields.mcomp_filter_type;

    cmd.DW2.RefFrameSignBias02              = vp9PicParams->RefFlags.fields.LastRefSignBias | 
                                              (vp9PicParams->RefFlags.fields.GoldenRefSignBias << 1) | 
                                              (vp9PicParams->RefFlags.fields.AltRefSignBias << 2);

    cmd.DW2.HybridPredictionMode            = vp9PicParams->PicFlags.fields.comp_prediction_mode == 2;
    cmd.DW2.SelectableTxMode                = params->ucTxMode == 4;
    cmd.DW2.RefreshFrameContext             = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode              = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode       = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                     = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel                  = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled             = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap           = vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.SegmentationTemporalUpdate      = vp9PicParams->PicFlags.fields.segmentation_temporal_update;
    cmd.DW2.LosslessMode                    = vp9PicParams->PicFlags.fields.LosslessFlag;

    cmd.DW3.Log2TileColumn                  = vp9PicParams->log2_tile_columns;
    cmd.DW3.Log2TileRow                     = vp9PicParams->log2_tile_rows;
    cmd.DW3.SseEnable                       = params->bSSEEnable;

    if (vp9PicParams->PicFlags.fields.frame_type && !vp9PicParams->PicFlags.fields.intra_only)
    {
        uint32_t curFrameWidth                     = vp9PicParams->SrcFrameWidthMinus1 + 1;
        uint32_t curFrameHeight                    = vp9PicParams->SrcFrameHeightMinus1 + 1;

        PCODEC_PICTURE refFrameList                       = &(vp9PicParams->RefFrameList[0]);

        cmd.DW2.LastFrameType               = !params->PrevFrameParams.fields.KeyFrame;

        cmd.DW2.UsePrevInFindMvReferences   = vp9PicParams->PicFlags.fields.error_resilient_mode ||
            params->PrevFrameParams.fields.KeyFrame ||
            params->PrevFrameParams.fields.IntraOnly ||
            !params->PrevFrameParams.fields.Display ||
            (curFrameWidth != params->dwPrevFrmWidth) ||
            (curFrameHeight != params->dwPrevFrmHeight) ? 0 : 1;

        if ((vp9PicParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x01) || (vp9PicParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x01))
        {
            MHW_ASSERT(!CodecHal_PictureIsInvalid(refFrameList[vp9PicParams->RefFlags.fields.LastRefIdx]));

            uint8_t  lastRefPicIndex = refFrameList[vp9PicParams->RefFlags.fields.LastRefIdx].FrameIdx;
            uint32_t lastRefFrameWidth = 0;
            uint32_t lastRefFrameHeight = 0;
            if (!params->bUseDysRefSurface)
            {
                lastRefFrameWidth = vp9RefList[lastRefPicIndex]->dwFrameWidth;
                lastRefFrameHeight = vp9RefList[lastRefPicIndex]->dwFrameHeight;
            }
            else
            {
                lastRefFrameWidth = curFrameWidth;
                lastRefFrameHeight = curFrameHeight;
            }

            cmd.DW4.HorizontalScaleFactorForLast    = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW4.VerticalScaleFactorForLast      = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW7.LastFrameWidthInPixelsMinus1    = lastRefFrameWidth - 1;
            cmd.DW7.LastFrameHieghtInPixelsMinus1   = lastRefFrameHeight - 1;
        }

        if ((vp9PicParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x02) || (vp9PicParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x02))
        {
            MHW_ASSERT(!CodecHal_PictureIsInvalid(refFrameList[vp9PicParams->RefFlags.fields.GoldenRefIdx]));

            uint8_t goldenRefPicIndex = refFrameList[vp9PicParams->RefFlags.fields.GoldenRefIdx].FrameIdx;
            uint32_t goldenRefFrameWidth = 0;
            uint32_t goldenRefFrameHeight = 0;
            if (!params->bUseDysRefSurface)
            {
                goldenRefFrameWidth = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
                goldenRefFrameHeight = vp9RefList[goldenRefPicIndex]->dwFrameHeight;
            }
            else
            {
                goldenRefFrameWidth = curFrameWidth;
                goldenRefFrameHeight = curFrameHeight;
            }

            cmd.DW5.HorizontalScaleFactorForGolden  = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW5.VerticalScaleFactorForGolden    = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW8.GoldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
            cmd.DW8.GoldenFrameHieghtInPixelsMinus1 = goldenRefFrameHeight - 1;
        }

        if ((vp9PicParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x04) || (vp9PicParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x04))
        {
            MHW_ASSERT(!CodecHal_PictureIsInvalid(refFrameList[vp9PicParams->RefFlags.fields.AltRefIdx]));

            uint8_t altRefPicIndex = refFrameList[vp9PicParams->RefFlags.fields.AltRefIdx].FrameIdx;
            uint32_t altRefFrameWidth = 0;
            uint32_t altRefFrameHeight = 0;
            if (!params->bUseDysRefSurface)
            {
                altRefFrameWidth = vp9RefList[altRefPicIndex]->dwFrameWidth;
                altRefFrameHeight = vp9RefList[altRefPicIndex]->dwFrameHeight;
            }
            else
            {
                altRefFrameWidth = curFrameWidth;
                altRefFrameHeight = curFrameHeight;
            }

            cmd.DW6.HorizontalScaleFactorForAltref      = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW6.VerticalScaleFactorForAltref        = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW9.AltrefFrameWidthInPixelsMinus1      = altRefFrameWidth - 1;
            cmd.DW9.AltrefFrameHieghtInPixelsMinus1     = altRefFrameHeight - 1;
        }
    }

    cmd.DW13.BaseQIndexSameAsLumaAc                     = vp9PicParams->LumaACQIndex;
    cmd.DW13.HeaderInsertionEnable                      = 1;

    cmd.DW14.ChromaacQindexdelta                        = Convert2SignMagnitude(vp9PicParams->ChromaACQIndexDelta, 5);
    cmd.DW14.ChromadcQindexdelta                        = Convert2SignMagnitude(vp9PicParams->ChromaDCQIndexDelta, 5);
    cmd.DW14.LumaDcQIndexDelta                          = Convert2SignMagnitude(vp9PicParams->LumaDCQIndexDelta, 5);

    cmd.DW15.LfRefDelta0                                = Convert2SignMagnitude(vp9PicParams->LFRefDelta[0], 7);
    cmd.DW15.LfRefDelta1                                = Convert2SignMagnitude(vp9PicParams->LFRefDelta[1], 7);
    cmd.DW15.LfRefDelta2                                = Convert2SignMagnitude(vp9PicParams->LFRefDelta[2], 7);
    cmd.DW15.LfRefDelta3                                = Convert2SignMagnitude(vp9PicParams->LFRefDelta[3], 7);

    cmd.DW16.LfModeDelta0                               = Convert2SignMagnitude(vp9PicParams->LFModeDelta[0], 7);
    cmd.DW16.LfModeDelta1                               = Convert2SignMagnitude(vp9PicParams->LFModeDelta[1], 7);

    cmd.DW17.Bitoffsetforlfrefdelta                     = vp9PicParams->BitOffsetForLFRefDelta;
    cmd.DW17.Bitoffsetforlfmodedelta                    = vp9PicParams->BitOffsetForLFModeDelta;
    cmd.DW18.Bitoffsetforlflevel                        = vp9PicParams->BitOffsetForLFLevel;
    cmd.DW18.Bitoffsetforqindex                         = vp9PicParams->BitOffsetForQIndex;
    cmd.DW32.Bitoffsetforfirstpartitionsize             = vp9PicParams->BitOffsetForFirstPartitionSize;

    cmd.DW19.VdencPakOnlyPass                           = params->bVdencPakOnlyPassFlag;

    if (params->uiMaxBitRate || params->uiMinBitRate)
    {
        // br stored in 4KB chunks when unit==1
        cmd.DW20.Framebitratemax                        = params->uiMaxBitRate >> 12;
        cmd.DW20.Framebitratemaxunit                    = cmd.DW21.Framebitrateminunit = 1;
        cmd.DW21.Framebitratemin                        = params->uiMinBitRate >> 12;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpVp9SegmentStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_SEGMENT_STATE     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g10_X::HCP_VP9_SEGMENT_STATE_CMD  cmd;

    cmd.DW1.SegmentId = params->ucCurrentSegmentId;

    if (!m_decodeInUse)
    {
        auto vp9SegData = params->pVp9EncodeSegmentParams->SegData[params->ucCurrentSegmentId];

        cmd.DW2.SegmentSkipped                      = vp9SegData.SegmentFlags.fields.SegmentSkipped;
        cmd.DW2.SegmentReference                    = vp9SegData.SegmentFlags.fields.SegmentReference;
        cmd.DW2.SegmentReferenceEnabled             = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

        cmd.DW7.SegmentLfLevelDeltaEncodeModeOnly   = Convert2SignMagnitude(vp9SegData.SegmentLFLevelDelta, 7);
        cmd.DW7.SegmentQindexDeltaEncodeModeOnly    = Convert2SignMagnitude(vp9SegData.SegmentQIndexDelta, 9);
    }
    else
    {
        auto vp9SegData = params->pVp9SegmentParams->SegData[params->ucCurrentSegmentId];

        cmd.DW2.SegmentSkipped          = vp9SegData.SegmentFlags.fields.SegmentReferenceSkipped;
        cmd.DW2.SegmentReference        = vp9SegData.SegmentFlags.fields.SegmentReference;
        cmd.DW2.SegmentReferenceEnabled = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

        cmd.DW3.Filterlevelref0Mode0    = vp9SegData.FilterLevel[0][0];
        cmd.DW3.Filterlevelref0Mode1    = vp9SegData.FilterLevel[0][1];
        cmd.DW3.Filterlevelref1Mode0    = vp9SegData.FilterLevel[1][0];
        cmd.DW3.Filterlevelref1Mode1    = vp9SegData.FilterLevel[1][1];

        cmd.DW4.Filterlevelref2Mode0    = vp9SegData.FilterLevel[2][0];
        cmd.DW4.Filterlevelref2Mode1    = vp9SegData.FilterLevel[2][1];
        cmd.DW4.Filterlevelref3Mode0    = vp9SegData.FilterLevel[3][0];
        cmd.DW4.Filterlevelref3Mode1    = vp9SegData.FilterLevel[3][1];

        cmd.DW5.LumaDcQuantScaleDecodeModeOnly      = vp9SegData.LumaDCQuantScale;
        cmd.DW5.LumaAcQuantScaleDecodeModeOnly      = vp9SegData.LumaACQuantScale;

        cmd.DW6.ChromaDcQuantScaleDecodeModeOnly    = vp9SegData.ChromaDCQuantScale;
        cmd.DW6.ChromaAcQuantScaleDecodeModeOnly    = vp9SegData.ChromaACQuantScale;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpHevcVp9RdoqStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcEncSeqParams);

    mhw_vdbox_hcp_g10_X::HEVC_VP9_RDOQ_STATE_CMD    cmd;
    uint16_t                                        lambdaTab[2][2][64];

    MHW_MI_CHK_NULL(params->pHevcEncPicParams);

    uint32_t sliceTypeIdx = (params->pHevcEncPicParams->CodingType == I_TYPE) ? 0 : 1;

    MOS_ZeroMemory(lambdaTab, sizeof(lambdaTab));
    if (params->pHevcEncSeqParams->bit_depth_luma_minus8 == 0)
    {
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[0][0], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][0][0]), 
            RDOQLamdas8bits[sliceTypeIdx][0][0], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][0][0])));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[0][1], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][0][1]), 
            RDOQLamdas8bits[sliceTypeIdx][0][1], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][0][1])));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[1][0], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][1][0]), 
            RDOQLamdas8bits[sliceTypeIdx][1][0], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][1][0])));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[1][1], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][1][1]), 
            RDOQLamdas8bits[sliceTypeIdx][1][1], 
            sizeof(RDOQLamdas8bits[sliceTypeIdx][1][1])));
    }
    else if (params->pHevcEncSeqParams->bit_depth_luma_minus8 == 2)
    {
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[0][0], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][0][0]), 
            RDOQLamdas10bits[sliceTypeIdx][0][0], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][0][0])));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[0][1], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][0][1]), 
            RDOQLamdas10bits[sliceTypeIdx][0][1], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][0][1])));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[1][0], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][1][0]), 
            RDOQLamdas10bits[sliceTypeIdx][1][0], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][1][0])));
        MHW_MI_CHK_STATUS(MOS_SecureMemcpy(
            lambdaTab[1][1], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][1][1]), 
            RDOQLamdas10bits[sliceTypeIdx][1][1], 
            sizeof(RDOQLamdas10bits[sliceTypeIdx][1][1])));
    }

    for (uint8_t i = 0; i < 32; i++)
    {
        cmd.Intralumalambda[i].DW0.Lambdavalue0 = lambdaTab[0][0][i * 2];
        cmd.Intralumalambda[i].DW0.Lambdavalue1 = lambdaTab[0][0][i * 2 + 1];

        cmd.Intrachromalambda[i].DW0.Lambdavalue0 = lambdaTab[0][1][i * 2];
        cmd.Intrachromalambda[i].DW0.Lambdavalue1 = lambdaTab[0][1][i * 2 + 1];

        cmd.Interlumalambda[i].DW0.Lambdavalue0 = lambdaTab[1][0][i * 2];
        cmd.Interlumalambda[i].DW0.Lambdavalue1 = lambdaTab[1][0][i * 2 + 1];

        cmd.Interchromalambda[i].DW0.Lambdavalue0 = lambdaTab[1][1][i * 2];
        cmd.Interchromalambda[i].DW0.Lambdavalue1 = lambdaTab[1][1][i * 2 + 1];
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::AddHcpHevcPicBrcBuffer(
    PMOS_RESOURCE                   hcpImgStates,
    PMHW_VDBOX_HEVC_PIC_STATE        hevcPicState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpImgStates);

    MOS_COMMAND_BUFFER                     constructedCmdBuf;
    mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD cmd;
    uint32_t*                              insertion = nullptr;
    MOS_LOCK_PARAMS                        lockFlags;
    m_brcNumPakPasses = hevcPicState->brcNumPakPasses;

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, hcpImgStates, &lockFlags);
    MHW_MI_CHK_NULL(data);

    constructedCmdBuf.pCmdBase      = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr       = (uint32_t *)data;
    constructedCmdBuf.iOffset       = 0;
    constructedCmdBuf.iRemaining    = BRC_IMG_STATE_SIZE_PER_PASS_G10 * (m_brcNumPakPasses);

    MHW_MI_CHK_STATUS(AddHcpPicStateCmd(&constructedCmdBuf, hevcPicState));

    cmd = *(mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD *)data;

    for (uint32_t i = 0; i < m_brcNumPakPasses; i++)
    {
        if (i == 0)
        {
            cmd.DW6.Nonfirstpassflag = false;
        }
        else
        {
            cmd.DW6.Nonfirstpassflag = true;
        }

        cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask  = true;
        cmd.DW6.FrameszunderstatusenFramebitrateminreportmask = true;
        cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask         = false; // BRC update kernel does not consider if there is any LCU whose size is too big

        *(mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD *)data     = cmd;

        /* add batch buffer end insertion flag */
        insertion = (uint32_t*)(data + mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD::byteSize);
        *insertion = 0x05000000;

        data += BRC_IMG_STATE_SIZE_PER_PASS_G10;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, hcpImgStates));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetOsResLaceOrAceOrRgbHistogramBufferSize(
    uint32_t                        width,
    uint32_t                        height,
    uint32_t                       *size)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    *size = m_veboxRgbHistogramSize;

    uint32_t dwSizeLace = MOS_ROUNDUP_DIVIDE(height, 64) *
        MOS_ROUNDUP_DIVIDE(width, 64)  *
        m_veboxLaceHistogram256BinPerBlock;

    uint32_t dwSizeNoLace = m_veboxAceHistogramSizePerFramePerSlice *
        m_veboxNumFramePreviousCurrent                   *
        m_veboxMaxSlices;

    *size += MOS_MAX(dwSizeLace, dwSizeNoLace);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG10::GetOsResStatisticsOutputBufferSize(
    uint32_t                        width,
    uint32_t                        height,
    uint32_t                       *size)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    width  = MOS_ALIGN_CEIL(width, 64);
    height = MOS_ROUNDUP_DIVIDE(height, 4) + MOS_ROUNDUP_DIVIDE(m_veboxStatisticsSize * sizeof(uint32_t), width);
    *size   = width * height;

    return eStatus;
}
