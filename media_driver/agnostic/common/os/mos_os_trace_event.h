/*
* Copyright (c) 2015-2017, Intel Corporation
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
    MT_LOG_ID_BASE       = 0x00000000, // marker for tool, don't change this line
    MT_MEM_ALLOC_ERR,
    MT_GRAPHIC_ALLOC_ERR,
    MT_ERR_NULL_CHECK,
    MT_ERR_HR_CHECK,
    MT_ERR_MOS_STATUS_CHECK,
    MT_ERR_CONDITION_CHECK,
    MT_ERR_INVALID_ARG,
    MT_ERR_LOCK_SURFACE,
    MT_MOS_GPUCXT_CREATE_FAIL,
    MT_MOS_GPUCXT_INVALID,
    MT_MOS_ADDCMD,
    MT_LOG_ID_CP_BASE    = 0x01000000,
    MT_LOG_ID_VP_BASE    = 0x02000000,
    MT_VP_BLT_START,
    MT_VP_BLT_END,
    MT_VP_BLT_BYPSSED,
    MT_VP_BLT_FORCE_COLORFILL,
    MT_VP_BLT_FAIL,
    MT_VP_BLT_PROCAMP_PARAM,
    MT_VP_BLT_DN_PARAM,
    MT_VP_BLT_IEF_PARAM,
    MT_VP_BLT_IECP_PARAM,
    MT_VP_BLT_RENDERPASS_DATA,
    MT_VP_BLT_PIPELINE,
    MT_VP_HAL_REALLOC_SURF,
    MT_VP_HAL_RENDER_VE,
    MT_VP_HAL_RENDER_VE_ISNEEDED,
    MT_VP_HAL_RENDER_VE_GETOUTPUTPIPE,
    MT_VP_HAL_RENDER_COMPOSITE,
    MT_VP_KERNEL_CSC,
    MT_VP_KERNEL_RULE,
    MT_VP_VE_SCALABILITY,
    MT_VP_VE_ADJUST_SURFPARAM,
    MT_MEDIA_COPY,
    MT_MEDIA_COPY_CPU,
    MT_MEDIA_COPY_BLT,
    MT_MEDIA_COPY_RENDER,
    MT_MEDIA_COPY_VE,
    MT_MEDIA_COPY_VE_LMITATION,
    MT_LOG_ID_DEC_BASE   = 0x03000000,
    MT_LOG_ID_ENC_BASE   = 0x04000000,
} MT_LOG_ID;

//!
//! \def media trace parameter id
//!
typedef enum _MT_PARAM_ID
{
    MT_PARAM_ID_BASE    = 0,
    MT_ERROR_CODE,
    MT_COMPONENT,
    MT_SUB_COMPONENT,
    MT_CODE_LINE,
    MT_GENERIC_VALUE,
    MT_MOS_GPU_NODE,
    MT_SURF_PTR,
    MT_SURF_ALLOC_HANDLE,
    MT_SURF_WIDTH,
    MT_SURF_HEIGHT,
    MT_SURF_PITCH,
    MT_SURF_MOS_FORMAT,
    MT_SURF_TILE_TYPE,
    MT_SURF_COMP_ABLE,
    MT_SURF_COMP_MODE,
    MT_SURF_GMM_FLAG_GPU,
    MT_SURF_GMM_FLAG_INF,
    MT_SURF_GMM_FLAG_WA,
    MT_SURF_IS_INPUT,
    MT_SURF_IS_OUTPUT,
    MT_RECT_LEFT,
    MT_RECT_TOP,
    MT_RECT_RIGHT,
    MT_RECT_BOTTOM,
    MT_PARAM_ID_CP_BASE  = 0x01000000,
    MT_CP_SESSION_TYPE,
    MT_CP_SESSION_MODE,
    MT_PARAM_ID_VP_BASE  = 0x02000000,
    MT_VP_HAL_PTR,
    MT_VP_INTERNAL_SURF_TYPE,
    MT_VP_COLORSPACE,
    MT_VP_BLT_PARAM_DATA,
    MT_VP_BLT_PARAM_FLAG,
    MT_VP_BLT_SRC_COUNT,
    MT_VP_BLT_PATH_APO,
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
    MT_VP_SCALINGMODE_SR,
    MT_VP_SKU_FTR_VERING,
    MT_VP_VE_SCALABILITY_EN,
    MT_VP_VE_SCALABILITY_USE_SFC,
    MT_VP_VE_SCALABILITY_IDX,
    MT_VP_VE_SCALABILITY_START_X,
    MT_VP_VE_SCALABILITY_END_X,
    MT_VP_VE_SCALABILITY_O_START_X,
    MT_VP_VE_SCALABILITY_O_END_X,
    MT_PARAM_ID_DEC_BASE = 0x03000000,
    MT_PARAM_ID_ENC_BASE = 0x04000000,
} MT_PARAM_ID;

#endif
