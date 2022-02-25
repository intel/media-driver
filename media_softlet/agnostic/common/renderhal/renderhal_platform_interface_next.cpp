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
//! \file      renderhal_platform_interface_next.cpp
//! \brief     abstract the platfrom specific APIs into one class
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!

#include "renderhal_platform_interface_next.h"

MOS_STATUS XRenderHal_Platform_Interface_Next::AddPipelineSelectCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    bool                        gpGpuPipe)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    auto& par = m_renderItf->MHW_GETPAR_F(PIPELINE_SELECT)();
    par = {};
    par.gpGpuPipe = gpGpuPipe;
    m_renderItf->MHW_ADDCMD_F(PIPELINE_SELECT)(pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendStateBaseAddress(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(STATE_BASE_ADDRESS, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddSipStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(STATE_SIP, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::AddCfeStateCmd(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VFE_PARAMS             params)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    SETPAR_AND_ADDCMD(CFE_STATE, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendChromaKey(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus                        = MOS_STATUS_SUCCESS;
    PMHW_CHROMAKEY_PARAMS pChromaKeyParams    = nullptr;
    MEDIA_WA_TABLE               *pWaTable    = nullptr;
    MOS_GPU_CONTEXT       renderGpuContext    = {};
    MHW_PIPE_CONTROL_PARAMS PipeControlParams = {};

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    pChromaKeyParams = pRenderHal->ChromaKey;
    for (int32_t i = pRenderHal->iChromaKeyCount; i > 0; i--, pChromaKeyParams++)
    {
        MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
        pWaTable = pRenderHal->pOsInterface->pfnGetWaTable(pRenderHal->pOsInterface);
        MHW_RENDERHAL_CHK_NULL_RETURN(pWaTable);
        renderGpuContext = pRenderHal->pOsInterface->pfnGetGpuContext(pRenderHal->pOsInterface);

        // Program stalling pipecontrol with HDC pipeline flush enabled before programming 3DSTATE_CHROMA_KEY for CCS W/L.
        if ((renderGpuContext == MOS_GPU_CONTEXT_COMPUTE)    ||
            (renderGpuContext == MOS_GPU_CONTEXT_CM_COMPUTE) ||
            (renderGpuContext == MOS_GPU_CONTEXT_COMPUTE_RA))
        {
            if (MEDIA_IS_WA(pWaTable, Wa_16011481064))
            {
                MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
                PipeControlParams.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
                PipeControlParams.bGenericMediaStateClear       = true;
                PipeControlParams.bIndirectStatePointersDisable = true;
                PipeControlParams.bDisableCSStall               = false;
                PipeControlParams.bHdcPipelineFlush             = true;
                MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeControlParams));
            }
        }

        MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
        SETPAR_AND_ADDCMD(_3DSTATE_CHROMA_KEY, m_renderItf, pCmdBuffer);
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SendPalette(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus                     = MOS_STATUS_SUCCESS;
    PMHW_PALETTE_PARAMS pPaletteLoadParams = nullptr;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    pPaletteLoadParams = pRenderHal->Palette;
    for (int32_t i = pRenderHal->iMaxPalettes; i > 0; i--, pPaletteLoadParams++)
    {
        if (pPaletteLoadParams->iNumEntries > 0)
        {
            SETPAR_AND_ADDCMD(PALETTE_ENTRY, m_renderItf, pCmdBuffer);
        }
    }

    return eStatus;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::SetL3Cache(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS eStatus    = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwRenderInterface);
    m_renderHal = pRenderHal;
    m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);
    MHW_RENDERHAL_CHK_STATUS_RETURN(m_renderItf->SetL3Cache(pCmdBuffer, pRenderHal->pMhwMiInterface));

    return eStatus;
}

PMHW_MI_MMIOREGISTERS XRenderHal_Platform_Interface_Next::GetMmioRegisters(
    PRENDERHAL_INTERFACE        pRenderHal)
{
    PMHW_MI_MMIOREGISTERS     pMmioRegisters = nullptr;
    if (pRenderHal && pRenderHal->pMhwRenderInterface)
    {
        m_renderItf = std::static_pointer_cast<mhw::render::Itf>(pRenderHal->pMhwRenderInterface->GetNewRenderInterface());
    }

    if (m_renderItf)
    {
        pMmioRegisters = m_renderItf->GetMmioRegisters();
    }

    return pMmioRegisters;
}

MOS_STATUS XRenderHal_Platform_Interface_Next::EnablePreemption(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMOS_COMMAND_BUFFER             pCmdBuffer)
{
    MOS_STATUS eStatus              = MOS_STATUS_SUCCESS;
    MEDIA_FEATURE_TABLE* m_skuTable = nullptr;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterParams = {};
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);

    m_skuTable = pRenderHal->pOsInterface->pfnGetSkuTable(pRenderHal->pOsInterface);
    MHW_MI_CHK_NULL(m_skuTable);

    if (MEDIA_IS_SKU(m_skuTable, FtrPerCtxtPreemptionGranularityControl))
    {
        MOS_ZeroMemory(&loadRegisterParams, sizeof(loadRegisterParams));
        loadRegisterParams.dwRegister = 0;
        loadRegisterParams.dwData     = 0;
        MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &loadRegisterParams));
    }

    return eStatus;
}
