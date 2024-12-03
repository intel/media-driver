/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mhw_vdbox_mfx_impl_xe3_lpm_base.h
//! \brief    MHW VDBOX MFX interface common base for all XE3_LPM platforms
//! \details
//!

#ifndef __MHW_VDBOX_MFX_IMPL_XE3_LPM_BASE_H__
#define __MHW_VDBOX_MFX_IMPL_XE3_LPM_BASE_H__

#include "mhw_vdbox_mfx_impl.h"
#include "mhw_mi_hwcmd_xe3_lpm_base.h"

#define AVC_VLF_ROWSTORE_BASEADDRESS_MBAFF 1280

namespace mhw
{
namespace vdbox
{
namespace mfx
{
namespace xe3_lpm_base
{
#define mpeg2WeightScaleSize 16

template <typename cmd_t>
class BaseImpl : public mfx::Impl<cmd_t>
{
public:
    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isShortFormat)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t maxSize =
            mhw::mi::xe3_lpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize +
            cmd_t::MFX_PIPE_MODE_SELECT_CMD::byteSize +
            cmd_t::MFX_SURFACE_STATE_CMD::byteSize +
            cmd_t::MFX_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            cmd_t::MFX_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            2 * mhw::mi::xe3_lpm_base::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +
            2 * mhw::mi::xe3_lpm_base::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize +
            8 * mhw::mi::xe3_lpm_base::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;

        uint32_t patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_IND_OBJ_BASE_ADDR_STATE_CMD) +
            (2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD)) +
            (2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD));

        uint32_t standard = CodecHal_GetStandardFromMode(mode);
        if (standard == CODECHAL_AVC)
        {
            maxSize +=
                cmd_t::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                cmd_t::MFD_AVC_PICID_STATE_CMD::byteSize +
                cmd_t::MFX_AVC_DIRECTMODE_STATE_CMD::byteSize +
                cmd_t::MFX_AVC_IMG_STATE_CMD::byteSize +
                cmd_t::MFX_QM_STATE_CMD::byteSize * 4;  // QM_State sent 4 times

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_PICID_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_DIRECTMODE_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_IMG_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_QM_STATE_CMD) * 4;

            if (mode == CODECHAL_ENCODE_MODE_AVC)
            {
                maxSize +=
                    mhw::mi::xe3_lpm_base::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +
                    mhw::mi::xe3_lpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize * 3 +            // 3 extra MI_FLUSH_DWs for encode
                    cmd_t::MFX_FQM_STATE_CMD::byteSize * 4 +                                    // FQM_State sent 4 times
                    mhw::mi::xe3_lpm_base::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize * 8 +  // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                    mhw::mi::xe3_lpm_base::Cmd::MI_STORE_DATA_IMM_CMD::byteSize * 3 +      // slice level commands for StatusReport, BrcPakStatistics
                    MHW_VDBOX_PAK_BITSTREAM_OVERFLOW_SIZE +                                     // accounting for the max DW payload for PAK_INSERT_OBJECT, for frame header payload
                    cmd_t::MFX_PAK_INSERT_OBJECT_CMD::byteSize * 4;                             // for inserting AU, SPS, PSP, SEI headers before first slice header

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) * 3 +  // 3 extra MI_FLUSH_DWs for encode
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_FQM_STATE_CMD) * 4 +  // FQM_State sent 4 times
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD) * 8 +  // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) * 3;       // slice level commands for StatusReport, BrcPakStatistics
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFC_AVC_PAK_INSERT_OBJECT_CMD) * 4;  // for inserting AU, SPS, PSP, SEI headers before first slice header
            }
        }
        else if (standard == CODECHAL_MPEG2)
        {
            maxSize += cmd_t::MFX_MPEG2_PIC_STATE_CMD::byteSize;
            patchListMaxSize += PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_MPEG2_PIC_STATE_CMD);

            if (mode == CODECHAL_DECODE_MODE_MPEG2VLD)
            {
                maxSize +=
                    cmd_t::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                    ((cmd_t::MFX_QM_STATE_CMD::byteSize + (16 * sizeof(uint32_t))) * 2);

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_QM_STATE_CMD) * 2;
            }
            else if (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
            {
                maxSize +=
                    mhw::mi::xe3_lpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize * 2;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) * 2;
            }
        }
        else if (standard == CODECHAL_VP8)
        {
            maxSize +=
                cmd_t::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                cmd_t::MFX_VP8_PIC_STATE_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_VP8_PIC_STATE_CMD);
        }
        else if (standard == CODECHAL_JPEG)
        {
            // Added to prevent error for JPEG
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported decode mode.");
            maxSize          = 0;
            patchListMaxSize = 0;
            eStatus          = MOS_STATUS_UNKNOWN;
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isModeSpecific)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t maxSize = 0, patchListMaxSize = 0;
        uint32_t standard = CodecHal_GetStandardFromMode(mode);

        if (standard == CODECHAL_AVC)
        {
            if (mode == CODECHAL_DECODE_MODE_AVCVLD)
            {
                maxSize =
                    cmd_t::MFX_AVC_SLICE_STATE_CMD::byteSize +
                    cmd_t::MFD_AVC_BSD_OBJECT_CMD::byteSize +
                    cmd_t::MFD_AVC_DPB_STATE_CMD::byteSize +
                    mhw::mi::xe3_lpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_SLICE_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_BSD_OBJECT_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_DPB_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD);

                if (isModeSpecific)
                {
                    // isModeSpecific = bShortFormat for AVC decode
                    maxSize +=
                        cmd_t::MFD_AVC_DPB_STATE_CMD::byteSize +
                        cmd_t::MFD_AVC_SLICEADDR_CMD::byteSize;

                    patchListMaxSize +=
                        PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_DPB_STATE_CMD) +
                        PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_SLICEADDR_CMD);
                }
                else
                {
                    maxSize +=
                        (2 * cmd_t::MFX_AVC_REF_IDX_STATE_CMD::byteSize) +
                        (2 * cmd_t::MFX_AVC_WEIGHTOFFSET_STATE_CMD::byteSize);

                    patchListMaxSize +=
                        (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_REF_IDX_STATE_CMD)) +
                        (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_WEIGHTOFFSET_STATE_CMD));
                }
            }
            else  // CODECHAL_ENCODE_MODE_AVC
            {
                // 1 PAK_INSERT_OBJECT inserted for every end of frame/stream with 1 DW payload
                maxSize          = cmd_t::MFX_PAK_INSERT_OBJECT_CMD::byteSize + sizeof(uint32_t);
                patchListMaxSize = PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFC_AVC_PAK_INSERT_OBJECT_CMD);

                if (isModeSpecific)
                {
                    // isModeSpecific = bSingleTaskPhaseSupported for AVC encode
                    maxSize += (2 * mhw::mi::xe3_lpm_base::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize);
                    patchListMaxSize += (2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD));
                }
                else
                {
                    maxSize +=
                        (2 * cmd_t::MFX_AVC_REF_IDX_STATE_CMD::byteSize) +
                        (2 * cmd_t::MFX_AVC_WEIGHTOFFSET_STATE_CMD::byteSize) +
                        cmd_t::MFX_AVC_SLICE_STATE_CMD::byteSize +
                        MHW_VDBOX_PAK_SLICE_HEADER_OVERFLOW_SIZE +  // slice header payload
                        (2 * cmd_t::MFX_PAK_INSERT_OBJECT_CMD::byteSize) +
                        mhw::mi::xe3_lpm_base::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize +
                        mhw::mi::xe3_lpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize;

                    patchListMaxSize +=
                        (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_REF_IDX_STATE_CMD)) +
                        (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_WEIGHTOFFSET_STATE_CMD)) +
                        PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_SLICE_STATE_CMD) +
                        (2 * PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFC_AVC_PAK_INSERT_OBJECT_CMD)) +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD);
                }
            }
        }
        else if (standard == CODECHAL_MPEG2)
        {
            if (mode == CODECHAL_DECODE_MODE_MPEG2VLD)
            {
                maxSize =
                    cmd_t::MFD_MPEG2_BSD_OBJECT_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_MPEG2_BSD_OBJECT_CMD);
            }
            else if (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
            {
                maxSize =
                    cmd_t::MFD_IT_OBJECT_CMD::byteSize +
                    cmd_t::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_MPEG2_IT_OBJECT_CMD);
            }
        }
        else if (standard == CODECHAL_VP8)
        {
            maxSize =
                cmd_t::MFD_VP8_BSD_OBJECT_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_VP8_BSD_OBJECT_CMD);
        }
        else if (standard == CODECHAL_JPEG)
        {
            maxSize +=
                cmd_t::MFX_FQM_STATE_CMD::byteSize * 3 +
                cmd_t::MFC_JPEG_HUFF_TABLE_STATE_CMD::byteSize * 2 +
                cmd_t::MFC_JPEG_SCAN_OBJECT_CMD::byteSize +
                cmd_t::MFX_PAK_INSERT_OBJECT_CMD::byteSize * 10;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported decode mode.");
            eStatus = MOS_STATUS_UNKNOWN;
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

    MOS_STATUS GetRowstoreCachingAddrs(PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(rowstoreParams);
        mfx::Impl<cmd_t>::GetRowstoreCachingAddrs(rowstoreParams);

        // VLF rowstore cache update for AVC Mbaff on Xe3
        bool avc          = rowstoreParams->Mode == CODECHAL_DECODE_MODE_AVCVLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC;
        bool vp8          = rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP8VLD || rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP8;
        bool widthLE4K    = rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K;
        bool mbaffOrField = rowstoreParams->bMbaff || !rowstoreParams->bIsFrame;

        Itf::m_deblockingFilterRowstoreCache.enabled = Itf::m_deblockingFilterRowstoreCache.supported && widthLE4K && (avc || vp8);
        if (Itf::m_deblockingFilterRowstoreCache.enabled)
        {
            Itf::m_deblockingFilterRowstoreCache.dwAddress = avc ? (mbaffOrField ? AVC_VLF_ROWSTORE_BASEADDRESS_MBAFF
                                                                            : AVC_VLF_ROWSTORE_BASEADDRESS)
                                                                            : VP8_VLF_ROWSTORE_BASEADDRESS;
        }
        else
        {
            Itf::m_deblockingFilterRowstoreCache.dwAddress = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = mfx::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__mfx__xe3_lpm_base__BaseImpl)
};
}  // namespace xe3_lpm_base
}  // namespace mfx
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_MFX_IMPL_XE3_LPM_BASE_H__
