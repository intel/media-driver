/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_hevc_vdenc_packet_xe_lpm_plus.cpp
//! \brief    Defines the interface for xe_lpm_plus hevc encode vdenc packet
//!
#include "encode_hevc_vdenc_packet_xe_lpm_plus.h"
#include "mos_solo_generic.h"
#include "encode_vdenc_lpla_analysis.h"

namespace encode
{
MOS_STATUS HevcVdencPktXe_Lpm_Plus::SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    // VDENC does not use batch buffer for slice state
    // add HCP_REF_IDX command
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_REF_IDX_STATE(&cmdBuffer));

    bool              vdencHucInUse    = false;
    PMHW_BATCH_BUFFER vdencBatchBuffer = nullptr;

    RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetVdencBatchBufferState, m_pipeline->m_currRecycledBufIdx, currSlcIdx, vdencBatchBuffer, vdencHucInUse);

    if (vdencHucInUse)
    {
        // 2nd level batch buffer
        PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = vdencBatchBuffer;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, secondLevelBatchBufferUsed)));
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(&cmdBuffer));
        secondLevelBatchBufferUsed->dwOffset = m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[currSlcIdx];
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, secondLevelBatchBufferUsed)));
    }
    else
    {
        // Weighted Prediction
        // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
        // If zero, then this command is not issued.
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_WEIGHTOFFSET_STATE(&cmdBuffer));

        m_basicFeature->m_useDefaultRoundingForHcpSliceState = false;
        SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &cmdBuffer);

        // add HCP_PAK_INSERT_OBJECTS command
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT(&cmdBuffer));

        SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &cmdBuffer);
    }
    SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, &cmdBuffer);
    return eStatus;
}

MOS_STATUS HevcVdencPktXe_Lpm_Plus::AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();
    params       = {};

    PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_nalUnitParams;

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    PBSBuffer pBsBuffer = &(m_basicFeature->m_bsBuffer);
    uint32_t  bitSize   = 0;
    uint32_t  offSet    = 0;

    if (cmdBuffer == nullptr)
    {
        ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    //insert AU, SPS, PSP headers before first slice header
    if (m_basicFeature->m_curNumSlices == 0)
    {
        uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for Length field in PAK_INSERT_OBJ cmd

        for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
        {
            uint32_t nalunitPosiSize   = ppNalUnitParams[i]->uiSize;
            uint32_t nalunitPosiOffset = ppNalUnitParams[i]->uiOffset;

            while (nalunitPosiSize > 0)
            {
                bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                offSet  = nalunitPosiOffset;

                params = {};

                params.dwPadding                 = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                params.bEmulationByteBitsInsert  = ppNalUnitParams[i]->bInsertEmulationBytes;
                params.uiSkipEmulationCheckCount = ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                params.dataBitsInLastDw          = bitSize % 32;
                if (params.dataBitsInLastDw == 0)
                {
                    params.dataBitsInLastDw = 32;
                }

                if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                {
                    nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                    nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                }
                else
                {
                    nalunitPosiSize = 0;
                }
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                uint32_t byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    MHW_MI_CHK_NULL(pBsBuffer);
                    MHW_MI_CHK_NULL(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
