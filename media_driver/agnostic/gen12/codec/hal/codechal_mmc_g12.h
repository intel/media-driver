/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_mmc_g12.h
//! \brief    Defines the public interface for Gen12 CodecHal Media Memory Compression
//!
#ifndef __CODECHAL_MMC_G12_H__
#define __CODECHAL_MMC_G12_H__

#include "codechal_mmc.h"

//! \class CodecHalMmcStateG12
//! \brief Gen12 common media memory compression state. This class defines the member fields
//!        functions etc used by Gen12 codec memory compression. 
//!
class CodecHalMmcStateG12 : public CodecHalMmcState
{
public:

    //!
    //! \brief    Constructor
    //!
    CodecHalMmcStateG12(
        CodechalHwInterface    *hwInterface);

    //!
    //! \brief    Destructor
    //!
    ~CodecHalMmcStateG12() {};

    MOS_STATUS SetSurfaceParams(
        PCODECHAL_SURFACE_CODEC_PARAMS surfaceParams);

    MOS_STATUS SetSurfaceState(
        PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams,
        PMOS_COMMAND_BUFFER cmdBuffer = nullptr);

    MOS_STATUS SendPrologCmd(
        MhwMiInterface      *miInterface,
        MOS_COMMAND_BUFFER  *cmdBuffer,
        MOS_GPU_CONTEXT     gpuContext);

    //!
    //! \brief    Indicate if mmc is enabled for G12
    //! \details  Suggested to use for G12 instead of static methos IsMmcEnabled, 
    //!           since it can return different values for each encode/decode instance.
    //! \return   bool
    //!
    bool IsMmcEnabledForComponent() { return m_mmcEnabledForComponent; }

protected:

    void InitDecodeMmcEnable(
        CodechalHwInterface    *hwInterface);

    void InitEncodeMmcEnable(
        CodechalHwInterface    *hwInterface);

    bool m_mmcEnabledForComponent = false;   //!< Indicate if mmc is enabled for g12, maybe different for encode/decode instance.
};

#endif  // __CODECHAL_MMC_G12_H__
