/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     decode_pipeline.h
//! \brief    Defines the common interface for decode pipeline
//! \details  The decode pipeline interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#ifndef __DECODE_PIPELINE_H__
#define __DECODE_PIPELINE_H__

#include "media_pipeline.h"

#include "codec_hw_next.h"
#include "codechal_debug.h"
#include "codec_def_decode.h"
#include "decode_allocator.h"
#include "decode_sub_pipeline_manager.h"
#include "decode_sub_packet_manager.h"
#include "decode_input_bitstream.h"
#include "decodecp_interface.h"
#include "decode_cp_bitstream.h"
#include "decode_mem_compression.h"
#include "decode_downsampling_feature.h"
#include "codechal_oca_debug.h"

namespace decode {

enum DecodePipeMode
{
    decodePipeModeBegin = 0,
    decodePipeModeProcess,
    decodePipeModeEnd
};

struct DecodePipelineParams
{
    CodechalDecodeParams *m_params = nullptr;
    DecodePipeMode m_pipeMode = decodePipeModeBegin;
};

class DecodeSubPacket;
class DecodeSubPacketManager;

class DecodePipeline : public MediaPipeline
{
public:
    //!
    //! \brief  DecodePipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    DecodePipeline(
        CodechalHwInterfaceNext *hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~DecodePipeline()
    {
        MOS_Delete(m_pCodechalOcaDumper);
    };

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(void *params) override;

    //!
    //! \brief  Indicates whether input bitstream is complete for current frame
    //! \return bool
    //!         true for compelte, false for incompelte
    //!
    bool IsCompleteBitstream();

    //!
    //! \brief    Help function to get sub packet
    //!
    //! \return   Sub packet if success, else nullptr
    //!
    DecodeSubPacket* GetSubPacket(uint32_t subPacketId);

    //!
    //! \brief  Get if SingleTaskPhaseSupported
    //! \return bool
    //!         value of SingleTaskPhaseSupported
    //!
    bool IsSingleTaskPhaseSupported() { return m_singleTaskPhaseSupported; };

    //!
    //! \brief  Get the resource allocator for decode
    //! \return DecodeAllocator *
    //!         pointer of decode allocator
    //!
    DecodeAllocator *GetDecodeAllocator() const { return m_allocator; }

    //!
    //! \brief  Get decode cp interface
    //! \return DecodeCpInterface *
    //!         pointer of DecodeCpInterface
    //!
    DecodeCpInterface *GetDecodeCp() const { return m_decodecp; }

    //!
    //! \brief  Get the debug interface
    //! \return CodechalDebugInterface *
    //!         pointer of m_debugInterface
    //!
    CodechalDebugInterface *GetDebugInterface() const { return m_debugInterface; }

    //!
    //! \brief  Get the HW interface
    //! \return CodechalHwInterface *
    //!         pointer of m_hwInterface
    //!
    CodechalHwInterfaceNext *GetHwInterface() const { return m_hwInterface; }

    //!
    //! \brief    Help function to get pipe number
    //!
    //! \return   Pipe number
    //!
    virtual uint8_t GetPipeNum() { return m_scalability->GetPipeNumber(); }

    //!
    //! \brief    Help function to get current pipe
    //!
    //! \return   Pass index
    //!
    virtual uint8_t GetCurrentPipe() { return m_scalability->GetCurrentPipe(); }

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
    //! \brief    Help function to get pass number
    //!
    //! \return   Pass number
    //!
    virtual uint16_t GetPassNum() { return m_scalability->GetPassNumber(); }

    //!
    //! \brief    Help function to get current pass
    //!
    //! \return   Pass index
    //!
    virtual uint16_t GetCurrentPass() { return m_scalability->GetCurrentPass(); }

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
    //! \brief    Help function to get component state
    //!
    //! \return   Point to component state
    //!
    virtual ComponentState *GetComponentState()
    {
        return m_scalability->GetComponentState();
    }

    //!
    //! \brief    Help function to check if phased submission mode
    //!
    //! \return   True if phased submission mode, otherwise return false
    //!
    virtual bool IsPhasedSubmission()
    {
        return m_osInterface->phasedSubmission;
    }

    //!
    //! \brief    Help function to check if guc submission mode
    //!
    //! \return   True if guc submission mode, otherwise return false
    //!
    virtual bool IsParallelSubmission()
    {
        return m_osInterface->bParallelSubmission;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    //!
    //! \brief  Check if down sampling supported for current frame
    //! \return Return if down sampling supported for current frame
    //!
    bool IsDownSamplingSupported();
#endif

    //!
    //! \brief  Get dummy reference surface
    //! \return Pointer of reference surface
    //!
    MOS_SURFACE* GetDummyReference();

    //!
    //! \brief  Get dummy reference status
    //! \return CODECHAL_DUMMY_REFERENCE_STATUS
    //!
    CODECHAL_DUMMY_REFERENCE_STATUS GetDummyReferenceStatus();

    //!
    //! \brief  Set dummy reference status
    //! \return void
    //!
    void SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status);

    //!
    //! \brief  Get mmc state
    //! \return Pointer of mmc state
    //!
    DecodeMemComp *GetMmcState() { return m_mmcState; }

    //!
    //! \brief  Get Wa table
    //! \return Pointer of Wa table
    //!
    MEDIA_WA_TABLE *GetWaTable() { return m_waTable; }

    //!
    //! \brief Get Sku table
    //! \return Pointer of SKU table
    //!
    MEDIA_FEATURE_TABLE *GetSkuTable() { return m_skuTable; }

    DeclareDecodePacketId(hucCopyPacketId);
    DeclareDecodePacketId(hucCpStreamOutPacketId);
    DeclareDecodePacketId(predicationSubPacketId);
    DeclareDecodePacketId(markerSubPacketId);
    DeclareDecodePacketId(downSamplingSubPacketId);

    //!
    //! \brief  Get decode context
    //! \return decode context
    //!
    MOS_GPU_CONTEXT GetDecodeContext() { return m_decodeContext; }

    //!
    //! \brief  Get decode context handle
    //! \return decode context handle
    //!
    GPU_CONTEXT_HANDLE GetDecodeContextHandle() { return m_decodeContextHandle; }
    
    //!
    //! \brief  Indicates whether current process pipe is first process pipe of current frame
    //! \return bool
    //!         true for first process pipe, false for other process pipe
    //!
    bool IsFirstProcessPipe(const DecodePipelineParams &pipelineParams);

    //!
    //! \brief    Gets oca dump interface.
    //! \return   CodechalOcaDumper
    //!           return oca debug interface
    //!
    CodechalOcaDumper *GetCodechalOcaDumper() { return m_pCodechalOcaDumper; }

    //!
    //! \brief    Set decode short/long format during runtime.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDecodeFormat(bool isShortFormat ){ return MOS_STATUS_UNIMPLEMENTED; };

protected:
    //!
    //! \brief  Initialize the decode pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings);

    //!
    //! \brief  Uninitialize the decode pipeline
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
    //! \brief  Get the number of Vdbox
    //! \return uint8_t
    //!         Return the number of Vdbox
    //!
    virtual uint8_t GetSystemVdboxNumber();

    //!
    //! \brief  Create status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateStatusReport();

    //!
    //! \brief  Finish the active packets execution
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteActivePackets() override;

    //!
    //! \brief  Create pre sub pipelines
    //! \param  [in] subPipelineManager
    //!         Point to subPipeline manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager);

    //!
    //! \brief  Create Post sub pipelines
    //! \param  [in] subPipelineManager
    //!         Point to subPipeline manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager);

    //!
    //! \brief  Create sub packets
    //! \param  [in] subPacketManager
    //!         Sub packet manager
    //! \param  [in] codecSettings
    //!         Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings);
    //!
    //! \brief  Declare Regkeys in the scope of decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

#if USE_CODECHAL_DEBUG_TOOL
#ifdef _DECODE_PROCESSING_SUPPORTED
    MOS_STATUS DumpDownSamplingParams(DecodeDownSamplingFeature &downSamplingParams);
#endif

    //!
    //! \brief  Dump Bitstream
    //! \param  [in] pBitstream
    //!         Bitstream Resource
    //! \param  [in] size
    //!         Bitstream Size
    //! \param  [in] offset
    //!         Bitstream Offset 
    //! \param  [in] attrName
    //!         Bitstream Name
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpBitstream(PMOS_RESOURCE pBitstream, uint32_t size, uint32_t offset, const char* attrName = nullptr);

    //!
    //! \brief  Add delay for dump render targets
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DelayForDumpOutput();

    //!
    //! \brief  Dump render targets
    //! \param  [in] reportData
    //!         Decode report data
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpOutput(const DecodeStatusReportData& reportData);
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief  User feature key report for Vdbox IDs
    //! \param  [in] status
    //!         Status report from HW
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReportVdboxIds(const DecodeStatusMfx& status);

    //!
    //! \brief  Earlier stop for hw error status
    //! \param  [in] status
    //!         Status report from HW
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HwStatusCheck(const DecodeStatusMfx &status)
    {
        return MOS_STATUS_SUCCESS;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    //!
    //! \brief  User feature key report for Sfc Linear Usage Status
    //! \param  [in] status
    //!         Status report from HW
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReportSfcLinearSurfaceUsage(const DecodeStatusReportData &reportData);
#endif

    //!
    //! \brief  Internal status check after each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StatusCheck();
#endif

protected:
    //!
    //! \brief  Create sub pipeline manager
    //! \param  [in] codecSettings
    //!         Point to codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateSubPipeLineManager(CodechalSetting* codecSettings);

    //!
    //! \brief  Create sub packet manager
    //! \param  [in] codecSettings
    //!         Point to codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateSubPacketManager(CodechalSetting* codecSettings);

    friend class DecodeSubPipelineManager;
    friend class DecodeStreamOut;

    CodechalHwInterfaceNext *m_hwInterface        = nullptr;  //!< Codechal HwInterface
    CodechalOcaDumper      *m_pCodechalOcaDumper = nullptr;
    CodechalDebugInterface* m_debugInterface = nullptr; //!< Debug Interface
    MediaTask *             m_task           = nullptr; //!< Command task

    DecodeSubPipelineManager* m_preSubPipeline = nullptr; //!< PreExecution sub pipeline
    DecodeSubPipelineManager *m_postSubPipeline = nullptr;  //!< PostExecution sub pipeline
    DecodeSubPacketManager*   m_subPacketManager = nullptr; //!< Sub packet manager

    DecodePipeMode          m_pipeMode = decodePipeModeBegin; //!< pipe mode

    DecodeAllocator *       m_allocator = nullptr;      //!< Resource allocator
    DecodeInputBitstream*   m_bitstream = nullptr;      //!< Decode input bitstream
    DecodeMemComp*          m_mmcState  = nullptr;      //!< Decode mmc state
    DecodeCpInterface*      m_decodecp = nullptr;       //!< DecodeCp interface
    DecodeStreamOut*        m_streamout = nullptr;      //!< Decode input bitstream

    uint8_t                 m_numVdbox  = 0;            //!< Number of Vdbox

    bool                    m_singleTaskPhaseSupported = true; //!< Indicates whether sumbit packets in single phase

    MOS_GPU_CONTEXT         m_decodeContext = MOS_GPU_CONTEXT_INVALID_HANDLE;    //!< decode context inuse
    GPU_CONTEXT_HANDLE      m_decodeContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;    //!< handle of decode context inuse

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t                m_statusCheckCount = 0;     //!< count for status check
    uint32_t                m_delayMiliseconds = 0;     //!< miliseconds delay after each frame
#endif

    PMOS_SURFACE            m_tempOutputSurf = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodePipeline)
};

}//decode

#endif // !__DECODE_PIPELINE_H__
