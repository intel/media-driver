/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     mhw_vdbox_vvcp_impl.h
//! \brief    MHW VDBOX VVCP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_VVCP_IMPL_H__
#define __MHW_VDBOX_VVCP_IMPL_H__

#include "mhw_vdbox_vvcp_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace vdbox
{
namespace vvcp
{

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _VVCP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);
        uint32_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

    MOS_STATUS GetVvcpBufSize(VvcpBufferType bufferType, VvcpBufferSizePar* vvcpBufSizeParam) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(vvcpBufSizeParam);

        uint32_t bufferSize      = 0;
        uint32_t picWidthInCtus  = 0;
        uint32_t picHeightInCtus = 0;
        uint8_t  bitDepthIdx     = (vvcpBufSizeParam->m_bitDepthIdc == 2) ? 1 : 0;

        switch (bufferType)
        {
        case vcMvTemporalBuffer:
            bufferSize = 8 * (((vvcpBufSizeParam->m_picWidth - 1) >> 6) + 1) * (((vvcpBufSizeParam->m_picHeight - 1) >> 6) + 1);
            break;
        case vcedLineBuffer:
        case vcmvLineBuffer:
        case vcprLineBuffer:
        case vclfYLineBuffer:
        case vclfULineBuffer:
        case vclfVLineBuffer:
        case vcSaoYLineBuffer:
        case vcSaoULineBuffer:
        case vcSaoVLineBuffer:
        case vcAlfLineBuffer:
            bufferSize = CodecVvcBufferSize[vvcpBufSizeParam->m_spsChromaFormatIdc][bufferType].m_clPerCtu[vvcpBufSizeParam->m_spsLog2CtuSizeMinus5][bitDepthIdx] * vvcpBufSizeParam->m_maxTileWidthInCtus + CodecVvcBufferSize[vvcpBufSizeParam->m_spsChromaFormatIdc][bufferType].m_extraCl;
            break;
        case vclfYTileRowBuffer:
        case vclfUTileRowBuffer:
        case vclfVTileRowBuffer:
        case vcSaoYTileRowBuffer:
        case vcSaoUTileRowBuffer:
        case vcSaoVTileRowBuffer:
        case vcAlfTileRowBuffer:
            picWidthInCtus = MOS_ROUNDUP_SHIFT(vvcpBufSizeParam->m_picWidth, vvcpBufSizeParam->m_spsLog2CtuSizeMinus5 + 5);
            bufferSize     = CodecVvcBufferSize[vvcpBufSizeParam->m_spsChromaFormatIdc][bufferType].m_clPerCtu[vvcpBufSizeParam->m_spsLog2CtuSizeMinus5][bitDepthIdx] * picWidthInCtus + CodecVvcBufferSize[vvcpBufSizeParam->m_spsChromaFormatIdc][bufferType].m_extraCl;
            break;
        case vclfYTileColumnBuffer:
        case vclfUTileColumnBuffer:
        case vclfVTileColumnBuffer:
        case vcSaoYTileColumnBuffer:
        case vcSaoUTileColumnBuffer:
        case vcSaoVTileColumnBuffer:
        case vcAlfYTileColumnBuffer:
        case vcAlfUTileColumnBuffer:
        case vcAlfVTileColumnBuffer:
            picHeightInCtus = MOS_ROUNDUP_SHIFT(vvcpBufSizeParam->m_picHeight, vvcpBufSizeParam->m_spsLog2CtuSizeMinus5 + 5);
            bufferSize      = CodecVvcBufferSize[vvcpBufSizeParam->m_spsChromaFormatIdc][bufferType].m_clPerCtu[vvcpBufSizeParam->m_spsLog2CtuSizeMinus5][bitDepthIdx] * picHeightInCtus + CodecVvcBufferSize[vvcpBufSizeParam->m_spsChromaFormatIdc][bufferType].m_extraCl;
            break;
        default:
            return MOS_STATUS_INVALID_PARAMETER;
        }

        vvcpBufSizeParam->m_bufferSize = bufferSize * MHW_CACHELINE_SIZE;

        return MOS_STATUS_SUCCESS;
    }

    bool IsRowStoreCachingSupported() override
    {
        return m_rowstoreCachingSupported;
    }

    bool IsBufferRowstoreCacheEnabled(VvcpBufferType bufferType) override
    {

        bool rowstoreCacheEnabled = false;
        switch (bufferType)
        {
        case vcedLineBuffer:
            rowstoreCacheEnabled = m_edlbRowstoreCache.enabled ? true : false;
            break;
        case vcmvLineBuffer:
            rowstoreCacheEnabled = m_mvlbRowstoreCache.enabled ? true : false;
            break;
        case vcprLineBuffer:
            rowstoreCacheEnabled = m_prlbRowstoreCache.enabled ? true : false;
            break;
        case vclfYLineBuffer:
            rowstoreCacheEnabled = m_lfylbRowstoreCache.enabled ? true : false;
            break;
        case vclfULineBuffer:
            rowstoreCacheEnabled = m_lfulbRowstoreCache.enabled ? true : false;
            break;
        case vclfVLineBuffer:
            rowstoreCacheEnabled = m_lfvlbRowstoreCache.enabled ? true : false;
            break;
        case vcSaoYLineBuffer:
            rowstoreCacheEnabled = m_saylbRowstoreCache.enabled ? true : false;
            break;
        case vcSaoULineBuffer:
            rowstoreCacheEnabled = m_saulbRowstoreCache.enabled ? true : false;
            break;
        case vcSaoVLineBuffer:
            rowstoreCacheEnabled = m_savlbRowstoreCache.enabled ? true : false;
            break;
        case vcAlfLineBuffer:
            rowstoreCacheEnabled = m_alflbRowstoreCache.enabled ? true : false;
            break;
        default:
            return false;
        }

        return rowstoreCacheEnabled;
    }

    MOS_STATUS GetRowstoreCachingAddrs(PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(rowstoreParams);

        uint32_t picWidth = rowstoreParams->dwPicWidth;
        uint8_t  ctuSize  = rowstoreParams->ucLCUSize;

        //EDLB
        if (m_edlbRowstoreCache.supported)
        {
            m_edlbRowstoreCache.enabled  = true;
            m_edlbRowstoreCache.dwAddress = 0;
        }

        //MVLB
        if (m_mvlbRowstoreCache.supported)
        {
            m_mvlbRowstoreCache.enabled  = true;
            if (ctuSize > 32)
            {
                m_mvlbRowstoreCache.dwAddress = (picWidth <= 4096) ? 64 : 132;
            }
            else
            {
                m_mvlbRowstoreCache.dwAddress = (picWidth <= 4064) ? 127 : 264;
            }
        }

        //PRLB
        if (m_prlbRowstoreCache.supported)
        {
            if (ctuSize > 32)
            {
                m_prlbRowstoreCache.enabled  = true;
                m_prlbRowstoreCache.dwAddress = (picWidth <= 4096) ? 320 : 660;
            }
            else
            {
                m_prlbRowstoreCache.enabled  = (picWidth <= 4064) ? true : false;
                m_prlbRowstoreCache.dwAddress = (picWidth <= 4064) ? 381 : 0;
            }
        }

        //LFYLB
        if (m_lfylbRowstoreCache.supported)
        {
            if (ctuSize > 32)
            {
                m_lfylbRowstoreCache.enabled  = (picWidth <= 4096) ? true : false;
                m_lfylbRowstoreCache.dwAddress = (picWidth <= 4096) ? 576 : 0;
            }
            else
            {
                m_lfylbRowstoreCache.enabled  = (picWidth <= 4064) ? true : false;
                m_lfylbRowstoreCache.dwAddress = (picWidth <= 4064) ? 635 : 0;
            }
        }

        //LFULB
        if (m_lfulbRowstoreCache.supported)
        {
            m_lfulbRowstoreCache.enabled = true;
            if (ctuSize > 32)
            {
                m_lfulbRowstoreCache.dwAddress = (picWidth <= 4096) ? 1280 : 1188;
            }
            else
            {
                m_lfulbRowstoreCache.dwAddress = (picWidth <= 4064) ? 1397 : 792;
            }
        }

        //LFVLB
        if (m_lfvlbRowstoreCache.supported)
        {
            m_lfvlbRowstoreCache.enabled = true;
            if (ctuSize > 32)
            {
                m_lfvlbRowstoreCache.dwAddress = (picWidth <= 4096) ? 1472 : 1584;
            }
            else
            {
                m_lfvlbRowstoreCache.dwAddress = (picWidth <= 4064) ? 1651 : 1320;
            }
        }

        //SAYLB
        if (m_saylbRowstoreCache.supported)
        {
            if (ctuSize > 32)
            {
                m_saylbRowstoreCache.enabled  = (picWidth <= 4096) ? true : false;
                m_saylbRowstoreCache.dwAddress = (picWidth <= 4096) ? 1664 : 0;
            }
            else
            {
                m_saylbRowstoreCache.enabled  = (picWidth <= 4064) ? true : false;
                m_saylbRowstoreCache.dwAddress = (picWidth <= 4064) ? 1905 : 0;
            }
        }

        //SAULB
        if (m_saulbRowstoreCache.supported)
        {
            m_saulbRowstoreCache.enabled  = true;
            if (ctuSize > 32)
            {
                m_saulbRowstoreCache.dwAddress = (picWidth <= 4096) ? 1857 : 1980;
            }
            else
            {
                m_saulbRowstoreCache.dwAddress = (picWidth <= 4064) ? 2160 : 1848;
            }
        }

        //SAVLB
        if (m_savlbRowstoreCache.supported)
        {
            m_savlbRowstoreCache.enabled  = true;
            if (ctuSize > 32)
            {
                m_savlbRowstoreCache.dwAddress = (picWidth <= 4096) ? 1986 : 2245;
            }
            else
            {
                m_savlbRowstoreCache.dwAddress = (picWidth <= 4064) ? 2288 : 2113;
            }
        }

        //ALFLB
        if (m_alflbRowstoreCache.supported)
        {
            if (ctuSize > 32)
            {
                m_alflbRowstoreCache.enabled  = (picWidth <= 4096) ? true : false;
                m_alflbRowstoreCache.dwAddress = (picWidth <= 4096) ? 2115 : 0;
            }
            else
            {
                m_alflbRowstoreCache.enabled  = (picWidth <= 4064) ? true : false;
                m_alflbRowstoreCache.dwAddress = (picWidth <= 4064) ? 2416 : 0;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetVvcpStateCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetVvcpSliceLvlCmdSize(
        uint32_t *sliceLvlCmdSize) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetVvcpPrimitiveCmdSize(
        uint32_t *sliceCommandsSize,
        uint32_t *slicePatchListSize,
        uint32_t *tileCommandsSize,
        uint32_t *tilePatchListSize) override
    {
        // Just return success here, please implement logic in platform sepecific impl class.
        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = Itf;
    MhwCpInterface *m_cpItf = nullptr;

    bool m_decodeInUse = true;  //!< Flag to indicate if the interface is for decoder or encoder use

    Impl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
        m_cpItf = cpItf;

        InitRowstoreUserFeatureSettings();
        InitMmioRegisters();
    }

    virtual ~Impl()
    {
        MHW_FUNCTION_ENTER;

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_edlbRowstoreCache.enabled ||
            m_mvlbRowstoreCache.enabled ||
            m_prlbRowstoreCache.enabled ||
            m_lfylbRowstoreCache.enabled ||
            m_lfulbRowstoreCache.enabled ||
            m_lfvlbRowstoreCache.enabled ||
            m_saylbRowstoreCache.enabled ||
            m_saulbRowstoreCache.enabled ||
            m_savlbRowstoreCache.enabled ||
            m_alflbRowstoreCache.enabled)
        {
            // Report rowstore cache usage status to regkey
            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED,
                1,
                MediaUserSetting::Group::Device);
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

    }

    virtual uint32_t GetMocsValue(MOS_HW_RESOURCE_DEF hwResType) = 0;

    RowStoreCache m_edlbRowstoreCache  = {};  //!< VCED Line Buffer(EDLB)
    RowStoreCache m_mvlbRowstoreCache  = {};  //!< VCMV Line Buffer (MVLB)
    RowStoreCache m_prlbRowstoreCache  = {};  //!< VCPR Line Buffer (PRLB)
    RowStoreCache m_lfylbRowstoreCache = {};  //!< VCLF Y Line Buffer (LFYLB)
    RowStoreCache m_lfulbRowstoreCache = {};  //!< VCLF U Line Buffer (LFULB)
    RowStoreCache m_lfvlbRowstoreCache = {};  //!< VCLF V Line Buffer (LFVLB)
    RowStoreCache m_saylbRowstoreCache = {};  //!< VCSAO Y Line Buffer (SAYLB)
    RowStoreCache m_saulbRowstoreCache = {};  //!< VCSAO U Line Buffer (SAULB)
    RowStoreCache m_savlbRowstoreCache = {};  //!< VCSAO V Line Buffer (SAVLB)
    RowStoreCache m_alflbRowstoreCache = {};  //!< VCALF Line Buffer (ALFLB)

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};  //!< Cacheability settings
private:

    MmioRegistersVvcp                m_mmioRegisters[MHW_VDBOX_NODE_MAX]                        = {};     //!< VVCP mmio registers
    bool                             m_rowstoreCachingSupported                                 = false;  //!< Flag to indicate if row store cache is supported

    void InitRowstoreUserFeatureSettings()
    {
        m_rowstoreCachingSupported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        auto userSettingPtr = m_osItf->pfnGetUserSettingInstance(m_osItf);

        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcEdlbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcMvlbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcPrlbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcLfylbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcLfulbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcLfvlbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcSaylbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcSaulbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcSavlbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);
        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            "DisableVvcAlflbRowstoreCache",
            MediaUserSetting::Group::Device,
            0,
            true);

        {
            MediaUserSetting::Value outValue;
            ReadUserSettingForDebug(userSettingPtr,
                outValue,
                "Disable RowStore Cache",
                MediaUserSetting::Group::Device);
            m_rowstoreCachingSupported = !(outValue.Get<bool>());
        }
#endif  // _DEBUG || _RELEASE_INTERNAL

        if (m_rowstoreCachingSupported)  // updated
        {
            m_edlbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcEdlbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_edlbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_mvlbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcMvlbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_mvlbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_prlbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcPrlbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_prlbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_lfylbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcLfylbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_lfylbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_lfulbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcLfulbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_lfulbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_lfvlbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcLfvlbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_lfvlbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_saylbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcSaylbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_saylbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_saulbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcSaulbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_saulbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_savlbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcSavlbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_savlbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL

            m_alflbRowstoreCache.supported = true;
#if (_DEBUG || _RELEASE_INTERNAL)
            {
                MediaUserSetting::Value outValue;
                ReadUserSettingForDebug(userSettingPtr,
                    outValue,
                    "DisableVvcAlflbRowstoreCache",
                    MediaUserSetting::Group::Device);
                m_alflbRowstoreCache.supported = !(outValue.Get<bool>());
            }
#endif  // _DEBUG || _RELEASE_INTERNAL
        }
    }

    void InitMmioRegisters()
    {
        // TODO: Enalbe VVC new mechanism for MMIO
    }

protected:
    
    _MHW_SETCMD_OVERRIDE_DECL(VVCP_VD_CONTROL_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_VD_CONTROL_STATE);

#define DO_FIELDS()                                                                                 \
    DO_FIELD(VdControlStateBody.DW0, PipelineInitialization, params.pipelineInitialization);        \
    DO_FIELD(VdControlStateBody.DW1, MemoryImplicitFlush, params.memoryImplicitFlush);              \
    DO_FIELD(VdControlStateBody.DW1, PipeScalableModePipeLock, params.pipeScalableModePipeLock);    \
    DO_FIELD(VdControlStateBody.DW1, PipeScalableModePipeUnlock, params.pipeScalableModePipeUnlock);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VVCP_PIPE_MODE_SELECT);

        MHW_MI_CHK_STATUS(m_cpItf->SetProtectionSettingsForReservedPipeModeSelect((uint32_t *)&cmd));

#define DO_FIELDS()                                                                       \
    DO_FIELD(DW1, CodecSelect, params.codecSelect);                                       \
    DO_FIELD(DW1, PicStatusErrorReportEnable, params.picStatusErrorReportEnable ? 1 : 0); \
    DO_FIELD(DW1, CodecStandardSelect, params.codecStandardSelect);                       \
    DO_FIELD(DW2, PicStatusErrorReportId, params.picStatusErrorReportId);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_SURFACE_STATE);

#define DO_FIELDS()                                                   \
    DO_FIELD(DW1, SurfaceId, params.surfaceId);                       \
    DO_FIELD(DW1, SurfacePitchMinus1, params.surfacePitchMinus1);     \
    DO_FIELD(DW2, SurfaceFormat, params.surfaceFormat);               \
    DO_FIELD(DW2, YOffsetForUCbInPixel, params.yOffsetForUCbInPixel); \
    DO_FIELD(DW4, CompressionFormat, params.compressionFormat)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        // 1. MHW_VDBOX_HCP_GENERAL_STATE_SHIFT(6) may not work with DecodedPicture
        // since it needs to be 4k aligned
        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        // Decoded Output Frame Buffer
        cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
            GetMocsValue(MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC);
        cmd.DecodedPictureMemoryAddressAttributes.DW0.Tilemode = GetHwTileType(params.decodedPic->TileType, params.decodedPic->TileModeGMM, params.decodedPic->bGMMTileEnabled);

        resourceParams.presResource    = &(params.decodedPic->OsResource);
        resourceParams.dwOffset        = params.decodedPic->dwOffset;
        resourceParams.pdwCmd          = (cmd.DecodedPictureBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable     = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osItf,
            m_currentCmdBuf,
            &resourceParams));

        // Reference frames
        for (uint32_t i = 0; i < vvcMaxNumRefFrame; i++)
        {
            // Reference Picture Buffer
            if (params.references[i] != nullptr)
            {
                MOS_SURFACE details = {};
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(m_osItf->pfnGetResourceInfo(m_osItf, params.references[i], &details));
                cmd.ReferencePicture[i].ReferencePictureMemoryAddressAttributes.DW0.Tilemode = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
                cmd.ReferencePicture[i].ReferencePictureMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                    GetMocsValue(MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC);

                resourceParams.presResource       = params.references[i];
                resourceParams.pdwCmd             = (cmd.ReferencePicture[i].ReferencePictureBaseAddress0Refaddr.DW0_1.Value);
                resourceParams.dwOffset           = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd    = (i * 3) + 4;
                resourceParams.bIsWritable        = false;
                resourceParams.dwSharedMocsOffset = 2;

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    m_osItf,
                    m_currentCmdBuf,
                    &resourceParams));
            }
        }

        // Collocated MV Temporal buffers
        for (uint32_t i = 0; i < vvcMaxNumRefFrame; i++)
        {
            if (params.colMvTemporalBuffer[i] != nullptr)
            {
                cmd.CollocatedMotionVectorTemporalBuffer[i].ReferencePictureMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                    GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

                resourceParams.presResource       = params.colMvTemporalBuffer[i];
                resourceParams.dwOffset           = 0;
                resourceParams.pdwCmd             = (cmd.CollocatedMotionVectorTemporalBuffer[i].ReferencePictureBaseAddress0Refaddr.DW0_1.Value);
                resourceParams.dwLocationInCmd    = (i * 3) + 49;
                resourceParams.bIsWritable        = true;
                resourceParams.dwSharedMocsOffset = 2;

                InitMocsParams(resourceParams, &cmd.CollocatedMotionVectorTemporalBuffer[i].ReferencePictureMemoryAddressAttributes.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    m_osItf,
                    m_currentCmdBuf,
                    &resourceParams));
            }
        }

        // Current Motion Vector Temporal Buffer
        if (params.curMvTemporalBuffer != nullptr)
        {
            cmd.CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.curMvTemporalBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.CurrentMotionVectorTemporalBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 94;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // APS Scaling List Buffer
        if (params.apsScalingListDataBuffer != nullptr)
        {
            cmd.ApsScalinglistDataBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.apsScalingListDataBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.ApsScalinglistDataBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 97;
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.ApsScalinglistDataBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // APS ALF buffer
        if (params.apsAlfBuffer != nullptr)
        {
            cmd.ApsAlfDataBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.apsAlfBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.ApsAlfDataBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 100;
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.ApsAlfDataBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // SPS Chroma QP buffer
        if (params.spsChromaQpTableBuffer != nullptr)
        {
            cmd.SpsChromaqpTableBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);
            resourceParams.presResource                                                                                = params.spsChromaQpTableBuffer;
            resourceParams.dwOffset                                                                                    = 0;
            resourceParams.pdwCmd                                                                                      = (cmd.SpsChromaqpTableBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd                                                                             = 103;
            resourceParams.bIsWritable                                                                                 = false;

            InitMocsParams(resourceParams, &cmd.SpsChromaqpTableBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // Rowstore cache
        // VCED Line Buffer (EDLB)
        if (m_edlbRowstoreCache.enabled)
        {
            cmd.VcedLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcedLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcedLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_edlbRowstoreCache.dwAddress;
        }
        else if (params.vcedLineBuffer != nullptr)
        {
            cmd.VcedLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcedLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcedLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 106;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcedLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCMV Line Buffer (MVLB)
        if (m_mvlbRowstoreCache.enabled)
        {
            cmd.VcmvLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcmvLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcmvLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_mvlbRowstoreCache.dwAddress;
        }
        else if (params.vcmvLineBuffer != nullptr)
        {
            cmd.VcmvLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcmvLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcmvLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 109;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcmvLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCPR Line Buffer (PRLB)
        if (m_prlbRowstoreCache.enabled)
        {
            cmd.VcprLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcprLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcprLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_prlbRowstoreCache.dwAddress;
        }
        else if (params.vcprLineBuffer != nullptr)
        {
            cmd.VcprLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcprLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcprLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 112;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcprLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF Y Line Buffer (LFYLB)
        if (m_lfylbRowstoreCache.enabled)
        {
            cmd.VclfYLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VclfYLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VclfYLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_lfylbRowstoreCache.dwAddress;
        }
        else if (params.vclfYLineBuffer != nullptr)
        {
            cmd.VclfYLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfYLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfYLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 115;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfYLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF Y Tile Row Buffer (LFYTR)
        if (params.vclfYTileRowBuffer != nullptr)
        {
            cmd.VclfYTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfYTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfYTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 118;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfYTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF Y Tile Column Buffer (LFYTC)
        if (params.vclfYTileColumnBuffer != nullptr)
        {
            cmd.VclfYTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfYTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfYTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 121;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfYTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF U Line Buffer (LFULB)
        if (m_lfulbRowstoreCache.enabled)
        {
            cmd.VclfULineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VclfULineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VclfULineBufferBaseAddress.DW0_1.BaseAddress                                           = m_lfulbRowstoreCache.dwAddress;
        }
        else if (params.vclfULineBuffer != nullptr)
        {
            cmd.VclfULineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfULineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfULineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 124;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfULineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF U Tile Row Buffer (LFUTR)
        if (params.vclfUTileRowBuffer != nullptr)
        {
            cmd.VclfUTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfUTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfUTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 127;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfUTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF U Tile Column Buffer (LFUTC)
        if (params.vclfUTileColumnBuffer != nullptr)
        {
            cmd.VclfUTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfUTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfUTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 130;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfUTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF V Line Buffer (LFVLB)
        if (m_lfvlbRowstoreCache.enabled)
        {
            cmd.VclfVLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VclfVLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VclfVLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_lfvlbRowstoreCache.dwAddress;
        }
        else if (params.vclfVLineBuffer != nullptr)
        {
            cmd.VclfVLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfVLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfVLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 133;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfVLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF V Tile Row Buffer (LFVTR)
        if (params.vclfVTileRowBuffer != nullptr)
        {
            cmd.VclfVTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfVTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfVTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 136;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfVTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCLF V Tile Column Buffer (LFVTC)
        if (params.vclfVTileColumnBuffer != nullptr)
        {
            cmd.VclfVTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vclfVTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VclfVTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 139;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VclfVTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO Y Line Buffer (SAYLB)
        if (m_saylbRowstoreCache.enabled)
        {
            cmd.VcsaoYLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcsaoYLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcsaoYLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_saylbRowstoreCache.dwAddress;
        }
        else if (params.vcSaoYLineBuffer != nullptr)
        {
            cmd.VcsaoYLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoYLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoYLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 142;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoYLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO Y Tile Row Buffer (SAYTR)
        if (params.vcSaoYTileRowBuffer != nullptr)
        {
            cmd.VcsaoYTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoYTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoYTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 145;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoYTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO Y Tile Column Buffer (SAYTC)
        if (params.vcSaoYTileColumnBuffer != nullptr)
        {
            cmd.VcsaoYTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoYTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoYTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 148;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoYTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO U Line Buffer (SAULB)
        if (m_saulbRowstoreCache.enabled)
        {
            cmd.VcsaoULineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcsaoULineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcsaoULineBufferBaseAddress.DW0_1.BaseAddress                                           = m_saulbRowstoreCache.dwAddress;
        }
        else if (params.vcSaoULineBuffer != nullptr)
        {
            cmd.VcsaoULineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoULineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoULineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 151;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoULineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO U Tile Row Buffer (SAUTR)
        if (params.vcSaoUTileRowBuffer != nullptr)
        {
            cmd.VcsaoUTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoUTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoUTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 154;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoUTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO U Tile Column Buffer (SAUTC)
        if (params.vcSaoUTileColumnBuffer != nullptr)
        {
            cmd.VcsaoUTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoUTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoUTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 157;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoUTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO V Line Buffer (SAVLB)
        if (m_savlbRowstoreCache.enabled)
        {
            cmd.VcsaoVLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcsaoVLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcsaoVLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_savlbRowstoreCache.dwAddress;
        }
        else if (params.vcSaoVLineBuffer != nullptr)
        {
            cmd.VcsaoVLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoVLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoVLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 160;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoVLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO V Tile Row Buffer (SAVTR)
        if (params.vcSaoVTileRowBuffer != nullptr)
        {
            cmd.VcsaoVTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoVTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoVTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 163;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoVTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCSAO V Tile Column Buffer (SAVTC)
        if (params.vcSaoVTileColumnBuffer != nullptr)
        {
            cmd.VcsaoVTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcSaoVTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcsaoVTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 166;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcsaoVTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCALF Line Buffer (ALFLB)
        if (m_alflbRowstoreCache.enabled)
        {
            cmd.VcalfLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.VcalfLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
            cmd.VcalfLineBufferBaseAddress.DW0_1.BaseAddress                                           = m_alflbRowstoreCache.dwAddress;
        }
        else if (params.vcAlfLineBuffer != nullptr)
        {
            cmd.VcalfLineBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcAlfLineBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcalfLineBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 169;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcalfLineBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCALF Tile Row Buffer (ALFTR)
        if (params.vcAlfTileRowBuffer != nullptr)
        {
            cmd.VcalfTileRowBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcAlfTileRowBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcalfTileRowBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 172;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcalfTileRowBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCALF Y Tile Column Buffer (ALYTC)
        if (params.vcAlfYTileColumnBuffer != nullptr)
        {
            cmd.VcalfYTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcAlfYTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcalfYTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 175;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcalfYTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCALF U Tile Column Buffer (ALUTC)
        if (params.vcAlfUTileColumnBuffer != nullptr)
        {
            cmd.VcalfUTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcAlfUTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcalfUTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 178;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcalfUTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        // VCALF V Tile Column Buffer (ALVTC)
        if (params.vcAlfVTileColumnBuffer != nullptr)
        {
            cmd.VcalfVTileColumnBufferMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                GetMocsValue(MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED);

            resourceParams.presResource    = params.vcAlfVTileColumnBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.VcalfVTileColumnBufferBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 181;
            resourceParams.bIsWritable     = true;

            InitMocsParams(resourceParams, &cmd.VcalfVTileColumnBufferMemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osItf,
                m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_IND_OBJ_BASE_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_IND_OBJ_BASE_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};
        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        // mode specific settings
        MHW_MI_CHK_NULL(params.presDataBuffer);

        resourceParams.presResource    = params.presDataBuffer;
        resourceParams.dwOffset        = params.dwDataOffset;
        resourceParams.pdwCmd          = &(cmd.VvcpIndirectBitstreamObjectBaseAddress.DW0_1.Value[0]);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwSize          = params.dwDataSize;
        resourceParams.bIsWritable     = false;

        // No Indirect Bitstream Object Access Upper Bound defined for VVC
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

        InitMocsParams(resourceParams, &cmd.VvcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value, 1, 6);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osItf,
            m_currentCmdBuf,
            &resourceParams));

        cmd.VvcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
            GetMocsValue(MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_PIC_STATE);

        // ALF
        for (auto i = 0; i < 8; i++)
        {
            if (params.dActiveapsid & (1 << i))
            {
                cmd.DPicApsalfparamsets[i].DW0.AlfLumaFilterSignalFlag          = params.alfApsArray[i].m_alfFlags.m_fields.m_alfLumaFilterSignalFlag;
                cmd.DPicApsalfparamsets[i].DW0.AlfChromaFilterSignalFlag        = params.alfApsArray[i].m_alfFlags.m_fields.m_alfChromaFilterSignalFlag;
                cmd.DPicApsalfparamsets[i].DW0.AlfCcCbFilterSignalFlag          = params.alfApsArray[i].m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag;
                cmd.DPicApsalfparamsets[i].DW0.AlfCcCrFilterSignalFlag          = params.alfApsArray[i].m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag;  //!< alf_cc_cr_filter_signal_flag
                cmd.DPicApsalfparamsets[i].DW0.AlfLumaClipFlag                  = params.alfApsArray[i].m_alfFlags.m_fields.m_alfLumaClipFlag;          //!< alf_luma_clip_flag
                cmd.DPicApsalfparamsets[i].DW0.AlfChromaClipFlag                = params.alfApsArray[i].m_alfFlags.m_fields.m_alfChromaClipFlag;        //!< alf_chroma_clip_flag
                cmd.DPicApsalfparamsets[i].DW0.Reserved6                        = 0;                                                                    //!< Reserved
                cmd.DPicApsalfparamsets[i].DW0.AlfLumaNumFiltersSignalledMinus1 = params.alfApsArray[i].m_alfLumaNumFiltersSignalledMinus1;             //!< alf_luma_num_filters_signalled_minus1
                cmd.DPicApsalfparamsets[i].DW0.AlfChromaNumAltFiltersMinus1     = params.alfApsArray[i].m_alfChromaNumAltFiltersMinus1;
            }
        }

#define DO_FIELDS()                                                                                          \
    DO_FIELD(DW1, SpsSubpicInfoPresentFlag, params.spsSubpicInfoPresentFlag);                                \
    DO_FIELD(DW1, SpsIndependentSubpicsFlag, params.spsIndependentSubpicsFlag);\
    DO_FIELD(DW1, SpsSubpicSameSizeFlag, params.spsSubpicSameSizeFlag);\
    DO_FIELD(DW1, SpsEntropyCodingSyncEnabledFlag, params.spsEntropyCodingSyncEnabledFlag);\
    DO_FIELD(DW1, SpsQtbttDualTreeIntraFlag, params.spsQtbttDualTreeIntraFlag);\
    DO_FIELD(DW1, SpsMaxLumaTransformSize64Flag, params.spsMaxLumaTransformSize64Flag);\
    DO_FIELD(DW1, SpsTransformSkipEnabledFlag, params.spsTransformSkipEnabledFlag);\
    DO_FIELD(DW1, SpsBdpcmEnabledFlag, params.spsBdpcmEnabledFlag);\
    DO_FIELD(DW1, SpsMtsEnabledFlag, params.spsMtsEnabledFlag);\
    DO_FIELD(DW1, SpsExplicitMtsIntraEnabledFlag, params.spsExplicitMtsIntraEnabledFlag);\
    DO_FIELD(DW1, SpsExplicitMtsInterEnabledFlag, params.spsExplicitMtsInterEnabledFlag);\
    DO_FIELD(DW1, SpsLfnstEnabledFlag, params.spsLfnstEnabledFlag);\
    DO_FIELD(DW1, SpsJointCbcrEnabledFlag, params.spsJointCbcrEnabledFlag);\
    DO_FIELD(DW1, SpsSameQpTableForChromaFlag, params.spsSameQpTableForChromaFlag);\
    DO_FIELD(DW1, DLmcsDisabledFlag, params.dLmcsDisabledFlag);\
    DO_FIELD(DW1, DDblkDisabledFlag, params.dDblkDisabledFlag);\
    DO_FIELD(DW1, DSaoLumaDisabledFlag, params.dSaoLumaDisabledFlag);\
    DO_FIELD(DW1, DSaoChromaDisabledFlag, params.dSaoChromaDisabledFlag);\
    DO_FIELD(DW1, DAlfDisabledFlag, params.dAlfDisabledFlag);\
    DO_FIELD(DW1, DAlfCbDisabledFlag, params.dAlfCbDisabledFlag);\
    DO_FIELD(DW1, DAlfCrDisabledFlag, params.dAlfCrDisabledFlag);\
    DO_FIELD(DW1, DAlfCcCbDisabledFlag, params.dAlfCcCbDisabledFlag);\
    DO_FIELD(DW1, DAlfCcCrDisabledFlag, params.dAlfCcCrDisabledFlag);\
    DO_FIELD(DW1, DSingleSliceFrameFlag, params.dSingleSliceFrameFlag);\
    DO_FIELD(DW2, SpsSbtmvpEnabledFlag, params.spsSbtmvpEnabledFlag);\
    DO_FIELD(DW2, SpsAmvrEnabledFlag, params.spsAmvrEnabledFlag);\
    DO_FIELD(DW2, SpsSmvdEnabledFlag, params.spsSmvdEnabledFlag);\
    DO_FIELD(DW2, SpsMmvdEnabledFlag, params.spsMmvdEnabledFlag);\
    DO_FIELD(DW2, SpsSbtEnabledFlag, params.spsSbtEnabledFlag);\
    DO_FIELD(DW2, SpsAffineEnabledFlag, params.spsAffineEnabledFlag);\
    DO_FIELD(DW2, Sps6ParamAffineEnabledFlag, params.sps6ParamAffineEnabledFlag);\
    DO_FIELD(DW2, SpsAffineAmvrEnabledFlag, params.spsAffineAmvrEnabledFlag);\
    DO_FIELD(DW2, SpsBcwEnabledFlag, params.spsBcwEnabledFlag);\
    DO_FIELD(DW2, SpsCiipEnabledFlag, params.spsCiipEnabledFlag);\
    DO_FIELD(DW2, SpsGpmEnabledFlag, params.spsGpmEnabledFlag);\
    DO_FIELD(DW2, SpsIspEnabledFlag, params.spsIspEnabledFlag);\
    DO_FIELD(DW2, SpsMrlEnabledFlag, params.spsMrlEnabledFlag);\
    DO_FIELD(DW2, SpsMipEnabledFlag, params.spsMipEnabledFlag);\
    DO_FIELD(DW2, SpsCclmEnabledFlag, params.spsCclmEnabledFlag);\
    DO_FIELD(DW2, SpsChromaHorizontalCollocatedFlag, params.spsChromaHorizontalCollocatedFlag);\
    DO_FIELD(DW2, SpsChromaVerticalCollocatedFlag, params.spsChromaVerticalCollocatedFlag);\
    DO_FIELD(DW2, SpsTemporalMvpEnabledFlag, params.spsTemporalMvpEnabledFlag);\
    DO_FIELD(DW2, SpsPaletteEnabledFlag, params.spsPaletteEnabledFlag);\
    DO_FIELD(DW2, SpsActEnabledFlag, params.spsActEnabledFlag);\
    DO_FIELD(DW2, SpsIbcEnabledFlag, params.spsIbcEnabledFlag);\
    DO_FIELD(DW2, SpsLadfEnabledFlag, params.spsLadfEnabledFlag);\
    DO_FIELD(DW2, SpsScalingMatrixForLfnstDisabledFlag, params.spsScalingMatrixForLfnstDisabledFlag);\
    DO_FIELD(DW2, SpsScalingMatrixForAlternativeColorSpaceDisabledFlag, params.spsScalingMatrixForAlternativeColorSpaceDisabledFlag);\
    DO_FIELD(DW2, SpsScalingMatrixDesignatedColorSpaceFlag, params.spsScalingMatrixDesignatedColorSpaceFlag);\
    DO_FIELD(DW3, PpsLoopFilterAcrossTilesEnabledFlag, params.ppsLoopFilterAcrossTilesEnabledFlag);\
    DO_FIELD(DW3, PpsRectSliceFlag, params.ppsRectSliceFlag);\
    DO_FIELD(DW3, PpsSingleSlicePerSubpicFlag, params.ppsSingleSlicePerSubpicFlag);\
    DO_FIELD(DW3, PpsLoopFilterAcrossSlicesEnabledFlag, params.ppsLoopFilterAcrossSlicesEnabledFlag);\
    DO_FIELD(DW3, PpsWeightedPredFlag, params.ppsWeightedPredFlag);\
    DO_FIELD(DW3, PpsWeightedBipredFlag, params.ppsWeightedBipredFlag);\
    DO_FIELD(DW3, PpsRefWraparoundEnabledFlag, params.ppsRefWraparoundEnabledFlag);\
    DO_FIELD(DW3, PpsCuQpDeltaEnabledFlag, params.ppsCuQpDeltaEnabledFlag);\
    DO_FIELD(DW3, Virtualboundariespresentflag, params.virtualboundariespresentflag);\
    DO_FIELD(DW3, PhNonRefPicFlag, params.phNonRefPicFlag);\
    DO_FIELD(DW3, PhChromaResidualScaleFlag, params.phChromaResidualScaleFlag);\
    DO_FIELD(DW3, PhTemporalMvpEnabledFlag, params.phTemporalMvpEnabledFlag);\
    DO_FIELD(DW3, PhMmvdFullpelOnlyFlag, params.phMmvdFullpelOnlyFlag);\
    DO_FIELD(DW3, PhMvdL1ZeroFlag, params.phMvdL1ZeroFlag);\
    DO_FIELD(DW3, PhBdofDisabledFlag, params.phBdofDisabledFlag);\
    DO_FIELD(DW3, PhDmvrDisabledFlag, params.phDmvrDisabledFlag);\
    DO_FIELD(DW3, PhProfDisabledFlag, params.phProfDisabledFlag);\
    DO_FIELD(DW3, PhJointCbcrSignFlag, params.phJointCbcrSignFlag);\
    DO_FIELD(DW4, SpsChromaFormatIdc, params.spsChromaFormatIdc);\
    DO_FIELD(DW4, SpsLog2CtuSizeMinus5, params.spsLog2CtuSizeMinus5);\
    DO_FIELD(DW4, SpsBitdepthMinus8, params.spsBitdepthMinus8);\
    DO_FIELD(DW4, SpsLog2MinLumaCodingBlockSizeMinus2, params.spsLog2MinLumaCodingBlockSizeMinus2);\
    DO_FIELD(DW4, SpsNumSubpicsMinus1, params.spsNumSubpicsMinus1);\
    DO_FIELD(DW5, SpsLog2TransformSkipMaxSizeMinus2, params.spsLog2TransformSkipMaxSizeMinus2);\
    DO_FIELD(DW5, SpsSixMinusMaxNumMergeCand, params.spsSixMinusMaxNumMergeCand);\
    DO_FIELD(DW5, SpsFiveMinusMaxNumSubblockMergeCand, params.spsFiveMinusMaxNumSubblockMergeCand);\
    DO_FIELD(DW5, DMaxNumGpmMergeCand, params.dMaxNumGpmMergeCand);\
    DO_FIELD(DW5, SpsLog2ParallelMergeLevelMinus2, params.spsLog2ParallelMergeLevelMinus2);\
    DO_FIELD(DW5, SpsMinQpPrimeTs, params.spsMinQpPrimeTs);\
    DO_FIELD(DW5, SpsSixMinusMaxNumIbcMergeCand, params.spsSixMinusMaxNumIbcMergeCand);\
    DO_FIELD(DW6, SpsLadfQpOffset0, params.spsLadfQpOffset0);\
    DO_FIELD(DW6, SpsLadfQpOffset1, params.spsLadfQpOffset1);\
    DO_FIELD(DW6, SpsLadfQpOffset2, params.spsLadfQpOffset2);\
    DO_FIELD(DW6, SpsLadfQpOffset3, params.spsLadfQpOffset3);\
    DO_FIELD(DW7, SpsLadfDeltaThresholdMinus10, params.spsLadfDeltaThresholdMinus10);\
    DO_FIELD(DW7, SpsLadfDeltaThresholdMinus11, params.spsLadfDeltaThresholdMinus11);\
    DO_FIELD(DW7, SpsLadfLowestIntervalQpOffset, params.spsLadfLowestIntervalQpOffset);\
    DO_FIELD(DW8, SpsLadfDeltaThresholdMinus12, params.spsLadfDeltaThresholdMinus12);\
    DO_FIELD(DW8, SpsLadfDeltaThresholdMinus13, params.spsLadfDeltaThresholdMinus13);\
    DO_FIELD(DW8, SpsNumLadfIntervalsMinus2, params.spsNumLadfIntervalsMinus2);\
    DO_FIELD(DW9, PpsPicWidthInLumaSamples, params.ppsPicWidthInLumaSamples);\
    DO_FIELD(DW9, PpsPicHeightInLumaSamples, params.ppsPicHeightInLumaSamples);\
    DO_FIELD(DW10, PpsScalingWinLeftOffset, params.ppsScalingWinLeftOffset);\
    DO_FIELD(DW11, PpsScalingWinRightOffset, params.ppsScalingWinRightOffset);\
    DO_FIELD(DW12, PpsScalingWinTopOffset, params.ppsScalingWinTopOffset);\
    DO_FIELD(DW13, PpsScalingWinBottomOffset, params.ppsScalingWinBottomOffset);\
    DO_FIELD(DW14, DNumtilerowsminus1, params.dNumtilerowsminus1);\
    DO_FIELD(DW14, DNumtilecolumnsminus1, params.dNumtilecolumnsminus1);\
    DO_FIELD(DW15, PpsCbQpOffset, params.ppsCbQpOffset);\
    DO_FIELD(DW15, PpsCrQpOffset, params.ppsCrQpOffset);\
    DO_FIELD(DW15, PpsJointCbcrQpOffsetValue, params.ppsJointCbcrQpOffsetValue);\
    DO_FIELD(DW15, PpsChromaQpOffsetListLenMinus1, params.ppsChromaQpOffsetListLenMinus1);\
    DO_FIELD(DW16, PpsCbQpOffsetList0, params.ppsCbQpOffsetList0);\
    DO_FIELD(DW16, PpsCbQpOffsetList1, params.ppsCbQpOffsetList1);\
    DO_FIELD(DW16, PpsCbQpOffsetList2, params.ppsCbQpOffsetList2);\
    DO_FIELD(DW16, PpsCbQpOffsetList3, params.ppsCbQpOffsetList3);\
    DO_FIELD(DW17, PpsCbQpOffsetList4, params.ppsCbQpOffsetList4);\
    DO_FIELD(DW17, PpsCbQpOffsetList5, params.ppsCbQpOffsetList5);\
    DO_FIELD(DW17, PpsPicWidthMinusWraparoundOffset, params.ppsPicWidthMinusWraparoundOffset);\
    DO_FIELD(DW18, PpsCrQpOffsetList0, params.ppsCrQpOffsetList0);\
    DO_FIELD(DW18, PpsCrQpOffsetList1, params.ppsCrQpOffsetList1);\
    DO_FIELD(DW18, PpsCrQpOffsetList2, params.ppsCrQpOffsetList2);\
    DO_FIELD(DW18, PpsCrQpOffsetList3, params.ppsCrQpOffsetList3);\
    DO_FIELD(DW19, PpsCrQpOffsetList4, params.ppsCrQpOffsetList4);\
    DO_FIELD(DW19, PpsCrQpOffsetList5, params.ppsCrQpOffsetList5);\
    DO_FIELD(DW20, PpsJointCbcrQpOffsetList0, params.ppsJointCbcrQpOffsetList0);\
    DO_FIELD(DW20, PpsJointCbcrQpOffsetList1, params.ppsJointCbcrQpOffsetList1);\
    DO_FIELD(DW20, PpsJointCbcrQpOffsetList2, params.ppsJointCbcrQpOffsetList2);\
    DO_FIELD(DW20, PpsJointCbcrQpOffsetList3, params.ppsJointCbcrQpOffsetList3);\
    DO_FIELD(DW21, PpsJointCbcrQpOffsetList4, params.ppsJointCbcrQpOffsetList4);\
    DO_FIELD(DW21, PpsJointCbcrQpOffsetList5, params.ppsJointCbcrQpOffsetList5);\
    DO_FIELD(DW22, Numvervirtualboundaries, params.numvervirtualboundaries);\
    DO_FIELD(DW22, Numhorvirtualboundaries, params.numhorvirtualboundaries);\
    DO_FIELD(DW22, PhLog2DiffMinQtMinCbIntraSliceLuma, params.phLog2DiffMinQtMinCbIntraSliceLuma);\
    DO_FIELD(DW22, PhMaxMttHierarchyDepthIntraSliceLuma, params.phMaxMttHierarchyDepthIntraSliceLuma);\
    DO_FIELD(DW22, PhLog2DiffMaxBtMinQtIntraSliceLuma, params.phLog2DiffMaxBtMinQtIntraSliceLuma);\
    DO_FIELD(DW22, PhLog2DiffMaxTtMinQtIntraSliceLuma, params.phLog2DiffMaxTtMinQtIntraSliceLuma);\
    DO_FIELD(DW22, PhLog2DiffMinQtMinCbIntraSliceChroma, params.phLog2DiffMinQtMinCbIntraSliceChroma);\
    DO_FIELD(DW22, PhMaxMttHierarchyDepthIntraSliceChroma, params.phMaxMttHierarchyDepthIntraSliceChroma);\
    DO_FIELD(DW23, DVirtualboundaryposxminus10, params.dVirtualboundaryposxminus10);\
    DO_FIELD(DW23, DVirtualboundaryposyminus10, params.dVirtualboundaryposyminus10);\
    DO_FIELD(DW24, DVirtualboundaryposxminus11, params.dVirtualboundaryposxminus11);\
    DO_FIELD(DW24, DVirtualboundaryposyminus11, params.dVirtualboundaryposyminus11);\
    DO_FIELD(DW25, DVirtualboundaryposxminus12, params.dVirtualboundaryposxminus12);\
    DO_FIELD(DW25, DVirtualboundaryposyminus12, params.dVirtualboundaryposyminus12);\
    DO_FIELD(DW26, PhLog2DiffMaxBtMinQtIntraSliceChroma, params.phLog2DiffMaxBtMinQtIntraSliceChroma);\
    DO_FIELD(DW26, PhLog2DiffMaxTtMinQtIntraSliceChroma, params.phLog2DiffMaxTtMinQtIntraSliceChroma);\
    DO_FIELD(DW26, PhCuQpDeltaSubdivIntraSlice, params.phCuQpDeltaSubdivIntraSlice);\
    DO_FIELD(DW26, PhCuChromaQpOffsetSubdivIntraSlice, params.phCuChromaQpOffsetSubdivIntraSlice);\
    DO_FIELD(DW26, PhLog2DiffMinQtMinCbInterSlice, params.phLog2DiffMinQtMinCbInterSlice);\
    DO_FIELD(DW26, PhMaxMttHierarchyDepthInterSlice, params.phMaxMttHierarchyDepthInterSlice);\
    DO_FIELD(DW27, PhLog2DiffMaxBtMinQtInterSlice, params.phLog2DiffMaxBtMinQtInterSlice);\
    DO_FIELD(DW27, PhLog2DiffMaxTtMinQtInterSlice, params.phLog2DiffMaxTtMinQtInterSlice);\
    DO_FIELD(DW27, PhCuQpDeltaSubdivInterSlice, params.phCuQpDeltaSubdivInterSlice);\
    DO_FIELD(DW27, PhCuChromaQpOffsetSubdivInterSlice, params.phCuChromaQpOffsetSubdivInterSlice);\
    DO_FIELD(DW28, DActiveapsid, params.dActiveapsid);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_DPB_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_DPB_STATE);
    
        for (auto i = 0; i < vvcMaxNumRefFrame; i++)
        {
            cmd.Entries[i].DW0.DRefscalingwinleftoffsetI   = (uint32_t)params.refFrameAttr[i].m_refscalingwinleftoffset;
            cmd.Entries[i].DW1.DRefscalingwinrightoffsetI  = (uint32_t)params.refFrameAttr[i].m_refscalingwinrightoffset;
            cmd.Entries[i].DW2.DRefscalingwintopoffsetI    = (uint32_t)params.refFrameAttr[i].m_refscalingwintopoffset;
            cmd.Entries[i].DW3.DRefscalingwinbottomoffsetI = (uint32_t)params.refFrameAttr[i].m_refscalingwinbottomoffset;
            cmd.Entries[i].DW4.DRefpicscalex0J             = (uint32_t)params.refPicScaleWidth[i];
            cmd.Entries[i].DW4.DRefpicscalex1J             = (uint32_t)params.refPicScaleHeight[i];
            cmd.Entries[i].DW5.DRefpicwidthI               = (uint32_t)params.refFrameAttr[i].m_refpicwidth;
            cmd.Entries[i].DW5.DRefpicheightI              = (uint32_t)params.refFrameAttr[i].m_refpicheight;
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_SLICE_STATE);

        // LMCS
        if (params.spsLmcsEnabledFlag && params.shLmcsUsedFlag)
        {
            uint8_t             sliceLmcsApsId = params.phLmcsApsId;
            CodecVvcLmcsData   *lmcsData       = &params.vvcLmcsData[sliceLmcsApsId];
            ApsLmcsReshapeInfo *lmcsShapeInfo  = &params.vvcLmcsShapeInfo[sliceLmcsApsId];

            cmd.DW9.LmcsMinBinIdx      = lmcsData->m_lmcsMinBinIdx;
            cmd.DW9.LmcsDeltaMaxBinIdx = lmcsData->m_lmcsDeltaMaxBinIdx;

            for (int i = 0; i < 16; i++)
            {
                cmd.DW10_17.Scalecoeff[i]       = static_cast<uint16_t>(lmcsShapeInfo->m_scaleCoeff[i]);
                cmd.DW18_25.Invscalecoeff[i]    = static_cast<uint16_t>(lmcsShapeInfo->m_invScaleCoeff[i]);
                cmd.DW26_33.Chromascalecoeff[i] = static_cast<uint16_t>(lmcsShapeInfo->m_chromaScaleCoeff[i]);
            }
            MOS_SecureMemcpy(cmd.Lmcspivot161, sizeof(cmd.Lmcspivot161), &lmcsShapeInfo->m_lmcsPivot[1], sizeof(cmd.Lmcspivot161));
        }

#define DO_FIELDS()                                                                                \
    DO_FIELD(DW1, ShAlfEnabledFlag, params.shAlfEnabledFlag);                                      \
    DO_FIELD(DW1, ShAlfCbEnabledFlag, params.shAlfCbEnabledFlag);                                  \
    DO_FIELD(DW1, ShAlfCrEnabledFlag, params.shAlfCrEnabledFlag);                                  \
    DO_FIELD(DW1, ShAlfCcCbEnabledFlag, params.shAlfCcCbEnabledFlag);                              \
    DO_FIELD(DW1, ShAlfCcCrEnabledFlag, params.shAlfCcCrEnabledFlag);                              \
    DO_FIELD(DW1, ShLmcsUsedFlag, params.shLmcsUsedFlag);                                          \
    DO_FIELD(DW1, ShExplicitScalingListUsedFlag, params.shExplicitScalingListUsedFlag);            \
    DO_FIELD(DW1, ShCabacInitFlag, params.shCabacInitFlag);                                        \
    DO_FIELD(DW1, ShCollocatedFromL0Flag, params.shCollocatedFromL0Flag);                          \
    DO_FIELD(DW1, ShCuChromaQpOffsetEnabledFlag, params.shCuChromaQpOffsetEnabledFlag);            \
    DO_FIELD(DW1, ShSaoLumaUsedFlag, params.shSaoLumaUsedFlag);                                    \
    DO_FIELD(DW1, ShSaoChromaUsedFlag, params.shSaoChromaUsedFlag);                                \
    DO_FIELD(DW1, ShDeblockingFilterDisabledFlag, params.shDeblockingFilterDisabledFlag);          \
    DO_FIELD(DW1, ShDepQuantUsedFlag, params.shDepQuantUsedFlag);                                  \
    DO_FIELD(DW1, ShSignDataHidingUsedFlag, params.shSignDataHidingUsedFlag);                      \
    DO_FIELD(DW1, ShTsResidualCodingDisabledFlag, params.shTsResidualCodingDisabledFlag);          \
    DO_FIELD(DW1, Nobackwardpredflag, params.nobackwardpredflag);                                  \
    DO_FIELD(DW1, PVvcpDebugEnable, params.pVvcpDebugEnable);                                      \
    DO_FIELD(DW1, DMultipleSlicesInTileFlag, params.dMultipleSlicesInTileFlag);                    \
    DO_FIELD(DW1, DIsbottommostsliceoftileFlag, params.dIsbottommostsliceoftileFlag);              \
    DO_FIELD(DW1, DIstopmostsliceoftileFlag, params.dIstopmostsliceoftileFlag);                    \
    DO_FIELD(DW1, DSubpicTreatedAsPicFlag, params.dSubpicTreatedAsPicFlag);                        \
    DO_FIELD(DW1, DLoopFilterAcrossSubpicEnabledFlag, params.dLoopFilterAcrossSubpicEnabledFlag);  \
    DO_FIELD(DW1, DIsRightMostSliceOfSubpicFlag, params.dIsRightMostSliceOfSubpicFlag);            \
    DO_FIELD(DW1, DIsLeftMostSliceOfSubpicFlag, params.dIsLeftMostSliceOfSubpicFlag);              \
    DO_FIELD(DW1, DIsBottomMostSliceOfSubpicFlag, params.dIsBottomMostSliceOfSubpicFlag);          \
    DO_FIELD(DW1, DIsTopMostSliceOfSubpicFlag, params.dIsTopMostSliceOfSubpicFlag);                \
    DO_FIELD(DW1, DLastsliceofpicFlag, params.dLastsliceofpicFlag);                                \
    DO_FIELD(DW2, Numctusincurrslice, params.numctusincurrslice);                                  \
    DO_FIELD(DW3, ShNumTilesInSliceMinus1, params.shNumTilesInSliceMinus1);                        \
    DO_FIELD(DW3, ShSliceType, params.shSliceType);                                                \
    DO_FIELD(DW3, ShNumAlfApsIdsLuma, params.shNumAlfApsIdsLuma);                                  \
    DO_FIELD(DW3, AlfChromaNumAltFiltersMinus1, params.alfChromaNumAltFiltersMinus1);              \
    DO_FIELD(DW3, AlfCcCbFiltersSignalledMinus1, params.alfCcCbFiltersSignalledMinus1);            \
    DO_FIELD(DW3, AlfCcCrFiltersSignalledMinus1, params.alfCcCrFiltersSignalledMinus1);            \
    DO_FIELD(DW4, ShAlfApsIdLuma0, params.shAlfApsIdLuma0);                                        \
    DO_FIELD(DW4, ShAlfApsIdLuma1, params.shAlfApsIdLuma1);                                        \
    DO_FIELD(DW4, ShAlfApsIdLuma2, params.shAlfApsIdLuma2);                                        \
    DO_FIELD(DW4, ShAlfApsIdLuma3, params.shAlfApsIdLuma3);                                        \
    DO_FIELD(DW4, ShAlfApsIdLuma4, params.shAlfApsIdLuma4);                                        \
    DO_FIELD(DW4, ShAlfApsIdLuma5, params.shAlfApsIdLuma5);                                        \
    DO_FIELD(DW4, ShAlfApsIdLuma6, params.shAlfApsIdLuma6);                                        \
    DO_FIELD(DW5, ShAlfApsIdChroma, params.shAlfApsIdChroma);                                      \
    DO_FIELD(DW5, ShAlfCcCbApsId, params.shAlfCcCbApsId);                                          \
    DO_FIELD(DW5, ShAlfCcCrApsId, params.shAlfCcCrApsId);                                          \
    DO_FIELD(DW5, Numrefidxactive0, params.numrefidxactive0);                                      \
    DO_FIELD(DW5, Numrefidxactive1, params.numrefidxactive1);                                      \
    DO_FIELD(DW5, ShCollocatedRefIdx, params.shCollocatedRefIdx);                                  \
    DO_FIELD(DW6, Sliceqpy, params.sliceqpy);                                                      \
    DO_FIELD(DW6, ShCbQpOffset, params.shCbQpOffset);                                              \
    DO_FIELD(DW6, ShCrQpOffset, params.shCrQpOffset);                                              \
    DO_FIELD(DW6, ShJointCbcrQpOffset, params.shJointCbcrQpOffset);                                \
    DO_FIELD(DW7, ShLumaBetaOffsetDiv2, params.shLumaBetaOffsetDiv2);                              \
    DO_FIELD(DW7, ShLumaTcOffsetDiv2, params.shLumaTcOffsetDiv2);                                  \
    DO_FIELD(DW7, ShCbBetaOffsetDiv2, params.shCbBetaOffsetDiv2);                                  \
    DO_FIELD(DW7, ShCbTcOffsetDiv2, params.shCbTcOffsetDiv2);                                      \
    DO_FIELD(DW8, ShCrBetaOffsetDiv2, params.shCrBetaOffsetDiv2);                                  \
    DO_FIELD(DW8, ShCrTcOffsetDiv2, params.shCrTcOffsetDiv2);                                      \
    DO_FIELD(DW42, DSubpicCtuTopLeftX, params.dSubpicCtuTopLeftX);                                 \
    DO_FIELD(DW42, DSubpicCtuTopLeftY, params.dSubpicCtuTopLeftY);                                 \
    DO_FIELD(DW43, DSubpicWidthMinus1, params.dSubpicWidthMinus1);                                 \
    DO_FIELD(DW43, DSubpicHeightMinus1, params.dSubpicHeightMinus1);                               \
    DO_FIELD(DW45, DToplefttilex, params.dToplefttilex);                                           \
    DO_FIELD(DW45, DToplefttiley, params.dToplefttiley);                                           \
    DO_FIELD(DW46, DSliceheightinctus, params.dSliceheightinctus);                                 \
    DO_FIELD(DW47, DSlicestartctbx, params.dSlicestartctbx);                                       \
    DO_FIELD(DW47, DSlicestartctby, params.dSlicestartctby);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_BSD_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(VVCP_BSD_OBJECT);
    
#define DO_FIELDS()                                                     \
    DO_FIELD(DW1, IndirectBsdDataLength, params.bsdDataLength);         \
    DO_FIELD(DW2, IndirectDataStartAddress, params.bsdDataStartOffset);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_REF_IDX_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_REF_IDX_STATE);

        for (uint8_t i = 0; i < static_cast<uint8_t>(params.numRefForList); i++)
        {
            uint8_t refFrameIDx = params.refPicList[params.listIdx][i].FrameIdx;

            if (refFrameIDx < vvcMaxNumRefFrame)
            {
                cmd.Entries[i].DW0.RefpiclistListidxI               = params.refPicList[params.listIdx][i].FrameIdx;
                cmd.Entries[i].DW0.StRefPicFlagListidxRplsidxI      = params.stRefPicFlag[params.listIdx][i];
                cmd.Entries[i].DW0.RprconstraintsactiveflagListidxI = params.rprConstraintsActiveFlag[params.listIdx][i];
                cmd.Entries[i].DW0.DUnavailablerefpicListidxI       = params.unavailableRefPic[params.listIdx][i];
                cmd.Entries[i].DW0.DDiffpicordercntListidxI         = params.diffPicOrderCnt[params.listIdx][i];
            }
            else
            {
                cmd.Entries[i].DW0.Value = 0x00;
            }
        }

        for (uint8_t i = (uint8_t)params.numRefForList; i < vvcMaxNumRefFrame; i++)
        {
            cmd.Entries[i].DW0.Value = 0x00;
        }

#define DO_FIELDS()                                                   \
    DO_FIELD(DW1, Listidx, params.listIdx);                           \
    DO_FIELD(DW1, Refidxsymlx, params.refIdxSymLx[params.listIdx]);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_WEIGHTOFFSET_STATE)
    {
        _MHW_SETCMD_CALLBASE(VVCP_WEIGHTOFFSET_STATE);

        auto wpInfo = params.wpInfo;
        MHW_ASSERT(wpInfo->m_lumaLog2WeightDenom + wpInfo->m_deltaChromaLog2WeightDenom >= 0 &&
                   wpInfo->m_lumaLog2WeightDenom + wpInfo->m_deltaChromaLog2WeightDenom < 8);
        int8_t chromalog2Weightdenom = wpInfo->m_lumaLog2WeightDenom + wpInfo->m_deltaChromaLog2WeightDenom;

        uint16_t lumaFlag = 0, chromaFlag = 0;
        if (params.listIdx == 0)
        {
            for (auto i = 0; i < 15; i++)
            {
                lumaFlag   |= wpInfo->m_lumaWeightL0Flag[i] << i;
                chromaFlag |= wpInfo->m_chromaWeightL0Flag[i] << i;
            }
            cmd.DW2.LumaWeightLxFlagI   = lumaFlag;
            cmd.DW2.ChromaWeightLxFlagI = chromaFlag;

            for (auto i = 0; i < 15; i++)
            {
                cmd.DLumaweightsoffsets[i].DW0.DeltaLumaWeightLxI        = wpInfo->m_deltaLumaWeightL0[i];
                cmd.DLumaweightsoffsets[i].DW0.LumaOffsetLxI             = wpInfo->m_lumaOffsetL0[i];
                cmd.DChromacbweightsoffsets[i].DW0.DeltaChromaWeightLxIJ = wpInfo->m_deltaChromaWeightL0[i][0];
                cmd.DChromacbweightsoffsets[i].DW0.DeltaChromaOffsetLxIJ = wpInfo->m_deltaChromaOffsetL0[i][0];
                cmd.DChromacrweightsoffsets[i].DW0.DeltaChromaWeightLxIJ = wpInfo->m_deltaChromaWeightL0[i][1];
                cmd.DChromacrweightsoffsets[i].DW0.DeltaChromaOffsetLxIJ = wpInfo->m_deltaChromaOffsetL0[i][1];
            }
        }
        else if (params.listIdx == 1)
        {
            for (auto i = 0; i < 15; i++)
            {
                lumaFlag   |= wpInfo->m_lumaWeightL1Flag[i] << i;
                chromaFlag |= wpInfo->m_chromaWeightL1Flag[i] << i;
            }
            cmd.DW2.LumaWeightLxFlagI   = lumaFlag;
            cmd.DW2.ChromaWeightLxFlagI = chromaFlag;

            for (auto i = 0; i < 15; i++)
            {
                cmd.DLumaweightsoffsets[i].DW0.DeltaLumaWeightLxI        = wpInfo->m_deltaLumaWeightL1[i];
                cmd.DLumaweightsoffsets[i].DW0.LumaOffsetLxI             = wpInfo->m_lumaOffsetL1[i];
                cmd.DChromacbweightsoffsets[i].DW0.DeltaChromaWeightLxIJ = wpInfo->m_deltaChromaWeightL1[i][0];
                cmd.DChromacbweightsoffsets[i].DW0.DeltaChromaOffsetLxIJ = wpInfo->m_deltaChromaOffsetL1[i][0];
                cmd.DChromacrweightsoffsets[i].DW0.DeltaChromaWeightLxIJ = wpInfo->m_deltaChromaWeightL1[i][1];
                cmd.DChromacrweightsoffsets[i].DW0.DeltaChromaOffsetLxIJ = wpInfo->m_deltaChromaOffsetL1[i][1];
            }
        }

#define DO_FIELDS()                                                    \
    DO_FIELD(DW1, Listidx, params.listIdx);                            \
    DO_FIELD(DW1, LumaLog2WeightDenom, wpInfo->m_lumaLog2WeightDenom); \
    DO_FIELD(DW1, Chromalog2Weightdenom, chromalog2Weightdenom);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VVCP_TILE_CODING)
    {
        _MHW_SETCMD_CALLBASE(VVCP_TILE_CODING);

#define DO_FIELDS()                                                                          \
    DO_FIELD(DW1, Tilecolbdval, params.tilecolbdval);                                        \
    DO_FIELD(DW1, Tilerowbdval, params.tilerowbdval);                                        \
    DO_FIELD(DW2, Colwidthval, params.colwidthval);                                          \
    DO_FIELD(DW2, Rowheightval, params.rowheightval);                                        \
    DO_FIELD(DW3, DCurrenttilecolumnposition, params.currenttilecolumnposition);             \
    DO_FIELD(DW3, DCurrenttilerowposition, params.currenttilerowposition);                   \
    DO_FIELD(DW4, DIsrightmosttileofsliceFlag, params.flags.m_isrightmosttileofsliceFlag);   \
    DO_FIELD(DW4, DIsleftmosttileofsliceFlag, params.flags.m_isleftmosttileofsliceFlag);     \
    DO_FIELD(DW4, DIsbottommosttileofsliceFlag, params.flags.m_isbottommosttileofsliceFlag); \
    DO_FIELD(DW4, DIstopmosttileofsliceFlag, params.flags.m_istopmosttileofsliceFlag);       \
    DO_FIELD(DW4, DIsrightmosttileofframeFlag, params.flags.m_isrightmosttileofframeFlag);   \
    DO_FIELD(DW4, DIsleftmosttileofframeFlag, params.flags.m_isleftmosttileofframeFlag);     \
    DO_FIELD(DW4, DIsbottommosttileofframeFlag, params.flags.m_isbottommosttileofframeFlag); \
    DO_FIELD(DW4, DIstopmosttileofframeFlag, params.flags.m_istopmosttileofframeFlag);

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vvcp__Impl)
};


}  // namespace vvcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VVCP_IMPL_H__
