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
    EVENT_DDE_MESSAGE,              //! event for debug message
    EVENT_OCA_ERROR,                //! event for OCA error.
    EVENT_DATA_DUMP                 //! event for debug data dump
} MEDIA_EVENT;

typedef enum _MEDIA_EVENT_TYPE
{
    EVENT_TYPE_INFO  = 0,           //! function information event
    EVENT_TYPE_START = 1,           //! function entry event
    EVENT_TYPE_END   = 2,           //! function exit event
} MEDIA_EVENT_TYPE;

#endif
