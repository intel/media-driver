/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     media_bin_register_g12_tgllp.h
//!
#ifndef __MEDIA_BIN_REGISTER_G12_TGLLP_H__
#define __MEDIA_BIN_REGISTER_G12_TGLLP_H__

#include "igvpkrn_isa_g12_tgllp.h"

#ifdef IGFX_GEN12_TGLLP_CMFC_SUPPORTED
#include "igvpkrn_g12_tgllp_cmfc.h"
#endif

#ifdef IGFX_GEN12_TGLLP_CMFCPATCH_SUPPORTED
#include "igvpkrn_g12_tgllp_cmfcpatch.h"
#endif

#ifdef IGFX_GEN12_TGLLP_CMFCCMLPCH_SUPPORTED
#include "igvpkrn_g12_tgllp_cmfccmlpch.h"
#endif

#endif // __MEDIA_BIN_REGISTER_G12_TGLLP_H__
