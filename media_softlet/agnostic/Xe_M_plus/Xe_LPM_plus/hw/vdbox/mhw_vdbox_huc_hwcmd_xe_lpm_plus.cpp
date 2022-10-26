/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file   mhw_vdbox_huc_hwcmd_xe_lpm_plus.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

// DO NOT EDIT
#include "mhw_vdbox_huc_hwcmd_xe_lpm_plus.h"

using namespace mhw::vdbox::huc::xe_lpm_plus_base::v0;

Cmd::MEMORYADDRESSATTRIBUTES_CMD::MEMORYADDRESSATTRIBUTES_CMD()
{
    DW0.Value                                        = 0x00000000;
}

Cmd::SPLITBASEADDRESS64BYTEALIGNED_CMD::SPLITBASEADDRESS64BYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;
}

Cmd::SPLITBASEADDRESS4KBYTEALIGNED_CMD::SPLITBASEADDRESS4KBYTEALIGNED_CMD()
{
    DW0_1.Value[0] = DW0_1.Value[1]                  = 0x00000000;
}

Cmd::HUC_PIPE_MODE_SELECT_CMD::HUC_PIPE_MODE_SELECT_CMD()
{
    DW0.Value                                        = 0x75800001;

    DW1.Value                                        = 0x00000000;
  
    DW2.Value                                        = 0x00000000;

}

Cmd::HUC_IMEM_STATE_CMD::HUC_IMEM_STATE_CMD()
{
    DW0.Value                                        = 0x75810003;
   
    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;

}

Cmd::HUC_DMEM_STATE_CMD::HUC_DMEM_STATE_CMD()
{
    DW0.Value                                        = 0x75820004;

    HucDataSourceAttributes.DW0.Value                = 0x00000000;

    DW4.Value                                        = 0x00000000;

    DW5.Value                                        = 0x00000000;

}

Cmd::HUC_CFG_STATE_CMD::HUC_CFG_STATE_CMD()
{
    DW0.Value                                        = 0x75830000;
   
    DW1.Value                                        = 0x00000000;
  
}

Cmd::HUC_VIRTUAL_ADDR_REGION_CMD::HUC_VIRTUAL_ADDR_REGION_CMD()
{
    HucSurfaceVirtualaddrregion015.DW0.Value         = 0x00000000;
}

Cmd::HUC_VIRTUAL_ADDR_STATE_CMD::HUC_VIRTUAL_ADDR_STATE_CMD()
{
    DW0.Value                                        = 0x7584002f;
}

Cmd::HUC_IND_OBJ_BASE_ADDR_STATE_CMD::HUC_IND_OBJ_BASE_ADDR_STATE_CMD()
{
    DW0.Value                                          = 0x75850009;

    HucIndirectStreamInObjectbaseAttributes.DW0.Value  = 0x00000000;

    HucIndirectStreamOutObjectbaseAttributes.DW0.Value = 0x00000000;
}

Cmd::HUC_STREAM_OBJECT_CMD::HUC_STREAM_OBJECT_CMD()
{
    DW0.Value                                        = 0x75a00003;
 
    DW1.Value                                        = 0x00000000;

    DW2.Value                                        = 0x00000000;
 
    DW3.Value                                        = 0x00000000;

    DW4.Value                                        = 0x00000000;
}

Cmd::HUC_START_CMD::HUC_START_CMD()
{
    DW0.Value                                        = 0x75a10000;
   
    DW1.Value                                        = 0x00000000;
 
}

