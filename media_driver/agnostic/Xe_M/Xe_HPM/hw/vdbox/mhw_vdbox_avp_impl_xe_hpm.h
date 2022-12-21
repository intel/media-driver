/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     mhw_vdbox_avp_impl_xe_hpm.h
//! \brief    MHW VDBOX AVP interface common base for Xe_HPM platforms
//! \details
//!

#ifndef __MHW_VDBOX_AVP_IMPL_XE_HPM_H__
#define __MHW_VDBOX_AVP_IMPL_XE_HPM_H__

#include "mhw_vdbox_avp_hwcmd_xe_hpm.h"
#include "mhw_vdbox_avp_impl.h"
#include "mhw_mi_hwcmd_g12_X.h" // refactor
#include "mhw_mi_itf.h"

namespace mhw
{
namespace vdbox
{
namespace avp
{
namespace xe_hpm
{
    class Impl : public avp::Impl<Cmd>
    {
    public:
        Impl(PMOS_INTERFACE osItf) : base_t(osItf){};

        MOS_STATUS GetAvpStateCmdSize(uint32_t* commandsSize, uint32_t* patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
        {
            MHW_FUNCTION_ENTER;

            MHW_MI_CHK_NULL(commandsSize);
            MHW_MI_CHK_NULL(patchListSize);

            uint32_t    maxSize = 0;
            uint32_t    patchListMaxSize = 0;

            maxSize =
                8 +
                mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_MODE_SELECT_CMD::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_SURFACE_STATE_CMD::byteSize * 11 +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_INLOOP_FILTER_STATE_CMD::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_INTER_PRED_STATE_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(avp::Itf::VD_PIPELINE_FLUSH_CMD) +
                PATCH_LIST_COMMAND(mi::Itf::MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(avp::Itf::AVP_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(avp::Itf::AVP_SURFACE_STATE_CMD) * 11 +
                PATCH_LIST_COMMAND(avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(avp::Itf::AVP_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(avp::Itf::AVP_INTER_PRED_STATE_CMD) +
                PATCH_LIST_COMMAND(avp::Itf::AVP_INLOOP_FILTER_STATE_CMD);

            if (params->bDecodeInUse)
            {
                maxSize +=
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIC_STATE_CMD::byteSize +
                    mhw_mi_g12_X::VD_CONTROL_STATE_CMD::byteSize * 2;

                patchListMaxSize += PATCH_LIST_COMMAND(avp::Itf::AVP_PIC_STATE_CMD);

                MHW_CHK_NULL_RETURN(params);
                auto paramsG12 = dynamic_cast<PMHW_VDBOX_STATE_CMDSIZE_PARAMS_G12>(params);
                MHW_CHK_NULL_RETURN(paramsG12);
                if (paramsG12->bScalableMode)
                {
                    // VD_CONTROL_STATE AVP lock and unlock
                    maxSize += 2 * mhw_mi_g12_X::VD_CONTROL_STATE_CMD::byteSize;
                }
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

            if (params->bDecodeInUse)
            {
                maxSize =
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_TILE_CODING_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_BSD_OBJECT_CMD::byteSize +
                    mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(avp::Itf::AVP_TILE_CODING_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_BSD_OBJECT_CMD);
            }
            else
            {
                maxSize = mhw_mi_g12_X::MI_BATCH_BUFFER_START_CMD::byteSize * 5 +
                    mhw_mi_g12_X::VD_CONTROL_STATE_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_MODE_SELECT_CMD::byteSize * 2 +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_SURFACE_STATE_CMD::byteSize * 11 +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIC_STATE_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_INTER_PRED_STATE_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_SEGMENT_STATE_CMD::byteSize * 8 +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_INLOOP_FILTER_STATE_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_TILE_CODING_CMD::byteSize +
                    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PAK_INSERT_OBJECT_CMD::byteSize * 9 +
                    mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(avp::Itf::VD_PIPELINE_FLUSH_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_PIPE_MODE_SELECT_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_SURFACE_STATE_CMD) * 11 +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_PIC_STATE_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_INTER_PRED_STATE_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_SEGMENT_STATE_CMD) * 8 +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_INLOOP_FILTER_STATE_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_TILE_CODING_CMD) +
                    PATCH_LIST_COMMAND(avp::Itf::AVP_PAK_INSERT_OBJECT_CMD);
            }

            *commandsSize  = maxSize;
            *patchListSize = patchListMaxSize;

            return MOS_STATUS_SUCCESS;
        }

    protected:
        using cmd_t  = Cmd;
        using base_t = avp::Impl<Cmd>;

        // Programming Note: CodecHAL layer must add MFX wait command
        //                   for both KIN and VRT before and after AVP_PIPE_MODE_SELECT
        _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_MODE_SELECT)
        {
            _MHW_SETCMD_CALLBASE(AVP_PIPE_MODE_SELECT);

#define DO_FIELDS()                                                                                      \
    DO_FIELD(DW1, TileStatsStreamoutEnable, params.tileStatsStreamoutEnable ? 1 : 0);                   \
                                                                                                         \
    DO_FIELD(DW6, PAKFrameLevelStreamOutEnable, params.pakFrameLevelStreamOutEnable);                   \
    DO_FIELD(DW6, MotionCompMemoryTrackerCntEnable, params.motionCompMemoryTrackerCntEnable ? 1 : 0);   \
    DO_FIELD(DW6, SourcePixelPrefetchLen, params.srcPixelPrefetchLen);                                  \
    DO_FIELD(DW6, SourcePixelPrefetchEnable, params.srcPixelPrefetchEnable ? 1 : 0)

#include "mhw_hwcmd_process_cmdfields.h"
        }

        _MHW_SETCMD_OVERRIDE_DECL(AVP_SURFACE_STATE)
        {
            _MHW_SETCMD_CALLBASE(AVP_SURFACE_STATE);

            constexpr uint32_t m_rawUVPlaneAlignment    = 4;
            constexpr uint32_t m_reconUVPlaneAlignment  = 8;
            constexpr uint32_t m_uvPlaneAlignmentLegacy = 8;

            uint32_t uvPlaneAlignment = m_uvPlaneAlignmentLegacy;
            if (params.surfaceStateId == srcInputPic)
            {
                uvPlaneAlignment = params.uvPlaneAlignment ? params.uvPlaneAlignment : m_rawUVPlaneAlignment;
            }
            else
            {
                uvPlaneAlignment = params.uvPlaneAlignment ? params.uvPlaneAlignment : m_reconUVPlaneAlignment;
            }

#define DO_FIELDS()                                       \
    DO_FIELD(DW2, YOffsetForUCbInPixel, MOS_ALIGN_CEIL(params.uOffset, uvPlaneAlignment)); \
                                                                 \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1IntraFrame, MmcEnabled(params.mmcState[intraFrame]) ? 0xff : 0);   \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1LastFrame, MmcEnabled(params.mmcState[lastFrame]) ? 0xff : 0);    \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1Last2Frame, MmcEnabled(params.mmcState[last2Frame]) ? 0xff : 0);   \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1Last3Frame, MmcEnabled(params.mmcState[last3Frame]) ? 0xff : 0);   \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1GoldenFrame, MmcEnabled(params.mmcState[goldenFrame]) ? 0xff : 0);  \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1BwdrefFrame, MmcEnabled(params.mmcState[bwdRefFrame]) ? 0xff : 0);  \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1Altref2Frame, MmcEnabled(params.mmcState[altRef2Frame]) ? 0xff : 0); \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1AltrefFrame, MmcEnabled(params.mmcState[altRefFrame]) ? 0xff : 0);  \
    DO_FIELD(DW4, CompressionTypeForIntraFrame, MmcRcEnabled(params.mmcState[intraFrame]) ? 0xff : 0);            \
    DO_FIELD(DW4, CompressionTypeForLastFrame, MmcRcEnabled(params.mmcState[lastFrame]) ? 0xff : 0);             \
    DO_FIELD(DW4, CompressionTypeForLast2Frame, MmcRcEnabled(params.mmcState[last2Frame]) ? 0xff : 0);            \
    DO_FIELD(DW4, CompressionTypeForLast3Frame, MmcRcEnabled(params.mmcState[last3Frame]) ? 0xff : 0);            \
    DO_FIELD(DW4, CompressionTypeForGoldenFrame, MmcRcEnabled(params.mmcState[goldenFrame]) ? 0xff : 0);           \
    DO_FIELD(DW4, CompressionTypeForBwdrefFrame, MmcRcEnabled(params.mmcState[bwdRefFrame]) ? 0xff : 0);           \
    DO_FIELD(DW4, CompressionTypeForAltref2Frame, MmcRcEnabled(params.mmcState[altRef2Frame]) ? 0xff : 0);          \
    DO_FIELD(DW4, CompressionTypeForAltrefFrame, MmcRcEnabled(params.mmcState[altRefFrame]) ? 0xff : 0)

#include "mhw_hwcmd_process_cmdfields.h"
        }

    };
} // xe_hpm
} // avp
} // vdbox
} // mhw

#endif // __MHW_VDBOX_AVP_IMPL_XE_HPM_H__
