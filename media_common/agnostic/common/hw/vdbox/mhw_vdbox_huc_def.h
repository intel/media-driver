/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file      mhw_vdbox_huc_def.h  
//!
#ifndef _MHW_VDBOX_HUC_DEF_H_
#define _MHW_VDBOX_HUC_DEF_H_

#include "mos_os_specific.h"

// structure for HuC VIRTUAL_ADDR commands
typedef struct _MHW_VDBOX_HUC_REGION_PARAMS
{
    PMOS_RESOURCE               presRegion;
    uint32_t                    dwOffset;
    bool                        isWritable;
} MHW_VDBOX_HUC_REGION_PARAMS, *PMHW_VDBOX_HUC_REGION_PARAMS;

#define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)
//!
//! \enum     CommandsNumberOfAddresses
//! \brief    Commands number of addresses
//!
enum CommandsNumberOfAddresses
{
    MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               = 1,  //  2 DW for  1 address field
    MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     = 1,  //  2 DW for  1 address field
    MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES = 1,  //  2 DW for  1 address field
    MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           = 1,  //  2 DW for  1 address field
    MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                 = 2,  //  4 DW for  2 address fields

    VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES = 0,  //  0 DW for  0 address fields

    HUC_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES        = 0,   //  0 DW for    address fields
    HUC_IMEM_STATE_CMD_NUMBER_OF_ADDRESSES              = 0,   //  0 DW for    address fields
    HUC_DMEM_STATE_CMD_NUMBER_OF_ADDRESSES              = 2,   //  3 DW for  2 address fields
    HUC_VIRTUAL_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES      = 16,  // 32 DW for 16 address fields
    HUC_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 4,   //  8 DW for  4 address fields
    HUC_STREAM_OBJECT_CMD_NUMBER_OF_ADDRESSES           = 0,   //  0 DW for    address fields
    HUC_START_CMD_NUMBER_OF_ADDRESSES                   = 0,   //  0 DW for    address fields
};

#endif
