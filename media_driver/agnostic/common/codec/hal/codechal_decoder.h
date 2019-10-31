/*
* Copyright (c) 2011-2019, Intel Corporation
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
//! \file     codechal_decoder.h
//! \brief    Defines the decode interface for CodecHal.
//! \details  The decode interface is further sub-divided by standard, this file is for the base interface which is shared by all decode standards.
//!

#ifndef __CODECHAL_DECODER_H__
#define __CODECHAL_DECODER_H__

#include "codechal.h"
#include "codechal_setting.h"
#include "codechal_hw.h"
#include "codechal_debug.h"
#include "codechal_decode_downsampling.h"
#include "codechal_decode_sfc.h"
#include "codechal_mmc.h"
#include "codechal_utilities.h"
#include "codec_def_decode.h"
#include "cm_wrapper.h"
#include "media_perf_profiler.h"
#include "codec_def_cenc_decode.h"

class CodechalSecureDecodeInterface;
class CodechalDecodeHistogram;

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_DECODE sub-comp
//------------------------------------------------------------------------------
#define CODECHAL_DECODE_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _expr)

#define CODECHAL_DECODE_COND_ASSERTMESSAGE(_expr, _message, ...)                        \
    if (_expr)                                                                          \
    {                                                                                   \
        CODECHAL_DECODE_ASSERTMESSAGE(_message, ##__VA_ARGS__)                          \
    }

#define CODECHAL_DECODE_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _message, ##__VA_ARGS__)

#define CODECHAL_DECODE_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _message, ##__VA_ARGS__)

#define CODECHAL_DECODE_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _message, ##__VA_ARGS__)

#define CODECHAL_DECODE_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE)

#define CODECHAL_DECODE_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _stmt)

#define CODECHAL_DECODE_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _stmt)

#define CODECHAL_DECODE_CHK_STATUS_BREAK(_stmt)                                         \
    MOS_CHK_STATUS_BREAK(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _stmt)

#define CODECHAL_DECODE_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_DECODE_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _ptr)

#define CODECHAL_DECODE_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _ptr)

#define CODECHAL_DECODE_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _ptr)

#define CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _ptr)

#define CODECHAL_DECODE_CHK_COND(_expr, _message, ...)                                  \
    MOS_CHK_COND(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE,_expr,_message, ##__VA_ARGS__)

#define CODECHAL_DECODE_CHK_COND_RETURN(_expr, _message, ...)                           \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE,_expr,_message, ##__VA_ARGS__)

#define CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS 5

#define CODECHAL_DECODE_STATUS_NUM             512

typedef enum _CODECHAL_CS_ENGINE_ID_DEF
{
    // Instance ID
    CODECHAL_CS_INSTANCE_ID_VDBOX0 = 0,
    CODECHAL_CS_INSTANCE_ID_VDBOX1 = 1,
    CODECHAL_CS_INSTANCE_ID_VDBOX2 = 2,
    CODECHAL_CS_INSTANCE_ID_VDBOX3 = 3,
    CODECHAL_CS_INSTANCE_ID_VDBOX4 = 4,
    CODECHAL_CS_INSTANCE_ID_VDBOX5 = 5,
    CODECHAL_CS_INSTANCE_ID_VDBOX6 = 6,
    CODECHAL_CS_INSTANCE_ID_VDBOX7 = 7,
    CODECHAL_CS_INSTANCE_ID_MAX,
    // Class ID
    CODECHAL_CLASS_ID_VIDEO_ENGINE = 1,
} CODECHAL_CS_ENGINE_ID_DEF;

typedef union _CODECHAL_CS_ENGINE_ID
{
    struct
    {
        uint32_t       ClassId            : 3;    //[0...4]
        uint32_t       ReservedFiled1     : 1;    //[0]
        uint32_t       InstanceId         : 6;    //[0...7]
        uint32_t       ReservedField2     : 22;   //[0]
    } fields;
    uint32_t            value;
} CODECHAL_CS_ENGINE_ID, *PCODECHAL_CS_ENGINE_ID;

typedef struct _CODECHAL_VLD_SLICE_RECORD
{
    uint32_t   dwSkip;
    uint32_t   dwOffset;
    uint32_t   dwLength;
    uint32_t   dwSliceStartMbOffset;
    bool       bIsLastSlice;
} CODECHAL_VLD_SLICE_RECORD, *PCODECHAL_VLD_SLICE_RECORD;

//!
//! \struct CodechalDecodeStatusReport
//! \brief  Information pertaining to a particular picture's decode operation
//!
struct CodechalDecodeStatusReport
{
    //! \brief Status for the picture associated with this status report
    CODECHAL_STATUS         m_codecStatus = CODECHAL_STATUS_SUCCESSFUL;
    //! \brief Status report number associated with the picture in this status report provided in Execute()
    uint32_t                m_statusReportNumber = 0;
    //! \brief Uncompressed frame information for the picture associated with this status report
    CODEC_PICTURE           m_currDecodedPic = {0};
    //! \brief Applies for VC1 and MPEG2 only, uncompressed frame information for the out of loop deblock destination
    CODEC_PICTURE           m_currDeblockedPic = {0};
    //! \brief Pointer to the resource for the decode render target for the picture associated with this status report
    MOS_RESOURCE            m_currDecodedPicRes = {0};
    //! \brief Applies when debug dumps are enabled for VC1 only, resource of deblocked picture
    MOS_RESOURCE            m_deblockedPicResOlp = {0};
    //! \brief number of MBs decoded or if unused set to 0xFFFF
    uint16_t                m_numMbsAffected = 0;
    //! \brief Crc of frame from MMIO
    uint32_t                m_frameCrc = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
    //! \brief Applies when debug dumps are enabled, pointer to SFC output resource for the picture associated with this status report
    PMOS_RESOURCE           m_currSfcOutputPicRes = nullptr;
    //! \brief Applies when debug dumps are enabled, stream out buffer
    PMOS_RESOURCE           m_streamOutBuf = nullptr;
    //! \brief Applies when debug dumps are enabled, index of the streamout buffer
    uint32_t                m_streamoutIdx = 0;
    //! \brief Applies when debug dumps are enabled, indicates whether or not this is the final field in the frame.
    bool                    m_secondField = false;
    //! \brief Applies to VC1 only, indicates whether or not the frame required OLP.
    bool                    m_olpNeeded = false;
    //! \brief Applies when debug dumps are enabled, frame type (I/P/B)
    uint16_t                m_frameType = 0;
#endif // (_DEBUG || _RELEASE_INTERNAL)
};

//!
//! \struct CodechalDecodeStatus
//! \brief  Codechal decode status for the frame
//!
struct CodechalDecodeStatus
{
    //! \brief Value stored by MFX engine
    uint32_t                m_hwStoredData = 0;
    //! \brief SW(driver) stored value
    uint32_t                m_swStoredData = 0;
    //! \brief Value of MMIO decoding effor eStatus register
    uint32_t                m_mmioErrorStatusReg = 0;
    //! \brief Value of MMIO decoding MB error register
    uint32_t                m_mmioMBCountReg = 0;
    //! \brief Frame CRC related to current frames
    uint32_t                m_mmioFrameCrcReg = 0;
    //! \brief Value of MMIO CS Engine ID register for each BB
    uint32_t                m_mmioCsEngineIdReg[CODECHAL_CS_INSTANCE_ID_MAX] = {0};
    //! \brief Huc error for HEVC Fix Function, DWORD0: mask value, DWORD1: reg value
    uint64_t                m_hucErrorStatus2 = 0;
    //! \brief Huc error for HEVC Fix Function, DWORD0: mask value, DWORD1: reg value
    uint64_t                m_hucErrorStatus = 0;

    CodechalDecodeStatusReport m_decodeStatusReport;
};

//!
//! \struct CodechalDecodeStatusBuffer
//! \brief  Codechal decode status buffer
//!
struct CodechalDecodeStatusBuffer
{
    //! \brief Codechal decode status
    CodechalDecodeStatus    *m_decodeStatus = nullptr;
    //! \brief Handle of status buffer
    MOS_RESOURCE            m_statusBuffer = {0};
    //! \brief Locked data point of status buffer
    uint32_t                *m_data = nullptr;
    //! \brief Software store data
    uint32_t                m_swStoreData = 0;
    //! \brief First index for status buffer
    uint16_t                m_firstIndex = 0;
    //! \brief Current index for status buffer
    uint16_t                m_currIndex = 0;
    //! \brief Offset to store data
    uint8_t                 m_storeDataOffset = 0;
    //! \brief Offset to decode error status
    uint8_t                 m_decErrorStatusOffset = 0;
    //! \brief Offset to decode frame CRC
    uint8_t                 m_decFrameCrcOffset = 0;
    //! \brief Offset to decode MB count
    uint8_t                 m_decMBCountOffset = 0;
    //! \brief Offset to CS engine ID
    uint8_t                 m_csEngineIdOffset = 0;

    //! \brief Offset to mask of MMIO HuCErrorStatus2
    uint8_t                 m_hucErrorStatus2MaskOffset = 0;
    //! \brief Offset to MMIO HuCErrorStatus2
    uint8_t                 m_hucErrorStatus2RegOffset = 0;
    //! \brief Offset to mask of MMIO HuCErrorStatus
    uint8_t                 m_hucErrorStatusMaskOffset = 0;
    //! \brief Offset to MMIO HuCErrorStatus
    uint8_t                 m_hucErrorStatusRegOffset = 0;
};

//!
//! \class CodechalDecode
//! \brief This class defines the common member fields, functions etc as decode base class.
//!
class CodechalDecode : public Codechal
{
public:
    //!
    //! \enum   CodechalHcpDecodePhase
    //! \brief  enum constant for HEVC/VP9 decode pass control
    //!
    enum CodechalHcpDecodePhase
    {
        CodechalHcpDecodePhaseInitialized = 0x00,   //!< Initial phase
        CodechalHcpDecodePhaseLegacyLong,           //!< Legacy long format phase
        CodechalHcpDecodePhaseLegacyS2L,            //!< Legacy short to long phase
        CodechalHcpDecodePhaseMax                   //!< Maximal phases
    };

    //!
    //! \enum  CodechalDecodeMotionType
    //! \brief Codechal decode motion type
    //!
    enum CodechalDecodeMotionType
    {
        CodechalDecodeMcField   = 1,    //!< Field motion type
        CodechalDecodeMcFrame   = 2,    //!< Frame motion type
        CodechalDecodeMc16x8    = 2,    //!< 16x8 motion type
        CodechalDecodeMcDmv     = 3     //!< DMV motion type
    };

    //!
    //! \enum  CodechalDecodeMvPacking
    //! \brief For motion vector packing: the equivilant derefences of a [2][2][2] array mapped as a [8] array
    //!
    enum CodechalDecodeMvPacking
    {
        CodechalDecodeRstFirstForwHorz = 0, //!< first forward horizontal
        CodechalDecodeRstFirstForwVert = 1, //!< first forward vertical
        CodechalDecodeRstFirstBackHorz = 2, //!< first backward horizontal
        CodechalDecodeRstFirstBackVert = 3, //!< first backward vertical
        CodechalDecodeRstSecndForwHorz = 4, //!< second forward horizontal
        CodechalDecodeRstSecndForwVert = 5, //!< second forward vertical
        CodechalDecodeRstSecndBackHorz = 6, //!< second backward horizontal
        CodechalDecodeRstSecndBackVert = 7  //!< second backward vertical
    };

    //!
    //! \enum  CodechalDecodeRefAddrIndex
    //! \brief Reference address indexes
    //!
    enum CodechalDecodeRefAddrIndex
    {
        // MPEG2/VC1 reference address indexes
        CodechalDecodeFwdRefTop     = 0,    //!< forward reference top field
        CodechalDecodeBwdRefTop     = 1,    //!< backward reference top field
        CodechalDecodeFwdRefBottom  = 2,    //!< forward reference bottom field
        CodechalDecodeBwdRefBottom  = 3,    //!< backward reference bottom field
        // VP8/VP9 reference address indexes
        CodechalDecodeLastRef       = 0,    //!< last reference
        CodechalDecodeGoldenRef     = 1,    //!< golden reference
        CodechalDecodeAlternateRef  = 2     //!< alternate reference
    };

    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecode(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecode(const CodechalDecode&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecode& operator=(const CodechalDecode&) = delete;

    //!
    //! \brief  Destructor
    //!
    virtual ~CodechalDecode();

    //!
    //! \brief  Get picture width for decode
    //! \return Width value
    //!
    virtual uint32_t GetWidth() { return m_width; }

    //!
    //! \brief  Get picture height for decode
    //! \return Height value
    //!
    virtual uint32_t GetHeight() { return m_height; }

    //!
    //! \brief  Help function to allocate a 1D linear buffer for each decode standard
    //! \param  [in,out] resource
    //!         Pointer to allocated buffer
    //! \param  [in] size
    //!         Buffer size
    //! \param  [in] name
    //!         Buffer name
    //! \param  [in] initialize
    //!         Initialization flag, by default is false
    //! \param  [in] value
    //!         Initialization value when intialize flag is true, by default is 0
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer(
        PMOS_RESOURCE resource,
        uint32_t size,
        const char* name,
        bool initialize = false,
        uint8_t value = 0,
        bool bPersistent = false);

    //!
    //! \brief    Help function to allocate a NV12 TILE_Y surface
    //! \details  Help function to allocate a NV12 TILE_Y surface for each decode standard
    //!
    //! \param    [in,out] surface
    //!           Pointer to allocated surface
    //! \param    [in] width
    //!           Surface width
    //! \param    [in] height
    //!           Surface height
    //! \param    [in] name
    //!           Surface name
    //! \param    [in] format
    //!           Surface format, by default is NV12
    //! \param    [in] isCompressible
    //!           Compressible flag, by default is false
    MOS_STATUS AllocateSurface(
        PMOS_SURFACE surface,
        uint32_t width,
        uint32_t height,
        const char* name,
        MOS_FORMAT format = Format_NV12,
        bool isCompressible = false);

    //!
    //! \brief  Entry to allocate and intialize the decode instance
    //! \param  [in] codecHalSettings
    //!         The settings to inialize the decode instance
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Allocate(CodechalSetting * codecHalSettings) override;

    //!
    //! \brief  The handle at the end of each frame.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EndFrame() override;

    //!
    //! \brief  The entry to decode each frame.
    //! \param  [in] params
    //!         Pointer to decode parameters of this frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute(void *params) override;

    //!
    //! \brief  Inserts the generic prologue command for a command buffer
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \param  [in] frameTrackingRequested
    //!         frame Tracking Requested
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                frameTrackingRequested);

    //!
    //! \brief  Inserts predication command for a command buffer
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendPredicationCommand(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Inserts marker commands for a command buffer
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \param  [in] isRender
    //!         Whether render engine workload or not
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMarkerCommand(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool isRender);

    //!
    //! \brief  The entry to get status report.
    //! \param  [out] status
    //!         The point to decode status
    //! \param  [in] numStatus
    //!         The requested number of status reports
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetStatusReport(
        void        *status,
        uint16_t    numStatus) override;

    //!
    //! \brief  Reset status report for GEN specific decoder
    //! \param  [in] nullHwInUse
    //!         Indicates whether or not null hardware is inuse
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResetStatusReport(
        bool nullHwInUse);

    //!
    //! \brief    Help function to copy resource
    //! \details  Help function to copy resource via Huc stream out function.
    //! \param    [out] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] src
    //!           Pointer to source resource
    //! \param    [out] dst
    //!           Pointer to destination resource
    //! \param    [in] copyLength
    //!           The copy length starts from source offset
    //! \param    [in] copyInputOffset
    //!           The copy offset relative to source offset, by default is 0.
    //! \param    [in] copyOutputOffset
    //!           The output offset relative to destination offset, by default is 0.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HucCopy(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst,
        uint32_t copyLength,
        uint32_t copyInputOffset = 0,
        uint32_t copyOutputOffset = 0);

    //!
    //! \brief  Gets the decode mode
    //! \return The decode mode \see m_mode
    //!
    uint32_t GetMode() { return m_mode; }

    //!
    //! \brief  Gets the decode standard
    //! \return The decode standard \see m_standard
    //!
    uint32_t GetStandard() { return m_standard; }

    //!
    //! \brief  Gets video context
    //! \return The video context \see m_videoContext
    //!
    MOS_GPU_CONTEXT GetVideoContext() { return m_videoContext; }

    //!
    //! \brief  Sets video context
    //! \return The video context \see m_videoContext
    //!
    void SetVideoContext(MOS_GPU_CONTEXT context) { m_videoContext = context; }

    //!
    //! \brief  Gets video WA context
    //! \return The video WA context \see m_videoContextForWa
    //!
    MOS_GPU_CONTEXT GetVideoWAContext() { return m_videoContextForWa; }

    //!
    //! \brief  Sets cenc decoder batch buffer
    //! \param    [in] cmdBuffer
    //!           Pointer of command buffer.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCencBatchBuffer( PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Indicates whether or not the status query reporting is enabled
    //! \return If status query reporting is enabled \see m_statusQueryReportingEnabled
    //!
    bool IsStatusQueryReportingEnabled() { return m_statusQueryReportingEnabled; }

    //!
    //! \brief  Gets decode status buffer
    //! \return The decode status buffer \see m_decodeStatusBuf
    //!
    CodechalDecodeStatusBuffer *GetDecodeStatusBuf() { return &m_decodeStatusBuf; }

    //!
    //! \brief  Gets vdbox index
    //! \return The vdbox index \see m_vdboxIndex
    //!
    MHW_VDBOX_NODE_IND  GetVdboxIndex() { return m_vdboxIndex; }

    //!
    //! \brief  Indicates whether or not the first execute call
    //! \return If first execute call \see m_executeCallIndex
    //!
    inline bool IsFirstExecuteCall() { return (m_executeCallIndex == 0); }

    //!
    //! \brief  Indicates whether or not the decoder is inuse
    //! \return If decoder is inuse \see m_isHybridDecoder
    //!
    bool IsHybridDecoder() { return m_isHybridDecoder; }

    //!
    //! \brief  Indicates whether or not the picture is incomplete
    //! \return If picture is incomplete \see m_incompletePicture
    //!
    bool IsIncompletePicture() { return m_incompletePicture; }

    //!
    //! \brief  Indicates whether or not the jpeg scan is incomplete
    //! \return If jpeg scan is incomplete \see m_incompleteJpegScan
    //!
    virtual bool IsIncompleteJpegScan() { return false; }

    //!
    //! \brief  Gets flags which indicates whether video context uses null hardware
    //! \return Flags which indicates whether video context uses null hardware \see m_videoContextUsesNullHw
    //!
    bool GetVideoContextUsesNullHw() { return m_videoContextUsesNullHw; }

    //!
    //! \brief  Gets flags which indicates whether video context for using null hardware
    //! \return Flags which indicates whether video context for using null hardware \see m_videoContextForWaUsesNullHw
    //!
    bool GetVideoContextForWaUsesNullHw() { return m_videoContextForWaUsesNullHw; }

    //!
    //! \brief  Gets flags which indicates whether render context uses null hardware
    //! \return Flags which indicates whether render context uses null hardware \see m_renderContextUsesNullHw
    //!
    bool GetRenderContextUsesNullHw() { return m_renderContextUsesNullHw; }

    //!
    //! \brief  Set decode histogram
    //! \return No return
    //!
    void SetDecodeHistogram(CodechalDecodeHistogram *decodeHistogram) { m_decodeHistogram = decodeHistogram; }

    //!
    //! \brief  Get decode histogram
    //! \return Pointer of codechal decode histogram
    //!
    CodechalDecodeHistogram* GetDecodeHistogram() { return m_decodeHistogram; }

#ifdef _DECODE_PROCESSING_SUPPORTED
    //!
    //! \brief  Indicates whether or not the vd sfc is supported
    //! \return If vd sfc is supported \see m_vdSfcSupported
    //!
    bool IsVdSfcSupported() { return m_vdSfcSupported; }

    //!
    //! \brief  Set if vd sfc supported
    //! \return No return
    //!
    void SetVdSfcSupportedFlag(bool isVdSfcSpported) { m_vdSfcSupported = isVdSfcSpported; }

    //! \brief Field scaling interface
    FieldScalingInterface       *m_fieldScalingInterface = nullptr;
#endif

    //!
    //! \brief  Get dummy reference surface
    //! \return Pointer of reference surface
    //!
    MOS_SURFACE* GetDummyReference() { return &m_dummyReference; }

    //!
    //! \brief  Get dummy reference status
    //! \return CODECHAL_DUMMY_REFERENCE_STATUS
    //!
    CODECHAL_DUMMY_REFERENCE_STATUS GetDummyReferenceStatus() { return m_dummyReferenceStatus; }

    //!
    //! \brief  Set dummy reference status
    //! \return void
    //!
    void SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status)
    {
        m_dummyReferenceStatus = status;
    }

protected:

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetGpuCtxCreatOption(CodechalSetting * settings);

    //!
    //! \brief  Allocate and initialize GEN specific decoder standard
    //! \param  [in] settings
    //!         Pointer to CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateStandard(CodechalSetting * settings)   = 0;

    //!
    //! \brief  Set states for each frame to prepare for decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetFrameStates()     = 0;

    //!
    //! \brief  State level function for standard specific decoder
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DecodeStateLevel()     = 0;

    //!
    //! \brief  Primitive level function for GEN specific decoder
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DecodePrimitiveLevel() = 0;

    //!
    //! \brief  Calculate downsample param for GEN specific decoder
    //! \param  [in] picParams
    //!         Pointer to picture parameter
    //! \param  [in] refSurfWidth
    //!         Reference surface width
    //! \param  [in] refSurfHeight
    //!         Reference surface height
    //! \param  [in] format
    //!         Pointer to MOS_FORMAT
    //! \param  [in] frameIdx
    //!         Frame index
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalcDownsamplingParams(
        void *picParams,
        uint32_t *refSurfWidth,
        uint32_t *refSurfHeight,
        MOS_FORMAT *format,
        uint8_t *frameIdx)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Downsampling for current Codec !");
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Get decoder status for GEN specific hybird decoder
    //! \param  [in] decodeStatus
    //!         Decode status
    //! \param  [in] index
    //!         Index of status buffer
    //! \param  [in] defaultStatus
    //!         Default status
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DecodeGetHybridStatus(
        CodechalDecodeStatus *decodeStatus,
        uint32_t index,
        uint32_t defaultStatus)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Start status report for GEN specific decoder
    //! \param  [out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StartStatusReport(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  End status report for GEN specific decoder
    //! \param  [in] decodeStatusReport
    //!         Decode status
    //! \param  [out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndStatusReport(
        CodechalDecodeStatusReport &decodeStatusReport,
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Initialize MMC state for specified decode device
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMmcState()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Linear to Y tiled address
    //!
    //! \param  [in] x
    //!         Position of X
    //! \param  [in] y
    //!         Position of Y
    //! \param  [in] pitch
    //!         Pitch
    //! \return uint32_t
    //!         Return 0 if success, else -1 if fail
    //!
    uint32_t LinearToYTiledAddress(
        uint32_t x,
        uint32_t y,
        uint32_t pitch);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpProcessingParams(
        PCODECHAL_DECODE_PROCESSING_PARAMS decProcParams);

#endif

private:

    //!
    //! \brief    Calculate command buffer size needed for picture level and slice level commands
    //! \param    [out] requestedSize
    //!           Return command buffer size for picture level and slice level command
    //! \param    [out] additionalSizeNeeded
    //!           Return additianl size needed
    //! \param    [out] requestedPatchListSize
    //!           return patch list size used in this command buffer
    //! \return   None
    //!
    virtual void CalcRequestedSpace(
        uint32_t       &requestedSize,
        uint32_t       &additionalSizeNeeded,
        uint32_t       &requestedPatchListSize);

    //!
    //! \brief  Verify command buffer size and patch list size, reallocate if required
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VerifySpaceAvailable ();

    //!
    //! \brief  Create GPU context for GEN specific decoder
    //! \param  [in] codecHalSettings
    //!         Pointer to CODECHAL Settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateGpuContexts(
        CodechalSetting * codecHalSettings);

    //!
    //! \brief  Indicates whether or not the SFC is inuse
    //! \param  [in] codecHalSettings
    //!         Pointer to CODECHAL Settings
    //! \return If SFC is inuse
    //!
    virtual bool IsSfcInUse(CodechalSetting * codecHalSettings) { return false; }

    //!
    //! \brief  Indicates whether or not the frame level multiple thread is enable
    //! \return If frame level multiple thread is enable
    //!
    virtual bool IsFrameMTEnabled() { return false; }

    //!
    //! \brief  The virtual function for decode standard to override the requested space size
    //! \param  [in] requestedSize
    //!         The intial request size computed by picture level and slice level
    //! \return The final requested space size
    //!
    virtual uint32_t RequestedSpaceSize(uint32_t requestedSize) { return requestedSize; }

    //!
    //! \brief  The virtual function for decode standard to override the extra requested space size
    //! \param  [in] requestedSize
    //!         The intial request size computed by picture level and slice level
    //! \param  [in] additionalSizeNeeded
    //!         The additional request size for command buffer
    //! \return The extra requested space size
    //!
    virtual MOS_STATUS VerifyExtraSpace(
        uint32_t requestedSize,
        uint32_t additionalSizeNeeded)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Allocate reference surfaces
    //! \details  Allocate reference surfaces for decode downsampling for all codec types
    //! \param    allocWidth
    //!           [in] Width of the surfaces to be allocated
    //! \param    allocHeight
    //!           [in] Height of the surfaces to be allocated
    //! \param    format
    //!           [in] Flag to indicate the format of the surfaces to be allocated
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateRefSurfaces(
        uint32_t allocWidth,
        uint32_t allocHeight,
        MOS_FORMAT format);
    //!
    //! \brief    Resize specific reference surfaces
    //! \details  Resize specific reference surfaces for decode downsampling for all codec types
    //! \param    frameIdx
    //!           [in] index of surfaces array
    //! \param    width
    //!           [in] Width of the surfaces to be allocated
    //! \param    height
    //!           [in] Height of the surfaces to be allocated
    //! \param    format
    //!           [in] Flag to indicate the format of the surfaces to be allocated
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
   MOS_STATUS RefSurfacesResize(
        uint32_t     frameIdx,
        uint32_t     width,
        uint32_t     height,
        MOS_FORMAT   format);
    //!
    //! \brief    Deallocate specific reference surfaces
    //! \details  Deallocate specific reference surfaces for decode downsampling for all codec types
    //! \param    frameIdx
    //!           [in] index of surfaces array
    //! \return   N/A
    //!
    void DeallocateSpecificRefSurfaces(uint32_t frameIdx);
    //!
    //! \brief    Deallocate reference surfaces
    //! \details  Deallocate reference surfaces for decode downsampling for all codec types
    //! \return   N/A
    //!
    void DeallocateRefSurfaces();

    //!
    //! \brief    Set dummy reference
    //! \details  Set dummy reference for error concealment
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDummyReference();

protected:
    //! \brief Mfx Interface
    MhwVdboxMfxInterface        *m_mfxInterface     = nullptr;
    //! \brief Hcp Interface
    MhwVdboxHcpInterface        *m_hcpInterface     = nullptr;
    //! \brief Huc Interface
    MhwVdboxHucInterface        *m_hucInterface     = nullptr;
    //! \brief Vdenc Interface
    MhwVdboxVdencInterface      *m_vdencInterface   = nullptr;
    //! \brief Common Mi Interface
    MhwMiInterface              *m_miInterface      = nullptr;
    //! \brief Cp Interface
    MhwCpInterface              *m_cpInterface      = nullptr;

    //! \brief Security Decode
    CodechalSecureDecodeInterface *m_secureDecoder    = nullptr;

    //! \brief WA table
    MEDIA_WA_TABLE              *m_waTable           = nullptr;
    //! \brief SKU table
    MEDIA_FEATURE_TABLE         *m_skuTable          = nullptr;

    //!< mmc state
    CodecHalMmcState            *m_mmc              = nullptr;
    //! \brief Decode parameters
    CodechalDecodeParams        m_decodeParams;
    //! \brief Decode mode
    uint32_t                    m_mode              = CODECHAL_UNSUPPORTED_MODE;
    //! \brief Decode standard
    uint32_t                    m_standard          = CODECHAL_UNDEFINED;
    //! \brief Current frame number
    uint32_t                    m_frameNum          = 0;
    //! \brief Indicates if current field is second field(bottom field)
    bool                        m_secondField       = false;
    //! \brief picture information of current render target
    CODEC_PICTURE               m_crrPic            = {0};
    //! \brief Video GPU node inuse
    MOS_GPU_NODE                m_videoGpuNode      = MOS_GPU_NODE_MAX;
    //! \brief Video context inuse
    MOS_GPU_CONTEXT             m_videoContext      = MOS_GPU_CONTEXT_INVALID_HANDLE;
    //! \brief Video WA context inuse
    MOS_GPU_CONTEXT             m_videoContextForWa = MOS_GPU_CONTEXT_INVALID_HANDLE;
    //! \brief Render context inuse
    MOS_GPU_CONTEXT             m_renderContext     = MOS_GPU_CONTEXT_RENDER;
    //! \brief Picture Width
    uint32_t                    m_width             = 0;
    //! \brief Picture Height
    uint32_t                    m_height            = 0;

    //! \brief Picture level command buffer size is required
    uint32_t                    m_commandBufferSizeNeeded = 0;
    //! \brief Picture level patch list size is required
    uint32_t                    m_commandPatchListSizeNeeded = 0;
    //! \brief Slice level command buffer size is required
    uint32_t                    m_standardDecodeSizeNeeded = 0;
    //! \brief Slice level patch list size is required
    uint32_t                    m_standardDecodePatchListSizeNeeded = 0;
    //! \brief Indicates if current input bitstream is incomplete
    bool                        m_incompletePicture = false;
    //! \brief The index of execution call in multiple execution call mode
    bool                        m_executeCallIndex  = 0;
    //! \brief Indicates if current is frist execution call in multiple execution call mode
    bool                        m_consecutiveMbErrorConcealmentInUse = false;
    //! \brief Indicates if phantom MBs is required for MPEG2 decode
    bool                        m_decodePhantomMbs  = false;

    //! \brief Flag to indicate if we support eStatus query reporting on current platform
    bool                        m_statusQueryReportingEnabled = false;
    //! \brief Flag to indicate if UMD Perf Profiler FE BE timing measurement is enabled
    bool                        m_perfFEBETimingEnabled = false;

#ifdef _DECODE_PROCESSING_SUPPORTED
    //! \brief Flag to indicate if vd sfc is supported
    bool                        m_vdSfcSupported = false;
#endif

    //! \brief Stores all the status_query related data
    CodechalDecodeStatusBuffer  m_decodeStatusBuf;
    //! \brief The feedback number reported by app in picparams call
    uint32_t                    m_statusReportFeedbackNumber = 0;
    //! \brief Flag to indicate if report frame CRC
    bool                        m_reportFrameCrc = false;

    //! \brief Indicates if stream out enabled
    bool                        m_streamOutEnabled  = false;
    //! \brief Stream out buffers
    MOS_RESOURCE                m_streamOutBuffer[CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS] = {{0}};
    //! \brief Indicates if stream out buffer is inuse with same index
    uint32_t                    m_streamOutCurrStatusIdx[CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS] = {CODECHAL_DECODE_STATUS_NUM};
    //! \brief Current stream out buffer index
    uint32_t                    m_streamOutCurrBufIdx = 0;

    //! \brief Performance testing parameters
    uint16_t                    m_perfType          = 0;

    //! \brief Indicates if video context uses null hardware
    bool                        m_videoContextUsesNullHw = false;
    //! \brief Indicates if video WA context uses null hardware
    bool                        m_videoContextForWaUsesNullHw = false;
    //! \brief Indicates if render context uses null hardware
    bool                        m_renderContextUsesNullHw = false;

    //! \brief Indicates if decode sync lock is disabled
    bool                        m_disableDecodeSyncLock = false;
    //! \brief Indicates if transcoe lock is disabled
    bool                        m_disableLockForTranscode = false;

    //! \brief Indicate how many passes is needed to finish decoding a picture
    //! \details Initialize decode pass number to 1, for those decoder need more than 1 decoding pass,
    //!          modify this value in specific decoder files.
    uint16_t                     m_decodePassNum     = 1;

    //! \brief MMIO Hcp Frame CRC report offset
    uint32_t                    m_hcpFrameCrcRegOffset = 0;

    //! \brief The vdbox index for current frame
    MHW_VDBOX_NODE_IND          m_vdboxIndex = MHW_VDBOX_NODE_1;
    //! \brief Indicates if HCP is inuse
    bool                        m_hcpInUse          = false;

    //! \brief Indicates if decoder is inuse
    bool                        m_isHybridDecoder   = false;

    //! \brief Indicates if downsampling is required
    bool                        m_downsamplingHinted = false;
    //! \brief Reference surface for downsampling
    PMOS_SURFACE                m_refSurfaces       = nullptr;
    //! \brief Number of reference surface for downsampling
    uint32_t                    m_refFrmCnt         = 0;

    //! \brief Internal buffer for predication
    MOS_RESOURCE               m_predicationBuffer = { 0 };

#if (_DEBUG || _RELEASE_INTERNAL)
    //! \brief Downsampled surfaces
    PMOS_SURFACE                m_downsampledSurfaces = nullptr;
#endif

    //! \brief    Decode histogram interface
    //! \details  Support YUV Luma histogram.
    CodechalDecodeHistogram    *m_decodeHistogram = nullptr;
    PMOS_GPUCTX_CREATOPTIONS   m_gpuCtxCreatOpt = nullptr;

    //! \brief Performance data profiler
    MediaPerfProfiler           *m_perfProfiler    = nullptr;

    // CencDecode buffer
    CencDecodeShareBuf          *m_cencBuf    = nullptr;

    //! \brief Dummy reference surface
    MOS_SURFACE                 m_dummyReference;

    //! \brief Indicate the status of dummy reference
    CODECHAL_DUMMY_REFERENCE_STATUS m_dummyReferenceStatus = CODECHAL_DUMMY_REFERENCE_INVALID;
};

#endif  // __CODECHAL_DECODER_H__
