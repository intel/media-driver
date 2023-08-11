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
//! \file     decode_vp8_slice_packet.cpp
//! \brief    Defines the interface for vp8 decode slice packet
//!
#include "decode_vp8_slice_packet.h"

namespace decode
{
    MOS_STATUS Vp8DecodeSlcPkt::Init()
    {
        DECODE_FUNC_CALL();

        m_vp8BasicFeature = dynamic_cast<Vp8BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        m_allocator = m_pipeline ->GetDecodeAllocator();
        m_decodecp = m_pipeline->GetDecodeCp();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_vp8Pipeline);
        DECODE_CHK_NULL(m_mfxItf);
        DECODE_CHK_NULL(m_vp8BasicFeature);
        DECODE_CHK_NULL(m_allocator);
        DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodeSlcPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vp8BasicFeature->m_vp8PicParams);

        m_vp8PicParams = m_vp8BasicFeature->m_vp8PicParams;
        m_vp8SliceParams = m_vp8BasicFeature->m_vp8SliceParams;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFD_VP8_BSD_OBJECT, Vp8DecodeSlcPkt){

        uint8_t numPartitions = (1 << m_vp8PicParams->CodedCoeffTokenPartition);

        params.CodedNumOfCoeffTokenPartitions = m_vp8PicParams->CodedCoeffTokenPartition;
        params.Partition0CpbacEntropyRange = m_vp8PicParams->uiP0EntropyRange;
        params.Partition0CpbacEntropyCount = m_vp8PicParams->ucP0EntropyCount;
        params.Partition0CpbacEntropyValue = m_vp8PicParams->ucP0EntropyValue;

        params.IndirectPartition0DataLength = m_vp8PicParams->uiPartitionSize[0] + 1;
        params.IndirectPartition0DataStartOffset = m_vp8PicParams->uiFirstMbByteOffset;

        params.IndirectPartition1DataLength = m_vp8PicParams->uiPartitionSize[1] + 1;
        params.IndirectPartition1DataStartOffset = params.IndirectPartition0DataStartOffset +
            m_vp8PicParams->uiPartitionSize[0] +
            (numPartitions - 1) * 3;      // Account for P Sizes: 3 bytes per partition
                                            // excluding partition 0 and last partition.

        int32_t i = 2;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition2DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition2DataStartOffset = params.IndirectPartition1DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 3;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition3DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition3DataStartOffset = params.IndirectPartition2DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 4;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition4DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition4DataStartOffset = params.IndirectPartition3DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 5;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition5DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition5DataStartOffset = params.IndirectPartition4DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 6;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition6DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition6DataStartOffset = params.IndirectPartition5DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 7;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition7DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition7DataStartOffset = params.IndirectPartition6DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        i = 8;
        if (i < ((1 + numPartitions)))
        {
            params.IndirectPartition8DataLength = m_vp8PicParams->uiPartitionSize[i] + 1;
            params.IndirectPartition8DataStartOffset = params.IndirectPartition7DataStartOffset + m_vp8PicParams->uiPartitionSize[i - 1];
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodeSlcPkt::CalculateCommandSize(uint32_t &commandBufferSize,
                                                      uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_sliceStatesSize;
        requestedPatchListSize = m_slicePatchListSize;

        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS Vp8DecodeSlcPkt::AddMiFlushDwCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
        MOS_ZeroMemory(&par, sizeof(par));
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodeSlcPkt::AddMiBatchBufferEnd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_END(&cmdBuffer, nullptr));

        return MOS_STATUS_SUCCESS;
    }



    MOS_STATUS Vp8DecodeSlcPkt::CalculateSliceStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Slice Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetMfxPrimitiveCommandsDataSize(
            m_vp8BasicFeature->m_mode,
            &m_sliceStatesSize,
            &m_slicePatchListSize,
            m_vp8BasicFeature->m_shortFormatInUse));

        return MOS_STATUS_SUCCESS;
    }

}
