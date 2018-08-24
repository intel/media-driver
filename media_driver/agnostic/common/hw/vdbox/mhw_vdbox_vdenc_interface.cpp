/*
* Copyright (c) 2017-2018, Intel Corporation
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

//! \file     mhw_vdbox_vdenc_interface.cpp
//! \brief    MHW interface for constructing Vdenc commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox Vdenc commands across all platforms 
//!

#include "mhw_vdbox_vdenc_interface.h"

const bool MhwVdboxVdencInterface::m_vdencFTQEnabled[NUM_VDENC_TARGET_USAGE_MODES] = {
    0, 1, 1, 1, 1, 1, 1, 0
};

const bool MhwVdboxVdencInterface::m_vdencBlockBasedSkipEnabled[NUM_VDENC_TARGET_USAGE_MODES] = {
    0, 1, 1, 1, 1, 1, 0, 0
};

MhwVdboxVdencInterface::MhwVdboxVdencInterface(PMOS_INTERFACE osInterface)
{
    MHW_FUNCTION_ENTER;

    m_osInterface = osInterface;

    MHW_ASSERT(m_osInterface);

    if (m_osInterface->bUsesGfxAddress)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else // bUsesPatchList
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
}

uint32_t MhwVdboxVdencInterface::MosToMediaStateFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
        return MHW_MEDIASTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
    case Format_422H:
    case Format_422V:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_422_8;
    case Format_AYUV:
    case Format_AUYV:
        return MHW_MEDIASTATE_SURFACEFORMAT_A8Y8U8V8_UNORM;
    case Format_NV12:
    case Format_NV11:
    case Format_P208:
    case Format_IMC1:
    case Format_IMC3:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8;
    case Format_400P:
    case Format_P8:
        return MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM;
    case Format_411P:
    case Format_411R:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8;
    case Format_UYVY:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPY;
    case Format_YVYU:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUV;
    case Format_VYUY:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUVY;
    case Format_YUY2:
    case Format_YUYV:
    case Format_444P:
    case Format_IMC2:
    case Format_IMC4:
    default:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
    }

    return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
}
