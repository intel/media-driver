/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_mpeg2_slice_packet.cpp
//! \brief    Defines the interface for mpeg2 decode slice packet
//!
#include "decode_mpeg2_slice_packet.h"

namespace decode{

MOS_STATUS Mpeg2DecodeSlcPkt::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_mpeg2Pipeline);
    DECODE_CHK_NULL(m_mfxItf);

    m_mpeg2BasicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_mpeg2BasicFeature);

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    m_decodecp = m_pipeline->GetDecodeCp();

    DECODE_CHK_STATUS(CalculateSliceStateCommandSize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeSlcPkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_mpeg2BasicFeature->m_mpeg2PicParams);
    DECODE_CHK_NULL(m_mpeg2BasicFeature->m_mpeg2SliceParams);

    m_mpeg2PicParams   = m_mpeg2BasicFeature->m_mpeg2PicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeSlcPkt::AddCmd_MFD_MPEG2_BSD_OBJECT(MHW_BATCH_BUFFER &batchBuffer, uint16_t slcIdx)
{
    DECODE_FUNC_CALL();

    auto &par = m_mfxItf->MHW_GETPAR_F(MFD_MPEG2_BSD_OBJECT)();
    par       = {};

    par.decodeInUse = true;

    CodecDecodeMpeg2SliceParams *slc = &m_mpeg2BasicFeature->m_sliceRecord[slcIdx].recordSliceParam;

    uint32_t endMb = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].sliceStartMbOffset + slc->m_numMbsForSlice;

    par.IndirectBsdDataLength    = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].length;
    par.IndirectDataStartAddress = slc->m_sliceDataOffset + m_mpeg2BasicFeature->m_sliceRecord[slcIdx].offset;
    par.FirstMacroblockBitOffset = (slc->m_macroblockOffset & 0x0007);

    par.IsLastMb = par.LastPicSlice = m_mpeg2BasicFeature->m_sliceRecord[slcIdx].isLastSlice;
    par.MbRowLastSlice = ((endMb / m_mpeg2BasicFeature->m_picWidthInMb) != slc->m_sliceVerticalPosition) ? 1 : 0;

    par.MacroblockCount         = slc->m_numMbsForSlice;
    par.SliceHorizontalPosition = slc->m_sliceHorizontalPosition;
    par.SliceVerticalPosition   = slc->m_sliceVerticalPosition;
    par.QuantizerScaleCode      = slc->m_quantiserScaleCode;

    if (par.IsLastMb)
    {
        par.NextSliceHorizontalPosition = 0;
        par.NextSliceVerticalPosition   = m_mpeg2BasicFeature->m_picWidthInMb;
    }
    else
    {
        par.NextSliceHorizontalPosition = endMb % m_mpeg2BasicFeature->m_picWidthInMb;
        par.NextSliceVerticalPosition   = endMb / m_mpeg2BasicFeature->m_picWidthInMb;
    }

    uint32_t offset = ((slc->m_macroblockOffset & 0x0000fff8) >> 3);

    par.presDataBuffer    = &m_mpeg2BasicFeature->m_resDataBuffer.OsResource;
    par.dwDataStartOffset = slc->m_sliceDataOffset + offset;

    DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_MPEG2_BSD_OBJECT)(nullptr, &batchBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeSlcPkt::AddAllCmdsInsertDummySlice(MHW_BATCH_BUFFER &batchBuffer, uint16_t startMB, uint16_t endMB)
{
    DECODE_FUNC_CALL();

    auto &par = m_mfxItf->MHW_GETPAR_F(MFD_MPEG2_BSD_OBJECT)();
    par       = {};

    par.decodeInUse = true;

    uint16_t intraVLDFormat     = m_mpeg2PicParams->W0.m_intraVlcFormat;
    uint16_t quantizerScaleType = m_mpeg2PicParams->W0.m_quantizerScaleType;
    uint16_t dummySliceIndex    = quantizerScaleType * 2 + intraVLDFormat;

    par.IndirectBsdDataLength    = m_mpeg2BasicFeature->Mpeg2DummySliceLengths[dummySliceIndex];
    par.IndirectDataStartAddress = m_mpeg2BasicFeature->Mpeg2DummySliceOffsets[dummySliceIndex] +
                               m_mpeg2BasicFeature->m_dummySliceDataOffset;

    uint32_t macroblockOffset    = 6;
    par.FirstMacroblockBitOffset = (macroblockOffset & 0x0007);
    par.QuantizerScaleCode       = 10;
    par.MacroblockCount          = 1;

    // force disable cp for dummy slices
    bool isCpEnabled = false;
    if (m_decodecp)
    {
        isCpEnabled = m_decodecp->IsCpEnabled();
        if (isCpEnabled)
        {
            m_decodecp->SetCpEnabled(false);
        }
    }

    uint16_t expectedEndMB = m_mpeg2BasicFeature->m_picWidthInMb * m_mpeg2BasicFeature->m_picHeightInMb;

    while (startMB < endMB)
    {

        par.SliceHorizontalPosition = startMB % m_mpeg2BasicFeature->m_picWidthInMb;
        par.SliceVerticalPosition   = startMB / m_mpeg2BasicFeature->m_picWidthInMb;

        par.IsLastMb = par.LastPicSlice = ((startMB + par.MacroblockCount) == expectedEndMB);
        par.MbRowLastSlice = ((startMB / m_mpeg2BasicFeature->m_picWidthInMb) != par.SliceVerticalPosition) ? 1 : 0;

        if (par.IsLastMb)
        {
            par.NextSliceHorizontalPosition = 0;
            par.NextSliceVerticalPosition   = m_mpeg2BasicFeature->m_picWidthInMb;
        }
        else
        {
            par.NextSliceHorizontalPosition = (startMB + par.MacroblockCount) % m_mpeg2BasicFeature->m_picWidthInMb;
            par.NextSliceVerticalPosition   = (startMB + par.MacroblockCount) / m_mpeg2BasicFeature->m_picWidthInMb;
        }

        DECODE_CHK_STATUS(m_mfxItf->MHW_ADDCMD_F(MFD_MPEG2_BSD_OBJECT)(nullptr, &batchBuffer));

        startMB++;
    }

    // restore Cp state
    if (m_decodecp && isCpEnabled)
    {
        m_decodecp->SetCpEnabled(true);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeSlcPkt::CalculateCommandSize(uint32_t &commandBufferSize,
                                                    uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = m_sliceStatesSize;
    requestedPatchListSize = m_slicePatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodeSlcPkt::CalculateSliceStateCommandSize()
{
    DECODE_FUNC_CALL();

    // Slice Level Commands
    DECODE_CHK_STATUS(m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mpeg2BasicFeature->m_mode,
        &m_sliceStatesSize,
        &m_slicePatchListSize,
        0));

    return MOS_STATUS_SUCCESS;
}

}
