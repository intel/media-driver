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
//! \file     mhw_vdbox_hcp_impl_xe_lpm_plus_base.h
//! \brief    MHW VDBOX HCP interface common base for all XE_LPM_PLUS platforms
//! \details
//!

#ifndef __MHW_VDBOX_HCP_IMPL_XE_LPM_PLUS_BASE_H__
#define __MHW_VDBOX_HCP_IMPL_XE_LPM_PLUS_BASE_H__

#include "mhw_vdbox_hcp_impl.h"
#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "mhw_vdbox_vdenc_impl_xe_lpm_plus.h"
#include "mhw_mi_hwcmd_xe_lpm_plus_base_next.h"
#include "mhw_sfc_hwcmd_xe_lpm_plus_next.h"

namespace mhw
{
namespace vdbox
{
namespace hcp
{
namespace xe_lpm_plus_base
{
template <typename cmd_t>
class BaseImpl : public hcp::Impl<cmd_t>
{
public:
    uint32_t GetHcpVp9PicStateCommandSize()
    {
        return cmd_t::HCP_VP9_PIC_STATE_CMD::byteSize;
    }

    uint32_t GetHcpVp9SegmentStateCommandSize()
    {
        return cmd_t::HCP_VP9_SEGMENT_STATE_CMD::byteSize;
    }

    MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t *                      commandsSize,
        uint32_t *                      patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;
        uint32_t standard         = CodecHal_GetStandardFromMode(mode);
        MHW_CHK_NULL_RETURN(params);
        auto par = dynamic_cast<mhw::vdbox::xe_lpm_plus_base::PMHW_VDBOX_STATE_CMDSIZE_PARAMS_XE_LPM_PLUS_BASE>(params);
        MHW_CHK_NULL_RETURN(par);

        if (standard == CODECHAL_HEVC)
        {
            maxSize =
                mhw::vdbox::vdenc::xe_lpm_plus_base::v0::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize +
                mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize +
                cmd_t::HCP_PIPE_MODE_SELECT_CMD::byteSize +
                cmd_t::HCP_SURFACE_STATE_CMD::byteSize +
                cmd_t::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                cmd_t::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize * 8;

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::VD_PIPELINE_FLUSH_CMD) +
                PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD);

            if (mode == CODECHAL_ENCODE_MODE_HEVC)
            {
                /* HCP_QM_STATE_CMD may be issued up to 20 times: 3x Colour Component plus 2x intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components.
            HCP_FQP_STATE_CMD may be issued up to 8 times: 4 scaling list per intra and inter. */
                maxSize +=
                    2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize +
                    cmd_t::HCP_SURFACE_STATE_CMD::byteSize +  // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
                    20 * cmd_t::HCP_QM_STATE_CMD::byteSize +
                    8 * cmd_t::HCP_FQM_STATE_CMD::byteSize +
                    cmd_t::HCP_PIC_STATE_CMD::byteSize +
                    cmd_t::HEVC_VP9_RDOQ_STATE_CMD::byteSize +                                      // RDOQ
                    2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +      // Slice level commands
                    2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize +            // need for Status report, Mfc Status and
                    10 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize + // 8 for BRCStatistics and 2 for RC6 WAs
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_MEM_CMD::byteSize +       // 1 for RC6 WA
                    2 * cmd_t::HCP_PAK_INSERT_OBJECT_CMD::byteSize +                                // Two PAK insert object commands are for headers before the slice header and the header for the end of stream
                    4 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +      // two (BRC+reference frame) for clean-up HW semaphore memory and another two for signal it
                    17 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize +     // Use HW wait command for each reference and one wait for current semaphore object
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize +          // Use HW wait command for each BRC pass
                    +mhw::mi::xe_lpm_plus_base_next::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize           // Use HW wait command for each VDBOX
                    + 2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize      // One is for reset and another one for set per VDBOX
                    + 8 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_COPY_MEM_MEM_CMD::byteSize        // Need to copy SSE statistics/ Slice Size overflow into memory
                    ;

                patchListMaxSize +=
                    20 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_QM_STATE_CMD) +
                    8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_FQM_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIC_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +       // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
                    2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) +       // Slice level commands
                    2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +             // need for Status report, Mfc Status and
                    11 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD) +  // 8 for BRCStatistics and 3 for RC6 WAs
                    22 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD)        // Use HW wait commands plus its memory clean-up and signal (4+ 16 + 1 + 1)
                    + 8 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD)   // At maximal, there are 8 batch buffers for 8 VDBOXes for VE. Each box has one BB.
                    + PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD)                 // Need one flush before copy command
                    + PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_WAIT_CMD)              // Need one wait after copy command
                    + 3 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD)       // one wait commands and two for reset and set semaphore memory
                    + 8 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_COPY_MEM_MEM_CMD)         // Need to copy SSE statistics/ Slice Size overflow into memory
                    ;
            }
            else
            {
                maxSize +=
                    2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize +  // VD_CONTROL_STATE Hcp init and flush
                    20 * cmd_t::HCP_QM_STATE_CMD::byteSize +
                    cmd_t::HCP_PIC_STATE_CMD::byteSize +
                    cmd_t::HCP_TILE_STATE_CMD::byteSize;

                patchListMaxSize +=
                    20 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_QM_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIC_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_TILE_STATE_CMD);

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
                    patchListMaxSize +=
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_STATE_CMD_NUMBER_OF_ADDRESSES +
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_CHROMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_LUMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_STATE_CMD_NUMBER_OF_ADDRESSES +
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_FRAME_START_CMD_NUMBER_OF_ADDRESSES +
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_IEF_STATE_CMD_NUMBER_OF_ADDRESSES +
                        mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_LOCK_CMD_NUMBER_OF_ADDRESSES;
                }

                if (par->bScalableMode)
                {
                    // VD_CONTROL_STATE Hcp lock and unlock
                    maxSize += 2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize;

                    // Due to the fact that there is no slice level command in BE status, we mainly consider commands in FE.
                    maxSize +=
                        4 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_ATOMIC_CMD::byteSize +                        // used to reset semaphore in BEs
                        2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +  // 1 Conditional BB END for FE hang, 1 for streamout buffer writing over allocated size
                        3 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize +                // for FE & BE0, BEs sync
                        15 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +               // for placeholder cmds to resolve the hazard between BEs sync
                        3 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +                // for FE status set and clear
                        3 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_IMM_CMD::byteSize +             // for FE status set
                        2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize +                      // 2 needed for command flush in slice level
                        2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize +            // store the carry flag of reported size in FE
                        4 * sizeof(MHW_MI_ALU_PARAMS) +                                                           // 4 ALU commands needed for substract opertaion in FE
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_MATH_CMD::byteSize +                              // 1 needed for FE status set
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;                  // 1 needed for FE status set
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_MATH_CMD::byteSize +                                  // 1 needed for FE status set
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;                  // 1 needed for FE status set

                    patchListMaxSize +=
                        4 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_ATOMIC_CMD) +
                        2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                        3 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_SEMAPHORE_WAIT_CMD) +
                        18 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) +
                        2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
                        2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD);

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
                        patchListMaxSize +=
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_STATE_CMD_NUMBER_OF_ADDRESSES +
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_CHROMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_LUMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_STATE_CMD_NUMBER_OF_ADDRESSES +
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_FRAME_START_CMD_NUMBER_OF_ADDRESSES +
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_IEF_STATE_CMD_NUMBER_OF_ADDRESSES +
                            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_LOCK_CMD_NUMBER_OF_ADDRESSES;
                    }
                }
            }
        }
        else if (standard == CODECHAL_VP9) // VP9 Clear Decode
        {
            maxSize =
                mhw::vdbox::vdenc::xe_lpm_plus_base::v0::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize +
                mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize +
                cmd_t::HCP_PIPE_MODE_SELECT_CMD::byteSize +
                cmd_t::HCP_SURFACE_STATE_CMD::byteSize * 4 +
                cmd_t::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                cmd_t::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                cmd_t::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                cmd_t::HCP_BSD_OBJECT_CMD::byteSize +
                mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize * 8;

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::VD_PIPELINE_FLUSH_CMD) +
                PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD) * 4 +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_BSD_OBJECT_CMD);

            if (mode == CODECHAL_ENCODE_MODE_VP9)
            {
                maxSize +=
                    cmd_t::HCP_VP9_PIC_STATE_CMD::byteSize +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize * 2 +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize * 4 +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize * 11 +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_COPY_MEM_MEM_CMD::byteSize * 4 +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize * 3 +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize * 2 +  // Slice level commands
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_MEM_CMD::byteSize * 2 +
                    cmd_t::HCP_PAK_INSERT_OBJECT_CMD::byteSize * 2 +
                    cmd_t::HCP_TILE_CODING_CMD::byteSize +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize +     // Use HW wait command for each VDBOX
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize * 3;  // One is for reset and another one for set per VDBOX, one for wait

                maxSize += 3 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize;  // VD_CONTROL_STATE Hcp init + flush + vdenc init

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_VP9_PIC_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) * 2 +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) * 4 +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD) * 11 +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_COPY_MEM_MEM_CMD) * 4 +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) * 3 +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) * 2 +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PAK_INSERT_OBJECT_CMD) * 2 +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND) +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +
                    PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) * 2;
            }
            else
            {
                maxSize += cmd_t::HCP_VP9_PIC_STATE_CMD::byteSize;

                // VD_CONTROL_STATE Hcp init and flush
                maxSize += 2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize;

                patchListMaxSize += PATCH_LIST_COMMAND(mhw::vdbox::hcp::HCP_VP9_PIC_STATE_CMD);

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

                if (par->bScalableMode)
                {
                    // VD_CONTROL_STATE Hcp lock and unlock
                    maxSize += 2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize;

                    maxSize +=
                        cmd_t::HCP_TILE_CODING_CMD::byteSize +
                        2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize +
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_ATOMIC_CMD::byteSize * 4 +                    // used to reset semaphore in BEs
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +  // for streamout buffer writing over allocated size
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize * 3 +            // for FE & BE0, BEs sync
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize * 15 +           // for placeholder cmds to resolve the hazard between BEs sync
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +                // for FE status set
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_IMM_CMD::byteSize * 3 +         // for FE status set
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize +                      // for command flush in partition level
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize * 2 +        // store the carry flag of reported size in FE
                        4 * sizeof(MHW_MI_ALU_PARAMS) +                                                       // 4 ALU commands needed for substract opertaion in FE
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_MATH_CMD::byteSize +                          // 1 needed for FE status set
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;              // 1 needed for FE status set
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_MATH_CMD::byteSize +                              // 1 needed for FE status set
                        mhw::mi::xe_lpm_plus_base_next::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;              // 1 needed for FE status set

                    patchListMaxSize +=
                        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND) +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_ATOMIC_CMD) * 4 +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_SEMAPHORE_WAIT_CMD) * 3 +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
                        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD) * 2;

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
                }
            }
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported standard.");
            eStatus = MOS_STATUS_UNKNOWN;
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

    MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      modeSpecific)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        uint32_t standard         = CodecHal_GetStandardFromMode(mode);
        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;

        if (standard == CODECHAL_HEVC)
        {
            if (mode == CODECHAL_ENCODE_MODE_HEVC)
            {
                maxSize =
                    2 * cmd_t::HCP_REF_IDX_STATE_CMD::byteSize +
                    2 * cmd_t::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                    cmd_t::HCP_SLICE_STATE_CMD::byteSize +
                    cmd_t::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
                    2 * mhw::mi::xe_lpm_plus_base_next::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize +
                    cmd_t::HCP_TILE_CODING_CMD::byteSize;  // one slice cannot be with more than one tile

                patchListMaxSize =
                    2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_REF_IDX_STATE_CMD) +
                    2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_WEIGHTOFFSET_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SLICE_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PAK_INSERT_OBJECT_CMD) +
                    2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +  // One is for the PAK command and another one is for the BB when BRC and single task mode are on
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND);         // HCP_TILE_CODING_STATE command
            }
            else
            {
                maxSize =
                    2 * cmd_t::HCP_REF_IDX_STATE_CMD::byteSize +
                    2 * cmd_t::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                    cmd_t::HCP_SLICE_STATE_CMD::byteSize +
                    2 * mhw::mi::xe_lpm_plus_base_next::Cmd::VD_CONTROL_STATE_CMD::byteSize +
                    cmd_t::HCP_TILE_CODING_CMD::byteSize +
                    cmd_t::HCP_PALETTE_INITIALIZER_STATE_CMD::byteSize +
                    cmd_t::HCP_BSD_OBJECT_CMD::byteSize +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;

                patchListMaxSize =
                    2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_REF_IDX_STATE_CMD) +
                    2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_WEIGHTOFFSET_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SLICE_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PALETTE_INITIALIZER_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_BSD_OBJECT_CMD);
            }
        }
        else if (standard == CODECHAL_VP9) // VP9 Clear decode does not require primitive level commands. VP9 DRM does.
        {
            if (modeSpecific) // VP9 DRM
            {
                maxSize +=
                    cmd_t::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                    cmd_t::HCP_VP9_PIC_STATE_CMD::byteSize +
                    cmd_t::HCP_BSD_OBJECT_CMD::byteSize +
                    mhw::mi::xe_lpm_plus_base_next::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_VP9_PIC_STATE_CMD) +
                    PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_BSD_OBJECT_CMD);
            }
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported standard.");
            eStatus = MOS_STATUS_UNKNOWN;
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

protected:
    using base_t = hcp::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__hcp__xe_lpm_plus_base__BaseImpl)
};
}  // namespace xe_lpm_plus_base
}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HCP_IMPL_XE_LPM_PLUS_BASE_H__
