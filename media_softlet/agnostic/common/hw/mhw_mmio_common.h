/*
* Copyright (c) 2221, Intel Corporation
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
//! \file     mhw_mmio_common.h
//! \brief    Define the MMIO registers access of common platform
//! \details  
//!

#ifndef __MHW_MMIO_COMMON_H__
#define __MHW_MMIO_COMMON_H__

#include <stdint.h>

// CS register offsets
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER0_LO_OFFSET                                      = 0x2600;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER0_HI_OFFSET                                      = 0x2604;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER4_LO_OFFSET                                      = 0x2620;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER4_HI_OFFSET                                      = 0x2624;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER11_LO_OFFSET                                     = 0x2658;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER11_HI_OFFSET                                     = 0x265C;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER12_LO_OFFSET                                     = 0x2660;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER12_HI_OFFSET                                     = 0x2664;

// Vebox register offsets
// Used in Commen MI
static constexpr uint32_t  GP_REGISTER0_LO_OFFSET                                                      = 0x1C8600;
static constexpr uint32_t  GP_REGISTER0_HI_OFFSET                                                      = 0x1C8604;
static constexpr uint32_t  GP_REGISTER4_LO_OFFSET                                                      = 0x1C8620;
static constexpr uint32_t  GP_REGISTER4_HI_OFFSET                                                      = 0x1C8624;
static constexpr uint32_t  GP_REGISTER11_LO_OFFSET                                                     = 0x1C8658;
static constexpr uint32_t  GP_REGISTER11_HI_OFFSET                                                     = 0x1C865C;
static constexpr uint32_t  GP_REGISTER12_LO_OFFSET                                                     = 0x1C8660;
static constexpr uint32_t  GP_REGISTER12_HI_OFFSET                                                     = 0x1C8664;

//VEBOX
static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_RCS                                              = 0x2178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS                                        = 0x217C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_VCS0                                             = 0x1C0178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0                                       = 0x1C017C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_VCS1                                             = 0x1C4178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1                                       = 0x1C417C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_VECS                                             = 0x1C8178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS                                       = 0x1C817C;

//Semaphore Token
static constexpr uint32_t   MMIO_SEMAPHORE_TOKEN_RCS                                                   = 0x022b4;
static constexpr uint32_t   MMIO_SEMAPHORE_TOKEN_CCS0                                                  = 0x1A2B4;
static constexpr uint32_t   MMIO_SEMAPHORE_TOKEN_BCS0                                                  = 0x222B4;
static constexpr uint32_t   MMIO_SEMAPHORE_TOKEN_TEE                                                   = 0x11A2B4;
static constexpr uint32_t   MMIO_SEMAPHORE_TOKEN_VCS0                                                  = 0x1C02B4;
static constexpr uint32_t   MMIO_SEMAPHORE_TOKEN_VECS0                                                 = 0x1C82B4;

//CS_GPR_CCS
static constexpr uint32_t  CCS_GP_REGISTER0_LO_OFFSET                                                  = 0x1A600;
static constexpr uint32_t  CCS_GP_REGISTER0_HI_OFFSET                                                  = 0x1A604;

//CS_GPR_BCS
static constexpr uint32_t  BCS_GP_REGISTER0_LO_OFFSET                                                  = 0x22600;
static constexpr uint32_t  BCS_GP_REGISTER0_HI_OFFSET                                                  = 0x22604;

//CS_GPR_TEE
static constexpr uint32_t  TEE_GP_REGISTER0_LO_OFFSET                                                  = 0x11A600;
static constexpr uint32_t  TEE_GP_REGISTER0_HI_OFFSET                                                  = 0x11A604;

//CS_GPR_VCS
static constexpr uint32_t VCS_GP_REGISTER0_LO_OFFSET                                                   = 0x1C0600;
static constexpr uint32_t VCS_GP_REGISTER0_HI_OFFSET                                                   = 0x1C0604;
#endif   //__MHW_MMIO_COMMON_H__
