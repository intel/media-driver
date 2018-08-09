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
//! \file     mhw_mmio_g10.h
//! \brief    Define the MMIO registers access of Gen10 
//! \details  
//!

#ifndef __MHW_MMIO_G10_H__
#define __MHW_MMIO_G10_H__

//Common MI
#define GP_REGISTER0_LO_OFFSET_G10                                           0x1A600
#define GP_REGISTER0_HI_OFFSET_G10                                           0x1A604
#define GP_REGISTER4_LO_OFFSET_G10                                           0x1A620
#define GP_REGISTER4_HI_OFFSET_G10                                           0x1A624

//VEBOX                                                                                                                                  
#define WATCHDOG_COUNT_CTRL_OFFSET_RCS_G10                                   0x2178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_RCS_G10                             0x217C

#define WATCHDOG_COUNT_CTRL_OFFSET_VCS0_G10                                  0x12178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS0_G10                            0x1217C

#define WATCHDOG_COUNT_CTRL_OFFSET_VCS1_G10                                  0x1C178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VCS1_G10                            0x1C17C

#define WATCHDOG_COUNT_CTRL_OFFSET_VECS_G10                                  0x1A178
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS_G10                            0x1A17C

//VDBOX HCP register offsets
#define HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G10                        0x1E9B8
#define HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G10                        0x1E9BC
#define HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G10              0x1E9A0
#define HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT_G10            0x1E9A8
#define HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G10    0x1E9A4
#define HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT_G10                          0x1E9C0
#define HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT_G10                              0x1E9C8
#define HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT_G10                         0x1E9DC
#define HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G10           0x1E9E0
#define HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G10 0x1E9E4
#define HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G10                    0x1E9F0
#define HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G10                    0x1E9F4
#define HCP_DEC_STATUS_REG_OFFSET_INIT_G10                                   0x1E900
#define HCP_CABAC_STATUS_REG_OFFSET_INIT_G10                                 0x1E904

//VDBOX HCP register initial values
#define WATCHDOG_COUNT_CTRL_OFFSET_INIT_G10                                  0
#define WATCHDOG_COUNT_THRESTHOLD_OFFSET_INIT_G10                            0
#define HCP_DEBUG_FE_STREAM_OUT_SIZE_REG_OFFSET_INIT_G10                     0
#define CS_ENGINE_ID_OFFSET_INIT_G10                                         0


//VDBOX HUC
#define HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_1_INIT_G10                      0x0D014
#define HUC_STATUS_REG_OFFSET_NODE_1_INIT_G10                                0x0D000
#define HUC_STATUS2_REG_OFFSET_NODE_1_INIT_G10                               0x0D3B0

#define HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_2_INIT_G10                      0x14814
#define HUC_STATUS_REG_OFFSET_NODE_2_INIT_G10                                0x14800
#define HUC_STATUS2_REG_OFFSET_NODE_2_INIT_G10                               0x14BB0

//VDBOX MFX register offsets 
#define GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT_G10                  0x12600
#define GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT_G10                  0x12604
#define GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT_G10                  0x12620
#define GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT_G10                  0x12624
#define MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G10                     0x128B4
#define MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G10                     0x128B8
#define MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT_G10                        0x12954
#define MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT_G10                           0x128BC
#define MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT_G10                            0x12800
#define MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT_G10                             0x12850
#define MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT_G10                              0x12868
#define MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G10             0x128A0
#define MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G10           0x128A4
#define MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT_G10             0x128D0
#define MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G10         0x12908
#define MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G10                 0x12900
#define MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G10                 0x12904
#define MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_1_INIT_G10                      0x12910
#define MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_1_INIT_G10                   0x12914
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_1_INIT_G10         0x12918
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_1_INIT_G10         0X1291C
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_1_INIT_G10      0X12920
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_1_INIT_G10      0X12924
#define MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_1_INIT_G10            0X12928

#define GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_2_INIT_G10                  0x1C600
#define GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_2_INIT_G10                  0x1C604
#define GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_2_INIT_G10                  0x1C620
#define GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_2_INIT_G10                  0x1C624
#define MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_2_INIT_G10                     0x1C8B4
#define MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_2_INIT_G10                     0x1C8B8
#define MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_2_INIT_G10                        0x1C954
#define MFC_QP_STATUS_COUNT_OFFSET_NODE_2_INIT_G10                           0x1C8BC
#define MFX_ERROR_FLAG_REG_OFFSET_NODE_2_INIT_G10                            0x1C800
#define MFX_FRAME_CRC_REG_OFFSET_NODE_2_INIT_G10                             0x1C850
#define MFX_MB_COUNT_REG_OFFSET_NODE_2_INIT_G10                              0x1C868
#define MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G10             0x1C8A0
#define MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G10           0x1C8A4
#define MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_2_INIT_G10             0x1C8D0
#define MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G10         0x1C908
#define MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_2_INIT_G10                 0x1C900
#define MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_2_INIT_G10                 0x1C904
#define MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_2_INIT_G10                      0x1C910
#define MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_2_INIT_G10                   0x1C914
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_2_INIT_G10         0x1C918
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_2_INIT_G10         0x1C91C
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_2_INIT_G10      0x1C920
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_2_INIT_G10      0x1C924
#define MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_2_INIT_G10            0x1C928

//VDBOX MFX register initial values
#define MFX_LRA0_REG_OFFSET_NODE_1_INIT_G10                                  0
#define MFX_LRA1_REG_OFFSET_NODE_1_INIT_G10                                  0
#define MFX_LRA2_REG_OFFSET_NODE_1_INIT_G10                                  0

#define MFX_LRA0_REG_OFFSET_NODE_2_INIT_G10                                  0
#define MFX_LRA1_REG_OFFSET_NODE_2_INIT_G10                                  0
#define MFX_LRA2_REG_OFFSET_NODE_2_INIT_G10                                  0

#endif   //__MHW_MMIO_G10_H__