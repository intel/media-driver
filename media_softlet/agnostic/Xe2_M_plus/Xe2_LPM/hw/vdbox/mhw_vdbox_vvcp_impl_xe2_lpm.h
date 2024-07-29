/*
# Copyright (c) 2024, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     mhw_vdbox_vvcp_impl_xe2_lpm.h
//! \brief    MHW VDBOX VVCP interface common base for Xe2_LPM
//! \details
//!

#ifndef __MHW_VDBOX_VVCP_IMPL_XE2_LPM_H__
#define __MHW_VDBOX_VVCP_IMPL_XE2_LPM_H__

#include "mhw_vdbox_vvcp_impl_xe2_lpm_base.h"
#include "mhw_mi_hwcmd_xe2_lpm_base_next.h"
#include "mhw_vdbox_vvcp_hwcmd_xe2_lpm_X.h"

namespace mhw
{
namespace vdbox
{
namespace vvcp
{
namespace xe2_lpm_base
{
namespace xe2_lpm
{

class Impl : public BaseImpl<Cmd>
{
protected:
    using cmd_t = Cmd;
    using base_t = BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};

    MOS_STATUS GetVvcpStateCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;

        maxSize =
            mhw::mi::xe2_lpm_base_next::Cmd::MI_FLUSH_DW_CMD::byteSize +
            cmd_t::VVCP_SURFACE_STATE_CMD::byteSize * 16 +
            cmd_t::VVCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            cmd_t::VVCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            cmd_t::VVCP_DPB_STATE_CMD::byteSize +
            cmd_t::VVCP_PIC_STATE_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_SURFACE_STATE_CMD) * 16 +
            PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_PIC_STATE_CMD);

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetVvcpSliceLvlCmdSize(
        uint32_t *sliceLvlCmdSize) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(sliceLvlCmdSize);
        if (m_decodeInUse)
        {
            *sliceLvlCmdSize =
                cmd_t::VVCP_SLICE_STATE_CMD::byteSize +
                cmd_t::VVCP_REF_IDX_STATE_CMD::byteSize * 2 +
                cmd_t::VVCP_WEIGHTOFFSET_STATE_CMD::byteSize * 2 +
                cmd_t::VVCP_BSD_OBJECT_CMD::byteSize +
                cmd_t::VVCP_TILE_CODING_CMD::byteSize +
                cmd_t::VVCP_VD_CONTROL_STATE_CMD::byteSize * 2 +
                cmd_t::VVCP_PIPE_MODE_SELECT_CMD::byteSize +
                mhw::mi::xe2_lpm_base_next::Cmd::MFX_WAIT_CMD::byteSize * 2 +
                mhw::mi::xe2_lpm_base_next::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize +
                mhw::mi::xe2_lpm_base_next::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetVvcpPrimitiveCmdSize(
        uint32_t *sliceCommandsSize,
        uint32_t *slicePatchListSize,
        uint32_t *tileCommandsSize,
        uint32_t *tilePatchListSize) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(sliceCommandsSize);
        MHW_MI_CHK_NULL(slicePatchListSize);
        MHW_MI_CHK_NULL(tileCommandsSize);
        MHW_MI_CHK_NULL(tilePatchListSize);

        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;

        if (m_decodeInUse)
        {
            // MI_BATCH_BUFFER_START is added here because each slice needs to jump to pic-level cmd in 2nd level BB
            maxSize =
                cmd_t::VVCP_SLICE_STATE_CMD::byteSize +
                cmd_t::VVCP_REF_IDX_STATE_CMD::byteSize * 2 +
                cmd_t::VVCP_WEIGHTOFFSET_STATE_CMD::byteSize * 2 +
                cmd_t::VVCP_BSD_OBJECT_CMD::byteSize +
                cmd_t::VVCP_TILE_CODING_CMD::byteSize +
                cmd_t::VVCP_VD_CONTROL_STATE_CMD::byteSize * 2 +
                cmd_t::VVCP_PIPE_MODE_SELECT_CMD::byteSize +
                mhw::mi::xe2_lpm_base_next::Cmd::MFX_WAIT_CMD::byteSize * 2 +
                mhw::mi::xe2_lpm_base_next::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize +
                mhw::mi::xe2_lpm_base_next::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize +
                8;//VD_PIPELINE_FLUSH_CMD::byteSize is 8

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_REF_IDX_STATE_CMD) * 2 +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_WEIGHTOFFSET_STATE_CMD) * 2 +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_BSD_OBJECT_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_TILE_CODING_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_VD_CONTROL_STATE_CMD) * 2 +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VD_PIPELINE_FLUSH_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::vvcp::VVCP_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD);

            *tileCommandsSize  = cmd_t::VVCP_TILE_CODING_CMD::byteSize + 
                                 mhw::mi::xe2_lpm_base_next::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;
            *tilePatchListSize = PATCH_LIST_COMMAND(VVCP_TILE_CODING_CMD);
        }

        *sliceCommandsSize  = maxSize;
        *slicePatchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetAlfApsDataBuffer(uint32_t *buffer, CodecVvcAlfData *alfApsArray, uint8_t activeAlfMask)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(buffer);
        MHW_MI_CHK_NULL(alfApsArray);

        cmd_t::VVCP_APS_ALF_PARAMSET *alfBuffer = (cmd_t::VVCP_APS_ALF_PARAMSET *)buffer;

        for (auto i = 0; i < 8; i++)
        {
            if ((activeAlfMask >> i) & 0x1)
            {
                // Luma Coeff Delta Idx
                alfBuffer[i].DW0.AlfLumaCoeffDeltaIdx0 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[0];
                alfBuffer[i].DW0.AlfLumaCoeffDeltaIdx1 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[1];
                alfBuffer[i].DW0.AlfLumaCoeffDeltaIdx2 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[2];
                alfBuffer[i].DW0.AlfLumaCoeffDeltaIdx3 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[3];
                alfBuffer[i].DW0.AlfLumaCoeffDeltaIdx4 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[4];
                alfBuffer[i].DW0.AlfLumaCoeffDeltaIdx5 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[5];

                alfBuffer[i].DW1.AlfLumaCoeffDeltaIdx6  = alfApsArray[i].m_alfLumaCoeffDeltaIdx[6];
                alfBuffer[i].DW1.AlfLumaCoeffDeltaIdx7  = alfApsArray[i].m_alfLumaCoeffDeltaIdx[7];
                alfBuffer[i].DW1.AlfLumaCoeffDeltaIdx8  = alfApsArray[i].m_alfLumaCoeffDeltaIdx[8];
                alfBuffer[i].DW1.AlfLumaCoeffDeltaIdx9  = alfApsArray[i].m_alfLumaCoeffDeltaIdx[9];
                alfBuffer[i].DW1.AlfLumaCoeffDeltaIdx10 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[10];
                alfBuffer[i].DW1.AlfLumaCoeffDeltaIdx11 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[11];

                alfBuffer[i].DW2.AlfLumaCoeffDeltaIdx12 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[12];
                alfBuffer[i].DW2.AlfLumaCoeffDeltaIdx13 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[13];
                alfBuffer[i].DW2.AlfLumaCoeffDeltaIdx14 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[14];
                alfBuffer[i].DW2.AlfLumaCoeffDeltaIdx15 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[15];
                alfBuffer[i].DW2.AlfLumaCoeffDeltaIdx16 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[16];
                alfBuffer[i].DW2.AlfLumaCoeffDeltaIdx17 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[17];

                alfBuffer[i].DW3.AlfLumaCoeffDeltaIdx18 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[18];
                alfBuffer[i].DW3.AlfLumaCoeffDeltaIdx19 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[19];
                alfBuffer[i].DW3.AlfLumaCoeffDeltaIdx20 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[20];
                alfBuffer[i].DW3.AlfLumaCoeffDeltaIdx21 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[21];
                alfBuffer[i].DW3.AlfLumaCoeffDeltaIdx22 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[22];
                alfBuffer[i].DW3.AlfLumaCoeffDeltaIdx23 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[23];

                alfBuffer[i].DW4.AlfLumaCoeffDeltaIdx24 = alfApsArray[i].m_alfLumaCoeffDeltaIdx[24];

                // Alf Luma Clip Idx
                for (auto j = 0; j < 25; j++)
                {
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx0  = alfApsArray[i].m_alfLumaClipIdx[j][0];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx1  = alfApsArray[i].m_alfLumaClipIdx[j][1];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx2  = alfApsArray[i].m_alfLumaClipIdx[j][2];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx3  = alfApsArray[i].m_alfLumaClipIdx[j][3];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx4  = alfApsArray[i].m_alfLumaClipIdx[j][4];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx5  = alfApsArray[i].m_alfLumaClipIdx[j][5];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx6  = alfApsArray[i].m_alfLumaClipIdx[j][6];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx7  = alfApsArray[i].m_alfLumaClipIdx[j][7];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx8  = alfApsArray[i].m_alfLumaClipIdx[j][8];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx9  = alfApsArray[i].m_alfLumaClipIdx[j][9];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx10 = alfApsArray[i].m_alfLumaClipIdx[j][10];
                    alfBuffer[i].AlfLumaClipIdx[j].alf_luma_clip_idx11 = alfApsArray[i].m_alfLumaClipIdx[j][11];
                }

                alfBuffer[i].DW30_Reserved = 0;
                alfBuffer[i].DW31_Reserved = 0;

                // CoeffL
                for (auto k = 0; k < 25; k++)
                {
                    int32_t clIdx   = k / 5;
                    int32_t startDW = (clIdx << 4) + (k % 5) * 3;

                    // CoeffL[K]
                    alfBuffer[i].AlfCoeffL[startDW]     = Pack4Bytes2DW(alfApsArray[i].m_alfCoeffL[k][0],
                        alfApsArray[i].m_alfCoeffL[k][1],
                        alfApsArray[i].m_alfCoeffL[k][2],
                        alfApsArray[i].m_alfCoeffL[k][3]);
                    alfBuffer[i].AlfCoeffL[startDW + 1] = Pack4Bytes2DW(alfApsArray[i].m_alfCoeffL[k][4],
                        alfApsArray[i].m_alfCoeffL[k][5],
                        alfApsArray[i].m_alfCoeffL[k][6],
                        alfApsArray[i].m_alfCoeffL[k][7]);
                    alfBuffer[i].AlfCoeffL[startDW + 2] = Pack4Bytes2DW(alfApsArray[i].m_alfCoeffL[k][8],
                        alfApsArray[i].m_alfCoeffL[k][9],
                        alfApsArray[i].m_alfCoeffL[k][10],
                        alfApsArray[i].m_alfCoeffL[k][11]);
                }

                // AlfCoeffC
                for (auto j = 0; j < 8; j += 2)
                {
                    int32_t startDW                     = (j >> 1) * 3;
                    alfBuffer[i].AlfCoeffC[startDW]     = Pack4Bytes2DW(alfApsArray[i].m_alfCoeffC[j][0],
                        alfApsArray[i].m_alfCoeffC[j][1],
                        alfApsArray[i].m_alfCoeffC[j][2],
                        alfApsArray[i].m_alfCoeffC[j][3]);
                    alfBuffer[i].AlfCoeffC[startDW + 1] = Pack4Bytes2DW(alfApsArray[i].m_alfCoeffC[j][4],
                        alfApsArray[i].m_alfCoeffC[j][5],
                        alfApsArray[i].m_alfCoeffC[j + 1][0],
                        alfApsArray[i].m_alfCoeffC[j + 1][1]);
                    alfBuffer[i].AlfCoeffC[startDW + 2] = Pack4Bytes2DW(alfApsArray[i].m_alfCoeffC[j + 1][2],
                        alfApsArray[i].m_alfCoeffC[j + 1][3],
                        alfApsArray[i].m_alfCoeffC[j + 1][4],
                        alfApsArray[i].m_alfCoeffC[j + 1][5]);
                }

                // alf_chroma_clip_idx
                alfBuffer[i].AlfChromaClipIdx[0] = alfApsArray[i].m_alfChromaClipIdx[0][0] |
                                                   (alfApsArray[i].m_alfChromaClipIdx[0][1] << 2) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[0][2] << 4) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[0][3] << 6) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[0][4] << 8) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[0][5] << 10) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[1][0] << 12) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[1][1] << 14) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[1][2] << 16) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[1][3] << 18) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[1][4] << 20) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[1][5] << 22) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[2][0] << 24) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[2][1] << 26) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[2][2] << 28) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[2][3] << 30);

                alfBuffer[i].AlfChromaClipIdx[1] = alfApsArray[i].m_alfChromaClipIdx[2][4] |
                                                   (alfApsArray[i].m_alfChromaClipIdx[2][5] << 2) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[3][0] << 4) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[3][1] << 6) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[3][2] << 8) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[3][3] << 10) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[3][4] << 12) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[3][5] << 14) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[4][0] << 16) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[4][1] << 18) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[4][2] << 20) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[4][3] << 22) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[4][4] << 24) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[4][5] << 26) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[5][0] << 28) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[5][1] << 30);

                alfBuffer[i].AlfChromaClipIdx[2] = alfApsArray[i].m_alfChromaClipIdx[5][2] |
                                                   (alfApsArray[i].m_alfChromaClipIdx[5][3] << 2) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[5][4] << 4) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[5][5] << 6) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[6][0] << 8) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[6][1] << 10) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[6][2] << 12) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[6][3] << 14) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[6][4] << 16) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[6][5] << 18) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[7][0] << 20) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[7][1] << 22) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[7][2] << 24) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[7][3] << 26) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[7][4] << 28) |
                                                   (alfApsArray[i].m_alfChromaClipIdx[7][5] << 30);

                alfBuffer[i].D127_Reserved = 0;

                // CC Cb Coeff Sign
                uint32_t ccCbCoeffSign = 0;
                for (auto j = 0; j < 4; j++)
                {
                    for (auto k = 0; k < 7; k++)
                    {
                        ccCbCoeffSign |= (alfApsArray[i].m_ccAlfApsCoeffCb[j][k] < 0 ? 0x1 : 0x0) << (j * 7 + k);
                    }
                }
                alfBuffer[i].DW128.AlfCcCbCoeffSign              = ccCbCoeffSign;
                alfBuffer[i].DW128.AlfCcCbFiltersSignalledMinus1 = alfApsArray[i].m_alfCcCbFiltersSignalledMinus1 & 0x3;

                //CC Cb Coeff ABS
                alfBuffer[i].AlfCcCbMappedCoeffAbs[0] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][0])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][1])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][2])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][3])) & 0x7);

                alfBuffer[i].AlfCcCbMappedCoeffAbs[1] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][4])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][5])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[0][6])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][0])) & 0x7);

                alfBuffer[i].AlfCcCbMappedCoeffAbs[2] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][1])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][2])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][3])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][4])) & 0x7);

                alfBuffer[i].AlfCcCbMappedCoeffAbs[3] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][5])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[1][6])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][0])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][1])) & 0x7);

                alfBuffer[i].AlfCcCbMappedCoeffAbs[4] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][2])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][3])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][4])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][5])) & 0x7);

                alfBuffer[i].AlfCcCbMappedCoeffAbs[5] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[2][6])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][0])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][1])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][2])) & 0x7);

                alfBuffer[i].AlfCcCbMappedCoeffAbs[6] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][3])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][4])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][5])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCb[3][6])) & 0x7);

                // CC Cr Coeff Sign
                uint32_t ccCrCoeffSign = 0;
                for (auto j = 0; j < 4; j++)
                {
                    for (auto k = 0; k < 7; k++)
                    {
                        ccCrCoeffSign |= (alfApsArray[i].m_ccAlfApsCoeffCr[j][k] < 0 ? 0x1 : 0x0) << (j * 7 + k);
                    }
                }
                alfBuffer[i].DW136.AlfCcCrCoeffSign              = ccCrCoeffSign;
                alfBuffer[i].DW136.AlfCcCrFiltersSignalledMinus1 = alfApsArray[i].m_alfCcCrFiltersSignalledMinus1 & 0x3;

                // CC Cr Coeff ABS
                alfBuffer[i].AlfCcCrMappedCoeffAbs[0] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][0])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][1])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][2])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][3])) & 0x7);

                alfBuffer[i].AlfCcCrMappedCoeffAbs[1] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][4])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][5])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[0][6])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][0])) & 0x7);

                alfBuffer[i].AlfCcCrMappedCoeffAbs[2] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][1])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][2])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][3])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][4])) & 0x7);

                alfBuffer[i].AlfCcCrMappedCoeffAbs[3] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][5])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[1][6])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][0])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][1])) & 0x7);

                alfBuffer[i].AlfCcCrMappedCoeffAbs[4] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][2])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][3])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][4])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][5])) & 0x7);

                alfBuffer[i].AlfCcCrMappedCoeffAbs[5] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[2][6])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][0])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][1])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][2])) & 0x7);

                alfBuffer[i].AlfCcCrMappedCoeffAbs[6] = Pack4Bytes2DW(
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][3])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][4])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][5])) & 0x7,
                    GetMappedCoeff(MOS_ABS(alfApsArray[i].m_ccAlfApsCoeffCr[3][6])) & 0x7);
            }
        }

        return MOS_STATUS_SUCCESS;
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vvcp__xe2_lpm_base__xe2_lpm__Impl)
};
}  // namespace xe2_lpm
}  // namespace xe2_lpm_base
}  // namespace vvcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VVCP_IMPL_XE2_LPM_H__
