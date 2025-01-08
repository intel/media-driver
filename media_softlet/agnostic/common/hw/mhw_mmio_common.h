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
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER1_LO_OFFSET                                      = 0x2608;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER1_HI_OFFSET                                      = 0x260C;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER2_LO_OFFSET                                      = 0x2610;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER2_HI_OFFSET                                      = 0x2614;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER3_LO_OFFSET                                      = 0x2618;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER3_HI_OFFSET                                      = 0x261C;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER4_LO_OFFSET                                      = 0x2620;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER4_HI_OFFSET                                      = 0x2624;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER11_LO_OFFSET                                     = 0x2658;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER11_HI_OFFSET                                     = 0x265C;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER12_LO_OFFSET                                     = 0x2660;
static constexpr uint32_t  CS_GENERAL_PURPOSE_REGISTER12_HI_OFFSET                                     = 0x2664;
static constexpr uint32_t  CS_PREDICATE_RESULT2                                                        = 0x23BC;

// Vebox register offsets
// Used in Commen MI
static constexpr uint32_t  GP_REGISTER0_LO_OFFSET                                                      = 0x1C8600;
static constexpr uint32_t  GP_REGISTER0_HI_OFFSET                                                      = 0x1C8604;
static constexpr uint32_t  GP_REGISTER1_LO_OFFSET                                                      = 0x1C8608;
static constexpr uint32_t  GP_REGISTER1_HI_OFFSET                                                      = 0x1C860c;
static constexpr uint32_t  GP_REGISTER2_LO_OFFSET                                                      = 0x1C8610;
static constexpr uint32_t  GP_REGISTER2_HI_OFFSET                                                      = 0x1C8614;
static constexpr uint32_t  GP_REGISTER3_LO_OFFSET                                                      = 0x1C8618;
static constexpr uint32_t  GP_REGISTER3_HI_OFFSET                                                      = 0x1C861C;
static constexpr uint32_t  GP_REGISTER4_LO_OFFSET                                                      = 0x1C8620;
static constexpr uint32_t  GP_REGISTER4_HI_OFFSET                                                      = 0x1C8624;
static constexpr uint32_t  GP_REGISTER11_LO_OFFSET                                                     = 0x1C8658;
static constexpr uint32_t  GP_REGISTER11_HI_OFFSET                                                     = 0x1C865C;
static constexpr uint32_t  GP_REGISTER12_LO_OFFSET                                                     = 0x1C8660;
static constexpr uint32_t  GP_REGISTER12_HI_OFFSET                                                     = 0x1C8664;
static constexpr uint32_t  VECS_PREDICATE_RESULT2                                                      = 0X1C83BC;

//VEBOX
static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_RCS                                              = 0x2178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS                                        = 0x217C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_VCS0                                             = 0x1C0178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0                                       = 0x1C017C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_VCS1                                             = 0x1C4178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1                                       = 0x1C417C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_VECS                                             = 0x1C8178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS                                       = 0x1C817C;

static constexpr uint32_t  WATCHDOG_COUNT_CTRL_OFFSET_TEECS                                            = 0x11A178;
static constexpr uint32_t  WATCHDOG_COUNT_THRESTHOLD_OFFSET_TEECS                                      = 0x11A17C;

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
static constexpr uint32_t  CCS_GP_REGISTER1_LO_OFFSET                                                  = 0x1A608;
static constexpr uint32_t  CCS_GP_REGISTER1_HI_OFFSET                                                  = 0x1A60C;
static constexpr uint32_t  CCS_GP_REGISTER2_LO_OFFSET                                                  = 0x1A610;
static constexpr uint32_t  CCS_GP_REGISTER2_HI_OFFSET                                                  = 0x1A614;
static constexpr uint32_t  CCS_GP_REGISTER3_LO_OFFSET                                                  = 0x1A618;
static constexpr uint32_t  CCS_GP_REGISTER3_HI_OFFSET                                                  = 0x1A61C;
static constexpr uint32_t  CCS_PREDICATE_RESULT2                                                       = 0x1A3BC;

//CS_GPR_BCS
static constexpr uint32_t  BCS_GP_REGISTER0_LO_OFFSET                                                  = 0x22600;
static constexpr uint32_t  BCS_GP_REGISTER0_HI_OFFSET                                                  = 0x22604;
static constexpr uint32_t  BCS_GP_REGISTER1_LO_OFFSET                                                  = 0x22608;
static constexpr uint32_t  BCS_GP_REGISTER1_HI_OFFSET                                                  = 0x2260C;
static constexpr uint32_t  BCS_GP_REGISTER2_LO_OFFSET                                                  = 0x22610;
static constexpr uint32_t  BCS_GP_REGISTER2_HI_OFFSET                                                  = 0x22614;
static constexpr uint32_t  BCS_GP_REGISTER3_LO_OFFSET                                                  = 0x22618;
static constexpr uint32_t  BCS_GP_REGISTER3_HI_OFFSET                                                  = 0x2261C;
static constexpr uint32_t  BCS_PREDICATE_RESULT2                                                       = 0x223BC;

//CS_GPR_TEE
static constexpr uint32_t  TEE_GP_REGISTER0_LO_OFFSET                                                  = 0x11A600;
static constexpr uint32_t  TEE_GP_REGISTER0_HI_OFFSET                                                  = 0x11A604;
static constexpr uint32_t  TEE_GP_REGISTER1_LO_OFFSET                                                  = 0x11A608;
static constexpr uint32_t  TEE_GP_REGISTER1_HI_OFFSET                                                  = 0x11A60C;
static constexpr uint32_t  TEE_GP_REGISTER2_LO_OFFSET                                                  = 0x11A610;
static constexpr uint32_t  TEE_GP_REGISTER2_HI_OFFSET                                                  = 0x11A614;
static constexpr uint32_t  TEE_GP_REGISTER3_LO_OFFSET                                                  = 0x11A618;
static constexpr uint32_t  TEE_GP_REGISTER3_HI_OFFSET                                                  = 0x11A61C;
static constexpr uint32_t  TEE_PREDICATE_RESULT2                                                       = 0x11A3BC;

//CS_GPR_VCS
static constexpr uint32_t  VCS_GP_REGISTER0_LO_OFFSET                                                  = 0x1C0600;
static constexpr uint32_t  VCS_GP_REGISTER0_HI_OFFSET                                                  = 0x1C0604;
static constexpr uint32_t  VCS_GP_REGISTER1_LO_OFFSET                                                  = 0x1C0608;
static constexpr uint32_t  VCS_GP_REGISTER1_HI_OFFSET                                                  = 0x1C060C;
static constexpr uint32_t  VCS_GP_REGISTER2_LO_OFFSET                                                  = 0x1C0610;
static constexpr uint32_t  VCS_GP_REGISTER2_HI_OFFSET                                                  = 0x1C0614;
static constexpr uint32_t  VCS_GP_REGISTER3_LO_OFFSET                                                  = 0x1C0618;
static constexpr uint32_t  VCS_GP_REGISTER3_HI_OFFSET                                                  = 0x1C061C;
static constexpr uint32_t  VCS_PREDICATE_RESULT2                                                       = 0x1C03BC;
#endif   //__MHW_MMIO_COMMON_H__
