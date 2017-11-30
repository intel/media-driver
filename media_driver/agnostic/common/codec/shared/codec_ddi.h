/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codec_ddi.h
//! \brief    Defines common functions, macros, and types shared by DDI layers.
//! \details  Should only be included by DDI layers
//!

#ifndef __CODEC_DDI_H__
#define __CODEC_DDI_H__

#include "mos_os.h"
#include "mos_util_debug.h"

// ---------------------------
// Definitions - used only from DDI
// ---------------------------
// The following are from MSSG's mpeg2dec.h file

#define ENCODE_MAX_PIC_WIDTH            1920
#define ENCODE_MAX_PIC_HEIGHT           1920                // Tablet usage in portrait mode, image resolution = 1200x1920, so change MAX_HEIGHT to 1920
#define ENCODE_4K_MAX_PIC_WIDTH         4096
#define ENCODE_4K_MAX_PIC_HEIGHT        4096

#define ENCODE_HEVC_8K_MAX_PIC_WIDTH    8192
#define ENCODE_HEVC_8K_MAX_PIC_HEIGHT   8192

#define ENCODE_16K_MAX_PIC_WIDTH        16384
#define ENCODE_16K_MAX_PIC_HEIGHT       16384

// Max supported resolution for JPEG encode is 16K X 16K
#define ENCODE_JPEG_MAX_PIC_WIDTH     16384
#define ENCODE_JPEG_MAX_PIC_HEIGHT    16384

#define JPEG_MAX_NUM_HUFF_TABLES      2    // Max 2 sets of Huffman Tables are allowed (2AC and 2 DC)

#define ENCODE_VP8_NUM_MAX_L0_REF     3    // used in LibVa

// ---------------------------
// Structures - used only from DDI
// ---------------------------
typedef struct tagENCODE_QUERY_PROCESSING_RATE_INPUT
{
	uint8_t        Profile;
	uint8_t        Level;
	uint8_t        TargetUsage;
	uint8_t        GopRefDist;
	uint16_t       GopPicSize;
} ENCODE_QUERY_PROCESSING_RATE_INPUT;

//------------------------------------------------------------------------------
// Macros for debug message, Assert, Null check and MOS eStatus check
// within Codec DDI files without the need to explicitly pass comp and sub-comp name
//------------------------------------------------------------------------------

#define CODEC_DDI_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _expr)

#define CODEC_DDI_COND_ASSERTMESSAGE(_expr, _message, ...)                        \
    if (_expr)                                                                       \
    {                                                                                \
        CODEC_DDI_ASSERTMESSAGE(_message, ##__VA_ARGS__) \
    }

#define CODEC_DDI_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define CODEC_DDI_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define CODEC_DDI_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define CODEC_DDI_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI)

#define CODEC_DDI_FUNCTION_EXIT(status)                                          \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, status)

#define CODEC_DDI_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _stmt)

#define CODEC_DDI_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _stmt, _message, ##__VA_ARGS__)

#define CODEC_DDI_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _ptr)

#define CODEC_DDI_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _ptr)

#define CODEC_DDI_CHK_HR(_ptr)                                                    \
    MOS_CHK_HR(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _ptr)

#define CODEC_DDI_CHK_NULL_WITH_HR(_ptr)                                          \
    MOS_CHK_NULL_WITH_HR(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _ptr)

#define CODEC_DDI_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _ptr)

#define CODEC_DDI_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                 \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DDI, _stmt, _message, ##__VA_ARGS__)

// ---------------------------
// Functions - used only from DDI
// ---------------------------
bool GetMbProcessingRateEnc(
	PLATFORM                            platform,
	MEDIA_FEATURE_TABLE                 *skuTable,
        MEDIA_SYSTEM_INFO                   *gtSystemInfo,
	uint32_t                            tuIdx,
    uint32_t                            codechalMode,
	bool                                vdencActive,
	uint32_t*                           mbProcessingRatePerSec
	);

bool GetMbProcessingRateDec(
	PLATFORM                            platform,
	MEDIA_FEATURE_TABLE                 *skuTable,
	uint32_t*                           mbProcessingRatePerSec
	);

#endif  // __CODEC_DDI_H__
