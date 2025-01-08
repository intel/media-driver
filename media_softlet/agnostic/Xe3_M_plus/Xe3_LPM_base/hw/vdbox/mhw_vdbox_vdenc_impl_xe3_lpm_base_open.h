/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_impl_xe3_lpm_base_open.h
//! \brief    VDENC interface for xe3_lpm+
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE3_LPM_BASE_OPEN_H__
#define __MHW_VDBOX_VDENC_IMPL_XE3_LPM_BASE_OPEN_H__

#ifndef _MEDIA_RESERVED

#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe3_lpm_base
{
template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _VDENC_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);
    MmioRegistersVdbox m_mmioRegisters[MHW_VDBOX_NODE_MAX] = {};  //!< Mfx mmio registers

public:

    MOS_STATUS SetRowstoreCachingOffsets(const RowStorePar &par) override
    {
        MHW_FUNCTION_ENTER;

        switch (par.mode)
        {
        case RowStorePar::AVC:
        {
            if (this->m_rowStoreCache.vdenc.supported)
            {
                this->m_rowStoreCache.vdenc.enabled   = true;
                this->m_rowStoreCache.vdenc.dwAddress = !par.isField ? 1280 : 1536;
            }
            if (this->m_rowStoreCache.ipdl.supported)
            {
                this->m_rowStoreCache.ipdl.enabled   = true;
                this->m_rowStoreCache.ipdl.dwAddress = 512;
            }

            break;
        }
        case RowStorePar::HEVC:
        {
            constexpr bool enable[16][5] =
            {
                {1, 1, 1, 0, 1}, {1, 1, 1, 1, 1}, {1, 1, 0, 0, 0}, {1, 1, 0, 1, 0},
                {1, 1, 1, 1, 1}, {1, 1, 0, 0, 1}, {1, 1, 1, 0, 0}, {1, 0, 1, 0, 1},
                {1, 1, 1, 0, 0}, {1, 0, 1, 0, 1}, {1, 1, 1, 1, 1}, {1, 1, 0, 1, 1},
                {1, 1, 1, 1, 1}, {1, 0, 1, 1, 1}, {1, 1, 1, 1, 1}, {1, 0, 1, 1, 1}
            };

            constexpr uint32_t address[16][5] =
            {
                {0, 256, 1280, 0, 2048}, {0, 256, 1280, 1824, 1792}, {0, 512, 0, 0, 0}, {0, 256, 0, 2304, 0},
                {0, 256, 1024, 0, 1792}, {0, 512, 0, 0, 2048}, {0, 256, 1792, 0, 0}, {0, 0, 512, 0, 2048},
                {0, 256, 1792, 0, 0}, {0, 0, 256, 0, 1792}, {0, 256, 1024, 1568, 1536}, {0, 512, 0, 2112, 2048},
                {0, 256, 1792, 2336, 2304}, {0, 0, 512, 1600, 1536}, {0, 128, 1664, 2336, 2304}, {0, 0, 256, 1600, 1536}
            };

            bool     isLcu32or64 = par.lcuSize == RowStorePar::SIZE_32 || par.lcuSize == RowStorePar::SIZE_64;
            bool     isGt4k      = par.frameWidth > 4096;
            bool     isGt8k      = par.frameWidth > 8192;
            uint32_t index       = 0;

            if (par.format != RowStorePar::YUV444)
            {
                index = 2 * isGt4k + isLcu32or64;
            }
            else
            {
                uint32_t subidx = par.bitDepth == RowStorePar::DEPTH_12 ? 2 : (par.bitDepth == RowStorePar::DEPTH_10 ? 1 : 0);
                index           = 4 + 6 * isLcu32or64 + 2 * subidx + isGt4k;
            }

            if (!isGt8k && this->m_rowStoreCache.vdenc.supported)
            {
                this->m_rowStoreCache.vdenc.enabled = enable[index][3];
                if (this->m_rowStoreCache.vdenc.enabled)
                {
                    this->m_rowStoreCache.vdenc.dwAddress = address[index][3];
                }
            }

            break;
        }
        case RowStorePar::AV1:
        {
            if (this->m_rowStoreCache.vdenc.supported)
            {
                this->m_rowStoreCache.vdenc.enabled   = true;
                this->m_rowStoreCache.vdenc.dwAddress = 2370;
            }
            if (this->m_rowStoreCache.ipdl.supported)
            {
                this->m_rowStoreCache.ipdl.enabled   = true;
                this->m_rowStoreCache.ipdl.dwAddress = 384;
            }

            break;
        }
        case RowStorePar::VP9:
        {
            // HVD, Meta/MV, DeBlock, VDEnc
            const bool enableVP9[13][4] =
            {
            { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 1, 1, 0, 1 },
            { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 0 }, { 1, 1, 0, 1 },
            { 1, 1, 1, 1 }, { 1, 1, 0, 1 }, { 1, 1, 1, 1 }, { 1, 1, 0, 1 },
            { 1, 1, 0, 1 }
            };

            const uint32_t addressVP9[13][4] =
            {
            { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,  64, 2368, }, { 0, 128,   0,  768, },
            { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,   0,    0, }, { 0, 128,   0,  768, },
            { 0,  64, 384, 2112, }, { 0, 128,   0,  768, }, { 0,  32, 192, 1920, }, { 0, 128,   0,  768, },
            { 0, 128,   0,  768, }
            };

            if(this->m_rowStoreCache.vdenc.supported)
            {
                bool     is8bit      = par.bitDepth == RowStorePar::DEPTH_8;
                bool     isGt2k      = par.frameWidth > 2048;
                bool     isGt4k      = par.frameWidth > 4096;
                bool     isGt8k      = par.frameWidth > 8192;
                uint32_t index       = 0;

                if((par.format >= RowStorePar::YUV420) && (par.format <= RowStorePar::YUV444))
                {
                    index = 4 * (par.format - RowStorePar::YUV420) + 2 * (!is8bit) + isGt4k;
                }
                else
                {
                    return MOS_STATUS_SUCCESS;
                }

                if(par.format == RowStorePar::YUV444 && !is8bit)
                {
                    index += isGt2k;
                }

                if(!isGt8k)
                {
                    this->m_rowStoreCache.vdenc.enabled = enableVP9[index][3];
                    if(this->m_rowStoreCache.vdenc.enabled)
                    {
                        this->m_rowStoreCache.vdenc.dwAddress = addressVP9[index][3];
                    }
                }
            }
        }
        default:
        {
            break;
        }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);

        size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

    bool IsPerfModeSupported() override
    {
        return m_perfModeSupported;
    }

    bool IsRhoDomainStatsEnabled() override
    {
        return m_rhoDomainStatsEnabled;
    }

    MmioRegistersVdbox *GetMmioRegisters(MHW_VDBOX_NODE_IND index) override
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }
private:
    //VDBOX register offsets
    static constexpr uint32_t MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT           = 0x1C08B4;
    static constexpr uint32_t MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT           = 0x1C08B8;
    static constexpr uint32_t MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT              = 0x1C0954;
    static constexpr uint32_t MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT                 = 0x1C08BC;
    static constexpr uint32_t MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT                  = 0x1C0800;
    static constexpr uint32_t MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT                   = 0x1C0850;
    static constexpr uint32_t MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT                    = 0x1C0868;
    static constexpr uint32_t MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT   = 0x1C08A0;
    static constexpr uint32_t MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT = 0x1C08A4;
    static constexpr uint32_t MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT   = 0x1C08D0;
    //VDBOX register initial value
    static constexpr uint32_t MFX_LRA0_REG_OFFSET_NODE_1_INIT = 0;
    static constexpr uint32_t MFX_LRA1_REG_OFFSET_NODE_1_INIT = 0;
    static constexpr uint32_t MFX_LRA2_REG_OFFSET_NODE_1_INIT = 0;

    void InitMmioRegisters()
    {
        MmioRegistersVdbox *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

        mmioRegisters->generalPurposeRegister0LoOffset           = mhw::mi::GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister0HiOffset           = mhw::mi::GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister4LoOffset           = mhw::mi::GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister4HiOffset           = mhw::mi::GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister11LoOffset          = mhw::mi::GENERAL_PURPOSE_REGISTER11_LO_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister11HiOffset          = mhw::mi::GENERAL_PURPOSE_REGISTER11_HI_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister12LoOffset          = mhw::mi::GENERAL_PURPOSE_REGISTER12_LO_OFFSET_NODE_1_INIT;
        mmioRegisters->generalPurposeRegister12HiOffset          = mhw::mi::GENERAL_PURPOSE_REGISTER12_HI_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcImageStatusMaskRegOffset               = MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcImageStatusCtrlRegOffset               = MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcAvcNumSlicesRegOffset                  = MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcQPStatusCountOffset                    = MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT;
        mmioRegisters->mfxErrorFlagsRegOffset                    = MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfxFrameCrcRegOffset                      = MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfxMBCountRegOffset                       = MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcBitstreamBytecountFrameRegOffset       = MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset      = MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfcBitstreamBytecountSliceRegOffset       = MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfxLra0RegOffset                          = MFX_LRA0_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfxLra1RegOffset                          = MFX_LRA1_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->mfxLra2RegOffset                          = MFX_LRA2_REG_OFFSET_NODE_1_INIT;

        m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
    }
protected:
    using base_t = Itf;

    struct
    {
        RowStoreCache vdenc;
        RowStoreCache ipdl;
    } m_rowStoreCache                                                                           = {};
    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};

    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;

        InitMmioRegisters();
        InitRowstoreUserFeatureSettings();
    }

    virtual MOS_STATUS InitRowstoreUserFeatureSettings()
    {
        MHW_FUNCTION_ENTER;

        bool rowstoreCachingDisableDefaultValue = false;
        if (this->m_osItf->bSimIsActive)
        {
            // Disable RowStore Cache on simulation by default
            rowstoreCachingDisableDefaultValue = true;
        }
        else
        {
            rowstoreCachingDisableDefaultValue = false;
        }
        bool rowstoreCachingSupported = !rowstoreCachingDisableDefaultValue;
#if (_DEBUG || _RELEASE_INTERNAL)
        auto userSettingPtr = m_osItf->pfnGetUserSettingInstance(m_osItf);
        {
            MediaUserSetting::Value outValue;
            ReadUserSettingForDebug(userSettingPtr,
                outValue,
                "Disable RowStore Cache",
                MediaUserSetting::Group::Device,
                rowstoreCachingDisableDefaultValue,
                true);
            rowstoreCachingSupported = !(outValue.Get<bool>());
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

        if (!rowstoreCachingSupported)
        {
            return MOS_STATUS_SUCCESS;
        }

        m_rowStoreCache.vdenc.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        {
            MediaUserSetting::Value outValue;
            ReadUserSettingForDebug(userSettingPtr,
                outValue,
                "DisableVDEncRowStoreCache",
                MediaUserSetting::Group::Device);
            m_rowStoreCache.vdenc.supported = !(outValue.Get<bool>());
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

        m_rowStoreCache.ipdl.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        {
            MediaUserSetting::Value outValue;
            ReadUserSettingForDebug(userSettingPtr,
                outValue,
                "DisableIntraRowStoreCache",
                MediaUserSetting::Group::Device);
            m_rowStoreCache.ipdl.supported = !(outValue.Get<bool>());
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

        return MOS_STATUS_SUCCESS;
    }

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__Impl)
};


template <typename cmd_t>
class BaseImpl : public Impl<cmd_t>
{
protected:
    using base_t = Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(VD_PIPELINE_FLUSH)
    {
        _MHW_SETCMD_CALLBASE(VD_PIPELINE_FLUSH);

#define DO_FIELDS()                                                           \
    DO_FIELD(DW1, VdaqmPipelineDone, params.waitDoneVDAQM);                   \
    DO_FIELD(DW1, VdaqmPipelineCommandFlush, params.flushVDAQM);              \
    DO_FIELD(DW1, VdCommandMessageParserDone, params.waitDoneVDCmdMsgParser); \
    DO_FIELD(DW1, VvcpPipelineDone, params.vvcpPipelineDone);                 \
    DO_FIELD(DW1, VvcpPipelineCommandFlush, params.vvcpPipelineCommandFlush);

#include "mhw_hwcmd_process_cmdfields.h"
    }

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe3_lpm_base__BaseImpl)
};
}  // namespace xe3_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif // _MEDIA_RESERVED
#endif  // __MHW_VDBOX_VDENC_IMPL_XE3_LPM_BASE_OPEN_H__
