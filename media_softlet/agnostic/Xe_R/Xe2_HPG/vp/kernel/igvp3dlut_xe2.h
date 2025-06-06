/*
 * Copyright (c) 2025, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

////////////////////////////////////////////////////////////////////////////////
// !!! WARNING - AUTO GENERATED FILE. DO NOT EDIT DIRECTLY. !!!
// Generated by KernelBinToSource.exe tool
////////////////////////////////////////////////////////////////////////////////

#ifndef __IGVP3DLUT_XE2_H__
#define __IGVP3DLUT_XE2_H__

#include "media_bin_mgr.h"
DECLARE_SHARED_ARRAY_SIZE_UINT32(IGVP3DLUT_GENERATION_XE2_SIZE);
DECLARE_SHARED_ARRAY_UINT32(IGVP3DLUT_GENERATION_XE2);

#if defined(MEDIA_BIN_SUPPORT)

#define IGVP3DLUT_GENERATION_XE2_NAME "IGVP3DLUT_XE2"

#if defined(MEDIA_BIN_DLL)
static bool registerIGVP3DLUT_XE2 = RegisterMediaBin(IGVP3DLUT_GENERATION_XE2_NAME, IGVP3DLUT_GENERATION_XE2_SIZE, IGVP3DLUT_GENERATION_XE2);

#endif  // defined(MEDIA_BIN_DLL)

#endif  // defined(MEDIA_BIN_SUPPORT)

#if !defined(MEDIA_BIN_DLL)
#include "igvp3dlut_args.h"
// Following functions are always needed in media driver dll no matter media_bin_supported or not
extern void AddVpNativeKernelEntryToList3dlutXe2(vp::VpPlatformInterface &vpPlatformInterface);
#endif  // !defined(MEDIA_BIN_DLL)

#endif // __IGVP3DLUT_XE2_H__
