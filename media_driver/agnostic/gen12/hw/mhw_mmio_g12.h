/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file     mhw_mmio_g12.h
//! \brief    Define the MMIO registers access of Gen12 
//! \details  
//!

#ifndef __MHW_MMIO_G12_H__
#define __MHW_MMIO_G12_H__

// CS register offsets
#define CS_GENERAL_PURPOSE_REGISTER0_LO_OFFSET_G12                                       0x2600
#define CS_GENERAL_PURPOSE_REGISTER0_HI_OFFSET_G12                                       0x2604
#define CS_GENERAL_PURPOSE_REGISTER4_LO_OFFSET_G12                                       0x2620
#define CS_GENERAL_PURPOSE_REGISTER4_HI_OFFSET_G12                                       0x2624
#define CS_GENERAL_PURPOSE_REGISTER11_LO_OFFSET_G12                                      0x2658
#define CS_GENERAL_PURPOSE_REGISTER11_HI_OFFSET_G12                                      0x265C
#define CS_GENERAL_PURPOSE_REGISTER12_LO_OFFSET_G12                                      0x2660
#define CS_GENERAL_PURPOSE_REGISTER12_HI_OFFSET_G12                                      0x2664

// Vebox register offsets
// Used in Commen MI
#define GP_REGISTER0_LO_OFFSET_G12                                                       0x1C8600
#define GP_REGISTER0_HI_OFFSET_G12                                                       0x1C8604
#define GP_REGISTER4_LO_OFFSET_G12                                                       0x1C8620
#define GP_REGISTER4_HI_OFFSET_G12                                                       0x1C8624
#define GP_REGISTER11_LO_OFFSET_G12                                                      0x1C8658
#define GP_REGISTER11_HI_OFFSET_G12                                                      0x1C865C
#define GP_REGISTER12_LO_OFFSET_G12                                                      0x1C8660
#define GP_REGISTER12_HI_OFFSET_G12                                                      0x1C8664

//VEBOX
#define WATCHDOG_COUNT_CTRL_OFFSET_RCS_G12                                               0x2178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS_G12                                         0x217C

#define WATCHDOG_COUNT_CTRL_OFFSET_VCS0_G12                                              0x1C0178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0_G12                                        0x1C017C

#define WATCHDOG_COUNT_CTRL_OFFSET_VCS1_G12                                              0x1C4178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1_G12                                        0x1C417C

#define WATCHDOG_COUNT_CTRL_OFFSET_VECS_G12                                              0x1C8178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS_G12                                        0x1C817C

//VDBOX HCP
#define WATCHDOG_COUNT_CTRL_OFFSET_INIT_G12                                              0x1C0178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT_G12                                        0x1C017C
#define HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT_G12                                 0x1C2828
#define HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G12                                    0x1C28B8
#define HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G12                                    0x1C28BC
#define HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G12                          0x1C28A0
#define HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT_G12                        0x1C28A8
#define HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G12                0x1C28A4
#define HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT_G12                                      0x1C28C0
#define HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT_G12                                          0x1C28C8
#define HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT_G12                                     0x1C28DC
#define HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G12                       0x1C28E0
#define HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G12             0x1C28E4
#define HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G12                                0x1C28F0
#define HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G12                                0x1C28F4
#define CS_ENGINE_ID_OFFSET_INIT_G12                                                     0x1C008C
#define HCP_DEC_STATUS_REG_OFFSET_INIT_G12                                               0x1C2800
#define HCP_CABAC_STATUS_REG_OFFSET_INIT_G12                                             0x1C2804
#define HCP_FRAME_CRC_REG_OFFSET_INIT_G12                                                0x1C2920


//VDBOX HUC
#define HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_1_INIT_G12                                  0x1C2014
#define HUC_STATUS_REG_OFFSET_NODE_1_INIT_G12                                            0x1C2000
#define HUC_STATUS2_REG_OFFSET_NODE_1_INIT_G12                                           0x1C23B0


//VDBOX MFX register offsets 
#define GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT_G12                              0x1C0600
#define GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT_G12                              0x1C0604
#define GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT_G12                              0x1C0620
#define GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT_G12                              0x1C0624
#define GENERAL_PURPOSE_REGISTER11_LO_OFFSET_NODE_1_INIT_G12                             0x1C0658
#define GENERAL_PURPOSE_REGISTER11_HI_OFFSET_NODE_1_INIT_G12                             0x1C065C
#define GENERAL_PURPOSE_REGISTER12_LO_OFFSET_NODE_1_INIT_G12                             0x1C0660
#define GENERAL_PURPOSE_REGISTER12_HI_OFFSET_NODE_1_INIT_G12                             0x1C0664
#define MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G12                                 0x1C08B4
#define MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G12                                 0x1C08B8
#define MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT_G12                                    0x1C0954
#define MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT_G12                                       0x1C08BC
#define MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT_G12                                        0x1C0800
#define MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT_G12                                         0x1C0850
#define MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT_G12                                          0x1C0868
#define MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G12                         0x1C08A0
#define MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G12                       0x1C08A4
#define MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT_G12                         0x1C08D0

//VDBOX VDENC
#define M_VDBOX_VDENC_REG_BASE                                                          {0x1C2D00, 0x1C6D00, 0x1D2D00, 0x1D6D00}


//VDBOX MFX register initial value
#define MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G12                     0
#define MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G12                             0
#define MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G12                             0
#define MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_1_INIT_G12                                  0
#define MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_1_INIT_G12                               0
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_1_INIT_G12                     0
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_1_INIT_G12                     0
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_1_INIT_G12                  0
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_1_INIT_G12                  0
#define MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_1_INIT_G12                        0
#define MFX_LRA0_REG_OFFSET_NODE_1_INIT_G12                                              0
#define MFX_LRA1_REG_OFFSET_NODE_1_INIT_G12                                              0
#define MFX_LRA2_REG_OFFSET_NODE_1_INIT_G12                                              0





// Media Engine 
#define M_MMIO_MAX_RELATIVE_OFFSET                                                       0x3FFF //!< Max reg relative offset in an engine
#define M_MMIO_MEDIA_LOW_OFFSET                                                          0x1C0000 //!< Low bound of VDBox and VEBox MMIO offset
#define M_MMIO_MEDIA_HIGH_OFFSET                                                         0x200000 //!< High bound of VDBox and VEBox MMIO offset


//Render 
#define M_MMIO_RCS_AUX_TABLE_BASE_LOW                                                    0x4200
#define M_MMIO_RCS_AUX_TABLE_BASE_HIGH                                                   0x4204
#define M_MMIO_RCS_AUX_TABLE_INVALIDATE                                                  0x4208
#define M_MMIO_RCS_HW_FE_REMAP_RANGE_BEGIN                                               0x2000
#define M_MMIO_RCS_HW_FE_REMAP_RANGE_END                                                 0x27FF
#define M_MMIO_RCS_AUX_TBL_REMAP_RANGE_BEGIN                                             0x4200
#define M_MMIO_RCS_AUX_TBL_REMAP_RANGE_END                                               0x420F
#define M_MMIO_RCS_TRTT_REMAP_RANGE_BEGIN                                                0x4400
#define M_MMIO_RCS_TRTT_REMAP_RANGE_END                                                  0x441F

//VD
#define M_MMIO_VD0_AUX_TABLE_BASE_LOW                                                    0x4210
#define M_MMIO_VD0_AUX_TABLE_BASE_HIGH                                                   0x4214
#define M_MMIO_VD0_AUX_TABLE_INVALIDATE                                                  0x4218
#define M_MMIO_VD1_AUX_TABLE_BASE_LOW                                                    0x4220
#define M_MMIO_VD1_AUX_TABLE_BASE_HIGH                                                   0x4224
#define M_MMIO_VD1_AUX_TABLE_INVALIDATE                                                  0x4228
#define M_MMIO_VD2_AUX_TABLE_BASE_LOW                                                    0x4290
#define M_MMIO_VD2_AUX_TABLE_BASE_HIGH                                                   0x4294
#define M_MMIO_VD2_AUX_TABLE_INVALIDATE                                                  0x4298
#define M_MMIO_VD3_AUX_TABLE_BASE_LOW                                                    0x42A0
#define M_MMIO_VD3_AUX_TABLE_BASE_HIGH                                                   0x42A4
#define M_MMIO_VD3_AUX_TABLE_INVALIDATE                                                  0x42A8

//VE
#define M_MMIO_VE0_AUX_TABLE_BASE_LOW                                                    0x4230
#define M_MMIO_VE0_AUX_TABLE_BASE_HIGH                                                   0x4234
#define M_MMIO_VE0_AUX_TABLE_INVALIDATE                                                  0x4238
#define M_MMIO_VE1_AUX_TABLE_BASE_LOW                                                    0x42B0
#define M_MMIO_VE1_AUX_TABLE_BASE_HIGH                                                   0x42B4
#define M_MMIO_VE1_AUX_TABLE_INVALIDATE                                                  0x42B8

//Compute
#define M_MMIO_CCS0_AUX_TABLE_BASE_LOW                                                   0x42C0
#define M_MMIO_CCS0_AUX_TABLE_BASE_HIGH                                                  0x42C4
#define M_MMIO_CCS0_AUX_TABLE_INVALIDATE                                                 0x42C8

//L3 cache configure
#define M_MMIO_RCS_L3ALLOCREG                                                            0xB134
#define M_MMIO_CCS0_L3ALLOCREG                                                           0xB234
#define M_MMIO_RCS_TCCNTLREG                                                             0xB138
#define M_MMIO_CCS0_TCCNTLREG                                                            0xB238

// HAL
#define REG_GPR_BASE_G12                                                                 CS_GENERAL_PURPOSE_REGISTER0_LO_OFFSET_G12
#define REG_TIMESTAMP_BASE_G12                                                           0x2358

#endif   //__MHW_MMIO_G12_H__
