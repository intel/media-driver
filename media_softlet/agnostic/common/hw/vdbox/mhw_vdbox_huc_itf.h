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
//! \file     mhw_vdbox_huc_itf.h
//! \brief    MHW VDBOX HUC interface common base
//! \details
//!

#ifndef __MHW_VDBOX_HUC_ITF_H__
#define __MHW_VDBOX_HUC_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_huc_cmdpar.h"

#define _HUC_CMD_DEF(DEF)             \
    DEF(HUC_PIPE_MODE_SELECT);        \
    DEF(HUC_IMEM_STATE);              \
    DEF(HUC_DMEM_STATE);              \
    DEF(HUC_VIRTUAL_ADDR_STATE);      \
    DEF(HUC_START);                   \
    DEF(HUC_IND_OBJ_BASE_ADDR_STATE); \
    DEF(HUC_STREAM_OBJECT)

namespace mhw
{
namespace vdbox
{
namespace huc
{

struct HucMmioRegisters
{
    uint32_t                    hucStatusRegOffset = 0;
    uint32_t                    hucUKernelHdrInfoRegOffset = 0;
    uint32_t                    hucStatus2RegOffset = 0;
};

class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _HUC_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;
    virtual HucMmioRegisters* GetMmioRegisters(MHW_VDBOX_NODE_IND index) = 0;

    _HUC_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
};
}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_ITF_H__
