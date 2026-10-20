/*
* Copyright (c) 2024-2026, Intel Corporation
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
//! \file     mhw_vdbox_huc_ppgtt_itf.h
//! \brief    MHW VDBOX Huc PPGTT interface for Xe3P+
//! \details
//!

#ifndef __MHW_VDBOX_HUC_PPGTT_ITF_H__
#define __MHW_VDBOX_HUC_PPGTT_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_huc_ppgtt_cmdpar.h"

// add all HUC PPGTT cmds here
#define _HUC_PPGTT_CMD_DEF(DEF) \
    DEF(HUC_IMEM_ADDR);

namespace mhw
{
namespace vdbox
{
namespace huc
{
class ItfPPGTT
{
public:
    //!
    //! \enum     CommandsNumberOfAddresses
    //! \brief    Commands number of addresses
    //!
    enum CommandsNumberOfAddresses
    {
        HUC_IMEM_ADDR_CMD_NUMBER_OF_ADDRESSES = 1, // 2 DW for 1 address field
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _HUC_PPGTT_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~ItfPPGTT() = default;

    _HUC_PPGTT_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

MEDIA_CLASS_DEFINE_END(mhw__vdbox__huc__ItfPPGTT)
};
}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_PPGTT_ITF_H__
