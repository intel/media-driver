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

// Media Engine 
static constexpr uint32_t M_MMIO_MAX_RELATIVE_OFFSET                                                  = 0x3FFF; //!< Max reg relative offset in an engine
static constexpr uint32_t M_MMIO_MEDIA_LOW_OFFSET                                                     = 0x1C0000; //!< Low bound of VDBox and VEBox MMIO offset
static constexpr uint32_t M_MMIO_MEDIA_HIGH_OFFSET                                                    = 0x2000000;//!< High bound of VDBox and VEBox MMIO offset

//Render 
static constexpr uint32_t M_MMIO_RCS_AUX_TABLE_BASE_LOW                                               = 0x4200;
static constexpr uint32_t M_MMIO_RCS_AUX_TABLE_BASE_HIGH                                              = 0x4204;
static constexpr uint32_t M_MMIO_RCS_AUX_TABLE_INVALIDATE                                             = 0x4208;
static constexpr uint32_t M_MMIO_RCS_HW_FE_REMAP_RANGE_BEGIN                                          = 0x2000;
static constexpr uint32_t M_MMIO_RCS_HW_FE_REMAP_RANGE_END                                            = 0x27FF;
static constexpr uint32_t M_MMIO_RCS_AUX_TBL_REMAP_RANGE_BEGIN                                        = 0x4200;
static constexpr uint32_t M_MMIO_RCS_AUX_TBL_REMAP_RANGE_END                                          = 0x420F;
static constexpr uint32_t M_MMIO_RCS_TRTT_REMAP_RANGE_BEGIN                                           = 0x4400;
static constexpr uint32_t M_MMIO_RCS_TRTT_REMAP_RANGE_END                                             = 0x441F;

static constexpr uint32_t M_MMIO_MEDIA_REG_BASE                                                       = 0X380000; //!<Media Engine Base

//VD
static constexpr uint32_t M_MMIO_VD0_AUX_TABLE_BASE_LOW                                               = 0x4210;
static constexpr uint32_t M_MMIO_VD0_AUX_TABLE_BASE_HIGH                                              = 0x4214;
static constexpr uint32_t M_MMIO_VD0_AUX_TABLE_INVALIDATE                                             = 0x4218;
static constexpr uint32_t M_MMIO_VD1_AUX_TABLE_BASE_LOW                                               = 0x4220;
static constexpr uint32_t M_MMIO_VD1_AUX_TABLE_BASE_HIGH                                              = 0x4224;
static constexpr uint32_t M_MMIO_VD1_AUX_TABLE_INVALIDATE                                             = 0x4228;
static constexpr uint32_t M_MMIO_VD2_AUX_TABLE_BASE_LOW                                               = 0x4290;
static constexpr uint32_t M_MMIO_VD2_AUX_TABLE_BASE_HIGH                                              = 0x4294;
static constexpr uint32_t M_MMIO_VD2_AUX_TABLE_INVALIDATE                                             = 0x4298;
static constexpr uint32_t M_MMIO_VD3_AUX_TABLE_BASE_LOW                                               = 0x42A0;
static constexpr uint32_t M_MMIO_VD3_AUX_TABLE_BASE_HIGH                                              = 0x42A4;
static constexpr uint32_t M_MMIO_VD3_AUX_TABLE_INVALIDATE                                             = 0x42A8;

//VE
static constexpr uint32_t M_MMIO_VE0_AUX_TABLE_BASE_LOW                                               = 0x4230;
static constexpr uint32_t M_MMIO_VE0_AUX_TABLE_BASE_HIGH                                              = 0x4234;
static constexpr uint32_t M_MMIO_VE0_AUX_TABLE_INVALIDATE                                             = 0x4238;
static constexpr uint32_t M_MMIO_VE1_AUX_TABLE_BASE_LOW                                               = 0x42B0;
static constexpr uint32_t M_MMIO_VE1_AUX_TABLE_BASE_HIGH                                              = 0x42B4;
static constexpr uint32_t M_MMIO_VE1_AUX_TABLE_INVALIDATE                                             = 0x42B8;

//Compute
static constexpr uint32_t M_MMIO_CCS0_AUX_TABLE_BASE_LOW                                              = 0x42C0;
static constexpr uint32_t M_MMIO_CCS0_AUX_TABLE_BASE_HIGH                                             = 0x42C4;
static constexpr uint32_t M_MMIO_CCS0_AUX_TABLE_INVALIDATE                                            = 0x42C8;
static constexpr uint32_t M_MMIO_CCS0_HW_FRONT_END_BASE_BEGIN                                         = 0x1A000;
static constexpr uint32_t M_MMIO_CCS0_HW_FRONT_END_BASE_END                                           = 0x1A7FF;
static constexpr uint32_t M_MMIO_CCS1_HW_FRONT_END_BASE_BEGIN                                         = 0x1C000;
static constexpr uint32_t M_MMIO_CCS1_HW_FRONT_END_BASE_END                                           = 0x1C7FF;
static constexpr uint32_t M_MMIO_CCS2_HW_FRONT_END_BASE_BEGIN                                         = 0x1E000;
static constexpr uint32_t M_MMIO_CCS2_HW_FRONT_END_BASE_END                                           = 0x1E7FF;
static constexpr uint32_t M_MMIO_CCS3_HW_FRONT_END_BASE_BEGIN                                         = 0x26000;
static constexpr uint32_t M_MMIO_CCS3_HW_FRONT_END_BASE_END                                           = 0x267FF;

//L3 cache configure
static constexpr uint32_t M_MMIO_RCS_L3ALLOCREG                                                       = 0xB134;
static constexpr uint32_t M_MMIO_CCS0_L3ALLOCREG                                                      = 0xB234;
static constexpr uint32_t M_MMIO_RCS_TCCNTLREG                                                        = 0xB138;
static constexpr uint32_t M_MMIO_CCS0_TCCNTLREG                                                       = 0xB238;

#endif   //__MHW_MMIO_COMMON_H__
