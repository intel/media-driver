/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_impl.h
//! \brief    MHW VDBOX VDENC interface common base
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_H__
#define __MHW_VDBOX_VDENC_IMPL_H__

#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_impl.h"
#include "mhw_mi_impl.h"

#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_impl_ext.h"
#include "mhw_vdbox_vdenc_hwcmd_ext.h"
#endif

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
inline SurfaceFormat MosFormatToVdencSurfaceRawFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
        return SurfaceFormat::rgba4444;
    case Format_NV12:
    case Format_NV11:
    case Format_P208:
    case Format_IMC1:
    case Format_IMC2:
    case Format_IMC3:
    case Format_IMC4:
        return SurfaceFormat::planar4208;
    case Format_400P:
    case Format_P8:
        return SurfaceFormat::y8Unorm;
    case Format_UYVY:
        return SurfaceFormat::yCrCbSwapY422;
    case Format_YVYU:
        return SurfaceFormat::yCrCbSwapUv422;
    case Format_VYUY:
        return SurfaceFormat::yCrCbSwapUvy422;
    case Format_444P:
    case Format_AYUV:
        return SurfaceFormat::yuv444;
    case Format_YUY2:
    case Format_YUYV:
        return SurfaceFormat::yuv422;
    case Format_P010:
        return SurfaceFormat::p010;
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        return SurfaceFormat::r10g10b10a2;
        // Only Y210 supported now, allocated as Y216 format by 3D driver
    case Format_Y210:
    case Format_Y216:
        return SurfaceFormat::y216;
    case Format_Y410:
        return SurfaceFormat::y410;
    case Format_NV21:
        return SurfaceFormat::nv21;
    default:
        return SurfaceFormat::planar4208;
    }
}

inline SurfaceFormat MosFormatToVdencSurfaceReconFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
        return SurfaceFormat::rgba4444;
    case Format_NV12:
    case Format_IMC1:
    case Format_IMC2:
    case Format_IMC3:
    case Format_IMC4:
        return SurfaceFormat::planar4208;
    case Format_400P:
    case Format_P8:
        return SurfaceFormat::y8Unorm;
    case Format_UYVY:
        return SurfaceFormat::yCrCbSwapY422;
    case Format_YVYU:
        return SurfaceFormat::yCrCbSwapUv422;
    case Format_VYUY:
        return SurfaceFormat::yCrCbSwapUvy422;
    case Format_444P:
    case Format_AYUV:
        return SurfaceFormat::ayuvVariant;
    case Format_YUY2:
    case Format_YUYV:
        return SurfaceFormat::yuyvVariant;
    case Format_P010:
        return SurfaceFormat::p010Variant;
    case Format_R10G10B10A2:
        return SurfaceFormat::r10g10b10a2;
    case Format_Y210:
    case Format_Y216:
        return SurfaceFormat::y216Variant;
    case Format_Y410:
        return SurfaceFormat::y416Variant;
    case Format_NV21:
        return SurfaceFormat::nv21;
    default:
        return SurfaceFormat::planar4208;
    }
}

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

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CONTROL_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CONTROL_STATE);

#define DO_FIELDS() \
    DO_FIELD(DW1, VdencInitialization, params.vdencInitialization)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_MODE_SELECT);

#define DO_FIELDS()                                                                           \
    DO_FIELD(DW1, StandardSelect, params.standardSelect);                                     \
    DO_FIELD(DW1, ScalabilityMode, params.scalabilityMode);                                   \
    DO_FIELD(DW1, FrameStatisticsStreamOutEnable, params.frameStatisticsStreamOut);           \
    DO_FIELD(DW1, VdencPakObjCmdStreamOutEnable, params.pakObjCmdStreamOut);                  \
    DO_FIELD(DW1, TlbPrefetchEnable, params.tlbPrefetch);                                     \
    DO_FIELD(DW1, PakThresholdCheckEnable, params.dynamicSlice);                              \
    DO_FIELD(DW1, VdencStreamInEnable, params.streamIn);                                      \
    DO_FIELD(DW1, BitDepth, params.bitDepthMinus8);                                           \
    DO_FIELD(DW1, PakChromaSubSamplingType, params.chromaType);                               \
    DO_FIELD(DW1, OutputRangeControlAfterColorSpaceConversion, params.outputRangeControlCsc); \
    DO_FIELD(DW1, TileReplayEnable, params.tileBasedReplayMode);                              \
    DO_FIELD(DW1, IsRandomAccess, params.randomAccess);                                       \
    DO_FIELD(DW1, RgbEncodingEnable, params.rgbEncodingMode);                                 \
    DO_FIELD(DW1, StreamingBufferConfig, params.streamingBufferConfig);                       \
                                                                                              \
    DO_FIELD(DW2, HmeRegionPreFetchenable, params.hmeRegionPrefetch);                         \
    DO_FIELD(DW2, Topprefetchenablemode, params.topPrefetchEnableMode);                       \
    DO_FIELD(DW2, LeftpreFetchatwraparound, params.leftPrefetchAtWrapAround);                 \
    DO_FIELD(DW2, Verticalshift32Minus1, params.verticalShift32Minus1);                       \
    DO_FIELD(DW2, Hzshift32Minus1, params.hzShift32Minus1);                                   \
    DO_FIELD(DW2, NumVerticalReqMinus1, params.numVerticalReqMinus1);                         \
    DO_FIELD(DW2, Numhzreqminus1, params.numHzReqMinus1);                                     \
    DO_FIELD(DW2, PreFetchOffsetForReferenceIn16PixelIncrement, params.prefetchOffset);       \
                                                                                              \
    DO_FIELD(DW5, CaptureMode, params.captureMode);                                           \
    DO_FIELD(DW5, ParallelCaptureAndEncodeSessionId, params.wirelessSessionId);               \
    DO_FIELD(DW5, TailPointerReadFrequency, params.tailPointerReadFrequency);                 \
    DO_FIELD(DW5, VDENC_PIPE_MODE_SELECT_DW5_BIT17, params.VdencPipeModeSelectPar0);          \
    DO_FIELD(DW5, VDENC_PIPE_MODE_SELECT_DW5_BIT8, params.VdencPipeModeSelectPar1);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_SRC_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_SRC_SURFACE_STATE);

#define DO_FIELDS()                                                                                                \
    DO_FIELD(Dwords25.DW0, Width, params.width - 1);                                                               \
    DO_FIELD(Dwords25.DW0, Height, params.height - 1);                                                             \
    DO_FIELD(Dwords25.DW0, ColorSpaceSelection, params.colorSpaceSelection);                                       \
    DO_FIELD(Dwords25.DW0, CrVCbUPixelOffsetVDirection, params.vDirection);                                        \
    DO_FIELD(Dwords25.DW0, SurfaceFormatByteSwizzle, params.displayFormatSwizzle);                                 \
                                                                                                                   \
    DO_FIELD(Dwords25.DW1, TileMode, GetHwTileType(params.tileType, params.tileModeGmm, params.gmmTileEn));        \
    DO_FIELD(Dwords25.DW1, SurfaceFormat, static_cast<uint32_t>(MosFormatToVdencSurfaceRawFormat(params.format))); \
    DO_FIELD(Dwords25.DW1, SurfacePitch, params.pitch - 1);                                                        \
    DO_FIELD(Dwords25.DW1, ChromaDownsampleFilterControl, params.chromaDownsampleFilterControl);                                                        \
                                                                                                                   \
    DO_FIELD(Dwords25.DW2, YOffsetForUCb, params.uOffset);                                                         \
    DO_FIELD(Dwords25.DW3, YOffsetForVCr, params.vOffset)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_REF_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_REF_SURFACE_STATE);

#define DO_FIELDS()                                                                                                  \
    DO_FIELD(Dwords25.DW0, Width, params.width - 1);                                                                 \
    DO_FIELD(Dwords25.DW0, Height, params.height - 1);                                                               \
    DO_FIELD(Dwords25.DW0, CrVCbUPixelOffsetVDirection, params.vDirection);                                          \
                                                                                                                     \
    DO_FIELD(Dwords25.DW1, TileMode, GetHwTileType(params.tileType, params.tileModeGmm, params.gmmTileEn));          \
    DO_FIELD(Dwords25.DW1, SurfacePitch, params.pitch - 1);                                                          \
    DO_FIELD(Dwords25.DW1, SurfaceFormat, static_cast<uint32_t>(MosFormatToVdencSurfaceReconFormat(params.format))); \
                                                                                                                     \
    DO_FIELD(Dwords25.DW2, YOffsetForUCb, params.uOffset);                                                           \
    DO_FIELD(Dwords25.DW3, YOffsetForVCr, params.vOffset)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_DS_REF_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_DS_REF_SURFACE_STATE);

        const bool stage2 = params.widthStage2 && params.heightStage2 && params.pitchStage2;

#define DO_FIELDS()                                                                                                                        \
    DO_FIELD(Dwords25.DW0, Width, params.widthStage1 - 1);                                                                                 \
    DO_FIELD(Dwords25.DW0, Height, params.heightStage1 - 1);                                                                               \
    DO_FIELD(Dwords25.DW0, CrVCbUPixelOffsetVDirection, params.vDirectionStage1);                                                          \
                                                                                                                                           \
    DO_FIELD(Dwords25.DW1, TileMode, GetHwTileType(params.tileTypeStage1, params.tileModeGmmStage1, params.gmmTileEnStage1));              \
    DO_FIELD(Dwords25.DW1, SurfaceFormat, cmd_t::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8);                             \
    DO_FIELD(Dwords25.DW1, SurfacePitch, params.pitchStage1 - 1);                                                                          \
                                                                                                                                           \
    DO_FIELD(Dwords25.DW2, YOffsetForUCb, params.uOffsetStage1);                                                                           \
                                                                                                                                           \
    DO_FIELD(Dwords25.DW3, YOffsetForVCr, params.vOffsetStage1);                                                                           \
                                                                                                                                           \
    DO_FIELD(Dwords69.DW0, Width, stage2 ? params.widthStage2 - 1 : 0);                                                                    \
    DO_FIELD(Dwords69.DW0, Height, stage2 ? params.heightStage2 - 1 : 0);                                                                  \
    DO_FIELD(Dwords69.DW0, CrVCbUPixelOffsetVDirection, stage2 ? params.vDirectionStage2 : 0);                                             \
                                                                                                                                           \
    DO_FIELD(Dwords69.DW1, TileMode, stage2 ? GetHwTileType(params.tileTypeStage2, params.tileModeGmmStage2, params.gmmTileEnStage2) : 0); \
    DO_FIELD(Dwords69.DW1, SurfaceFormat, stage2 ? cmd_t::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8 : 0);                \
    DO_FIELD(Dwords69.DW1, SurfacePitch, stage2 ? params.pitchStage2 - 1 : 0);                                                             \
                                                                                                                                           \
    DO_FIELD(Dwords69.DW2, YOffsetForUCb, stage2 ? params.uOffsetStage2 : 0);                                                              \
                                                                                                                                           \
    DO_FIELD(Dwords69.DW3, YOffsetForVCr, stage2 ? params.vOffsetStage2 : 0)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        if (params.surfaceRaw)
        {
            cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(params.mmcStateRaw);
            cmd.OriginalUncompressedPicture.PictureFields.DW0.CompressionType         = MmcRcEnabled(params.mmcStateRaw);
            cmd.OriginalUncompressedPicture.PictureFields.DW0.CompressionFormat       = params.compressionFormatRaw;

            resourceParams                 = {};
            resourceParams.presResource    = &params.surfaceRaw->OsResource;
            resourceParams.dwOffset        = params.surfaceRaw->dwOffset;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.OriginalUncompressedPicture.LowerAddress);
            //resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(OriginalUncompressedPicture.LowerAddress);
            resourceParams.dwLocationInCmd = 10;
            resourceParams.bIsWritable     = false;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.OriginalUncompressedPicture.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (this->m_rowStoreCache.vdenc.enabled)
        {
            cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.CacheSelect = cmd_t::VDENC_Surface_Control_Bits_CMD::CACHE_SELECT_UNNAMED1;
            cmd.RowStoreScratchBuffer.LowerAddress.DW0.Value              = this->m_rowStoreCache.vdenc.dwAddress << 6;
        }
        else if (!Mos_ResourceIsNull(params.intraRowStoreScratchBuffer))
        {
            resourceParams.presResource    = params.intraRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.RowStoreScratchBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(RowStoreScratchBuffer.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.streamOutBuffer))
        {
            resourceParams.presResource    = params.streamOutBuffer;
            resourceParams.dwOffset        = params.streamOutOffset;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.VdencStatisticsStreamout.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(VdencStatisticsStreamout.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.VdencStatisticsStreamout.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.streamInBuffer))
        {
            resourceParams.presResource    = params.streamInBuffer;
            resourceParams.dwOffset        = params.streamInBuffer->dwResourceOffset;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.StreaminDataPicture.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(StreaminDataPicture.LowerAddress);
            resourceParams.bIsWritable     = false;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.StreaminDataPicture.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        // SCC will use BwdRef0 as extra slot to store IBC if max fwd ref num is 3.
        typename cmd_t::VDENC_Reference_Picture_CMD *fwdRefs[] =
            {&cmd.FwdRef0, &cmd.FwdRef1, &cmd.FwdRef2, &cmd.BwdRef0};
        uint32_t fwdRefsDwLoaction[] =
            {_MHW_CMD_DW_LOCATION(FwdRef0), _MHW_CMD_DW_LOCATION(FwdRef1), _MHW_CMD_DW_LOCATION(FwdRef2), _MHW_CMD_DW_LOCATION(BwdRef0)};

        typename cmd_t::VDENC_Down_Scaled_Reference_Picture_CMD *fwdRefsDsStage1[] =
            {&cmd.DsFwdRef0, &cmd.DsFwdRef1};
        uint32_t fwdRefsDsStage1DwLoaction[] =
            {_MHW_CMD_DW_LOCATION(DsFwdRef0), _MHW_CMD_DW_LOCATION(DsFwdRef1)};

        typename cmd_t::VDENC_Down_Scaled_Reference_Picture_CMD *fwdRefsDsStage2[] =
            {&cmd.DsFwdRef04X, &cmd.DsFwdRef14X, &cmd.Additional4xDsFwdRef};
        uint32_t fwdRefsDsStage2DwLoaction[] =
            {_MHW_CMD_DW_LOCATION(DsFwdRef04X), _MHW_CMD_DW_LOCATION(DsFwdRef14X), _MHW_CMD_DW_LOCATION(Additional4xDsFwdRef)};

        uint8_t refIdx;
        for (refIdx = 0; refIdx < params.numActiveRefL0; refIdx++)
        {
            if (!Mos_ResourceIsNull(params.refs[refIdx]) && refIdx < sizeof(fwdRefs) / sizeof(fwdRefs[0]))
            {
                MOS_SURFACE details = {};
                details.Format      = Format_Invalid;
                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refs[refIdx], &details));

                resourceParams.presResource    = params.refs[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = fwdRefsDwLoaction[refIdx];
                resourceParams.bIsWritable     = false;
                resourceParams.pdwCmd          = (uint32_t *)&fwdRefs[refIdx]->LowerAddress;
                resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                uint8_t  mmcSkip   = (params.mmcSkipMask) & (1 << refIdx);
                auto     mmcMode   = MOS_MEMCOMP_DISABLED;
                uint32_t mmcFormat = 0;
                if (params.mmcEnabled)
                {
                    MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionMode(
                        this->m_osItf, params.refs[refIdx], &mmcMode));
                    MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionFormat(
                        this->m_osItf, params.refs[refIdx], &mmcFormat));
                }

                fwdRefs[refIdx]->PictureFields.DW0.MemoryCompressionEnable = mmcSkip ? 0 : MmcEnabled(mmcMode);
                fwdRefs[refIdx]->PictureFields.DW0.CompressionType         = mmcSkip ? MOS_MEMCOMP_DISABLED : MmcRcEnabled(mmcMode);
                fwdRefs[refIdx]->PictureFields.DW0.CompressionFormat       = mmcFormat;

                InitMocsParams(resourceParams, &fwdRefs[refIdx]->PictureFields.DW0.Value, 1, 6);

                MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (!Mos_ResourceIsNull(params.refsDsStage1[refIdx]) && refIdx < sizeof(fwdRefsDsStage1) / sizeof(fwdRefsDsStage1[0]))
            {
                MOS_SURFACE details = {};
                details.Format      = Format_Invalid;
                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refsDsStage1[refIdx], &details));

                resourceParams.presResource    = params.refsDsStage1[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = fwdRefsDsStage1DwLoaction[refIdx];
                resourceParams.bIsWritable     = false;
                resourceParams.pdwCmd          = (uint32_t *)&(fwdRefsDsStage1[refIdx]->LowerAddress);
                resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                auto mmcMode = params.mmcStateDsStage1;

                fwdRefsDsStage1[refIdx]->PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
                fwdRefsDsStage1[refIdx]->PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

                InitMocsParams(resourceParams, &fwdRefsDsStage1[refIdx]->PictureFields.DW0.Value, 1, 6);

                MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (!Mos_ResourceIsNull(params.refsDsStage2[refIdx]) && refIdx < sizeof(fwdRefsDsStage2) / sizeof(fwdRefsDsStage2[0]))
            {
                MOS_SURFACE details = {};
                details.Format      = Format_Invalid;
                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refsDsStage2[refIdx], &details));

                resourceParams.presResource    = params.refsDsStage2[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = fwdRefsDsStage2DwLoaction[refIdx];
                resourceParams.bIsWritable     = false;
                resourceParams.pdwCmd          = (uint32_t *)&(fwdRefsDsStage2[refIdx]->LowerAddress);
                resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                auto mmcMode = params.mmcStateDsStage2;

                fwdRefsDsStage2[refIdx]->PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
                fwdRefsDsStage2[refIdx]->PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

                InitMocsParams(resourceParams, &fwdRefsDsStage2[refIdx]->PictureFields.DW0.Value, 1, 6);

                MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));

                if (params.numActiveRefL0 == 2 && params.numActiveRefL1 == 1 && refIdx == 1)
                {
                    resourceParams.dwLocationInCmd = fwdRefsDsStage2DwLoaction[refIdx + 1];
                    resourceParams.pdwCmd          = (uint32_t *)&(fwdRefsDsStage2[refIdx + 1]->LowerAddress);
                    resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                    fwdRefsDsStage2[refIdx + 1]->PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
                    fwdRefsDsStage2[refIdx + 1]->PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

                    InitMocsParams(resourceParams, &fwdRefsDsStage2[refIdx + 1]->PictureFields.DW0.Value, 1, 6);

                    MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                        this->m_osItf,
                        this->m_currentCmdBuf,
                        &resourceParams));
                }
            }
        }

        if ((!params.lowDelayB && params.numActiveRefL1) || params.isPFrame)  //HW request HEVC PFrame to set address in BwdRef0 to be same as FwdRef0
        {
            if (!Mos_ResourceIsNull(params.refs[refIdx]))
            {
                MOS_SURFACE details = {};
                details.Format      = Format_Invalid;
                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refs[refIdx], &details));

                resourceParams.presResource    = params.refs[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(BwdRef0);
                resourceParams.bIsWritable     = false;
                resourceParams.pdwCmd          = (uint32_t *)&(cmd.BwdRef0.LowerAddress);
                resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                auto     mmcMode   = MOS_MEMCOMP_DISABLED;
                uint32_t mmcFormat = 0;
                if (params.mmcEnabled)
                {
                    MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionMode(
                        this->m_osItf, params.refs[refIdx], &mmcMode));
                    MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetMemoryCompressionFormat(
                        this->m_osItf, params.refs[refIdx], &mmcFormat));
                }

                cmd.BwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
                cmd.BwdRef0.PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);
                cmd.BwdRef0.PictureFields.DW0.CompressionFormat       = mmcFormat;

                InitMocsParams(resourceParams, &cmd.BwdRef0.PictureFields.DW0.Value, 1, 6);

                MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (!Mos_ResourceIsNull(params.refsDsStage1[refIdx]))
            {
                MOS_SURFACE details = {};
                details.Format      = Format_Invalid;
                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refsDsStage1[refIdx], &details));

                resourceParams.presResource    = params.refsDsStage1[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DsBwdRef0);
                resourceParams.bIsWritable     = false;
                resourceParams.pdwCmd          = (uint32_t *)&(cmd.DsBwdRef0.LowerAddress);
                resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                auto mmcMode = params.mmcStateDsStage1;

                cmd.DsBwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
                cmd.DsBwdRef0.PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

                InitMocsParams(resourceParams, &cmd.DsBwdRef0.PictureFields.DW0.Value, 1, 6);

                MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }

            if (!Mos_ResourceIsNull(params.refsDsStage2[refIdx]))
            {
                MOS_SURFACE details = {};
                details.Format      = Format_Invalid;
                MHW_CHK_STATUS_RETURN(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.refsDsStage2[refIdx], &details));

                resourceParams.presResource    = params.refsDsStage2[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DsBwdRef04X);
                resourceParams.bIsWritable     = false;
                resourceParams.pdwCmd          = (uint32_t *)&(cmd.DsBwdRef04X.LowerAddress);
                resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

                auto mmcMode = params.mmcStateDsStage2;

                cmd.DsBwdRef04X.PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
                cmd.DsBwdRef04X.PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

                InitMocsParams(resourceParams, &cmd.DsBwdRef04X.PictureFields.DW0.Value, 1, 6);

                MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        if (!Mos_ResourceIsNull(params.colocatedMvReadBuffer))
        {
            resourceParams.presResource    = params.colocatedMvReadBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.ColocatedMv.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ColocatedMv.LowerAddress);
            resourceParams.bIsWritable     = false;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionEnable = 0;
            cmd.ColocatedMv.PictureFields.DW0.CompressionType         = 0;

            InitMocsParams(resourceParams, &cmd.ColocatedMv.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.colMvTempBuffer[0]))
        {
            resourceParams.presResource    = params.colMvTempBuffer[0];
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.ColocatedMv.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ColocatedMv.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionEnable = 0;
            cmd.ColocatedMv.PictureFields.DW0.CompressionType         = 0;

            InitMocsParams(resourceParams, &cmd.ColocatedMv.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.surfaceDsStage1)
        {
            resourceParams.presResource    = &params.surfaceDsStage1->OsResource;
            resourceParams.dwOffset        = params.surfaceDsStage1->dwOffset;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.ScaledReferenceSurfaceStage1.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ScaledReferenceSurfaceStage1.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            auto mmcMode = params.mmcStateDsStage1;

            cmd.ScaledReferenceSurfaceStage1.PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
            cmd.ScaledReferenceSurfaceStage1.PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

            InitMocsParams(resourceParams, &cmd.ScaledReferenceSurfaceStage1.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (params.surfaceDsStage2)
        {
            resourceParams.presResource    = &params.surfaceDsStage2->OsResource;
            resourceParams.dwOffset        = params.surfaceDsStage2->dwOffset;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.ScaledReferenceSurfaceStage2.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ScaledReferenceSurfaceStage2.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            auto mmcMode = params.mmcStateDsStage2;

            cmd.ScaledReferenceSurfaceStage2.PictureFields.DW0.MemoryCompressionEnable = MmcEnabled(mmcMode);
            cmd.ScaledReferenceSurfaceStage2.PictureFields.DW0.CompressionType         = MmcRcEnabled(mmcMode);

            InitMocsParams(resourceParams, &cmd.ScaledReferenceSurfaceStage2.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.pakObjCmdStreamOutBuffer))
        {
            resourceParams.presResource    = params.pakObjCmdStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.VdencLcuPakObjCmdBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(VdencLcuPakObjCmdBuffer.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryCompressionEnable = 0;
            cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.CompressionType         = 0;

            InitMocsParams(resourceParams, &cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.segmentMapStreamInBuffer))
        {
            resourceParams.presResource    = params.segmentMapStreamInBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.Vp9SegmentationMapStreaminBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Vp9SegmentationMapStreaminBuffer.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.Vp9SegmentationMapStreaminBuffer.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.segmentMapStreamOutBuffer))
        {
            resourceParams.presResource    = params.segmentMapStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.Vp9SegmentationMapStreamoutBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(Vp9SegmentationMapStreamoutBuffer.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.Vp9SegmentationMapStreamoutBuffer.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        cmd.DW61.WeightsHistogramStreamoutOffset = 3 * MHW_CACHELINE_SIZE;

        if (!Mos_ResourceIsNull(params.tileRowStoreBuffer))
        {
            resourceParams.presResource    = params.tileRowStoreBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.VdencTileRowStoreBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(VdencTileRowStoreBuffer.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.VdencTileRowStoreBuffer.BufferPictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (this->m_rowStoreCache.ipdl.enabled)
        {
            cmd.IntraPredictionRowstoreBaseAddress.BufferPictureFields.DW0.CacheSelect = cmd_t::VDENC_Surface_Control_Bits_CMD::CACHE_SELECT_UNNAMED1;
            cmd.IntraPredictionRowstoreBaseAddress.LowerAddress.DW0.Value              = this->m_rowStoreCache.ipdl.dwAddress << 6;
        }
        else if (!Mos_ResourceIsNull(params.mfdIntraRowStoreScratchBuffer))
        {
            resourceParams.presResource    = params.mfdIntraRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.IntraPredictionRowstoreBaseAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(IntraPredictionRowstoreBaseAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            cmd.IntraPredictionRowstoreBaseAddress.BufferPictureFields.DW0.MemoryCompressionEnable = 0;

            InitMocsParams(resourceParams, &cmd.IntraPredictionRowstoreBaseAddress.BufferPictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.cumulativeCuCountStreamOutBuffer))
        {
            resourceParams.presResource    = params.cumulativeCuCountStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.VdencCumulativeCuCountStreamoutSurface.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(VdencCumulativeCuCountStreamoutSurface.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            InitMocsParams(resourceParams, &cmd.VdencCumulativeCuCountStreamoutSurface.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.colocatedMvWriteBuffer))
        {
            resourceParams.presResource    = params.colocatedMvWriteBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.ColocatedMvAvcWriteBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ColocatedMvAvcWriteBuffer.LowerAddress);
            resourceParams.bIsWritable     = true;
            resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

            cmd.ColocatedMvAvcWriteBuffer.PictureFields.DW0.MemoryCompressionEnable = 0;
            cmd.ColocatedMvAvcWriteBuffer.PictureFields.DW0.CompressionType         = 0;

            InitMocsParams(resourceParams, &cmd.ColocatedMvAvcWriteBuffer.PictureFields.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_WEIGHTSOFFSETS_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_WEIGHTSOFFSETS_STATE);

#define DO_FIELDS()                                                                                                   \
    DO_FIELD(DW1, WeightsForwardReference0, Clip3(-128, 127, params.weightsLuma[0][0] + params.denomLuma));           \
    DO_FIELD(DW1, OffsetForwardReference0, params.offsetsLuma[0][0]);                                                 \
    DO_FIELD(DW1, WeightsForwardReference1, Clip3(-128, 127, params.weightsLuma[0][1] + params.denomLuma));           \
    DO_FIELD(DW1, OffsetForwardReference1, params.offsetsLuma[0][1]);                                                 \
                                                                                                                      \
    DO_FIELD(DW2, WeightsForwardReference2, Clip3(-128, 127, params.weightsLuma[0][2] + params.denomLuma));           \
    DO_FIELD(DW2, OffsetForwardReference2, params.offsetsLuma[0][2]);                                                 \
    DO_FIELD(DW2, WeightsBackwardReference0, Clip3(-128, 127, params.weightsLuma[1][0] + params.denomLuma));          \
    DO_FIELD(DW2, OffsetBackwardReference0, params.offsetsLuma[1][0]);                                                \
                                                                                                                      \
    DO_FIELD(DW3, CbWeightsForwardReference0, Clip3(-128, 127, params.weightsChroma[0][0][0] + params.denomChroma));  \
    DO_FIELD(DW3, CbOffsetForwardReference0, params.offsetsChroma[0][0][0]);                                          \
    DO_FIELD(DW3, CbWeightsForwardReference1, Clip3(-128, 127, params.weightsChroma[0][1][0] + params.denomChroma));  \
    DO_FIELD(DW3, CbOffsetForwardReference1, params.offsetsChroma[0][1][0]);                                          \
                                                                                                                      \
    DO_FIELD(DW4, CbWeightsForwardReference2, Clip3(-128, 127, params.weightsChroma[0][2][0] + params.denomChroma));  \
    DO_FIELD(DW4, CbOffsetForwardReference2, params.offsetsChroma[0][2][0]);                                          \
    DO_FIELD(DW4, CbWeightsBackwardReference0, Clip3(-128, 127, params.weightsChroma[1][0][0] + params.denomChroma)); \
    DO_FIELD(DW4, CbOffsetBackwardReference0, params.offsetsChroma[1][0][0]);                                         \
                                                                                                                      \
    DO_FIELD(DW5, CrWeightsForwardReference0, Clip3(-128, 127, params.weightsChroma[0][0][1] + params.denomChroma));  \
    DO_FIELD(DW5, CrOffsetForwardReference0, params.offsetsChroma[0][0][1]);                                          \
    DO_FIELD(DW5, CrWeightsForwardReference1, Clip3(-128, 127, params.weightsChroma[0][1][1] + params.denomChroma));  \
    DO_FIELD(DW5, CrOffsetForwardReference1, params.offsetsChroma[0][1][1]);                                          \
                                                                                                                      \
    DO_FIELD(DW6, CrWeightsForwardReference2, Clip3(-128, 127, params.weightsChroma[0][2][1] + params.denomChroma));  \
    DO_FIELD(DW6, CrOffsetForwardReference2, params.offsetsChroma[0][2][1]);                                          \
    DO_FIELD(DW6, CrWeightsBackwardReference0, Clip3(-128, 127, params.weightsChroma[1][0][1] + params.denomChroma)); \
    DO_FIELD(DW6, CrOffsetBackwardReference0, params.offsetsChroma[1][0][1])

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_HEVC_VP9_TILE_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_HEVC_VP9_TILE_SLICE_STATE);

#define DO_FIELDS()                                                                                                       \
    DO_FIELD(DW3, NumParEngine, params.numPipe);                                                                          \
    DO_FIELD(DW3, TileNumber, params.tileId);                                                                             \
    DO_FIELD(DW3, TileRowStoreSelect, params.tileRowStoreSelect);                                                         \
    DO_FIELD(DW3, Log2WeightDenomLuma, params.log2WeightDenomLuma);                                                       \
    DO_FIELD(DW3, HevcVp9Log2WeightDenomLuma, params.hevcVp9Log2WeightDenomLuma);                                         \
    DO_FIELD(DW3, Log2WeightDenomChroma, params.log2WeightDenomChroma);                                                   \
                                                                                                                          \
    DO_FIELD(DW4, TileStartCtbX, params.tileStartLCUX *params.ctbSize);                                                   \
    DO_FIELD(DW4, TileStartCtbY, params.tileStartLCUY *params.ctbSize);                                                   \
                                                                                                                          \
    DO_FIELD(DW5, TileWidth, (params.tileWidth >= 256 ? MOS_ALIGN_CEIL(params.tileWidth, 8) : params.tileWidth) - 1);     \
    DO_FIELD(DW5, TileHeight, (params.tileHeight >= 128 ? MOS_ALIGN_CEIL(params.tileHeight, 8) : params.tileHeight) - 1); \
                                                                                                                          \
    DO_FIELD(DW6, StreaminOffsetEnable, params.tileEnable);                                                               \
    DO_FIELD(DW6, TileStreaminOffset, params.tileStreamInOffset);                                                         \
                                                                                                                          \
    DO_FIELD(DW7, RowStoreOffsetEnable, cmd.DW4.TileStartCtbY == 0 ? params.tileEnable : 0);                              \
    DO_FIELD(DW7, TileRowstoreOffset, params.tileRowstoreOffset);                                                         \
                                                                                                                          \
    DO_FIELD(DW8, TileStreamoutOffsetEnable, params.tileEnable);                                                          \
    DO_FIELD(DW8, TileStreamoutOffset, params.tileId * 19);                                                               \
                                                                                                                          \
    DO_FIELD(DW9, LcuStreamOutOffsetEnable, params.tileEnable);                                                           \
    DO_FIELD(DW9, TileLcuStreamOutOffset, params.tileLCUStreamOutOffset);                                                 \
                                                                                                                          \
    DO_FIELD(DW11, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW11_BIT8, params.VdencHEVCVP9TileSlicePar0);                          \
                                                                                                                          \
    DO_FIELD(DW12, IbcControl, params.ibcControl);                                                                        \
    DO_FIELD(DW12, PaletteModeEnable, params.paletteModeEnable);                                                          \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT24, params.VdencHEVCVP9TileSlicePar1);                         \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT0, params.VdencHEVCVP9TileSlicePar5);                          \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT18, params.VdencHEVCVP9TileSlicePar2);                         \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT16, params.VdencHEVCVP9TileSlicePar3);                         \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT8, params.VdencHEVCVP9TileSlicePar4);                          \
                                                                                                                          \
    DO_FIELD(DW13, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW13_BIT16, params.VdencHEVCVP9TileSlicePar6);                         \
    DO_FIELD(DW13, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW13_BIT0, params.VdencHEVCVP9TileSlicePar7);                          \
                                                                                                                          \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT8, params.VdencHEVCVP9TileSlicePar8);                          \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT0, params.VdencHEVCVP9TileSlicePar9);                          \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT16, params.VdencHEVCVP9TileSlicePar10);                        \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT31, params.VdencHEVCVP9TileSlicePar11);                        \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT24, params.VdencHEVCVP9TileSlicePar12);                        \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT21, params.VdencHEVCVP9TileSlicePar13);                        \
                                                                                                                          \
    DO_FIELD(DW15, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW15_BIT0, params.VdencHEVCVP9TileSlicePar15);                         \
    DO_FIELD(DW15, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW15_BIT16, params.VdencHEVCVP9TileSlicePar14);                        \
                                                                                                                          \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT31, params.VdencHEVCVP9TileSlicePar16[2]);                     \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT30, params.VdencHEVCVP9TileSlicePar16[1]);                     \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT29, params.VdencHEVCVP9TileSlicePar16[0]);                     \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT24, params.VdencHEVCVP9TileSlicePar23);                        \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT16, params.VdencHEVCVP9TileSlicePar17[2]);                     \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT8, params.VdencHEVCVP9TileSlicePar17[1]);                      \
    DO_FIELD(DW16, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW16_BIT0, params.VdencHEVCVP9TileSlicePar17[0]);                      \
                                                                                                                          \
    DO_FIELD(DW17, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW17_BIT0, params.VdencHEVCVP9TileSlicePar18);                         \
    DO_FIELD(DW17, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW17_BIT6, params.VdencHEVCVP9TileSlicePar19);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_WALKER_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_WALKER_STATE);

#define DO_FIELDS()                                                               \
    DO_FIELD(DW1, FirstSuperSlice, params.firstSuperSlice);                       \
    DO_FIELD(DW1, MbLcuStartXPosition, params.tileSliceStartLcuMbX);              \
    DO_FIELD(DW1, MbLcuStartYPosition, params.tileSliceStartLcuMbY);              \
                                                                                  \
    DO_FIELD(DW2, NextsliceMbLcuStartXPosition, params.nextTileSliceStartLcuMbX); \
    DO_FIELD(DW2, NextsliceMbStartYPosition, params.nextTileSliceStartLcuMbY)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VD_PIPELINE_FLUSH)
    {
        _MHW_SETCMD_CALLBASE(VD_PIPELINE_FLUSH);

#define DO_FIELDS()                                                           \
    DO_FIELD(DW1, HevcPipelineDone, params.waitDoneHEVC);                     \
    DO_FIELD(DW1, VdencPipelineDone, params.waitDoneVDENC);                   \
    DO_FIELD(DW1, MflPipelineDone, params.waitDoneMFL);                       \
    DO_FIELD(DW1, MfxPipelineDone, params.waitDoneMFX);                       \
    DO_FIELD(DW1, VdCommandMessageParserDone, params.waitDoneVDCmdMsgParser); \
    DO_FIELD(DW1, HevcPipelineCommandFlush, params.flushHEVC);                \
    DO_FIELD(DW1, VdencPipelineCommandFlush, params.flushVDENC);              \
    DO_FIELD(DW1, MflPipelineCommandFlush, params.flushMFL);                  \
    DO_FIELD(DW1, MfxPipelineCommandFlush, params.flushMFX)

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VD_PIPELINE_FLUSH_IMPL_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_AVC_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_AVC_SLICE_STATE);

#define DO_FIELDS()                                                     \
        DO_FIELD(DW1, RoundIntra, params.roundIntra);                   \
        DO_FIELD(DW1, RoundIntraEnable, params.roundIntraEnable);       \
        DO_FIELD(DW1, RoundInter, params.roundInter);                   \
        DO_FIELD(DW1, RoundInterEnable, params.roundInterEnable);       \
                                                                        \
        DO_FIELD(DW3, Log2WeightDenomLuma, params.log2WeightDenomLuma)
#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD1)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD1);

#define DO_FIELDS()                                                  \
    DO_FIELD(DW1, VDENC_CMD1_DW1_BIT0, params.vdencCmd1Par2[0]);     \
    DO_FIELD(DW1, VDENC_CMD1_DW1_BIT8, params.vdencCmd1Par2[1]);     \
    DO_FIELD(DW1, VDENC_CMD1_DW1_BIT16, params.vdencCmd1Par2[2]);    \
    DO_FIELD(DW1, VDENC_CMD1_DW1_BIT24, params.vdencCmd1Par2[3]);    \
                                                                     \
    DO_FIELD(DW2, VDENC_CMD1_DW2_BIT0, params.vdencCmd1Par2[4]);     \
    DO_FIELD(DW2, VDENC_CMD1_DW2_BIT8, params.vdencCmd1Par2[5]);     \
    DO_FIELD(DW2, VDENC_CMD1_DW2_BIT16, params.vdencCmd1Par2[6]);    \
    DO_FIELD(DW2, VDENC_CMD1_DW2_BIT24, params.vdencCmd1Par2[7]);    \
                                                                     \
    DO_FIELD(DW3, VDENC_CMD1_DW3_BIT0, params.vdencCmd1Par3[0]);     \
    DO_FIELD(DW3, VDENC_CMD1_DW3_BIT8, params.vdencCmd1Par3[1]);     \
    DO_FIELD(DW3, VDENC_CMD1_DW3_BIT16, params.vdencCmd1Par3[2]);    \
    DO_FIELD(DW3, VDENC_CMD1_DW3_BIT24, params.vdencCmd1Par3[3]);    \
                                                                     \
    DO_FIELD(DW4, VDENC_CMD1_DW4_BIT0, params.vdencCmd1Par3[4]);     \
    DO_FIELD(DW4, VDENC_CMD1_DW4_BIT8, params.vdencCmd1Par3[5]);     \
    DO_FIELD(DW4, VDENC_CMD1_DW4_BIT16, params.vdencCmd1Par3[6]);    \
    DO_FIELD(DW4, VDENC_CMD1_DW4_BIT24, params.vdencCmd1Par3[7]);    \
                                                                     \
    DO_FIELD(DW5, VDENC_CMD1_DW5_BIT0, params.vdencCmd1Par3[8]);     \
    DO_FIELD(DW5, VDENC_CMD1_DW5_BIT8, params.vdencCmd1Par3[9]);     \
    DO_FIELD(DW5, VDENC_CMD1_DW5_BIT16, params.vdencCmd1Par3[10]);   \
    DO_FIELD(DW5, VDENC_CMD1_DW5_BIT24, params.vdencCmd1Par3[11]);   \
                                                                     \
    DO_FIELD(DW6, VDENC_CMD1_DW6_BIT0, params.vdencCmd1Par4[0]);     \
    DO_FIELD(DW6, VDENC_CMD1_DW6_BIT8, params.vdencCmd1Par4[1]);     \
    DO_FIELD(DW6, VDENC_CMD1_DW6_BIT16, params.vdencCmd1Par4[2]);    \
    DO_FIELD(DW6, VDENC_CMD1_DW6_BIT24, params.vdencCmd1Par4[3]);    \
                                                                     \
    DO_FIELD(DW7, VDENC_CMD1_DW7_BIT0, params.vdencCmd1Par4[4]);     \
    DO_FIELD(DW7, VDENC_CMD1_DW7_BIT8, params.vdencCmd1Par4[5]);     \
    DO_FIELD(DW7, VDENC_CMD1_DW7_BIT16, params.vdencCmd1Par4[6]);    \
    DO_FIELD(DW7, VDENC_CMD1_DW7_BIT24, params.vdencCmd1Par4[7]);    \
                                                                     \
    DO_FIELD(DW8, VDENC_CMD1_DW8_BIT0, params.vdencCmd1Par4[8]);     \
    DO_FIELD(DW8, VDENC_CMD1_DW8_BIT8, params.vdencCmd1Par4[9]);     \
    DO_FIELD(DW8, VDENC_CMD1_DW8_BIT16, params.vdencCmd1Par4[10]);   \
    DO_FIELD(DW8, VDENC_CMD1_DW8_BIT24, params.vdencCmd1Par4[11]);   \
                                                                     \
    DO_FIELD(DW9, VDENC_CMD1_DW9_BIT0, params.vdencCmd1Par5);        \
    DO_FIELD(DW9, VDENC_CMD1_DW9_BIT8, params.vdencCmd1Par6);        \
    DO_FIELD(DW9, VDENC_CMD1_DW9_BIT16, params.vdencCmd1Par7);       \
                                                                     \
    DO_FIELD(DW10, VDENC_CMD1_DW10_BIT0, params.vdencCmd1Par8[0]);   \
    DO_FIELD(DW10, VDENC_CMD1_DW10_BIT8, params.vdencCmd1Par12[0]);  \
    DO_FIELD(DW10, VDENC_CMD1_DW10_BIT16, params.vdencCmd1Par9[0]);  \
    DO_FIELD(DW10, VDENC_CMD1_DW10_BIT24, params.vdencCmd1Par13[0]); \
                                                                     \
    DO_FIELD(DW11, VDENC_CMD1_DW11_BIT0, params.vdencCmd1Par10[0]);  \
    DO_FIELD(DW11, VDENC_CMD1_DW11_BIT8, params.vdencCmd1Par14[0]);  \
    DO_FIELD(DW11, VDENC_CMD1_DW11_BIT16, params.vdencCmd1Par11[0]); \
    DO_FIELD(DW11, VDENC_CMD1_DW11_BIT24, params.vdencCmd1Par15[0]); \
                                                                     \
    DO_FIELD(DW12, VDENC_CMD1_DW12_BIT0, params.vdencCmd1Par16);     \
    DO_FIELD(DW12, VDENC_CMD1_DW12_BIT8, params.vdencCmd1Par17);     \
    DO_FIELD(DW12, VDENC_CMD1_DW12_BIT16, params.vdencCmd1Par18);    \
    DO_FIELD(DW12, VDENC_CMD1_DW12_BIT24, params.vdencCmd1Par19);    \
                                                                     \
    DO_FIELD(DW13, VDENC_CMD1_DW13_BIT0, params.vdencCmd1Par20);     \
    DO_FIELD(DW13, VDENC_CMD1_DW13_BIT8, params.vdencCmd1Par21);     \
    DO_FIELD(DW13, VDENC_CMD1_DW13_BIT16, params.vdencCmd1Par22);    \
    DO_FIELD(DW13, VDENC_CMD1_DW13_BIT24, params.vdencCmd1Par23);    \
                                                                     \
    DO_FIELD(DW14, VDENC_CMD1_DW14_BIT0, params.vdencCmd1Par24);     \
    DO_FIELD(DW14, VDENC_CMD1_DW14_BIT8, params.vdencCmd1Par25);     \
    DO_FIELD(DW14, VDENC_CMD1_DW14_BIT16, params.vdencCmd1Par26);    \
    DO_FIELD(DW14, VDENC_CMD1_DW14_BIT24, params.vdencCmd1Par27);    \
                                                                     \
    DO_FIELD(DW15, VDENC_CMD1_DW15_BIT0, params.vdencCmd1Par28);     \
    DO_FIELD(DW15, VDENC_CMD1_DW15_BIT8, params.vdencCmd1Par29);     \
    DO_FIELD(DW15, VDENC_CMD1_DW15_BIT16, params.vdencCmd1Par30);    \
    DO_FIELD(DW15, VDENC_CMD1_DW15_BIT24, params.vdencCmd1Par31);    \
                                                                     \
    DO_FIELD(DW16, VDENC_CMD1_DW16_BIT0, params.vdencCmd1Par32);     \
    DO_FIELD(DW16, VDENC_CMD1_DW16_BIT8, params.vdencCmd1Par33);     \
    DO_FIELD(DW16, VDENC_CMD1_DW16_BIT16, params.vdencCmd1Par34);    \
    DO_FIELD(DW16, VDENC_CMD1_DW16_BIT24, params.vdencCmd1Par35);    \
                                                                     \
    DO_FIELD(DW17, VDENC_CMD1_DW17_BIT0, params.vdencCmd1Par36);     \
    DO_FIELD(DW17, VDENC_CMD1_DW17_BIT8, params.vdencCmd1Par37);     \
    DO_FIELD(DW17, VDENC_CMD1_DW17_BIT16, params.vdencCmd1Par38);    \
    DO_FIELD(DW17, VDENC_CMD1_DW17_BIT24, params.vdencCmd1Par39);    \
                                                                     \
    DO_FIELD(DW18, VDENC_CMD1_DW18_BIT0, params.vdencCmd1Par40);     \
    DO_FIELD(DW18, VDENC_CMD1_DW18_BIT8, params.vdencCmd1Par41);     \
    DO_FIELD(DW18, VDENC_CMD1_DW18_BIT16, params.vdencCmd1Par42);    \
    DO_FIELD(DW18, VDENC_CMD1_DW18_BIT24, params.vdencCmd1Par43);    \
                                                                     \
    DO_FIELD(DW19, VDENC_CMD1_DW19_BIT8, params.vdencCmd1Par44);     \
    DO_FIELD(DW19, VDENC_CMD1_DW19_BIT16, params.vdencCmd1Par45);    \
    DO_FIELD(DW19, VDENC_CMD1_DW19_BIT24, params.vdencCmd1Par46);    \
                                                                     \
    DO_FIELD(DW20, VDENC_CMD1_DW20_BIT0, params.vdencCmd1Par47);     \
    DO_FIELD(DW20, VDENC_CMD1_DW20_BIT8, params.vdencCmd1Par48);     \
    DO_FIELD(DW20, VDENC_CMD1_DW20_BIT16, params.vdencCmd1Par49);    \
    DO_FIELD(DW20, VDENC_CMD1_DW20_BIT24, params.vdencCmd1Par50);    \
                                                                     \
    DO_FIELD(DW21, VDENC_CMD1_DW21_BIT0, params.vdencCmd1Par51);     \
    DO_FIELD(DW21, VDENC_CMD1_DW21_BIT8, params.vdencCmd1Par52);     \
    DO_FIELD(DW21, VDENC_CMD1_DW21_BIT16, params.vdencCmd1Par53);    \
    DO_FIELD(DW21, VDENC_CMD1_DW21_BIT24, params.vdencCmd1Par54);    \
                                                                     \
    DO_FIELD(DW22, VDENC_CMD1_DW22_BIT0, params.vdencCmd1Par0);      \
    DO_FIELD(DW22, VDENC_CMD1_DW22_BIT16, params.vdencCmd1Par1);     \
                                                                     \
    DO_FIELD(DW23, VDENC_CMD1_DW23_BIT0, params.vdencCmd1Par55);     \
    DO_FIELD(DW23, VDENC_CMD1_DW23_BIT8, params.vdencCmd1Par56);     \
    DO_FIELD(DW23, VDENC_CMD1_DW23_BIT16, params.vdencCmd1Par57);    \
    DO_FIELD(DW23, VDENC_CMD1_DW23_BIT24, params.vdencCmd1Par58);    \
                                                                     \
    DO_FIELD(DW24, VDENC_CMD1_DW24_BIT0, params.vdencCmd1Par59);     \
    DO_FIELD(DW24, VDENC_CMD1_DW24_BIT8, params.vdencCmd1Par60);     \
    DO_FIELD(DW24, VDENC_CMD1_DW24_BIT16, params.vdencCmd1Par61);    \
    DO_FIELD(DW24, VDENC_CMD1_DW24_BIT24, params.vdencCmd1Par62);    \
                                                                     \
    DO_FIELD(DW25, VDENC_CMD1_DW25_BIT0, params.vdencCmd1Par63);     \
    DO_FIELD(DW25, VDENC_CMD1_DW25_BIT8, params.vdencCmd1Par64);     \
    DO_FIELD(DW25, VDENC_CMD1_DW25_BIT16, params.vdencCmd1Par65);    \
    DO_FIELD(DW25, VDENC_CMD1_DW25_BIT24, params.vdencCmd1Par66);    \
                                                                     \
    DO_FIELD(DW26, VDENC_CMD1_DW26_BIT0, params.vdencCmd1Par67);     \
    DO_FIELD(DW26, VDENC_CMD1_DW26_BIT8, params.vdencCmd1Par68);     \
    DO_FIELD(DW26, VDENC_CMD1_DW26_BIT16, params.vdencCmd1Par69);    \
    DO_FIELD(DW26, VDENC_CMD1_DW26_BIT24, params.vdencCmd1Par70);    \
                                                                     \
    DO_FIELD(DW27, VDENC_CMD1_DW27_BIT0, params.vdencCmd1Par71);     \
    DO_FIELD(DW27, VDENC_CMD1_DW27_BIT8, params.vdencCmd1Par72);     \
    DO_FIELD(DW27, VDENC_CMD1_DW27_BIT16, params.vdencCmd1Par73);    \
    DO_FIELD(DW27, VDENC_CMD1_DW27_BIT24, params.vdencCmd1Par74);    \
                                                                     \
    DO_FIELD(DW28, VDENC_CMD1_DW28_BIT0, params.vdencCmd1Par75);     \
    DO_FIELD(DW28, VDENC_CMD1_DW28_BIT8, params.vdencCmd1Par76);     \
    DO_FIELD(DW28, VDENC_CMD1_DW28_BIT16, params.vdencCmd1Par77);    \
    DO_FIELD(DW28, VDENC_CMD1_DW28_BIT24, params.vdencCmd1Par78);    \
                                                                     \
    DO_FIELD(DW29, VDENC_CMD1_DW29_BIT0, params.vdencCmd1Par79);     \
    DO_FIELD(DW29, VDENC_CMD1_DW29_BIT8, params.vdencCmd1Par80);     \
    DO_FIELD(DW29, VDENC_CMD1_DW29_BIT16, params.vdencCmd1Par81);    \
    DO_FIELD(DW29, VDENC_CMD1_DW29_BIT24, params.vdencCmd1Par82);    \
                                                                     \
    DO_FIELD(DW30, VDENC_CMD1_DW30_BIT0, params.vdencCmd1Par83);     \
    DO_FIELD(DW30, VDENC_CMD1_DW30_BIT8, params.vdencCmd1Par84);     \
    DO_FIELD(DW30, VDENC_CMD1_DW30_BIT16, params.vdencCmd1Par85);    \
    DO_FIELD(DW30, VDENC_CMD1_DW30_BIT24, params.vdencCmd1Par86);    \
                                                                     \
    DO_FIELD(DW31, VDENC_CMD1_DW31_BIT0, params.vdencCmd1Par87);     \
    DO_FIELD(DW31, VDENC_CMD1_DW31_BIT8, params.vdencCmd1Par88);     \
    DO_FIELD(DW31, VDENC_CMD1_DW31_BIT16, params.vdencCmd1Par89);    \
                                                                     \
    DO_FIELD(DW32, VDENC_CMD1_DW32_BIT0, params.vdencCmd1Par90);     \
    DO_FIELD(DW32, VDENC_CMD1_DW32_BIT8, params.vdencCmd1Par91);     \
    DO_FIELD(DW32, VDENC_CMD1_DW32_BIT16, params.vdencCmd1Par92)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD2)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD2);

#define DO_FIELDS()                                                                                      \
    DO_FIELD(DW1, FrameWidthInPixelsMinusOne, MOS_ALIGN_CEIL(params.width, 8) - 1);                      \
    DO_FIELD(DW1, FrameHeightInPixelsMinusOne, MOS_ALIGN_CEIL(params.height, 8) - 1);                    \
                                                                                                         \
    DO_FIELD(DW2, PictureType, params.pictureType);                                                      \
    DO_FIELD(DW2, TemporalMvpEnableFlag, params.temporalMvp);                                            \
    DO_FIELD(DW2, Collocatedfroml0Flag, params.collocatedFromL0);                                        \
    DO_FIELD(DW2, LongTermReferenceFlagsL0, params.longTermReferenceFlagsL0);                            \
    DO_FIELD(DW2, LongTermReferenceFlagsL1, params.longTermReferenceFlagsL1);                            \
    DO_FIELD(DW2, TransformSkip, params.transformSkip);                                                  \
    DO_FIELD(DW2, ConstrainedIntraPredFlag, params.constrainedIntraPred);                                \
                                                                                                         \
    DO_FIELD(DW3, FwdPocNumberForRefid0InL0, params.pocL0Ref0);                                          \
    DO_FIELD(DW3, BwdPocNumberForRefid0InL1, params.pocL1Ref0);                                          \
    DO_FIELD(DW3, PocNumberForRefid1InL0, params.pocL0Ref1);                                             \
    DO_FIELD(DW3, PocNumberForRefid1InL1, params.pocL1Ref1);                                             \
                                                                                                         \
    DO_FIELD(DW4, PocNumberForRefid2InL0, params.pocL0Ref2);                                             \
    DO_FIELD(DW4, PocNumberForRefid2InL1, params.pocL1Ref2);                                             \
    DO_FIELD(DW4, PocNumberForRefid3InL0, params.pocL0Ref3);                                             \
    DO_FIELD(DW4, PocNumberForRefid3InL1, params.pocL1Ref3);                                             \
                                                                                                         \
    DO_FIELD(DW5, StreaminRoiEnable, params.roiStreamIn);                                                \
    DO_FIELD(DW5, NumRefIdxL0Minus1, params.numRefL0 > 0 ? params.numRefL0 - 1 : 0);                     \
    DO_FIELD(DW5, NumRefIdxL1Minus1, params.numRefL1 > 0 ? params.numRefL1 - 1 : 0);                     \
    DO_FIELD(DW5, SubPelMode, params.subPelMode);                                                        \
                                                                                                         \
    DO_FIELD(DW7, SegmentationEnable, params.segmentation);                                              \
    DO_FIELD(DW7, SegmentationMapTemporalPredictionEnable, params.segmentationTemporal);                 \
    DO_FIELD(DW7, TilingEnable, params.tiling);                                                          \
    DO_FIELD(DW7, VdencStreamInEnable, params.vdencStreamIn);                                            \
    DO_FIELD(DW7, PakOnlyMultiPassEnable, params.pakOnlyMultiPass);                                      \
                                                                                                         \
    DO_FIELD(DW11, FwdRef0RefPic, params.frameIdxL0Ref0);                                                \
    DO_FIELD(DW11, FwdRef1RefPic, params.frameIdxL0Ref1);                                                \
    DO_FIELD(DW11, FwdRef2RefPic, params.frameIdxL0Ref2);                                                \
    DO_FIELD(DW11, BwdRef0RefPic, params.frameIdxL1Ref0);                                                \
                                                                                                         \
    DO_FIELD(DW16, MinQp, params.minQp);                                                                 \
    DO_FIELD(DW16, MaxQp, params.maxQp);                                                                 \
                                                                                                         \
    DO_FIELD(DW17, TemporalMVEnableForIntegerSearch, params.temporalMvEnableForIntegerSearch);           \
                                                                                                         \
    DO_FIELD(DW21, IntraRefreshPos, params.intraRefreshPos);                                             \
    DO_FIELD(DW21, IntraRefreshMBSizeMinusOne, params.intraRefreshMbSizeMinus1);                         \
    DO_FIELD(DW21, IntraRefreshMode, params.intraRefreshMode);                                           \
    DO_FIELD(DW21, IntraRefreshEnable, params.intraRefresh);                                             \
    DO_FIELD(DW21, QpAdjustmentForRollingI, params.qpAdjustmentForRollingI);                             \
                                                                                                         \
    DO_FIELD(DW24, QpForSeg0, params.qpForSegs[0]);                                                      \
    DO_FIELD(DW24, QpForSeg1, params.qpForSegs[1]);                                                      \
    DO_FIELD(DW24, QpForSeg2, params.qpForSegs[2]);                                                      \
    DO_FIELD(DW24, QpForSeg3, params.qpForSegs[3]);                                                      \
                                                                                                         \
    DO_FIELD(DW25, QpForSeg4, params.qpForSegs[4]);                                                      \
    DO_FIELD(DW25, QpForSeg5, params.qpForSegs[5]);                                                      \
    DO_FIELD(DW25, QpForSeg6, params.qpForSegs[6]);                                                      \
    DO_FIELD(DW25, QpForSeg7, params.qpForSegs[7]);                                                      \
                                                                                                         \
    DO_FIELD(DW26, Vp9DynamicSliceEnable, params.vp9DynamicSlice);                                       \
                                                                                                         \
    DO_FIELD(DW27, QpPrimeYDc, params.qpPrimeYDc);                                                       \
    DO_FIELD(DW27, QpPrimeYAc, params.qpPrimeYAc);                                                       \
                                                                                                         \
    DO_FIELD(DW36, IntraRefreshBoundaryRef0, params.intraRefreshBoundary[0]);                            \
    DO_FIELD(DW36, IntraRefreshBoundaryRef1, params.intraRefreshBoundary[1]);                            \
    DO_FIELD(DW36, IntraRefreshBoundaryRef2, params.intraRefreshBoundary[2]);                            \
                                                                                                         \
    DO_FIELD(DW61, Av1L0RefID0, params.av1RefId[0][0]);                                                  \
    DO_FIELD(DW61, Av1L1RefID0, params.av1RefId[1][0]);                                                  \
    DO_FIELD(DW61, Av1L0RefID1, params.av1RefId[0][1]);                                                  \
    DO_FIELD(DW61, Av1L1RefID1, params.av1RefId[1][1]);                                                  \
    DO_FIELD(DW61, Av1L0RefID2, params.av1RefId[0][2]);                                                  \
    DO_FIELD(DW61, Av1L1RefID2, params.av1RefId[1][2]);                                                  \
    DO_FIELD(DW61, Av1L0RefID3, params.av1RefId[0][3]);                                                  \
    DO_FIELD(DW61, Av1L1RefID3, params.av1RefId[1][3])

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD2_IMPL_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD3)
    {
    _MHW_SETCMD_CALLBASE(VDENC_CMD3);

    for (auto i = 0; i < 12; i++)
    {
        cmd.VDENC_CMD3_DW3_5[i] = params.vdencCmd3Par1[i];
        cmd.VDENC_CMD3_DW6_8[i] = params.vdencCmd3Par2[i];
    }

#define DO_FIELDS()                                                 \
    DO_FIELD(DW1_2, VDENC_CMD3_DW1_BIT0,  params.vdencCmd3Par0[0]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW1_BIT8,  params.vdencCmd3Par0[1]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW1_BIT16, params.vdencCmd3Par0[2]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW1_BIT24, params.vdencCmd3Par0[3]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW2_BIT0,  params.vdencCmd3Par0[4]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW2_BIT8,  params.vdencCmd3Par0[5]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW2_BIT16, params.vdencCmd3Par0[6]); \
    DO_FIELD(DW1_2, VDENC_CMD3_DW2_BIT24, params.vdencCmd3Par0[7]); \
                                                                    \
    DO_FIELD(DW10, VDENC_CMD3_DW10_BIT16, params.vdencCmd3Par3);    \
    DO_FIELD(DW10, VDENC_CMD3_DW10_BIT24, params.vdencCmd3Par4);    \
                                                                    \
    DO_FIELD(DW12, VDENC_CMD3_DW12_BIT0,  params.vdencCmd3Par5);    \
    DO_FIELD(DW12, VDENC_CMD3_DW12_BIT8,  params.vdencCmd3Par6);    \
    DO_FIELD(DW12, VDENC_CMD3_DW12_BIT16, params.vdencCmd3Par7);    \
    DO_FIELD(DW12, VDENC_CMD3_DW12_BIT24, params.vdencCmd3Par8);    \
                                                                    \
    DO_FIELD(DW13, VDENC_CMD3_DW13_BIT0,  params.vdencCmd3Par9);    \
    DO_FIELD(DW13, VDENC_CMD3_DW13_BIT8,  params.vdencCmd3Par10);   \
    DO_FIELD(DW13, VDENC_CMD3_DW13_BIT16, params.vdencCmd3Par11);   \
    DO_FIELD(DW13, VDENC_CMD3_DW13_BIT24, params.vdencCmd3Par12);   \
                                                                    \
    DO_FIELD(DW14, VDENC_CMD3_DW14_BIT8,  params.vdencCmd3Par13);   \
    DO_FIELD(DW14, VDENC_CMD3_DW14_BIT16, params.vdencCmd3Par14);   \
    DO_FIELD(DW14, VDENC_CMD3_DW14_BIT24, params.vdencCmd3Par15);   \
                                                                    \
    DO_FIELD(DW15, VDENC_CMD3_DW15_BIT8,  params.vdencCmd3Par16);   \
    DO_FIELD(DW15, VDENC_CMD3_DW15_BIT16, params.vdencCmd3Par17);   \
    DO_FIELD(DW15, VDENC_CMD3_DW15_BIT24, params.vdencCmd3Par18);   \
                                                                    \
    DO_FIELD(DW16, VDENC_CMD3_DW16_BIT16, params.vdencCmd3Par19);   \
                                                                    \
    DO_FIELD(DW17, VDENC_CMD3_DW17_BIT0,  params.vdencCmd3Par20);   \
    DO_FIELD(DW17, VDENC_CMD3_DW17_BIT8,  params.vdencCmd3Par23);   \
    DO_FIELD(DW17, VDENC_CMD3_DW17_BIT16, params.vdencCmd3Par21);   \
    DO_FIELD(DW17, VDENC_CMD3_DW17_BIT24, params.vdencCmd3Par22);   \
                                                                    \
    DO_FIELD(DW19, VDENC_CMD3_DW19_BIT8,  params.vdencCmd3Par24);   \
    DO_FIELD(DW19, VDENC_CMD3_DW19_BIT16, params.vdencCmd3Par25);   \
    DO_FIELD(DW19, VDENC_CMD3_DW19_BIT24, params.vdencCmd3Par26);   \
                                                                    \
    DO_FIELD(DW20, VDENC_CMD3_DW20_BIT0, params.vdencCmd3Par27);    \
    DO_FIELD(DW20, VDENC_CMD3_DW20_BIT8, params.vdencCmd3Par28);    \
                                                                    \
    DO_FIELD(DW21, VDENC_CMD3_DW21_BIT0, params.vdencCmd3Par29);    \
    DO_FIELD(DW21, VDENC_CMD3_DW21_BIT8, params.vdencCmd3Par30);    \
                                                                    \
    DO_FIELD(DW22, VDENC_CMD3_DW22_BIT0,  params.vdencCmd3Par31);   \
    DO_FIELD(DW22, VDENC_CMD3_DW22_BIT16, params.vdencCmd3Par32)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_AVC_IMG_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_AVC_IMG_STATE);

#define DO_FIELDS()                                                                                                 \
    DO_FIELD(DW1, PictureType, params.pictureType);                                                                 \
    DO_FIELD(DW1, Transform8X8Flag, params.transform8X8Flag);                                                       \
    DO_FIELD(DW1, colloc_mv_wr_en, params.colMVWriteEnable);                                                        \
    DO_FIELD(DW1, SubpelMode, params.subpelMode);                                                                   \
                                                                                                                    \
    DO_FIELD(DW2, colloc_mv_rd_en, params.colMVReadEnable);                                                         \
    DO_FIELD(DW2, BidirectionalWeight, params.bidirectionalWeight);                                                 \
                                                                                                                    \
    DO_FIELD(DW3, PictureHeightMinusOne, params.pictureHeightMinusOne);                                             \
    DO_FIELD(DW3, PictureWidth, params.pictureWidth);                                                               \
                                                                                                                    \
    DO_FIELD(DW5, FwdRefIdx0ReferencePicture, params.fwdRefIdx0ReferencePicture);                                   \
    DO_FIELD(DW5, BwdRefIdx0ReferencePicture, params.bwdRefIdx0ReferencePicture);                                   \
    DO_FIELD(DW5, FwdRefIdx1ReferencePicture, params.fwdRefIdx1ReferencePicture);                                   \
    DO_FIELD(DW5, FwdRefIdx2ReferencePicture, params.fwdRefIdx2ReferencePicture);                                   \
    DO_FIELD(DW5, NumberOfL0ReferencesMinusOne, params.numberOfL0ReferencesMinusOne);                               \
    DO_FIELD(DW5, NumberOfL1ReferencesMinusOne, params.numberOfL1ReferencesMinusOne);                               \
                                                                                                                    \
    DO_FIELD(DW6, IntraRefreshMbPos, params.intraRefreshMbPos);                                                     \
    DO_FIELD(DW6, IntraRefreshMbSizeMinusOne, params.intraRefreshMbSizeMinusOne);                                   \
    DO_FIELD(DW6, IntraRefreshEnableRollingIEnable, params.intraRefreshEnableRollingIEnable);                       \
    DO_FIELD(DW6, IntraRefreshMode, params.intraRefreshMode);                                                       \
    DO_FIELD(DW6, QpAdjustmentForRollingI, params.qpAdjustmentForRollingI);                                         \
                                                                                                                    \
    DO_FIELD(DW9, RoiQpAdjustmentForZone0, params.roiQpAdjustmentForZone0);                                         \
    DO_FIELD(DW9, RoiQpAdjustmentForZone1, params.roiQpAdjustmentForZone1);                                         \
    DO_FIELD(DW9, RoiQpAdjustmentForZone2, params.roiQpAdjustmentForZone2);                                         \
    DO_FIELD(DW9, RoiQpAdjustmentForZone3, params.roiQpAdjustmentForZone3);                                         \
                                                                                                                    \
    DO_FIELD(DW12, MinQp, params.minQp);                                                                            \
    DO_FIELD(DW12, MaxQp, params.maxQp);                                                                            \
                                                                                                                    \
    DO_FIELD(DW13, RoiEnable, params.roiEnable);                                                                    \
    DO_FIELD(DW13, MbLevelQpEnable, params.mbLevelQpEnable);                                                        \
    DO_FIELD(DW13, MbLevelDeltaQpEnable, params.mbLevelDeltaQpEnable);                                              \
    DO_FIELD(DW13, LongtermReferenceFrameBwdRef0Indicator, params.longtermReferenceFrameBwdRef0Indicator);          \
                                                                                                                    \
    DO_FIELD(DW14, QpPrimeY, params.qpPrimeY);                                                                      \
    DO_FIELD(DW14, TrellisQuantEn, params.trellisQuantEn);                                                          \
                                                                                                                    \
    DO_FIELD(DW15, PocNumberForCurrentPicture, params.pocNumberForCurrentPicture);                                  \
    DO_FIELD(DW16, PocNumberForFwdRef0, params.pocNumberForFwdRef0);                                                \
    DO_FIELD(DW17, PocNumberForFwdRef1, params.pocNumberForFwdRef1);                                                \
    DO_FIELD(DW18, PocNumberForFwdRef2, params.pocNumberForFwdRef2);                                                \
    DO_FIELD(DW19, PocNumberForBwdRef0, params.pocNumberForBwdRef0)

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_AVC_IMG_STATE_IMPL_EXT)
    
#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__Impl)
};
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_H__
