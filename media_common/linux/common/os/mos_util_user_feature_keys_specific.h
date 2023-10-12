/*
* Copyright (c) 2009-2022, Intel Corporation
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

/*---------------------------------------------------------------------------\
** This table defines the bits for the Media Reg Internal DWORD.
** ---------------------------------------------------------------------------
** |  Bit  |                          Description                            |
** |-------|-----------------------------------------------------------------|
** | 31:1  | Available                                                       |
** |   0   | Reserved for DL, but not used now                               |
\*--------------------------------------------------------------------------*/
#define MEDIA_USER_SETTING_INTERNAL             0x1
#define MEDIA_USER_SETTING_INTERNAL_REPORT      0x4

#define __MEDIA_USER_FEATURE_VALUE_ENABLE_SOFTPIN       "Enable Softpin"
#define __MEDIA_USER_FEATURE_VALUE_DISABLE_KMD_WATCHDOG "Disable KMD Watchdog"
#define __MEDIA_USER_FEATURE_VALUE_ENABLE_VM_BIND       "Enable VM Bind"

#endif // __MOS_UTIL_USER_FEATURE_KEYS_SPECIFIC_H__
