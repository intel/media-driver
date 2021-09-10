/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_huc_copy_packet_g12.cpp
//! \brief    Defines the interface for huc copy packet
//!
#include "decode_huc_copy_packet_g12.h"
#include "mhw_vdbox.h"
#include "decode_pipeline.h"

namespace decode
{

MOS_STATUS HucCopyPktG12::PushCopyParams(HucCopyParams &copyParams)
{
    m_copyParamsList.push_back(copyParams);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucCopyPktG12::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(commandBuffer);

    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog = false;

    if ((!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase) && (m_pipeline->GetPipeNum() == 1))
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }
    DECODE_CHK_STATUS(Execute(*commandBuffer, requestProlog));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucCopyPktG12::Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_hucInterface);

    SetPerfTag();

    for (m_copyParamsIdx = 0; m_copyParamsIdx < m_copyParamsList.size(); m_copyParamsIdx++)
    {
        if (prologNeeded && (m_copyParamsIdx == 0))
        {
            DECODE_CHK_STATUS(SendPrologCmds(cmdBuffer));
        }

        DECODE_CHK_STATUS(AddHucPipeModeSelect(cmdBuffer));
        DECODE_CHK_STATUS(AddHucIndObj(cmdBuffer));
        CODEC_HEVC_SLICE_PARAMS unused;
        DECODE_CHK_STATUS(AddHucStreamObject(cmdBuffer, unused));

        // Flush the engine to ensure memory written out
        DECODE_CHK_STATUS(MemoryFlush(cmdBuffer));

        DECODE_CHK_STATUS(m_allocator->SyncOnResource(m_copyParamsList[m_copyParamsIdx].srcBuffer, false));
        DECODE_CHK_STATUS(m_allocator->SyncOnResource(m_copyParamsList[m_copyParamsIdx].destBuffer, true));
    }

    // clear copy params since it is consumed
    m_copyParamsList.clear();

    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetImemParameters(MHW_VDBOX_HUC_IMEM_STATE_PARAMS &imemParams)
{
    DECODE_FUNC_CALL();
}

MOS_STATUS HucCopyPktG12::AddHucImem(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetHucPipeModeSelectParameters(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &pipeModeSelectParams)
{
    DECODE_FUNC_CALL();
    pipeModeSelectParams.Mode = m_basicFeature->m_mode;
    pipeModeSelectParams.dwMediaSoftResetCounterValue = 2400;
    pipeModeSelectParams.bStreamObjectUsed = true;
    pipeModeSelectParams.bStreamOutEnabled = true;
    pipeModeSelectParams.disableProtectionSetting = true;
}

MOS_STATUS HucCopyPktG12::AddHucPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    SetHucPipeModeSelectParameters(pipeModeSelectParams);
    DECODE_CHK_STATUS(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));
    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetDmemParameters(MHW_VDBOX_HUC_DMEM_STATE_PARAMS &dmemParams)
{
    DECODE_FUNC_CALL();
}

MOS_STATUS HucCopyPktG12::AddHucDmem(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetRegionParameters(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS &virtualAddrParams)
{
    DECODE_FUNC_CALL();
}

MOS_STATUS HucCopyPktG12::AddHucRegion(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetIndObjParameters(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjParams)
{
    DECODE_FUNC_CALL();

    HucCopyParams& copyParams = m_copyParamsList.at(m_copyParamsIdx);

    uint32_t dataSize   = copyParams.srcOffset + copyParams.copyLength;
    uint32_t dataOffset = MOS_ALIGN_FLOOR(copyParams.srcOffset, MHW_PAGE_SIZE);
    uint32_t inputRelativeOffset  = copyParams.srcOffset - dataOffset;

    uint32_t destSize   = copyParams.destOffset + copyParams.copyLength;
    uint32_t destOffset = MOS_ALIGN_FLOOR(copyParams.destOffset, MHW_PAGE_SIZE);
    uint32_t outputRelativeOffset = copyParams.destOffset - destOffset;

    // Enlarge the stream in/out data size to avoid upper bound hit assert in HuC
    dataSize += inputRelativeOffset;
    destSize += outputRelativeOffset;

    // pass bit-stream buffer by Ind Obj Addr command
    indObjParams.presDataBuffer = copyParams.srcBuffer;
    indObjParams.dwDataSize = MOS_ALIGN_CEIL(dataSize, MHW_PAGE_SIZE);
    indObjParams.dwDataOffset = dataOffset;
    indObjParams.presStreamOutObjectBuffer = copyParams.destBuffer;
    indObjParams.dwStreamOutObjectSize = MOS_ALIGN_CEIL(destSize, MHW_PAGE_SIZE);
    indObjParams.dwStreamOutObjectOffset = destOffset;
}

MOS_STATUS HucCopyPktG12::AddHucIndObj(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjParams;
    MOS_ZeroMemory(&indObjParams, sizeof(indObjParams));
    SetIndObjParameters(indObjParams);
    DECODE_CHK_STATUS(m_hucInterface->AddHucIndObjBaseAddrStateCmd(&cmdBuffer, &indObjParams));
    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetStreamObjectParameters(MHW_VDBOX_HUC_STREAM_OBJ_PARAMS &streamObjParams,
                                           CODEC_HEVC_SLICE_PARAMS &sliceParams)
{
    DECODE_FUNC_CALL();

    HucCopyParams& copyParams = m_copyParamsList.at(m_copyParamsIdx);

    uint32_t dataOffset = MOS_ALIGN_FLOOR(copyParams.srcOffset, MHW_PAGE_SIZE);
    uint32_t inputRelativeOffset  = copyParams.srcOffset - dataOffset;

    uint32_t destOffset = MOS_ALIGN_FLOOR(copyParams.destOffset, MHW_PAGE_SIZE);
    uint32_t outputRelativeOffset = copyParams.destOffset - destOffset;

    // set stream object with stream out enabled
    streamObjParams.dwIndStreamInLength = copyParams.copyLength;
    streamObjParams.dwIndStreamInStartAddrOffset = inputRelativeOffset;
    streamObjParams.dwIndStreamOutStartAddrOffset = outputRelativeOffset;
    streamObjParams.bHucProcessing = true;
    streamObjParams.bStreamInEnable = true;
    streamObjParams.bStreamOutEnable = true;
}

MOS_STATUS HucCopyPktG12::AddHucStreamObject(MOS_COMMAND_BUFFER &cmdBuffer, CODEC_HEVC_SLICE_PARAMS &sliceParams)
{
    DECODE_FUNC_CALL();
    MHW_VDBOX_HUC_STREAM_OBJ_PARAMS streamObjParams;
    MOS_ZeroMemory(&streamObjParams, sizeof(streamObjParams));
    SetStreamObjectParameters(streamObjParams, sliceParams);
    DECODE_CHK_STATUS(m_hucInterface->AddHucStreamObjectCmd(&cmdBuffer, &streamObjParams));
    return MOS_STATUS_SUCCESS;
}

void HucCopyPktG12::SetPerfTag()
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((m_basicFeature->m_mode << 4) & 0xF0) | COPY_TYPE;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
}

MOS_STATUS HucCopyPktG12::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    uint32_t hucCommandsSize = 0;
    uint32_t hucPatchListSize = 0;
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    if (m_hwInterface)
    {
        DECODE_CHK_STATUS(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));
    }

    commandBufferSize = hucCommandsSize;
    requestedPatchListSize = m_osInterface->bUsesPatchList ? hucPatchListSize : 0;

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, 0x1000);

    return MOS_STATUS_SUCCESS;
}

}
