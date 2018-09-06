/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file     mhw_mmio_g11.h
//! \brief    Define the MMIO registers access of Gen11 
//! \details  
//!


#ifndef __MHW_MMIO_G11_H__
#define __MHW_MMIO_G11_H__

//Common MI
#define GP_REGISTER0_LO_OFFSET_G11                                                 0x1C8600
#define GP_REGISTER0_HI_OFFSET_G11                                                 0x1C8604
#define GP_REGISTER4_LO_OFFSET_G11                                                 0x1C8620
#define GP_REGISTER4_HI_OFFSET_G11                                                 0x1C8624

//VEBOX
#define WATCHDOG_COUNT_CTRL_OFFSET_RCS_G11                                         0x2178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS_G11                                   0x217C

#define WATCHDOG_COUNT_CTRL_OFFSET_VCS0_G11                                        0x1C0178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0_G11                                  0x1C017C

#define WATCHDOG_COUNT_CTRL_OFFSET_VCS1_G11                                        0x1C4178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1_G11                                  0x1C417C

#define WATCHDOG_COUNT_CTRL_OFFSET_VECS_G11                                        0x1C8178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS_G11                                  0x1C817C

//VDBOX HCP
#define WATCHDOG_COUNT_CTRL_OFFSET_INIT_G11                                        0x1C0178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT_G11                                  0x1C017C
#define HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT_G11                           0x1C2828
#define HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G11                              0x1C28B8
#define HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G11                              0x1C28BC
#define HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G11                    0x1C28A0
#define HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT_G11                  0x1C28A8
#define HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G11          0x1C28A4
#define HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT_G11                                0x1C28C0
#define HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT_G11                                    0x1C28C8
#define HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT_G11                               0x1C28DC
#define HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G11                 0x1C28E0
#define HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G11       0x1C28E4
#define HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G11                          0x1C28F0
#define HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G11                          0x1C28F4
#define CS_ENGINE_ID_OFFSET_INIT_G11                                               0x1C008C
#define HCP_DEC_STATUS_REG_OFFSET_INIT_G11                                         0x1C2800
#define HCP_CABAC_STATUS_REG_OFFSET_INIT_G11                                       0x1C2804

//VDBOX HUC
#define HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_1_INIT_G11                            0x1C2014
#define HUC_STATUS_REG_OFFSET_NODE_1_INIT_G11                                      0x1C2000
#define HUC_STATUS2_REG_OFFSET_NODE_1_INIT_G11                                     0x1C23B0

//VDBOX MFX register offsets
#define GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT_G11                        0x1C0600
#define GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT_G11                        0x1C0604
#define GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT_G11                        0x1C0620
#define GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT_G11                        0x1C0624
#define MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G11                           0x1C08B4
#define MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G11                           0x1C08B8
#define MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT_G11                              0x1C0954
#define MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT_G11                                 0x1C08BC
#define MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT_G11                                  0x1C0800
#define MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT_G11                                   0x1C0850
#define MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT_G11                                    0x1C0868
#define MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G11                   0x1C08A0
#define MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G11                 0x1C08A4
#define MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT_G11                   0x1C08D0
#define MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G11               0x1C0908
#define MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G11                       0x1C0900
#define MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G11                       0x1C0904
#define MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_1_INIT_G11                            0x1C0910
#define MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_1_INIT_G11                         0x1C0914
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_1_INIT_G11               0x1C0918
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_1_INIT_G11               0x1C091C
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_1_INIT_G11            0x1C0920
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_1_INIT_G11            0x1C0924
#define MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_1_INIT_G11                  0x1C0928


//VDBOX MFX register initial values
#define MFX_LRA0_REG_OFFSET_NODE_1_INIT_G11                                        0
#define MFX_LRA1_REG_OFFSET_NODE_1_INIT_G11                                        0
#define MFX_LRA2_REG_OFFSET_NODE_1_INIT_G11                                        0

#endif   //__MHW_MMIO_G11_H__