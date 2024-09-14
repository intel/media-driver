/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     mhw_vdbox_aqm_itf.h
//! \brief    MHW VDBOX AQM interface common base
//! \details
//!

#ifndef __MHW_VDBOX_AQM_ITF_H__
#define __MHW_VDBOX_AQM_ITF_H__

#include "mhw_impl.h"
#include "mhw_vdbox_aqm_cmdpar.h"

#ifdef IGFX_AQM_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_aqm_itf_ext.h"
#endif

#define _AQM_CMD_DEF(DEF)               \
    DEF(AQM_FRAME_START);               \
    DEF(AQM_PIC_STATE);                 \
    DEF(AQM_SURFACE_STATE);             \
    DEF(AQM_PIPE_BUF_ADDR_STATE);       \
    DEF(AQM_TILE_CODING);               \
    DEF(AQM_VD_CONTROL_STATE);          \
    DEF(AQM_SLICE_STATE)

namespace mhw
{
namespace vdbox
{
namespace aqm
{
class Itf
{
public:

    enum CommandsNumberOfAddresses
    {
        AQM_VD_CONTROL_STATE_CMD_NUMBER_OF_ADDRESSES          = 0,
        AQM_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES             = 0,
        AQM_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES       = 16,
        AQM_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                 = 0,
        AQM_TILE_CODING_CMD_NUMBER_OF_ADDRESSES               = 0,
        AQM_FRAME_START_CMD_NUMBER_OF_ADDRESSES               = 0,
        AQM_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,
#if _MEDIA_RESERVED
        __MHW_VDBOX_AQM_WRAPPER_EXT(AQM_CMD_ADDRESS_EXT)
#endif
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _AQM_CMD_DEF(_MHW_SETPAR_DEF);
#if _MEDIA_RESERVED
        _AQM_CMD_DEF_EXT(_MHW_SETPAR_DEF);  
#endif
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    _AQM_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
#if _MEDIA_RESERVED
    _AQM_CMD_DEF_EXT(_MHW_CMD_ALL_DEF_FOR_ITF);
#endif

MEDIA_CLASS_DEFINE_END(mhw__vdbox__aqm__Itf)
};
}  // namespace aqm
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AQM_ITF_H__
