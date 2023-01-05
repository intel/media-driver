/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     igvpkrn_isa_g12_tgllp.h
//! \brief    Head File for Gen12 LP ISA File
//! \details  Head File for Gen12 LP ISA File
//!
#ifndef __IGVPKRN_ISA_G12_TGLLP_H__
#define __IGVPKRN_ISA_G12_TGLLP_H__

#include "media_bin_mgr.h"
DECLARE_SHARED_ARRAY_SIZE_UINT32(IGVP3DLUT_GENERATION_G12_TGLLP_SIZE);
DECLARE_SHARED_ARRAY_UINT32(IGVP3DLUT_GENERATION_G12_TGLLP);

#if defined(MEDIA_BIN_SUPPORT)
#define IGVP3DLUT_GENERATION_G12_TGLLP_NAME      "IGVP3DLUT_GENERATION_G12_TGLLP"

#if defined(MEDIA_BIN_DLL)
static bool registerIGVP3DLUT_GENERATION_G12_TGLLP = RegisterMediaBin(IGVP3DLUT_GENERATION_G12_TGLLP_NAME, IGVP3DLUT_GENERATION_G12_TGLLP_SIZE, IGVP3DLUT_GENERATION_G12_TGLLP);
#endif  // defined(MEDIA_BIN_DLL)

#endif  // defined(MEDIA_BIN_SUPPORT)

#endif // __IGVPKRN_ISA_G12_TGLLP_H__
