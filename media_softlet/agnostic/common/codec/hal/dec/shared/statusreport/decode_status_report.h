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
//! \file     decode_status_report.h
//! \brief    Defines the class for decode status report
//! \details
//!
#ifndef __DECODE_STATUS_REPORT_H__
#define __DECODE_STATUS_REPORT_H__

#include "media_status_report.h"
#include "decode_status_report_defs.h"
#include "codec_def_common.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "decode_utils.h"
#include <stdint.h>

#define CODECHAL_DECODE_STATUS_NUM 512

namespace decode {
    class DecodeAllocator;

    //!
    //! \struct DecodeStatusReportData
    //! \brief  Decode status report structure
    //!
    struct DecodeStatusReportData
    {
        //! \brief Status for the picture associated with this status report
        CODECHAL_STATUS         codecStatus = CODECHAL_STATUS_SUCCESSFUL;
        //! \brief Status report number associated with the picture in this status report provided in Execute()
        uint32_t                statusReportNumber = 0;
        //! \brief Uncompressed frame information for the picture associated with this status report
        CODEC_PICTURE           currDecodedPic = { 0 };
        //! \brief Applies for VC1 and MPEG2 only, uncompressed frame information for the out of loop deblock destination
        CODEC_PICTURE           currDeblockedPic = { 0 };
        //! \brief Pointer to the resource for the decode render target for the picture associated with this status report
        MOS_RESOURCE            currDecodedPicRes = { 0 };
        //! \brief Applies when debug dumps are enabled for VC1 only, resource of deblocked picture
        MOS_RESOURCE            deblockedPicResOlp = { 0 };
        //! \brief number of MBs decoded or if unused set to 0xFFFF
        uint16_t                numMbsAffected = 0;
        //! \brief Crc of frame from MMIO
        uint32_t                frameCrc = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
        //! \brief Applies when debug dumps are enabled, pointer to SFC output resource for the picture associated with this status report
        PMOS_SURFACE            currSfcOutputSurface = nullptr;
        //! \brief Applies when debug dumps are enabled, pointer to histogram output resource for the picture associated with this status report
        PMOS_RESOURCE           currHistogramOutBuf = nullptr;
        //! \brief Applies when debug dumps are enabled, pointer to AV1 film grain output resource for the picture associated with this status report
        PMOS_RESOURCE           currFgOutputPicRes = nullptr;
        //! \brief Applies when debug dumps are enabled, stream out buffer (legacy)
        PMOS_RESOURCE           streamOutBuf = nullptr;
        //! \brief Applies when debug dumps are enabled, index of the streamout buffer (legacy)
        uint32_t                streamoutIdx = 0;
        //! \brief Applies when debug dumps are enabled, stream in buffer
        MOS_RESOURCE            streamInBufRes = { 0 };
        //! \brief Applies when debug dumps are enabled, stream out buffer
        MOS_RESOURCE            streamOutBufRes = { 0 };
        //! \brief Applies when debug dumps are enabled, stream size
        uint32_t                streamSize = 0;
        //! \brief Applies when debug dumps are enabled, indicates whether or not this is the final field in the frame.
        bool                    secondField = false;
        //! \brief Applies to VC1 only, indicates whether or not the frame required OLP.
        bool                    olpNeeded = false;
        //! \brief Applies when debug dumps are enabled, frame type (I/P/B)
        uint16_t                frameType = 0;
#endif // (_DEBUG || _RELEASE_INTERNAL)
    };

    class DecodeStatusReport : public MediaStatusReport
    {
    public:
        DecodeStatusReport(DecodeAllocator *alloc, bool enableRcs, PMOS_INTERFACE osInterface = nullptr);
        virtual ~DecodeStatusReport();

        //!
        //! \brief  Create resources for status report and do initialization
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Create() override;
        //!
        //! \brief  Destroy resources for status report
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Destroy();
        //!
        //! \brief  Initialize the status in report for each item
        //! 
        //! \details Called per frame for normal usages.
        //!          It can be called per tilerow if enable tile replay mode.
        //!
        //! \param  [in] inputPar
        //!         Pointer to parameters pass to status report.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init(void *inputPar) override;
        //!
        //! \brief  Reset Status
        //! 
        //! \details Called per frame for normal usages.
        //!          It can be called per tilerow if enable tile replay mode.
        //!
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Reset() override;

        //!
        //! \brief  Get Mfx status for frame specified by counter
        //! \param  [in] counter
        //!         The decode counter of requesting frame
        //! \return DecodeStatusMfx
        //!         The Mfx status specified by counter
        //!
        const DecodeStatusMfx& GetMfxStatus(uint32_t counter);

        //!
        //! \brief  Get report data for frame specified by counter
        //! \param  [in] counter
        //!         The decode counter of requesting frame
        //! \return DecodeStatusReportData
        //!         The report data specified by counter
        //!
        const DecodeStatusReportData& GetReportData(uint32_t counter);

    protected:
        //!
        //! \brief  Collect the status report information into report buffer.
        //! \param  [in] report
        //!         The report buffer address provided by DDI.
        //! \param  [in] index
        //!         The index of current requesting report.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS ParseStatus(void *report, uint32_t index) override;

        virtual MOS_STATUS SetStatus(void *report, uint32_t index, bool outOfRange = false) override;

        //!
        //! \brief  Set size for Mfx status buffer.
        //! \return void
        //!
        virtual void SetSizeForStatusBuf();

        //!
        //! \brief  Set offsets for Mfx status buffer.
        //! \return void
        //!
        virtual void SetOffsetsForStatusBuf();

        //!
        //! \brief  Update the status result of current report.
        //! \param  [in] statusReportData
        //!         The pointer to DecodeStatusReportData.
        //! \param  [in] decodeStatus
        //!         The RCS status report buffer.
        //! \param  [in] completed
        //!         Whether the request frame completed.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS UpdateCodecStatus(
            DecodeStatusReportData* statusReportData,
            DecodeStatusMfx* decodeStatus,
            bool completed);

    protected:

        bool                   m_enableRcs = false;
        DecodeAllocator*       m_allocator = nullptr;  //!< Decode allocator

        DecodeStatusReportData m_statusReportData[m_statusNum] = {};

        const uint32_t         m_completedCountSize = sizeof(uint32_t) * 2;
        uint32_t               m_statusBufSizeMfx   = 0;
        uint32_t               m_statusBufSizeRcs   = 0;

        PMOS_BUFFER            m_statusBufMfx = nullptr;
        PMOS_BUFFER            m_statusBufRcs = nullptr;
        uint8_t               *m_dataStatusMfx = nullptr;
        uint8_t               *m_dataStatusRcs = nullptr;
        PMOS_INTERFACE        m_osInterface = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__DecodeStatusReport)
    };
}

#endif // !__DECODE_STATUS_REPORT_H__
