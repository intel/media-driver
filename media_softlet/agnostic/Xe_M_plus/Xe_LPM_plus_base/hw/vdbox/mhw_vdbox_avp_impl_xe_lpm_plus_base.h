/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

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
//! \file     mhw_vdbox_avp_impl_xe_lpm_plus_base.h
//! \brief    MHW VDBOX AVP interface common base for Xe_LPM_plus+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_AVP_IMPL_XE_LPM_PLUS_BASE_H__
#define __MHW_VDBOX_AVP_IMPL_XE_LPM_PLUS_BASE_H__

#include "mhw_vdbox_avp_impl.h"
#include "mhw_vdbox_vdenc_impl_xe_lpm_plus.h"
#include "mhw_mi_hwcmd_xe_lpm_plus_base_next.h"
#include "mhw_sfc_hwcmd_xe_lpm_plus_next.h"
#include "mhw_mi_itf.h"
#include "codec_def_decode_av1.h"

namespace mhw
{
namespace vdbox
{
namespace avp
{
namespace xe_lpm_plus_base
{
class MHW_AVP_FILM_GRAIN_PARAMS_XE_LPM_PLUS_BASE
{
public:
    MHW_AVP_FILM_GRAIN_PARAMS_XE_LPM_PLUS_BASE() {}
    ~MHW_AVP_FILM_GRAIN_PARAMS_XE_LPM_PLUS_BASE() {}

    CodecAv1PicParams* picParams = nullptr;
MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__xe_lpm_plus_base__MHW_AVP_FILM_GRAIN_PARAMS_XE_LPM_PLUS_BASE)
};

template <typename cmd_t>
class BaseImpl : public avp::Impl<cmd_t>
{
public:
    MOS_STATUS GetAvpStateCmdSize(uint32_t* commandsSize, uint32_t* patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t    maxSize = 0;
        uint32_t    patchListMaxSize = 0;

        maxSize =
            mhw::vdbox::vdenc::xe_lpm_plus_base::v0::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize   +
            cmd_t::AVP_PIPE_MODE_SELECT_CMD::byteSize         +
            cmd_t::AVP_SURFACE_STATE_CMD::byteSize * 11       +
            cmd_t::AVP_PIPE_BUF_ADDR_STATE_CMD::byteSize      +
            cmd_t::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize  +
            cmd_t::AVP_SEGMENT_STATE_CMD::byteSize * 8        +
            cmd_t::AVP_INLOOP_FILTER_STATE_CMD::byteSize      +
            cmd_t::AVP_INTER_PRED_STATE_CMD::byteSize         +
            cmd_t::AVP_FILM_GRAIN_STATE_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(avp::Itf::VD_PIPELINE_FLUSH_CMD)           +
            PATCH_LIST_COMMAND(mi::Itf::MI_FLUSH_DW_CMD)                  +
            PATCH_LIST_COMMAND(avp::Itf::AVP_PIPE_MODE_SELECT_CMD)        +
            PATCH_LIST_COMMAND(avp::Itf::AVP_SURFACE_STATE_CMD) * 11      +
            PATCH_LIST_COMMAND(avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD)     +
            PATCH_LIST_COMMAND(avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(avp::Itf::AVP_SEGMENT_STATE_CMD) * 8       +
            PATCH_LIST_COMMAND(avp::Itf::AVP_INTER_PRED_STATE_CMD)        +
            PATCH_LIST_COMMAND(avp::Itf::AVP_INLOOP_FILTER_STATE_CMD)     +
            PATCH_LIST_COMMAND(avp::Itf::AVP_FILM_GRAIN_STATE_CMD);

        if (params->bDecodeInUse)
        {
            maxSize +=
                cmd_t::AVP_PIC_STATE_CMD::byteSize +
                mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize * 2;

            patchListMaxSize += PATCH_LIST_COMMAND(avp::Itf::AVP_PIC_STATE_CMD);
        }
        if (params->bSfcInUse)
        {
            maxSize +=
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_LOCK_CMD::byteSize +
                2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize +
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_STATE_CMD::byteSize +
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_AVS_STATE_CMD::byteSize +
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_AVS_LUMA_Coeff_Table_CMD::byteSize +
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_AVS_CHROMA_Coeff_Table_CMD::byteSize +
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_IEF_STATE_CMD::byteSize +
                mhw::sfc::xe_lpm_plus_next::Cmd::SFC_FRAME_START_CMD::byteSize;
        }
        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetAvpPrimitiveCmdSize(uint32_t* commandsSize, uint32_t* patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t    maxSize = 0;
        uint32_t    patchListMaxSize = 0;

        if(params->bDecodeInUse)
        {
            maxSize =
                cmd_t::AVP_TILE_CODING_CMD::byteSize     +
                cmd_t::AVP_BSD_OBJECT_CMD::byteSize      +
                mhw::mi::xe_lpm_plus_base_next::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(avp::Itf::AVP_TILE_CODING_CMD) +
                PATCH_LIST_COMMAND(avp::Itf::AVP_BSD_OBJECT_CMD);
        }

        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }
protected:
    using base_t = avp::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};

    // Programming Note: CodecHAL layer must add MFX wait command
    //                   for both KIN and VRT before and after AVP_PIPE_MODE_SELECT

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_MODE_SELECT);

#define DO_FIELDS()                                                                                     \
    DO_FIELD(DW1, TileStatisticsStreamoutEnable, params.tileStatsStreamoutEnable ? 1 : 0);              \
                                                                                                        \
    DO_FIELD(DW6, DownscaledSourcePixelPrefetchLength, params.srcPixelPrefetchLen);                     \
    DO_FIELD(DW6, DownscaledSourcePixelPrefetchEnable, params.srcPixelPrefetchEnable ? 1 : 0);          \
    DO_FIELD(DW6, OriginalSourcePixelPrefetchLength, params.srcPixelPrefetchLen);                       \
    DO_FIELD(DW6, OriginalSourcePixelPrefetchEnable, params.srcPixelPrefetchEnable ? 1 : 0)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_FILM_GRAIN_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_FILM_GRAIN_STATE);

        MOS_SecureMemcpy(cmd.PointLumaValueI0To13,
            sizeof(params.pointYValue),
            params.pointYValue,
            sizeof(params.pointYValue)
        );

        MOS_SecureMemcpy(cmd.PointLumaScalingI0To13,
            sizeof(params.pointYScaling),
            params.pointYScaling,
            sizeof(params.pointYScaling)
        );

        MOS_SecureMemcpy(cmd.PointCbValueI0To9,
            sizeof(params.pointCbValue),
            params.pointCbValue,
            sizeof(params.pointCbValue)
        );

        MOS_SecureMemcpy(cmd.PointCbScalingI0To9,
            sizeof(params.pointCbScaling),
            params.pointCbScaling,
            sizeof(params.pointCbScaling)
        );

        MOS_SecureMemcpy(cmd.PointCrValueI0To9,
            sizeof(params.pointCrValue),
            params.pointCrValue,
            sizeof(params.pointCrValue)
        );

        MOS_SecureMemcpy(cmd.PointCrScalingI0To9,
            sizeof(params.pointCrScaling),
            params.pointCrScaling,
            sizeof(params.pointCrScaling)
        );

        MOS_SecureMemcpy(cmd.ArCoeffLumaPlus128I023,
            sizeof(params.arCoeffsY),
            params.arCoeffsY,
            sizeof(params.arCoeffsY)
        );

        MOS_SecureMemcpy(cmd.ArCoeffChromaCbPlus128I024,
            sizeof(params.arCoeffsCb),
            params.arCoeffsCb,
            sizeof(params.arCoeffsCb)
        );

        MOS_SecureMemcpy(cmd.ArCoeffChromaCrPlus128I024,
            sizeof(params.arCoeffsCr),
            params.arCoeffsCr,
            sizeof(params.arCoeffsCr)
        );

#define DO_FIELDS()                                                                                 \
        DO_FIELD(DW1, GrainRandomSeed, params.grainRandomSeed);                                     \
        DO_FIELD(DW1, ClipToRestrictedRangeFlag, params.clipToRestrictedRange);                     \
        DO_FIELD(DW1, NumberOfLumaPoints, params.numOfYPoints);                                     \
        DO_FIELD(DW1, NumberOfChromaCbPoints, params.numOfCbPoints);                                \
        DO_FIELD(DW1, NumberOfChromaCrPoints, params.numOfCrPoints);                                \
        DO_FIELD(DW1, McIdentityFlag, params.matrixCoefficients);                                   \
        DO_FIELD(DW2, GrainScalingMinus8, params.grainScalingMinus8);                               \
        DO_FIELD(DW2, ArCoeffLag, params.arCoeffLag);                                               \
        DO_FIELD(DW2, ArCoeffShiftMinus6, params.arCoeffShiftMinus6);                               \
        DO_FIELD(DW2, GrainScaleShift, params.grainScaleShift);                                     \
        DO_FIELD(DW2, ChromaScalingFromLumaFlag, params.chromaScalingFromLuma);                     \
        DO_FIELD(DW2, GrainNoiseOverlapFlag, params.grainNoiseOverlap);                             \
                                                                                                    \
        DO_FIELD(DW43, CbMult, params.cbMult);                                                      \
        DO_FIELD(DW43, CbLumaMult, params.cbLumaMult);                                              \
        DO_FIELD(DW43, CbOffset, params.cbOffset);                                                  \
        DO_FIELD(DW44, CrMult, params.crMult);                                                      \
        DO_FIELD(DW44, CrLumaMult, params.crLumaMult);                                              \
        DO_FIELD(DW44, CrOffset, params.crOffset)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_TILE_CODING)
    {
        _MHW_SETCMD_CALLBASE(AVP_TILE_CODING);

#if (_DEBUG || _RELEASE_INTERNAL)
#define DO_FIELDS()                                                                             \
    DO_FIELD(DW1, FrameTileId, params.tileId);                                                  \
    DO_FIELD(DW1, TgTileNum, params.tgTileNum);                                                   \
    DO_FIELD(DW1, TileGroupId, params.tileGroupId);                                             \
                                                                                                \
    DO_FIELD(DW2, TileColumnPositionInSbUnit, params.tileColPositionInSb);                      \
    DO_FIELD(DW2, TileRowPositionInSbUnit, params.tileRowPositionInSb);                         \
                                                                                                \
    DO_FIELD(DW3, TileWidthInSuperblockUnitMinus1, params.tileWidthInSbMinus1);                 \
    DO_FIELD(DW3, TileHeightInSuperblockUnitMinus1, params.tileHeightInSbMinus1);               \
    DO_FIELD(DW4, FirstTileInAFrame, params.firstTileInAFrame ? 1 : 0);                         \
    DO_FIELD(DW4, AvpCrcEnable, params.enableAvpDebugMode ? 1 : 0);                             \
    DO_FIELD(DW4, IslasttileofcolumnFlag, params.lastTileOfColumn ? 1 : 0);                     \
    DO_FIELD(DW4, IslasttileofrowFlag, params.lastTileOfRow ? 1 : 0);                           \
    DO_FIELD(DW4, IsstarttileoftilegroupFlag, params.firstTileOfTileGroup ? 1 : 0);             \
    DO_FIELD(DW4, IsendtileoftilegroupFlag, params.lastTileOfTileGroup ? 1 : 0);                \
    DO_FIELD(DW4, IslasttileofframeFlag, params.lastTileOfFrame ? 1 : 0);                       \
    DO_FIELD(DW4, DisableCdfUpdateFlag, params.disableCdfUpdateFlag ? 1 : 0);                   \
    DO_FIELD(DW4, DisableFrameContextUpdateFlag, params.disableFrameContextUpdateFlag ? 1 : 0); \
                                                                                                \
    DO_FIELD(DW5, NumberOfActiveBePipes, params.numOfActiveBePipes);                            \
    DO_FIELD(DW5, NumOfTileColumnsMinus1InAFrame, params.numOfTileColumnsInFrame - 1);          \
    DO_FIELD(DW5, NumOfTileRowsMinus1InAFrame, params.numOfTileRowsInFrame - 1);                \
                                                                                                \
    DO_FIELD(DW6, OutputDecodedTileColumnPositionInSbUnit, params.outputDecodedTileColPos);     \
    DO_FIELD(DW6, OutputDecodedTileRowPositionInSbUnit, params.outputDecodedTileRowPos)
#else
#define DO_FIELDS()                                                                             \
    DO_FIELD(DW1, FrameTileId, params.tileId);                                                  \
    DO_FIELD(DW1, TgTileNum, params.tgTileNum);                                                   \
    DO_FIELD(DW1, TileGroupId, params.tileGroupId);                                             \
                                                                                                \
    DO_FIELD(DW2, TileColumnPositionInSbUnit, params.tileColPositionInSb);                      \
    DO_FIELD(DW2, TileRowPositionInSbUnit, params.tileRowPositionInSb);                         \
                                                                                                \
    DO_FIELD(DW3, TileWidthInSuperblockUnitMinus1, params.tileWidthInSbMinus1);                 \
    DO_FIELD(DW3, TileHeightInSuperblockUnitMinus1, params.tileHeightInSbMinus1);               \
    DO_FIELD(DW4, FirstTileInAFrame, params.firstTileInAFrame ? 1 : 0);                         \
    DO_FIELD(DW4, IslasttileofcolumnFlag, params.lastTileOfColumn ? 1 : 0);                     \
    DO_FIELD(DW4, IslasttileofrowFlag, params.lastTileOfRow ? 1 : 0);                           \
    DO_FIELD(DW4, IsstarttileoftilegroupFlag, params.firstTileOfTileGroup ? 1 : 0);             \
    DO_FIELD(DW4, IsendtileoftilegroupFlag, params.lastTileOfTileGroup ? 1 : 0);                \
    DO_FIELD(DW4, IslasttileofframeFlag, params.lastTileOfFrame ? 1 : 0);                       \
    DO_FIELD(DW4, DisableCdfUpdateFlag, params.disableCdfUpdateFlag ? 1 : 0);                   \
    DO_FIELD(DW4, DisableFrameContextUpdateFlag, params.disableFrameContextUpdateFlag ? 1 : 0); \
                                                                                                \
    DO_FIELD(DW5, NumberOfActiveBePipes, params.numOfActiveBePipes);                            \
    DO_FIELD(DW5, NumOfTileColumnsMinus1InAFrame, params.numOfTileColumnsInFrame - 1);          \
    DO_FIELD(DW5, NumOfTileRowsMinus1InAFrame, params.numOfTileRowsInFrame - 1);                \
                                                                                                \
    DO_FIELD(DW6, OutputDecodedTileColumnPositionInSbUnit, params.outputDecodedTileColPos);     \
    DO_FIELD(DW6, OutputDecodedTileRowPositionInSbUnit, params.outputDecodedTileRowPos)
#endif

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        // Film grain related
        if (!Mos_ResourceIsNull(params.filmGrainOutputSurface))
        {
            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.filmGrainOutputSurface, &details));

            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;
            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.TileMode = this->GetHwTileType(params.filmGrainOutputSurface->TileType,
                                                                                    params.filmGrainOutputSurface->TileModeGMM,
                                                                                    params.filmGrainOutputSurface->bGMMTileEnabled);

            MOS_MEMCOMP_STATE mmcState = MOS_MEMCOMP_DISABLED;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetMemoryCompressionMode(this->m_osItf,
                params.filmGrainOutputSurface, &mmcState));
            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = this->MmcEnabled(mmcState);
            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.CompressionType = this->MmcRcEnabled(mmcState);

            resourceParams.presResource = params.filmGrainOutputSurface;
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.pdwCmd = (cmd.FilmGrainInjectedOutputFrameBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FilmGrainInjectedOutputFrameBufferAddress); // 21;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.filmGrainSampleTemplateBuffer))
        {
            cmd.FilmGrainSampleTemplateAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;

            resourceParams.presResource = params.filmGrainSampleTemplateBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.FilmGrainSampleTemplateAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FilmGrainSampleTemplateAddress); // 59;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.lrTileColumnAlignBuffer))
        {
            cmd.LoopRestorationFilterTileColumnAlignmentReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;

            resourceParams.presResource = params.lrTileColumnAlignBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnAlignmentReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileColumnAlignmentReadWriteBufferAddress); // 170;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.filmGrainTileColumnDataBuffer))
        {
            cmd.FilmGrainTileColumnDataReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;

            resourceParams.presResource = params.filmGrainTileColumnDataBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.FilmGrainTileColumnDataReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FilmGrainTileColumnDataReadWriteBufferAddress); // 173;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__xe_lpm_plus_base__BaseImpl)
};
}  // namespace xe_lpm_plus_base
}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_IMPL_XE_LPM_PLUS_BASE_H__
