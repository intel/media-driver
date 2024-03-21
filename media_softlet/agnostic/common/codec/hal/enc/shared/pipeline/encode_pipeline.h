/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_pipeline.h
//! \brief    Defines the common interface for encode pipeline
//! \details  The encode pipeline interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#ifndef __ENCODE_PIPELINE_H__
#define __ENCODE_PIPELINE_H__

#include "media_pipeline.h"

#include "codec_hw_next.h"
#include "codec_def_encode.h"
#include "media_scalability.h"
#include "encode_allocator.h"
#include "encode_basic_feature.h"
#include "encode_tracked_buffer.h"
#include "encode_recycle_resource.h"
#include "encode_mem_compression.h"
#include "encodecp.h"
#include "encode_packet_utilities.h"
#include "encode_scalability_defs.h"

#define CONSTRUCTPACKETID(_componentId, _subComponentId, _packetId) \
    (_componentId << 24 | _subComponentId << 16 | _packetId)

namespace encode
{
class EncodePipeline : public MediaPipeline
{
public:
    //!
    //! \brief  EncodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    EncodePipeline(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~EncodePipeline() {}

    virtual MOS_STATUS Prepare(void *params) override;

    MOS_STATUS ContextSwitchBack();

    EncodeAllocator *    GetEncodeAllocator() { return m_allocator; };
    CodechalHwInterfaceNext *GetHWInterface() { return m_hwInterface; };
    PacketUtilities *    GetPacketUtilities() { return m_packetUtilities; };

    //!
    //! \brief  Get if SingleTaskPhaseSupported
    //! \return bool
    //!         value of SingleTaskPhaseSupported
    //!
    bool IsSingleTaskPhaseSupported() { return m_singleTaskPhaseSupported; };

    //!
    //! \brief  Get if m_singleTaskPhaseSupportedInPak
    //! \return bool
    //!         value of m_singleTaskPhaseSupportedInPak
    //!
    bool IsSingleTaskPhaseSupportedInPak() { return m_singleTaskPhaseSupportedInPak; };

    //!
    //! \brief  Get the Debug interface
    //! \return CodechalDebugInterface *
    //!         pointer of m_debugInterface
    //!
    CodechalDebugInterface *GetDebugInterface() const { return m_debugInterface; }

    //!
    //! \brief  Get the Debug interface for status report
    //! \return CodechalDebugInterface *
    //!         pointer of m_statusReportDebugInterface
    //!
    CodechalDebugInterface *GetStatusReportDebugInterface() const { return m_statusReportDebugInterface; }
    EncodeCp *               GetEncodeCp() { return m_encodecp; }

    //!
    //! \brief  Update frame tracking flag
    //! \return void
    //!
    void SetFrameTrackingForMultiTaskPhase();

    enum ComponentPacketIds
    {
        PACKET_COMPONENT_COMMON = 0,
        PACKET_COMPONENT_ENCODE,
        PACKET_COMPONENT_DECODE,
        PACKET_COMPONENT_VP,
    };

    enum SubComponentPacketIds
    {
        PACKET_SUBCOMPONENT_COMMON = 0,
        PACKET_SUBCOMPONENT_HEVC,
        PACKET_SUBCOMPONENT_VP9,
        PACKET_SUBCOMPONENT_AVC,
        PACKET_SUBCOMPONENT_AV1,
        PACKET_SUBCOMPONENT_JPEG
    };

    enum CommonPacketIds
    {
        basicPacket  = CONSTRUCTPACKETID(PACKET_COMPONENT_ENCODE, PACKET_SUBCOMPONENT_COMMON, 0),
        encodePreEncPacket,
#if _MEDIA_RESERVED
#define ENCODE_PACKET_IDS_EXT
#include "encode_pipeline_ext.h"
#undef ENCODE_PACKET_IDS_EXT
#endif
    };

protected:
    //!
    //! \brief  Initialize the encode pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings);

    //!
    //! \brief  Uninitialize the encode pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize();

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  Create buffer tracker, the derived class can overload it if
    //!         requires different buffer count
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateBufferTracker() = 0;

    //!
    //! \brief  Create status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateStatusReport() = 0;

    virtual MOS_STATUS GetSystemVdboxNumber();

    MOS_STATUS WaitForBatchBufferComplete();

    //!
    //! \brief  Finish the active packets execution
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteActivePackets() override;

    //! \brief  Calculate Command Size for all packets in active packets list
    //!
    //! \param  [in, out] commandBufferSize
    //!         cmd buffer size to calculate
    //! \param  [in, out] requestedPatchListSize
    //!         patch list size to calculate
    //! \return uint32_t
    //!         Command size calculated
    //!
    MOS_STATUS CalculateCmdBufferSizeFromActivePackets(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize);

    //!
    //! \brief  Declare Regkeys in the scope of encode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

public:
    //!
    //! \brief    Help function to get current pipe
    //!
    //! \return   Current pipe value
    //!
    virtual uint8_t GetCurrentPipe()
    {
        return m_scalability->GetCurrentPipe();
    }

    //!
    //! \brief    Help function to get current PAK pass
    //!
    //! \return   Current PAK pass
    //!
    virtual uint16_t GetCurrentPass()
    {
        return m_scalability->GetCurrentPass();
    }

    //!
    //! \brief  Create encode parameter
    //! \param  [in] params
    //!         Pointer to EncoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    //virtual MOS_STATUS CreateEncodeBasicFeature() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Help function to check if current pipe is first pipe
    //!
    //! \return   True if current pipe is first pipe, otherwise return false
    //!
    virtual bool IsFirstPipe()
    {
        return GetCurrentPipe() == 0 ? true : false;
    }

    //!
    //! \brief    Help function to check if current pipe is last pipe
    //!
    //! \return   True if current pipe is last pipe, otherwise return false
    //!
    virtual bool IsLastPipe()
    {
        return GetCurrentPipe() == GetPipeNum() - 1 ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is first pass
    //!
    //! \return   True if current PAK pass is first pass, otherwise return false
    //!
    virtual bool IsFirstPass()
    {
        return GetCurrentPass() == 0 ? true : false;
    }

    //!
    //! \brief    Help function to check if current PAK pass is last pass
    //!
    //! \return   True if current PAK pass is last pass, otherwise return false
    //!
    virtual bool IsLastPass()
    {
        return GetCurrentPass() == GetPassNum() - 1 ? true : false;
    }

    //!
    //! \brief    Help function to get pipe number
    //!
    //! \return   Pipe number
    //!
    virtual uint8_t GetPipeNum()
    {
        return m_scalability->GetPipeNumber();
    }

    //!
    //! \brief    Help function to get ddi target usage
    //!
    //! \return   DDI Target Usage
    //!
    virtual uint8_t GetDDITU()
    {
        return m_featureManager->GetDDITargetUsage();
    }

    //!
    //! \brief    Help function to get pass number
    //!
    //! \return   Pass number
    //!
    virtual uint16_t GetPassNum()
    {
        return m_scalability->GetPassNumber();
    }

    virtual uint8_t GetCurrentRow()
    {
        return m_scalability->GetCurrentRow();
    }

    virtual uint8_t GetCurrentSubPass()
    {
        return m_scalability->GetCurrentSubPass();
    }

    EncodeMemComp *GetMmcState() { return m_mmcState; }

    MOS_STATUS ExecuteResolveMetaData(PMOS_RESOURCE pInput, PMOS_RESOURCE pOutput);

    MOS_STATUS ReportErrorFlag(PMOS_RESOURCE pMetadataBuffer,
        uint32_t size, uint32_t offset, uint32_t flag);

#define CODECHAL_ENCODE_RECYCLED_BUFFER_NUM 6
#define VDENC_BRC_NUM_OF_PASSES 2

    uint8_t m_currRecycledBufIdx = 0;  //!< Current recycled buffer index
protected:
    uint32_t          m_standard = 0;   //!< The encode state's standard
    uint32_t          m_mode     = 0;   //!< The encode mode
    CODECHAL_FUNCTION m_codecFunction = CODECHAL_FUNCTION_INVALID;  //!< The encode state's codec function used

    CodechalHwInterfaceNext *m_hwInterface = nullptr;  //!< CodechalHwInterface
    MOS_INTERFACE *      m_osInterface = nullptr;
    EncodeAllocator *    m_allocator   = nullptr;
    TrackedBuffer *      m_trackedBuf  = nullptr;
    RecycleResource *    m_recycleBuf  = nullptr;
    EncodeMemComp *      m_mmcState    = nullptr;
    EncodeCp *           m_encodecp    = nullptr;
    PacketUtilities *    m_packetUtilities = nullptr;

    CodechalDebugInterface *m_statusReportDebugInterface = nullptr;  //!< Interface used for debug dumps in status report callback function

    uint8_t m_numVdbox = 0;

    bool m_singleTaskPhaseSupported      = true;
    bool m_singleTaskPhaseSupportedInPak = false;

    uint32_t m_recycledBufStatusNum[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {0};  //!< Recycled buffer status num list

    std::shared_ptr<EncodeScalabilityPars> m_scalPars = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodePipeline)
};

}  // namespace encode

#endif  // !__ENCODE_PIPELINE_H__
