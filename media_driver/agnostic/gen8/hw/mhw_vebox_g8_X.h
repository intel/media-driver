/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_vebox_g8_X.h
//! \brief    Defines functions for constructing vebox commands on Gen8-based platforms

//!
#ifndef __MHW_VEBOX_G8_X_H__
#define __MHW_VEBOX_G8_X_H__

#include "mhw_vebox_generic.h"
#include "mhw_vebox_hwcmd_g8_X.h"

#define ABSMAXABSMIN_THRESHOLD_DEFAULT_G8        20
//!
//! \brief      VEBOX Settings for G8
//!
const MHW_VEBOX_SETTINGS g_Vebox_Settings_g8 =
{
    MHW_MAX_VEBOX_STATES,                                                     //!< uiNumInstances
    MHW_SYNC_SIZE,                                                            //!< uiSyncSize
    MHW_PAGE_SIZE,                                                            //!< uiDndiStateSize
    MHW_PAGE_SIZE,                                                            //!< uiIecpStateSize
    MHW_PAGE_SIZE,                                                            //!< uiGamutStateSize
    MHW_PAGE_SIZE,                                                            //!< uiVertexTableSize
    MHW_PAGE_SIZE,                                                            //!< uiCapturePipeStateSize
    0,                                                                        //!< uiGammaCorrectionStateSize
    0                                                                         //!< ui3DLUTSize
};

class MhwVeboxInterfaceG8 : public MhwVeboxInterfaceGeneric<mhw_vebox_g8_X>
{
public:
    MhwVeboxInterfaceG8(
        PMOS_INTERFACE pInputInterface);

     virtual ~MhwVeboxInterfaceG8() { MHW_FUNCTION_ENTER; }

     MOS_STATUS VeboxAdjustBoundary(
         PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
         uint32_t                  *pdwSurfaceWidth,
         uint32_t                  *pdwSurfaceHeight,
         bool                      bDIEnable);

     MOS_STATUS AddVeboxState(
         PMOS_COMMAND_BUFFER         pCmdBuffer,
         PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams,
         bool                        bUseCmBuffer);

     MOS_STATUS AddVeboxSurfaces(
         PMOS_COMMAND_BUFFER                 pCmdBuffer,
         PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams);

     MOS_STATUS AddVeboxDiIecp(
         PMOS_COMMAND_BUFFER           pCmdBuffer,
         PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams);

     MOS_STATUS AddVeboxDndiState(
         PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams);

     MOS_STATUS AddVeboxIecpState(
         PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams);

     MOS_STATUS AddVeboxGamutState(
         PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
         PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams);

     MOS_STATUS AddVeboxIecpAceState(
         PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams);

     MOS_STATUS GetVeboxAce_FullImageHistogram(
         uint32_t *pFullImageHistogram);

     MOS_STATUS AddVeboxSurfaceControlBits(
         PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
         uint32_t                       *pSurfCtrlBits)
     {
         MOS_STATUS eStatus = MOS_STATUS_PLATFORM_NOT_SUPPORTED;
         MHW_ASSERTMESSAGE("Unsupported on this platform.");
         return eStatus;
     }

private:

    void SetVeboxSurfaces(
        PMHW_VEBOX_SURFACE_PARAMS               pSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS               pDerivedSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS               pSkinScoreSurfaceParam,
        mhw_vebox_g8_X::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
        bool                                    bIsOutputSurface,
        bool                                    bDIEnable);

    //!
    //! \brief      Set Vebox Iecp State Back-End CSC 
    //! \details    Set Back-End CSC part of the VEBOX IECP States
    //! \param      [in] pVeboxIecpState
    //!             Pointer to VEBOX IECP States
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \param      [in] bEnableFECSC
    //!             Flag to enable FECSC
    //! \return     void
    //!
    void  SetVeboxIecpStateBecsc(
        mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
        PMHW_VEBOX_IECP_PARAMS               pVeboxIecpParams,
        bool                                 bEnableFECSC);

    void SetVeboxIecpStateFecsc(
        mhw_vebox_g8_X::VEBOX_FRONT_END_CSC_STATE_CMD *pVeboxFecscState,
        PMHW_VEBOX_IECP_PARAMS                        pVeboxIecpParams);

    void  SetVeboxIecpStateCcm(
        mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
        PMHW_CAPPIPE_PARAMS                  pCapPipeParams,
        const unsigned int                   uCoeffValue);

    //!
    //! \brief      init VEBOX IECP States 
    //! \details    init Vebox IECP states STD/E,ACE,TCC,AlphaAoi,
    //! \param      [in] pVeboxIecpStateCmd
    //!             Pointer to VEBOX_IECP_STATE_CMD Structure
    //! \return     void
    //!
    void IecpStateInitialization(
        mhw_vebox_g8_X::VEBOX_IECP_STATE_CMD  *pVeboxIecpState);

    //!
    //! \brief      init VEBOX Gammute State
    //! \details    init VEBOX Gammute State
    //! \param      [in] pGamutState
    //!             Pointer to VEBOX_GAMUT_STATE_CMD Structure
    //! \return     void
    //!
    void GamutStateInitialization(
        mhw_vebox_g8_X::VEBOX_GAMUT_STATE_CMD  *pGamutState);
};

#endif
