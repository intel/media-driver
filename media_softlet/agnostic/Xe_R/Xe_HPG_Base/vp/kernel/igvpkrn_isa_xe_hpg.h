/*
 * Copyright (c) 2021, Intel Corporation
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
#ifndef __IGVPKRN_ISA_XE_HPG_H__
#define __IGVPKRN_ISA_XE_HPG_H__

#include "media_bin_mgr.h"

DECLARE_SHARED_ARRAY_SIZE_UINT32(IGVP3DLUT_GENERATION_XE_HPG_SIZE);
DECLARE_SHARED_ARRAY_UINT32(IGVP3DLUT_GENERATION_XE_HPG);
DECLARE_SHARED_ARRAY_SIZE_UINT32(IGVPHVS_DENOISE_XE_HPG_SIZE);
DECLARE_SHARED_ARRAY_UINT32(IGVPHVS_DENOISE_XE_HPG);

#if defined(MEDIA_BIN_SUPPORT)

#define IGVP3DLUT_GENERATION_XE_HPG_NAME "IGVP3DLUT_GENERATION_XE_HPG"
#define IGVPHVS_DENOISE_XE_HPG_NAME      "IGVPHVS_DENOISE_XE_HPG"

#if defined(MEDIA_BIN_DLL)
static bool registerIGVP3DLUT_GENERATION_XE_HPG = RegisterMediaBin(IGVP3DLUT_GENERATION_XE_HPG_NAME, IGVP3DLUT_GENERATION_XE_HPG_SIZE, IGVP3DLUT_GENERATION_XE_HPG);
static bool registerIGVPHVS_DENOISE_XE_HPG = RegisterMediaBin(IGVPHVS_DENOISE_XE_HPG_NAME, IGVPHVS_DENOISE_XE_HPG_SIZE, IGVPHVS_DENOISE_XE_HPG);
#endif  // defined(MEDIA_BIN_DLL)
#endif  // defined(MEDIA_BIN_SUPPORT)

#endif
