/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file      mhw_vebox_g11_X.h
//! \brief     Defines functions for constructing vebox commands on Gen11-based platforms

#ifndef __MHW_VEBOX_G11_X_H__
#define __MHW_VEBOX_G11_X_H__

#include "mhw_vebox_generic.h"
#include "mhw_vebox_hwcmd_g11_X.h"

#define MHW_VEBOX_TIMESTAMP_PER_TICK_IN_NS_G11              83.333f

#if (_DEBUG || _RELEASE_INTERNAL)
#define MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(keyval, VDId, shift, mask, bUseVD) \
do\
{\
    int32_t TmpVal = keyval;\
    while (TmpVal != 0) \
    {\
        if (((TmpVal) & (mask)) == (VDId))\
        {\
            bUseVD = true;\
            break;\
        }\
        TmpVal >>= (shift);\
    };\
}while(0)
#endif

class MhwVeboxInterfaceG11 : public MhwVeboxInterfaceGeneric<mhw_vebox_g11_X>
{
public:
    // Chroma parameters
    typedef struct _MHW_VEBOX_CHROMA_PARAMS
    {
        uint32_t  dwPixRangeThresholdChromaU[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwPixRangeWeightChromaU[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwPixRangeThresholdChromaV[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwPixRangeWeightChromaV[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwHotPixelThresholdChromaU;
        uint32_t  dwHotPixelCountChromaU;
        uint32_t  dwHotPixelThresholdChromaV;
        uint32_t  dwHotPixelCountChromaV;
    }MHW_VEBOX_CHROMA_PARAMS;

    MhwVeboxInterfaceG11(
        PMOS_INTERFACE pInputInterface);

    virtual ~MhwVeboxInterfaceG11() { MHW_FUNCTION_ENTER; }

    MOS_STATUS VeboxAdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
        uint32_t                  *pdwSurfaceWidth,
        uint32_t                  *pdwSurfaceHeight,
        bool                      bDIEnable);

    MOS_STATUS AddVeboxState(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams,
        bool                        bUseCmBuffer);

    virtual MOS_STATUS AddVeboxDiIecp(
        PMOS_COMMAND_BUFFER           pCmdBuffer,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams);

    MOS_STATUS AddVeboxDndiState(
        PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams);

    MOS_STATUS AddVeboxIecpState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams);

    MOS_STATUS AddVeboxGamutState(
        PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams);

    MOS_STATUS FindVeboxGpuNodeToUse(
        PMHW_VEBOX_GPUNODE_LIMIT pGpuNodeLimit);

    MOS_STATUS AddVeboxIecpAceState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams);

    MOS_STATUS SetVeboxIecpStateSTE(
        mhw_vebox_g11_X::VEBOX_STD_STE_STATE_CMD *pVeboxStdSteState,
        PMHW_COLORPIPE_PARAMS                     pColorPipeParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL(pVeboxStdSteState);
        MHW_CHK_NULL(pColorPipeParams);

        MhwVeboxInterfaceGeneric<mhw_vebox_g11_X>::SetVeboxIecpStateSTE(pVeboxStdSteState, pColorPipeParams);
        // Enable Skin Score Output surface to be written by Vebox
        pVeboxStdSteState->DW1.StdScoreOutput = pColorPipeParams->bEnableLACE && pColorPipeParams->LaceParams.bSTD;

    finish:
        return eStatus;
    }

    //!
    //! \brief      Add VEBOX Scalar States for Gen10+
    //! \details    Add vebox scalar states
    //! \param      [in] pVeboxScalarParams
    //!             Pointer to VEBOX Scalar State Params
    //! \return     MOS_STATUS
    //!
    MOS_STATUS AddVeboxScalarState(
        PMHW_VEBOX_SCALAR_PARAMS pVeboxScalarParams);

    //!
    //! \brief    Set which vebox can be used by HW
    //! \details  VPHAL set whehter the VEBOX can be use by HW
    //! \param    [in] inputVebox0;
    //!           set whether Vebox0 can be used by HW
    //! \param    [in] inputVebox01;
    //!           set whether vebox1 can be use by HW
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxInUse(
        bool inputVebox0,
        bool inputVebox1);

    //!
    //! \brief    Set which vebox can be used by DNDI
    //! \details  VPHAL set Chroma parameters can be use by DNDI
    //! \param    [in] chromaParams;
    //!           Chroma parameters which will be used by DNDI
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxChromaParams(
        MHW_VEBOX_CHROMA_PARAMS *chromaParams);

    //! \brief    get the status for vebox's Scalability.
    //! \details  VPHAL will check whether the veobx support Scalabiltiy. 
    //! \return   bool
    //!           false or true
    bool IsScalabilitySupported();

    virtual MOS_STATUS AddVeboxSurfaceControlBits(
        PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
        uint32_t                       *pSurfCtrlBits);

    //!
    //! \brief    Create Gpu Context for Vebox
    //! \details  Create Gpu Context for Vebox
    //! \param    [in] pOsInterface
    //!           OS interface
    //! \param    [in] VeboxGpuContext
    //!           Vebox Gpu Context
    //! \param    [in] VeboxGpuNode
    //!           Vebox Gpu Node
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateGpuContext(
        PMOS_INTERFACE  pOsInterface,
        MOS_GPU_CONTEXT VeboxGpuContext,
        MOS_GPU_NODE    VeboxGpuNode);

    virtual MOS_STATUS AdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
        uint32_t                  *pdwSurfaceWidth,
        uint32_t                  *pdwSurfaceHeight,
        bool                      bDIEnable);

private:
    uint32_t m_BT2020InvPixelValue[256];
    uint32_t m_BT2020FwdPixelValue[256];
    uint32_t m_BT2020InvGammaLUT[256];
    uint32_t m_BT2020FwdGammaLUT[256];
    bool  m_vebox0InUse;
    bool  m_vebox1InUse;
    bool  m_veboxScalabilitySupported;

    uint32_t                m_veboxSplitRatio;
    MHW_VEBOX_CHROMA_PARAMS m_chromaParams;

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
    void SetVeboxIecpStateBecsc(
        mhw_vebox_g11_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
        PMHW_VEBOX_IECP_PARAMS                 pVeboxIecpParams,
        bool                                   bEnableFECSC);
    //!
    //! \brief      Manual mode H2S
    //! \details    Manual mode H2S
    //              Based on WW23'16 algorithm design.
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \param      [in] pVeboxGamutParams
    //!             Pointer to VEBOX Gamut State Params
    //! \return     MOS_STATUS 
    //!
    MOS_STATUS VeboxInterface_H2SManualMode(
        PMHW_VEBOX_HEAP             pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams);

    //!
    //! \brief    Back end CSC setting, BT2020 YUV Full/Limited to BT2020 RGB Full (G10 +)
    //! \details  Back end CSC setting, BT2020 YUV Full/Limited to BT2020 RGB Full
    //! \param    [in] pVeboxIecpParams
    //!           Pointer to VEBOX IECP State Params
    //! \param    [in] pVeboxGamutParams
    //!           Pointer to VEBOX Gamut State Params
    //! \return   MOS_STATUS
    //!
    MOS_STATUS VeboxInterface_BT2020YUVToRGB(
        PMHW_VEBOX_HEAP             pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams);

    //!
    //! \brief      init VEBOX IECP States
    //! \details    init Vebox IECP states STD/E, ACE, TCC, Gamut 
    //! \param      [in]  pVeboxIecpStateCmd
    //!             Pointer to Vebox Interface Structure
    //! \return     viod
    //!
    void IecpStateInitialization(
        mhw_vebox_g11_X::VEBOX_IECP_STATE_CMD    *pVeboxIecpState);

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS ValidateVeboxScalabilityConfig();
#endif

    void SetVeboxSurfaces(
        PMHW_VEBOX_SURFACE_PARAMS                 pSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                 pDerivedSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                 pSkinScoreSurfaceParam,
        mhw_vebox_g11_X::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
        bool                                      bIsOutputSurface,
        bool                                      bDIEnable);
};

#endif
