/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_huc_copy_packet.cpp
//! \brief    Defines the interface for huc copy packet
//!
#include "decode_huc_copy_packet.h"
#include "mhw_vdbox.h"
#include "decode_pipeline.h"

namespace decode
{
MOS_STATUS HucCopyPkt::PushCopyParams(HucCopyParams &copyParams)
{
    DECODE_CHK_COND(copyParams.copyLength <= 0, "HucCopyPkt: Invalid copy params!");

    m_copyParamsList.push_back(copyParams);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucCopyPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(commandBuffer);

    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog    = false;

    if ((!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase) && (m_pipeline->GetPipeNum() == 1))
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }
    DECODE_CHK_STATUS(Execute(*commandBuffer, requestProlog));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucCopyPkt::Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_hucItf);

    SetPerfTag();

    for (m_copyParamsIdx = 0; m_copyParamsIdx < m_copyParamsList.size(); m_copyParamsIdx++)
    {
        if (prologNeeded && (m_copyParamsIdx == 0))
        {
            DECODE_CHK_STATUS(SendPrologCmds(cmdBuffer));
        }

        DECODE_CHK_STATUS(AddCmd_HUC_PIPE_MODE_SELECT(cmdBuffer));
        SETPAR_AND_ADDCMD(HUC_IND_OBJ_BASE_ADDR_STATE, m_hucItf, &cmdBuffer);
        DECODE_CHK_STATUS(AddHucIndState(cmdBuffer));
        SETPAR_AND_ADDCMD(HUC_STREAM_OBJECT, m_hucItf, &cmdBuffer);

        // Flush the engine to ensure memory written out
        DECODE_CHK_STATUS(MemoryFlush(cmdBuffer));

        DECODE_CHK_STATUS(m_allocator->SyncOnResource(m_copyParamsList[m_copyParamsIdx].srcBuffer, false));
        DECODE_CHK_STATUS(m_allocator->SyncOnResource(m_copyParamsList[m_copyParamsIdx].destBuffer, true));
    }

    // clear copy params since it is consumed
    m_copyParamsList.clear();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucCopyPkt::AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    //for gen 11+, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    auto &par = m_hucItf->MHW_GETPAR_F(HUC_PIPE_MODE_SELECT)();
    par                                     = {};
    par.mediaSoftResetCounterValue          = 2400;
    par.streamOutEnabled                    = true;
    par.disableProtectionSetting            = true;
    DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_PIPE_MODE_SELECT)(&cmdBuffer));

    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucCopyPkt::AddHucIndState(MOS_COMMAND_BUFFER &cmdBuffer)
{
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IND_OBJ_BASE_ADDR_STATE, HucCopyPkt)
{
    DECODE_FUNC_CALL();

    const HucCopyParams &copyParams = m_copyParamsList.at(m_copyParamsIdx);

    uint32_t dataSize            = copyParams.srcOffset + copyParams.copyLength;
    uint32_t dataOffset          = MOS_ALIGN_FLOOR(copyParams.srcOffset, MHW_PAGE_SIZE);
    uint32_t inputRelativeOffset = copyParams.srcOffset - dataOffset;

    uint32_t destSize             = copyParams.destOffset + copyParams.copyLength;
    uint32_t destOffset           = MOS_ALIGN_FLOOR(copyParams.destOffset, MHW_PAGE_SIZE);
    uint32_t outputRelativeOffset = copyParams.destOffset - destOffset;

    // Enlarge the stream in/out data size to avoid upper bound hit assert in HuC
    dataSize += inputRelativeOffset;
    destSize += outputRelativeOffset;

    // pass bit-stream buffer by Ind Obj Addr command
    params.DataBuffer            = copyParams.srcBuffer;
    params.DataSize              = MOS_ALIGN_CEIL(dataSize, MHW_PAGE_SIZE);
    params.DataOffset            = dataOffset;
    params.StreamOutObjectBuffer = copyParams.destBuffer;
    params.StreamOutObjectSize   = MOS_ALIGN_CEIL(destSize, MHW_PAGE_SIZE);
    params.StreamOutObjectOffset = destOffset;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_STREAM_OBJECT, HucCopyPkt)
{
    DECODE_FUNC_CALL();

    const HucCopyParams &copyParams = m_copyParamsList.at(m_copyParamsIdx);

    uint32_t dataOffset          = MOS_ALIGN_FLOOR(copyParams.srcOffset, MHW_PAGE_SIZE);
    uint32_t inputRelativeOffset = copyParams.srcOffset - dataOffset;

    uint32_t destOffset           = MOS_ALIGN_FLOOR(copyParams.destOffset, MHW_PAGE_SIZE);
    uint32_t outputRelativeOffset = copyParams.destOffset - destOffset;

    // set stream object with stream out enabled
    params.IndirectStreamInDataLength    = copyParams.copyLength;
    params.IndirectStreamInStartAddress  = inputRelativeOffset;
    params.IndirectStreamOutStartAddress = outputRelativeOffset;
    params.HucProcessing                 = true;
    params.HucBitstreamEnable            = true;
    params.StreamOut                     = true;

    return MOS_STATUS_SUCCESS;
}

void HucCopyPkt::SetPerfTag()
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((m_basicFeature->m_mode << 4) & 0xF0) | COPY_TYPE;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
}

MOS_STATUS HucCopyPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    uint32_t                       hucCommandsSize  = 0;
    uint32_t                       hucPatchListSize = 0;
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    if (m_hwInterface)
    {
        DECODE_CHK_STATUS(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));
    }

    commandBufferSize      = hucCommandsSize;
    requestedPatchListSize = m_osInterface->bUsesPatchList ? hucPatchListSize : 0;

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, 0x1000);

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
