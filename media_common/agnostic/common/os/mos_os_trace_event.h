/*
* Copyright (c) 2015-2023, Intel Corporation
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
//! \file     mos_os_trace_event.h
//! \brief    MOS trace event header file.
//! \details  only contain trace event id and type definition, which will keep
//!           growing.
//!

#ifndef __MOS_OS_TRACE_H__
#define __MOS_OS_TRACE_H__

//!
//! \brief helper to expand trace event data from dynmac size Marco
//!

#define _TR_PRE(x) x
#define _TR_CAT_(a, b) a##b
#define _TR_CAT(a, b) _TR_CAT_(a, b)

//!
//! \brief internal marco to calc the number of args in __VA_ARGS__, support count upto 10 args
//! Due to C99 compiler definition, _TR_COUNT_() return 1. need new marco to use.
//!
#define _TR_CNT_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, cnt, ...) cnt
#define _TR_COUNT(...) _TR_PRE(_TR_CNT_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

//!
//! \brief internal marco to calc byte size of all args, and memcpy to pre-defined buffer
//!
#define _TR_OPsize(x)  +sizeof(x)
#define _TR_OPfield(x) {auto _t=x;memcpy(_buf + _i, &(_t), sizeof(x));_i += sizeof(x);}

//!
//! \brief internal marco to calc byte size of all args, and memcpy to pre-defined buffer
//!
#define _TR_EXPAND1(o, x, ...) _TR_PRE(_TR_CAT(_TR_OP, o)(x))
#define _TR_EXPAND2(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND1(o, __VA_ARGS__))
#define _TR_EXPAND3(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND2(o, __VA_ARGS__))
#define _TR_EXPAND4(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND3(o, __VA_ARGS__))
#define _TR_EXPAND5(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND4(o, __VA_ARGS__))
#define _TR_EXPAND6(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND5(o, __VA_ARGS__))
#define _TR_EXPAND7(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND6(o, __VA_ARGS__))
#define _TR_EXPAND8(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND7(o, __VA_ARGS__))
#define _TR_EXPAND9(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND8(o, __VA_ARGS__))
#define _TR_EXPAND10(o, x, ...) _TR_EXPAND1(o, x) _TR_PRE(_TR_EXPAND9(o, __VA_ARGS__))
// Note: increase _TR_EXPANDxx to support more param

#define _TR_PARAM_SIZE(...) _TR_PRE(_TR_CAT(_TR_EXPAND, _TR_COUNT(__VA_ARGS__))(size, __VA_ARGS__))
#define _TR_PARAM_FIELD(...) _TR_PRE(_TR_CAT(_TR_EXPAND, _TR_COUNT(__VA_ARGS__))(field, __VA_ARGS__))

//!
//! \brief trace marco interface, for external usage, split 2 marco for customzed trace write interface
//!
#define TR_FILL_PARAM(...)                       \
    int  _i = 0;                                 \
    char _buf[0 _TR_PARAM_SIZE(__VA_ARGS__)];    \
    _TR_PARAM_FIELD(__VA_ARGS__);

#define TR_WRITE_PARAM(func, id, op)             \
    func(id, op, _buf, sizeof(_buf), nullptr, 0);

//!
//! \brief Keyword for ETW tracing, 1bit per keyworld, total 64bits
//!
typedef enum _MEDIA_EVENT_FILTER_KEYID
{
    TR_KEY_DECODE_PICPARAM = 0,
    TR_KEY_DECODE_SLICEPARAM,
    TR_KEY_DECODE_TILEPARAM,
    TR_KEY_DECODE_QMATRIX,
    TR_KEY_DECODE_BITSTREAM_INFO,
    TR_KEY_DECODE_BITSTREAM,
    TR_KEY_DECODE_INTERNAL,
    TR_KEY_DECODE_COMMAND,
    TR_KEY_DECODE_DSTYUV,
    TR_KEY_DECODE_REFYUV,
    TR_KEY_DECODE_MV,
    TR_KEY_DECODE_SUBSET,
    TR_KEY_MOSMSG_ALL = 12,
    TR_KEY_CALL_STACK,
    TR_KEY_DATA_DUMP  = 16,
    TR_KEY_MOSMSG_CP,
    TR_KEY_MOSMSG_VP,
    TR_KEY_MOSMSG_CODEC,
    TR_KEY_MOSMSG_DDI,
    TR_KEY_MOSMSG_MOS,
    TR_KEY_MOSMSG_MHW,
    TR_KEY_DECODE_INFO,
    TR_KEY_ENCODE_EVENT_DDI = 24,
    TR_KEY_ENCODE_EVENT_API_STICKER,
    TR_KEY_ENCODE_EVENT_INTERNAL,
    TR_KEY_ENCODE_DATA_INPUT_SURFACE,
    TR_KEY_ENCODE_DATA_REF_SURFACE,
    TR_KEY_ENCODE_DATA_RECON_SURFACE,
    TR_KEY_ENCODE_DATA_BITSTREAM,
    TR_KEY_DECODE_DSTYUV_IN_TRACE,
    TR_KEY_ENCODE_DATA_HUC_DMEM,
    TR_KEY_ENCODE_DATA_HUC_REGION,
} MEDIA_EVENT_FILTER_KEYID;

#pragma pack(push, 8)
struct MtSetting
{
    struct FastDump
    {
        uint8_t  allowDataLoss : 1;
        uint8_t  frameIdxBasedSampling : 1;
        uint8_t  memUsagePolicy : 2;
        uint8_t  writeMode : 2;
        uint8_t  informOnError : 1;
        uint8_t  rsv0 : 1;
        uint8_t  rsv1;
        uint8_t  maxPrioritizedMem;
        uint8_t  maxDeprioritizedMem;
        uint8_t  weightRenderCopy;
        uint8_t  weightVECopy;
        uint8_t  weightBLTCopy;
        uint8_t  rsv2;
        uint64_t samplingTime;
        uint64_t samplingInterval;
        uint64_t bufferSize;
        char     filePath[1024];
    } fastDump;

    struct HwcmdParser
    {
        char filePath[1024];
    } hwcmdParser;

    uint8_t rsv[1504];
};

// 4KB in total
struct MtControlData
{
    uint32_t enable;
    uint8_t  level;
    uint8_t  rsv[3];
    uint64_t filter[63];

    MtSetting setting;
};
#pragma pack(pop)

enum class MT_EVENT_LEVEL
{
    ALWAYS,
    CRITICAL,
    ERR,
    WARNING,
    INFO,
    VERBOSE,
};

enum class MT_DATA_LEVEL
{
    FIRST_64B,  // dump first min(DataSize, 64) bytes
    QUARTER,    // dump first min(DataSize, max(DataSize/4, 64)) bytes
    HALF,       // dump first min(DataSize, max(DataSize/2, 64)) bytes
    FULL,       // dump all data
};

enum class MT_LOG_LEVEL
{
    ALWAYS,
    CRITICAL,
    NORMAL,
    VERBOSE,
    FUNCTION_ENTRY,
    FUNCTION_EXIT,
    FUNCTION_ENTRY_VERBOSE,
    MEMNINJA,
};

typedef enum _MEDIA_EVENT
{
    UNDEFINED_EVENT = 0,            //! reserved id, should not used in driver
    EVENT_RESOURCE_ALLOCATE,        //! event for MOS resource allocate
    EVENT_RESOURCE_FREE,            //! event for MOS resource free
    EVENT_RESOURCE_REGISTER,        //! event for register MOS resource to gpu command
    EVENT_RESOURCE_PATCH,           //! event for patch MOS resource
    EVENT_PPED_HUC,                 //! event for PPED HuC path
    EVENT_PPED_FW,                  //! event for PPED FW path
    EVENT_PPED_AUDIO,               //! event for PPED audio path
    EVENT_BLT_ENC,                  //! event for blt enc mode
    EVENT_BLT_DEC,                  //! event for blt dec mode
    EVENT_PPED_HW_CAPS,             //! event for PPED HW capability
    EVENT_MOS_MESSAGE,              //! event for MOS debug message
    EVENT_CODEC_NV12ToP010,         //! event for NV12 to P010 in codechal
    EVENT_CODEC_CENC,               //! event for CENC Decode
    EVENT_CODEC_DECODE_DDI,         //! event for Decoder decode DDI level
    EVENT_CODEC_DECODE,             //! event for Decoder decode
    EVENT_CODEC_ENCODE_DDI,         //! event for Decoder encode DDI level
    EVENT_ENCODER_CREATE,           //! event for encoder create
    EVENT_ENCODER_DESTROY,          //! event for encoder destory.
    EVENT_CODECHAL_CREATE,          //! event for codechal create.
    EVENT_CODECHAL_EXECUTE,         //! event for codechal execute.
    EVENT_CODECHAL_DESTROY,         //! event for codechal destory.
    EVENT_MHW_PROLOG,               //! event for MHW GPU cmd prolog.
    EVENT_MHW_EPILOG,               //! event for MHW GPU cmd epilog.
    EVENT_KEYEXCHANGE_WV,           //! event for WV key exchange
    EVENT_TEST1,                    //! event for immediate event trace usage in debug.
    EVENT_TEST2,                    //! avoid build manifest for temp event.
    EVENT_TEST3,                    //! pre allocate 3 events.
    EVENT_CP_CREATE,                //! event for cp session create
    EVENT_CP_DESTROY,               //! event for cp session destroy
    EVENT_HECI_IOMSG,               //! event for heci IO message send receive
    EVENT_CP_CHECK_SESSION_STATUS,  //! event for cp session status check
    EVENT_CP_RESOURCR_SESSION,      //! event for cp resource session create
    EVENT_PREPARE_RESOURCES,        //! event for prepare resource
    EVENT_DDI_CODEC_CREATE,         //! event for ddi tracking - codec create
    EVENT_DDI_CODEC_DESTROY,        //! event for ddi tracking - codec destroy
    EVENT_DDI_CODEC_VIEW,           //! event for ddi tracking - codec view
    EVENT_DDI_VP_CREATE,            //! event for ddi tracking - vp create
    EVENT_DDI_VP_DESTROY,           //! event for ddi tracking - vp destroy
    EVENT_DDI_VP_VIEW,              //! event for ddi tracking - vp view
    EVENT_DDI_VP_BLT,               //! event for ddi tracking - vp blt
    EVENT_DDI_VP_BLT_SETSTATE,      //! event for ddi tracking - vp blt stream state
    EVENT_DDI_VPHAL_REPORT,         //! event for ddi tracking - vp hal report
    EVENT_DDI_VP_BLT_HINTS,         //! event for ddi tracking - vp blt hints
    EVENT_CP_CERT_COUNT,            //! event for number of certs
    EVENT_CP_CERT_NOT_FOUND,        //! event for certificate not found
    EVENT_DDI_VIDEOVIEW_CLEAR,      //! event for ddi tracking - ClearVideoView
    EVENT_DDE_FUNCTION,             //! event for function enter/exit
    EVENT_DDE_QUERY_HDCP_INTERFACE, //! event for cp ddi
    EVENT_DDE_CB_REPORT_AUTH_RESULT,
    EVENT_DDE_ESCAPE,
    EVENT_DDE_AKE_INIT,             //! event for HDCP 2 messages
    EVENT_DDE_AKE_SEND_CERT,
    EVENT_DDE_AKE_NOSTORED_KM,
    EVENT_DDE_AKE_STORED_KM,
    EVENT_DDE_AKE_SEND_RRX,
    EVENT_DDE_AKE_SEND_HRPIME,
    EVENT_DDE_SKE_SEND_EKS,
    EVENT_DDE_AKE_TRANSMITTER_INFO,
    EVENT_DDE_AKE_RECEIVER_INFO,
    EVENT_DDE_REPAUTH_SEND_RXIDLIST,
    EVENT_DDE_REPAUTH_SEND_ACK,
    EVENT_DDE_REPAUTH_STREAM_MANAGE,
    EVENT_DDE_REPAUTH_STREAM_READY,
    EVENT_DDE_RECEIVER_AUTHSTATUS,
    EVENT_DDE_CREATE_HDCP_CONTEXT,
    EVENT_DDE_DESTROY_HDCP_CONTEXT,
    EVENT_DDE_RECEIVE_DATA,
    EVENT_DDE_CB_REPORT_ENCRYPTION_STATUS,
    EVENT_DDE_CB_REPORT_LINK_STATUS,
    EVENT_DDE_CB_SEND_DATA,
    EVENT_DDE_MESSAGE,                             //! event for debug message
    EVENT_OCA_ERROR,                               //! event for OCA error.
    EVENT_DATA_DUMP,                               //! event for debug data dump
    EVENT_HECI_OBJ,                                //! event for heci duplicate handle
    EVENT_PLAT_INFO,                               //! event for static platform info
    EVENT_DATA_DICTIONARY,                         //! event for data dictionary (name:value pair)
    EVENT_MEDIA_COPY,                              //! event for media decompresss/copy/blt
    EVENT_MOS_BATCH_SUBMIT,                        //! event for batch buffer submission
    EVENT_VA_PICTURE,                              //! event for VA begin/render/end picture
    EVENT_VA_SYNC,                                 //! event for VA sync surface/buffer
    EVENT_VA_GET,                                  //! event for VA get image
    EVENT_VA_CONFIG,                               //! event for VA query config
    EVENT_VA_SURFACE,                              //! event for VA create surface
    EVENT_VA_FREE_SURFACE,                         //! event for VA destroy surface
    EVENT_VA_DERIVE,                               //! event for VA derive surface to image
    EVENT_VA_MAP,                                  //! event for VA map buffer
    EVENT_VA_UNMAP,                                //! event for VA unmap buffer
    EVENT_VA_LOCK,                                 //! event for VA lock surface
    EVENT_VA_UNLOCK,                               //! event for VA unlock surface
    EVENT_VA_BUFFER,                               //! event for VA create buffer
    EVENT_VA_FREE_BUFFER,                          //! event for VA destroy buffer
    EVENT_VA_IMAGE,                                //! event for VA create image
    EVENT_VA_FREE_IMAGE,                           //! event for VA destroy image
    EVENT_VA_PUT,                                  //! event for VA put image
    EVENT_PIPE_EXE,                                //! event for pipeline execute
    EVENT_PIPE_PACKET,                             //! event for pipeline ActivatePacket
    EVENT_DDI_CREATE_DEVICE,                       //! event for Refactor DDI Create Device
    EVENT_DDI_DESTROY_DEVICE,                      //! event for Refactor DDI Destroy Device
    EVENT_DDI_DESTROY_RESOURCE_INFO,               //! event for Refactor DDI Destroy Resource Info
    EVENT_DDI_SYNC_CALLBACK,                       //! event for Refactor DDI Sync Callback
    EVENT_DDI_LOCK_SYNC_CALLBACK,                  //! event for Refactor DDI Lock Sync
    EVENT_DDI_TRIM_RESIDENCY_MEDIA,                //! event for Refactor DDI Trim Residency Media
    EVENT_DDI_TRIM_RESIDENCY_MEDIA_INTERNAL,       //! event for Refactor DDI Trim Residency Media Internal
    EVENT_DDI_UPDATE_MEDIA_RESIDENCY_LIST,         //! event for Refactor DDI Update Media Residency List
    EVENT_DDI_IS_PROTECTION_ENABLED,               //! event for Refactor DDI Is Protection Enable
    EVENT_DDI_PROTECTION_TRIGGERED,                //! event for Refactor DDI Protection Triggered
    EVENT_DDI_INIT_ARBITRATOR_SESSION_RES,         //! event for Refactor DDI Init Arbitrator Session Res
    EVENT_DDI_IS_OVERLAY_OR_FULLSCREEN_REQUIRED,   //! event for Refactor DDI Is Overlay Or Fullscreen Required
    EVENT_DDI_MEDIA_MEM_DECOMP_CALLBACK,           //! event for Refactor DDI Media Mem Decomp Callback
    EVENT_DDI_MEDIA_MEM_COPY_CALLBACK,             //! event for Refactor DDI Media Mem Copy Callback
    EVENT_DDI_GET_TRANSCRYPTED_SHADER,             //! event for Refactor DDI Get Transcrypted Shader
    EVENT_DDI_CLEAR_VIDEO_VIEW,                    //! event for Refactor DDI Clear Video View
    EVENT_GPU_CONTEXT_CREATE,                      //! event for gpu context create
    EVENT_GPU_CONTEXT_DESTROY,                     //! event for gpu context destroy
    EVENT_PIC_PARAM_AVC,                           //! event for AVC picture param
    EVENT_PIC_PARAM_HEVC,                          //! event for HEVC picture param
    EVENT_PIC_PARAM_VP9,                           //! event for VP9 picture param
    EVENT_PIC_PARAM_AV1,                           //! event for AV1 picture param
    EVENT_MEDIA_LOG,                               //! event for media log
    EVENT_MEDIA_LOG_RESERVE,                       //! event for more media log
    EVENT_MEDIA_ERR,                               //! event for media error
    EVENT_MEDIA_ERR_RESERVE,                       //! event for more media error
    EVENT_DECODE_DDI_11_GETPROFILECOUNT,           //! event for Decode Get Profile Count
    EVENT_DECODE_DDI_11_GETPROFILE,                //! event for Decode Get Profile
    EVENT_DECODE_DDI_11_CHECKFORMAT,               //! event for Decode Check Format
    EVENT_DECODE_DDI_11_GETCONFIGCOUNT,            //! event for Decode Config Count
    EVENT_DECODE_DDI_11_GETCONFIG,                 //! event for Decode Config
    EVENT_DECODE_DDI_11_GETBUFFERTYPECOUNT,        //! event for Decode Buffer Count
    EVENT_DECODE_DDI_11_GETBUFFERINFO,             //! event for Decode Buffer Info
    EVENT_DECODE_DDI_11_CREATEVIDEODECODER,        //! event for Decode Create Device
    EVENT_DECODE_DDI_11_CREATEOUTPUTVIEW,          //! event for Decode Create Output View
    EVENT_DECODE_DDI_11_BEGINFRAME,                //! event for Decode Begin Frame
    EVENT_DECODE_DDI_11_SUBMITBUFFERS,             //! event for Decode Execute
    EVENT_DECODE_DDI_11_EXTENSIONEXECUTE,          //! event for Decode Extension Execute
    EVENT_DECODE_DDI_11_ENDFRAME,                  //! event for Decode End Frame
    EVENT_DECODE_DDI_11_DESTROYOUTPUTVIEW,         //! event for Decode Destroy Output View
    EVENT_DECODE_DDI_11_DESTROYVIDEODECODER,       //! event for Decode Destroy Device
    EVENT_DECODE_BUFFER_PICPARAM_VP9,              //! event for Decode VP9 Pic Paramters
    EVENT_DECODE_BUFFER_SEGPARAM_VP9,              //! event for Decode VP9 Segment Paramters
    EVENT_DECODE_BUFFER_SLICEPARAM_VP9,            //! event for Decode VP9 Slice Paramters
    EVENT_DECODE_INFO_BITSTREAM,                   //! event for Decode Bitstream Info
    EVENT_DECODE_INFO_PICTURE,                     //! event for Decode Picture Info
    EVENT_DECODE_CMD_HCP_SURFACESTATE,             //! event for Decode HcpSurfaceState Cmd
    EVENT_DECODE_CMD_HCP_PIPEBUFADDRSTATE,         //! event for Decode HcpPipeBufAddrState Cmd
    EVENT_DECODE_CMD_HCP_INDOBJBASEADDRSTATE,      //! event for Decode HcpIndObjBaseAddrState Cmd
    EVENT_DECODE_CMD_HCP_SEGMENTSTATE_VP9,         //! event for Decode HcpVp9SegmentState Cmd
    EVENT_DECODE_CMD_HCP_PICSTATE_VP9,             //! event for Decode HcpVp9PicState Cmd
    EVENT_DECODE_BUFFER_IQPARAM_AVC,               //! event for Decode AVC IQ Paramters
    EVENT_DECODE_BUFFER_PICPARAM_AV1,              //! event for Decode AV1 Pic Paramters
    EVENT_DECODE_BUFFER_SEGPARAM_AV1,              //! event for Decode AV1 Segment Paramters
    EVENT_DECODE_BUFFER_FILMGRAINPARAM_AV1,        //! event for Decode AV1 Film Grain Paramters
    EVENT_DECODE_BUFFER_TILEPARAM_AV1,             //! event for Decode AV1 Tile Paramters
    EVENT_DECODE_BUFFER_PICPARAM_AVC,              //! event for Decode AVC Pic Paramters
    EVENT_DECODE_BUFFER_SLICEPARAM_AVC,            //! event for Decode AVC Slice Paramters
    EVENT_DECODE_BUFFER_PICPARAM_HEVC,             //! event for Decode HEVC Pic Paramters
    EVENT_DECODE_BUFFER_REXTPICPARAM_HEVC,         //! event for Decode HEVC REXT Pic Paramters
    EVENT_DECODE_BUFFER_SCCPICPARAM_HEVC,          //! event for Decode HEVC SCC Pic Paramters
    EVENT_DECODE_BUFFER_SLICEPARAM_HEVC,           //! event for Decode HEVC Slice Paramters
    EVENT_DECODE_BUFFER_LONGSLICEPARAM_HEVC,       //! event for Decode HEVC Long Slice Paramters
    EVENT_DECODE_BUFFER_REXTLONGSLICEPARAM_HEVC,   //! event for Decode HEVC RExt Long Slice Paramters
    EVENT_DECODE_BUFFER_LONGSLICEPARAM_AVC,        //! event for Decode AVC Long Slice Paramters
    EVENT_DECODE_INFO_MMC,                         //! event for Decode Info MMC
    EVENT_DECODE_INFO_SCALABILITY,                 //! event for Decode Info Scalability
    EVENT_DECODE_INFO_SFC,                         //! event for Decode Info SFC
    EVENT_DECODE_INFO_DECODEMODE_REPORT,           //! event for Decode Info Decode Mode Report
    EVENT_DECODE_DUMPINFO_DST,                     //! event for Decode Dst Dump Info
    EVENT_DECODE_DUMPINFO_REF,                     //! event for Decode Ref Dump Info
    EVENT_CALL_STACK,                              //! event for call stack dump
    EVENT_ENCODE_DDI_11_CREATEVIDEOENCODER,        //! event for Encode Create Device
    EVENT_ENCODE_DDI_11_ENCODEFRAME,               //! event for Encode frame, mainly excute
    EVENT_ENCODE_DDI_11_GETCAPS,                   //! event for Encode getting caps
    EVENT_ENCODE_DDI_11_GETPROFILECOUNT,           //! event for Encode get profile count
    EVENT_ENCODE_DDI_11_GETPROFILE,                //! event for Encode get profile
    EVENT_ENCODE_DDI_11_CHECKFORMAT,               //! event for Encode check format
    EVENT_ENCODE_DDI_11_GETCONFIGCOUNT,            //! event for Encode get config count
    EVENT_ENCODE_DDI_11_GETCONFIG,                 //! event for Encode get config
    EVENT_ENCODE_DDI_STATUS_REPORT_HEVC,           //! event for HEVC encode status report
    EVENT_ENCODE_DDI_SLICE_STATUS_REPORT_HEVC,     //! event for HEVC encode slice status report
    EVENT_ENCODE_DDI_EXT_STATUS_REPORT_HEVC,       //! event for HEVC encode ext status report
    EVENT_ENCODE_DDI_CAPS_HEVC,                    //! event for HEVC encode caps
    EVENT_ENCODE_DDI_SEQ_PARAM_HEVC,               //! event for HEVC encode sequence param
    EVENT_ENCODE_DDI_PIC_PARAM_HEVC,               //! event for HEVC encode picture param
    EVENT_ENCODE_DDI_SLC_PARAM_HEVC,               //! event for HEVC encode slice param
    EVENT_ENCODE_DDI_VERSION_HEVC,                 //! event for HEVC encode DDI version
    EVENT_ENCODE_API_STICKER_HEVC,                 //! event for HEVC encode API sticker
    EVENT_DECODE_DDI_DISPLAYINFOVA,                //! event for Decode DDI DisplayInfo
    EVENT_DECODE_DDI_CREATEBUFFERVA,               //! event for Decode DDI CreateBuffer
    EVENT_DECODE_DDI_BEGINPICTUREVA,               //! event for Decode DDI BeginPicture
    EVENT_DECODE_DDI_ENDPICTUREVA,                 //! event for Decode DDI EndPicture
    EVENT_DECODE_DDI_RENDERPICTUREVA,              //! event for Decode DDI RenderPicture
    EVENT_DECODE_DDI_CLEARUPVA,                    //! event for Decode DDI ClearUp
    EVENT_DECODE_DDI_STATUSREPORTVA,               //! event for Decode DDI StatusReport
    EVENT_DECODE_DDI_CREATECONTEXTVA,              //! event for Decode DDI CreateContext
    EVENT_DECODE_DDI_DESTROYCONTEXTVA,             //! event for Decode DDI DestroyContext
    EVENT_DECODE_DDI_GETDECCTXFROMBUFFERIDVA,      //! event for Decode DDI GetDecCtxFromBufferID
    EVENT_DECODE_DDI_FREEBUFFERHEAPELEMENTSVA,     //! event for Decode DDI FreeBufferHeapElements
    EVENT_DECODE_DDI_SETGPUPRIORITYVA,             //! event for Decode DDI SetGpuPriority
    EVENT_DECODE_FEATURE_DECODEMODE_REPORTVA,      //! event for Decode Feature Decode Mode Report
    EVENT_DECODE_INFO_PICTUREVA,                   //! event for Decode Picture Info VA
    EVENT_DECODE_IP_ALIGNMENT,                     //! event for Decode IP Alignment
    EVENT_ENCODE_IP_ALIGNMENT,                     //! event for Encode IP Alignment
    EVENT_VPP_IP_ALIGNMENT,                        //! event for VPP IP Alignment
} MEDIA_EVENT;

typedef enum _MEDIA_EVENT_TYPE
{
    EVENT_TYPE_INFO  = 0,           //! function information event
    EVENT_TYPE_START = 1,           //! function entry event
    EVENT_TYPE_END   = 2,           //! function exit event
    EVENT_TYPE_INFO2 = 3,           //! function extra information event
} MEDIA_EVENT_TYPE;

typedef enum _MT_LEVEL
{
    MT_VERBOSE  = 0,  //! verbos runtime log
    MT_NORMAL   = 1,  //! normal runtime log
    MT_CRITICAL = 2,  //! critical runtime log
} MT_LEVEL;

class MtEnable
{
public:
    MtEnable(bool flag = false) : m_flag(flag) {}

    MtEnable(const uint32_t *pFlag) : m_pFlag(pFlag) {}

    ~MtEnable()
    {
        Reset();
    }

    operator bool() const
    {
        return m_pFlag ? *(m_pFlag) : m_flag;
    }

    void Reset()
    {
        m_pFlag = nullptr;
    }

private:
    bool            m_flag  = false;
    const uint32_t *m_pFlag = nullptr;
};

class MtFilter
{
public:
    MtFilter(const uint64_t *filter = nullptr, size_t filterNum = 0) : m_filter(filter), m_maxKeyNum(filterNum * N)
    {
        if (filter && filterNum == 0)
        {
            m_maxKeyNum = N;
        }
    }

    ~MtFilter()
    {
        Reset();
    }

    bool operator()(MEDIA_EVENT_FILTER_KEYID key)
    {
        return m_filter && static_cast<size_t>(key) < m_maxKeyNum ? (m_filter[key / N] & (1ULL << (key % N))) : false;
    }

    void Reset()
    {
        m_filter    = nullptr;
        m_maxKeyNum = 0;
    }

private:
    static constexpr size_t N = sizeof(uint64_t) << 3;

    const uint64_t *m_filter;
    size_t          m_maxKeyNum;
};

class MtLevel
{
public:
    MtLevel(const uint8_t *level = nullptr) : m_level(reinterpret_cast<const Level *>(level)) {}

    ~MtLevel() { Reset(); }

    bool operator()(MT_EVENT_LEVEL level)
    {
        return m_level ? (level <= static_cast<MT_EVENT_LEVEL>(m_level->event)) : false;
    }

    bool operator()(MT_DATA_LEVEL level)
    {
        return m_level ? (level <= static_cast<MT_DATA_LEVEL>(m_level->data)) : false;
    }

    bool operator()(MT_LOG_LEVEL level)
    {
        return m_level ? (level <= static_cast<MT_LOG_LEVEL>(m_level->log)) : false;
    }

    void Reset() { m_level = nullptr; }

private:
    struct Level
    {
        uint8_t event : 3;  // MT_EVENT_LEVEL
        uint8_t data : 2;   // MT_DATA_LEVEL
        uint8_t log : 3;    // MT_LOG_LEVEL
    };

private:
    const Level *m_level;
};

#pragma pack(1)
typedef struct _MT_PARAM
{
    int32_t id;
    int64_t value;
} MT_PARAM;
#pragma pack()

//!
//! \def media trace log id
//!  |------------|------------------------------------| total 32bits
//!  8bits comp id  24bits user specific id
//!
typedef enum _MT_LOG_ID
{
    MT_LOG_ID_BASE = 0x00000000, // marker for tool, don't change this line
    MT_ERR_MEM_ALLOC,
    MT_ERR_GRAPHIC_ALLOC,
    MT_ERR_NULL_CHECK,
    MT_ERR_HR_CHECK,
    MT_ERR_MOS_STATUS_CHECK,
    MT_ERR_CONDITION_CHECK,
    MT_ERR_INVALID_ARG,
    MT_ERR_LOCK_SURFACE,
    MT_MOS_GPUCXT_CREATE,
    MT_MOS_GPUCXT_DESTROY,
    MT_MOS_GPUCXT_GET,
    MT_MOS_GPUCXT_PRIMARIES,
    MT_MOS_ADDCMD,
    MT_MOS_GPUCXT_SETHANDLE,
    MT_MOS_SYNC,
    MT_MOS_GPUCXT_SET,
    MT_MOS_GPUCXT_VALID,
    MT_MOS_GPUVA_MAP,
    MT_MOS_GPUVA_FREE,
    MT_MOS_MM_ALLOCATE_EXTRES,
    MT_MOS_MM_MAKERESIDENT,
    MT_MOS_MM_UPDATERESIDENCY,
    MT_MOS_MM_EVICT,
    MT_MOS_MM_BIND_GPU_RESOURCE_VIRTUAL,
    MT_MOS_MM_UNBIND_GPU_RESOURCE_VIRTUAL,
    MT_ERR_CRITICAL_MESSAGE,
    MT_MOS_ALLOCATE_MEMORY,
    MT_MOS_DESTROY_MEMORY,
    MT_MOS_MEMORY_NINJA_COUNTER,
    MT_GPU_ALLOCATE_MEMORY,
    MT_GPU_DESTROY_MEMORY,
    MT_LOG_ID_CP_BASE = 0x01000000,
    MT_CP_HAL_NOT_INITIALIZED,
    MT_CP_HAL_FAIL,
    MT_CP_HAL_KEY_RULE,
    MT_CP_HAL_FW_RULE,
    MT_CP_HAL_EPID_CERT,
    MT_CP_HAL_VERIFY_TRANS_KERNEL,
    MT_CP_HAL_METADATA,
    MT_CP_HAL_EPID_STATUS,
    MT_CP_HAL_STATUS_CHECK,
    MT_CP_PROVISION_CERT_CHECK,
    MT_CP_PROVISION_CERT_NOT_FOUND,
    MT_CP_HUC_NOT_AUTHENTICATED,
    MT_CP_KERNEL_RULE,
    MT_CP_KERNEL_TRANSCRYPT,
    MT_CP_BUFFER_RULE,
    MT_CP_MEM_COPY,
    MT_CP_TRANSCODE_SESSION,
    MT_CP_KEY_EXCHANGE,
    MT_CP_CMD_BUFFER_OVERFLOW,
    MT_CP_CAST_FAIL,
    MT_CP_PED_PACKET_SIZE_CHECK,
    MT_CP_CRYPT_COPY_PARAM,
    MT_CP_INVALID_ENCRYPT_TYPE,
    MT_CP_INVALID_CACHED_KEY,
    MT_CP_STATUS_UNINITIALIZED,
    MT_CP_HAL_QUERY_STATUS,
    MT_CP_HAl_ROOT_FAIL,
    MT_CP_ENCRYPT_FAIL,
    MT_CP_SESSION_INIT,
    MT_CP_SESSION_CLEANUP,
    MT_CP_SESSION_CREATE,
    MT_CP_RETRY_FAIL,
    MT_CP_CMD_SEND_FAIL,
    MT_CP_CMD_EXECUTE_FAIL,
    MT_CP_INVALID_SLOT,
    MT_CP_INIT_FW,
    MT_CP_COMMUNICATION_FAIL,
    MT_CP_INVALID_BUFFER,
    MT_CP_RESOURCE_DATA,
    MT_CP_IO_MSG,
    MT_CP_INIT_MOS_INTERFACE,
    MT_CP_RESOURCE_ALLOC,
    MT_CP_RESOURCE_DEALLOC,
    MT_CP_RESOURCE_GET,
    MT_CP_RESOURCE_UPDATE,
    MT_CP_RESOURCE_LOCK,
    MT_CP_RESOURCE_COPY,
    MT_CP_CONSTRUCTOR_FAIL,
    MT_CP_ATTACH_SESSION,
    MT_CP_DETACH_SESSION,
    MT_CP_ESCAPE_CALL,
    MT_CP_SESSION_STATUS_GET,
    MT_CP_CMDLIST_ASSOCIATE_FAIL,
    MT_CP_CMD_BUFFER_GET,
    MT_CP_CMD_SUBMIT,
    MT_CP_CMD_BUILD,
    MT_CP_CMD_OBJECT_CREATE,
    MT_CP_SKU_TABLE_GET,
    MT_CP_USAGE_TABLE_GET,
    MT_CP_CMD_RECORDER_INIT,
    MT_CP_CMD_POOL_INIT,
    MT_CP_CMD_LIST_INIT,
    MT_CP_MMIO_MAPPING_INIT,
    MT_CP_MEM_MAP_FAIL,
    MT_CP_REG_SET,
    MT_CP_DATA_SEND_FAIL,
    MT_CP_BLT_SURF,
    MT_CP_REG_READ_FAIL,
    MT_CP_DEC_DEVICE_ADD,
    MT_CP_DEC_DEVICE_INIT,
    MT_CP_DEC_DEVICE_VERIFY,
    MT_CP_RESOURCE_ESCAPE_CALL,
    MT_CP_SESSION_ASSOCIATE,
    MT_CP_SESSION_ALLOC,
    MT_CP_SESSION_MODE_SET,
    MT_CP_SESSION_TYPE_SET,
    MT_CP_SESSION_TERMINATE,
    MT_CP_PROPIETARY_FUNC,
    MT_CP_ENCRYPTION_BLT_FAIL,
    MT_CP_GET_DATA_FAIL,
    MT_CP_VECTOR_EXCEPTION,
    MT_CP_ENCRYPT_TYPE_SET,
    MT_CP_ME_OPERATION_FAIL,
    MT_CP_ACCESS_ME_FAIL,
    MT_CP_OBJECT_CREATE_FAIL,
    MT_CP_HW_PREPARE_INPUT_BUFFER,
    MT_CP_HW_PROCESS_MSG,
    MT_CP_SESSION_KEY_REFRESH,
    MT_CP_STORE_KEY_BLOB,
    MT_CP_HECI_INIT,
    MT_CP_RT_CALLBACK__FAIL,
    MT_CP_OPEN_SESSION,
    MT_CP_QUERY_COUNTER_FAIL,
    MT_CP_FUNC_NOT_IMPL,
    MT_CP_PERFORM_SW_FAIL,
    MT_CP_SESSION_NOT_ALIVE,
    MT_CP_FUNC_FAIL,
    MT_CP_CENC_STATUS_CHECK,
    MT_CP_CENC_DECODE_CREATE,
    MT_CP_MHW_ID_BASE = 0x01004000,
    MT_CP_MHW_INTERFACE_CREATE_FAIL,
    MT_CP_MHW_ALLOCATION_FAIL,
    MT_CP_MHW_UNIT_NOT_SUPPORT,
    MT_CP_MHW_UNSUPPORTED,
    MT_CP_MHW_IV_SIZE,
    MT_CP_MHW_INVALID_KEY,
    MT_CP_MHW_EARLY_EXIT_CHECK,
    MT_CP_MHW_STATUS_READ,
    MT_CP_DDI_ID_BASE = 0x01005000,
    MT_CP_DDI_CAPS,
    MT_CP_DDI_CAPS_NOT_SUPPORT,
    MT_CP_DDI_DEC_PROFILE_NOT_SUPPORT,
    MT_CP_DDI_NOT_AVAILABLE,
    MT_CP_DDI_CHANNEL_ALLOC,
    MT_CP_DDI_CHANNEL_CREATE,
    MT_CP_DDI_CHANNEL_TYPE,
    MT_CP_DDI_CHANNEL_ESCAPE_CALL,
    MT_CP_DDI_OMAC_CREATE,
    MT_CP_DDI_OMAC_VERIFY,
    MT_CP_DDI_CHANNEL_QUERY_OUTPUT,
    MT_CP_DDI_SEQNUM_SET,
    MT_CP_DDI_ASSOCIATE_HANDLE,
    MT_CP_DDI_ADD_TRUSTED_PROCESS,
    MT_CP_DDI_PERFORM_CONFIG_FAIL,
    MT_CP_DDI_INVALID_CALL,
    MT_CP_DDI_COUNTER_COPY,
    MT_CP_DDI_RESOURCE_CREATE,
    MT_CP_DDI_OS_RESOURCE_INIT,
    MT_CP_DDI_FUNC_UNSUPPORTED,
    MT_CP_DDI_LBDM_FALLBACK_SWDRM,
    MT_LOG_ID_VP_BASE = 0x02000000,
    MT_VP_CREATE,
    MT_VP_DESTROY,
    MT_VP_BLT,
    MT_VP_BLT_START,
    MT_VP_BLT_END,
    MT_VP_BLT_BYPSSED,
    MT_VP_BLT_FORCE_COLORFILL,
    MT_VP_BLT_PROCAMP_PARAM,
    MT_VP_BLT_DN_PARAM,
    MT_VP_BLT_IEF_PARAM,
    MT_VP_BLT_IECP_PARAM,
    MT_VP_BLT_SR_PARAM,
    MT_VP_BLT_RENDERPASS_DATA,
    MT_VP_CLEARVIEW,
    MT_VP_BLT_SETSTATE,
    MT_VP_BLT_TARGETSURF,
    MT_VP_BLT_INPUTSURF,
    MT_VP_BLT_SRC_RECT,
    MT_VP_BLT_DST_RECT,
    MT_VP_BLT_TARGET_RECT,
    MT_VP_BLT_HDRPARAM,
    MT_VP_BLT_FDFBPARAM,
    MT_VP_BLT_SEGMENTPARAM,
    MT_VP_BLT_MCPYPARAM,
    MT_VP_USERFEATURE_CTRL,
    MT_VP_FTR_REPORT,
    MT_VP_FEATURE_GRAPH_ID_BASE = 0x02000200,
    MT_VP_FEATURE_GRAPH_EXECUTE_VPPIPELINE_START,
    MT_VP_FEATURE_GRAPH_EXECUTE_VPPIPELINE_END,
    MT_VP_FEATURE_GRAPH_SETUPEXECUTESWFILTER_START,
    MT_VP_FEATURE_GRAPH_SETUPEXECUTESWFILTER_END,
    MT_VP_FEATURE_GRAPH_EXECUTEFILTER,
    MT_VP_FEATURE_GRAPH_SWFILTERALPHA,
    MT_VP_FEATURE_GRAPH_SWFILTERBLENDING,
    MT_VP_FEATURE_GRAPH_SWFILTERCGC,
    MT_VP_FEATURE_GRAPH_SWFILTERCOLORFILL,
    MT_VP_FEATURE_GRAPH_SWFILTERCSC,
    MT_VP_FEATURE_GRAPH_SWFILTERDEINTERLACE,
    MT_VP_FEATURE_GRAPH_SWFILTERDENOISE,
    MT_VP_FEATURE_GRAPH_SWFILTERHDR,
    MT_VP_FEATURE_GRAPH_SWFILTERLUMAKEY,
    MT_VP_FEATURE_GRAPH_SWFILTERPROCAMP,
    MT_VP_FEATURE_GRAPH_SWFILTERROTMIR,
    MT_VP_FEATURE_GRAPH_SWFILTERSCALING,
    MT_VP_FEATURE_GRAPH_SWFILTERSTE,
    MT_VP_FEATURE_GRAPH_SWFILTERTCC,
    MT_VP_FEATURE_GRAPH_SWFILTERACE,
    MT_VP_FEATURE_GRAPH_SWFILTERCAPPIPE,
    MT_VP_FEATURE_GRAPH_SWFILTERDV,
    MT_VP_FEATURE_GRAPH_SWFILTERFDFB,
    MT_VP_FEATURE_GRAPH_SWFILTERLACE,
    MT_VP_FEATURE_GRAPH_SWFILTERS3D,
    MT_VP_FEATURE_GRAPH_SWFILTERSECURECOPY,
    MT_VP_FEATURE_GRAPH_SWFILTERVEBOXUPDATE,
    MT_VP_FEATURE_GRAPH_SWFILTERSTD,
    MT_VP_FEATURE_GRAPH_INPUTSWFILTER,
    MT_VP_FEATURE_GRAPH_OUTPUTSWFILTER,
    MT_VP_FEATURE_GRAPH_INPUT_SURFACE_INFO,
    MT_VP_FEATURE_GRAPH_INTERMEIDATE_SURFACE_INFO,
    MT_VP_FEATURE_GRAPH_OUTPUT_SURFACE_INFO,
    MT_VP_FEATURE_GRAPH_SURFACE_ALLOCATIONHANDLE,
    MT_VP_FEATURE_GRAPH_GET_RENDERTARGETTYPE,
    MT_VP_FEATURE_GRAPH_SWFILTERSR,
    MT_VP_FEATURE_GRAPH_FEATUREPIPE_REUSE,
    MT_VP_FEATURE_GRAPH_EXECUTE_SINGLE_VPPIPELINE_START,
    MT_VP_FEATURE_GRAPH_EXECUTE_SINGLE_VPPIPELINE_END,
    MT_VP_HAL_ID_BASE = 0x02000400,
    MT_VP_HAL_PIPELINE_ADAPTER,
    MT_VP_HAL_PIPELINE_ADAPTER_EXT_ENTRY,
    MT_VP_HAL_PIPELINE_ADAPTER_EXT_EXIT,
    MT_VP_HAL_PIPELINE,
    MT_VP_HAL_PIPELINE_PREPARE,
    MT_VP_HAL_PIPELINE_EXT,
    MT_VP_HAL_POLICY,
    MT_VP_HAL_HWFILTER,
    MT_VP_HAL_SWWFILTER,
    MT_VP_HAL_INIT,
    MT_VP_HAL_DESTROY,
    MT_VP_HAL_RENDER,
    MT_VP_HAL_RENDER_VE,
    MT_VP_HAL_RENDER_VE_ISNEEDED,
    MT_VP_HAL_RENDER_VE_GETOUTPUTPIPE,
    MT_VP_HAL_RENDER_SFC,
    MT_VP_HAL_RENDER_COMPOSITE,
    MT_VP_HAL_ALLOC_SURF,
    MT_VP_HAL_REALLOC_SURF,
    MT_VP_HAL_SWWFILTER_ADD,
    MT_VP_HAL_ONNEWFRAME_PROC_START,
    MT_VP_HAL_ONNEWFRAME_PROC_END,
    MT_VP_HAL_POLICY_GET_EXTCAPS4FTR,
    MT_VP_HAL_POLICY_GET_INPIPECAPS,
    MT_VP_HAL_POLICY_GET_OUTPIPECAPS,
    MT_VP_HAL_POLICY_INIT_EXECCAPS,
    MT_VP_HAL_FC_SCALINGINFO,
    MT_VP_HAL_VESFC_HWLIMIT,
    MT_VP_HAL_RENDER_SETUP_WALKER_PARAM,
    MT_VP_HAL_RENDER_SETUP_CURBE_STATE,
    MT_VP_HAL_POLICY_FLITER_FTR_COMBINE,
    MT_VP_HAL_FC_UPDATE_COMP_PARAM,
    MT_VP_HAL_FC_GET_CURBE_STATE,
    MT_VP_HAL_DESTROY_SURF,
    MT_VP_HAL_MEMORY_FOOTPRINT_EXECUTION_ENTRY,
    MT_VP_HAL_MEMORY_FOOTPRINT_EXECUTION_EXIT,
    MT_VP_HAL_VEBOXNUM_CHECK,
    MT_VP_HAL_VEBOXNUM_RESET,
    MT_VP_MHW_ID_BASE = 0x02002000,
    MT_VP_MHW_VE_SURFSTATE_INPUT,
    MT_VP_MHW_VE_SURFSTATE_OUT,
    MT_VP_MHW_VE_SURFSTATE_DNOUT,
    MT_VP_MHW_VE_SURFSTATE_SKINSCORE,
    MT_VP_MHW_VE_SURFSTATE_STMM,
    MT_VP_MHW_VE_SCALABILITY,
    MT_VP_MHW_VE_ADJUST_SURFPARAM,
    MT_VP_MHW_CACHE_MOCS_TABLE,
    MT_VP_KERNEL_ID_BASE = 0x02003000,
    MT_VP_KERNEL_CSC,
    MT_VP_KERNEL_RULE,
    MT_VP_KERNEL_LIST_ADD,
    MT_VP_KERNEL_Init,
    MT_MEDIA_COPY_ID_BASE = 0x02004000,
    MT_VE_DECOMP_COPY,
    MT_MEDIA_COPY,
    MT_MEDIA_COPY_BLT,
    MT_MEDIA_COPY_RENDER,
    MT_MEDIA_COPY_VE,
    MT_LOG_ID_DEC_BASE   = 0x03000000,
    MT_DEC_HEVC,
    MT_LOG_ID_ENC_BASE   = 0x04000000,
} MT_LOG_ID;

//!
//! \def media trace parameter id
//!
typedef enum _MT_PARAM_ID
{
    MT_PARAM_ID_BASE = 0,
    MT_ERROR_CODE,
    MT_COMPONENT,
    MT_SUB_COMPONENT,
    MT_CODE_LINE,
    MT_GENERIC_VALUE,
    MT_PRODUCT_FAMILY,
    MT_SURF_PTR,
    MT_SURF_ALLOC_HANDLE,
    MT_SURF_WIDTH,
    MT_SURF_HEIGHT,
    MT_SURF_PITCH,
    MT_SURF_MOS_FORMAT,
    MT_SURF_TILE_TYPE,
    MT_SURF_TILE_MODE,
    MT_SURF_COMP_ABLE,
    MT_SURF_COMP_MODE,
    MT_SURF_GMM_FLAG_GPU,
    MT_SURF_GMM_FLAG_INF,
    MT_SURF_GMM_FLAG_WA,
    MT_SURF_RES_ARRAYSIZE,
    MT_SURF_RES_INDEX,
    MT_SURF_CP_TAG,
    MT_SURF_IS_INPUT,
    MT_SURF_IS_OUTPUT,
    MT_RECT_LEFT,
    MT_RECT_TOP,
    MT_RECT_RIGHT,
    MT_RECT_BOTTOM,
    MT_SYSMEM_PTR,
    MT_SYSMEM_WIDTH,
    MT_SYSMEM_HSTRIDE,
    MT_SYSMEM_VSTRIDE,
    MT_FUNC_START,
    MT_FUNC_END,
    MT_FUNC_RET,
    MT_VIEW_TYPE,
    MT_SURF_GMM_PATIDX,
    MT_SURF_GMM_RESUSAGE,
    MT_SURF_GMM_GPUVA,
    MT_SURF_GMM_PAGINGFENCE,
    MT_SURF_MOS_RESOURCE_USAGE,
    MT_SURF_ALLOCINFO_PTR,
    MT_SURF_ALLOCINFO_ISMEDIAINTERNAL,
    MT_SURF_ALLOCINFO_ISPERSISTENT,
    MT_SURF_ALLOCINFO_3DRESOURCE_PTR,
    MT_SURF_ALLOCINFO_ISNEW,
    MT_SURF_MEDIARESINFO_PTR,
    MT_SURF_BE_INTERNAL_RESIDENT_MAP,
    MT_SURF_MAPPED_ALLOCINFO,
    MT_DEVICE_HANDLE,
    MT_COMMAND_GPUVA,
    MT_FUNC_NAME,
    MT_FUNC_LINE,
    MT_MEMORY_PTR,
    MT_MEMORY_SIZE,
    MT_MEMORY_INDEX,
    MT_MEMORY_NINJA_START_COUNTER,
    MT_MEMORY_NINJA_END_COUNTER,
    MT_MEMORY_NINJA_IS_START,
    MT_PARAM_ID_MOS_BASE = 0x00001000,
    MT_MOS_STATUS,
    MT_MOS_GPU_NODE,
    MT_MOS_GPUCXT_MGR_PTR,
    MT_MOS_GPUCXT_PTR,
    MT_MOS_GPUCXT_HANDLE,
    MT_MOS_GPUCXT_COUNT,
    MT_MOS_GPUCXT_NUMPRIMARIES,
    MT_MOS_GPUCXT,
    MT_MOS_SYNC_HAZARDTYPE,
    MT_MOS_SYNC_BUSYCTX,
    MT_MOS_SYNC_REQCTX,
    MT_MOS_MM_EXT_RESOURCE_NEED_MAKE_RESIDENT,
    MT_SYNC_WAIT_LOOPCOUNTER,
    MT_SYNC_CUR_FENCE_TAG,
    MT_SYNC_LAST_FENCE_TAG,
    MT_SYNC_WAIT_BEFORE_SYNC,
    MT_SYNC_WAIT_MICROSECOND,
    MT_SYNC_WAIT_RESULT,
    MT_PARAM_ID_CP_BASE  = 0x01000000,
    MT_CP_SESSION_TYPE,
    MT_CP_SESSION_MODE,
    MT_CP_STREAM_ID,
    MT_CP_FW_CAPABILITY,
    MT_CP_KEY_LENGTH,
    MT_CP_COMMAND_ID,
    MT_CP_COMMAND,
    MT_CP_GROUP_ID,
    MT_CP_METADATA_INFO_VERSION,
    MT_CP_FW_API_VERSION,
    MT_CP_BUFFER_NAME,
    MT_CP_CMD_BUFFER_REMAIN,
    MT_CP_PRODUCT_FAMILY_ID,
    MT_CP_KEY_EXCHANGE_TYPE,
    MT_CP_QUERY_OPERATION,
    MT_CP_CRYPT_COPY_ADDR_CMD,
    MT_CP_CRYPT_COPY_CMD,
    MT_CP_IV_SIZE,
    MT_CP_MHW_GPR0,
    MT_CP_MHW_SCRATCH_BUFFER,
    MT_CP_ENCRYPT_TYPE,
    MT_CP_COMMAND_TYPE,
    MT_CP_CPTO_TYPE,
    MT_CP_CAPS,
    MT_CP_SESSION_UPDATE_TYPE,
    MT_CP_INPUTSIZE,
    MT_CP_OUTPUTSIZE,
    MT_CP_CHANNEL_TYPE,
    MT_CP_SESSION_ID,
    MT_CP_FUNC_ID,
    MT_CP_DRM_TYPE,
    MT_CP_CTX_TYPE,
    MT_CP_CTX_PTR,
    MT_PARAM_ID_VP_BASE  = 0x02000000,
    MT_VP_SCALINGMODE_SR,
    MT_PARAM_ID_VP_FTR_BASE = 0x02000200,
    MT_VP_SKU_FTR_VERING,
    MT_VP_SKU_FTR_MCPY,
    MT_VP_UF_CTRL_DISABLE_VEOUT,
    MT_VP_UF_CTRL_DISABLE_SFC,
    MT_VP_UF_CTRL_CCS,
    MT_PARAM_ID_VP_BLT_BASE = 0x02000300,
    MT_VP_BLT_PARAM_DATA,
    MT_VP_BLT_PARAM_FLAG,
    MT_VP_BLT_SRC_COUNT,
    MT_VP_BLT_AUTO_PROCESSING_MODE,
    MT_VP_BLT_OUTPUT_FRAME,
    MT_VP_BLT_STREAM_COUNT,
    MT_VP_BLT_SAMPLE_TYPE,
    MT_VP_BLT_CSPACE,
    MT_VP_BLT_ROTATION,
    MT_VP_BLT_SURF_TYPE,
    MT_VP_BLT_CHROMASITING,
    MT_VP_BLT_HDRPARAM_EOTF,
    MT_VP_BLT_HDRPARAM_MAX_DISPLUMA,
    MT_VP_BLT_HDRPARAM_MIN_DISPLUMA,
    MT_VP_BLT_HDRPARAM_MAXCLL,
    MT_VP_BLT_HDRPARAM_MAXFALL,
    MT_VP_BLT_FDFBPARAM_MODE,
    MT_VP_BLT_FDFBPARAM_FACECOUNT,
    MT_VP_BLT_FDFBPARAM_FBMAXFACECOUNT,
    MT_VP_BLT_SR_MODE,
    MT_VP_BLT_SR_EU,
    MT_VP_BLT_SR_GEN,
    MT_PARAM_ID_VP_HAL_BASE = 0x02000400,
    MT_VP_HAL_APO,
    MT_VP_HAL_PTR,
    MT_VP_HAL_PIPE_CNT,
    MT_VP_HAL_INTER_SURF_TYPE,
    MT_VP_RENDERPASS_FLAG_COMP_NEEDED,
    MT_VP_RENDERPASS_FLAG_HDR_NEEDED,
    MT_VP_RENDERPASS_FLAG_FASTCOLORFILL,
    MT_VP_RENDERPASS_FLAG_BYPASS_HDRKERNEL,
    MT_VP_RENDERPASS_FLAG_USEVEHDRSFC,
    MT_VP_RENDERDATA_OUTPUT_PIPE,
    MT_VP_RENDERDATA_2PASS_CSC,
    MT_VP_RENDERDATA_HDRCSCCUSDS,
    MT_VP_RENDERDATA_HDRSFC,
    MT_VP_RENDERDATA_HDR3DLUT,
    MT_VP_RENDERDATA_HDR1DLUT,
    MT_VP_RENDERDATA_BPROCAMP,
    MT_VP_RENDERDATA_BIECP,
    MT_VP_RENDERDATA_DV_TONAMAPPING,
    MT_VP_RENDER_VE_2PASS_SFC,
    MT_VP_RENDER_VE_USE_HDRTEMPSURF,
    MT_VP_RENDER_VE_HDRMODE,
    MT_VP_RENDER_VE_NEEDED,
    MT_VP_RENDER_VE_HITLIMITATION,
    MT_VP_RENDER_VE_8KFORCERENDER,
    MT_VP_RENDER_VE_CROPPING,
    MT_VP_RENDER_VE_SFCONLYFORVE,
    MT_VP_RENDER_VE_COMPBYPASSFEASIBLE,
    MT_VP_HAL_PIPE_INDEX,
    MT_VP_HAL_PIPE_ISINPUT,
    MT_VP_HAL_FEATUERTYPE,
    MT_VP_HAL_ENGINECAPS,
    MT_VP_HAL_ENGINECAPS_EN,
    MT_VP_HAL_ENGINECAPS_VE_NEEDED,
    MT_VP_HAL_ENGINECAPS_SFC_NEEDED,
    MT_VP_HAL_ENGINECAPS_RENDER_NEEDED,
    MT_VP_HAL_ENGINECAPS_FC_SUPPORT,
    MT_VP_HAL_ENGINECAPS_ISOLATED,
    MT_VP_HAL_EXECCAPS,
    MT_VP_HAL_EXECCAPS_VE,
    MT_VP_HAL_EXECCAPS_SFC,
    MT_VP_HAL_EXECCAPS_RENDER,
    MT_VP_HAL_EXECCAPS_COMP,
    MT_VP_HAL_EXECCAPS_OUTPIPE_FTRINUSE,
    MT_VP_HAL_EXECCAPS_IECP,
    MT_VP_HAL_EXECCAPS_FORCE_CSC2RENDER,
    MT_VP_HAL_EXECCAPS_DI_2NDFIELD,
    MT_VP_HAL_ONNEWFRAME_COUNTER,
    MT_VP_HAL_SCALING_MODE,
    MT_VP_HAL_SCALING_MODE_FORCE,
    MT_VP_HAL_SAMPLER_TYPE,
    MT_VP_HAL_SAMPLER_FILTERMODE,
    MT_VP_HAL_SAMPLER_INDEX,
    MT_VP_HAL_FC_LAYER,
    MT_VP_HAL_FC_LAYER_SURFENTRY,
    MT_VP_RENDER_VE_FTRINUSE,
    MT_VP_RENDER_VE_PREPROC_TCC,
    MT_VP_RENDER_VE_PREPROC_IEF,
    MT_VP_HAL_EXECCAPS_FORCE_DI2RENDER,
    MT_VP_HAL_EUFUSION_BYPASS,
    MT_VP_HAL_MMCINUSE,
    MT_VP_HAL_FRC_MODE,
    MT_VP_HAL_CAPTURE_PIPE,
    MT_VP_HAL_SURF_ALLOC_PARAM_PTR,
    MT_VP_HAL_SURF_ALLOC_PARAM_MOS_SURF_PTR,
    MT_VP_HAL_SURF_ALLOC_PARAM_IS_RES_OWNER,
    MT_VP_HAL_SURF_ALLOC_PARAM_HANDLE,
    MT_VP_HAL_SURF_ALLOC_PARAM_SIZE,
    MT_VP_HAL_SURF_ALLOC_PARAM_NAME,
    MT_VP_HAL_SURF_ALLOC_PARAM_PEAK_SIZE,
    MT_VP_HAL_SURF_ALLOC_PARAM_TOTAL_SIZE,
    MT_VP_HAL_VEBOX_NUMBER,
    MT_PARAM_ID_VP_FEATURE_GRAPH_BASE = 0x02001400,
    MT_VP_FEATURE_GRAPH_FILTER_SWFILTERPIPE_COUNT,
    MT_VP_FEATURE_GRAPH_FILTER_LAYERINDEXES_COUNT,
    MT_VP_FEATURE_GRAPH_FILTER_INPUTCOLORSPACE,
    MT_VP_FEATURE_GRAPH_FILTER_OUTPUTCOLORSPACE,
    MT_VP_FEATURE_GRAPH_FILTER_INPUTFORMAT,
    MT_VP_FEATURE_GRAPH_FILTER_OUTPUTFORMAT,
    MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE,
    MT_VP_FEATURE_GRAPH_FILTER_CALCULATINGALPHA,
    MT_VP_FEATURE_GRAPH_FILTER_ALPHAMODE,
    MT_VP_FEATURE_GRAPH_FILTER_FALPHA,
    MT_VP_FEATURE_GRAPH_FILTER_BLENDTYPE,
    MT_VP_FEATURE_GRAPH_FILTER_LUMAHIGH,
    MT_VP_FEATURE_GRAPH_FILTER_LUMALOW,
    MT_VP_FEATURE_GRAPH_FILTER_HDRMODE,
    MT_VP_FEATURE_GRAPH_FILTER_LUTMODE,
    MT_VP_FEATURE_GRAPH_FILTER_GPUGENERATE3DLUT,
    MT_VP_FEATURE_GRAPH_FILTER_BRIGHTNESS,
    MT_VP_FEATURE_GRAPH_FILTER_CONTRAST,
    MT_VP_FEATURE_GRAPH_FILTER_HUE,
    MT_VP_FEATURE_GRAPH_FILTER_SATURATION,
    MT_VP_FEATURE_GRAPH_FILTER_DISABLECFINSFC,
    MT_VP_FEATURE_GRAPH_FILTER_TCCRED,
    MT_VP_FEATURE_GRAPH_FILTER_TCCGREEN,
    MT_VP_FEATURE_GRAPH_FILTER_TCCBLUE,
    MT_VP_FEATURE_GRAPH_FILTER_STEFACTOR,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLESTD,
    MT_VP_FEATURE_GRAPH_FILTER_SAMPLETYPEINPUT,
    MT_VP_FEATURE_GRAPH_FILTER_FMDKERNELENABLE,
    MT_VP_FEATURE_GRAPH_FILTER_SINGLEFIELD,
    MT_VP_FEATURE_GRAPH_FILTER_DIMODE,
    MT_VP_FEATURE_GRAPH_FILTER_CHROMADN,
    MT_VP_FEATURE_GRAPH_FILTER_LUMADN,
    MT_VP_FEATURE_GRAPH_FILTER_AUTODETECT,
    MT_VP_FEATURE_GRAPH_FILTER_HVSDN,
    MT_VP_FEATURE_GRAPH_FILTER_DNFACTOR,
    MT_VP_FEATURE_GRAPH_FILTER_SECUREDNNEED,
    MT_VP_FEATURE_GRAPH_FILTER_ROTATION,
    MT_VP_FEATURE_GRAPH_FILTER_INPUTHEIGHT,
    MT_VP_FEATURE_GRAPH_FILTER_INPUTWIDTH,
    MT_VP_FEATURE_GRAPH_FILTER_INPUTTILEMODE,
    MT_VP_FEATURE_GRAPH_FILTER_OUTPUTHEIGHT,
    MT_VP_FEATURE_GRAPH_FILTER_OUTPUTWIDTH,
    MT_VP_FEATURE_GRAPH_FILTER_OUTPUTTILEMODE,
    MT_VP_FEATURE_GRAPH_FILTER_ISCALINGTYPE,
    MT_VP_FEATURE_GRAPH_FILTER_SCALINGMODE,
    MT_VP_FEATURE_GRAPH_FILTER_GCOMPMODE,
    MT_VP_FEATURE_GRAPH_FILTER_BT2020TORGB,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLEACE,
    MT_VP_FEATURE_GRAPH_FILTER_ACELEVELCHANGED,
    MT_VP_FEATURE_GRAPH_FILTER_ACELEVEL,
    MT_VP_FEATURE_GRAPH_FILTER_ACESTRENGTH,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLECAPPIPE,
    MT_VP_FEATURE_GRAPH_FILTER_LGCAKERNELENABLED,
    MT_VP_FEATURE_GRAPH_FILTER_LGCAPHASE,
    MT_VP_FEATURE_GRAPH_FILTER_1DLUTSIZE,
    MT_VP_FEATURE_GRAPH_FILTER_3DLUTSIZE,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLEFB,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLEFD,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLEFLD,
    MT_VP_FEATURE_GRAPH_FILTER_FDFBSTAGE,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLELACE,
    MT_VP_FEATURE_GRAPH_FILTER_INPUTSAMPLETYPE,
    MT_VP_FEATURE_GRAPH_FILTER_LACESTAGE,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLES3D,
    MT_VP_FEATURE_GRAPH_FILTER_STEREOFORMAT,
    MT_VP_FEATURE_GRAPH_FILTER_VEBOXSTATECOPYNEEDED,
    MT_VP_FEATURE_GRAPH_FILTER_STD_OUTPUT_ENABLE,
    MT_VP_FEATURE_GRAPH_FILTER_STD_OUTPUT_TO_STDPARAM,
    MT_VP_FEATURE_GRAPH_FILTER_STD_OUTPUT_TO_OUTPUT_SURFACE,
    MT_VP_FEATURE_GRAPH_SURFACE_WIDTH,
    MT_VP_FEATURE_GRAPH_SURFACE_HEIGHT,
    MT_VP_FEATURE_GRAPH_SURFACE_PITCH,
    MT_VP_FEATURE_GRAPH_SURFACE_COLORSPACE,
    MT_VP_FEATURE_GRAPH_SURFACE_FORMAT,
    MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_LEFT,
    MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_TOP,
    MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_RIGHT,
    MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_BOTTOM,
    MT_VP_FEATURE_GRAPH_SURFACE_RCDST_LEFT,
    MT_VP_FEATURE_GRAPH_SURFACE_RCDST_TOP,
    MT_VP_FEATURE_GRAPH_SURFACE_RCDST_RIGHT,
    MT_VP_FEATURE_GRAPH_SURFACE_RCDST_BOTTOM,
    MT_VP_FEATURE_GRAPH_RENDERTARGETTYPE,
    MT_VP_FEATURE_GRAPH_FILTER_ESRMODE,
    MT_VP_FEATURE_GRAPH_FILTER_SRMODEL,
    MT_VP_FEATURE_GRAPH_FILTER_SRSTAGE,
    MT_VP_FEATURE_GRAPH_FILTER_FRAMENUMBER,
    MT_VP_FEATURE_GRAPH_FILTER_FIRSTFRAME,
    MT_VP_FEATURE_GRAPH_FILTER_ENABLESR,
    MT_VP_FEATURE_GRAPH_FILTER_FRAMEID,
    MT_VP_FEATURE_GRAPH_FILTER_PIPEID,
    MT_VP_FEATURE_GRAPH_FILTER_PIPELINEBYPASS,
    MT_PARAM_ID_VP_MHW_BASE = 0x02002000,
    MT_VP_MHW_VE_SCALABILITY_EN,
    MT_VP_MHW_VE_SCALABILITY_USE_SFC,
    MT_VP_MHW_VE_SCALABILITY_IDX,
    MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE,
    MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME,
    MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_TYPE,
    MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_WIDTH,
    MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_HEIGHT,
    MT_VP_MHW_CACHE_MEMORY_OBJECT_SURFACE_FORMAT,
    MT_PARAM_ID_VP_KERNEL_BASE = 0x02003000,
    MT_VP_KERNEL_CSPACE, 
    MT_VP_KERNEL_RULE_ID,
    MT_VP_KERNEL_RULE_LAYERNUM,
    MT_VP_KERNEL_RULE_SEARCH_STATE,
    MT_VP_KERNEL_ID,
    MT_PARAM_ID_MEDIA_COPY_BASE = 0x02004000,
    MT_VE_DECOMP_COPY_SURF_LOCK_STATUS,
    MT_MEDIA_COPY_CAPS, 
    MT_MEDIA_COPY_DIRECTION,
    MT_MEDIA_COPY_METHOD,
    MT_MEDIA_COPY_DEVICE_PTR,
    MT_MEDIA_COPY_DATASIZE,
    MT_MEDIA_COPY_PLANE_NUM,
    MT_MEDIA_COPY_PLANE_PITCH,
    MT_MEDIA_COPY_PLANE_OFFSET,
    MT_MEDIA_COPY_LIMITATION,
    MT_PARAM_ID_DEC_BASE = 0x03000000,
    MT_DEC_HUC_ERROR_STATUS2,
    MT_CODEC_HAL_MODE,
    MT_DEC_HUC_STATUS_CRITICAL_ERROR,
    MT_PARAM_ID_ENC_BASE = 0x04000000,
} MT_PARAM_ID;

#endif
