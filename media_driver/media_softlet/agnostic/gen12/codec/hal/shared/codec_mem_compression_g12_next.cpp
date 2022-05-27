/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     codec_mem_compression_g12_next.cpp
//! \brief    Defines the common interface for codec mmc.
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of codec
//!

#include "codec_mem_compression_g12_next.h"

MOS_STATUS CodecMmcAuxTableG12Next::LoadAuxTableMmio(
    PMOS_INTERFACE      osItf,
    mhw::mi::Itf        &miItf,
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                bRcsIsUsed)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, cmdBuffer);

    uint64_t auxTableBaseAddr = osItf->pfnGetAuxTableBaseAddr(osItf);

    if (0 != auxTableBaseAddr)
    {
        auto &lriParams = miItf.MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();

        if (bRcsIsUsed)
        {
            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_RCS_AUX_TABLE_BASE_LOW);
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_RCS_AUX_TABLE_BASE_HIGH);
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_CCS0_AUX_TABLE_BASE_LOW);
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_CCS0_AUX_TABLE_BASE_HIGH);
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);
        }
        else
        {
            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_VD0_AUX_TABLE_BASE_LOW);
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_VD0_AUX_TABLE_BASE_HIGH);
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_VD2_AUX_TABLE_BASE_LOW);
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = miItf.GetMmioInterfaces(mhw::mi::MHW_MMIO_VD2_AUX_TABLE_BASE_HIGH);
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = miItf.MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);
        }
    }
    return MOS_STATUS_SUCCESS;
}
