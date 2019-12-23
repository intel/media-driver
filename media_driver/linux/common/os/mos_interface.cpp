/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file     mos_interface.cpp
//! \brief    MOS interface implementation
//!

#include "mos_interface.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_os_specific_next.h"
#include "media_libva_common.h"
#include "mos_auxtable_mgr.h"
#include "mos_os_virtualengine_singlepipe_specific_next.h"
#include "mos_os_virtualengine_scalability_specific_next.h"
#include "mos_graphicsresource_specific_next.h"

MOS_STATUS MosInterface::CreateOsDeviceContext(DDI_DEVICE_CONTEXT ddiDeviceContext, MOS_DEVICE_HANDLE *deviceContext)
{
    MOS_OS_FUNCTION_ENTER;
    
    MOS_OS_CHK_NULL_RETURN(deviceContext);
    MOS_OS_CHK_NULL_RETURN(ddiDeviceContext);

    *deviceContext = MOS_New(OsContextSpecificNext);

    MOS_OS_CHK_NULL_RETURN(*deviceContext);
   
    MOS_OS_CHK_STATUS_RETURN((*deviceContext)->Init((PMOS_CONTEXT)ddiDeviceContext));
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsDeviceContext(MOS_DEVICE_HANDLE deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    if (deviceContext)
    {
        deviceContext->CleanUp();
        MosUtilities::MOS_Delete(deviceContext);
        deviceContext = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsStreamState(
    MOS_STREAM_HANDLE *streamState,
    MOS_DEVICE_HANDLE  deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(deviceContext);

    *streamState = MosUtilities::MOS_New(MosStreamState);
    (*streamState)->osDeviceContext = deviceContext;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyOsStreamState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    MosUtilities::MOS_Delete(streamState);
    streamState = nullptr;

    return MOS_STATUS_SUCCESS;
}

uint32_t MosInterface::GetInterfaceVersion(MOS_DEVICE_HANDLE deviceContext)
{
    MOS_OS_FUNCTION_ENTER;

    // No interface version to get in Linux

    return 0;
}

PLATFORM *MosInterface::GetPlatform(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetPlatformInfo();
    }

    return nullptr;
}

MEDIA_FEATURE_TABLE *MosInterface::GetSkuTable(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetSkuTable();
    }

    return nullptr;
}

MEDIA_WA_TABLE *MosInterface::GetWaTable(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetWaTable();
    }

    return nullptr;
}

MEDIA_SYSTEM_INFO *MosInterface::GetGtSystemInfo(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetGtSysInfo();
    }

    return nullptr;
}

ADAPTER_INFO *MosInterface::GetAdapterInfo(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    // No adapter Info in Linux

    return nullptr;
}

MOS_STATUS MosInterface::CreateGpuContext(
    MOS_STREAM_HANDLE             streamState,
    GpuContextCreateOption       &createOption,
    GPU_CONTEXT_HANDLE           &gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    auto osDeviceContext = streamState->osDeviceContext;

    auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);

    auto cmdBufMgr = osDeviceContext->GetCmdBufferMgr();
    MOS_OS_CHK_NULL_RETURN(cmdBufMgr);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osParameters);

    if (createOption.gpuNode == MOS_GPU_NODE_3D && createOption.SSEUValue != 0)
    {
        struct drm_i915_gem_context_param_sseu sseu;
        MosUtilities::MosZeroMemory(&sseu, sizeof(sseu));
        sseu.engine.engine_class    = I915_ENGINE_CLASS_RENDER;
        sseu.engine.engine_instance = 0;

        if (mos_get_context_param_sseu(osParameters->intel_context, &sseu))
        {
            MOS_OS_ASSERTMESSAGE("Failed to get sseu configuration.");
            return MOS_STATUS_UNKNOWN;
        };

        if (mos_hweight8(sseu.subslice_mask) > createOption.packed.SubSliceCount)
        {
            sseu.subslice_mask = mos_switch_off_n_bits(sseu.subslice_mask,
                mos_hweight8(sseu.subslice_mask) - createOption.packed.SubSliceCount);
        }

        if (mos_set_context_param_sseu(osParameters->intel_context, sseu))
        {
            MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
            return MOS_STATUS_UNKNOWN;
        };
    }

    MOS_GPU_NODE gpuNode = MOS_GPU_NODE_3D;
    gpuNode = static_cast<MOS_GPU_NODE>(createOption.gpuNode);

    auto gpuContext = gpuContextMgr->CreateGpuContext(gpuNode, cmdBufMgr);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    auto gpuContextSpecific = static_cast<GpuContextSpecificNext *>(gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextSpecific);

    MOS_OS_CHK_STATUS_RETURN(gpuContextSpecific->Init(gpuContextMgr->GetOsContext(), streamState, &createOption));

    gpuContextHandle = gpuContextSpecific->GetGpuContextHandle();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyGpuContext(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    MOS_OS_ASSERT(gpuContext != MOS_GPU_CONTEXT_INVALID_HANDLE);

    auto gpuContextMgr = streamState->osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
    auto gpuContextInstance = gpuContextMgr->GetGpuContext(gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextInstance);

    gpuContextMgr->DestroyGpuContext(gpuContextInstance);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetGpuContext(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContextMgr = streamState->osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
    auto gpuContextPtr = gpuContextMgr->GetGpuContext(gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextPtr);

    streamState->currentGpuContextHandle = gpuContext;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::AddCommand(
    COMMAND_BUFFER_HANDLE cmdBuffer,
    const void *          cmd,
    uint32_t              cmdSize)
{
    MOS_OS_FUNCTION_ENTER;

    uint32_t cmdSizeDwAligned = 0;

    MOS_OS_CHK_NULL_RETURN(cmdBuffer);
    MOS_OS_CHK_NULL_RETURN(cmd);

    if (cmdSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("Incorrect command size to add to command buffer.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    cmdSizeDwAligned = MOS_ALIGN_CEIL(cmdSize, sizeof(uint32_t));

    cmdBuffer->iOffset += cmdSizeDwAligned;
    cmdBuffer->iRemaining -= cmdSizeDwAligned;

    if (cmdBuffer->iRemaining < 0)
    {
        cmdBuffer->iOffset -= cmdSizeDwAligned;
        cmdBuffer->iRemaining += cmdSizeDwAligned;
        MOS_OS_ASSERTMESSAGE("Unable to add command (no space).");
        return MOS_STATUS_UNKNOWN;
    }

    MosUtilities::MosSecureMemcpy(cmdBuffer->pCmdPtr, cmdSize, cmd, cmdSize);
    cmdBuffer->pCmdPtr += (cmdSizeDwAligned / sizeof(uint32_t));

    return MOS_STATUS_SUCCESS;
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
MOS_STATUS MosInterface::DumpCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    static uint32_t dwCommandBufferNumber = 0;
    MOS_STATUS      eStatus               = MOS_STATUS_UNKNOWN;
    char *          pOutputBuffer         = nullptr;
    // Each hex value should have 9 chars.
    uint32_t SIZE_OF_ONE_WORD = 9;
    uint32_t dwBytesWritten   = 0;
    uint32_t dwNumberOfDwords = 0;
    uint32_t dwSizeToAllocate = 0;
    char     sFileName[MOS_MAX_HLT_FILENAME_LEN];
    // Maximum length of engine name is 6
    char sEngName[6];
    size_t nSizeFileNamePrefix   = 0;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    // Set the name of the engine that is going to be used.
    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);
    MOS_GPU_NODE gpuNode = gpuContext->GetContextNode();
    switch (gpuNode)
    {
    case MOS_GPU_NODE_VIDEO:
    case MOS_GPU_NODE_VIDEO2:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_VIDEO_ENGINE);
        break;
    case MOS_GPU_NODE_COMPUTE:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_RENDER_ENGINE);
        break;
    case MOS_GPU_NODE_VE:
        MosUtilities::MosSecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_VEBOX_ENGINE);
        break;
    default:
        MOS_OS_ASSERTMESSAGE("Unsupported GPU context.");
        return eStatus;
    }

    dwNumberOfDwords = cmdBuffer->iOffset / sizeof(uint32_t);

    dwSizeToAllocate =
        dwNumberOfDwords * (SIZE_OF_ONE_WORD + 1)  // Add 1 byte for the space following each Dword.
        + 3 * SIZE_OF_ONE_WORD;                    // For engine and platform names.

    // Alloc output buffer.
    pOutputBuffer = (char *)MOS_AllocAndZeroMemory(dwSizeToAllocate);
    MOS_OS_CHK_NULL_RETURN(pOutputBuffer);

    dwBytesWritten = MosUtilities::MosSecureStringPrint(
        pOutputBuffer,
        SIZE_OF_ONE_WORD * 3,
        SIZE_OF_ONE_WORD * 3,
        "Eng=%s ",
        sEngName);

    if (streamState->dumpCommandBufferToFile)
    {
        // Set the file name.
        eStatus = MOS_LogFileNamePrefix(sFileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create log file prefix. Status = %d", eStatus);
            return eStatus;
        }

        nSizeFileNamePrefix = strnlen(sFileName, sizeof(sFileName));
        MosUtilities::MosSecureStringPrint(
            sFileName + nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            "%c%s%c%s_%d.txt",
            MOS_DIR_SEPERATOR,
            MOS_COMMAND_BUFFER_OUT_DIR,
            MOS_DIR_SEPERATOR,
            MOS_COMMAND_BUFFER_OUT_FILE,
            dwCommandBufferNumber);

        // Write the output buffer to file.
        MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosWriteFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
    }

    if (streamState->dumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    MosUtilities::MosZeroMemory(pOutputBuffer, dwBytesWritten);
    dwBytesWritten = 0;

    // Fill in the output buffer with the command buffer dwords.
    for (uint32_t dwIndex = 0; dwIndex < dwNumberOfDwords; dwIndex++)
    {
        dwBytesWritten += MosUtilities::MosSecureStringPrint(
            pOutputBuffer + dwBytesWritten,
            SIZE_OF_ONE_WORD + 1,
            SIZE_OF_ONE_WORD + 1,
            "%.8x ",
            cmdBuffer->pCmdBase[dwIndex]);

        if (dwBytesWritten % (SIZE_OF_ONE_WORD + 1) == 0)
        {
            if (streamState->dumpCommandBufferToFile)
            {
                MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
            }
            if (streamState->dumpCommandBufferAsMessages)
            {
                MOS_OS_NORMALMESSAGE(pOutputBuffer);
            }

            MosUtilities::MosZeroMemory(pOutputBuffer, dwBytesWritten);
            dwBytesWritten = 0;
        }
    }

    if (streamState->dumpCommandBufferToFile)
    {
        MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
    }

    if (streamState->dumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    dwCommandBufferNumber++;

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;

}
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

MOS_STATUS MosInterface::GetCommandBuffer(
    MOS_STREAM_HANDLE      streamState,
    COMMAND_BUFFER_HANDLE &cmdBuffer,
    uint32_t               pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->GetCommandBuffer(cmdBuffer, pipeIdx));
}

MOS_STATUS MosInterface::ReturnCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    (gpuContext->ReturnCommandBuffer(cmdBuffer, pipeIdx));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SubmitCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    bool                  nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->SubmitCommandBuffer(streamState, cmdBuffer, nullRendering));
}

MOS_STATUS MosInterface::ResetCommandBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    // Reset the explicitly provided cmd buffer, or reset GPU context states
    if (cmdBuffer)
    {
        MOS_OS_CHK_STATUS_RETURN(gpuContext->ResetCommandBuffer());
    }
    else
    {
        gpuContext->ResetGpuContextStatus();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::VerifyCommandBufferSize(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              requestedSize,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->VerifyCommandBufferSize(requestedSize));
}

MOS_STATUS MosInterface::ResizeCommandBufferAndPatchList(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer,
    uint32_t              requestedSize,
    uint32_t              requestedPatchListSize,
    uint32_t              pipeIdx)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->ResizeCommandBufferAndPatchList(requestedSize, requestedPatchListSize, pipeIdx));
}

MOS_STATUS MosInterface::SetPatchEntry(
    MOS_STREAM_HANDLE       streamState,
    PMOS_PATCH_ENTRY_PARAMS params)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(params);
    MOS_OS_CHK_NULL_RETURN(streamState);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_OS_CHK_STATUS_RETURN(gpuContext->SetPatchEntry(streamState, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::GetIndirectState(
    MOS_STREAM_HANDLE streamState,
    uint8_t **indirectState,
    uint32_t &offset,
    uint32_t &size)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_OS_CHK_STATUS_RETURN(gpuContext->GetIndirectState(offset, size));
    if (indirectState)
    {
        MOS_OS_CHK_STATUS_RETURN(gpuContext->GetIndirectStatePointer(indirectState));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetupIndirectState(
    MOS_STREAM_HANDLE     streamState,
    uint32_t              size)
{
    MOS_OS_FUNCTION_ENTER;

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_OS_CHK_STATUS_RETURN(gpuContext->SetIndirectStateSize(size));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetupAttributeVeBuffer(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    // no VE attribute buffer to setup

    return MOS_STATUS_SUCCESS;
}

MOS_CMD_BUF_ATTRI_VE *MosInterface::GetAttributeVeBuffer(
        COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;

    // no VE attribute buffer to get
    return nullptr;
}

static GMM_RESOURCE_USAGE_TYPE GmmResourceUsage[MOS_HW_RESOURCE_DEF_MAX] =
{
    //
    // CODEC USAGES
    //
    GMM_RESOURCE_USAGE_BEGIN_CODEC,
    GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC,
    GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC_PARTIALENCSURFACE,
    GMM_RESOURCE_USAGE_POST_DEBLOCKING_CODEC,
    GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE,
    GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE,
    GMM_RESOURCE_USAGE_STREAMOUT_DATA_CODEC,
    GMM_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC,
    GMM_RESOURCE_USAGE_MACROBLOCK_STATUS_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE,
    GMM_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC,
    GMM_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE,
    GMM_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC,
    GMM_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_BITPLANE_READ_CODEC,
    GMM_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SURFACE_CURR_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_REF_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_FF,
    GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST,
    GMM_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_PAK_OBJECT_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MAD_ENCODE,
    GMM_RESOURCE_USAGE_VP8_BLOCK_MODE_COST_ENCODE,
    GMM_RESOURCE_USAGE_VP8_MB_MODE_COST_ENCODE,
    GMM_RESOURCE_USAGE_VP8_MBENC_OUTPUT_ENCODE,
    GMM_RESOURCE_USAGE_VP8_HISTOGRAM_ENCODE,
    GMM_RESOURCE_USAGE_VP8_L3_LLC_ENCODE,
    GMM_RESOURCE_USAGE_MFX_STANDALONE_DEBLOCKING_CODEC,
    GMM_RESOURCE_USAGE_HCP_MD_CODEC,
    GMM_RESOURCE_USAGE_HCP_SAO_CODEC,
    GMM_RESOURCE_USAGE_HCP_MV_CODEC,
    GMM_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC,
    GMM_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC,
    GMM_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC,
    GMM_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_VDENC_STREAMIN_CODEC,
    GMM_RESOURCE_USAGE_SURFACE_MB_QP_CODEC,
    GMM_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SSE_SRC_PIXEL_ROW_STORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SLICE_STATE_STREAM_OUT_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_CABAC_SYNTAX_STREAM_OUT_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_PRED_COL_STORE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MBENC_BRC_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MB_BRC_CONST_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_ROI_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_SLICE_MAP_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_VDENC_IMAGESTATE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_UNCACHED,
    GMM_RESOURCE_USAGE_SURFACE_ELLC_ONLY,
    GMM_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY,
    GMM_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3,
    GMM_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_SOFTWARE_SCOREBOARD_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_MV_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_4XME_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_INTRA_DISTORTION_ENCODE,
    GMM_RESOURCE_USAGE_MB_STATS_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_PAK_STATS_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_PIC_STATE_READ_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_PIC_STATE_WRITE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_COMBINED_ENC_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_BRC_CONSTANT_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_INTERMEDIATE_CU_RECORD_SURFACE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_SCRATCH_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_LCU_LEVEL_DATA_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_HISTORY_INPUT_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_HISTORY_OUTPUT_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_DEBUG_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_CONSTANT_TABLE_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_CU_RECORD_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_MV_TEMPORAL_BUFFER_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_CU_PACKET_FOR_PAK_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_BCOMBINED1_ENCODE,
    GMM_RESOURCE_USAGE_SURFACE_ENC_BCOMBINED2_ENCODE,
    GMM_RESOURCE_USAGE_FRAME_STATS_STREAMOUT_DATA_CODEC,
    GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_LINE_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_COLUMN_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_HCP_MD_TILE_LINE_CODEC,
    GMM_RESOURCE_USAGE_HCP_MD_TILE_COLUMN_CODEC,
    GMM_RESOURCE_USAGE_HCP_SAO_TILE_LINE_CODEC,
    GMM_RESOURCE_USAGE_HCP_SAO_TILE_COLUMN_CODEC,
    GMM_RESOURCE_USAGE_VP9_PROBABILITY_COUNTER_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_HUC_VIRTUAL_ADDR_REGION_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_SIZE_STREAMOUT_CODEC,
    GMM_RESOURCE_USAGE_COMPRESSED_HEADER_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_PROBABILITY_DELTA_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_TILE_RECORD_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_TILE_SIZE_STAS_BUFFER_CODEC,
    GMM_RESOURCE_USAGE_END_CODEC,

    //
    // CM USAGES
    //
    CM_RESOURCE_USAGE_SurfaceState,
    CM_RESOURCE_USAGE_StateHeap,
    CM_RESOURCE_USAGE_NO_L3_SurfaceState,
    CM_RESOURCE_USAGE_NO_LLC_ELLC_SurfaceState,
    CM_RESOURCE_USAGE_NO_LLC_SurfaceState,
    CM_RESOURCE_USAGE_NO_ELLC_SurfaceState,
    CM_RESOURCE_USAGE_NO_LLC_L3_SurfaceState,
    CM_RESOURCE_USAGE_NO_ELLC_L3_SurfaceState,
    CM_RESOURCE_USAGE_NO_CACHE_SurfaceState,
    CM_RESOURCE_USAGE_L1_Enabled_SurfaceState,

    //
    // MP USAGES
    //
    MP_RESOURCE_USAGE_BEGIN,
    MP_RESOURCE_USAGE_DEFAULT,
    MP_RESOURCE_USAGE_DEFAULT_FF,
    MP_RESOURCE_USAGE_DEFAULT_RCS,
    MP_RESOURCE_USAGE_SurfaceState,
    MP_RESOURCE_USAGE_SurfaceState_FF,
    MP_RESOURCE_USAGE_SurfaceState_RCS,
    MP_RESOURCE_USAGE_AGE3_SurfaceState,
    MP_RESOURCE_USAGE_EDRAM_SurfaceState,
    MP_RESOURCE_USAGE_EDRAM_AGE3_SurfaceState,
    MP_RESOURCE_USAGE_No_L3_SurfaceState,
    MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState,
    MP_RESOURCE_USAGE_No_LLC_L3_AGE_SurfaceState,
    MP_RESOURCE_USAGE_No_LLC_eLLC_L3_AGE_SurfaceState,
    MP_RESOURCE_USAGE_PartialEnc_No_LLC_L3_AGE_SurfaceState,
    MP_RESOURCE_USAGE_END,

    // MHW - SFC
    MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,                    //!< SFC output surface
    MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface_PartialEncSurface,  //!< SFC output surface for partial secure surfaces
    MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,                    //!< SFC AVS Line buffer Surface
    MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,                    //!< SFC IEF Line buffer Surface

};

MEMORY_OBJECT_CONTROL_STATE MosInterface::GetCachePolicyMemoryObject(
    MOS_STREAM_HANDLE   streamState,
    MOS_HW_RESOURCE_DEF mosUsage)
{
    MOS_OS_FUNCTION_ENTER;

    //auto gmmClientContext = MosInterface::GetGmmClientContext(streamState);
    // Force convert to stream handle for wrapper
    auto gmmClientContext = (GMM_CLIENT_CONTEXT *)streamState;
    MOS_OS_ASSERT(gmmClientContext);

    GMM_RESOURCE_USAGE_TYPE usage = GmmResourceUsage[mosUsage];
    if (gmmClientContext->GetCachePolicyElement(usage).Initialized)
    {
        return gmmClientContext->CachePolicyGetMemoryObject(nullptr, usage);
    }
    else
    {
        return gmmClientContext->GetCachePolicyUsage()[GMM_RESOURCE_USAGE_UNKNOWN].MemoryObjectOverride;
    }

    return {0};
}

uint8_t MosInterface::GetCachePolicyL1Config(
    MOS_STREAM_HANDLE streamState,
    MOS_HW_RESOURCE_DEF mosUsage)
{
    MOS_OS_FUNCTION_ENTER;
    return 0;
}

MOS_STATUS MosInterface::ConvertResourceFromDdi(
    OsSpecificRes osResource,
    MOS_RESOURCE_HANDLE &resource,
    uint32_t firstArraySlice,
    uint32_t mipSlice)
{
    MOS_OS_FUNCTION_ENTER;

    if (firstArraySlice == OS_SPECIFIC_RESOURCE_INVALID || firstArraySlice >= OS_SPECIFIC_RESOURCE_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Cannot Convert Resource From Ddi, invalid ddi resource type!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_OS_CHK_NULL_RETURN(osResource);
    MOS_OS_CHK_NULL_RETURN(resource);

    if (firstArraySlice == OS_SPECIFIC_RESOURCE_SURFACE)
    {
        DDI_MEDIA_SURFACE *mediaSurface = (DDI_MEDIA_SURFACE *)osResource;

        switch (mediaSurface->format)
        {
        case Media_Format_NV12:
            resource->Format = Format_NV12;
            break;
        case Media_Format_NV21:
            resource->Format = Format_NV21;
            break;
        case Media_Format_YUY2:
            resource->Format = Format_YUY2;
            break;
        case Media_Format_X8R8G8B8:
            resource->Format = Format_X8R8G8B8;
            break;
        case Media_Format_X8B8G8R8:
            resource->Format = Format_X8B8G8R8;
            break;
        case Media_Format_A8B8G8R8:
        case Media_Format_R8G8B8A8:
            resource->Format = Format_A8B8G8R8;
            break;
        case Media_Format_A8R8G8B8:
            resource->Format = Format_A8R8G8B8;
            break;
        case Media_Format_R5G6B5:
            resource->Format = Format_R5G6B5;
            break;
        case Media_Format_R8G8B8:
            resource->Format = Format_R8G8B8;
            break;
        case Media_Format_444P:
            resource->Format = Format_444P;
            break;
        case Media_Format_411P:
            resource->Format = Format_411P;
            break;
        case Media_Format_IMC3:
            resource->Format = Format_IMC3;
            break;
        case Media_Format_400P:
            resource->Format = Format_400P;
            break;
        case Media_Format_422H:
            resource->Format = Format_422H;
            break;
        case Media_Format_422V:
            resource->Format = Format_422V;
            break;
        case Media_Format_Buffer:
            resource->Format = Format_Any;
        case Media_Format_P010:
            resource->Format = Format_P010;
            break;
        case Media_Format_P016:
            resource->Format = Format_P016;
            break;
        case Media_Format_Y210:
            resource->Format = Format_Y210;
            break;
        case Media_Format_Y216:
            resource->Format = Format_Y216;
            break;
        case Media_Format_AYUV:
            resource->Format = Format_AYUV;
            break;
        case Media_Format_Y410:
            resource->Format = Format_Y410;
            break;
        case Media_Format_Y416:
            resource->Format = Format_Y416;
            break;
        case Media_Format_Y8:
            resource->Format = Format_Y8;
            break;
        case Media_Format_Y16S:
            resource->Format = Format_Y16S;
            break;
        case Media_Format_Y16U:
            resource->Format = Format_Y16U;
            break;
        case Media_Format_R10G10B10A2:
            resource->Format = Format_R10G10B10A2;
            break;
        case Media_Format_B10G10R10A2:
            resource->Format = Format_B10G10R10A2;
            break;
        case Media_Format_UYVY:
            resource->Format = Format_UYVY;
            break;
        case Media_Format_VYUY:
            resource->Format = Format_VYUY;
            break;
        case Media_Format_YVYU:
            resource->Format = Format_YVYU;
            break;
        case Media_Format_A16R16G16B16:
            resource->Format = Format_A16R16G16B16;
            break;
        case Media_Format_A16B16G16R16:
            resource->Format = Format_A16B16G16R16;
            break;
        default:
            MOS_OS_ASSERTMESSAGE("MOS: unsupported media format for surface.");
            break;
        }
        resource->iWidth   = mediaSurface->iWidth;
        resource->iHeight  = mediaSurface->iHeight;
        resource->iPitch   = mediaSurface->iPitch;
        resource->iCount   = mediaSurface->iRefCount;
        resource->isTiled  = mediaSurface->isTiled;
        resource->TileType = LinuxToMosTileType(mediaSurface->TileType);
        resource->bo       = mediaSurface->bo;
        resource->name     = mediaSurface->name;

        resource->ppCurrentFrameSemaphore   = &mediaSurface->pCurrentFrameSemaphore;
        resource->ppReferenceFrameSemaphore = &mediaSurface->pReferenceFrameSemaphore;
        resource->bSemInitialized           = false;
        resource->bMapped                   = false;

        if (mediaSurface->bMapped == true)
        {
            resource->pData = mediaSurface->pData;
        }
        else
        {
            resource->pData = nullptr;
        }
        resource->pGmmResInfo  = mediaSurface->pGmmResourceInfo;
        resource->dwGfxAddress = 0;
    }
    else if (firstArraySlice == OS_SPECIFIC_RESOURCE_BUFFER)
    {
        DDI_MEDIA_BUFFER *mediaBuffer = (DDI_MEDIA_BUFFER *)osResource;
        switch (mediaBuffer->format)
        {
        case Media_Format_Buffer:
            resource->Format  = Format_Buffer;
            resource->iWidth  = mediaBuffer->iSize;
            resource->iHeight = 1;
            resource->iPitch  = mediaBuffer->iSize;
            break;
        case Media_Format_Perf_Buffer:
            resource->Format  = Format_Buffer;
            resource->iWidth  = mediaBuffer->iSize;
            resource->iHeight = 1;
            resource->iPitch  = mediaBuffer->iSize;
            break;
        case Media_Format_2DBuffer:
            resource->Format  = Format_Buffer_2D;
            resource->iWidth  = mediaBuffer->uiWidth;
            resource->iHeight = mediaBuffer->uiHeight;
            resource->iPitch  = mediaBuffer->uiPitch;
            break;
        case Media_Format_CPU:
            return MOS_STATUS_SUCCESS;
        default:
            resource->iWidth  = mediaBuffer->iSize;
            resource->iHeight = 1;
            resource->iPitch  = mediaBuffer->iSize;
            MOS_OS_ASSERTMESSAGE("MOS: unsupported media format for surface.");
            break;
        }
        resource->iCount   = mediaBuffer->iRefCount;
        resource->isTiled  = 0;
        resource->TileType = LinuxToMosTileType(mediaBuffer->TileType);
        resource->bo       = mediaBuffer->bo;
        resource->name     = mediaBuffer->name;
        resource->bMapped  = false;

        if (mediaBuffer->bMapped == true)
        {
            resource->pData = mediaBuffer->pData;
        }
        else
        {
            resource->pData = nullptr;
        }
        resource->dwGfxAddress = 0;
        resource->pGmmResInfo  = mediaBuffer->pGmmResourceInfo;
    }

    resource->bConvertedFromDDIResource = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateOsSpecificResourceInfo(OsSpecificRes resource, bool isInternal)
{
    MOS_OS_FUNCTION_ENTER;

    // No OsSpecificResourceInfo in Linux

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroySpecificResourceInfo(OsSpecificRes resource)
{
    MOS_OS_FUNCTION_ENTER;

    // No OsSpecificResourceInfo in Linux

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::AllocateResource(
    MOS_STREAM_HANDLE        streamState,
    PMOS_ALLOC_GFXRES_PARAMS params,
    MOS_RESOURCE_HANDLE     &resource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    resource->bConvertedFromDDIResource = false;
    if (!params->bBypassMODImpl)
    {
        resource->pGfxResourceNext = GraphicsResourceNext::CreateGraphicResource(GraphicsResourceNext::osSpecificResource);
        MOS_OS_CHK_NULL_RETURN(resource->pGfxResourceNext);

        GraphicsResourceNext::CreateParams createParams(params);
        auto eStatus = resource->pGfxResourceNext->Allocate(streamState->osDeviceContext, createParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Allocate graphic resource failed");
            return MOS_STATUS_INVALID_HANDLE;
        }
        
        eStatus = resource->pGfxResourceNext->ConvertToMosResource(resource);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Convert graphic resource failed");
            return MOS_STATUS_INVALID_HANDLE;
        }

        return eStatus;
    }

    return GraphicsResourceSpecificNext::AllocateExternalResource(streamState, params, resource);
}

MOS_STATUS MosInterface::FreeResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t            flag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    if (!resource->bConvertedFromDDIResource && resource->pGfxResourceNext)
    {
        if (resource && resource->pGfxResourceNext)
        {
            resource->pGfxResourceNext->Free(streamState->osDeviceContext);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Received an empty Graphics Resource, skip free");
        }
        MOS_Delete(resource->pGfxResourceNext);
        resource->pGfxResourceNext = nullptr;

        return MOS_STATUS_SUCCESS;
    }

    return GraphicsResourceSpecificNext::FreeExternalResource(streamState, resource, flag);
}

MOS_STATUS MosInterface::GetResourceInfo(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MosResourceInfo     &details)  //MOS_SURFACE
{
    MOS_OS_FUNCTION_ENTER;

    GMM_RESOURCE_INFO * gmmResourceInfo = nullptr;
    GMM_DISPLAY_FRAME   gmmChannel = GMM_DISPLAY_FRAME_MAX;
    GMM_REQ_OFFSET_INFO reqInfo[3] = {};
    GMM_RESOURCE_FLAG   gmmFlags = {};
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);

    // Get Gmm resource info
    gmmResourceInfo = (GMM_RESOURCE_INFO*)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(gmmResourceInfo);

    gmmFlags = gmmResourceInfo->GetResFlags();

    // Get resource information
    if (resource->b16UsrPtrMode)
    {
        // if usrptr surface, do not query those values from gmm, app will configure them.
        details.dwWidth         = resource->iWidth;
        details.dwHeight        = resource->iHeight;
        details.dwPitch         = resource->iPitch;
    }
    else
    {
        details.dwWidth         = GFX_ULONG_CAST(gmmResourceInfo->GetBaseWidth());
        details.dwHeight        = gmmResourceInfo->GetBaseHeight();
        details.dwPitch         = GFX_ULONG_CAST(gmmResourceInfo->GetRenderPitch());
    }
    details.dwDepth         = MOS_MAX(1, gmmResourceInfo->GetBaseDepth());
    details.dwLockPitch     = GFX_ULONG_CAST(gmmResourceInfo->GetRenderPitch());

    details.dwQPitch = gmmResourceInfo->GetQPitch();

    details.bCompressible   = gmmFlags.Gpu.MMC ?
        (gmmResourceInfo->GetMmcHint(0) == GMM_MMC_HINT_ON) : false;
    details.bIsCompressed   = gmmResourceInfo->IsMediaMemoryCompressed(0);
    details.CompressionMode = (MOS_RESOURCE_MMC_MODE)gmmResourceInfo->GetMmcMode(0);

    if (0 == details.dwPitch)
    {
        MOS_OS_ASSERTMESSAGE("Pitch from GmmResource is 0, unexpected.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    // check resource's tile type
    details.TileModeGMM     = (MOS_TILE_MODE_GMM)gmmResourceInfo->GetTileModeSurfaceState();
    details.bGMMTileEnabled = true;
    switch (gmmResourceInfo->GetTileType())
    {
    case GMM_TILED_Y:
          if (gmmFlags.Info.TiledYf)
          {
              details.TileType = MOS_TILE_YF;
          }
          else if (gmmFlags.Info.TiledYs)
          {
              details.TileType = MOS_TILE_YS;
          }
          else
          {
              details.TileType = MOS_TILE_Y;
          }
          break;
    case GMM_TILED_X:
          details.TileType = MOS_TILE_X;
          break;
    case GMM_NOT_TILED:
          details.TileType = MOS_TILE_LINEAR;
          break;
    default:
          details.TileType = MOS_TILE_Y;
          break;
    }
    details.Format   = resource->Format;

    // Get planes
    if (resource->b16UsrPtrMode)
    {
        // if usrptr surface, do not query those values from gmm, app will configure them.
        details.RenderOffset.YUV.Y.BaseOffset = resource->YPlaneOffset.iSurfaceOffset;
        details.RenderOffset.YUV.U.BaseOffset = resource->UPlaneOffset.iSurfaceOffset;
        details.RenderOffset.YUV.U.XOffset    = resource->UPlaneOffset.iXOffset;
        details.RenderOffset.YUV.U.YOffset    = resource->UPlaneOffset.iYOffset;
        details.RenderOffset.YUV.V.BaseOffset = resource->VPlaneOffset.iSurfaceOffset;
        details.RenderOffset.YUV.V.XOffset    = resource->VPlaneOffset.iXOffset;
        details.RenderOffset.YUV.V.YOffset    = resource->VPlaneOffset.iYOffset;
    }
    else
    {
        MosUtilities::MosZeroMemory(reqInfo, sizeof(reqInfo));
        gmmChannel = GMM_DISPLAY_BASE;
        // Get the base offset of the surface (plane Y)
        reqInfo[2].ReqRender = true;
        reqInfo[2].Plane     = GMM_PLANE_Y;
        reqInfo[2].Frame     = gmmChannel;
        reqInfo[2].CubeFace  = __GMM_NO_CUBE_MAP;
        reqInfo[2].ArrayIndex = 0;
        gmmResourceInfo->GetOffset(reqInfo[2]);
        details.RenderOffset.YUV.Y.BaseOffset = reqInfo[2].Render.Offset;

        // Get U/UV plane information (plane offset, X/Y offset)
        reqInfo[0].ReqRender = true;
        reqInfo[0].Plane     = GMM_PLANE_U;
        reqInfo[0].Frame     = gmmChannel;
        reqInfo[0].CubeFace  = __GMM_NO_CUBE_MAP;
        reqInfo[0].ArrayIndex = 0;
        gmmResourceInfo->GetOffset(reqInfo[0]);

        details.RenderOffset.YUV.U.BaseOffset = reqInfo[0].Render.Offset;
        details.RenderOffset.YUV.U.XOffset    = reqInfo[0].Render.XOffset;
        details.RenderOffset.YUV.U.YOffset    = reqInfo[0].Render.YOffset;

        // Get V plane information (plane offset, X/Y offset)
        reqInfo[1].ReqRender = true;
        reqInfo[1].Plane     = GMM_PLANE_V;
        reqInfo[1].Frame     = gmmChannel;
        reqInfo[1].CubeFace  = __GMM_NO_CUBE_MAP;
        reqInfo[1].ArrayIndex = 0;
        gmmResourceInfo->GetOffset(reqInfo[1]);

        details.RenderOffset.YUV.V.BaseOffset = reqInfo[1].Render.Offset;
        details.RenderOffset.YUV.V.XOffset    = reqInfo[1].Render.XOffset;
        details.RenderOffset.YUV.V.YOffset    = reqInfo[1].Render.YOffset;
    }

    return eStatus;
}

void *MosInterface::LockMosResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    PMOS_LOCK_PARAMS    flags)
{
    MOS_OS_FUNCTION_ENTER;

    void *pData    = nullptr;

    if (nullptr == streamState)
    {
        MOS_OS_ASSERTMESSAGE("input parameter streamState is NULL.");
        return nullptr;
    }

    if (nullptr == resource)
    {
        MOS_OS_ASSERTMESSAGE("input parameter resource is NULL.");
        return nullptr;
    }

    if ((!resource->bConvertedFromDDIResource) && (resource->pGfxResourceNext))
    {
        if (nullptr == streamState->osDeviceContext)
        {
            MOS_OS_ASSERTMESSAGE("invalid osDeviceContext, skip lock");
            return nullptr;
        }

        if (resource->pGfxResourceNext)
        {
            GraphicsResourceNext::LockParams params(flags);
            pData = resource->pGfxResourceNext->Lock(streamState->osDeviceContext, params);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Received an empty Graphics Resource, skip lock");
            return nullptr;
        }
        return pData;
    }

    pData = GraphicsResourceSpecificNext::LockExternalResource(streamState, resource, flags);
    return pData;
}

MOS_STATUS MosInterface::UnlockMosResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    if ((!resource->bConvertedFromDDIResource) && (resource->pGfxResourceNext))
    {
        if (resource->pGfxResourceNext)
        {
            eStatus = resource->pGfxResourceNext->Unlock(streamState->osDeviceContext);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Received an empty Graphics Resource, skip unlock");
        }
        return eStatus;
    }

    eStatus = GraphicsResourceSpecificNext::UnlockExternalResource(streamState, resource);

    return eStatus;
}

MOS_STATUS MosInterface::RegisterResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    bool                write)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    auto gpuContext = MosInterface::GetGpuContext(streamState, streamState->currentGpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    return (gpuContext->RegisterResource(resource, write));
}

uint64_t MosInterface::GetResourceGfxAddress(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;
    
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);

    if (!mos_gem_bo_is_softpin(resource->bo))
    {
        mos_bo_set_softpin(resource->bo);
    }
    return resource->bo->offset64;
}

uint32_t MosInterface::GetResourceAllocationIndex(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

MOS_STATUS MosInterface::SkipResourceSync(
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    // No resource sync to skip 

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SyncOnResource(
    MOS_STREAM_HANDLE streamState,
    MOS_RESOURCE_HANDLE resource,
    bool writeOperation,
    GPU_CONTEXT_HANDLE requsetorGpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do sync on resource

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::ResourceSync(
    OsSpecificRes       resource,
    MOS_DEVICE_HANDLE   deviceContext,
    uint32_t            index,
    SYNC_HAZARD         hazardType,
    GPU_CONTEXT_HANDLE  busyCtx,
    GPU_CONTEXT_HANDLE  requestorCtx,
    OS_HANDLE           osHandle)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do resource sync

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::LockSync(
    OsSpecificRes       resource,
    MOS_DEVICE_HANDLE   deviceContext,
    uint32_t            index,
    SYNC_HAZARD         hazardType,
    GPU_CONTEXT_HANDLE  busyCtx,
    bool                doNotWait)
{
    MOS_OS_FUNCTION_ENTER;

    // No need to do Lock sync

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::TrimResidency(
    bool      periodicTrim,
    bool      restartPeriodicTrim,
    uint64_t &numBytesToTrim,
    bool      trimToMinimum,
    bool      trimOnlyMediaResources)
{
    MOS_OS_FUNCTION_ENTER;

    // No residency to trim

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DecompResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetMemoryCompressionMode(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MOS_MEMCOMP_STATE   resMmcMode)
{
    MOS_OS_FUNCTION_ENTER;

    PGMM_RESOURCE_INFO pGmmResourceInfo = nullptr;
    GMM_RESOURCE_MMC_INFO GmmResMmcMode = GMM_MMC_DISABLED;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    MOS_OS_CHK_NULL_RETURN(resource);

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);

    switch (resMmcMode)
    {
    case MOS_MEMCOMP_HORIZONTAL:
        GmmResMmcMode = GMM_MMC_HORIZONTAL;
        break;
    case MOS_MEMCOMP_VERTICAL:
        GmmResMmcMode = GMM_MMC_VERTICAL;
        break;
    case MOS_MEMCOMP_DISABLED:
    default:
        GmmResMmcMode = GMM_MMC_DISABLED;
        break;
    }

    pGmmResourceInfo->SetMmcMode(GmmResMmcMode, 0);

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::GetMemoryCompressionMode(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    MOS_MEMCOMP_STATE  &resMmcMode)
{
    MOS_OS_FUNCTION_ENTER;

    PGMM_RESOURCE_INFO gmmResourceInfo = nullptr;
    GMM_RESOURCE_FLAG  flags;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    MOS_OS_CHK_NULL_RETURN(resource);
    MosUtilities::MosZeroMemory(&flags, sizeof(GMM_RESOURCE_FLAG));

    // Get Gmm resource info
    gmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(gmmResourceInfo);

    flags = resource->pGmmResInfo->GetResFlags();
    if (flags.Info.MediaCompressed || flags.Info.RenderCompressed)
    {
        resMmcMode = flags.Info.RenderCompressed ? MOS_MEMCOMP_RC : MOS_MEMCOMP_MC;
    }
    else
    {
        switch (gmmResourceInfo->GetMmcMode(0))
        {
        case GMM_MMC_HORIZONTAL:
            resMmcMode = MOS_MEMCOMP_HORIZONTAL;
            break;
        case GMM_MMC_VERTICAL:
            resMmcMode = MOS_MEMCOMP_VERTICAL;
            break;
        case GMM_MMC_DISABLED:
        default:
            resMmcMode = MOS_MEMCOMP_DISABLED;
            break;
        }
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::SetMemoryCompressionHint(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    bool                hintOn)
{
    MOS_OS_FUNCTION_ENTER;

    PGMM_RESOURCE_INFO pGmmResourceInfo = nullptr;
    uint32_t uiArrayIndex = 0;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CHK_NULL_RETURN(resource);

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);

    pGmmResourceInfo->SetMmcHint(hintOn ? GMM_MMC_HINT_ON : GMM_MMC_HINT_OFF, uiArrayIndex);

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS MosInterface::GetMemoryCompressionFormat(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t *          resMmcFormat)
{
    MOS_OS_FUNCTION_ENTER;
    
    MOS_STATUS         eStatus = MOS_STATUS_UNKNOWN;
    PGMM_RESOURCE_INFO pGmmResourceInfo;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(resMmcFormat);

    pGmmResourceInfo = (GMM_RESOURCE_INFO *)resource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);
    MOS_OS_CHK_NULL_RETURN(MosInterface::GetGmmClientContext(streamState));
    // Get compression format from GMM RESOURCE FORMAT
    GMM_RESOURCE_FORMAT gmmResFmt;
    gmmResFmt = pGmmResourceInfo->GetResourceFormat();
    MOS_MEMCOMP_STATE MmcMode   = MOS_MEMCOMP_DISABLED;
    uint32_t          MmcFormat = 0;
    MosInterface::GetMemoryCompressionMode(streamState, resource, MmcMode);
    switch (MmcMode)
    {
    case MOS_MEMCOMP_MC:
         MmcFormat = static_cast<uint32_t>(MosInterface::GetGmmClientContext(streamState)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));
         break;
    case MOS_MEMCOMP_RC:
         MmcFormat = static_cast<uint32_t>(MosInterface::GetGmmClientContext(streamState)->GetSurfaceStateCompressionFormat(gmmResFmt));
         break;
    default:
          MmcFormat = 0;
    }

    if (MmcFormat > 0x1F)
    {
        MOS_OS_ASSERTMESSAGE("Get a incorrect Compression format(%d) from GMM", MmcFormat); 
    }
    else
    {
        *resMmcFormat = MmcFormat;
        MOS_OS_VERBOSEMESSAGE("GMM compression mode %d, compression format %d", MmcMode, MmcFormat);
    }

    eStatus = MOS_STATUS_SUCCESS;
    
    return eStatus;
}

MOS_STATUS MosInterface::DoubleBufferCopyResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    bool                outputCompressed)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(inputResource);
    MOS_OS_CHK_NULL_RETURN(outputResource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    
    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osParameters);

    if (inputResource && inputResource->bo && inputResource->pGmmResInfo &&
        outputResource && outputResource->bo && outputResource->pGmmResInfo)
    {
        // Double Buffer Copy can support any tile status surface with/without compression
        osParameters->pfnMediaMemoryCopy(osParameters, inputResource, outputResource, outputCompressed);
    }

    return status;
}

MOS_STATUS MosInterface::MediaCopyResource2D(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE inputResource,
    MOS_RESOURCE_HANDLE outputResource,
    uint32_t            copyWidth,
    uint32_t            copyHeight,
    uint32_t            copyInputOffset,
    uint32_t            copyOutputOffset,
    bool                outputCompressed)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(inputResource);
    MOS_OS_CHK_NULL_RETURN(outputResource);
    MOS_OS_CHK_NULL_RETURN(streamState);

    auto osParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osParameters);
    
    if (inputResource && inputResource->bo && inputResource->pGmmResInfo &&
        outputResource && outputResource->bo && outputResource->pGmmResInfo)
    {
        // Double Buffer Copy can support any tile status surface with/without compression
        osParameters->pfnMediaMemoryCopy2D(osParameters, inputResource, outputResource,
            copyWidth, copyHeight, copyInputOffset, copyOutputOffset, outputCompressed);
    }

    return status;
}

uint32_t MosInterface::GetGpuStatusTag(
        MOS_STREAM_HANDLE  streamState,
        GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState)
    {
        auto gpuContextIns = MosInterface::GetGpuContext(streamState, gpuContext);
        if (gpuContextIns == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Get GPU Status Tag failed.");
            return 0;
        }

        return gpuContextIns->GetGpuStatusTag();
    }
    MOS_OS_ASSERTMESSAGE("Get GPU Status Tag failed.");
    
    return 0;
}

MOS_STATUS MosInterface::IncrementGpuStatusTag(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    auto gpuContextIns = MosInterface::GetGpuContext(streamState, gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextIns);

    gpuContextIns->IncrementGpuStatusTag();
    
    return MOS_STATUS_SUCCESS;
}

uint64_t MosInterface::GetGpuStatusSyncTag(
    MOS_STREAM_HANDLE  streamState,
    GPU_CONTEXT_HANDLE gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    // No Gpu Status Sync Tag in Linux

    return 0;
}

MOS_STATUS MosInterface::GetGpuStatusBufferResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    GPU_CONTEXT_HANDLE  gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(resource);

    auto gpuContextIns = MosInterface::GetGpuContext(streamState, gpuContext);
    MOS_OS_CHK_NULL_RETURN(gpuContextIns);

    auto gfxResource = gpuContextIns->GetStatusBufferResource();
    MOS_OS_CHK_NULL_RETURN(gfxResource);

    MOS_OS_CHK_STATUS_RETURN(gfxResource->ConvertToMosResource(resource));

    return MOS_STATUS_SUCCESS;
}

GMM_CLIENT_CONTEXT *MosInterface::GetGmmClientContext(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        return streamState->osDeviceContext->GetGmmClientContext();
    }

    return nullptr;
}

MosCpInterface *MosInterface::GetCpInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return streamState ? streamState->osCpInterface : nullptr;
}

MosOcaInterface *MosInterface::GetOcaInterface(MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return nullptr;
}

MOS_VE_HANDLE MosInterface::GetVirtualEngineState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    return streamState ? streamState->virtualEngineInterface : nullptr;
}

MOS_STATUS MosInterface::SetVirtualEngineState(
    MOS_STREAM_HANDLE streamState,
    MOS_VE_HANDLE veState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(veState);

    streamState->virtualEngineInterface = veState;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::CreateVirtualEngineState(
    MOS_STREAM_HANDLE streamState,
    PMOS_VIRTUALENGINE_INIT_PARAMS veInitParms,
    MOS_VE_HANDLE& veState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);

    if (veInitParms->bScalabilitySupported)
    {
        streamState->virtualEngineInterface = MosUtilities::MOS_New(MosOsVeScalabilitySpecific);
    }
    else
    {
        streamState->virtualEngineInterface = MosUtilities::MOS_New(MosOsVeSinglePipeSpecific);
    }
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    MOS_OS_CHK_STATUS_RETURN(streamState->virtualEngineInterface->Initialize(streamState, veInitParms));

    veState = streamState->virtualEngineInterface;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::DestroyVirtualEngineState(
    MOS_STREAM_HANDLE streamState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    streamState->virtualEngineInterface->Destroy();
    MosUtilities::MOS_Delete(streamState->virtualEngineInterface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::SetVeHintParams(
    MOS_STREAM_HANDLE             streamState,
    PMOS_VIRTUALENGINE_SET_PARAMS veParams)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    return streamState->virtualEngineInterface->SetHintParams(veParams);
}

MOS_STATUS MosInterface::GetVeHintParams(
    MOS_STREAM_HANDLE               streamState,
    bool                            scalableMode,
    PMOS_VIRTUALENGINE_HINT_PARAMS* hintParams)
{
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);
    MOS_OS_CHK_NULL_RETURN(hintParams);

    return streamState->virtualEngineInterface->GetHintParams(scalableMode, hintParams);
}

MOS_STATUS MosInterface::SetVeSubmissionType(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuf,
    MOS_SUBMISSION_TYPE   type)
{
    MOS_OS_CHK_NULL_RETURN(cmdBuf);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->virtualEngineInterface);

    return streamState->virtualEngineInterface->SetSubmissionType(cmdBuf, type);
}

#if _DEBUG || _RELEASE_INTERNAL

uint8_t MosInterface::GetVeEngineCount(
    MOS_STREAM_HANDLE streamState)
{
    return streamState && streamState->virtualEngineInterface ?
        streamState->virtualEngineInterface->GetEngineCount() : 0;
}

uint8_t MosInterface::GetEngineLogicId(
    MOS_STREAM_HANDLE streamState,
    uint32_t instanceIdx)
{
    return streamState && streamState->virtualEngineInterface ?
        streamState->virtualEngineInterface->GetEngineLogicId(instanceIdx) : 0;
}

#endif  // _DEBUG || _RELEASE_INTERNAL
MOS_STATUS MosInterface::ComposeCommandBufferHeader(
    MOS_STREAM_HANDLE     streamState,
    COMMAND_BUFFER_HANDLE cmdBuffer)
{
    MOS_OS_FUNCTION_ENTER;
    
    // No Command buffer header to compose

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosInterface::MosLoadLibrary(
    MOS_STREAM_HANDLE           streamState,
    PCCHAR                      pFileName,
    PHMODULE                    ppvModule)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_UNUSED(streamState);
    MOS_OS_ASSERT(pFileName);
    return MosUtilities::MosLoadLibrary(pFileName, ppvModule);
}

MOS_STATUS MosInterface::MosFreeLibrary(HMODULE hLibModule)
{
    MOS_OS_ASSERT(hLibModule);

    uint32_t ret = MosUtilities::MosFreeLibrary(hLibModule);

    return (ret == true) ? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}

GpuContextSpecificNext *MosInterface::GetGpuContext(MOS_STREAM_HANDLE streamState, GPU_CONTEXT_HANDLE handle)
{
    MOS_OS_FUNCTION_ENTER;

    if (streamState && streamState->osDeviceContext)
    {
        auto osDeviceContext = streamState->osDeviceContext;

        auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
        if (gpuContextMgr)
        {
            GpuContextNext *gpuCtx = gpuContextMgr->GetGpuContext(handle);

            return static_cast<GpuContextSpecificNext *>(gpuCtx);
        }
    }

    MOS_OS_ASSERTMESSAGE("GetGpuContext failed!");
    return nullptr;
}
