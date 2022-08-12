/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     vphal_render_vebox_xe_xpm.h
//! \brief    Interface and structure specific for Xe_XPM Vebox
//! \details  Interface and structure specific for Xe_XPM Vebox
//!
#ifndef __VPHAL_RENDER_VEBOX_XE_XPM_H__
#define __VPHAL_RENDER_VEBOX_XE_XPM_H__

#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_g12_base.h"
#include "vphal_render_vebox_xe_xpm_denoise.h"
#include "mhw_vebox_xe_xpm.h"

//!
//! \brief Macro for Vebox Scalable
//!
#define VPHAL_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET_G12 0x5C

typedef class VPHAL_VEBOX_STATE_XE_XPM *PVPHAL_VEBOX_STATE_XE_XPM;
class VPHAL_VEBOX_STATE_XE_XPM:virtual public VPHAL_VEBOX_STATE_G12_BASE
{
public:
    VPHAL_VEBOX_STATE_XE_XPM(
        PMOS_INTERFACE                 pOsInterface,
        PMHW_VEBOX_INTERFACE           pVeboxInterface,
        PMHW_SFC_INTERFACE             pSfcInterface,
        PRENDERHAL_INTERFACE           pRenderHal,
        PVPHAL_VEBOX_EXEC_STATE        pVeboxExecState,
        PVPHAL_RNDR_PERF_DATA          pPerfData,
        const VPHAL_DNDI_CACHE_CNTL    &dndiCacheCntl,
        MOS_STATUS                     *peStatus);

    virtual ~VPHAL_VEBOX_STATE_XE_XPM();

    //!
    //! \brief    Vebox is needed on Xe_XPM
    //! \details  Check if Vebox Render operation can be applied
    //! \param    [in] pcRenderParams
    //!           Pointer to VpHal render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   bool
    //!           return true if Vebox is needed, otherwise false
    //!
    virtual bool IsNeeded(
        PCVPHAL_RENDER_PARAMS pcRenderParams,
        RenderpassData       *pRenderPassData) override;
    virtual MOS_STATUS AllocateResources() override;
    virtual void FreeResources() override;

    // Vebox Scalability
    bool                            bVeboxScalableMode  = false;                     //!< Vebox Scalable Mode
    std::vector<PMOS_COMMAND_BUFFER> m_veCmdBuffers     = {};                        //!< Command Buffer for Vebox Scalable
    uint32_t                        dwVECmdBufSize      = 0;                         //!< Command Buffer Size
    MOS_RESOURCE                     VESemaMemS[MHW_VEBOX_MAX_SEMAPHORE_NUM_G12]    = {};   //!< Semphore for Vebox Scalable
    MOS_RESOURCE                     VESemaMemSAdd[MHW_VEBOX_MAX_SEMAPHORE_NUM_G12] = {};   //!< Semphore for Vebox Scalable addition
    uint32_t                        dwNumofVebox      = 0;                          //!< Number of Vebox
    bool                            bScalingHQPefMode = false;                      //!< set scalingHQ as perf mode

    // Vebox HVS
    VphalHVSDenoiserHpm             *m_hvsDenoiser    = nullptr;                    //!< Human Vision System Based Denoiser - Media Kernel to generate DN parameter

protected:

    virtual VphalSfcState* CreateSfcState() override;

    VPHAL_OUTPUT_PIPE_MODE  GetOutputPipe(
      PCVPHAL_RENDER_PARAMS           pcRenderParams,
      PVPHAL_SURFACE                  pSrcSurface,
      RenderpassData*                 pRenderData) override;

    virtual MOS_STATUS VeboxRenderVeboxCmd(
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
        VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
        MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
        MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams) override;

    virtual MOS_STATUS SetupDiIecpState(
        bool                            bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS   pVeboxDiIecpCmdParams) override;

    //!
    //! \brief    Initialize the Cmd buffer and insert prolog with VE params
    //! \details  Initialize the Cmd buffer and insert prolog with VE params
    //! \param    [in] pRenderHal
    //!           pointer to Renderhal interface
    //! \param    [in,out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [in] pGenericPrologParams
    //!           pointer to Generic prolog params struct to send to cmd buffer header
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitCmdBufferWithVeParams(
        PRENDERHAL_INTERFACE                    pRenderHal,
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

    //!
    //! \brief    Alloc Batch Buffers with VE interface
    //! \details  Allocate Batch Buffers with VE interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocVESecondaryCmdBuffers();

    //!
    //! \brief    Alloc Semaphore Resources with VE interface
    //! \details  Allocate Semaphore Resources with VE interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocVESemaphoreResources();

    //!
    //! \brief    Init Batch Buffers with VE interface
    //! \details  Init Batch Buffers with VE interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitVESecondaryCmdBuffers();

    //!
    //! \brief    Unlock Batch Buffers with VE interface
    //! \details  Unlock Batch Buffers with VE interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UnLockVESecondaryCmdBuffers();

    //!
    //! \brief    Free Resources with VE interface
    //! \details  Free Resources with VE interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeVEResources();

    virtual MOS_STATUS AddMiBatchBufferEnd(
        PMOS_INTERFACE      pOsInterface,
        PMHW_MI_INTERFACE   pMhwMiInterface,
        PMOS_COMMAND_BUFFER pCmdBufferInUse);

    virtual MOS_STATUS VeboxSetHVSDNParams(
        PVPHAL_DENOISE_PARAMS pDNParams);

    //!
    //! \brief    Get output surface of Vebox
    //! \details  Get output surface of Vebox in current operation
    //! \param    bDiVarianceEnable
    //!           [in] Is DI/Variances report enabled
    //! \return   PVPHAL_SURFACE
    //!           Corresponding output surface pointer
    //!
    virtual PVPHAL_SURFACE GetSurfOutput(
        bool    bDiVarianceEnable) override;

    // TGNE

    uint32_t dwGlobalNoiseLevel_Temporal  = 0;      //!< Global Temporal Noise Level for Y
    uint32_t dwGlobalNoiseLevelU_Temporal = 0;      //!< Global Temporal Noise Level for U
    uint32_t dwGlobalNoiseLevelV_Temporal = 0;      //!< Global Temporal Noise Level for V
    uint32_t curNoiseLevel_Temporal       = 0;      //!< Temporal Noise Level for Y
    uint32_t curNoiseLevelU_Temporal      = 0;      //!< Temporal Noise Level for U
    uint32_t curNoiseLevelV_Temporal      = 0;      //!< Temporal Noise Level for V
    bool     bTGNE_Valid                  = false;  //!< TGNE Flag of Valid
    bool     bTGNE_FirstFrame             = true;   //!< TGNE Flag of First Frame
    bool     m_bTgneEnable                = true;   //!< Enable TGNE for Denoise

};

#endif // __VPHAL_RENDER_VEBOX_XE_XPM_H__

