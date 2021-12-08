/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_vdbox_huc_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_HUC_CMDPAR_H__
#define __MHW_VDBOX_HUC_CMDPAR_H__


#include "mhw_vdbox_cmdpar.h"
#include "encode_utils.h"
#include "mhw_vdbox_huc_def.h"

namespace mhw
{
namespace vdbox
{
namespace huc
{

struct _MHW_PAR_T(HUC_PIPE_MODE_SELECT)
{
    uint32_t mode                       = 0;
    uint32_t mediaSoftResetCounterValue = 0;
    bool     streamOutEnabled           = false;
    bool     disableProtectionSetting   = false;   
};


struct _MHW_PAR_T(HUC_IND_OBJ_BASE_ADDR_STATE)
{
    PMOS_RESOURCE DataBuffer            = nullptr;       
    uint32_t      DataOffset            = 0;
    uint32_t      DataSize              = 0; 
    PMOS_RESOURCE StreamOutObjectBuffer = nullptr;    
    uint32_t      StreamOutObjectOffset = 0;
    uint32_t      StreamOutObjectSize   = 0;
};

struct _MHW_PAR_T(HUC_STREAM_OBJECT)
{
    uint32_t IndirectStreamInDataLength      = 0;
    uint32_t IndirectStreamInStartAddress    = 0;
    bool     HucProcessing = false;
    uint32_t IndirectStreamOutStartAddress   = 0;
    uint8_t  StreamOut                       = 0;
    uint8_t  HucBitstreamEnable              = 0;
    uint8_t  EmulationPreventionByteRemoval  = 0;
    uint8_t  StartCodeSearchEngine           = 0;
    uint8_t  Drmlengthmode                   = 0;
    uint8_t  StartCodeByte2                  = 0;
    uint8_t  StartCodeByte1                  = 0;
    uint8_t  StartCodeByte0                  = 0;    
};

struct _MHW_PAR_T(HUC_IMEM_STATE)
{
    uint32_t      kernelWopcmOffset    = 0;  // 32KB-aligned kernel offset in WOPCM
    uint32_t      kernelDescriptor     = 0;  // kernel descriptor
    PMOS_RESOURCE hucBinaryImageBuffer = nullptr;   
};

struct _MHW_PAR_T(HUC_DMEM_STATE)
{
    HuCFunction   function      = BRC_INIT;
    uint8_t       passNum       = 0;
    uint8_t       currentPass   = 0;
    uint32_t      dataLength    = 0;        // length in bytes of the HUC data. Must be in increments of 64B
    uint32_t      dmemOffset    = 0;        // DMEM offset in the HuC Kernel. This is different for ViperOS vs GEMS.
    PMOS_RESOURCE hucDataSource = nullptr;  // resource for HuC data source
};

struct _MHW_PAR_T(HUC_VIRTUAL_ADDR_STATE)
{
    HuCFunction                 function = BRC_INIT;    

    MHW_VDBOX_HUC_REGION_PARAMS regionParams[16];    
};

struct _MHW_PAR_T(HUC_START)
{
    bool lastStreamObject = true;
};

}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_CMDPAR_H__
