/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_itf.h
//! \brief    MHW VDBOX VDENC interface common base
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_ITF_H__
#define __MHW_VDBOX_VDENC_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "mhw_utilities.h"
#include "mhw_vdbox.h"

#define _VDENC_CMD_DEF(DEF)               \
    DEF(VDENC_CONTROL_STATE);             \
    DEF(VDENC_PIPE_MODE_SELECT);          \
    DEF(VDENC_SRC_SURFACE_STATE);         \
    DEF(VDENC_REF_SURFACE_STATE);         \
    DEF(VDENC_DS_REF_SURFACE_STATE);      \
    DEF(VDENC_PIPE_BUF_ADDR_STATE);       \
    DEF(VDENC_WEIGHTSOFFSETS_STATE);      \
    DEF(VDENC_HEVC_VP9_TILE_SLICE_STATE); \
    DEF(VDENC_WALKER_STATE);              \
    DEF(VD_PIPELINE_FLUSH);               \
    DEF(VDENC_AVC_SLICE_STATE);           \
    DEF(VDENC_AVC_IMG_STATE);             \
    DEF(VDENC_CMD1);                      \
    DEF(VDENC_CMD2);                      \
    DEF(VDENC_CMD3)

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
class Itf
{
public:
    enum CommandsNumberOfAddresses
    {
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 21
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _VDENC_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetRowstoreCachingOffsets(const RowStorePar &par) = 0;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    virtual bool IsPerfModeSupported() = 0;

    virtual bool IsRhoDomainStatsEnabled() = 0;

    virtual MmioRegistersVdbox *GetMmioRegisters(MHW_VDBOX_NODE_IND index) = 0;
     
    //!
    //! \brief    Convert from Vdbox mmio registers to MI mmio register
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //! \param    [in] mmioRegister
    //!           reference to MHW_MI_MMIOREGISTERS.
    //!
    //! \return   [out] bool
    //!           return true if mmio register if found, otherwise return false
    //!
    virtual inline bool ConvertToMiRegister(MHW_VDBOX_NODE_IND index, MHW_MI_MMIOREGISTERS &mmioRegister)
    {
        MmioRegistersVdbox *vdboxMmioReg = GetMmioRegisters(index);
        if (vdboxMmioReg)
        {
            mmioRegister.generalPurposeRegister0LoOffset  = vdboxMmioReg->generalPurposeRegister0LoOffset;
            mmioRegister.generalPurposeRegister0HiOffset  = vdboxMmioReg->generalPurposeRegister0HiOffset;
            mmioRegister.generalPurposeRegister4LoOffset  = vdboxMmioReg->generalPurposeRegister4LoOffset;
            mmioRegister.generalPurposeRegister4HiOffset  = vdboxMmioReg->generalPurposeRegister4HiOffset;
            mmioRegister.generalPurposeRegister11LoOffset = vdboxMmioReg->generalPurposeRegister11LoOffset;
            mmioRegister.generalPurposeRegister11HiOffset = vdboxMmioReg->generalPurposeRegister11HiOffset;
            mmioRegister.generalPurposeRegister12LoOffset = vdboxMmioReg->generalPurposeRegister12LoOffset;
            mmioRegister.generalPurposeRegister12HiOffset = vdboxMmioReg->generalPurposeRegister12HiOffset;
            return true;
        }
        else
            return false;
    }

    _VDENC_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);

    bool m_perfModeSupported     = false;
    bool m_rhoDomainStatsEnabled = false;
    MHW_VDBOX_NODE_IND m_maxVdboxIndex = MHW_VDBOX_NODE_1;  //!< max vdbox index

    MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__Itf)
};
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_ITF_H__
