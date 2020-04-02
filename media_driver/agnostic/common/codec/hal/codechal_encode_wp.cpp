/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_wp.cpp
//! \brief    Defines base class for weighted prediction kernel
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_wp.h"
#include "hal_oca_interface.h"

MOS_STATUS CodechalEncodeWP::AllocateResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (Mos_ResourceIsNull(&m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx].OsResource))
    {
        MOS_ZeroMemory(&m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx], sizeof(MOS_SURFACE));

        MOS_ALLOC_GFXRES_PARAMS  allocParamsForBufferNV12;
        MOS_ZeroMemory(&allocParamsForBufferNV12, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferNV12.Type     = MOS_GFXRES_2D;
        allocParamsForBufferNV12.TileType = MOS_TILE_Y;
        allocParamsForBufferNV12.Format   = Format_NV12;
        allocParamsForBufferNV12.dwWidth  = m_frameWidth;
        allocParamsForBufferNV12.dwHeight = m_frameHeight;
        allocParamsForBufferNV12.pBufName = "WP Scaled output Buffer";
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferNV12,
            &m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx].OsResource),
            "Failed to allocate WP Scaled output Buffer.");

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface,
            &m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx]));
    }

    return eStatus;
}

void CodechalEncodeWP::ReleaseResources()
{
    for (auto i = 0; i < CODEC_NUM_WP_FRAME; i++)
    {
        if (!Mos_ResourceIsNull(&m_surfaceParams.weightedPredOutputPicList[i].OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_surfaceParams.weightedPredOutputPicList[i].OsResource);
        }
    }
}

uint8_t CodechalEncodeWP::GetBTCount()
{
    return (uint8_t)wpNumSurfaces;
}

MOS_STATUS CodechalEncodeWP::InitKernelState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_kernelState)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_kernelState = MOS_New(MHW_KERNEL_STATE));
    }

    uint8_t* binary;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kernelUID,
        &binary,
        &m_combinedKernelSize));

    auto kernelSize = m_combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        binary,
        ENC_WP,
        0,
        &currKrnHeader,
        &kernelSize));

    m_kernelState->KernelParams.iBTCount          = wpNumSurfaces;
    m_kernelState->KernelParams.iThreadCount      = m_renderInterface->GetHwCaps()->dwMaxThreads;
    m_kernelState->KernelParams.iCurbeLength      = m_curbeLength;
    m_kernelState->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
    m_kernelState->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
    m_kernelState->KernelParams.iIdCount          = 1;
    m_kernelState->KernelParams.iInlineDataLength = 0;
    m_kernelState->dwCurbeOffset                  = m_stateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    m_kernelState->KernelParams.pBinary           = binary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    m_kernelState->KernelParams.iSize             = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->CalculateSshAndBtSizesRequested(
        m_kernelState->KernelParams.iBTCount,
        &m_kernelState->dwSshSize,
        &m_kernelState->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_renderInterface->m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_renderInterface->m_stateHeapInterface, m_kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeWP::SetCurbe()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS        eStatus = MOS_STATUS_SUCCESS;
    CurbeData         curbe;

    MOS_ZeroMemory(&curbe, sizeof(CurbeData));
    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    //C Model hard code log2WeightDenom = 6. No need to send WD paramters to WP Kernel.
    curbe.DW0.defaultWeight  = m_curbeParams.slcParams->weights[m_curbeParams.refPicListIdx][m_curbeParams.wpIdx][0][0];
    curbe.DW0.defaultOffset  = m_curbeParams.slcParams->weights[m_curbeParams.refPicListIdx][m_curbeParams.wpIdx][0][1];

    curbe.DW49.inputSurface  = wpInputRefSurface;
    curbe.DW50.outputSurface = wpOutputScaledSurface;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_kernelState->m_dshRegion.AddData(
        &curbe,
        m_kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeWP::SendSurface(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto currFieldPicture = CodecHal_PictureIsField(m_currOriginalPic);
    // Program the surface based on current picture's field/frame mode
    uint32_t refBindingTableOffset;
    uint32_t refVerticalLineStride;
    uint32_t refVerticalLineStrideOffset;
    uint8_t  refVDirection;
    if (currFieldPicture) // if current picture is field
    {
        if (m_surfaceParams.refIsBottomField)
        {
            refVDirection               = CODECHAL_VDIRECTION_BOT_FIELD;
            refVerticalLineStride       = CODECHAL_VLINESTRIDE_FIELD;
            refVerticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_BOT_FIELD;
        }
        else
        {
            refVDirection               = CODECHAL_VDIRECTION_TOP_FIELD;
            refVerticalLineStride       = CODECHAL_VLINESTRIDE_FIELD;
            refVerticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;
        }
    }
    else // if current picture is frame
    {
        refVDirection               = CODECHAL_VDIRECTION_FRAME;
        refVerticalLineStride       = CODECHAL_VLINESTRIDE_FRAME;
        refVerticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;
    }

    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
    surfaceCodecParams.bIs2DSurface               = true;
    surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.psSurface                  = m_surfaceParams.refFrameInput; // Input surface
    surfaceCodecParams.bIsWritable                = false;
    surfaceCodecParams.bRenderTarget              = false;
    surfaceCodecParams.dwBindingTableOffset       = wpInputRefSurface;
    surfaceCodecParams.dwCacheabilityControl      = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE].Value;
    surfaceCodecParams.dwVerticalLineStride       = refVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = refVerticalLineStrideOffset;
    surfaceCodecParams.ucVDirection               = refVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        m_kernelState));

    MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
    surfaceCodecParams.bIs2DSurface               = true;
    surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.psSurface                  = &m_surfaceParams.weightedPredOutputPicList[m_surfaceParams.wpOutListIdx]; // output surface
    surfaceCodecParams.bIsWritable                = true;
    surfaceCodecParams.bRenderTarget              = true;
    surfaceCodecParams.dwBindingTableOffset       = wpOutputScaledSurface;
    surfaceCodecParams.dwCacheabilityControl      = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE].Value;
    surfaceCodecParams.dwVerticalLineStride       = refVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = refVerticalLineStrideOffset;
    surfaceCodecParams.ucVDirection               = refVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        m_kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeWP::Execute(KernelParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    if (params->slcWPParams && params->slcWPParams->luma_log2_weight_denom != 6)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_ASSERTMESSAGE("Weighted Prediction Kernel does not support Log2LumaWeightDenom != 6!");
        return eStatus;
    }

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_WP_KERNEL);

    if (params->useRefPicList1)
    {
        *(params->useWeightedSurfaceForL1) = true;
        m_surfaceParams.wpOutListIdx = CODEC_WP_OUTPUT_L1_START + params->wpIndex;
    }
    else
    {
        *(params->useWeightedSurfaceForL0) = true;
        m_surfaceParams.wpOutListIdx = CODEC_WP_OUTPUT_L0_START + params->wpIndex;
    }
    if (m_surfaceParams.wpOutListIdx >= CODEC_NUM_WP_FRAME)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_ASSERTMESSAGE("index exceeds maximum value of array weightedPredOutputPicList.");
        return eStatus;
    }

    // Allocate output surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources());

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        auto maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : m_kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->RequestSshSpaceForCmdBuf(maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->VerifySpaceAvailable());
    }

    // setup DSH and Interface Descriptor
    auto stateHeapInterface = m_renderInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        m_kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = m_kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SetInterfaceDescriptor(1, &idParams));

    // Setup Curbe
    m_curbeParams.refPicListIdx = (params->useRefPicList1) ? LIST_1 : LIST_0;
    m_curbeParams.wpIdx         = params->wpIndex;
    m_curbeParams.slcParams     = params->slcWPParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbe());

    auto encFunctionType = CODECHAL_MEDIA_STATE_ENC_WP;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            m_kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            m_kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            m_kernelState));
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = m_kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SetBindingTable(m_kernelState));

    (params->useRefPicList1) ? (*params->useWeightedSurfaceForL1 = true) : (*params->useWeightedSurfaceForL0 = true);
    CodecHalGetResourceInfo(m_osInterface, params->refFrameInput);

    //Set Surface States
    m_surfaceParams.refFrameInput    = params->refFrameInput;
    m_surfaceParams.refIsBottomField = params->refIsBottomField;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSurface(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            m_kernelState));
    )

    // Thread Dispatch Pattern - MEDIA OBJECT WALKER
    if (m_hwWalker)
    {
        auto resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth);
        auto resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);

        CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
        MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
        walkerCodecParams.WalkerMode              = m_walkerMode;
        walkerCodecParams.bUseScoreboard          = m_useHwScoreboard;
        walkerCodecParams.dwResolutionX           = resolutionX;
        walkerCodecParams.dwResolutionY           = resolutionY;
        walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
        walkerCodecParams.ucGroupId               = m_groupId;
        walkerCodecParams.bNoDependency           = true;

        MHW_WALKER_PARAMS walkerParams;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(m_hwInterface, &walkerParams, &walkerCodecParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaObjectWalkerCmd(&cmdBuffer, &walkerParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SubmitBlocks(m_kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->UpdateGlobalCmdBufId());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(
        &cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

CodechalEncodeWP::CodechalEncodeWP(CodechalEncoderState *encoder)
    : m_useHwScoreboard(encoder->m_useHwScoreboard),
    m_renderContextUsesNullHw(encoder->m_renderContextUsesNullHw),
    m_groupIdSelectSupported(encoder->m_groupIdSelectSupported),
    m_singleTaskPhaseSupported(encoder->m_singleTaskPhaseSupported),
    m_firstTaskInPhase(encoder->m_firstTaskInPhase),
    m_lastTaskInPhase(encoder->m_lastTaskInPhase),
    m_hwWalker(encoder->m_hwWalker),
    m_groupId(encoder->m_groupId),
    m_pictureCodingType(encoder->m_pictureCodingType),
    m_mode(encoder->m_mode),
    m_verticalLineStride(encoder->m_verticalLineStride),
    m_maxBtCount(encoder->m_maxBtCount),
    m_vmeStatesSize(encoder->m_vmeStatesSize),
    m_storeData(encoder->m_storeData),
    m_frameWidth(encoder->m_frameWidth),
    m_frameHeight(encoder->m_frameHeight),
    m_frameFieldHeight(encoder->m_frameFieldHeight),
    m_currOriginalPic(encoder->m_currOriginalPic),
    m_walkerMode(encoder->m_walkerMode)
{
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(encoder);

    // Initilize interface pointers
    m_encoder            = encoder;
    m_osInterface        = encoder->GetOsInterface();
    m_hwInterface        = encoder->GetHwInterface();
    m_debugInterface     = encoder->GetDebugInterface();
    m_miInterface        = m_hwInterface->GetMiInterface();
    m_renderInterface    = m_hwInterface->GetRenderInterface();
    m_stateHeapInterface = m_renderInterface->m_stateHeapInterface->pStateHeapInterface;
    m_curbeLength        = sizeof(CurbeData);
}

CodechalEncodeWP::~CodechalEncodeWP()
{
    // free weighted prediction surface
    ReleaseResources();

    MOS_Delete(m_kernelState);
    m_kernelState = nullptr;
}