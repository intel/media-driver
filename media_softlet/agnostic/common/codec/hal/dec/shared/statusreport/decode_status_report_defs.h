/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     decode_status_report_defs.h
//! \brief    Defines the common struture for decode status report
//! \details
//!
#ifndef __DECODE_STATUS_REPORT_DEFS_H__
#define __DECODE_STATUS_REPORT_DEFS_H__

#include "mos_defs.h"
#include "codec_def_common.h"
#include "media_status_report.h"

namespace decode
{

enum CsEngineIdDef
{
    // Instance ID
    csInstanceIdVdbox0 = 0,
    csInstanceIdVdbox1 = 1,
    csInstanceIdVdbox2 = 2,
    csInstanceIdVdbox3 = 3,
    csInstanceIdVdbox4 = 4,
    csInstanceIdVdbox5 = 5,
    csInstanceIdVdbox6 = 6,
    csInstanceIdVdbox7 = 7,
    csInstanceIdMax,
    // Class ID
    classIdVideoEngine = 1,
};

union CsEngineId
{
    struct
    {
        uint32_t       classId            : 3;    //[0...4]
        uint32_t       reservedFiled1     : 1;    //[0]
        uint32_t       instanceId         : 6;    //[0...7]
        uint32_t       reservedField2     : 22;   //[0]
    } fields;
    uint32_t            value;
};

enum DecodeStatusReportType
{
    statusReportGlobalCount = STATUS_REPORT_GLOBAL_COUNT,
    statusReportMfx,

    //! \brief decode error status
    DecErrorStatusOffset,

    //! \brief decode MB count
    DecMBCountOffset,

    //! \brief decode frame CRC
    DecFrameCrcOffset,

    //! \brief CS engine ID
    CsEngineIdOffset_0,
    CsEngineIdOffset_1,
    CsEngineIdOffset_2,
    CsEngineIdOffset_3,
    CsEngineIdOffset_4,
    CsEngineIdOffset_5,
    CsEngineIdOffset_6,
    CsEngineIdOffset_7,

    //! \brief MMIO HuCErrorStatus2
    HucErrorStatus2Reg,

    //! \brief mask of MMIO HuCErrorStatus2
    HucErrorStatus2Mask,

    //! \brief MMIO HuCErrorStatus
    HucErrorStatusReg,
    //! \brief mask of MMIO HuCErrorStatus
    HucErrorStatusMask,

    statusReportRcs,
    statusReportMaxNum
};

struct DecodeStatusParameters
{
    uint32_t           statusReportFeedbackNumber;
    uint32_t           numberTilesInFrame;
    uint16_t           pictureCodingType;
    CODEC_PICTURE      currOriginalPic;
    CODECHAL_FUNCTION  codecFunction;
    uint8_t            numUsedVdbox;
    PCODEC_REF_LIST    currRefList;
    uint16_t           picWidthInMb;
    uint16_t           frameFieldHeightInMb;
    uint32_t           numSlices;
    MOS_RESOURCE       currDecodedPicRes;
    bool               isSecondField;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_SURFACE       *sfcOutputSurface;
    MOS_RESOURCE      *histogramOutputBuf;
    MOS_RESOURCE      *fgOutputPicRes;
    MOS_RESOURCE      *streamInBufRes;
    MOS_RESOURCE      *streamOutBufRes;
    uint32_t           streamSize;
#endif
};

struct DecodeStatusMfx
{
    //!< HW requires a QW aligned offset for data storage
    uint32_t                status = 0;
    //! \brief Value of MMIO decoding effor eStatus register
    uint32_t                m_mmioErrorStatusReg = 0;
    //! \brief Value of MMIO decoding MB error register
    uint32_t                m_mmioMBCountReg = 0;
    //! \brief Frame CRC related to current frames
    uint32_t                m_mmioFrameCrcReg = 0;
    //! \brief Value of MMIO CS Engine ID register for each BB
    uint32_t                m_mmioCsEngineIdReg[csInstanceIdMax] = { 0 };
    //! \brief Huc error for HEVC Fix Function, DWORD0: mask value, DWORD1: reg value
    uint64_t                m_hucErrorStatus2 = 0;
    //! \brief Huc error for HEVC Fix Function, DWORD0: mask value, DWORD1: reg value
    uint64_t                m_hucErrorStatus = 0;
};

struct DecodeStatusRcs
{
    uint32_t                    status;
    uint32_t                    pad;        //!< Pad
};

}

#endif