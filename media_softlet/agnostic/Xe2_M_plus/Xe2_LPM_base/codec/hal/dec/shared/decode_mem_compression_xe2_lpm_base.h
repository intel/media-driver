/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_mem_compression_xe2_lpm_base.h
//! \brief    Defines the common interface for decode mmc
//! \details  The decode mmc is to handle mmc operations
//!

#ifndef __MEDIA_DECODE_MEM_COMPRESSION_XE2_LPM_BASE_H__
#define __MEDIA_DECODE_MEM_COMPRESSION_XE2_LPM_BASE_H__

#include "decode_mem_compression.h"

class DecodeMemCompXe2_Lpm_Base : public DecodeMemComp
{
    public:
    //!
    //! \brief    Construct
    //!
    DecodeMemCompXe2_Lpm_Base(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Destructor
    //!
    virtual ~DecodeMemCompXe2_Lpm_Base() {};

    //!
    //! \brief    SendPrologCmd
    //!
    virtual MOS_STATUS SendPrologCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool bRcsIsUsed);
    //
    //!
    //! \brief    SetSurfaceMmcState
    //!
    virtual MOS_STATUS SetSurfaceMmcState(
        PMOS_SURFACE surface);
    //!
    //! \brief    GetSurfaceMmcState
    //!
    virtual MOS_STATUS GetSurfaceMmcState(
        PMOS_SURFACE       surface,
        MOS_MEMCOMP_STATE *mmcMode) override;

    //!
    //! \brief    SetSurfaceMmcMode
    //!
    virtual MOS_STATUS SetSurfaceMmcMode(
        PMOS_SURFACE surface) override;

    //!
    //! \brief    GetResourceMmcState
    //!
    virtual MOS_STATUS GetResourceMmcState(
        PMOS_RESOURCE      resource,
        MOS_MEMCOMP_STATE &mmcMode) override;

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface) override;
#endif

    MEDIA_CLASS_DEFINE_END(DecodeMemCompXe2_Lpm_Base)
};

#endif //__MEDIA_DECODE_MEM_COMPRESSION_XE2_LPM_BASE_H__
