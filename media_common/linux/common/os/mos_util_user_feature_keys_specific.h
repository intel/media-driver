/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     mos_util_user_feature_keys_specific.h
//! \brief    OS specific definitions for mos_util_user_feature_keys
//!

#ifndef __MOS_UTIL_USER_FEATURE_KEYS_SPECIFIC_H__
#define __MOS_UTIL_USER_FEATURE_KEYS_SPECIFIC_H__

#define __MEDIA_USER_FEATURE_SUBKEY_INTERNAL                        "LibVa"
#define __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE                     ""
#define __MEDIA_USER_FEATURE_SUBKEY_PERMANENT                       ""
#define __MEDIA_USER_FEATURE_SUBKEY_REPORT                          "Report"
#define __MEDIA_USER_FEATURE_VALUE_VP_DBG_STATE_DUMP_LOCATION       ""
#define __MOS_USER_FEATURE_KEY_XML_FILEPATH_LOCATION                ""

//!
//! \brief User feature keys to define debug message levels and assertions.
//!

#if MOS_MESSAGES_ENABLED
//!
//! \brief Default setting is only critical prints are enabled, so users are not bombarded with irrelevant messages
//!
#define __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE                (0x1)
#endif // MOS_MESSAGES_ENABLED

#endif // __MOS_UTIL_USER_FEATURE_KEYS_SPECIFIC_H__
