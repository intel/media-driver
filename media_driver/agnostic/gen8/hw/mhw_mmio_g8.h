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
//! \file     mhw_mmio_g8.h
//! \brief    Define the MMIO registers access of Gen8 
//! \details  
//!


#ifndef __MHW_MMIO_G8_H__
#define __MHW_MMIO_G8_H__


//Common MI 
#define GP_REGISTER0_LO_OFFSET_G8                                           0x1A600
#define GP_REGISTER0_HI_OFFSET_G8                                           0x1A604
#define GP_REGISTER4_LO_OFFSET_G8                                           0x1A620
#define GP_REGISTER4_HI_OFFSET_G8                                           0x1A624

//VDBOX MFX register offsets
#define GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT_G8                  0x12600
#define GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT_G8                  0x12604
#define GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT_G8                  0x12620
#define GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT_G8                  0x12624
#define MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G8                     0x128B4
#define MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G8                     0x128B8
#define MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT_G8                           0x128BC
#define MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT_G8                            0x12800
#define MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT_G8                             0x12850
#define MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT_G8                              0x12868
#define MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G8             0x128A0
#define MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G8           0x128A4
#define MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT_G8             0x128D0
#define MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G8         0x12908
#define MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G8                 0x12900
#define MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G8                 0x12904
#define MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_1_INIT_G8                      0x12910
#define MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_1_INIT_G8                   0x12914
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_1_INIT_G8         0x12918
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_1_INIT_G8         0X1291C
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_1_INIT_G8      0X12920
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_1_INIT_G8      0X12924
#define MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_1_INIT_G8            0X12928

#define GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_2_INIT_G8                  0x1C600
#define GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_2_INIT_G8                  0x1C604
#define GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_2_INIT_G8                  0x1C620
#define GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_2_INIT_G8                  0x1C624
#define MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_2_INIT_G8                     0x1C8B4
#define MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_2_INIT_G8                     0x1C8B8
#define MFC_QP_STATUS_COUNT_OFFSET_NODE_2_INIT_G8                           0x1C8BC
#define MFX_ERROR_FLAG_REG_OFFSET_NODE_2_INIT_G8                            0x1C800
#define MFX_FRAME_CRC_REG_OFFSET_NODE_2_INIT_G8                             0x1C850
#define MFX_MB_COUNT_REG_OFFSET_NODE_2_INIT_G8                              0x1C868
#define MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G8             0x1C8A0
#define MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G8           0x1C8A4
#define MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_2_INIT_G8             0x1C8D0
#define MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G8         0x1C908
#define MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_2_INIT_G8                 0x1C900
#define MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_2_INIT_G8                 0x1C904
#define MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_2_INIT_G8                      0x1C910
#define MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_2_INIT_G8                   0x1C914
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_2_INIT_G8         0X1C918
#define MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_2_INIT_G8         0X1C91C
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_2_INIT_G8      0X1C920
#define MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_2_INIT_G8      0X1C924
#define MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_2_INIT_G8            0X1C928


//VDBOX MFX register initial values
#define MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT_G8                        0
#define MFX_LRA0_REG_OFFSET_NODE_1_INIT_G8                                  0
#define MFX_LRA1_REG_OFFSET_NODE_1_INIT_G8                                  0
#define MFX_LRA2_REG_OFFSET_NODE_1_INIT_G8                                  0
#define MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_2_INIT_G8                        0
#define MFX_LRA0_REG_OFFSET_NODE_2_INIT_G8                                  0
#define MFX_LRA1_REG_OFFSET_NODE_2_INIT_G8                                  0
#define MFX_LRA2_REG_OFFSET_NODE_2_INIT_G8                                  0 

// RENDER 
#define L3_CACHE_SQC1_REG_OFFSET_G8                                         0xB100
#define L3_CACHE_SQC1_REG_VALUE_G8                                         (0x00610000)
#define L3_CACHE_CNTL_REG_VALUE_ALLOCATE_SLM_D0WA_G8                        0x60000021

// SLM     URB    Rest   DC       RO     I/S     C     T      Sum
// {192,   128,    0,    256,     128,   0,      0,    0       }
#define CM_L3_CACHE_CONFIG_SQCREG1_VALUE_G8                                 0x00610000
#define CM_L3_CACHE_CONFIG_CNTLREG_VALUE_G8                                 0x00808021

//| MMIO register offsets used for the EU debug support
#define INSTPM_SET_BITS_G8                                                 (0x3 << 13)

                              
#endif   //__MHW_MMIO_G8_H__