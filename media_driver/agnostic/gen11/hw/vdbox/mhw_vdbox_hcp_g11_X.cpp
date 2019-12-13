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
//! \file     mhw_vdbox_hcp_g11_X.cpp
//! \brief    Constructs VdBox HCP commands on Gen11-based platforms

#include "mhw_vdbox_hcp_g11_X.h"
#include "mhw_mi_hwcmd_g11_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g11_X.h"
#include "mhw_vdbox_g11_X.h"
#include "mhw_mmio_g11.h"

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

void MhwVdboxHcpInterfaceG11::InitMmioRegisters()
{
    MmioRegistersHcp *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

    mmioRegisters->watchdogCountCtrlOffset                           = WATCHDOG_COUNT_CTRL_OFFSET_INIT_G11;
    mmioRegisters->watchdogCountThresholdOffset                      = WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT_G11;
    mmioRegisters->hcpDebugFEStreamOutSizeRegOffset                  = HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncImageStatusMaskRegOffset                    = HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncImageStatusCtrlRegOffset                    = HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset            = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset           = HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset    = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncQpStatusCountRegOffset                      = HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncSliceCountRegOffset                         = HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpEncVdencModeTimerRegOffset                     = HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset         = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpVp9EncBitstreamBytecountFrameNoHeaderRegOffset = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpVp9EncImageStatusMaskRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G11;
    mmioRegisters->csEngineIdOffset                                  = CS_ENGINE_ID_OFFSET_INIT_G11;
    mmioRegisters->hcpDecStatusRegOffset                             = HCP_DEC_STATUS_REG_OFFSET_INIT_G11;
    mmioRegisters->hcpCabacStatusRegOffset                           = HCP_CABAC_STATUS_REG_OFFSET_INIT_G11;

    m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
}

void MhwVdboxHcpInterfaceG11::InitRowstoreUserFeatureSettings()
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    if (MEDIA_IS_SKU(m_skuTable, FtrSimulationMode))
    {
        // Disable RowStore Cache on simulation by default
        userFeatureData.u32Data = 1;
    }
    else
    {
        userFeatureData.u32Data = 0;
    }

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
            __MEDIA_USER_FEATURE_VALUE_VP9_DATROWSTORECACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_vp9DatRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

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

MOS_STATUS MhwVdboxHcpInterfaceG11::GetRowstoreCachingAddrs(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(rowstoreParams);

    if (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC)
    {
        if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1920)
        {
            if (rowstoreParams->ucBitDepthMinus8 > 0)
            {
                if (rowstoreParams->ucChromaFormat == HCP_CHROMA_FORMAT_YUV444)
                {
                    if (m_hevcDatRowStoreCache.bSupported)
                    {
                        m_hevcDatRowStoreCache.bEnabled = true;
                        m_hevcDatRowStoreCache.dwAddress = 0;
                    }

                    if (m_hevcDfRowStoreCache.bSupported)
                    {
                        m_hevcDfRowStoreCache.bEnabled = false;
                        m_hevcDfRowStoreCache.dwAddress = 0;
                    }

                    if (m_hevcSaoRowStoreCache.bSupported)
                    {
                        m_hevcSaoRowStoreCache.bEnabled = true;
                        m_hevcSaoRowStoreCache.dwAddress = HEVCSAOROWSTORE_BASEADDRESS_444_10BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }
                }
                else
                {
                    if (m_hevcDatRowStoreCache.bSupported)
                    {
                        m_hevcDatRowStoreCache.bEnabled = true;
                        m_hevcDatRowStoreCache.dwAddress = 0;
                    }

                    if (m_hevcDfRowStoreCache.bSupported)
                    {
                        m_hevcDfRowStoreCache.bEnabled = true;
                        m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_NON444_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }

                    if (m_hevcSaoRowStoreCache.bSupported)
                    {
                        m_hevcSaoRowStoreCache.bEnabled = false;
                        m_hevcSaoRowStoreCache.dwAddress = 0;
                    }
                }
            }
            else //8bit
            {
                if (rowstoreParams->ucChromaFormat == HCP_CHROMA_FORMAT_YUV444)
                {
                    if (m_hevcDatRowStoreCache.bSupported)
                    {
                        m_hevcDatRowStoreCache.bEnabled = false;
                        m_hevcDatRowStoreCache.dwAddress = 0;
                    }

                    if (m_hevcDfRowStoreCache.bSupported)
                    {
                        m_hevcDfRowStoreCache.bEnabled = true;
                        m_hevcDfRowStoreCache.dwAddress = 0;
                    }

                    if (m_hevcSaoRowStoreCache.bSupported)
                    {
                        m_hevcSaoRowStoreCache.bEnabled = true;
                        m_hevcSaoRowStoreCache.dwAddress = HEVCSAOROWSTORE_BASEADDRESS_8BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }
                }
                else
                {
                    if (m_hevcDatRowStoreCache.bSupported)
                    {
                        m_hevcDatRowStoreCache.bEnabled = true;
                        m_hevcDatRowStoreCache.dwAddress = 0;
                    }

                    if (m_hevcDfRowStoreCache.bSupported)
                    {
                        m_hevcDfRowStoreCache.bEnabled = true;
                        m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_NON444_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }

                    if (m_hevcSaoRowStoreCache.bSupported)
                    {
                        m_hevcSaoRowStoreCache.bEnabled = true;
                        m_hevcSaoRowStoreCache.dwAddress = HEVCSAOROWSTORE_BASEADDRESS_8BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }
                }
            }
        }
        else if (rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_1920 && rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_3840)
        {
            if (m_hevcDatRowStoreCache.bSupported)
            {
                m_hevcDatRowStoreCache.bEnabled = false;
                m_hevcDatRowStoreCache.dwAddress = 0;
            }

            if (m_hevcDfRowStoreCache.bSupported)
            {
                m_hevcDfRowStoreCache.bEnabled = false;
                m_hevcDfRowStoreCache.dwAddress = 0;
            }

            if (m_hevcSaoRowStoreCache.bSupported)
            {
                m_hevcSaoRowStoreCache.bEnabled = true;
                m_hevcSaoRowStoreCache.dwAddress = 0;
            }
        }
        else
        {
            m_hevcDatRowStoreCache.bEnabled = false;
            m_hevcDfRowStoreCache.bEnabled = false;
            m_hevcSaoRowStoreCache.bEnabled = false;
        }
    }

    if (rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP9VLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP9)
    {
        if (m_vp9HvdRowStoreCache.bSupported)
        {
            m_vp9HvdRowStoreCache.bEnabled = true;
            m_vp9HvdRowStoreCache.dwAddress = 0;
        }

        bool vp9420b8LessThan1920Flag = false;
        if (rowstoreParams->ucChromaFormat == HCP_CHROMA_FORMAT_YUV420)
        {
            if (rowstoreParams->ucBitDepthMinus8 == 0)
            {
                if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1920)
                {
                    vp9420b8LessThan1920Flag = true;
                    if (m_vp9DatRowStoreCache.bSupported)
                    {
                        m_vp9DatRowStoreCache.bEnabled = false;
                        m_vp9DatRowStoreCache.dwAddress = 0;
                    }

                    if (m_vp9DfRowStoreCache.bSupported)
                    {
                        m_vp9DfRowStoreCache.bEnabled = true;
                        m_vp9DfRowStoreCache.dwAddress = VP9DFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }
                }
            }
        }

        if (vp9420b8LessThan1920Flag == false)
        {
            if (m_vp9DatRowStoreCache.bSupported)
            {
                m_vp9DatRowStoreCache.bEnabled = true;
                m_vp9DatRowStoreCache.dwAddress = (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1920) ? VP9DATROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920 : VP9DATROWSTORE_BASEADDRESS_PICWIDTH_BETWEEN_1920_AND_3840;
            }

            if (m_vp9DfRowStoreCache.bSupported)
            {
                m_vp9DfRowStoreCache.bEnabled = false;
                m_vp9DfRowStoreCache.dwAddress = 0;
            }
        }

        if (rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_3840)
        {
            m_vp9HvdRowStoreCache.bEnabled = false;
            m_vp9HvdRowStoreCache.dwAddress = 0;
            m_vp9DatRowStoreCache.bEnabled = false;
            m_vp9DatRowStoreCache.dwAddress = 0;
            m_vp9DfRowStoreCache.bEnabled = false;
            m_vp9DfRowStoreCache.dwAddress = 0;
        }
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::GetHcpStateCommandSize(
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

    MHW_CHK_NULL_RETURN(params);
    auto paramsG11 = dynamic_cast<PMHW_VDBOX_STATE_CMDSIZE_PARAMS_G11>(params);
    MHW_CHK_NULL_RETURN(paramsG11);

    if (standard == CODECHAL_HEVC)
    {
        maxSize =
            mhw_vdbox_vdenc_g11_X::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize * 8;

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
                mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD::byteSize + // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
                20 * mhw_vdbox_hcp_g11_X::HCP_QM_STATE_CMD::byteSize +
                8 * mhw_vdbox_hcp_g11_X::HCP_FQM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HEVC_VP9_RDOQ_STATE_CMD::byteSize + // RDOQ
                2 * mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize + // Slice level commands
                2 * mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize + // need for Status report, Mfc Status and
                10 * mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD::byteSize + // 8 for BRCStatistics and 2 for RC6 WAs
                mhw_mi_g11_X::MI_LOAD_REGISTER_MEM_CMD::byteSize + // 1 for RC6 WA
                2 * mhw_vdbox_hcp_g11_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize + // Two PAK insert object commands are for headers before the slice header and the header for the end of stream
                4 * mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize +    // two (BRC+reference frame) for clean-up HW semaphore memory and another two for signal it
                17 * mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize +// Use HW wait command for each reference and one wait for current semaphore object
                mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize +     // Use HW wait command for each BRC pass
                + mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize    // Use HW wait command for each VDBOX
                + 2 * mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize  // One is for reset and another one for set per VDBOX
                + 8 * mhw_mi_g11_X::MI_COPY_MEM_MEM_CMD::byteSize // Need to copy SSE statistics/ Slice Size overflow into memory
                ;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                8 * PATCH_LIST_COMMAND(HCP_FQM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) + // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
                2 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) + // Slice level commands
                2 * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +       // need for Status report, Mfc Status and
                11 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) + // 8 for BRCStatistics and 3 for RC6 WAs
                22 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD)      // Use HW wait commands plus its memory clean-up and signal (4+ 16 + 1 + 1)
                + 8 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) // At maximal, there are 8 batch buffers for 8 VDBOXes for VE. Each box has one BB.
                + PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD)               // Need one flush before copy command
                + PATCH_LIST_COMMAND(MFX_WAIT_CMD)                  // Need one wait after copy command
                + 3 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD)     // one wait commands and two for reset and set semaphore memory
                + 8 * PATCH_LIST_COMMAND(MI_COPY_MEM_MEM_CMD)       // Need to copy SSE statistics/ Slice Size overflow into memory
                ;
        }
        else
        {
            maxSize +=
                20 * mhw_vdbox_hcp_g11_X::HCP_QM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_TILE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_TILE_CODING_CMD::byteSize;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_TILE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_TILE_CODING_COMMAND);

            if (paramsG11->bScalableMode)
            {
                // Due to the fact that there is no slice level command in BE status, we mainly consider commands in FE.
                maxSize +=
                    4 * mhw_mi_g11_X::MI_ATOMIC_CMD::byteSize +                       // used to reset semaphore in BEs
                    2 * mhw_mi_g11_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize + // 1 Conditional BB END for FE hang, 1 for streamout buffer writing over allocated size
                    3 * mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize +             // for FE & BE0, BEs sync
                    15 * mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize +              // for placeholder cmds to resolve the hazard between BEs sync
                    3 * mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize +               // for FE status set and clear
                    3 * mhw_mi_g11_X::MI_LOAD_REGISTER_IMM_CMD::byteSize +            // for FE status set
                    2 * mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize +                     // 2 needed for command flush in slice level
                    2 * mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD::byteSize +           // store the carry flag of reported size in FE
                    4 * sizeof(MHW_MI_ALU_PARAMS) +                                    // 4 ALU commands needed for substract opertaion in FE
                    mhw_mi_g11_X::MI_MATH_CMD::byteSize +                             // 1 needed for FE status set
                    mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize;                 // 1 needed for FE status set
                mhw_mi_g11_X::MI_MATH_CMD::byteSize +                             // 1 needed for FE status set
                    mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize;                // 1 needed for FE status set

                patchListMaxSize +=
                    4 * PATCH_LIST_COMMAND(MI_ATOMIC_CMD) +
                    2 * PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                    3 * PATCH_LIST_COMMAND(MI_SEMAPHORE_WAIT_CMD) +
                    3 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) +
                    2 * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
                    2 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD);
            }
        }
    }
    else if (standard == CODECHAL_VP9)     // VP9 Clear Decode
    {
        maxSize =
            mhw_vdbox_vdenc_g11_X::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD::byteSize * 4 +
            mhw_vdbox_hcp_g11_X::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g11_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
            mhw_vdbox_hcp_g11_X::HCP_BSD_OBJECT_CMD::byteSize +
            mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize * 8;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) * 4 +
            PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
            PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);

        if (mode == CODECHAL_ENCODE_MODE_VP9)
        {
            maxSize +=
                mhw_vdbox_hcp_g11_X::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize * 2 +
                mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize * 4 +
                mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 11 +
                mhw_mi_g11_X::MI_COPY_MEM_MEM_CMD::byteSize * 4 +
                mhw_mi_g11_X::MI_BATCH_BUFFER_START_CMD::byteSize * 3 +
                mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize * 2 +   // Slice level commands
                mhw_mi_g11_X::MI_LOAD_REGISTER_MEM_CMD::byteSize * 2 +
                mhw_vdbox_hcp_g11_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize * 2 +
                mhw_vdbox_hcp_g11_X::HCP_TILE_CODING_CMD::byteSize +
                mhw_mi_g11_X::MI_BATCH_BUFFER_START_CMD::byteSize +
                mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize * 3 + // Use HW wait command for each pass(3) level barrier and after huc probability update
                mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize +     // Use HW wait command before pak integration kernel
                mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize * 50; // One is for reset and another one for set per VDBOX, 15 for semaphore wait in pass(3) level barrier


            patchListMaxSize +=
                PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2 +
                PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) * 4 +
                PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) * 11 +
                PATCH_LIST_COMMAND(MI_COPY_MEM_MEM_CMD) * 4 +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) * 3 +
                PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) * 2 +
                PATCH_LIST_COMMAND(HCP_PAK_INSERT_OBJECT_CMD) * 2 +
                PATCH_LIST_COMMAND(HCP_TILE_CODING_COMMAND) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) +
                PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) * 50;
        }
        else
        {
            maxSize += mhw_vdbox_hcp_g11_X::HCP_VP9_PIC_STATE_CMD::byteSize;

            patchListMaxSize += PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD);

            if (paramsG11->bScalableMode)
            {
                maxSize +=
                    mhw_vdbox_hcp_g11_X::HCP_TILE_CODING_CMD::byteSize +
                    mhw_mi_g11_X::MI_ATOMIC_CMD::byteSize * 4 +   // used to reset semaphore in BEs
                    mhw_mi_g11_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +   // for streamout buffer writing over allocated size
                    mhw_mi_g11_X::MI_SEMAPHORE_WAIT_CMD::byteSize * 3 +   // for FE & BE0, BEs sync
                    mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize * 15 +   // for placeholder cmds to resolve the hazard between BEs sync
                    mhw_mi_g11_X::MI_STORE_DATA_IMM_CMD::byteSize +   // for FE status set
                    mhw_mi_g11_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 3 +   // for FE status set
                    mhw_mi_g11_X::MI_FLUSH_DW_CMD::byteSize +   // for command flush in partition level
                    mhw_mi_g11_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 2 +   // store the carry flag of reported size in FE
                    4 * sizeof(MHW_MI_ALU_PARAMS) +                                    // 4 ALU commands needed for substract opertaion in FE
                    mhw_mi_g11_X::MI_MATH_CMD::byteSize +                             // 1 needed for FE status set
                    mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize;                 // 1 needed for FE status set
                mhw_mi_g11_X::MI_MATH_CMD::byteSize +   // 1 needed for FE status set
                    mhw_mi_g11_X::MI_LOAD_REGISTER_REG_CMD::byteSize;          // 1 needed for FE status set

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(HCP_TILE_CODING_COMMAND) +
                    PATCH_LIST_COMMAND(MI_ATOMIC_CMD) * 4 +
                    PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                    PATCH_LIST_COMMAND(MI_SEMAPHORE_WAIT_CMD) * 3 +
                    PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) +
                    PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
                    PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) * 2;
            }
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

MOS_STATUS MhwVdboxHcpInterfaceG11::GetHcpPrimitiveCommandSize(
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
                2 * mhw_vdbox_hcp_g11_X::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g11_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
                2 * mhw_mi_g11_X::MI_BATCH_BUFFER_START_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_TILE_CODING_CMD::byteSize; // one slice cannot be with more than one tile

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PAK_INSERT_OBJECT_CMD) +
                2 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) + // One is for the PAK command and another one is for the BB when BRC and single task mode are on
                PATCH_LIST_COMMAND(HCP_TILE_CODING_COMMAND); // HCP_TILE_CODING_STATE command
        }
        else
        {
            maxSize =
                2 * mhw_vdbox_hcp_g11_X::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g11_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g11_X::MI_BATCH_BUFFER_END_CMD::byteSize;

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
                mhw_vdbox_hcp_g11_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw_vdbox_hcp_g11_X::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g11_X::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g11_X::MI_BATCH_BUFFER_END_CMD::byteSize;

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

MOS_STATUS MhwVdboxHcpInterfaceG11::GetHevcBufferSize(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpBufSizeParam);

    uint8_t bitDepthMultFactor = 0;
    uint32_t bufferSize = 0;
    double  dbFormatMultFactor = 0;

    uint8_t maxBitDepth = hcpBufSizeParam->ucMaxBitDepth;
    uint32_t ctbLog2SizeY = hcpBufSizeParam->dwCtbLog2SizeY;
    uint32_t picWidth = hcpBufSizeParam->dwPicWidth;
    uint32_t picHeight = hcpBufSizeParam->dwPicHeight;
    uint32_t maxFrameSize = hcpBufSizeParam->dwMaxFrameSize;
    uint32_t rowStoreSzLCU = 0;
    uint32_t colStoreSzLCU = 0;

    uint32_t widthInCtb = (picWidth % 16) ? (picWidth / 16 + 1) : (picWidth / 16); //using smallest LCU to get max width
    uint32_t heightInCtb = (picHeight % 16) ? (picHeight / 16 + 1) : (picHeight / 16);
    uint32_t numBaseUnitsInLCU = 1 << (ctbLog2SizeY - 2);//size in number of 4x4 in the LCU per column
    HCP_CHROMA_FORMAT_IDC chromaFormat = (HCP_CHROMA_FORMAT_IDC)hcpBufSizeParam->ucChromaFormat;

    uint32_t mvtSize = 0;
    uint32_t mvtbSize = 0;

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
        dbFormatMultFactor = (chromaFormat == HCP_CHROMA_FORMAT_YUV444) ? 1.5 : 1;
        bitDepthMultFactor = (maxBitDepth > 8) ? 2 : 1;
        rowStoreSzLCU = (uint32_t)(((2 * numBaseUnitsInLCU * dbFormatMultFactor * 128 * bitDepthMultFactor) + 511) / 512);
        bufferSize = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        dbFormatMultFactor = (chromaFormat == HCP_CHROMA_FORMAT_YUV420) ? 1 : 1.5;
        bitDepthMultFactor = (maxBitDepth > 8) ? 2 : 1;
        colStoreSzLCU = (uint32_t)(((2 * numBaseUnitsInLCU * dbFormatMultFactor * 128 * bitDepthMultFactor + 3 * 128 * bitDepthMultFactor) + 511) / 512);
        bufferSize = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RT_COL:
        colStoreSzLCU = (ctbLog2SizeY == 6) ? 2 : 1;
        bufferSize = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        rowStoreSzLCU = (ctbLog2SizeY == 6) ? 2 : 1;
        bufferSize = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        colStoreSzLCU = (ctbLog2SizeY == 6) ? 2 : 1;
        bufferSize = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL:
    {
        uint32_t colStoreSizeLCU[2][3]; //[bitdepth 8/10][LCU 16/32/64]
        if (chromaFormat == HCP_CHROMA_FORMAT_YUV422 || chromaFormat == HCP_CHROMA_FORMAT_YUV444)
        {
            colStoreSizeLCU[0][0] = 1; colStoreSizeLCU[0][1] = 2; colStoreSizeLCU[0][2] = 3;
            colStoreSizeLCU[1][0] = 2; colStoreSizeLCU[1][1] = 3; colStoreSizeLCU[1][2] = 6;
        }
        else
        {
            colStoreSizeLCU[0][0] = 1; colStoreSizeLCU[0][1] = 1; colStoreSizeLCU[0][2] = 2;
            colStoreSizeLCU[1][0] = 1; colStoreSizeLCU[1][1] = 2; colStoreSizeLCU[1][2] = 4;
        }
        colStoreSzLCU = colStoreSizeLCU[(maxBitDepth == 8) ? 0 : 1][ctbLog2SizeY - 4];
        bufferSize = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
        break;
    }
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE:
    {
        uint32_t uiRowStoreSizeLCU[2][3];//[bitdepth 8/10][LCU 16/32/64]
        if (chromaFormat == HCP_CHROMA_FORMAT_YUV420 || chromaFormat == HCP_CHROMA_FORMAT_YUV422)
        {
            uiRowStoreSizeLCU[0][0] = 2; uiRowStoreSizeLCU[0][1] = 3; uiRowStoreSizeLCU[0][2] = 5;
            uiRowStoreSizeLCU[1][0] = 2; uiRowStoreSizeLCU[1][1] = 3; uiRowStoreSizeLCU[1][2] = 5;
        }
        else
        {
            uiRowStoreSizeLCU[0][0] = 2; uiRowStoreSizeLCU[0][1] = 4; uiRowStoreSizeLCU[0][2] = 7;
            uiRowStoreSizeLCU[1][0] = 3; uiRowStoreSizeLCU[1][1] = 4; uiRowStoreSizeLCU[1][2] = 8;
        }
        rowStoreSzLCU = uiRowStoreSizeLCU[(maxBitDepth == 8) ? 0 : 1][ctbLog2SizeY - 4];
        bufferSize = rowStoreSzLCU * MHW_CACHELINE_SIZE * widthInCtb;
        break;
    }
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL:
    {
        // [chroma_format_idc][lcu_size] = [420/422/444][lcu16/lcu32/lcu64]
        uint32_t formatMultFactorTab[3][3] = { { 4,5,9 },{ 5,7,12 },{ 5,7,12 } };
        uint32_t formatMultFactor;

        if (chromaFormat == HCP_CHROMA_FORMAT_MONOCHROME)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("invalid input chroma format.\n");
            return eStatus;
        }
        formatMultFactor = formatMultFactorTab[chromaFormat - 1][ctbLog2SizeY - 4];
        colStoreSzLCU = formatMultFactor;
        bufferSize = colStoreSzLCU * MHW_CACHELINE_SIZE * heightInCtb;
        break;
    }
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
        mvtSize = ((((picWidth + 63) >> 6) * (((picHeight + 15) >> 4)) + 1)&(-2));
        mvtbSize = ((((picWidth + 31) >> 5) * (((picHeight + 31) >> 5)) + 1)&(-2));
        bufferSize = MOS_MAX(mvtSize, mvtbSize) * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT:
        //From sas, cabac stream out buffer size =
        //(#LCU) in picture * (Worst case LCU_CU_TU_info) + 1 byte aligned per LCU + Bitstream Size * 3
        if ((chromaFormat == HCP_CHROMA_FORMAT_YUV420) && (maxBitDepth == 8))
        {
            bufferSize = widthInCtb * heightInCtb * MHW_HCP_WORST_CASE_CU_TU_INFO
                + widthInCtb * heightInCtb
                + maxFrameSize * 3;
        }
        else
        {
            bufferSize = widthInCtb * heightInCtb * MHW_HCP_WORST_CASE_CU_TU_INFO_REXT
                + widthInCtb * heightInCtb
                + maxFrameSize * 3;
        }
        bufferSize = MOS_ALIGN_CEIL(bufferSize, MHW_CACHELINE_SIZE);
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    hcpBufSizeParam->dwBufferSize = bufferSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::GetVp9BufferSize(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpBufSizeParam);

    uint32_t  bufferSize = 0;
    uint32_t  dblkRsbSizeMultiplier = 0;
    uint32_t  dblkCsbSizeMultiplier = 0;
    uint32_t  intraPredMultiplier = 0;

    uint8_t maxBitDepth = hcpBufSizeParam->ucMaxBitDepth;
    uint32_t widthInSb = hcpBufSizeParam->dwPicWidth;
    uint32_t heightInSb = hcpBufSizeParam->dwPicHeight;
    uint32_t widthInMinCb = widthInSb * 64 / 8; //using smallest cb to get max width
    uint32_t heightInMinCb = heightInSb * 64 / 8;
    HCP_CHROMA_FORMAT_IDC chromaFormat = (HCP_CHROMA_FORMAT_IDC)hcpBufSizeParam->ucChromaFormat;
    uint32_t maxFrameSize = hcpBufSizeParam->dwMaxFrameSize;

    if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
    {
        dblkRsbSizeMultiplier = (maxBitDepth > 8) ? 36 : 18;
        dblkCsbSizeMultiplier = (maxBitDepth > 8) ? 34 : 17;
        intraPredMultiplier = (maxBitDepth > 8) ? 4 : 2;
    }
    else if (chromaFormat == HCP_CHROMA_FORMAT_YUV444)
    {
        dblkRsbSizeMultiplier = (maxBitDepth > 8) ? 54 : 27;
        dblkCsbSizeMultiplier = (maxBitDepth > 8) ? 50 : 25;
        intraPredMultiplier = (maxBitDepth > 8) ? 6 : 3;
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
        bufferSize = widthInSb * dblkRsbSizeMultiplier * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        bufferSize = heightInSb * dblkCsbSizeMultiplier * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        bufferSize = widthInSb * 5 * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        bufferSize = heightInSb * 5 * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL:
        bufferSize = widthInSb * heightInSb * 9 * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID:
        bufferSize = widthInSb * heightInSb * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE:
        bufferSize = widthInSb * MHW_CACHELINE_SIZE;
        break;
        //scalable mode specific buffers
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL:
        bufferSize = intraPredMultiplier * heightInSb * MHW_CACHELINE_SIZE;
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT:
        //From sas, cabac stream out buffer size =
        //(#LCU) in picture * (Worst case LCU_CU_TU_info) + 1 byte aligned per LCU + Bitstream Size * 3
        if ((chromaFormat == HCP_CHROMA_FORMAT_YUV420) && (maxBitDepth == 8))
        {
            bufferSize = widthInMinCb * heightInMinCb * MHW_HCP_WORST_CASE_CU_TU_INFO
                + widthInMinCb * heightInMinCb
                + maxFrameSize * 3;
        }
        else
        {
            bufferSize = widthInMinCb * heightInMinCb * MHW_HCP_WORST_CASE_CU_TU_INFO_REXT
                + widthInMinCb * heightInMinCb
                + maxFrameSize * 3;
        }
        bufferSize = MOS_ALIGN_CEIL(bufferSize, MHW_CACHELINE_SIZE);
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    hcpBufSizeParam->dwBufferSize = bufferSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::IsHevcBufferReallocNeeded(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(reallocParam);

    bool realloc = false;
    uint32_t picWidth = reallocParam->dwPicWidth;
    uint32_t picHeight = reallocParam->dwPicHeight;
    uint32_t ctbLog2SizeY = reallocParam->dwCtbLog2SizeY;
    uint8_t maxBitDepth = reallocParam->ucMaxBitDepth;
    HCP_CHROMA_FORMAT_IDC chromaFormat = (HCP_CHROMA_FORMAT_IDC)reallocParam->ucChromaFormat;
    uint32_t picWidthAlloced = reallocParam->dwPicWidthAlloced;
    uint32_t picHeightAlloced = reallocParam->dwPicHeightAlloced;
    uint32_t ctbLog2SizeYMax = reallocParam->dwCtbLog2SizeYMax;
    uint32_t frameSize = reallocParam->dwFrameSize;
    uint32_t frameSizeAlloced = reallocParam->dwFrameSizeAlloced;

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
        realloc = (picWidth > picWidthAlloced || ctbLog2SizeY > ctbLog2SizeYMax);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        realloc = (picHeight > picHeightAlloced || ctbLog2SizeY > ctbLog2SizeYMax);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        if ((ctbLog2SizeYMax < 6 && ctbLog2SizeY == 6) || (picWidth > picWidthAlloced))
        {
            realloc = true;
        }
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_MV_UP_RT_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        if ((ctbLog2SizeYMax < 6 && ctbLog2SizeY == 6) || (picHeight > picHeightAlloced))
        {
            realloc = true;
        }
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL:
        if (maxBitDepth == 8 && chromaFormat == HCP_CHROMA_FORMAT_YUV420)
        {
            realloc = ((ctbLog2SizeYMax < 6 && ctbLog2SizeY == 6) || (picHeight > picHeightAlloced));
        }
        else
        {
            realloc = (picHeight > picHeightAlloced || ctbLog2SizeY > ctbLog2SizeYMax);
        }
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE:
        realloc = (picWidth > picWidthAlloced || ctbLog2SizeY > ctbLog2SizeYMax);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL:
        realloc = (picHeight > picHeightAlloced || ctbLog2SizeY > ctbLog2SizeYMax);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
        realloc = (picWidth > picWidthAlloced || picHeight > picHeightAlloced);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT:
        realloc = (frameSize > frameSizeAlloced);
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    reallocParam->bNeedBiggerSize = realloc;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::IsVp9BufferReallocNeeded(
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(reallocParam);

    bool     realloc = false;
    uint32_t widthInSb = reallocParam->dwPicWidth;
    uint32_t heightInSb = reallocParam->dwPicHeight;
    uint32_t picWidthInSbAlloced = reallocParam->dwPicWidthAlloced;
    uint32_t picHeightInSbAlloced = reallocParam->dwPicHeightAlloced;
    uint32_t frameSize = reallocParam->dwFrameSize;
    uint32_t frameSizeAlloced = reallocParam->dwFrameSizeAlloced;

    switch (bufferType)
    {
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE:
        realloc = (widthInSb > picWidthInSbAlloced);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_UP_RIGHT_COL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_INTRA_PRED_LFT_RECON_COL:
        realloc = (heightInSb > picHeightInSbAlloced);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL:
    case MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID:
        realloc = (heightInSb > picHeightInSbAlloced || widthInSb > picWidthInSbAlloced);
        break;
    case MHW_VDBOX_HCP_INTERNAL_BUFFER_CABAC_STREAMOUT:
        realloc = (frameSize > frameSizeAlloced);
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    reallocParam->bNeedBiggerSize = realloc;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    bool       scalableEncode = false;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    auto paramsG11 = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11>(params);
    MHW_MI_CHK_NULL(paramsG11);
    mhw_vdbox_hcp_g11_X::HCP_PIPE_MODE_SELECT_CMD   cmd;

    //for gen 11, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
    MHW_MI_CHK_STATUS(m_miInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    // Secure scalable encode workloads requires special handling for Gen11+
    scalableEncode = (paramsG11->MultiEngineMode != MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY && !m_decodeInUse);
    MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForHcpPipeModeSelect((uint32_t *)&cmd, scalableEncode));

    cmd.DW1.AdvancedRateControlEnable    = params->bAdvancedRateControlEnable;
    cmd.DW1.CodecStandardSelect          = CodecHal_GetStandardFromMode(params->Mode) - CODECHAL_HCP_BASE;
    cmd.DW1.PakPipelineStreamoutEnable   = params->bStreamOutEnabled || params->pakPiplnStrmoutEnabled;
    cmd.DW1.DeblockerStreamoutEnable     = params->bDeblockerStreamOutEnable;
    cmd.DW1.VdencMode                    = params->bVdencEnabled;
    cmd.DW1.RdoqEnabledFlag              = params->bRdoqEnable;
    cmd.DW1.PakFrameLevelStreamoutEnable = params->bStreamOutEnabled || params->pakFrmLvlStrmoutEnable;
    cmd.DW1.PipeWorkingMode              = paramsG11->PipeWorkMode;
    cmd.DW1.MultiEngineMode              = paramsG11->MultiEngineMode;

    if (m_decodeInUse)
    {
        cmd.DW1.CodecSelect = cmd.CODEC_SELECT_DECODE;
    }
    else
    {
        cmd.DW1.CodecSelect = cmd.CODEC_SELECT_ENCODE;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, params->pBatchBuffer, &cmd, sizeof(cmd)));

    //for gen 11, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
    MHW_MI_CHK_STATUS(m_miInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g11_X>::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        if (params->ChromaType == HCP_CHROMA_FORMAT_YUV420 && params->psSurface->Format == Format_NV12)// 4:2:0 8bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_PLANAR4208;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV420 && params->psSurface->Format == Format_P010)// 4:2:0 10bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV422 && params->psSurface->Format == Format_YUY2) // 4:2:2 8bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_YUY2FORMAT;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV422 && params->psSurface->Format == Format_Y210) // 4:2:2 10bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y216Y210FORMAT;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV444 && params->psSurface->Format == Format_AYUV) // 4:4:4 8bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_AYUV4444FORMAT;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV444 && params->psSurface->Format == Format_Y410) // 4:4:4 10bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y410FORMAT;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else // only support bitdepth <= 10bit
    {
        if (params->ChromaType == HCP_CHROMA_FORMAT_YUV420 && params->psSurface->Format == Format_P010)// 4:2:0
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV422 && params->psSurface->Format == Format_Y210) // 4:2:2
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y216Y210FORMAT;;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV444 && params->psSurface->Format == Format_Y410) // 4:4:4
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y410FORMAT;;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    cmd->DW3.DefaultAlphaValue = 0; // needs to be programmable

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpEncodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g11_X::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g11_X>::AddHcpEncodeSurfaceStateCmd(cmdBuffer, params));

    bool surf10bit= (params->psSurface->Format == Format_P010) ||
                   (params->psSurface->Format == Format_P210) ||
                   (params->psSurface->Format == Format_Y210) ||
                   (params->psSurface->Format == Format_Y410) ||
                   (params->psSurface->Format == Format_R10G10B10A2) ||
                   (params->psSurface->Format == Format_B10G10R10A2);

    if (params->ChromaType == HCP_CHROMA_FORMAT_YUV422)
    {
        if (params->ucBitDepthLumaMinus8 > 0)
        {
            if (params->ucSurfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                cmd->DW2.SurfaceFormat = surf10bit ?
                    cmd->SURFACE_FORMAT_Y216Y210FORMAT : cmd->SURFACE_FORMAT_YUY2FORMAT;
            }
            else
            {
                cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y216VARIANT;
            }
        }
        else
        {
            cmd->DW2.SurfaceFormat = (params->ucSurfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID) ?
                cmd->SURFACE_FORMAT_YUY2FORMAT : cmd->SURFACE_FORMAT_YUY2VARIANT;
        }
    }
    else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV444)
    {
        if (params->ucBitDepthLumaMinus8 == 0)
        {
            cmd->DW2.SurfaceFormat = params->ucSurfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID ?
                cmd->SURFACE_FORMAT_AYUV4444FORMAT : cmd->SURFACE_FORMAT_AYUV4444VARIANT;
        }
        else if (params->ucBitDepthLumaMinus8 <= 2)
        {
            if (params->ucSurfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                cmd->DW2.SurfaceFormat = surf10bit ?
                    cmd->SURFACE_FORMAT_Y410FORMAT : cmd->SURFACE_FORMAT_AYUV4444FORMAT;
            }
            else
            {
                cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y416VARIANT;
            }
        }
        else
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_Y416FORMAT;
        }
    }
    else    //params->ChromaType == HCP_CHROMA_FORMAT_YUV420
    {
        if (params->ucBitDepthLumaMinus8 > 0)
        {
            if (params->ucSurfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                cmd->DW2.SurfaceFormat = surf10bit ?
                    cmd->SURFACE_FORMAT_P010 : cmd->SURFACE_FORMAT_PLANAR4208;
            }
            else
            {
                cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010VARIANT;
            }
        }
        else
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_PLANAR4208;
        }
    }

    cmd->DW2.YOffsetForUCbInPixel  = cmd->DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;

    //Set U/V offsets for Variant surfaces
    if (cmd->DW2.SurfaceFormat == cmd->SURFACE_FORMAT_Y416VARIANT ||
        cmd->DW2.SurfaceFormat == cmd->SURFACE_FORMAT_AYUV4444VARIANT)
    {
        cmd->DW2.YOffsetForUCbInPixel = params->dwReconSurfHeight;
        cmd->DW3.YOffsetForVCr = params->dwReconSurfHeight << 1;
    }
    else if (cmd->DW2.SurfaceFormat == cmd->SURFACE_FORMAT_Y216VARIANT ||
        cmd->DW2.SurfaceFormat == cmd->SURFACE_FORMAT_YUY2VARIANT)
    {
        cmd->DW2.YOffsetForUCbInPixel =
        cmd->DW3.YOffsetForVCr = params->dwReconSurfHeight;
    }


    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_SURFACE details;
    mhw_vdbox_hcp_g11_X::HCP_PIPE_BUF_ADDR_STATE_CMD cmd;

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

    resourceParams.presResource = &(params->psPreDeblockSurface->OsResource);
    resourceParams.dwOffset = params->psPreDeblockSurface->dwOffset;
    resourceParams.pdwCmd = (cmd.DecodedPicture.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.DeblockingFilterLineBuffer.DW0_1.Value);
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
        cmd.DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_LINE_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presDeblockingFilterTileRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockingFilterTileLineBuffer.DW0_1.Value);
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
        cmd.DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_COLUMN_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presDeblockingFilterColumnRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockingFilterTileColumnBuffer.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.MetadataLineBuffer.DW0_1.Value);
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
        cmd.MetadataTileLineBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_TILE_LINE_CODEC].Value;

        resourceParams.presResource = params->presMetadataTileLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.MetadataTileLineBuffer.DW0_1.Value);
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
        cmd.MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value |= m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_TILE_COLUMN_CODEC].Value;

        resourceParams.presResource = params->presMetadataTileColumnBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.MetadataTileColumnBuffer.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.SaoLineBuffer.DW0_1.Value);
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
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_TILE_LINE_CODEC].Value;

        resourceParams.presResource = params->presSaoTileLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SaoTileLineBuffer.DW0_1.Value);
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
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_TILE_COLUMN_CODEC].Value;

        resourceParams.presResource = params->presSaoTileColumnBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SaoTileColumnBuffer.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.CurrentMotionVectorTemporalBuffer.DW0_1.Value);
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

    bool                firstRefPic = true;
    MOS_MEMCOMP_STATE   mmcMode = MOS_MEMCOMP_DISABLED;

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
            resourceParams.pdwCmd = (cmd.ReferencePictureBaseAddressRefaddr07[i].DW0_1.Value);
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
        MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &params->psRawSurface->OsResource, &mmcMode));

        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;
        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = (mmcMode != MOS_MEMCOMP_DISABLED) ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode = (mmcMode == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = (cmd.OriginalUncompressedPictureSource.DW0_1.Value);
        resourceParams.dwLocationInCmd = 54;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // StreamOut Data Destination, Decoder only
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
        resourceParams.pdwCmd = (cmd.StreamoutDataDestination.DW0_1.Value);
        resourceParams.dwLocationInCmd = 57;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Pak Cu Level Streamout Data
    if (params->presPakCuLevelStreamoutBuffer != nullptr)
    {
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;
        resourceParams.presResource = params->presPakCuLevelStreamoutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.StreamoutDataDestination.DW0_1.Value);
        resourceParams.dwLocationInCmd = 57;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Decoded Picture Status / Error Buffer Base Address
    if (params->presLcuBaseAddressBuffer != nullptr)
    {
        cmd.DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC].Value;

        resourceParams.presResource = params->presLcuBaseAddressBuffer;
        resourceParams.dwOffset = params->dwLcuStreamOutOffset;
        resourceParams.pdwCmd = (cmd.DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.LcuIldbStreamoutBuffer.DW0_1.Value);
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
            resourceParams.pdwCmd = (cmd.CollocatedMotionVectorTemporalBuffer07[i].DW0_1.Value);
            resourceParams.dwLocationInCmd = (i * 2) + 66;
            resourceParams.bIsWritable = false;

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
        resourceParams.pdwCmd = (cmd.Vp9ProbabilityBufferReadWrite.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.DW86_87.Value);
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
        resourceParams.pdwCmd = (cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.Vp9HvdTileRowstoreBufferReadWrite.DW0_1.Value);
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
        cmd.SaoRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

        resourceParams.presResource = params->presSaoStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DW95_96.Value);
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
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_FRAME_STATS_STREAMOUT_DATA_CODEC].Value;

        resourceParams.presResource = params->presFrameStatStreamOutBuffer;
        resourceParams.dwOffset = params->dwFrameStatStreamOutOffset;
        resourceParams.pdwCmd = (cmd.FrameStatisticsStreamoutDataDestinationBufferBaseAddress.DW0_1.Value);
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
        resourceParams.pdwCmd = (cmd.SseSourcePixelRowstoreBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 101;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    //Gen11 new added buffer
    auto paramsG11 = dynamic_cast<PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11>(params);
    MHW_MI_CHK_NULL(paramsG11);

    // Slice state stream out buffer
    if (paramsG11->presSliceStateStreamOutBuffer != nullptr)
    {
        cmd.HcpScalabilitySliceStateBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SLICE_STATE_STREAM_OUT_BUFFER_CODEC].Value;

        resourceParams.presResource = paramsG11->presSliceStateStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.HcpScalabilitySliceStateBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 104;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // CABAC Syntax stream out buffer
    if (paramsG11->presCABACSyntaxStreamOutBuffer != nullptr)
    {
        cmd.HcpScalabilityCabacDecodedSyntaxElementsBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_CABAC_SYNTAX_STREAM_OUT_BUFFER_CODEC].Value;

        resourceParams.presResource = paramsG11->presCABACSyntaxStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.HcpScalabilityCabacDecodedSyntaxElementsBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 107;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // MV Upper Right Col Store
    if (paramsG11->presMvUpRightColStoreBuffer != nullptr)
    {
        cmd.MotionVectorUpperRightColumnStoreBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRED_COL_STORE_BUFFER_CODEC].Value;

        resourceParams.presResource = paramsG11->presMvUpRightColStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.MotionVectorUpperRightColumnStoreBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 110;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // IntraPred Upper Right Col Store
    if (paramsG11->presIntraPredUpRightColStoreBuffer != nullptr)
    {
        cmd.IntraPredictionUpperRightColumnStoreBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRED_COL_STORE_BUFFER_CODEC].Value;

        resourceParams.presResource = paramsG11->presIntraPredUpRightColStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntraPredictionUpperRightColumnStoreBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 113;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // IntraPred Left Recon Col Store
    if (paramsG11->presIntraPredLeftReconColStoreBuffer != nullptr)
    {
        cmd.IntraPredictionLeftReconColumnStoreBufferAttributesReadWrite.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRED_COL_STORE_BUFFER_CODEC].Value;

        resourceParams.presResource = paramsG11->presIntraPredLeftReconColStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntraPredictionLeftReconColumnStoreBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 116;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // CABAC Syntax Stream Out Buffer Max Address
    if (paramsG11->presCABACSyntaxStreamOutMaxAddr != nullptr)
    {
        resourceParams.presResource = paramsG11->presCABACSyntaxStreamOutMaxAddr;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.HcpScalabilityCabacDecodedSyntaxElementsBufferMaxAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 119;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpIndObjBaseAddrCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    mhw_vdbox_hcp_g11_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

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
        resourceParams.pdwCmd = (cmd.HcpIndirectBitstreamObjectBaseAddress.DW0_1.Value);
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
            resourceParams.pdwCmd = (cmd.DW6_7.Value);
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
            resourceParams.pdwCmd = (cmd.DW9_10.Value);
            resourceParams.dwLocationInCmd = 9;
            resourceParams.dwSize = MOS_ALIGN_FLOOR(params->dwPakBaseObjectSize, 0x1000);
            resourceParams.bIsWritable = true;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
        }

        if (params->presCompressedHeaderBuffer)
        {
            cmd.HcpVp9PakCompressedHeaderSyntaxStreaminMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_COMPRESSED_HEADER_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presCompressedHeaderBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DW14_15.Value);
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
            resourceParams.dwOffset = params->dwProbabilityCounterOffset;
            resourceParams.pdwCmd = (cmd.DW17_18.Value);
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
            resourceParams.pdwCmd = (cmd.DW20_21.Value);
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
            resourceParams.pdwCmd = (cmd.DW23_24.Value);
            resourceParams.dwLocationInCmd = 23;
            resourceParams.dwSize = params->dwTileRecordSize;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));

        }

        if (params->presPakTileSizeStasBuffer)
        {
            cmd.HcpVp9PakTileRecordStreamoutMemoryAddressAttributes.DW0.Value |=
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SIZE_STREAMOUT_CODEC].Value;

            resourceParams.presResource = params->presPakTileSizeStasBuffer;
            resourceParams.dwOffset = params->dwPakTileSizeRecordOffset;
            resourceParams.pdwCmd = (cmd.DW23_24.Value);
            resourceParams.dwLocationInCmd = 23;
            resourceParams.dwSize = params->dwPakTileSizeStasBufferSize;
            resourceParams.bIsWritable = WRITE_WA;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpDecodePicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcPicParams);

    auto paramsG11 = dynamic_cast<PMHW_VDBOX_HEVC_PIC_STATE_G11>(params);
    MHW_MI_CHK_NULL(paramsG11);
    auto hevcPicParams = paramsG11->pHevcPicParams;
    auto hevcExtPicParams = paramsG11->pHevcExtPicParams;

    if (hevcExtPicParams && hevcExtPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag == 1)
    {
        MHW_ASSERTMESSAGE("HW decoder doesn't support HEVC High Throughput profile so far.");
        MHW_ASSERTMESSAGE("So cabac_bypass_alignment_enabled_flag cannot equal to 1.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g11_X>::AddHcpDecodePicStateCmd(cmdBuffer, params));

    // RExt fields
    cmd->DW2.ChromaSubsampling           = hevcPicParams->chroma_format_idc;
    cmd->DW3.Log2Maxtransformskipsize    = 0x2;
    if (hevcExtPicParams)
    {
        cmd->DW3.Log2Maxtransformskipsize            = hevcExtPicParams->log2_max_transform_skip_block_size_minus2 + 2;
        cmd->DW3.CrossComponentPredictionEnabledFlag = hevcExtPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag;
        cmd->DW3.CabacBypassAlignmentEnabledFlag     = hevcExtPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag;
        cmd->DW3.PersistentRiceAdaptationEnabledFlag = hevcExtPicParams->PicRangeExtensionFlags.fields.persistent_rice_adaptation_enabled_flag;
        cmd->DW3.IntraSmoothingDisabledFlag          = hevcExtPicParams->PicRangeExtensionFlags.fields.intra_smoothing_disabled_flag;
        cmd->DW3.ExplicitRdpcmEnabledFlag            = hevcExtPicParams->PicRangeExtensionFlags.fields.explicit_rdpcm_enabled_flag;
        cmd->DW3.ImplicitRdpcmEnabledFlag            = hevcExtPicParams->PicRangeExtensionFlags.fields.implicit_rdpcm_enabled_flag;
        cmd->DW3.TransformSkipContextEnabledFlag     = hevcExtPicParams->PicRangeExtensionFlags.fields.transform_skip_context_enabled_flag;
        cmd->DW3.TransformSkipRotationEnabledFlag    = hevcExtPicParams->PicRangeExtensionFlags.fields.transform_skip_rotation_enabled_flag;
        cmd->DW3.HighPrecisionOffsetsEnableFlag      = hevcExtPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag;
        cmd->DW2.ChromaQpOffsetListEnabledFlag       = hevcExtPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag;
        cmd->DW2.DiffCuChromaQpOffsetDepth           = hevcExtPicParams->diff_cu_chroma_qp_offset_depth;
        cmd->DW2.ChromaQpOffsetListLenMinus1         = hevcExtPicParams->chroma_qp_offset_list_len_minus1;
        cmd->DW2.Log2SaoOffsetScaleLuma              = hevcExtPicParams->log2_sao_offset_scale_luma;
        cmd->DW2.Log2SaoOffsetScaleChroma            = hevcExtPicParams->log2_sao_offset_scale_chroma;

        cmd->DW32.CbQpOffsetList0 = hevcExtPicParams->cb_qp_offset_list[0];
        cmd->DW32.CbQpOffsetList1 = hevcExtPicParams->cb_qp_offset_list[1];
        cmd->DW32.CbQpOffsetList2 = hevcExtPicParams->cb_qp_offset_list[2];
        cmd->DW32.CbQpOffsetList3 = hevcExtPicParams->cb_qp_offset_list[3];
        cmd->DW32.CbQpOffsetList4 = hevcExtPicParams->cb_qp_offset_list[4];
        cmd->DW32.CbQpOffsetList5 = hevcExtPicParams->cb_qp_offset_list[5];
        cmd->DW33.CrQpOffsetList0 = hevcExtPicParams->cr_qp_offset_list[0];
        cmd->DW33.CrQpOffsetList1 = hevcExtPicParams->cr_qp_offset_list[1];
        cmd->DW33.CrQpOffsetList2 = hevcExtPicParams->cr_qp_offset_list[2];
        cmd->DW33.CrQpOffsetList3 = hevcExtPicParams->cr_qp_offset_list[3];
        cmd->DW33.CrQpOffsetList4 = hevcExtPicParams->cr_qp_offset_list[4];
        cmd->DW33.CrQpOffsetList5 = hevcExtPicParams->cr_qp_offset_list[5];

        if (MEDIA_IS_WA(m_waTable, WaCheckCrossComponentPredictionEnabledFlag))
        {
            MHW_NORMALMESSAGE("cross_component_prediction_enabled_flag:%d\n",
                hevcExtPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag);
            printf("cross_component_prediction_enabled_flag:%d\n",
                hevcExtPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag);
        }
    }

    cmd->DW5.BitDepthChromaMinus8 = hevcPicParams->bit_depth_chroma_minus8;
    cmd->DW5.BitDepthLumaMinus8 = hevcPicParams->bit_depth_luma_minus8;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpEncodePicStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE       params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
    MHW_MI_CHK_NULL(params->pHevcEncPicParams);

    PMHW_BATCH_BUFFER                       batchBuffer = nullptr;
    mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD  cmd;

    auto hevcSeqParams = params->pHevcEncSeqParams;
    auto hevcPicParams = params->pHevcEncPicParams;

    if (params->bBatchBufferInUse)
    {
        MHW_MI_CHK_NULL(params->pBatchBuffer);
        batchBuffer = params->pBatchBuffer;
    }

    cmd.DW1.Framewidthinmincbminus1     = hevcSeqParams->wFrameWidthInMinCbMinus1;
    cmd.DW1.PakTransformSkipEnable      = cmd.DW4.TransformSkipEnabledFlag = hevcPicParams->transform_skip_enabled_flag;
    cmd.DW1.Frameheightinmincbminus1    = hevcSeqParams->wFrameHeightInMinCbMinus1;

    cmd.DW2.Mincusize                   = hevcSeqParams->log2_min_coding_block_size_minus3;
    cmd.DW2.CtbsizeLcusize              = hevcSeqParams->log2_max_coding_block_size_minus3;
    cmd.DW2.Maxtusize                   = hevcSeqParams->log2_max_transform_block_size_minus2;
    cmd.DW2.Mintusize                   = hevcSeqParams->log2_min_transform_block_size_minus2;
    cmd.DW2.Minpcmsize = 0; // Not supported in CNL
    cmd.DW2.Maxpcmsize = 0; // Not supported in CNL

    cmd.DW3.Colpicisi = 0;
    cmd.DW3.Curpicisi = 0;

    cmd.DW4.SampleAdaptiveOffsetEnabledFlag         = params->bSAOEnable;
    cmd.DW4.PcmEnabledFlag                          = 0; // Not supported in CNL
    cmd.DW4.CuQpDeltaEnabledFlag                    = hevcPicParams->cu_qp_delta_enabled_flag; // In VDENC mode, this field should always be set to 1.
    cmd.DW4.DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth  = hevcPicParams->diff_cu_qp_delta_depth;
    cmd.DW4.PcmLoopFilterDisableFlag                = hevcSeqParams->pcm_loop_filter_disable_flag;
    cmd.DW4.ConstrainedIntraPredFlag                = 0;
    cmd.DW4.Log2ParallelMergeLevelMinus2            = 0;
    cmd.DW4.SignDataHidingFlag                      = 0; // currently not supported in encoder
    cmd.DW4.LoopFilterAcrossTilesEnabledFlag        = 0;
    cmd.DW4.EntropyCodingSyncEnabledFlag            = 0; // not supported as per Dimas notes. PAK restriction
    cmd.DW4.TilesEnabledFlag                        = 0; // not supported in encoder
    cmd.DW4.WeightedPredFlag                        = hevcPicParams->weighted_pred_flag;
    cmd.DW4.WeightedBipredFlag                      = hevcPicParams->weighted_bipred_flag;
    cmd.DW4.Fieldpic                                = 0;
    cmd.DW4.Bottomfield                             = 0;
    cmd.DW4.AmpEnabledFlag                          = hevcSeqParams->amp_enabled_flag;
    cmd.DW4.TransquantBypassEnableFlag              = hevcPicParams->transquant_bypass_enabled_flag;
    cmd.DW4.StrongIntraSmoothingEnableFlag          = hevcSeqParams->strong_intra_smoothing_enable_flag;
    cmd.DW4.CuPacketStructure                       = 0; // output from HW VME, 1/2 CL per CU

    cmd.DW5.PicCbQpOffset                           = hevcPicParams->pps_cb_qp_offset & 0x1f;
    cmd.DW5.PicCrQpOffset                           = hevcPicParams->pps_cr_qp_offset & 0x1f;
    cmd.DW5.MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra = hevcSeqParams->max_transform_hierarchy_depth_intra;
    cmd.DW5.MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter = hevcSeqParams->max_transform_hierarchy_depth_inter;
    cmd.DW5.PcmSampleBitDepthChromaMinus1                           = hevcSeqParams->pcm_sample_bit_depth_chroma_minus1;
    cmd.DW5.PcmSampleBitDepthLumaMinus1                             = hevcSeqParams->pcm_sample_bit_depth_luma_minus1;
    cmd.DW5.BitDepthChromaMinus8                                    = hevcSeqParams->bit_depth_chroma_minus8;
    cmd.DW5.BitDepthLumaMinus8                                      = hevcSeqParams->bit_depth_luma_minus8;

    cmd.DW6.LcuMaxBitsizeAllowed                                    = hevcPicParams->LcuMaxBitsizeAllowed;
    cmd.DW6.Nonfirstpassflag                                        = 0;    // needs to be updated for HEVC VDEnc
    cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask                   = 0;
    cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask            = 0;
    cmd.DW6.FrameszunderstatusenFramebitrateminreportmask           = 0;
    cmd.DW6.LoadSlicePointerFlag                                    = 0; // must be set to 0 for encoder

    cmd.DW19.RdoqEnable                                             = params->bHevcRdoqEnabled;
    //only 420 format support SSE in DP encode
    cmd.DW19.SseEnable                                              = params->bUseVDEnc || params->sseEnabledInVmeEncode;
    // only for VDEnc
    cmd.DW19.RhodomainRateControlEnable                             = params->bUseVDEnc || params->rhodomainRCEnable;   // DW19[6]
                                                                // RhoDomainFrameLevelQP: This QP is used for RhoDomain Frame level statistics.
    cmd.DW19.Rhodomainframelevelqp                                  = cmd.DW19.RhodomainRateControlEnable ? hevcPicParams->QpY : 0;  // DW19[13:8]
    cmd.DW19.FractionalQpAdjustmentEnable                           = params->bUseVDEnc;   // DW19[17]

    cmd.DW19.FirstSliceSegmentInPicFlag                             = 1;
    cmd.DW19.Nalunittypeflag                                        = 1;

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

    cmd.DW4.TilesEnabledFlag                 = hevcPicParams->tiles_enabled_flag;
    cmd.DW2.ChromaSubsampling                = hevcSeqParams->chroma_format_idc;
    cmd.DW4.LoopFilterAcrossTilesEnabledFlag = hevcPicParams->loop_filter_across_tiles_flag;

    // Disable HEVC RDOQ for Intra blocks
    cmd.DW20.Intratucountbasedrdoqdisable   = params->bRDOQIntraTUDisable;
    cmd.DW37.Rdoqintratuthreshold           = params->wRDOQIntraTUThreshold;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpTileStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_TILE_STATE       params)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pTileColWidth);
    MHW_MI_CHK_NULL(params->pTileRowHeight);

    mhw_vdbox_hcp_g11_X::HCP_TILE_STATE_CMD     cmd;

    auto hevcPicParams = params->pHevcPicParams;

    MHW_CHK_COND(hevcPicParams->num_tile_rows_minus1 >= HEVC_NUM_MAX_TILE_ROW, "num_tile_rows_minus1 is out of range!");
    MHW_CHK_COND(hevcPicParams->num_tile_columns_minus1 >= HEVC_NUM_MAX_TILE_COLUMN, "num_tile_columns_minus1 is out of range!");

    cmd.DW1.Numtilecolumnsminus1 = hevcPicParams->num_tile_columns_minus1;
    cmd.DW1.Numtilerowsminus1    = hevcPicParams->num_tile_rows_minus1;

    uint32_t column        = hevcPicParams->num_tile_columns_minus1 + 1;
    uint32_t lastDwEleNum  = column % 4;
    uint32_t count       = column / 4;

    for (uint8_t i = 0; i < 5; i++)
    {
        cmd.CtbColumnPositionOfTileColumn[i].DW0.Value = 0;
    }

    for (uint8_t i = 0; i < 6; i++)
    {
        cmd.CtbRowPositionOfTileRow[i].DW0.Value = 0;
    }

    cmd.CtbColumnPositionMsb.DW0.Value = 0;
    cmd.CtbColumnPositionMsb.DW1.Value = 0;
    cmd.CtbRowPositionMsb.DW0.Value = 0;
    cmd.CtbRowPositionMsb.DW1.Value = 0;

    uint32_t colCumulativeValue = 0;
    uint32_t rowCumulativeValue = 0;

    // Column Position
    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t &CtbColumnMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbColumnPositionMsb.DW0.Value : cmd.CtbColumnPositionMsb.DW1.Value;

        cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos0I   = colCumulativeValue & 0xFF;//lower 8bits
        CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                              (((colCumulativeValue >> 8) & 0x3) << ((i*8) + 0));//MSB 2bits
        colCumulativeValue                                 += params->pTileColWidth[4 * i];

        cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos1I   = colCumulativeValue & 0xFF;//lower 8bits
        CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                              (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 2));//MSB 2bits
        colCumulativeValue                                 += params->pTileColWidth[4 * i + 1];

        cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos2I   = colCumulativeValue & 0xFF;//lower 8bits
        CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                              (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 4));//MSB 2bits
        colCumulativeValue                                 += params->pTileColWidth[4 * i + 2];

        cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos3I   = colCumulativeValue & 0xFF;//lower 8bits
        CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                              (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 6));//MSB 2bits
        colCumulativeValue                                 += params->pTileColWidth[4 * i + 3];
    }

    if (lastDwEleNum)
    {
        uint32_t i = count;
        uint32_t &CtbColumnMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbColumnPositionMsb.DW0.Value : cmd.CtbColumnPositionMsb.DW1.Value;

        if (i < 5)
        {
            cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos0I   = colCumulativeValue & 0xFF;//lower 8bits
            CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                                  (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 0));//MSB 2bits
            if (lastDwEleNum > 1)
            {
                colCumulativeValue += params->pTileColWidth[4 * i];
                cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos1I   = colCumulativeValue & 0xFF;//lower 8bits
                CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                                      (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 2));//MSB 2bits

                if (lastDwEleNum > 2)
                {
                    colCumulativeValue += params->pTileColWidth[4 * i + 1];
                    cmd.CtbColumnPositionOfTileColumn[i].DW0.Ctbpos2I   = colCumulativeValue & 0xFF;//lower 8bits
                    CtbColumnMsbValue                                   = CtbColumnMsbValue |
                                                                          (((colCumulativeValue >> 8) & 0x3) << ((i * 8) + 4));//MSB 2bits
                }
            }
        }
    }

    // Row Postion
    uint32_t row = hevcPicParams->num_tile_rows_minus1 + 1;
    lastDwEleNum = row % 4;
    count = row / 4;

    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t &CtbRowMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbRowPositionMsb.DW0.Value : cmd.CtbRowPositionMsb.DW1.Value;

        cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos0I = rowCumulativeValue & 0xFF;//lower 8bits
        CtbRowMsbValue                              = CtbRowMsbValue |
                                                      (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 0));//MSB 2bits
        rowCumulativeValue                         += params->pTileRowHeight[4 * i];

        cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos1I = rowCumulativeValue & 0xFF;//lower 8bits
        CtbRowMsbValue                              = CtbRowMsbValue |
                                                      (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 2));//MSB 2bits
        rowCumulativeValue                         += params->pTileRowHeight[4 * i + 1];

        cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos2I = rowCumulativeValue & 0xFF;//lower 8bits
        CtbRowMsbValue                              = CtbRowMsbValue |
                                                      (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 4));//MSB 2bits
        rowCumulativeValue                         += params->pTileRowHeight[4 * i + 2];

        cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos3I = rowCumulativeValue & 0xFF;//lower 8bits
        CtbRowMsbValue                              = CtbRowMsbValue |
                                                      (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 6));//MSB 2bits
        rowCumulativeValue                         += params->pTileRowHeight[4 * i + 3];
    }

    if (lastDwEleNum)
    {
        uint32_t i = count;
        uint32_t &CtbRowMsbValue = ((i << 3) >> 5) == 0 ? cmd.CtbRowPositionMsb.DW0.Value : cmd.CtbRowPositionMsb.DW1.Value;
        if(i < 6)
        {
            cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos0I = rowCumulativeValue & 0xFF;//lower 8bits
            CtbRowMsbValue                              = CtbRowMsbValue |
                                                          (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 0));//MSB 2bits

            if (lastDwEleNum > 1)
            {
                rowCumulativeValue                         += params->pTileRowHeight[4 * i];
                cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos1I = rowCumulativeValue & 0xFF;//lower 8bits
                CtbRowMsbValue                              = CtbRowMsbValue |
                                                              (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 2));//MSB 2bits

                if (lastDwEleNum > 2)
                {
                    rowCumulativeValue                         += params->pTileRowHeight[4 * i + 1];
                    cmd.CtbRowPositionOfTileRow[i].DW0.Ctbpos2I = rowCumulativeValue & 0xFF;//lower 8bits
                    CtbRowMsbValue                              = CtbRowMsbValue |
                                                                  (((rowCumulativeValue >> 8) & 0x3) << ((i * 8) + 4));//MSB 2bits
                }
            }
        }
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpWeightOffsetStateCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_BATCH_BUFFER                    batchBuffer,
    PMHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_WEIGHTOFFSET_STATE_CMD cmd;

    uint8_t i = 0;
    uint8_t refIdx = 0;

    cmd.DW1.Refpiclistnum = i = params->ucList;

    // Luma
    for (refIdx = 0; refIdx < CODEC_MAX_NUM_REF_FRAME_HEVC; refIdx++)
    {
        cmd.Lumaoffsets[refIdx].DW0.DeltaLumaWeightLxI  = params->LumaWeights[i][refIdx];
        cmd.Lumaoffsets[refIdx].DW0.LumaOffsetLxI       = (char)(params->LumaOffsets[i][refIdx] & 0xFF);//lower 8bits
        cmd.Lumaoffsets[refIdx].DW0.LumaOffsetLxIMsbyte = (char)((params->LumaOffsets[i][refIdx] >> 8) & 0xFF);//MSB 8bits
    }

    // Chroma
    for (refIdx = 0; refIdx < CODEC_MAX_NUM_REF_FRAME_HEVC; refIdx++)
    {
        //Cb
        cmd.Chromaoffsets[refIdx].DW0.DeltaChromaWeightLxI0 = params->ChromaWeights[i][refIdx][0];
        cmd.Chromaoffsets[refIdx].DW0.ChromaoffsetlxI0      = (char)(params->ChromaOffsets[i][refIdx][0] & 0xFF);//lower 8bits

        //Cr
        cmd.Chromaoffsets[refIdx].DW0.DeltaChromaWeightLxI1 = params->ChromaWeights[i][refIdx][1];
        cmd.Chromaoffsets[refIdx].DW0.ChromaoffsetlxI1      = (char)(params->ChromaOffsets[i][refIdx][1] & 0xFF);//lower 8bits
    }

    for (refIdx = 0; refIdx < CODEC_MAX_NUM_REF_FRAME_HEVC - 1; refIdx += 2)//MSB 8bits
    {
        cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI0Msbyte     = (char)((params->ChromaOffsets[i][refIdx][0] >> 8) & 0xFF);
        cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI10Msbyte    = (char)((params->ChromaOffsets[i][refIdx + 1][0] >> 8) & 0xFF);
        cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI1Msbyte     = (char)((params->ChromaOffsets[i][refIdx][1] >> 8) & 0xFF);
        cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI11Msbyte    = (char)((params->ChromaOffsets[i][refIdx + 1][1] >> 8) & 0xFF);
    }

    //last one
    cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI0Msbyte         = (char)((params->ChromaOffsets[i][refIdx][0] >> 8) & 0xFF);//MSB 8bits
    cmd.Chromaoffsetsext[refIdx >> 1].DW0.ChromaoffsetlxI1Msbyte         = (char)((params->ChromaOffsets[i][refIdx][1] >> 8) & 0xFF);//MSB 8bits

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpFqmStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_QM_PARAMS             params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_FQM_STATE_CMD cmd;

    if (params->Standard == CODECHAL_HEVC)
    {
        MHW_MI_CHK_NULL(params->pHevcIqMatrix);

        auto iqMatrix = params->pHevcIqMatrix;
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

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpDecodeSliceStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcSliceParams);

    auto hevcSliceStateG11  = dynamic_cast<PMHW_VDBOX_HEVC_SLICE_STATE_G11>(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceStateG11);
    auto hevcSliceParams    = hevcSliceState->pHevcSliceParams;
    auto hevcExtSliceParams = hevcSliceStateG11->pHevcExtSliceParams;
    auto hevcPicParams      = hevcSliceState->pHevcPicParams;
    auto hevcExtPicParams   = hevcSliceStateG11->pHevcExtPicParam;

    uint32_t ctbSize      = 1 << (hevcPicParams->log2_diff_max_min_luma_coding_block_size +
                            hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    uint32_t widthInPix   = (1 << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3)) *
                            (hevcPicParams->PicWidthInMinCbsY);
    uint32_t widthInCtb   = MOS_ROUNDUP_DIVIDE(widthInPix, ctbSize);

    mhw_vdbox_hcp_g11_X::HCP_SLICE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g11_X::HCP_SLICE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g11_X>::AddHcpDecodeSliceStateCmd(cmdBuffer, hevcSliceState));

    int32_t sliceQP = hevcSliceParams->slice_qp_delta + hevcPicParams->init_qp_minus26 + 26;
    cmd->DW3.SliceqpSignFlag = (sliceQP >= 0) ? 0 : 1;
    cmd->DW3.Sliceqp = ABS(sliceQP);

    cmd->DW1.SlicestartctbxOrSliceStartLcuXEncoder = hevcSliceParams->slice_segment_address % widthInCtb;
    cmd->DW1.SlicestartctbyOrSliceStartLcuYEncoder = hevcSliceParams->slice_segment_address / widthInCtb;

    if (hevcExtPicParams && hevcExtSliceParams)
    {
        // DW3[23]
        if (hevcExtPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag)
        {
            cmd->DW3.CuChromaQpOffsetEnabledFlag = hevcExtSliceParams->cu_chroma_qp_offset_enabled_flag;
        }

        // DW3[24:25]
        cmd->DW3.Lastsliceoftile = hevcSliceState->bLastSliceInTile;
        cmd->DW3.Lastsliceoftilecolumn = hevcSliceState->bLastSliceInTileColumn;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpEncodeSliceStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcSliceParams);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcSeqParams);

    mhw_vdbox_hcp_g11_X::HCP_SLICE_STATE_CMD    cmd;
    MOS_COMMAND_BUFFER                          constructedCmdBuf;

    auto hevcSliceParams = hevcSliceState->pEncodeHevcSliceParams;
    auto hevcPicParams = hevcSliceState->pEncodeHevcPicParams;
    auto hevcSeqParams = hevcSliceState->pEncodeHevcSeqParams;

    uint32_t ctbSize       = 1 << (hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t widthInPix    = (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) *
        (hevcSeqParams->wFrameWidthInMinCbMinus1 + 1);
    uint32_t widthInCtb    = (widthInPix / ctbSize) +
        ((widthInPix % ctbSize) ? 1 : 0);  // round up

    uint32_t ctbAddr       = hevcSliceParams->slice_segment_address;
    cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder   = ctbAddr % widthInCtb;
    cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder   = ctbAddr / widthInCtb;

    if (hevcSliceState->bLastSlice)
    {
        cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder         = 0;
        cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder         = 0;
    }
    else
    {
        ctbAddr = hevcSliceParams->slice_segment_address + hevcSliceParams->NumLCUsInSlice;
        cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder         = ctbAddr % widthInCtb;
        cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder         = ctbAddr / widthInCtb;
    }

    cmd.DW3.SliceType                   = hevcSliceParams->slice_type;
    cmd.DW3.Lastsliceofpic              = hevcSliceState->bLastSlice;
    cmd.DW3.SliceqpSignFlag             = ((hevcSliceParams->slice_qp_delta + hevcPicParams->QpY) >= 0)
        ? 0 : 1; //8 bit will have 0 as sign bit adn 10 bit might have 1 as sign bit depending on Qp
    cmd.DW3.DependentSliceFlag          = 0; // Not supported on encoder
    cmd.DW3.SliceTemporalMvpEnableFlag  = hevcSliceParams->slice_temporal_mvp_enable_flag;
    cmd.DW3.Sliceqp                     = abs(hevcSliceParams->slice_qp_delta + hevcPicParams->QpY);
    cmd.DW3.SliceCbQpOffset             = hevcSliceParams->slice_cb_qp_offset;
    cmd.DW3.SliceCbQpOffset             = hevcSliceParams->slice_cr_qp_offset;
    cmd.DW3.Intrareffetchdisable        = hevcSliceState->bIntraRefFetchDisable;

    cmd.DW4.SliceHeaderDisableDeblockingFilterFlag          = hevcSliceParams->slice_deblocking_filter_disable_flag;
    cmd.DW4.SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder     = hevcSliceParams->tc_offset_div2;
    cmd.DW4.SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder = hevcSliceParams->beta_offset_div2;
    cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag          = 0;
    cmd.DW4.SliceSaoChromaFlag                              = hevcSliceParams->slice_sao_chroma_flag;
    cmd.DW4.SliceSaoLumaFlag                                = hevcSliceParams->slice_sao_luma_flag;
    cmd.DW4.MvdL1ZeroFlag                                   = 0; // Decoder only - set to 0 for encoder
    cmd.DW4.Islowdelay                                      = hevcSliceState->bIsLowDelay;
    cmd.DW4.CollocatedFromL0Flag                            = hevcSliceParams->collocated_from_l0_flag;
    cmd.DW4.Chromalog2Weightdenom                           = (hevcPicParams->weighted_pred_flag || hevcPicParams->weighted_bipred_flag) ?
                                                              (hevcPicParams->bEnableGPUWeightedPrediction ? 6 : hevcSliceParams->luma_log2_weight_denom + hevcSliceParams->delta_chroma_log2_weight_denom) : 0;
    cmd.DW4.LumaLog2WeightDenom                             = (hevcPicParams->weighted_pred_flag || hevcPicParams->weighted_bipred_flag) ?
                                                              (hevcPicParams->bEnableGPUWeightedPrediction ? 6 : hevcSliceParams->luma_log2_weight_denom) : 0;
    cmd.DW4.CabacInitFlag                                   = hevcSliceParams->cabac_init_flag;
    cmd.DW4.Maxmergeidx                                     = hevcSliceParams->MaxNumMergeCand - 1;

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

            cmd.DW4.Collocatedrefidx    = collocatedFrameIdx;
        }
    }
    else
    {
        cmd.DW4.Collocatedrefidx    = 0;
    }

    cmd.DW5.Sliceheaderlength       = 0; // Decoder only, setting to 0 for Encoder

    // Currently setting to defaults used in prototype
    cmd.DW6.Roundinter = 4;
    cmd.DW6.Roundintra = 10;

    cmd.DW7.Cabaczerowordinsertionenable    = hevcSliceState->bVdencInUse ? 0 : 1;
    cmd.DW7.Emulationbytesliceinsertenable  = 1;
    cmd.DW7.TailInsertionEnable             = (hevcPicParams->bLastPicInSeq || hevcPicParams->bLastPicInStream) && hevcSliceState->bLastSlice;
    cmd.DW7.SlicedataEnable                 = 1;
    cmd.DW7.HeaderInsertionEnable           = 1;

    cmd.DW8.IndirectPakBseDataStartOffsetWrite = hevcSliceState->dwHeaderBytesInserted;

    // Transform skip related parameters
    if (hevcPicParams->transform_skip_enabled_flag)
    {
        cmd.DW9.TransformskipLambda                     = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_lambda;
        cmd.DW10.TransformskipNumzerocoeffsFactor0      = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numzerocoeffs_Factor0;
        cmd.DW10.TransformskipNumnonzerocoeffsFactor0   = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numnonzerocoeffs_Factor0;
        cmd.DW10.TransformskipNumzerocoeffsFactor1      = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numzerocoeffs_Factor1;
        cmd.DW10.TransformskipNumnonzerocoeffsFactor1   = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numnonzerocoeffs_Factor1;
    }

    if (hevcSliceState->bLastSlice)
    {
        cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder         = 0;
        cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder         = 0;
    }
    else
    {
        if(hevcPicParams->tiles_enabled_flag)
        {
            // when tile is enabled, need to consider if slice is within one tile
            cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder         = hevcSliceParams[1].slice_segment_address % widthInCtb;
            cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder         = hevcSliceParams[1].slice_segment_address / widthInCtb;
        }
        else
        {
            ctbAddr                                                       = hevcSliceParams->slice_segment_address + hevcSliceParams->NumLCUsInSlice;
            cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder           = ctbAddr % widthInCtb;
            cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder           = ctbAddr / widthInCtb;
        }
    }


    cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag = hevcPicParams->loop_filter_across_slices_flag;
    cmd.DW3.Lastsliceoftile          = hevcSliceState->bLastSliceInTile;
    cmd.DW3.Lastsliceoftilecolumn    = hevcSliceState->bLastSliceInTileColumn;

    // Currently setting to defaults used in prototype
    cmd.DW6.Roundinter = 4;
    cmd.DW6.Roundintra = 10;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, hevcSliceState->pBatchBufferForPakSlices, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpPakInsertObject(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_PAK_INSERT_PARAMS     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_PAK_INSERT_OBJECT_CMD  cmd;

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

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpVp9PicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_PIC_STATE         params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);

    mhw_vdbox_hcp_g11_X::HCP_VP9_PIC_STATE_CMD cmd;
    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList = params->ppVp9RefList;

    cmd.DW0.DwordLength                         = mhw_vdbox_hcp_g11_X::GetOpLength(12); //VP9_PIC_STATE command is common for both Decoder and Encoder. Decoder uses only 12 DWORDS of the generated 33 DWORDS

    uint32_t curFrameWidth                      = vp9PicParams->FrameWidthMinus1 + 1;
    uint32_t curFrameHeight                     = vp9PicParams->FrameHeightMinus1 + 1;

    cmd.DW1.FrameWidthInPixelsMinus1            = MOS_ALIGN_CEIL(curFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    cmd.DW1.FrameHeightInPixelsMinus1           = MOS_ALIGN_CEIL(curFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    cmd.DW2.FrameType                           = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag              = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag                       = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.RefreshFrameContext                 = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode                  = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode           = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                         = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel                      = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled                 = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap               = cmd.DW2.SegmentationEnabled && vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.LosslessMode                        = vp9PicParams->PicFlags.fields.LosslessFlag;
    cmd.DW2.SegmentIdStreamoutEnable            = cmd.DW2.SegmentationUpdateMap;

    cmd.DW3.Log2TileRow                    = vp9PicParams->log2_tile_rows;        // No need to minus 1 here.
    cmd.DW3.Log2TileColumn                 = vp9PicParams->log2_tile_columns;     // No need to minus 1 here.
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
        PCODEC_PICTURE refFrameList   = &(vp9PicParams->RefFrameList[0]);

        uint8_t lastRefPicIndex       = refFrameList[vp9PicParams->PicFlags.fields.LastRefIdx].FrameIdx;
        uint32_t lastRefFrameWidth    = vp9RefList[lastRefPicIndex]->dwFrameWidth;
        uint32_t lastRefFrameHeight   = vp9RefList[lastRefPicIndex]->dwFrameHeight;

        uint8_t goldenRefPicIndex     = refFrameList[vp9PicParams->PicFlags.fields.GoldenRefIdx].FrameIdx;
        uint32_t goldenRefFrameWidth  = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
        uint32_t goldenRefFrameHeight = vp9RefList[goldenRefPicIndex]->dwFrameHeight;

        uint8_t altRefPicIndex        = refFrameList[vp9PicParams->PicFlags.fields.AltRefIdx].FrameIdx;
        uint32_t altRefFrameWidth     = vp9RefList[altRefPicIndex]->dwFrameWidth;
        uint32_t altRefFrameHeight    = vp9RefList[altRefPicIndex]->dwFrameHeight;

        bool isScaling                = (curFrameWidth == params->dwPrevFrmWidth) && (curFrameHeight == params->dwPrevFrmHeight) ?
                                                    false : true;

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

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpVp9PicStateEncCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_VDBOX_VP9_ENCODE_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);
    MHW_MI_CHK_NULL(params->pVp9SeqParams);
    MHW_MI_CHK_NULL(params->ppVp9RefList);

    mhw_vdbox_hcp_g11_X::HCP_VP9_PIC_STATE_CMD cmd;

    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList = params->ppVp9RefList;
    auto vp9SeqParams = params->pVp9SeqParams;

    cmd.DW1.FrameWidthInPixelsMinus1    = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameWidthMinus1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    cmd.DW1.FrameHeightInPixelsMinus1   = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameHeightMinus1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    cmd.DW2.FrameType                   = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag      = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag               = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.AllowHiPrecisionMv          = vp9PicParams->PicFlags.fields.allow_high_precision_mv;
    cmd.DW2.McompFilterType             = vp9PicParams->PicFlags.fields.mcomp_filter_type;

    cmd.DW2.RefFrameSignBias02          = vp9PicParams->RefFlags.fields.LastRefSignBias |
                                          (vp9PicParams->RefFlags.fields.GoldenRefSignBias << 1) |
                                          (vp9PicParams->RefFlags.fields.AltRefSignBias << 2);

    cmd.DW2.HybridPredictionMode        = vp9PicParams->PicFlags.fields.comp_prediction_mode == 2;
    cmd.DW2.SelectableTxMode            = params->ucTxMode == 4;
    cmd.DW2.RefreshFrameContext         = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode          = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode   = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                 = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel              = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled         = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap       = vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.SegmentationTemporalUpdate  = vp9PicParams->PicFlags.fields.segmentation_temporal_update;
    cmd.DW2.LosslessMode                = vp9PicParams->PicFlags.fields.LosslessFlag;

    cmd.DW3.Log2TileColumn              = vp9PicParams->log2_tile_columns;
    cmd.DW3.Log2TileRow                 = vp9PicParams->log2_tile_rows;
    cmd.DW3.SseEnable                   = params->bSSEEnable;
    // Gen 11 REXT inputs for chroma formats and bit depth
    cmd.DW3.ChromaSamplingFormat        = vp9SeqParams->SeqFlags.fields.EncodedFormat;
    switch (vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
    {
        case VP9_ENCODED_BIT_DEPTH_8:
            cmd.DW3.Bitdepthminus8 = 0;
            break;
        case VP9_ENCODED_BIT_DEPTH_10:
            cmd.DW3.Bitdepthminus8 = 2;
            break;
        default:
            cmd.DW3.Bitdepthminus8 = 0;
            break;
    }

    if (vp9PicParams->PicFlags.fields.frame_type && !vp9PicParams->PicFlags.fields.intra_only)
    {
        uint32_t curFrameWidth              = vp9PicParams->SrcFrameWidthMinus1 + 1;
        uint32_t curFrameHeight             = vp9PicParams->SrcFrameHeightMinus1 + 1;

        PCODEC_PICTURE refFrameList         = &(vp9PicParams->RefFrameList[0]);

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

            uint8_t lastRefPicIndex = refFrameList[vp9PicParams->RefFlags.fields.LastRefIdx].FrameIdx;
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

            cmd.DW5.HorizontalScaleFactorForGolden      = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW5.VerticalScaleFactorForGolden        = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW8.GoldenFrameWidthInPixelsMinus1      = goldenRefFrameWidth - 1;
            cmd.DW8.GoldenFrameHieghtInPixelsMinus1     = goldenRefFrameHeight - 1;
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

    cmd.DW13.BaseQIndexSameAsLumaAc = vp9PicParams->LumaACQIndex;
    cmd.DW13.HeaderInsertionEnable  = 1;

    cmd.DW14.ChromaacQindexdelta = Convert2SignMagnitude(vp9PicParams->ChromaACQIndexDelta, 5);
    cmd.DW14.ChromadcQindexdelta = Convert2SignMagnitude(vp9PicParams->ChromaDCQIndexDelta, 5);
    cmd.DW14.LumaDcQIndexDelta   = Convert2SignMagnitude(vp9PicParams->LumaDCQIndexDelta, 5);

    cmd.DW15.LfRefDelta0 = Convert2SignMagnitude(vp9PicParams->LFRefDelta[0], 7);
    cmd.DW15.LfRefDelta1 = Convert2SignMagnitude(vp9PicParams->LFRefDelta[1], 7);
    cmd.DW15.LfRefDelta2 = Convert2SignMagnitude(vp9PicParams->LFRefDelta[2], 7);
    cmd.DW15.LfRefDelta3 = Convert2SignMagnitude(vp9PicParams->LFRefDelta[3], 7);

    cmd.DW16.LfModeDelta0 = Convert2SignMagnitude(vp9PicParams->LFModeDelta[0], 7);
    cmd.DW16.LfModeDelta1 = Convert2SignMagnitude(vp9PicParams->LFModeDelta[1], 7);

    cmd.DW17.Bitoffsetforlfrefdelta         = vp9PicParams->BitOffsetForLFRefDelta;
    cmd.DW17.Bitoffsetforlfmodedelta        = vp9PicParams->BitOffsetForLFModeDelta;
    cmd.DW18.Bitoffsetforlflevel            = vp9PicParams->BitOffsetForLFLevel;
    cmd.DW18.Bitoffsetforqindex             = vp9PicParams->BitOffsetForQIndex;
    cmd.DW32.Bitoffsetforfirstpartitionsize = vp9PicParams->BitOffsetForFirstPartitionSize;

    cmd.DW19.VdencPakOnlyPass               = params->bVdencPakOnlyPassFlag;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpVp9SegmentStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_SEGMENT_STATE     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_VP9_SEGMENT_STATE_CMD cmd;

    cmd.DW1.SegmentId = params->ucCurrentSegmentId;

    if (!m_decodeInUse)
    {
        CODEC_VP9_ENCODE_SEG_PARAMS             vp9SegData;

        vp9SegData = params->pVp9EncodeSegmentParams->SegData[params->ucCurrentSegmentId];

        cmd.DW2.SegmentSkipped                      = vp9SegData.SegmentFlags.fields.SegmentSkipped;
        cmd.DW2.SegmentReference                    = vp9SegData.SegmentFlags.fields.SegmentReference;
        cmd.DW2.SegmentReferenceEnabled             = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

        cmd.DW7.SegmentLfLevelDeltaEncodeModeOnly   = Convert2SignMagnitude(vp9SegData.SegmentLFLevelDelta, 7);
        cmd.DW7.SegmentQindexDeltaEncodeModeOnly    = Convert2SignMagnitude(vp9SegData.SegmentQIndexDelta, 9);
    }
    else
    {
        CODEC_VP9_SEG_PARAMS                    vp9SegData;

        vp9SegData = params->pVp9SegmentParams->SegData[params->ucCurrentSegmentId];

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

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpHevcVp9RdoqStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcEncSeqParams);

    mhw_vdbox_hcp_g11_X::HEVC_VP9_RDOQ_STATE_CMD    cmd;
    uint16_t                                        lambdaTab[2][2][64];

    MHW_MI_CHK_NULL(params->pHevcEncPicParams);

    uint32_t sliceTypeIdx = (params->pHevcEncPicParams->CodingType == I_TYPE) ? 0 : 1;

    //Intra lambda
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
        cmd.Intralumalambda[i].DW0.Lambdavalue0     = lambdaTab[0][0][i * 2];
        cmd.Intralumalambda[i].DW0.Lambdavalue1     = lambdaTab[0][0][i * 2 + 1];

        cmd.Intrachromalambda[i].DW0.Lambdavalue0   = lambdaTab[0][1][i * 2];
        cmd.Intrachromalambda[i].DW0.Lambdavalue1   = lambdaTab[0][1][i * 2 + 1];

        cmd.Interlumalambda[i].DW0.Lambdavalue0     = lambdaTab[1][0][i * 2];
        cmd.Interlumalambda[i].DW0.Lambdavalue1     = lambdaTab[1][0][i * 2 + 1];

        cmd.Interchromalambda[i].DW0.Lambdavalue0   = lambdaTab[1][1][i * 2];
        cmd.Interchromalambda[i].DW0.Lambdavalue1   = lambdaTab[1][1][i * 2 + 1];
    }

    if (m_hevcRDOQPerfDisabled)
    {
        cmd.DW1.DisableHtqPerformanceFix1 = true;
        cmd.DW1.DisableHtqPerformanceFix0 = true;
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS  MhwVdboxHcpInterfaceG11::AddHcpDecodeTileCodingCmd(
    PMOS_COMMAND_BUFFER                   cmdBuffer,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_TILE_CODING_CMD cmd;
    MHW_RESOURCE_PARAMS                      resourceParams;
    MEDIA_SYSTEM_INFO  *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    uint8_t          numVdbox = (uint8_t)gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled;

    MHW_ASSERT(params->ucNumDecodePipes <= numVdbox);
    MHW_ASSERT(params->ucPipeIdx < params->ucNumDecodePipes);

    cmd.DW1.NumberOfActiveBePipes    = params->ucNumDecodePipes;
    cmd.DW1.NumOfTileColumnsInAFrame = params->ucNumDecodePipes;
    cmd.DW2.TileRowPosition          = 0;
    cmd.DW2.TileColumnPosition       = params->TileStartLCUX;
    cmd.DW3.Tileheightinmincbminus1  = params->TileHeightInMinCbMinus1;
    cmd.DW3.Tilewidthinmincbminus1   = params->TileWidthInMinCbMinus1;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS  MhwVdboxHcpInterfaceG11::AddHcpEncodeTileCodingCmd(
    PMOS_COMMAND_BUFFER                   cmdBuffer,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g11_X::HCP_TILE_CODING_CMD cmd;

    cmd.DW1.NumberOfActiveBePipes    = params->NumberOfActiveBePipes;
    cmd.DW1.NumOfTileColumnsInAFrame = params->NumOfTileColumnsInFrame; //This field is not used by HW. This field should be same as "Number of Active BE Pipes".
    cmd.DW2.TileColumnPosition       = params->TileStartLCUX;
    cmd.DW2.TileRowPosition          = params->TileStartLCUY;
    cmd.DW2.Islasttileofcolumn       = params->IsLastTileofColumn;
    cmd.DW3.Tileheightinmincbminus1  = params->TileHeightInMinCbMinus1;
    cmd.DW3.Tilewidthinmincbminus1   = params->TileWidthInMinCbMinus1;

    cmd.DW8.CuRecordOffset           = params->CuRecordOffset;
    cmd.DW4.BitstreamByteOffset      = params->BitstreamByteOffset;
    cmd.DW5.PakFrameStatisticsOffset = params->PakTileStatisticsOffset;
    cmd.DW6.CuLevelStreamoutOffset   = params->CuLevelStreamoutOffset;
    cmd.DW7.SliceSizeStreamoutOffset = params->SliceSizeStreamoutOffset;
    cmd.DW9.SseRowstoreOffset        = params->SseRowstoreOffset;
    cmd.DW10.SaoRowstoreOffset       = params->SaoRowstoreOffset;
    cmd.DW11.TileSizeStreamoutOffset = params->TileSizeStreamoutOffset;
    cmd.DW12.Vp9ProbabilityCounterStreamoutOffset = params->Vp9ProbabilityCounterStreamoutOffset;

    if (params->presHcpSyncBuffer) // this buffer is not used in HEVC/VP9 decode or encode
    {
        MHW_RESOURCE_PARAMS                 resourceParams;

        cmd.HcpScalabilitySynchronizeBufferAttributes.DW0.Value   |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum         = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType    = MOS_MFX_PIPE_BUF_ADDR;
        resourceParams.presResource     = params->presHcpSyncBuffer;
        resourceParams.dwOffset         = 0;
        resourceParams.pdwCmd           = (cmd.HcpScalabilitySynchronizeBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd  = 13;
        resourceParams.bIsWritable      = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpHevcPicBrcBuffer(
    PMOS_RESOURCE                   hcpImgStates,
    PMHW_VDBOX_HEVC_PIC_STATE        hevcPicState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpImgStates);

    MOS_COMMAND_BUFFER                      constructedCmdBuf;
    mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD  cmd;
    uint32_t*                               insertion = nullptr;
    MOS_LOCK_PARAMS                         lockFlags;
    m_brcNumPakPasses = hevcPicState->brcNumPakPasses;

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, hcpImgStates, &lockFlags);
    MHW_MI_CHK_NULL(data);

    constructedCmdBuf.pCmdBase      = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr       = (uint32_t *)data;
    constructedCmdBuf.iOffset       = 0;
    constructedCmdBuf.iRemaining    = BRC_IMG_STATE_SIZE_PER_PASS_G11 * (m_brcNumPakPasses);

    MHW_MI_CHK_STATUS(AddHcpPicStateCmd(&constructedCmdBuf, hevcPicState));

    cmd = *(mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD *)data;

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

        *(mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD *)data     = cmd;

        /* add batch buffer end insertion flag */
        insertion = (uint32_t*)(data + mhw_vdbox_hcp_g11_X::HCP_PIC_STATE_CMD::byteSize);
        *insertion = 0x05000000;

        data += BRC_IMG_STATE_SIZE_PER_PASS_G11;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, hcpImgStates));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::GetOsResLaceOrAceOrRgbHistogramBufferSize(
    uint32_t                        width,
    uint32_t                        height,
    uint32_t                       *size)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    *size  = m_veboxRgbHistogramSize;
    *size += m_veboxRgbAceHistogramSizeReserved;

    uint32_t sizeLace = MOS_ROUNDUP_DIVIDE(height, 64) *
        MOS_ROUNDUP_DIVIDE(width, 64)  *
        m_veboxLaceHistogram256BinPerBlock;

    uint32_t sizeNoLace = m_veboxAceHistogramSizePerFramePerSlice *
        m_veboxNumFramePreviousCurrent                   *
        m_veboxMaxSlices;

    *size += MOS_MAX(sizeLace, sizeNoLace);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG11::GetOsResStatisticsOutputBufferSize(
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

MOS_STATUS MhwVdboxHcpInterfaceG11::AddHcpTileCodingCmd(
    PMOS_COMMAND_BUFFER                   cmdBuffer,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddHcpDecodeTileCodingCmd(cmdBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddHcpEncodeTileCodingCmd(cmdBuffer, params));
    }

    return eStatus;
}
