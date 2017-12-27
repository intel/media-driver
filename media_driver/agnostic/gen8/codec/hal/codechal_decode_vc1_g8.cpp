/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_decode_vc1_g8.cpp
//! \brief    Implements the decode interface extension for Gen8 VC1.
//! \details  Implements all functions required by CodecHal for Gen8 VC1 decoding.
//!

#include "codeckrnheader.h"
#include "igcodeckrn_g8.h"
#include "codechal_decode_vc1_g8.h"

const CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8 g_cInit_CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8 =
{
    // uint32_t 0
    {
        { 0 }
    },

    // uint32_t 1
    {
        {
            16,     // BlockWidth in Byte
            16      // BlockHeight in Byte
        }
    },

    // uint32_t 2
    {
        {
            0,      // Profile
            0,      // RangeExpansionFlag
            0,      // UpsamplingFlag
            0,      // InterlaceFieldFlag
            0,      // RangeMapUV
            0,      // RangeMapUVFlag
            0,      // RangeMapY
            0,      // RangeMapYFlag
            0       // ComponentFlag
        }
    },

    // uint32_t 3
    {
        { 0 }       // Reserved
    },

    // uint32_t 4
    {
        { 0 }       // Reserved
    },

    // uint32_t 5
    {
        { 0 }       // Reserved
    },

    // uint32_t 6
    {
        { 0 }       // Reserved
    },

    // uint32_t 7
    {
        { 0 }       // Reserved
    }
};

const CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8  g_cInit_CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8 =
{
    // uint32_t 0
    {
        {
            0,      // BlockOriginY
            0       // BlockOriginX
        }
    },

    // uint32_t 1
    {
        { 0 }       // ComponentFlag
    },

    // uint32_t 2
    {
        {
            0,      // SourceDataBindingIndex
            0       // DestDataBindingIndex
        }
    },

    // uint32_t 3 - 7
    {
        0           // Reserved
    },
};

MOS_STATUS CodechalDecodeVc1G8::SetCurbeOlp()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface()->m_stateHeapInterface);

    CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8 cmd = g_cInit_CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8;

    cmd.DW2.InterlaceFieldFlag      = CodecHal_PictureIsField(pVc1PicParams->CurrPic);
    cmd.DW2.PictureUpsamplingFlag   = pVc1PicParams->UpsamplingFlag;
    cmd.DW2.RangeExpansionFlag      = (pVc1PicParams->range_mapping_fields.range_mapping_enabled != 0);
    cmd.DW2.Profile                 = pVc1PicParams->sequence_fields.AdvancedProfileFlag;

    if (pVc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        cmd.DW2.RangeMapUV          = pVc1PicParams->range_mapping_fields.chroma;
        cmd.DW2.RangeMapUVFlag      = pVc1PicParams->range_mapping_fields.chroma_flag;
        cmd.DW2.RangeMapY           = pVc1PicParams->range_mapping_fields.luma;
        cmd.DW2.RangeMapYFlag       = pVc1PicParams->range_mapping_fields.luma_flag;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(OlpKernelState.m_dshRegion.AddData(
        &cmd,
        OlpKernelState.dwCurbeOffset,
        sizeof(cmd)));


    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G8::AddVc1OlpMediaObjectsBB(
    PMHW_BATCH_BUFFER               batchBuffer)
{
    MOS_STATUS                             eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    uint16_t surfaceWidthInBlock = u16OlpPicWidthInMb;
    uint16_t surfaceHeightInBlock = u16OlpPicHeightInMb;

    CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8 inlineData = g_cInit_CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8;

    MHW_MEDIA_OBJECT_PARAMS                mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    mediaObjectParams.dwInterfaceDescriptorOffset = OlpKernelState.dwIdOffset;
    mediaObjectParams.dwInlineDataSize = sizeof(CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8);
    mediaObjectParams.pInlineData = &inlineData;

    // Process each Block (16x16) for Y surface
    inlineData.DW1.ComponentFlag = 0;
    inlineData.DW2.DestDataBindingIndex = 3;
    inlineData.DW2.SourceDataBindingIndex = 0;

    uint32_t tileX = 0, tileY = 0;
    for (tileY = 0; tileY < surfaceHeightInBlock; tileY++)
    {
        for (tileX = 0; tileX < surfaceWidthInBlock; tileX++)
        {
            inlineData.DW0.BlockOriginX = tileX * 16;
            inlineData.DW0.BlockOriginY = tileY * 16;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(nullptr, batchBuffer, &mediaObjectParams));
        }
    }

    surfaceHeightInBlock = MOS_ALIGN_CEIL(u16OlpPicHeightInMb, 2) / 2;
    inlineData.DW1.ComponentFlag = 1;
    inlineData.DW2.DestDataBindingIndex = 4;
    inlineData.DW2.SourceDataBindingIndex = 1;

    for (tileY = 0; tileY < surfaceHeightInBlock; tileY++)
    {
        for (tileX = 0; tileX < surfaceWidthInBlock; tileX++)
        {
            inlineData.DW0.BlockOriginX = tileX * 16;
            inlineData.DW0.BlockOriginY = tileY * 16;
            mediaObjectParams.pInlineData = &inlineData;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(nullptr, batchBuffer, &mediaObjectParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G8::UpdateVc1KernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    PCODECHAL_DECODE_VC1_KERNEL_HEADER decodeKernel;
    PMHW_KERNEL_STATE kernelState = &OlpKernelState;

    decodeKernel = (PCODECHAL_DECODE_VC1_KERNEL_HEADER)kernelState->KernelParams.pBinary;
    kernelState->dwKernelBinaryOffset =
        decodeKernel->OLP.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    kernelState->KernelParams.iInlineDataLength = sizeof(CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8);

    u32OlpDshSize =
        stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() +
        MOS_ALIGN_CEIL(u32OlpCurbeStaticDataLength, stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G8::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVc1::AllocateResources());

    // Second level batch buffer for OLP
    {
        MOS_ZeroMemory(&OlpBatchBuffer, sizeof(OlpBatchBuffer));
        uint32_t u32Size = m_hwInterface->GetMediaObjectBufferSize(
            (u32NumMacroblocks + u32NumMacroblocksUV),
            sizeof(CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8));
        CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &OlpBatchBuffer,
            nullptr,
            u32Size));
        OlpBatchBuffer.bSecondLevel = true;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G8::AddVc1OlpCmd(
    PCODECHAL_DECODE_VC1_OLP_PARAMS vc1OlpParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(vc1OlpParams->pCmdBuffer, &OlpBatchBuffer));

    // check if we need to reset the batch buffer
    if ((u16PicWidthInMb - u16OlpPicWidthInMb) ||
        (u16PicHeightInMb - u16OlpPicHeightInMb))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_LockBb(m_osInterface, &OlpBatchBuffer));
        u16OlpPicWidthInMb = u16PicWidthInMb;
        u16OlpPicHeightInMb = u16PicHeightInMb;

        CODECHAL_DECODE_CHK_STATUS_RETURN(AddVc1OlpMediaObjectsBB(
            &OlpBatchBuffer));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &OlpBatchBuffer));

        CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, &OlpBatchBuffer, true));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &OlpBatchBuffer,
            CODECHAL_MEDIA_STATE_OLP,
            "_DEC"));
    )

        return eStatus;
}

CodechalDecodeVc1G8::CodechalDecodeVc1G8(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeVc1(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);

    u32OlpCurbeStaticDataLength = CODECHAL_DECODE_VC1_CURBE_SIZE_OLP_G8;

    MOS_ZeroMemory(&OlpBatchBuffer, sizeof(OlpBatchBuffer));

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        (uint8_t*)IGCODECKRN_G8,
        IDR_CODEC_AllVC1_NV12,
        &OlpKernelBase,
        &OlpKernelSize);

    CODECHAL_DECODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_DECODE_VC1_NUM_SYNC_TAGS;
    hwInterface->GetStateHeapSettings()->dwIshSize =
        MOS_ALIGN_CEIL(OlpKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_DECODE_VC1_INITIAL_DSH_SIZE;
}

CodechalDecodeVc1G8::~CodechalDecodeVc1G8()
{
    Mhw_FreeBb(m_osInterface, &OlpBatchBuffer, nullptr);
};
