/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     codec_mem_compression_g12.cpp
//! \brief    Defines the common interface for codec mmc.
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of codec
//!

#include "codec_mem_compression_g12.h"
#include "mhw_mi_g12_X.h"

MOS_STATUS CodecMmcAuxTableG12::LoadAuxTableMmio(
    PMOS_INTERFACE    m_osInterface,
    MhwMiInterface    *m_mhwMiInterface,
    PMOS_COMMAND_BUFFER    cmdBuffer,
    bool    bRcsIsUsed)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, cmdBuffer);
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, m_mhwMiInterface);

    uint64_t auxTableBaseAddr = m_osInterface->pfnGetAuxTableBaseAddr(m_osInterface);

    if (0 != auxTableBaseAddr)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams;
        MOS_ZeroMemory(&lriParams, sizeof(lriParams));

        if (bRcsIsUsed)
        {
            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseLow;
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseHigh;
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseLow;
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseHigh;
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);
        }
        else
        {
            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd0AuxTableBaseLow;
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd0AuxTableBaseHigh;
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd2AuxTableBaseLow;
            lriParams.dwData     = (auxTableBaseAddr & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);

            lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVd2AuxTableBaseHigh;
            lriParams.dwData     = ((auxTableBaseAddr >> 32) & 0xffffffff);
            status               = m_mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams);
            MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MMC, MOS_MMC_SUBCOMP_SELF, status);
        }
    }
    return MOS_STATUS_SUCCESS;
}
